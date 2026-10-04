#include <cassert>
#include <string>

#include "gameplay/progression/v2/BattlePassProgressApplicationService.h"

class FakeProgressPort final : public gameplay::progression::IBattlePassProgressMutationPort
{
public:
    int calls = 0;
    bool succeed = true;

    bool AddExperience(const std::string& accountId,
                       int amount,
                       gameplay::progression::BattlePassProgressResult& result) override
    {
        ++calls;
        if(!succeed)
            return false;
        result = gameplay::progression::BattlePassProgressService::AddExperience(
            gameplay::progression::BattlePassProgressState{900, 3, 1}, amount);
        return accountId == "account";
    }
};

int main()
{
    FakeProgressPort port;
    gameplay::progression::BattlePassProgressResult result;

    assert(gameplay::progression::BattlePassProgressApplicationService::AddExperience(
        port, "account", 150, result));
    assert(port.calls == 1);
    assert(result.changed);
    assert(result.levelsGained == 1);
    assert(result.state.level == 4);
    assert(result.state.experience == 51);

    assert(!gameplay::progression::BattlePassProgressApplicationService::AddExperience(
        port, "", 100, result));
    assert(port.calls == 1);

    port.succeed = false;
    assert(!gameplay::progression::BattlePassProgressApplicationService::AddExperience(
        port, "account", 100, result));
    assert(port.calls == 2);
    return 0;
}
