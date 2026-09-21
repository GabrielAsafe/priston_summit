#include "systems/rendering/sky/SkyStreamingV3.h"

#include <atomic>
#include <cstdint>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "core/concurrency/JobSystem.h"
#include "core/concurrency/SourceAssetLock.h"
#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/rendering/legacy_3d/smRead3d.h"
#include "systems/rendering/sky/legacy/HoSky.h"

extern int AssetLoadV2PrewarmMaterialGroupStep(smMATERIAL_GROUP *group, DWORD ownerMask,
    int MipMap, std::uint32_t maxAttempts, int requestPriority);

namespace
{
struct SummitSkyRequest
{
    SummitSkyRequest()
        : target(0), generation(0), clipping(false), prepared(0), success(false)
    {
    }

    skyResInfo *target;
    std::uint64_t generation;
    std::string sourceName;
    bool clipping;
    smPAT3D *prepared;
    bool success;
};

struct SummitSkyStreamingState
{
    SummitSkyStreamingState() : serial(1u) {}

    std::mutex mutex;
    std::unordered_map<skyResInfo *, std::uint64_t> generations;
    std::deque<std::shared_ptr<SummitSkyRequest> > completed;
    std::atomic<std::uint64_t> serial;
};

static SummitSkyStreamingState &SummitSkyState()
{
    static SummitSkyStreamingState *state = new SummitSkyStreamingState();
    return *state;
}

static void SummitPrepareSkyOnWorker(const std::shared_ptr<SummitSkyRequest> &request)
{
    if (!request || request->sourceName.empty())
        return;

    smPAT3D *pattern = 0;
    {
        summit::core::concurrency::SourceAssetLock sourceLock(request->sourceName.c_str());
        pattern = smASE_Read(const_cast<char *>(request->sourceName.c_str()));
    }

    if (pattern)
    {
        pattern->ZeroNormals();
        for (int i = 0; i < pattern->nObj3d; ++i)
        {
            if (!pattern->obj3d[i])
                continue;
            pattern->obj3d[i]->ClipStates = request->clipping
                ? (SMCLIP_NEARZ | SMCLIP_LEFT | SMCLIP_RIGHT | SMCLIP_TOP | SMCLIP_BOTTOM)
                : (SMCLIP_NEARZ | SMCLIP_TOP);
            pattern->obj3d[i]->ColorEffect = FALSE;
        }
        request->prepared = pattern;
        request->success = true;
    }

    std::lock_guard<std::mutex> lock(SummitSkyState().mutex);
    SummitSkyState().completed.push_back(request);
}

static bool SummitSkyRequestStillCurrent(const std::shared_ptr<SummitSkyRequest> &request)
{
    if (!request || !request->target)
        return false;
    std::lock_guard<std::mutex> lock(SummitSkyState().mutex);
    const std::unordered_map<skyResInfo *, std::uint64_t>::const_iterator it =
        SummitSkyState().generations.find(request->target);
    return it != SummitSkyState().generations.end() && it->second == request->generation;
}
}

int SummitRequestSkyModelV3(skyResInfo *target, const char *sourceName, bool clippingFlag)
{
    if (!target || !sourceName || !sourceName[0])
        return FALSE;
    if (target->stageSky)
        return TRUE;

    std::shared_ptr<SummitSkyRequest> request(new SummitSkyRequest());
    request->target = target;
    request->sourceName = sourceName;
    request->clipping = clippingFlag;
    request->generation = SummitSkyState().serial.fetch_add(1u, std::memory_order_acq_rel);

    {
        std::lock_guard<std::mutex> lock(SummitSkyState().mutex);
        SummitSkyState().generations[target] = request->generation;
    }

    if (!summit::core::concurrency::JobSystem::Instance().Submit(
        [request]() { SummitPrepareSkyOnWorker(request); },
        summit::core::concurrency::JobPriority::Low))
    {
        std::lock_guard<std::mutex> lock(SummitSkyState().mutex);
        const std::unordered_map<skyResInfo *, std::uint64_t>::iterator it =
            SummitSkyState().generations.find(target);
        if (it != SummitSkyState().generations.end() && it->second == request->generation)
            SummitSkyState().generations.erase(it);
        return FALSE;
    }
    return TRUE;
}

void SummitCancelSkyModelV3(skyResInfo *target)
{
    if (!target)
        return;
    std::lock_guard<std::mutex> lock(SummitSkyState().mutex);
    SummitSkyState().generations.erase(target);
}

void SummitPumpSkyStreamingV3()
{
    // Sky resources are purely cosmetic. Publish at most one mesh per frame so
    // their material registration never competes with a field transition.
    std::shared_ptr<SummitSkyRequest> request;
    {
        std::lock_guard<std::mutex> lock(SummitSkyState().mutex);
        if (SummitSkyState().completed.empty())
            return;
        request = SummitSkyState().completed.front();
        SummitSkyState().completed.pop_front();
    }

    if (!request)
        return;

    if (!SummitSkyRequestStillCurrent(request))
    {
        delete request->prepared;
        request->prepared = 0;
        return;
    }

    if (!request->success || !request->prepared)
        return;

    skyResInfo *target = request->target;
    if (target->stageSky)
        delete target->stageSky;
    target->stageSky = request->prepared;
    request->prepared = 0;

    if (target->stageSky->smMaterialGroup)
        AssetLoadV2PrewarmMaterialGroupStep(target->stageSky->smMaterialGroup,
            SMASSET_OWNER_SKY, 1, 2u, 2);
}

void SummitResetSkyStreamingV3()
{
    // Called only after JobSystem::WaitIdle(). No worker can append after the
    // queue is detached, so stale targets can be discarded without dereference.
    std::deque<std::shared_ptr<SummitSkyRequest> > completed;
    {
        std::lock_guard<std::mutex> lock(SummitSkyState().mutex);
        SummitSkyState().generations.clear();
        completed.swap(SummitSkyState().completed);
    }

    while (!completed.empty())
    {
        delete completed.front()->prepared;
        completed.front()->prepared = 0;
        completed.pop_front();
    }
}
