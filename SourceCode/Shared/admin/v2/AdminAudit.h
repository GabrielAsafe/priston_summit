#pragma once

#include <string>

namespace gameplay::admin
{
struct AdminAuditEvent
{
    std::string command;
    int actorSecurityLevel = 0;
    int legacyRecordLevel = 0;
};

class AdminAuditService final
{
public:
    static AdminAuditEvent BuildEvent(const char* command,
                                      int actorSecurityLevel,
                                      int legacyRecordLevel)
    {
        AdminAuditEvent event;
        event.command = command ? command : "";
        event.actorSecurityLevel = actorSecurityLevel;
        event.legacyRecordLevel = legacyRecordLevel;
        return event;
    }
};
}
