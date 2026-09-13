-- Summit SQLite migration 008: ServerDB PremiumData active contract.
-- Active callers reference CharacterName, TimeLeft and ItemType by name. The
-- former SELECT * load now explicitly projects TimeLeft/ItemType, so no legacy
-- IDENTITY/unused-column order needs to be guessed.

CREATE TABLE IF NOT EXISTS "ServerDB__PremiumData"
(
    "CharacterName" TEXT NOT NULL,
    "TimeLeft" INTEGER NOT NULL,
    "ItemType" INTEGER NOT NULL
);
CREATE INDEX IF NOT EXISTS "ServerDB__PremiumData_CharacterName"
    ON "ServerDB__PremiumData" ("CharacterName");
