-- Migration 028: ClanDB tables recovered directly from the supplied SQL Server backup.
-- Schema-only completion. Names/order/affinity/NULLability/IDENTITY come from the recovered catalog.

CREATE TABLE IF NOT EXISTS "ClanDB__BBSMain" (
    "IDX" INTEGER,
    "MIDX" INTEGER,
    "Userid" TEXT,
    "WriteName" TEXT,
    "Title" TEXT,
    "Content" TEXT,
    "SelServer" INTEGER,
    "RegiDate" TEXT,
    "RegiIP" TEXT,
    "Hit" INTEGER,
    "CountCom" INTEGER,
    "NickName" TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS "ClanDB__BBSSub" (
    "SIDX" INTEGER,
    "MINDEX" INTEGER,
    "Userid" TEXT,
    "WriteName" TEXT,
    "Content" TEXT,
    "RegiDate" TEXT,
    "RegiIP" TEXT,
    "NickName" TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS "ClanDB__ChipLog" (
    "LgIDX" INTEGER,
    "CIDX" INTEGER,
    "CServer" INTEGER,
    "Userid" TEXT,
    "Chname" TEXT,
    "Permi" TEXT,
    "RegiDate" TEXT
);

CREATE TABLE IF NOT EXISTS "ClanDB__ClanHome" (
    "IDX" INTEGER,
    "MIDX" INTEGER,
    "SelServer" INTEGER,
    "Intro" TEXT NOT NULL,
    "Image" TEXT NOT NULL,
    "Skin" INTEGER NOT NULL,
    "PrivHome" INTEGER NOT NULL,
    "PrivMem" INTEGER NOT NULL,
    "PrivBoard" INTEGER NOT NULL
);

CREATE TABLE IF NOT EXISTS "ClanDB__ClanMainCharChangeLog" (
    "IDX" INTEGER,
    "UserID" TEXT NOT NULL,
    "BeforeCharName" TEXT NOT NULL,
    "AfterCharName" TEXT NOT NULL,
    "CServer" INTEGER NOT NULL,
    "RegiDate" TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS "ClanDB__ClanMainView" (
    "IDX" INTEGER,
    "ClanName" TEXT,
    "Note" TEXT,
    "NoteCnt" INTEGER,
    "UserID" TEXT NOT NULL,
    "ClanZang" TEXT,
    "Flag" INTEGER,
    "MemCnt" INTEGER,
    "MIconCnt" INTEGER,
    "RegiDate" TEXT,
    "LimitDate" TEXT,
    "DelActive" TEXT,
    "PFlag" INTEGER,
    "KFlag" INTEGER,
    "Cpoint" INTEGER NOT NULL,
    "CWin" INTEGER NOT NULL,
    "CFail" INTEGER NOT NULL,
    "ClanMoney" INTEGER NOT NULL,
    "CNFlag" INTEGER NOT NULL,
    "SiegeMoney" INTEGER NOT NULL,
    "Server" TEXT
);

CREATE TABLE IF NOT EXISTS "ClanDB__ClanMoneyLog" (
    "IDX" INTEGER,
    "CIDX" INTEGER NOT NULL,
    "UserID" TEXT NOT NULL,
    "chName" TEXT NOT NULL,
    "ServerName" TEXT NOT NULL,
    "Operator" TEXT NOT NULL,
    "OperCode" TEXT NOT NULL,
    "ClanMoney" INTEGER NOT NULL,
    "RegistDay" TEXT NOT NULL,
    "Note" TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS "ClanDB__ClanSubCharChangeLog" (
    "IDX" INTEGER,
    "UserID" TEXT NOT NULL,
    "BeforeCharName" TEXT NOT NULL,
    "AfterCharName" TEXT NOT NULL,
    "CServer" INTEGER NOT NULL,
    "RegiDate" TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS "ClanDB__ClanSubView" (
    "IDX" INTEGER,
    "MIDX" INTEGER,
    "userid" TEXT,
    "ChName" TEXT,
    "ChType" INTEGER NOT NULL,
    "ChLv" INTEGER NOT NULL,
    "ClanName" TEXT NOT NULL,
    "Permi" TEXT,
    "JoinDate" TEXT,
    "DelActive" TEXT,
    "PFlag" INTEGER,
    "KFlag" INTEGER,
    "Server" TEXT
);

CREATE TABLE IF NOT EXISTS "ClanDB__ClanTicket" (
    "SNo" INTEGER NOT NULL,
    "ServerName" TEXT NOT NULL,
    "MIDX" INTEGER NOT NULL,
    "ClanName" TEXT NOT NULL,
    "ClanJang" INTEGER NOT NULL,
    "ClanImage" TEXT NOT NULL,
    "UserID" TEXT NOT NULL,
    "ChName" TEXT NOT NULL,
    "GPCode" TEXT NOT NULL,
    "LogonTime" TEXT NOT NULL,
    "IP" TEXT NOT NULL,
    "RNo" INTEGER NOT NULL,
    "Flag" INTEGER NOT NULL
);

CREATE TABLE IF NOT EXISTS "ClanDB__CT" (
    "SNo" INTEGER NOT NULL,
    "ServerName" TEXT NOT NULL,
    "MIDX" INTEGER NOT NULL,
    "ClanName" TEXT NOT NULL,
    "ClanJang" INTEGER NOT NULL,
    "ClanImage" TEXT NOT NULL,
    "UserID" TEXT NOT NULL,
    "ChName" TEXT NOT NULL,
    "GPCode" TEXT NOT NULL,
    "LogonTime" TEXT NOT NULL,
    "IP" TEXT NOT NULL,
    "RNo" INTEGER NOT NULL,
    "Flag" INTEGER NOT NULL
);

CREATE TABLE IF NOT EXISTS "ClanDB__GameNotice" (
    "UserID" TEXT NOT NULL,
    "ChName" TEXT NOT NULL,
    "Server" INTEGER NOT NULL,
    "PFlag" INTEGER NOT NULL,
    "Txt" TEXT NOT NULL,
    "Flag" INTEGER NOT NULL
);

CREATE TABLE IF NOT EXISTS "ClanDB__GameNoticeGroup" (
    "IDX" INTEGER,
    "GroupCD" INTEGER,
    "MainCD" INTEGER,
    "DetailCD" INTEGER,
    "Title" TEXT,
    "Content" TEXT,
    "FromDay" TEXT,
    "ToDay" TEXT,
    "RegistDay" TEXT,
    "DelActive" INTEGER,
    "Hit" INTEGER,
    "Flag" INTEGER NOT NULL
);

CREATE TABLE IF NOT EXISTS "ClanDB__GameNoticePrivate" (
    "IDX" INTEGER,
    "Userid" TEXT,
    "Title" TEXT,
    "Content" TEXT,
    "FromDay" TEXT,
    "ToDay" TEXT,
    "RegistDay" TEXT,
    "DelActive" INTEGER,
    "Hit" INTEGER,
    "Flag" INTEGER NOT NULL
);

CREATE TABLE IF NOT EXISTS "ClanDB__LI" (
    "IMG" INTEGER,
    "ID" INTEGER
);

CREATE TABLE IF NOT EXISTS "ClanDB__Minutos" (
    "ChName" TEXT NOT NULL,
    "Minutos" INTEGER NOT NULL
);

CREATE TABLE IF NOT EXISTS "ClanDB__Notice" (
    "IDX" INTEGER,
    "MIDX" INTEGER,
    "Userid" TEXT,
    "WriteName" TEXT,
    "Title" TEXT,
    "Content" TEXT,
    "SelServer" INTEGER,
    "RegiDate" TEXT,
    "RegiIP" TEXT,
    "Hit" INTEGER
);

CREATE TABLE IF NOT EXISTS "ClanDB__PRBbsFile" (
    "IDX" INTEGER,
    "MINDEX" INTEGER,
    "FileName" TEXT,
    "FileSize" TEXT,
    "RegiDate" TEXT
);

CREATE TABLE IF NOT EXISTS "ClanDB__PRBBSMain" (
    "IDX" INTEGER,
    "PMNo" TEXT,
    "Userid" TEXT,
    "Title" TEXT,
    "Content" TEXT,
    "UserName" TEXT,
    "ClanIdx" INTEGER,
    "SelServer" INTEGER,
    "RegiDate" TEXT,
    "RegiIP" TEXT,
    "Hit" INTEGER,
    "CountCom" INTEGER
);

CREATE TABLE IF NOT EXISTS "ClanDB__PRBBSSub" (
    "SIDX" INTEGER,
    "MINDEX" INTEGER,
    "PMNo" TEXT,
    "userid" TEXT,
    "writename" TEXT,
    "Content" TEXT,
    "RegiDate" TEXT,
    "RegiIP" TEXT,
    "UserName" TEXT
);

CREATE TABLE IF NOT EXISTS "ClanDB__SiegeClanList" (
    "IDX" INTEGER,
    "UserID" TEXT,
    "CharName" TEXT,
    "ClanName" TEXT,
    "TaxRate" INTEGER,
    "ServerName" TEXT,
    "Server" INTEGER,
    "OperFrom" TEXT NOT NULL,
    "OperTo" TEXT NOT NULL,
    "RegistDay" TEXT
);

CREATE TABLE IF NOT EXISTS "ClanDB__SiegeCurrentList" (
    "IDX" INTEGER,
    "CIDX" INTEGER NOT NULL,
    "UserID" TEXT NOT NULL,
    "chName" TEXT NOT NULL,
    "ServerName" TEXT NOT NULL,
    "Operator" TEXT NOT NULL,
    "OperCode" TEXT NOT NULL,
    "SiegeMoney" INTEGER NOT NULL,
    "RegistDay" TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS "ClanDB__SiegeMoneyLog" (
    "IDX" INTEGER PRIMARY KEY AUTOINCREMENT,
    "CIDX" INTEGER NOT NULL,
    "UserID" TEXT NOT NULL,
    "chName" TEXT NOT NULL,
    "ServerName" TEXT NOT NULL,
    "Operator" TEXT NOT NULL,
    "OperCode" TEXT NOT NULL,
    "SiegeMoney" INTEGER NOT NULL,
    "RegistDay" TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS "ClanDB__SiegeMoneyTax" (
    "DNo" INTEGER PRIMARY KEY AUTOINCREMENT,
    "IDX" INTEGER NOT NULL,
    "Mixing" INTEGER NOT NULL,
    "Aging" INTEGER NOT NULL,
    "Shop" INTEGER NOT NULL,
    "Poison1" INTEGER NOT NULL,
    "Poison2" INTEGER NOT NULL,
    "Poison3" INTEGER NOT NULL,
    "Force" INTEGER NOT NULL,
    "Warpgate" INTEGER NOT NULL,
    "Skill" INTEGER NOT NULL,
    "Total" INTEGER NOT NULL,
    "Tax" INTEGER NOT NULL,
    "ServerName" TEXT NOT NULL,
    "RegistDay" TEXT NOT NULL
);
