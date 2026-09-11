-- Summit SQLite migration 002: CoinShopDB tables proven by active source contracts.
-- Column order is part of the legacy SELECT * API and must not be changed.

CREATE TABLE IF NOT EXISTS "CoinShopDB__ItemList"
(
    "szItemCode" TEXT NOT NULL,
    "ItemSection" INTEGER NOT NULL,
    "Price" INTEGER NOT NULL
);

CREATE TABLE IF NOT EXISTS "CoinShopDB__UserInfo"
(
    "UserID" TEXT NOT NULL,
    "UserCoin" INTEGER NOT NULL,
    "LastChange" TEXT NULL
);

CREATE TABLE IF NOT EXISTS "CoinShopDB__ItemList_T"
(
    "szItemCode" TEXT NOT NULL,
    "ItemSection" INTEGER NOT NULL,
    "Price" INTEGER NOT NULL
);

CREATE TABLE IF NOT EXISTS "CoinShopDB__UserInfo_T"
(
    "UserID" TEXT NOT NULL,
    "UserMinutos" INTEGER NOT NULL,
    "LastChange" TEXT NULL
);
