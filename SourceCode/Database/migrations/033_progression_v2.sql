-- TAKEOFF P9: canonical persistent storage for Covenant and StoryState V2.
--
-- Runtime DML is issued through DATABASEID_ServerDB. SQLiteQueryCompat rewrites
-- bare logical table names to the ServerDB__ physical namespace, so the schema
-- must use the same physical names. P2/P3 originally created unprefixed fallback
-- tables, which could never satisfy rewritten SELECT/INSERT/UPDATE statements.

CREATE TABLE IF NOT EXISTS "ServerDB__CovenantMembership"
(
    "PlayerKey" TEXT NOT NULL,
    "CovenantId" INTEGER NOT NULL,
    "Reputation" INTEGER NOT NULL,
    "RankId" INTEGER NOT NULL,
    "Active" INTEGER NOT NULL CHECK ("Active" IN (0, 1)),
    "UpdatedAt" TEXT NOT NULL DEFAULT (datetime('now','localtime')),
    PRIMARY KEY ("PlayerKey", "CovenantId")
);

CREATE TABLE IF NOT EXISTS "ServerDB__CovenantReputationLedger"
(
    "PlayerKey" TEXT NOT NULL,
    "CovenantId" INTEGER NOT NULL,
    "SourceKey" TEXT NOT NULL,
    "Delta" INTEGER NOT NULL,
    "ReputationAfter" INTEGER NOT NULL,
    "RankIdAfter" INTEGER NOT NULL,
    "AppliedAt" TEXT NOT NULL DEFAULT (datetime('now','localtime')),
    PRIMARY KEY ("PlayerKey", "CovenantId", "SourceKey")
);

CREATE INDEX IF NOT EXISTS "ServerDB__CovenantMembership_Active"
    ON "ServerDB__CovenantMembership" ("PlayerKey", "Active");

CREATE TABLE IF NOT EXISTS "ServerDB__StoryProgress"
(
    "PlayerKey" TEXT NOT NULL,
    "StoryId" INTEGER NOT NULL,
    "ChapterId" INTEGER NOT NULL,
    "NodeId" INTEGER NOT NULL,
    "State" INTEGER NOT NULL,
    "EnterApplied" INTEGER NOT NULL CHECK ("EnterApplied" IN (0, 1)),
    "CompletionApplied" INTEGER NOT NULL CHECK ("CompletionApplied" IN (0, 1)),
    "UpdatedAt" TEXT NOT NULL DEFAULT (datetime('now','localtime')),
    PRIMARY KEY ("PlayerKey", "StoryId")
);

CREATE TABLE IF NOT EXISTS "ServerDB__StoryPlayerFlag"
(
    "PlayerKey" TEXT NOT NULL,
    "FlagKey" TEXT NOT NULL,
    "IntValue" INTEGER NOT NULL,
    PRIMARY KEY ("PlayerKey", "FlagKey")
);

CREATE TABLE IF NOT EXISTS "ServerDB__StoryWorldFlag"
(
    "FlagKey" TEXT NOT NULL PRIMARY KEY,
    "IntValue" INTEGER NOT NULL
);

CREATE TABLE IF NOT EXISTS "ServerDB__StoryEffectReceipt"
(
    "PlayerKey" TEXT NOT NULL,
    "StoryId" INTEGER NOT NULL,
    "NodeId" INTEGER NOT NULL,
    "EffectKey" TEXT NOT NULL,
    "Applied" INTEGER NOT NULL DEFAULT 0 CHECK ("Applied" IN (0, 1)),
    "UpdatedAt" TEXT NOT NULL DEFAULT (datetime('now','localtime')),
    PRIMARY KEY ("PlayerKey", "StoryId", "NodeId", "EffectKey")
);

CREATE INDEX IF NOT EXISTS "ServerDB__StoryEffectReceipt_Pending"
    ON "ServerDB__StoryEffectReceipt" ("Applied", "UpdatedAt");
