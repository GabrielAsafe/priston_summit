#if defined(SUMMIT_COVENANT_CHAT_SMOKE_TEST)

#include <cassert>
#include <map>
#include <string>
#include <vector>

#include "systems/gameplay/covenants/v2/CovenantChatCommand.h"
#include "systems/gameplay/progression/v2/ProgressionRuntime.h"
#include "gameplay/covenants/v2/CovenantCatalog.h"

namespace
{
std::vector<gameplay::covenants::CovenantCatalogEntry> g_catalog;
std::map<int, gameplay::covenants::CovenantMembershipState> g_memberships;
int g_artisanMode = 0;

void BuildCatalog()
{
    const char* keys[] = { "hunters", "gatherers", "artisans", "fortune", "ascendants" };
    for (int i = 0; i < 5; ++i)
    {
        gameplay::covenants::CovenantCatalogEntry covenant;
        covenant.id = i + 1;
        covenant.key = keys[i];
        for (int rankId = 1; rankId <= 3; ++rankId)
        {
            gameplay::covenants::CovenantRankCatalogEntry rank;
            rank.id = rankId;
            rank.key = rankId == 1 ? "tier_one" : rankId == 2 ? "tier_two" : "tier_three";
            rank.minReputation = rankId == 1 ? 0 : rankId == 2 ? 1000 : 3000;
            gameplay::modifiers::GameplayModifier modifier;
            modifier.statKey = "test.effect";
            modifier.scopeKey = "global";
            modifier.operation = gameplay::modifiers::ModifierOperation::AdditivePercent;
            modifier.value = 0.05 * rankId;
            rank.benefits.push_back(modifier);
            covenant.ranks.push_back(rank);
        }
        g_catalog.push_back(covenant);
    }
}

bool Contains(const std::vector<std::string>& lines, const std::string& needle)
{
    for (std::vector<std::string>::const_iterator it = lines.begin(); it != lines.end(); ++it)
        if (it->find(needle) != std::string::npos)
            return true;
    return false;
}
}

