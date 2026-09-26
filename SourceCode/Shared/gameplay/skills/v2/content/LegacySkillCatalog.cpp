#include "gameplay/skills/v2/content/LegacySkillCatalog.h"

#include <cstdio>
#include <cstdlib>

#include "gameplay/skills/v2/SkillRegistry.h"

namespace gameplay
{
namespace skills
{
namespace
{

SkillDefinition MakeSkill(SkillId id,
                          int ownerClassId,
                          int useSlot,
                          int tier,
                          SkillBehavior behavior,
                          bool executable,
                          const char* name,
                          std::uint32_t legacyPlayCode,
                          std::uint32_t presentationPlayCode,
                          int finalMotionEvent,
                          bool logicalMotionEvents,
                          const char* actorParticle,
                          const char* targetParticle,
                          bool persistentVisual,
                          int castSound,
                          const char* canonicalTimerPath,
                          const char* fallbackTimerPath)
{
    SkillDefinition definition;
    definition.id = id;
    definition.ownerClassId = ownerClassId;
    definition.useSlot = useSlot;
    definition.tier = tier;
    definition.behavior = behavior;
    definition.executable = executable;
    definition.name = name ? name : "";

    definition.legacy.playCode = legacyPlayCode;
    definition.legacy.presentationPlayCode =
        presentationPlayCode ? presentationPlayCode : legacyPlayCode;
    definition.legacy.finalMotionEvent = finalMotionEvent;
    definition.legacy.logicalMotionEvents = logicalMotionEvents;

    definition.presentation.actorParticle = actorParticle ? actorParticle : "";
    definition.presentation.targetParticle = targetParticle ? targetParticle : "";
    definition.presentation.persistentVisual = persistentVisual;
    definition.presentation.castSound = castSound;
    definition.presentation.canonicalTimerPath = canonicalTimerPath ? canonicalTimerPath : "";
    definition.presentation.fallbackTimerPath = fallbackTimerPath ? fallbackTimerPath : "";
    return definition;
}

}

std::vector<SkillDefinition> LegacySkillCatalog()
{
    std::vector<SkillDefinition> definitions;
    definitions.reserve(160);
    definitions.push_back(MakeSkill(0x01010001u, 2, 1, 1, SkillBehavior::BuffSelf, true, "Extreme Shield", 0x01u, 0x01u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x01010002u, 2, 2, 1, SkillBehavior::DamageArea, true, "Mechanic Bomb", 0x02u, 0x02u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x01010003u, 2, 3, 1, SkillBehavior::BuffSelf, true, "Physical Absorption", 0x03u, 0x03u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x01010004u, 2, 4, 1, SkillBehavior::Passive, false, "Poison Resistance", 0x00u, 0x00u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x01020001u, 2, 5, 2, SkillBehavior::DamageSingle, true, "Great Smash", 0x05u, 0x05u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x01020002u, 2, 6, 2, SkillBehavior::BuffSelf, true, "Maximize", 0x06u, 0x06u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x01020003u, 2, 7, 2, SkillBehavior::BuffSelf, true, "Automation", 0x07u, 0x07u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x01020004u, 2, 8, 2, SkillBehavior::DamageArea, true, "Spark", 0x08u, 0x08u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x01040001u, 2, 9, 3, SkillBehavior::BuffSelf, true, "Metal Armor", 0x09u, 0x09u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x01040002u, 2, 10, 3, SkillBehavior::DamageSingle, true, "Grand Smash", 0x0Au, 0x0Au, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x01040003u, 2, 11, 3, SkillBehavior::Passive, false, "Mechanic Weapon Mastery", 0x00u, 0x00u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x01040004u, 2, 12, 3, SkillBehavior::Reactive, true, "Spark Shield", 0x0Bu, 0x0Bu, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x01080001u, 2, 13, 4, SkillBehavior::DamageArea, true, "Impulsion", 0x0Cu, 0x0Cu, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x01080002u, 2, 14, 4, SkillBehavior::BuffGroup, true, "Compulsion", 0x0Du, 0x0Du, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x01080003u, 2, 15, 4, SkillBehavior::BuffSelf, true, "Magnetic Sphere", 0x0Eu, 0x0Eu, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x01080004u, 2, 16, 4, SkillBehavior::Summon, true, "Metal Golem", 0x0Fu, 0x0Fu, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x02010001u, 1, 1, 1, SkillBehavior::Passive, false, "Weapon Mastery", 0x00u, 0x00u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x02010002u, 1, 2, 1, SkillBehavior::Passive, false, "Fire Resistance", 0x00u, 0x00u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x02010003u, 1, 3, 1, SkillBehavior::DamageSingle, true, "Raving", 0x23u, 0x23u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x02010004u, 1, 4, 1, SkillBehavior::DamageSingle, true, "Impact", 0x24u, 0x24u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x02020001u, 1, 5, 2, SkillBehavior::DamageSingle, true, "Triple Impact", 0x25u, 0x25u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x02020002u, 1, 6, 2, SkillBehavior::DamageSingle, true, "Brutal Swing", 0x26u, 0x26u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x02020003u, 1, 7, 2, SkillBehavior::Control, true, "Roar", 0x27u, 0x27u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x02020004u, 1, 8, 2, SkillBehavior::DamageArea, true, "Rage of Zecram", 0x28u, 0x28u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x02040001u, 1, 9, 3, SkillBehavior::BuffSelf, true, "Concentration", 0x29u, 0x29u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x02040002u, 1, 10, 3, SkillBehavior::DamageSingle, true, "Avenging Crash", 0x2Au, 0x2Au, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x02040003u, 1, 11, 3, SkillBehavior::BuffSelf, true, "Swift Axe", 0x2Bu, 0x2Bu, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x02040004u, 1, 12, 3, SkillBehavior::DamageSingle, true, "Bone Crash", 0x2Cu, 0x2Cu, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x02080001u, 1, 13, 4, SkillBehavior::DamageSingle, true, "Destroyer", 0x2Du, 0x2Du, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x02080002u, 1, 14, 4, SkillBehavior::BuffSelf, true, "Berserker", 0x2Eu, 0x2Eu, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x02080003u, 1, 15, 4, SkillBehavior::DamageArea, true, "Cyclone Strike", 0x2Fu, 0x2Fu, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x02080004u, 1, 16, 4, SkillBehavior::Passive, false, "Boost Health", 0x00u, 0x00u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x03010001u, 4, 1, 1, SkillBehavior::Control, true, "Pike Wind", 0x12u, 0x12u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x03010002u, 4, 2, 1, SkillBehavior::Passive, false, "Ice Resistance", 0x00u, 0x00u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x03010003u, 4, 3, 1, SkillBehavior::DamageSingle, true, "Critical Hit", 0x13u, 0x13u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x03010004u, 4, 4, 1, SkillBehavior::DamageSingle, true, "Jumping Crash", 0x14u, 0x14u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x03020001u, 4, 5, 2, SkillBehavior::DamageArea, true, "Ground Pike", 0x15u, 0x15u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x03020002u, 4, 6, 2, SkillBehavior::DamageArea, true, "Tornado", 0x16u, 0x16u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x03020003u, 4, 7, 2, SkillBehavior::Passive, false, "Weapon Defence Mastery", 0x00u, 0x00u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x03020004u, 4, 8, 2, SkillBehavior::DamageArea, true, "Expansion", 0x18u, 0x18u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x03040001u, 4, 9, 3, SkillBehavior::DamageArea, true, "Venom Spear", 0x19u, 0x19u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x03040002u, 4, 10, 3, SkillBehavior::BuffSelf, true, "Vanish", 0x1Au, 0x1Au, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x03040003u, 4, 11, 3, SkillBehavior::Passive, false, "Critical Mastery", 0x00u, 0x00u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x03040004u, 4, 12, 3, SkillBehavior::DamageSingle, true, "Chain Lance", 0x1Bu, 0x1Bu, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x03080001u, 4, 13, 4, SkillBehavior::DebuffSingle, true, "Assassin Eye", 0x1Cu, 0x1Cu, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x03080002u, 4, 14, 4, SkillBehavior::DamageSingle, true, "Charging Strike", 0x1Du, 0x1Du, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x03080003u, 4, 15, 4, SkillBehavior::BuffSelf, true, "Vague", 0x1Eu, 0x1Eu, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x03080004u, 4, 16, 4, SkillBehavior::DamageSingle, true, "Shadow Master", 0x1Fu, 0x1Fu, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x04010001u, 3, 1, 1, SkillBehavior::BuffSelf, true, "Scout Hawk", 0x41u, 0x41u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x04010002u, 3, 2, 1, SkillBehavior::Passive, false, "Shooting Mastery", 0x00u, 0x00u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x04010003u, 3, 3, 1, SkillBehavior::DamageSingle, true, "Wind Arrow", 0x43u, 0x43u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x04010004u, 3, 4, 1, SkillBehavior::DamageSingle, true, "Perfect Aim", 0x44u, 0x44u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x04020001u, 3, 5, 2, SkillBehavior::Passive, false, "Dions Eye", 0x00u, 0x00u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x04020002u, 3, 6, 2, SkillBehavior::Summon, true, "Falcon", 0x46u, 0x46u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x04020003u, 3, 7, 2, SkillBehavior::DamageArea, true, "Arrow of Rage", 0x47u, 0x47u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x04020004u, 3, 8, 2, SkillBehavior::DamageSingle, true, "Avalanche", 0x48u, 0x48u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x04040001u, 3, 9, 3, SkillBehavior::DamageSingle, true, "Elemental Shot", 0x49u, 0x49u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x04040002u, 3, 10, 3, SkillBehavior::Summon, true, "Golden Falcon", 0x4Au, 0x4Au, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x04040003u, 3, 11, 3, SkillBehavior::DamageArea, true, "Bomb Shot", 0x4Bu, 0x4Bu, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x04040004u, 3, 12, 3, SkillBehavior::DamageArea, true, "Perforation", 0x4Cu, 0x4Cu, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x04080001u, 3, 13, 4, SkillBehavior::Summon, true, "Recall Wolverine", 0x4Du, 0x4Du, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x04080002u, 3, 14, 4, SkillBehavior::Passive, false, "Evasion Mastery", 0x00u, 0x00u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x04080003u, 3, 15, 4, SkillBehavior::DamageArea, true, "Phoenix Shot", 0x4Eu, 0x4Eu, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x04080004u, 3, 16, 4, SkillBehavior::BuffGroup, true, "Force of Nature", 0x4Fu, 0x4Fu, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x05010001u, 6, 1, 1, SkillBehavior::DamageArea, true, "Sword Blast", 0x51u, 0x51u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x05010002u, 6, 2, 1, SkillBehavior::BuffSelf, true, "Holy Body", 0x52u, 0x52u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x05010003u, 6, 3, 1, SkillBehavior::Passive, false, "Physical Training", 0x00u, 0x00u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x05010004u, 6, 4, 1, SkillBehavior::DamageSingle, true, "Double Crash", 0x54u, 0x54u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x05020001u, 6, 5, 2, SkillBehavior::BuffGroup, true, "Holy Valor", 0x55u, 0x55u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x05020002u, 6, 6, 2, SkillBehavior::DamageArea, true, "Brandish", 0x56u, 0x56u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x05020003u, 6, 7, 2, SkillBehavior::DamageArea, true, "Piercing", 0x57u, 0x57u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x05020004u, 6, 8, 2, SkillBehavior::BuffSelf, true, "Drastic Spirit", 0x58u, 0x58u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x05040001u, 6, 9, 3, SkillBehavior::Passive, false, "Sword Mastery", 0x00u, 0x00u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x05040002u, 6, 10, 3, SkillBehavior::BuffSelf, true, "Divine Inhalation", 0x5Au, 0x5Au, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x05040003u, 6, 11, 3, SkillBehavior::DebuffSingle, true, "Holy Incantation", 0x5Bu, 0x5Bu, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x05040004u, 6, 12, 3, SkillBehavior::DamageSingle, true, "Grand Cross", 0x5Cu, 0x5Cu, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x05080001u, 6, 13, 4, SkillBehavior::DamageArea, true, "Sword of Justice", 0x50u, 0x50u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x05080002u, 6, 14, 4, SkillBehavior::BuffSelf, true, "Godly Shield", 0x5Eu, 0x5Eu, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x05080003u, 6, 15, 4, SkillBehavior::BuffGroup, true, "God Bless", 0x5Fu, 0x5Fu, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x05080004u, 6, 16, 4, SkillBehavior::DamageSingle, true, "Divine Piercing", 0x5Du, 0x5Du, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x06010001u, 5, 1, 1, SkillBehavior::Control, true, "Shield Strike", 0x61u, 0x61u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x06010002u, 5, 2, 1, SkillBehavior::DamageSingle, true, "Farina", 0x62u, 0x62u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x06010003u, 5, 3, 1, SkillBehavior::Passive, false, "Throwing Mastery", 0x00u, 0x00u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x06010004u, 5, 4, 1, SkillBehavior::DamageSingle, true, "Vigor Spear", 0x64u, 0x64u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x06020001u, 5, 5, 2, SkillBehavior::BuffSelf, true, "Windy", 0x65u, 0x65u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x06020002u, 5, 6, 2, SkillBehavior::DamageSingle, true, "Twist Javelin", 0x66u, 0x66u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x06020003u, 5, 7, 2, SkillBehavior::DamageArea, true, "Soul Sucker", 0x67u, 0x67u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x06020004u, 5, 8, 2, SkillBehavior::DamageSingle, true, "Fire Javelin", 0x68u, 0x68u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x06040001u, 5, 9, 3, SkillBehavior::DamageSingle, true, "Split Javelin", 0x69u, 0x69u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x06040002u, 5, 10, 3, SkillBehavior::BuffGroup, true, "Triumph of Valhalla", 0x6Au, 0x6Au, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x06040003u, 5, 11, 3, SkillBehavior::DamageArea, true, "Lightning Javelin", 0x6Bu, 0x6Bu, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x06040004u, 5, 12, 3, SkillBehavior::DamageArea, true, "Storm Javelin", 0x6Cu, 0x6Cu, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x06080001u, 5, 13, 4, SkillBehavior::BuffGroup, true, "Hall of Valhalla", 0x6Du, 0x6Du, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x06080002u, 5, 14, 4, SkillBehavior::DamageArea, true, "X Rage", 0x6Eu, 0x6Eu, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x06080003u, 5, 15, 4, SkillBehavior::BuffSelf, true, "Frost Javelin", 0x6Fu, 0x6Fu, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x06080004u, 5, 16, 4, SkillBehavior::DamageSingle, true, "Vengeance", 0x60u, 0x60u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x07010001u, 8, 1, 1, SkillBehavior::HealSingle, true, "Healing", 0x71u, 0x71u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x07010002u, 8, 2, 1, SkillBehavior::DamageSingle, true, "Holy Bolt", 0x72u, 0x72u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x07010003u, 8, 3, 1, SkillBehavior::DamageSingle, true, "Multi Spark", 0x73u, 0x73u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x07010004u, 8, 4, 1, SkillBehavior::DebuffSingle, true, "Holy Mind", 0x74u, 0x74u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x07020001u, 8, 5, 2, SkillBehavior::Passive, false, "Meditation", 0x00u, 0x00u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x07020002u, 8, 6, 2, SkillBehavior::DamageArea, true, "Divine Lightning", 0x76u, 0x76u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x07020003u, 8, 7, 2, SkillBehavior::BuffSelf, true, "Holy Reflection", 0x77u, 0x77u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x07020004u, 8, 8, 2, SkillBehavior::HealArea, true, "Grand Healing", 0x78u, 0x78u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x07040001u, 8, 9, 3, SkillBehavior::DamageSingle, true, "Vigor Ball", 0x79u, 0x79u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x07040002u, 8, 10, 3, SkillBehavior::Utility, true, "Resurrection", 0x7Au, 0x7Au, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x07040003u, 8, 11, 3, SkillBehavior::Control, true, "Extinction", 0x7Bu, 0x7Bu, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x07040004u, 8, 12, 3, SkillBehavior::BuffGroup, true, "Virtual Life", 0x7Cu, 0x7Cu, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x07080001u, 8, 13, 4, SkillBehavior::DamageArea, true, "Glacial Spike", 0x7Du, 0x7Du, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x07080002u, 8, 14, 4, SkillBehavior::BuffGroup, true, "Regeneration Field", 0x7Eu, 0x7Eu, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x07080003u, 8, 15, 4, SkillBehavior::DamageArea, true, "Chain Lightning", 0x7Fu, 0x7Fu, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x07080004u, 8, 16, 4, SkillBehavior::Summon, true, "Summon Muspell", 0x70u, 0x70u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x08010001u, 7, 1, 1, SkillBehavior::Resource, true, "Agony", 0x81u, 0x81u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x08010002u, 7, 2, 1, SkillBehavior::DamageSingle, true, "Fire Bolt", 0x82u, 0x82u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x08010003u, 7, 3, 1, SkillBehavior::BuffSelf, true, "Zenith", 0x83u, 0x83u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x08010004u, 7, 4, 1, SkillBehavior::DamageArea, true, "Fire Ball", 0x84u, 0x84u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x08020001u, 7, 5, 2, SkillBehavior::Passive, false, "Mental Mastery", 0x00u, 0x00u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x08020002u, 7, 6, 2, SkillBehavior::DamageArea, true, "Watornado", 0x86u, 0x86u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x08020003u, 7, 7, 2, SkillBehavior::BuffTarget, true, "Enchant Weapon", 0x87u, 0x87u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x08020004u, 7, 8, 2, SkillBehavior::DamageSingle, true, "Dead Ray", 0x88u, 0x88u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x08040001u, 7, 9, 3, SkillBehavior::BuffSelf, true, "Energy Shield", 0x89u, 0x89u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x08040002u, 7, 10, 3, SkillBehavior::DamageArea, true, "Diastrophism", 0x8Au, 0x8Au, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x08040003u, 7, 11, 3, SkillBehavior::BuffSelf, true, "Spirit Elemental", 0x8Bu, 0x8Bu, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x08040004u, 7, 12, 3, SkillBehavior::Summon, true, "Dancing Sword", 0x8Cu, 0x8Cu, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x08080001u, 7, 13, 4, SkillBehavior::Summon, true, "Fire Elemental", 0x8Du, 0x8Du, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x08080002u, 7, 14, 4, SkillBehavior::DamageArea, true, "Flame Wave", 0x8Eu, 0x8Eu, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x08080003u, 7, 15, 4, SkillBehavior::DebuffArea, true, "Distortion", 0x8Fu, 0x8Fu, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x08080004u, 7, 16, 4, SkillBehavior::DamageArea, true, "Meteor", 0x80u, 0x80u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x01100001u, 2, 17, 5, SkillBehavior::DamageSingle, true, "Rupture", 0xE1u, 0xE1u, 3, true, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x01100002u, 2, 18, 5, SkillBehavior::DamageSingle, true, "Swarm Shot", 0xE2u, 0xE2u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x01100003u, 2, 19, 5, SkillBehavior::BuffSelf, true, "Hardened Ward", 0xE3u, 0x0Bu, 0, false, "", "", false, 0, "", "image//Sinimage//skill//keep//Mc3_S_SHIELD.tga"));
    definitions.push_back(MakeSkill(0x01100004u, 2, 20, 5, SkillBehavior::DamageArea, true, "Magnetic Discharge", 0xE4u, 0xE4u, 0, false, "", "", false, 0, "game\\images\\skills\\timers\\Ms5_MagneticDischarge.tga", ""));
    definitions.push_back(MakeSkill(0x02100001u, 1, 17, 5, SkillBehavior::Passive, false, "Survival Instinct", 0x00u, 0x00u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x02100002u, 1, 18, 5, SkillBehavior::DamageArea, true, "Seismic Impact", 0xE2u, 0xE1u, 4, true, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x02100003u, 1, 19, 5, SkillBehavior::BuffSelf, true, "Might of Zecram", 0xE3u, 0x2Eu, 0, false, "", "", false, 0, "", "image//Sinimage//skill//keep//Ft4_BERSERKER.tga"));
    definitions.push_back(MakeSkill(0x02100004u, 1, 20, 5, SkillBehavior::DamageArea, true, "Raging Strikes", 0xE4u, 0xE4u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x03100001u, 4, 17, 5, SkillBehavior::Control, true, "Ring of Spears", 0xE1u, 0x1Cu, 1, true, "", "PS5RingOfSpears", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x03100002u, 4, 18, 5, SkillBehavior::BuffSelf, true, "Death Mantra", 0xE2u, 0x1Eu, 0, false, "", "", false, 0, "", "image//Sinimage//skill//keep//Pk4_VAGUE.tga"));
    definitions.push_back(MakeSkill(0x03100003u, 4, 19, 5, SkillBehavior::BuffSelf, true, "Ex-Death", 0xE3u, 0x1Fu, 0, false, "", "", false, 0, "", "image//Sinimage//skill//Pikeman//Button//TP70 S_Master.bmp"));
    definitions.push_back(MakeSkill(0x03100004u, 4, 20, 5, SkillBehavior::DamageArea, true, "Sinister Spin", 0xE4u, 0x16u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x04100001u, 3, 17, 5, SkillBehavior::BuffSelf, true, "Lethal Sight", 0xE1u, 0x4Fu, 0, false, "Ac5LethalSight", "", true, 17216, "game\\images\\skills\\timers\\Ac5_LethalSight.tga", "image//Sinimage//skill//keep//Ac4_F_O_NATURE.tga"));
    definitions.push_back(MakeSkill(0x04100002u, 3, 18, 5, SkillBehavior::DamageSingle, true, "Fierce Wind", 0xE2u, 0xE1u, 2, true, "Ac5FierceWind", "", false, 18505, "", ""));
    definitions.push_back(MakeSkill(0x04100003u, 3, 19, 5, SkillBehavior::Control, true, "Entaling Roots", 0xE3u, 0xE3u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x04100004u, 3, 20, 5, SkillBehavior::DamageSingle, true, "Bombardment", 0xE4u, 0xE4u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x05100001u, 6, 17, 5, SkillBehavior::BuffGroup, true, "Holy Conviction", 0xE1u, 0x5Fu, 0, false, "", "", true, 9488, "game\\images\\skills\\timers\\Kn5_HolyConviction.tga", "image//Sinimage//skill//keep//Kt4_G_BLESS.tga"));
    definitions.push_back(MakeSkill(0x05100002u, 6, 18, 5, SkillBehavior::DamageArea, true, "Divine Inquisition", 0xE2u, 0xE2u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x05100003u, 6, 19, 5, SkillBehavior::BuffSelf, true, "Glorious Shield", 0xE3u, 0x5Eu, 0, false, "", "", false, 0, "", "image//Sinimage//skill//keep//Kt4_G_SHIELD.tga"));
    definitions.push_back(MakeSkill(0x05100004u, 6, 20, 5, SkillBehavior::DamageSingle, true, "Divine Cross", 0xE4u, 0xE4u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x06100001u, 5, 17, 5, SkillBehavior::Summon, true, "Nature's Call", 0xE1u, 0x6Du, 0, false, "", "", false, 0, "game\\images\\skills\\timers\\Ata5_SummonTiger.tga", "image//Sinimage//skill//keep//At4_H_O_VALHALLA.tga"));
    definitions.push_back(MakeSkill(0x06100002u, 5, 18, 5, SkillBehavior::Passive, false, "Golden Apple", 0x00u, 0x00u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x06100003u, 5, 19, 5, SkillBehavior::DamageArea, true, "Plague Javelin", 0xE3u, 0xE3u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x06100004u, 5, 20, 5, SkillBehavior::DamageSingle, true, "Triple Threat", 0xE4u, 0xE1u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x07100001u, 8, 17, 5, SkillBehavior::Summon, true, "Ice Elemental", 0xE1u, 0x70u, 0, false, "", "", false, 18192, "game\\images\\skills\\timers\\Pr5_IceElemental.tga", "image//Sinimage//skill//keep//Pr4_S_MUSPELL.tga"));
    definitions.push_back(MakeSkill(0x07100002u, 8, 18, 5, SkillBehavior::DamageArea, true, "Lightning Surge", 0xE2u, 0xE2u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x07100003u, 8, 19, 5, SkillBehavior::DebuffArea, true, "Heavenly Light", 0xE3u, 0xE3u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x07100004u, 8, 20, 5, SkillBehavior::BuffGroup, true, "Consecration", 0xE4u, 0x7Eu, 0, false, "", "", false, 0, "", "image//Sinimage//skill//keep//Pr4_R_FIELD.tga"));
    definitions.push_back(MakeSkill(0x08100001u, 7, 17, 5, SkillBehavior::BuffSelf, true, "Wizard's Trance", 0xE1u, 0x8Bu, 1, true, "MG5OrbMastery", "", false, 0, "game\\images\\skills\\timers\\Mg5_WizardTrance.tga", "image//Sinimage//skill//keep//Mg3_S_ELEMENTAL.tga"));
    definitions.push_back(MakeSkill(0x08100002u, 7, 18, 5, SkillBehavior::BuffSelf, true, "Stone Skin", 0xE2u, 0x89u, 0, false, "R5StoneSkin", "", false, 0, "", "image//Sinimage//skill//keep//Mg3_E_SHIELD.tga"));
    definitions.push_back(MakeSkill(0x08100003u, 7, 19, 5, SkillBehavior::DamageSingle, true, "Elemental Ray", 0xE3u, 0xE2u, 0, false, "", "", false, 0, "", ""));
    definitions.push_back(MakeSkill(0x08100004u, 7, 20, 5, SkillBehavior::DamageSingle, true, "Cataclysm", 0xE4u, 0xE4u, 0, false, "", "", false, 0, "", ""));
    return definitions;
}

bool BuildDefaultSkillRegistry(SkillRegistry& registry, std::string* error)
{
    SkillRegistry candidate;
    if(!candidate.RegisterAll(LegacySkillCatalog(), error))
        return false;

    const std::vector<SkillDefinition>& all = candidate.AllSkills();
    if(all.size() != 160u)
    {
        if(error) *error = "default skill catalog must contain exactly 160 T1-T5 skills";
        return false;
    }

    int perTier[5] = {0, 0, 0, 0, 0};
    int active = 0;
    int passive = 0;
    for(std::vector<SkillDefinition>::const_iterator it = all.begin(); it != all.end(); ++it)
    {
        if(it->tier >= 1 && it->tier <= 5)
            ++perTier[it->tier - 1];
        if(it->IsPassive()) ++passive;
        else if(it->IsExecutable()) ++active;
    }

    for(int i = 0; i < 5; ++i)
    {
        if(perTier[i] != 32)
        {
            if(error) *error = "default skill catalog tier cardinality mismatch";
            return false;
        }
    }
    if(active != 142 || passive != 18)
    {
        if(error) *error = "default skill catalog active/passive cardinality mismatch";
        return false;
    }

    registry = candidate;
    if(error) error->clear();
    return true;
}

const SkillRegistry& DefaultSkillRegistry()
{
    static const SkillRegistry registry = []()
    {
        SkillRegistry value;
        std::string error;
        if(!BuildDefaultSkillRegistry(value, &error))
        {
            std::fprintf(stderr, "SkillRegistry startup validation failed: %s\n", error.c_str());
            std::abort();
        }
        return value;
    }();
    return registry;
}

}
}
