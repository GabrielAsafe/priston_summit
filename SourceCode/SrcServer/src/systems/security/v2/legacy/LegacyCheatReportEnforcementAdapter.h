#pragma once

#include <string>

#include "security/v2/LegacyCheatReportPolicy.h"
#include "core/server/legacy_sin_runtime/sinLinkHeader.h"
#include "core/server/legacy_runtime/onserver.h"

bool recordCheatLog(rsPLAYINFO* player, const std::string& log);
bool banAccount(rsPLAYINFO* player);
void BannedIP(rsPLAYINFO* player);

namespace security::v2::legacy
{
class LegacyCheatReportEnforcementAdapter
{
public:
    void Apply(rsPLAYINFO* player,
               const LegacyCheatReportDecision& decision,
               const std::string& auditText) const
    {
        if(!player)
            return;

        if(decision.banAccount)
            (void)banAccount(player);

        if(decision.recordAudit)
            (void)recordCheatLog(player, auditText);

        if(decision.banIp)
            BannedIP(player);

        if(decision.disconnect && player->lpsmSock)
            (void)DisconnectUser(player->lpsmSock);
    }
};

inline const LegacyCheatReportEnforcementAdapter& ServerLegacyCheatReportEnforcementAdapter()
{
    static const LegacyCheatReportEnforcementAdapter adapter;
    return adapter;
}
}
