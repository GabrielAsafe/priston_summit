#pragma once

#include <cstdint>

namespace security
{
namespace v2
{
enum class SecurityFindingCategory
{
    InputValidation = 0,
    CodeIntegrity,
    ProcessIntegrity,
    Timing,
    PacketValidation,
    InventoryIntegrity,
    ThirdPartyProbe
};

enum class SecurityFindingSeverity
{
    Informational = 0,
    Low,
    Medium,
    High,
    Critical
};

enum class SecurityFindingCode
{
    Unknown = 0,
    MalformedLoginRequest,
    ClientFunctionMismatch,
    ProcessSignatureMatched,
    PacketMalformed,
    InventoryMismatch,
    ClientThreadHack,
    ClientChecksumMismatch,
    ClientWindowHack,
    ClientDebuggerDetected,
    ClientDefenseMultiplier,
    ClientRegenFormula,
    ClientVersionMismatch
};

enum class SecurityFindingSource
{
    Authentication = 0,
    IntegrityProbe,
    AntiCheatClient,
    PacketFilter,
    InventoryRuntime
};

struct SecurityFinding
{
    SecurityFindingCode code;
    SecurityFindingCategory category;
    SecurityFindingSeverity severity;
    SecurityFindingSource source;
    std::uint64_t sessionId;
    std::uint32_t evidenceCode;
    std::uint64_t evidenceValue;
    std::uint64_t occurredAt;

    SecurityFinding()
        : code(SecurityFindingCode::Unknown), category(SecurityFindingCategory::InputValidation),
          severity(SecurityFindingSeverity::Informational), source(SecurityFindingSource::Authentication),
          sessionId(0u), evidenceCode(0u), evidenceValue(0u), occurredAt(0u) {}
};
}
}
