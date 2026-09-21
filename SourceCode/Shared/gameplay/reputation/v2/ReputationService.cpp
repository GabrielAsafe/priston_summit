#include "gameplay/reputation/v2/ReputationService.h"

#include <limits>

#include "gameplay/reputation/v2/ReputationEvents.h"

namespace gameplay
{
namespace reputation
{
namespace
{
void SetError(std::string* error, const char* message)
{
    if (error)
        *error = message ? message : "invalid reputation definition";
}
}

bool ReputationService::RegisterDefinition(const ReputationDefinition& definition)
{
    if (definition.key.empty() || definition.key.size() > 255u ||
        definition.displayNameKey.size() > 255u ||
        (definition.hasBounds && (definition.minValue > 0 || definition.maxValue < 0 ||
                                  definition.minValue > definition.maxValue)) ||
        definitions_.count(definition.key) != 0u)
        return false;

    definitions_[definition.key] = definition;
    return true;
}

bool ReputationService::RegisterDefinitions(
    const std::vector<ReputationDefinition>& definitions)
{
    const std::map<std::string, ReputationDefinition> original = definitions_;
    for (std::vector<ReputationDefinition>::const_iterator it = definitions.begin();
         it != definitions.end(); ++it)
    {
        if (!RegisterDefinition(*it))
        {
            definitions_ = original;
            return false;
        }
    }
    return true;
}

bool ReputationService::ValidateDefinitions(std::string* error) const
{
    for (std::map<std::string, ReputationDefinition>::const_iterator it = definitions_.begin();
         it != definitions_.end(); ++it)
    {
        const ReputationDefinition& definition = it->second;
        if (definition.key.empty() || definition.key.size() > 255u ||
            definition.displayNameKey.size() > 255u)
        {
            SetError(error, "reputation key/display key is invalid");
            return false;
        }
        if (definition.hasBounds && (definition.minValue > 0 || definition.maxValue < 0 ||
                                     definition.minValue > definition.maxValue))
        {
            SetError(error, "reputation bounds must contain the implicit zero value");
            return false;
        }
    }
    return true;
}

const ReputationDefinition* ReputationService::FindDefinition(const std::string& key) const
{
    std::map<std::string, ReputationDefinition>::const_iterator it = definitions_.find(key);
    return it == definitions_.end() ? 0 : &it->second;
}

std::vector<std::string> ReputationService::ReputationKeys() const
{
    std::vector<std::string> keys;
    for (std::map<std::string, ReputationDefinition>::const_iterator it = definitions_.begin();
         it != definitions_.end(); ++it)
        keys.push_back(it->first);
    return keys;
}

void ReputationService::GetCatalog(std::vector<ReputationCatalogEntry>& entries) const
{
    entries.clear();
    entries.reserve(definitions_.size());
    for (std::map<std::string, ReputationDefinition>::const_iterator it = definitions_.begin();
         it != definitions_.end(); ++it)
    {
        ReputationCatalogEntry entry;
        entry.key = it->second.key;
        entry.displayNameKey = it->second.displayNameKey;
        entry.hasBounds = it->second.hasBounds;
        entry.minValue = it->second.minValue;
        entry.maxValue = it->second.maxValue;
        entries.push_back(entry);
    }
}

bool ReputationService::GetSnapshots(const std::string& playerKey,
                                     std::vector<ReputationSnapshot>& snapshots)
{
    snapshots.clear();
    if (playerKey.empty() || playerKey.size() > 255u)
        return false;

    snapshots.reserve(definitions_.size());
    for (std::map<std::string, ReputationDefinition>::const_iterator it = definitions_.begin();
         it != definitions_.end(); ++it)
    {
        int value = 0;
        if (!GetValue(playerKey, it->first, value))
        {
            snapshots.clear();
            return false;
        }

        ReputationSnapshot snapshot;
        snapshot.key = it->second.key;
        snapshot.displayNameKey = it->second.displayNameKey;
        snapshot.value = value;
        snapshot.hasBounds = it->second.hasBounds;
        snapshot.minValue = it->second.minValue;
        snapshot.maxValue = it->second.maxValue;
        snapshots.push_back(snapshot);
    }
    return true;
}

bool ReputationService::GetValue(const std::string& playerKey,
                                 const std::string& reputationKey,
                                 int& value)
{
    if (playerKey.empty() || playerKey.size() > 255u ||
        reputationKey.empty() || !FindDefinition(reputationKey))
        return false;

    int storedValue = 0;
    const ReputationLoadResult loaded = repository_.LoadValue(playerKey, reputationKey, storedValue);
    if (loaded == ReputationLoadResult::Failed)
        return false;

    value = loaded == ReputationLoadResult::Found ? storedValue : 0;
    return true;
}

int ReputationService::ClampValue(const ReputationDefinition& definition, long long value) const
{
    long long minimum = static_cast<long long>(std::numeric_limits<int>::min());
    long long maximum = static_cast<long long>(std::numeric_limits<int>::max());
    if (definition.hasBounds)
    {
        minimum = definition.minValue;
        maximum = definition.maxValue;
    }
    if (value < minimum)
        value = minimum;
    if (value > maximum)
        value = maximum;
    return static_cast<int>(value);
}

bool ReputationService::Add(const std::string& playerKey,
                            const std::string& reputationKey,
                            int amount,
                            const std::string& sourceKey)
{
    const ReputationDefinition* definition = FindDefinition(reputationKey);
    if (!definition || playerKey.empty() || playerKey.size() > 255u ||
        sourceKey.empty() || sourceKey.size() > 255u || amount == 0)
        return false;

    const int maxAttempts = 4;
    for (int attempt = 0; attempt < maxAttempts; ++attempt)
    {
        int oldValue = 0;
        const ReputationLoadResult loaded = repository_.LoadValue(playerKey, reputationKey, oldValue);
        if (loaded == ReputationLoadResult::Failed)
            return false;
        if (loaded == ReputationLoadResult::NotFound)
            oldValue = 0;

        const int newValue = ClampValue(*definition,
            static_cast<long long>(oldValue) + static_cast<long long>(amount));

        const ReputationChangeResult changed = repository_.ApplyChange(
            playerKey, reputationKey, oldValue, newValue, amount, sourceKey);
        if (changed == ReputationChangeResult::Conflict)
            continue;
        if (changed == ReputationChangeResult::Failed)
            return false;
        if (changed == ReputationChangeResult::AlreadyApplied)
            return true;

        if (eventHub_ && oldValue != newValue)
        {
            ReputationChangedEvent event;
            event.playerKey = playerKey;
            event.reputationKey = reputationKey;
            event.oldValue = oldValue;
            event.newValue = newValue;
            event.sourceKey = sourceKey;
            eventHub_->Publish(event);
        }
        return true;
    }
    return false;
}
}
}
