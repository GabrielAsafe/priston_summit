#include "gameplay/classes/v2/content/LegacyClassCatalog.h"

#include <cstdio>

#include "gameplay/classes/v2/content/CompositeClassContentSource.h"
#include "gameplay/classes/v2/content/ReservedClassCatalog.h"

namespace gameplay
{
namespace classes
{
namespace
{
namespace legacy_skill_group
{
static const std::uint32_t Mechanician = 0x01000000u;
static const std::uint32_t Fighter = 0x02000000u;
static const std::uint32_t Pikeman = 0x03000000u;
static const std::uint32_t Archer = 0x04000000u;
static const std::uint32_t Knight = 0x05000000u;
static const std::uint32_t Atalanta = 0x06000000u;
static const std::uint32_t Priestess = 0x07000000u;
static const std::uint32_t Magician = 0x08000000u;
}

namespace legacy_weapon
{
static const std::uint32_t Axe = 0x01010000u;
static const std::uint32_t Claw = 0x01020000u;
static const std::uint32_t Magic = 0x01040000u;
static const std::uint32_t Pike = 0x01050000u;
static const std::uint32_t Bow = 0x01060000u;
static const std::uint32_t Sword = 0x01070000u;
static const std::uint32_t Javelin = 0x01080000u;
}

void PreferredWeapon(PlayableClassDefinition& definition, std::uint32_t itemType)
{
    definition.equipment.preferredWeaponTypes.push_back(itemType);
    if (definition.equipment.defaultWeaponType == 0u)
        definition.equipment.defaultWeaponType = itemType;
}

std::vector<std::string> BodyModels(const char* prefix)
{
    std::vector<std::string> result;
    char path[64];
    for (int index = 1; index <= 7; ++index)
    {
        std::snprintf(path, sizeof(path), "char//tmABCD//%s%03d.ini", prefix, index);
        result.push_back(path);
    }
    return result;
}

ClassFaceOption Face(const char* model,
                     const char* alternateModel,
                     const char* previewNormal,
                     const char* previewSelected)
{
    ClassFaceOption face;
    face.model = model ? model : "";
    face.alternateModel = alternateModel ? alternateModel : "";
    face.previewNormal = previewNormal ? previewNormal : "";
    face.previewSelected = previewSelected ? previewSelected : "";
    return face;
}

ClassRankDefinition Rank(int rank,
                         std::uint32_t bit,
                         const char* key,
                         const char* displayKey)
{
    ClassRankDefinition definition;
    definition.rank = rank;
    definition.legacyJobBitCode = bit;
    definition.key = key ? key : "";
    definition.displayKey = displayKey ? displayKey : "";
    return definition;
}

void RankAlias(PlayableClassDefinition& definition, int rankNumber, const char* alias)
{
    if (!alias || !alias[0])
        return;
    for (std::vector<ClassRankDefinition>::iterator rank = definition.ranks.begin();
         rank != definition.ranks.end(); ++rank)
    {
        if (rank->rank == rankNumber)
        {
            rank->contentAliases.push_back(alias);
            return;
        }
    }
}

PlayableClassDefinition BaseClass(int jobCode,
                                  std::uint32_t jobBitCode,
                                  int brood,
                                  const char* key,
                                  const char* displayKey,
                                  int armorModelIndex,
                                  ClassCapability family,
                                  int selectionOrder,
                                  const CharacterAttributes& attributes,
                                  const CharacterFormulaProfile& formula)
{
    PlayableClassDefinition definition;
    definition.id = jobCode;
    definition.legacyJobCode = jobCode;
    definition.jobBitCode = jobBitCode;
    definition.brood = brood;
    definition.key = key;
    definition.displayKey = displayKey;
    definition.skills.treeKey = key;
    definition.skills.usesLegacyTier5Slots = true;
    definition.appearance.legacyCatalogKey = key;
    definition.appearance.maxPromotionVisualChangeJob = 3;
    definition.appearance.bodyModelRoots.push_back("char//tmABCD//");
    definition.armor.legacyModelIndex = armorModelIndex;
    definition.armor.modelRoot = "char//tmABCD//";
    definition.animation.legacyMotionJobMask = static_cast<std::uint32_t>(1u << (jobCode - 1));
    definition.enabled = true;
    definition.capabilities = CapabilityBit(family);
    definition.formula = formula;
    definition.creation.selectable = true;
    definition.creation.selectionOrder = selectionOrder;
    definition.creation.baseAttributes = attributes;
    return definition;
}

void ItemRequirementRevision(PlayableClassDefinition& definition,
                             int levelMin, int levelMax,
                             int strengthMin, int strengthMax,
                             int spiritMin, int spiritMax,
                             int talentMin, int talentMax,
                             int dexterityMin, int dexterityMax,
                             int healthMin, int healthMax)
{
    definition.itemRequirementRevision.level = ClassPercentRange(levelMin, levelMax);
    definition.itemRequirementRevision.strength = ClassPercentRange(strengthMin, strengthMax);
    definition.itemRequirementRevision.spirit = ClassPercentRange(spiritMin, spiritMax);
    definition.itemRequirementRevision.talent = ClassPercentRange(talentMin, talentMax);
    definition.itemRequirementRevision.dexterity = ClassPercentRange(dexterityMin, dexterityMax);
    definition.itemRequirementRevision.health = ClassPercentRange(healthMin, healthMax);
}

CharacterFormulaProfile Formula(int sight,
                                int life,
                                int mana,
                                int stamina,
                                int damage0,
                                int damage1,
                                int damage2)
{
    CharacterFormulaProfile result;
    result.sight = sight;
    result.lifeFunction = life;
    result.manaFunction = mana;
    result.staminaFunction = stamina;
    result.damageFunction[0] = damage0;
    result.damageFunction[1] = damage1;
    result.damageFunction[2] = damage2;
    return result;
}

void Presentation(PlayableClassDefinition& definition,
                  const char* labelNormal,
                  const char* labelSelected,
                  const char* description,
                  const char* legacyLevel8HelpMessageKey,
                  const char* legacyMasterTitle,
                  const char* rankingIcon)
{
    definition.creation.presentation.labelNormal = labelNormal;
    definition.creation.presentation.labelSelected = labelSelected;
    definition.creation.presentation.description = description;
    definition.presentation.legacyLevel8HelpMessageKey =
        legacyLevel8HelpMessageKey ? legacyLevel8HelpMessageKey : "";
    definition.presentation.legacyMasterTitle = legacyMasterTitle ? legacyMasterTitle : "";
    definition.presentation.rankingIcon = rankingIcon ? rankingIcon : "";

    char killIcon[64];
    std::snprintf(killIcon, sizeof(killIcon), "game//kill//class%d.png", definition.legacyJobCode);
    definition.presentation.killNotificationIcon = killIcon;

    // Party UI historically used a separate naming scheme and indexed a fixed
    // [2][8] matrix by JOB_CODE - 1. Keep the filenames in presentation
    // metadata so the UI can key textures by the actual job code instead.
    definition.presentation.partyAliveIcon = std::string("game//Party//Class//") + definition.displayKey + ".tga";
    definition.presentation.partyDeadIcon = std::string("game//Party//Class//") + definition.displayKey + "_.tga";

    char sodIcon[96];
    std::snprintf(sodIcon, sizeof(sodIcon), "image//Sinimage//help//sod_cr-%d.bmp", definition.legacyJobCode);
    definition.presentation.sodRankIcon = sodIcon;
}

PlayableClassDefinition Mechanician()
{
    PlayableClassDefinition d = BaseClass(2, 0x00000001u, 1, "mechanician", "Mechanician", 0,
                                          ClassCapability::Tempskron, 0,
                                          CharacterAttributes(24, 8, 25, 18, 24),
                                          Formula(65, 2, 2, 2, 2, 2, 0));
    ItemRequirementRevision(d, 0, 0, 5, 15, -20, -10, 0, 0, -25, -15, 0, 0);
    d.combat.criticalScalingAttribute = CharacterAttribute::Strength;
    d.gender = CharacterGender::Male;
    d.skills.legacyGroupCode = legacy_skill_group::Mechanician;
    d.capabilities |= CapabilityBit(ClassCapability::Melee);
    d.equipment.affinityUsesItemSpecialization = true;
    d.equipment.affinityUniqueItemCode = 2;
    d.equipment.defaultWeaponType = legacy_weapon::Claw;
    d.creation.bodyModels = BodyModels("a");
    d.creation.faces.push_back(Face("char//tmABCD//tmh-a01.inf", "char//tmABCD//tmh-a01a.inf", "TmA01-01.tga", "TmA01-02.tga"));
    d.creation.faces.push_back(Face("char//tmABCD//tmh-a02.inf", "char//tmABCD//tmh-a02a.inf", "TmA02-01.tga", "TmA02-02.tga"));
    d.creation.faces.push_back(Face("char//tmABCD//tmh-a03.inf", "char//tmABCD//tmh-a03a.inf", "TmA03-01.tga", "TmA03-02.tga"));
    Presentation(d, "T_cls-meca_.tga", "T_cls-meca.tga", "T_chr-meca.tga", "*Special_Message8_Mecha", "[Mestre dos Mecanicos]", "game//ranking//Class//ms.bmp");
    d.ranks.push_back(Rank(1, 0x00000001u, "mechanician", "Mechanician"));
    d.ranks.push_back(Rank(2, 0x00000010u, "mechanic_master", "Mechanic Master"));
    d.ranks.push_back(Rank(3, 0x00000100u, "metal_leader", "Metal Leader"));
    d.ranks.push_back(Rank(4, 0x00001000u, "metallion", "Metallion"));
    d.ranks.push_back(Rank(5, 0x00010000u, "metalrage", "Metalrage"));
    RankAlias(d, 2, "MechanicMaster");
    RankAlias(d, 3, "MetalLeader");
    RankAlias(d, 4, "Metalion");
    RankAlias(d, 4, "HeavyMetal");
    return d;
}

PlayableClassDefinition Fighter()
{
    PlayableClassDefinition d = BaseClass(1, 0x00000002u, 1, "fighter", "Fighter", 1,
                                          ClassCapability::Tempskron, 1,
                                          CharacterAttributes(28, 6, 21, 17, 27),
                                          Formula(70, 1, 3, 1, 1, 2, 0));
    ItemRequirementRevision(d, 0, 0, 10, 15, -20, -15, 0, 0, -20, -15, 0, 0);
    d.combat.criticalScalingAttribute = CharacterAttribute::Strength;
    d.gender = CharacterGender::Male;
    d.skills.legacyGroupCode = legacy_skill_group::Fighter;
    d.capabilities |= CapabilityBit(ClassCapability::Melee);
    PreferredWeapon(d, legacy_weapon::Axe);
    d.creation.bodyModels = BodyModels("b");
    d.creation.faces.push_back(Face("char//tmABCD//tmh-b01.inf", "char//tmABCD//tmh-b01a.inf", "TmB01-01.tga", "TmB01-02.tga"));
    d.creation.faces.push_back(Face("char//tmABCD//tmh-b02.inf", "char//tmABCD//tmh-b02a.inf", "TmB02-01.tga", "TmB02-02.tga"));
    d.creation.faces.push_back(Face("char//tmABCD//tmh-b03.inf", "char//tmABCD//tmh-b03a.inf", "TmB03-01.tga", "TmB03-02.tga"));
    Presentation(d, "T_cls-ft_.tga", "T_cls-ft.tga", "T_chr-ft.tga", "*Special_Message8_Fighter", "[Mestre dos Lutadores]", "game//ranking//Class//fs.bmp");
    d.ranks.push_back(Rank(1, 0x00000002u, "fighter", "Fighter"));
    d.ranks.push_back(Rank(2, 0x00000020u, "warrior", "Warrior"));
    d.ranks.push_back(Rank(3, 0x00000200u, "champion", "Champion"));
    d.ranks.push_back(Rank(4, 0x00002000u, "immortal_warrior", "Immortal Warrior"));
    d.ranks.push_back(Rank(5, 0x00020000u, "warlord", "Warlord"));
    RankAlias(d, 4, "ImmortalWarrior");
    return d;
}

PlayableClassDefinition Pikeman()
{
    PlayableClassDefinition d = BaseClass(4, 0x00000004u, 1, "pikeman", "Pikeman", 2,
                                          ClassCapability::Tempskron, 2,
                                          CharacterAttributes(26, 9, 20, 19, 25),
                                          Formula(75, 1, 3, 1, 1, 2, 0));
    ItemRequirementRevision(d, 0, 0, 10, 15, -20, -15, 0, 0, -25, -15, 0, 0);
    d.combat.criticalScalingAttribute = CharacterAttribute::Strength;
    d.gender = CharacterGender::Male;
    d.skills.legacyGroupCode = legacy_skill_group::Pikeman;
    d.capabilities |= CapabilityBit(ClassCapability::Melee);
    PreferredWeapon(d, legacy_weapon::Pike);
    d.creation.bodyModels = BodyModels("c");
    d.creation.faces.push_back(Face("char//tmABCD//tmh-c01.inf", "char//tmABCD//tmh-c01a.inf", "TmC01-01.tga", "TmC01-02.tga"));
    d.creation.faces.push_back(Face("char//tmABCD//tmh-c02.inf", "char//tmABCD//tmh-c02a.inf", "TmC02-01.tga", "TmC02-02.tga"));
    d.creation.faces.push_back(Face("char//tmABCD//tmh-c03.inf", "char//tmABCD//tmh-c03a.inf", "TmC03-01.tga", "TmC03-02.tga"));
    Presentation(d, "T_cls-pkm_.tga", "T_cls-pkm.tga", "T_chr-pkm.tga", "*Special_Message8_Pikeman", "[Mestre dos Pikemans]", "game//ranking//Class//ps.bmp");
    d.ranks.push_back(Rank(1, 0x00000004u, "pikeman", "Pikeman"));
    d.ranks.push_back(Rank(2, 0x00000040u, "combatant", "Combatant"));
    d.ranks.push_back(Rank(3, 0x00000400u, "lancer", "Lancer"));
    d.ranks.push_back(Rank(4, 0x00004000u, "lancelot", "Lancelot"));
    d.ranks.push_back(Rank(5, 0x00040000u, "baylolunt", "Baylolunt"));
    return d;
}

PlayableClassDefinition Archer()
{
    PlayableClassDefinition d = BaseClass(3, 0x00000008u, 1, "archer", "Archer", 3,
                                          ClassCapability::Tempskron, 3,
                                          CharacterAttributes(17, 11, 21, 27, 23),
                                          Formula(80, 3, 3, 2, 3, 1, 0));
    ItemRequirementRevision(d, 0, 0, -25, -15, -20, -10, 0, 0, 15, 25, 0, 0);
    d.combat.criticalScalingAttribute = CharacterAttribute::Dexterity;
    d.gender = CharacterGender::Female;
    d.animation.refreshPatternAfterRecordRestore = true;
    d.skills.legacyGroupCode = legacy_skill_group::Archer;
    d.capabilities |= CapabilityBit(ClassCapability::UsesBow) | CapabilityBit(ClassCapability::UsesProjectile);
    d.animation.attackAnchor = ClassAttackAnchor::LeftFoot;
    PreferredWeapon(d, legacy_weapon::Bow);
    d.creation.bodyModels = BodyModels("d");
    d.creation.faces.push_back(Face("char//tmABCD//tfh-D01.inf", "char//tmABCD//tfh-D01a.inf", "TfD01-01.tga", "TfD01-02.tga"));
    d.creation.faces.push_back(Face("char//tmABCD//tfh-d02.inf", "char//tmABCD//tfh-d02a.inf", "TfD02-01.tga", "TfD02-02.tga"));
    d.creation.faces.push_back(Face("char//tmABCD//tfh-d03.inf", "char//tmABCD//tfh-d03a.inf", "TfD03-01.tga", "TfD03-02.tga"));
    Presentation(d, "T_cls-ac_.tga", "T_cls-ac.tga", "T_chr-ac.tga", "*Special_Message8_Archer", "[Mestre dos Arqueiros]", "game//ranking//Class//as.bmp");
    d.ranks.push_back(Rank(1, 0x00000008u, "archer", "Archer"));
    d.ranks.push_back(Rank(2, 0x00000080u, "hunter_master", "Hunter Master"));
    d.ranks.push_back(Rank(3, 0x00000800u, "dions_disciple", "Dion's Disciple"));
    d.ranks.push_back(Rank(4, 0x00008000u, "sagittarion", "Sagittarion"));
    d.ranks.push_back(Rank(5, 0x00080000u, "iskar", "Iskar"));
    RankAlias(d, 2, "HunterMaster");
    return d;
}

PlayableClassDefinition Knight()
{
    PlayableClassDefinition d = BaseClass(6, 0x00010000u, 2, "knight", "Knight", 4,
                                          ClassCapability::Morayion, 0,
                                          CharacterAttributes(26, 13, 17, 19, 24),
                                          Formula(190, 2, 2, 2, 1, 2, 0));
    ItemRequirementRevision(d, 0, 0, 5, 15, -15, -10, 5, 10, -25, -15, 0, 0);
    d.combat.criticalScalingAttribute = CharacterAttribute::Strength;
    d.gender = CharacterGender::Male;
    d.skills.legacyGroupCode = legacy_skill_group::Knight;
    d.capabilities |= CapabilityBit(ClassCapability::Melee);
    PreferredWeapon(d, legacy_weapon::Sword);
    d.creation.bodyModels = BodyModels("ma");
    d.creation.faces.push_back(Face("char//tmABCD//Mmh-A01.inf", "char//tmABCD//Mmh-A01.inf", "Moryon//MmA01-a.tga", "Moryon//MmA01-b.tga"));
    d.creation.faces.push_back(Face("char//tmABCD//Mmh-A02.inf", "char//tmABCD//Mmh-A02.inf", "Moryon//MmA02-a.tga", "Moryon//MmA02-b.tga"));
    d.creation.faces.push_back(Face("char//tmABCD//Mmh-A03.inf", "char//tmABCD//Mmh-A03.inf", "Moryon//MmA03-a.tga", "Moryon//MmA03-b.tga"));
    Presentation(d, "T-cls-knt_.tga", "T-cls-knt.tga", "T_chr-knt.tga", "*Special_Message8_Knight", "[Mestre dos Cavaleiros]", "game//ranking//Class//ks.bmp");
    d.ranks.push_back(Rank(1, 0x00010000u, "knight", "Knight"));
    d.ranks.push_back(Rank(2, 0x00100000u, "paladin", "Paladin"));
    d.ranks.push_back(Rank(3, 0x01000000u, "holy_knight", "HolyKnight"));
    d.ranks.push_back(Rank(4, 0x10000000u, "saint_knight", "SaintKnight"));
    d.ranks.push_back(Rank(5, 0x10010000u, "valiant", "Valiant"));
    return d;
}

PlayableClassDefinition Atalanta()
{
    PlayableClassDefinition d = BaseClass(5, 0x00020000u, 2, "atalanta", "Atalanta", 5,
                                          ClassCapability::Morayion, 1,
                                          CharacterAttributes(23, 15, 19, 19, 23),
                                          Formula(200, 2, 2, 2, 2, 1, 0));
    ItemRequirementRevision(d, 0, 0, -20, -15, -20, -10, 0, 0, 15, 25, 0, 0);
    d.combat.criticalScalingAttribute = CharacterAttribute::Dexterity;
    d.gender = CharacterGender::Female;
    d.animation.refreshPatternAfterRecordRestore = true;
    d.skills.legacyGroupCode = legacy_skill_group::Atalanta;
    d.capabilities |= CapabilityBit(ClassCapability::UsesProjectile);
    d.animation.attackAnchor = ClassAttackAnchor::LeftFoot;
    PreferredWeapon(d, legacy_weapon::Javelin);
    d.creation.bodyModels = BodyModels("mb");
    d.creation.faces.push_back(Face("char//tmABCD//Mfh-B01.inf", "char//tmABCD//Mfh-B01.inf", "Moryon//MfB02-a.tga", "Moryon//MfB02-b.tga"));
    d.creation.faces.push_back(Face("char//tmABCD//Mfh-B02.inf", "char//tmABCD//Mfh-B02.inf", "Moryon//MfB01-a.tga", "Moryon//MfB01-b.tga"));
    d.creation.faces.push_back(Face("char//tmABCD//Mfh-B03.inf", "char//tmABCD//Mfh-B03.inf", "Moryon//MfB03-a.tga", "Moryon//MfB03-b.tga"));
    Presentation(d, "T_cls-atlt_.tga", "T_cls-atlt.tga", "T_chr-atlt.tga", "*Special_Message8_Atalanta", "[Mestre das Atalantas]", "game//ranking//Class//ata.bmp");
    d.ranks.push_back(Rank(1, 0x00020000u, "atalanta", "Atalanta"));
    d.ranks.push_back(Rank(2, 0x00200000u, "valkyrie", "Valkyrie"));
    d.ranks.push_back(Rank(3, 0x02000000u, "brunhild", "Brunhild"));
    d.ranks.push_back(Rank(4, 0x20000000u, "valhalla", "Valhalla"));
    d.ranks.push_back(Rank(5, 0x20020000u, "queen_of_valhalla", "Queen of Valhalla"));
    RankAlias(d, 3, "BrunHild");
    return d;
}

PlayableClassDefinition Priestess()
{
    PlayableClassDefinition d = BaseClass(8, 0x00040000u, 2, "priestess", "Priestess", 6,
                                          ClassCapability::Morayion, 2,
                                          CharacterAttributes(15, 28, 21, 15, 20),
                                          Formula(200, 4, 1, 3, 3, 2, 2));
    ItemRequirementRevision(d, 0, 0, -25, -20, 15, 20, -15, -10, -20, -15, 0, 0);
    d.combat.criticalScalingAttribute = CharacterAttribute::Spirit;
    d.gender = CharacterGender::Female;
    d.skills.legacyGroupCode = legacy_skill_group::Priestess;
    d.animation.motionContentAliases.push_back("Priest");
    // sinWM1 | sin04 (0x01040000 | 0x00000400): preserve the historical
    // Priestess animation timing without a JOB_CODE branch in character.cpp.
    d.animation.itemFrameAdjustments.push_back(ClassAnimationItemFrameAdjustment(0x01040400u, -16));
    d.capabilities |= CapabilityBit(ClassCapability::Caster);
    PreferredWeapon(d, legacy_weapon::Magic);
    d.creation.bodyModels = BodyModels("mc");
    d.creation.faces.push_back(Face("char//tmABCD//Mfh-C01.inf", "char//tmABCD//Mfh-C01.inf", "Moryon//MfC01-a.tga", "Moryon//MfC01-b.tga"));
    d.creation.faces.push_back(Face("char//tmABCD//Mfh-C02.inf", "char//tmABCD//Mfh-C02.inf", "Moryon//MfC02-a.tga", "Moryon//MfC02-b.tga"));
    d.creation.faces.push_back(Face("char//tmABCD//Mfh-C03.inf", "char//tmABCD//Mfh-C03.inf", "Moryon//MfC03-a.tga", "Moryon//MfC03-b.tga"));
    Presentation(d, "T_cls-prts_.tga", "T_cls-prts.tga", "T_chr-prts.tga", "*Special_Message8_Priestess", "[Mestre das Sacerdotisas]", "game//ranking//Class//prs.bmp");
    d.ranks.push_back(Rank(1, 0x00040000u, "priestess", "Priestess"));
    d.ranks.push_back(Rank(2, 0x00400000u, "saintess", "Saintess"));
    d.ranks.push_back(Rank(3, 0x04000000u, "bishop", "Bishop"));
    d.ranks.push_back(Rank(4, 0x40000000u, "celestial", "Celestial"));
    d.ranks.push_back(Rank(5, 0x40040000u, "arc_angel", "Arc Angel"));
    return d;
}

PlayableClassDefinition Magician()
{
    PlayableClassDefinition d = BaseClass(7, 0x00080000u, 2, "magician", "Magician", 7,
                                          ClassCapability::Morayion, 3,
                                          CharacterAttributes(16, 29, 19, 14, 21),
                                          Formula(190, 5, 1, 3, 3, 2, 1));
    ItemRequirementRevision(d, 0, 0, -25, -20, 15, 25, -15, -10, -20, -15, 0, 0);
    d.combat.criticalScalingAttribute = CharacterAttribute::Spirit;
    d.gender = CharacterGender::Male;
    d.animation.refreshPatternAfterRecordRestore = true;
    d.skills.legacyGroupCode = legacy_skill_group::Magician;
    d.capabilities |= CapabilityBit(ClassCapability::Caster);
    d.animation.attackAnchor = ClassAttackAnchor::Head;
    PreferredWeapon(d, legacy_weapon::Magic);
    d.creation.bodyModels = BodyModels("md");
    d.creation.faces.push_back(Face("char//tmABCD//Mmh-D01.inf", "char//tmABCD//Mmh-D01.inf", "Moryon//MmD01-a.tga", "Moryon//MmD01-b.tga"));
    d.creation.faces.push_back(Face("char//tmABCD//Mmh-D02.inf", "char//tmABCD//Mmh-D02.inf", "Moryon//MmD02-a.tga", "Moryon//MmD02-b.tga"));
    d.creation.faces.push_back(Face("char//tmABCD//Mmh-D03.inf", "char//tmABCD//Mmh-D03.inf", "Moryon//MmD03-a.tga", "Moryon//MmD03-b.tga"));
    Presentation(d, "T_cls-mgc_.tga", "T_cls-mgc.tga", "T_chr-mgc.tga", "*Special_Message8_Magician", "[Mestre dos Magos]", "game//ranking//Class//mg.bmp");
    d.ranks.push_back(Rank(1, 0x00080000u, "magician", "Magician"));
    d.ranks.push_back(Rank(2, 0x00800000u, "wizard", "Wizard"));
    d.ranks.push_back(Rank(3, 0x08000000u, "royal_wizard", "RoyalWizard"));
    d.ranks.push_back(Rank(4, 0x80000000u, "arch_mage", "ArchMage"));
    d.ranks.push_back(Rank(5, 0x80080000u, "elemental_master", "Elemental Master"));
    RankAlias(d, 3, "RoyalKnight");
    return d;
}

}

bool LegacyEightClassesContentSource::Load(std::vector<PlayableClassDefinition>& out,
                                           std::string* error) const
{
    out.clear();
    out.push_back(Fighter());
    out.push_back(Mechanician());
    out.push_back(Archer());
    out.push_back(Pikeman());
    out.push_back(Atalanta());
    out.push_back(Knight());
    out.push_back(Magician());
    out.push_back(Priestess());
    if(error) error->clear();
    return true;
}

bool LegacyClassContentSource::Load(std::vector<PlayableClassDefinition>& out,
                                    std::string* error) const
{
    LegacyEightClassesContentSource legacy;
    ReservedClassesContentSource reserved;
    CompositeClassContentSource sources;
    sources.Add(legacy).Add(reserved);
    return sources.Load(out, error);
}

bool RegisterLegacyEightClasses(PlayableClassRegistry& registry)
{
    const PlayableClassDefinition definitions[] = {
        Fighter(), Mechanician(), Archer(), Pikeman(), Atalanta(), Knight(), Magician(), Priestess()
    };
    for (std::size_t i = 0; i < sizeof(definitions) / sizeof(definitions[0]); ++i)
        if (!registry.Register(definitions[i])) return false;
    return true;
}

bool RegisterLegacyClassCatalog(PlayableClassRegistry& registry)
{
    LegacyClassContentSource source;
    std::string error;
    return LoadClassContent(registry, source, &error);
}
}
}
