#include <cassert>
#include <cstdint>
#include <string>
#include <vector>
#include "gameplay/classes/v2/content/CompositeClassContentSource.h"
#include "gameplay/classes/v2/content/DefaultClassContent.h"
#include "gameplay/classes/v2/content/LegacyClassCatalog.h"
#include "gameplay/classes/v2/content/ReservedClassCatalog.h"
#include "gameplay/classes/v2/legacy/LegacyJobDataView.h"
#include "gameplay/classes/v2/legacy/LegacyClassCombatAdapter.h"
#include "gameplay/classes/v2/legacy/LegacyClassCastleAdapter.h"

#include "gameplay/classes/v2/LegacyClassAdapter.h"
#include "gameplay/classes/v2/services/ClassService.h"

namespace
{
class StaticClassContentSource : public gameplay::classes::IClassContentSource
{
public:
    explicit StaticClassContentSource(const std::vector<gameplay::classes::PlayableClassDefinition>& definitions)
        : definitions_(definitions), fail_(false)
    {
    }

    StaticClassContentSource(const std::vector<gameplay::classes::PlayableClassDefinition>& definitions,
                             bool fail)
        : definitions_(definitions), fail_(fail)
    {
    }

    virtual bool Load(std::vector<gameplay::classes::PlayableClassDefinition>& out,
                      std::string* error = 0) const
    {
        if (fail_)
        {
            if (error) *error = "synthetic source failure";
            return false;
        }
        out = definitions_;
        if (error) error->clear();
        return true;
    }

private:
    std::vector<gameplay::classes::PlayableClassDefinition> definitions_;
    bool fail_;
};

gameplay::classes::PlayableClassDefinition CustomClass()
{
    using namespace gameplay::classes;
    PlayableClassDefinition custom;
    custom.id = 42;
    custom.legacyJobCode = 42;
    custom.jobBitCode = 0x00000010u;
    custom.brood = 4;
    custom.key = "test_class";
    custom.displayKey = "Test Class";
    custom.skills.treeKey = "test_skills";
    custom.skills.legacyGroupCode = 0x2A000000u;
    custom.armor.modelRoot = "char/test/armor/";
    custom.armor.normalModels.push_back("body000.ini");
    custom.armor.comicModels.push_back("body000-comic.ini");
    custom.gender = CharacterGender::Male;
    custom.enabled = true;
    custom.capabilities = CapabilityBit(ClassCapability::Sopphetio) | CapabilityBit(ClassCapability::Melee);
    custom.equipment.preferredWeaponTypes.push_back(0x01010000u);
    custom.equipment.defaultWeaponType = 0x01010000u;
    custom.equipment.visualModelSlots.push_back(2);
    custom.equipment.visualModelSlots.push_back(4);
    custom.combat.criticalScalingAttribute = CharacterAttribute::Strength;
    custom.animation.attackAnchor = ClassAttackAnchor::LeftFoot;
    custom.animation.legacyMotionJobMask = 0x00000100u;
    custom.formula.sight = 100;
    custom.formula.lifeFunction = 1;
    custom.formula.manaFunction = 1;
    custom.formula.staminaFunction = 1;
    custom.creation.selectable = true;
    custom.creation.selectionOrder = 0;
    custom.creation.baseAttributes = CharacterAttributes(10, 10, 10, 10, 10);
    custom.creation.bodyModels.push_back("char/test/body.ini");
    ClassFaceOption face;
    face.model = "char/test/face.inf";
    face.previewNormal = "test-face-normal.tga";
    face.previewSelected = "test-face-selected.tga";
    custom.creation.faces.push_back(face);
    custom.creation.presentation.labelNormal = "test-label-normal.tga";
    custom.creation.presentation.labelSelected = "test-label-selected.tga";
    custom.creation.presentation.description = "test-description.tga";
    custom.presentation.killNotificationIcon = "game/test/class42.png";
    custom.presentation.partyAliveIcon = "game/test/party42.tga";
    custom.presentation.partyDeadIcon = "game/test/party42_.tga";
    custom.presentation.sodRankIcon = "game/test/sod42.bmp";
    custom.presentation.rankingIcon = "game/test/ranking42.bmp";
    custom.appearance.bodyModelRoots.push_back("char/test/");
    custom.appearance.headModels.push_back(std::vector<std::string>(1, "char/test/face.inf"));
    custom.appearance.defaultHeadModels.push_back("char/test/face.inf");
    ClassRankDefinition rank;
    rank.rank = 1;
    rank.legacyJobBitCode = custom.jobBitCode;
    rank.key = "test_class";
    custom.ranks.push_back(rank);
    return custom;
}
}

