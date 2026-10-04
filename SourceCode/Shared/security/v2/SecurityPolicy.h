#pragma once

#include "security/v2/SecurityFinding.h"

namespace security
{
namespace v2
{
enum class SecurityEnforcementAction
{
    Ignore = 0,
    Audit,
    Quarantine,
    Disconnect,
    DisconnectAndEscalate
};

class ISecurityPolicy
{
public:
    virtual ~ISecurityPolicy() {}
    virtual SecurityEnforcementAction Evaluate(const SecurityFinding& finding) const = 0;
};

// Migration policy intentionally mirrors current enforcement.  It is not a
// hardening policy: moving a check into V2 must not silently increase its
// sanction while the legacy behavior is being preserved.
class LegacyEquivalentSecurityPolicy final : public ISecurityPolicy
{
public:
    SecurityEnforcementAction Evaluate(const SecurityFinding& finding) const override
    {
        if(finding.code == SecurityFindingCode::MalformedLoginRequest)
            return SecurityEnforcementAction::DisconnectAndEscalate;

        switch(finding.severity)
        {
        case SecurityFindingSeverity::Informational:
        case SecurityFindingSeverity::Low:
        case SecurityFindingSeverity::Medium:
            return SecurityEnforcementAction::Audit;
        case SecurityFindingSeverity::High:
            return SecurityEnforcementAction::Disconnect;
        case SecurityFindingSeverity::Critical:
            return SecurityEnforcementAction::DisconnectAndEscalate;
        }
        return SecurityEnforcementAction::Audit;
    }
};
}
}
