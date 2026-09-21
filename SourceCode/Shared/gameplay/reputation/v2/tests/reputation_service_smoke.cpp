#include <cassert>
#include <map>
#include <string>

#include "gameplay/reputation/v2/ReputationEvents.h"
#include "gameplay/reputation/v2/ReputationService.h"

namespace
{
class Repository : public gameplay::reputation::IReputationRepository
{
public:
    gameplay::reputation::ReputationLoadResult LoadValue(
        const std::string& playerKey,
        const std::string& reputationKey,
        int& value)
    {
        const std::string key = playerKey + ":" + reputationKey;
        std::map<std::string, int>::const_iterator it = values.find(key);
        if (it == values.end())
            return gameplay::reputation::ReputationLoadResult::NotFound;
        value = it->second;
        return gameplay::reputation::ReputationLoadResult::Found;
    }

    gameplay::reputation::ReputationChangeResult ApplyChange(
        const std::string& playerKey,
        const std::string& reputationKey,
        int expectedValue,
        int newValue,
        int delta,
        const std::string& sourceKey)
    {
        const std::string valueKey = playerKey + ":" + reputationKey;
        const std::string ledgerKey = valueKey + ":" + sourceKey;
        std::map<std::string, int>::const_iterator ledgerIt = ledger.find(ledgerKey);
        if (ledgerIt != ledger.end())
            return ledgerIt->second == delta
                       ? gameplay::reputation::ReputationChangeResult::AlreadyApplied
                       : gameplay::reputation::ReputationChangeResult::Failed;

        const int current = values.count(valueKey) ? values[valueKey] : 0;
        if (current != expectedValue)
            return gameplay::reputation::ReputationChangeResult::Conflict;
        values[valueKey] = newValue;
        ledger[ledgerKey] = delta;
        return gameplay::reputation::ReputationChangeResult::Applied;
    }

    std::map<std::string, int> values;
    std::map<std::string, int> ledger;
};
}

int main()
{
    using namespace gameplay;

    Repository repository;
    events::GameplayEventHub events;
    reputation::ReputationService service(repository, &events);

    reputation::ReputationDefinition faction;
    faction.key = "faction.alpha";
    faction.displayNameKey = "faction.alpha.name";
    faction.hasBounds = true;
    faction.minValue = -100;
    faction.maxValue = 100;
    assert(service.RegisterDefinition(faction));
    assert(!service.RegisterDefinition(faction));
    assert(service.ValidateDefinitions());

    std::vector<reputation::ReputationCatalogEntry> catalog;
    service.GetCatalog(catalog);
    assert(catalog.size() == 1u);
    assert(catalog[0].key == "faction.alpha");
    assert(catalog[0].displayNameKey == "faction.alpha.name");
    assert(catalog[0].hasBounds);
    assert(catalog[0].minValue == -100 && catalog[0].maxValue == 100);

    std::vector<reputation::ReputationSnapshot> snapshots;
    assert(service.GetSnapshots("Alice", snapshots));
    assert(snapshots.size() == 1u);
    assert(snapshots[0].key == "faction.alpha");
    assert(snapshots[0].value == 0);

    int eventCount = 0;
    int lastOld = 0;
    int lastNew = 0;
    events.Subscribe<reputation::ReputationChangedEvent>(
        [&](const reputation::ReputationChangedEvent& event)
        {
            ++eventCount;
            assert(event.playerKey == "Alice");
            assert(event.reputationKey == "faction.alpha");
            lastOld = event.oldValue;
            lastNew = event.newValue;
        });

    int value = 999;
    assert(service.GetValue("Alice", "faction.alpha", value));
    assert(value == 0);
    assert(service.Add("Alice", "faction.alpha", 25, "quest.1.reward"));
    assert(service.GetValue("Alice", "faction.alpha", value));
    assert(value == 25);
    assert(service.GetSnapshots("Alice", snapshots));
    assert(snapshots.size() == 1u && snapshots[0].value == 25);
    assert(eventCount == 1 && lastOld == 0 && lastNew == 25);

    // Same source + same delta is a successful retry and does not emit another change.
    assert(service.Add("Alice", "faction.alpha", 25, "quest.1.reward"));
    assert(service.GetValue("Alice", "faction.alpha", value) && value == 25);
    assert(eventCount == 1);

    // Same source + different delta is rejected by the source ledger.
    assert(!service.Add("Alice", "faction.alpha", 30, "quest.1.reward"));
    assert(service.GetValue("Alice", "faction.alpha", value) && value == 25);

    // Bounds are applied before the atomic ledger write.
    assert(service.Add("Alice", "faction.alpha", 500, "quest.2.reward"));
    assert(service.GetValue("Alice", "faction.alpha", value) && value == 100);
    assert(eventCount == 2 && lastOld == 25 && lastNew == 100);

    assert(service.Add("Alice", "faction.alpha", -500, "quest.3.reward"));
    assert(service.GetValue("Alice", "faction.alpha", value) && value == -100);
    assert(eventCount == 3 && lastOld == 100 && lastNew == -100);

    assert(!service.Add("Alice", "unknown", 1, "quest.4.reward"));
    assert(!service.Add("Alice", "faction.alpha", 0, "quest.5.reward"));
    assert(!service.GetValue("Alice", "unknown", value));
    return 0;
}
