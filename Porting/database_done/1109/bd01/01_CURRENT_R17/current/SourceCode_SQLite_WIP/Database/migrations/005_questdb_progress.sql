-- Summit SQLite migration 005: QuestDB Progress positional contract.
-- Active INSERT INTO Progress statements provide exactly five values, while
-- SELECT * callers read QuestID/QuestKind/QuestType from columns 1/2/3.
-- Therefore this five-column order is part of the legacy persistence ABI.

CREATE TABLE IF NOT EXISTS "QuestDB__Progress"
(
    "QuestID" INTEGER NOT NULL,
    "QuestKind" INTEGER NOT NULL,
    "QuestType" INTEGER NOT NULL,
    "PlayerName" TEXT NOT NULL,
    "StartDate" TEXT NOT NULL
);

CREATE INDEX IF NOT EXISTS "QuestDB__Progress_PlayerName"
    ON "QuestDB__Progress" ("PlayerName");

CREATE INDEX IF NOT EXISTS "QuestDB__Progress_QuestKind_PlayerName"
    ON "QuestDB__Progress" ("QuestKind", "PlayerName");

CREATE INDEX IF NOT EXISTS "QuestDB__Progress_Quest_Player"
    ON "QuestDB__Progress" ("QuestID", "QuestKind", "QuestType", "PlayerName");

CREATE INDEX IF NOT EXISTS "QuestDB__Complete_PlayerName"
    ON "QuestDB__Complete" ("PlayerName");
