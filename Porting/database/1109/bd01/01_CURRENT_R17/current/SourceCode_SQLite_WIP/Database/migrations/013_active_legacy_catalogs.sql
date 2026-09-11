-- Summit SQLite migration 013: active legacy tables proven by supplied .bak catalogs.
-- Scope is deliberately limited to tables referenced by active raw/singleton SQL.
-- No production rows are embedded here; data migration is a separate step.

-- ClanDB202405120913.bak
CREATE TABLE IF NOT EXISTS "ClanDB__CL"
(
    "IDX" INTEGER PRIMARY KEY AUTOINCREMENT,
    "ClanName" TEXT NULL,
    "Note" TEXT NULL,
    "NoteCnt" INTEGER NULL,
    "UserID" TEXT NOT NULL,
    "ClanZang" TEXT NULL,
    "Flag" INTEGER NULL,
    "MemCnt" INTEGER NULL,
    "MIconCnt" INTEGER NULL,
    "RegiDate" TEXT NULL,
    "LimitDate" TEXT NULL,
    "DelActive" TEXT NULL,
    "PFlag" INTEGER NULL,
    "KFlag" INTEGER NULL,
    "Cpoint" INTEGER NOT NULL,
    "CWin" INTEGER NOT NULL,
    "CFail" INTEGER NOT NULL,
    "ClanMoney" INTEGER NOT NULL,
    "CNFlag" INTEGER NOT NULL,
    "SiegeMoney" INTEGER NOT NULL,
    "LIDER_BC" INTEGER NOT NULL
);
CREATE INDEX IF NOT EXISTS "ClanDB__CL_Cpoint" ON "ClanDB__CL" ("Cpoint" DESC);
CREATE INDEX IF NOT EXISTS "ClanDB__CL_MemCnt" ON "ClanDB__CL" ("MemCnt" DESC);
CREATE INDEX IF NOT EXISTS "ClanDB__CL_MIconCnt" ON "ClanDB__CL" ("MIconCnt");
CREATE INDEX IF NOT EXISTS "ClanDB__CL_ClanName" ON "ClanDB__CL" ("ClanName");

-- Jobs202405120913.bak. Physical prefix uses JOBS because that is the database
-- name present in the qualified legacy SQL; SQLite identifiers are case-insensitive.
CREATE TABLE IF NOT EXISTS "JOBS__Jobs_Info"
(
    "ID" TEXT NOT NULL,
    "NICK" TEXT NOT NULL,
    "JOB" INTEGER NOT NULL,
    "Job_Level" INTEGER NOT NULL,
    "Job_xp" INTEGER NOT NULL,
    "M_A" INTEGER NOT NULL,
    "M_F" INTEGER NOT NULL,
    "M_L" INTEGER NOT NULL,
    "M_M" INTEGER NOT NULL,
    "M_R" INTEGER NOT NULL
);
CREATE INDEX IF NOT EXISTS "JOBS__Jobs_Info_ID" ON "JOBS__Jobs_Info" ("ID");

CREATE TABLE IF NOT EXISTS "JOBS__Jobs_shop"
(
    "ID" TEXT NOT NULL,
    "NICK" TEXT NOT NULL,
    "MINE_TYPE" INTEGER NOT NULL,
    "MINE_QTD" INTEGER NOT NULL,
    "MINE_PRICE" INTEGER NOT NULL,
    "CHECKSUM" INTEGER NOT NULL
);
CREATE INDEX IF NOT EXISTS "JOBS__Jobs_shop_CHECKSUM" ON "JOBS__Jobs_shop" ("CHECKSUM");

-- BattlePass202405120913.bak
CREATE TABLE IF NOT EXISTS "BattlePass__BP_Item"
(
    "Tag" INTEGER NOT NULL,
    "CodItem" TEXT NOT NULL,
    "Name" TEXT NOT NULL,
    "Level_Req" INTEGER NOT NULL,
    "ItemQtd" INTEGER NOT NULL,
    "ItemLock" INTEGER NOT NULL
);
CREATE INDEX IF NOT EXISTS "BattlePass__BP_Item_Tag" ON "BattlePass__BP_Item" ("Tag");

