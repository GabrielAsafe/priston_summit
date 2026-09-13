#pragma once

SKILL_DATA_CODE	SkillDataCode[] = {
{ "xxxxxxxxxxxxx"			,	0 },

	//MECHANICIAN
{ "ExtremeShield"			,	SKILL_PLAY_EXTREME_SHIELD },
{ "MechanicBomb"			,	SKILL_PLAY_MECHANIC_BOMB },
{ "PhysicalAbsorption"		,	SKILL_PLAY_PHYSICAL_ABSORB },
{ "PoisonAttribute"			,	0 },

{ "roundimpact"				,	SKILL_PLAY_GREAT_SMASH },
{ "Maximize"				,	SKILL_PLAY_MAXIMIZE },
{ "Automation"				,	SKILL_PLAY_AUTOMATION },
{ "Spark"					,	SKILL_PLAY_SPARK },

{ "MetalArmor"				,	SKILL_PLAY_METAL_ARMOR },
{ "GrandSmash"				,	SKILL_PLAY_GRAND_SMASH },
//{ "MechanicMastery"		,	0 },
{ "SparkShield"				,	SKILL_PLAY_SPARK_SHIELD },

{ "IMPULSION"				,	SKILL_PLAY_IMPULSION },
{ "COMPULSION"				,	SKILL_PLAY_COMPULSION },
{ "MAGNETICSPHERE"			,	SKILL_PLAY_MAGNETIC_SPHERE },
{ "METALGOLEM"				,	SKILL_PLAY_METAL_GOLEM },

// Summit Tier 5 motion registry. The legacy motion loader resolves animation
// SkillCodeList names through this table; keep both the old animation aliases
// and the current EU skill names mapped to the shared T51..T54 play slots.
{ "LandMining"			,	SKILL_PLAY_T51 },
{ "Rupture"				,	SKILL_PLAY_T51 },
{ "HyperSonic"			,	SKILL_PLAY_T52 },
{ "ParasitShot"			,	SKILL_PLAY_T52 },
{ "RollingSmash"			,	SKILL_PLAY_T53 },
{ "HardeningShield"		,	SKILL_PLAY_T53 },
{ "PowerEnhance"			,	SKILL_PLAY_T54 },
{ "MagneticDischarge"	,	SKILL_PLAY_T54 },

//FIGHTER
{ "MeleeMastery"			,	0 },
{ "FireAttribute"			,	0 },
{ "Raving"					,	SKILL_PLAY_RAVING },
{ "Impact"					,	SKILL_PLAY_IMPACT },

{ "TripleImpact"			,	SKILL_PLAY_TRIPLE_IMPACT },
{ "BrutalSwing"				,	SKILL_PLAY_BRUTAL_SWING },
{ "Roar"					,	SKILL_PLAY_ROAR },
{ "RageofZecram"			,	SKILL_PLAY_RAGEOF_ZECRAM },

{ "Concentration"			,	SKILL_PLAY_CONCENTRATION },
{ "AvangingCrash"			,	SKILL_PLAY_AVANGING_CRASH },
{ "SwiftAxe"				,	SKILL_PLAY_SWIFT_AXE },
{ "BoneSmash"				,	SKILL_PLAY_BONE_SMASH },

{ "DESTROYER"				,	SKILL_PLAY_DESTROYER },
{ "BERSERKER"				,	SKILL_PLAY_BERSERKER },
{ "CYCLONESTRIKE"			,	SKILL_PLAY_CYCLONE_STRIKE },
//{ "BoostHealth"			,	0 },

{ "DOWNHIT"				,	SKILL_PLAY_T51 },
{ "SesmicImpact"			,	SKILL_PLAY_T51 },
{ "SeismicImpact"			,	SKILL_PLAY_T51 },
{ "POWERDASH"				,	SKILL_PLAY_T52 },
{ "MOTALBLOW"				,	SKILL_PLAY_T53 },
{ "RuthlessofZecram"		,	SKILL_PLAY_T53 },
{ "BLOODYBERSERKER"		,	SKILL_PLAY_T54 },
{ "Whirlwind"				,	SKILL_PLAY_T54 },

//PIKEMAN
{ "PikeWind"				,	SKILL_PLAY_PIKEWIND },
{ "IceAttribute"			,	0 },
{ "CriticalHit"				,	SKILL_PLAY_CRITICAL_HIT },
{ "JumpingCrash"			,	SKILL_PLAY_JUMPING_CRASH },

{ "GroundPike"				,	SKILL_PLAY_GROUND_PIKE },
{ "Tornado"					,	SKILL_PLAY_TORNADO },
{ "WeaponDefenceMastery"	,	0 },
{ "Expansion"				,	SKILL_PLAY_EXPANSION },

{ "VenomSpear"				,	SKILL_PLAY_VENOM_SPEAR },
//{ "Vanish"				,	SKILL_PLAY_VANISH },
{ "BladeOfBlaze"			,	SKILL_PLAY_BLADE_OF_BLAZE },//{ "CriticalMastery"		,	0 },
{ "ChainLance"				,	SKILL_PLAY_CHAIN_LANCE },

{ "ASSASSINEYE"				,	SKILL_PLAY_ASSASSIN_EYE },
{ "CHARGINGSTRIKE"			,	SKILL_PLAY_CHARGING_STRIKE },
{ "VAGUE"					,	SKILL_PLAY_VAGUE },
{ "SHADOWMASTER"			,	SKILL_PLAY_SHADOW_MASTER },

{ "DANCINGREAPER"			,	SKILL_PLAY_T51 },
{ "RingofSpears"			,	SKILL_PLAY_T51 },
{ "RingOfSpears"			,	SKILL_PLAY_T51 },
{ "FINALSPEAR"			,	SKILL_PLAY_T52 },
{ "Focus"					,	SKILL_PLAY_T52 },
{ "DeathMantra"			,	SKILL_PLAY_T52 },
{ "AMPLIFIED"				,	SKILL_PLAY_T53 },
{ "DeathMaster"			,	SKILL_PLAY_T53 },
{ "ExDeath"				,	SKILL_PLAY_T53 },
{ "SIDESPINATTACK"		,	SKILL_PLAY_T54 },
{ "Twister"				,	SKILL_PLAY_T54 },
{ "SinisterSpin"			,	SKILL_PLAY_T54 },

//ARCHER
{ "ScoutHawk"				,	SKILL_PLAY_SCOUT_HAWK },
{ "ShootingMastery"			,	0 },
{ "WindArrow"				,	SKILL_PLAY_WIND_ARROW },
{ "PerfectAim"				,	SKILL_PLAY_PERFECT_AIM },

{ "DionsEye"				,	0 },
{ "Falcon"					,	SKILL_PLAY_FALCON },
{ "ArrowofRage"				,	SKILL_PLAY_ARROWOF_RAGE },
{ "Avalanche"				,	SKILL_PLAY_AVALANCHE },

{ "ElementalShot"			,	SKILL_PLAY_ELEMENTAL_SHOT },
{ "GoldenFalcon"			,	SKILL_PLAY_GOLDEN_FALCON },
{ "BombShot"				,	SKILL_PLAY_BOMB_SHOT },
{ "Perforation"				,	SKILL_PLAY_PERFORATION },

{ "RECALLWOLVERIN"			,	SKILL_PLAY_RECALL_WOLVERIN },
//{ "EvasionMastery"		,	0 },
{ "PHOENIXSHOT"				,	SKILL_PLAY_PHOENIX_SHOT },
{ "FORCEOFNATURE"			,	SKILL_PLAY_FORCE_OF_NATURE },

{ "EVADESHOT"				,	SKILL_PLAY_T51 },
{ "LethalSight"			,	SKILL_PLAY_T51 },
{ "THUNDERROOP"			,	SKILL_PLAY_T52 },
{ "FierceWind"				,	SKILL_PLAY_T52 },
{ "NATURESPLASH"			,	SKILL_PLAY_T53 },
{ "EntalingRoots"			,	SKILL_PLAY_T53 },
{ "EntanglingRoots"		,	SKILL_PLAY_T53 },
{ "CIRCLETRAP"			,	SKILL_PLAY_T54 },
{ "Bombardment"			,	SKILL_PLAY_T54 },

//KNIGHT
{ "SwordBlast"				,	SKILL_PLAY_SWORD_BLAST },
{ "HolyBody"				,	SKILL_PLAY_HOLY_BODY },
{ "PhysicalTraning"			,	0 },
{ "DoubleCrash"				,	SKILL_PLAY_DOUBLE_CRASH },

{ "holyteam"				,	SKILL_PLAY_HOLY_VALOR },
{ "Brandish"				,	SKILL_PLAY_BRANDISH },
{ "Piercing"				,	SKILL_PLAY_PIERCING },
{ "DrasticSpirit"			,	SKILL_PLAY_DRASTIC_SPIRIT },

{ "FlameBrandish"			,	SKILL_PLAY_FLAME_BRANDISH },//{ "SwordMastery"			,	0 },
{ "Divineinhalation"		,	SKILL_PLAY_DIVINE_INHALATION },
{ "HolyIncantation"			,	SKILL_PLAY_HOLY_INCANTATION },
{ "GrandCross"				,	SKILL_PLAY_GRAND_CROSS },

{ "DIVINEPIERCING"			,	SKILL_PLAY_DIVINE_PIERCING },
{ "DIVINEPIERCING2"			,	SKILL_PLAY_DIVINE_PIERCING2 },
{ "DIVINEPIERCING3"			,	SKILL_PLAY_DIVINE_PIERCING3 },
{ "GODLYSHIELD"				,	SKILL_PLAY_GODLY_SHIELD },
{ "GODSBLESS"				,	SKILL_PLAY_GODS_BLESS },
{ "SWORDOFJUSTICE"			,	SKILL_PLAY_SWORD_OF_JUSTICE },

{ "SOULBREAKER"			,	SKILL_PLAY_T51 },
{ "HolyConviction"			,	SKILL_PLAY_T51 },
{ "CRESCENTMOON"			,	SKILL_PLAY_T52 },
{ "DivineInquisition"		,	SKILL_PLAY_T52 },
{ "SAINTBLADE"			,	SKILL_PLAY_T53 },
{ "GloriousShield"			,	SKILL_PLAY_T53 },
{ "HOLYBENEDIC"			,	SKILL_PLAY_T54 },
{ "DivineCross"				,	SKILL_PLAY_T54 },

//ATALANTA
{ "ShieldStrike"			,	SKILL_PLAY_SHIELD_STRIKE },
{ "Farina"					,	SKILL_PLAY_FARINA },
{ "ThrowingMastery"			,	0 },
{ "VigorSpear"				,	SKILL_PLAY_VIGOR_SPEAR },

{ "Windy"					,	SKILL_PLAY_WINDY },
{ "TwistJavelin"			,	SKILL_PLAY_TWIST_JAVELIN },
{ "SoulSucker"				,	SKILL_PLAY_SOUL_SUCKER },
{ "FireJavelin"				,	SKILL_PLAY_FIRE_JAVELIN },

{ "SplitJavelin"			,	SKILL_PLAY_SPLIT_JAVELIN },
{ "TriumphOfValhalla"		,	SKILL_PLAY_TRIUMPH_OF_VALHALLA },
{ "LightningJavelin"		,	SKILL_PLAY_LIGHTNING_JAVELIN },
{ "StormJavelin"			,	SKILL_PLAY_STORM_JAVELIN },

{ "HALLOFVALHALLA"			,	SKILL_PLAY_HALL_OF_VALHALLA },
{ "XRAGE"					,	SKILL_PLAY_X_RAGE },
{ "FROSTJAVELIN"			,	SKILL_PLAY_FROST_JAVELIN },
{ "VENGEANCE"				,	SKILL_PLAY_VENGEANCE },

{ "TALARIA"				,	SKILL_PLAY_T51 },
{ "ComboJavelin"			,	SKILL_PLAY_T51 },
{ "SummonTiger"			,	SKILL_PLAY_T51 },
{ "GALAXYCOUP"			,	SKILL_PLAY_T52 },
{ "GoldenApple"			,	SKILL_PLAY_T52 },
{ "SUMMONARCUDA"			,	SKILL_PLAY_T53 },
{ "PlagueJavelin"			,	SKILL_PLAY_T53 },
{ "SNIPPYFEAR"			,	SKILL_PLAY_T54 },
{ "TripleThreat"			,	SKILL_PLAY_T54 },

//PRIESTESS
{ "Healing"					,	SKILL_PLAY_HEALING },
{ "HolyBolt"				,	SKILL_PLAY_HOLY_BOLT },
{ "MultiSpark"				,	SKILL_PLAY_MULTI_SPARK },
{ "HolyMind"				,	SKILL_PLAY_HOLY_MIND },

{ "Meditation"				,	0 },
{ "DivineLightning"			,	SKILL_PLAY_DIVINE_LIGHTNING },
{ "HolyReflection"			,	SKILL_PLAY_HOLY_REFLECTION },
{ "GrandHealing"			,	SKILL_PLAY_GREAT_HEALING },

{ "VigorBall"				,	SKILL_PLAY_VIGOR_BALL },
{ "Resurrection"			,	SKILL_PLAY_RESURRECTION },
{ "Extinction"				,	SKILL_PLAY_EXTINCTION },
{ "VirtualLife"				,	SKILL_PLAY_VIRTUAL_LIFE },

{ "GLACIALSPIKE"			,	SKILL_PLAY_GLACIAL_SPIKE },
{ "REGENERATIONFIELD"		,	SKILL_PLAY_REGENERATION_FIELD },
{ "CHAINLIGHTNING"			,	SKILL_PLAY_CHAIN_LIGHTNING },
{ "SUMMONMUSPELL"			,	SKILL_PLAY_SUMMON_MUSPELL },

{ "SpiritImpact"			,	SKILL_PLAY_T51 },
{ "IceElemental"			,	SKILL_PLAY_T51 },
{ "PiercingIce"			,	SKILL_PLAY_T52 },
{ "LightningSurge"			,	SKILL_PLAY_T52 },
{ "SummonRamiel"			,	SKILL_PLAY_T53 },
{ "HeavenlyLight"			,	SKILL_PLAY_T53 },
{ "BlessingKrishna"		,	SKILL_PLAY_T54 },
{ "Consecration"			,	SKILL_PLAY_T54 },

//MAGICIAN
{ "Agony"					,	SKILL_PLAY_AGONY },
{ "FireBolt"				,	SKILL_PLAY_FIRE_BOLT },
{ "Zenith"					,	SKILL_PLAY_ZENITH },
{ "FireBall"				,	SKILL_PLAY_FIRE_BALL },

{ "MentalMastery"			,	0 },
{ "ColumnOfWater"			,	SKILL_PLAY_COLUMN_OF_WATER },
{ "EnchantWeapon"			,	SKILL_PLAY_ENCHANT_WEAPON },
{ "DeadRay"					,	SKILL_PLAY_DEAD_RAY },

{ "EnergyShield"			,	SKILL_PLAY_ENERGY_SHIELD },
{ "Diastrophism"			,	SKILL_PLAY_DIASTROPHISM },
{ "SpiritElemental"			,	SKILL_PLAY_SPIRIT_ELEMENTAL },
{ "DancingSword"			,	SKILL_PLAY_DANCING_SWORD },

{ "FIREELEMENTAL"			,	SKILL_PLAY_FIRE_ELEMENTAL },
{ "FLAMEWAVE"				,	SKILL_PLAY_FLAME_WAVE },
{ "DISTORTION"				,	SKILL_PLAY_DISTORTION },
{ "METEO"					,	SKILL_PLAY_METEO },

{ "Silraphim"				,	SKILL_PLAY_T51 },
{ "StoneSkin"				,	SKILL_PLAY_T51 },
{ "Vistenus"				,	SKILL_PLAY_T52 },
{ "RedRay"					,	SKILL_PLAY_T52 },
{ "Primaignis"			,	SKILL_PLAY_T53 },
{ "Thirdanima"			,	SKILL_PLAY_T54 },
{ "Cataclysm"				,	SKILL_PLAY_T54 },

//
////ASSASSIN
//{ "Stingger"				,	SKILL_PLAY_STINGGER },
//{ "RunningHit"			,	SKILL_PLAY_RUNNING_HIT },
//{ "DualSwordMastery"		,	0 },
//{ "Wisp"					,	SKILL_PLAY_WISP },
//
//{ "VenomThrone"			,	SKILL_PLAY_VENOM_THRONE },
//{ "Alas"					,	SKILL_PLAY_ALAS },
//{ "SoulShock"				,	SKILL_PLAY_SOUL_SHOCK },
//{ "AttackMastery"			,	0 },
//
//{ "SoreSword"				,	SKILL_PLAY_SORE_SWORD },
//{ "BeatUp"				,	SKILL_PLAY_BEAT_UP },
//{ "Inpes"					,	SKILL_PLAY_INPES },
//{ "Blind"					,	SKILL_PLAY_BLIND },
//
//{ "FrostWind"				,	SKILL_PLAY_FROST_WIND },
//{ "CriticalMastery"		,	0 },
//{ "Polluted"				,	SKILL_PLAY_POLLUTED },
//{ "PastingShadow"			,	SKILL_PLAY_PASTING_SHADOW },
//
//{ "ShadowBomb"			,	SKILL_PLAY_SHADOW_BOMB },
//{ "RisingSlash"			,	SKILL_PLAY_RISING_SLASH },
//{ "ViolenceStab"			,	SKILL_PLAY_VIOLENCE_STAB },
//{ "Storm"					,	SKILL_PLAY_STORM },
//
////SHAMAN
//{ "DarkBolt"				,	SKILL_PLAY_DARK_BOLT },
//{ "DarkWave"				,	SKILL_PLAY_DARK_WAVE },
//{ "CurseLazy"				,	SKILL_PLAY_CURSE_LAZY },
//{ "InnerPeace"			,	0 },
//
//{ "SpiritualFlare"		,	SKILL_PLAY_SPIRITUAL_FLARE },
//{ "SoulManacle"			,	SKILL_PLAY_SOUL_MANACLE },
//{ "ChasingHunt"			,	SKILL_PLAY_CHASING_HUNT },
//{ "AdventMigal"			,	SKILL_PLAY_ADVENT_MIGAL },
//
//{ "RainMaker"				,	SKILL_PLAY_RAIN_MAKER },
//{ "LandOfGhost"			,	SKILL_PLAY_LAND_OF_GHOST },
//{ "Haunt"					,	SKILL_PLAY_HAUNT },
//{ "Scratch"				,	SKILL_PLAY_SCRATCH },
//
//{ "RecallBloodyKnight"	,	SKILL_PLAY_RECALL_BLOODY_KNIGHT },
//{ "Judgement"				,	SKILL_PLAY_JUDGEMENT },
//{ "AdventMidranda"		,	SKILL_PLAY_ADVENT_MIDRANDA },
//{ "MourningOfPray"		,	SKILL_PLAY_MOURNING_OF_PRAY },
//
//{ "Creed"					,	SKILL_PLAY_CREED },
//{ "PressOfDeity"			,	SKILL_PLAY_PRESS_OF_DEITY },
//{ "GhostyNail"			,	SKILL_PLAY_GHOST_NAIL },
//{ "HighRegeneration"		,	SKILL_HIGH_REGENERATION },
//
////
//{ "JUMPATTACK"			,	SKILL_PLAY_JUMPATTACK },
//{ "CONTATTACK1"			,	SKILL_PLAY_CONTATTACK1 },
//{ "CONTATTACK2"			,	SKILL_PLAY_CONTATTACK2 },
//{ "CONTATTACK3"			,	SKILL_PLAY_CONTATTACK3 },
//{ "CONTATTACK4"			,	SKILL_PLAY_CONTATTACK4 },

{ ""						,	0 }
};



