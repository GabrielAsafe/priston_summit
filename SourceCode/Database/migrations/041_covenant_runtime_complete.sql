-- Covenant V1 runtime-complete state/content.
-- No world/NPC dependency: chat/UI/world surfaces consume the same server state.

CREATE TABLE IF NOT EXISTS ServerDB__CovenantPlayerState (
    PlayerKey TEXT NOT NULL PRIMARY KEY,
    SwitchCooldownUntil INTEGER NOT NULL DEFAULT 0,
    UpdatedAt TEXT NOT NULL DEFAULT (datetime('now','localtime'))
);

CREATE TABLE IF NOT EXISTS ServerDB__CovenantMastery (
    PlayerKey TEXT NOT NULL,
    CovenantId INTEGER NOT NULL CHECK(CovenantId > 0),
    MasteredAt INTEGER NOT NULL,
    PRIMARY KEY(PlayerKey,CovenantId)
);

CREATE TABLE IF NOT EXISTS ServerDB__CovenantCollection (
    PlayerKey TEXT NOT NULL,
    CovenantId INTEGER NOT NULL CHECK(CovenantId > 0),
    CollectibleKey TEXT NOT NULL,
    AcquiredAt INTEGER NOT NULL,
    PRIMARY KEY(PlayerKey,CovenantId,CollectibleKey)
);

CREATE TABLE IF NOT EXISTS ServerDB__CovenantActivityDefinition (
    CovenantId INTEGER NOT NULL,
    ActivityKey TEXT NOT NULL,
    ReputationPerUnit INTEGER NOT NULL CHECK(ReputationPerUnit >= 0),
    Enabled INTEGER NOT NULL DEFAULT 1 CHECK(Enabled IN (0,1)),
    PRIMARY KEY(CovenantId,ActivityKey)
);

CREATE TABLE IF NOT EXISTS ServerDB__CovenantChallengeDefinition (
    CovenantId INTEGER NOT NULL,
    ChallengeKey TEXT NOT NULL,
    ActivityKey TEXT NOT NULL,
    TargetCount INTEGER NOT NULL CHECK(TargetCount > 0),
    ReputationReward INTEGER NOT NULL DEFAULT 0 CHECK(ReputationReward >= 0),
    SortOrder INTEGER NOT NULL DEFAULT 0,
    Enabled INTEGER NOT NULL DEFAULT 1 CHECK(Enabled IN (0,1)),
    PRIMARY KEY(CovenantId,ChallengeKey)
);

CREATE TABLE IF NOT EXISTS ServerDB__CovenantChallengeProgress (
    PlayerKey TEXT NOT NULL,
    CovenantId INTEGER NOT NULL,
    ChallengeKey TEXT NOT NULL,
    Progress INTEGER NOT NULL DEFAULT 0 CHECK(Progress >= 0),
    Completed INTEGER NOT NULL DEFAULT 0 CHECK(Completed IN (0,1)),
    RewardClaimed INTEGER NOT NULL DEFAULT 0 CHECK(RewardClaimed IN (0,1)),
    UpdatedAt TEXT NOT NULL DEFAULT (datetime('now','localtime')),
    PRIMARY KEY(PlayerKey,CovenantId,ChallengeKey)
);

CREATE TABLE IF NOT EXISTS ServerDB__CovenantPowerDefinition (
    CovenantId INTEGER NOT NULL,
    PowerKey TEXT NOT NULL,
    RequiredRankId INTEGER NOT NULL CHECK(RequiredRankId > 0),
    DurationSeconds INTEGER NOT NULL CHECK(DurationSeconds > 0),
    CooldownSeconds INTEGER NOT NULL CHECK(CooldownSeconds >= DurationSeconds),
    TargetMode INTEGER NOT NULL DEFAULT 0 CHECK(TargetMode IN (0,1)), -- 0=self, 1=hunt target
    Enabled INTEGER NOT NULL DEFAULT 1 CHECK(Enabled IN (0,1)),
    PRIMARY KEY(CovenantId,PowerKey)
);

