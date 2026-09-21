#include "gameplay/classes/v2/PlayableClassRegistry.h"

#include <algorithm>
#include <cctype>
#include <map>
#include <sstream>

#include "gameplay/classes/v2/content/IClassContentSource.h"

namespace gameplay
{
namespace classes
{
namespace
{
void SetError(std::string* error, const std::string& value)
{
    if (error) *error = value;
}

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

bool RegisterContentToken(std::map<std::string, std::uint32_t>& owners,
                          const std::string& token,
                          std::uint32_t bit,
                          std::string* error)
{
    const std::string normalized = NormalizeContentToken(token);
    if (normalized.empty())
        return true;

    std::map<std::string, std::uint32_t>::const_iterator existing = owners.find(normalized);
    if (existing != owners.end())
    {
        if (existing->second != bit)
        {
            SetError(error, "ambiguous class content token: " + token);
            return false;
        }
        return true;
    }

    owners[normalized] = bit;
    return true;
}
}

bool PlayableClassRegistry::Register(const PlayableClassDefinition& definition)
{
    if (definition.id <= 0 || definition.legacyJobCode <= 0 || definition.key.empty() ||
        definition.brood <= 0 || (definition.enabled && definition.jobBitCode == 0u) ||
        !definition.creation.IsDefined())
        return false;

    for (std::vector<PlayableClassDefinition>::const_iterator it = definitions_.begin();
         it != definitions_.end(); ++it)
    {
        if (it->id == definition.id || it->legacyJobCode == definition.legacyJobCode ||
            it->key == definition.key ||
            (definition.jobBitCode != 0u && it->jobBitCode == definition.jobBitCode))
            return false;

        if (definition.creation.selectable && it->creation.selectable &&
            definition.brood == it->brood &&
            definition.creation.selectionOrder == it->creation.selectionOrder)
            return false;
    }

    for (std::vector<ClassRankDefinition>::const_iterator rank = definition.ranks.begin();
         rank != definition.ranks.end(); ++rank)
    {
        if (rank->rank <= 0 || rank->legacyJobBitCode == 0u || rank->key.empty())
            return false;
        for (std::vector<ClassRankDefinition>::const_iterator other = definition.ranks.begin();
             other != rank; ++other)
        {
            if (other->rank == rank->rank || other->key == rank->key)
                return false;
        }
    }

    definitions_.push_back(definition);
    return true;
}

bool PlayableClassRegistry::RegisterAll(const std::vector<PlayableClassDefinition>& definitions,
                                        std::string* error)
{
    // Register transactionally so a bad content source cannot leave a partially
    // populated registry behind. This is useful for future file-backed sources.
    PlayableClassRegistry candidate = *this;
    for (std::vector<PlayableClassDefinition>::const_iterator it = definitions.begin();
         it != definitions.end(); ++it)
    {
        if (!candidate.Register(*it))
        {
            SetError(error, "invalid or duplicate class definition: " + it->key);
            return false;
        }
    }
    *this = candidate;
    if (error) error->clear();
    return true;
}

bool LoadClassContent(PlayableClassRegistry& registry,
                      const IClassContentSource& source,
                      std::string* error)
{
    std::vector<PlayableClassDefinition> definitions;
    std::string sourceError;
    if (!source.Load(definitions, &sourceError))
    {
        SetError(error, sourceError.empty() ? "failed to load class content" : sourceError);
        return false;
    }

    PlayableClassRegistry candidate = registry;
    if (!candidate.RegisterAll(definitions, error))
        return false;
    if (!candidate.Validate(error))
        return false;

    registry = candidate;
    if (error) error->clear();
    return true;
}

bool PlayableClassRegistry::Validate(std::string* error) const
{
    if (definitions_.empty())
    {
        SetError(error, "class registry is empty");
        return false;
    }

    for (std::vector<PlayableClassDefinition>::const_iterator it = definitions_.begin();
         it != definitions_.end(); ++it)
    {
        if (it->enabled)
        {
            if (!it->formula.IsDefined())
            {
                SetError(error, "enabled class has no formula profile: " + it->key);
                return false;
            }
            if (!it->armor.IsDefined())
            {
                SetError(error, "enabled class has no armor model profile: " + it->key);
                return false;
            }
            if (it->gender == CharacterGender::Unspecified)
            {
                SetError(error, "enabled class has no character gender: " + it->key);
                return false;
            }
            if (!it->combat.IsDefined())
            {
                SetError(error, "enabled class has no combat profile: " + it->key);
                return false;
            }
            if (it->animation.legacyMotionJobMask == 0u)
            {
                SetError(error, "enabled class has no legacy motion job mask: " + it->key);
                return false;
            }
            if (!it->skills.IsDefined())
            {
                SetError(error, "enabled class has no skill tree route: " + it->key);
                return false;
            }
            if (it->skills.legacyGroupCode == 0u)
            {
                SetError(error, "enabled class has no legacy skill group route: " + it->key);
                return false;
            }
            if (!it->appearance.IsDefined())
            {
                SetError(error, "enabled class has no appearance profile: " + it->key);
                return false;
            }
            const ClassRankDefinition* baseRank = it->FindRank(1);
            if (it->ranks.empty() || !baseRank)
            {
                SetError(error, "enabled class has no rank chain: " + it->key);
                return false;
            }
            if (baseRank->legacyJobBitCode != it->jobBitCode)
            {
                SetError(error, "enabled class rank-1 bit differs from class specialization bit: " + it->key);
                return false;
            }

            // smCHAR_INFO::ChangeJob is a zero-based legacy progression index.
            // Until the wire/storage contract changes, an enabled class must expose
            // a contiguous 1..N rank chain so ChangeJob can map to rank+1 without
            // consumers inventing their own clamping or tier assumptions.
            for (int expectedRank = 1; expectedRank <= static_cast<int>(it->ranks.size()); ++expectedRank)
            {
                const ClassRankDefinition* rankDefinition = it->FindRank(expectedRank);
                if (!rankDefinition || rankDefinition->legacyJobBitCode == 0u ||
                    rankDefinition->key.empty())
                {
                    SetError(error, "enabled class has non-contiguous/invalid rank chain: " + it->key);
                    return false;
                }
            }

            const bool tempskron = it->HasCapability(ClassCapability::Tempskron);
            const bool morayion = it->HasCapability(ClassCapability::Morayion);
            const bool sopphetio = it->HasCapability(ClassCapability::Sopphetio);
            const int familyCount = (tempskron ? 1 : 0) + (morayion ? 1 : 0) + (sopphetio ? 1 : 0);
            if (familyCount != 1 ||
                (it->brood == brood::Tempskron && !tempskron) ||
                (it->brood == brood::Morayion && !morayion) ||
                (it->brood == brood::Sopphetio && !sopphetio))
            {
                SetError(error, "enabled class brood/capability mismatch: " + it->key);
                return false;
            }
        }
    }
    // Content tokens are parser identity. Different classes/ranks must never
    // normalize to the same token, otherwise OpenItem resolution would depend
    // on registration order. Multiple spellings that resolve to the SAME bit
    // are intentionally allowed (for example the class key and its rank-1 key).
    std::map<std::string, std::uint32_t> contentTokenOwners;
    for (std::vector<PlayableClassDefinition>::const_iterator it = definitions_.begin();
         it != definitions_.end(); ++it)
    {
        if (!it->enabled) continue;
        if (!RegisterContentToken(contentTokenOwners, it->key, it->jobBitCode, error) ||
            !RegisterContentToken(contentTokenOwners, it->displayKey, it->jobBitCode, error))
            return false;

        for (std::vector<ClassRankDefinition>::const_iterator rank = it->ranks.begin();
             rank != it->ranks.end(); ++rank)
        {
            if (!RegisterContentToken(contentTokenOwners, rank->key, rank->legacyJobBitCode, error) ||
                !RegisterContentToken(contentTokenOwners, rank->displayKey, rank->legacyJobBitCode, error))
                return false;
            for (std::vector<std::string>::const_iterator alias = rank->contentAliases.begin();
                 alias != rank->contentAliases.end(); ++alias)
            {
                if (!RegisterContentToken(contentTokenOwners, *alias, rank->legacyJobBitCode, error))
                    return false;
            }
        }
    }

    // Model motion applicability uses a separate bit protocol from item/job
    // specialization. Keep both the masks and parser tokens unambiguous.
    std::map<std::string, std::uint32_t> motionTokenOwners;
    for (std::vector<PlayableClassDefinition>::const_iterator left = definitions_.begin();
         left != definitions_.end(); ++left)
    {
        if (!left->enabled) continue;
        if (!RegisterContentToken(motionTokenOwners, left->key,
                                  left->animation.legacyMotionJobMask, error) ||
            !RegisterContentToken(motionTokenOwners, left->displayKey,
                                  left->animation.legacyMotionJobMask, error))
            return false;
        for (std::vector<std::string>::const_iterator alias = left->animation.motionContentAliases.begin();
             alias != left->animation.motionContentAliases.end(); ++alias)
        {
            if (!RegisterContentToken(motionTokenOwners, *alias,
                                      left->animation.legacyMotionJobMask, error))
                return false;
        }

        for (std::vector<PlayableClassDefinition>::const_iterator right = left + 1;
             right != definitions_.end(); ++right)
        {
            if (right->enabled &&
                left->animation.legacyMotionJobMask == right->animation.legacyMotionJobMask)
            {
                SetError(error, "duplicate legacy motion job mask: " + left->key + " / " + right->key);
                return false;
            }
        }
    }

    // Active legacy skill groups select disjoint ranges in the current skill
    // code layout and therefore must be unique. Rank bits are intentionally
    // NOT globally unique in the historical protocol (some rank-5 bits overlap
    // another class base bit), so validation must preserve that legacy contract.
    for (std::vector<PlayableClassDefinition>::const_iterator left = definitions_.begin();
         left != definitions_.end(); ++left)
    {
        if (!left->enabled) continue;
        for (std::vector<PlayableClassDefinition>::const_iterator right = left + 1;
             right != definitions_.end(); ++right)
        {
            if (!right->enabled) continue;
            if (left->skills.legacyGroupCode != 0u &&
                left->skills.legacyGroupCode == right->skills.legacyGroupCode)
            {
                SetError(error, "duplicate legacy skill group route: " + left->key + " / " + right->key);
                return false;
            }
        }
    }

    if (error) error->clear();
    return true;
}

const PlayableClassDefinition* PlayableClassRegistry::FindById(PlayableClassId id) const
{
    for (std::vector<PlayableClassDefinition>::const_iterator it = definitions_.begin();
         it != definitions_.end(); ++it)
        if (it->id == id) return &*it;
    return 0;
}

const PlayableClassDefinition* PlayableClassRegistry::FindByLegacyJobCode(int legacyJobCode) const
{
    for (std::vector<PlayableClassDefinition>::const_iterator it = definitions_.begin();
         it != definitions_.end(); ++it)
        if (it->legacyJobCode == legacyJobCode) return &*it;
    return 0;
}

const PlayableClassDefinition* PlayableClassRegistry::FindByKey(const std::string& key) const
{
    for (std::vector<PlayableClassDefinition>::const_iterator it = definitions_.begin();
         it != definitions_.end(); ++it)
        if (it->key == key) return &*it;
    return 0;
}

const PlayableClassDefinition* PlayableClassRegistry::FindByBroodAndSelectionOrder(int brood,
                                                                                    int selectionOrder) const
{
    for (std::vector<PlayableClassDefinition>::const_iterator it = definitions_.begin();
         it != definitions_.end(); ++it)
    {
        if (it->brood == brood && it->creation.selectable &&
            it->creation.selectionOrder == selectionOrder)
            return &*it;
    }
    return 0;
}

bool PlayableClassRegistry::IsEnabled(int legacyJobCode) const
{
    const PlayableClassDefinition* definition = FindByLegacyJobCode(legacyJobCode);
    return definition && definition->enabled;
}

std::vector<PlayableClassDefinition> PlayableClassRegistry::EnabledClasses() const
{
    std::vector<PlayableClassDefinition> result;
    for (std::vector<PlayableClassDefinition>::const_iterator it = definitions_.begin();
         it != definitions_.end(); ++it)
        if (it->enabled) result.push_back(*it);
    return result;
}

std::vector<PlayableClassDefinition> PlayableClassRegistry::EnabledClassesForBrood(int brood) const
{
    std::vector<PlayableClassDefinition> result;
    for (std::vector<PlayableClassDefinition>::const_iterator it = definitions_.begin();
         it != definitions_.end(); ++it)
        if (it->enabled && it->brood == brood) result.push_back(*it);

    struct SortBySelectionOrder
    {
        bool operator()(const PlayableClassDefinition& lhs, const PlayableClassDefinition& rhs) const
        {
            return lhs.creation.selectionOrder < rhs.creation.selectionOrder;
        }
    };
    std::sort(result.begin(), result.end(), SortBySelectionOrder());
    return result;
}

}
}
