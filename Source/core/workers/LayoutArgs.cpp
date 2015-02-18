#include "config.h"
#include "core/workers/LayoutArgs.h"

namespace blink {

PassRefPtrWillBeRawPtr<LayoutArgs> LayoutArgs::create(double arg1, double arg2)
{
    return adoptRefWillBeNoop(new LayoutArgs(arg1, arg2));
}

LayoutArgs::LayoutArgs(double arg1, double arg2)
    : m_arg1(arg1)
    , m_arg2(arg2)
{
}

LayoutArgs::~LayoutArgs()
{
}

} // namespace blink
