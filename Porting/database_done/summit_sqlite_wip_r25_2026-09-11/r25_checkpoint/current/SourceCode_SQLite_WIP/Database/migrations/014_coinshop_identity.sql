-- Summit SQLite migration 014: CoinShop logical account identity.
-- The supplied CoinShopDB backup proves UserInfo(UserID, UserCoin, LastChange)
-- and UserInfo_T(UserID, UserMinutos, LastChange). Active callers treat each
-- UserID as a single logical balance row. Enforce that invariant before other
-- legacy features (notably JOBS market) share the same local balance table.

CREATE UNIQUE INDEX IF NOT EXISTS "CoinShopDB__UserInfo_UserID_UQ"
    ON "CoinShopDB__UserInfo" ("UserID");

CREATE UNIQUE INDEX IF NOT EXISTS "CoinShopDB__UserInfo_T_UserID_UQ"
    ON "CoinShopDB__UserInfo_T" ("UserID");
