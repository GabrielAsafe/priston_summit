#include <cassert>
#include "gameplay/professions/v2/ProfessionService.h"

int main()
{
    gameplay::professions::ProfessionService service;
    using gameplay::professions::ProfessionProgress;
    using gameplay::professions::ProfessionType;

    service.SetProgress("miner", ProfessionType::Miner, ProfessionProgress(7, 120));
    assert(service.GetLevel("miner", ProfessionType::Miner) == 7);
    assert(service.GetExperience("miner", ProfessionType::Miner) == 120);
    assert(service.AddExperience("miner", ProfessionType::Miner, 30));
    assert(service.GetExperience("miner", ProfessionType::Miner) == 150);
    assert(service.GetLevel("missing", ProfessionType::Miner) == 0);
    service.ClearPlayer("miner");
    assert(service.GetLevel("miner", ProfessionType::Miner) == 0);
    return 0;
}
