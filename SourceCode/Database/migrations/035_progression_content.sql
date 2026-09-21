-- TAKEOFF progression V2 production content catalog.
-- Intentionally data-empty: this migration creates contracts only.
-- Physical names are used because all logical ServerDB tables share summit.sqlite3.

CREATE TABLE IF NOT EXISTS ServerDB__ProgressionCovenantDefinition (
    CovenantId INTEGER NOT NULL PRIMARY KEY,
    Key TEXT NOT NULL UNIQUE CHECK(length(Key) BETWEEN 1 AND 255),
    DisplayNameKey TEXT NOT NULL DEFAULT '' CHECK(length(DisplayNameKey) <= 255),
    ExclusiveMembership INTEGER NOT NULL DEFAULT 0 CHECK(ExclusiveMembership IN (0,1)),
    HasReputationBounds INTEGER NOT NULL DEFAULT 0 CHECK(HasReputationBounds IN (0,1)),
    MinReputation INTEGER NOT NULL DEFAULT 0,
    MaxReputation INTEGER NOT NULL DEFAULT 0,
    Enabled INTEGER NOT NULL DEFAULT 1 CHECK(Enabled IN (0,1)),
    CHECK(HasReputationBounds = 0 OR MinReputation <= MaxReputation)
);

CREATE TABLE IF NOT EXISTS ServerDB__ProgressionCovenantRank (
    CovenantId INTEGER NOT NULL,
    RankId INTEGER NOT NULL,
    Key TEXT NOT NULL CHECK(length(Key) BETWEEN 1 AND 255),
    DisplayNameKey TEXT NOT NULL DEFAULT '' CHECK(length(DisplayNameKey) <= 255),
    MinReputation INTEGER NOT NULL,
    PRIMARY KEY(CovenantId, RankId),
    UNIQUE(CovenantId, Key),
    UNIQUE(CovenantId, MinReputation)
);

CREATE TABLE IF NOT EXISTS ServerDB__ProgressionCovenantJoinCondition (
    CovenantId INTEGER NOT NULL,
    SortOrder INTEGER NOT NULL CHECK(SortOrder >= 0),
    ConditionType INTEGER NOT NULL CHECK(ConditionType BETWEEN 0 AND 11 AND ConditionType <> 2),
    Key TEXT NOT NULL DEFAULT '' CHECK(length(Key) <= 255),
    Id INTEGER NOT NULL DEFAULT 0,
    Value INTEGER NOT NULL DEFAULT 0,
    AuxiliaryId INTEGER NOT NULL DEFAULT 0,
    CHECK(ConditionType = 0 OR
          (ConditionType = 1 AND Value >= 0) OR
          (ConditionType = 3 AND length(Key) BETWEEN 1 AND 255) OR
          (ConditionType = 4 AND Id > 0 AND AuxiliaryId > 0 AND Value BETWEEN 0 AND 2) OR
          (ConditionType = 5 AND Id > 0 AND Value BETWEEN 0 AND 2) OR
          (ConditionType = 6 AND Id > 0) OR
          (ConditionType = 7 AND Id > 0 AND Value > 0) OR
          (ConditionType = 8 AND Id > 0) OR
          (ConditionType = 9 AND length(Key) BETWEEN 1 AND 255 AND Value >= 0) OR
          (ConditionType = 10 AND length(Key) BETWEEN 1 AND 255) OR
          (ConditionType = 11 AND length(Key) BETWEEN 1 AND 255)),
    PRIMARY KEY(CovenantId, SortOrder)
);

CREATE TABLE IF NOT EXISTS ServerDB__ProgressionCovenantRankBenefit (
    CovenantId INTEGER NOT NULL,
    RankId INTEGER NOT NULL,
    SortOrder INTEGER NOT NULL CHECK(SortOrder >= 0),
    StatKey TEXT NOT NULL CHECK(length(StatKey) BETWEEN 1 AND 255),
    Operation INTEGER NOT NULL CHECK(Operation BETWEEN 0 AND 2),
    Value REAL NOT NULL,
    ScopeKey TEXT NOT NULL DEFAULT '' CHECK(length(ScopeKey) <= 255),
    PRIMARY KEY(CovenantId, RankId, SortOrder)
);

