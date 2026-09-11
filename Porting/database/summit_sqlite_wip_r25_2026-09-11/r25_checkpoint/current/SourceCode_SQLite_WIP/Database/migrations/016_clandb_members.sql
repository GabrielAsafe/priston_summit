-- Summit SQLite migration 016: ClanDB member catalog proven by ClanDB202405120913.bak.
-- UL is required to replace read-only clan membership web endpoints with local DB reads.

CREATE TABLE IF NOT EXISTS "ClanDB__UL"
(
    "IDX" INTEGER NULL,
    "MIDX" INTEGER PRIMARY KEY AUTOINCREMENT,
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
    "MIconCnt" INTEGER NULL
);

CREATE INDEX IF NOT EXISTS "ClanDB__UL_ChName" ON "ClanDB__UL" ("ChName");
CREATE INDEX IF NOT EXISTS "ClanDB__UL_userid" ON "ClanDB__UL" ("userid");
CREATE INDEX IF NOT EXISTS "ClanDB__UL_ClanName" ON "ClanDB__UL" ("ClanName");
CREATE INDEX IF NOT EXISTS "ClanDB__CL_UserID" ON "ClanDB__CL" ("UserID");
