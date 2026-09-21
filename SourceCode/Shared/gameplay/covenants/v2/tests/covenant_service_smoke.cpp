#include <cassert>
#include <climits>
#include <map>
#include <string>
#include <vector>

#include "gameplay/covenants/v2/CovenantService.h"

namespace
{
class FakeRepo : public gameplay::covenants::ICovenantRepository
{
public:
    bool LoadAll(const std::string& playerKey,
                 std::vector<gameplay::covenants::CovenantMembershipState>& memberships)
    {
        memberships = data[playerKey];
        return true;
    }

    bool Save(const std::string& playerKey,
              const gameplay::covenants::CovenantMembershipState& membership)
    {
        std::vector<gameplay::covenants::CovenantMembershipState>& list = data[playerKey];
        for (std::vector<gameplay::covenants::CovenantMembershipState>::iterator it = list.begin();
             it != list.end(); ++it)
        {
            if (it->covenantId == membership.covenantId)
            {
                *it = membership;
                return true;
            }
        }
        list.push_back(membership);
        return true;
    }

    gameplay::covenants::CovenantReputationSaveResult SaveReputationChange(
        const std::string& playerKey,
        const gameplay::covenants::CovenantMembershipState& membership,
        int,
        const std::string& sourceKey)
    {
        const std::string ledgerKey = playerKey + ":" +
                                      std::to_string(membership.covenantId) + ":" + sourceKey;
        if (ledger.count(ledgerKey) != 0u)
            return gameplay::covenants::CovenantReputationSaveResult::AlreadyApplied;
        if (!Save(playerKey, membership))
            return gameplay::covenants::CovenantReputationSaveResult::Failed;
        ledger[ledgerKey] = true;
        return gameplay::covenants::CovenantReputationSaveResult::Applied;
    }

    std::map<std::string, std::vector<gameplay::covenants::CovenantMembershipState> > data;
    std::map<std::string, bool> ledger;
};

class FakeContext : public gameplay::conditions::ConditionContext
{
public:
    explicit FakeContext(int levelValue) : level(levelValue) {}
    int PlayerLevel() const { return level; }
    int LegacyJobCode() const { return 1; }
    bool HasStoryFlag(const std::string&) const { return false; }
    int StoryNodeState(int, int) const { return 0; }
    int QuestState(int) const { return 0; }
    bool HasCovenantMembership(int) const { return false; }
    int CovenantRank(int) const { return 0; }
    int CovenantReputation(int) const { return 0; }
    int ProfessionLevel(const std::string&) const { return 0; }
    bool EvaluateCustomCondition(const gameplay::conditions::ConditionDefinition&) const { return false; }
    int level;
};
}