CREATE TABLE IF NOT EXISTS ServerDB__ProgressionCovenantRelatedQuest (
    CovenantId INTEGER NOT NULL,
    SortOrder INTEGER NOT NULL CHECK(SortOrder >= 0),
    QuestId INTEGER NOT NULL CHECK(QuestId > 0),
    PRIMARY KEY(CovenantId, SortOrder),
    UNIQUE(CovenantId, QuestId)
);

CREATE TABLE IF NOT EXISTS ServerDB__ProgressionCovenantRelatedStory (
    CovenantId INTEGER NOT NULL,
    SortOrder INTEGER NOT NULL CHECK(SortOrder >= 0),
    StoryId INTEGER NOT NULL CHECK(StoryId > 0),
    PRIMARY KEY(CovenantId, SortOrder),
    UNIQUE(CovenantId, StoryId)
);

CREATE TABLE IF NOT EXISTS ServerDB__ProgressionStoryDefinition (
    StoryId INTEGER NOT NULL PRIMARY KEY,
    Key TEXT NOT NULL UNIQUE CHECK(length(Key) BETWEEN 1 AND 255),
    DisplayNameKey TEXT NOT NULL DEFAULT '' CHECK(length(DisplayNameKey) <= 255),
    StartMode INTEGER NOT NULL DEFAULT 0 CHECK(StartMode IN (0,1)),
    InitialChapterId INTEGER NOT NULL CHECK(InitialChapterId > 0),
    EntryNodeId INTEGER NOT NULL CHECK(EntryNodeId > 0),
    Enabled INTEGER NOT NULL DEFAULT 1 CHECK(Enabled IN (0,1))
);

CREATE TABLE IF NOT EXISTS ServerDB__ProgressionStoryStartCondition (
    StoryId INTEGER NOT NULL,
    SortOrder INTEGER NOT NULL CHECK(SortOrder >= 0),
    ConditionType INTEGER NOT NULL CHECK(ConditionType BETWEEN 0 AND 11 AND ConditionType <> 2),
    Key TEXT NOT NULL DEFAULT '' CHECK(length(Key) <= 255),
    Id INTEGER NOT NULL DEFAULT 0,
    Value INTEGER NOT NULL DEFAULT 0,
    AuxiliaryId INTEGER NOT NULL DEFAULT 0,
    CHECK(ConditionType = 0 OR
          (ConditionType = 1 AND Value >= 0) OR
          (ConditionType = 3 AND length(Key) BETWEEN 1 AND 255) OR
          (ConditionType = 4 AND Id > 0 AND AuxiliaryId > 0 AND Value BETWEEN 0 AND 2) OR
          (ConditionType = 5 AND Id > 0 AND Value BETWEEN 0 AND 2) OR
          (ConditionType = 6 AND Id > 0) OR
          (ConditionType = 7 AND Id > 0 AND Value > 0) OR
          (ConditionType = 8 AND Id > 0) OR
          (ConditionType = 9 AND length(Key) BETWEEN 1 AND 255 AND Value >= 0) OR
          (ConditionType = 10 AND length(Key) BETWEEN 1 AND 255) OR
          (ConditionType = 11 AND length(Key) BETWEEN 1 AND 255)),
    PRIMARY KEY(StoryId, SortOrder)
);

CREATE TABLE IF NOT EXISTS ServerDB__ProgressionStoryArc (
    StoryId INTEGER NOT NULL,
    ArcId INTEGER NOT NULL CHECK(ArcId > 0),
    Key TEXT NOT NULL CHECK(length(Key) BETWEEN 1 AND 255),
    DisplayNameKey TEXT NOT NULL DEFAULT '' CHECK(length(DisplayNameKey) <= 255),
    PRIMARY KEY(StoryId, ArcId),
    UNIQUE(StoryId, Key)
);