int main()
{
    using namespace gameplay::classes;

    // Content composition is independent from the registry. The canonical
    // runtime is currently assembled from proven legacy classes plus reserved
    // identities, but either source can be replaced/augmented independently.
    LegacyEightClassesContentSource legacySource;
    ReservedClassesContentSource reservedSource;
    CompositeClassContentSource composedSource;
    composedSource.Add(legacySource).Add(reservedSource);
    assert(composedSource.SourceCount() == 2u);
    std::vector<PlayableClassDefinition> composedDefinitions;
    std::string compositionError;
    assert(composedSource.Load(composedDefinitions, &compositionError));
    assert(compositionError.empty());
    assert(composedDefinitions.size() == 9u);
    assert(composedDefinitions[8].legacyJobCode == 9);
    assert(!composedDefinitions[8].enabled);

    // A source failure must not partially overwrite the caller's output.
    std::vector<PlayableClassDefinition> sentinelDefinitions(1u, CustomClass());
    StaticClassContentSource failingSource(std::vector<PlayableClassDefinition>(), true);
    CompositeClassContentSource failingComposition;
    failingComposition.Add(legacySource).Add(failingSource);
    assert(!failingComposition.Load(sentinelDefinitions, &compositionError));
    assert(compositionError == "synthetic source failure");
    assert(sentinelDefinitions.size() == 1u);
    assert(sentinelDefinitions[0].legacyJobCode == 42);

    assert(brood::Tempskron == 1);
    assert(brood::Morayion == 2);
    assert(brood::Sopphetio == 4);
    int castleTargetJob = 0;
    assert(gameplay::classes::legacy::TryBlessCastleClassStoneTarget(0x080A0900u, castleTargetJob));
    assert(castleTargetJob == 1);
    assert(gameplay::classes::legacy::TryBlessCastleClassStoneTarget(0x080A1000u, castleTargetJob));
    assert(castleTargetJob == 8);
    assert(!gameplay::classes::legacy::TryBlessCastleClassStoneTarget(0x080A1100u, castleTargetJob));
    assert(castleTargetJob == 0);
    PlayableClassRegistry registry;
    assert(RegisterLegacyEightClasses(registry));
    assert(registry.AllClasses().size() == 8u);
    assert(registry.EnabledClasses().size() == 8u);
    std::string validationError;
    assert(registry.Validate(&validationError));
    assert(validationError.empty());

    // The source->registry boundary is transactional as well. Duplicates or
    // invalid definitions must leave an already-valid registry unchanged.
    PlayableClassRegistry loadedFromSources;
    assert(LoadClassContent(loadedFromSources, composedSource, &compositionError));
    assert(compositionError.empty());
    assert(loadedFromSources.AllClasses().size() == 9u);
    assert(loadedFromSources.EnabledClasses().size() == 8u);

    std::vector<PlayableClassDefinition> duplicatePayload;
    duplicatePayload.push_back(CustomClass());
    PlayableClassDefinition duplicatePayloadClass = CustomClass();
    duplicatePayloadClass.id = 43;
    duplicatePayloadClass.legacyJobCode = 43;
    duplicatePayloadClass.key = "test_class_2";
    duplicatePayloadClass.displayKey = "Test Class 2";
    duplicatePayloadClass.creation.selectionOrder = 1;
    // Intentionally keep jobBitCode identical to CustomClass().
    duplicatePayload.push_back(duplicatePayloadClass);
    StaticClassContentSource duplicateSource(duplicatePayload);
    const std::size_t beforeRejectedLoad = loadedFromSources.AllClasses().size();
    assert(!LoadClassContent(loadedFromSources, duplicateSource, &compositionError));
    assert(!compositionError.empty());
    assert(loadedFromSources.AllClasses().size() == beforeRejectedLoad);

    const std::uint32_t expectedBits[9] = {
        0u, 0x00000002u, 0x00000001u, 0x00000008u, 0x00000004u,
        0x00020000u, 0x00010000u, 0x00080000u, 0x00040000u
    };
    const int expectedBrood[9] = {0, 1, 1, 1, 1, 2, 2, 2, 2};
    const int expectedArmor[9] = {0, 1, 0, 3, 2, 5, 4, 7, 6};
    const std::uint32_t expectedMotionMask[9] = {0u, 0x0001u, 0x0002u, 0x0004u, 0x0008u, 0x0010u, 0x0020u, 0x0040u, 0x0080u};

    LegacyClassAdapter adapter(registry);
    for (int job = 1; job <= 8; ++job)
    {
        const PlayableClassDefinition* definition = registry.FindByLegacyJobCode(job);
        assert(definition != 0);
        assert(definition->id == job);
        assert(definition->enabled);
        assert(definition->jobBitCode == expectedBits[job]);
        assert(definition->brood == expectedBrood[job]);
        assert(definition->armor.legacyModelIndex == expectedArmor[job]);
        assert(definition->animation.legacyMotionJobMask == expectedMotionMask[job]);
        assert(definition->formula.IsDefined());
        assert(definition->creation.IsDefined());
        assert(definition->creation.selectable);
        assert(definition->creation.bodyModels.size() == 7u);
        assert(definition->creation.faces.size() == 3u);
        assert(definition->ranks.size() == 5u);
        assert(definition->gender != CharacterGender::Unspecified);
        assert(definition->armor.IsDefined());

        std::uint32_t bit = 0;
        int value = -1;
        CharacterFormulaProfile formula;
        assert(adapter.TryJobBitCode(job, bit) && bit == expectedBits[job]);
        assert(adapter.TryBrood(job, value) && value == expectedBrood[job]);
        assert(adapter.TryArmorModelIndex(job, value) && value == expectedArmor[job]);
        assert(adapter.TryFormula(job, formula) && formula.IsDefined());
    }

    ClassService service(registry);
    CharacterAttributes baseAttributes;
    assert(service.TryBaseAttributes(2, baseAttributes));
    assert(baseAttributes.strength == 24);
    assert(service.IsInBrood(2, brood::Tempskron));
    assert(!service.IsInBrood(2, brood::Morayion));
    assert(service.IsInBrood(6, brood::Morayion));
    assert(service.FindEnabled(9) == 0);
    std::vector<const PlayableClassDefinition*> tempskron = service.SelectableClasses(brood::Tempskron);
    std::vector<const PlayableClassDefinition*> morayion = service.SelectableClasses(brood::Morayion);
    assert(tempskron.size() == 4u);
    assert(morayion.size() == 4u);
    assert(tempskron[0]->legacyJobCode == 2); // Mechanician selection order is legacy-compatible.
    assert(tempskron[1]->legacyJobCode == 1);
    assert(tempskron[2]->legacyJobCode == 4);
    assert(tempskron[3]->legacyJobCode == 3);
    std::uint32_t contentTokenBit = 0u;
    assert(service.TryJobBitCodeByContentToken("Mechanician", contentTokenBit));
    assert(contentTokenBit == expectedBits[2]);
    assert(service.TryJobBitCodeByContentToken("mechanician", contentTokenBit));
    assert(service.TryJobBitCodeByContentToken("MECHANICIAN", contentTokenBit));
    assert(service.TryJobBitCodeByContentToken("Mechanic Master", contentTokenBit));
    assert(contentTokenBit == 0x00000010u);
    assert(service.TryJobBitCodeByContentToken("holy_knight", contentTokenBit));
    assert(contentTokenBit == 0x01000000u);
    assert(service.TryJobBitCodeByContentToken("MechanicMaster", contentTokenBit));
    assert(contentTokenBit == 0x00000010u);
    assert(service.TryJobBitCodeByContentToken("HeavyMetal", contentTokenBit));
    assert(contentTokenBit == 0x00001000u);
    assert(service.TryJobBitCodeByContentToken("RoyalKnight", contentTokenBit));
    assert(contentTokenBit == 0x08000000u);
    std::uint32_t motionJobMask = 0u;
    assert(service.TryMotionJobMask(1, motionJobMask) && motionJobMask == 0x0001u);
    assert(service.TryMotionJobMask(2, motionJobMask) && motionJobMask == 0x0002u);
    assert(service.TryMotionJobMask(8, motionJobMask) && motionJobMask == 0x0080u);
    assert(service.TryMotionJobMaskByContentToken("Fighter", motionJobMask) && motionJobMask == 0x0001u);
    assert(service.TryMotionJobMaskByContentToken("Priest", motionJobMask) && motionJobMask == 0x0080u);
    assert(service.TryMotionJobMaskByContentToken("Priestess", motionJobMask) && motionJobMask == 0x0080u);
    assert(!service.TryMotionJobMaskByContentToken("Assassine", motionJobMask));
    int jobFromBit = 0;
    assert(service.TryLegacyJobCodeByJobBitCode(0x00000002u, jobFromBit));
    assert(jobFromBit == 1);
    assert(service.TryLegacyJobCodeByJobBitCode(0x00040000u, jobFromBit));
    assert(jobFromBit == 8);
    assert(!service.TryLegacyJobCodeByJobBitCode(0u, jobFromBit));
    assert(service.MaxLegacyChangeJob(1) == 4);
    assert(service.IsValidLegacyChangeJob(1, 0));
    assert(service.IsValidLegacyChangeJob(1, 4));
    assert(!service.IsValidLegacyChangeJob(1, 5));
    assert(!service.IsValidLegacyChangeJob(9, 0));
    const ClassRankDefinition* currentRank = service.RankForLegacyChangeJob(1, 0);
    assert(currentRank && currentRank->rank == 1 && currentRank->key == "fighter");
    currentRank = service.RankForLegacyChangeJob(1, 4);
    assert(currentRank && currentRank->rank == 5 && currentRank->key == "warlord");
    int nextChangeJob = -1;
    assert(service.TryNextLegacyChangeJob(1, 0, nextChangeJob) && nextChangeJob == 1);
    assert(service.TryNextLegacyChangeJob(1, 3, nextChangeJob) && nextChangeJob == 4);
    assert(!service.TryNextLegacyChangeJob(1, 4, nextChangeJob));
    ClassItemRequirementRevisionProfile itemRevision;
    assert(service.TryItemRequirementRevisionByJobBitCode(0x00000002u, itemRevision));
    assert(itemRevision.strength.minimum == 10 && itemRevision.strength.maximum == 15);
    assert(itemRevision.spirit.minimum == -20 && itemRevision.spirit.maximum == -15);
    assert(itemRevision.dexterity.minimum == -20 && itemRevision.dexterity.maximum == -15);
    assert(!service.TryItemRequirementRevisionByJobBitCode(0u, itemRevision));
    assert(!service.TryItemRequirementRevisionByJobBitCode(0xDEADBEEFu, itemRevision));

    // Item specialization presentation reproduces the historical JobDataBase
    // order (brood -> rank -> selection order) while remaining registry-driven.
    const std::vector<ClassSpecializationMatch> specializationMatches =
        service.LegacySpecializationMatches(0x00000002u | 0x00000008u | 0x00000020u);
    assert(specializationMatches.size() == 3u);
    assert(specializationMatches[0].legacyJobCode == 1 && specializationMatches[0].rank == 1);
    assert(specializationMatches[1].legacyJobCode == 3 && specializationMatches[1].rank == 1);
    assert(specializationMatches[2].legacyJobCode == 1 && specializationMatches[2].rank == 2);
    // Rank 5 values are intentionally not added to the item-mask scan. The
    // overlapping 0x00020000 value must still mean Atalanta rank 1 here.
    const std::vector<ClassSpecializationMatch> overlapMatches =
        service.LegacySpecializationMatches(0x00020000u);
    assert(overlapMatches.size() == 1u);
    assert(overlapMatches[0].legacyJobCode == 5 && overlapMatches[0].rank == 1);

    assert(!service.TryJobBitCodeByContentToken("Assassine", contentTokenBit));
    std::uint32_t skillGroup = 0u;
    assert(service.TryLegacySkillGroupCode(2, skillGroup));
    assert(skillGroup == 0x01000000u);
    assert(service.UsesLegacyTier5Slots(2));
    assert(!service.UsesLegacyTier5Slots(9));
    const std::string* skillTree = service.SkillTreeKey(2);
    assert(skillTree != 0 && *skillTree == "mechanician");
    const std::string* partyAlive = service.PartyIcon(1, true);
    const std::string* partyDead = service.PartyIcon(1, false);
    const std::string* sodIcon = service.SodRankIcon(1);
    const std::string* rankingIcon = service.RankingIcon(1);
    const std::string* masterTitle = service.LegacyMasterTitle(1);
    assert(partyAlive && *partyAlive == "game//Party//Class//Fighter.tga");
    assert(partyDead && *partyDead == "game//Party//Class//Fighter_.tga");
    assert(sodIcon && *sodIcon == "image//Sinimage//help//sod_cr-1.bmp");
    assert(rankingIcon && *rankingIcon == "game//ranking//Class//fs.bmp");
    assert(masterTitle && *masterTitle == "[Mestre dos Lutadores]");

    CharacterCreationTemplate creation;
    assert(service.BuildCreationTemplate(1, 0, 0, 0, false, creation));
    assert(creation.legacyJobCode == 2);
    assert(creation.attributes.strength == 24);
    assert(creation.bodyModel == "char//tmABCD//a001.ini");
    assert(creation.faceModel == "char//tmABCD//tmh-a01.inf");

    PlayableClassDefinition duplicate = *registry.FindByLegacyJobCode(1);
    duplicate.id = 99;
    duplicate.key = "duplicate";
    assert(!registry.Register(duplicate));
    assert(!registry.IsEnabled(9));

    // Bulk registration is transactional: one invalid definition must not leave
    // earlier definitions from the same content load committed.
    PlayableClassRegistry transactional;
    std::vector<PlayableClassDefinition> batch;
    PlayableClassDefinition validBatchClass = CustomClass();
    validBatchClass.id = 50;
    validBatchClass.legacyJobCode = 50;
    validBatchClass.key = "batch_valid";
    validBatchClass.displayKey = "Batch Valid";
    validBatchClass.skills.legacyGroupCode = 0x32000000u;
    validBatchClass.creation.selectionOrder = 1;
    batch.push_back(validBatchClass);
    PlayableClassDefinition invalidBatchClass = validBatchClass;
    invalidBatchClass.id = 51;
    invalidBatchClass.legacyJobCode = 51;
    invalidBatchClass.key = "batch_invalid";
    invalidBatchClass.jobBitCode = 0u;
    batch.push_back(invalidBatchClass);
    std::string bulkError;
    assert(!transactional.RegisterAll(batch, &bulkError));
    assert(!bulkError.empty());
    assert(transactional.AllClasses().empty());

    // Parser-facing content tokens must be unambiguous after normalization.
    // Aliases may repeat a token only when they resolve to the exact same bit.
    PlayableClassRegistry ambiguousTokens;
    PlayableClassDefinition tokenA = CustomClass();
    tokenA.id = 60;
    tokenA.legacyJobCode = 60;
    tokenA.key = "token_a";
    tokenA.displayKey = "Token A";
    tokenA.jobBitCode = 0x00000040u;
    tokenA.ranks[0].legacyJobBitCode = tokenA.jobBitCode;
    tokenA.ranks[0].key = tokenA.key;
    tokenA.ranks[0].contentAliases.push_back("shared_alias");
    tokenA.skills.legacyGroupCode = 0x3C000000u;
    assert(ambiguousTokens.Register(tokenA));

    PlayableClassDefinition tokenB = CustomClass();
    tokenB.id = 61;
    tokenB.legacyJobCode = 61;
    tokenB.key = "token_b";
    tokenB.displayKey = "Token B";
    tokenB.jobBitCode = 0x00000080u;
    tokenB.ranks[0].legacyJobBitCode = tokenB.jobBitCode;
    tokenB.ranks[0].key = tokenB.key;
    tokenB.ranks[0].contentAliases.push_back("Shared-Alias");
    tokenB.skills.legacyGroupCode = 0x3D000000u;
    tokenB.creation.selectionOrder = 1;
    assert(ambiguousTokens.Register(tokenB));
    std::string ambiguousError;
    assert(!ambiguousTokens.Validate(&ambiguousError));
    assert(!ambiguousError.empty());

    PlayableClassRegistry inconsistentBaseRank;
    PlayableClassDefinition badBaseRank = CustomClass();
    badBaseRank.ranks[0].legacyJobBitCode = 0x00000020u;
    assert(inconsistentBaseRank.Register(badBaseRank));
    std::string inconsistentError;
    assert(!inconsistentBaseRank.Validate(&inconsistentError));
    assert(!inconsistentError.empty());

    PlayableClassRegistry nonContiguousRanks;
    PlayableClassDefinition badRankChain = CustomClass();
    ClassRankDefinition thirdRank;
    thirdRank.rank = 3;
    thirdRank.legacyJobBitCode = 0x00001000u;
    thirdRank.key = "test_class_rank3";
    badRankChain.ranks.push_back(thirdRank);
    assert(nonContiguousRanks.Register(badRankChain));
    assert(!nonContiguousRanks.Validate(&inconsistentError));
    assert(!inconsistentError.empty());

    PlayableClassRegistry inconsistentBrood;
    PlayableClassDefinition badBrood = CustomClass();
    badBrood.capabilities &= ~CapabilityBit(ClassCapability::Sopphetio);
    badBrood.capabilities |= CapabilityBit(ClassCapability::Tempskron);
    assert(inconsistentBrood.Register(badBrood));
    assert(!inconsistentBrood.Validate(&inconsistentError));
    assert(!inconsistentError.empty());

    assert(RegisterReservedAssassinClass(registry));
    assert(registry.AllClasses().size() == 9u);
    assert(registry.EnabledClasses().size() == 8u);
    const PlayableClassDefinition* assassin = registry.FindByLegacyJobCode(9);
    assert(assassin != 0);
    assert(assassin->id == 9);
    assert(assassin->brood == 1);
    assert(assassin->jobBitCode == 0u);
    assert(assassin->armor.legacyModelIndex == -1);
    assert(!assassin->enabled);
    assert(!assassin->creation.selectable);
    assert(assassin->HasCapability(ClassCapability::Tempskron));
    std::uint32_t unresolvedBit = 0;
    int unresolvedArmor = 0;
    assert(!adapter.TryJobBitCode(9, unresolvedBit));
    assert(!adapter.TryArmorModelIndex(9, unresolvedArmor));

    const PlayableClassRegistry& canonical = DefaultPlayableClassRegistry();
    assert(canonical.AllClasses().size() == 9u);
    assert(canonical.EnabledClasses().size() == 8u);
    assert(canonical.FindByLegacyJobCode(9) != 0);
    assert(!canonical.IsEnabled(9));
    assert(canonical.Validate(&validationError));

    PlayableClassRegistry rebuiltDefault;
    assert(BuildDefaultPlayableClassRegistry(rebuiltDefault, &validationError));
    assert(validationError.empty());
    assert(rebuiltDefault.AllClasses().size() == canonical.AllClasses().size());
    assert(rebuiltDefault.EnabledClasses().size() == canonical.EnabledClasses().size());

    // Proof that new content can be registered without editing legacy selection,
    // formulas, or armor mapping code: all required metadata lives in one definition.
    PlayableClassRegistry isolated;
    PlayableClassDefinition custom = CustomClass();
    assert(isolated.Register(custom));
    assert(isolated.Validate(&validationError));
    ClassService isolatedService(isolated);
    assert(isolatedService.BuildCreationTemplate(4, 0, 0, 0, false, creation));
    assert(creation.legacyJobCode == 42);
    assert(creation.bodyModel == "char/test/body.ini");
    const std::string* customHead = isolatedService.HeadModel(42, 0u, 0u);
    assert(customHead && *customHead == "char/test/face.inf");
    std::size_t foundFace = 99u, foundHair = 99u;
    assert(isolatedService.FindHeadModel(42, "CHAR/TEST/FACE.INF", foundFace, foundHair));
    assert(foundFace == 0u && foundHair == 0u);
    assert(isolatedService.DefaultHeadModel(42, 0u) != 0);
    assert(isolatedService.IsGender(42, CharacterGender::Male));
    const std::string* killIcon = isolatedService.KillNotificationIcon(42);
    assert(killIcon && *killIcon == "game/test/class42.png");
    assert(isolatedService.PartyIcon(42, true) && *isolatedService.PartyIcon(42, true) == "game/test/party42.tga");
    assert(isolatedService.PartyIcon(42, false) && *isolatedService.PartyIcon(42, false) == "game/test/party42_.tga");
    assert(isolatedService.SodRankIcon(42) && *isolatedService.SodRankIcon(42) == "game/test/sod42.bmp");
    assert(isolatedService.RankingIcon(42) && *isolatedService.RankingIcon(42) == "game/test/ranking42.bmp");
    assert(service.KillNotificationIcon(9) == 0);
    assert(service.RankingIcon(9) == 0);
    assert(isolatedService.IsBodyModelPathAllowed(42, "CHAR/TEST/BODY.INI"));
    assert(isolatedService.IsBodyModelPathAllowed(42, "CHAR/TEST/ARMOR/BODY000.INI"));
    assert(!isolatedService.IsBodyModelPathAllowed(42, "char/other/body.ini"));
    assert(!isolatedService.TryArmorModelIndex(42, unresolvedArmor));
    const std::string* customArmor = isolatedService.ArmorModel(42, ArmorVisualVariant::Normal, 0u);
    assert(customArmor && *customArmor == "body000.ini");
    ArmorVisualVariant foundVariant = ArmorVisualVariant::Normal;
    std::size_t foundArmor = 99u;
    assert(isolatedService.FindArmorModel(42, "BODY000-COMIC.INI", foundVariant, foundArmor));
    assert(foundVariant == ArmorVisualVariant::Comic && foundArmor == 0u);
    assert(isolatedService.HasWeaponAffinity(42, 0x01010000u, 0u, 0));
    assert(!isolatedService.HasWeaponAffinity(42, 0x01060000u, 0u, 0));
    std::uint32_t defaultWeaponType = 0u;
    assert(isolatedService.TryDefaultWeaponType(42, defaultWeaponType));
    assert(defaultWeaponType == 0x01010000u);
    assert(service.TryDefaultWeaponType(2, defaultWeaponType));
    assert(defaultWeaponType == 0x01020000u);
    const std::vector<int> customSlots = isolatedService.EquipmentModelSlots(42, 7);
    assert(customSlots.size() == 2u && customSlots[0] == 2 && customSlots[1] == 4);
    const std::vector<int> fighterSlots = service.EquipmentModelSlots(1, 7);
    assert(fighterSlots.size() == 1u && fighterSlots[0] == 7);
    assert(service.EquipmentModelSlots(9, 7).size() == 1u);
    assert(isolatedService.HasCapability(42, ClassCapability::Melee));
    CharacterAttribute criticalAttribute = CharacterAttribute::Unspecified;
    assert(isolatedService.TryCriticalScalingAttribute(42, criticalAttribute));
    assert(criticalAttribute == CharacterAttribute::Strength);
    assert(service.TryCriticalScalingAttribute(8, criticalAttribute));
    assert(criticalAttribute == CharacterAttribute::Spirit);
    assert(service.TryCriticalScalingAttribute(3, criticalAttribute));
    assert(criticalAttribute == CharacterAttribute::Dexterity);
    int criticalValue = 0;
    assert(gameplay::classes::legacy::ResolveCriticalScalingValue(
        service, 8, 11, 22, 33, 44, 55, criticalValue));
    assert(criticalValue == 22);
    assert(gameplay::classes::legacy::ResolveCriticalScalingValue(
        service, 3, 11, 22, 33, 44, 55, criticalValue));
    assert(criticalValue == 44);
    assert(gameplay::classes::legacy::ResolveCriticalScalingValue(
        service, 1, 11, 22, 33, 44, 55, criticalValue));
    assert(criticalValue == 11);
    assert(!gameplay::classes::legacy::ResolveCriticalScalingValue(
        service, 9, 11, 22, 33, 44, 55, criticalValue));
    ClassAttackAnchor anchor = ClassAttackAnchor::Default;
    assert(isolatedService.TryAttackAnchor(42, anchor) && anchor == ClassAttackAnchor::LeftFoot);
    assert(isolatedService.TryMotionJobMask(42, motionJobMask) && motionJobMask == 0x00000100u);
    assert(isolatedService.TryMotionJobMaskByContentToken("Test Class", motionJobMask) && motionJobMask == 0x00000100u);
    assert(!isolatedService.ShouldRefreshPatternAfterRecordRestore(42));
    assert(service.ShouldRefreshPatternAfterRecordRestore(3));
    assert(service.ShouldRefreshPatternAfterRecordRestore(5));
    assert(service.ShouldRefreshPatternAfterRecordRestore(7));
    assert(!service.ShouldRefreshPatternAfterRecordRestore(8));
    assert(service.AttackFrameStepAdjustment(8, 0x01040400u) == -16);
    assert(service.AttackFrameStepAdjustment(8, 0x01040500u) == 0);
    assert(service.AttackFrameStepAdjustment(1, 0x01040400u) == 0);
    assert(service.PromotionVisualChangeJobIndex(1, 0) == 0);
    assert(service.PromotionVisualChangeJobIndex(1, 3) == 3);
    assert(service.PromotionVisualChangeJobIndex(1, 4) == 3);
    assert(isolatedService.PromotionVisualChangeJobIndex(42, 0) == 0);
    assert(isolatedService.PromotionVisualChangeJobIndex(42, 4) == 0);

    gameplay::classes::legacy::LegacyJobDataView legacyView;
    assert(gameplay::classes::legacy::BuildLegacyJobDataView(service, 1, 0, legacyView));
    assert(legacyView.jobCode == 1 && legacyView.jobBitCode == 0x00000002u);
    assert(legacyView.formula.sight == 70);
    assert(gameplay::classes::legacy::BuildLegacyJobDataView(service, 1, 4, legacyView));
    assert(legacyView.name == "Warlord" && legacyView.jobBitCode == 0x00020000u);
    assert(legacyView.formula.sight == 0);
    assert(gameplay::classes::legacy::BuildLegacyJobDataView(isolatedService, 42, 0, legacyView));
    assert(legacyView.jobCode == 42 && legacyView.jobBitCode == 0x00000010u);
    assert(!gameplay::classes::legacy::BuildLegacyJobDataView(service, 9, 0, legacyView));

    PlayableClassRegistry extendedRankRegistry;
    PlayableClassDefinition extendedRankClass = CustomClass();
    for (int rankNumber = 2; rankNumber <= 6; ++rankNumber)
    {
        ClassRankDefinition extraRank;
        extraRank.rank = rankNumber;
        extraRank.legacyJobBitCode = static_cast<std::uint32_t>(0x00000010u << (rankNumber - 1));
        extraRank.key = std::string("test_class_rank_") + static_cast<char>('0' + rankNumber);
        extraRank.displayKey = extraRank.key;
        extendedRankClass.ranks.push_back(extraRank);
    }
    assert(extendedRankRegistry.Register(extendedRankClass));
    assert(extendedRankRegistry.Validate(&validationError));
    ClassService extendedRankService(extendedRankRegistry);
    assert(extendedRankService.MaxLegacyChangeJob(42) == 5);
    assert(extendedRankService.IsValidLegacyChangeJob(42, 5));
    assert(gameplay::classes::legacy::BuildLegacyJobDataView(extendedRankService, 42, 5, legacyView));
    assert(legacyView.name == "test_class_rank_6");

    return 0;
}
