-- Migration 020: GameLogDB tables recovered directly from the supplied SQL Server backup.
-- Schema-only completion. Names/order/affinity/NULLability/IDENTITY come from the recovered catalog.

CREATE TABLE IF NOT EXISTS "GameLogDB__CharGameRecordLg" (
    "CharNo" INTEGER PRIMARY KEY AUTOINCREMENT,
    "UserID" TEXT NOT NULL,
    "CharType" INTEGER NOT NULL,
    "GameLogonID" TEXT NOT NULL,
    "GPCode" TEXT NOT NULL,
    "ServerName" TEXT NOT NULL,
    "IP" TEXT NOT NULL,
    "GameLevel" INTEGER NOT NULL,
    "Exp" INTEGER NOT NULL,
    "GameMoney" INTEGER NOT NULL,
    "RegistDay" TEXT NOT NULL,
    "DisuseDay" TEXT NOT NULL,
    "EditDay" TEXT NOT NULL,
    "Active" INTEGER NOT NULL
);

CREATE TABLE IF NOT EXISTS "GameLogDB__CharTop" (
    "IDX" INTEGER PRIMARY KEY AUTOINCREMENT,
    "userid" TEXT NOT NULL,
    "chartype" INTEGER NOT NULL,
    "charname" TEXT NOT NULL,
    "servername" TEXT NOT NULL,
    "glevel" INTEGER NOT NULL,
    "editday" TEXT NOT NULL,
    "exp" INTEGER NOT NULL
);

CREATE TABLE IF NOT EXISTS "GameLogDB__FlowerUserID" (
    "UserID" TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS "GameLogDB__GameOnRecord" (
    "SNo" TEXT NOT NULL,
    "UserID" TEXT NOT NULL,
    "GPCode" TEXT NOT NULL,
    "ServerName" TEXT NOT NULL,
    "LogonTime" TEXT NOT NULL,
    "IP" TEXT NOT NULL,
    "RNo" INTEGER NOT NULL
);

CREATE TABLE IF NOT EXISTS "GameLogDB__GameOutRecord" (
    "SNo" TEXT NOT NULL,
    "UserID" TEXT NOT NULL,
    "GPCode" TEXT NOT NULL,
    "ServerName" TEXT NOT NULL,
    "LogoutTime" TEXT NOT NULL,
    "IP" TEXT NOT NULL,
    "RNo" INTEGER NOT NULL
);

CREATE TABLE IF NOT EXISTS "GameLogDB__GiveYouItem" (
    "GNo" INTEGER PRIMARY KEY AUTOINCREMENT,
    "UserID" TEXT,
    "CharName" TEXT,
    "Server" TEXT,
    "Item" TEXT,
    "ItemCode" INTEGER,
    "JobCode" INTEGER,
    "Reason" TEXT,
    "Flag" TEXT,
    "Operator" TEXT NOT NULL,
    "RegistDay" TEXT,
    "EditDay" TEXT NOT NULL,
    "CancelReason" TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS "GameLogDB__HackLog0301" (
    "HNo" INTEGER,
    "UserID" TEXT,
    "CharType" INTEGER NOT NULL,
    "GameLogonId" TEXT NOT NULL,
    "GPCode" TEXT NOT NULL,
    "HCode" INTEGER NOT NULL,
    "ServerName" TEXT NOT NULL,
    "IP" TEXT NOT NULL,
    "RegistDay" TEXT NOT NULL,
    "Active" TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS "GameLogDB__ID20050304" (
    "UserID" TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS "GameLogDB__ItemBillGiveLog" (
    "INo" INTEGER PRIMARY KEY AUTOINCREMENT,
    "userid" TEXT,
    "ServerName" TEXT,
    "ServerCode" INTEGER,
    "ItemCode" TEXT,
    "ItemCount" INTEGER,
    "Active" TEXT,
    "ConfirmDate" TEXT NOT NULL,
    "CancelDate" TEXT NOT NULL,
    "RegistDay" TEXT,
    "EditDate" TEXT,
    "Note" TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS "GameLogDB__ITEMCode" (
    "ITEMNo" INTEGER PRIMARY KEY AUTOINCREMENT,
    "Category" TEXT NOT NULL,
    "CodeHName" TEXT NOT NULL,
    "CodeEName" TEXT NOT NULL,
    "CodeJName" TEXT NOT NULL,
    "Code" INTEGER NOT NULL,
    "ChangeJob" TEXT
);

CREATE TABLE IF NOT EXISTS "GameLogDB__ItemGiveList" (
    "IDX" INTEGER PRIMARY KEY AUTOINCREMENT,
    "UserID" TEXT NOT NULL,
    "ServerName" TEXT NOT NULL,
    "Flag" INTEGER NOT NULL
);

CREATE TABLE IF NOT EXISTS "GameLogDB__ItemGiveList3" (
    "IDX" INTEGER PRIMARY KEY AUTOINCREMENT,
    "UserID" TEXT,
    "ServerName" TEXT,
    "Flag" INTEGER NOT NULL
);

CREATE TABLE IF NOT EXISTS "GameLogDB__ItemGiveLog" (
    "INo" INTEGER PRIMARY KEY AUTOINCREMENT,
    "UserID" TEXT,
    "ServerCode" INTEGER,
    "ItemCategory" TEXT,
    "ItemCount" INTEGER,
    "Flag" INTEGER NOT NULL,
    "Active" TEXT,
    "ConfirmDate" TEXT NOT NULL,
    "RegistDay" TEXT,
    "Note" TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS "GameLogDB__LongCharge" (
    "IDX" INTEGER NOT NULL,
    "UserID" TEXT NOT NULL,
    "Email" TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS "GameLogDB__ServerUP" (
    "SNo" INTEGER PRIMARY KEY AUTOINCREMENT,
    "ServerName" TEXT NOT NULL,
    "UCount" INTEGER NOT NULL,
    "RegistDay" TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS "GameLogDB__tempcharrecord" (
    "UserID" TEXT NOT NULL,
    "ServerName" TEXT NOT NULL,
    "exp" INTEGER NOT NULL
);

CREATE TABLE IF NOT EXISTS "GameLogDB__UserListTemp" (
    "IDX" INTEGER PRIMARY KEY AUTOINCREMENT,
    "UserID" TEXT NOT NULL,
    "Server" TEXT NOT NULL
);
