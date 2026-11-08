-- SUMMIT 372-378. SQLite-only materials. Physical DAT grant always disabled.
CREATE TABLE IF NOT EXISTS ServerDB__ProfessionRecipe378 (
 RecipeKey TEXT PRIMARY KEY CHECK(length(RecipeKey) BETWEEN 3 AND 96),
 ProfessionType INTEGER NOT NULL CHECK(ProfessionType IN (1,2,5)),
 MinLevel INTEGER NOT NULL CHECK(MinLevel BETWEEN 0 AND 50),
 OutputKey TEXT NOT NULL, OutputCount INTEGER NOT NULL CHECK(OutputCount BETWEEN 1 AND 100),
 Physical INTEGER NOT NULL CHECK(Physical IN (0,1)),
 LegacyItemCode TEXT NOT NULL DEFAULT '',
 XP INTEGER NOT NULL DEFAULT 0 CHECK(XP BETWEEN 0 AND 1000),
 Enabled INTEGER NOT NULL DEFAULT 0 CHECK(Enabled IN (0,1)),
 SourceVersion TEXT NOT NULL
);
CREATE TABLE IF NOT EXISTS ServerDB__ProfessionIngredient378 (
 RecipeKey TEXT NOT NULL, ResourceKey TEXT NOT NULL,
 Amount INTEGER NOT NULL CHECK(Amount BETWEEN 1 AND 10000),
 PRIMARY KEY(RecipeKey,ResourceKey),
 FOREIGN KEY(RecipeKey) REFERENCES ServerDB__ProfessionRecipe378(RecipeKey)
);
CREATE TABLE IF NOT EXISTS ServerDB__ProfessionCraftReceipt378 (
 ReceiptId INTEGER PRIMARY KEY AUTOINCREMENT,
 PlayerKey TEXT NOT NULL, RecipeKey TEXT NOT NULL, ResultKey TEXT NOT NULL,
 ResultAmount INTEGER NOT NULL, State TEXT NOT NULL CHECK(State='SQLITE_COMMITTED'),
 CreatedAt TEXT NOT NULL DEFAULT (datetime('now'))
);
CREATE TABLE IF NOT EXISTS ServerDB__PhysicalGrantIntent378 (
 IntentId TEXT PRIMARY KEY,
 PlayerKey TEXT NOT NULL, RecipeKey TEXT NOT NULL, ItemCode TEXT NOT NULL,
 State TEXT NOT NULL DEFAULT 'BLOCKED_NO_DAT_ADAPTER' CHECK(State='BLOCKED_NO_DAT_ADAPTER'),
 CreatedAt TEXT NOT NULL DEFAULT (datetime('now'))
);
CREATE TABLE IF NOT EXISTS ServerDB__GatheringBonusDrop378 (
 PrimaryResourceKey TEXT NOT NULL, ExtraResourceKey TEXT NOT NULL,
 ChancePer10000 INTEGER NOT NULL CHECK(ChancePer10000 BETWEEN 0 AND 10000),
 MinAmount INTEGER NOT NULL CHECK(MinAmount BETWEEN 1 AND 100),
 MaxAmount INTEGER NOT NULL CHECK(MaxAmount BETWEEN MinAmount AND 100),
 Enabled INTEGER NOT NULL DEFAULT 0 CHECK(Enabled IN (0,1)),
 PRIMARY KEY(PrimaryResourceKey,ExtraResourceKey)
);
CREATE TABLE IF NOT EXISTS ServerDB__GatheringXP378 (
 ResourceKey TEXT PRIMARY KEY, XP INTEGER NOT NULL CHECK(XP BETWEEN 0 AND 1000),
 Enabled INTEGER NOT NULL DEFAULT 1 CHECK(Enabled IN (0,1))
);
CREATE TABLE IF NOT EXISTS ServerDB__HunterDropRule378 (
 MonsterCode INTEGER NOT NULL, ResourceKey TEXT NOT NULL,
 ChancePer10000 INTEGER NOT NULL CHECK(ChancePer10000 BETWEEN 0 AND 10000),
 MinHunterLevel INTEGER NOT NULL CHECK(MinHunterLevel BETWEEN 0 AND 50),
 Enabled INTEGER NOT NULL DEFAULT 0 CHECK(Enabled IN (0,1)),
 PRIMARY KEY(MonsterCode,ResourceKey)
);
CREATE TABLE IF NOT EXISTS ServerDB__HunterKillReceipt378 (
 PlayerKey TEXT NOT NULL, MonsterCode INTEGER NOT NULL, MonsterSerial INTEGER NOT NULL,
 ResourceKey TEXT NOT NULL, CreatedAt TEXT NOT NULL DEFAULT (datetime('now')),
 PRIMARY KEY(PlayerKey,MonsterSerial,ResourceKey)
);

INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('artisan.refine_core_crystalline',5,0,'core_crystalline',1,0,'',4,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.refine_core_crystalline','raw_ore_crystalline',2);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('artisan.refine_core_energetic',5,5,'core_energetic',1,0,'',6,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.refine_core_energetic','raw_ore_energetic',2);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.refine_core_energetic','charcoal',1);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('artisan.refine_core_deep',5,10,'core_deep',1,0,'',8,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.refine_core_deep','raw_ore_deep',2);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.refine_core_deep','purified_resin',1);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('artisan.refine_core_arcane',5,15,'core_arcane',1,0,'',10,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.refine_core_arcane','raw_ore_arcane',2);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.refine_core_arcane','crystalline_sap',1);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('artisan.refine_core_runic',5,20,'core_runic',1,0,'',12,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.refine_core_runic','raw_ore_runic',2);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.refine_core_runic','arcane_charcoal',1);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('artisan.refine_core_primordial',5,25,'core_primordial',1,0,'',14,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.refine_core_primordial','raw_ore_primordial',2);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.refine_core_primordial','arcane_ash',1);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('artisan.process_silver_dust',5,0,'silver_dust',1,0,'',4,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.process_silver_dust','ore_silver',1);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('artisan.process_ground_amethyst',5,0,'ground_amethyst',1,0,'',4,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.process_ground_amethyst','ore_amethyst',1);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('artisan.process_gold_dust',5,0,'gold_dust',1,0,'',4,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.process_gold_dust','ore_gold',1);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('artisan.process_topaz_dust',5,0,'topaz_dust',1,0,'',4,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.process_topaz_dust','ore_topaz',1);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('artisan.process_ground_sapphire',5,0,'ground_sapphire',1,0,'',4,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.process_ground_sapphire','ore_sapphire',1);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('artisan.process_ground_citrine',5,0,'ground_citrine',1,0,'',4,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.process_ground_citrine','ore_citrine',1);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('artisan.process_ground_ruby',5,0,'ground_ruby',1,0,'',4,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.process_ground_ruby','ore_ruby',1);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('artisan.process_ground_emerald',5,0,'ground_emerald',1,0,'',4,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.process_ground_emerald','ore_emerald',1);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('artisan.make_planks',5,0,'planks',1,0,'',4,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.make_planks','common_logs',2);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('artisan.make_resistant_planks',5,5,'resistant_planks',1,0,'',6,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.make_resistant_planks','resistant_logs',2);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('artisan.make_charcoal',5,0,'charcoal',1,0,'',4,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.make_charcoal','common_logs',2);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('artisan.make_treated_planks',5,5,'treated_planks',1,0,'',6,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.make_treated_planks','planks',3);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.make_treated_planks','dense_resin',1);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('artisan.make_reinforced_handle',5,5,'reinforced_handle',1,0,'',6,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.make_reinforced_handle','resistant_planks',2);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.make_reinforced_handle','iron_ingot',1);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('artisan.make_purified_resin',5,5,'purified_resin',1,0,'',6,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.make_purified_resin','sap',3);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.make_purified_resin','charcoal',1);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('artisan.make_arcane_charcoal',5,15,'arcane_charcoal',1,0,'',10,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.make_arcane_charcoal','arcane_wood',2);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.make_arcane_charcoal','arcane_essence',1);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('artisan.make_crystalline_sap',5,10,'crystalline_sap',1,0,'',8,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.make_crystalline_sap','energetic_sap',2);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.make_crystalline_sap','purified_resin',1);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('artisan.make_dark_resin',5,10,'dark_resin',1,0,'',8,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.make_dark_resin','dense_resin',1);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.make_dark_resin','dark_essence',1);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('artisan.make_arcane_ash',5,15,'arcane_ash',1,0,'',10,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.make_arcane_ash','arcane_wood',1);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.make_arcane_ash','charcoal',1);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('artisan.make_primordial_essence',5,25,'primordial_essence',1,0,'',14,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.make_primordial_essence','tree_heart',1);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.make_primordial_essence','core_primordial',1);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('artisan.make_iron_ingot',5,0,'iron_ingot',1,0,'',4,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.make_iron_ingot','raw_ore_crystalline',2);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.make_iron_ingot','charcoal',1);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('artisan.make_arcane_essence',5,15,'arcane_essence',1,0,'',10,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.make_arcane_essence','mystic_resin',2);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.make_arcane_essence','crystalline_sap',1);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('artisan.make_purified_gold',5,20,'purified_gold',1,0,'',12,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.make_purified_gold','gold_dust',2);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.make_purified_gold','charcoal',1);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('artisan.make_scroll_blank',5,5,'scroll_blank',1,0,'',6,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.make_scroll_blank','treated_planks',1);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.make_scroll_blank','sap',1);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('artisan.make_scroll_enhancement',5,15,'scroll_enhancement',1,0,'',10,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.make_scroll_enhancement','scroll_blank',1);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.make_scroll_enhancement','arcane_ash',1);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.make_scroll_enhancement','ground_citrine',1);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('artisan.make_mystic_solvent',5,10,'mystic_solvent',1,0,'',8,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.make_mystic_solvent','purified_resin',1);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.make_mystic_solvent','crystalline_sap',1);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('artisan.make_machine_treated',5,10,'machine_treated',1,0,'',8,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.make_machine_treated','treated_planks',3);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.make_machine_treated','iron_ingot',2);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('artisan.make_tool_reinforced',5,10,'tool_reinforced',1,0,'',8,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.make_tool_reinforced','reinforced_handle',1);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.make_tool_reinforced','iron_ingot',2);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('artisan.sheltom_lucidi',5,0,'sheltom_lucidi',1,1,'',4,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.sheltom_lucidi','core_crystalline',1);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('artisan.sheltom_sereneo',5,0,'sheltom_sereneo',1,1,'OS102',4,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.sheltom_sereneo','core_crystalline',1);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.sheltom_sereneo','silver_dust',1);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('artisan.sheltom_fadeo',5,0,'sheltom_fadeo',1,1,'OS103',4,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.sheltom_fadeo','core_crystalline',1);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.sheltom_fadeo','ground_amethyst',1);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('artisan.sheltom_sparky',5,5,'sheltom_sparky',1,1,'OS104',6,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.sheltom_sparky','core_energetic',1);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.sheltom_sparky','sheltom_lucidi',1);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('artisan.sheltom_raident',5,5,'sheltom_raident',1,1,'OS105',6,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.sheltom_raident','core_energetic',1);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.sheltom_raident','sheltom_sereneo',1);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.sheltom_raident','gold_dust',1);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('artisan.sheltom_transparo',5,5,'sheltom_transparo',1,1,'OS106',6,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.sheltom_transparo','core_energetic',1);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.sheltom_transparo','sheltom_fadeo',1);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.sheltom_transparo','topaz_dust',1);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('artisan.sheltom_murky',5,10,'sheltom_murky',1,1,'OS107',8,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.sheltom_murky','core_deep',1);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.sheltom_murky','sheltom_transparo',1);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.sheltom_murky','dark_essence',1);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('artisan.sheltom_devine',5,10,'sheltom_devine',1,1,'OS108',8,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.sheltom_devine','core_deep',1);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.sheltom_devine','sheltom_raident',1);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.sheltom_devine','purified_salt',1);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('artisan.sheltom_celesto',5,10,'sheltom_celesto',1,1,'OS109',8,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.sheltom_celesto','core_deep',1);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.sheltom_celesto','sheltom_sparky',1);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.sheltom_celesto','ground_sapphire',1);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('artisan.sheltom_mirage',5,15,'sheltom_mirage',1,1,'OS110',10,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.sheltom_mirage','core_arcane',1);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.sheltom_mirage','sheltom_murky',1);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.sheltom_mirage','illusory_essence',1);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('artisan.sheltom_inferna',5,15,'sheltom_inferna',1,1,'OS111',10,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.sheltom_inferna','core_arcane',1);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.sheltom_inferna','sheltom_devine',1);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.sheltom_inferna','ground_ruby',1);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('artisan.sheltom_enigma',5,15,'sheltom_enigma',1,1,'OS112',10,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.sheltom_enigma','core_arcane',1);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.sheltom_enigma','sheltom_celesto',1);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.sheltom_enigma','arcane_essence',1);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('artisan.sheltom_bellum',5,20,'sheltom_bellum',1,1,'OS113',12,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.sheltom_bellum','core_runic',1);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.sheltom_bellum','sheltom_inferna',1);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.sheltom_bellum','war_fragment',1);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('artisan.sheltom_ordo',5,20,'sheltom_ordo',1,1,'OS114',12,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.sheltom_ordo','core_runic',1);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.sheltom_ordo','sheltom_enigma',1);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.sheltom_ordo','purified_gold',1);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('artisan.sheltom_gemma',5,20,'sheltom_gemma',1,1,'OS115',12,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.sheltom_gemma','core_runic',1);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.sheltom_gemma','sheltom_mirage',1);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.sheltom_gemma','ground_emerald',1);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('artisan.sheltom_purus',5,25,'sheltom_purus',1,1,'OS116',14,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.sheltom_purus','core_primordial',1);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.sheltom_purus','sheltom_ordo',1);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.sheltom_purus','sheltom_gemma',1);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('artisan.sheltom_veritas',5,25,'sheltom_veritas',1,1,'OS117',14,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.sheltom_veritas','core_primordial',1);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.sheltom_veritas','sheltom_purus',1);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.sheltom_veritas','sheltom_bellum',1);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('artisan.sheltom_veritas','primordial_essence',1);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('blacksmith.equip_wa103',2,5,'crafted_wa103',1,1,'WA103',6,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('blacksmith.equip_wa103','reinforced_handle',2);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('blacksmith.equip_wa103','iron_ingot',2);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('blacksmith.equip_da106',2,5,'crafted_da106',1,1,'DA106',6,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('blacksmith.equip_da106','treated_planks',2);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('blacksmith.equip_da106','iron_ingot',2);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('blacksmith.equip_ws103',2,0,'crafted_ws103',1,1,'WS103',4,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('blacksmith.equip_ws103','planks',2);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('blacksmith.equip_ws103','iron_ingot',2);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('blacksmith.equip_wm110',2,10,'crafted_wm110',1,1,'WM110',8,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('blacksmith.equip_wm110','mystic_solvent',2);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('blacksmith.equip_wm110','iron_ingot',2);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('arcanist.potion_hp',1,0,'potion_hp_enhanced',1,1,'',4,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('arcanist.potion_hp','purified_resin',1);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('arcanist.potion_hp','sap',1);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('arcanist.potion_mp',1,5,'potion_mp_enhanced',1,1,'',6,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('arcanist.potion_mp','mystic_solvent',1);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('arcanist.potion_mp','energetic_sap',1);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion) VALUES('arcanist.potion_stamina',1,10,'potion_stamina_enhanced',1,1,'',8,1,'366A-378');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('arcanist.potion_stamina','crystalline_sap',1);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount) VALUES('arcanist.potion_stamina','arcane_ash',1);
INSERT OR IGNORE INTO ServerDB__GatheringXP378(ResourceKey,XP,Enabled) VALUES('raw_ore_crystalline',2,1);
INSERT OR IGNORE INTO ServerDB__GatheringXP378(ResourceKey,XP,Enabled) VALUES('raw_ore_energetic',3,1);
INSERT OR IGNORE INTO ServerDB__GatheringXP378(ResourceKey,XP,Enabled) VALUES('raw_ore_deep',4,1);
INSERT OR IGNORE INTO ServerDB__GatheringXP378(ResourceKey,XP,Enabled) VALUES('raw_ore_arcane',5,1);
INSERT OR IGNORE INTO ServerDB__GatheringXP378(ResourceKey,XP,Enabled) VALUES('raw_ore_runic',6,1);
INSERT OR IGNORE INTO ServerDB__GatheringXP378(ResourceKey,XP,Enabled) VALUES('raw_ore_primordial',7,1);
INSERT OR IGNORE INTO ServerDB__GatheringXP378(ResourceKey,XP,Enabled) VALUES('common_logs',2,1);
INSERT OR IGNORE INTO ServerDB__GatheringBonusDrop378(PrimaryResourceKey,ExtraResourceKey,ChancePer10000,MinAmount,MaxAmount,Enabled) VALUES('common_logs','sap',3500,1,2,1);
INSERT OR IGNORE INTO ServerDB__GatheringBonusDrop378(PrimaryResourceKey,ExtraResourceKey,ChancePer10000,MinAmount,MaxAmount,Enabled) VALUES('common_logs','bark',1000,1,2,1);
INSERT OR IGNORE INTO ServerDB__GatheringXP378(ResourceKey,XP,Enabled) VALUES('resistant_logs',3,1);
INSERT OR IGNORE INTO ServerDB__GatheringBonusDrop378(PrimaryResourceKey,ExtraResourceKey,ChancePer10000,MinAmount,MaxAmount,Enabled) VALUES('resistant_logs','dense_resin',3500,1,2,1);
INSERT OR IGNORE INTO ServerDB__GatheringXP378(ResourceKey,XP,Enabled) VALUES('crystalline_wood',4,1);
INSERT OR IGNORE INTO ServerDB__GatheringBonusDrop378(PrimaryResourceKey,ExtraResourceKey,ChancePer10000,MinAmount,MaxAmount,Enabled) VALUES('crystalline_wood','energetic_sap',3500,1,2,1);
INSERT OR IGNORE INTO ServerDB__GatheringXP378(ResourceKey,XP,Enabled) VALUES('arcane_wood',5,1);
INSERT OR IGNORE INTO ServerDB__GatheringBonusDrop378(PrimaryResourceKey,ExtraResourceKey,ChancePer10000,MinAmount,MaxAmount,Enabled) VALUES('arcane_wood','mystic_resin',3500,1,2,1);
INSERT OR IGNORE INTO ServerDB__GatheringBonusDrop378(PrimaryResourceKey,ExtraResourceKey,ChancePer10000,MinAmount,MaxAmount,Enabled) VALUES('arcane_wood','tree_heart',1000,1,2,1);
