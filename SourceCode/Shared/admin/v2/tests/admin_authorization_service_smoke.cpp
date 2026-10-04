#include "Shared/admin/v2/AdminAuthorizationService.h"
#include <cassert>
#include <iostream>

int main()
{
    gameplay::admin::AdminAuthorizationService service;
    service.ReplaceAll({
        {"alice", "secret", 3, true},
        {"disabled", "secret", 4, false},
    });

    assert(service.Authenticate("alice", "secret", 0) == 3);
    assert(service.Authenticate("alice", "wrong", 0) == 0);
    assert(service.Authenticate("missing", "secret", 0) == 0);
    assert(service.Authenticate("disabled", "secret", 0) == 0);
    assert(service.ResolveActiveSecurity("alice", 0) == 3);
    assert(service.ResolveActiveSecurity("disabled", 0) == 0);

    service.ReplaceAll({{"bob", "pw", 2, true}});
    assert(service.ResolveActiveSecurity("alice", 0) == 0);
    assert(service.Authenticate("bob", "pw", 0) == 2);

    std::cout << "admin authorization service smoke: PASS\n";
    return 0;
}
