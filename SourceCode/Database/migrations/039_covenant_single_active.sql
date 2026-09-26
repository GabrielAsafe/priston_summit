-- Covenant V0.1 lifecycle hardening.
-- Membership/progress is durable for every joined Covenant, but exactly one
-- specialization may be active for a player at a time.
--
-- Repair pre-V0.1 databases before adding the unique invariant. If an older
-- build managed to leave more than one membership active, keep the most
-- recently inserted membership (highest rowid) active and preserve every
-- membership's rank/reputation. No progress row is deleted.

UPDATE "ServerDB__CovenantMembership"
SET "Active" = 0,
    "UpdatedAt" = datetime('now','localtime')
WHERE "Active" = 1
  AND rowid NOT IN
  (
      SELECT MAX(rowid)
      FROM "ServerDB__CovenantMembership"
      WHERE "Active" = 1
      GROUP BY "PlayerKey"
  );

CREATE UNIQUE INDEX IF NOT EXISTS "ServerDB__CovenantMembership_OneActive"
    ON "ServerDB__CovenantMembership" ("PlayerKey")
    WHERE "Active" = 1;
