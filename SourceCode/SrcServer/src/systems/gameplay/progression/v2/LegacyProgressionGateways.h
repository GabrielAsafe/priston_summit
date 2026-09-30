#pragma once

#include <string>

#include "gameplay/progression/v2/ProgressionGateways.h"
#include "gameplay/progression/v2/ProgressionCustomHandlers.h"
#include "gameplay/inventory/v2/Inventory.h"
#include "gameplay/inventory/v2/ItemCatalog.h"
#include "gameplay/quests/v2/QuestService.h"
#include "gameplay/quests/v2/QuestCatalog.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"

namespace gameplay
{
namespace progression
{
class LegacyProgressionRewardGateway : public rewards::IRewardGateway
{
public:
    LegacyProgressionRewardGateway(story::IStoryRepository& storyRepository,
                                   covenants::CovenantService& covenantService,
                                   reputation::ReputationService& reputationService,
                                   ProgressionCustomHandlerRegistry& customHandlers,
                                   const inventory::IItemCatalog& itemCatalog,
                                   inventory::IInventoryCommands& inventoryCommands,
                                   const player::legacy::LegacyOnlinePlayerRegistry& players);

    bool Supports(rewards::RewardType type) const;
    bool Validate(const rewards::RewardDefinition& reward) const;
    bool IsRetrySafe(rewards::RewardType type) const;
    bool IsRetrySafe(const rewards::RewardDefinition& reward) const;
    rewards::RewardResult Grant(const std::string& playerKey,
                                const rewards::RewardDefinition& reward,
                                const std::string& sourceKey);

    void SetItemCatalogReady(bool ready);

private:
    bool ResolveGrantableItem(const std::string& contentKey, inventory::ItemDefinition& out) const;

    ProgressionRewardGateway core_;
    ProgressionCustomHandlerRegistry& customHandlers_;
    const inventory::IItemCatalog& itemCatalog_;
    inventory::IInventoryCommands& inventoryCommands_;
    const player::legacy::LegacyOnlinePlayerRegistry& players_;
    bool itemCatalogReady_;
};

class LegacyProgressionStoryCommandGateway : public story::IStoryCommandGateway
{
public:
    LegacyProgressionStoryCommandGateway(covenants::CovenantService& covenantService,
                                         ProgressionCustomHandlerRegistry& customHandlers,
                                         const player::legacy::LegacyOnlinePlayerRegistry& players,
                                         const quests::IQuestCatalog& questCatalog,
                                         quests::QuestService& questService);

    bool Supports(story::StoryActionType type) const;
    bool ValidateAction(const story::StoryActionDefinition& action) const;
    bool IsRetrySafe(story::StoryActionType type) const;
    bool IsRetrySafe(const story::StoryActionDefinition& action) const;
    bool StartQuest(const std::string& playerKey, int questId);
    bool AddCovenantReputation(const std::string& playerKey,
                               int covenantId,
                               int amount,
                               const std::string& sourceKey);
    bool ExecuteCustomAction(const std::string& playerKey,
                             const story::StoryActionDefinition& action,
                             const std::string& sourceKey);

private:
    ProgressionStoryCommandGateway core_;
    ProgressionCustomHandlerRegistry& customHandlers_;
    const player::legacy::LegacyOnlinePlayerRegistry& players_;
    const quests::IQuestCatalog& questCatalog_;
    quests::QuestService& questService_;
};
}
}
