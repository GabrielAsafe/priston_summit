#pragma once

#include <cstdint>

#include "security/v2/SecurityFinding.h"

namespace security
{
namespace v2
{
struct LegacyCheatReportDecision
{
    SecurityFinding finding;
    bool recognized;
    bool banAccount;
    bool recordAudit;
    bool banIp;
    bool disconnect;

    LegacyCheatReportDecision()
        : recognized(false), banAccount(false), recordAudit(false), banIp(false), disconnect(false) {}
};

// Mirrors the existing FIND_CHEAT switch exactly.  This migration policy is
// deliberately not a hardening pass: known legacy sanctions stay unchanged,
// including the privileged-account exemption and the no-action wrong-version
// report.
class LegacyCheatReportPolicy final
{
public:
    LegacyCheatReportDecision Evaluate(std::uint32_t cheatId,
                                       bool privilegedAccount,
                                       std::uint64_t sessionId = 0u,
                                       std::uint64_t occurredAt = 0u) const
    {
        LegacyCheatReportDecision decision;
        decision.finding.category = SecurityFindingCategory::CodeIntegrity;
        decision.finding.source = SecurityFindingSource::AntiCheatClient;
        decision.finding.sessionId = sessionId;
        decision.finding.evidenceCode = cheatId;
        decision.finding.occurredAt = occurredAt;

        switch(cheatId)
        {
        case 8720u:
            MarkCritical(decision, SecurityFindingCode::ClientThreadHack);
            break;
        case 8954u:
            MarkCritical(decision, SecurityFindingCode::ClientChecksumMismatch);
            break;
        case 8955u:
            MarkCritical(decision, SecurityFindingCode::ClientWindowHack);
            break;
        case 8957u:
            MarkCritical(decision, SecurityFindingCode::ClientDebuggerDetected);
            break;
        case 8960u:
            MarkCritical(decision, SecurityFindingCode::ClientDefenseMultiplier);
            break;
        case 8961u:
            MarkCritical(decision, SecurityFindingCode::ClientRegenFormula);
            break;
        case 8962u:
            decision.recognized = true;
            decision.finding.code = SecurityFindingCode::ClientVersionMismatch;
            decision.finding.severity = SecurityFindingSeverity::Informational;
            // Legacy formats a message for this case but performs no audit,
            // ban or disconnect. Preserve that behavior during extraction.
            break;
        default:
            decision.finding.code = SecurityFindingCode::Unknown;
            decision.finding.severity = SecurityFindingSeverity::Informational;
            return decision;
        }

        if(privilegedAccount)
        {
            decision.banAccount = false;
            decision.recordAudit = false;
            decision.banIp = false;
            decision.disconnect = false;
        }

        return decision;
    }

private:
    static void MarkCritical(LegacyCheatReportDecision& decision, SecurityFindingCode code)
    {
        decision.recognized = true;
        decision.finding.code = code;
        decision.finding.severity = SecurityFindingSeverity::Critical;
        decision.banAccount = true;
        decision.recordAudit = true;
        decision.banIp = true;
        decision.disconnect = true;
    }
};
}
}
