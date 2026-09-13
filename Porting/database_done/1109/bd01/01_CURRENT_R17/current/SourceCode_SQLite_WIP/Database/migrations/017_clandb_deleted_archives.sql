-- Summit SQLite migration 017: ClanDB deletion archives proven by ClanDB202405120913.bak.
-- These tables preserve the historical clanRemove.asp audit trail while clan
-- dissolution moves from the web layer into one local SQLite transaction.

CREATE TABLE IF NOT EXISTS "ClanDB__ClanMainDeleted"
(
    "IDX" INTEGER NULL,
    "MIDX" INTEGER NULL,
    "ClanName" TEXT NULL,
    "Userid" TEXT NOT NULL,
    "ClanZang" TEXT NULL,
    "Flag" INTEGER NULL,
    "MemCnt" INTEGER NULL,
    "MIConCnt" INTEGER NULL,
    "RegiDate" TEXT NULL,
    "LimitDate" TEXT NULL,
    "DelDate" TEXT NULL,
    "DelActive" TEXT NULL,
    "PFlag" INTEGER NULL,
    "KFlag" INTEGER NULL,
    "DelState" TEXT NULL,
    "DelCase" TEXT NULL,
    "Server" INTEGER NULL
);

CREATE INDEX IF NOT EXISTS "ClanDB__ClanMainDeleted_ClanName"
    ON "ClanDB__ClanMainDeleted" ("ClanName");
CREATE INDEX IF NOT EXISTS "ClanDB__ClanMainDeleted_Userid"
    ON "ClanDB__ClanMainDeleted" ("Userid");

CREATE TABLE IF NOT EXISTS "ClanDB__ClanSubDeleted"
(
    "IDX" INTEGER NULL,
    "MIDX" INTEGER NULL,
    "userid" TEXT NULL,
    "ChName" TEXT NULL,
    "ChType" INTEGER NOT NULL,
    "ChLv" INTEGER NOT NULL,
    "ClanName" TEXT NOT NULL,
    "Permi" TEXT NULL,
    "JoinDate" TEXT NULL,
    "DelActive" TEXT NULL,
    "PFlag" INTEGER NULL,
    "KFlag" INTEGER NULL,
    "DelDate" TEXT NULL,
    "DelState" TEXT NULL,
    "DelCase" TEXT NOT NULL,
    "Server" INTEGER NULL
);

CREATE INDEX IF NOT EXISTS "ClanDB__ClanSubDeleted_ClanName"
    ON "ClanDB__ClanSubDeleted" ("ClanName");
CREATE INDEX IF NOT EXISTS "ClanDB__ClanSubDeleted_userid"
    ON "ClanDB__ClanSubDeleted" ("userid");
