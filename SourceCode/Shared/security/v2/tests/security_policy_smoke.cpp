#include "security/v2/SecurityPolicy.h"

#include <cassert>

int main()
{
    security::v2::LegacyEquivalentSecurityPolicy policy;
    security::v2::SecurityFinding finding;
    finding.code = security::v2::SecurityFindingCode::MalformedLoginRequest;
    finding.category = security::v2::SecurityFindingCategory::InputValidation;
    finding.severity = security::v2::SecurityFindingSeverity::Critical;
    finding.source = security::v2::SecurityFindingSource::Authentication;
    finding.sessionId = 99;
    finding.evidenceCode = 7;
    assert(policy.Evaluate(finding) == security::v2::SecurityEnforcementAction::DisconnectAndEscalate);

    finding.code = security::v2::SecurityFindingCode::ClientFunctionMismatch;
    finding.severity = security::v2::SecurityFindingSeverity::Medium;
    assert(policy.Evaluate(finding) == security::v2::SecurityEnforcementAction::Audit);

    finding.severity = security::v2::SecurityFindingSeverity::High;
    assert(policy.Evaluate(finding) == security::v2::SecurityEnforcementAction::Disconnect);
    return 0;
}
