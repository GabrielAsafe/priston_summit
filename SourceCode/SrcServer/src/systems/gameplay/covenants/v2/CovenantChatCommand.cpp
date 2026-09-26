#include "systems/gameplay/covenants/v2/CovenantChatCommand.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <ctime>

#include "systems/gameplay/progression/v2/ProgressionRuntime.h"
#include "gameplay/covenants/v2/CovenantCatalog.h"

namespace gameplay
{
namespace covenants
{
namespace
{
std::string Lower(const std::string& value)
{
    std::string result = value;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return result;
}

std::string Trim(const std::string& value)
{
    std::string::size_type first = 0;
    while (first < value.size() && std::isspace(static_cast<unsigned char>(value[first])))
        ++first;
    std::string::size_type last = value.size();
    while (last > first && std::isspace(static_cast<unsigned char>(value[last - 1])))
        --last;
    return value.substr(first, last - first);
}

std::vector<std::string> Tokens(const std::string& value)
{
    std::vector<std::string> result;
    std::istringstream stream(value);
    std::string token;
    while (stream >> token)
        result.push_back(token);
    return result;
}

std::string FriendlyName(const std::string& key)
{
    if (key == "hunters") return "Hunters";
    if (key == "gatherers") return "Gatherers";
    if (key == "artisans") return "Artisans";
    if (key == "fortune") return "Fortune";
    if (key == "ascendants") return "Ascendants";
    return key;
}

std::string CanonicalKey(const std::string& raw)
{
    const std::string value = Lower(raw);
    if (value == "hunter") return "hunters";
    if (value == "gatherer") return "gatherers";
    if (value == "artisan") return "artisans";
    if (value == "ascendant") return "ascendants";
    return value;
}

const CovenantCatalogEntry* FindCovenant(const std::vector<CovenantCatalogEntry>& catalog,
                                         const std::string& rawKey)
{
    const std::string key = CanonicalKey(rawKey);
    for (std::vector<CovenantCatalogEntry>::const_iterator it = catalog.begin();
         it != catalog.end(); ++it)
    {
        if (Lower(it->key) == key)
            return &*it;
    }
    return 0;
}

std::string PowerKeyForCovenant(CovenantId covenantId)
{
    if (covenantId == catalog::kHunters) return "hunters_mark";
    if (covenantId == catalog::kFortune) return "tempt_fate";
    if (covenantId == catalog::kAscendants) return "focused_training";
    return std::string();
}

std::string FormatModifier(const modifiers::GameplayModifier& modifier)
{
    std::ostringstream out;
    out << modifier.statKey;
    if (!modifier.scopeKey.empty())
        out << "[" << modifier.scopeKey << "]";
    out << " ";
    if (modifier.operation == modifiers::ModifierOperation::Flat)
    {
        if (modifier.value == 1.0)
            out << "enabled";
        else
            out << modifier.value;
    }
    else
    {
        const double percent = modifier.value * 100.0;
        if (percent >= 0.0)
            out << "+";
        out << std::fixed << std::setprecision(percent == static_cast<int>(percent) ? 0 : 1)
            << percent << "%";
    }
    return out.str();
}

const char* ArtisanModeName(int mode)
{
    switch (mode)
    {
    case 1: return "reforge";
    case 2: return "forgemaster";
    case 3: return "master";
    default: return "off";
    }
}

bool CovenantFlagUnlocked(const std::string& playerKey,
                         const char* statKey,
                         const char* scopeKey)
{
    CovenantModifierAggregate aggregate;
    return progression::GetServerCovenantModifierAggregate(
               playerKey, statKey, scopeKey, aggregate) &&
           CovenantFlagEnabled(aggregate);
}

int CountInventoryPrefix(const std::vector<CovenantInventoryItem>& items,
                         const std::string& prefix)
{
    int total = 0;
    for (std::vector<CovenantInventoryItem>::const_iterator it = items.begin();
         it != items.end(); ++it)
        if (it->quantity > 0 && it->key.compare(0, prefix.size(), prefix) == 0)
            total += it->quantity;
    return total;
}

void AddHelp(CovenantChatCommandResponse& response)
{
    response.lines.push_back("Covenant commands:");
    response.lines.push_back("/covenant list");
    response.lines.push_back("/covenant info <hunters|gatherers|artisans|fortune|ascendants>");
    response.lines.push_back("/covenant join <name>");
    response.lines.push_back("/covenant activate <name>");
    response.lines.push_back("/covenant status");
    response.lines.push_back("/covenant challenges [name]");
    response.lines.push_back("/covenant mastery");
    response.lines.push_back("/covenant sigils");
    response.lines.push_back("/covenant inventory");
    response.lines.push_back("/covenant survey");
    response.lines.push_back("/covenant artisan [off|reforge|forgemaster|master]");
    response.lines.push_back("/covenant refine <trophy:key>");
    response.lines.push_back("/covenant power");
}

std::string EligibilityMessage(CovenantJoinEligibility result)
{
    switch (result)
    {
    case CovenantJoinEligibility::Eligible: return "eligible";
    case CovenantJoinEligibility::InvalidRequest: return "invalid request";
    case CovenantJoinEligibility::UnknownCovenant: return "unknown Covenant";
    case CovenantJoinEligibility::ConditionsNotMet: return "join conditions not met";
    case CovenantJoinEligibility::AlreadyActive: return "Covenant is already active";
    case CovenantJoinEligibility::AlreadyMember: return "already joined; use /covenant activate";
    case CovenantJoinEligibility::ExclusiveConflict: return "exclusive Covenant conflict";
    case CovenantJoinEligibility::PersistenceFailure: return "persistence/runtime failure";
    }
    return "unknown error";
}

std::string ActivationMessage(CovenantActivationResult result)
{
    switch (result)
    {
    case CovenantActivationResult::Activated: return "activated";
    case CovenantActivationResult::InvalidRequest: return "invalid request";
    case CovenantActivationResult::UnknownCovenant: return "unknown Covenant";
    case CovenantActivationResult::NotMember: return "join this Covenant first";
    case CovenantActivationResult::AlreadyActive: return "already active";
    case CovenantActivationResult::CooldownActive: return "switch cooldown active";
    case CovenantActivationResult::PersistenceFailure: return "persistence/runtime failure";
    }
    return "unknown error";
}

void AddSnapshotStatus(const CovenantSnapshot& snapshot,
                       CovenantChatCommandResponse& response)
{
    std::ostringstream status;
    status << (snapshot.membership.active ? "[ACTIVE] " : "[JOINED] ")
           << FriendlyName(snapshot.key) << " - rank "
           << (snapshot.rankKey.empty() ? "none" : snapshot.rankKey)
           << " - reputation " << snapshot.membership.reputation;
    response.lines.push_back(status.str());

    if (snapshot.membership.active && !snapshot.activeBenefits.empty())
    {
        response.lines.push_back("Active effects:");
        for (std::vector<modifiers::GameplayModifier>::const_iterator it =
                 snapshot.activeBenefits.begin(); it != snapshot.activeBenefits.end(); ++it)
            response.lines.push_back(std::string("  ") + FormatModifier(*it));
    }
}

void AddRankProgress(const CovenantSnapshot& snapshot,
                     const CovenantCatalogEntry& catalogEntry,
                     CovenantChatCommandResponse& response)
{
    const CovenantRankCatalogEntry* next = 0;
    for (std::vector<CovenantRankCatalogEntry>::const_iterator it = catalogEntry.ranks.begin();
         it != catalogEntry.ranks.end(); ++it)
    {
        if (it->minReputation > snapshot.membership.reputation)
        {
            next = &*it;
            break;
        }
    }
    if (!next)
    {
        response.lines.push_back("Progress: maximum configured rank reached");
        return;
    }
    std::ostringstream progress;
    progress << "Progress: " << snapshot.membership.reputation << "/"
             << next->minReputation << " ("
             << (next->minReputation - snapshot.membership.reputation)
             << " reputation to " << next->key << ")";
    response.lines.push_back(progress.str());
}
}

CovenantChatCommandResponse HandleServerCovenantChatCommand(
    const CovenantChatCommandContext& context,
    const std::string& text)
{
    CovenantChatCommandResponse response;
    const std::string trimmed = Trim(text);
    const std::vector<std::string> tokens = Tokens(trimmed);
    if (tokens.empty() || Lower(tokens[0]) != "/covenant")
        return response;

    response.handled = true;
    if (context.playerKey.empty())
    {
        response.lines.push_back("Covenant: invalid player context.");
        return response;
    }
    if (!progression::IsServerProgressionRuntimeReady())
    {
        response.lines.push_back("Covenant runtime is not ready.");
        return response;
    }

    std::vector<CovenantCatalogEntry> catalog;
    progression::GetServerCovenantCatalog(catalog);
    if (tokens.size() == 1u || Lower(tokens[1]) == "help")
    {
        response.success = true;
        AddHelp(response);
        return response;
    }

    const std::string command = Lower(tokens[1]);
    if (command == "list" || command == "progress")
    {
        response.lines.push_back("Covenants:");
        for (std::vector<CovenantCatalogEntry>::const_iterator it = catalog.begin();
             it != catalog.end(); ++it)
        {
            CovenantSnapshot snapshot;
            if (progression::GetServerCovenantSnapshot(context.playerKey, it->id, snapshot))
            {
                AddSnapshotStatus(snapshot, response);
                AddRankProgress(snapshot, *it, response);
            }
            else
                response.lines.push_back(std::string("[NOT JOINED] ") + FriendlyName(it->key));
        }
        response.success = true;
        return response;
    }

    if (command == "status")
    {
        CovenantSnapshot snapshot;
        if (!progression::GetServerActiveCovenantSnapshot(context.playerKey, snapshot))
        {
            response.lines.push_back("No active Covenant. Join one with /covenant join <name>.");
            response.success = true;
            return response;
        }
        AddSnapshotStatus(snapshot, response);
        for (std::vector<CovenantCatalogEntry>::const_iterator it = catalog.begin();
             it != catalog.end(); ++it)
            if (it->id == snapshot.covenantId)
            {
                AddRankProgress(snapshot, *it, response);
                break;
            }

        const long long now = static_cast<long long>(std::time(0));
        long long switchCooldownUntil = 0;
        if (progression::GetServerCovenantSwitchCooldown(
                context.playerKey, switchCooldownUntil) && switchCooldownUntil > now)
        {
            std::ostringstream cooldown;
            cooldown << "Switch cooldown: " << (switchCooldownUntil - now) << "s remaining";
            response.lines.push_back(cooldown.str());
        }

        const std::string powerKey = PowerKeyForCovenant(snapshot.covenantId);
        if (!powerKey.empty())
        {
            CovenantPowerDefinition definition;
            CovenantPowerState state;
            if (progression::GetServerCovenantPowerState(
                    context.playerKey, powerKey, definition, state))
            {
                std::ostringstream power;
                if (snapshot.membership.rankId < definition.requiredRankId)
                    power << "Power locked: " << powerKey << " (requires rank " << definition.requiredRankId << ")";
                else if (state.activeUntil > now)
                    power << "Power active: " << powerKey << " (" << (state.activeUntil - now) << "s remaining)";
                else if (state.cooldownUntil > now)
                    power << "Power cooldown: " << powerKey << " (" << (state.cooldownUntil - now) << "s remaining)";
                else
                    power << "Power ready: " << powerKey;
                response.lines.push_back(power.str());
            }
        }
        response.success = true;
        return response;
    }

    if (command == "survey")
    {
        CovenantSnapshot active;
        if (!progression::GetServerActiveCovenantSnapshot(context.playerKey, active) ||
            active.covenantId != catalog::kGatherers)
        {
            response.lines.push_back("Surveyor requires active Gatherers Covenant.");
            return response;
        }
        CovenantModifierAggregate surveyor;
        if (!progression::GetServerCovenantModifierAggregate(
                context.playerKey, catalog::modifier::kGatheringSurveyorEnabled,
                "gathering", surveyor) || !surveyor.matched ||
            covenants::ApplyCovenantModifiers(0.0, surveyor) <= 0.0)
        {
            response.lines.push_back("Surveyor is not unlocked at the current Gatherers rank.");
            return response;
        }
        response.lines.push_back("Surveyor - known mining areas:");
        if (context.surveyAreas.empty())
            response.lines.push_back("  no mining resource nodes are registered in the current world content");
        for (std::vector<CovenantSurveyArea>::const_iterator it = context.surveyAreas.begin();
             it != context.surveyAreas.end(); ++it)
        {
            std::ostringstream line;
            line << "  " << it->resourceKey << " - area " << it->area
                 << " - deposits " << it->nodeCount;
            response.lines.push_back(line.str());
        }
        response.success = true;
        return response;
    }

    if (command == "artisan")
    {
        CovenantSnapshot active;
        if (!progression::GetServerActiveCovenantSnapshot(context.playerKey, active) ||
            active.covenantId != catalog::kArtisans)
        {
            response.lines.push_back("Artisan workshop requires active Artisans Covenant.");
            return response;
        }

        int mode = 0;
        if (!progression::GetServerCovenantArtisanMode(context.playerKey, mode))
        {
            response.lines.push_back("Could not load Artisan workshop mode.");
            return response;
        }

        if (tokens.size() >= 3u)
        {
            const std::string requested = Lower(tokens[2]);
            int requestedMode = -1;
            const char* capability = 0;
            if (requested == "off") requestedMode = 0;
            else if (requested == "reforge") { requestedMode = 1; capability = "crafting.reforge_enabled"; }
            else if (requested == "forgemaster") { requestedMode = 2; capability = "crafting.forgemaster_enabled"; }
            else if (requested == "master" || requested == "master_artisan")
            { requestedMode = 3; capability = "crafting.master_artisan_enabled"; }
            else
            {
                response.lines.push_back("Usage: /covenant artisan [off|reforge|forgemaster|master]");
                return response;
            }

            if (capability && !CovenantFlagUnlocked(context.playerKey, capability, "crafting"))
            {
                response.lines.push_back("That Artisan workshop mode is not unlocked at the current rank.");
                return response;
            }
            if (!progression::SetServerCovenantArtisanMode(context.playerKey, requestedMode))
            {
                response.lines.push_back("Could not persist Artisan workshop mode.");
                return response;
            }
            mode = requestedMode;
            response.lines.push_back(std::string("Artisan workshop mode set to ") + ArtisanModeName(mode) + ".");
        }

        std::vector<CovenantInventoryItem> items;
        if (!progression::GetServerCovenantInventory(context.playerKey, items))
        {
            response.lines.push_back("Could not load Covenant inventory.");
            return response;
        }
        std::ostringstream status;
        status << "Artisan workshop: " << ArtisanModeName(mode);
        response.lines.push_back(status.str());
        std::ostringstream materials;
        materials << "Materials: special " << CountInventoryPrefix(items, "special_material:")
                  << ", refined trophies " << CountInventoryPrefix(items, "refined_trophy:")
                  << ", refined materials " << CountInventoryPrefix(items, "refined_material:");
        response.lines.push_back(materials.str());
        response.lines.push_back("Reforge consumes 1 special material and changes one specialization property.");
        response.lines.push_back("Forgemaster consumes 1 refined trophy and class-specializes the next eligible craft.");
        response.lines.push_back("Master consumes 1 refined material and perfect-rolls the next eligible craft.");
        response.success = true;
        return response;
    }

    if (command == "mastery")
    {
        std::vector<int> ids;
        if (!progression::GetServerCovenantMasteries(context.playerKey, ids))
        { response.lines.push_back("Could not load Covenant mastery."); return response; }
        std::ostringstream line; line << "Covenant Mastery: " << ids.size() << "/5";
        response.lines.push_back(line.str());
        for (std::vector<int>::const_iterator it=ids.begin();it!=ids.end();++it)
            for (std::vector<CovenantCatalogEntry>::const_iterator c=catalog.begin();c!=catalog.end();++c)
                if (c->id == *it) response.lines.push_back(std::string("  mastered: ")+FriendlyName(c->key));
        response.success=true; return response;
    }

    if (command == "sigils" || command == "collection")
    {
        std::vector<std::string> keys;
        if (!progression::GetServerCovenantCollectibles(context.playerKey, keys))
        { response.lines.push_back("Could not load Covenant collection."); return response; }
        response.lines.push_back("Covenant collection:");
        if (keys.empty()) response.lines.push_back("  no Sigils collected yet");
        for (std::vector<std::string>::const_iterator it=keys.begin();it!=keys.end();++it)
            response.lines.push_back(std::string("  ")+*it);
        response.success=true; return response;
    }

    if (command == "inventory")
    {
        std::vector<CovenantInventoryItem> items;
        if (!progression::GetServerCovenantInventory(context.playerKey, items))
        { response.lines.push_back("Could not load Covenant inventory."); return response; }
        response.lines.push_back("Covenant inventory:");
        if (items.empty())
            response.lines.push_back("  empty");
        for (std::vector<CovenantInventoryItem>::const_iterator it = items.begin();
             it != items.end(); ++it)
        {
            std::ostringstream line;
            line << "  " << it->key << " x" << it->quantity;
            response.lines.push_back(line.str());
        }
        response.success = true;
        return response;
    }

    if (command == "refine")
    {
        if (tokens.size() < 3u)
        { response.lines.push_back("Usage: /covenant refine <trophy:key>"); return response; }
        std::string refined;
        if (!progression::RefineServerCovenantTrophy(context.playerKey, tokens[2], &refined))
        {
            response.lines.push_back(
                "Refine failed: requires active Hunters Tier II+ and one matching trophy.");
            return response;
        }
        response.lines.push_back(std::string("Refined trophy created: ") + refined + ".");
        response.success = true;
        return response;
    }

    if (command == "challenges")
    {
        int covenantId=0; std::string covenantName;
        if (tokens.size() >= 3u)
        {
            const CovenantCatalogEntry* selected=FindCovenant(catalog,tokens[2]);
            if(!selected){response.lines.push_back("Unknown Covenant name.");return response;}
            covenantId=selected->id;covenantName=FriendlyName(selected->key);
        }
        else
        {
            CovenantSnapshot active;
            if(!progression::GetServerActiveCovenantSnapshot(context.playerKey,active))
            {response.lines.push_back("No active Covenant.");return response;}
            covenantId=active.covenantId;covenantName=FriendlyName(active.key);
        }
        std::vector<CovenantChallengeState> states;
        if(!progression::GetServerCovenantChallenges(context.playerKey,covenantId,states))
        {response.lines.push_back("Could not load challenges.");return response;}
        response.lines.push_back(covenantName+" challenges:");
        for(std::vector<CovenantChallengeState>::const_iterator it=states.begin();it!=states.end();++it)
        {
            std::ostringstream line;line<<(it->completed?"[DONE] ":"[ ] ")<<it->key<<" "<<it->progress<<"/"<<it->targetCount;
            if(it->reputationReward>0)line<<" reward +"<<it->reputationReward<<" rep";
            response.lines.push_back(line.str());
        }
        response.success=true;return response;
    }

    if (command == "power")
    {
        CovenantSnapshot active;
        if(!progression::GetServerActiveCovenantSnapshot(context.playerKey,active))
        {response.lines.push_back("No active Covenant.");return response;}
        const std::string powerKey = PowerKeyForCovenant(active.covenantId);
        unsigned int target=0;
        if (powerKey.empty())
        {response.lines.push_back("This Covenant has no active Tier III power in the current content.");return response;}
        if(active.covenantId==catalog::kHunters)
        {
            if(!context.currentTargetIsHuntTarget || context.currentTargetSerial==0)
            {response.lines.push_back("Hunter's Mark requires a current boss/Hunt target.");return response;}
            target=context.currentTargetSerial;
        }
        std::string error;
        if(!progression::UseServerCovenantPower(context.playerKey,powerKey,target,&error))
        {response.lines.push_back(std::string("Power failed: ")+error+".");return response;}
        response.lines.push_back(std::string("Covenant power activated: ")+powerKey+".");
        response.success=true;return response;
    }

    if (tokens.size() < 3u)
    {
        response.lines.push_back("Covenant name is required.");
        AddHelp(response);
        return response;
    }

    const CovenantCatalogEntry* covenant = FindCovenant(catalog, tokens[2]);
    if (!covenant)
    {
        response.lines.push_back("Unknown Covenant name.");
        return response;
    }

    if (command == "info")
    {
        response.lines.push_back(FriendlyName(covenant->key));
        for (std::vector<CovenantRankCatalogEntry>::const_iterator rank = covenant->ranks.begin();
             rank != covenant->ranks.end(); ++rank)
        {
            std::ostringstream rankLine;
            rankLine << "Tier " << rank->id << " " << rank->key
                     << " - reputation " << rank->minReputation;
            response.lines.push_back(rankLine.str());
            for (std::vector<modifiers::GameplayModifier>::const_iterator modifier =
                     rank->benefits.begin(); modifier != rank->benefits.end(); ++modifier)
                response.lines.push_back(std::string("  ") + FormatModifier(*modifier));
        }
        CovenantSnapshot snapshot;
        if (progression::GetServerCovenantSnapshot(context.playerKey, covenant->id, snapshot))
        {
            AddSnapshotStatus(snapshot, response);
            AddRankProgress(snapshot, *covenant, response);
        }
        response.success = true;
        return response;
    }

    if (command == "join")
    {
        const CovenantJoinEligibility eligibility =
            progression::CheckServerCovenantJoinEligibility(
                context.playerKey, covenant->id, context.playerLevel, context.legacyJobCode);
        if (eligibility != CovenantJoinEligibility::Eligible)
        {
            response.lines.push_back(std::string("Join failed: ") + EligibilityMessage(eligibility) + ".");
            return response;
        }
        if (!progression::JoinServerCovenant(
                context.playerKey, covenant->id, context.playerLevel, context.legacyJobCode))
        {
            response.lines.push_back("Join failed: persistence/runtime failure.");
            return response;
        }
        CovenantSnapshot snapshot;
        if (progression::GetServerCovenantSnapshot(context.playerKey, covenant->id, snapshot))
        {
            response.lines.push_back(std::string("Joined ") + FriendlyName(covenant->key) + ".");
            response.lines.push_back(snapshot.membership.active
                                         ? "This Covenant is now active."
                                         : "Your previous Covenant remains active. Use /covenant activate to switch.");
        }
        response.success = true;
        return response;
    }

    if (command == "activate" || command == "switch")
    {
        if (!context.canSwitch)
        {
            response.lines.push_back(std::string("Covenant switch blocked: ") + (context.switchBlockReason.empty() ? "player state" : context.switchBlockReason) + ".");
            return response;
        }
        const CovenantActivationResult result = progression::ActivateServerCovenant(
            context.playerKey, covenant->id, context.playerLevel, context.legacyJobCode);
        response.success = result == CovenantActivationResult::Activated ||
                           result == CovenantActivationResult::AlreadyActive;
        response.lines.push_back(FriendlyName(covenant->key) + ": " + ActivationMessage(result) + ".");
        return response;
    }

    if (command == "rep")
    {
        if (!context.allowAdminCommands)
        {
            response.lines.push_back("Admin Covenant command denied.");
            return response;
        }
        if (tokens.size() < 5u)
        {
            response.lines.push_back("Usage: /covenant rep <name> <amount> <unique-source-key>");
            return response;
        }
        const int amount = std::atoi(tokens[3].c_str());
        if (amount == 0)
        {
            response.lines.push_back("Reputation amount must be non-zero.");
            return response;
        }
        if (!progression::AddServerCovenantReputation(
                context.playerKey, covenant->id, amount, tokens[4],
                context.playerLevel, context.legacyJobCode))
        {
            response.lines.push_back("Reputation update failed (Covenant must be active and source key unique).");
            return response;
        }
        response.lines.push_back("Covenant reputation updated.");
        response.success = true;
        return response;
    }

    response.lines.push_back("Unknown Covenant command.");
    AddHelp(response);
    return response;
}
}
}
