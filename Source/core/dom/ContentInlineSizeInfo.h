#ifndef ContentInlineSizeInfo_h
#define ContentInlineSizeInfo_h

#include "bindings/core/v8/ScriptWrappable.h"
#include "platform/heap/Handle.h"
#include "wtf/PassRefPtr.h"
#include "wtf/RefCounted.h"

namespace blink {

class ContentInlineSizeInfo final : public RefCountedWillBeGarbageCollected<ContentInlineSizeInfo>, public ScriptWrappable {
    DEFINE_WRAPPERTYPEINFO();
public:
    static PassRefPtrWillBeRawPtr<ContentInlineSizeInfo> create(double min, double max)
    {
        return adoptRefWillBeNoop<ContentInlineSizeInfo>(new ContentInlineSizeInfo(min, max));
    }

    double minContentInlineSize() const { return m_min; }
    double maxContentInlineSize() const { return m_max; }

    void trace(Visitor* visitor) { }
private:
    ContentInlineSizeInfo(double min, double max)
        : m_min(min)
        , m_max(max)
    {
    }

    double m_min;
    double m_max;
};

} // namespace blink

#endif // ContentInlineSizeInfo_h
