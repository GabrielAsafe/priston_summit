#pragma once

#include <string>
#include <utility>
#include <vector>

namespace gameplay::admin
{
struct AdminIdentity
{
    std::string accountId;
    std::string password;
    int securityLevel = 0;
    bool active = false;
};

class AdminAuthorizationService
{
public:
    void ReplaceAll(std::vector<AdminIdentity> identities)
    {
        identities_ = std::move(identities);
    }

    int Authenticate(const std::string& accountId,
                     const std::string& password,
                     int playerSecurityLevel) const noexcept
    {
        for (const auto& identity : identities_)
        {
            if (identity.accountId != accountId)
                continue;
            if (identity.password != password || !identity.active)
                return playerSecurityLevel;
            return identity.securityLevel;
        }
        return playerSecurityLevel;
    }

    int ResolveActiveSecurity(const std::string& accountId, int playerSecurityLevel) const noexcept
    {
        for (const auto& identity : identities_)
        {
            if (identity.accountId == accountId)
                return identity.active ? identity.securityLevel : playerSecurityLevel;
        }
        return playerSecurityLevel;
    }

private:
    std::vector<AdminIdentity> identities_;
};
}
