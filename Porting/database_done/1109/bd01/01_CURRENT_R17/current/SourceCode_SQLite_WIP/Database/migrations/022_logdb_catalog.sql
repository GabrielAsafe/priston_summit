-- Migration 022: LogDB tables recovered directly from the supplied SQL Server backup.
-- Schema-only completion. Names/order/affinity/NULLability/IDENTITY come from the recovered catalog.

CREATE TABLE IF NOT EXISTS "LogDB__AccountLogin" (
    "ID" INTEGER PRIMARY KEY AUTOINCREMENT,
    "Account" TEXT NOT NULL,
    "Password" TEXT NOT NULL,
    "IP" TEXT NOT NULL,
    "MacAddress" TEXT NOT NULL,
    "Path" TEXT NOT NULL,
    "PCName" TEXT NOT NULL,
    "Date" TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS "LogDB__ChangePassword" (
    "AccountName" TEXT NOT NULL,
    "Password" TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS "LogDB__CheatLog" (
    "ID" INTEGER PRIMARY KEY AUTOINCREMENT,
    "IP" TEXT,
    "AccountName" TEXT,
    "Action" INTEGER,
    "LogID" INTEGER,
    "Description" TEXT,
    "Date" TEXT
);

CREATE TABLE IF NOT EXISTS "LogDB__ClanLogin" (
    "AccountName" TEXT NOT NULL,
    "Password" TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS "LogDB__Register" (
    "AccountName" TEXT NOT NULL,
    "Password" TEXT NOT NULL
);
