-- TAKEOFF Generic Reputation V2.
-- Adds an explicit server-local reputation namespace rather than aliasing Covenant/Clan.
-- Existing progression content tables are rebuilt transactionally by the migration runner
-- so RewardType=3 and ConditionType=12 become valid without rewriting migrations 035-037.

CREATE TABLE IF NOT EXISTS ServerDB__ProgressionReputationDefinition (
    ReputationKey TEXT NOT NULL PRIMARY KEY CHECK(length(ReputationKey) BETWEEN 1 AND 255),
    DisplayNameKey TEXT NOT NULL DEFAULT '' CHECK(length(DisplayNameKey) <= 255),
    HasBounds INTEGER NOT NULL DEFAULT 0 CHECK(HasBounds IN (0,1)),
    MinValue INTEGER NOT NULL DEFAULT 0,
    MaxValue INTEGER NOT NULL DEFAULT 0,
    Enabled INTEGER NOT NULL DEFAULT 1 CHECK(Enabled IN (0,1)),
    CHECK(HasBounds = 0 OR (MinValue <= 0 AND MaxValue >= 0 AND MinValue <= MaxValue))
);

CREATE TABLE IF NOT EXISTS ServerDB__GenericReputation (
    PlayerKey TEXT NOT NULL CHECK(length(PlayerKey) BETWEEN 1 AND 255),
    ReputationKey TEXT NOT NULL CHECK(length(ReputationKey) BETWEEN 1 AND 255),
    Value INTEGER NOT NULL,
    UpdatedAt TEXT NOT NULL DEFAULT (datetime('now','localtime')),
    PRIMARY KEY(PlayerKey, ReputationKey)
);

CREATE TABLE IF NOT EXISTS ServerDB__GenericReputationLedger (
    PlayerKey TEXT NOT NULL CHECK(length(PlayerKey) BETWEEN 1 AND 255),
    ReputationKey TEXT NOT NULL CHECK(length(ReputationKey) BETWEEN 1 AND 255),
    SourceKey TEXT NOT NULL CHECK(length(SourceKey) BETWEEN 1 AND 255),
    Delta INTEGER NOT NULL CHECK(Delta <> 0),
    ValueAfter INTEGER NOT NULL,
    AppliedAt TEXT NOT NULL DEFAULT (datetime('now','localtime')),
    PRIMARY KEY(PlayerKey, ReputationKey, SourceKey)
);

-- Rebuild Condition-bearing content tables with ReputationAtLeast=12 enabled.
CREATE TABLE IF NOT EXISTS ServerDB__ProgressionCovenantJoinCondition__v38 (
    CovenantId INTEGER NOT NULL,
    SortOrder INTEGER NOT NULL CHECK(SortOrder >= 0),
    ConditionType INTEGER NOT NULL CHECK(ConditionType BETWEEN 0 AND 12 AND ConditionType <> 2),
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
          (ConditionType = 11 AND length(Key) BETWEEN 1 AND 255) OR
          (ConditionType = 12 AND length(Key) BETWEEN 1 AND 255)),
    PRIMARY KEY(CovenantId, SortOrder)
);
DELETE FROM ServerDB__ProgressionCovenantJoinCondition__v38;
INSERT INTO ServerDB__ProgressionCovenantJoinCondition__v38
    SELECT * FROM ServerDB__ProgressionCovenantJoinCondition;
DROP TABLE ServerDB__ProgressionCovenantJoinCondition;
ALTER TABLE ServerDB__ProgressionCovenantJoinCondition__v38 RENAME TO ServerDB__ProgressionCovenantJoinCondition;

CREATE TABLE IF NOT EXISTS ServerDB__ProgressionStoryStartCondition__v38 (
    StoryId INTEGER NOT NULL,
    SortOrder INTEGER NOT NULL CHECK(SortOrder >= 0),
    ConditionType INTEGER NOT NULL CHECK(ConditionType BETWEEN 0 AND 12 AND ConditionType <> 2),
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
          (ConditionType = 11 AND length(Key) BETWEEN 1 AND 255) OR
          (ConditionType = 12 AND length(Key) BETWEEN 1 AND 255)),
    PRIMARY KEY(StoryId, SortOrder)
);
DELETE FROM ServerDB__ProgressionStoryStartCondition__v38;
INSERT INTO ServerDB__ProgressionStoryStartCondition__v38
    SELECT * FROM ServerDB__ProgressionStoryStartCondition;
