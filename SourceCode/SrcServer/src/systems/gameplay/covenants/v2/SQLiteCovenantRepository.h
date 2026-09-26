#pragma once

#include "gameplay/covenants/v2/ICovenantRepository.h"

namespace gameplay
{
namespace covenants
{
class SQLiteCovenantRepository : public ICovenantRepository
{
public:
    bool Ready() { return EnsureSchema(); }
    bool LoadAll(const std::string& playerKey,
                 std::vector<CovenantMembershipState>& memberships);
    bool Save(const std::string& playerKey,
              const CovenantMembershipState& membership);
    bool TryJoin(const std::string& playerKey,
                 const CovenantMembershipState& membership,
                 bool targetExclusive,
                 const std::vector<CovenantId>& knownCovenantIds,
                 const std::vector<CovenantId>& exclusiveCovenantIds);
    bool SaveIfUnchanged(const std::string& playerKey,
                         const CovenantMembershipState& expected,
                         const CovenantMembershipState& membership);
    bool SwitchActive(const std::string& playerKey,
                      CovenantId targetCovenantId,
                      const std::vector<CovenantId>& knownCovenantIds);
    CovenantReputationSaveResult SaveReputationChange(
        const std::string& playerKey,
        const CovenantMembershipState& membership,
        int delta,
        const std::string& sourceKey);
    CovenantReputationSaveResult SaveReputationChangeChecked(
        const std::string& playerKey,
        const CovenantMembershipState& expected,
        const CovenantMembershipState& membership,
        int delta,
        const std::string& sourceKey);
    CovenantReputationSaveResult SaveRepeatableReputationChangeChecked(
        const std::string& playerKey,
        const CovenantMembershipState& expected,
        const CovenantMembershipState& membership,
        int delta);
    CovenantEffectClaimResult ClaimEffect(const std::string& playerKey,
                                          CovenantId covenantId,
                                          const std::string& effectKey);
    bool MarkEffectApplied(const std::string& playerKey,
                           CovenantId covenantId,
                           const std::string& effectKey);
    CovenantEffectReceiptLoadResult GetEffectReceiptStatus(
        const std::string& playerKey,
        CovenantId covenantId,
        const std::string& effectKey,
        CovenantEffectReceiptStatus& status);
    bool ListPendingEffects(const std::string& playerKey,
                            std::vector<CovenantEffectReceiptEntry>& receipts);
    bool AcknowledgePendingEffect(const std::string& playerKey,
                                  CovenantId covenantId,
                                  const std::string& effectKey);

private:
    bool EnsureSchema();
};
}
}
