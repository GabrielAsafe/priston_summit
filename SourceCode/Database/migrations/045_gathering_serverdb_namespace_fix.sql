-- Fix Gathering/Mining V2 physical table namespace.
-- SQLConnection(DATABASEID_ServerDB) rewrites bare table names to ServerDB__*.
-- Migration 044 created Gathering tables without that prefix, so the runtime
-- could not see the seeded catalog. This migration preserves any existing
-- resources, nodes and player inventory while moving them to the namespace
-- expected by the SQLite compatibility layer.

-- Ensure the legacy/unprefixed source tables exist even on a DB whose
-- user_version was advanced manually/incompletely.
CREATE TABLE IF NOT EXISTS GatheringResourceDefinition (
    ResourceId INTEGER NOT NULL PRIMARY KEY,
    ResourceKey TEXT NOT NULL UNIQUE,
    Kind INTEGER NOT NULL,
    RequiredProfession INTEGER NOT NULL DEFAULT 0,
    RequiredLevel INTEGER NOT NULL DEFAULT 0,
    MinimumYield INTEGER NOT NULL DEFAULT 1,
    MaximumYield INTEGER NOT NULL DEFAULT 1,
    Enabled INTEGER NOT NULL DEFAULT 1
);

CREATE TABLE IF NOT EXISTS GatheringNodeDefinition (
    NodeId INTEGER NOT NULL PRIMARY KEY,
    ResourceId INTEGER NOT NULL,
    Area INTEGER NOT NULL,
    X INTEGER NOT NULL,
    Y INTEGER NOT NULL DEFAULT 0,
    Z INTEGER NOT NULL,
    InteractionRadius INTEGER NOT NULL DEFAULT 160,
    RespawnMilliseconds INTEGER NOT NULL DEFAULT 30000,
    Enabled INTEGER NOT NULL DEFAULT 1
);

CREATE TABLE IF NOT EXISTS GatheringPlayerInventory (
    PlayerKey TEXT NOT NULL,
    ResourceKey TEXT NOT NULL,
    Quantity INTEGER NOT NULL DEFAULT 0,
    UpdatedAt TEXT NOT NULL DEFAULT (datetime('now','localtime')),
    PRIMARY KEY(PlayerKey, ResourceKey)
);

CREATE TABLE IF NOT EXISTS ServerDB__GatheringResourceDefinition (
    ResourceId INTEGER NOT NULL PRIMARY KEY,
    ResourceKey TEXT NOT NULL UNIQUE,
    Kind INTEGER NOT NULL,
    RequiredProfession INTEGER NOT NULL DEFAULT 0,
    RequiredLevel INTEGER NOT NULL DEFAULT 0,
    MinimumYield INTEGER NOT NULL DEFAULT 1,
    MaximumYield INTEGER NOT NULL DEFAULT 1,
    Enabled INTEGER NOT NULL DEFAULT 1
);

CREATE TABLE IF NOT EXISTS ServerDB__GatheringNodeDefinition (
    NodeId INTEGER NOT NULL PRIMARY KEY,
    ResourceId INTEGER NOT NULL,
    Area INTEGER NOT NULL,
    X INTEGER NOT NULL,
    Y INTEGER NOT NULL DEFAULT 0,
    Z INTEGER NOT NULL,
    InteractionRadius INTEGER NOT NULL DEFAULT 160,
    RespawnMilliseconds INTEGER NOT NULL DEFAULT 30000,
    Enabled INTEGER NOT NULL DEFAULT 1
);

CREATE TABLE IF NOT EXISTS ServerDB__GatheringPlayerInventory (
    PlayerKey TEXT NOT NULL,
    ResourceKey TEXT NOT NULL,
    Quantity INTEGER NOT NULL DEFAULT 0,
    UpdatedAt TEXT NOT NULL DEFAULT (datetime('now','localtime')),
    PRIMARY KEY(PlayerKey, ResourceKey)
);

INSERT OR REPLACE INTO ServerDB__GatheringResourceDefinition
(ResourceId,ResourceKey,Kind,RequiredProfession,RequiredLevel,MinimumYield,MaximumYield,Enabled)
SELECT ResourceId,ResourceKey,Kind,RequiredProfession,RequiredLevel,MinimumYield,MaximumYield,Enabled
FROM GatheringResourceDefinition;

INSERT OR REPLACE INTO ServerDB__GatheringNodeDefinition
(NodeId,ResourceId,Area,X,Y,Z,InteractionRadius,RespawnMilliseconds,Enabled)
SELECT NodeId,ResourceId,Area,X,Y,Z,InteractionRadius,RespawnMilliseconds,Enabled
FROM GatheringNodeDefinition;

INSERT OR REPLACE INTO ServerDB__GatheringPlayerInventory
(PlayerKey,ResourceKey,Quantity,UpdatedAt)
SELECT PlayerKey,ResourceKey,Quantity,UpdatedAt
FROM GatheringPlayerInventory;

-- Guarantee the starter catalog even if an incomplete database was marked v44.
INSERT OR IGNORE INTO ServerDB__GatheringResourceDefinition
(ResourceId,ResourceKey,Kind,RequiredProfession,RequiredLevel,MinimumYield,MaximumYield,Enabled)
VALUES
(1001,'iron_ore',0,4,0,1,2,1),
(1002,'copper_ore',0,4,0,1,3,1),
(1003,'mystic_crystal',0,4,0,1,1,1),
(2001,'herb_bundle',3,0,0,1,2,1),
(2002,'ancient_resin',3,0,0,1,1,1),
(3001,'timber',2,3,0,1,2,1);

DROP TABLE IF EXISTS GatheringPlayerInventory;
DROP TABLE IF EXISTS GatheringNodeDefinition;
DROP TABLE IF EXISTS GatheringResourceDefinition;

CREATE INDEX IF NOT EXISTS IX_ServerDB_GatheringNode_Area
    ON ServerDB__GatheringNodeDefinition(Area, Enabled);
