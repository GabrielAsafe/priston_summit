#include "systems/gameplay/progression/v2/SQLiteProgressionContentLoader.h"

#include <map>
#include <memory>
#include <sstream>
#include <utility>

#include "systems/persistence/database/core/Database.h"

namespace gameplay
{
namespace progression
{
namespace
{
typedef std::pair<int, int> PairKey;

void SetError(std::string* error, const std::string& value)
{
    if (error)
        *error = value;
}

bool OpenServerDatabase(std::shared_ptr<SQLConnection>& db, std::string* error)
{
    db = SQLConnection::GetConnection(DATABASEID_ServerDB);
    if (!db || !db->Open())
    {
        SetError(error, "failed to open ServerDB for progression content");
        return false;
    }
    return true;
}

bool BeginQuery(SQLConnection& db, const char* query, std::string* error)
{
    if (db.Prepare(query) && db.Execute(false))
        return true;
    std::string message = "progression content query failed";
    const char* detail = db.GetLastError();
    if (detail && detail[0])
    {
        message += ": ";
        message += detail;
    }
    SetError(error, message);
    return false;
}

bool CursorOk(SQLConnection& db, std::string* error)
{
    const char* detail = db.GetLastError();
    if (!detail || detail[0] == 0)
        return true;
    SetError(error, std::string("progression content cursor failed: ") + detail);
    return false;
}

bool ReadInt(SQLConnection& db, int column, int& value, std::string* error)
{
    if (db.GetData(&value, sizeof(value), column, PARAMTYPE_Integer))
        return true;
    SetError(error, "failed to read integer progression content column");
    return false;
}

bool ReadFloat(SQLConnection& db, int column, float& value, std::string* error)
{
    if (db.GetData(&value, sizeof(value), column, PARAMTYPE_Float))
        return true;
    SetError(error, "failed to read numeric progression content column");
    return false;
}

bool ReadString(SQLConnection& db, int column, std::string& value, std::string* error)
{
    char buffer[256] = {0};
    if (!db.GetData(buffer, sizeof(buffer), column, PARAMTYPE_String))
    {
        SetError(error, "failed to read string progression content column");
        return false;
    }
    value = buffer;
    return true;
}

bool FindCovenant(const std::map<int, std::size_t>& index,
                  int covenantId,
                  std::size_t& position,
                  std::string* error)
{
    std::map<int, std::size_t>::const_iterator it = index.find(covenantId);
    if (it == index.end())
    {
        SetError(error, "enabled covenant content references an unknown covenant");
        return false;
    }
    position = it->second;
    return true;
}

bool FindStory(const std::map<int, std::size_t>& index,
               int storyId,
               std::size_t& position,
               std::string* error)
{
    std::map<int, std::size_t>::const_iterator it = index.find(storyId);
    if (it == index.end())
    {
        SetError(error, "enabled story content references an unknown story");
        return false;
    }
    position = it->second;
    return true;
}

bool ReadCondition(SQLConnection& db,
                   int firstColumn,
                   conditions::ConditionDefinition& condition,
                   std::string* error)
{
    int rawType = 0;
    if (!ReadInt(db, firstColumn, rawType, error) ||
        !ReadString(db, firstColumn + 1, condition.key, error) ||
        !ReadInt(db, firstColumn + 2, condition.id, error) ||
        !ReadInt(db, firstColumn + 3, condition.value, error) ||
        !ReadInt(db, firstColumn + 4, condition.auxiliaryId, error))
        return false;
    if (rawType < static_cast<int>(conditions::ConditionType::Always) ||
        rawType > static_cast<int>(conditions::ConditionType::ReputationAtLeast))
    {
        SetError(error, "invalid condition type in progression content");
        return false;
    }
    condition.type = static_cast<conditions::ConditionType>(rawType);
    return true;
}

bool ReadAction(SQLConnection& db,
                int firstColumn,
                story::StoryActionDefinition& action,
                std::string* error)
{
    int rawType = 0;
    if (!ReadInt(db, firstColumn, rawType, error) ||
        !ReadString(db, firstColumn + 1, action.key, error) ||
        !ReadInt(db, firstColumn + 2, action.id, error) ||
        !ReadInt(db, firstColumn + 3, action.value, error))
        return false;
    if (rawType < static_cast<int>(story::StoryActionType::SetPlayerFlag) ||
        rawType > static_cast<int>(story::StoryActionType::Custom))
    {
        SetError(error, "invalid story action type in progression content");
        return false;
    }
    action.type = static_cast<story::StoryActionType>(rawType);
    return true;
}

bool ReadReward(SQLConnection& db,
                int firstColumn,
                rewards::RewardDefinition& reward,
                std::string* error)
{
    int rawType = 0;
    if (!ReadInt(db, firstColumn, rawType, error) ||
        !ReadString(db, firstColumn + 1, reward.key, error) ||
        !ReadInt(db, firstColumn + 2, reward.id, error) ||
        !ReadInt(db, firstColumn + 3, reward.amount, error))
        return false;
    if (rawType < static_cast<int>(rewards::RewardType::Item) ||
        rawType > static_cast<int>(rewards::RewardType::Custom))
    {
        SetError(error, "invalid reward type in progression content");
        return false;
    }
    reward.type = static_cast<rewards::RewardType>(rawType);
    return true;
}
}

bool SQLiteProgressionContentLoader::Load(ProgressionContentCatalog& catalog,
                                          std::string* error) const
{
    catalog = ProgressionContentCatalog();
    std::shared_ptr<SQLConnection> db;
    if (!OpenServerDatabase(db, error))
        return false;

    std::map<int, std::size_t> covenantIndex;
    std::map<PairKey, std::size_t> rankIndex;
    std::map<int, std::size_t> storyIndex;
    std::map<PairKey, std::size_t> chapterIndex;
    std::map<PairKey, std::size_t> nodeIndex;

    bool ok = BeginQuery(*db,
        "SELECT ReputationKey,DisplayNameKey,HasBounds,MinValue,MaxValue "
        "FROM ProgressionReputationDefinition WHERE Enabled=1 ORDER BY ReputationKey", error);
    while (ok && db->NextRow())
    {
        reputation::ReputationDefinition definition;
        int bounds = 0;
        if (!ReadString(*db, 1, definition.key, error) ||
            !ReadString(*db, 2, definition.displayNameKey, error) ||
            !ReadInt(*db, 3, bounds, error) ||
            !ReadInt(*db, 4, definition.minValue, error) ||
            !ReadInt(*db, 5, definition.maxValue, error))
        {
            ok = false;
            break;
        }
        definition.hasBounds = bounds != 0;
        catalog.reputations.push_back(definition);
    }
    if (ok) ok = CursorOk(*db, error);

    if (ok) ok = BeginQuery(*db,
        "SELECT CovenantId,Key,DisplayNameKey,ExclusiveMembership,HasReputationBounds,"
        "MinReputation,MaxReputation FROM ProgressionCovenantDefinition "
        "WHERE Enabled=1 ORDER BY CovenantId", error);
    while (ok && db->NextRow())
    {
        CovenantContentDefinition content;
        int exclusive = 0;
        int bounds = 0;
        if (!ReadInt(*db, 1, content.definition.id, error) ||
            !ReadString(*db, 2, content.definition.key, error) ||
            !ReadString(*db, 3, content.definition.displayNameKey, error) ||
            !ReadInt(*db, 4, exclusive, error) ||
            !ReadInt(*db, 5, bounds, error) ||
            !ReadInt(*db, 6, content.definition.minReputation, error) ||
            !ReadInt(*db, 7, content.definition.maxReputation, error))
        {
            ok = false;
            break;
        }
        content.definition.exclusiveMembership = exclusive != 0;
        content.definition.hasReputationBounds = bounds != 0;
        covenantIndex[content.definition.id] = catalog.covenants.size();
        catalog.covenants.push_back(content);
    }
    if (ok) ok = CursorOk(*db, error);

    if (ok) ok = BeginQuery(*db,
        "SELECT r.CovenantId,r.RankId,r.Key,r.DisplayNameKey,r.MinReputation "
        "FROM ProgressionCovenantRank r JOIN ProgressionCovenantDefinition c "
        "ON c.CovenantId=r.CovenantId WHERE c.Enabled=1 "
        "ORDER BY r.CovenantId,r.MinReputation,r.RankId", error);
    while (ok && db->NextRow())
    {
        covenants::CovenantRankDefinition rank;
        if (!ReadInt(*db, 1, rank.covenantId, error) ||
            !ReadInt(*db, 2, rank.id, error) ||
            !ReadString(*db, 3, rank.key, error) ||
            !ReadString(*db, 4, rank.displayNameKey, error) ||
            !ReadInt(*db, 5, rank.minReputation, error))
        {
            ok = false;
            break;
        }
        std::size_t covenantPosition = 0;
        if (!FindCovenant(covenantIndex, rank.covenantId, covenantPosition, error))
        {
            ok = false;
            break;
        }
        rankIndex[PairKey(rank.covenantId, rank.id)] = catalog.covenants[covenantPosition].ranks.size();
        catalog.covenants[covenantPosition].ranks.push_back(rank);
    }
    if (ok) ok = CursorOk(*db, error);

    if (ok) ok = BeginQuery(*db,
        "SELECT r.CovenantId,r.RankId,r.RewardKey,r.RewardType,r.Key,r.Id,r.Amount "
        "FROM ProgressionCovenantReward r JOIN ProgressionCovenantDefinition c "
        "ON c.CovenantId=r.CovenantId WHERE c.Enabled=1 "
        "ORDER BY r.CovenantId,r.RankId,r.SortOrder", error);
    while (ok && db->NextRow())
    {
        int covenantId = 0;
        int rankId = 0;
        std::string rewardKey;
        rewards::RewardDefinition reward;
        std::size_t covenantPosition = 0;
        if (!ReadInt(*db, 1, covenantId, error) ||
            !ReadInt(*db, 2, rankId, error) ||
            !ReadString(*db, 3, rewardKey, error) ||
            !ReadReward(*db, 4, reward, error) ||
            !FindCovenant(covenantIndex, covenantId, covenantPosition, error))
        {
            ok = false;
            break;
        }

        covenants::CovenantRewardDefinition definition;
        definition.key = rewardKey;
        definition.reward = reward;
        if (rankId == 0)
        {
            catalog.covenants[covenantPosition].definition.joinRewards.push_back(definition);
            continue;
        }

        std::map<PairKey, std::size_t>::const_iterator rankIt =
            rankIndex.find(PairKey(covenantId, rankId));
        if (rankIt == rankIndex.end())
        {
            SetError(error, "covenant reward references an unknown rank");
            ok = false;
            break;
        }
        catalog.covenants[covenantPosition].ranks[rankIt->second].entryRewards.push_back(definition);
    }
    if (ok) ok = CursorOk(*db, error);

    if (ok) ok = BeginQuery(*db,
        "SELECT j.CovenantId,j.ConditionType,j.Key,j.Id,j.Value,j.AuxiliaryId "
        "FROM ProgressionCovenantJoinCondition j JOIN ProgressionCovenantDefinition c "
        "ON c.CovenantId=j.CovenantId WHERE c.Enabled=1 ORDER BY j.CovenantId,j.SortOrder", error);
    while (ok && db->NextRow())
    {
        int covenantId = 0;
        conditions::ConditionDefinition condition;
        std::size_t covenantPosition = 0;
        if (!ReadInt(*db, 1, covenantId, error) ||
            !ReadCondition(*db, 2, condition, error) ||
            !FindCovenant(covenantIndex, covenantId, covenantPosition, error))
        {
            ok = false;
            break;
        }
        catalog.covenants[covenantPosition].definition.joinConditions.push_back(condition);
    }
    if (ok) ok = CursorOk(*db, error);

    if (ok) ok = BeginQuery(*db,
        "SELECT b.CovenantId,b.RankId,b.StatKey,b.Operation,b.Value,b.ScopeKey "
        "FROM ProgressionCovenantRankBenefit b JOIN ProgressionCovenantDefinition c "
        "ON c.CovenantId=b.CovenantId WHERE c.Enabled=1 "
        "ORDER BY b.CovenantId,b.RankId,b.SortOrder", error);
    while (ok && db->NextRow())
    {
        int covenantId = 0;
        int rankId = 0;
        int operation = 0;
        float rawValue = 0.0f;
        modifiers::GameplayModifier modifier;
        if (!ReadInt(*db, 1, covenantId, error) ||
            !ReadInt(*db, 2, rankId, error) ||
            !ReadString(*db, 3, modifier.statKey, error) ||
            !ReadInt(*db, 4, operation, error) ||
            !ReadFloat(*db, 5, rawValue, error) ||
            !ReadString(*db, 6, modifier.scopeKey, error))
        {
            ok = false;
            break;
        }
        std::size_t covenantPosition = 0;
        if (!FindCovenant(covenantIndex, covenantId, covenantPosition, error))
        {
            ok = false;
            break;
        }
        std::map<PairKey, std::size_t>::const_iterator rankIt = rankIndex.find(PairKey(covenantId, rankId));
        if (rankIt == rankIndex.end() || operation < 0 || operation > 2)
        {
            SetError(error, "covenant benefit references an unknown rank or operation");
            ok = false;
            break;
        }
        modifier.operation = static_cast<modifiers::ModifierOperation>(operation);
        modifier.value = static_cast<double>(rawValue);
        catalog.covenants[covenantPosition].ranks[rankIt->second].benefits.push_back(modifier);
    }
    if (ok) ok = CursorOk(*db, error);

    const char* covenantRelationQueries[] =
    {
        "SELECT q.CovenantId,q.QuestId FROM ProgressionCovenantRelatedQuest q "
        "JOIN ProgressionCovenantDefinition c ON c.CovenantId=q.CovenantId "
        "WHERE c.Enabled=1 ORDER BY q.CovenantId,q.SortOrder",
        "SELECT s.CovenantId,s.StoryId FROM ProgressionCovenantRelatedStory s "
        "JOIN ProgressionCovenantDefinition c ON c.CovenantId=s.CovenantId "
        "WHERE c.Enabled=1 ORDER BY s.CovenantId,s.SortOrder"
    };
    for (int relationKind = 0; ok && relationKind < 2; ++relationKind)
    {
        ok = BeginQuery(*db, covenantRelationQueries[relationKind], error);
        while (ok && db->NextRow())
        {
            int covenantId = 0;
            int relatedId = 0;
            std::size_t covenantPosition = 0;
            if (!ReadInt(*db, 1, covenantId, error) ||
                !ReadInt(*db, 2, relatedId, error) ||
                !FindCovenant(covenantIndex, covenantId, covenantPosition, error))
            {
                ok = false;
                break;
            }
            if (relationKind == 0)
                catalog.covenants[covenantPosition].definition.relatedQuestIds.push_back(relatedId);
            else
                catalog.covenants[covenantPosition].definition.relatedStoryIds.push_back(relatedId);
        }
        if (ok) ok = CursorOk(*db, error);
    }

    if (ok) ok = BeginQuery(*db,
        "SELECT StoryId,Key,DisplayNameKey,StartMode,InitialChapterId,EntryNodeId "
        "FROM ProgressionStoryDefinition WHERE Enabled=1 ORDER BY StoryId", error);
    while (ok && db->NextRow())
    {
        StoryContentDefinition content;
        int startMode = 0;
        if (!ReadInt(*db, 1, content.definition.id, error) ||
            !ReadString(*db, 2, content.definition.key, error) ||
            !ReadString(*db, 3, content.definition.displayNameKey, error) ||
            !ReadInt(*db, 4, startMode, error) ||
            !ReadInt(*db, 5, content.definition.initialChapterId, error) ||
            !ReadInt(*db, 6, content.definition.entryNodeId, error))
        {
            ok = false;
            break;
        }
        if (startMode < static_cast<int>(story::StoryStartMode::Automatic) ||
            startMode > static_cast<int>(story::StoryStartMode::Explicit))
        {
            SetError(error, "invalid story start mode in progression content");
            ok = false;
            break;
        }
        content.definition.startMode = static_cast<story::StoryStartMode>(startMode);
        storyIndex[content.definition.id] = catalog.stories.size();
        catalog.stories.push_back(content);
    }
    if (ok) ok = CursorOk(*db, error);

    if (ok) ok = BeginQuery(*db,
        "SELECT c.StoryId,c.ConditionType,c.Key,c.Id,c.Value,c.AuxiliaryId "
        "FROM ProgressionStoryStartCondition c JOIN ProgressionStoryDefinition s "
        "ON s.StoryId=c.StoryId WHERE s.Enabled=1 ORDER BY c.StoryId,c.SortOrder", error);
    while (ok && db->NextRow())
    {
        int storyId = 0;
        conditions::ConditionDefinition condition;
        std::size_t storyPosition = 0;
        if (!ReadInt(*db, 1, storyId, error) ||
            !ReadCondition(*db, 2, condition, error) ||
            !FindStory(storyIndex, storyId, storyPosition, error))
        {
            ok = false;
            break;
        }
        catalog.stories[storyPosition].definition.startConditions.push_back(condition);
    }
    if (ok) ok = CursorOk(*db, error);

    if (ok) ok = BeginQuery(*db,
        "SELECT a.StoryId,a.ArcId,a.Key,a.DisplayNameKey FROM ProgressionStoryArc a "
        "JOIN ProgressionStoryDefinition s ON s.StoryId=a.StoryId WHERE s.Enabled=1 "
        "ORDER BY a.StoryId,a.ArcId", error);
    while (ok && db->NextRow())
    {
        story::StoryArcDefinition arc;
        std::size_t storyPosition = 0;
        if (!ReadInt(*db, 1, arc.storyId, error) ||
            !ReadInt(*db, 2, arc.id, error) ||
            !ReadString(*db, 3, arc.key, error) ||
            !ReadString(*db, 4, arc.displayNameKey, error) ||
            !FindStory(storyIndex, arc.storyId, storyPosition, error))
        {
            ok = false;
            break;
        }
        catalog.stories[storyPosition].arcs.push_back(arc);
    }
    if (ok) ok = CursorOk(*db, error);

    if (ok) ok = BeginQuery(*db,
        "SELECT c.StoryId,c.ChapterId,c.ArcId,c.Key,c.DisplayNameKey "
        "FROM ProgressionStoryChapter c JOIN ProgressionStoryDefinition s "
        "ON s.StoryId=c.StoryId WHERE s.Enabled=1 ORDER BY c.StoryId,c.ChapterId", error);
    while (ok && db->NextRow())
    {
        story::StoryChapterDefinition chapter;
        std::size_t storyPosition = 0;
        if (!ReadInt(*db, 1, chapter.storyId, error) ||
            !ReadInt(*db, 2, chapter.id, error) ||
            !ReadInt(*db, 3, chapter.arcId, error) ||
            !ReadString(*db, 4, chapter.key, error) ||
            !ReadString(*db, 5, chapter.displayNameKey, error) ||
            !FindStory(storyIndex, chapter.storyId, storyPosition, error))
        {
            ok = false;
            break;
        }
        chapterIndex[PairKey(chapter.storyId, chapter.id)] = catalog.stories[storyPosition].chapters.size();
        catalog.stories[storyPosition].chapters.push_back(chapter);
    }
    if (ok) ok = CursorOk(*db, error);

    if (ok) ok = BeginQuery(*db,
        "SELECT c.StoryId,c.ChapterId,c.ConditionType,c.Key,c.Id,c.Value,c.AuxiliaryId "
        "FROM ProgressionStoryChapterCondition c JOIN ProgressionStoryDefinition s "
        "ON s.StoryId=c.StoryId WHERE s.Enabled=1 "
        "ORDER BY c.StoryId,c.ChapterId,c.SortOrder", error);
    while (ok && db->NextRow())
    {
        int storyId = 0;
        int chapterId = 0;
        conditions::ConditionDefinition condition;
        std::size_t storyPosition = 0;
        if (!ReadInt(*db, 1, storyId, error) || !ReadInt(*db, 2, chapterId, error) ||
            !ReadCondition(*db, 3, condition, error) ||
            !FindStory(storyIndex, storyId, storyPosition, error))
        {
            ok = false;
            break;
        }
        std::map<PairKey, std::size_t>::const_iterator chapterIt = chapterIndex.find(PairKey(storyId, chapterId));
        if (chapterIt == chapterIndex.end())
        {
            SetError(error, "story chapter condition references an unknown chapter");
            ok = false;
            break;
        }
        catalog.stories[storyPosition].chapters[chapterIt->second].entryConditions.push_back(condition);
    }
    if (ok) ok = CursorOk(*db, error);

    if (ok) ok = BeginQuery(*db,
        "SELECT n.StoryId,n.NodeId,n.ChapterId,n.Key,n.DialogueKey,n.SpeakerKey,n.QuestId,n.CovenantId "
        "FROM ProgressionStoryNode n JOIN ProgressionStoryDefinition s ON s.StoryId=n.StoryId "
        "WHERE s.Enabled=1 ORDER BY n.StoryId,n.NodeId", error);
    while (ok && db->NextRow())
    {
        int storyId = 0;
        story::StoryNodeDefinition node;
        std::size_t storyPosition = 0;
        if (!ReadInt(*db, 1, storyId, error) || !ReadInt(*db, 2, node.id, error) ||
            !ReadInt(*db, 3, node.chapterId, error) || !ReadString(*db, 4, node.key, error) ||
            !ReadString(*db, 5, node.dialogueKey, error) || !ReadString(*db, 6, node.speakerKey, error) ||
            !ReadInt(*db, 7, node.questId, error) || !ReadInt(*db, 8, node.covenantId, error) ||
            !FindStory(storyIndex, storyId, storyPosition, error))
        {
            ok = false;
            break;
        }
        nodeIndex[PairKey(storyId, node.id)] = catalog.stories[storyPosition].nodes.size();
        catalog.stories[storyPosition].nodes.push_back(node);
    }
    if (ok) ok = CursorOk(*db, error);

    const char* nodeConditionQueries[] =
    {
        "SELECT c.StoryId,c.NodeId,c.ConditionType,c.Key,c.Id,c.Value,c.AuxiliaryId "
        "FROM ProgressionStoryNodeEntryCondition c JOIN ProgressionStoryDefinition s "
        "ON s.StoryId=c.StoryId WHERE s.Enabled=1 ORDER BY c.StoryId,c.NodeId,c.SortOrder",
        "SELECT c.StoryId,c.NodeId,c.ConditionType,c.Key,c.Id,c.Value,c.AuxiliaryId "
        "FROM ProgressionStoryNodeCompletionCondition c JOIN ProgressionStoryDefinition s "
        "ON s.StoryId=c.StoryId WHERE s.Enabled=1 ORDER BY c.StoryId,c.NodeId,c.SortOrder"
    };
    for (int conditionKind = 0; ok && conditionKind < 2; ++conditionKind)
    {
        ok = BeginQuery(*db, nodeConditionQueries[conditionKind], error);
        while (ok && db->NextRow())
        {
            int storyId = 0;
            int nodeId = 0;
            conditions::ConditionDefinition condition;
            std::size_t storyPosition = 0;
            if (!ReadInt(*db, 1, storyId, error) || !ReadInt(*db, 2, nodeId, error) ||
                !ReadCondition(*db, 3, condition, error) ||
                !FindStory(storyIndex, storyId, storyPosition, error))
            {
                ok = false;
                break;
            }
            std::map<PairKey, std::size_t>::const_iterator nodeIt = nodeIndex.find(PairKey(storyId, nodeId));
            if (nodeIt == nodeIndex.end())
            {
                SetError(error, "story node condition references an unknown node");
                ok = false;
                break;
            }
            if (conditionKind == 0)
                catalog.stories[storyPosition].nodes[nodeIt->second].entryConditions.push_back(condition);
            else
                catalog.stories[storyPosition].nodes[nodeIt->second].completionConditions.push_back(condition);
        }
        if (ok) ok = CursorOk(*db, error);
    }

    const char* actionQueries[] =
    {
        "SELECT a.StoryId,a.NodeId,a.ActionType,a.Key,a.Id,a.Value "
        "FROM ProgressionStoryNodeEnterAction a JOIN ProgressionStoryDefinition s "
        "ON s.StoryId=a.StoryId WHERE s.Enabled=1 ORDER BY a.StoryId,a.NodeId,a.SortOrder",
        "SELECT a.StoryId,a.NodeId,a.ActionType,a.Key,a.Id,a.Value "
        "FROM ProgressionStoryNodeCompletionAction a JOIN ProgressionStoryDefinition s "
        "ON s.StoryId=a.StoryId WHERE s.Enabled=1 ORDER BY a.StoryId,a.NodeId,a.SortOrder"
    };
    for (int actionKind = 0; ok && actionKind < 2; ++actionKind)
    {
        ok = BeginQuery(*db, actionQueries[actionKind], error);
        while (ok && db->NextRow())
        {
            int storyId = 0;
            int nodeId = 0;
            story::StoryActionDefinition action;
            std::size_t storyPosition = 0;
            if (!ReadInt(*db, 1, storyId, error) || !ReadInt(*db, 2, nodeId, error) ||
                !ReadAction(*db, 3, action, error) ||
                !FindStory(storyIndex, storyId, storyPosition, error))
            {
                ok = false;
                break;
            }
            std::map<PairKey, std::size_t>::const_iterator nodeIt = nodeIndex.find(PairKey(storyId, nodeId));
            if (nodeIt == nodeIndex.end())
            {
                SetError(error, "story action references an unknown node");
                ok = false;
                break;
            }
            if (actionKind == 0)
                catalog.stories[storyPosition].nodes[nodeIt->second].onEnterActions.push_back(action);
            else
                catalog.stories[storyPosition].nodes[nodeIt->second].onCompleteActions.push_back(action);
        }
        if (ok) ok = CursorOk(*db, error);
    }

    if (ok) ok = BeginQuery(*db,
        "SELECT t.StoryId,t.FromNodeId,t.ToNodeId FROM ProgressionStoryTransition t "
        "JOIN ProgressionStoryDefinition s ON s.StoryId=t.StoryId WHERE s.Enabled=1 "
        "ORDER BY t.StoryId,t.FromNodeId,t.SortOrder", error);
    while (ok && db->NextRow())
    {
        int storyId = 0;
        int nodeId = 0;
        int targetId = 0;
        std::size_t storyPosition = 0;
        if (!ReadInt(*db, 1, storyId, error) || !ReadInt(*db, 2, nodeId, error) ||
            !ReadInt(*db, 3, targetId, error) || !FindStory(storyIndex, storyId, storyPosition, error))
        {
            ok = false;
            break;
        }
        std::map<PairKey, std::size_t>::const_iterator nodeIt = nodeIndex.find(PairKey(storyId, nodeId));
        if (nodeIt == nodeIndex.end())
        {
            SetError(error, "story transition references an unknown source node");
            ok = false;
            break;
        }
        catalog.stories[storyPosition].nodes[nodeIt->second].transitions.push_back(targetId);
    }
    if (ok) ok = CursorOk(*db, error);

    if (ok) ok = BeginQuery(*db,
        "SELECT r.StoryId,r.NodeId,r.RewardType,r.Key,r.Id,r.Amount "
        "FROM ProgressionStoryNodeReward r JOIN ProgressionStoryDefinition s "
        "ON s.StoryId=r.StoryId WHERE s.Enabled=1 ORDER BY r.StoryId,r.NodeId,r.SortOrder", error);
    while (ok && db->NextRow())
    {
        int storyId = 0;
        int nodeId = 0;
        rewards::RewardDefinition reward;
        std::size_t storyPosition = 0;
        if (!ReadInt(*db, 1, storyId, error) || !ReadInt(*db, 2, nodeId, error) ||
            !ReadReward(*db, 3, reward, error) || !FindStory(storyIndex, storyId, storyPosition, error))
        {
            ok = false;
            break;
        }
        std::map<PairKey, std::size_t>::const_iterator nodeIt = nodeIndex.find(PairKey(storyId, nodeId));
        if (nodeIt == nodeIndex.end())
        {
            SetError(error, "story reward references an unknown node");
            ok = false;
            break;
        }
        catalog.stories[storyPosition].nodes[nodeIt->second].completionRewards.push_back(reward);
    }
    if (ok) ok = CursorOk(*db, error);

    if (ok) ok = BeginQuery(*db,
        "SELECT o.StoryId,o.NodeId,o.OptionKey FROM ProgressionStoryDialogueOption o "
        "JOIN ProgressionStoryDefinition s ON s.StoryId=o.StoryId WHERE s.Enabled=1 "
        "ORDER BY o.StoryId,o.NodeId,o.SortOrder", error);
    while (ok && db->NextRow())
    {
        int storyId = 0;
        int nodeId = 0;
        std::string optionKey;
        std::size_t storyPosition = 0;
        if (!ReadInt(*db, 1, storyId, error) || !ReadInt(*db, 2, nodeId, error) ||
            !ReadString(*db, 3, optionKey, error) || !FindStory(storyIndex, storyId, storyPosition, error))
        {
            ok = false;
            break;
        }
        std::map<PairKey, std::size_t>::const_iterator nodeIt = nodeIndex.find(PairKey(storyId, nodeId));
        if (nodeIt == nodeIndex.end())
        {
            SetError(error, "story dialogue option references an unknown node");
            ok = false;
            break;
        }
        catalog.stories[storyPosition].nodes[nodeIt->second].dialogueOptionKeys.push_back(optionKey);
    }
    if (ok) ok = CursorOk(*db, error);

    if (ok) ok = BeginQuery(*db,
        "SELECT c.StoryId,c.NodeId,c.ChoiceKey,c.DisplayKey,c.ToNodeId FROM ProgressionStoryDialogueChoice c "
        "JOIN ProgressionStoryDefinition s ON s.StoryId=c.StoryId WHERE s.Enabled=1 "
        "ORDER BY c.StoryId,c.NodeId,c.SortOrder", error);
    while (ok && db->NextRow())
    {
        int storyId = 0;
        int nodeId = 0;
        story::StoryDialogueChoiceDefinition choice;
        std::size_t storyPosition = 0;
        if (!ReadInt(*db, 1, storyId, error) || !ReadInt(*db, 2, nodeId, error) ||
            !ReadString(*db, 3, choice.key, error) ||
            !ReadString(*db, 4, choice.displayKey, error) ||
            !ReadInt(*db, 5, choice.targetNodeId, error) ||
            !FindStory(storyIndex, storyId, storyPosition, error))
        {
            ok = false;
            break;
        }
        std::map<PairKey, std::size_t>::const_iterator nodeIt = nodeIndex.find(PairKey(storyId, nodeId));
        if (nodeIt == nodeIndex.end() ||
            nodeIndex.find(PairKey(storyId, choice.targetNodeId)) == nodeIndex.end())
        {
            SetError(error, "story dialogue choice references an unknown node");
            ok = false;
            break;
        }
        catalog.stories[storyPosition].nodes[nodeIt->second].dialogueChoices.push_back(choice);
    }
    if (ok) ok = CursorOk(*db, error);

    if (ok) ok = BeginQuery(*db,
        "SELECT c.StoryId,c.NodeId,c.ChoiceKey,c.ConditionType,c.Key,c.Id,c.Value,c.AuxiliaryId "
        "FROM ProgressionStoryDialogueChoiceCondition c JOIN ProgressionStoryDefinition s "
        "ON s.StoryId=c.StoryId WHERE s.Enabled=1 "
        "ORDER BY c.StoryId,c.NodeId,c.ChoiceKey,c.SortOrder", error);
    while (ok && db->NextRow())
    {
        int storyId = 0;
        int nodeId = 0;
        std::string choiceKey;
        conditions::ConditionDefinition condition;
        std::size_t storyPosition = 0;
        if (!ReadInt(*db, 1, storyId, error) || !ReadInt(*db, 2, nodeId, error) ||
            !ReadString(*db, 3, choiceKey, error) ||
            !ReadCondition(*db, 4, condition, error) ||
            !FindStory(storyIndex, storyId, storyPosition, error))
        {
            ok = false;
            break;
        }
        std::map<PairKey, std::size_t>::const_iterator nodeIt = nodeIndex.find(PairKey(storyId, nodeId));
        if (nodeIt == nodeIndex.end())
        {
            SetError(error, "story dialogue choice condition references an unknown node");
            ok = false;
            break;
        }
        std::vector<story::StoryDialogueChoiceDefinition>& choices =
            catalog.stories[storyPosition].nodes[nodeIt->second].dialogueChoices;
        bool found = false;
        for (std::vector<story::StoryDialogueChoiceDefinition>::iterator choice = choices.begin();
             choice != choices.end(); ++choice)
        {
            if (choice->key == choiceKey)
            {
                choice->conditions.push_back(condition);
                found = true;
                break;
            }
        }
        if (!found)
        {
            SetError(error, "story dialogue choice condition references an unknown choice");
            ok = false;
            break;
        }
    }
    if (ok) ok = CursorOk(*db, error);

    db->Close();
    if (!ok)
    {
        catalog = ProgressionContentCatalog();
        return false;
    }
    return true;
}
}
}
