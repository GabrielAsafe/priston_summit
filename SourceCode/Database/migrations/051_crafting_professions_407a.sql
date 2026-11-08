-- SUMMIT 407A. Playable material recipes for Arcanist and Blacksmith.
-- Their XP persists via the existing SQLite craft transaction.
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378
 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion)
 VALUES('arcanist.distill_arcane_essence',1,0,'arcane_essence',1,0,'',4,1,'407A');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount)
 VALUES('arcanist.distill_arcane_essence','herb_bundle',1);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount)
 VALUES('arcanist.distill_arcane_essence','sap',1);
INSERT OR IGNORE INTO ServerDB__ProfessionRecipe378
 (RecipeKey,ProfessionType,MinLevel,OutputKey,OutputCount,Physical,LegacyItemCode,XP,Enabled,SourceVersion)
 VALUES('blacksmith.smelt_iron_ingot',2,0,'iron_ingot',1,0,'',4,1,'407A');
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount)
 VALUES('blacksmith.smelt_iron_ingot','raw_ore_crystalline',2);
INSERT OR IGNORE INTO ServerDB__ProfessionIngredient378(RecipeKey,ResourceKey,Amount)
 VALUES('blacksmith.smelt_iron_ingot','common_logs',1);
