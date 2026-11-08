-- SUMMIT 361A - staging ONLY. All recipes disabled, no physical grants.
-- Explicitly scoped names match the actual supplied SQLite database.
PRAGMA foreign_keys=ON;
CREATE TABLE IF NOT EXISTS ServerDB__ProfessionCraftRecipe361 (
 RecipeKey TEXT PRIMARY KEY,
 ProfessionType INTEGER NOT NULL CHECK(ProfessionType IN (1,2,5)),
 DisplayName TEXT NOT NULL,
 RequiredLevel INTEGER NOT NULL DEFAULT 0 CHECK(RequiredLevel>=0),
 ResultItemCode TEXT NOT NULL,
 ResultQuantity INTEGER NOT NULL DEFAULT 1 CHECK(ResultQuantity BETWEEN 1 AND 12),
 Enabled INTEGER NOT NULL DEFAULT 0 CHECK(Enabled IN (0,1)),
 DefinitionSource TEXT NOT NULL DEFAULT 'SUMMIT-original-test',
 CHECK(length(ResultItemCode)=10)
);
CREATE TABLE IF NOT EXISTS ServerDB__ProfessionCraftIngredient361 (
 RecipeKey TEXT NOT NULL,
 ResourceKey TEXT NOT NULL,
 Quantity INTEGER NOT NULL CHECK(Quantity BETWEEN 1 AND 1000),
 PRIMARY KEY(RecipeKey,ResourceKey),
 FOREIGN KEY(RecipeKey) REFERENCES ServerDB__ProfessionCraftRecipe361(RecipeKey)
);
CREATE TABLE IF NOT EXISTS ServerDB__ProfessionCraftIntent361 (
 IntentId TEXT PRIMARY KEY,
 PlayerKey TEXT NOT NULL,
 RecipeKey TEXT NOT NULL,
 State TEXT NOT NULL CHECK(State IN ('pending','committed','cancelled')),
 RequestedAt TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
 FOREIGN KEY(RecipeKey) REFERENCES ServerDB__ProfessionCraftRecipe361(RecipeKey)
);
-- No LIVE crafting commands or item consumption may use these tables until
-- a verified atomic item-grant/rollback bridge is introduced.
CREATE TABLE IF NOT EXISTS ServerDB__SpawnPlan361 (
 PlanId INTEGER PRIMARY KEY,
 MapId INTEGER NOT NULL CHECK(MapId BETWEEN 0 AND 255),
 X INTEGER NOT NULL, Y INTEGER NOT NULL, Z INTEGER NOT NULL,
 Kind TEXT NOT NULL CHECK(Kind IN ('monster','gather','fishing')),
 ContentKey TEXT NOT NULL,
 Density TEXT NOT NULL CHECK(Density IN ('low','medium','high')),
 MaxAlive INTEGER NOT NULL CHECK(MaxAlive BETWEEN 1 AND 30),
 Radius INTEGER NOT NULL CHECK(Radius BETWEEN 30 AND 1500),
 RespawnSeconds INTEGER NOT NULL CHECK(RespawnSeconds BETWEEN 10 AND 3600),
 Enabled INTEGER NOT NULL DEFAULT 0 CHECK(Enabled IN (0,1)),
 CreatedAt TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
);
CREATE INDEX IF NOT EXISTS IX_SpawnPlan361_Map ON ServerDB__SpawnPlan361(MapId,Enabled);
CREATE TABLE IF NOT EXISTS ClanDB__ClanV2ActionAudit361 (
 Id INTEGER PRIMARY KEY,
 ActorAccount TEXT NOT NULL,
 ClanId INTEGER,
 Action TEXT NOT NULL,
 Status TEXT NOT NULL,
 RecordedAt TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
);
CREATE TABLE IF NOT EXISTS ServerDB__FishingMinigamePlan361 (
 PlanKey TEXT PRIMARY KEY,
 MapId INTEGER NOT NULL,
 NodeId INTEGER NOT NULL,
 DummyModelKey TEXT NOT NULL,
 PhasesJson TEXT NOT NULL,
 Enabled INTEGER NOT NULL DEFAULT 0 CHECK(Enabled IN (0,1))
);
INSERT OR IGNORE INTO ServerDB__FishingMinigamePlan361
(PlanKey,MapId,NodeId,DummyModelKey,PhasesJson,Enabled)
VALUES('sea_58_test',58,910004,'DUMMY_SEA',
 '["cast","wait","hook","reel","collect"]',0);
