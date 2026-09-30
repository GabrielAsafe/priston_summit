#include <cassert>
#include <string>

#include "presentation/v2/LightingService.h"

class FakeLightingService final : public presentation::ILightingService
{
public:
    bool PulseAt(const presentation::LightId& light,
                 const presentation::PresentationTarget& target) override
    {
        key = light.key;
        last = target;
        return light.Valid();
    }

    std::string key;
    presentation::PresentationTarget last;
};

int main()
{
    FakeLightingService lights;
    assert(lights.PulseAt(
        presentation::LightId("light.character.resurrection_restart"),
        presentation::PresentationTarget(10, 20, 30)));
    assert(lights.key == "light.character.resurrection_restart");
    assert(lights.last.x == 10 && lights.last.y == 20 && lights.last.z == 30);
    assert(!presentation::LightId().Valid());
    return 0;
}
