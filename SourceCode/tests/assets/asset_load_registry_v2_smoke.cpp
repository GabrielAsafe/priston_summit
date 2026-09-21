#include "rendering/assets/v2/AssetLoadRegistry.h"

#include <cassert>
#include <iostream>

using namespace summit::rendering::assets::v2;

int main()
{
    AssetLoadRegistry<8> registry;
    AssetLoadPolicy policy = AssetLoadPolicy::Defaults();
    policy.maxTransientFailures = 3;
    policy.residentBudgetBytes = 100;
    assert(policy.globalPrewarmLimit == 64u);
    assert(policy.scopedPrewarmLimit == 128u);
    assert(policy.maxExternalRuntimeDimension == 2048u);
    registry.SetPolicy(policy);

    registry.Register(0, ASSET_OWNER_STAGE);
    assert(registry.IsPendingGlobal(0));
    assert(registry.CanAttempt(0));
    registry.BeginLoad(0);
    registry.MarkLoaded(0, 8192, 8192, 4096, 4096, 64);
    assert(registry.Record(0).state == ASSET_LOADED);
    assert(registry.ResidentBytes() == 64);

    registry.Register(1, ASSET_OWNER_EFFECT);
    registry.BeginLoad(1);
    assert(!registry.MarkTransientFailure(1));
    registry.BeginLoad(1);
    assert(!registry.MarkTransientFailure(1));
    registry.BeginLoad(1);
    assert(registry.MarkTransientFailure(1));
    assert(registry.Record(1).state == ASSET_FAILED_PERMANENT);
    assert(!registry.CanAttempt(1));

    registry.Register(2, ASSET_OWNER_UI);
    registry.BeginLoad(2);
    registry.MarkLoaded(2, 512, 512, 512, 512, 32);
    registry.Touch(2);
    assert(registry.ResidentBytes() == 96);

    for (int i = 0; i < 32; ++i)
        registry.Touch(2);
    std::size_t victim = registry.SelectEvictionCandidate(2);
    assert(victim == 0);
    registry.MarkEvicted(victim);
    assert(registry.Record(0).state == ASSET_EVICTED);
    assert(registry.ResidentBytes() == 32);
    assert(registry.CanAttempt(0));

    registry.BeginLoad(0);
    registry.MarkLoaded(0, 8192, 8192, 2048, 2048, 16);
    assert(registry.Record(0).successfulLoads == 2);

    AssetTargetDimensions d = ComputeTargetDimensions(8192, 8192, policy, 0, true);
    assert(d.width <= policy.maxRuntimeDimension);
    assert(d.height <= policy.maxRuntimeDimension);
    assert(d.downscaled);

    registry.MarkPermanentFailure(3, 43472, 57732);
    assert(!registry.CanAttempt(3));

    registry.Register(4, ASSET_OWNER_CHARACTER);
    registry.BeginLoad(4);
    registry.MarkWaitingForDevice(4, 2048, 2048);
    assert(registry.Record(4).state == ASSET_WAITING_DEVICE);
    assert(registry.Record(4).transientFailures == 0);
    assert(!registry.CanAttempt(4));
    registry.MarkDeviceRestoreNeeded(4);
    assert(registry.Record(4).state == ASSET_UNLOADED);
    assert(registry.CanAttempt(4));

    registry.MarkWaitingForDevice(4, 2048, 2048);
    const AssetLoadStats stats = registry.Stats();
    assert(stats.loadedCount == 2);
    assert(stats.permanentFailureCount == 2);
    assert(stats.waitingDeviceCount == 1);

    std::cout << "Asset Load Registry V2 smoke: PASS\n";
    return 0;
}
