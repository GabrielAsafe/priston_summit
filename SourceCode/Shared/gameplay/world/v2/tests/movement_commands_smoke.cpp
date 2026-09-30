#include "gameplay/world/v2/MovementCommands.h"

#include <cassert>

namespace
{
class FakeMovementCommands final : public gameplay::world::IMovementCommands
{
public:
    gameplay::world::MovementCommandResult PushAway(gameplay::EntityId target,
                                                     gameplay::EntityId source,
                                                     int distance) override
    {
        gameplay::world::MovementCommandResult result;
        if(target.Valid() && source.Valid() && distance > 0)
        {
            result.code = gameplay::world::MovementCommandCode::Ok;
            result.moved = true;
        }
        return result;
    }
};
}

int main()
{
    FakeMovementCommands commands;
    assert(commands.PushAway(gameplay::EntityId(1u), gameplay::EntityId(2u), 10).Ok());
    assert(!commands.PushAway(gameplay::EntityId(), gameplay::EntityId(2u), 10).Ok());
    return 0;
}
