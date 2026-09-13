-- Summit SQLite migration 012: StartUserDB schema recovered from the supplied
-- StartUserDB202405120913.bak catalog. The backup contains exactly Level + Items;
-- it contains no ExtraItems table, so ExtraItems is deliberately not invented.

CREATE TABLE IF NOT EXISTS "StartUserDB__Level"
(
    "Level" INTEGER NOT NULL
);

-- Exact 16-column layout from the backup. The active legacy structure has three
-- additional Shield/Aging6/Mix6 fields; the caller now projects explicit fallback
-- values for those absent legacy columns instead of reading past SELECT *.
CREATE TABLE IF NOT EXISTS "StartUserDB__Items"
(
    "Classe" INTEGER PRIMARY KEY AUTOINCREMENT,
    "Weapon" TEXT NOT NULL,
    "Aging1" INTEGER NOT NULL,
    "Mix1" INTEGER NOT NULL,
    "Armor" TEXT NOT NULL,
    "Aging2" INTEGER NOT NULL,
    "Mix2" INTEGER NOT NULL,
    "Boots" TEXT NOT NULL,
    "Aging3" INTEGER NOT NULL,
    "Mix3" INTEGER NOT NULL,
    "Gloves" TEXT NOT NULL,
    "Aging4" INTEGER NOT NULL,
    "Mix4" INTEGER NOT NULL,
    "Bracelet" TEXT NOT NULL,
    "Aging5" INTEGER NOT NULL,
    "Mix5" INTEGER NOT NULL
);