CREATE TABLE IF NOT EXISTS ServerDB__CovenantPowerModifier (
    CovenantId INTEGER NOT NULL,
    PowerKey TEXT NOT NULL,
    SortOrder INTEGER NOT NULL,
    StatKey TEXT NOT NULL,
    Operation INTEGER NOT NULL,
    Value REAL NOT NULL,
    ScopeKey TEXT NOT NULL DEFAULT '',
    PRIMARY KEY(CovenantId,PowerKey,SortOrder)
);

CREATE TABLE IF NOT EXISTS ServerDB__CovenantPowerState (
    PlayerKey TEXT NOT NULL,
    CovenantId INTEGER NOT NULL,
    PowerKey TEXT NOT NULL,
    ActiveUntil INTEGER NOT NULL DEFAULT 0,
    CooldownUntil INTEGER NOT NULL DEFAULT 0,
    TargetSerial INTEGER NOT NULL DEFAULT 0,
    UpdatedAt TEXT NOT NULL DEFAULT (datetime('now','localtime')),
    PRIMARY KEY(PlayerKey,CovenantId,PowerKey)
);

-- Repeatable activity reputation intentionally has no per-action receipt ledger.
-- Challenge completion rewards remain idempotent via ChallengeProgress.RewardClaimed.
INSERT OR REPLACE INTO ServerDB__CovenantActivityDefinition VALUES
 (1,'hunt.kill',10,1),
 (2,'gathering.harvest',2,1),
 (3,'crafting.success',10,1),
 (4,'monster.kill',1,1),
 (5,'monster.kill',1,1);

INSERT OR REPLACE INTO ServerDB__CovenantChallengeDefinition VALUES
 (1,'hunter_trial_i','hunt.kill',5,100,10,1),
 (1,'hunter_trial_ii','hunt.kill',25,300,20,1),
 (1,'hunter_trial_iii','hunt.kill',100,600,30,1),
 (2,'gatherer_trial_i','gathering.harvest',10,100,10,1),
 (2,'gatherer_trial_ii','gathering.harvest',50,300,20,1),
 (2,'gatherer_trial_iii','gathering.harvest',200,600,30,1),
 (3,'artisan_trial_i','crafting.success',5,100,10,1),
 (3,'artisan_trial_ii','crafting.success',25,300,20,1),
 (3,'artisan_trial_iii','crafting.success',100,600,30,1),
 (4,'fortune_trial_i','monster.kill',25,100,10,1),
 (4,'fortune_trial_ii','monster.kill',100,300,20,1),
 (4,'fortune_trial_iii','monster.kill',500,600,30,1),
 (5,'ascendant_trial_i','monster.kill',25,100,10,1),
 (5,'ascendant_trial_ii','monster.kill',100,300,20,1),
 (5,'ascendant_trial_iii','monster.kill',500,600,30,1);

-- Tier III powers. Balance lives here, not in gameplay callsites.
INSERT OR REPLACE INTO ServerDB__CovenantPowerDefinition VALUES
 (1,'hunters_mark',3,120,600,1,1),
 (4,'tempt_fate',3,60,600,0,1),
 (5,'focused_training',3,1800,3600,0,1);

INSERT OR REPLACE INTO ServerDB__CovenantPowerModifier VALUES
 (1,'hunters_mark',0,'hunt.drop_chance',1,0.15,'hunt_target'),
 (1,'hunters_mark',1,'hunt.trophy_chance',1,0.10,'hunt_target'),
 (1,'hunters_mark',2,'combat.damage_taken',1,0.15,'hunt_target'),
 (4,'tempt_fate',0,'monster.drop_chance',1,0.30,'monster'),
 (4,'tempt_fate',1,'loot.extra_roll_chance',1,0.15,'monster'),
 (4,'tempt_fate',2,'combat.damage_taken',1,0.25,'global'),
 (5,'focused_training',0,'xp.monster_gain',1,0.25,'monster'),
 (5,'focused_training',1,'monster.drop_chance',1,-0.50,'monster');