CREATE TABLE IF NOT EXISTS "BattlePass__BP_Info"
(
    "ID" TEXT NOT NULL,
    "BP_Exp" INTEGER NOT NULL,
    "BP_Level" INTEGER NOT NULL,
    "BP_Info" INTEGER NOT NULL,
    "BP_Resg1" INTEGER NOT NULL,
    "BP_Resg2" INTEGER NOT NULL,
    "BP_Resg3" INTEGER NOT NULL,
    "BP_Resg4" INTEGER NOT NULL,
    "BP_Resg5" INTEGER NOT NULL,
    "BP_Resg6" INTEGER NOT NULL,
    "BP_Resg7" INTEGER NOT NULL,
    "BP_Resg8" INTEGER NOT NULL,
    "BP_Resg9" INTEGER NOT NULL,
    "BP_Resg10" INTEGER NOT NULL,
    "BP_Resg11" INTEGER NOT NULL,
    "BP_Resg12" INTEGER NOT NULL,
    "BP_Resg13" INTEGER NOT NULL,
    "BP_Resg14" INTEGER NOT NULL,
    "BP_Resg15" INTEGER NOT NULL,
    "BP_Resg16" INTEGER NOT NULL,
    "BP_Resg17" INTEGER NOT NULL,
    "BP_Resg18" INTEGER NOT NULL,
    "BP_Resg19" INTEGER NOT NULL,
    "BP_Resg20" INTEGER NOT NULL,
    "BP_Resg21" INTEGER NOT NULL,
    "BP_Resg22" INTEGER NOT NULL,
    "BP_Resg23" INTEGER NOT NULL,
    "BP_Resg24" INTEGER NOT NULL,
    "BP_Resg25" INTEGER NOT NULL,
    "BP_Resg26" INTEGER NOT NULL,
    "BP_Resg27" INTEGER NOT NULL,
    "BP_Resg28" INTEGER NOT NULL,
    "BP_Resg29" INTEGER NOT NULL,
    "BP_Resg30" INTEGER NOT NULL,
    "BP_Resg31" INTEGER NOT NULL,
    "BP_Resg32" INTEGER NOT NULL,
    "BP_Resg33" INTEGER NOT NULL,
    "BP_Resg34" INTEGER NOT NULL,
    "BP_Resg35" INTEGER NOT NULL,
    "BP_Resg36" INTEGER NOT NULL,
    "BP_Resg37" INTEGER NOT NULL,
    "BP_Resg38" INTEGER NOT NULL,
    "BP_Resg39" INTEGER NOT NULL,
    "BP_Resg40" INTEGER NOT NULL
);
CREATE INDEX IF NOT EXISTS "BattlePass__BP_Info_ID" ON "BattlePass__BP_Info" ("ID");

-- PristonDB202405120913.bak
CREATE TABLE IF NOT EXISTS "PristonDB__Coroa" ("ID" INTEGER NOT NULL);

CREATE TABLE IF NOT EXISTS "PristonDB__Coroas"
(
    "SubTag" INTEGER NOT NULL,
    "Name" TEXT NOT NULL,
    "CodeIT" TEXT NOT NULL,
    "ID" INTEGER NOT NULL
);
CREATE INDEX IF NOT EXISTS "PristonDB__Coroas_Name" ON "PristonDB__Coroas" ("Name");

CREATE TABLE IF NOT EXISTS "PristonDB__Tags"
(
    "SubTag" INTEGER NOT NULL,
    "Clan" TEXT NOT NULL,
    "CodeIT" TEXT NOT NULL,
    "Slot" INTEGER NOT NULL
);
CREATE INDEX IF NOT EXISTS "PristonDB__Tags_Clan" ON "PristonDB__Tags" ("Clan");
CREATE INDEX IF NOT EXISTS "PristonDB__Tags_CodeIT" ON "PristonDB__Tags" ("CodeIT");

-- SoD2DB202405120913.bak
CREATE TABLE IF NOT EXISTS "SoD2DB__SOD2RecBySandurr"
(
    "SNo" INTEGER PRIMARY KEY AUTOINCREMENT,
    "UserID" TEXT NOT NULL,
    "CharName" TEXT NOT NULL,
    "CharType" INTEGER NOT NULL,
    "Point" INTEGER NOT NULL,
    "KillCount" INTEGER NOT NULL,
    "GLevel" INTEGER NOT NULL,
    "TotalPoint" INTEGER NOT NULL,
    "TotalUser" INTEGER NOT NULL,
    "SuccessUser" INTEGER NOT NULL,
    "ServerName" TEXT NOT NULL,
    "PCRNo" INTEGER NOT NULL,
    "GPCode" TEXT NOT NULL,
    "BusinessName" TEXT NOT NULL,
    "BAddress1" TEXT NOT NULL,
    "PMNo" INTEGER NOT NULL,
    "Contribute" INTEGER NOT NULL,
    "RegistDay" TEXT NOT NULL,
    "ServerTime" INTEGER NOT NULL,
    "Flag" INTEGER NOT NULL,
    "ClanIDX" INTEGER NOT NULL,
    "ClanName" TEXT NOT NULL
);
CREATE INDEX IF NOT EXISTS "SoD2DB__SOD2RecBySandurr_Point"
    ON "SoD2DB__SOD2RecBySandurr" ("Point" DESC);

-- LogadoPremiado202303061618.bak
CREATE TABLE IF NOT EXISTS "LogadoPremiado__ItemList"
(
    "ItemCode" TEXT NULL
);