CREATE TABLE IF NOT EXISTS ServerDB__ProgressionStoryChapter (
    StoryId INTEGER NOT NULL,
    ChapterId INTEGER NOT NULL CHECK(ChapterId > 0),
    ArcId INTEGER NOT NULL DEFAULT 0 CHECK(ArcId >= 0),
    Key TEXT NOT NULL CHECK(length(Key) BETWEEN 1 AND 255),
    DisplayNameKey TEXT NOT NULL DEFAULT '' CHECK(length(DisplayNameKey) <= 255),
    PRIMARY KEY(StoryId, ChapterId),
    UNIQUE(StoryId, Key)
);

CREATE TABLE IF NOT EXISTS ServerDB__ProgressionStoryChapterCondition (
    StoryId INTEGER NOT NULL,
    ChapterId INTEGER NOT NULL,
    SortOrder INTEGER NOT NULL CHECK(SortOrder >= 0),
    ConditionType INTEGER NOT NULL CHECK(ConditionType BETWEEN 0 AND 11 AND ConditionType <> 2),
    Key TEXT NOT NULL DEFAULT '' CHECK(length(Key) <= 255),
    Id INTEGER NOT NULL DEFAULT 0,
    Value INTEGER NOT NULL DEFAULT 0,
    AuxiliaryId INTEGER NOT NULL DEFAULT 0,
    CHECK(ConditionType = 0 OR
          (ConditionType = 1 AND Value >= 0) OR
          (ConditionType = 3 AND length(Key) BETWEEN 1 AND 255) OR
          (ConditionType = 4 AND Id > 0 AND AuxiliaryId > 0 AND Value BETWEEN 0 AND 2) OR
          (ConditionType = 5 AND Id > 0 AND Value BETWEEN 0 AND 2) OR
          (ConditionType = 6 AND Id > 0) OR
          (ConditionType = 7 AND Id > 0 AND Value > 0) OR
          (ConditionType = 8 AND Id > 0) OR
          (ConditionType = 9 AND length(Key) BETWEEN 1 AND 255 AND Value >= 0) OR
          (ConditionType = 10 AND length(Key) BETWEEN 1 AND 255) OR
          (ConditionType = 11 AND length(Key) BETWEEN 1 AND 255)),
    PRIMARY KEY(StoryId, ChapterId, SortOrder)
);

CREATE TABLE IF NOT EXISTS ServerDB__ProgressionStoryNode (
    StoryId INTEGER NOT NULL,
    NodeId INTEGER NOT NULL CHECK(NodeId > 0),
    ChapterId INTEGER NOT NULL CHECK(ChapterId > 0),
    Key TEXT NOT NULL CHECK(length(Key) BETWEEN 1 AND 255),
    DialogueKey TEXT NOT NULL DEFAULT '' CHECK(length(DialogueKey) <= 255),
    SpeakerKey TEXT NOT NULL DEFAULT '' CHECK(length(SpeakerKey) <= 255),
    QuestId INTEGER NOT NULL DEFAULT 0 CHECK(QuestId >= 0),
    CovenantId INTEGER NOT NULL DEFAULT 0 CHECK(CovenantId >= 0),
    PRIMARY KEY(StoryId, NodeId),
    UNIQUE(StoryId, Key)
);

