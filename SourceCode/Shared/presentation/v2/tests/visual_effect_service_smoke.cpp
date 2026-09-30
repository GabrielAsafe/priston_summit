#include <cassert>
#include <string>

#include "presentation/v2/VisualEffectService.h"

class FakeEffectService final : public presentation::IVisualEffectService
{
public:
    FakeEffectService() : calls(0), last() {}
    bool PlayAt(const presentation::EffectId& effect,
                const presentation::PresentationTarget& target) override
    {
        ++calls;
        key = effect.key;
        last = target;
        return effect.Valid();
    }
    int calls;
    std::string key;
    presentation::PresentationTarget last;
};

int main()
{
    FakeEffectService effects;
    assert(effects.PlayAt(
        presentation::EffectId("effect.ui.stat_reset"),
        presentation::PresentationTarget(10, 20, 30)));
    assert(effects.calls == 1);
    assert(effects.key == "effect.ui.stat_reset");
    assert(effects.last.x == 10 && effects.last.y == 20 && effects.last.z == 30);
    assert(!presentation::EffectId().Valid());
    return 0;
}
