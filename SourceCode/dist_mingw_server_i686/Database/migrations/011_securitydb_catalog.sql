-- Summit SQLite migration 011: SecurityDB tables recovered from the supplied
-- SecurityDB202405120913.bak catalog. This is schema only; production rows are
-- intentionally not embedded in migrations.

CREATE TABLE IF NOT EXISTS "SecurityDB__CheatLog"
(
    "ID" INTEGER PRIMARY KEY AUTOINCREMENT,
    "IP" TEXT NULL,
    "AccountName" TEXT NULL,
    "Action" INTEGER NULL,
    "LogID" INTEGER NULL,
    "Description" TEXT NULL,
    "Date" TEXT NULL
);

CREATE TABLE IF NOT EXISTS "SecurityDB__FunctionChecksumList"
(
    "ID" INTEGER PRIMARY KEY AUTOINCREMENT,
    "Size" INTEGER NULL,
    "Address" TEXT NULL,
    "Checksum" TEXT NULL
);

CREATE TABLE IF NOT EXISTS "SecurityDB__WindowCheatList"
(
    "ID" INTEGER PRIMARY KEY AUTOINCREMENT,
    "WindowName" TEXT NULL
);
CREATE INDEX IF NOT EXISTS "SecurityDB__WindowCheatList_ID"
    ON "SecurityDB__WindowCheatList" ("ID" DESC);
