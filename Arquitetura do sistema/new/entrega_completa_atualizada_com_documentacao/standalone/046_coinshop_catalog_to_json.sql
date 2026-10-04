-- Summit SQLite migration 046: move CoinShop static item catalog to canonical JSON.
-- The canonical catalog lives at Content/canonical/commerce/coinshop.json.
-- Mutable player balances, payments and transaction history intentionally remain in DB.
DROP TABLE IF EXISTS "CoinShopDB__ItemList";
DROP TABLE IF EXISTS "CoinShopDB__ItemList_T";
