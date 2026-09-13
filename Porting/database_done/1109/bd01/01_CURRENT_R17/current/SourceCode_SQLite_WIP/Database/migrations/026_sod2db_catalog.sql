-- Migration 026: SoD2DB tables recovered directly from the supplied SQL Server backup.
-- Schema-only completion. Names/order/affinity/NULLability/IDENTITY come from the recovered catalog.

CREATE TABLE IF NOT EXISTS "SoD2DB__SoD2BBSMain" (
    "IDX" INTEGER,
    "Userid" TEXT,
    "WriteName" TEXT,
    "Title" TEXT NOT NULL,
    "Job" INTEGER NOT NULL,
    "Content" TEXT,
    "SelServer" INTEGER,
    "RegiDate" TEXT,
    "RegiIP" TEXT,
    "Hit" INTEGER,
    "CountCom" INTEGER,
    "OwnerID" TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS "SoD2DB__SoD2BBSSub" (
    "SIDX" INTEGER,
    "MINDEX" INTEGER,
    "Userid" TEXT,
    "WriteName" TEXT,
    "Content" TEXT,
    "RegiDate" TEXT,
    "RegiIP" TEXT
);

CREATE TABLE IF NOT EXISTS "SoD2DB__Sod2Config" (
    "MIDX" INTEGER NOT NULL,
    "ClanName" TEXT NOT NULL,
    "UserID" TEXT NOT NULL,
    "CharName" TEXT NOT NULL,
    "SelServer" INTEGER,
    "ServerName" TEXT NOT NULL,
    "TaxRates" INTEGER,
    "OperFrom" TEXT,
    "OperTo" TEXT,
    "TotalEnterMoney" INTEGER NOT NULL,
    "BeforeMoney" INTEGER NOT NULL,
    "TodayMoney" INTEGER NOT NULL,
    "TotalMoney" INTEGER NOT NULL,
    "TotalEnterNum" INTEGER NOT NULL,
    "UpdateDate" TEXT NOT NULL,
    "Note" TEXT NOT NULL,
    "MIDX1" INTEGER NOT NULL,
    "MIDX2" INTEGER NOT NULL,
    "M0money" INTEGER NOT NULL,
    "M1money" INTEGER NOT NULL,
    "M2money" INTEGER NOT NULL,
    "VInfo" TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS "SoD2DB__Sod2ConfigLog" (
    "IDX" INTEGER,
    "MIDX" INTEGER NOT NULL,
    "ClanName" TEXT NOT NULL,
    "UserID" TEXT NOT NULL,
    "CharName" TEXT NOT NULL,
    "SelServer" INTEGER,
    "ServerName" TEXT NOT NULL,
    "TaxRates" INTEGER,
    "OperFrom" TEXT,
    "OperTo" TEXT,
    "TotalEnterMoney" INTEGER NOT NULL,
    "BeforeMoney" INTEGER NOT NULL,
    "TodayMoney" INTEGER NOT NULL,
    "TotalMoney" INTEGER NOT NULL,
    "TotalEnterNum" INTEGER NOT NULL,
    "UpdateDate" TEXT NOT NULL,
    "Note" TEXT NOT NULL,
    "MIDX1" INTEGER NOT NULL,
    "MIDX2" INTEGER NOT NULL,
    "M0money" INTEGER NOT NULL,
    "M1money" INTEGER NOT NULL,
    "M2money" INTEGER NOT NULL
);

CREATE TABLE IF NOT EXISTS "SoD2DB__SoD2Notice" (
    "IDX" INTEGER,
    "MIDX" INTEGER,
    "SelServer" INTEGER,
    "UserID" TEXT,
    "Notice" TEXT,
    "RegiDate" TEXT
);

CREATE TABLE IF NOT EXISTS "SoD2DB__Sod2TaxLog" (
    "IDX" INTEGER,
    "MIDX" INTEGER NOT NULL,
    "ClanName" TEXT NOT NULL,
    "MCount" INTEGER NOT NULL,
    "UserID" TEXT NOT NULL,
    "CharName" TEXT NOT NULL,
    "SelServer" INTEGER NOT NULL,
    "TaxRates" INTEGER NOT NULL,
    "OperFrom" TEXT NOT NULL,
    "OperTo" TEXT NOT NULL,
    "RegiDate" TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS "SoD2DB__temp" (
    "IDX" INTEGER,
    "CIDX" INTEGER,
    "CServerTime" INTEGER,
    "Server" INTEGER,
    "ClanName" TEXT,
    "ServerName" TEXT,
    "CPoint" INTEGER,
    "CRegistDay" TEXT,
    "ServerTime_Temp" INTEGER NOT NULL,
    "Point_Temp" INTEGER NOT NULL
);

CREATE TABLE IF NOT EXISTS "SoD2DB__test" (
    "IDX" INTEGER,
    "CIDX" INTEGER,
    "CServerTime" INTEGER,
    "Server" INTEGER,
    "ClanName" TEXT,
    "ServerName" TEXT,
    "CPoint" INTEGER,
    "CRegistDay" TEXT,
    "ServerTime_Temp" INTEGER NOT NULL,
    "Point_Temp" INTEGER NOT NULL
);

CREATE TABLE IF NOT EXISTS "SoD2DB__UserBlackList" (
    "SOD2No" INTEGER,
    "UserID" TEXT,
    "CharName" TEXT,
    "ServerCode" INTEGER,
    "Memo" TEXT NOT NULL,
    "RegistDay" TEXT NOT NULL
);
