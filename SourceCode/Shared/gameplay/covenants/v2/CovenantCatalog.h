#pragma once

#include "gameplay/covenants/v2/CovenantTypes.h"

namespace gameplay
{
namespace covenants
{
namespace catalog
{
// Stable IDs seeded by Database/migrations/040_covenant_default_content.sql.
static const CovenantId kHunters = 1;
static const CovenantId kGatherers = 2;
static const CovenantId kArtisans = 3;
static const CovenantId kFortune = 4;
static const CovenantId kAscendants = 5;

namespace modifier
{
// Percent modifiers use fractional values: 0.10 == +10%, -0.10 == -10%.
static const char* const kHuntDropChance = "hunt.drop_chance";
static const char* const kHuntTrophyChance = "hunt.trophy_chance";
static const char* const kHuntTrophyRefiningEnabled = "hunt.trophy_refining_enabled";
static const char* const kHuntMarkEnabled = "hunt.mark_enabled";
static const char* const kCombatDamageTaken = "combat.damage_taken";

static const char* const kGatheringSurveyorEnabled = "gathering.surveyor_enabled";
static const char* const kGatheringExtraYieldChance = "gathering.extra_yield_chance";
static const char* const kGatheringSpecialMaterialChance = "gathering.special_material_chance";
static const char* const kGatheringRefinedMaterialChance = "gathering.refined_material_chance";
static const char* const kGatheringYield = "gathering.yield";

static const char* const kCraftingReforgeEnabled = "crafting.reforge_enabled";
static const char* const kCraftingForgemasterEnabled = "crafting.forgemaster_enabled";
static const char* const kCraftingMasterArtisanEnabled = "crafting.master_artisan_enabled";
static const char* const kVendorBlacksmithSellValue = "vendor.blacksmith_sell_value";

static const char* const kMonsterDropChance = "monster.drop_chance";
static const char* const kLootExtraRollChance = "loot.extra_roll_chance";
static const char* const kFortuneTemptFateEnabled = "fortune.tempt_fate_enabled";

static const char* const kMonsterExperienceGain = "xp.monster_gain";
static const char* const kSkillProgressGain = "skill.progress_gain";
static const char* const kAscendantFocusedTrainingEnabled = "ascendant.focused_training_enabled";
}
}
}
}