CREATE TABLE IF NOT EXISTS ServerDB__ProgressionStoryNodeEntryCondition (
    StoryId INTEGER NOT NULL,
    NodeId INTEGER NOT NULL,
    SortOrder INTEGER NOT NULL CHECK(SortOrder >= 0),
    ConditionType INTEGER NOT NULL CHECK(ConditionType BETWEEN 0 AND 11 AND ConditionType <> 2),
    Key TEXT NOT NULL DEFAULT '' CHECK(length(Key) <= 255),
    Id INTEGER NOT NULL DEFAULT 0,
    Value INTEGER NOT NULL DEFAULT 0,
    AuxiliaryId INTEGER NOT NULL DEFAULT 0,
    CHECK(ConditionType = 0 OR
          (ConditionType = 1 AND Value >= 0) OR
          (ConditionType = 3 AND length(Key) BETWEEN 1 AND 255) OR
          (ConditionType = 4 AND Id > 0 AND AuxiliaryId > 0 AND Value BETWEEN 0 AND 2) OR
          (ConditionType = 5 AND Id > 0 AND Value BETWEEN 0 AND 2) OR
          (ConditionType = 6 AND Id > 0) OR
          (ConditionType = 7 AND Id > 0 AND Value > 0) OR
          (ConditionType = 8 AND Id > 0) OR
          (ConditionType = 9 AND length(Key) BETWEEN 1 AND 255 AND Value >= 0) OR
          (ConditionType = 10 AND length(Key) BETWEEN 1 AND 255) OR
          (ConditionType = 11 AND length(Key) BETWEEN 1 AND 255)),
    PRIMARY KEY(StoryId, NodeId, SortOrder)
);

CREATE TABLE IF NOT EXISTS ServerDB__ProgressionStoryNodeCompletionCondition (
    StoryId INTEGER NOT NULL,
    NodeId INTEGER NOT NULL,
    SortOrder INTEGER NOT NULL CHECK(SortOrder >= 0),
    ConditionType INTEGER NOT NULL CHECK(ConditionType BETWEEN 0 AND 11 AND ConditionType <> 2),
    Key TEXT NOT NULL DEFAULT '' CHECK(length(Key) <= 255),
    Id INTEGER NOT NULL DEFAULT 0,
    Value INTEGER NOT NULL DEFAULT 0,
    AuxiliaryId INTEGER NOT NULL DEFAULT 0,
    CHECK(ConditionType = 0 OR
          (ConditionType = 1 AND Value >= 0) OR
          (ConditionType = 3 AND length(Key) BETWEEN 1 AND 255) OR
          (ConditionType = 4 AND Id > 0 AND AuxiliaryId > 0 AND Value BETWEEN 0 AND 2) OR
          (ConditionType = 5 AND Id > 0 AND Value BETWEEN 0 AND 2) OR
          (ConditionType = 6 AND Id > 0) OR
          (ConditionType = 7 AND Id > 0 AND Value > 0) OR
          (ConditionType = 8 AND Id > 0) OR
          (ConditionType = 9 AND length(Key) BETWEEN 1 AND 255 AND Value >= 0) OR
          (ConditionType = 10 AND length(Key) BETWEEN 1 AND 255) OR
          (ConditionType = 11 AND length(Key) BETWEEN 1 AND 255)),
    PRIMARY KEY(StoryId, NodeId, SortOrder)
);

CREATE TABLE IF NOT EXISTS ServerDB__ProgressionStoryNodeEnterAction (
    StoryId INTEGER NOT NULL,
    NodeId INTEGER NOT NULL,
    SortOrder INTEGER NOT NULL CHECK(SortOrder >= 0),
    ActionType INTEGER NOT NULL CHECK(ActionType BETWEEN 0 AND 5),
    Key TEXT NOT NULL DEFAULT '' CHECK(length(Key) <= 255),
    Id INTEGER NOT NULL DEFAULT 0,
    Value INTEGER NOT NULL DEFAULT 0,
    PRIMARY KEY(StoryId, NodeId, SortOrder),
    CHECK((ActionType IN (0,1) AND length(Key) BETWEEN 1 AND 255) OR
          (ActionType = 2 AND Id > 0) OR
          (ActionType = 3 AND Id > 0 AND Value <> 0) OR
          ActionType = 4 OR
          (ActionType = 5 AND length(Key) BETWEEN 1 AND 255))
);

