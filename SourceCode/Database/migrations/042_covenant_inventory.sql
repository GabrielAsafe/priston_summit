-- Covenant V1.1 virtual inventory.
-- Covenant-specific collectibles/resources live here until they are intentionally
-- bridged into legacy item IDs. They never control Covenant activation.

CREATE TABLE IF NOT EXISTS ServerDB__CovenantInventory (
    PlayerKey TEXT NOT NULL,
    CovenantId INTEGER NOT NULL CHECK(CovenantId > 0),
    ItemKey TEXT NOT NULL,
    Quantity INTEGER NOT NULL DEFAULT 0 CHECK(Quantity >= 0),
    UpdatedAt TEXT NOT NULL DEFAULT (datetime('now','localtime')),
    PRIMARY KEY(PlayerKey,CovenantId,ItemKey)
);

CREATE INDEX IF NOT EXISTS ServerDB__CovenantInventory_ByPlayer
    ON ServerDB__CovenantInventory(PlayerKey,CovenantId);

-- Upgrade/backfill: every existing membership owns its Covenant Sigil. The
-- collection is the permanent unlock record; the inventory row is the tangible
-- Covenant-side item. INSERT OR IGNORE keeps migration reruns idempotent.
INSERT OR IGNORE INTO ServerDB__CovenantCollection
    (PlayerKey,CovenantId,CollectibleKey,AcquiredAt)
SELECT PlayerKey,
       CovenantId,
       'sigil:' || CASE CovenantId
           WHEN 1 THEN 'hunters'
           WHEN 2 THEN 'gatherers'
           WHEN 3 THEN 'artisans'
           WHEN 4 THEN 'fortune'
           WHEN 5 THEN 'ascendants'
           ELSE 'covenant_' || CovenantId
       END,
       CAST(strftime('%s','now') AS INTEGER)
FROM ServerDB__CovenantMembership;

INSERT OR IGNORE INTO ServerDB__CovenantInventory
    (PlayerKey,CovenantId,ItemKey,Quantity)
SELECT PlayerKey,
       CovenantId,
       'sigil:' || CASE CovenantId
           WHEN 1 THEN 'hunters'
           WHEN 2 THEN 'gatherers'
           WHEN 3 THEN 'artisans'
           WHEN 4 THEN 'fortune'
           WHEN 5 THEN 'ascendants'
           ELSE 'covenant_' || CovenantId
       END,
       1
FROM ServerDB__CovenantMembership;
