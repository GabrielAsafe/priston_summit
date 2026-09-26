#pragma once

#include <string>
#include <vector>

#include "gameplay/covenants/v2/CovenantTypes.h"

namespace gameplay
{
namespace covenants
{
enum class CovenantReputationSaveResult
{
    Applied,
    AlreadyApplied,
    Conflict,
    Failed
};

enum class CovenantEffectClaimResult
{
    Claimed,
    AlreadyApplied,
    Pending,
    Failed
};

enum class CovenantEffectReceiptStatus
{
    Pending,
    Applied
};

enum class CovenantEffectReceiptLoadResult
{
    Found,
    NotFound,
    Failed
};

struct CovenantEffectReceiptEntry
{
    CovenantId covenantId;
    std::string effectKey;
    CovenantEffectReceiptStatus status;

    CovenantEffectReceiptEntry()
        : covenantId(0), status(CovenantEffectReceiptStatus::Pending)
    {
    }
};

class ICovenantRepository
{
public:
    virtual ~ICovenantRepository() {}
    virtual bool LoadAll(const std::string& playerKey,
                         std::vector<CovenantMembershipState>& memberships) = 0;
    virtual bool Save(const std::string& playerKey,
                      const CovenantMembershipState& membership) = 0;
    virtual bool TryJoin(const std::string& playerKey,
                         const CovenantMembershipState& membership,
                         bool targetExclusive,
                         const std::vector<CovenantId>& knownCovenantIds,
                         const std::vector<CovenantId>& exclusiveCovenantIds)
    {
        (void)targetExclusive;
        (void)knownCovenantIds;
        (void)exclusiveCovenantIds;
        return Save(playerKey, membership);
    }
    virtual bool SaveIfUnchanged(const std::string& playerKey,
                                 const CovenantMembershipState& expected,
                                 const CovenantMembershipState& membership)
    {
        (void)expected;
        return Save(playerKey, membership);
    }
    virtual bool SwitchActive(const std::string& playerKey,
                              CovenantId targetCovenantId,
                              const std::vector<CovenantId>& knownCovenantIds)
    {
        if (playerKey.empty() || targetCovenantId <= 0)
            return false;
        std::vector<CovenantMembershipState> memberships;
        if (!LoadAll(playerKey, memberships))
            return false;
        bool foundTarget = false;
        for (std::vector<CovenantMembershipState>::const_iterator it = memberships.begin();
             it != memberships.end(); ++it)
        {
            bool known = false;
            for (std::vector<CovenantId>::const_iterator knownIt = knownCovenantIds.begin();
                 knownIt != knownCovenantIds.end(); ++knownIt)
            {
                if (*knownIt == it->covenantId)
                {
                    known = true;
                    break;
                }
            }
            if (!known)
                return false;
            if (it->covenantId == targetCovenantId)
                foundTarget = true;
        }
        if (!foundTarget)
            return false;
        for (std::vector<CovenantMembershipState>::iterator it = memberships.begin();
             it != memberships.end(); ++it)
        {
            const bool shouldBeActive = it->covenantId == targetCovenantId;
            if (it->active == shouldBeActive)
                continue;
            it->active = shouldBeActive;
            if (!Save(playerKey, *it))
                return false;
        }
        return true;
    }
    virtual CovenantReputationSaveResult SaveReputationChange(
        const std::string& playerKey,
        const CovenantMembershipState& membership,
        int delta,
        const std::string& sourceKey) = 0;
    virtual CovenantReputationSaveResult SaveReputationChangeChecked(
        const std::string& playerKey,
        const CovenantMembershipState& expected,
        const CovenantMembershipState& membership,
        int delta,
        const std::string& sourceKey)
    {
        (void)expected;
        return SaveReputationChange(playerKey, membership, delta, sourceKey);
    }
    virtual CovenantReputationSaveResult SaveRepeatableReputationChangeChecked(
        const std::string& playerKey,
        const CovenantMembershipState& expected,
        const CovenantMembershipState& membership,
        int delta)
    {
        (void)delta;
        return SaveIfUnchanged(playerKey, expected, membership)
                   ? CovenantReputationSaveResult::Applied
                   : CovenantReputationSaveResult::Conflict;
    }
    virtual CovenantEffectClaimResult ClaimEffect(const std::string& playerKey,
                                                   CovenantId covenantId,
                                                   const std::string& effectKey)
    {
        (void)playerKey;
        (void)covenantId;
        (void)effectKey;
        return CovenantEffectClaimResult::Failed;
    }
    virtual bool MarkEffectApplied(const std::string& playerKey,
                                   CovenantId covenantId,
                                   const std::string& effectKey)
    {
        (void)playerKey;
        (void)covenantId;
        (void)effectKey;
        return false;
    }
    virtual CovenantEffectReceiptLoadResult GetEffectReceiptStatus(
        const std::string& playerKey,
        CovenantId covenantId,
        const std::string& effectKey,
        CovenantEffectReceiptStatus& status)
    {
        (void)playerKey;
        (void)covenantId;
        (void)effectKey;
        (void)status;
        return CovenantEffectReceiptLoadResult::Failed;
    }
    virtual bool ListPendingEffects(const std::string& playerKey,
                                    std::vector<CovenantEffectReceiptEntry>& receipts)
    {
        (void)playerKey;
        receipts.clear();
        return false;
    }
    virtual bool AcknowledgePendingEffect(const std::string& playerKey,
                                          CovenantId covenantId,
                                          const std::string& effectKey)
    {
        CovenantEffectReceiptStatus status = CovenantEffectReceiptStatus::Pending;
        const CovenantEffectReceiptLoadResult loaded =
            GetEffectReceiptStatus(playerKey, covenantId, effectKey, status);
        if (loaded != CovenantEffectReceiptLoadResult::Found)
            return false;
        if (status == CovenantEffectReceiptStatus::Applied)
            return true;
        return MarkEffectApplied(playerKey, covenantId, effectKey);
    }
};
}
}
