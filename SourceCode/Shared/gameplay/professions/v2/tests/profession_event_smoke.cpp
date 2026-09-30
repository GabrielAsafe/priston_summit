#include "gameplay/professions/v2/ProfessionService.h"

#include <cassert>
#include <iostream>
#include <map>
#include <utility>

namespace
{
class FakeProfessionRepository final : public gameplay::professions::IProfessionRepository
{
public:
    bool failSave = false;
    std::map<std::pair<std::string, int>, gameplay::professions::ProfessionProgress> values;

    bool LoadPlayer(const std::string& playerName,
                    std::vector<gameplay::professions::ProfessionRecord>& out) override
    {
        out.clear();
        for (std::map<std::pair<std::string, int>, gameplay::professions::ProfessionProgress>::const_iterator it = values.begin();
             it != values.end(); ++it)
        {
            if (it->first.first == playerName)
                out.push_back(gameplay::professions::ProfessionRecord(
                    static_cast<gameplay::professions::ProfessionType>(it->first.second), it->second));
        }
        return true;
    }

    bool SaveProgress(const std::string& playerName,
                      gameplay::professions::ProfessionType type,
                      const gameplay::professions::ProfessionProgress& progress) override
    {
        if (failSave)
            return false;
        values[std::make_pair(playerName, static_cast<int>(type))] = progress;
        return true;
    }
};
}

int main()
{
    using namespace gameplay;
    professions::ProfessionType type = professions::ProfessionType::Miner;
    FakeProfessionRepository repository;
    repository.values[std::make_pair(std::string("miner"), static_cast<int>(type))] =
        professions::ProfessionProgress(7, 120);

    events::GameplayEventHub hub;
    int published = 0;
    events::ProfessionProgressedEvent observed;
    hub.Subscribe<events::ProfessionProgressedEvent>(
        [&](const events::ProfessionProgressedEvent& event)
        {
            ++published;
            observed = event;
        });

    professions::ProfessionService service(&repository, &hub);
    assert(service.AddExperience("miner", type, 30));
    assert(published == 1);
    assert(observed.playerName == "miner");
    assert(observed.profession == type);
    assert(observed.previous.level == 7 && observed.previous.experience == 120);
    assert(observed.current.level == 7 && observed.current.experience == 150);

    service.SetProgress("miner", type, professions::ProfessionProgress(8, 200));
    assert(published == 2);
    assert(observed.previous.level == 7 && observed.previous.experience == 150);
    assert(observed.current.level == 8 && observed.current.experience == 200);

    service.SetProgress("miner", type, professions::ProfessionProgress(8, 200));
    assert(published == 2);

    repository.failSave = true;
    assert(!service.AddExperience("miner", type, 10));
    assert(published == 2);
    assert(service.GetExperience("miner", type) == 200);

    std::cout << "profession progressed event smoke: PASS\n";
    return 0;
}
