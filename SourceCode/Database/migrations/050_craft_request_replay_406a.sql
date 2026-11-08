-- SUMMIT 406A. Adds only a replay ledger for SQLite MATERIAL crafts.
-- No DAT or player items are touched. Apply with server stopped and backup.
CREATE TABLE IF NOT EXISTS ServerDB__ProfessionCraftRequest406 (
 PlayerKey TEXT NOT NULL CHECK(length(PlayerKey) BETWEEN 1 AND 255),
 RequestKey TEXT NOT NULL CHECK(length(RequestKey) BETWEEN 1 AND 48),
 RecipeKey TEXT NOT NULL CHECK(length(RecipeKey) BETWEEN 3 AND 96),
 CreatedAt TEXT NOT NULL DEFAULT (datetime('now')),
 PRIMARY KEY(PlayerKey,RequestKey)
);
