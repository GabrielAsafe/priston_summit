#include <cassert>
#include "gameplay/pets/v2/PetRuntime.h"

namespace
{
class FakePetRuntime final : public gameplay::pets::IPetRuntime
{
public:
    gameplay::pets::PetRuntimeSnapshot Snapshot(gameplay::PlayerId player) const override
    {
        return player.Valid() ? gameplay::pets::PetRuntimeSnapshot(true, 3u, 11u, 7u)
                              : gameplay::pets::PetRuntimeSnapshot();
    }

    gameplay::pets::PetSyncStatus ApplyAreaSync(gameplay::PlayerId player,
                                                const gameplay::pets::PetAreaSync&) override
    {
        return player.Valid() ? gameplay::pets::PetSyncStatus::Applied
                              : gameplay::pets::PetSyncStatus::InvalidPlayer;
    }

    gameplay::pets::PetPublishStatus PublishAreaSync(gameplay::PlayerId player,
                                                       gameplay::pets::PetRuntimeKind) override
    {
        return player.Valid() ? gameplay::pets::PetPublishStatus::Published
                              : gameplay::pets::PetPublishStatus::InvalidPlayer;
    }
};
}

int main()
{
    FakePetRuntime runtime;
    gameplay::pets::PetRuntimeSnapshot snapshot = runtime.Snapshot(gameplay::PlayerId(9u));
    assert(snapshot.online && snapshot.PhoenixActive() && snapshot.HelpActive());
    assert(runtime.Snapshot(gameplay::PlayerId()).online == false);
    gameplay::pets::PetAreaSync sync(gameplay::pets::PetRuntimeKind::Phoenix, 1u, 2u, 3u, 6u, 4u);
    assert(runtime.ApplyAreaSync(gameplay::PlayerId(9u), sync) == gameplay::pets::PetSyncStatus::Applied);
    assert(runtime.ApplyAreaSync(gameplay::PlayerId(), sync) == gameplay::pets::PetSyncStatus::InvalidPlayer);
    assert(runtime.PublishAreaSync(gameplay::PlayerId(9u), gameplay::pets::PetRuntimeKind::Help) == gameplay::pets::PetPublishStatus::Published);
    assert(runtime.PublishAreaSync(gameplay::PlayerId(), gameplay::pets::PetRuntimeKind::Help) == gameplay::pets::PetPublishStatus::InvalidPlayer);
    return 0;
}
