#include <cassert>

#include "gameplay/world/v2/ITeleportService.h"

namespace
{
class FakeTeleportService final : public gameplay::world::ITeleportService
{
public:
    gameplay::PlayerId player;
    gameplay::WorldPosition destination;

    gameplay::world::TeleportResult Teleport(
        gameplay::PlayerId value,
        const gameplay::WorldPosition& target) override
    {
        player = value;
        destination = target;
        return gameplay::world::TeleportResult(gameplay::world::TeleportStatus::Sent);
    }
};
}

int main()
{
    FakeTeleportService service;
    const gameplay::WorldPosition destination(gameplay::MapId(23u), -927744, 0, -11052032);
    const gameplay::world::TeleportResult result = service.Teleport(gameplay::PlayerId(42u), destination);
    assert(result.Succeeded());
    assert(service.player.value == 42u);
    assert(service.destination.map.value == 23u);
    assert(service.destination.x == -927744);
    assert(service.destination.z == -11052032);
    return 0;
}
