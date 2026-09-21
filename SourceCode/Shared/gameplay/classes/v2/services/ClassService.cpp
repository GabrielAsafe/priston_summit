#include "gameplay/classes/v2/services/ClassService.h"

#include <algorithm>
#include <cctype>

#include "gameplay/classes/v2/content/DefaultClassContent.h"

namespace gameplay
{
namespace classes
{
namespace
{
std::string NormalizeContentToken(const std::string& value)
{
    std::string normalized;
    normalized.reserve(value.size());
    for (std::string::const_iterator it = value.begin(); it != value.end(); ++it)
    {
        const unsigned char ch = static_cast<unsigned char>(*it);
        if (ch == ' ' || ch == '-' || ch == '_')
            continue;
        normalized.push_back(static_cast<char>(std::tolower(ch)));
    }
    return normalized;
}

bool EqualsCaseInsensitive(const std::string& lhs, const std::string& rhs)
{
    if (lhs.size() != rhs.size())
        return false;
    for (std::size_t i = 0; i < lhs.size(); ++i)
    {
        const unsigned char left = static_cast<unsigned char>(lhs[i]);
        const unsigned char right = static_cast<unsigned char>(rhs[i]);
        if (std::tolower(left) != std::tolower(right))
            return false;
    }
    return true;
}

struct SelectionOrderLess
{
    bool operator()(const PlayableClassDefinition* lhs, const PlayableClassDefinition* rhs) const
    {
        return lhs->creation.selectionOrder < rhs->creation.selectionOrder;
    }
};

struct LegacySpecializationOrderLess
{
    bool operator()(const ClassSpecializationMatch& lhs, const ClassSpecializationMatch& rhs) const
    {
        if (lhs.brood != rhs.brood)
            return lhs.brood < rhs.brood;
        if (lhs.rank != rhs.rank)
            return lhs.rank < rhs.rank;
        if (lhs.selectionOrder != rhs.selectionOrder)
            return lhs.selectionOrder < rhs.selectionOrder;
        return lhs.legacyJobCode < rhs.legacyJobCode;
    }
};
}

const PlayableClassDefinition* ClassService::Find(int legacyJobCode) const
{
    return registry_.FindByLegacyJobCode(legacyJobCode);
}

const PlayableClassDefinition* ClassService::FindEnabled(int legacyJobCode) const
{
    const PlayableClassDefinition* definition = Find(legacyJobCode);
    return definition && definition->enabled ? definition : 0;
}

const PlayableClassDefinition* ClassService::FindByContentToken(const std::string& token) const
{
    const std::string normalized = NormalizeContentToken(token);
    if (normalized.empty())
        return 0;

    const std::vector<PlayableClassDefinition>& definitions = registry_.AllClasses();
    for (std::vector<PlayableClassDefinition>::const_iterator it = definitions.begin();
         it != definitions.end(); ++it)
    {
        if (!it->enabled)
            continue;
        if (NormalizeContentToken(it->key) == normalized ||
            NormalizeContentToken(it->displayKey) == normalized)
            return &*it;
    }
    return 0;
}

std::vector<const PlayableClassDefinition*> ClassService::EnabledClasses() const
{
    std::vector<const PlayableClassDefinition*> result;
    const std::vector<PlayableClassDefinition>& all = registry_.AllClasses();
    for (std::vector<PlayableClassDefinition>::const_iterator it = all.begin();
         it != all.end(); ++it)
        if (it->enabled) result.push_back(&*it);
    return result;
}

std::vector<int> ClassService::EnabledJobCodes() const
{
    std::vector<int> result;
    const std::vector<PlayableClassDefinition>& all = registry_.AllClasses();
    for (std::vector<PlayableClassDefinition>::const_iterator it = all.begin();
         it != all.end(); ++it)
        if (it->enabled) result.push_back(it->legacyJobCode);
    return result;
}

std::vector<const PlayableClassDefinition*> ClassService::SelectableClasses(int brood) const
{
    std::vector<const PlayableClassDefinition*> result;
    const std::vector<PlayableClassDefinition>& all = registry_.AllClasses();
    for (std::vector<PlayableClassDefinition>::const_iterator it = all.begin(); it != all.end(); ++it)
    {
        if (it->enabled && it->brood == brood && it->creation.selectable)
            result.push_back(&*it);
    }
    std::sort(result.begin(), result.end(), SelectionOrderLess());
    return result;
}

const PlayableClassDefinition* ClassService::FindSelectableClass(int brood, int selectionOrder) const
{
    const PlayableClassDefinition* definition = registry_.FindByBroodAndSelectionOrder(brood, selectionOrder);
    if (!definition || !definition->enabled || !definition->creation.selectable)
        return 0;
    return definition;
}

bool ClassService::BuildCreationTemplate(int brood,
                                         int selectionOrder,
                                         std::size_t bodyIndex,
                                         std::size_t faceIndex,
                                         bool alternateFace,
                                         CharacterCreationTemplate& out) const
{
    const PlayableClassDefinition* definition = FindSelectableClass(brood, selectionOrder);
    if (!definition || bodyIndex >= definition->creation.bodyModels.size() ||
        faceIndex >= definition->creation.faces.size())
        return false;

    const ClassFaceOption& face = definition->creation.faces[faceIndex];
    const std::string& selectedFace = alternateFace && !face.alternateModel.empty()
                                          ? face.alternateModel
                                          : face.model;
    if (selectedFace.empty())
        return false;

    out.classId = definition->id;
    out.legacyJobCode = definition->legacyJobCode;
    out.brood = definition->brood;
    out.attributes = definition->creation.baseAttributes;
    out.bodyModel = definition->creation.bodyModels[bodyIndex];
    out.faceModel = selectedFace;
    return true;
}

bool ClassService::TryFormula(int legacyJobCode, CharacterFormulaProfile& out) const
{
    const PlayableClassDefinition* definition = Find(legacyJobCode);
    if (!definition || !definition->enabled || !definition->formula.IsDefined())
        return false;
    out = definition->formula;
    return true;
}

bool ClassService::TryBaseAttributes(int legacyJobCode, CharacterAttributes& out) const
{
    const PlayableClassDefinition* definition = FindEnabled(legacyJobCode);
    if (!definition || !definition->creation.selectable || !definition->creation.baseAttributes.IsDefined())
        return false;
    out = definition->creation.baseAttributes;
    return true;
}

bool ClassService::IsInBrood(int legacyJobCode, int broodValue) const
{
    const PlayableClassDefinition* definition = FindEnabled(legacyJobCode);
    return definition && definition->brood == broodValue;
}

bool ClassService::TryArmorModelIndex(int legacyJobCode, int& out) const
{
    const PlayableClassDefinition* definition = Find(legacyJobCode);
    if (!definition || !definition->enabled || definition->armor.legacyModelIndex < 0)
        return false;
    out = definition->armor.legacyModelIndex;
    return true;
}

const std::string* ClassService::ArmorModel(int legacyJobCode, ArmorVisualVariant variant, std::size_t armorIndex) const
{
    const PlayableClassDefinition* definition = FindEnabled(legacyJobCode);
    if (!definition || !definition->armor.HasDataDrivenModels())
        return 0;

    const std::vector<std::string>* models = &definition->armor.normalModels;
    switch (variant)
    {
    case ArmorVisualVariant::Comic: models = &definition->armor.comicModels; break;
    case ArmorVisualVariant::NewSet: models = &definition->armor.newSetModels; break;
    case ArmorVisualVariant::NewSet2: models = &definition->armor.newSet2Models; break;
    case ArmorVisualVariant::Normal: break;
    }

    // Optional variants fall back to the normal model, matching the legacy
    // expectation that a class can still render when it has no special skin.
    if (armorIndex < models->size() && !(*models)[armorIndex].empty())
        return &(*models)[armorIndex];
    return armorIndex < definition->armor.normalModels.size()
               ? &definition->armor.normalModels[armorIndex]
               : 0;
}

bool ClassService::FindArmorModel(int legacyJobCode, const std::string& fileName,
                                  ArmorVisualVariant& variant, std::size_t& armorIndex) const
{
    const PlayableClassDefinition* definition = FindEnabled(legacyJobCode);
    if (!definition || !definition->armor.HasDataDrivenModels() || fileName.empty())
        return false;

    const std::vector<std::string>* groups[] = {
        &definition->armor.normalModels, &definition->armor.comicModels,
        &definition->armor.newSetModels, &definition->armor.newSet2Models
    };
    const ArmorVisualVariant variants[] = {
        ArmorVisualVariant::Normal, ArmorVisualVariant::Comic,
        ArmorVisualVariant::NewSet, ArmorVisualVariant::NewSet2
    };
    for (std::size_t group = 0; group < 4u; ++group)
    {
        for (std::size_t index = 0; index < groups[group]->size(); ++index)
        {
            if (EqualsCaseInsensitive((*groups[group])[index], fileName))
            {
                variant = variants[group];
                armorIndex = index;
                return true;
            }
        }
    }
    return false;
}

bool ClassService::IsBodyModelPathAllowed(int legacyJobCode, const std::string& modelPath) const
{
    const PlayableClassDefinition* definition = FindEnabled(legacyJobCode);
    if (!definition || modelPath.size() < 4u)
        return false;

    const std::string extension = modelPath.substr(modelPath.size() - 3u);
    if (!EqualsCaseInsensitive(extension, "ini"))
        return false;

    for (std::vector<std::string>::const_iterator root = definition->appearance.bodyModelRoots.begin();
         root != definition->appearance.bodyModelRoots.end(); ++root)
    {
        if (modelPath.size() >= root->size() &&
            EqualsCaseInsensitive(modelPath.substr(0, root->size()), *root))
            return true;
    }

    // Data-driven armor may live outside the historical body root. Treat the
    // configured armor root as part of the class-owned model namespace.
    if (!definition->armor.modelRoot.empty() &&
        modelPath.size() >= definition->armor.modelRoot.size() &&
        EqualsCaseInsensitive(modelPath.substr(0, definition->armor.modelRoot.size()),
                              definition->armor.modelRoot))
        return true;

    return false;
}

bool ClassService::TryGender(int legacyJobCode, CharacterGender& out) const
{
    const PlayableClassDefinition* definition = FindEnabled(legacyJobCode);
    if (!definition || definition->gender == CharacterGender::Unspecified)
        return false;
    out = definition->gender;
    return true;
}

bool ClassService::IsGender(int legacyJobCode, CharacterGender gender) const
{
    CharacterGender value = CharacterGender::Unspecified;
    return TryGender(legacyJobCode, value) && value == gender;
}

bool ClassService::HasCapability(int legacyJobCode, ClassCapability capability) const
{
    const PlayableClassDefinition* definition = FindEnabled(legacyJobCode);
    return definition && definition->HasCapability(capability);
}

bool ClassService::TryCriticalScalingAttribute(int legacyJobCode, CharacterAttribute& out) const
{
    const PlayableClassDefinition* definition = FindEnabled(legacyJobCode);
    if (!definition || !definition->combat.IsDefined())
        return false;
    out = definition->combat.criticalScalingAttribute;
    return true;
}

bool ClassService::TryAttackAnchor(int legacyJobCode, ClassAttackAnchor& out) const
{
    const PlayableClassDefinition* definition = FindEnabled(legacyJobCode);
    if (!definition)
        return false;
    out = definition->animation.attackAnchor;
    return true;
}

bool ClassService::TryMotionJobMask(int legacyJobCode, std::uint32_t& out) const
{
    const PlayableClassDefinition* definition = FindEnabled(legacyJobCode);
    if (!definition || definition->animation.legacyMotionJobMask == 0u)
        return false;
    out = definition->animation.legacyMotionJobMask;
    return true;
}

bool ClassService::TryMotionJobMaskByContentToken(const std::string& token, std::uint32_t& out) const
{
    const std::string normalized = NormalizeContentToken(token);
    if (normalized.empty())
        return false;

    const std::vector<PlayableClassDefinition>& definitions = registry_.AllClasses();
    for (std::vector<PlayableClassDefinition>::const_iterator it = definitions.begin();
         it != definitions.end(); ++it)
    {
        if (!it->enabled || it->animation.legacyMotionJobMask == 0u)
            continue;
        bool matches = NormalizeContentToken(it->key) == normalized ||
                       NormalizeContentToken(it->displayKey) == normalized;
        if (!matches)
        {
            for (std::vector<std::string>::const_iterator alias = it->animation.motionContentAliases.begin();
                 alias != it->animation.motionContentAliases.end(); ++alias)
            {
                if (NormalizeContentToken(*alias) == normalized)
                {
                    matches = true;
                    break;
                }
            }
        }
        if (matches)
        {
            out = it->animation.legacyMotionJobMask;
            return true;
        }
    }
    return false;
}

bool ClassService::ShouldRefreshPatternAfterRecordRestore(int legacyJobCode) const
{
    const PlayableClassDefinition* definition = FindEnabled(legacyJobCode);
    return definition && definition->animation.refreshPatternAfterRecordRestore;
}

int ClassService::AttackFrameStepAdjustment(int legacyJobCode, std::uint32_t itemCode) const
{
    const PlayableClassDefinition* definition = FindEnabled(legacyJobCode);
    if (!definition || itemCode == 0u)
        return 0;

    for (std::vector<ClassAnimationItemFrameAdjustment>::const_iterator it =
             definition->animation.itemFrameAdjustments.begin();
         it != definition->animation.itemFrameAdjustments.end(); ++it)
    {
        if (it->itemCode == itemCode)
            return it->frameStepDelta;
    }
    return 0;
}

const std::string* ClassService::SkillTreeKey(int legacyJobCode) const
{
    const PlayableClassDefinition* definition = FindEnabled(legacyJobCode);
    if (!definition || !definition->skills.IsDefined())
        return 0;
    return &definition->skills.treeKey;
}

const std::string* ClassService::LegacyLevel8HelpMessageKey(int legacyJobCode) const
{
    const PlayableClassDefinition* definition = FindEnabled(legacyJobCode);
    if (!definition || definition->presentation.legacyLevel8HelpMessageKey.empty())
        return 0;
    return &definition->presentation.legacyLevel8HelpMessageKey;
}

const std::string* ClassService::LegacyMasterTitle(int legacyJobCode) const
{
    const PlayableClassDefinition* definition = FindEnabled(legacyJobCode);
    if (!definition || definition->presentation.legacyMasterTitle.empty())
        return 0;
    return &definition->presentation.legacyMasterTitle;
}

const std::string* ClassService::KillNotificationIcon(int legacyJobCode) const
{
    const PlayableClassDefinition* definition = FindEnabled(legacyJobCode);
    if (!definition || definition->presentation.killNotificationIcon.empty())
        return 0;
    return &definition->presentation.killNotificationIcon;
}

const std::string* ClassService::PartyIcon(int legacyJobCode, bool alive) const
{
    const PlayableClassDefinition* definition = FindEnabled(legacyJobCode);
    if (!definition)
        return 0;
    const std::string& icon = alive ? definition->presentation.partyAliveIcon
                                    : definition->presentation.partyDeadIcon;
    return icon.empty() ? 0 : &icon;
}

const std::string* ClassService::SodRankIcon(int legacyJobCode) const
{
    const PlayableClassDefinition* definition = FindEnabled(legacyJobCode);
    if (!definition || definition->presentation.sodRankIcon.empty())
        return 0;
    return &definition->presentation.sodRankIcon;
}

const std::string* ClassService::RankingIcon(int legacyJobCode) const
{
    const PlayableClassDefinition* definition = FindEnabled(legacyJobCode);
    if (!definition || definition->presentation.rankingIcon.empty())
        return 0;
    return &definition->presentation.rankingIcon;
}

bool ClassService::TryLegacySkillGroupCode(int legacyJobCode, std::uint32_t& out) const
{
    const PlayableClassDefinition* definition = FindEnabled(legacyJobCode);
    if (!definition || definition->skills.legacyGroupCode == 0u)
        return false;
    out = definition->skills.legacyGroupCode;
    return true;
}

bool ClassService::UsesLegacyTier5Slots(int legacyJobCode) const
{
    const PlayableClassDefinition* definition = FindEnabled(legacyJobCode);
    return definition && definition->skills.usesLegacyTier5Slots;
}

bool ClassService::HasWeaponAffinity(int legacyJobCode, std::uint32_t weaponType,
                                     std::uint32_t itemJobMask, int uniqueItemCode,
                                     std::uint32_t characterJobBitMask) const
{
    const PlayableClassDefinition* definition = FindEnabled(legacyJobCode);
    if (!definition)
        return false;

    for (std::vector<std::uint32_t>::const_iterator it = definition->equipment.preferredWeaponTypes.begin();
         it != definition->equipment.preferredWeaponTypes.end(); ++it)
        if (*it == weaponType) return true;

    const std::uint32_t effectiveJobMask = characterJobBitMask != 0u ? characterJobBitMask : definition->jobBitCode;
    if (definition->equipment.affinityUsesItemSpecialization && itemJobMask != 0u &&
        (effectiveJobMask & itemJobMask) != 0u)
        return true;

    return definition->equipment.affinityUniqueItemCode != 0 &&
           definition->equipment.affinityUniqueItemCode == uniqueItemCode;
}

bool ClassService::TryDefaultWeaponType(int legacyJobCode, std::uint32_t& out) const
{
    const PlayableClassDefinition* definition = FindEnabled(legacyJobCode);
    if (!definition || definition->equipment.defaultWeaponType == 0u)
        return false;
    out = definition->equipment.defaultWeaponType;
    return true;
}

std::vector<int> ClassService::EquipmentModelSlots(int legacyJobCode, int defaultModelSlot) const
{
    std::vector<int> result;
    const PlayableClassDefinition* definition = FindEnabled(legacyJobCode);
    if (definition && !definition->equipment.visualModelSlots.empty())
        result = definition->equipment.visualModelSlots;
    else if (defaultModelSlot > 0)
        result.push_back(defaultModelSlot);
    return result;
}

bool ClassService::TryItemRequirementRevisionByJobBitCode(
    std::uint32_t jobBitCode, ClassItemRequirementRevisionProfile& out) const
{
    if (jobBitCode == 0u)
        return false;

    const std::vector<PlayableClassDefinition>& definitions = registry_.AllClasses();
    for (std::vector<PlayableClassDefinition>::const_iterator it = definitions.begin();
         it != definitions.end(); ++it)
    {
        if (it->enabled && it->jobBitCode == jobBitCode && it->itemRequirementRevision.IsDefined())
        {
            out = it->itemRequirementRevision;
            return true;
        }
    }
    return false;
}

bool ClassService::TryJobBitCode(int legacyJobCode, std::uint32_t& out) const
{
    const PlayableClassDefinition* definition = Find(legacyJobCode);
    if (!definition || !definition->enabled || definition->jobBitCode == 0u)
        return false;
    out = definition->jobBitCode;
    return true;
}

bool ClassService::TryLegacyJobCodeByJobBitCode(std::uint32_t jobBitCode, int& out) const
{
    if(jobBitCode == 0u)
        return false;

    const std::vector<PlayableClassDefinition>& definitions = registry_.AllClasses();
    for(std::vector<PlayableClassDefinition>::const_iterator it = definitions.begin();
        it != definitions.end(); ++it)
    {
        if(it->enabled && it->jobBitCode == jobBitCode)
        {
            out = it->legacyJobCode;
            return true;
        }
    }
    return false;
}

bool ClassService::TryJobBitCodeByContentToken(const std::string& token, std::uint32_t& out) const
{
    const std::string normalized = NormalizeContentToken(token);
    if (normalized.empty())
        return false;

    const std::vector<PlayableClassDefinition>& definitions = registry_.AllClasses();
    for (std::vector<PlayableClassDefinition>::const_iterator it = definitions.begin();
         it != definitions.end(); ++it)
    {
        if (!it->enabled)
            continue;
        if ((NormalizeContentToken(it->key) == normalized ||
             NormalizeContentToken(it->displayKey) == normalized) && it->jobBitCode != 0u)
        {
            out = it->jobBitCode;
            return true;
        }
        for (std::vector<ClassRankDefinition>::const_iterator rank = it->ranks.begin();
             rank != it->ranks.end(); ++rank)
        {
            bool rankMatches = NormalizeContentToken(rank->key) == normalized ||
                               NormalizeContentToken(rank->displayKey) == normalized;
            if (!rankMatches)
            {
                for (std::vector<std::string>::const_iterator alias = rank->contentAliases.begin();
                     alias != rank->contentAliases.end(); ++alias)
                {
                    if (NormalizeContentToken(*alias) == normalized)
                    {
                        rankMatches = true;
                        break;
                    }
                }
            }
            if (rankMatches && rank->legacyJobBitCode != 0u)
            {
                out = rank->legacyJobBitCode;
                return true;
            }
        }
    }
    return false;
}

bool ClassService::TryRankBitCode(int legacyJobCode, int rank, std::uint32_t& out) const
{
    const PlayableClassDefinition* definition = Find(legacyJobCode);
    if (!definition || !definition->enabled)
        return false;
    const ClassRankDefinition* rankDefinition = definition->FindRank(rank);
    if (!rankDefinition || rankDefinition->legacyJobBitCode == 0u)
        return false;
    out = rankDefinition->legacyJobBitCode;
    return true;
}

const ClassRankDefinition* ClassService::RankForLegacyChangeJob(int legacyJobCode, int changeJob) const
{
    const PlayableClassDefinition* definition = FindEnabled(legacyJobCode);
    if (!definition || changeJob < 0)
        return 0;
    return definition->FindRank(changeJob + 1);
}

int ClassService::MaxLegacyChangeJob(int legacyJobCode) const
{
    const PlayableClassDefinition* definition = FindEnabled(legacyJobCode);
    if (!definition || definition->ranks.empty())
        return -1;

    int highestRank = 0;
    for (std::vector<ClassRankDefinition>::const_iterator it = definition->ranks.begin();
         it != definition->ranks.end(); ++it)
    {
        if (it->rank > highestRank)
            highestRank = it->rank;
    }
    return highestRank > 0 ? highestRank - 1 : -1;
}

bool ClassService::IsValidLegacyChangeJob(int legacyJobCode, int changeJob) const
{
    return RankForLegacyChangeJob(legacyJobCode, changeJob) != 0;
}

bool ClassService::TryNextLegacyChangeJob(int legacyJobCode, int currentChangeJob, int& out) const
{
    if (!IsValidLegacyChangeJob(legacyJobCode, currentChangeJob))
        return false;
    const int next = currentChangeJob + 1;
    if (!IsValidLegacyChangeJob(legacyJobCode, next))
        return false;
    out = next;
    return true;
}

std::vector<ClassSpecializationMatch> ClassService::LegacySpecializationMatches(
    std::uint32_t jobMask, int highestRank) const
{
    std::vector<ClassSpecializationMatch> result;
    if (jobMask == 0u || highestRank < 1)
        return result;

    // Legacy item tooltips historically scan JobDataBase, which contains ranks
    // 1-4 only. Rank 5 is deliberately excluded because several rank-5 EU
    // values overlap old specialization masks. Keep that protocol quirk at the
    // compatibility boundary rather than making inventory know the table layout.
    if (highestRank > 4)
        highestRank = 4;

    const std::vector<PlayableClassDefinition>& definitions = registry_.AllClasses();
    for (std::vector<PlayableClassDefinition>::const_iterator definition = definitions.begin();
         definition != definitions.end(); ++definition)
    {
        if (!definition->enabled)
            continue;

        for (int rank = 1; rank <= highestRank; ++rank)
        {
            const ClassRankDefinition* rankDefinition = definition->FindRank(rank);
            if (!rankDefinition || rankDefinition->legacyJobBitCode == 0u ||
                (rankDefinition->legacyJobBitCode & jobMask) == 0u)
                continue;

            ClassSpecializationMatch match;
            match.legacyJobCode = definition->legacyJobCode;
            match.brood = definition->brood;
            match.selectionOrder = definition->creation.selectionOrder;
            match.rank = rank;
            match.jobBitCode = rankDefinition->legacyJobBitCode;
            match.displayKey = !rankDefinition->displayKey.empty()
                                   ? rankDefinition->displayKey
                                   : (!rankDefinition->key.empty() ? rankDefinition->key
                                                                   : definition->displayKey);
            result.push_back(match);
        }
    }
    std::sort(result.begin(), result.end(), LegacySpecializationOrderLess());
    return result;
}

const std::string* ClassService::HeadModel(int legacyJobCode, std::size_t faceIndex, std::size_t hairIndex) const
{
    const PlayableClassDefinition* definition = FindEnabled(legacyJobCode);
    if (!definition || faceIndex >= definition->appearance.headModels.size())
        return 0;
    const std::vector<std::string>& row = definition->appearance.headModels[faceIndex];
    return hairIndex < row.size() ? &row[hairIndex] : 0;
}

int ClassService::PromotionVisualChangeJobIndex(int legacyJobCode, int changeJob) const
{
    const PlayableClassDefinition* definition = FindEnabled(legacyJobCode);
    if (!definition || changeJob < 0)
        return 0;

    int maximum = definition->appearance.maxPromotionVisualChangeJob;
    if (maximum < 0 && !definition->appearance.defaultHeadModels.empty())
        maximum = static_cast<int>(definition->appearance.defaultHeadModels.size()) - 1;

    if (maximum >= 0 && changeJob > maximum)
        return maximum;
    return changeJob;
}

const std::string* ClassService::DefaultHeadModel(int legacyJobCode, std::size_t rankIndex) const
{
    const PlayableClassDefinition* definition = FindEnabled(legacyJobCode);
    if (!definition || rankIndex >= definition->appearance.defaultHeadModels.size())
        return 0;
    return &definition->appearance.defaultHeadModels[rankIndex];
}

bool ClassService::FindHeadModel(int legacyJobCode, const std::string& model,
                                 std::size_t& faceIndex, std::size_t& hairIndex) const
{
    const PlayableClassDefinition* definition = FindEnabled(legacyJobCode);
    if (!definition)
        return false;
    for (std::size_t face = 0; face < definition->appearance.headModels.size(); ++face)
    {
        const std::vector<std::string>& row = definition->appearance.headModels[face];
        for (std::size_t hair = 0; hair < row.size(); ++hair)
        {
            if (EqualsCaseInsensitive(row[hair], model))
            {
                faceIndex = face;
                hairIndex = hair;
                return true;
            }
        }
    }
    return false;
}

bool ClassService::Validate(std::string* error) const
{
    return registry_.Validate(error);
}

const ClassService& DefaultClassService()
{
    static const ClassService service(DefaultPlayableClassRegistry());
    return service;
}
}
}
