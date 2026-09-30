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
    int loads = 0;
    int saves = 0;
    std::map<std::pair<std::string, int>, gameplay::professions::ProfessionProgress> values;

    bool LoadPlayer(const std::string& playerName,
                    std::vector<gameplay::professions::ProfessionRecord>& out) override
    {
        ++loads;
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
        ++saves;
        if (failSave)
            return false;
        values[std::make_pair(playerName, static_cast<int>(type))] = progress;
        return true;
    }
};
}

int main()
{
    using namespace gameplay::professions;
    FakeProfessionRepository repository;
    repository.values[std::make_pair(std::string("miner"), static_cast<int>(ProfessionType::Miner))] =
        ProfessionProgress(7, 120);

    ProfessionService service(&repository);
    assert(service.GetLevel("miner", ProfessionType::Miner) == 7);
    assert(service.GetExperience("miner", ProfessionType::Miner) == 120);
    assert(repository.loads == 1);
    assert(service.GetLevel("miner", ProfessionType::Miner) == 7);
    assert(repository.loads == 1);

    assert(service.AddExperience("miner", ProfessionType::Miner, 30));
    assert(service.GetExperience("miner", ProfessionType::Miner) == 150);
    assert(repository.saves == 1);

    repository.failSave = true;
    assert(!service.AddExperience("miner", ProfessionType::Miner, 10));
    assert(service.GetExperience("miner", ProfessionType::Miner) == 150);

    service.ClearPlayer("miner");
    repository.failSave = false;
    assert(service.GetExperience("miner", ProfessionType::Miner) == 150);
    assert(repository.loads == 2);

    std::cout << "profession persistence smoke: PASS\n";
    return 0;
}
