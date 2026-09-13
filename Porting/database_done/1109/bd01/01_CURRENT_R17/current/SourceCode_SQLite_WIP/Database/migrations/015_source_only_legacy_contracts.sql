-- Summit SQLite migration 015: source-only legacy persistence contracts.
-- IMPORTANT: these tables are NOT claimed to reconstruct a supplied SQL Server
-- backup. They are absent from the supplied .bak catalogs. Their SQLite layouts
-- are deliberately minimal and are based only on fields the active server reads
-- or writes after callers were changed to use named columns.

-- StartUser_PresentItem_Init only consumes an item code. The old SELECT * read
-- column 2, but the backup has no ExtraItems table, so no unused legacy columns
-- are invented here.
CREATE TABLE IF NOT EXISTS "StartUserDB__ExtraItems"
(
    "ItemCode" TEXT NOT NULL
);

-- Ban checks only require existence by IP/MAC. BannedIP writes the account and
-- local server time for audit. BannedMac currently has no active writer; it can
-- be populated by the future backend/admin tooling.
CREATE TABLE IF NOT EXISTS "SecurityDB__BannedIP"
(
    "ID" INTEGER PRIMARY KEY AUTOINCREMENT,
    "IP" TEXT NOT NULL,
    "AccountID" TEXT NULL,
    "Date" TEXT NOT NULL
);
CREATE INDEX IF NOT EXISTS "SecurityDB__BannedIP_IP"
    ON "SecurityDB__BannedIP" ("IP");

CREATE TABLE IF NOT EXISTS "SecurityDB__BannedMac"
(
    "ID" INTEGER PRIMARY KEY AUTOINCREMENT,
    "MacAddress" TEXT NOT NULL
);
CREATE INDEX IF NOT EXISTS "SecurityDB__BannedMac_MacAddress"
    ON "SecurityDB__BannedMac" ("MacAddress");

-- The active writer supplies exactly these four aging fields and there is no
-- active reader. Keep a local surrogate ID for stable administration/history.
CREATE TABLE IF NOT EXISTS "SecurityDB__AgingRecord"
(
    "ID" INTEGER PRIMARY KEY AUTOINCREMENT,
    "ItemName" TEXT NOT NULL,
    "ItemAging" INTEGER NOT NULL,
    "Head" INTEGER NOT NULL,
    "ChkSum" INTEGER NOT NULL
);
