#include <cassert>
#include "admin/v2/AdminAudit.h"

int main()
{
    const auto event = gameplay::admin::AdminAuditService::BuildEvent("/kick Hero", 4, 2);
    assert(event.command == "/kick Hero");
    assert(event.actorSecurityLevel == 4);
    assert(event.legacyRecordLevel == 2);

    const auto empty = gameplay::admin::AdminAuditService::BuildEvent(nullptr, 1, 3);
    assert(empty.command.empty());
    assert(empty.actorSecurityLevel == 1);
    assert(empty.legacyRecordLevel == 3);
    return 0;
}
