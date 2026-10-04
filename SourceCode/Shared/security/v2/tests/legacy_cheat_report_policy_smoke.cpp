#include <cassert>
#include <cstdint>

#include "security/v2/LegacyCheatReportPolicy.h"

int main()
{
    const security::v2::LegacyCheatReportPolicy policy;
    const std::uint32_t bannedIds[] = {8720u, 8954u, 8955u, 8957u, 8960u, 8961u};

    for(std::uint32_t id : bannedIds)
    {
        const auto decision = policy.Evaluate(id, false, 77u, 123u);
        assert(decision.recognized);
        assert(decision.finding.severity == security::v2::SecurityFindingSeverity::Critical);
        assert(decision.finding.source == security::v2::SecurityFindingSource::AntiCheatClient);
        assert(decision.finding.sessionId == 77u);
        assert(decision.finding.evidenceCode == id);
        assert(decision.finding.occurredAt == 123u);
        assert(decision.banAccount);
        assert(decision.recordAudit);
        assert(decision.banIp);
        assert(decision.disconnect);

        const auto privileged = policy.Evaluate(id, true);
        assert(privileged.recognized);
        assert(!privileged.banAccount);
        assert(!privileged.recordAudit);
        assert(!privileged.banIp);
        assert(!privileged.disconnect);
    }

    const auto version = policy.Evaluate(8962u, false);
    assert(version.recognized);
    assert(version.finding.code == security::v2::SecurityFindingCode::ClientVersionMismatch);
    assert(version.finding.severity == security::v2::SecurityFindingSeverity::Informational);
    assert(!version.banAccount && !version.recordAudit && !version.banIp && !version.disconnect);

    const auto unknown = policy.Evaluate(123u, false);
    assert(!unknown.recognized);
    assert(!unknown.banAccount && !unknown.recordAudit && !unknown.banIp && !unknown.disconnect);

    return 0;
}
