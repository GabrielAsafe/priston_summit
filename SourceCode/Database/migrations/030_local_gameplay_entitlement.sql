-- R21: server-local gameplay entitlements.
--
-- This table deliberately does not reuse AccountDB GameUser billing columns.
-- The historical PC-room provider remains compatible through Bl_RNo, while
-- local servers can grant the same gameplay benefit without manufacturing a
-- billing/reservation identity.
CREATE TABLE IF NOT EXISTS "LocalAuth__GameplayEntitlement"
(
    "UserID" TEXT NOT NULL COLLATE NOCASE,
    "Entitlement" TEXT NOT NULL COLLATE NOCASE,
    "Enabled" INTEGER NOT NULL DEFAULT 1 CHECK ("Enabled" IN (0, 1)),
    "ExpiresAt" TEXT NULL,
    "UpdatedAt" TEXT NOT NULL DEFAULT (datetime('now','localtime')),
    PRIMARY KEY ("UserID", "Entitlement")
);

CREATE INDEX IF NOT EXISTS "LocalAuth__GameplayEntitlement_Enabled"
    ON "LocalAuth__GameplayEntitlement" ("Entitlement", "Enabled", "ExpiresAt");
