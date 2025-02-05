/*
 * Copyright (C) 2009 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "bindings/core/v8/V8ScriptRunner.h"

#include "bindings/core/v8/ScriptSourceCode.h"
#include "bindings/core/v8/ScriptStreamer.h"
#include "bindings/core/v8/V8Binding.h"
#include "bindings/core/v8/V8GCController.h"
#include "bindings/core/v8/V8RecursionScope.h"
#include "bindings/core/v8/V8ThrowException.h"
#include "core/dom/ExecutionContext.h"
#include "core/fetch/CachedMetadata.h"
#include "core/fetch/ScriptResource.h"
#include "platform/ScriptForbiddenScope.h"
#include "platform/TraceEvent.h"
#include "public/platform/Platform.h"
#include "third_party/snappy/src/snappy.h"
#include "wtf/CurrentTime.h"

#if defined(WTF_OS_WIN)
#include <malloc.h>
#else
#include <alloca.h>
#endif

namespace blink {

namespace {

const int kMaxRecursionDepth = 22;

class V8CompileHistogram {
public:
    enum Cacheability { Cacheable, Noncacheable };
    explicit V8CompileHistogram(Cacheability);
    ~V8CompileHistogram();

private:
    Cacheability m_cacheability;
    double m_timeStamp;
};

V8CompileHistogram::V8CompileHistogram(V8CompileHistogram::Cacheability cacheability)
    : m_cacheability(cacheability)
    , m_timeStamp(WTF::currentTime())
{
}

V8CompileHistogram::~V8CompileHistogram()
{
    int64_t elapsedMicroSeconds = static_cast<int64_t>((WTF::currentTime() - m_timeStamp) * 1000000);
    const char* name = (m_cacheability == Cacheable) ? "V8.CompileCacheableMicroSeconds" : "V8.CompileNoncacheableMicroSeconds";
    blink::Platform::current()->histogramCustomCounts(name, elapsedMicroSeconds, 0, 1000000, 50);
}

// In order to make sure all pending messages to be processed in
// v8::Function::Call, we don't call handleMaxRecursionDepthExceeded
// directly. Instead, we create a v8::Function of
// throwStackOverflowException and call it.
void throwStackOverflowException(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    V8ThrowException::throwRangeError(info.GetIsolate(), "Maximum call stack size exceeded.");
}

v8::Local<v8::Value> throwStackOverflowExceptionIfNeeded(v8::Isolate* isolate)
{
    if (V8PerIsolateData::from(isolate)->isHandlingRecursionLevelError()) {
        // If we are already handling a recursion level error, we should
        // not invoke v8::Function::Call.
        return v8::Undefined(isolate);
    }
    V8PerIsolateData::from(isolate)->setIsHandlingRecursionLevelError(true);
    v8::Local<v8::Value> result = v8::Function::New(isolate, throwStackOverflowException)->Call(v8::Undefined(isolate), 0, 0);
    V8PerIsolateData::from(isolate)->setIsHandlingRecursionLevelError(false);
    return result;
}

// Compile a script without any caching or compile options.
v8::Local<v8::Script> compileWithoutOptions(V8CompileHistogram::Cacheability cacheability, v8::Isolate* isolate, v8::Handle<v8::String> code, v8::ScriptOrigin origin)
{
    V8CompileHistogram histogramScope(cacheability);
    v8::ScriptCompiler::Source source(code, origin);
    return v8::ScriptCompiler::Compile(isolate, &source, v8::ScriptCompiler::kNoCompileOptions);
}

// Compile a script, and consume a V8 cache that was generated previously.
v8::Local<v8::Script> compileAndConsumeCache(ScriptResource* resource, unsigned tag, v8::ScriptCompiler::CompileOptions compileOptions, bool compressed, v8::Isolate* isolate, v8::Handle<v8::String> code, v8::ScriptOrigin origin)
{
    V8CompileHistogram histogramScope(V8CompileHistogram::Cacheable);
    CachedMetadata* cachedMetadata = resource->cachedMetadata(tag);
    const char* data = cachedMetadata->data();
    int length = cachedMetadata->size();
    std::string uncompressedOutput;
    bool invalidCache = false;
    if (compressed) {
        if (snappy::Uncompress(data, length, &uncompressedOutput)) {
            data = uncompressedOutput.data();
            length = uncompressedOutput.length();
        } else {
            invalidCache = true;
        }
    }
    v8::Local<v8::Script> script;
    if (invalidCache) {
        v8::ScriptCompiler::Source source(code, origin);
        script = v8::ScriptCompiler::Compile(isolate, &source, v8::ScriptCompiler::kNoCompileOptions);
    } else {
        v8::ScriptCompiler::CachedData* cachedData = new v8::ScriptCompiler::CachedData(
            reinterpret_cast<const uint8_t*>(data), length, v8::ScriptCompiler::CachedData::BufferNotOwned);
        v8::ScriptCompiler::Source source(code, origin, cachedData);
        script = v8::ScriptCompiler::Compile(isolate, &source, compileOptions);
        invalidCache = cachedData->rejected;
    }
    if (invalidCache)
        resource->clearCachedMetadata(Resource::SendToPlatform);
    return script;
}

// Compile a script, and produce a V8 cache for future use.
v8::Local<v8::Script> compileAndProduceCache(ScriptResource* resource, unsigned tag, v8::ScriptCompiler::CompileOptions compileOptions, bool compressed, Resource::MetadataCacheType cacheType, v8::Isolate* isolate, v8::Handle<v8::String> code, v8::ScriptOrigin origin)
{
    V8CompileHistogram histogramScope(V8CompileHistogram::Cacheable);
    v8::ScriptCompiler::Source source(code, origin);
    v8::Local<v8::Script> script = v8::ScriptCompiler::Compile(isolate, &source, compileOptions);
    const v8::ScriptCompiler::CachedData* cachedData = source.GetCachedData();
    if (cachedData) {
        const char* data = reinterpret_cast<const char*>(cachedData->data);
        int length = cachedData->length;
        std::string compressedOutput;
        if (compressed) {
            snappy::Compress(data, length, &compressedOutput);
            data = compressedOutput.data();
            length = compressedOutput.length();
        }
        if (length > 1024) {
            // Omit histogram samples for small cache data to avoid outliers.
            int cacheSizeRatio = static_cast<int>(100.0 * length / code->Length());
            blink::Platform::current()->histogramCustomCounts("V8.CodeCacheSizeRatio", cacheSizeRatio, 0, 10000, 50);
        }
        resource->clearCachedMetadata();
        resource->setCachedMetadata(tag, data, length, cacheType);
    }
    return script;
}

// Compile a script, and consume or produce a V8 Cache, depending on whether the
// given resource already has cached data available.
v8::Local<v8::Script> compileAndConsumeOrProduce(ScriptResource* resource, unsigned tag, v8::ScriptCompiler::CompileOptions consumeOptions, v8::ScriptCompiler::CompileOptions produceOptions, bool compressed, Resource::MetadataCacheType cacheType, v8::Isolate* isolate, v8::Handle<v8::String> code, v8::ScriptOrigin origin)
{
    return resource->cachedMetadata(tag)
        ? compileAndConsumeCache(resource, tag, consumeOptions, compressed, isolate, code, origin)
        : compileAndProduceCache(resource, tag, produceOptions, compressed, cacheType, isolate, code, origin);
}

enum CacheTagKind {
    CacheTagParser = 0,
    CacheTagCode = 1,
    CacheTagCodeCompressed = 2,
    CacheTagTimeStamp = 3,
    CacheTagLast
};

static const int kCacheTagKindSize = 2;

unsigned cacheTag(CacheTagKind kind, Resource* resource)
{
    static_assert((1 << kCacheTagKindSize) >= CacheTagLast, "CacheTagLast must be large enough");

    static unsigned v8CacheDataVersion = v8::ScriptCompiler::CachedDataVersionTag() << kCacheTagKindSize;

    // A script can be (successfully) interpreted with different encodings,
    // depending on the page it appears in. The cache doesn't know anything
    // about encodings, but the cached data is specific to one encoding. If we
    // later load the script from the cache and interpret it with a different
    // encoding, the cached data is not valid for that encoding.
    return (v8CacheDataVersion | kind) + StringHash::hash(resource->encoding());
}

// Store a timestamp to the cache as hint.
void setCacheTimeStamp(ScriptResource* resource)
{
    double now = WTF::currentTime();
    unsigned tag = cacheTag(CacheTagTimeStamp, resource);
    resource->setCachedMetadata(tag, reinterpret_cast<char*>(&now), sizeof(now), Resource::SendToPlatform);
}

// Check previously stored timestamp.
bool isResourceHotForCaching(ScriptResource* resource, int hotHours)
{
    const double kCacheWithinSeconds = hotHours * 60 * 60;
    unsigned tag = cacheTag(CacheTagTimeStamp, resource);
    CachedMetadata* cachedMetadata = resource->cachedMetadata(tag);
    if (!cachedMetadata)
        return false;
    double timeStamp;
    const int size = sizeof(timeStamp);
    ASSERT(cachedMetadata->size() == size);
    memcpy(&timeStamp, cachedMetadata->data(), size);
    return (WTF::currentTime() - timeStamp) < kCacheWithinSeconds;
}

// Final compile call for a streamed compilation. Most decisions have already
// been made, but we need to write back data into the cache.
v8::Local<v8::Script> postStreamCompile(ScriptResource* resource, ScriptStreamer* streamer, v8::Isolate* isolate, v8::Handle<v8::String> code, v8::ScriptOrigin origin)
{
    V8CompileHistogram histogramScope(V8CompileHistogram::Noncacheable);
    v8::Local<v8::Script> script = v8::ScriptCompiler::Compile(isolate, streamer->source(), code, origin);

    // Whether to produce the cached data or not is decided when the
    // streamer is started. Here we only need to get the data out.
    const v8::ScriptCompiler::CachedData* newCachedData = streamer->source()->GetCachedData();
    if (newCachedData) {
        resource->clearCachedMetadata();
        v8::ScriptCompiler::CompileOptions options = streamer->compileOptions();
        switch (options) {
        case v8::ScriptCompiler::kProduceParserCache:
            resource->setCachedMetadata(cacheTag(CacheTagParser, resource), reinterpret_cast<const char*>(newCachedData->data), newCachedData->length, Resource::CacheLocally);
            break;
        case v8::ScriptCompiler::kProduceCodeCache:
            resource->setCachedMetadata(cacheTag(CacheTagCode, resource), reinterpret_cast<const char*>(newCachedData->data), newCachedData->length, Resource::SendToPlatform);
            break;
        default:
            break;
        }
    }

    return script;
}

typedef Function<v8::Local<v8::Script>(v8::Isolate*, v8::Handle<v8::String>, v8::ScriptOrigin)> CompileFn;

// A notation convenience: WTF::bind<...> needs to be given the right argument
// types. We have an awful lot of bind calls below, all with the same types, so
// this local bind lets WTF::bind to all the work, but 'knows' the right
// parameter types.
// This version isn't quite as smart as the real WTF::bind, though, so you
// sometimes may still have to call the original.
template<typename... A>
PassOwnPtr<CompileFn> bind(const A&... args)
{
    return WTF::bind<v8::Isolate*, v8::Handle<v8::String>, v8::ScriptOrigin>(args...);
}

// Select a compile function from any of the above, mainly depending on
// cacheOptions.
PassOwnPtr<CompileFn> selectCompileFunction(V8CacheOptions cacheOptions, ScriptResource* resource, v8::Handle<v8::String> code)
{
    static const int minimalCodeLength = 1024;

    if (!resource || !resource->url().protocolIsInHTTPFamily())
        // Caching is not available in this case.
        return bind(compileWithoutOptions, V8CompileHistogram::Noncacheable);

    if (cacheOptions == V8CacheOptionsNone)
        return bind(compileWithoutOptions, V8CompileHistogram::Cacheable);

    // The cacheOptions will guide our strategy:
    // FIXME: Clean up code caching options. crbug.com/455187.
    switch (cacheOptions) {
    case V8CacheOptionsDefault:
    case V8CacheOptionsParseMemory:
        if (code->Length() < minimalCodeLength) {
            // Do not cache for small scripts, though caching is available.
            return bind(compileWithoutOptions, V8CompileHistogram::Cacheable);
        }
        // Use parser-cache; in-memory only.
        return bind(compileAndConsumeOrProduce, resource, cacheTag(CacheTagParser, resource), v8::ScriptCompiler::kConsumeParserCache, v8::ScriptCompiler::kProduceParserCache, false, Resource::CacheLocally);
        break;

    case V8CacheOptionsParse:
        // Use parser-cache.
        return bind(compileAndConsumeOrProduce, resource, cacheTag(CacheTagParser, resource), v8::ScriptCompiler::kConsumeParserCache, v8::ScriptCompiler::kProduceParserCache, false, Resource::SendToPlatform);
        break;

    case V8CacheOptionsHeuristicsDefault:
    case V8CacheOptionsCode:
        // Always use code caching.
        return bind(compileAndConsumeOrProduce, resource, cacheTag(CacheTagCode, resource), v8::ScriptCompiler::kConsumeCodeCache, v8::ScriptCompiler::kProduceCodeCache, false, Resource::SendToPlatform);
        break;

    case V8CacheOptionsHeuristicsDefaultMobile:
    case V8CacheOptionsCodeCompressed:
        // Always use code caching. Compress depending on cacheOptions.
        return bind(compileAndConsumeOrProduce, resource, cacheTag(CacheTagCodeCompressed, resource), v8::ScriptCompiler::kConsumeCodeCache, v8::ScriptCompiler::kProduceCodeCache, true, Resource::SendToPlatform);
        break;

    case V8CacheOptionsHeuristics:
    case V8CacheOptionsHeuristicsMobile:
    case V8CacheOptionsRecent:
    case V8CacheOptionsRecentSmall: {
        bool compress = (cacheOptions == V8CacheOptionsRecentSmall || cacheOptions == V8CacheOptionsHeuristicsMobile);
        unsigned codeCacheTag = cacheTag(compress ? CacheTagCodeCompressed : CacheTagCode, resource);
        CachedMetadata* codeCache = resource->cachedMetadata(codeCacheTag);
        if (codeCache)
            return bind(compileAndConsumeCache, resource, codeCacheTag, v8::ScriptCompiler::kConsumeCodeCache, compress);
        int hotHours = (cacheOptions == V8CacheOptionsRecent || cacheOptions == V8CacheOptionsRecentSmall) ? 36 : 72;
        if (!isResourceHotForCaching(resource, hotHours)) {
            setCacheTimeStamp(resource);
            return bind(compileWithoutOptions, V8CompileHistogram::Cacheable);
        }
        return bind(compileAndProduceCache, resource, codeCacheTag, v8::ScriptCompiler::kProduceCodeCache, compress, Resource::SendToPlatform);
        break;
    }

    case V8CacheOptionsNone:
        // Shouldn't happen, as this is handled above.
        // Case is here so that compiler can check all cases are handled.
        ASSERT_NOT_REACHED();
        break;
    }

    // All switch branches should return and we should never get here.
    // But some compilers aren't sure, hence this default.
    ASSERT_NOT_REACHED();
    return bind(compileWithoutOptions, V8CompileHistogram::Cacheable);
}

// Select a compile function for a streaming compile.
PassOwnPtr<CompileFn> selectCompileFunction(ScriptResource* resource, ScriptStreamer* streamer)
{
    // We don't stream scripts which don't have a Resource.
    ASSERT(resource);
    // Failed resources should never get this far.
    ASSERT(!resource->errorOccurred());
    ASSERT(streamer->isFinished());
    ASSERT(!streamer->streamingSuppressed());
    return WTF::bind<v8::Isolate*, v8::Handle<v8::String>, v8::ScriptOrigin>(postStreamCompile, resource, streamer);
}
} // namespace

v8::Local<v8::Script> V8ScriptRunner::compileScript(const ScriptSourceCode& source, v8::Isolate* isolate, AccessControlStatus corsStatus, V8CacheOptions cacheOptions)
{
    return compileScript(v8String(isolate, source.source()), source.url(), source.startPosition(), source.resource(), source.streamer(), isolate, corsStatus, cacheOptions);
}

v8::Local<v8::Script> V8ScriptRunner::compileScript(v8::Handle<v8::String> code, const String& fileName, const TextPosition& scriptStartPosition, ScriptResource* resource, ScriptStreamer* streamer, v8::Isolate* isolate, AccessControlStatus corsStatus, V8CacheOptions cacheOptions, bool isInternalScript)
{
    TRACE_EVENT1("v8", "v8.compile", "fileName", fileName.utf8());
    TRACE_EVENT_SCOPED_SAMPLING_STATE("v8", "V8Compile");

    // NOTE: For compatibility with WebCore, ScriptSourceCode's line starts at
    // 1, whereas v8 starts at 0.
    v8::ScriptOrigin origin(
        v8String(isolate, fileName),
        v8::Integer::New(isolate, scriptStartPosition.m_line.zeroBasedInt()),
        v8::Integer::New(isolate, scriptStartPosition.m_column.zeroBasedInt()),
        v8Boolean(corsStatus == SharableCrossOrigin, isolate),
        v8::Handle<v8::Integer>(),
        v8Boolean(isInternalScript, isolate));

    OwnPtr<CompileFn> compileFn = streamer
        ? selectCompileFunction(resource, streamer)
        : selectCompileFunction(cacheOptions, resource, code);

    return (*compileFn)(isolate, code, origin);
}

v8::Local<v8::Value> V8ScriptRunner::runCompiledScript(v8::Isolate* isolate, v8::Handle<v8::Script> script, ExecutionContext* context)
{
    if (script.IsEmpty())
        return v8::Local<v8::Value>();
    TRACE_EVENT_SCOPED_SAMPLING_STATE("v8", "V8Execution");
    TRACE_EVENT1("v8", "v8.run", "fileName", TRACE_STR_COPY(*v8::String::Utf8Value(script->GetUnboundScript()->GetScriptName())));

    if (V8RecursionScope::recursionLevel(isolate) >= kMaxRecursionDepth)
        return throwStackOverflowExceptionIfNeeded(isolate);

    RELEASE_ASSERT(!context->isIteratingOverObservers());

    // Run the script and keep track of the current recursion depth.
    v8::Local<v8::Value> result;
    {
        if (ScriptForbiddenScope::isScriptForbidden())
            return v8::Local<v8::Value>();
        V8RecursionScope recursionScope(isolate);
        result = script->Run();
    }

    if (result.IsEmpty())
        return v8::Local<v8::Value>();

    crashIfV8IsDead();
    return result;
}

v8::Local<v8::Value> V8ScriptRunner::compileAndRunInternalScript(v8::Handle<v8::String> source, v8::Isolate* isolate, const String& fileName, const TextPosition& scriptStartPosition)
{
    v8::Handle<v8::Script> script = V8ScriptRunner::compileScript(source, fileName, scriptStartPosition, 0, 0, isolate, SharableCrossOrigin, V8CacheOptionsDefault, true);
    if (script.IsEmpty())
        return v8::Local<v8::Value>();

    TRACE_EVENT0("v8", "v8.run");
    TRACE_EVENT_SCOPED_SAMPLING_STATE("v8", "V8Execution");
    V8RecursionScope::MicrotaskSuppression recursionScope(isolate);
    v8::Local<v8::Value> result = script->Run();
    crashIfV8IsDead();
    return result;
}

v8::Local<v8::Value> V8ScriptRunner::runCompiledInternalScript(v8::Isolate* isolate, v8::Handle<v8::Script> script)
{
    TRACE_EVENT0("v8", "v8.run");
    TRACE_EVENT_SCOPED_SAMPLING_STATE("v8", "V8Execution");
    V8RecursionScope::MicrotaskSuppression recursionScope(isolate);
    v8::Local<v8::Value> result = script->Run();
    crashIfV8IsDead();
    return result;
}

v8::Local<v8::Value> V8ScriptRunner::callFunction(v8::Handle<v8::Function> function, ExecutionContext* context, v8::Handle<v8::Value> receiver, int argc, v8::Handle<v8::Value> args[], v8::Isolate* isolate)
{
    TRACE_EVENT0("v8", "v8.callFunction");
    TRACE_EVENT_SCOPED_SAMPLING_STATE("v8", "V8Execution");

    if (V8RecursionScope::recursionLevel(isolate) >= kMaxRecursionDepth)
        return throwStackOverflowExceptionIfNeeded(isolate);

    RELEASE_ASSERT(!context->isIteratingOverObservers());

    if (ScriptForbiddenScope::isScriptForbidden())
        return v8::Local<v8::Value>();
    V8RecursionScope recursionScope(isolate);
    v8::Local<v8::Value> result = function->Call(receiver, argc, args);
    crashIfV8IsDead();
    return result;
}

v8::Local<v8::Value> V8ScriptRunner::callInternalFunction(v8::Handle<v8::Function> function, v8::Handle<v8::Value> receiver, int argc, v8::Handle<v8::Value> args[], v8::Isolate* isolate)
{
    TRACE_EVENT0("v8", "v8.callFunction");
    TRACE_EVENT_SCOPED_SAMPLING_STATE("v8", "V8Execution");
    V8RecursionScope::MicrotaskSuppression recursionScope(isolate);
    v8::Local<v8::Value> result = function->Call(receiver, argc, args);
    crashIfV8IsDead();
    return result;
}

v8::Local<v8::Value> V8ScriptRunner::callAsFunction(v8::Isolate* isolate, v8::Handle<v8::Object> object, v8::Handle<v8::Value> receiver, int argc, v8::Handle<v8::Value> args[])
{
    TRACE_EVENT0("v8", "v8.callFunction");
    TRACE_EVENT_SCOPED_SAMPLING_STATE("v8", "V8Execution");

    V8RecursionScope::MicrotaskSuppression recursionScope(isolate);
    v8::Local<v8::Value> result = object->CallAsFunction(receiver, argc, args);
    crashIfV8IsDead();
    return result;
}

v8::Local<v8::Object> V8ScriptRunner::instantiateObject(v8::Isolate* isolate, v8::Handle<v8::ObjectTemplate> objectTemplate)
{
    TRACE_EVENT0("v8", "v8.newInstance");
    TRACE_EVENT_SCOPED_SAMPLING_STATE("v8", "V8Execution");

    V8RecursionScope::MicrotaskSuppression scope(isolate);
    v8::Local<v8::Object> result = objectTemplate->NewInstance();
    crashIfV8IsDead();
    return result;
}

v8::Local<v8::Object> V8ScriptRunner::instantiateObject(v8::Isolate* isolate, v8::Handle<v8::Function> function, int argc, v8::Handle<v8::Value> argv[])
{
    TRACE_EVENT0("v8", "v8.newInstance");
    TRACE_EVENT_SCOPED_SAMPLING_STATE("v8", "V8Execution");

    V8RecursionScope::MicrotaskSuppression scope(isolate);
    v8::Local<v8::Object> result = function->NewInstance(argc, argv);
    crashIfV8IsDead();
    return result;
}

v8::Local<v8::Object> V8ScriptRunner::instantiateObjectInDocument(v8::Isolate* isolate, v8::Handle<v8::Function> function, ExecutionContext* context, int argc, v8::Handle<v8::Value> argv[])
{
    TRACE_EVENT0("v8", "v8.newInstance");
    TRACE_EVENT_SCOPED_SAMPLING_STATE("v8", "V8Execution");
    if (ScriptForbiddenScope::isScriptForbidden())
        return v8::Local<v8::Object>();
    V8RecursionScope scope(isolate);
    v8::Local<v8::Object> result = function->NewInstance(argc, argv);
    crashIfV8IsDead();
    return result;
}

unsigned V8ScriptRunner::tagForParserCache(Resource* resource)
{
    return cacheTag(CacheTagParser, resource);
}

unsigned V8ScriptRunner::tagForCodeCache(Resource* resource)
{
    return cacheTag(CacheTagCode, resource);
}

} // namespace blink
