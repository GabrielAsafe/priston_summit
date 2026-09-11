-- R23: explicit ownership of account authentication and the legacy billing DLL.
-- Existing installations preserve compatibility because the runtime fallback is
-- seeded enabled. Account policy rows are opt-in and case-insensitive.

CREATE TABLE IF NOT EXISTS LocalAuth__AccountPolicy (
    UserID TEXT COLLATE NOCASE NOT NULL PRIMARY KEY,
    AuthMode TEXT COLLATE NOCASE NOT NULL
        CHECK (lower(AuthMode) IN ('local','legacy')),
    Enabled INTEGER NOT NULL DEFAULT 1
        CHECK (Enabled IN (0,1)),
    UpdatedAt TEXT NOT NULL DEFAULT (datetime('now','localtime'))
);

CREATE TABLE IF NOT EXISTS LocalAuth__RuntimePolicy (
    PolicyKey TEXT COLLATE NOCASE NOT NULL PRIMARY KEY,
    PolicyValue TEXT NOT NULL,
    UpdatedAt TEXT NOT NULL DEFAULT (datetime('now','localtime'))
);

INSERT OR IGNORE INTO LocalAuth__RuntimePolicy(PolicyKey, PolicyValue, UpdatedAt)
VALUES ('legacy_billing_fallback', '1', datetime('now','localtime'));
