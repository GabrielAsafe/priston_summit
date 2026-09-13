-- Migration 023: PCRoom tables recovered directly from the supplied SQL Server backup.
-- Schema-only completion. Names/order/affinity/NULLability/IDENTITY come from the recovered catalog.

CREATE TABLE IF NOT EXISTS "PCRoom__IPChangeLog" (
    "INo" INTEGER,
    "RNo" INTEGER,
    "UserID" TEXT,
    "GPCode" TEXT,
    "TIPAddress1_G" TEXT NOT NULL,
    "TIPAddress2_G" TEXT NOT NULL,
    "TIPAddress3_G" TEXT NOT NULL,
    "TIPAddress1" TEXT NOT NULL,
    "TIPAddress2" TEXT NOT NULL,
    "TIPAddress3" TEXT NOT NULL,
    "TIPAddress4" TEXT NOT NULL,
    "TIPAddress5" TEXT NOT NULL,
    "TIPAddress6" TEXT NOT NULL,
    "TIPAddress7" TEXT NOT NULL,
    "TIPAddress8" TEXT NOT NULL,
    "TIPAddress9" TEXT NOT NULL,
    "TIPAddress10" TEXT NOT NULL,
    "BonusIP" TEXT NOT NULL,
    "IPCount" INTEGER,
    "AddIPCount" INTEGER,
    "RegistDay" TEXT NOT NULL,
    "DisuseDay" TEXT NOT NULL,
    "UsePeriod" INTEGER NOT NULL,
    "Active" TEXT,
    "UNo" INTEGER NOT NULL,
    "PMNo" INTEGER,
    "Status" TEXT,
    "Note" TEXT NOT NULL,
    "EditDate" TEXT
);

CREATE TABLE IF NOT EXISTS "PCRoom__IPList" (
    "INo" INTEGER,
    "PMNo" INTEGER,
    "RNo" INTEGER,
    "IP" TEXT
);

CREATE TABLE IF NOT EXISTS "PCRoom__RegIPAddress" (
    "RNo" INTEGER,
    "UserID" TEXT,
    "GPCode" TEXT,
    "TIPAddress1_G" TEXT NOT NULL,
    "TIPAddress2_G" TEXT NOT NULL,
    "TIPAddress3_G" TEXT NOT NULL,
    "TIPAddress1" TEXT NOT NULL,
    "TIPAddress2" TEXT NOT NULL,
    "TIPAddress3" TEXT NOT NULL,
    "TIPAddress4" TEXT NOT NULL,
    "TIPAddress5" TEXT NOT NULL,
    "TIPAddress6" TEXT NOT NULL,
    "TIPAddress7" TEXT NOT NULL,
    "TIPAddress8" TEXT NOT NULL,
    "TIPAddress9" TEXT NOT NULL,
    "TIPAddress10" TEXT NOT NULL,
    "BonusIP" TEXT NOT NULL,
    "IPCount" INTEGER,
    "AddIPCount" INTEGER,
    "RegistDay" TEXT NOT NULL,
    "DisuseDay" TEXT NOT NULL,
    "UsePeriod" INTEGER NOT NULL,
    "Active" TEXT,
    "UNo" INTEGER NOT NULL,
    "PMNo" INTEGER
);