-- Dummy is a catalog plan, never a spawned NPC until validated by the engine.
INSERT OR IGNORE INTO ServerDB__ProfessionCraftRecipe361(RecipeKey,ProfessionType,DisplayName,ResultItemCode,DefinitionSource) VALUES('artisan_amulet_1',5,'Amuleto simples','0x03030100','SUMMIT-original-test');
INSERT OR IGNORE INTO ServerDB__ProfessionCraftIngredient361(RecipeKey,ResourceKey,Quantity) VALUES('artisan_amulet_1','timber',1);
INSERT OR IGNORE INTO ServerDB__ProfessionCraftIngredient361(RecipeKey,ResourceKey,Quantity) VALUES('artisan_amulet_1','mystic_crystal',1);
-- Clan legacy tables are NOT altered; the server's existing WebDB routes
-- still need an authenticated V2 packet/service replacement.
INSERT OR IGNORE INTO ServerDB__ProfessionCraftRecipe361(RecipeKey,ProfessionType,DisplayName,ResultItemCode,DefinitionSource) VALUES('blacksmith_axe_1',2,'Machado simples','0x01010100','SUMMIT-original-test');
INSERT OR IGNORE INTO ServerDB__ProfessionCraftIngredient361(RecipeKey,ResourceKey,Quantity) VALUES('blacksmith_axe_1','iron_ore',3);
INSERT OR IGNORE INTO ServerDB__ProfessionCraftIngredient361(RecipeKey,ResourceKey,Quantity) VALUES('blacksmith_axe_1','timber',1);
INSERT OR IGNORE INTO ServerDB__ProfessionCraftRecipe361(RecipeKey,ProfessionType,DisplayName,ResultItemCode,DefinitionSource) VALUES('blacksmith_claw_1',2,'Garra simples','0x01020100','SUMMIT-original-test');
INSERT OR IGNORE INTO ServerDB__ProfessionCraftIngredient361(RecipeKey,ResourceKey,Quantity) VALUES('blacksmith_claw_1','copper_ore',3);
INSERT OR IGNORE INTO ServerDB__ProfessionCraftIngredient361(RecipeKey,ResourceKey,Quantity) VALUES('blacksmith_claw_1','timber',1);
INSERT OR IGNORE INTO ServerDB__ProfessionCraftRecipe361(RecipeKey,ProfessionType,DisplayName,ResultItemCode,DefinitionSource) VALUES('blacksmith_sword_1',2,'Espada simples','0x01060100','SUMMIT-original-test');
INSERT OR IGNORE INTO ServerDB__ProfessionCraftIngredient361(RecipeKey,ResourceKey,Quantity) VALUES('blacksmith_sword_1','iron_ore',4);
INSERT OR IGNORE INTO ServerDB__ProfessionCraftIngredient361(RecipeKey,ResourceKey,Quantity) VALUES('blacksmith_sword_1','timber',2);
INSERT OR IGNORE INTO ServerDB__ProfessionCraftRecipe361(RecipeKey,ProfessionType,DisplayName,ResultItemCode,DefinitionSource) VALUES('arcanist_staff_1',1,'Cajado simples','0x01040100','SUMMIT-original-test');
INSERT OR IGNORE INTO ServerDB__ProfessionCraftIngredient361(RecipeKey,ResourceKey,Quantity) VALUES('arcanist_staff_1','mystic_crystal',1);
INSERT OR IGNORE INTO ServerDB__ProfessionCraftIngredient361(RecipeKey,ResourceKey,Quantity) VALUES('arcanist_staff_1','timber',2);
INSERT OR IGNORE INTO ServerDB__ProfessionCraftRecipe361(RecipeKey,ProfessionType,DisplayName,ResultItemCode,DefinitionSource) VALUES('arcanist_orb_1',1,'Orbe simples','0x03010100','SUMMIT-original-test');
INSERT OR IGNORE INTO ServerDB__ProfessionCraftIngredient361(RecipeKey,ResourceKey,Quantity) VALUES('arcanist_orb_1','mystic_crystal',2);
INSERT OR IGNORE INTO ServerDB__ProfessionCraftIngredient361(RecipeKey,ResourceKey,Quantity) VALUES('arcanist_orb_1','herb_bundle',1);
INSERT OR IGNORE INTO ServerDB__ProfessionCraftRecipe361(RecipeKey,ProfessionType,DisplayName,ResultItemCode,DefinitionSource) VALUES('arcanist_ring_1',1,'Anel simples','0x03040100','SUMMIT-original-test');
INSERT OR IGNORE INTO ServerDB__ProfessionCraftIngredient361(RecipeKey,ResourceKey,Quantity) VALUES('arcanist_ring_1','mystic_crystal',1);
INSERT OR IGNORE INTO ServerDB__ProfessionCraftIngredient361(RecipeKey,ResourceKey,Quantity) VALUES('arcanist_ring_1','copper_ore',2);
INSERT OR IGNORE INTO ServerDB__ProfessionCraftRecipe361(RecipeKey,ProfessionType,DisplayName,ResultItemCode,DefinitionSource) VALUES('artisan_armor_1',5,'Armadura simples','0x02010100','SUMMIT-original-test');
INSERT OR IGNORE INTO ServerDB__ProfessionCraftIngredient361(RecipeKey,ResourceKey,Quantity) VALUES('artisan_armor_1','iron_ore',3);
INSERT OR IGNORE INTO ServerDB__ProfessionCraftIngredient361(RecipeKey,ResourceKey,Quantity) VALUES('artisan_armor_1','timber',2);
INSERT OR IGNORE INTO ServerDB__ProfessionCraftRecipe361(RecipeKey,ProfessionType,DisplayName,ResultItemCode,DefinitionSource) VALUES('artisan_boots_1',5,'Botas simples','0x02020100','SUMMIT-original-test');
INSERT OR IGNORE INTO ServerDB__ProfessionCraftIngredient361(RecipeKey,ResourceKey,Quantity) VALUES('artisan_boots_1','copper_ore',3);
INSERT OR IGNORE INTO ServerDB__ProfessionCraftIngredient361(RecipeKey,ResourceKey,Quantity) VALUES('artisan_boots_1','ancient_resin',1);
INSERT OR IGNORE INTO ServerDB__ProfessionCraftRecipe361(RecipeKey,ProfessionType,DisplayName,ResultItemCode,DefinitionSource) VALUES('artisan_gloves_1',5,'Luvas simples','0x02030100','SUMMIT-original-test');
INSERT OR IGNORE INTO ServerDB__ProfessionCraftIngredient361(RecipeKey,ResourceKey,Quantity) VALUES('artisan_gloves_1','timber',2);
INSERT OR IGNORE INTO ServerDB__ProfessionCraftIngredient361(RecipeKey,ResourceKey,Quantity) VALUES('artisan_gloves_1','ancient_resin',1);
