#include "systems/gameplay/progression/v2/LegacyProgressionGateways.h"
#include "systems/gameplay/progression/v2/ProgressionTriggerRuntime.h"

#include "core/server/legacy_runtime/onserver.h"

int rsSendMoneyToUser(rsPLAYINFO* lpPlayInfo, int Money);

namespace gameplay
{
namespace progression
{
LegacyProgressionRewardGateway::LegacyProgressionRewardGateway(
    story::IStoryRepository& storyRepository,
    covenants::CovenantService& covenantService,
    reputation::ReputationService& reputationService,
    ProgressionCustomHandlerRegistry& customHandlers,
    const inventory::IItemCatalog& itemCatalog,
    inventory::IInventoryCommands& inventoryCommands,
    const player::legacy::LegacyOnlinePlayerRegistry& players)
    : core_(storyRepository, covenantService, reputationService), customHandlers_(customHandlers),
      itemCatalog_(itemCatalog), inventoryCommands_(inventoryCommands), players_(players), itemCatalogReady_(false)
{
}

bool LegacyProgressionRewardGateway::Supports(rewards::RewardType type) const
{
    return core_.Supports(type) ||
           type == rewards::RewardType::Gold ||
           type == rewards::RewardType::Experience ||
           type == rewards::RewardType::Item ||
           type == rewards::RewardType::Custom;
}

bool LegacyProgressionRewardGateway::Validate(const rewards::RewardDefinition& reward) const
{
    if (!Supports(reward.type))
        return false;

    if (core_.Supports(reward.type))
        return core_.Validate(reward);

    switch (reward.type)
    {
    case rewards::RewardType::Gold:
    case rewards::RewardType::Experience:
        return reward.amount > 0;
    case rewards::RewardType::Item:
    {
        if (reward.key.empty() || reward.amount != 1)
            return false;
        if(!itemCatalogReady_)
            return true;
        inventory::ItemDefinition definition;
        return ResolveGrantableItem(reward.key, definition);
    }
    case rewards::RewardType::Custom:
        return customHandlers_.ValidateReward(reward);
    case rewards::RewardType::Reputation:
    case rewards::RewardType::StoryFlag:
    case rewards::RewardType::CovenantReputation:
        return false;
    }
    return false;
}

bool LegacyProgressionRewardGateway::IsRetrySafe(rewards::RewardType type) const
{
    return core_.Supports(type) ? core_.IsRetrySafe(type) : false;
}

bool LegacyProgressionRewardGateway::IsRetrySafe(const rewards::RewardDefinition& reward) const
{
    if (core_.Supports(reward.type))
        return core_.IsRetrySafe(reward.type);
    if (reward.type == rewards::RewardType::Custom)
        return customHandlers_.IsRewardRetrySafe(reward);
    return false;
}

rewards::RewardResult LegacyProgressionRewardGateway::Grant(
    const std::string& playerKey,
    const rewards::RewardDefinition& reward,
    const std::string& sourceKey)
{
    if (core_.Supports(reward.type))
        return core_.Grant(playerKey, reward, sourceKey);

    if (!Validate(reward))
        return Supports(reward.type) ? rewards::RewardResult::Rejected
                                     : rewards::RewardResult::Unsupported;

    rsPLAYINFO* player = players_.ResolveByPlayerKey(playerKey);
    if (!player || !player->lpsmSock)
        return rewards::RewardResult::Failed;

    switch (reward.type)
    {
    case rewards::RewardType::Gold:
        return rsSendMoneyToUser(player, reward.amount)
                   ? rewards::RewardResult::Granted
                   : rewards::RewardResult::Failed;
    case rewards::RewardType::Experience:
        player->AddExp(static_cast<INT64>(reward.amount), 0, false);
        // AddExp may change PlayerLevelAtLeast conditions. Reentrant processing is
        // coalesced by ProgressionRuntime, so this never recursively walks Story.
        NotifyServerProgressionPlayerStateChanged(players_.IdOf(player));
        return rewards::RewardResult::Granted;
    case rewards::RewardType::Item:
    {
        inventory::ItemDefinition definition;
        if(!ResolveGrantableItem(reward.key, definition))
            return rewards::RewardResult::Failed;
        const PlayerId playerId(static_cast<std::uint32_t>(player->dwObjectSerial));
        return inventoryCommands_.Grant(
                   inventory::ItemGrant(playerId, definition.id, 1u)).Granted()
                   ? rewards::RewardResult::Granted
                   : rewards::RewardResult::Failed;
    }
    case rewards::RewardType::Custom:
        return customHandlers_.GrantReward(playerKey, reward, sourceKey);
    case rewards::RewardType::Reputation:
    case rewards::RewardType::StoryFlag:
    case rewards::RewardType::CovenantReputation:
        break;
    }
    return rewards::RewardResult::Unsupported;
}

void LegacyProgressionRewardGateway::SetItemCatalogReady(bool ready)
{
    itemCatalogReady_ = ready;
}

bool LegacyProgressionRewardGateway::ResolveGrantableItem(
    const std::string& contentKey,
    inventory::ItemDefinition& out) const
{
    return itemCatalog_.TryGetDefinitionByContentKey(contentKey, out) && out.defaultGrantable;
}

LegacyProgressionStoryCommandGateway::LegacyProgressionStoryCommandGateway(
    covenants::CovenantService& covenantService,
    ProgressionCustomHandlerRegistry& customHandlers,
    const player::legacy::LegacyOnlinePlayerRegistry& players,
    const quests::IQuestCatalog& questCatalog,
    quests::QuestService& questService)
    : core_(covenantService), customHandlers_(customHandlers), players_(players),
      questCatalog_(questCatalog), questService_(questService)
{
}

bool LegacyProgressionStoryCommandGateway::Supports(story::StoryActionType type) const
{
    return core_.Supports(type) || type == story::StoryActionType::StartQuest ||
           type == story::StoryActionType::Custom;
}

bool LegacyProgressionStoryCommandGateway::ValidateAction(
    const story::StoryActionDefinition& action) const
{
    if (core_.Supports(action.type))
        return core_.ValidateAction(action);

    if (action.type == story::StoryActionType::StartQuest && action.id > 0)
    {
        quests::QuestDefinition definition;
        return questCatalog_.TryGetDefinition(QuestId(static_cast<std::uint32_t>(action.id)), definition);
    }
    if (action.type == story::StoryActionType::Custom)
        return customHandlers_.ValidateAction(action);
    return false;
}

bool LegacyProgressionStoryCommandGateway::IsRetrySafe(story::StoryActionType type) const
{
    return core_.Supports(type) ? core_.IsRetrySafe(type) : false;
}

bool LegacyProgressionStoryCommandGateway::IsRetrySafe(
    const story::StoryActionDefinition& action) const
{
    if (core_.Supports(action.type))
        return core_.IsRetrySafe(action.type);
    if (action.type == story::StoryActionType::Custom)
        return customHandlers_.IsActionRetrySafe(action);
    return false;
}

bool LegacyProgressionStoryCommandGateway::StartQuest(const std::string& playerKey, int questId)
{
    if (questId <= 0)
        return false;

    quests::QuestDefinition definition;
    if (!questCatalog_.TryGetDefinition(QuestId(static_cast<std::uint32_t>(questId)), definition))
        return false;

    rsPLAYINFO* player = players_.ResolveByPlayerKey(playerKey);
    if (!player || !player->lpsmSock)
        return false;

    quests::QuestStartCommand command;
    command.quest = QuestId(static_cast<std::uint32_t>(questId));
    command.kind = definition.kind;
    command.type = definition.type;
    command.partyFlag = definition.partyFlag;
    return questService_.EnsureStarted(players_.IdOf(player), player->getName(), command);
}

bool LegacyProgressionStoryCommandGateway::AddCovenantReputation(
    const std::string& playerKey,
    int covenantId,
    int amount,
    const std::string& sourceKey)
{
    return core_.AddCovenantReputation(playerKey, covenantId, amount, sourceKey);
}

bool LegacyProgressionStoryCommandGateway::ExecuteCustomAction(
    const std::string& playerKey,
    const story::StoryActionDefinition& action,
    const std::string& sourceKey)
{
    if (action.type != story::StoryActionType::Custom)
        return false;
    return customHandlers_.ExecuteAction(playerKey, action, sourceKey);
}


}
}
