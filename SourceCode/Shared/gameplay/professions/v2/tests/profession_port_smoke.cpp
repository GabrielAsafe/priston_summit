#include "gameplay/professions/v2/ProfessionService.h"

#include <cassert>
#include <iostream>

int main()
{
    using namespace gameplay::professions;
    ProfessionService concrete;
    IProfessionService& service = concrete;

    service.SetProgress("miner", ProfessionType::Miner, ProfessionProgress(4, 90));
    assert(service.GetLevel("miner", ProfessionType::Miner) == 4);
    assert(service.GetExperience("miner", ProfessionType::Miner) == 90);
    assert(service.AddExperience("miner", ProfessionType::Miner, 10));
    assert(service.GetExperience("miner", ProfessionType::Miner) == 100);
    service.ClearPlayer("miner");
    assert(service.GetLevel("miner", ProfessionType::Miner) == 0);

    std::cout << "profession service port smoke: PASS\n";
    return 0;
}
