#include "systems/gameplay/covenants/v2/CovenantUIHandler.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>
#include <vector>

#include "gameplay/covenants/v2/CovenantCatalog.h"
#include "systems/gameplay/progression/v2/ProgressionRuntime.h"
#include "systems/gameplay/mining/v2/MiningRuntime.h"
#include "systems/gameplay/mining/v2/MiningInteractionRuntime.h"
#include "systems/gameplay/gathering/v2/GatheringRuntime.h"
#include "systems/network/transport/legacy/smwsock.h"
#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "core/server/legacy_runtime/onserver.h"

extern DWORD dwPlayServTime;

namespace gameplay
{
namespace covenants
{
namespace
{
using namespace protocol;

void CopyText(char* dst, std::size_t size, const std::string& value)
{
    if (!dst || size == 0)
        return;
    std::memset(dst, 0, size);
    if (!value.empty())
        std::strncpy(dst, value.c_str(), size - 1);
}

bool ContainsId(const std::vector<int>& ids, int id)
{
    return std::find(ids.begin(), ids.end(), id) != ids.end();
}

const CovenantCatalogEntry* FindCatalog(const std::vector<CovenantCatalogEntry>& catalog, int id)
{
    for (std::vector<CovenantCatalogEntry>::const_iterator it = catalog.begin(); it != catalog.end(); ++it)
        if (it->id == id)
            return &*it;
    return 0;
}

const CovenantRankCatalogEntry* FindRank(const CovenantCatalogEntry& covenant, int rankId)
{
    for (std::vector<CovenantRankCatalogEntry>::const_iterator it = covenant.ranks.begin();
         it != covenant.ranks.end(); ++it)
        if (it->id == rankId)
            return &*it;
    return covenant.ranks.empty() ? 0 : &covenant.ranks.front();
}

std::string PowerKeyForCovenant(int covenantId)
{
    if (covenantId == catalog::kHunters) return "hunters_mark";
    if (covenantId == catalog::kFortune) return "tempt_fate";
    if (covenantId == catalog::kAscendants) return "focused_training";
    return std::string();
}

bool CanSwitch(rsPLAYINFO* owner, std::string& reason)
{
    reason.clear();
    if (!owner)
    {
        reason = "invalid player";
        return false;
    }
    if (owner->smCharInfo.Life[0] <= 0)
    {
        reason = "Nao pode trocar Covenant enquanto estiver morto.";
        return false;
    }
    if (owner->dwLastAttackTime && (dwPlayServTime - owner->dwLastAttackTime) < 10000)
    {
        reason = "Nao pode trocar Covenant durante combate.";
        return false;
    }
    if (owner->dwLastTradeTime && (dwPlayServTime - owner->dwLastTradeTime) < 8000)
    {
        reason = "Nao pode trocar Covenant durante trade.";
        return false;
    }
    return true;
}

int NextRankReputation(const CovenantCatalogEntry& entry, int reputation)
{
    for (std::vector<CovenantRankCatalogEntry>::const_iterator it = entry.ranks.begin();
         it != entry.ranks.end(); ++it)
        if (it->minReputation > reputation)
            return it->minReputation;
    return 0;
}


const char* HarvestStatusText(gathering::HarvestStatus status)
{
    switch(status)
    {
        case gathering::HarvestStatus::Success: return "success";
        case gathering::HarvestStatus::InvalidPlayer: return "jogador invalido";
        case gathering::HarvestStatus::UnknownNode: return "nenhum node encontrado";
        case gathering::HarvestStatus::UnknownResource: return "recurso desconhecido";
        case gathering::HarvestStatus::WrongArea: return "node em outra area";
        case gathering::HarvestStatus::TooFar: return "chegue mais perto do recurso";
        case gathering::HarvestStatus::WrongProfession: return "profissao incorreta";
        case gathering::HarvestStatus::ProfessionLevelTooLow: return "nivel de profissao insuficiente";
        case gathering::HarvestStatus::Depleted: return "recurso em respawn";
    }
    return "falha de coleta";
}

int ArtisanMaxModeForRank(int rankId)
{
    if (rankId >= 3) return 3;
    if (rankId == 2) return 2;
    if (rankId == 1) return 1;
    return 0;
}

void FillWireModifier(CovenantUIModifier& out, const modifiers::GameplayModifier& modifier)
{
    out.operation = static_cast<int>(modifier.operation);
    out.valueBasisPoints = static_cast<int>(std::lround(modifier.value * 10000.0));
    CopyText(out.statKey, sizeof(out.statKey), modifier.statKey);
    CopyText(out.scopeKey, sizeof(out.scopeKey), modifier.scopeKey);
}

void FillPowerState(const std::string& playerKey,
                    int covenantId,
                    int rankId,
                    CovenantUIPowerState& out)
{
    const std::string powerKey = PowerKeyForCovenant(covenantId);
    if (powerKey.empty())
        return;
    CovenantPowerDefinition definition;
    CovenantPowerState state;
    if (!progression::GetServerCovenantPowerState(playerKey, powerKey, definition, state))
        return;
    out.available = rankId >= definition.requiredRankId ? 1 : 0;
    out.requiredRankId = definition.requiredRankId;
    out.durationSeconds = definition.durationSeconds;
    out.cooldownSeconds = definition.cooldownSeconds;
    out.targetMode = static_cast<int>(definition.targetMode);
    out.activeUntil = state.activeUntil;
    out.cooldownUntil = state.cooldownUntil;
    out.targetSerial = state.targetSerial;
    CopyText(out.key, sizeof(out.key), powerKey);
}

void FillSurvey(CovenantUIStatePacket& packet)
{
    std::vector<gathering::ResourceDefinition> resources;
    std::vector<gathering::GatheringNodeDefinition> nodes;
    mining::ServerMiningService().ListResources(resources);
    mining::ServerMiningService().ListNodes(nodes);
    std::vector<gathering::ResourceDefinition> gatheringResources;
    std::vector<gathering::GatheringNodeDefinition> gatheringNodes;
    gathering::ServerGatheringService().ListResources(gatheringResources);
    gathering::ServerGatheringService().ListNodes(gatheringNodes);
    resources.insert(resources.end(), gatheringResources.begin(), gatheringResources.end());
    nodes.insert(nodes.end(), gatheringNodes.begin(), gatheringNodes.end());

    for (std::vector<gathering::GatheringNodeDefinition>::const_iterator node = nodes.begin();
         node != nodes.end() && packet.surveyAreaCount < COVENANT_UI_MAX_SURVEY_AREAS; ++node)
    {
        std::string resourceKey;
        for (std::vector<gathering::ResourceDefinition>::const_iterator resource = resources.begin();
             resource != resources.end(); ++resource)
        {
            if (resource->resourceId == node->resourceId)
            {
                resourceKey = resource->key;
                break;
            }
        }
        if (resourceKey.empty())
            continue;

        bool merged = false;
        for (int i = 0; i < packet.surveyAreaCount; ++i)
        {
            CovenantUISurveyArea& area = packet.surveyAreas[i];
            if (area.area == node->area && resourceKey == area.resourceKey)
            {
                ++area.nodeCount;
                merged = true;
                break;
            }
        }
        if (!merged)
        {
            CovenantUISurveyArea& area = packet.surveyAreas[packet.surveyAreaCount++];
            area.area = node->area;
            area.nodeCount = 1;
            CopyText(area.resourceKey, sizeof(area.resourceKey), resourceKey);
        }
    }
}
}

void SendCovenantUIState(rsPLAYINFO* owner,
                         int selectedCovenantId,
                         int lastResult,
                         const char* lastMessage)
{
    if (!owner || !owner->lpsmSock || !owner->getName() || !owner->getName()[0])
        return;

    CovenantUIStatePacket packet{};
    packet.size = sizeof(packet);
    packet.code = OPCODE_COVENANT_UI_STATE;
    packet.version = COVENANT_UI_PROTOCOL_VERSION;
    packet.lastResult = lastResult;
    packet.serverNow = static_cast<std::int64_t>(std::time(0));
    if (lastMessage)
        CopyText(packet.lastMessage, sizeof(packet.lastMessage), lastMessage);

    if (!progression::IsServerProgressionRuntimeReady())
    {
        packet.lastResult = COVENANT_UI_RESULT_RUNTIME_NOT_READY;
        CopyText(packet.lastMessage, sizeof(packet.lastMessage), "Covenant runtime indisponivel.");
        owner->lpsmSock->Send(reinterpret_cast<char*>(&packet), packet.size, TRUE);
        return;
    }

    const std::string playerKey = owner->getName();
    std::vector<CovenantCatalogEntry> catalogEntries;
    progression::GetServerCovenantCatalog(catalogEntries);
    std::vector<int> masteries;
    (void)progression::GetServerCovenantMasteries(playerKey, masteries);

    CovenantSnapshot activeSnapshot;
    const bool hasActive = progression::GetServerActiveCovenantSnapshot(playerKey, activeSnapshot);
    if (!FindCatalog(catalogEntries, selectedCovenantId))
        selectedCovenantId = hasActive ? activeSnapshot.covenantId :
            (!catalogEntries.empty() ? catalogEntries.front().id : 0);
    packet.selectedCovenantId = selectedCovenantId;
    packet.activeCovenantId = hasActive ? activeSnapshot.covenantId : 0;

    if (hasActive)
    {
        const CovenantCatalogEntry* activeCatalog = FindCatalog(catalogEntries, activeSnapshot.covenantId);
        if (activeCatalog)
        {
            const CovenantRankCatalogEntry* activeRank = FindRank(*activeCatalog, activeSnapshot.membership.rankId);
            if (activeRank)
            {
                for (std::vector<modifiers::GameplayModifier>::const_iterator modifier = activeRank->benefits.begin();
                     modifier != activeRank->benefits.end() && packet.activeModifierCount < COVENANT_UI_MAX_MODIFIERS; ++modifier)
                    FillWireModifier(packet.activeModifiers[packet.activeModifierCount++], *modifier);
            }
            FillPowerState(playerKey, activeSnapshot.covenantId, activeSnapshot.membership.rankId, packet.activePower);
        }
    }

    for (std::vector<CovenantCatalogEntry>::const_iterator it = catalogEntries.begin();
         it != catalogEntries.end() && packet.entryCount < COVENANT_UI_MAX_COVENANTS; ++it)
    {
        CovenantUIEntry& out = packet.entries[packet.entryCount++];
        out.covenantId = it->id;
        out.mastered = ContainsId(masteries, it->id) ? 1 : 0;
        CopyText(out.key, sizeof(out.key), it->key);

        CovenantSnapshot snapshot;
        if (progression::GetServerCovenantSnapshot(playerKey, it->id, snapshot))
        {
            out.joined = 1;
            out.active = snapshot.membership.active ? 1 : 0;
            out.rankId = snapshot.membership.rankId;
            out.reputation = snapshot.membership.reputation;
            out.nextRankReputation = NextRankReputation(*it, snapshot.membership.reputation);
            CopyText(out.rankKey, sizeof(out.rankKey), snapshot.rankKey);
        }
        else
        {
            const CovenantJoinEligibility eligibility = progression::CheckServerCovenantJoinEligibility(
                playerKey, it->id, owner->smCharInfo.Level, owner->smCharInfo.JOB_CODE);
            out.canJoin = eligibility == CovenantJoinEligibility::Eligible ? 1 : 0;
        }
    }

    long long switchCooldown = 0;
    if (progression::GetServerCovenantSwitchCooldown(playerKey, switchCooldown))
        packet.switchCooldownUntil = static_cast<std::int64_t>(switchCooldown);

    const CovenantCatalogEntry* selected = FindCatalog(catalogEntries, selectedCovenantId);
    CovenantSnapshot selectedSnapshot;
    const bool joined = selected && progression::GetServerCovenantSnapshot(
        playerKey, selectedCovenantId, selectedSnapshot);

    if (selected)
    {
        for (std::vector<CovenantRankCatalogEntry>::const_iterator rank = selected->ranks.begin();
             rank != selected->ranks.end() && packet.rankCount < COVENANT_UI_MAX_RANKS; ++rank)
        {
            CovenantUIRankSummary& out = packet.ranks[packet.rankCount++];
            out.rankId = rank->id;
            out.minReputation = rank->minReputation;
            CopyText(out.key, sizeof(out.key), rank->key);
        }

        const int displayRankId = joined ? selectedSnapshot.membership.rankId :
            (!selected->ranks.empty() ? selected->ranks.front().id : 0);
        const CovenantRankCatalogEntry* displayRank = FindRank(*selected, displayRankId);
        if (displayRank)
        {
            for (std::vector<modifiers::GameplayModifier>::const_iterator modifier = displayRank->benefits.begin();
                 modifier != displayRank->benefits.end() && packet.modifierCount < COVENANT_UI_MAX_MODIFIERS; ++modifier)
            {
                FillWireModifier(packet.modifiers[packet.modifierCount++], *modifier);
            }
        }
    }

    if (joined)
    {
        std::vector<CovenantChallengeState> challenges;
        if (progression::GetServerCovenantChallenges(playerKey, selectedCovenantId, challenges))
        {
            for (std::vector<CovenantChallengeState>::const_iterator challenge = challenges.begin();
                 challenge != challenges.end() && packet.challengeCount < COVENANT_UI_MAX_CHALLENGES; ++challenge)
            {
                CovenantUIChallenge& out = packet.challenges[packet.challengeCount++];
                out.targetCount = challenge->targetCount;
                out.progress = challenge->progress;
                out.reputationReward = challenge->reputationReward;
                out.completed = challenge->completed ? 1 : 0;
                out.rewardClaimed = challenge->rewardClaimed ? 1 : 0;
                CopyText(out.key, sizeof(out.key), challenge->key);
                CopyText(out.activityKey, sizeof(out.activityKey), challenge->activityKey);
            }
        }

        std::vector<CovenantInventoryItem> inventory;
        if (progression::GetServerCovenantInventory(playerKey, inventory))
        {
            for (std::vector<CovenantInventoryItem>::const_iterator item = inventory.begin();
                 item != inventory.end() && packet.inventoryCount < COVENANT_UI_MAX_INVENTORY; ++item)
            {
                if (item->covenantId != selectedCovenantId || item->quantity <= 0)
                    continue;
                CovenantUIInventoryItem& out = packet.inventory[packet.inventoryCount++];
                out.covenantId = item->covenantId;
                out.quantity = item->quantity;
                CopyText(out.key, sizeof(out.key), item->key);
            }
        }
    }

    if (selectedCovenantId == catalog::kGatherers && joined && selectedSnapshot.membership.rankId >= 1)
    {
        FillSurvey(packet);
        std::vector<gathering::GatheringInventoryEntry> resources;
        if (gathering::ServerGatheringRepository().GetInventory(playerKey, resources))
        {
            for (std::vector<gathering::GatheringInventoryEntry>::const_iterator item = resources.begin();
                 item != resources.end() && packet.resourceCount < COVENANT_UI_MAX_RESOURCES; ++item)
            {
                if (item->quantity <= 0)
                    continue;
                CovenantUIResourceItem& out = packet.resources[packet.resourceCount++];
                out.quantity = item->quantity;
                CopyText(out.key, sizeof(out.key), item->resourceKey);
            }
        }
    }

    if (selectedCovenantId == catalog::kArtisans)
    {
        (void)progression::GetServerCovenantArtisanMode(playerKey, packet.artisanMode);
        packet.artisanMaxMode = joined ? ArtisanMaxModeForRank(selectedSnapshot.membership.rankId) : 0;
    }

    if (joined && selectedSnapshot.membership.active)
        FillPowerState(playerKey, selectedCovenantId, selectedSnapshot.membership.rankId, packet.power);

    owner->lpsmSock->Send(reinterpret_cast<char*>(&packet), packet.size, TRUE);
}

void HandleCovenantUIAction(rsPLAYINFO* owner,
                            const CovenantUIActionPacket* packet)
{
    if (!owner || !packet || packet->size != sizeof(CovenantUIActionPacket) ||
        packet->code != OPCODE_COVENANT_UI_ACTION ||
        packet->version != COVENANT_UI_PROTOCOL_VERSION)
        return;

    const int covenantId = packet->covenantId;
    const std::string playerKey = owner->getName() ? owner->getName() : "";
    int result = COVENANT_UI_RESULT_NONE;
    std::string message;

    switch (packet->action)
    {
        case COVENANT_UI_REFRESH:
            break;

        case COVENANT_UI_JOIN:
        {
            const CovenantJoinEligibility eligibility = progression::CheckServerCovenantJoinEligibility(
                playerKey, covenantId, owner->smCharInfo.Level, owner->smCharInfo.JOB_CODE);
            if (eligibility != CovenantJoinEligibility::Eligible)
            {
                result = COVENANT_UI_RESULT_REJECTED;
                message = eligibility == CovenantJoinEligibility::AlreadyMember ?
                    "Voce ja pertence a esta Covenant." : "Requisitos de entrada nao atendidos.";
                break;
            }
            if (progression::JoinServerCovenant(
                    playerKey, covenantId, owner->smCharInfo.Level, owner->smCharInfo.JOB_CODE))
            {
                result = COVENANT_UI_RESULT_OK;
                message = "Covenant adicionada ao personagem.";
            }
            else
            {
                result = COVENANT_UI_RESULT_REJECTED;
                message = "Nao foi possivel entrar na Covenant.";
            }
            break;
        }

        case COVENANT_UI_ACTIVATE:
        {
            std::string blockReason;
            if (!CanSwitch(owner, blockReason))
            {
                result = COVENANT_UI_RESULT_REJECTED;
                message = blockReason;
                break;
            }
            const CovenantActivationResult activation = progression::ActivateServerCovenant(
                playerKey, covenantId, owner->smCharInfo.Level, owner->smCharInfo.JOB_CODE);
            if (activation == CovenantActivationResult::Activated)
            {
                result = COVENANT_UI_RESULT_OK;
                message = "Covenant ativada.";
            }
            else
            {
                result = COVENANT_UI_RESULT_REJECTED;
                if (activation == CovenantActivationResult::CooldownActive) message = "Troca de Covenant em cooldown.";
                else if (activation == CovenantActivationResult::NotMember) message = "Entre nesta Covenant primeiro.";
                else if (activation == CovenantActivationResult::AlreadyActive) message = "Esta Covenant ja esta ativa.";
                else message = "Nao foi possivel ativar a Covenant.";
            }
            break;
        }

        case COVENANT_UI_USE_POWER:
        {
            const std::string powerKey = PowerKeyForCovenant(covenantId);
            unsigned int targetSerial = 0;
            if (covenantId == catalog::kHunters)
            {
                if (!owner->lpChrTarget || owner->lpChrTarget->smCharInfo.wPlayClass[0] != MONSTER_CLASS_BOSS)
                {
                    result = COVENANT_UI_RESULT_REJECTED;
                    message = "Hunter's Mark requer um Hunt/Boss selecionado.";
                    break;
                }
                targetSerial = owner->lpChrTarget->dwObjectSerial;
            }
            std::string error;
            if (!powerKey.empty() && progression::UseServerCovenantPower(
                    playerKey, powerKey, targetSerial, &error))
            {
                result = COVENANT_UI_RESULT_OK;
                message = "Poder da Covenant ativado.";
            }
            else
            {
                result = COVENANT_UI_RESULT_REJECTED;
                message = error.empty() ? "Poder indisponivel." : error;
            }
            break;
        }

        case COVENANT_UI_SET_ARTISAN_MODE:
            if (covenantId != catalog::kArtisans || packet->value < 0 || packet->value > 3)
            {
                result = COVENANT_UI_RESULT_INVALID;
                message = "Modo Artisan invalido.";
            }
            else
            {
                CovenantSnapshot snapshot;
                const bool active = progression::GetServerCovenantSnapshot(playerKey, covenantId, snapshot) &&
                                    snapshot.membership.active;
                const int maxMode = active ? ArtisanMaxModeForRank(snapshot.membership.rankId) : 0;
                if (!active || packet->value > maxMode)
                {
                    result = COVENANT_UI_RESULT_REJECTED;
                    message = "Modo Artisan bloqueado pelo rank/Covenant ativa.";
                }
                else if (progression::SetServerCovenantArtisanMode(playerKey, packet->value))
                {
                    result = COVENANT_UI_RESULT_OK;
                    message = "Modo Artisan atualizado.";
                }
                else
                {
                    result = COVENANT_UI_RESULT_REJECTED;
                    message = "Falha ao persistir modo Artisan.";
                }
            }
            break;


        case COVENANT_UI_HARVEST_MINING:
        case COVENANT_UI_HARVEST_GATHERING:
        {
            const bool miningAction = packet->action == COVENANT_UI_HARVEST_MINING;
            gathering::GatheringNodeDefinition node;
            gathering::ResourceDefinition resource;
            const int area = owner->Position.Area;
            const int x = owner->Position.x >> FLOATNS;
            const int y = owner->Position.y >> FLOATNS;
            const int z = owner->Position.z >> FLOATNS;
            const gathering::GatheringKind kind = miningAction ? gathering::GatheringKind::Mining : gathering::GatheringKind::Custom;
            if (!gathering::FindNearestGatheringNode(area, x, y, z, kind, miningAction, node, resource))
            {
                result = COVENANT_UI_RESULT_REJECTED;
                message = miningAction ? "Nenhum node de mineracao nesta area." : "Nenhum node de coleta nesta area.";
                break;
            }
            gathering::HarvestResult harvest = miningAction ?
                mining::HarvestMiningNodeForPlayer(owner, node.nodeId, static_cast<std::uint64_t>(dwPlayServTime), static_cast<std::uint32_t>(rand())) :
                gathering::HarvestGatheringNodeForPlayer(owner, node.nodeId, static_cast<std::uint64_t>(dwPlayServTime), static_cast<std::uint32_t>(rand()));
            if (!harvest.Succeeded())
            {
                result = COVENANT_UI_RESULT_REJECTED;
                message = HarvestStatusText(harvest.status);
            }
            else
            {
                result = COVENANT_UI_RESULT_OK;
                char buffer[96] = {0};
                std::snprintf(buffer, sizeof(buffer), "%s x%d coletado.", harvest.resourceKey.c_str(), harvest.amount);
                message = buffer;
            }
            break;
        }

        case COVENANT_UI_REFINE_TROPHY:
        {
            std::string refined;
            const std::string key = packet->key;
            if (progression::RefineServerCovenantTrophy(playerKey, key, &refined))
            {
                result = COVENANT_UI_RESULT_OK;
                message = std::string("Trofeu refinado: ") + refined;
            }
            else
            {
                result = COVENANT_UI_RESULT_REJECTED;
                message = "Nao foi possivel refinar o trofeu.";
            }
            break;
        }

        default:
            result = COVENANT_UI_RESULT_INVALID;
            message = "Acao Covenant invalida.";
            break;
    }

    SendCovenantUIState(owner, covenantId, result, message.empty() ? 0 : message.c_str());
}
}
}
