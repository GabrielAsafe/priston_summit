#include "systems/ui/v2/legacy/LegacyCovenantGateway.h"

#include <algorithm>
#include <cstring>

#include "systems/network/client/netplay.h"
#include "systems/network/transport/legacy/smwsock.h"

namespace ui::v2
{
namespace
{
using namespace gameplay::covenants::protocol;

std::int32_t ToWireAction(CovenantUiAction action)
{
    switch (action)
    {
        case CovenantUiAction::Refresh: return COVENANT_UI_REFRESH;
        case CovenantUiAction::Join: return COVENANT_UI_JOIN;
        case CovenantUiAction::Activate: return COVENANT_UI_ACTIVATE;
        case CovenantUiAction::UsePower: return COVENANT_UI_USE_POWER;
        case CovenantUiAction::SetArtisanMode: return COVENANT_UI_SET_ARTISAN_MODE;
        case CovenantUiAction::RefineTrophy: return COVENANT_UI_REFINE_TROPHY;
        case CovenantUiAction::HarvestMining: return COVENANT_UI_HARVEST_MINING;
        case CovenantUiAction::HarvestGathering: return COVENANT_UI_HARVEST_GATHERING;
    }
    return COVENANT_UI_REFRESH;
}
}

LegacyCovenantGateway& GetLegacyCovenantGateway()
{
    static LegacyCovenantGateway gateway;
    return gateway;
}

void LegacyCovenantGateway::SelectCovenant(int covenantId)
{
    if (covenantId <= 0)
        return;
    selectedCovenantId_ = covenantId;
    RequestRefresh();
}

void LegacyCovenantGateway::RequestRefresh()
{
    Execute(CovenantUiAction::Refresh);
}

void LegacyCovenantGateway::Execute(CovenantUiAction action, int value, const std::string& key)
{
    using namespace gameplay::covenants::protocol;
    if (!smWsockServer)
        return;

    CovenantUIActionPacket packet{};
    packet.size = sizeof(packet);
    packet.code = OPCODE_COVENANT_UI_ACTION;
    packet.version = COVENANT_UI_PROTOCOL_VERSION;
    packet.action = ToWireAction(action);
    packet.covenantId = selectedCovenantId_;
    packet.value = value;
    if (!key.empty())
        std::strncpy(packet.key, key.c_str(), sizeof(packet.key) - 1);

    smWsockServer->Send(reinterpret_cast<char*>(&packet), packet.size, TRUE);
}

void LegacyCovenantGateway::ApplyState(const gameplay::covenants::protocol::CovenantUIStatePacket& packet)
{
    using namespace gameplay::covenants::protocol;
    if (packet.version != COVENANT_UI_PROTOCOL_VERSION)
        return;

    snapshot_ = CovenantUiSnapshot{};
    snapshot_.valid = true;
    snapshot_.selectedCovenantId = packet.selectedCovenantId;
    snapshot_.activeCovenantId = packet.activeCovenantId;
    selectedCovenantId_ = packet.selectedCovenantId > 0 ? packet.selectedCovenantId : selectedCovenantId_;
    snapshot_.entryCount = std::clamp(packet.entryCount, 0, COVENANT_UI_MAX_COVENANTS);
    snapshot_.rankCount = std::clamp(packet.rankCount, 0, COVENANT_UI_MAX_RANKS);
    snapshot_.modifierCount = std::clamp(packet.modifierCount, 0, COVENANT_UI_MAX_MODIFIERS);
    snapshot_.activeModifierCount = std::clamp(packet.activeModifierCount, 0, COVENANT_UI_MAX_MODIFIERS);
    snapshot_.challengeCount = std::clamp(packet.challengeCount, 0, COVENANT_UI_MAX_CHALLENGES);
    snapshot_.inventoryCount = std::clamp(packet.inventoryCount, 0, COVENANT_UI_MAX_INVENTORY);
    snapshot_.surveyAreaCount = std::clamp(packet.surveyAreaCount, 0, COVENANT_UI_MAX_SURVEY_AREAS);
    snapshot_.resourceCount = std::clamp(packet.resourceCount, 0, COVENANT_UI_MAX_RESOURCES);
    snapshot_.artisanMode = packet.artisanMode;
    snapshot_.artisanMaxMode = packet.artisanMaxMode;
    snapshot_.lastResult = packet.lastResult;
    snapshot_.serverNow = packet.serverNow;
    snapshot_.switchCooldownUntil = packet.switchCooldownUntil;
    snapshot_.lastMessage = packet.lastMessage;

    snapshot_.power.available = packet.power.available != 0;
    snapshot_.power.requiredRankId = packet.power.requiredRankId;
    snapshot_.power.durationSeconds = packet.power.durationSeconds;
    snapshot_.power.cooldownSeconds = packet.power.cooldownSeconds;
    snapshot_.power.targetMode = packet.power.targetMode;
    snapshot_.power.activeUntil = packet.power.activeUntil;
    snapshot_.power.cooldownUntil = packet.power.cooldownUntil;
    snapshot_.power.targetSerial = packet.power.targetSerial;
    snapshot_.power.key = packet.power.key;

    snapshot_.activePower.available = packet.activePower.available != 0;
    snapshot_.activePower.requiredRankId = packet.activePower.requiredRankId;
    snapshot_.activePower.durationSeconds = packet.activePower.durationSeconds;
    snapshot_.activePower.cooldownSeconds = packet.activePower.cooldownSeconds;
    snapshot_.activePower.targetMode = packet.activePower.targetMode;
    snapshot_.activePower.activeUntil = packet.activePower.activeUntil;
    snapshot_.activePower.cooldownUntil = packet.activePower.cooldownUntil;
    snapshot_.activePower.targetSerial = packet.activePower.targetSerial;
    snapshot_.activePower.key = packet.activePower.key;

    for (int i = 0; i < snapshot_.entryCount; ++i)
    {
        const auto& source = packet.entries[i];
        auto& target = snapshot_.entries[static_cast<std::size_t>(i)];
        target.covenantId = source.covenantId;
        target.joined = source.joined != 0;
        target.active = source.active != 0;
        target.mastered = source.mastered != 0;
        target.rankId = source.rankId;
        target.reputation = source.reputation;
        target.nextRankReputation = source.nextRankReputation;
        target.canJoin = source.canJoin != 0;
        target.key = source.key;
        target.rankKey = source.rankKey;
    }
    for (int i = 0; i < snapshot_.rankCount; ++i)
    {
        const auto& source = packet.ranks[i];
        auto& target = snapshot_.ranks[static_cast<std::size_t>(i)];
        target.rankId = source.rankId;
        target.minReputation = source.minReputation;
        target.key = source.key;
    }
    for (int i = 0; i < snapshot_.modifierCount; ++i)
    {
        const auto& source = packet.modifiers[i];
        auto& target = snapshot_.modifiers[static_cast<std::size_t>(i)];
        target.operation = source.operation;
        target.valueBasisPoints = source.valueBasisPoints;
        target.statKey = source.statKey;
        target.scopeKey = source.scopeKey;
    }
    for (int i = 0; i < snapshot_.activeModifierCount; ++i)
    {
        const auto& source = packet.activeModifiers[i];
        auto& target = snapshot_.activeModifiers[static_cast<std::size_t>(i)];
        target.operation = source.operation;
        target.valueBasisPoints = source.valueBasisPoints;
        target.statKey = source.statKey;
        target.scopeKey = source.scopeKey;
    }
    for (int i = 0; i < snapshot_.challengeCount; ++i)
    {
        const auto& source = packet.challenges[i];
        auto& target = snapshot_.challenges[static_cast<std::size_t>(i)];
        target.targetCount = source.targetCount;
        target.progress = source.progress;
        target.reputationReward = source.reputationReward;
        target.completed = source.completed != 0;
        target.rewardClaimed = source.rewardClaimed != 0;
        target.key = source.key;
        target.activityKey = source.activityKey;
    }
    for (int i = 0; i < snapshot_.inventoryCount; ++i)
    {
        const auto& source = packet.inventory[i];
        auto& target = snapshot_.inventory[static_cast<std::size_t>(i)];
        target.covenantId = source.covenantId;
        target.quantity = source.quantity;
        target.key = source.key;
    }
    for (int i = 0; i < snapshot_.surveyAreaCount; ++i)
    {
        const auto& source = packet.surveyAreas[i];
        auto& target = snapshot_.surveyAreas[static_cast<std::size_t>(i)];
        target.area = source.area;
        target.nodeCount = source.nodeCount;
        target.resourceKey = source.resourceKey;
    }
    for (int i = 0; i < snapshot_.resourceCount; ++i)
    {
        const auto& source = packet.resources[i];
        auto& target = snapshot_.resources[static_cast<std::size_t>(i)];
        target.quantity = source.quantity;
        target.key = source.key;
    }
}


void LegacyCovenantGateway::Tick(unsigned long nowMilliseconds)
{
    if (!smWsockServer)
    {
        lastRefreshTick_ = 0;
        return;
    }
    const unsigned long interval = snapshot_.valid ? 5000UL : 1000UL;
    if (lastRefreshTick_ == 0 || static_cast<unsigned long>(nowMilliseconds - lastRefreshTick_) >= interval)
    {
        lastRefreshTick_ = nowMilliseconds;
        RequestRefresh();
    }
}

void LegacyCovenantGateway::Reset()
{
    snapshot_ = CovenantUiSnapshot{};
    selectedCovenantId_ = 1;
    lastRefreshTick_ = 0;
}

void ApplyLegacyCovenantStatePacket(const gameplay::covenants::protocol::CovenantUIStatePacket& packet)
{
    GetLegacyCovenantGateway().ApplyState(packet);
}
}
