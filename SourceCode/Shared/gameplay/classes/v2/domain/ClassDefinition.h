#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace gameplay
{
namespace classes
{
typedef int PlayableClassId;

namespace brood
{
static const int Tempskron = 1;
static const int Morayion = 2;
static const int Sopphetio = 4;
}


enum class CharacterGender
{
    Unspecified = 0,
    Male,
    Female
};

enum class CharacterAttribute
{
    Unspecified = 0,
    Strength,
    Spirit,
    Talent,
    Dexterity,
    Health
};

struct ClassCombatProfile
{
    // Attribute copied into the legacy damage packet's secondary critical
    // field. This used to be selected by a switch over the eight JOB_CODEs.
    CharacterAttribute criticalScalingAttribute;

    ClassCombatProfile() : criticalScalingAttribute(CharacterAttribute::Unspecified) {}

    bool IsDefined() const
    {
        return criticalScalingAttribute != CharacterAttribute::Unspecified;
    }
};

enum class ClassAttackAnchor
{
    Default = 0,
    LeftFoot,
    Head
};

struct ClassAnimationItemFrameAdjustment
{
    std::uint32_t itemCode;
    int frameStepDelta;

    ClassAnimationItemFrameAdjustment() : itemCode(0u), frameStepDelta(0) {}
    ClassAnimationItemFrameAdjustment(std::uint32_t itemCodeValue, int frameStepDeltaValue)
        : itemCode(itemCodeValue), frameStepDelta(frameStepDeltaValue) {}
};

struct ClassAnimationProfile
{
    ClassAttackAnchor attackAnchor;
    // Legacy model/motion applicability mask used by *APPLICABLE_JOB. This is
    // a distinct protocol from item specialization JobBitCode.
    std::uint32_t legacyMotionJobMask;
    std::vector<std::string> motionContentAliases;

    // Some historical models need SetPattern() to be re-applied after a record
    // restore. Keep that compatibility fact in class metadata instead of a
    // JOB_CODE list inside networking code.
    bool refreshPatternAfterRecordRestore;

    // Legacy class+item animation quirks live here so character.cpp does not
    // need to know concrete jobs. New content should normally leave this empty.
    std::vector<ClassAnimationItemFrameAdjustment> itemFrameAdjustments;

    ClassAnimationProfile()
        : attackAnchor(ClassAttackAnchor::Default), legacyMotionJobMask(0u),
          refreshPatternAfterRecordRestore(false) {}
};

struct ClassSkillProfile
{
    // Stable content route owned by the class domain. The legacy group code is
    // only a compatibility value consumed by the existing sinSkill runtime.
    std::string treeKey;
    std::uint32_t legacyGroupCode;
    bool usesLegacyTier5Slots;

    ClassSkillProfile() : legacyGroupCode(0u), usesLegacyTier5Slots(false) {}

    bool IsDefined() const
    {
        return !treeKey.empty();
    }
};

enum class ArmorVisualVariant
{
    Normal = 0,
    Comic,
    NewSet,
    NewSet2
};

struct ClassEquipmentProfile
{
    // Legacy item family masks used only at the compatibility boundary.
    // New content can list one or more affinity families without adding JOB_CODE switches.
    std::vector<std::uint32_t> preferredWeaponTypes;
    std::uint32_t defaultWeaponType;
    bool affinityUsesItemSpecialization;
    int affinityUniqueItemCode;

    // Optional compatibility routing for character-model slots. Generic classes
    // leave this empty and the item's SetModelPosi remains authoritative. A
    // future class may explicitly opt into multiple visual slots without adding
    // JOB_CODE branches to inventory code.
    std::vector<int> visualModelSlots;

    ClassEquipmentProfile()
        : defaultWeaponType(0u), affinityUsesItemSpecialization(false), affinityUniqueItemCode(0)
    {
    }
};

struct ClassArmorProfile
{
    // Existing classes reuse one historical row. New classes may set this to -1
    // and provide data-driven filenames below.
    int legacyModelIndex;
    std::string modelRoot;
    std::vector<std::string> normalModels;
    std::vector<std::string> comicModels;
    std::vector<std::string> newSetModels;
    std::vector<std::string> newSet2Models;

    ClassArmorProfile() : legacyModelIndex(-1) {}

    bool HasDataDrivenModels() const
    {
        return !normalModels.empty();
    }

    bool IsDefined() const
    {
        if (legacyModelIndex >= 0)
            return true;
        if (modelRoot.empty() || normalModels.empty())
            return false;
        for (std::vector<std::string>::const_iterator it = normalModels.begin(); it != normalModels.end(); ++it)
            if (it->empty()) return false;
        return true;
    }
};
enum class ClassCapability : std::uint32_t
{
    None = 0,
    UsesBow = 1u << 0,
    UsesProjectile = 1u << 1,
    Caster = 1u << 2,
    Melee = 1u << 3,
    Tempskron = 1u << 4,
    Morayion = 1u << 5,
    Sopphetio = 1u << 6
};

inline std::uint32_t CapabilityBit(ClassCapability capability)
{
    return static_cast<std::uint32_t>(capability);
}

struct CharacterAttributes
{
    int strength;
    int spirit;
    int talent;
    int dexterity;
    int health;

    CharacterAttributes()
        : strength(0), spirit(0), talent(0), dexterity(0), health(0)
    {
    }

    CharacterAttributes(int strengthValue,
                        int spiritValue,
                        int talentValue,
                        int dexterityValue,
                        int healthValue)
        : strength(strengthValue), spirit(spiritValue), talent(talentValue),
          dexterity(dexterityValue), health(healthValue)
    {
    }

    bool IsDefined() const
    {
        return strength > 0 && spirit > 0 && talent > 0 && dexterity > 0 && health > 0;
    }
};

struct ClassPercentRange
{
    int minimum;
    int maximum;

    ClassPercentRange() : minimum(0), maximum(0) {}
    ClassPercentRange(int minimumValue, int maximumValue)
        : minimum(minimumValue), maximum(maximumValue) {}

    bool IsActive() const { return minimum != 0; }
};

struct ClassItemRequirementRevisionProfile
{
    // Percentage revisions applied when an item rolls this class as its base
    // specialization. This used to live in a fixed eight-row global table.
    ClassPercentRange level;
    ClassPercentRange strength;
    ClassPercentRange spirit;
    ClassPercentRange talent;
    ClassPercentRange dexterity;
    ClassPercentRange health;

    bool IsDefined() const
    {
        return level.IsActive() || strength.IsActive() || spirit.IsActive() ||
               talent.IsActive() || dexterity.IsActive() || health.IsActive();
    }
};

struct CharacterFormulaProfile
{
    int sight;
    int lifeFunction;
    int manaFunction;
    int staminaFunction;
    int damageFunction[3];

    CharacterFormulaProfile()
        : sight(0), lifeFunction(0), manaFunction(0), staminaFunction(0)
    {
        damageFunction[0] = 0;
        damageFunction[1] = 0;
        damageFunction[2] = 0;
    }

    bool IsDefined() const
    {
        return sight > 0 && lifeFunction > 0 && manaFunction > 0 && staminaFunction > 0;
    }
};

struct ClassFaceOption
{
    std::string model;
    std::string alternateModel;
    std::string previewNormal;
    std::string previewSelected;

    bool IsDefined() const
    {
        return !model.empty() && !previewNormal.empty() && !previewSelected.empty();
    }
};

struct ClassSelectionPresentation
{
    std::string labelNormal;
    std::string labelSelected;
    std::string description;

    bool IsDefined() const
    {
        return !labelNormal.empty() && !labelSelected.empty() && !description.empty();
    }
};


struct ClassPresentationProfile
{
    // Presentation content that belongs to the class but not specifically to
    // character creation/selection. Legacy consumers read it through ClassService.
    std::string legacyLevel8HelpMessageKey;
    std::string legacyMasterTitle;
    std::string killNotificationIcon;
    std::string partyAliveIcon;
    std::string partyDeadIcon;
    std::string sodRankIcon;
    std::string rankingIcon;
};

struct ClassAppearanceProfile
{
    // Maximum ChangeJob index that has a distinct promotion head/skin. A value
    // below zero lets data-driven content derive the cap from defaultHeadModels.
    // The eight legacy classes intentionally cap at ChangeJob 3 (rank 4), even
    // though gameplay progression has a fifth rank.
    int maxPromotionVisualChangeJob;

    // Existing classes can delegate their historical 18x31 head matrix to the
    // legacy bridge. New classes should prefer headModels/defaultHeadModels so
    // no engine switch needs to be extended.
    std::string legacyCatalogKey;
    std::vector<std::string> bodyModelRoots;
    std::vector<std::vector<std::string> > headModels;
    std::vector<std::string> defaultHeadModels;

    ClassAppearanceProfile() : maxPromotionVisualChangeJob(-1) {}

    bool HasDataDrivenModels() const
    {
        return !headModels.empty();
    }

    bool IsDefined() const
    {
        if (bodyModelRoots.empty())
            return false;
        if (!legacyCatalogKey.empty())
            return true;
        if (headModels.empty() || defaultHeadModels.empty())
            return false;
        for (std::vector<std::vector<std::string> >::const_iterator row = headModels.begin();
             row != headModels.end(); ++row)
        {
            if (row->empty())
                return false;
            for (std::vector<std::string>::const_iterator model = row->begin();
                 model != row->end(); ++model)
                if (model->empty()) return false;
        }
        for (std::vector<std::string>::const_iterator model = defaultHeadModels.begin();
             model != defaultHeadModels.end(); ++model)
            if (model->empty()) return false;
        return true;
    }
};

struct CharacterCreationProfile
{
    bool selectable;
    int selectionOrder;
    CharacterAttributes baseAttributes;
    std::vector<std::string> bodyModels;
    std::vector<ClassFaceOption> faces;
    ClassSelectionPresentation presentation;

    CharacterCreationProfile() : selectable(false), selectionOrder(-1) {}

    bool IsDefined() const
    {
        if (!selectable)
            return true;
        if (selectionOrder < 0 || !baseAttributes.IsDefined() || bodyModels.empty() || faces.empty() ||
            !presentation.IsDefined())
            return false;
        for (std::vector<ClassFaceOption>::const_iterator it = faces.begin(); it != faces.end(); ++it)
            if (!it->IsDefined()) return false;
        return true;
    }
};

struct ClassRankDefinition
{
    int rank;
    std::uint32_t legacyJobBitCode;
    std::string key;
    std::string displayKey;
    std::vector<std::string> contentAliases;

    ClassRankDefinition() : rank(0), legacyJobBitCode(0u) {}
};

struct PlayableClassDefinition
{
    PlayableClassId id;
    int legacyJobCode;
    std::uint32_t jobBitCode;
    int brood;
    std::string key;
    std::string displayKey;
    CharacterGender gender;
    bool enabled;
    std::uint32_t capabilities;

    CharacterFormulaProfile formula;
    CharacterCreationProfile creation;
    ClassPresentationProfile presentation;
    ClassAppearanceProfile appearance;
    ClassArmorProfile armor;
    ClassEquipmentProfile equipment;
    ClassItemRequirementRevisionProfile itemRequirementRevision;
    ClassCombatProfile combat;
    ClassAnimationProfile animation;
    ClassSkillProfile skills;
    std::vector<ClassRankDefinition> ranks;

    PlayableClassDefinition()
        : id(0), legacyJobCode(0), jobBitCode(0), brood(0),
          gender(CharacterGender::Unspecified), enabled(false), capabilities(0)
    {
    }

    bool HasCapability(ClassCapability capability) const
    {
        return (capabilities & CapabilityBit(capability)) != 0u;
    }

    const ClassRankDefinition* FindRank(int rank) const
    {
        for (std::vector<ClassRankDefinition>::const_iterator it = ranks.begin(); it != ranks.end(); ++it)
            if (it->rank == rank) return &*it;
        return 0;
    }
};
}
}
