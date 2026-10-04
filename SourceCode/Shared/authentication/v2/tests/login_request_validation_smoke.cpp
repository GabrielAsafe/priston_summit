#include "authentication/v2/LoginRequestValidation.h"

#include <cassert>
#include <cstring>

using authentication::v2::LoginRequestValidationCode;
using authentication::v2::LoginRequestValidationService;
using authentication::v2::LoginRequestView;

namespace
{
LoginRequestView Make(const char* account, const char* password, const char* hash, const char* mac)
{
    return LoginRequestView(account, 32u, password, 32u, hash, 33u, mac, 30u);
}
}

int main()
{
    LoginRequestValidationService service;
    const char* hash = "0123456789abcdef0123456789abcdef";
    assert(service.Validate(Make("account", "password", hash, "00-11-22")).Accepted());
    // Legacy schema did not mark account/password required; preserve that behavior.
    assert(service.Validate(Make("", "", hash, "x")).Accepted());

    assert(service.Validate(Make("a", "b", "short", "x")).code == LoginRequestValidationCode::HashLengthInvalid);
    assert(service.Validate(Make("a", "b", "", "x")).code == LoginRequestValidationCode::HashRequired);
    assert(service.Validate(Make("a", "b", hash, "")).code == LoginRequestValidationCode::MacRequired);

    char account[32];
    std::memset(account, 'A', sizeof(account));
    LoginRequestView unterminated(account, sizeof(account), "b", 2u, hash, 33u, "x", 2u);
    assert(service.Validate(unterminated).code == LoginRequestValidationCode::UnterminatedAccount);

    char mac[30];
    std::memset(mac, 'M', 29u);
    mac[29] = '\0';
    assert(service.Validate(LoginRequestView("a", 2u, "b", 2u, hash, 33u, mac, sizeof(mac))).Accepted());
    return 0;
}
