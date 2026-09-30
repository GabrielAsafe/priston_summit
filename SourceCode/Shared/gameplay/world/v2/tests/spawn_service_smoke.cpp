#include <cassert>

#include "gameplay/world/v2/ISpawnService.h"

namespace
{
class FakeSpawnService final : public gameplay::world::ISpawnService
{
public:
    gameplay::world::SpawnRequest last;

    gameplay::world::SpawnResult Spawn(const gameplay::world::SpawnRequest& request) override
    {
        last = request;
        return gameplay::world::SpawnResult(gameplay::world::SpawnStatus::Spawned, gameplay::EntityId(77u));
    }
};
}

int main()
{
    FakeSpawnService service;
    const gameplay::world::SpawnRequest request(
        "boss", gameplay::WorldPosition(gameplay::MapId(23u), -1, 2, -3));
    const gameplay::world::SpawnResult result = service.Spawn(request);
    assert(result.Succeeded());
    assert(result.entity.value == 77u);
    assert(service.last.templateName == "boss");
    assert(service.last.position.map.value == 23u);
    assert(!service.last.owner.Valid());
    return 0;
}
