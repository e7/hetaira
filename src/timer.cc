#include "timer.h"


namespace u2ush {
bool cmp_timer::operator ()(e7::common::smart_pointer<timer> a,
                            e7::common::smart_pointer<timer> b)
{
    return a->timeout > b->timeout;
}
} // end of namespace u2ush
