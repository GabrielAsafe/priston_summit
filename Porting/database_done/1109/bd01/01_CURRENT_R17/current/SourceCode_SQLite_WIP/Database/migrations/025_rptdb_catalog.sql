-- Migration 025: rPTDB tables recovered directly from the supplied SQL Server backup.
-- Schema-only completion. Names/order/affinity/NULLability/IDENTITY come from the recovered catalog.

CREATE TABLE IF NOT EXISTS "rPTDB__LevelList" (
    "CharName" TEXT NOT NULL,
    "CharLevel" INTEGER NOT NULL,
    "CharClass" TEXT NOT NULL,
    "ID" INTEGER NOT NULL,
    "ClanName" TEXT NOT NULL,
    "ExpChar" INTEGER NOT NULL
);
