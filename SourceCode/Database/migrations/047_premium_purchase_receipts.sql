-- Durable idempotency receipts for Coin Shop and Time Shop purchase requests.
-- Scope: 0 = CoinShop, 1 = TimeShop. State: 0 = reserved/in-flight, 1 = committed.
CREATE TABLE IF NOT EXISTS "CoinShopDB__PremiumPurchaseReceipt" (
    "PurchaserKey" TEXT NOT NULL,
    "Scope" INTEGER NOT NULL CHECK ("Scope" IN (0,1)),
    "RequestId" INTEGER NOT NULL CHECK ("RequestId" > 0),
    "ItemCode" TEXT NOT NULL,
    "State" INTEGER NOT NULL CHECK ("State" IN (0,1)),
    "CreatedAt" TEXT NOT NULL,
    "UpdatedAt" TEXT NOT NULL,
    PRIMARY KEY ("PurchaserKey", "Scope", "RequestId")
);
CREATE INDEX IF NOT EXISTS "CoinShopDB__PremiumPurchaseReceipt_State"
    ON "CoinShopDB__PremiumPurchaseReceipt" ("State", "UpdatedAt");
