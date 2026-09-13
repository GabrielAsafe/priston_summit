-- Summit SQLite migration 003: QuestDB contracts proven by active source/seed.
-- Only tables whose complete positional layout is directly supported by source evidence are created here.
-- Progress, Rewards and ItemRewards remain intentionally absent until their full legacy layouts are proven.

CREATE TABLE IF NOT EXISTS "QuestDB__RepeatedQuest"
(
    "QuestID" INTEGER NOT NULL,
    "QuestName" TEXT NOT NULL,
    "QuestTitle" TEXT NOT NULL,
    "LevelMin" INTEGER NOT NULL,
    "LevelMax" INTEGER NOT NULL,
    "PartyFlag" INTEGER NOT NULL,
    "QuestDesc" TEXT NOT NULL,
    "QuestType" INTEGER NOT NULL
);

CREATE TABLE IF NOT EXISTS "QuestDB__DailyQuest"
(
    "QuestID" INTEGER NOT NULL,
    "QuestName" TEXT NOT NULL,
    "QuestTitle" TEXT NOT NULL,
    "LevelMin" INTEGER NOT NULL,
    "LevelMax" INTEGER NOT NULL,
    "PartyFlag" INTEGER NOT NULL,
    "QuestDesc" TEXT NOT NULL,
    "QuestType" INTEGER NOT NULL
);

CREATE TABLE IF NOT EXISTS "QuestDB__SingleQuest"
(
    "QuestID" INTEGER NOT NULL,
    "QuestName" TEXT NOT NULL,
    "QuestTitle" TEXT NOT NULL,
    "LevelMin" INTEGER NOT NULL,
    "LevelMax" INTEGER NOT NULL,
    "PartyFlag" INTEGER NOT NULL,
    "QuestDesc" TEXT NOT NULL,
    "QuestType" INTEGER NOT NULL
);

-- Received QuestDB_QuestMonsters_seed.sql names all six columns explicitly.
-- cQuestHandler positional reads confirm MonsterName=3, Quantity=4 and QuestMap=6.
CREATE TABLE IF NOT EXISTS "QuestDB__QuestMonsters"
(
    "QuestID" INTEGER NOT NULL,
    "QuestKind" INTEGER NOT NULL,
    "MonsterName" TEXT NOT NULL,
    "Quantity" INTEGER NOT NULL,
    "QuestType" INTEGER NOT NULL,
    "QuestMap" INTEGER NOT NULL
);

-- INSERT INTO MonsterKilled VALUES (...) plus positional GetData() proves this nine-column order.
CREATE TABLE IF NOT EXISTS "QuestDB__MonsterKilled"
(
    "MonsterName" TEXT NOT NULL,
    "Killed" INTEGER NOT NULL,
    "Count" INTEGER NOT NULL,
    "QuestID" INTEGER NOT NULL,
    "QuestKind" INTEGER NOT NULL,
    "QuestType" INTEGER NOT NULL,
    "PlayerName" TEXT NOT NULL,
    "PartyFlag" INTEGER NOT NULL,
    "QuestMap" INTEGER NOT NULL
);

-- The legacy caller implemented IF NOT EXISTS(key) INSERT ELSE UPDATE EndDate.
-- The UNIQUE key is therefore required for the WIP INSERT OR REPLACE to preserve that contract.
CREATE TABLE IF NOT EXISTS "QuestDB__Complete"
(
    "QuestID" INTEGER NOT NULL,
    "QuestKind" INTEGER NOT NULL,
    "QuestType" INTEGER NOT NULL,
    "PlayerName" TEXT NOT NULL,
    "EndDate" TEXT NOT NULL,
    UNIQUE ("QuestID", "QuestKind", "QuestType", "PlayerName")
);

-- Idempotent SQLite form of the received QuestDB_QuestMonsters_seed.sql.
INSERT INTO "QuestDB__QuestMonsters"
    ("QuestID", "QuestKind", "MonsterName", "Quantity", "QuestType", "QuestMap")
SELECT
    16, 7, 'Hopi', 5, 16, 2
WHERE NOT EXISTS
(
    SELECT 1
    FROM "QuestDB__QuestMonsters"
    WHERE "QuestID" = 16
      AND "QuestKind" = 7
      AND "MonsterName" = 'Hopi'
      AND "QuestType" = 16
      AND "QuestMap" = 2
);