namespace gameplay
{
namespace progression
{
bool IsServerProgressionRuntimeReady() { return true; }

void GetServerCovenantCatalog(std::vector<covenants::CovenantCatalogEntry>& entries)
{
    entries = g_catalog;
}

bool GetServerCovenantSnapshot(const std::string&, int covenantId,
                               covenants::CovenantSnapshot& snapshot)
{
    std::map<int, covenants::CovenantMembershipState>::const_iterator it = g_memberships.find(covenantId);
    if (it == g_memberships.end())
        return false;
    snapshot = covenants::CovenantSnapshot();
    snapshot.covenantId = covenantId;
    snapshot.key = g_catalog[covenantId - 1].key;
    snapshot.membership = it->second;
    snapshot.rankKey = "tier_one";
    if (snapshot.membership.active)
        snapshot.activeBenefits = g_catalog[covenantId - 1].ranks[0].benefits;
    return true;
}

bool GetServerActiveCovenantSnapshot(const std::string& playerKey,
                                     covenants::CovenantSnapshot& snapshot)
{
    for (std::map<int, covenants::CovenantMembershipState>::const_iterator it = g_memberships.begin();
         it != g_memberships.end(); ++it)
        if (it->second.active)
            return GetServerCovenantSnapshot(playerKey, it->first, snapshot);
    return false;
}

covenants::CovenantJoinEligibility CheckServerCovenantJoinEligibility(
    const std::string&, int covenantId, int, int)
{
    std::map<int, covenants::CovenantMembershipState>::const_iterator it = g_memberships.find(covenantId);
    if (it == g_memberships.end())
        return covenants::CovenantJoinEligibility::Eligible;
    return it->second.active ? covenants::CovenantJoinEligibility::AlreadyActive
                             : covenants::CovenantJoinEligibility::AlreadyMember;
}

bool JoinServerCovenant(const std::string&, int covenantId, int, int)
{
    if (g_memberships.count(covenantId))
        return false;
    bool anyActive = false;
    for (std::map<int, covenants::CovenantMembershipState>::const_iterator it = g_memberships.begin();
         it != g_memberships.end(); ++it)
        anyActive = anyActive || it->second.active;
    covenants::CovenantMembershipState state;
    state.covenantId = covenantId;
    state.rankId = 1;
    state.active = !anyActive;
    g_memberships[covenantId] = state;
    return true;
}

covenants::CovenantActivationResult ActivateServerCovenant(
    const std::string&, int covenantId, int, int)
{
    if (!g_memberships.count(covenantId))
        return covenants::CovenantActivationResult::NotMember;
    if (g_memberships[covenantId].active)
        return covenants::CovenantActivationResult::AlreadyActive;
    for (std::map<int, covenants::CovenantMembershipState>::iterator it = g_memberships.begin();
         it != g_memberships.end(); ++it)
        it->second.active = it->first == covenantId;
    return covenants::CovenantActivationResult::Activated;
}

bool AddServerCovenantReputation(const std::string&, int covenantId, int amount,
                                 const std::string&, int, int)
{
    if (!g_memberships.count(covenantId) || !g_memberships[covenantId].active)
        return false;
    g_memberships[covenantId].reputation += amount;
    return true;
}

bool GetServerCovenantMasteries(const std::string&, std::vector<int>& covenantIds)
{
    covenantIds.clear();
    return true;
}

bool GetServerCovenantCollectibles(const std::string&, std::vector<std::string>& keys)
{
    keys.clear();
    keys.push_back("sigil:hunters");
    return true;
}

bool GetServerCovenantChallenges(const std::string&, int covenantId,
                                 std::vector<covenants::CovenantChallengeState>& states)
{
    states.clear();
    covenants::CovenantChallengeState state;
    state.covenantId = covenantId;
    state.key = "smoke.challenge";
    state.targetCount = 5;
    state.progress = 2;
    states.push_back(state);
    return true;
}

bool GetServerCovenantInventory(const std::string&, std::vector<covenants::CovenantInventoryItem>& items)
{
    items.clear();
    covenants::CovenantInventoryItem item;
    item.covenantId = 1;
    item.key = "trophy:100";
    item.quantity = 2;
    items.push_back(item);
    item.covenantId = 2;
    item.key = "special_material:iron_ore";
    item.quantity = 3;
    items.push_back(item);
    item.covenantId = 1;
    item.key = "refined_trophy:100";
    item.quantity = 2;
    items.push_back(item);
    item.covenantId = 2;
    item.key = "refined_material:iron_ore";
    item.quantity = 1;
    items.push_back(item);
    return true;
}

bool GetServerCovenantArtisanMode(const std::string&, int& mode)
{
    mode = g_artisanMode;
    return true;
}

bool SetServerCovenantArtisanMode(const std::string&, int mode)
{
    if (mode < 0 || mode > 3)
        return false;
    g_artisanMode = mode;
    return true;
}

bool RefineServerCovenantTrophy(const std::string&, const std::string& trophyKey, std::string* refinedKey)
{
    if (trophyKey.compare(0, 7, "trophy:") != 0)
        return false;
    if (refinedKey)
        *refinedKey = std::string("refined_") + trophyKey;
    return true;
}

bool GetServerCovenantSwitchCooldown(const std::string&, long long& cooldownUntil)
{
    cooldownUntil = 0;
    return true;
}

bool GetServerCovenantModifierAggregate(
    const std::string&, const std::string& statKey, const std::string&,
    covenants::CovenantModifierAggregate& aggregate, bool, unsigned int)
{
    aggregate = covenants::CovenantModifierAggregate();
    if (statKey == covenants::catalog::modifier::kGatheringSurveyorEnabled)
    {
        aggregate.matched = true;
        aggregate.flat = 1.0;
        return true;
    }
    std::map<int, covenants::CovenantMembershipState>::const_iterator artisan = g_memberships.find(3);
    if (artisan != g_memberships.end() && artisan->second.active)
    {
        int requiredRank = 0;
        if (statKey == "crafting.reforge_enabled") requiredRank = 1;
        else if (statKey == "crafting.forgemaster_enabled") requiredRank = 2;
        else if (statKey == "crafting.master_artisan_enabled") requiredRank = 3;
        if (requiredRank > 0 && artisan->second.rankId >= requiredRank)
        {
            aggregate.matched = true;
            aggregate.flat = 1.0;
        }
    }
    return true;
}

bool GetServerCovenantPowerState(const std::string&, const std::string&,
                                  covenants::CovenantPowerDefinition& definition,
                                  covenants::CovenantPowerState& state)
{
    definition.requiredRankId = 3;
    definition.durationSeconds = 60;
    definition.cooldownSeconds = 600;
    state.activeUntil = 0;
    state.cooldownUntil = 0;
    return true;
}

bool UseServerCovenantPower(const std::string&, const std::string&, unsigned int, std::string* error)
{
    if (error)
        error->clear();
    return true;
}
}
}

