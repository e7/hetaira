#include "timer.h"


namespace sqlagent {
bool cmp_timer::operator ()(e7::common::smart_pointer<timer> a,
                            e7::common::smart_pointer<timer> b)
{
    return a->timeout > b->timeout;
}
} // end of namespace sqlagent