int main()
{
    using namespace gameplay;
    FakeRepo repo;
    events::GameplayEventHub hub;
    covenants::CovenantService service(repo, &hub);

    covenants::CovenantDefinition guild;
    guild.id = 1;
    guild.key = "guild.explorers";
    guild.displayNameKey = "covenant.explorers";
    guild.relatedQuestIds.push_back(101);
    guild.relatedStoryIds.push_back(201);
    guild.exclusiveMembership = true;
    guild.hasReputationBounds = true;
    guild.minReputation = -100;
    guild.maxReputation = 200;
    conditions::ConditionDefinition minLevel;
    minLevel.type = conditions::ConditionType::PlayerLevelAtLeast;
    minLevel.value = 10;
    guild.joinConditions.push_back(minLevel);
    assert(service.RegisterCovenant(guild));

    covenants::CovenantDefinition rival = guild;
    rival.id = 2;
    rival.key = "guild.rival";
    rival.displayNameKey = "covenant.rival";
    assert(service.RegisterCovenant(rival));

    covenants::CovenantRankDefinition rivalNovice;
    rivalNovice.id = 10;
    rivalNovice.covenantId = 2;
    rivalNovice.key = "novice";
    rivalNovice.displayNameKey = "covenant.rank.novice";
    rivalNovice.minReputation = 0;
    assert(service.RegisterRank(rivalNovice));

    covenants::CovenantRankDefinition novice;
    novice.id = 10;
    novice.covenantId = 1;
    novice.key = "novice";
    novice.displayNameKey = "covenant.rank.novice";
    novice.minReputation = 0;
    assert(service.RegisterRank(novice));

    covenants::CovenantRankDefinition veteran;
    veteran.id = 20;
    veteran.covenantId = 1;
    veteran.key = "veteran";
    veteran.displayNameKey = "covenant.rank.veteran";
    veteran.minReputation = 100;
    modifiers::GameplayModifier benefit;
    benefit.statKey = "test.stat";
    benefit.value = 1.0;
    veteran.benefits.push_back(benefit);
    assert(service.RegisterRank(veteran));
    assert(service.ValidateDefinitions());

    FakeRepo invalidRepo;
    covenants::CovenantService invalidService(invalidRepo);
    covenants::CovenantDefinition invalidCovenant;
    invalidCovenant.id = 50;
    invalidCovenant.key = "guild.invalid";
    conditions::ConditionDefinition malformedJoin;
    malformedJoin.type = conditions::ConditionType::QuestState;
    malformedJoin.id = 10;
    malformedJoin.value = 99;
    invalidCovenant.joinConditions.push_back(malformedJoin);
    assert(invalidService.RegisterCovenant(invalidCovenant));
    covenants::CovenantRankDefinition invalidRank;
    invalidRank.id = 1;
    invalidRank.covenantId = 50;
    invalidRank.key = "rank";
    invalidRank.minReputation = 0;
    assert(invalidService.RegisterRank(invalidRank));
    assert(!invalidService.ValidateDefinitions());

    const std::vector<covenants::CovenantId> covenantIds = service.CovenantIds();
    assert(covenantIds.size() == 2u);
    assert(covenantIds[0] == 1 && covenantIds[1] == 2);
    std::vector<covenants::CovenantCatalogEntry> covenantCatalog;
    service.GetCatalog(covenantCatalog);
    assert(covenantCatalog.size() == 2u);
    assert(covenantCatalog[0].id == 1);
    assert(covenantCatalog[0].key == "guild.explorers");
    assert(covenantCatalog[0].displayNameKey == "covenant.explorers");
    assert(covenantCatalog[0].exclusiveMembership);
    assert(covenantCatalog[0].relatedQuestIds.size() == 1u && covenantCatalog[0].relatedQuestIds[0] == 101);
    assert(covenantCatalog[0].relatedStoryIds.size() == 1u && covenantCatalog[0].relatedStoryIds[0] == 201);
    assert(covenantCatalog[0].ranks.size() == 2u);
    assert(covenantCatalog[0].ranks[0].id == 10 && covenantCatalog[0].ranks[0].key == "novice");
    assert(covenantCatalog[0].ranks[1].id == 20 && covenantCatalog[0].ranks[1].key == "veteran");
    assert(covenantCatalog[1].id == 2 && covenantCatalog[1].key == "guild.rival");

    // Rank ids are part of the ConditionContext contract (CovenantRankAtLeast compares ids).
    // Reject catalogs where ids decrease as reputation thresholds increase.
    FakeRepo unorderedRepo;
    covenants::CovenantService unorderedService(unorderedRepo);
    covenants::CovenantDefinition unordered = guild;
    unordered.id = 99;
    unordered.key = "guild.unordered";
    assert(unorderedService.RegisterCovenant(unordered));
    covenants::CovenantRankDefinition highIdFirst;
    highIdFirst.id = 20;
    highIdFirst.covenantId = 99;
    highIdFirst.key = "first";
    highIdFirst.minReputation = 0;
    assert(unorderedService.RegisterRank(highIdFirst));
    covenants::CovenantRankDefinition lowIdLater;
    lowIdLater.id = 10;
    lowIdLater.covenantId = 99;
    lowIdLater.key = "later";
    lowIdLater.minReputation = 100;
    assert(unorderedService.RegisterRank(lowIdLater));
    assert(!unorderedService.ValidateDefinitions());

    int joins = 0;
    int leaves = 0;
    int repChanges = 0;
    int rankChanges = 0;
    hub.Subscribe<covenants::CovenantJoinedEvent>([&](const covenants::CovenantJoinedEvent&) { ++joins; });
    hub.Subscribe<covenants::CovenantLeftEvent>([&](const covenants::CovenantLeftEvent&) { ++leaves; });
    hub.Subscribe<covenants::CovenantReputationChangedEvent>([&](const covenants::CovenantReputationChangedEvent&) { ++repChanges; });
    hub.Subscribe<covenants::CovenantRankChangedEvent>([&](const covenants::CovenantRankChangedEvent&) { ++rankChanges; });

    FakeContext tooLow(9);
    assert(service.CheckJoinEligibility("", 1, tooLow) ==
           covenants::CovenantJoinEligibility::InvalidRequest);
    assert(service.CheckJoinEligibility("Alice", 999, tooLow) ==
           covenants::CovenantJoinEligibility::UnknownCovenant);
    assert(service.CheckJoinEligibility("Alice", 1, tooLow) ==
           covenants::CovenantJoinEligibility::ConditionsNotMet);
    assert(!service.Join("Alice", 1, tooLow));
    FakeContext eligible(10);
    assert(service.CheckJoinEligibility("Alice", 1, eligible) ==
           covenants::CovenantJoinEligibility::Eligible);
    assert(service.Join("Alice", 1, eligible));
    assert(service.CheckJoinEligibility("Alice", 1, eligible) ==
           covenants::CovenantJoinEligibility::AlreadyActive);
    assert(joins == 1);
    assert(service.CheckJoinEligibility("Alice", 2, eligible) ==
           covenants::CovenantJoinEligibility::ExclusiveConflict);
    assert(!service.Join("Alice", 2, eligible));
    assert(service.GetRank("Alice", 1) == 10);

    assert(!service.AddReputation("Alice", 1, 0, "quest.noop"));
    assert(!service.AddReputation("Alice", 1, 1, std::string(256, 'x')));
    assert(service.AddReputation("Alice", 1, 150, "quest.reward"));
    assert(service.GetRank("Alice", 1) == 20);
    assert(service.HasRankAtLeast("Alice", 1, 10));
    const std::vector<modifiers::GameplayModifier> activeBenefits = service.GetActiveBenefits("Alice", 1);
    assert(activeBenefits.size() == 1u);
    assert(activeBenefits[0].sourceKey == "covenant:guild.explorers:rank:veteran");
    std::vector<modifiers::GameplayModifier> allBenefits;
    assert(service.GetAllActiveBenefits("Alice", allBenefits));
    assert(allBenefits.size() == 1u);
    assert(allBenefits[0].sourceKey == "covenant:guild.explorers:rank:veteran");
    covenants::CovenantSnapshot snapshot;
    assert(service.GetSnapshot("Alice", 1, snapshot));
    assert(snapshot.key == "guild.explorers");
    assert(snapshot.rankKey == "veteran");
    assert(snapshot.activeBenefits.size() == 1u);
    assert(snapshot.activeBenefits[0].sourceKey == "covenant:guild.explorers:rank:veteran");
    assert(snapshot.relatedQuestIds.size() == 1u && snapshot.relatedQuestIds[0] == 101);
    assert(snapshot.relatedStoryIds.size() == 1u && snapshot.relatedStoryIds[0] == 201);
    std::vector<covenants::CovenantSnapshot> activeSnapshots;
    assert(service.GetActiveSnapshots("Alice", activeSnapshots));
    assert(activeSnapshots.size() == 1u);
    assert(activeSnapshots[0].covenantId == 1);
    assert(activeSnapshots[0].rankKey == "veteran");
    assert(repChanges == 1 && rankChanges == 1);

    // Same authoritative source key is idempotent: no second delta or duplicate events.
    assert(service.AddReputation("Alice", 1, 150, "quest.reward"));
    covenants::CovenantMembershipState state;
    assert(service.GetMembership("Alice", 1, state));
    assert(state.reputation == 150);
    assert(repChanges == 1 && rankChanges == 1);

    state.rankId = 10;
    assert(repo.Save("Alice", state));
    assert(service.ReevaluateRank("Alice", 1));
    assert(service.GetMembership("Alice", 1, state));
    assert(state.rankId == 20);
    assert(rankChanges == 2);

    assert(service.AddReputation("Alice", 1, INT_MAX, "admin.test"));
    assert(service.GetMembership("Alice", 1, state));
    assert(state.reputation == 200);

    assert(service.AddReputation("Alice", 1, INT_MIN, "penalty.test"));
    assert(service.GetMembership("Alice", 1, state));
    assert(state.reputation == -100);
    assert(state.rankId == 0);
    assert(service.GetAllActiveBenefits("Alice", allBenefits));
    assert(allBenefits.empty());

    assert(service.Leave("Alice", 1));
    assert(leaves == 1);
    assert(service.GetActiveSnapshots("Alice", activeSnapshots));
    assert(activeSnapshots.empty());
    assert(service.Join("Alice", 2, eligible));
    assert(joins == 2);
    return 0;
}
