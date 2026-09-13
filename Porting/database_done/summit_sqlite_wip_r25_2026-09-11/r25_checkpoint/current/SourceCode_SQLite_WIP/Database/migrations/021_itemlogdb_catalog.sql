-- Migration 021: ItemLogDB tables recovered directly from the supplied SQL Server backup.
-- Schema-only completion. Names/order/affinity/NULLability/IDENTITY come from the recovered catalog.

CREATE TABLE IF NOT EXISTS "ItemLogDB__AItemCode" (
    "ItemCode" TEXT,
    "KorName" TEXT,
    "EngName" TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS "ItemLogDB__IL" (
    "ITEMNo" INTEGER PRIMARY KEY AUTOINCREMENT,
    "UserID" TEXT NOT NULL,
    "CharName" TEXT NOT NULL,
    "IP" TEXT NOT NULL,
    "Flag" INTEGER NOT NULL,
    "TUserID" TEXT NOT NULL,
    "TCharName" TEXT NOT NULL,
    "TMoney" INTEGER NOT NULL,
    "TIP" TEXT NOT NULL,
    "ItemCount" INTEGER NOT NULL,
    "ItemCode" INTEGER NOT NULL,
    "ItemINo" INTEGER NOT NULL,
    "ItemINo_1" INTEGER NOT NULL,
    "RegistDay" TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS "ItemLogDB__ITEMCODE" (
    "ITEMNo" INTEGER,
    "Category" TEXT NOT NULL,
    "CodeHName" TEXT NOT NULL,
    "CodeEName" TEXT NOT NULL,
    "Code" INTEGER NOT NULL
);
