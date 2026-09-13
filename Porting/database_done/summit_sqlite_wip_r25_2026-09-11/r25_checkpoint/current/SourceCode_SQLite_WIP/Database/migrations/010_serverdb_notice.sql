-- Summit SQLite migration 010: ServerDB NoticeSystem.
-- Exact four-column order recovered from the supplied ServerDB.bak catalog.

CREATE TABLE IF NOT EXISTS "ServerDB__NoticeSystem"
(
    "ID" INTEGER NULL,
    "Message" TEXT NOT NULL,
    "Type" INTEGER NOT NULL,
    "Activated" INTEGER NOT NULL
);
CREATE INDEX IF NOT EXISTS "ServerDB__NoticeSystem_Activated"
    ON "ServerDB__NoticeSystem" ("Activated");
