-- Gathering/Mining V2 runtime content + persistent resource inventory.
-- Nodes are intentionally data-owned: production coordinates can be inserted
-- without recompiling the server or changing the harvest contract.

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

CREATE INDEX IF NOT EXISTS IX_GatheringNode_Area
    ON GatheringNodeDefinition(Area, Enabled);

CREATE TABLE IF NOT EXISTS GatheringPlayerInventory (
    PlayerKey TEXT NOT NULL,
    ResourceKey TEXT NOT NULL,
    Quantity INTEGER NOT NULL DEFAULT 0,
    UpdatedAt TEXT NOT NULL DEFAULT (datetime('now','localtime')),
    PRIMARY KEY(PlayerKey, ResourceKey)
);

-- Starter definitions only. World nodes are not guessed here; admins/content
-- can place them at authoritative map coordinates with the runtime command.
INSERT OR IGNORE INTO GatheringResourceDefinition
(ResourceId,ResourceKey,Kind,RequiredProfession,RequiredLevel,MinimumYield,MaximumYield,Enabled)
VALUES
(1001,'iron_ore',0,4,0,1,2,1),
(1002,'copper_ore',0,4,0,1,3,1),
(1003,'mystic_crystal',0,4,0,1,1,1),
(2001,'herb_bundle',3,0,0,1,2,1),
(2002,'ancient_resin',3,0,0,1,1,1),
(3001,'timber',2,3,0,1,2,1);
