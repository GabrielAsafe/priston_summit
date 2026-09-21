#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "gameplay/classes/v2/PlayableClassRegistry.h"

namespace gameplay
{
namespace classes
{
struct ClassSpecializationMatch
{
    int legacyJobCode;
    int brood;
    int selectionOrder;
    int rank;
    std::uint32_t jobBitCode;
    std::string displayKey;

    ClassSpecializationMatch()
        : legacyJobCode(0), brood(0), selectionOrder(-1), rank(0), jobBitCode(0u) {}
};

struct CharacterCreationTemplate
{
    PlayableClassId classId;
    int legacyJobCode;
    int brood;
    CharacterAttributes attributes;
    std::string bodyModel;
    std::string faceModel;

    CharacterCreationTemplate() : classId(0), legacyJobCode(0), brood(0) {}
};

class ClassService
{
public:
    explicit ClassService(const PlayableClassRegistry& registry) : registry_(registry) {}

    const PlayableClassDefinition* Find(int legacyJobCode) const;
    const PlayableClassDefinition* FindEnabled(int legacyJobCode) const;
    const PlayableClassDefinition* FindByContentToken(const std::string& token) const;
    std::vector<const PlayableClassDefinition*> EnabledClasses() const;
    std::vector<int> EnabledJobCodes() const;
    std::vector<const PlayableClassDefinition*> SelectableClasses(int brood) const;
    const PlayableClassDefinition* FindSelectableClass(int brood, int selectionOrder) const;

    bool BuildCreationTemplate(int brood,
                               int selectionOrder,
                               std::size_t bodyIndex,
                               std::size_t faceIndex,
                               bool alternateFace,
                               CharacterCreationTemplate& out) const;

    bool TryFormula(int legacyJobCode, CharacterFormulaProfile& out) const;
    bool TryBaseAttributes(int legacyJobCode, CharacterAttributes& out) const;
    bool IsInBrood(int legacyJobCode, int brood) const;
    bool TryArmorModelIndex(int legacyJobCode, int& out) const;
    const std::string* ArmorModel(int legacyJobCode, ArmorVisualVariant variant, std::size_t armorIndex) const;
    bool FindArmorModel(int legacyJobCode, const std::string& fileName, ArmorVisualVariant& variant, std::size_t& armorIndex) const;
    bool IsBodyModelPathAllowed(int legacyJobCode, const std::string& modelPath) const;
    bool TryGender(int legacyJobCode, CharacterGender& out) const;
    bool IsGender(int legacyJobCode, CharacterGender gender) const;
    bool HasCapability(int legacyJobCode, ClassCapability capability) const;
    bool TryCriticalScalingAttribute(int legacyJobCode, CharacterAttribute& out) const;
    bool TryAttackAnchor(int legacyJobCode, ClassAttackAnchor& out) const;
    bool TryMotionJobMask(int legacyJobCode, std::uint32_t& out) const;
    bool TryMotionJobMaskByContentToken(const std::string& token, std::uint32_t& out) const;
    bool ShouldRefreshPatternAfterRecordRestore(int legacyJobCode) const;
    int AttackFrameStepAdjustment(int legacyJobCode, std::uint32_t itemCode) const;
    const std::string* SkillTreeKey(int legacyJobCode) const;
    const std::string* LegacyLevel8HelpMessageKey(int legacyJobCode) const;
    const std::string* LegacyMasterTitle(int legacyJobCode) const;
    const std::string* KillNotificationIcon(int legacyJobCode) const;
    const std::string* PartyIcon(int legacyJobCode, bool alive) const;
    const std::string* SodRankIcon(int legacyJobCode) const;
    const std::string* RankingIcon(int legacyJobCode) const;
    bool TryLegacySkillGroupCode(int legacyJobCode, std::uint32_t& out) const;
    bool UsesLegacyTier5Slots(int legacyJobCode) const;
    bool HasWeaponAffinity(int legacyJobCode, std::uint32_t weaponType, std::uint32_t itemJobMask,
                           int uniqueItemCode, std::uint32_t characterJobBitMask = 0u) const;
    bool TryDefaultWeaponType(int legacyJobCode, std::uint32_t& out) const;
    bool TryItemRequirementRevisionByJobBitCode(
        std::uint32_t jobBitCode, ClassItemRequirementRevisionProfile& out) const;
    std::vector<int> EquipmentModelSlots(int legacyJobCode, int defaultModelSlot) const;
    bool TryJobBitCode(int legacyJobCode, std::uint32_t& out) const;
    bool TryLegacyJobCodeByJobBitCode(std::uint32_t jobBitCode, int& out) const;
    bool TryJobBitCodeByContentToken(const std::string& token, std::uint32_t& out) const;
    bool TryRankBitCode(int legacyJobCode, int rank, std::uint32_t& out) const;
    const ClassRankDefinition* RankForLegacyChangeJob(int legacyJobCode, int changeJob) const;
    int MaxLegacyChangeJob(int legacyJobCode) const;
    bool IsValidLegacyChangeJob(int legacyJobCode, int changeJob) const;
    bool TryNextLegacyChangeJob(int legacyJobCode, int currentChangeJob, int& out) const;
    std::vector<ClassSpecializationMatch> LegacySpecializationMatches(
        std::uint32_t jobMask, int highestRank = 4) const;

    const std::string* HeadModel(int legacyJobCode, std::size_t faceIndex, std::size_t hairIndex) const;
    const std::string* DefaultHeadModel(int legacyJobCode, std::size_t rankIndex) const;
    int PromotionVisualChangeJobIndex(int legacyJobCode, int changeJob) const;
    bool FindHeadModel(int legacyJobCode, const std::string& model, std::size_t& faceIndex, std::size_t& hairIndex) const;
    bool Validate(std::string* error = 0) const;

private:
    const PlayableClassRegistry& registry_;
};

const ClassService& DefaultClassService();
}
}
