-- Covenant V0.1 default content.
--
-- Product contract:
--   * a player may join/progress every Covenant;
--   * one Covenant is active at a time;
--   * the first joined Covenant activates automatically in runtime;
--   * Sigils/items are rewards/collection content and DO NOT activate effects;
--   * rank benefits are cumulative in content (Option A): every later rank
--     repeats all effects from earlier ranks with its current tuned values;
--   * world NPC/altar interaction is intentionally deferred; chat/UI call the
--     same server runtime.
--
-- AdditivePercent values use fractional scalars (0.10 = 10%).

INSERT OR IGNORE INTO ServerDB__ProgressionCovenantDefinition
    (CovenantId,Key,DisplayNameKey,ExclusiveMembership,HasReputationBounds,MinReputation,MaxReputation,Enabled)
VALUES
    (1,'hunters','covenant.hunters.name',0,1,0,100000,1),
    (2,'gatherers','covenant.gatherers.name',0,1,0,100000,1),
    (3,'artisans','covenant.artisans.name',0,1,0,100000,1),
    (4,'fortune','covenant.fortune.name',0,1,0,100000,1),
    (5,'ascendants','covenant.ascendants.name',0,1,0,100000,1);

-- All five are initially available. Later content can replace Always with
-- explicit level/story/quest requirements without changing the runtime API.
INSERT OR IGNORE INTO ServerDB__ProgressionCovenantJoinCondition
    (CovenantId,SortOrder,ConditionType,Key,Id,Value,AuxiliaryId)
VALUES
    (1,0,0,'',0,0,0),
    (2,0,0,'',0,0,0),
    (3,0,0,'',0,0,0),
    (4,0,0,'',0,0,0),
    (5,0,0,'',0,0,0);

INSERT OR IGNORE INTO ServerDB__ProgressionCovenantRank
    (CovenantId,RankId,Key,DisplayNameKey,MinReputation)
VALUES
    (1,1,'tracker','covenant.hunters.rank.tracker',0),
    (1,2,'trophy_refining','covenant.hunters.rank.trophy_refining',1000),
    (1,3,'hunters_mark','covenant.hunters.rank.hunters_mark',3000),

    (2,1,'surveyor','covenant.gatherers.rank.surveyor',0),
    (2,2,'prosperous_harvest','covenant.gatherers.rank.prosperous_harvest',1000),
    (2,3,'refined_materials','covenant.gatherers.rank.refined_materials',3000),

    (3,1,'reforge','covenant.artisans.rank.reforge',0),
    (3,2,'forgemaster','covenant.artisans.rank.forgemaster',1000),
    (3,3,'master_artisan','covenant.artisans.rank.master_artisan',3000),

    (4,1,'fortune','covenant.fortune.rank.fortune',0),
    (4,2,'fortune_roll','covenant.fortune.rank.fortune_roll',1000),
    (4,3,'tempt_fate','covenant.fortune.rank.tempt_fate',3000),

    (5,1,'disciple','covenant.ascendants.rank.disciple',0),
    (5,2,'training','covenant.ascendants.rank.training',1000),
    (5,3,'ascension','covenant.ascendants.rank.ascension',3000);

-- Hunters: loot specialization against Hunt targets; risk scales with reward.
INSERT OR IGNORE INTO ServerDB__ProgressionCovenantRankBenefit
    (CovenantId,RankId,SortOrder,StatKey,Operation,Value,ScopeKey)
VALUES
    (1,1,0,'hunt.drop_chance',1,0.10,'hunt_target'),
    (1,1,1,'combat.damage_taken',1,0.08,'hunt_target'),

    (1,2,0,'hunt.drop_chance',1,0.12,'hunt_target'),
    (1,2,1,'combat.damage_taken',1,0.10,'hunt_target'),
    (1,2,2,'hunt.trophy_refining_enabled',0,1.0,'hunt'),
    (1,2,3,'hunt.trophy_chance',1,0.05,'hunt_target'),

    (1,3,0,'hunt.drop_chance',1,0.15,'hunt_target'),
    (1,3,1,'combat.damage_taken',1,0.15,'hunt_target'),
    (1,3,2,'hunt.trophy_refining_enabled',0,1.0,'hunt'),
    (1,3,3,'hunt.trophy_chance',1,0.10,'hunt_target'),
    (1,3,4,'hunt.mark_enabled',0,1.0,'hunt');