DROP TABLE ServerDB__ProgressionStoryStartCondition;
ALTER TABLE ServerDB__ProgressionStoryStartCondition__v38 RENAME TO ServerDB__ProgressionStoryStartCondition;

CREATE TABLE IF NOT EXISTS ServerDB__ProgressionStoryChapterCondition__v38 (
    StoryId INTEGER NOT NULL,
    ChapterId INTEGER NOT NULL,
    SortOrder INTEGER NOT NULL CHECK(SortOrder >= 0),
    ConditionType INTEGER NOT NULL CHECK(ConditionType BETWEEN 0 AND 12 AND ConditionType <> 2),
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
          (ConditionType = 11 AND length(Key) BETWEEN 1 AND 255) OR
          (ConditionType = 12 AND length(Key) BETWEEN 1 AND 255)),
    PRIMARY KEY(StoryId, ChapterId, SortOrder)
);
DELETE FROM ServerDB__ProgressionStoryChapterCondition__v38;
INSERT INTO ServerDB__ProgressionStoryChapterCondition__v38
    SELECT * FROM ServerDB__ProgressionStoryChapterCondition;
DROP TABLE ServerDB__ProgressionStoryChapterCondition;
ALTER TABLE ServerDB__ProgressionStoryChapterCondition__v38 RENAME TO ServerDB__ProgressionStoryChapterCondition;

CREATE TABLE IF NOT EXISTS ServerDB__ProgressionStoryNodeEntryCondition__v38 (
    StoryId INTEGER NOT NULL,
    NodeId INTEGER NOT NULL,
    SortOrder INTEGER NOT NULL CHECK(SortOrder >= 0),
    ConditionType INTEGER NOT NULL CHECK(ConditionType BETWEEN 0 AND 12 AND ConditionType <> 2),
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
          (ConditionType = 11 AND length(Key) BETWEEN 1 AND 255) OR
          (ConditionType = 12 AND length(Key) BETWEEN 1 AND 255)),
    PRIMARY KEY(StoryId, NodeId, SortOrder)
);
DELETE FROM ServerDB__ProgressionStoryNodeEntryCondition__v38;
INSERT INTO ServerDB__ProgressionStoryNodeEntryCondition__v38
    SELECT * FROM ServerDB__ProgressionStoryNodeEntryCondition;
DROP TABLE ServerDB__ProgressionStoryNodeEntryCondition;
ALTER TABLE ServerDB__ProgressionStoryNodeEntryCondition__v38 RENAME TO ServerDB__ProgressionStoryNodeEntryCondition;

CREATE TABLE IF NOT EXISTS ServerDB__ProgressionStoryNodeCompletionCondition__v38 (
    StoryId INTEGER NOT NULL,
    NodeId INTEGER NOT NULL,
    SortOrder INTEGER NOT NULL CHECK(SortOrder >= 0),
    ConditionType INTEGER NOT NULL CHECK(ConditionType BETWEEN 0 AND 12 AND ConditionType <> 2),
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
          (ConditionType = 11 AND length(Key) BETWEEN 1 AND 255) OR
          (ConditionType = 12 AND length(Key) BETWEEN 1 AND 255)),
    PRIMARY KEY(StoryId, NodeId, SortOrder)
);
DELETE FROM ServerDB__ProgressionStoryNodeCompletionCondition__v38;
INSERT INTO ServerDB__ProgressionStoryNodeCompletionCondition__v38
    SELECT * FROM ServerDB__ProgressionStoryNodeCompletionCondition;
DROP TABLE ServerDB__ProgressionStoryNodeCompletionCondition;
ALTER TABLE ServerDB__ProgressionStoryNodeCompletionCondition__v38 RENAME TO ServerDB__ProgressionStoryNodeCompletionCondition;

CREATE TABLE IF NOT EXISTS ServerDB__ProgressionStoryDialogueChoiceCondition__v38 (
    StoryId INTEGER NOT NULL CHECK(StoryId > 0),
    NodeId INTEGER NOT NULL CHECK(NodeId > 0),
    ChoiceKey TEXT NOT NULL CHECK(length(ChoiceKey) BETWEEN 1 AND 255),
    SortOrder INTEGER NOT NULL CHECK(SortOrder >= 0),
    ConditionType INTEGER NOT NULL CHECK(ConditionType BETWEEN 0 AND 12 AND ConditionType <> 2),
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
          (ConditionType = 11 AND length(Key) BETWEEN 1 AND 255) OR
          (ConditionType = 12 AND length(Key) BETWEEN 1 AND 255)),
    PRIMARY KEY(StoryId, NodeId, ChoiceKey, SortOrder)
);
DELETE FROM ServerDB__ProgressionStoryDialogueChoiceCondition__v38;
INSERT INTO ServerDB__ProgressionStoryDialogueChoiceCondition__v38
    SELECT * FROM ServerDB__ProgressionStoryDialogueChoiceCondition;