int main()
{
    BuildCatalog();
    gameplay::covenants::CovenantChatCommandContext context;
    context.playerKey = "Alice";
    context.playerLevel = 10;
    context.legacyJobCode = 1;

    gameplay::covenants::CovenantChatCommandResponse ignored =
        gameplay::covenants::HandleServerCovenantChatCommand(context, "hello");
    assert(!ignored.handled);

    gameplay::covenants::CovenantChatCommandResponse list =
        gameplay::covenants::HandleServerCovenantChatCommand(context, "/covenant list");
    assert(list.handled && list.success && Contains(list.lines, "[NOT JOINED] Hunters"));

    gameplay::covenants::CovenantChatCommandResponse joinHunters =
        gameplay::covenants::HandleServerCovenantChatCommand(context, "/covenant join hunter");
    assert(joinHunters.success && g_memberships[1].active);

    gameplay::covenants::CovenantChatCommandResponse joinGatherers =
        gameplay::covenants::HandleServerCovenantChatCommand(context, "/covenant join gatherer");
    assert(joinGatherers.success && !g_memberships[2].active && g_memberships[1].active);

    gameplay::covenants::CovenantChatCommandResponse activate =
        gameplay::covenants::HandleServerCovenantChatCommand(context, "/covenant activate gatherers");
    assert(activate.success && g_memberships[2].active && !g_memberships[1].active);

    gameplay::covenants::CovenantChatCommandResponse status =
        gameplay::covenants::HandleServerCovenantChatCommand(context, "/covenant status");
    assert(status.success && Contains(status.lines, "[ACTIVE] Gatherers"));

    gameplay::covenants::CovenantSurveyArea surveyArea;
    surveyArea.resourceKey = "iron_ore";
    surveyArea.area = 7;
    surveyArea.nodeCount = 3;
    context.surveyAreas.push_back(surveyArea);
    gameplay::covenants::CovenantChatCommandResponse survey =
        gameplay::covenants::HandleServerCovenantChatCommand(context, "/covenant survey");
    assert(survey.success && Contains(survey.lines, "iron_ore") &&
           Contains(survey.lines, "area 7") && Contains(survey.lines, "deposits 3"));

    gameplay::covenants::CovenantChatCommandResponse joinArtisans =
        gameplay::covenants::HandleServerCovenantChatCommand(context, "/covenant join artisans");
    assert(joinArtisans.success && !g_memberships[3].active);
    gameplay::covenants::CovenantChatCommandResponse activateArtisans =
        gameplay::covenants::HandleServerCovenantChatCommand(context, "/covenant activate artisans");
    assert(activateArtisans.success && g_memberships[3].active);

    gameplay::covenants::CovenantChatCommandResponse artisanStatus =
        gameplay::covenants::HandleServerCovenantChatCommand(context, "/covenant artisan");
    assert(artisanStatus.success && Contains(artisanStatus.lines, "Artisan workshop: off") &&
           Contains(artisanStatus.lines, "special 3") && Contains(artisanStatus.lines, "refined trophies 2"));

    gameplay::covenants::CovenantChatCommandResponse artisanReforge =
        gameplay::covenants::HandleServerCovenantChatCommand(context, "/covenant artisan reforge");
    assert(artisanReforge.success && g_artisanMode == 1);
    gameplay::covenants::CovenantChatCommandResponse artisanLocked =
        gameplay::covenants::HandleServerCovenantChatCommand(context, "/covenant artisan forgemaster");
    assert(!artisanLocked.success && g_artisanMode == 1);
    g_memberships[3].rankId = 2;
    gameplay::covenants::CovenantChatCommandResponse artisanForge =
        gameplay::covenants::HandleServerCovenantChatCommand(context, "/covenant artisan forgemaster");
    assert(artisanForge.success && g_artisanMode == 2);
    g_memberships[3].rankId = 3;
    gameplay::covenants::CovenantChatCommandResponse artisanMaster =
        gameplay::covenants::HandleServerCovenantChatCommand(context, "/covenant artisan master");
    assert(artisanMaster.success && g_artisanMode == 3);

    // Restore Gatherers for the remaining smoke commands.
    gameplay::covenants::CovenantChatCommandResponse reactivateGatherers =
        gameplay::covenants::HandleServerCovenantChatCommand(context, "/covenant activate gatherers");
    assert(reactivateGatherers.success && g_memberships[2].active);

    gameplay::covenants::CovenantChatCommandResponse challenges =
        gameplay::covenants::HandleServerCovenantChatCommand(context, "/covenant challenges gatherers");
    assert(challenges.success && Contains(challenges.lines, "smoke.challenge"));

    gameplay::covenants::CovenantChatCommandResponse sigils =
        gameplay::covenants::HandleServerCovenantChatCommand(context, "/covenant sigils");
    assert(sigils.success && Contains(sigils.lines, "sigil:hunters"));

    gameplay::covenants::CovenantChatCommandResponse inventory =
        gameplay::covenants::HandleServerCovenantChatCommand(context, "/covenant inventory");
    assert(inventory.success && Contains(inventory.lines, "trophy:100 x2"));

    gameplay::covenants::CovenantChatCommandResponse refine =
        gameplay::covenants::HandleServerCovenantChatCommand(context, "/covenant refine trophy:100");
    assert(refine.success && Contains(refine.lines, "refined_trophy:100"));

    gameplay::covenants::CovenantChatCommandResponse mastery =
        gameplay::covenants::HandleServerCovenantChatCommand(context, "/covenant mastery");
    assert(mastery.success && Contains(mastery.lines, "0/5"));

    gameplay::covenants::CovenantChatCommandResponse info =
        gameplay::covenants::HandleServerCovenantChatCommand(context, "/covenant info fortune");
    assert(info.success && Contains(info.lines, "Tier 3") && Contains(info.lines, "+15"));

    gameplay::covenants::CovenantChatCommandResponse denied =
        gameplay::covenants::HandleServerCovenantChatCommand(context, "/covenant rep gatherers 50 smoke.1");
    assert(!denied.success);
    context.allowAdminCommands = true;
    gameplay::covenants::CovenantChatCommandResponse rep =
        gameplay::covenants::HandleServerCovenantChatCommand(context, "/covenant rep gatherers 50 smoke.1");
    assert(rep.success && g_memberships[2].reputation == 50);
    return 0;
}

#endif // SUMMIT_COVENANT_CHAT_SMOKE_TEST
