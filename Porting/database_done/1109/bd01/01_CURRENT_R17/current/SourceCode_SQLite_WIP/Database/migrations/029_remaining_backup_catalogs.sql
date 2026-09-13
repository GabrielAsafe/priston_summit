-- Migration 029: remaining backup-proven catalogs not already materialized by migrations 001-028.
-- Domains: BattlePass, CoinShopDB, PristonDB, SecurityDB.
-- Schema-only completion from the supplied SQL Server backup catalogs.

-- BattlePass: BattlePass202405120913.bak
CREATE TABLE IF NOT EXISTS "BattlePass__BP_XP" (
    "Mobs" INTEGER NOT NULL,
    "PVP" INTEGER NOT NULL
);

-- CoinShopDB: CoinShopDB202405120913.bak
CREATE TABLE IF NOT EXISTS "CoinShopDB__PendingPayments" (
    "ID" INTEGER PRIMARY KEY AUTOINCREMENT,
    "UserID" TEXT,
    "Amount" REAL,
    "Coins" INTEGER,
    "MPPaymentID" TEXT,
    "Status" TEXT,
    "CreatedDate" TEXT,
    "CompletedDate" TEXT NOT NULL,
    "IP" TEXT NOT NULL
);

-- CoinShopDB: CoinShopDB202405120913.bak
CREATE TABLE IF NOT EXISTS "CoinShopDB__Transactions" (
    "Id" INTEGER PRIMARY KEY AUTOINCREMENT,
    "UserID" TEXT,
    "PaymentID" TEXT,
    "Amount" REAL,
    "Coins" INTEGER,
    "Status" TEXT,
    "Date" TEXT
);

-- PristonDB: PristonDB202405120913.bak
CREATE TABLE IF NOT EXISTS "PristonDB__AgeFail" (
    "SubTag" INTEGER NOT NULL,
    "Hora" INTEGER NOT NULL,
    "Minuto" INTEGER NOT NULL,
    "Segundo" INTEGER NOT NULL,
    "ID" TEXT NOT NULL,
    "Nome" TEXT NOT NULL,
    "IP" TEXT NOT NULL,
    "Item" TEXT NOT NULL,
    "Aging" INTEGER NOT NULL,
    "Head" INTEGER NOT NULL,
    "Checksum" INTEGER NOT NULL,
    "Dwtime" TEXT NOT NULL,
    "Dwversion" INTEGER NOT NULL,
    "Resgate" INTEGER NOT NULL,
    "Dia" INTEGER NOT NULL,
    "Mes" INTEGER NOT NULL,
    "Ano" INTEGER NOT NULL
);

-- PristonDB: PristonDB202405120913.bak
CREATE TABLE IF NOT EXISTS "PristonDB__Beta" (
    "accountId" TEXT NOT NULL
);

-- PristonDB: PristonDB202405120913.bak
CREATE TABLE IF NOT EXISTS "PristonDB__CheatLog" (
    "ID" INTEGER PRIMARY KEY AUTOINCREMENT,
    "IP" TEXT,
    "AccountName" TEXT,
    "Action" INTEGER,
    "LogID" INTEGER,
    "Description" TEXT,
    "Date" TEXT
);

-- PristonDB: PristonDB202405120913.bak
CREATE TABLE IF NOT EXISTS "PristonDB__FunctionChecksumList" (
    "ID" INTEGER PRIMARY KEY AUTOINCREMENT,
    "Size" INTEGER,
    "Address" TEXT,
    "Checksum" TEXT
);

-- PristonDB: PristonDB202405120913.bak
CREATE TABLE IF NOT EXISTS "PristonDB__LiberarPais" (
    "IdLiberarPais" INTEGER PRIMARY KEY AUTOINCREMENT,
    "Nome" TEXT,
    "IndSit" INTEGER
);

