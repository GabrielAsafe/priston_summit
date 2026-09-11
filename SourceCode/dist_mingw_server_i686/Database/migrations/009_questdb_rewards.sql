-- Summit SQLite migration 009: QuestDB Rewards/ItemRewards.
-- Source of truth for physical layout: supplied QuestDB202405120913.bak catalog.
-- These layouts also match every active SELECT * + positional GetData contract.

CREATE TABLE IF NOT EXISTS "QuestDB__Rewards"
(
    "QuestID" INTEGER NOT NULL,
    "QuestKind" INTEGER NOT NULL,
    "Gold" INTEGER NOT NULL,
    "Exp" INTEGER NOT NULL,
    "QuestType" INTEGER NOT NULL,
    "Coin" INTEGER NOT NULL,
    "TCoin" INTEGER NOT NULL
);
CREATE INDEX IF NOT EXISTS "QuestDB__Rewards_Quest"
    ON "QuestDB__Rewards" ("QuestID", "QuestKind", "QuestType");

-- The SQL Server backup contains historical rows where QuestType is NULL even
-- though the catalog metadata is inconsistent about nullability. Do not add a
-- NOT NULL constraint here; WHERE QuestType=? naturally excludes such rows.
CREATE TABLE IF NOT EXISTS "QuestDB__ItemRewards"
(
    "QuestID" INTEGER NOT NULL,
    "QuestKind" INTEGER NOT NULL,
    "ItemCode" TEXT NOT NULL,
    "ItemCount" INTEGER NOT NULL,
    "QuestType" INTEGER NULL
);
CREATE INDEX IF NOT EXISTS "QuestDB__ItemRewards_Quest"
    ON "QuestDB__ItemRewards" ("QuestID", "QuestKind", "QuestType");
