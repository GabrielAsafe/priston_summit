#include <cassert>

#include "gameplay/commerce/v2/PersonalShopSessionService.h"

int main()
{
    using namespace gameplay;
    using namespace gameplay::commerce;

    PersonalShopSessionService service(2u, 500u);
    const PlayerHandle first(PlayerId(10u), 3u);
    const PlayerHandle reconnect(PlayerId(10u), 4u);
    const PersonalShopSessionFingerprint sword(100u, 1u, 2u, 5000);
    const PersonalShopSessionFingerprint shield(101u, 3u, 4u, 6000);
    const PersonalShopSessionFingerprint bow(102u, 5u, 6u, 7000);

    assert(!service.CooldownActive(first, 1000u));
    PersonalShopSessionRecordResult recorded = service.RecordPrepared(first, 1000u, sword);
    assert(recorded.code == PersonalShopSessionRecordCode::Recorded);
    assert(service.CooldownActive(first, 1499u));
    assert(!service.CooldownActive(first, 1500u));

    PersonalShopSessionRecordResult duplicate = service.RecordPrepared(first, 1500u, sword);
    assert(duplicate.code == PersonalShopSessionRecordCode::Duplicate);
    assert(service.RecordPrepared(first, 2000u, shield).Accepted());
    assert(service.RecordPrepared(first, 2500u, bow).Accepted());

    // A reconnect with the same recyclable serial but a new generation gets a
    // fresh session and prunes the stale generation on first write.
    assert(!service.CooldownActive(reconnect, 2501u));
    assert(service.RecordPrepared(reconnect, 2501u, sword).Accepted());
    assert(service.Clear(first) == 0u);
    assert(service.Clear(reconnect) == 1u);
    return 0;
}
