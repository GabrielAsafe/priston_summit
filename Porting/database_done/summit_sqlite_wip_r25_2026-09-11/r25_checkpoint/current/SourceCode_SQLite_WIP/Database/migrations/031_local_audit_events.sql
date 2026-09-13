-- R22: local audit sinks for events that were historically written only
-- through Database.dll. These tables intentionally preserve the server-owned
-- event payload instead of reconstructing provider-specific commercial logic.

CREATE TABLE IF NOT EXISTS "GameLogDB__LocalCharacterEvent" (
    "ID" INTEGER PRIMARY KEY AUTOINCREMENT,
    "EventType" INTEGER NOT NULL,
    "UserID" TEXT NOT NULL,
    "CharName" TEXT NOT NULL,
    "CharType" INTEGER NOT NULL,
    "GPCode" TEXT NOT NULL,
    "IP" TEXT NOT NULL,
    "ServerName" TEXT NOT NULL,
    "GameLevel" INTEGER NOT NULL,
    "Exp" INTEGER NOT NULL,
    "GameMoney" INTEGER NOT NULL,
    "CreatedAt" TEXT NOT NULL
);
CREATE INDEX IF NOT EXISTS "GameLogDB__LocalCharacterEvent_UserID"
    ON "GameLogDB__LocalCharacterEvent" ("UserID", "CreatedAt");
CREATE INDEX IF NOT EXISTS "GameLogDB__LocalCharacterEvent_CharName"
    ON "GameLogDB__LocalCharacterEvent" ("CharName", "CreatedAt");

CREATE TABLE IF NOT EXISTS "ItemLogDB__LocalItemEvent" (
    "ID" INTEGER PRIMARY KEY AUTOINCREMENT,
    "EventType" INTEGER NOT NULL,
    "UserID" TEXT NOT NULL,
    "CharName" TEXT NOT NULL,
    "IP" INTEGER NOT NULL,
    "Flag" INTEGER NOT NULL,
    "TargetUserID" TEXT NOT NULL,
    "TargetCharName" TEXT NOT NULL,
    "TargetIP" INTEGER NOT NULL,
    "Money" INTEGER NOT NULL,
    "GiftNo" INTEGER NOT NULL,
    "Telephone" TEXT NOT NULL,
    "ItemCount" INTEGER NOT NULL,
    "CreatedAt" TEXT NOT NULL
);
CREATE INDEX IF NOT EXISTS "ItemLogDB__LocalItemEvent_UserID"
    ON "ItemLogDB__LocalItemEvent" ("UserID", "CreatedAt");
CREATE INDEX IF NOT EXISTS "ItemLogDB__LocalItemEvent_TargetUserID"
    ON "ItemLogDB__LocalItemEvent" ("TargetUserID", "CreatedAt");

CREATE TABLE IF NOT EXISTS "ItemLogDB__LocalItemEventEntry" (
    "EventID" INTEGER NOT NULL,
    "ItemIndex" INTEGER NOT NULL,
    "ItemCode" INTEGER NOT NULL,
    "ItemINo" INTEGER NOT NULL,
    "ItemINo1" INTEGER NOT NULL,
    PRIMARY KEY ("EventID", "ItemIndex"),
    FOREIGN KEY ("EventID") REFERENCES "ItemLogDB__LocalItemEvent"("ID") ON DELETE CASCADE
);
CREATE INDEX IF NOT EXISTS "ItemLogDB__LocalItemEventEntry_ItemCode"
    ON "ItemLogDB__LocalItemEventEntry" ("ItemCode");
