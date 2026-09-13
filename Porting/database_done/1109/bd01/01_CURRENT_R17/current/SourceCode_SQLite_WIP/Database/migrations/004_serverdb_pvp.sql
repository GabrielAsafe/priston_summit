-- Summit SQLite migration 004: ServerDB PVP table.
-- The legacy caller guarded INSERT with NOT EXISTS(Name=?). The UNIQUE(Name)
-- constraint is required for the WIP INSERT OR IGNORE form to preserve that rule.

CREATE TABLE IF NOT EXISTS "ServerDB__PVP"
(
    "ID" TEXT NOT NULL,
    "Name" TEXT NOT NULL,
    "Class" INTEGER NOT NULL,
    "Lvl" INTEGER NOT NULL,
    "Kills" INTEGER NOT NULL,
    "Deads" INTEGER NOT NULL,
    UNIQUE ("Name")
);
