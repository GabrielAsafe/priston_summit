#include "systems/gameplay/progression/v2/LegacyProgressionGateways.h"
#include "systems/gameplay/progression/v2/LegacyQuestContract.h"
#include "systems/gameplay/progression/v2/ProgressionTriggerRuntime.h"

#include <cctype>
#include <cstring>
#include <memory>

#include "core/server/legacy_runtime/onserver.h"
#include "persistence/database/Database_sqlite_server.h"
#include "systems/gameplay/quests/server/cQuestHandler.h"
#include "systems/inventory/legacy/sinItem.h"

int rsSendMoneyToUser(rsPLAYINFO* lpPlayInfo, int Money);

namespace gameplay
{
namespace progression
{
namespace
{
bool EqualsIgnoreCase(const char* left, const std::string& right)
{
    if (!left)
        return false;

    const std::size_t length = std::strlen(left);
    if (length != right.size())
        return false;

    for (std::size_t i = 0; i < length; ++i)
    {
        const unsigned char a = static_cast<unsigned char>(left[i]);
        const unsigned char b = static_cast<unsigned char>(right[i]);
        if (std::tolower(a) != std::tolower(b))
            return false;
    }
    return true;
}

rsPLAYINFO* FindOnlinePlayer(const std::string& playerKey)
{
    if (playerKey.empty())
        return 0;

    for (int i = 0; i < CONNECTMAX; ++i)
    {
        rsPLAYINFO* player = &rsPlayInfo[i];
        if (player->lpsmSock && EqualsIgnoreCase(player->getName(), playerKey))
            return player;
    }
    return 0;
}

bool ReadQuestRows(const char* tableName,
                   int questKind,
                   int questId,
                   int& matchCount,
                   LegacyProgressionStoryCommandGateway::QuestDefinition& match)
{
    std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_QuestDB);
    if (!db->Open())
        return false;

    std::string query = "SELECT PartyFlag,QuestType FROM ";
    query += tableName;
    query += " WHERE QuestID=?";

    if (!db->Prepare(query.c_str()))
    {
        db->Close();
        return false;
    }

    int id = questId;
    db->BindInputParameter(&id, 1, PARAMTYPE_Integer);
    if (!db->Execute(false))
    {
        db->Close();
        return false;
    }

    while (db->NextRow())
    {
        int partyFlag = 0;
        int questType = 0;
        db->GetData(&partyFlag, 4, 1, PARAMTYPE_Integer);
        db->GetData(&questType, 4, 2, PARAMTYPE_Integer);
        ++matchCount;
        if (matchCount == 1)
        {
            match.kind = questKind;
            match.type = questType;
            match.partyFlag = partyFlag;
        }
    }

    db->Close();
    return true;
}
}

LegacyProgressionRewardGateway::LegacyProgressionRewardGateway(
    story::IStoryRepository& storyRepository,
    covenants::CovenantService& covenantService,
    reputation::ReputationService& reputationService,
    ProgressionCustomHandlerRegistry& customHandlers)
    : core_(storyRepository, covenantService, reputationService), customHandlers_(customHandlers), itemCatalogReady_(false)
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
        if (reward.key.empty() || reward.amount != 1)
            return false;
        return !itemCatalogReady_ || ResolveItemCode(reward.key) != 0;
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

    rsPLAYINFO* player = FindOnlinePlayer(playerKey);
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
        NotifyServerProgressionPlayerStateChanged(player);
        return rewards::RewardResult::Granted;
    case rewards::RewardType::Item:
    {
        const unsigned long itemCode = ResolveItemCode(reward.key);
        return itemCode != 0 && rsPutItemChecked(player, static_cast<DWORD>(itemCode))
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

unsigned long LegacyProgressionRewardGateway::ResolveItemCode(const std::string& shortCode) const
{
    if (shortCode.empty() || DefaultItemCount <= 0)
        return 0;

    unsigned long code = 0;
    for (int i = 0; i < MAX_ITEM; ++i)
    {
        if (sItem[i].CODE && EqualsIgnoreCase(sItem[i].LastCategory, shortCode))
        {
            code = static_cast<unsigned long>(sItem[i].CODE);
            break;
        }
    }
    if (code == 0)
        return 0;

    for (int i = 0; i < DefaultItemCount; ++i)
    {
        if (static_cast<unsigned long>(DefaultItems[i].Item.CODE) == code &&
            !DefaultItems[i].Item.UniqueItem)
            return code;
    }
    return 0;
}

LegacyProgressionStoryCommandGateway::LegacyProgressionStoryCommandGateway(
    covenants::CovenantService& covenantService,
    ProgressionCustomHandlerRegistry& customHandlers)
    : core_(covenantService), customHandlers_(customHandlers)
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
        QuestDefinition definition;
        return ResolveQuestDefinition(action.id, definition);
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

    QuestDefinition definition;
    if (!ResolveQuestDefinition(questId, definition))
        return false;

    rsPLAYINFO* player = FindOnlinePlayer(playerKey);
    if (!player || !player->lpsmSock)
        return false;

    bool completed = false;
    if (!cQuestHandler.QueryCompletedQuest(player->getName(), questId, &completed))
        return false;
    if (completed)
        return false;
    if (cQuestHandler.IsQuestInProgress(player, questId))
        return true;

    cQuestHandler.StartQuest(player, questId, definition.kind, definition.type,
                             definition.partyFlag);
    return cQuestHandler.IsQuestInProgress(player, questId);
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

bool LegacyProgressionStoryCommandGateway::ResolveQuestDefinition(
    int questId,
    QuestDefinition& definition) const
{
    if (questId <= 0)
        return false;

    int matches = 0;
    QuestDefinition match;
    if (!ReadQuestRows("RepeatedQuest", legacy_quest::kRepeatedQuestKind, questId, matches, match) ||
        !ReadQuestRows("DailyQuest", legacy_quest::kDailyQuestKind, questId, matches, match) ||
        !ReadQuestRows("SingleQuest", legacy_quest::kSingleQuestKind, questId, matches, match))
        return false;

    if (matches != 1)
        return false;
    definition = match;
    return true;
}
}
}
