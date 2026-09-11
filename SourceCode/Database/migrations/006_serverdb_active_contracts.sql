-- Summit SQLite migration 006: ServerDB tables whose active layout is proven
-- by named writes and/or positional SELECT * reads in the current server.

-- InsertDB names all four columns; SELECT * reads IdVip from column 2.
CREATE TABLE IF NOT EXISTS "ServerDB__Vip"
(
    "PlayerName" TEXT NOT NULL,
    "IdVip" INTEGER NOT NULL,
    "DateStart" TEXT NOT NULL,
    "DateEnd" TEXT NOT NULL
);
CREATE INDEX IF NOT EXISTS "ServerDB__Vip_PlayerName"
    ON "ServerDB__Vip" ("PlayerName");

-- OpenPremiumBox now projects ItemCode/Count/Class explicitly and filters by Kind;
-- the migration no longer depends on an unknown legacy SELECT * column order.
CREATE TABLE IF NOT EXISTS "ServerDB__BoxRewards"
(
    "Kind" INTEGER NOT NULL,
    "ItemCode" TEXT NOT NULL,
    "Count" INTEGER NOT NULL,
    "Class" INTEGER NOT NULL
);
CREATE INDEX IF NOT EXISTS "ServerDB__BoxRewards_Kind"
    ON "ServerDB__BoxRewards" ("Kind");

-- The active contract selects only ItemCode; no positional SELECT * dependency.
CREATE TABLE IF NOT EXISTS "ServerDB__DropLogAlert"
(
    "ItemCode" TEXT NOT NULL
);
