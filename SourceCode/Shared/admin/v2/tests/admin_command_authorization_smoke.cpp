#include "Shared/admin/v2/AdminCommandAuthorization.h"
#include <cassert>
#include <iostream>

int main()
{
    using gameplay::admin::AdminCommandAuthorizationRequest;
    using gameplay::admin::AdminCommandAuthorizationService;
    assert(AdminCommandAuthorizationService::Allows({3, 2, false}));
    assert(AdminCommandAuthorizationService::Allows({2, 2, false}));
    assert(!AdminCommandAuthorizationService::Allows({1, 2, false}));
    assert(AdminCommandAuthorizationService::Allows({0, 4, true}));
    std::cout << "admin command authorization smoke: PASS\n";
    return 0;
}
