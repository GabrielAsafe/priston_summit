-- TAKEOFF Covenant durable rewards and effect receipts.
-- Data-empty by default. RankId=0 means a one-shot join reward; RankId>0 means rank-entry milestone.

CREATE TABLE IF NOT EXISTS ServerDB__CovenantEffectReceipt (
    PlayerKey TEXT NOT NULL CHECK(length(PlayerKey) BETWEEN 1 AND 255),
    CovenantId INTEGER NOT NULL CHECK(CovenantId > 0),
    EffectKey TEXT NOT NULL CHECK(length(EffectKey) BETWEEN 1 AND 255),
    Applied INTEGER NOT NULL DEFAULT 0 CHECK(Applied IN (0,1)),
    ClaimedAt TEXT NOT NULL DEFAULT (datetime('now','localtime')),
    UpdatedAt TEXT NOT NULL DEFAULT (datetime('now','localtime')),
    PRIMARY KEY(PlayerKey, CovenantId, EffectKey)
);

CREATE INDEX IF NOT EXISTS ServerDB__CovenantEffectReceipt_Pending
    ON ServerDB__CovenantEffectReceipt(PlayerKey, Applied, CovenantId, EffectKey);

CREATE TABLE IF NOT EXISTS ServerDB__ProgressionCovenantReward (
    CovenantId INTEGER NOT NULL CHECK(CovenantId > 0),
    RewardKey TEXT NOT NULL CHECK(length(RewardKey) BETWEEN 1 AND 200),
    RankId INTEGER NOT NULL DEFAULT 0 CHECK(RankId >= 0),
    SortOrder INTEGER NOT NULL CHECK(SortOrder >= 0),
    RewardType INTEGER NOT NULL CHECK(RewardType BETWEEN 0 AND 6 AND RewardType <> 3),
    Key TEXT NOT NULL DEFAULT '' CHECK(length(Key) <= 255),
    Id INTEGER NOT NULL DEFAULT 0,
    Amount INTEGER NOT NULL DEFAULT 0,
    PRIMARY KEY(CovenantId, RewardKey),
    UNIQUE(CovenantId, RankId, SortOrder),
    CHECK((RewardType = 0 AND length(Key) BETWEEN 1 AND 255 AND Amount = 1) OR
          (RewardType IN (1,2) AND Amount > 0) OR
          (RewardType = 4 AND length(Key) BETWEEN 1 AND 255) OR
          (RewardType = 5 AND Id > 0 AND Amount <> 0) OR
          (RewardType = 6 AND length(Key) BETWEEN 1 AND 255))
);

CREATE INDEX IF NOT EXISTS ServerDB__ProgressionCovenantReward_ByMilestone
    ON ServerDB__ProgressionCovenantReward(CovenantId, RankId, SortOrder);
