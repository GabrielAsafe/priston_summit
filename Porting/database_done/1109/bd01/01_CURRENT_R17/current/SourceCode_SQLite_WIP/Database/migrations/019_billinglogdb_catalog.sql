-- Migration 019: BillingLogDB tables recovered directly from the supplied SQL Server backup.
-- Schema-only completion. Names/order/affinity/NULLability/IDENTITY come from the recovered catalog.

CREATE TABLE IF NOT EXISTS "BillingLogDB__Bill_Error_Message" (
    "ENo" INTEGER,
    "Error_Num" TEXT NOT NULL,
    "Error_Message" TEXT NOT NULL,
    "Kor_Error_Message" TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS "BillingLogDB__ChannelLog" (
    "NUM" INTEGER,
    "BNum" INTEGER,
    "ChannelName" TEXT,
    "UserID" TEXT,
    "Operater" TEXT,
    "OperCode" TEXT,
    "Note" TEXT NOT NULL,
    "RegistDay" TEXT
);

CREATE TABLE IF NOT EXISTS "BillingLogDB__GameLog" (
    "NUM" INTEGER,
    "BNum" INTEGER,
    "ChannelName" TEXT,
    "UserID" TEXT,
    "Operater" TEXT,
    "OperCode" TEXT,
    "Note" TEXT NOT NULL,
    "RegistDay" TEXT
);

CREATE TABLE IF NOT EXISTS "BillingLogDB__PersonalLog" (
    "NUM" INTEGER,
    "BNum" INTEGER,
    "ChannelName" TEXT,
    "UserID" TEXT,
    "Operater" TEXT,
    "OperCode" TEXT,
    "Note" TEXT NOT NULL,
    "RegistDay" TEXT
);

CREATE TABLE IF NOT EXISTS "BillingLogDB__Security" (
    "AccountId" TEXT NOT NULL,
    "IP" TEXT NOT NULL,
    "Date" TEXT,
    "Protocol" TEXT
);

CREATE TABLE IF NOT EXISTS "BillingLogDB__UpdateSuccess" (
    "NUM" INTEGER,
    "Job" TEXT,
    "SuccessCount" INTEGER,
    "RegistDay" TEXT
);
