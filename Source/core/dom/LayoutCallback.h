#ifndef LayoutCallback_h
#define LayoutCallback_h

#include "platform/heap/Handle.h"

namespace blink {

class LayoutContext;

class LayoutCallback : public GarbageCollectedFinalized<LayoutCallback> {
public:
    virtual ~LayoutCallback() { }
    virtual void trace(Visitor*) { }
    virtual void handleEvent(LayoutContext*) = 0;
};

} // namespace blink

#endif // LayoutCallback_h
