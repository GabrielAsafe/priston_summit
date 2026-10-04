#pragma once

#include <string>

#include "security/v2/SecurityPolicy.h"
#include "core/server/legacy_sin_runtime/sinLinkHeader.h"
#include "core/server/legacy_runtime/onserver.h"

bool recordAttack(rsPLAYINFO* player, const std::string& log);
bool banFirewall(rsPLAYINFO* player);

namespace security::v2::legacy
{
class LegacySecurityEnforcementAdapter
{
public:
    bool Apply(rsPLAYINFO* player,
               SecurityEnforcementAction action,
               const std::string& auditText) const
    {
        if (action == SecurityEnforcementAction::Ignore)
            return true;

        const bool audited = recordAttack(player, auditText);

        if (action == SecurityEnforcementAction::DisconnectAndEscalate)
            (void)banFirewall(player);

        if (player && player->lpsmSock &&
            (action == SecurityEnforcementAction::Disconnect ||
             action == SecurityEnforcementAction::DisconnectAndEscalate))
        {
            (void)DisconnectUser(player->lpsmSock);
        }

        return audited;
    }
};

inline const LegacySecurityEnforcementAdapter& ServerLegacySecurityEnforcementAdapter()
{
    static const LegacySecurityEnforcementAdapter adapter;
    return adapter;
}
}
