#include <cassert>
#include "authentication/v2/DuplicateLoginPolicy.h"

int main()
{
    using namespace authentication::v2;

    assert(DuplicateLoginPolicy::Evaluate({false, true, 100u, 70000u}) ==
           DuplicateLoginDecision::NoConflict);
    assert(DuplicateLoginPolicy::Evaluate({true, false, 100u, 70000u}) ==
           DuplicateLoginDecision::NoConflict);

    assert(DuplicateLoginPolicy::Evaluate({true, true, 1000u, 61000u}) ==
           DuplicateLoginDecision::RejectActiveSession);
    assert(DuplicateLoginPolicy::Evaluate({true, true, 1000u, 61001u}) ==
           DuplicateLoginDecision::RetireStaleAndRejectNewSession);
    return 0;
}