-- PristonDB: PristonDB202405120913.bak
CREATE TABLE IF NOT EXISTS "PristonDB__ListaNegra" (
    "IdListaNegra" INTEGER PRIMARY KEY AUTOINCREMENT,
    "IdCliente" INTEGER,
    "CodigoPc" TEXT,
    "Ip" TEXT,
    "Data" TEXT,
    "IndSit" INTEGER
);

-- PristonDB: PristonDB202405120913.bak
CREATE TABLE IF NOT EXISTS "PristonDB__LoginAutenticado" (
    "IdLoginAutenticado" INTEGER PRIMARY KEY AUTOINCREMENT,
    "Ip" TEXT,
    "DataAutenticado" TEXT
);

-- PristonDB: PristonDB202405120913.bak
CREATE TABLE IF NOT EXISTS "PristonDB__LoginMacAdresss" (
    "IdLoginMacAdresss" INTEGER PRIMARY KEY AUTOINCREMENT,
    "Login" TEXT,
    "MacAddress" TEXT,
    "IP" TEXT,
    "Bloqueado" INTEGER,
    "DataLogin" TEXT
);

-- PristonDB: PristonDB202405120913.bak
CREATE TABLE IF NOT EXISTS "PristonDB__LogsIpsOnline" (
    "IdLogsIpsOnline" INTEGER PRIMARY KEY AUTOINCREMENT,
    "Ip" TEXT,
    "DataOnline" TEXT
);

-- PristonDB: PristonDB202405120913.bak
CREATE TABLE IF NOT EXISTS "PristonDB__LogsProtecao" (
    "IdLogsProtecao" INTEGER PRIMARY KEY AUTOINCREMENT,
    "CodigoPc" TEXT,
    "Ip" TEXT,
    "DataLog" TEXT
);

-- PristonDB: PristonDB202405120913.bak
CREATE TABLE IF NOT EXISTS "PristonDB__NoticeSystem" (
    "ID" INTEGER,
    "Message" TEXT NOT NULL,
    "Type" INTEGER NOT NULL,
    "Activated" INTEGER NOT NULL
);

-- PristonDB: PristonDB202405120913.bak
CREATE TABLE IF NOT EXISTS "PristonDB__PremiumData" (
    "ID" INTEGER PRIMARY KEY AUTOINCREMENT,
    "CharacterName" TEXT,
    "TimeLeft" INTEGER,
    "ItemType" INTEGER
);

-- PristonDB: PristonDB202405120913.bak
CREATE TABLE IF NOT EXISTS "PristonDB__PVP" (
    "ID" TEXT NOT NULL,
    "Name" TEXT NOT NULL,
    "Class" INTEGER NOT NULL,
    "Lvl" INTEGER NOT NULL,
    "Kills" INTEGER NOT NULL,
    "Deads" INTEGER NOT NULL
);

-- PristonDB: PristonDB202405120913.bak
CREATE TABLE IF NOT EXISTS "PristonDB__Security" (
    "AccountId" TEXT NOT NULL,
    "IP" TEXT NOT NULL,
    "Date" TEXT,
    "Protocol" TEXT
);

-- PristonDB: PristonDB202405120913.bak
CREATE TABLE IF NOT EXISTS "PristonDB__TopLevel" (
    "ID" TEXT,
    "NAME" TEXT,
    "CLASS" INTEGER,
    "LVL" INTEGER,
    "XP" INTEGER
);

-- PristonDB: PristonDB202405120913.bak
CREATE TABLE IF NOT EXISTS "PristonDB__WindowCheatList" (
    "ID" INTEGER PRIMARY KEY AUTOINCREMENT,
    "WindowName" TEXT
);

-- SecurityDB: SecurityDB202405120913.bak
CREATE TABLE IF NOT EXISTS "SecurityDB__AttackLog" (
    "id" INTEGER PRIMARY KEY AUTOINCREMENT,
    "accountId" TEXT NOT NULL,
    "IP" TEXT NOT NULL,
    "description" TEXT NOT NULL,
    "date" TEXT
);
