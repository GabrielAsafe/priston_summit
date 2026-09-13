-- Summit SQLite migration 007: SecurityDB contracts directly supported by
-- active queries. Ambiguous legacy tables remain intentionally absent.

-- InsertAgingFailed now names all seven used columns explicitly. SELECT/DELETE
-- callers also use named predicates/projections, removing any dependency on an
-- unused SQL Server IDENTITY column that may have existed in the legacy table.
CREATE TABLE IF NOT EXISTS "SecurityDB__AgingFailed"
(
    "PlayerID" TEXT NOT NULL,
    "PlayerName" TEXT NOT NULL,
    "ItemName" TEXT NOT NULL,
    "ItemAging" INTEGER NOT NULL,
    "Head" INTEGER NOT NULL,
    "ChkSum" INTEGER NOT NULL,
    "Date" TEXT NOT NULL
);
CREATE INDEX IF NOT EXISTS "SecurityDB__AgingFailed_PlayerItem"
    ON "SecurityDB__AgingFailed" ("PlayerID", "PlayerName", "Head", "ChkSum");

-- recordAttack names every column explicitly, so column order is not a caller ABI.
CREATE TABLE IF NOT EXISTS "SecurityDB__Security"
(
    "AccountId" TEXT NOT NULL,
    "IP" TEXT NOT NULL,
    "Date" TEXT NOT NULL,
    "Protocol" TEXT NOT NULL
);

-- RecordWindowLog only tests whether a row exists for ClassName.
CREATE TABLE IF NOT EXISTS "SecurityDB__ClassCheatList"
(
    "ClassName" TEXT NOT NULL
);
CREATE INDEX IF NOT EXISTS "SecurityDB__ClassCheatList_ClassName"
    ON "SecurityDB__ClassCheatList" ("ClassName");