CREATE TABLE IF NOT EXISTS ServerDB__ProgressionStoryNodeCompletionAction (
    StoryId INTEGER NOT NULL,
    NodeId INTEGER NOT NULL,
    SortOrder INTEGER NOT NULL CHECK(SortOrder >= 0),
    ActionType INTEGER NOT NULL CHECK(ActionType BETWEEN 0 AND 5),
    Key TEXT NOT NULL DEFAULT '' CHECK(length(Key) <= 255),
    Id INTEGER NOT NULL DEFAULT 0,
    Value INTEGER NOT NULL DEFAULT 0,
    PRIMARY KEY(StoryId, NodeId, SortOrder),
    CHECK((ActionType IN (0,1) AND length(Key) BETWEEN 1 AND 255) OR
          (ActionType = 2 AND Id > 0) OR
          (ActionType = 3 AND Id > 0 AND Value <> 0) OR
          ActionType = 4 OR
          (ActionType = 5 AND length(Key) BETWEEN 1 AND 255))
);

CREATE TABLE IF NOT EXISTS ServerDB__ProgressionStoryTransition (
    StoryId INTEGER NOT NULL,
    FromNodeId INTEGER NOT NULL CHECK(FromNodeId > 0),
    SortOrder INTEGER NOT NULL CHECK(SortOrder >= 0),
    ToNodeId INTEGER NOT NULL CHECK(ToNodeId > 0),
    PRIMARY KEY(StoryId, FromNodeId, SortOrder),
    UNIQUE(StoryId, FromNodeId, ToNodeId)
);

CREATE TABLE IF NOT EXISTS ServerDB__ProgressionStoryNodeReward (
    StoryId INTEGER NOT NULL,
    NodeId INTEGER NOT NULL,
    SortOrder INTEGER NOT NULL CHECK(SortOrder >= 0),
    RewardType INTEGER NOT NULL CHECK(RewardType BETWEEN 0 AND 6 AND RewardType <> 3),
    Key TEXT NOT NULL DEFAULT '' CHECK(length(Key) <= 255),
    Id INTEGER NOT NULL DEFAULT 0,
    Amount INTEGER NOT NULL DEFAULT 0,
    PRIMARY KEY(StoryId, NodeId, SortOrder),
    CHECK((RewardType = 0 AND length(Key) BETWEEN 1 AND 255 AND Amount = 1) OR
          (RewardType IN (1,2) AND Amount > 0) OR
          (RewardType = 4 AND length(Key) BETWEEN 1 AND 255) OR
          (RewardType = 5 AND Id > 0 AND Amount <> 0) OR
          (RewardType = 6 AND length(Key) BETWEEN 1 AND 255))
);

CREATE TABLE IF NOT EXISTS ServerDB__ProgressionStoryDialogueOption (
    StoryId INTEGER NOT NULL,
    NodeId INTEGER NOT NULL,
    SortOrder INTEGER NOT NULL CHECK(SortOrder >= 0),
    OptionKey TEXT NOT NULL CHECK(length(OptionKey) BETWEEN 1 AND 255),
    PRIMARY KEY(StoryId, NodeId, SortOrder),
    UNIQUE(StoryId, NodeId, OptionKey)
);

CREATE INDEX IF NOT EXISTS ServerDB__ProgressionCovenantRank_ByCovenant
    ON ServerDB__ProgressionCovenantRank(CovenantId, MinReputation);
CREATE INDEX IF NOT EXISTS ServerDB__ProgressionStoryChapter_ByStory
    ON ServerDB__ProgressionStoryChapter(StoryId, ChapterId);
CREATE INDEX IF NOT EXISTS ServerDB__ProgressionStoryNode_ByStory
    ON ServerDB__ProgressionStoryNode(StoryId, NodeId);
CREATE INDEX IF NOT EXISTS ServerDB__ProgressionStoryTransition_ByNode
    ON ServerDB__ProgressionStoryTransition(StoryId, FromNodeId, SortOrder);
