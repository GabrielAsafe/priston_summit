#pragma once

namespace gameplay::admin
{
struct AdminCommandAuthorizationRequest
{
    int actualSecurityLevel = 0;
    int requiredSecurityLevel = 0;
    bool legacyBypass = false;
};

class AdminCommandAuthorizationService
{
public:
    static bool Allows(const AdminCommandAuthorizationRequest& request) noexcept
    {
        return request.legacyBypass || request.actualSecurityLevel >= request.requiredSecurityLevel;
    }
};
}