-- Gatherers: navigation/yield/material specialization at the cost of monster loot.
INSERT OR IGNORE INTO ServerDB__ProgressionCovenantRankBenefit
    (CovenantId,RankId,SortOrder,StatKey,Operation,Value,ScopeKey)
VALUES
    (2,1,0,'gathering.surveyor_enabled',0,1.0,'gathering'),
    (2,1,1,'monster.drop_chance',1,-0.05,'monster'),

    (2,2,0,'gathering.surveyor_enabled',0,1.0,'gathering'),
    (2,2,1,'monster.drop_chance',1,-0.10,'monster'),
    (2,2,2,'gathering.extra_yield_chance',1,0.15,'gathering'),
    (2,2,3,'gathering.special_material_chance',1,0.05,'gathering'),

    (2,3,0,'gathering.surveyor_enabled',0,1.0,'gathering'),
    (2,3,1,'monster.drop_chance',1,-0.15,'monster'),
    (2,3,2,'gathering.extra_yield_chance',1,0.20,'gathering'),
    (2,3,3,'gathering.special_material_chance',1,0.10,'gathering'),
    (2,3,4,'gathering.refined_material_chance',1,0.05,'gathering');

-- Artisans: increasing crafting control; worse direct material acquisition.
INSERT OR IGNORE INTO ServerDB__ProgressionCovenantRankBenefit
    (CovenantId,RankId,SortOrder,StatKey,Operation,Value,ScopeKey)
VALUES
    (3,1,0,'crafting.reforge_enabled',0,1.0,'crafting'),
    (3,1,1,'gathering.yield',1,-0.10,'gathering'),
    (3,1,2,'hunt.trophy_chance',1,-0.10,'hunt_target'),

    (3,2,0,'crafting.reforge_enabled',0,1.0,'crafting'),
    (3,2,1,'gathering.yield',1,-0.12,'gathering'),
    (3,2,2,'hunt.trophy_chance',1,-0.12,'hunt_target'),
    (3,2,3,'crafting.forgemaster_enabled',0,1.0,'crafting'),
    (3,2,4,'vendor.blacksmith_sell_value',1,0.20,'blacksmith'),

    (3,3,0,'crafting.reforge_enabled',0,1.0,'crafting'),
    (3,3,1,'gathering.yield',1,-0.15,'gathering'),
    (3,3,2,'hunt.trophy_chance',1,-0.15,'hunt_target'),
    (3,3,3,'crafting.forgemaster_enabled',0,1.0,'crafting'),
    (3,3,4,'vendor.blacksmith_sell_value',1,0.20,'blacksmith'),
    (3,3,5,'crafting.master_artisan_enabled',0,1.0,'crafting');

-- Fortune: universal loot specialization with a matching incoming-damage risk.
INSERT OR IGNORE INTO ServerDB__ProgressionCovenantRankBenefit
    (CovenantId,RankId,SortOrder,StatKey,Operation,Value,ScopeKey)
VALUES
    (4,1,0,'monster.drop_chance',1,0.08,'monster'),
    (4,1,1,'combat.damage_taken',1,0.08,'global'),

    (4,2,0,'monster.drop_chance',1,0.12,'monster'),
    (4,2,1,'combat.damage_taken',1,0.12,'global'),
    (4,2,2,'loot.extra_roll_chance',1,0.05,'monster'),

    (4,3,0,'monster.drop_chance',1,0.15,'monster'),
    (4,3,1,'combat.damage_taken',1,0.15,'global'),
    (4,3,2,'loot.extra_roll_chance',1,0.08,'monster'),
    (4,3,3,'fortune.tempt_fate_enabled',0,1.0,'fortune');

-- Ascendants: XP/progression specialization paid for with reduced monster loot.
INSERT OR IGNORE INTO ServerDB__ProgressionCovenantRankBenefit
    (CovenantId,RankId,SortOrder,StatKey,Operation,Value,ScopeKey)
VALUES
    (5,1,0,'xp.monster_gain',1,0.10,'monster'),
    (5,1,1,'monster.drop_chance',1,-0.10,'monster'),

    (5,2,0,'xp.monster_gain',1,0.15,'monster'),
    (5,2,1,'monster.drop_chance',1,-0.15,'monster'),

    (5,3,0,'xp.monster_gain',1,0.20,'monster'),
    (5,3,1,'monster.drop_chance',1,-0.20,'monster'),
    (5,3,2,'ascendant.focused_training_enabled',0,1.0,'ascendant');