DROP TABLE ServerDB__ProgressionStoryDialogueChoiceCondition;
ALTER TABLE ServerDB__ProgressionStoryDialogueChoiceCondition__v38 RENAME TO ServerDB__ProgressionStoryDialogueChoiceCondition;

-- Rebuild Reward-bearing content tables with generic Reputation=3 enabled.
CREATE TABLE IF NOT EXISTS ServerDB__ProgressionStoryNodeReward__v38 (
    StoryId INTEGER NOT NULL,
    NodeId INTEGER NOT NULL,
    SortOrder INTEGER NOT NULL CHECK(SortOrder >= 0),
    RewardType INTEGER NOT NULL CHECK(RewardType BETWEEN 0 AND 6),
    Key TEXT NOT NULL DEFAULT '' CHECK(length(Key) <= 255),
    Id INTEGER NOT NULL DEFAULT 0,
    Amount INTEGER NOT NULL DEFAULT 0,
    PRIMARY KEY(StoryId, NodeId, SortOrder),
    CHECK((RewardType = 0 AND length(Key) BETWEEN 1 AND 255 AND Amount = 1) OR
          (RewardType IN (1,2) AND Amount > 0) OR
          (RewardType = 3 AND length(Key) BETWEEN 1 AND 255 AND Id = 0 AND Amount <> 0) OR
          (RewardType = 4 AND length(Key) BETWEEN 1 AND 255) OR
          (RewardType = 5 AND Id > 0 AND Amount <> 0) OR
          (RewardType = 6 AND length(Key) BETWEEN 1 AND 255))
);
DELETE FROM ServerDB__ProgressionStoryNodeReward__v38;
INSERT INTO ServerDB__ProgressionStoryNodeReward__v38
    SELECT * FROM ServerDB__ProgressionStoryNodeReward;
DROP TABLE ServerDB__ProgressionStoryNodeReward;
ALTER TABLE ServerDB__ProgressionStoryNodeReward__v38 RENAME TO ServerDB__ProgressionStoryNodeReward;

CREATE TABLE IF NOT EXISTS ServerDB__ProgressionCovenantReward__v38 (
    CovenantId INTEGER NOT NULL CHECK(CovenantId > 0),
    RewardKey TEXT NOT NULL CHECK(length(RewardKey) BETWEEN 1 AND 200),
    RankId INTEGER NOT NULL DEFAULT 0 CHECK(RankId >= 0),
    SortOrder INTEGER NOT NULL CHECK(SortOrder >= 0),
    RewardType INTEGER NOT NULL CHECK(RewardType BETWEEN 0 AND 6),
    Key TEXT NOT NULL DEFAULT '' CHECK(length(Key) <= 255),
    Id INTEGER NOT NULL DEFAULT 0,
    Amount INTEGER NOT NULL DEFAULT 0,
    PRIMARY KEY(CovenantId, RewardKey),
    UNIQUE(CovenantId, RankId, SortOrder),
    CHECK((RewardType = 0 AND length(Key) BETWEEN 1 AND 255 AND Amount = 1) OR
          (RewardType IN (1,2) AND Amount > 0) OR
          (RewardType = 3 AND length(Key) BETWEEN 1 AND 255 AND Id = 0 AND Amount <> 0) OR
          (RewardType = 4 AND length(Key) BETWEEN 1 AND 255) OR
          (RewardType = 5 AND Id > 0 AND Amount <> 0) OR
          (RewardType = 6 AND length(Key) BETWEEN 1 AND 255))
);
DELETE FROM ServerDB__ProgressionCovenantReward__v38;
INSERT INTO ServerDB__ProgressionCovenantReward__v38
    SELECT * FROM ServerDB__ProgressionCovenantReward;
DROP TABLE ServerDB__ProgressionCovenantReward;
ALTER TABLE ServerDB__ProgressionCovenantReward__v38 RENAME TO ServerDB__ProgressionCovenantReward;
CREATE INDEX IF NOT EXISTS ServerDB__ProgressionCovenantReward_ByMilestone
    ON ServerDB__ProgressionCovenantReward(CovenantId, RankId, SortOrder);
