-- Covenant Artisan chat-first crafting mode.
-- Mode: 0=off, 1=reforge, 2=forgemaster, 3=master artisan.
-- The selected mode is only honored while Artisans is the active Covenant and
-- the corresponding cumulative rank capability is unlocked.
CREATE TABLE IF NOT EXISTS ServerDB__CovenantArtisanState (
    PlayerKey TEXT NOT NULL PRIMARY KEY,
    Mode INTEGER NOT NULL DEFAULT 0 CHECK(Mode BETWEEN 0 AND 3),
    UpdatedAt TEXT NOT NULL DEFAULT (datetime('now','localtime'))
);
