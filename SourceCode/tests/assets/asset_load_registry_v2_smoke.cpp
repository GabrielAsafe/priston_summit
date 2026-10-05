#include "rendering/assets/v2/AssetLoadRegistry.h"

#include <cassert>
#include <cstdint>

using namespace summit::rendering::assets::v2;

int main()
{
    const AssetLoadPolicy defaults = AssetLoadPolicy::Defaults();
    assert(defaults.maxSourceDimension == 8192u);
    assert(defaults.maxRuntimeDimension == 4096u);
    assert(defaults.maxTransientFailures == 3u);
    assert(IsStructurallyValidSource(4096u, 4096u, defaults));
    assert(!IsStructurallyValidSource(0u, 4096u, defaults));
    assert(!IsStructurallyValidSource(8193u, 1u, defaults));

    const AssetTargetDimensions target = ComputeTargetDimensions(8192u, 4096u, defaults, 0u, true);
    assert(target.downscaled);
    assert(target.width <= defaults.maxRuntimeDimension);
    assert(target.height <= defaults.maxRuntimeDimension);
    assert((target.width % 4u) == 0u);
    assert((target.height % 4u) == 0u);

    AssetLoadRegistry<4> registry;
    registry.Register(0, ASSET_OWNER_STAGE);
    assert(registry.IsPendingGlobal(0));
    assert(registry.CanAttempt(0));
    registry.BeginLoad(0);
    assert(!registry.CanAttempt(0));
    const std::uint64_t resident = 16ull * 1024ull * 1024ull;
    registry.MarkLoaded(0, 4096u, 4096u, target.width, target.height, resident);
    assert(registry.Record(0).state == ASSET_LOADED);
    assert(registry.ResidentBytes() == resident);
    assert(registry.Record(0).successfulLoads == 1u);

    registry.Register(1, ASSET_OWNER_EFFECT);
    registry.BeginLoad(1);
    assert(!registry.MarkTransientFailure(1, 1024u, 1024u));
    assert(registry.Record(1).state == ASSET_FAILED_TRANSIENT);
    registry.BeginLoad(1);
    assert(!registry.MarkTransientFailure(1, 1024u, 1024u));
    registry.BeginLoad(1);
    assert(registry.MarkTransientFailure(1, 1024u, 1024u));
    assert(registry.Record(1).state == ASSET_FAILED_PERMANENT);
    assert(!registry.CanAttempt(1));

    registry.Register(2, ASSET_OWNER_UI);
    registry.MarkWaitingForDevice(2, 512u, 512u);
    assert(registry.Record(2).state == ASSET_WAITING_DEVICE);
    assert(!registry.CanAttempt(2));
    registry.MarkDeviceRestoreNeeded(2);
    assert(registry.Record(2).state == ASSET_UNLOADED);
    assert(registry.IsPendingGlobal(2));

    registry.MarkEvicted(0);
    assert(registry.Record(0).state == ASSET_EVICTED);
    assert(registry.ResidentBytes() == 0u);
    assert(registry.Record(0).evictionCount == 1u);

    const AssetLoadStats stats = registry.Stats();
    assert(stats.registeredCount == 3u);
    assert(stats.permanentFailureCount == 1u);
    assert(stats.evictedCount == 1u);
    assert(stats.unloadedCount >= 1u);
    return 0;
}
