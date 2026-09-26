// Legacy server metadata bridge for skills whose requirements/cost arrays are not
// present in the old global sSkill_Info table. Domain identity/behavior/play-code
// comes from SkillRegistry; this file only adapts old server arrays to that domain.

// Server-side legacy requirement/cost bridge. It is intentionally independent of
// sinSkill.UseSkill[] globals because multiple actors/classes can coexist. Skill identity,
// behavior and play/presentation codes live only in SkillRegistry; this bridge owns only
// data that still comes from old server arrays (level/stamina/mana/weapons).
struct LegacyExtendedSkillMeta
{
    DWORD skillCode;
    DWORD jobCode;
    BYTE useSlot;
    BYTE tier;
    int requireLevel;
    int staminaBase;
    int staminaPerPoint;
    const int *manaTable;
    DWORD weaponCode[8];
};

#define EXT_SKILL(code, job, slot, tierNo, level, stamina0, stamina1, mana, w0,w1,w2,w3,w4,w5,w6,w7) \
    { code, job, slot, tierNo, level, stamina0, stamina1, mana, {w0,w1,w2,w3,w4,w5,w6,w7} }

static const LegacyExtendedSkillMeta kLegacyExtendedSkills[] =
{
    EXT_SKILL(SKILL_T5_RUPTURE, JOBCODE_MECHANICIAN, 17,5,100,71,2,Rupture_UseMana, sinWA1,sinWC1,sinWH1,sinWP1,sinWS2,0,0,0),
    EXT_SKILL(SKILL_T5_PARASIT_SHOT, JOBCODE_MECHANICIAN, 18,5,103,65,2,Parasit_Shot_UseMana, sinWA1,sinWC1,sinWH1,sinWP1,sinWS2,0,0,0),
    EXT_SKILL(SKILL_T5_HARDENING_SHIELD, JOBCODE_MECHANICIAN, 19,5,106,90,2,Hardening_Shield_UseMana, sinWA1,sinWC1,sinWH1,sinWP1,sinWS2,sinWS1,sinWT1,0),
    EXT_SKILL(SKILL_T5_MAGNETIC_DISCHARGE, JOBCODE_MECHANICIAN, 20,5,110,95,2,Magnetic_Discharge_UseMana, sinWA1,sinWC1,sinWH1,sinWP1,sinWS2,sinWS1,sinWT1,0),

    EXT_SKILL(SKILL_T5_SURVIVAL_INSTINCT, JOBCODE_FIGHTER, 17,5,103,0,0,NULL, 0,0,0,0,0,0,0,0),
    EXT_SKILL(SKILL_T5_SEISMIC_IMPACT, JOBCODE_FIGHTER, 18,5,100,68,2,Fighter_T5_Seismic_UseMana, sinWA1,sinWH1,sinWP1,sinWS2,0,0,0,0),
    EXT_SKILL(SKILL_T5_RUTHLESS_OF_ZECRAM, JOBCODE_FIGHTER, 19,5,106,70,2,Fighter_T5_Ruthless_UseMana, sinWA1,sinWH1,0,0,0,0,0,0),
    EXT_SKILL(SKILL_T5_WHIRLWIND, JOBCODE_FIGHTER, 20,5,110,93,2,Fighter_T5_Whirlwind_ActivationMana, sinWA1,0,0,0,0,0,0,0),

    EXT_SKILL(SKILL_T5_RING_OF_SPEARS, JOBCODE_PIKEMAN, 17,5,100,75,2,Pike_T5_Ring_UseMana, sinWC1,sinWP1,sinWS2,0,0,0,0,0),
    EXT_SKILL(SKILL_T5_FOCUS, JOBCODE_PIKEMAN, 18,5,103,70,2,Pike_T5_Focus_UseMana, sinWP1,0,0,0,0,0,0,0),
    EXT_SKILL(SKILL_T5_DEATH_MASTER, JOBCODE_PIKEMAN, 19,5,106,93,2,Pike_T5_Death_UseMana, sinWP1,sinWS2,0,0,0,0,0,0),
    EXT_SKILL(SKILL_T5_TWISTER, JOBCODE_PIKEMAN, 20,5,110,81,2,Pike_T5_Twister_UseMana, sinWP1,sinWS2,0,0,0,0,0,0),

    EXT_SKILL(SKILL_T5_LETHAL_SIGHT, JOBCODE_ARCHER, 17,5,100,52,2,Archer_T5_Lethal_UseMana, sinWS1,0,0,0,0,0,0,0),
    EXT_SKILL(SKILL_T5_FIERCE_WIND, JOBCODE_ARCHER, 18,5,103,52,2,Archer_T5_Fierce_UseMana, sinWS1,0,0,0,0,0,0,0),
    EXT_SKILL(SKILL_T5_ENTALING_ROOTS, JOBCODE_ARCHER, 19,5,106,57,2,Archer_T5_Roots_UseMana, sinWS1,0,0,0,0,0,0,0),
    EXT_SKILL(SKILL_T5_BOMBARDMENT, JOBCODE_ARCHER, 20,5,110,95,2,Archer_T5_Bombard_UseMana, sinWS1,0,0,0,0,0,0,0),

    EXT_SKILL(SKILL_T5_HOLY_CONVICTION, JOBCODE_KNIGHT, 17,5,100,69,2,Knight_T5_HolyConviction_UseMana, sinWS2,0,0,0,0,0,0,0),
    EXT_SKILL(SKILL_T5_DIVINE_INQUISITON, JOBCODE_KNIGHT, 18,5,103,70,2,Knight_T5_DivineInquisition_UseMana, sinWS2,0,0,0,0,0,0,0),
    EXT_SKILL(SKILL_T5_GLORIOUS_SHIELD, JOBCODE_KNIGHT, 19,5,106,75,2,Knight_T5_GloriousShield_UseMana, sinWS2,sinWH1,0,0,0,0,0,0),
    EXT_SKILL(SKILL_T5_DIVINE_CROSS, JOBCODE_KNIGHT, 20,5,110,93,2,Knight_T5_DivineCross_UseMana, sinWA1,sinWC1,sinWH1,sinWP1,sinWS2,sinWS1,sinWT1,0),

    EXT_SKILL(SKILL_T5_SUMMON_TIGER, JOBCODE_ATALANTA, 17,5,100,95,2,Atalanta_T5_SummonTiger_UseMana, 0,0,0,0,0,0,0,0),
    EXT_SKILL(SKILL_T5_GOLDEN_APPLE, JOBCODE_ATALANTA, 18,5,103,0,0,NULL, 0,0,0,0,0,0,0,0),
    EXT_SKILL(SKILL_T5_PLAGUE_JAVELIN, JOBCODE_ATALANTA, 19,5,106,68,2,Atalanta_T5_Plague_UseMana, sinWA1,sinWC1,sinWH1,sinWP1,sinWS1,sinWT1,0,0),
    EXT_SKILL(SKILL_T5_COMBO_JAVELIN, JOBCODE_ATALANTA, 20,5,110,57,2,Atalanta_T5_Combo_UseMana, sinWT1,0,0,0,0,0,0,0),

    EXT_SKILL(SKILL_T5_ICE_ELEMENTAL, JOBCODE_PRIESTESS, 17,5,100,45,2,Priestess_T5_IceElemental_UseMana, 0,0,0,0,0,0,0,0),
    EXT_SKILL(SKILL_T5_LIGHTNING_SURGE, JOBCODE_PRIESTESS, 18,5,103,50,2,Priestess_T5_LightningSurge_UseMana, sinWM1,0,0,0,0,0,0,0),
    EXT_SKILL(SKILL_T5_HEAVENLY_LIGHT, JOBCODE_PRIESTESS, 19,5,106,54,2,Priestess_T5_HeavenlyLight_UseMana, sinWM1,0,0,0,0,0,0,0),
    EXT_SKILL(SKILL_T5_CONSECRATION, JOBCODE_PRIESTESS, 20,5,110,60,2,Priestess_T5_Consecration_UseMana, sinWM1,0,0,0,0,0,0,0),

    EXT_SKILL(SKILL_T5_WIZARD_TRANCE, JOBCODE_MAGICIAN, 17,5,100,54,2,Magician_T5_WizardTrance_UseMana, sinWM1,0,0,0,0,0,0,0),
    EXT_SKILL(SKILL_T5_STONE_SKIN, JOBCODE_MAGICIAN, 18,5,103,45,2,Magician_T5_StoneSkin_UseMana, sinWM1,0,0,0,0,0,0,0),
    EXT_SKILL(SKILL_T5_RED_RAY, JOBCODE_MAGICIAN, 19,5,106,50,2,Magician_T5_RedRay_UseMana, sinWM1,0,0,0,0,0,0,0),
    EXT_SKILL(SKILL_T5_CATACLYSM, JOBCODE_MAGICIAN, 20,5,110,60,2,Magician_T5_Cataclysm_UseMana, sinWM1,0,0,0,0,0,0,0),
};

#undef EXT_SKILL

static const int kLegacyExtendedSkillCount =
    (int)(sizeof(kLegacyExtendedSkills) / sizeof(kLegacyExtendedSkills[0]));

static const LegacyExtendedSkillMeta *FindLegacyExtendedSkillMeta(DWORD skillCode)
{
    for(int i = 0; i < kLegacyExtendedSkillCount; ++i)
        if(kLegacyExtendedSkills[i].skillCode == skillCode)
            return &kLegacyExtendedSkills[i];
    return NULL;
}
