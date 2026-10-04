#pragma once

#include "admin/v2/AdminAudit.h"
#include "core/server/legacy_sin_runtime/sinLinkHeader.h"

int rsRecordAdminCommand(rsPLAYINFO* lpPlayInfo, char* szChatCommand, int Level);

namespace gameplay::admin::legacy
{
class LegacyAdminAuditAdapter final
{
public:
    bool Record(rsPLAYINFO* player, const char* command, int legacyRecordLevel) const
    {
        if(!player)
            return false;
        const AdminAuditEvent event = AdminAuditService::BuildEvent(
            command, static_cast<int>(player->getSecurity()), legacyRecordLevel);
        return rsRecordAdminCommand(
            player, const_cast<char*>(event.command.c_str()), event.legacyRecordLevel) == TRUE;
    }
};

inline const LegacyAdminAuditAdapter& ServerLegacyAdminAuditAdapter()
{
    static const LegacyAdminAuditAdapter adapter;
    return adapter;
}
}
