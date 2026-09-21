-- TAKEOFF Story server-authoritative dialogue choices.
-- Choices are explicit branch contracts; existing dialogue options remain presentation metadata only.

CREATE TABLE IF NOT EXISTS ServerDB__ProgressionStoryDialogueChoice (
    StoryId INTEGER NOT NULL CHECK(StoryId > 0),
    NodeId INTEGER NOT NULL CHECK(NodeId > 0),
    SortOrder INTEGER NOT NULL CHECK(SortOrder >= 0),
    ChoiceKey TEXT NOT NULL CHECK(length(ChoiceKey) BETWEEN 1 AND 255),
    DisplayKey TEXT NOT NULL CHECK(length(DisplayKey) BETWEEN 1 AND 255),
    ToNodeId INTEGER NOT NULL CHECK(ToNodeId > 0),
    PRIMARY KEY(StoryId, NodeId, ChoiceKey),
    UNIQUE(StoryId, NodeId, SortOrder)
);

CREATE TABLE IF NOT EXISTS ServerDB__ProgressionStoryDialogueChoiceCondition (
    StoryId INTEGER NOT NULL CHECK(StoryId > 0),
    NodeId INTEGER NOT NULL CHECK(NodeId > 0),
    ChoiceKey TEXT NOT NULL CHECK(length(ChoiceKey) BETWEEN 1 AND 255),
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
    PRIMARY KEY(StoryId, NodeId, ChoiceKey, SortOrder)
);

CREATE INDEX IF NOT EXISTS ServerDB__ProgressionStoryDialogueChoice_ByNode
    ON ServerDB__ProgressionStoryDialogueChoice(StoryId, NodeId, SortOrder);
