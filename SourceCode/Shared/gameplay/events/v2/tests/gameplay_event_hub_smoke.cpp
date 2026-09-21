#include <cassert>
#include <string>

#include "gameplay/events/v2/GameplayEventHub.h"

namespace
{
struct MonsterKilled
{
    int monsterId;
};

struct FishCaught
{
    std::string species;
};
}

int main()
{
    gameplay::events::GameplayEventHub hub;
    int questProgress = 0;
    int huntingProgress = 0;
    int fishProgress = 0;

    const gameplay::events::GameplayEventHub::SubscriptionId questSub =
        hub.Subscribe<MonsterKilled>([&questProgress](const MonsterKilled& event)
        {
            questProgress += event.monsterId;
        });
    hub.Subscribe<MonsterKilled>([&huntingProgress](const MonsterKilled&)
    {
        ++huntingProgress;
    });
    hub.Subscribe<FishCaught>([&fishProgress](const FishCaught& event)
    {
        if (!event.species.empty())
            ++fishProgress;
    });

    assert(hub.SubscriberCount<MonsterKilled>() == 2);
    assert(hub.SubscriberCount<FishCaught>() == 1);

    hub.Publish(MonsterKilled{7});
    assert(questProgress == 7);
    assert(huntingProgress == 1);
    assert(fishProgress == 0);

    assert(hub.Unsubscribe<MonsterKilled>(questSub));
    assert(!hub.Unsubscribe<MonsterKilled>(questSub));
    hub.Publish(MonsterKilled{11});
    assert(questProgress == 7);
    assert(huntingProgress == 2);

    hub.Publish(FishCaught{"Pike"});
    assert(fishProgress == 1);

    hub.Clear();
    assert(hub.SubscriberCount<MonsterKilled>() == 0);
    assert(hub.SubscriberCount<FishCaught>() == 0);
    return 0;
}
