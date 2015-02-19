#ifndef LayoutArgs_h
#define LayoutArgs_h

#include "bindings/core/v8/ScriptWrappable.h"
#include "wtf/RefPtr.h"

namespace blink {

class LayoutArgs : public RefCountedWillBeGarbageCollectedFinalized<LayoutArgs>, public ScriptWrappable {
    DEFINE_WRAPPERTYPEINFO();
public:
    static PassRefPtrWillBeRawPtr<LayoutArgs> create(double arg1, double arg2);
    virtual ~LayoutArgs();

    double arg1() const { return m_arg1; }
    double arg2() const { return m_arg2; }

private:
    LayoutArgs(double arg1, double arg2);

    double m_arg1;
    double m_arg2;

};

} // namespace blink

#endif // LayoutArgs_h
