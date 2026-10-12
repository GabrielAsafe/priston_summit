#include "content/v1/definition/content/DefinitionRuntime.h"
#include "opcodes.h"
#include <cstdint>
#include <cstdlib>
#include <atomic>
#include <climits>
#include <deque>
#include <memory>
#include <mutex>
#include <new>
#include <string>
#include <vector>

#include "core/concurrency/JobSystem.h"
#include "core/concurrency/SourceAssetLock.h"
#include "systems/rendering/legacy_3d/smd3d.h"
#include "rendering/legacy_3d/stage/StageAreaQuery.h"
#include "rendering/legacy_3d/stage/StageHeightQuery.h"
#include "rendering/legacy_3d/stage/StageSurfaceQuery.h"
#include "systems/rendering/assets/v3/AssetStreamingV3.h"
#include "systems/security/legacy/process_integrity/cracker.h"
#include "systems/network/transport/legacy/smwsock.h"
#include "systems/network/v2/legacy/LegacyClientCommandSender.h"
#include "systems/gameplay/character/core/character.h"
#include "authentication/character_selection/CharacterSelectionTrace.h"
#include "systems/media/video/legacy_avi/avictrl.h"
#include "core/gameplay/runtime/playmain.h"
#include "systems/audio/playback/legacy/Dxwav.h"
#include "core/data/legacy_loader/fileread.h"
#include "gameplay/classes/v2/services/ClassService.h"
#include "diagnostics/RuntimeAssetProbe.h"

#include "systems/rendering/effects/particle/legacy/particle.h"
#include "systems/network/client/netplay.h"

#include "compat/legacy/sin/sinLinkHeader.h"
#include "systems/presentation/opening/legacy_runtime/HoLinkHeader.h"

#include "systems/world/field/field.h"
#include "systems/audio/effects/legacy/effectsnd.h"

#include "systems/persistence/character_record/record.h"
#include "core/gameplay/legacy_support/playsub.h"
#include "systems/combat/skills/client/SkillSub.h"
#include "systems/combat/damage/damage.h"

#include "systems/social/clan/tjclanDEF.h"
#include "systems/social/clan/tjclan.h"
#include "systems/social/clan/cE_CViewClanInfo.h"

#include "systems/localization/legacy_catalog/language.h"

#include "systems/combat/damage/damage.h"
#include "systems/network/region/AreaServer.h"

#include "compat/client_server/server_legacy/onserver.h"

#include "systems/configuration/client/Settings.h"

extern int WinSizeX;
extern int WinSizeY;
extern HWND hwnd;
extern int MouseButton[3];

namespace
{
inline bool SummitPlayMainClientSocketSend(smWINSOCK* socket, const void* data, int size)
{
	if(!socket || !data || size <= 0) return false;
	return gameplay::network::legacy::ClientLegacyCommandSender().SendToLegacySocket(
		socket, gameplay::network::PacketView(data, static_cast<std::size_t>(size), 0u)).Succeeded();
}
}

int LoadFieldMap(int PosNum, sFIELD *lpField, smSTAGE3D *lpStage);
extern char szGameStageName[2][64];


smSTAGE3D		*smStage = 0;
smSTAGE3D		*smGameStage[2] = { 0,0 };
sFIELD			*StageField[2] = { 0,0 };

// Asset/Stage V3 bridges. Texture CPU preparation is background work; the
// render thread only commits ready D3D resources.
extern int AssetLoadV2PrewarmMaterialGroupStep(smMATERIAL_GROUP *group, DWORD ownerMask,
	int MipMap, std::uint32_t maxAttempts, int requestPriority);

namespace
{

struct SummitStageObjectSpec
{
    std::string file;
    bool hasBip;
};

struct SummitPreparedStageObject
{
    SummitPreparedStageObject() : pattern(0), bipPattern(0) {}
    smPAT3D *pattern;
    smPAT3D *bipPattern;
};

struct SummitStageLoadRequest
{
    SummitStageLoadRequest()
        : id(0), field(0), anchorField(0), targetSlot(0), stage(0), success(false),
          cancelled(false), lightX(0), lightY(0), lightZ(0), bright(0), contrast(0)
    {
    }

    std::uint64_t id;
    sFIELD *field;
    sFIELD *anchorField;
    int targetSlot;
    std::string stageFile;
    std::vector<SummitStageObjectSpec> objectSpecs;
    std::vector<SummitPreparedStageObject> objects;
    smSTAGE3D *stage;
    bool success;
    std::atomic<bool> cancelled;
    int lightX;
    int lightY;
    int lightZ;
    int bright;
    int contrast;
};

struct SummitFieldTransition
{
    SummitFieldTransition()
        : armed(false), active(false), committing(false), field(0), armedAt(0), startedAt(0),
          x(0), y(0), z(0), ax(0), ay(0), az(0)
    {
    }

    bool armed;
    bool active;
    bool committing;
    sFIELD *field;
    DWORD armedAt;
    DWORD startedAt;
    int x;
    int y;
    int z;
    int ax;
    int ay;
    int az;
};

static std::mutex gStageStreamingMutex;
static std::shared_ptr<SummitStageLoadRequest> gStageStreamingPending[2];
static std::deque<std::shared_ptr<SummitStageLoadRequest> > gStageStreamingCompleted;
static std::atomic<std::uint64_t> gStageStreamingSerial(1u);
static smSTAGE3D *gStageTexturePrewarm[2] = { 0, 0 };
static SummitFieldTransition gFieldTransition;
// Legacy gameplay assumes that OnStageField >= 0 always implies a valid
// StageField[OnStageField]. During the very first asynchronous stage load there
// is no old world to keep resident, so publish the FIELD METADATA early while
// geometry remains private to the worker. This preserves the legacy invariant
// without making stage loading synchronous.
static bool gStageBootstrapPending = false;
// Bootstrap readiness is explicit: FIELD metadata can exist before geometry,
// but gameplay is not released until a real solid spawn surface is resolved.
static bool gStageBootstrapSpawnResolved = false;
static bool gStageBootstrapSpawnCoordinatesReady = false;
static int gStageBootstrapSpawnX = 0;
static int gStageBootstrapSpawnZ = 0;
static int gStageBootstrapResolvedFloorY = CLIP_OUT;
static DWORD gStageBootstrapVisualStartedAt = 0;
static std::vector<smTEXTUREHANDLE *> gStageBootstrapFloorTextures;
static std::vector<smTEXTUREHANDLE *> gStageBootstrapPriorityTextures;

// Temporary bootstrap diagnostics. The log is intentionally bootstrap-only and
// rate-limited so it can stay enabled during this investigation without
// reintroducing the old hot-path logging problem.
static std::mutex gStageBootstrapDiagnosticMutex;
static DWORD gStageBootstrapDiagnosticLastStateAt = 0;

static void SummitBootstrapDiagnosticEvent(const char *eventName, const char *detail = 0)
{
    CreateDirectory("LOGS", NULL);
    std::lock_guard<std::mutex> lock(gStageBootstrapDiagnosticMutex);
    FILE *file = 0;
    fopen_s(&file, "LOGS\\streaming_v3_bootstrap_diag.txt", "a");
    if (!file)
        return;
    fprintf(file, "[%lu] EVENT %s%s%s\n",
        static_cast<unsigned long>(GetCurrentTime()),
        eventName ? eventName : "unknown",
        detail && *detail ? " " : "",
        detail && *detail ? detail : "");
    fclose(file);
}

static DWORD SummitStageTransitionMaxWarmWait()
{
    static const DWORD configured = []() -> DWORD
    {
        const char *value = getenv("SUMMIT_STAGE_TRANSITION_MAX_WARM_MS");
        if (!value || !*value)
            return 3000u;
        const unsigned long parsed = strtoul(value, 0, 10);
        if (parsed < 250ul) return 250u;
        if (parsed > 10000ul) return 10000u;
        return static_cast<DWORD>(parsed);
    }();
    return configured;
}

static int SummitFindStageSlot(sFIELD *field)
{
    if (!field)
        return -1;
    if (StageField[0] == field && smGameStage[0])
        return 0;
    if (StageField[1] == field && smGameStage[1])
        return 1;
    return -1;
}

static void SummitDeletePreparedStageRequest(const std::shared_ptr<SummitStageLoadRequest> &request)
{
    if (!request)
        return;

    if (request->stage)
    {
        delete request->stage;
        request->stage = 0;
    }

    for (std::size_t i = 0; i < request->objects.size(); ++i)
    {
        if (request->objects[i].pattern)
            delete request->objects[i].pattern;
        if (request->objects[i].bipPattern && request->objects[i].bipPattern != request->objects[i].pattern)
            delete request->objects[i].bipPattern;
        request->objects[i].pattern = 0;
        request->objects[i].bipPattern = 0;
    }
}

static void SummitParseStageObject(const SummitStageObjectSpec &spec, SummitPreparedStageObject &out)
{
    if (spec.file.empty())
        return;

    smASE_SetPhysique(0);
    if (spec.hasBip)
    {
        {
            summit::core::concurrency::SourceAssetLock sourceLock(spec.file.c_str());
            out.bipPattern = smASE_ReadBone(const_cast<char *>(spec.file.c_str()));
        }
        if (!out.bipPattern)
            return;
        smASE_SetPhysique(out.bipPattern);
    }

    {
        summit::core::concurrency::SourceAssetLock sourceLock(spec.file.c_str());
        out.pattern = smASE_Read(const_cast<char *>(spec.file.c_str()));
    }
    smASE_SetPhysique(0);
    // Do not destroy partially prepared patterns on a worker. Their material
    // destructors can touch shared texture handles; ownership stays in the
    // request and cleanup is performed by the main-thread completion path.
}

static void SummitCompleteStageRequest(const std::shared_ptr<SummitStageLoadRequest> &request)
{
    if (request)
    {
        char detail[256] = { 0 };
        sprintf_s(detail, sizeof(detail),
            "id=%llu slot=%d success=%d cancelled=%d stage=%p head=%d file=%s",
            static_cast<unsigned long long>(request->id), request->targetSlot,
            request->success ? 1 : 0,
            request->cancelled.load(std::memory_order_acquire) ? 1 : 0,
            static_cast<void *>(request->stage),
            request->stage && request->stage->Head ? 1 : 0,
            request->stageFile.c_str());
        SummitBootstrapDiagnosticEvent("stage-worker-complete", detail);
    }

    std::lock_guard<std::mutex> lock(gStageStreamingMutex);
    gStageStreamingCompleted.push_back(request);
}

static void SummitBuildStageOnWorker(const std::shared_ptr<SummitStageLoadRequest> &request)
{
    if (!request)
        return;

    {
        char detail[256] = { 0 };
        sprintf_s(detail, sizeof(detail), "id=%llu slot=%d file=%s",
            static_cast<unsigned long long>(request->id), request->targetSlot,
            request->stageFile.c_str());
        SummitBootstrapDiagnosticEvent("stage-worker-start", detail);
    }

    if (request->cancelled.load(std::memory_order_acquire))
    {
        SummitCompleteStageRequest(request);
        return;
    }

    smSTAGE3D *stage = new(std::nothrow) smSTAGE3D;
    if (!stage)
    {
        SummitCompleteStageRequest(request);
        return;
    }
    // From this point the request owns the stage even on failure/cancellation.
    // The main thread is the only place that destroys it.
    request->stage = stage;

    stage->VectLight.x = request->lightX;
    stage->VectLight.y = request->lightY;
    stage->VectLight.z = request->lightZ;
    stage->Bright = request->bright;
    stage->Contrast = request->contrast;

    {
        summit::core::concurrency::SourceAssetLock stageSourceLock(request->stageFile.c_str());
        if (!smSTAGE3D_ReadASE(const_cast<char *>(request->stageFile.c_str()), stage))
        {
            SummitCompleteStageRequest(request);
            return;
        }
    }

    if (request->cancelled.load(std::memory_order_acquire))
    {
        SummitCompleteStageRequest(request);
        return;
    }

    request->objects.resize(request->objectSpecs.size());
    summit::core::concurrency::JobSystem &jobs = summit::core::concurrency::JobSystem::Instance();

    // Each static stage object has independent ASE/model data. Fan it out over
    // the pool; JobGroup::Wait cooperatively executes work when this worker is
    // waiting, so nested jobs also work with a forced one-worker pool.
    if (request->objectSpecs.size() > 1)
    {
        summit::core::concurrency::JobGroup objectGroup;
        for (std::size_t i = 0; i < request->objectSpecs.size(); ++i)
        {
            const summit::core::concurrency::JobPriority objectPriority =
                request->targetSlot == 0 ? summit::core::concurrency::JobPriority::Normal
                                         : summit::core::concurrency::JobPriority::Low;
            jobs.Submit(objectGroup, [request, i]() {
                if (!request->cancelled.load(std::memory_order_acquire))
                    SummitParseStageObject(request->objectSpecs[i], request->objects[i]);
            }, objectPriority);
        }
        objectGroup.Wait();
    }
    else if (!request->objectSpecs.empty())
    {
        SummitParseStageObject(request->objectSpecs[0], request->objects[0]);
    }

    if (request->cancelled.load(std::memory_order_acquire))
    {
        SummitCompleteStageRequest(request);
        return;
    }

    if (stage->StageObject)
    {
        for (std::size_t i = 0; i < request->objects.size(); ++i)
        {
            SummitPreparedStageObject &prepared = request->objects[i];
            if (!prepared.pattern)
                continue;

            if (stage->StageObject->AddObjectPrepared(prepared.pattern, prepared.bipPattern))
            {
                prepared.pattern = 0;
                prepared.bipPattern = 0;
            }
        }
    }

    // The legacy stage loader explicitly raised Head only after a successful
    // smSTAGE3D_ReadASE.  The V3 worker bypasses smLoaderThreadProc, so carry
    // that lifecycle transition forward before publishing the prepared stage.
    // GetHeight/other collision queries deliberately treat Head == FALSE as
    // "geometry not resident" even when Vertex/Face/StageArea are populated.
    stage->Head = TRUE;

    request->success = request->stage && request->stage->StageObject && request->stage->Head;
    SummitCompleteStageRequest(request);
}

static bool SummitRequestIsCurrent(const std::shared_ptr<SummitStageLoadRequest> &request)
{
    if (!request || request->targetSlot < 0 || request->targetSlot > 1)
        return false;
    std::lock_guard<std::mutex> lock(gStageStreamingMutex);
    const std::shared_ptr<SummitStageLoadRequest> &pending = gStageStreamingPending[request->targetSlot];
    return pending && pending->id == request->id;
}

static void SummitClearCurrentRequest(const std::shared_ptr<SummitStageLoadRequest> &request)
{
    if (!request || request->targetSlot < 0 || request->targetSlot > 1)
        return;
    std::lock_guard<std::mutex> lock(gStageStreamingMutex);
    std::shared_ptr<SummitStageLoadRequest> &pending = gStageStreamingPending[request->targetSlot];
    if (pending && pending->id == request->id)
        pending.reset();
}

static void SummitAbortBootstrapField(sFIELD *field)
{
    if (!gStageBootstrapPending || !field)
        return;

    // Keep StageField[0] metadata published even when geometry preparation
    // fails. A large amount of legacy gameplay dereferences FieldCode/State
    // whenever OnStageField >= 0, so clearing the metadata would turn a load
    // failure into a null/negative-index crash. A later request can retry the
    // same slot without treating the field as resident (SummitFindStageSlot
    // also requires smGameStage[0]).
    // Do not release gameplay into a world whose primary stage failed.  The
    // old synchronous loader could never expose that state; fail closed behind
    // the bootstrap barrier instead of spawning at an invalid Y.
    gStageBootstrapPending = true;
    gStageBootstrapSpawnResolved = false;
    gStageBootstrapSpawnCoordinatesReady = false;
    gStageBootstrapResolvedFloorY = CLIP_OUT;
    gStageBootstrapVisualStartedAt = 0;
    gStageBootstrapFloorTextures.clear();
    gStageBootstrapPriorityTextures.clear();
    AssetLoadV3SetLoadingBoost(false);
}

static void SummitAbortTransitionForField(sFIELD *field)
{
    if (!field || gFieldTransition.field != field)
        return;
    gFieldTransition.armed = false;
    gFieldTransition.active = false;
    gFieldTransition.field = 0;
}

static DWORD SummitBootstrapVisualMaxWait()
{
    static const DWORD configured = []() -> DWORD
    {
        const char *value = getenv("SUMMIT_STAGE_BOOTSTRAP_VISUAL_WAIT_MS");
        if (!value || !*value)
            return 8000u;
        const unsigned long parsed = strtoul(value, 0, 10);
        if (parsed < 1000ul) return 1000u;
        if (parsed > 20000ul) return 20000u;
        return static_cast<DWORD>(parsed);
    }();
    return configured;
}

static std::size_t SummitBootstrapPriorityTextureLimit()
{
    static const std::size_t configured = []() -> std::size_t
    {
        const char *value = getenv("SUMMIT_STAGE_BOOTSTRAP_LOCAL_TEXTURES");
        if (!value || !*value)
            return 24u;
        const unsigned long parsed = strtoul(value, 0, 10);
        if (parsed < 4ul) return 4u;
        if (parsed > 64ul) return 64u;
        return static_cast<std::size_t>(parsed);
    }();
    return configured;
}

static void SummitAppendUniqueTexture(std::vector<smTEXTUREHANDLE *> &textures,
    smTEXTUREHANDLE *texture, std::size_t limit)
{
    if (!texture || texture->UseCounter <= 0 || textures.size() >= limit)
        return;
    for (std::size_t i = 0; i < textures.size(); ++i)
        if (textures[i] == texture)
            return;
    textures.push_back(texture);
}

static void SummitAppendMaterialTextures(const smMATERIAL *material,
    std::vector<smTEXTUREHANDLE *> &textures, std::size_t limit, bool firstAnimationFrameOnly)
{
    if (!material)
        return;
    for (DWORD index = 0; index < material->TextureCounter && textures.size() < limit; ++index)
        SummitAppendUniqueTexture(textures, material->smTexture[index], limit);

    const DWORD animationCount = firstAnimationFrameOnly && material->AnimTexCounter > 0
        ? 1u : material->AnimTexCounter;
    for (DWORD index = 0; index < animationCount && textures.size() < limit; ++index)
        SummitAppendUniqueTexture(textures, material->smAnimTexture[index], limit);
}

static void SummitRequestBootstrapTextureSet(const std::vector<smTEXTUREHANDLE *> &textures)
{
    for (std::size_t index = 0; index < textures.size(); ++index)
    {
        smTEXTUREHANDLE *texture = textures[index];
        if (!texture)
            continue;
        const int readiness = AssetLoadV3GetTextureReadiness(texture);
        if (readiness == ASSET_LOAD_V3_TEXTURE_RESIDENT ||
            readiness == ASSET_LOAD_V3_TEXTURE_PENDING ||
            readiness == ASSET_LOAD_V3_TEXTURE_FAILED_PERMANENT)
            continue;
        AssetLoadV3RequestTexture(texture, 1, 0);
    }
}

struct SummitBootstrapTextureDiagnosticCounts
{
    SummitBootstrapTextureDiagnosticCounts()
        : resident(0), pending(0), retryable(0), failedPermanent(0), ignored(0) {}
    int resident;
    int pending;
    int retryable;
    int failedPermanent;
    int ignored;
};

static SummitBootstrapTextureDiagnosticCounts SummitCountBootstrapTextureStates(
    const std::vector<smTEXTUREHANDLE *> &textures)
{
    SummitBootstrapTextureDiagnosticCounts counts;
    for (std::size_t index = 0; index < textures.size(); ++index)
    {
        smTEXTUREHANDLE *texture = textures[index];
        if (!texture || texture->UseCounter <= 0)
        {
            ++counts.ignored;
            continue;
        }

        switch (AssetLoadV3GetTextureReadiness(texture))
        {
        case ASSET_LOAD_V3_TEXTURE_RESIDENT: ++counts.resident; break;
        case ASSET_LOAD_V3_TEXTURE_PENDING: ++counts.pending; break;
        case ASSET_LOAD_V3_TEXTURE_RETRYABLE: ++counts.retryable; break;
        case ASSET_LOAD_V3_TEXTURE_FAILED_PERMANENT: ++counts.failedPermanent; break;
        default: ++counts.ignored; break;
        }
    }
    return counts;
}

static void SummitLogBootstrapStateRateLimited()
{
    if (!gStageBootstrapPending)
        return;

    const DWORD now = GetCurrentTime();
    if (gStageBootstrapDiagnosticLastStateAt &&
        now - gStageBootstrapDiagnosticLastStateAt < 1000u)
        return;
    gStageBootstrapDiagnosticLastStateAt = now;

    std::uint64_t pendingId0 = 0;
    std::uint64_t pendingId1 = 0;
    std::size_t completedCount = 0;
    {
        std::lock_guard<std::mutex> lock(gStageStreamingMutex);
        if (gStageStreamingPending[0]) pendingId0 = gStageStreamingPending[0]->id;
        if (gStageStreamingPending[1]) pendingId1 = gStageStreamingPending[1]->id;
        completedCount = gStageStreamingCompleted.size();
    }

    const SummitBootstrapTextureDiagnosticCounts floorCounts =
        SummitCountBootstrapTextureStates(gStageBootstrapFloorTextures);
    const SummitBootstrapTextureDiagnosticCounts priorityCounts =
        SummitCountBootstrapTextureStates(gStageBootstrapPriorityTextures);

    int heightProbe = CLIP_OUT;
    if (gStageBootstrapSpawnCoordinatesReady && smGameStage[0] && smGameStage[0]->Head)
        heightProbe = smGameStage[0]->GetHeight(gStageBootstrapSpawnX, gStageBootstrapSpawnZ);

    const DWORD visualElapsed = gStageBootstrapVisualStartedAt
        ? now - gStageBootstrapVisualStartedAt : 0u;

    char detail[1024] = { 0 };
    sprintf_s(detail, sizeof(detail),
        "pending=%d coords=%d x=%d z=%d stage0=%p field0=%p spawnResolved=%d floorY=%d getHeight=%d "
        "player=%p playerXYZ=%d,%d,%d floorTex=%u[r=%d p=%d t=%d f=%d i=%d] "
        "priorityTex=%u[r=%d p=%d t=%d f=%d i=%d] visualMs=%lu "
        "stageReq0=%llu stageReq1=%llu completed=%u",
        gStageBootstrapPending ? 1 : 0,
        gStageBootstrapSpawnCoordinatesReady ? 1 : 0,
        gStageBootstrapSpawnX, gStageBootstrapSpawnZ,
        static_cast<void *>(smGameStage[0]), static_cast<void *>(StageField[0]),
        gStageBootstrapSpawnResolved ? 1 : 0, gStageBootstrapResolvedFloorY, heightProbe,
        static_cast<void *>(lpCurPlayer),
        lpCurPlayer ? lpCurPlayer->pX : 0,
        lpCurPlayer ? lpCurPlayer->pY : 0,
        lpCurPlayer ? lpCurPlayer->pZ : 0,
        static_cast<unsigned>(gStageBootstrapFloorTextures.size()),
        floorCounts.resident, floorCounts.pending, floorCounts.retryable,
        floorCounts.failedPermanent, floorCounts.ignored,
        static_cast<unsigned>(gStageBootstrapPriorityTextures.size()),
        priorityCounts.resident, priorityCounts.pending, priorityCounts.retryable,
        priorityCounts.failedPermanent, priorityCounts.ignored,
        static_cast<unsigned long>(visualElapsed),
        static_cast<unsigned long long>(pendingId0),
        static_cast<unsigned long long>(pendingId1),
        static_cast<unsigned>(completedCount));
    SummitBootstrapDiagnosticEvent("state", detail);
}

static bool SummitResolveBootstrapSpawnSurface(int slot, int x, int z,
    int &outHeight, smSTAGE_FACE *&outFace)
{
    outHeight = CLIP_OUT;
    outFace = 0;
    if (slot < 0 || slot > 1 || !smGameStage[slot] || !smGameStage[slot]->Head)
        return false;

    smSTAGE3D &stage = *smGameStage[slot];

    // Preserve the exact synchronous bootstrap contract. SetPosi(y == 0) used
    // smSTAGE3D::GetHeight(x,z) after the stage was already resident, so the
    // authoritative spawn Y must come from that same query. Do not derive Y
    // from lpCurPlayer->pY or from a parallel face-selection heuristic.
    const int legacyHeight = stage.GetHeight(x, z);
    const int legacyNoFloor = -200 * fONE;
    if (legacyHeight <= legacyNoFloor)
        return false;

    outHeight = legacyHeight;

    // The face is needed only to identify the floor material for visual
    // prioritisation. Failure to recover the face must not invalidate a Y that
    // the legacy GetHeight query has already proven valid.
    smSTAGE_FACE *faces[summit::rendering::legacy3d::stage_area_query::LegacyFaceBufferCapacity] = { 0 };
    const int faceCount = summit::rendering::legacy3d::stage_area_query::CollectFacesAroundPoint(
        stage, x, z, INT_MAX, INT_MIN, faces,
        summit::rendering::legacy3d::stage_area_query::LegacyFaceBufferCapacity);

    int bestDelta = INT_MAX;
    for (int index = 0; index < faceCount; ++index)
    {
        smSTAGE_FACE *face = faces[index];
        if (!face || !summit::rendering::legacy3d::stage_surface::IsSolidSurface(stage, *face))
            continue;

        const int candidate = summit::rendering::legacy3d::stage_height::FaceHeightAtXZ(
            stage, *face, x, z);
        if (candidate == CLIP_OUT)
            continue;

        const int delta = candidate >= legacyHeight
            ? candidate - legacyHeight : legacyHeight - candidate;
        if (delta < bestDelta)
        {
            bestDelta = delta;
            outFace = face;
            if (delta == 0)
                break;
        }
    }

    return true;
}

static void SummitPrepareBootstrapVisualSet(int slot)
{
    gStageBootstrapFloorTextures.clear();
    gStageBootstrapPriorityTextures.clear();
    gStageBootstrapVisualStartedAt = 0;

    if (slot < 0 || slot > 1 || !smGameStage[slot] ||
        !smGameStage[slot]->smMaterialGroup || !gStageBootstrapSpawnResolved ||
        !gStageBootstrapSpawnCoordinatesReady)
        return;

    smSTAGE3D &stage = *smGameStage[slot];
    smSTAGE_FACE *floorFace = 0;
    int floorHeight = CLIP_OUT;
    if (!SummitResolveBootstrapSpawnSurface(
        slot, gStageBootstrapSpawnX, gStageBootstrapSpawnZ, floorHeight, floorFace))
    {
        gStageBootstrapSpawnResolved = false;
        gStageBootstrapResolvedFloorY = CLIP_OUT;
        return;
    }

    // The visual gate and the physical spawn now use the exact same surface.
    // This prevents a provisional player Y from selecting a different lower
    // face/material and declaring the wrong ground texture ready.
    gStageBootstrapResolvedFloorY = floorHeight;
    gStageBootstrapVisualStartedAt = GetCurrentTime();

    smSTAGE_FACE *nearFaces[256] = { 0 };
    const int verticalRange = 256 * fONE;
    const int faceCount = summit::rendering::legacy3d::stage_area_query::CollectFacesAroundPoint(
        stage, gStageBootstrapSpawnX, gStageBootstrapSpawnZ,
        floorHeight + verticalRange, floorHeight - verticalRange,
        nearFaces, sizeof(nearFaces) / sizeof(nearFaces[0]));

    if (floorFace)
    {
        const smMATERIAL *floorMaterial =
            summit::rendering::legacy3d::stage_surface::MaterialForFace(stage, *floorFace);
        SummitAppendMaterialTextures(floorMaterial, gStageBootstrapFloorTextures, 8u, true);
        SummitAppendMaterialTextures(floorMaterial, gStageBootstrapPriorityTextures,
            SummitBootstrapPriorityTextureLimit(), false);
    }

    // Warm the materials immediately surrounding the spawn before generic map
    // prewarm starts.  This avoids spending the first seconds on distant map
    // textures while the floor under the character is still unresolved.
    const std::size_t priorityLimit = SummitBootstrapPriorityTextureLimit();
    for (int index = 0; index < faceCount && gStageBootstrapPriorityTextures.size() < priorityLimit; ++index)
    {
        smSTAGE_FACE *face = nearFaces[index];
        if (!face)
            continue;
        const smMATERIAL *material =
            summit::rendering::legacy3d::stage_surface::MaterialForFace(stage, *face);
        SummitAppendMaterialTextures(material, gStageBootstrapPriorityTextures, priorityLimit, true);
    }

    SummitRequestBootstrapTextureSet(gStageBootstrapFloorTextures);
    SummitRequestBootstrapTextureSet(gStageBootstrapPriorityTextures);

    char detail[160] = { 0 };
    sprintf_s(detail, sizeof(detail), "floorY=%d floorTextures=%u priorityTextures=%u",
        gStageBootstrapResolvedFloorY,
        static_cast<unsigned>(gStageBootstrapFloorTextures.size()),
        static_cast<unsigned>(gStageBootstrapPriorityTextures.size()));
    SummitBootstrapDiagnosticEvent("bootstrap-visual-set", detail);
}

static bool SummitBootstrapFloorVisualReady()
{
    if (!gStageBootstrapPending)
        return true;
    if (!smGameStage[0] || !StageField[0] || !gStageBootstrapSpawnResolved)
        return false;

    auto textureSetReady = [](const std::vector<smTEXTUREHANDLE *> &textures) -> bool
    {
        bool ready = true;
        for (std::size_t index = 0; index < textures.size(); ++index)
        {
            smTEXTUREHANDLE *texture = textures[index];
            if (!texture || texture->UseCounter <= 0)
                continue;

            const int state = AssetLoadV3GetTextureReadiness(texture);
            if (state == ASSET_LOAD_V3_TEXTURE_RESIDENT ||
                state == ASSET_LOAD_V3_TEXTURE_FAILED_PERMANENT)
                continue;

            ready = false;
            if (state == ASSET_LOAD_V3_TEXTURE_RETRYABLE)
                AssetLoadV3RequestTexture(texture, 1, 0);
        }
        return ready;
    };

    // The actual floor material is non-negotiable: never show the character
    // while its resolved ground is still waiting on an upload.  An empty set is
    // valid only after a solid floor face was resolved (for example an
    // intentionally untextured material).
    const bool floorResident = textureSetReady(gStageBootstrapFloorTextures);
    if (!floorResident)
        return false;

    // Nearby materials make the first visible frame coherent instead of
    // letting walls/props pop in around an already visible character.  This
    // secondary set is bounded and may time out; the physical floor may not.
    if (textureSetReady(gStageBootstrapPriorityTextures))
        return true;

    return gStageBootstrapVisualStartedAt != 0 &&
        GetCurrentTime() - gStageBootstrapVisualStartedAt >= SummitBootstrapVisualMaxWait();
}

static void SummitTryReleaseBootstrapVisualBarrier()
{
    if (!gStageBootstrapPending || !gStageBootstrapSpawnResolved ||
        !SummitBootstrapFloorVisualReady())
        return;
    SummitBootstrapDiagnosticEvent("bootstrap-release");
    gStageBootstrapPending = false;
    gStageBootstrapSpawnResolved = false;
    gStageBootstrapSpawnCoordinatesReady = false;
    gStageBootstrapResolvedFloorY = CLIP_OUT;
    gStageBootstrapVisualStartedAt = 0;
    gStageBootstrapFloorTextures.clear();
    gStageBootstrapPriorityTextures.clear();
    AssetLoadV3SetLoadingBoost(false);
}

static bool SummitFinalizeBootstrapPlayerPlacement(int slot)
{
    if (!gStageBootstrapPending || slot < 0 || slot > 1 || !lpCurPlayer ||
        !smGameStage[slot] || !gStageBootstrapSpawnCoordinatesReady)
        return false;

    int floorHeight = CLIP_OUT;
    smSTAGE_FACE *floorFace = 0;
    if (!SummitResolveBootstrapSpawnSurface(
        slot, gStageBootstrapSpawnX, gStageBootstrapSpawnZ, floorHeight, floorFace))
    {
        gStageBootstrapSpawnResolved = false;
        gStageBootstrapResolvedFloorY = CLIP_OUT;
        lpCurPlayer->MoveFlag = 0;
        lpCurPlayer->MoveMode = FALSE;
        return false;
    }

    // Reproduce the semantic guarantee of the old synchronous bootstrap: the
    // first playable frame starts on a real collision surface.  Never use the
    // temporary pY produced while the stage was absent as an input here.
    lpCurPlayer->pX = gStageBootstrapSpawnX;
    lpCurPlayer->pZ = gStageBootstrapSpawnZ;
    lpCurPlayer->pY = floorHeight;
    lpCurPlayer->PHeight = floorHeight;
    lpCurPlayer->FallHeight = 0;
    lpCurPlayer->OnStageField = slot;
    lpCurPlayer->MoveFlag = 0;
    lpCurPlayer->MoveMode = FALSE;
    gStageBootstrapSpawnResolved = true;
    gStageBootstrapResolvedFloorY = floorHeight;

    TraceCameraPosi.x = gStageBootstrapSpawnX;
    TraceCameraPosi.y = floorHeight;
    TraceCameraPosi.z = gStageBootstrapSpawnZ;
    TraceTargetPosi = TraceCameraPosi;

    char detail[160] = { 0 };
    sprintf_s(detail, sizeof(detail), "slot=%d x=%d z=%d floorY=%d floorFace=%p",
        slot, gStageBootstrapSpawnX, gStageBootstrapSpawnZ, floorHeight,
        static_cast<void *>(floorFace));
    SummitBootstrapDiagnosticEvent("bootstrap-spawn-resolved", detail);
    return true;
}

static void SummitPublishStageOnMainThread(const std::shared_ptr<SummitStageLoadRequest> &request)
{
    if (!request)
        return;

    if (!SummitRequestIsCurrent(request))
    {
        SummitDeletePreparedStageRequest(request);
        return;
    }
    SummitClearCurrentRequest(request);

    if (request->cancelled.load(std::memory_order_acquire) || !request->success || !request->stage)
    {
        char detail[256] = { 0 };
        sprintf_s(detail, sizeof(detail),
            "id=%llu slot=%d success=%d cancelled=%d stage=%p",
            static_cast<unsigned long long>(request->id), request->targetSlot,
            request->success ? 1 : 0,
            request->cancelled.load(std::memory_order_acquire) ? 1 : 0,
            static_cast<void *>(request->stage));
        SummitBootstrapDiagnosticEvent("stage-publish-failed", detail);
        SummitAbortTransitionForField(request->field);
        SummitAbortBootstrapField(request->field);
        SummitDeletePreparedStageRequest(request);
        return;
    }

    const int targetSlot = request->targetSlot;
    if (targetSlot < 0 || targetSlot > 1)
    {
        SummitDeletePreparedStageRequest(request);
        return;
    }

    if (request->field && SummitFindStageSlot(request->field) >= 0)
    {
        SummitDeletePreparedStageRequest(request);
        return;
    }

    // A primary transition owns slot 1 as its rollback/previous-world slot.
    // Cancel any unrelated speculative neighbour still targeting that slot.
    if (targetSlot == 0)
    {
        std::lock_guard<std::mutex> lock(gStageStreamingMutex);
        if (gStageStreamingPending[1])
            gStageStreamingPending[1]->cancelled.store(true, std::memory_order_release);
    }

    // A primary transition keeps the previous primary stage as slot 1. This
    // lets the player remain safely in the old world while target textures are
    // warming and also makes walking back through the gate instant.
    if (targetSlot == 0 && smGameStage[0] && StageField[0] != request->field)
    {
        if (smGameStage[1] && smGameStage[1] != smGameStage[0])
            delete smGameStage[1];
        smGameStage[1] = smGameStage[0];
        StageField[1] = StageField[0];
        gStageTexturePrewarm[1] = 0;
        if (StageField[1])
            LoadFieldMap(1, StageField[1], smGameStage[1]);
        if (lpCurPlayer && lpCurPlayer->OnStageField == 0)
            lpCurPlayer->OnStageField = 1;
        smGameStage[0] = 0;
        StageField[0] = 0;
    }
    else if (smGameStage[targetSlot])
    {
        delete smGameStage[targetSlot];
        smGameStage[targetSlot] = 0;
        StageField[targetSlot] = 0;
    }

    smGameStage[targetSlot] = request->stage;
    StageField[targetSlot] = request->field;
    request->stage = 0;

    {
        char detail[256] = { 0 };
        sprintf_s(detail, sizeof(detail),
            "id=%llu slot=%d stage=%p head=%d field=%p bootstrapPending=%d",
            static_cast<unsigned long long>(request->id), targetSlot,
            static_cast<void *>(smGameStage[targetSlot]),
            smGameStage[targetSlot] && smGameStage[targetSlot]->Head ? 1 : 0,
            static_cast<void *>(StageField[targetSlot]),
            gStageBootstrapPending ? 1 : 0);
        SummitBootstrapDiagnosticEvent("stage-published", detail);
    }

    const bool bootstrapPrimary = targetSlot == 0 && gStageBootstrapPending;

    // LoadFieldMap ends in a global LOW-priority ReadTextures sweep. During the
    // first-world bootstrap, queue the resolved floor/local textures first so
    // generic minimap/UI/global work cannot occupy every I/O slot ahead of the
    // ground that gates spawn. Runtime transitions keep their legacy ordering.
    if (request->field && !bootstrapPrimary)
        LoadFieldMap(targetSlot, request->field, smGameStage[targetSlot]);

    // Bootstrap is allowed to publish FIELD metadata early for legacy callers,
    // but the player must not become visible/playable until collision geometry
    // exists.  Snap to the real floor on the owner thread before the renderer
    // is allowed to leave the loading barrier.
    if (targetSlot == 0 && gStageBootstrapPending)
        SummitFinalizeBootstrapPlayerPlacement(targetSlot);

    if (request->field)
        strcpy_s(szGameStageName[targetSlot], request->field->szName);
    else
        strcpy_s(szGameStageName[targetSlot], sizeof(szGameStageName[targetSlot]), request->stageFile.c_str());

    // Register stage texture ownership immediately; actual disk/decode work is
    // queued to workers by PumpStageTexturePrewarm().
    if (smGameStage[targetSlot]->smMaterialGroup)
    {
        // Register stage ownership first. During the initial world bootstrap do
        // not start the generic material-order prewarm yet: first queue the
        // floor/local visual set so the character never appears above an
        // untextured ground while distant textures consume all I/O slots.
        AssetLoadV2PrewarmMaterialGroupStep(
            smGameStage[targetSlot]->smMaterialGroup, SMASSET_OWNER_STAGE, 1,
            bootstrapPrimary ? 0u : 1u, targetSlot == 0 ? 0 : 2);
        gStageTexturePrewarm[targetSlot] = smGameStage[targetSlot];
        if (bootstrapPrimary && gStageBootstrapSpawnResolved)
            SummitPrepareBootstrapVisualSet(targetSlot);
    }
    else
    {
        gStageTexturePrewarm[targetSlot] = 0;
        if (targetSlot == 0 && gStageBootstrapPending)
            SummitTryReleaseBootstrapVisualBarrier();
    }

    if (request->field && bootstrapPrimary)
        LoadFieldMap(targetSlot, request->field, smGameStage[targetSlot]);
}

static int SummitResolveRequestSlot(sFIELD *field, sFIELD *anchorField)
{
    if (!anchorField)
        return 0;
    if (anchorField == StageField[0])
        return 1;
    if (anchorField == StageField[1])
        return 0;
    return -1;
}

static bool SummitStageRequestAlreadyPending(sFIELD *field)
{
    if (!field)
        return false;
    std::lock_guard<std::mutex> lock(gStageStreamingMutex);
    for (int slot = 0; slot < 2; ++slot)
    {
        if (gStageStreamingPending[slot] && gStageStreamingPending[slot]->field == field &&
            !gStageStreamingPending[slot]->cancelled.load(std::memory_order_acquire))
            return true;
    }
    return false;
}

static int SummitQueueStageRequest(const std::shared_ptr<SummitStageLoadRequest> &request)
{
    if (!request || request->targetSlot < 0 || request->targetSlot > 1)
        return FALSE;

    {
        std::lock_guard<std::mutex> lock(gStageStreamingMutex);
        std::shared_ptr<SummitStageLoadRequest> &previous = gStageStreamingPending[request->targetSlot];
        if (previous)
            previous->cancelled.store(true, std::memory_order_release);
        previous = request;
    }

    {
        char detail[256] = { 0 };
        sprintf_s(detail, sizeof(detail), "id=%llu slot=%d file=%s",
            static_cast<unsigned long long>(request->id), request->targetSlot,
            request->stageFile.c_str());
        SummitBootstrapDiagnosticEvent("stage-queued", detail);
    }

    if (!summit::core::concurrency::JobSystem::Instance().Submit(
        [request]() { SummitBuildStageOnWorker(request); },
        request->targetSlot == 0 ? summit::core::concurrency::JobPriority::High
                                 : summit::core::concurrency::JobPriority::Low))
    {
        std::lock_guard<std::mutex> lock(gStageStreamingMutex);
        std::shared_ptr<SummitStageLoadRequest> &pending = gStageStreamingPending[request->targetSlot];
        if (pending && pending->id == request->id)
            pending.reset();
        SummitBootstrapDiagnosticEvent("stage-submit-failed");
        return FALSE;
    }
    return TRUE;
}

static std::shared_ptr<SummitStageLoadRequest> SummitMakeFieldStageRequest(
    sFIELD *field, sFIELD *anchorField, int targetSlot)
{
    if (!field || !field->szName[0])
        return std::shared_ptr<SummitStageLoadRequest>();

    std::shared_ptr<SummitStageLoadRequest> request(new SummitStageLoadRequest());
    request->id = gStageStreamingSerial.fetch_add(1u, std::memory_order_acq_rel);
    request->field = field;
    request->anchorField = anchorField;
    request->targetSlot = targetSlot;
    request->stageFile = field->szName;
    request->lightX = smConfig.MapLightVector.x;
    request->lightY = smConfig.MapLightVector.y;
    request->lightZ = smConfig.MapLightVector.z;
    request->bright = smConfig.MapBright;
    request->contrast = smConfig.MapContrast;

    request->objectSpecs.reserve(field->StgObjCount);
    for (int cnt = 0; cnt < field->StgObjCount; ++cnt)
    {
        char objectName[128] = { 0 };
        const int bip = field->GetStageObjectName(cnt, objectName, sizeof(objectName));
        if (!objectName[0])
            continue;
        SummitStageObjectSpec spec;
        spec.file = objectName;
        spec.hasBip = bip != 0;
        request->objectSpecs.push_back(spec);
    }
    return request;
}

static std::shared_ptr<SummitStageLoadRequest> SummitMakeRawStageRequest(
    const char *stageFile, int targetSlot)
{
    if (!stageFile || !stageFile[0] || targetSlot < 0 || targetSlot > 1)
        return std::shared_ptr<SummitStageLoadRequest>();

    std::shared_ptr<SummitStageLoadRequest> request(new SummitStageLoadRequest());
    request->id = gStageStreamingSerial.fetch_add(1u, std::memory_order_acq_rel);
    request->targetSlot = targetSlot;
    request->stageFile = stageFile;
    request->lightX = smConfig.MapLightVector.x;
    request->lightY = smConfig.MapLightVector.y;
    request->lightZ = smConfig.MapLightVector.z;
    request->bright = smConfig.MapBright;
    request->contrast = smConfig.MapContrast;
    return request;
}

static void SummitArmFieldTransition(sFIELD *field)
{
    if (!field || !lpCurPlayer)
        return;
    if (!smGameStage[0] && !smGameStage[1])
        return; // bootstrap: there is no old world to preserve.
    if (SummitFindStageSlot(field) >= 0)
        return;

    gFieldTransition.armed = true;
    gFieldTransition.active = false;
    gFieldTransition.field = field;
    gFieldTransition.armedAt = GetCurrentTime();
    if (field->FieldCode == 53 || field->FieldCode == 54 || field->FieldCode == 58)
    {
        char detail[256] = { 0 };
        sprintf_s(detail, sizeof(detail), "map=%d playerXYZ=%d,%d,%d target=%p",
            field->FieldCode, lpCurPlayer->pX, lpCurPlayer->pY,
            lpCurPlayer->pZ, static_cast<void *>(field));
        SummitBootstrapDiagnosticEvent("wsb-transition-armed", detail);
    }
}

static bool SummitTargetTexturesWarming(int slot)
{
    return slot >= 0 && slot < 2 && gStageTexturePrewarm[slot] != 0;
}

static void SummitTryCommitFieldTransition()
{
    if (!gFieldTransition.active || !gFieldTransition.field || !lpCurPlayer)
        return;

    const int slot = SummitFindStageSlot(gFieldTransition.field);
    if (slot < 0)
    {
        lpCurPlayer->MoveFlag = 0;
        return;
    }

    const DWORD now = GetCurrentTime();
    if (SummitTargetTexturesWarming(slot) &&
        now - gFieldTransition.startedAt < SummitStageTransitionMaxWarmWait())
    {
        lpCurPlayer->MoveFlag = 0;
        return;
    }

    gFieldTransition.committing = true;
    lpCurPlayer->SetPosi(
        gFieldTransition.x, gFieldTransition.y, gFieldTransition.z,
        gFieldTransition.ax, gFieldTransition.ay, gFieldTransition.az);
    gFieldTransition.committing = false;

    if (gFieldTransition.field->FieldCode == 53 ||
        gFieldTransition.field->FieldCode == 54 ||
        gFieldTransition.field->FieldCode == 58)
    {
        const int floorHeight = smGameStage[slot] && smGameStage[slot]->Head
            ? smGameStage[slot]->GetHeight(lpCurPlayer->pX, lpCurPlayer->pZ)
            : CLIP_OUT;
        char detail[320] = { 0 };
        sprintf_s(detail, sizeof(detail),
            "map=%d slot=%d requestedXYZ=%d,%d,%d actualXYZ=%d,%d,%d getHeight=%d move=%d",
            gFieldTransition.field->FieldCode, slot,
            gFieldTransition.x, gFieldTransition.y, gFieldTransition.z,
            lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ,
            floorHeight, lpCurPlayer->MoveFlag);
        SummitBootstrapDiagnosticEvent("wsb-transition-committed", detail);
    }

    lpCurPlayer->OnStageField = slot;
    lpCurPlayer->MoveFlag = 0;
    TraceCameraPosi.x = lpCurPlayer->pX;
    TraceCameraPosi.y = lpCurPlayer->pY;
    TraceCameraPosi.z = lpCurPlayer->pZ;
    TraceTargetPosi = TraceCameraPosi;

    sFIELD *committedField = gFieldTransition.field;
    gFieldTransition.armed = false;
    gFieldTransition.active = false;
    gFieldTransition.field = 0;

    // Keep the normal two-stage cache hot after a teleport as well. This is a
    // speculative request and never blocks the frame.
    if (committedField && committedField->GateCount > 0 && committedField->FieldGate[0].lpsField)
        RequestStageFromFieldAsync(committedField->FieldGate[0].lpsField, committedField);
}

} // anonymous namespace

// Two-stage streaming: CPU work scales with the worker pool while GPU uploads
// are fed incrementally on the render thread.
static std::uint32_t StageTexturePrewarmPerTick()
{
    const char *value = getenv("SUMMIT_STAGE_STREAM_TEXTURES_PER_TICK");
    if (value && *value)
    {
        const long parsed = strtol(value, 0, 10);
        if (parsed <= 0) return 0u;
        return static_cast<std::uint32_t>(parsed > 16 ? 16 : parsed);
    }

    const std::size_t workers = summit::core::concurrency::JobSystem::Instance().WorkerCount();
    const std::size_t automatic = workers < 2 ? 2 : workers;
    return static_cast<std::uint32_t>(automatic > 8 ? 8 : automatic);
}

void PumpStageTexturePrewarm()
{
    SummitLogBootstrapStateRateLimited();
    if (!d3ddev)
        return;

    // Bootstrap has a two-level readiness contract: geometry/collision first,
    // then the local floor material. Keep generic whole-map prewarm from
    // competing until that local visual set has either become resident or hit
    // the bounded missing-asset timeout.
    if (gStageBootstrapPending && smGameStage[0] && StageField[0])
    {
        // Spawn resolution is fail-closed.  Geometry can be published while the
        // legacy character still carries a placeholder Y from pre-stage SetPosi;
        // retry the pure geometry query here until a real solid surface exists.
        if (!gStageBootstrapSpawnResolved && SummitFinalizeBootstrapPlayerPlacement(0))
            SummitPrepareBootstrapVisualSet(0);

        if (!gStageBootstrapSpawnResolved)
            return;

        SummitRequestBootstrapTextureSet(gStageBootstrapFloorTextures);
        SummitRequestBootstrapTextureSet(gStageBootstrapPriorityTextures);
        SummitTryReleaseBootstrapVisualBarrier();
        if (gStageBootstrapPending)
            return;
    }

    const std::uint32_t totalPerTick = StageTexturePrewarmPerTick();
    if (!totalPerTick)
        return;

    // Slot 0 is not permanently the active map. During a normal walking
    // boundary crossing, legacy collision code can switch OnStageField to 1
    // while both stages remain resident. The old scheduler still gave slot 0
    // all/most of the texture budget, so the map the player had just entered
    // could continue streaming at only one low-priority texture per tick.
    int activeSlot = 0;
    if (lpCurPlayer && lpCurPlayer->OnStageField >= 0 && lpCurPlayer->OnStageField < 2 &&
        smGameStage[lpCurPlayer->OnStageField])
    {
        activeSlot = lpCurPlayer->OnStageField;
    }
    const int neighbourSlot = activeSlot == 0 ? 1 : 0;

    const bool activeWarming = gStageTexturePrewarm[activeSlot] != 0;
    const bool neighbourWarming = gStageTexturePrewarm[neighbourSlot] != 0;

    // Reserve part of the budget for the speculative neighbour so its
    // materials can actually become resident before the player crosses the
    // boundary. With the default 8-attempt budget this is 6 active + 2
    // neighbour; small budgets still guarantee one attempt to each side.
    std::uint32_t neighbourBudget = 0u;
    if (neighbourWarming)
    {
        if (!activeWarming)
            neighbourBudget = totalPerTick;
        else if (totalPerTick > 1u)
        {
            neighbourBudget = totalPerTick / 4u;
            if (!neighbourBudget)
                neighbourBudget = 1u;
        }
    }

    std::uint32_t activeBudget = activeWarming ? totalPerTick - neighbourBudget : 0u;
    if (activeWarming && !activeBudget)
    {
        activeBudget = 1u;
        if (neighbourBudget)
            --neighbourBudget;
    }

    const int slots[2] = { activeSlot, neighbourSlot };
    const std::uint32_t budgets[2] = { activeBudget, neighbourBudget };
    for (int pass = 0; pass < 2; ++pass)
    {
        const int slot = slots[pass];
        const std::uint32_t stageBudget = budgets[pass];
        if (!stageBudget)
            continue;

        smSTAGE3D *stage = gStageTexturePrewarm[slot];
        if (!stage)
            continue;
        if (stage != smGameStage[slot] || !stage->smMaterialGroup)
        {
            gStageTexturePrewarm[slot] = 0;
            continue;
        }

        const bool isActive = slot == activeSlot;
        const int remaining = AssetLoadV2PrewarmMaterialGroupStep(
            stage->smMaterialGroup, SMASSET_OWNER_STAGE, 1, stageBudget,
            isActive ? 0 : 1);
        if (remaining == 0)
        {
            stage->smMaterialGroup->CheckMatreialTextureSwap();
            gStageTexturePrewarm[slot] = 0;
        }
    }
}

int RequestStageFromFieldAsync(sFIELD *lpField, sFIELD *lpSecondField)
{
    if (!lpField || !lpField->szName[0])
        return FALSE;
    if (SummitFindStageSlot(lpField) >= 0)
        return TRUE;

    // Speculative neighbour requests must not race the primary bootstrap.
    // StageField metadata is published early so legacy code can inspect
    // FieldCode/State, but no adjacent-map work is useful until the anchor
    // stage has actual collision/geometry resident.
    if (lpSecondField && SummitFindStageSlot(lpSecondField) < 0)
        return FALSE;

    if (SummitStageRequestAlreadyPending(lpField))
        return FALSE;

    const int targetSlot = SummitResolveRequestSlot(lpField, lpSecondField);
    if (targetSlot < 0)
        return FALSE;

    return SummitQueueStageRequest(SummitMakeFieldStageRequest(lpField, lpSecondField, targetSlot));
}

void PumpStageStreamingV3()
{
    // Publish at most two CPU-ready stages per frame. Publication does no disk
    // I/O and no texture decode; those are already worker-side / queued.
    for (int published = 0; published < 2; ++published)
    {
        std::shared_ptr<SummitStageLoadRequest> request;
        {
            std::lock_guard<std::mutex> lock(gStageStreamingMutex);
            if (gStageStreamingCompleted.empty())
                break;
            request = gStageStreamingCompleted.front();
            gStageStreamingCompleted.pop_front();
        }
        SummitPublishStageOnMainThread(request);
    }

    // An armed transition should be consumed by SetPosi in the same legacy
    // call chain. Expire it defensively instead of intercepting an unrelated
    // SetPosi much later.
    if (gFieldTransition.armed && GetCurrentTime() - gFieldTransition.armedAt > 500u)
    {
        gFieldTransition.armed = false;
        if (!gFieldTransition.active)
            gFieldTransition.field = 0;
    }

    SummitTryCommitFieldTransition();
}

bool IsStageFromFieldPendingAsync(sFIELD *lpField)
{
    if (!lpField)
        return false;
    if (SummitFindStageSlot(lpField) >= 0)
        return false;
    return SummitStageRequestAlreadyPending(lpField);
}

bool IsStageBootstrapBlockingGameplayAsync()
{
    // Keep gameplay and world rendering behind the barrier for the whole
    // bootstrap contract, not merely until the stage pointer is published.
    // gStageBootstrapPending now covers geometry/collision plus the floor/local
    // visual set around the spawn point.
    return gStageBootstrapPending;
}

// Called from smCHAR::SetPosi. Runtime warps still use the legacy call graph,
// but their SetPosi is captured instead of moving the player into an unloaded
// world. Stage publication later replays the exact position on the main thread.
bool SummitCapturePendingFieldTransitionSetPosi(smCHAR *character,
    int x, int y, int z, int ax, int ay, int az)
{
    if (!character || character != lpCurPlayer || gFieldTransition.committing)
        return false;
    if (!gFieldTransition.armed || !gFieldTransition.field)
        return false;
    if (GetCurrentTime() - gFieldTransition.armedAt > 500u)
    {
        gFieldTransition.armed = false;
        gFieldTransition.field = 0;
        return false;
    }

    gFieldTransition.armed = false;
    gFieldTransition.active = true;
    gFieldTransition.startedAt = GetCurrentTime();
    gFieldTransition.x = x;
    gFieldTransition.y = y;
    gFieldTransition.z = z;
    gFieldTransition.ax = ax;
    gFieldTransition.ay = ay;
    gFieldTransition.az = az;

    character->MoveFlag = 0;
    if (gFieldTransition.field &&
        (gFieldTransition.field->FieldCode == 53 ||
         gFieldTransition.field->FieldCode == 54 ||
         gFieldTransition.field->FieldCode == 58))
    {
        char detail[256] = { 0 };
        sprintf_s(detail, sizeof(detail), "map=%d requestedXYZ=%d,%d,%d",
            gFieldTransition.field->FieldCode, x, y, z);
        SummitBootstrapDiagnosticEvent("wsb-transition-captured", detail);
    }
    return true;
}


rsRECORD_DBASE	rsRecorder;

char szGameStageName[2][64];
DWORD			dwPlayTime = 0;
DWORD			dwMemError = 8080;

int	smStageCount = 0;
int SkipNextField = 0;

int	NightDayTime = 0;

int invPlayerArmor = 0;

DWORD	dwBattleTime = 0;
DWORD	dwBattleQuitTime = 0;
DWORD	dwCloseBoxTime = 0;

int		RestartPlayCount = 0;

DWORD SkillFalconPoint;

int InvArmorFileCode[] =
{ 0 ,9,1, 5 };

int	DarkLevel = 0;
int	BackColor_R = 0;
int	BackColor_G = 0;
int	BackColor_B = 0;

int	BackColor_mR = 0;
int	BackColor_mG = 0;
int	BackColor_mB = 0;

int	DarkLevel_Fix = 0;

int	DarkLightRange = 180 * fONE;

DWORD	dwGameWorldTime = 0;
DWORD	dwGameHour = 12;
DWORD	dwGameMin = 0;
DWORD	dwGameTimeOffset = 0;

int DarkLight = 0;


int WeatherMode = 0;
int Thunder = 0;


int	PkMode = 0;

int DispApp_SkillPoint = 0;

int rcnt2;

int modelX = 255;
int modelZ = 512;
int wflag = 0;
int wrad = 0;

DWORD	UseEtherCoreCode = 0;

int	RecordFailCount = 0;


int	Record_BlockCount = 0;
int	Record_DefenceCount = 0;
int Record_RecvDamageCount = 0;
int Record_TotalRecvDamageCount = 0;



DWORD	AutoMouse_LastCheckTime = 0;
DWORD	AutoMouse_StartCheckTime = 0;
int		AutoMouse_WM_Count = 0;
int 	AutoMouse_Distance = 0;
int		AutoMouse_Attack_Count = 0;





smPAT3D *PatArrow;

smPAT3D *PatEffectShield = 0;

smPAT3D *PatSparkShield = 0;

smPAT3D *PatDivineShield = 0;

smPAT3D *PatGodlyShield = 0;

smPAT3D *PatCrown = 0;

smPAT3D *PatCrown2 = 0;

smPAT3D *PatCrown3 = 0;

smPAT3D *PatShadow[SHADOW_PAT_MAX] = { 0,0,0,0 };



smPAT3D *PatIceFoot[ICE_FOOT_PAT_MAX];


int	Moving_DblClick = 0;
POINT3D	MousePos_DblClick;

POINT3D	TracePos_DblClick;
int TraceMode_DblClick = 0;

int	ActionGameMode = 0;



smCHAR		*lpCurPlayer = 0;

DWORD	dwRotPlayDelayTime = 0;




scITEM	scItems[DISP_ITEM_MAX];

smCHAR	chrOtherPlayer[OTHER_PLAYER_MAX];

smCHAR_INFO	smCharInfo_Back;


int	MatEachBox;
int	MatEachMenuBox;
int MatEnergyBox[2];
int MatLight[4];

int	MatEachTradeButton[2];
int MatMonBroodImage[8];


smTRANS_COMMAND	sPartyPosion;
DWORD			dwPartyPosionTime;

LPDIRECT3DTEXTURE9	lpDDS_ParTradeButton[4][2];
LPDIRECT3DTEXTURE9	lpDDS_ParTradeToolTip[4];



char *szFlame[8] = {
	"image//flame1.bmp",
	"image//flame2.bmp",
	"image//flame3.bmp",
	"image//flame4.bmp",
	"image//flame5.bmp",
	"image//flame6.bmp",
	"image//flame7.bmp",
	"image//flame8.bmp"
};

char *szShine[] = {
	"image//shine0.bmp",
	"image//shine2.bmp",
	"image//shine4.bmp",
	"image//shine6.bmp",
	"image//shine8.bmp",
	""
};

char *szBlood1[] = {
	"image//blood//b1_001.tga",
	"image//blood//b1_002.tga",
	"image//blood//b1_003.tga",
	"image//blood//b1_004.tga"
};

char *szBlood2[] = {
	"image//blood//b2_001.tga",
	"image//blood//b2_002.tga",
	"image//blood//b2_003.tga",
	"image//blood//b2_004.tga"
};


char *szShadowImageFile = "image//bshadow.bmp";




char *g_IceFootImageFileBuff[ICE_FOOT_PAT_MAX] =
{
	"image//mk-r.tga",
	"image//mk-l.tga",
};





void InitStageLoader()
{
    std::deque<std::shared_ptr<SummitStageLoadRequest> > abandoned;
    {
        std::lock_guard<std::mutex> lock(gStageStreamingMutex);
        for (int slot = 0; slot < 2; ++slot)
        {
            if (gStageStreamingPending[slot])
                gStageStreamingPending[slot]->cancelled.store(true, std::memory_order_release);
            gStageStreamingPending[slot].reset();
        }
        abandoned.swap(gStageStreamingCompleted);
    }
    while (!abandoned.empty())
    {
        SummitDeletePreparedStageRequest(abandoned.front());
        abandoned.pop_front();
    }

    gStageTexturePrewarm[0] = 0;
    gStageTexturePrewarm[1] = 0;
    gFieldTransition = SummitFieldTransition();
    gStageBootstrapPending = false;
    gStageBootstrapSpawnResolved = false;
    gStageBootstrapSpawnCoordinatesReady = false;
    gStageBootstrapResolvedFloorY = CLIP_OUT;
    gStageBootstrapVisualStartedAt = 0;
    gStageBootstrapFloorTextures.clear();
    gStageBootstrapPriorityTextures.clear();
    gStageBootstrapDiagnosticLastStateAt = 0;
    AssetLoadV3SetLoadingBoost(false);

    if (smGameStage[1]) delete smGameStage[1];
    if (smGameStage[0]) delete smGameStage[0];

    szGameStageName[0][0] = 0;
    szGameStageName[1][0] = 0;
    StageField[0] = 0;
    StageField[1] = 0;
    smGameStage[0] = 0;
    smGameStage[1] = 0;
}

smSTAGE3D *LoadStageFromField(sFIELD *lpField, sFIELD *lpSecondField)
{
    if (!lpField || !lpField->szName[0])
        return NULL;

    const int residentSlot = SummitFindStageSlot(lpField);
    if (residentSlot >= 0)
        return smGameStage[residentSlot];

    const bool bootstrap = !lpSecondField && !smGameStage[0] && !smGameStage[1];
    if (bootstrap)
    {
        // Legacy gameplay uses StageField metadata before geometry is needed.
        // Preserve that contract while the heavy stage stays private to the
        // worker until publication.
        StageField[0] = lpField;
        if (lpCurPlayer)
            lpCurPlayer->OnStageField = 0;
        gStageBootstrapPending = true;
        gStageBootstrapSpawnResolved = false;
        gStageBootstrapSpawnCoordinatesReady = false;
        gStageBootstrapResolvedFloorY = CLIP_OUT;
        gStageBootstrapVisualStartedAt = 0;
        gStageBootstrapFloorTextures.clear();
        gStageBootstrapPriorityTextures.clear();
        AssetLoadV3SetLoadingBoost(true);

        char detail[256] = { 0 };
        sprintf_s(detail, sizeof(detail), "field=%p file=%s",
            static_cast<void *>(lpField), lpField->szName);
        SummitBootstrapDiagnosticEvent("bootstrap-begin", detail);
    }

    // A primary request during gameplay is a real field transition. The
    // legacy caller will call SetPosi immediately afterwards; capture that
    // SetPosi and replay it only when this stage is ready.
    if (!lpSecondField)
        SummitArmFieldTransition(lpField);

    const int queued = RequestStageFromFieldAsync(lpField, lpSecondField);
    if (!queued && bootstrap && !IsStageFromFieldPendingAsync(lpField) && SummitFindStageSlot(lpField) < 0)
        SummitAbortBootstrapField(lpField);
    return NULL;
}

smSTAGE3D *LoadStageFromField2(sFIELD *lpField, sFIELD *lpSecondField)
{
    return LoadStageFromField(lpField, lpSecondField);
}

smSTAGE3D *LoadStage(char *szStageFile)
{
    if (!szStageFile || !szStageFile[0])
        return NULL;
    if (lstrcmpi(szGameStageName[0], szStageFile) == 0) return smGameStage[0];
    if (lstrcmpi(szGameStageName[1], szStageFile) == 0) return smGameStage[1];

    smStageCount = (smStageCount + 1) & 1;
    SummitQueueStageRequest(SummitMakeRawStageRequest(szStageFile, smStageCount));
    return NULL;
}

smSTAGE3D *LoadStage2(char *szStageFile)
{
    return LoadStage(szStageFile);
}


int PlayCounter = 0;

char PlayerName[64] = { 0,0,0,0 };


void InitStage()
{

	InitStageLoader();










	InitFieldMap();


	if(!InitField())
	{
		fprintf(stderr, "[CONTENT][MAP] FATAL: client field catalog was not initialized; aborting world bootstrap\n");
		fflush(stderr);
		return;
	}

	if(GameStartField < 0)
	{

		StartField();
	}
	else
	{







		if(CharacterPosX == PrisonX && CharacterPosZ == PrisonZ)
		{

			WarpPrisonField((int *)&smConfig.Posi_Player.x, (int *)&smConfig.Posi_Player.y);
		}
		else
		{
			WarpFieldNearPos(GameStartField, CharacterPosX, CharacterPosZ, (int *)&smConfig.Posi_Player.x, (int *)&smConfig.Posi_Player.y);
		}
	}

	// WarpField/WarpFieldNearPos has now selected the definitive bootstrap X/Z.
	// Capture it independently of lpCurPlayer: stage workers may complete before
	// or after InitPat3D constructs/positions the character, but the requested
	// spawn coordinates themselves are stable from this point onward.
	if (gStageBootstrapPending)
	{
		gStageBootstrapSpawnX = smConfig.Posi_Player.x;
		gStageBootstrapSpawnZ = smConfig.Posi_Player.y;
		gStageBootstrapSpawnCoordinatesReady = true;

		char detail[128] = { 0 };
		sprintf_s(detail, sizeof(detail), "x=%d z=%d",
			gStageBootstrapSpawnX, gStageBootstrapSpawnZ);
		SummitBootstrapDiagnosticEvent("bootstrap-coordinates-captured", detail);
	}



	smRender.Color_R = 0;
	smRender.Color_G = 0;
	smRender.Color_B = 0;
	smRender.Color_A = 0;



	// Stage publication calls LoadFieldMap once the worker has completed.
}

void CloseStage()
{
	InitStageLoader();
}


struct _DRAW_LIGHTLIST
{
	smLIGHT3D	*smLight;
	int			x, y, z;
};


_DRAW_LIGHTLIST	DrawLight[64];
_DRAW_LIGHTLIST	DrawLightSmall[64];
int DrawLightCnt;
int DrawLightSmallCnt;

int DrawLightImage(smSTAGE3D *lpStage, int x, int y, int z)
{




	POINT	Pos;
	RECT	Rect;

	int cnt, cnt2;
	int dx, dy, dz, dist;
	int tz;




	int x1, y1, z1, x2, y2, z2, x3, y3, z3;
	smRENDFACE *rendface;
	smRENDVERTEX *rv;
	int flg;






	int type;


	for(cnt = 0; cnt < lpStage->nLight; cnt++)
	{

		type = lpStage->smLight[cnt].type;

		if((type&smLIGHT_TYPE_LENS) &&
			(!(lpStage->smLight[cnt].type&smLIGHT_TYPE_NIGHT) || NightDayTime))
		{

			dx = (x - lpStage->smLight[cnt].x) >> FLOATNS;
			dy = (y - lpStage->smLight[cnt].y) >> FLOATNS;
			dz = (z - lpStage->smLight[cnt].z) >> FLOATNS;
			dist = dx*dx + dy*dy + dz*dz;

			if(dist < DIST_TRANSLEVEL_LOW && abs(dx) < 64 * 38 && abs(dz) < 64 * 38)
			{
				if(tz = smRender.GetRect2D(lpStage->smLight[cnt].x, lpStage->smLight[cnt].y, lpStage->smLight[cnt].z,
					0, 0, &Rect, &Pos))
				{

					if(Pos.x > 0 && Pos.x < WinSizeX && Pos.y>0 && Pos.y < WinSizeY)
					{
						tz -= 64 * fONE;

						if(tz > (64 * 22 * fONE))
						{
							DrawLightSmall[DrawLightSmallCnt].smLight = &lpStage->smLight[cnt];
							DrawLightSmall[DrawLightSmallCnt].x = Pos.x;
							DrawLightSmall[DrawLightSmallCnt].y = Pos.y;
							DrawLightSmall[DrawLightSmallCnt].z = tz;
							DrawLightSmallCnt++;
						}
						else
						{

							for(cnt2 = 0; cnt2 < smRender.nRendFace; cnt2++)
							{
								if(!(smRender.RendFace[cnt2].ClipStatus&SMCLIP_DISPLAYOUT))
								{
									rendface = &smRender.RendFace[cnt2];

									rv = rendface->lpRendVertex[0];
									x1 = (int)rv->xy[0];
									y1 = (int)rv->xy[1];
									z1 = rv->tz;
									rv = rendface->lpRendVertex[1];
									x2 = (int)rv->xy[0];
									y2 = (int)rv->xy[1];
									z2 = rv->tz;
									rv = rendface->lpRendVertex[2];
									x3 = (int)rv->xy[0];
									y3 = (int)rv->xy[1];
									z3 = rv->tz;

									flg = 0;
									if(tz > z1 && tz > z2 && tz > z3)
									{

										if(Pos.x < x1 && Pos.x < x2 && Pos.x < x3) flg++;
										if(Pos.x > x1 && Pos.x > x2 && Pos.x > x3) flg++;
										if(Pos.y < y1 && Pos.y < y2 && Pos.y < y3) flg++;
										if(Pos.y > y1 && Pos.y > y2 && Pos.y > y3) flg++;
										if(flg == 0) break;



									}
								}

							}

							if(cnt2 >= smRender.nRendFace)
							{
								DrawLight[DrawLightCnt].smLight = &lpStage->smLight[cnt];
								DrawLight[DrawLightCnt].x = Pos.x;
								DrawLight[DrawLightCnt].y = Pos.y;
								DrawLight[DrawLightCnt].z = tz;
								DrawLightCnt++;
							}
							else
							{
								DrawLightSmall[DrawLightSmallCnt].smLight = &lpStage->smLight[cnt];
								DrawLightSmall[DrawLightSmallCnt].x = Pos.x;
								DrawLightSmall[DrawLightSmallCnt].y = Pos.y;
								DrawLightSmall[DrawLightSmallCnt].z = tz;
								DrawLightSmallCnt++;
							}

						}

					}
				}
			}
		}
	}

	return TRUE;
}

int DrawLightImage()
{
	smFACE2D face2d;
	int size;
	smRENDFACE *RendFace;
	int cnt;

	smRender.SetMaterialGroup(smMaterialGroup);

	smRender.Init();

	for(cnt = 0; cnt < DrawLightSmallCnt; cnt++)
	{

		size = 40 * fONE;


		if(size > 0)
		{

			face2d.TexRect.left = 0;
			face2d.TexRect.right = 1;
			face2d.TexRect.top = 0;
			face2d.TexRect.bottom = 1;
			face2d.width = size;
			face2d.height = size;
			face2d.Transparency = 255;
			face2d.r = 255; face2d.g = 255; face2d.b = 255;
			face2d.x = DrawLightSmall[cnt].smLight->x;
			face2d.y = DrawLightSmall[cnt].smLight->y;
			face2d.z = DrawLightSmall[cnt].smLight->z;
			face2d.MatNum = MatLight[1];

			RendFace = smRender.AddFace2D(&face2d);
		}
	}

	if(smRender.nRendFace > 0)
	{
		smRender.SetClipStates(SMCLIP_LEFT | SMCLIP_RIGHT | SMCLIP_TOP | SMCLIP_BOTTOM);
		smRender.ClipRendFace();
		smRender.GeomVertex2D(FALSE);
		smRender.RenderD3D();
	}


	smRender.Init();

	for(cnt = 0; cnt < DrawLightCnt; cnt++)
	{
		size = 160 * fONE;



		if(size > 0)
		{

			face2d.TexRect.left = 0;
			face2d.TexRect.right = 1;
			face2d.TexRect.top = 0;
			face2d.TexRect.bottom = 1;
			face2d.width = size;
			face2d.height = size;
			face2d.Transparency = 255;
			face2d.r = 255; face2d.g = 255; face2d.b = 255;
			face2d.x = DrawLight[cnt].smLight->x;
			face2d.y = DrawLight[cnt].smLight->y;
			face2d.z = DrawLight[cnt].smLight->z;
			face2d.MatNum = MatLight[0];

			RendFace = smRender.AddFace2D(&face2d);
		}
	}

	if(smRender.nRendFace > 0)
	{
		smRender.SetClipStates(SMCLIP_LEFT | SMCLIP_RIGHT | SMCLIP_TOP | SMCLIP_BOTTOM);
		smRender.ClipRendFace();
		smRender.GeomVertex2D(FALSE);
		for(cnt = 0; cnt < smRender.nRendVertex; cnt++)
		{
			smRender.RendVertex[cnt].zb = 0.001f;
		}

		smRender.RenderD3D();

	}


	return TRUE;

}

int SetStageDynLight(smSTAGE3D *lpStage, int x, int y, int z)
{

	int cnt;
	int dx, dy, dz, dist;
	int type;


	for(cnt = 0; cnt < lpStage->nLight; cnt++)
	{

		dx = (x - lpStage->smLight[cnt].x) >> FLOATNS;
		dy = (y - lpStage->smLight[cnt].y) >> FLOATNS;
		dz = (z - lpStage->smLight[cnt].z) >> FLOATNS;
		dist = dx*dx + dy*dy + dz*dz;




		type = lpStage->smLight[cnt].type;

		if(dist < 0x300000 && (type&smLIGHT_TYPE_DYNAMIC) && abs(dx) < 0x2000 && abs(dz) < 0x2000)
		{
			if((lpStage->smLight[cnt].type&smLIGHT_TYPE_NIGHT) && NightDayTime)
			{



				smRender.AddDynamicLight(lpStage->smLight[cnt].type,
					lpStage->smLight[cnt].x, lpStage->smLight[cnt].y, lpStage->smLight[cnt].z,
					lpStage->smLight[cnt].r, lpStage->smLight[cnt].g, lpStage->smLight[cnt].b,
					0, lpStage->smLight[cnt].Range);

			}
			else
			{
				if(DarkLevel > 0)
				{


					smRender.AddDynamicLight(lpStage->smLight[cnt].type,
						lpStage->smLight[cnt].x, lpStage->smLight[cnt].y, lpStage->smLight[cnt].z,

						(lpStage->smLight[cnt].r*DarkLevel) >> 8,
						(lpStage->smLight[cnt].g*DarkLevel) >> 8,
						(lpStage->smLight[cnt].b*DarkLevel) >> 8,
						0, lpStage->smLight[cnt].Range);

				}
			}
		}
	}

	return TRUE;

}




int doop = 0;



int DisplayStage(int x, int y, int z, int angX, int angY, int angZ)
{

	int cnt;
	int r, g, b, l;
	int wsbDrawResult[2] = { -1, -1 };



	DrawLightCnt = 0;
	DrawLightSmallCnt = 0;


	smRender.ZWriteAuto = TRUE;


	for(cnt = 0; cnt < 2; cnt++)
	{
		if(smGameStage[cnt])
		{



			if(DarkLevel > 0 && StageField[cnt]->State == FIELD_STATE_VILLAGE)
			{

				r = smRender.Color_R;
				g = smRender.Color_G;
				b = smRender.Color_B;
				l = smRender.nRendLight;

				smRender.Color_R >>= 1;
				smRender.Color_G >>= 1;
				smRender.Color_B >>= 1;

				if(smRender.nRendLight > 0 && smRender.smRendLight[0].Range == DarkLightRange)
				{
					smRender.smRendLight[0].Range = 0;
					l = 1;
				}
				else
					l = 0;

				if((wsbDrawResult[cnt] = smGameStage[cnt]->DrawStage2(x, y, z, angX, angY, angZ)) == TRUE)
				{




					DrawLightImage(smGameStage[cnt], x, y, z);
					smGameStage[cnt]->StageObject->Draw(x, y, z, angX, angY, angZ);
				}

				smRender.Color_R = r;
				smRender.Color_G = g;
				smRender.Color_B = b;

				if(l) smRender.smRendLight[0].Range = DarkLightRange;



			}
			else
			{
				if((wsbDrawResult[cnt] = smGameStage[cnt]->DrawStage2(x, y, z, angX, angY, angZ)) == TRUE)
				{



					DrawLightImage(smGameStage[cnt], x, y, z);
					smGameStage[cnt]->StageObject->Draw(x, y, z, angX, angY, angZ);
				}
			}

		}
	}



	// WSB diagnostics only. Never override movement, camera or scene culling.
	static DWORD wsbLastFieldDiagAt = 0;
	const DWORD wsbNow = GetCurrentTime();
	if (!wsbLastFieldDiagAt || wsbNow - wsbLastFieldDiagAt >= 500u)
	{
		const int wsbSlot = lpCurPlayer ? lpCurPlayer->OnStageField : -1;
		if (wsbSlot >= 0 && wsbSlot < 2 && lpCurPlayer && StageField[wsbSlot])
		{
			const int wsbMap = StageField[wsbSlot]->FieldCode;
			if (wsbMap == 53 || wsbMap == 54 || wsbMap == 58)
			{
				const smSTAGE3D *wsbStage = smGameStage[wsbSlot];
				const int wsbFloor = wsbStage && wsbStage->Head
					? smGameStage[wsbSlot]->GetHeight(lpCurPlayer->pX, lpCurPlayer->pZ)
					: CLIP_OUT;
				const RECT wsbRect = wsbStage ? wsbStage->StageMapRect : RECT{ 0, 0, 0, 0 };
				char wsbDetail[840] = { 0 };
				sprintf_s(wsbDetail, sizeof(wsbDetail),
					"map=%d slot=%d stage=%p head=%d draw0=%d draw1=%d "
					"cameraXYZ=%d,%d,%d playerXYZ=%d,%d,%d floor=%d "
					"move=%d mode=%d trace=%d mouse0=%d mouse1=%d pending=%d transition=%d "
					"rect=%ld,%ld,%ld,%ld",
					wsbMap, wsbSlot, static_cast<const void *>(wsbStage),
					wsbStage && wsbStage->Head ? 1 : 0,
					wsbDrawResult[0], wsbDrawResult[1],
					x, y, z, lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ,
					wsbFloor, lpCurPlayer->MoveFlag, lpCurPlayer->MoveMode,
					MsTraceMode, MouseButton[0], MouseButton[1],
					gStageBootstrapPending ? 1 : 0, gFieldTransition.active ? 1 : 0,
					static_cast<long>(wsbRect.left), static_cast<long>(wsbRect.top),
					static_cast<long>(wsbRect.right), static_cast<long>(wsbRect.bottom));
				SummitBootstrapDiagnosticEvent("wsb-field-state", wsbDetail);
				wsbLastFieldDiagAt = wsbNow;
			}
		}
	}

	smRender.ZWriteAuto = FALSE;

	if((MaterialFrame & 0x3F) == 0)
	{


		CheckInvenItemCode();
	}

	return TRUE;
}



void CreatePatIceFoot(int Num, int sizeX, int sizeZ)
{
	int MatShadow = CreateTextureMaterial(g_IceFootImageFileBuff[Num], 0, 0, 0, 0, SMMAT_BLEND_ALPHA);

	smTPOINT tPoint[4];
	tPoint[0].u = 0;
	tPoint[0].v = 0;
	tPoint[1].u = 256;
	tPoint[1].v = 0;
	tPoint[2].u = 0;
	tPoint[2].v = 256;
	tPoint[3].u = 256;
	tPoint[3].v = 256;

	smOBJ3D *objIceFoot = new smOBJ3D(4, 2);
	objIceFoot->AddVertex(-sizeX, -sizeZ, 0);
	objIceFoot->AddVertex(sizeX, -sizeZ, 0);
	objIceFoot->AddVertex(-sizeX, sizeZ, 0);
	objIceFoot->AddVertex(sizeX, sizeZ, 0);

	objIceFoot->AddFace(0, 1, 2, &tPoint[2], &tPoint[3], &tPoint[0]);
	objIceFoot->AddFace(2, 1, 3, &tPoint[0], &tPoint[3], &tPoint[1]);
	objIceFoot->ZeroNormals();

	objIceFoot->SetFaceMaterial(0, MatShadow);
	objIceFoot->SetFaceMaterial(1, MatShadow);

	PatIceFoot[Num] = new smPAT3D;
	PatIceFoot[Num]->AddObject(objIceFoot);
}


void CreatePatShadow(int Num, int sizeX, int sizeZ)
{
	smTPOINT tPoint[4];
	smOBJ3D *objShadow;

	int MatShadow;

	MatShadow = CreateTextureMaterial(szShadowImageFile, 0, 0, 0, 0, SMMAT_BLEND_SHADOW);
	smMaterial[MatShadow].SelfIllum = 1;

	tPoint[0].u = 0;
	tPoint[0].v = 0;
	tPoint[1].u = 256;
	tPoint[1].v = 0;
	tPoint[2].u = 0;
	tPoint[2].v = 256;
	tPoint[3].u = 256;
	tPoint[3].v = 256;


	objShadow = new smOBJ3D(4, 2);
	objShadow->AddVertex(-sizeX, -sizeZ, 0);
	objShadow->AddVertex(sizeX, -sizeZ, 0);
	objShadow->AddVertex(-sizeX, sizeZ, 0);
	objShadow->AddVertex(sizeX, sizeZ, 0);

	objShadow->AddFace(0, 1, 2, &tPoint[0], &tPoint[1], &tPoint[2]);
	objShadow->AddFace(2, 1, 3, &tPoint[2], &tPoint[1], &tPoint[3]);
	objShadow->ZeroNormals();

	objShadow->SetFaceMaterial(0, MatShadow);
	objShadow->SetFaceMaterial(1, MatShadow);

	PatShadow[Num] = new smPAT3D;
	PatShadow[Num]->AddObject(objShadow);
}

void CreatePatShadow2(int Num, int sizeX, int sizeZ)
{
	smTPOINT tPoint[9];
	smOBJ3D *objShadow;

	int MatShadow;

	MatShadow = CreateTextureMaterial(szShadowImageFile, 0, 0, 0, 0, SMMAT_BLEND_SHADOW);
	smMaterial[MatShadow].SelfIllum = 1;

	tPoint[0].u = 0;
	tPoint[0].v = 0;
	tPoint[1].u = 128;
	tPoint[1].v = 0;
	tPoint[2].u = 256;
	tPoint[2].v = 0;

	tPoint[3].u = 0;
	tPoint[3].v = 128;
	tPoint[4].u = 128;
	tPoint[4].v = 128;
	tPoint[5].u = 256;
	tPoint[5].v = 128;

	tPoint[6].u = 0;
	tPoint[6].v = 256;
	tPoint[7].u = 128;
	tPoint[7].v = 256;
	tPoint[8].u = 256;
	tPoint[8].v = 256;


	objShadow = new smOBJ3D(9, 8);
	objShadow->AddVertex(-sizeX, -sizeZ, 0);
	objShadow->AddVertex(0, -sizeZ, 0);
	objShadow->AddVertex(sizeX, -sizeZ, 0);

	objShadow->AddVertex(-sizeX, 0, 0);
	objShadow->AddVertex(0, 0, 0);
	objShadow->AddVertex(sizeX, 0, 0);

	objShadow->AddVertex(-sizeX, sizeZ, 0);
	objShadow->AddVertex(0, sizeZ, 0);
	objShadow->AddVertex(sizeX, sizeZ, 0);

	objShadow->AddFace(0, 1, 3, &tPoint[0], &tPoint[1], &tPoint[3]);
	objShadow->AddFace(3, 1, 4, &tPoint[3], &tPoint[1], &tPoint[4]);

	objShadow->AddFace(1, 2, 4, &tPoint[0], &tPoint[2], &tPoint[4]);
	objShadow->AddFace(4, 2, 5, &tPoint[4], &tPoint[2], &tPoint[5]);

	objShadow->AddFace(3, 4, 6, &tPoint[3], &tPoint[4], &tPoint[6]);
	objShadow->AddFace(6, 4, 7, &tPoint[6], &tPoint[4], &tPoint[7]);

	objShadow->AddFace(4, 5, 7, &tPoint[4], &tPoint[5], &tPoint[7]);
	objShadow->AddFace(7, 5, 8, &tPoint[7], &tPoint[5], &tPoint[8]);

	objShadow->SetFaceMaterial(0, MatShadow);
	objShadow->SetFaceMaterial(1, MatShadow);
	objShadow->SetFaceMaterial(2, MatShadow);
	objShadow->SetFaceMaterial(3, MatShadow);
	objShadow->SetFaceMaterial(4, MatShadow);
	objShadow->SetFaceMaterial(5, MatShadow);
	objShadow->SetFaceMaterial(6, MatShadow);
	objShadow->SetFaceMaterial(7, MatShadow);

	PatShadow[Num] = new smPAT3D;
	PatShadow[Num]->AddObject(objShadow);
}



int AdminCharMode = 0;

int AdminCharDisable = 0;


static char szDataDirectory[256];
char szCharFileList[256][256];
int CharFileCnt;
int CharPlayCnt = 0;


static char *playmain_SetDirectoryFromFile(char *filename)
{
	int len;
	int cnt;

	strcpy_s(szDataDirectory, filename);

	len = lstrlen(szDataDirectory);

	for(cnt = len; cnt >= 0; cnt--)
		if(szDataDirectory[cnt] == '/' || szDataDirectory[cnt] == '\\') break;

	szDataDirectory[cnt + 1] = NULL;

	return szDataDirectory;
}


int SetCharFileList(char *szCharFile)
{
	CharFileCnt = 0;
	char szFindPath[256];

	playmain_SetDirectoryFromFile(szCharFile);

	HANDLE hFindHandle;
	WIN32_FIND_DATA	fd;

	strcpy_s(szFindPath, szDataDirectory);
	lstrcat(szFindPath, "//*.ini");


	hFindHandle = FindFirstFile(szFindPath, &fd);
	if(hFindHandle == INVALID_HANDLE_VALUE)
	{
		return 0;
	}
	while(1)
	{
		strcpy_s(szCharFileList[CharFileCnt], szDataDirectory);
		lstrcat(szCharFileList[CharFileCnt], fd.cFileName);
		CharFileCnt++;

		if(FindNextFile(hFindHandle, &fd) == FALSE)
			break;
		if(CharFileCnt > 255)
			break;
	}
	FindClose(hFindHandle);

	return TRUE;
}

void SetNextPlayerPattern()
{
	if(lpCurPlayer->PatLoading)
		return;


	AdminCharDisable = TRUE;

	if(AdminCharMode)
	{
		memcpy(&lpCurPlayer->smCharInfo, &smCharInfo_Back, sizeof(smCHAR_INFO));
		ReformCharForm();
		AdminCharMode = 0;
		lpCurPlayer->PatLoading = TRUE;
		AddLoaderPattern(lpCurPlayer, lpCurPlayer->smCharInfo.szModelName, lpCurPlayer->smCharInfo.szModelName2);
		lpCurPlayer->DisplayTools = TRUE;
		return;
	}

	AdminCharMode = 0;
	lpCurPlayer->PatLoading = TRUE;
	AddLoaderPattern(lpCurPlayer, szCharFileList[CharPlayCnt], lpCurPlayer->smCharInfo.szModelName2);
	lpCurPlayer->DisplayTools = TRUE;

	CharPlayCnt++;
	if(CharPlayCnt >= CharFileCnt) CharPlayCnt = 0;
}

void SetPlayerBodyPattern(char *szBodyFile)
{
	char szBuff[256];

	if(lpCurPlayer->PatLoading)
		return;

	wsprintf(szBuff, "char//tmABCD//%s.ini", szBodyFile);
	FILE *fp = fopen(szBuff, "rb");
	if(!fp) return;
	fclose(fp);

	lpCurPlayer->PatLoading = TRUE;
	AddLoaderPattern(lpCurPlayer, szBuff, lpCurPlayer->smCharInfo.szModelName2);
	lpCurPlayer->DisplayTools = TRUE;
}

void SetPlayerPattern(char *szFile)
{
	if(lpCurPlayer->PatLoading)
		return;

	FILE *fp = fopen(szFile, "rb");
	if(!fp) return;
	fclose(fp);

	lpCurPlayer->PatLoading = TRUE;
	AddLoaderPattern(lpCurPlayer, szFile, 0);
	lpCurPlayer->DisplayTools = TRUE;

	ReformCharForm();

	lpCurPlayer->smCharInfo.bUpdateInfo[0]++;
	SendSetObjectSerial(lpCurPlayer->dwObjectSerial);

}

char szNpcFileList[64][64];
int NpcFileCnt;
int NpcPlayCnt = 0;

char *NpcSelectedName = 0;


int SetNpcFileList(char *szNpcDirectory)
{
	NpcFileCnt = 0;
	if(summit::content::definition::CompiledDefinitionsEnabled())
	{
		std::vector<summit::content::definition::DefinitionEntry> entries;
		if(summit::content::definition::ListCompiledDefinitions(szNpcDirectory, ".npc", true, entries, summit::content::definition::DefinitionKindNpc))
		{
			for(std::size_t index=0; index<entries.size() && NpcFileCnt<64; ++index)
			{
				strcpy_s(szNpcFileList[NpcFileCnt], entries[index].logicalPath.c_str());
				NpcFileCnt++;
			}
			return NpcFileCnt ? TRUE : 0;
		}
		if(summit::content::definition::DefinitionStrictMode()) return 0;
	}

	char szFindPath[256];
	HANDLE hFindHandle;
	WIN32_FIND_DATA fd;
	strcpy_s(szFindPath, szNpcDirectory);
	lstrcat(szFindPath, "*.npc");
	hFindHandle = FindFirstFile(szFindPath, &fd);
	if(hFindHandle == INVALID_HANDLE_VALUE) return 0;
	while(1)
	{
		strcpy_s(szNpcFileList[NpcFileCnt], szNpcDirectory);
		lstrcat(szNpcFileList[NpcFileCnt], fd.cFileName);
		NpcFileCnt++;
		if(FindNextFile(hFindHandle, &fd) == FALSE || NpcFileCnt >= 64) break;
	}
	FindClose(hFindHandle);
	return TRUE;
}

void SetNextNpcPattern()
{
	if(lpCurPlayer->PatLoading)
		return;

	if(AdminCharMode == 0)
		memcpy(&smCharInfo_Back, &lpCurPlayer->smCharInfo, sizeof(smCHAR_INFO));


	AdminCharDisable = TRUE;

	AdminCharMode = 1;


	lpCurPlayer->smCharInfo.dwCharSoundCode = 0;


	if(smCharDecode(szNpcFileList[NpcPlayCnt], &lpCurPlayer->smCharInfo, 0))
	{
		lpCurPlayer->PatLoading = TRUE;
		AddLoaderPattern(lpCurPlayer, lpCurPlayer->smCharInfo.szModelName);

		lpCurPlayer->DisplayTools = FALSE;
		lpCurPlayer->smCharInfo.SizeLevel = 1;

		NpcSelectedName = szNpcFileList[NpcPlayCnt];

		NpcPlayCnt++;
		if(NpcPlayCnt >= NpcFileCnt) NpcPlayCnt = 0;
	}
	ReformCharForm();
}


int LaunchPartyPostion()
{
	int x, y, z, dist, ddist;
	int cnt;
	smCHAR *lpChar;

	x = lpCurPlayer->pX;
	y = lpCurPlayer->pY + 48 * fONE;
	z = lpCurPlayer->pZ;

	if(sPartyPosion.code == OpCode::OPCODE_PARTY_POTION)
	{

		if(lpCurPlayer->MotionInfo->State != CHRMOTION_STATE_DEAD)
		{
			CheckCharForm();
			if(sPartyPosion.WParam)
			{
				sinSetLife(sinGetLife() + sPartyPosion.WParam);
				StartEffect(x, y, z, EFFECT_POTION1);
			}
			if(sPartyPosion.LParam)
			{
				sinSetMana(sinGetMana() + sPartyPosion.LParam);
				StartEffect(x, y, z, EFFECT_POTION2);
			}
			if(sPartyPosion.SParam)
			{
				sinSetStamina(sinGetStamina() + sPartyPosion.SParam);
				StartEffect(x, y, z, EFFECT_POTION3);
			}
			ResetEnergyGraph(3);
			ReformCharForm();
			sinPlaySound(SIN_SOUND_EAT_POTION, GetDistVolume(x, y, z));

		}



		for(cnt = 0; cnt < OTHER_PLAYER_MAX; cnt++)
		{
			if(chrOtherPlayer[cnt].Flag && chrOtherPlayer[cnt].dwObjectSerial && chrOtherPlayer[cnt].PartyFlag &&
				chrOtherPlayer[cnt].dwObjectSerial != sPartyPosion.EParam &&
				chrOtherPlayer[cnt].MotionInfo->State != CHRMOTION_STATE_DEAD &&
				chrOtherPlayer[cnt].RendSucess)
			{

				x = chrOtherPlayer[cnt].pX;
				y = chrOtherPlayer[cnt].pY + 48 * fONE;
				z = chrOtherPlayer[cnt].pZ;

				if(sPartyPosion.WParam) 	StartEffect(x, y, z, EFFECT_POTION1);
				if(sPartyPosion.LParam) 	StartEffect(x, y, z, EFFECT_POTION2);
				if(sPartyPosion.SParam) 	StartEffect(x, y, z, EFFECT_POTION3);
			}
		}


	}

	switch(sPartyPosion.code)
	{
	case OpCode::OPCODE_HEALING:

		if(lpCurPlayer->MotionInfo->State != CHRMOTION_STATE_DEAD)
		{
			CheckCharForm();
			if(sPartyPosion.WParam)
			{
				sinSetLife(sinGetLife() + sPartyPosion.WParam);
			}
			ResetEnergyGraph(0);
			ReformCharForm();
		}
		break;

	case OpCode::OPCODE_GRAND_HEALING:

		if(sPartyPosion.EParam == lpCurPlayer->dwObjectSerial)
		{
			lpChar = lpCurPlayer;
		}
		else
			lpChar = FindAutoPlayer(sPartyPosion.EParam);

		if(!lpChar) break;

		ddist = sPartyPosion.SParam*sPartyPosion.SParam;

		x = (lpCurPlayer->pX - lpChar->pX) >> FLOATNS;
		y = (lpCurPlayer->pY - lpChar->pY) >> FLOATNS;
		z = (lpCurPlayer->pZ - lpChar->pZ) >> FLOATNS;
		dist = x*x + y*y + z*z;
		if(dist < ddist && lpCurPlayer->MotionInfo->State != CHRMOTION_STATE_DEAD)
		{

			if(sPartyPosion.LParam)
			{
				CheckCharForm();
				sinSetLife(sinGetLife() + sPartyPosion.LParam);
				ResetEnergyGraph(0);
				ReformCharForm();
			}

			if(lpChar != lpCurPlayer)
				sinSkillEffect_Great_Healing_Party(lpCurPlayer);
		}



		for(cnt = 0; cnt < OTHER_PLAYER_MAX; cnt++)
		{
			if(chrOtherPlayer[cnt].Flag && chrOtherPlayer[cnt].dwObjectSerial && lpChar != &chrOtherPlayer[cnt] &&
				chrOtherPlayer[cnt].PartyFlag && chrOtherPlayer[cnt].MotionInfo->State != CHRMOTION_STATE_DEAD)
			{

				x = (chrOtherPlayer[cnt].pX - lpChar->pX) >> FLOATNS;
				y = (chrOtherPlayer[cnt].pY - lpChar->pY) >> FLOATNS;
				z = (chrOtherPlayer[cnt].pZ - lpChar->pZ) >> FLOATNS;
				dist = x*x + y*y + z*z;

				if(dist < ddist)
				{
					sinSkillEffect_Great_Healing_Party(&chrOtherPlayer[cnt]);
				}
			}
		}
		break;

	case OpCode::OPCODE_VAMPRIC_CUSPID:
		if(lpCurPlayer->MotionInfo->State != CHRMOTION_STATE_DEAD)
		{
			CheckCharForm();
			if(sPartyPosion.WParam)
			{
				sinSetLife(sinGetLife() + sPartyPosion.WParam);
			}
			ResetEnergyGraph(0);
			ReformCharForm();
		}
		break;


	case OpCode::OPCODE_VAMPRIC_CUSPID_EX:
		if(lpCurPlayer->MotionInfo->State != CHRMOTION_STATE_DEAD)
		{
			CheckCharForm();
			if(sPartyPosion.WParam)
			{
				sinSetLife(sinGetLife() + sPartyPosion.WParam);
			}
			ResetEnergyGraph(0);
			ReformCharForm();
		}
		break;

	case OpCode::OPCODE_MANA_RECHARGING:
		if(lpCurPlayer->MotionInfo->State != CHRMOTION_STATE_DEAD)
		{
			CheckCharForm();
			if(sPartyPosion.WParam)
			{
				sinSetMana(sinGetMana() + sPartyPosion.WParam);
			}
			ResetEnergyGraph(1);
			ReformCharForm();
		}
		break;

	case OpCode::OPCODE_STAMINA_RECHARGING:
		if (lpCurPlayer->MotionInfo->State != CHRMOTION_STATE_DEAD)
		{
			CheckCharForm();
			if (sPartyPosion.WParam)
			{
				sinSetStamina(sinGetStamina() + sPartyPosion.WParam);
			}
			ResetEnergyGraph(2);
			ReformCharForm();
		}
		break;

	}




	sPartyPosion.code = 0;
	dwPartyPosionTime = 0;


	return TRUE;
}


int SetPartyPosion(smTRANS_COMMAND *lpTransCommand)
{
	if(dwPartyPosionTime)
	{

		LaunchPartyPostion();
	}

	dwPartyPosionTime = dwPlayTime + 1000;
	memcpy(&sPartyPosion, lpTransCommand, sizeof(smTRANS_COMMAND));

	return TRUE;
}



char szPlayerCode[32];
int CharacterJob = 0;

char *szAnimShieldEffectFile[] = {
	"Weapons//test1.tga",
	"Weapons//test2.tga",
	"Weapons//test3.tga",
	"Weapons//test2.tga"
};

char *szBangImage = "weapons//bang2.bmp";
char *szClowImage = "weapons//crown.tga";

void ClanCustom::RecvCrownToServer(CROWN* Shop)
{
	//LoadOftenItem();
	PQPCROWN = Shop->idnumber;
	smMATERIAL* mat;
	smASE_SetPhysique(0);
	//CopyMemory(&Tidnumber, idnumber , sizeof(CROWN));

	if (PQPCROWN == 1)
	{
		char* szClowImage = "weapons//crown.tga";
		PatCrown = smASE_Read("weapons//Crown.ASE");
		PatCrown->Frame = 0;

		mat = PatCrown->smMaterialGroup->FindMaterialFromTextureName(szClowImage);

		if (mat)
		{
			mat->BlendType = SMMAT_BLEND_ADDCOLOR;
			mat->SelfIllum = 1;
		}
		smASE_SetPhysique(0);
		//LoadOftenItem();


	}
	else if (PQPCROWN == 2)
	{
		char* szClowImage = "weapons//crow2.tga";
		PatCrown = smASE_Read("weapons//Crownasa.ASE");

		PatCrown->Frame = 0;

		mat = PatCrown->smMaterialGroup->FindMaterialFromTextureName(szClowImage);

		if (mat)
		{
			mat->BlendType = SMMAT_BLEND_ADDCOLOR;
			mat->SelfIllum = 1;
		}
		smASE_SetPhysique(0);
		//LoadOftenItem();

	}
	else if (PQPCROWN == 3)
	{
		char* szClowImage = "weapons//crow3.tga";
		PatCrown = smASE_Read("weapons//crownasademo.ASE");

		PatCrown->Frame = 0;

		mat = PatCrown->smMaterialGroup->FindMaterialFromTextureName(szClowImage);

		if (mat)
		{
			mat->BlendType = SMMAT_BLEND_ADDCOLOR;
			mat->SelfIllum = 1;
		}
		smASE_SetPhysique(0);
		//LoadOftenItem();

	}
	else
	{
		char* szClowImage = "weapons//crow4.tga";
		PatCrown = smASE_Read("weapons//crownaice.ASE");

		PatCrown->Frame = 0;

		mat = PatCrown->smMaterialGroup->FindMaterialFromTextureName(szClowImage);

		if (mat)
		{
			mat->BlendType = SMMAT_BLEND_ADDCOLOR;
			mat->SelfIllum = 1;
		}
		smASE_SetPhysique(0);
		//LoadOftenItem();
	}

}

extern	char *szNpcInfoPath;

DWORD	dwSoD_ResultTime = 0;

sBILLING_INFO	*sBiInfo = 0;

int InitPat3D()
{
	int cnt, cnt2, size;
	POINT3D Posi, Angle;


	DWORD nsize = 62;

	if(!sBiInfo)
	{
		sBiInfo = new sBILLING_INFO;
		ZeroMemory(sBiInfo, sizeof(sBILLING_INFO));
	}

	if(PlayerName[0] == 0)
		GetComputerName(PlayerName, &nsize);

	InterfaceParty.chatbuild_INIfiles();


	rsRecorder.dwRecordTime = 0;
	rsRecorder.dwNextRecordTime = GetCurrentTime() + 10 * 60 * 1000;



#ifdef	_LANGUAGE_THAI		
	cInterFace.sInterFlags.CameraAutoFlag = 0;
#else	
	cInterFace.sInterFlags.CameraAutoFlag = 2;
#endif	

	cInterFace.sInterFlags.MapOnFlag = TRUE;
	cInterFace.sInterFlags.RunFlag = TRUE;

	if(((GetCurrentTime() >> 4) & 1) == 0) strcpy_s(szPlayerCode, "p");
	else strcpy_s(szPlayerCode, "f");



	CreatePatIceFoot(0, 6 * fONE, 6 * fONE);
	CreatePatIceFoot(1, 6 * fONE, 6 * fONE);



	CreatePatShadow(0, 12 * fONE, 10 * fONE);
	CreatePatShadow(1, 17 * fONE, 14 * fONE);
	CreatePatShadow2(2, 26 * fONE, 20 * fONE);
	CreatePatShadow2(3, 38 * fONE, 30 * fONE);




	Posi.x = smConfig.Posi_Player.x;
	Posi.z = smConfig.Posi_Player.y;
	Posi.y = 0;
	Angle.x = 0;
	Angle.y = 0;
	Angle.z = 0;


	invPlayerArmor = 0;



	SetCharFileList("char//tmABCD//a001.ini");



#ifdef _W_SERVER

	SetNpcFileList(szNpcInfoPath);
#endif
	//Loading
	//LoadOftenArmor();
	//LoadOftenFace();
	
	LoadOftenItem();
	
	//LoadOftenNpc();
	//LoadOftenModels();


	sinChar = &lpCurPlayer->smCharInfo;

	CheckCharForm();


	if(CharacterName1[0])
	{
		gameplay::classes::CharacterAttributes canonicalAttributes;
		if(gameplay::classes::DefaultClassService().TryBaseAttributes(NewCharInfo.JobCode, canonicalAttributes))
		{
			// NewCharInfo is a legacy transport object. Class V2 is the source of truth
			// for creation stats, so packet/UI values cannot create an invalid build.
			lpCurPlayer->smCharInfo.JOB_CODE = NewCharInfo.JobCode;
			lpCurPlayer->smCharInfo.Dexterity = canonicalAttributes.dexterity;
			lpCurPlayer->smCharInfo.Health = canonicalAttributes.health;
			lpCurPlayer->smCharInfo.Spirit = canonicalAttributes.spirit;
			lpCurPlayer->smCharInfo.Strength = canonicalAttributes.strength;
			lpCurPlayer->smCharInfo.Talent = canonicalAttributes.talent;
		}
		else
		{
			// Unknown or disabled classes fail closed at the legacy boundary.
			lpCurPlayer->smCharInfo.JOB_CODE = 0;
			lpCurPlayer->smCharInfo.Dexterity = 1;
			lpCurPlayer->smCharInfo.Health = 8;
			lpCurPlayer->smCharInfo.Spirit = 1;
			lpCurPlayer->smCharInfo.Strength = 8;
			lpCurPlayer->smCharInfo.Talent = 1;
			lpCurPlayer->smCharInfo.StatePoint = 0;
		}
	}


	if(CharacterName1[0])
	{
		SetLoadPattern(lpCurPlayer, CharacterName1, CharacterName2, &Posi, &Angle);

		cnt2 = 0;
		for(cnt = 0; cnt < (int)lpCurPlayer->smMotionInfo->MotionCount; cnt++)
		{
			if(lpCurPlayer->smMotionInfo->MotionInfo[cnt].State == CHRMOTION_STATE_DEAD)
			{

				size = lpCurPlayer->smMotionInfo->MotionInfo[cnt].EndFrame - lpCurPlayer->smMotionInfo->MotionInfo[cnt].StartFrame;
				if(size > 8) cnt2++;
			}

			if(lpCurPlayer->smMotionInfo->MotionInfo[cnt].State == CHRMOTION_STATE_ATTACK)
			{
				size = lpCurPlayer->smMotionInfo->MotionInfo[cnt].EndFrame - lpCurPlayer->smMotionInfo->MotionInfo[cnt].StartFrame;

				if(size < 40 && smConfig.getSecurity() < AccountTypes::SEC_MODERATOR)
				{
					/* MINGW_DISCONNECT_ORIGIN_PROBE_V1 */
					FILE *fp = fopen("LOGS\\mingw_disconnect_origin.txt", "a");
					if(fp)
					{
						fprintf(fp,
							"ORIGIN=PLAYMAIN_ATTACK size=%d job=%d char1='%s' char2='%s'\n",
							size,
							lpCurPlayer->smCharInfo.JOB_CODE,
							CharacterName1,
							CharacterName2);
						fflush(fp);
						fclose(fp);
					}

					DisconnectFlag = GetCurrentTime();
					quit = TRUE;
				}
			}
		}

		if(cnt2 == 0 && smConfig.getSecurity() < AccountTypes::SEC_MODERATOR)
		{
			FILE *fp = fopen("LOGS\\mingw_disconnect_origin.txt", "a");
			if(fp)
			{
				fprintf(fp,
					"ORIGIN=PLAYMAIN_NO_DEAD_MOTION cnt2=%d job=%d char1='%s' char2='%s'\n",
					cnt2,
					lpCurPlayer->smCharInfo.JOB_CODE,
					CharacterName1,
					CharacterName2);
				fflush(fp);
				fclose(fp);
			}

			DisconnectFlag = GetCurrentTime();
			quit = TRUE;
		}
	}
	else
		SetLoadPattern(lpCurPlayer, smConfig.szFile_Player, 0, &Posi, &Angle);

	//Level Inicial
	lpCurPlayer->smCharInfo.Level = 1;

	lpCurPlayer->smCharInfo.Attack_Damage[0] = 5;
	lpCurPlayer->smCharInfo.Attack_Damage[1] = 10;

	strcpy_s(lpCurPlayer->smCharInfo.szName, PlayerName);
	lpCurPlayer->smCharInfo.State = smCHAR_STATE_USER;
	lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_RESTART);

	lpCurPlayer->smCharInfo.SizeLevel = 1;
	lpCurPlayer->PatSizeLevel = 1;

	lpCurPlayer->smCharInfo.Attack_Rating = 100;
	lpCurPlayer->smCharInfo.Next_Exp = 1000;

	lpCurPlayer->smMonsterInfo.dwAutoCharCode = 0;

	lpCurPlayer->smCharInfo.Life[0] = 1000;
	lpCurPlayer->smCharInfo.Life[1] = 1000;

	lpCurPlayer->MoveMode = TRUE;
	lpCurPlayer->MoveCnt = 0;

	PkMode = 0;

	ReformCharForm();



	// The legacy synchronous loader guaranteed a valid logical field by this
	// point. With async bootstrap the metadata can be ready before collision.
	// Keep OnStageField bound to that metadata until geometry is published.
	cnt = CLIP_OUT;
	lpCurPlayer->OnStageField = -1;
	if(smGameStage[0])
	{
		cnt = smGameStage[0]->GetFloorHeight(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, lpCurPlayer->Pattern->SizeHeight);
		lpCurPlayer->OnStageField = 0;
	}
	else if(StageField[0])
	{
		lpCurPlayer->OnStageField = 0;
	}
	if(cnt == CLIP_OUT && smGameStage[1])
	{
		lpCurPlayer->OnStageField = 1;
	}


	for(cnt = 0; cnt < OTHER_PLAYER_MAX; cnt++)
	{
		chrOtherPlayer[cnt].Init();
	}


	smMATERIAL *mat;

	smASE_SetPhysique(0);
	PatArrow = smASE_Read("weapons//arrow.ase");
	PatEffectShield = smASE_Read("weapons//Test8.ASE");
	PatSparkShield = smASE_Read("weapons//Mc3_sparkShield.ASE");
	PatDivineShield = smASE_Read("weapons//Nt3_DivineI.ASE");
	PatGodlyShield = smASE_Read("weapons//Godly_Shield.ASE");
	PatCrown = smASE_Read("weapons//crownasa.ASE");
	PatCrown = smASE_Read("weapons//crownasademo.ASE");
	PatCrown = smASE_Read("weapons//crownaice.ASE");
	PatCrown = smASE_Read("weapons//crown.ASE");
	PatCrown2 = smASE_Read("weapons//Crown2.ASE");
	PatCrown3 = smASE_Read("weapons//Crown3.ASE");

	if(PatEffectShield)
	{
		PatEffectShield->Frame = 0;

		mat = PatEffectShield->smMaterialGroup->FindMaterialFromTextureName(szBangImage);
		if(mat)
		{
			mat->BlendType = SMMAT_BLEND_ADDCOLOR;
			mat->SelfIllum = 1;


		}
	}

	if(PatCrown)
	{
		PatCrown->Frame = 0;

		mat = PatCrown->smMaterialGroup->FindMaterialFromTextureName(szClowImage);

		if (mat)
		{
			mat->BlendType = SMMAT_BLEND_ADDCOLOR;
			mat->SelfIllum = 1;
		}
	}

	if(PatCrown2)
	{
		PatCrown2->Frame = 0;

		mat = PatCrown2->smMaterialGroup->FindMaterialFromTextureName("weapons//crown2.tga");

		if (mat)
		{
			mat->BlendType = SMMAT_BLEND_ADDCOLOR;
			mat->SelfIllum = 1;
		}
	}

	if(PatCrown3)
	{
		PatCrown3->Frame = 0;

		mat = PatCrown3->smMaterialGroup->FindMaterialFromTextureName("weapons//crown3.tga");

		if (mat)
		{
			mat->BlendType = SMMAT_BLEND_ADDCOLOR;
			mat->SelfIllum = 1;
		}
	}

	smASE_SetPhysique(0);


	MatEachBox = CreateTextureMaterial("image//cw.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);


	MatEachMenuBox = CreateTextureMaterial("image//CW-1.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);
	MatEachTradeButton[0] = CreateTextureMaterial("image//CW-trade0.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);
	MatEachTradeButton[1] = CreateTextureMaterial("image//CW-trade1.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);

#ifdef	_NEW_PARTRADE


	lpDDS_ParTradeButton[0][0] = LoadDibSurfaceOffscreen("image//party//icon-c5_.bmp");
	lpDDS_ParTradeButton[0][1] = LoadDibSurfaceOffscreen("image//party//icon-c5.bmp");

	lpDDS_ParTradeButton[1][0] = LoadDibSurfaceOffscreen("image//party//icon-c1_.bmp");
	lpDDS_ParTradeButton[1][1] = LoadDibSurfaceOffscreen("image//party//icon-c1.bmp");

	lpDDS_ParTradeButton[2][0] = LoadDibSurfaceOffscreen("image//party//icon-c2_.bmp");
	lpDDS_ParTradeButton[2][1] = LoadDibSurfaceOffscreen("image//party//icon-c2.bmp");

	lpDDS_ParTradeButton[3][0] = LoadDibSurfaceOffscreen("image//party//icon_clan_.bmp");
	lpDDS_ParTradeButton[3][1] = LoadDibSurfaceOffscreen("image//party//icon_clan.bmp");


	lpDDS_ParTradeToolTip[0] = LoadDibSurfaceOffscreen("image//party//ButtonInfo-c5.bmp");
	lpDDS_ParTradeToolTip[1] = LoadDibSurfaceOffscreen("image//party//ButtonInfo-c1.bmp");
	lpDDS_ParTradeToolTip[2] = LoadDibSurfaceOffscreen("image//party//ButtonInfo-c0.bmp");
	lpDDS_ParTradeToolTip[3] = LoadDibSurfaceOffscreen("image//party//ButtonInfo-c10.bmp");


#endif


	MatEnergyBox[0] = CreateTextureMaterial("image//Energy_Red.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);
	MatEnergyBox[1] = CreateTextureMaterial("image//Energy_Blue.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);

	MatLight[0] = CreateTextureMaterial("image//glow01.bmp", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ADDCOLOR);
	smMaterial[MatLight[0]].SelfIllum = TRUE;
	MatLight[1] = CreateTextureMaterial("image//glow02.bmp", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ADDCOLOR);
	smMaterial[MatLight[1]].SelfIllum = TRUE;

	MatMonBroodImage[0] = CreateTextureMaterial("image//M_Nm.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);
	MatMonBroodImage[1] = CreateTextureMaterial("image//M_Mt.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);
	MatMonBroodImage[2] = CreateTextureMaterial("image//M_Un.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);
	MatMonBroodImage[3] = CreateTextureMaterial("image//M_D.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);
	MatMonBroodImage[4] = CreateTextureMaterial("image//M_mc.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);


	InitWeatherSystem();


	InitParticle();

	InitDynLight();

	UseEtherCoreCode = 0;

	dwPartyPosionTime = 0;


	InitTimeEffect();

#ifdef	_SET_PROCTECT_MEM2

	cnt = Check_PlaySubReleaseFunc();
	if(cnt)
	{
		SendSetHackUser2(1120, cnt);
	}
#endif

	ResetEnergyGraph(3);
	ResetInvenItemCode();
	ResetInvenMoney();

	CheckDLL_Module();

	ZeroMemory(&rsBlessCastle, sizeof(rsBLESS_CASTLE));

	return TRUE;

}

int ClosePat3D()
{
	int cnt;

	ClosePatterns();



	if(PatArrow)
	{
		delete PatArrow;
		PatArrow = NULL;
	}

	if(PatEffectShield)
	{
		delete PatEffectShield;
		PatEffectShield = NULL;
	}




	for(cnt = 0; cnt < ICE_FOOT_PAT_MAX; cnt++)
	{
		if(PatIceFoot[cnt])
		{
			delete PatIceFoot[cnt];
			PatIceFoot[cnt] = NULL;
		}
	}



	for(cnt = SHADOW_PAT_MAX - 1; cnt >= 0; cnt--)
		if(PatShadow[cnt]) delete PatShadow[cnt];


	if(lpDDS_ParTradeButton[0][0])	lpDDS_ParTradeButton[0][0]->Release();
	if(lpDDS_ParTradeButton[0][1])	lpDDS_ParTradeButton[0][1]->Release();
	if(lpDDS_ParTradeButton[1][0])	lpDDS_ParTradeButton[1][0]->Release();
	if(lpDDS_ParTradeButton[1][1])	lpDDS_ParTradeButton[1][1]->Release();
	if(lpDDS_ParTradeButton[2][0])	lpDDS_ParTradeButton[2][0]->Release();
	if(lpDDS_ParTradeButton[2][1])	lpDDS_ParTradeButton[2][1]->Release();
	if(lpDDS_ParTradeButton[3][0])	lpDDS_ParTradeButton[3][0]->Release();
	if(lpDDS_ParTradeButton[3][1])	lpDDS_ParTradeButton[3][1]->Release();

	if(lpDDS_ParTradeToolTip[0])	lpDDS_ParTradeToolTip[0]->Release();
	if(lpDDS_ParTradeToolTip[1])	lpDDS_ParTradeToolTip[1]->Release();
	if(lpDDS_ParTradeToolTip[2])	lpDDS_ParTradeToolTip[2]->Release();
	if(lpDDS_ParTradeToolTip[3])	lpDDS_ParTradeToolTip[3]->Release();

	if(sBiInfo) delete sBiInfo;

	return TRUE;
}

int PartCnt = 0;


int		MsTraceCount = 0;
int		MsTraceMode = 0;
RECT	MsSelRect;
POINT	MsSelPos;

int		SelMouseButton = 0;

smCHAR	*lpCharSelPlayer = 0;
smCHAR	*lpCharMsTrace = 0;

scITEM	*lpSelItem = 0;
scITEM	*lpMsTraceItem = 0;

int AttFlag = 0;
int	AttCancel;
int	AttAutoCont = 0;


int TraceAttackPlay()
{
	if(lpCharSelPlayer)
	{
		MsTraceMode = TRUE;
		lpCharMsTrace = lpCharSelPlayer;
		AttFlag = TRUE;
		if(VRKeyBuff[VK_SHIFT] && lpCharSelPlayer->smCharInfo.State == smCHAR_STATE_ENEMY) AttAutoCont = TRUE;
		else AttAutoCont = FALSE;


		if(VRKeyBuff['O'] && smConfig.getSecurity() >= AccountTypes::SEC_MODERATOR)
			SkillWarriorBetserker(lpCharSelPlayer, 15);

		return TRUE;
	}

	if(lpSelItem)
	{
		MsTraceMode = TRUE;
		lpMsTraceItem = lpSelItem;
		return TRUE;
	}

	return FALSE;

}


int CancelAttack()
{
	SelMouseButton = 0;
	MsTraceMode = FALSE;
	lpCharMsTrace = 0;
	lpMsTraceItem = 0;
	AttFlag = 0;

	return TRUE;
}


int	DamageExp(smCHAR *lpChar, int AttackDamage)
{
	int AttackResult;







	if(AttackDamage && lpChar->smCharInfo.State == smCHAR_STATE_ENEMY &&
		lpChar->smCharInfo.Brood != smCHAR_MONSTER_USER && lpChar->smCharInfo.Life[0] > 0)
	{

		AttackResult = (AttackDamage << 8) / lpChar->smCharInfo.Life[1];
		if(AttackResult > 256) AttackResult = 256;
		AttackResult = (lpChar->smCharInfo.Exp * AttackResult) >> 8;
		AttackResult /= 10;
		if(AttackResult <= 0) AttackResult = 1;
		lpCurPlayer->AttackExp = AttackResult;
	}

	return TRUE;
}

#define AUTO_DEAD_COUNT		(70*60*8)
int AutoDeathCounter = 0;
int FixedWidth = 200;
DWORD	dwClassDelayTime = 0;



static void SummitEragonDiagnosticBypassTick()
{
    // Diagnostic-only bypass for the field-52 gate. It is opt-in so normal
    // gameplay/collision remains untouched. The target is the preserved
    // canonical dark_boss.ASE start point on the far side of the entrance.
    const char *enabled = std::getenv("SUMMIT_ERAGON_DIAG_BYPASS");
    if(!enabled || !*enabled || *enabled == '0' || !lpCurPlayer)
        return;

    static bool f11WasDown = false;
    const bool f11Down = (GetAsyncKeyState(VK_F11) & 0x8000) != 0;
    if(!f11Down)
    {
        f11WasDown = false;
        return;
    }
    if(f11WasDown)
        return;
    f11WasDown = true;

    const int slot = lpCurPlayer->OnStageField;
    if(slot < 0 || slot >= 2 || !StageField[slot] || StageField[slot]->FieldCode != 52)
        return;

    const int targetX = 198280 * fONE;
    const int targetZ = 242864 * fONE;
    summit::diagnostics::RuntimeAssetProbeAttempt(
        "DIAGNOSTIC", "ERAGON_BYPASS_F11", "field52@198280,242864");
    lpCurPlayer->SetPosi(targetX, 0, targetZ, 0, 0, 0);
    TraceCameraPosi.x = lpCurPlayer->pX;
    TraceCameraPosi.y = lpCurPlayer->pY;
    TraceCameraPosi.z = lpCurPlayer->pZ;
    TraceTargetPosi = TraceCameraPosi;
}

int PlayPat3D()
{

	SummitEragonDiagnosticBypassTick();

	int cnt;
	int attack;
	int	attack_UserMonster;
	int	attack_RegState;

	int wx, wz, x, z;
	int	attackMode;
	sSKILL *lpAttackSkill;
	smWINSOCK *lpsmSock;
	int	skFlag;

#ifdef	GAME_GM

	if(SkipNextField)
	{
		if(SkipNextField == 1)
		{
			WarpNextField(&wx, &wz);
			lpCurPlayer->SetPosi(wx, 0, wz, 0, 0, 0);
			SkipNextField = 0;
			TraceCameraPosi.x = lpCurPlayer->pX;
			TraceCameraPosi.y = lpCurPlayer->pY;
			TraceCameraPosi.z = lpCurPlayer->pZ;
			TraceTargetPosi.x = lpCurPlayer->pX;
			TraceTargetPosi.y = lpCurPlayer->pY;
			TraceTargetPosi.z = lpCurPlayer->pZ;
		}
		else
		{

			WarpCustomField(&wx, &wz);
			lpCurPlayer->SetPosi(wx, 0, wz, 0, 0, 0);
			SkipNextField = 0;
			TraceCameraPosi.x = lpCurPlayer->pX;
			TraceCameraPosi.y = lpCurPlayer->pY;
			TraceCameraPosi.z = lpCurPlayer->pZ;
			TraceTargetPosi.x = lpCurPlayer->pX;
			TraceTargetPosi.y = lpCurPlayer->pY;
			TraceTargetPosi.z = lpCurPlayer->pZ;

		}
	}

#endif





	lpCurPlayer->Main();

	EachPlayerMain();


	AutoDeathCounter++;

	if(PkMode > 0) PkMode--;

#ifdef _LANGUAGE_BRAZIL
	if(VRKeyBuff[VK_CONTROL]) PkMode = FALSE;
#else
	if(VRKeyBuff[VK_CONTROL]) PkMode = TRUE;
#endif

	else PkMode = FALSE;



	if(lpCurPlayer->PlayStunCount)
	{
		if(MsTraceMode)
		{
			CancelAttack();
			lpCurPlayer->MoveFlag = 0;
			MouseButton[0] = 0;
			MouseButton[1] = 0;
			MouseButton[2] = 0;
		}
		if(lpCurPlayer->MotionInfo->State == CHRMOTION_STATE_WALK || lpCurPlayer->MotionInfo->State == CHRMOTION_STATE_RUN)
		{
			lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_STAND);
			CancelAttack();
			lpCurPlayer->MoveFlag = 0;
			MouseButton[0] = 0;
			MouseButton[1] = 0;
			MouseButton[2] = 0;
		}
		lpCurPlayer->dwNextMotionCode = 0;
		lpCurPlayer->MotionLoop = 0;
	}


	if(MsTraceMode && lpCurPlayer->MotionInfo->State != CHRMOTION_STATE_ATTACK && lpCurPlayer->MotionInfo->State != CHRMOTION_STATE_EAT &&
		lpCurPlayer->MotionInfo->State != CHRMOTION_STATE_SKILL)
	{

		attackMode = FALSE;
		AttCancel = FALSE;
		attack_UserMonster = FALSE;

		CheckPlaySubFunc2();

		if((MsTraceCount & 0x1F) == 0)
		{


			lpCurPlayer->MoveFlag = TRUE;
			lpCurPlayer->Angle.y = GetMouseSelAngle();
		}

		int AttackRange = 0;
		int	AttackResult = 0;

		lpAttackSkill = 0;

		switch(SelMouseButton)
		{
		case 1:
			if(sinSkill.pLeftSkill && sinSkill.pLeftSkill->CODE != SKILL_NORMAL_ATTACK)
				lpAttackSkill = sinSkill.pLeftSkill;
			break;
		case 2:
			if(sinSkill.pRightSkill && sinSkill.pRightSkill->CODE != SKILL_NORMAL_ATTACK)
				lpAttackSkill = sinSkill.pRightSkill;
			break;
		}


		if(lpCurPlayer->OnStageField >= 0 && StageField[lpCurPlayer->OnStageField]->State == FIELD_STATE_VILLAGE)
			lpAttackSkill = 0;

		if(lpCharMsTrace)
		{
			if(lpCharMsTrace->smCharInfo.Life[0] <= 0 ||
				(lpCharMsTrace->dwLastTransTime + DISPLAY_TRANS_TIME_OVER) < dwPlayTime ||
				cTrade.OpenFlag)
			{

				CancelAttack();
			}
			else
			{

				if(!MouseButton[0] && !MouseButton[1]) AttFlag = 0;

				if(lpCurPlayer->smCharInfo.State == smCHAR_STATE_ENEMY)
				{

					attackMode = TRUE;
				}
				else
				{
					if(PkMode)
						attackMode = TRUE;

				}

				if(lpCharMsTrace->smCharInfo.dwCharSoundCode == snCHAR_SOUND_KELVEZU)
				{
					AttackRange = 130 * fONE;
				}
				else
					AttackRange = 50 * fONE;

				if(lpCurPlayer->dwActionItemCode)
				{
					AttackRange += lpCurPlayer->AttackToolRange;
				}


				if(lpCurPlayer->ShootingMode && lpCharMsTrace->smCharInfo.State)
				{
					if(lpCharMsTrace->smCharInfo.State != smCHAR_STATE_USER || attackMode)
						AttackRange = lpCurPlayer->smCharInfo.Shooting_Range*fONE;




				}

				if(lpCharMsTrace->smCharInfo.dwCharSoundCode == snCHAR_SOUND_CASTLE_TOWER_B)
					AttackRange += 40 * fONE;

				attack = lpCharMsTrace->smCharInfo.State;

				if(lpCharMsTrace->smCharInfo.State == smCHAR_STATE_USER)
				{
					if(!attackMode ||
						(lpCurPlayer->OnStageField >= 0 && StageField[lpCurPlayer->OnStageField]->State == FIELD_STATE_VILLAGE))
						attack = 0;
				}

				if(lpCharMsTrace->smCharInfo.State == smCHAR_STATE_ENEMY && lpCharMsTrace->smCharInfo.Brood == smCHAR_MONSTER_USER)
				{
					if(!PkMode)
					{
						attack_UserMonster = TRUE;;
						attack = 0;
					}
				}

				if(lpAttackSkill)
				{
					if(lpAttackSkill->CODE == SKILL_HEALING || lpAttackSkill->CODE == SKILL_ENCHANT_WEAPON ||
						lpAttackSkill->CODE == SKILL_VIRTUAL_LIFE || lpAttackSkill->CODE == SKILL_TRIUMPH_OF_VALHALLA)
					{
						if(attack_UserMonster && lpAttackSkill->CODE == SKILL_HEALING)
							attack_UserMonster = 0;

						skFlag = 1;
					}
					else
						skFlag = 0;

					if((lpCharMsTrace->smCharInfo.State == smCHAR_STATE_ENEMY && !attack_UserMonster) ||
						(PkMode && lpCharMsTrace->smCharInfo.State == smCHAR_STATE_USER) || skFlag)
					{


						cnt = GetSkillDistRange(lpAttackSkill, AttackRange, attack);
						if(cnt)
							AttackRange = cnt;

						attack = 0;
					}
				}

				if(!lpCurPlayer->ShootingFlag &&
					lpCurPlayer->PlayAttackFromPosi(
						lpCharMsTrace->pX,
						lpCharMsTrace->pY,
						lpCharMsTrace->pZ,
						AttackRange, attack) == TRUE &&
					lpCurPlayer->MotionInfo->State != CHRMOTION_STATE_EAT)
				{


					LastAttackDamage = 0;
					AutoMouse_Attack_Count++;

					if(lpCharMsTrace->smCharInfo.State == smCHAR_STATE_USER)
					{
						if(lpCharMsTrace->dwTradeMsgCode)
						{
							Send_ShowMyShopItem(lpCharMsTrace);
						}
						else
						{
							EachTradeButton = 1;

#ifdef  __CLANSUBCHIP__
							if((cldata.myPosition == 101 || cldata.myPosition == 104) && !lpCharMsTrace->smCharInfo.ClassClan)
#else
							if(cldata.myPosition == 101 && !lpCharMsTrace->smCharInfo.ClassClan)
#endif
								EachTradeButtonMode = TRUE;
							else
								EachTradeButtonMode = FALSE;
						}

					}
					else
					{
						EachTradeButton = 0;
					}

					if(!lpCharMsTrace->smCharInfo.State)
					{
						OpenEachPlayer(lpCharMsTrace);
						DispEachMode = TRUE;
					}

					lpCurPlayer->chrAttackTarget = lpCharMsTrace;
					lpCurPlayer->AttackExp = 0;

					lpCurPlayer->AttackCritcal = FALSE;


					if(!attack_UserMonster &&
						(lpCharMsTrace->smCharInfo.State != smCHAR_STATE_USER ||
						(lpCurPlayer->OnStageField >= 0 && StageField[lpCurPlayer->OnStageField]->State != FIELD_STATE_VILLAGE)))
					{

						AttackResult = TRUE;


						if(!PlaySkillAttack(lpAttackSkill, lpCharMsTrace))
						{

							if(attack || lpCharMsTrace->smCharInfo.State != smCHAR_STATE_USER)
							{

								if(lpCurPlayer->ShootingMode && lpCharMsTrace->smCharInfo.State != smCHAR_STATE_NPC)
								{
									if(lpCurPlayer->CheckShootingTest(lpCharMsTrace) == TRUE)
									{

										AttackResult = dm_SendTransDamage(lpCharMsTrace, 0, 0, 0, 0,
											lpCurPlayer->AttackSkil, TRUE);

									}
									else
									{
										lpCurPlayer->AttackCritcal = -1;
										lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_STAND);
									}
								}
								else
								{
									attack_RegState = 0;

									if(lpCharMsTrace->smCharInfo.State == smCHAR_STATE_NPC)
									{
										if(sinQuest_ChangeJob2.CODE)
										{

											attack_RegState = (sinQuest_ChangeJob2.CODE << 16);
											if(sinQuest_ChangeJob2.CODE == SIN_QUEST_CODE_CHANGEJOB2_NPC_M ||
												sinQuest_ChangeJob2.CODE == SIN_QUEST_CODE_CHANGEJOB2_NPC_D)
											{

												if(!cInvenTory.SearchItemCode(sinMA1 | sin01) &&
													!cInvenTory.SearchItemCode(sinQT1 | sin04) &&
													!cInvenTory.SearchItemCode(sinQT1 | sin05))
													attack_RegState |= 1;

											}
										}


										if(dwClassDelayTime < dwPlayTime)
										{
											if(lpCharMsTrace->smCharInfo.wPlayClass[0])
											{
												switch(lpCharMsTrace->smCharInfo.wPlayClass[0])
												{

												case 1001:
													if(gameplay::classes::DefaultClassService().IsInBrood(lpCurPlayer->smCharInfo.JOB_CODE, gameplay::classes::brood::Tempskron))
														sinCheckLevelQuest30();
													break;
												case 1002:
													if(gameplay::classes::DefaultClassService().IsInBrood(lpCurPlayer->smCharInfo.JOB_CODE, gameplay::classes::brood::Morayion))
														sinCheckLevelQuest30();
													break;
												case 1003:
													if(gameplay::classes::DefaultClassService().IsInBrood(lpCurPlayer->smCharInfo.JOB_CODE, gameplay::classes::brood::Tempskron))
														sinCheckLevelQuest55();
													break;
												case 1004:
													if(gameplay::classes::DefaultClassService().IsInBrood(lpCurPlayer->smCharInfo.JOB_CODE, gameplay::classes::brood::Morayion))
														sinCheckLevelQuest55_2();
													break;
												case 1005:
													sinCheckLevelQuest70();
													sinCheckLevelQuest80();
													sinCheckLevelQuest85();
													break;
												case 1006:
													sinCheckLevelQuest90();
													break;

												case 1010:
													sinCheckLevelQuest80_2(0);
													break;
												case 1011:
													sinCheckLevelQuest80_2(1);
													break;
												case 1012:
													sinCheckLevelQuest80_2(0);
													sinCheckLevelQuest90_2(0);
													break;
												case 1013:
													sinCheckLevelQuest90_2(1);
													chaQuest.showFuryQuest();
													break;

												}


												HaCheckQuestNpc(lpCharMsTrace->smCharInfo.wPlayClass[0]);

												dwClassDelayTime = dwPlayTime + 1000 * 5;
											}
										}


									}


									AttackResult = dm_SendTransDamage(lpCharMsTrace, 0, 0, 0, attack_RegState,
										lpCurPlayer->AttackSkil, TRUE);

								}


								if((AttackResult && lpCurPlayer->WeaponEffect == (sITEMINFO_LIGHTING + 1) || AttackResult && lpCurPlayer->WeaponEffect == (sITEMINFO_ICE + 1) || AttackResult && lpCurPlayer->WeaponEffect == (sITEMINFO_FIRE + 1))
									&& lpCurPlayer->EnchantEffect_Point > 0 && lpCurPlayer->EnchantEffect_Point <= 10)
								{


									if(lpCurPlayer->AttackCritcal >= 0)
									{

										dm_SelectRange(lpCharMsTrace->pX, lpCharMsTrace->pY, lpCharMsTrace->pZ, 50, TRUE);

										dm_SendRangeDamage(lpCharMsTrace->pX, lpCharMsTrace->pY, lpCharMsTrace->pZ, 0, 0, 0, 0, 0,
											(SKILL_PLAY_ENCHANT_WEAPON | (lpCurPlayer->EnchantEffect_Point << 8)));
									}
								}
							}
							if(!lpAttackSkill && !AttackResult)
							{
								lpCurPlayer->AttackCritcal = -1;
							}
							else
							{

								DamageExp(lpCharMsTrace, AttackResult);
							}
						}
					}


					if(!AttackResult)	lpCurPlayer->AttackCritcal = -1;

					if(!AttCancel)
					{

						lpCurPlayer->MoveFlag = FALSE;
						lpCurPlayer->CriticalLog = lpCurPlayer->AttackCritcal;

						if(lpCharMsTrace && (lpCharMsTrace->smCharInfo.State == smCHAR_STATE_NPC || lpCharMsTrace->dwTradeMsgCode))
						{
							MouseButton[0] = 0;
							MouseButton[1] = 0;
							AttAutoCont = FALSE;
						}

						if(!AttFlag && !AttAutoCont)
						{
							MsTraceMode = 0;
							lpCharMsTrace = 0;
						}
					}
				}
				else
				{
					if(!lpCharMsTrace->Flag || lpCharMsTrace->MotionInfo->State == CHRMOTION_STATE_DEAD)
					{
						lpCharMsTrace = NULL;
						MsTraceMode = 0;
						lpCurPlayer->MoveFlag = FALSE;
						AttAutoCont = FALSE;
					}
				}
			}
		}

		if(lpMsTraceItem)
		{
			if(lpCurPlayer->PlayAttackFromPosi(
				lpMsTraceItem->pX,
				lpMsTraceItem->pY,
				lpMsTraceItem->pZ,
				8000, FALSE) == TRUE)
			{


				smTRANS_ACITON_ITEM		TransActionItem;
				char	szBuff[64];
				int		len;

				TransActionItem.code = OpCode::OPCODE_PUTITEM;
				TransActionItem.size = sizeof(smTRANS_ACITON_ITEM);

				TransActionItem.x = lpMsTraceItem->pX;
				TransActionItem.y = lpMsTraceItem->pY;
				TransActionItem.z = lpMsTraceItem->pZ;
				TransActionItem.lpStgArea = lpMsTraceItem->lpStgArea;



				if(CheckRecvTrnsItemQue() == TRUE)
				{

					cnt = 0;

					if((lpMsTraceItem->ItemCode&sinITEM_MASK2) == sinCH1)
					{
						const gameplay::classes::ClassService& classService = gameplay::classes::DefaultClassService();
						const int playerJobCode = lpCurPlayer->smCharInfo.JOB_CODE;
						const bool isMale = classService.IsGender(playerJobCode, gameplay::classes::CharacterGender::Male);
						const bool isFemale = classService.IsGender(playerJobCode, gameplay::classes::CharacterGender::Female);

						switch(lpMsTraceItem->ItemCode)
						{
#ifdef _LANGUAGE_ENGLISH
						case (sinCH1 | sin01):
							if(isFemale)
							{
								cMessageBox.ShowMessage(MESSAGE_NOT_PICKUP_MALE);
								cnt = 1;
							}
							break;
						case (sinCH1 | sin02):
							if(isMale)
							{
								cMessageBox.ShowMessage(MESSAGE_NOT_PICKUP_FEMALE);
								cnt = 1;
							}
							break;
#else
						case (sinCH1 | sin01):
						case (sinCH1 | sin02):
							if(isMale)
							{
								cMessageBox.ShowMessage(MESSAGE_NOT_PICKUP_FEMALE);
								cnt = 1;
							}
							break;
#endif
						case (sinCH1 | sin03):
						case (sinCH1 | sin04):
							if(isFemale)
							{
								cMessageBox.ShowMessage(MESSAGE_NOT_PICKUP_MALE);
								cnt = 1;
							}
							break;
						}
					}

					if(!cnt)
					{
						if(lpMsTraceItem->ItemCode == (sinGG1 | sin01))
						{
							strcpy_s(szBuff, lpMsTraceItem->szName);
							len = lstrlen(szBuff);
							for(cnt = 0; cnt < len; cnt++)
							{
								if(szBuff[cnt]<'0' || szBuff[cnt]>'9')
								{
									szBuff[cnt] = 0;
									break;
								}
							}
							cnt = atoi(szBuff);
							if(cInvenTory.CheckMoneyLimit(cnt) == TRUE)
							{
								lpsmSock = GetServerSock(lpMsTraceItem->ServerCode);
								if(lpsmSock)
								{
									SummitPlayMainClientSocketSend(lpsmSock, (char *)&TransActionItem, TransActionItem.size);
								}
							}
						}
						else
						{
							lpsmSock = GetServerSock(lpMsTraceItem->ServerCode);
							if(lpsmSock)
							{
								SummitPlayMainClientSocketSend(lpsmSock, (char *)&TransActionItem, TransActionItem.size);
							}
						}
					}
				}

				MsTraceMode = 0;
				lpMsTraceItem = NULL;
				lpCurPlayer->MoveFlag = FALSE;
				MouseButton[0] = 0;
			}
			else
			{
				if(!lpMsTraceItem->Flag)
				{
					lpMsTraceItem = NULL;
					MsTraceMode = 0;
					lpCurPlayer->MoveFlag = FALSE;
				}
			}
		}


	}
	MsTraceCount++;

	for(cnt = 0; cnt < OTHER_PLAYER_MAX; cnt++)
	{
		if(chrOtherPlayer[cnt].Flag)
		{
			chrOtherPlayer[cnt].Main();
			if(chrOtherPlayer[cnt].dwTarget)
			{
				if(lpCurPlayer->MotionInfo->State != CHRMOTION_STATE_DAMAGE &&
					chrOtherPlayer[cnt].MotionInfo->State == CHRMOTION_STATE_ATTACK)

					lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_DAMAGE);
			}
		}
	}


#ifdef GAME_GM



	// SUMMIT_V17_2_DISABLED_PATTERN_HOTKEYS
	// Shift+F3/F4 changed live player/NPC patterns and could leave AdminChar/PatLoading
	// state inconsistent. Keep the legacy code for reference, but no keyboard path.
	/*
	if(smConfig.getSecurity() >= AccountTypes::SEC_MODERATOR && VRKeyBuff[VK_SHIFT] && !VRKeyBuff[VK_CONTROL])
	{

		if(VRKeyBuff[VK_F4])
		{
			SetNextPlayerPattern();
			VRKeyBuff[VK_F4] = 0;
		}
		if(VRKeyBuff[VK_F3])
		{
			SetNextNpcPattern();
			VRKeyBuff[VK_F3] = 0;
		}
	}
	*/
#endif




	MainFieldMap();


	PlayParticle();

	DynLightMain();

	BGMain();


	BGM_Main();


	SodScoreMain();


	PlayCounter++;

	FieldMain(lpCurPlayer->pX, 0, lpCurPlayer->pZ);


	int fldSky, fflag;

	int	RainTime = 0x200;
	int	RainLength = 60 * 7;
	int	BackSel = 0;

	fldSky = -1;
	fflag = 0;

	if((PlayCounter & 0xFF) == 0 && lpCurPlayer->dwObjectSerial)
	{




		if(dwSoD_ResultTime < dwPlayTime)
		{

			dwSoD_ResultTime = dwPlayTime + 12000;
			if(cSinHelp.GetSodRankBoxState() == TRUE)
			{
				SendCommand_AreaServer(OpCode::OPCODE_SOD_RESULT2, 0, 0, 0, 0);
			}
		}

		if(cldata.myPosition)
		{
			if(lpCurPlayer->smCharInfo.ClassClan != cldata.intClanMark)
			{
#ifdef  __CLANSUBCHIP__				
				if(cldata.myPosition == 100 || cldata.myPosition == 101 || cldata.myPosition == 104)
				{
#else
				if(cldata.myPosition == 100 || cldata.myPosition == 101)
				{
#endif

					lpCurPlayer->smCharInfo.ClassClan = cldata.intClanMark;
					lpCurPlayer->dwClanManageBit = cldata.CNFlag;
					lpCurPlayer->smCharInfo.bUpdateInfo[0]++;
					SendUpdateClanInfo();







				}
				else
				{
					cldata.intClanMark = 0;
					lpCurPlayer->smCharInfo.ClassClan = 0;
					lpCurPlayer->smCharInfo.bUpdateInfo[0]++;

					SendUpdateClanInfo();
				}
			}
		}


		if(lpCurPlayer->smCharInfo.wPlayerKilling[0] > 0)
		{
			lpCurPlayer->smCharInfo.wPlayerKilling[0]--;
			if(lpCurPlayer->smCharInfo.wPlayerKilling[0] == 0)
			{


				lpCurPlayer->MoveFlag = FALSE;
				WarpStartField(&x, &z);
				lpCurPlayer->SetPosi(x, 0, z, 0, 0, 0);
				TraceCameraPosi.x = lpCurPlayer->pX;
				TraceCameraPosi.y = lpCurPlayer->pY;
				TraceCameraPosi.z = lpCurPlayer->pZ;
				TraceTargetPosi.x = lpCurPlayer->pX;
				TraceTargetPosi.y = lpCurPlayer->pY;
				TraceTargetPosi.z = lpCurPlayer->pZ;
				lpCurPlayer->SetMotionFromCode(CHRMOTION_STATE_RESTART);
				StartEffect(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, EFFECT_GAME_START1);
				SetDynLight(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, 100, 100, 100, 0, 200);
				CharPlaySound(lpCurPlayer);


				lpCurPlayer->OnStageField = -1;
				if(smGameStage[0])
				{
					cnt = smGameStage[0]->GetFloorHeight(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, lpCurPlayer->Pattern->SizeHeight);
					lpCurPlayer->OnStageField = 0;
				}
				if(cnt == CLIP_OUT && smGameStage[1])
				{
					lpCurPlayer->OnStageField = 1;
				}
			}
		}


		if(WeatherMode == 0)
		{
			if(dwGameHour < GAME_HOUR_DAY || dwGameHour >= GAME_HOUR_NIGHT)
			{
				BackSel = 2;
			}
			else
			{
				if(dwGameHour >= GAME_HOUR_DAY && dwGameHour < GAME_HOUR_GLOW)
					BackSel = 0;
				else
					BackSel = 1;
			}
		}


		if(lpCurPlayer->OnStageField >= 0)
		{

			if(StageField[lpCurPlayer->OnStageField]->FieldCode == rsSOD_FIELD)
			{

				cnt = rsGetSoD_AreaCode(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ);
				switch(cnt)
				{
				case 0:
				case 1:
				case 2:
				case 3:
					fldSky = FIELD_BACKIMAGE_SODNONE;
					break;

				case 4:
					fldSky = FIELD_BACKIMAGE_SODSUN;
					break;
				case 5:
					fldSky = FIELD_BACKIMAGE_SODMOON;
					break;
				case 6:
					fldSky = FIELD_BACKIMAGE_SODSKY;
					break;
				}
				if(cnt == 2)
					esPlayEffectBgm(4);
				else
					esPlayEffectBgm(3);

				if(fldSky >= 0) ChangeSky(fldSky);
			}
			else if(StageField[lpCurPlayer->OnStageField]->FieldCode == BabelPlayField && dwBabelPlayTime > dwPlayTime)
			{

				ChangeSky(FIELD_BACKIMAGE_IRONBOSS);
				esPlayEffectBgm(11);
			}
			else
			{
				fldSky = StageField[lpCurPlayer->OnStageField]->BackImageCode[BackSel];

				if(!WeatherMode && fldSky >= 0) ChangeSky(fldSky);

				if(WeatherMode == 0)
				{
					switch(StageField[lpCurPlayer->OnStageField]->State)
					{

					case FIELD_STATE_RUIN:
						esPlayEffectBgm(3);
						break;

					case FIELD_STATE_DESERT:
						esPlayEffectBgm(4);
						break;

					case FIELD_STATE_DUNGEON:
						if(StageField[lpCurPlayer->OnStageField]->FieldCode <= 16 ||
							StageField[lpCurPlayer->OnStageField]->FieldCode == 32)
						{
							esPlayEffectBgm(5);
							break;
						}
						if(StageField[lpCurPlayer->OnStageField]->FieldCode <= 23)
						{
							esPlayEffectBgm(6);
							break;
						}
						if(StageField[lpCurPlayer->OnStageField]->FieldCode <= 25)
						{
							esPlayEffectBgm(7);
							break;
						}
						if(StageField[lpCurPlayer->OnStageField]->FieldCode <= 26)
						{
							esPlayEffectBgm(8);
							break;
						}
						break;


					case FIELD_STATE_FOREST:
						if(dwGameHour < GAME_HOUR_DAY || dwGameHour >= GAME_HOUR_DARKNESS)
						{
							esPlayEffectBgm(2);
						}
						else
						{
							esPlayEffectBgm(1);
						}
						break;

					case FIELD_STATE_VILLAGE:
						esPlayEffectBgm(0);
						break;

					case FIELD_STATE_ROOM:
						esPlayEffectBgm(-1);
						break;

					case FIELD_STATE_IRON:
						esPlayEffectBgm(9);
						break;

					case FIELD_STATE_ICE:
						esPlayEffectBgm(12);
						break;

					}
				}
				else
					esPlayEffectBgm(-1);

				if(StageField[lpCurPlayer->OnStageField]->BackMusicCode)
					ChangeBGM(StageField[lpCurPlayer->OnStageField]->BackMusicCode);
			}

		}
	}

	if((PlayCounter & 0x3) == 0)
	{

		if(StageField[lpCurPlayer->OnStageField]->State != FIELD_STATE_ROOM &&
			(dwGameHour < GAME_HOUR_DAY || dwGameHour >= GAME_HOUR_DARKNESS ||
				StageField[lpCurPlayer->OnStageField]->State == FIELD_STATE_DUNGEON))
		{
			NightDayTime = 1;
		}
		else
		{
			NightDayTime = 0;
		}

		cnt = DarkLight;
		if(DarkLevel < cnt)
			DarkLevel++;
		if(DarkLevel > cnt)
			DarkLevel--;

		if((PlayCounter & 0x7) == 0)
		{
			if(BackColor_mR < BackColor_R) BackColor_R--;
			if(BackColor_mR > BackColor_R) BackColor_R++;

			if(BackColor_mG < BackColor_G) BackColor_G--;
			if(BackColor_mG > BackColor_G) BackColor_G++;

			if(BackColor_mB < BackColor_B) BackColor_B--;
			if(BackColor_mB > BackColor_B) BackColor_B++;
		}

		if(smConfig.getSecurity() >= AccountTypes::SEC_MODERATOR)
		{
			if(DarkLevel_Fix)
				DarkLevel = DarkLevel_Fix;
		}

		if((PlayCounter & 0xFF) == 0)
		{
			Ptect_GetAttackCount();

			if((PlayCounter & 0xFFF) == 0)
			{
				if((PlayCounter & 0x3FFF) == 0)
					SendUsingItemListToServer2();
				else
					SendUsingItemListToServer();
			}

			if (SETTINGSHANDLER->getWeather())
			{
				cnt = dwGameWorldTime & 0x1FFF;

				if(cnt > RainTime && cnt < (RainTime + RainLength))
				{
					if(cnt > RainTime + 60 && cnt < (RainTime + RainLength - 60))
					{
						if(WeatherMode != 2 && (lpCurPlayer->OnStageField >= 0 &&
							StageField[lpCurPlayer->OnStageField]->State != FIELD_STATE_DUNGEON  &&
							StageField[lpCurPlayer->OnStageField]->FieldCode != rsSOD_FIELD		&&
							StageField[lpCurPlayer->OnStageField]->State != FIELD_STATE_ICE))
						{

							PlayWav(1, 1, 400, 2205);
							WeatherMode = 2;
							ChangeSky(0);
						}
					}
					else
					{
						switch(WeatherMode)
						{
						case 0:
							ChangeSky(0);
							break;
						case 2:
							StopWav(1);
							break;
						}

						WeatherMode = 1;
					}
				}
				else
					WeatherMode = 0;
			}


			if(lpCurPlayer->smCharInfo.dwEventTime_T)
			{
				if(GetPlayTime_T() > lpCurPlayer->smCharInfo.dwEventTime_T)
				{

					ChangeBigHeadMode(0, 0);
					StartSkill(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, 0, 0, 0, SKILL_UP1);
					SkillPlaySound(SKILL_SOUND_LEARN, lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ);


					cnt = PlayerArmor;
					PlayerArmor = -1;
					SetPlayerArmor(cnt, -1);

					cInvenTory.IncreViewDamage(0);

					if(RestartPlayCount < 350) RestartPlayCount = 350;
				}
			}


			if(rsRecorder.dwNextRecordTime < dwPlayTime)
			{

				if(GetSaveResult() == TRUE)
				{

					SaveGameData();

				}
			}
			if(rsRecorder.dwRecordTime && (rsRecorder.dwRecordTime + 1000 * 8) < dwPlayTime)
			{
#ifdef GAME_GM
				if(smConfig.getSecurity() >= AccountTypes::SEC_MODERATOR) AddChatBuff("SaveGame TimeOut / Retry SaveGame");
#endif
				RecordFailCount++;

				rsRecorder.dwRecordTime = 0;

				SaveGameData();
			}


			if(!DisconnectFlag && RecordFailCount >= 6)
			{

				SendSetHackUser2(2700, RecordFailCount);
				DisconnectFlag = dwPlayTime;
#ifdef GAME_GM
				if(smConfig.getSecurity() >= AccountTypes::SEC_MODERATOR) AddChatBuff("SaveGame Failed over times");
#endif
			}


			if(lpCurPlayer->OnStageField >= 0 && StageField[lpCurPlayer->OnStageField]->State == FIELD_STATE_FOREST &&
				(dwGameHour < GAME_HOUR_DAY || dwGameHour >= GAME_HOUR_DARKNESS) && (PlayCounter & 0x1FF) == 0)
			{

				if(((dwPlayTime >> 3) & 1) == 0)
				{
					StartEffect(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, EFFECT_BANDI1);
				}
			}
		}


		if(lpCurPlayer->OnStageField >= 0)
		{

			StageField[lpCurPlayer->OnStageField]->PlayAmbient();
		}
		else
		{
			if(ResetLastVolumeDist() > 800)
			{
				esPlayObjectSound2(-1, 0, 0, 0, 0);
			}
		}


	}



	dsMenuCursorPos(&pCursorPos, 4);

	MainInterfaceParty();
	dsMenuCursorPos(&pCursorPos, 0);



	if(lpCurPlayer->OnStageField >= 0 && StageField[lpCurPlayer->OnStageField])
	{
		StageField[lpCurPlayer->OnStageField]->CheckWarpGate(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ);
	}

	if(RestartPlayCount > 0) RestartPlayCount--;


	if((PlayCounter & 0xFFF) == 0)
	{


		CheckCracker(NULL);

		CheckCharForm();

		if(!lpCurPlayer->HoSkillCode && !lpCurPlayer->dwDispAppSkill)
		{

			if(!dwRotPlayDelayTime || dwRotPlayDelayTime < dwPlayTime)
			{
				smCHAR	*lpTempChar;
				smCHAR	*lpOldChar;

				lpOldChar = lpCurPlayer;

				lpTempChar = SelectRotPlayer(lpCurPlayer);
				if(lpTempChar)
				{
					lpCurPlayer = lpTempChar;
					sinChar = &lpCurPlayer->smCharInfo;


					for(cnt = 0; cnt < OTHER_PLAYER_MAX; cnt++)
					{
						if(chrOtherPlayer[cnt].Flag && chrOtherPlayer[cnt].chrAttackTarget == lpOldChar)
							chrOtherPlayer[cnt].chrAttackTarget = lpCurPlayer;
					}

				}
			}
		}
		else
		{
			dwRotPlayDelayTime = dwPlayTime + 30 * 1000;
		}

		CheckEnergyGraphError();
	}


	TimeEffectMain();

	CheckEnergyGraph();




#ifdef	_SET_PROCTECT_MEM2

	if((PlayCounter & 0x7FF) == 0)
	{

		cnt = Check_PlaySubReleaseFunc();
		if(cnt)
		{
			SendSetHackUser2(1120, cnt);
		}
	}
#endif


	if(dwPartyPosionTime && dwPartyPosionTime < dwPlayTime)
	{

		LaunchPartyPostion();
	}



	if((PlayCounter & 0x1FF) == 0)
	{
		CheckEnergyGraphError();

		if(AutoMouse_StartCheckTime)
		{
			if((AutoMouse_StartCheckTime + 30000) < dwPlayTime)
			{

				if(AutoMouse_Distance > 20000 && AutoMouse_Attack_Count > 20 && AutoMouse_WM_Count > 110 && (AutoMouse_LastCheckTime + 30 * 60 * 1000) < dwPlayTime)
				{
					AutoMouse_LastCheckTime = dwPlayTime;

					SendSetHackUser2(8100, AutoMouse_WM_Count);

					if(smConfig.getSecurity() >= AccountTypes::SEC_MODERATOR)
					{
						AddChatBuff(">Warning AutoMouse", 0);
					}
				}


				AutoMouse_StartCheckTime = dwPlayTime;
				AutoMouse_WM_Count = 0;
				AutoMouse_Distance = 0;
				AutoMouse_Attack_Count = 0;
			}
		}
		else
		{
			AutoMouse_StartCheckTime = dwPlayTime;
			AutoMouse_WM_Count = 0;
			AutoMouse_Distance = 0;
			AutoMouse_Attack_Count = 0;
		}


	}

	HackTrap_CheckOpenFlag();

#ifdef _TEST_SERVER

	if(smConfig.getSecurity() >= AccountTypes::SEC_MODERATOR && VRKeyBuff[VK_CONTROL] && VRKeyBuff['N'] && sinGetKeyClick('M'))
	{



		if(!ParkPlaying)
			Play_ParkPlayer("mms:
		else
				Stop_ParkPlayer();

	}
#endif

	return TRUE;
}

extern int BGMatNum[2];
extern int RestoreTexFlag;
extern POINT	pCursorPos;



int GetMouseSelAngle()
{
	int x1, x2, z1, z2;

	if(lpCharMsTrace)
	{
		x1 = lpCurPlayer->pX >> FLOATNS;
		z1 = lpCurPlayer->pZ >> FLOATNS;

		x2 = lpCharMsTrace->pX >> FLOATNS;
		z2 = lpCharMsTrace->pZ >> FLOATNS;

		return GetRadian2D(x1, z1, x2, z2);
	}

	if(lpMsTraceItem)
	{
		x1 = lpCurPlayer->pX >> FLOATNS;
		z1 = lpCurPlayer->pZ >> FLOATNS;

		x2 = lpMsTraceItem->pX >> FLOATNS;
		z2 = lpMsTraceItem->pZ >> FLOATNS;

		return GetRadian2D(x1, z1, x2, z2);
	}

	return -1;
}


int GetCharAngle(smCHAR *lpChar1, smCHAR *lpChar2)
{
	int x1, x2, z1, z2;

	x1 = lpChar1->pX >> FLOATNS;
	z1 = lpChar1->pZ >> FLOATNS;

	x2 = lpChar2->pX >> FLOATNS;
	z2 = lpChar2->pZ >> FLOATNS;

	return GetRadian2D(x1, z1, x2, z2);
}


int testframe = 0;
int	LastRendSucessCount = 0;
int	DispCharLimitMode = 0;

#define	DISP_CHAR_LIMIT			20
#define	DISP_CHAR_LIMIT_MAX		30
#define	DISP_CHAR_NEAR_DIST		( (10*64)*(10*64) )

int DrawPat3D(int x, int y, int z, int ax, int ay, int az)
{



	int cnt;
	int SelZ;
	int sez;
	RECT	rect;
	RECT	SelRect;
	POINT	SelPos;
	RECT	*lpRect;

	smCHAR	*lpSelChar;
	scITEM	*lpItem;
	int		dx, dz;
	int		pow;

	POINT	sPos;
	int		RendSucessCount;
	int		DispFlag;
	int		r, g, b;
	DWORD	dwDispAppSKill;


	dwDispAppSKill = lpCurPlayer->dwDispAppSkill;
	lpCurPlayer->dwDispAppSkill = 0;

	for(cnt = 0; cnt < MAX_CONTINUE_SKILL; cnt++)
	{
		if(ContinueSkill[cnt].Flag)
		{
			switch(ContinueSkill[cnt].CODE)
			{
			case SKILL_EXTREME_SHIELD:
				lpCurPlayer->dwDispAppSkill |= SKILL_APPMASK_EXTREME_SHIELD;
				break;

			case SKILL_PHYSICAL_ABSORB:
				lpCurPlayer->dwDispAppSkill |= SKILL_APPMASK_PHYSICAL_ABSORB;
				break;

			case SKILL_AUTOMATION:
				lpCurPlayer->dwDispAppSkill |= SKILL_APPMASK_AUTOMATION;
				break;

			case SKILL_MAXIMIZE:
				lpCurPlayer->dwDispAppSkill |= SKILL_APPMASK_AUTOMATION;
				break;

			case SKILL_CRITICAL_HIT:
				lpCurPlayer->dwDispAppSkill |= SKILL_APPMASK_ANGER;
				break;

			case SKILL_HOLY_BODY:
				lpCurPlayer->dwDispAppSkill |= SKILL_APPMASK_PHYSICAL_ABSORB;
				lpCurPlayer->dwDispAppSkill |= SKILL_APPMASK_HOLY_BODY;
				DispApp_SkillPoint = ContinueSkill[cnt].Point;
				break;

			case SKILL_ZENITH:
				lpCurPlayer->dwDispAppSkill |= SKILL_APPMASK_ANGER;
				break;

			case SKILL_SPARK_SHIELD:
			case SKILL_DIVINE_INHALATION:
				lpCurPlayer->dwDispAppSkill |= SKILL_APPMASK_SPARK_SHIELD;
				break;

			case SKILL_GODLY_SHIELD:
			case SKILL_FROST_JAVELIN:
			case SKILL_SUMMON_MUSPELL:
				lpCurPlayer->dwDispAppSkill |= SKILL_APPMASK_GODLY_SHIELD;
				break;
			}
		}
	}


	if((lpCurPlayer->dwDispAppSkill&SKILL_APPMASK_PHYSICAL_ABSORB) == 0 && (dwDispAppSKill&SKILL_APPMASK_PHYSICAL_ABSORB) != 0)
	{
		SkillStopSound(SKILL_SOUND_PHYSICAL_ABSORB3);
	}

	if((lpCurPlayer->dwDispAppSkill&SKILL_APPMASK_AUTOMATION) == 0 && (dwDispAppSKill&SKILL_APPMASK_AUTOMATION) != 0)
	{
		SkillStopSound(SKILL_SOUND_SKILL_AUTOMATION3);
	}

	if((lpCurPlayer->dwDispAppSkill&SKILL_APPMASK_HOLY_BODY) == 0 && (dwDispAppSKill&SKILL_APPMASK_HOLY_BODY) != 0)
	{
		SkillStopSound(SKILL_SOUND_PHYSICAL_ABSORB3);
	}






	SelZ = 65536 * 16384;

	smRender.SetCameraPosi(x, y, z, x, ay, az);
	if(smGameStage[0])
		SetStageDynLight(smGameStage[0], x, y, z);
	if(smGameStage[1])
		SetStageDynLight(smGameStage[1], x, y, z);



	smCHAR_SetCameraPosi(x, y, z, ax, ay, az);
	if(!DebugPlayer)
	{

		cnt = RestartPlayCount;
		if(lpCurPlayer->PlayInvincible > 0) cnt = lpCurPlayer->PlayInvincible;

		if(cnt && (cnt & 0x10) == 0)
		{
			r = smRender.Color_R;
			g = smRender.Color_G;
			b = smRender.Color_B;

			smRender.Color_R += 80;
			smRender.Color_B += 80;



			lpCurPlayer->Draw();

			smRender.Color_R = r;
			smRender.Color_G = g;
			smRender.Color_B = b;

		}
		else
		{
			if(lpCurPlayer->PlayVanish <= 0) lpCurPlayer->Draw();
		}


		if(rsBlessCastle.dwMasterClan && lpCurPlayer->smCharInfo.ClassClan)
			SetClanMaster_Player(rsBlessCastle.dwMasterClan);

	}

	lpSelChar = 0;
	RendSucessCount = 0;

	for(cnt = 0; cnt < OTHER_PLAYER_MAX; cnt++)
	{
		if(chrOtherPlayer[cnt].Flag && chrOtherPlayer[cnt].DisplayAlpha == 0)
		{

			DispFlag = TRUE;

			if(DispCharLimitMode)
			{

				dx = (lpCurPlayer->pX - chrOtherPlayer[cnt].pX) >> FLOATNS;
				dz = (lpCurPlayer->pZ - chrOtherPlayer[cnt].pZ) >> FLOATNS;
				if((dx*dx + dz*dz) > DISP_CHAR_NEAR_DIST) DispFlag = FALSE;
			}

			if(DispFlag)
			{

				r = smRender.Color_R;
				g = smRender.Color_G;
				b = smRender.Color_B;


				if(lpCurPlayer->HoSkillCode && chrOtherPlayer[cnt].smCharInfo.Life[0])
				{
					switch(lpCurPlayer->HoSkillCode)
					{
					case SKILL_SCOUT_HAWK:
						pow = GetSkillPower(lpCurPlayer, &chrOtherPlayer[cnt], 0, 0, 0);
						if(pow)
						{


							pow = DarkLevel >> 2;
							smRender.Color_R += 40 + pow;
							smRender.Color_G += 40 + pow;
							smRender.Color_B += 100 + pow;

						}
						break;
					}
				}

				if(lpCharSelPlayer && lpCharSelPlayer == &chrOtherPlayer[cnt])
				{
					smRender.Color_R = r + 40;
					smRender.Color_G = g + 60;
					smRender.Color_B = b + 60;


					DWORD OutLineColor;
					switch(lpCharSelPlayer->smCharInfo.State)
					{
					case smCHAR_STATE_USER:
						OutLineColor = D3DCOLOR_RGBA(255, 255, 255, 255);
						break;

					case smCHAR_STATE_NPC:
						OutLineColor = D3DCOLOR_RGBA(0, 255, 100, 255);
						break;

					default:
						OutLineColor = D3DCOLOR_RGBA(255, 30, 0, 255);
						break;
					}

					int ix = (lpCharSelPlayer->pX - smCHAR_CameraX) >> FLOATNS;
					int iy = (lpCharSelPlayer->pY - smCHAR_CameraY) >> FLOATNS;
					int iz = (lpCharSelPlayer->pZ - smCHAR_CameraZ) >> FLOATNS;

					int Dist = (int)sqrt(ix*ix + iy*iy + iz*iz);
					smRender.SetD3D_OutLine(1, Dist, 1.4f, 0, OutLineColor);


				}

				if(!chrOtherPlayer[cnt].RenderLatter && chrOtherPlayer[cnt].AttackIce == 0 && chrOtherPlayer[cnt].RendAlpha == 0 && chrOtherPlayer[cnt].PlayVanish <= 0)
				{


					if(chrOtherPlayer[cnt].PlayInvincible && (chrOtherPlayer[cnt].PlayInvincible & 0x10) == 0)
					{
						smRender.Color_R += 80;
						smRender.Color_B += 80;
					}

					chrOtherPlayer[cnt].Draw();
				}
				else
				{
					chrOtherPlayer[cnt].VirtualDraw();
				}

				smRender.Color_R = r;
				smRender.Color_G = g;
				smRender.Color_B = b;



				smRender.SetD3D_OutLine(0);


				if(chrOtherPlayer[cnt].RendSucess)
				{

					RendSucessCount++;

					sez = chrOtherPlayer[cnt].RendPoint.z;
					lpRect = &chrOtherPlayer[cnt].RendRect2D;

					if(chrOtherPlayer[cnt].smCharInfo.Life[0] > 0 && sez > (96 * fONE) && sez < SelZ &&
						lpRect->left<pCursorPos.x && lpRect->right>pCursorPos.x &&
						lpRect->top<pCursorPos.y && lpRect->bottom>pCursorPos.y)
					{

						SelZ = sez;
						lpSelChar = &chrOtherPlayer[cnt];
					}
				}
			}
			else
			{
				chrOtherPlayer[cnt].RendSucess = FALSE;
				RendSucessCount++;
			}
		}
	}


	LastRendSucessCount = RendSucessCount;


	if(DispCharLimitMode)
	{
		if(RendSucessCount < DISP_CHAR_LIMIT)
			DispCharLimitMode = FALSE;
	}
	else
	{
		if(RendSucessCount > DISP_CHAR_LIMIT_MAX)
			DispCharLimitMode = TRUE;
	}

	if(VRKeyBuff['A'])
	{
		lpSelChar = 0;
		SelZ = 65536 * 16384;
	}


	lpItem = 0;

	for(cnt = 0; cnt < DISP_ITEM_MAX; cnt++)
	{
		if(scItems[cnt].Flag)
		{
			if(scItems[cnt].ItemCode != 0 || DisplayDebug)
			{
				scItems[cnt].Draw();

				smRender.SetCameraPosi(x, y, z, ax, ay, az);

				sez = smRender.GetRect2D(scItems[cnt].pX, scItems[cnt].pY + (16 * fONE), scItems[cnt].pZ,
					32 * fONE, 32 * fONE, &rect, &sPos);

				scItems[cnt].RendPoint.x = sPos.x;
				scItems[cnt].RendPoint.y = sPos.y;
				scItems[cnt].RendPoint.z = sez;

				if(sez > 0 && sez < SelZ &&
					rect.left<pCursorPos.x && rect.right>pCursorPos.x &&
					rect.top<pCursorPos.y && rect.bottom>pCursorPos.y)
				{

					SelZ = sez;
					lpItem = &scItems[cnt];
					memcpy(&SelRect, &rect, sizeof(RECT));
					memcpy(&SelPos, &sPos, sizeof(POINT));
				}
			}

		}
	}

	if(lpSelChar)
	{
		lpCharSelPlayer = lpSelChar;
		lpItem = 0;
	}
	else
		lpCharSelPlayer = 0;

	if(lpItem)
	{
		lpSelItem = lpItem;
		memcpy(&MsSelRect, &SelRect, sizeof(RECT));
		memcpy(&MsSelPos, &SelPos, sizeof(POINT));
	}
	else
		lpSelItem = 0;



	if(cShop.OpenFlag || cCharShop.OpenFlag)
	{

		lpCharSelPlayer = 0;
		lpSelItem = 0;

		if(CursorClass == SIN_CURSOR_ATTACK ||
			CursorClass == SIN_CURSOR_TALK ||
			CursorClass == SIN_CURSOR_GETITEM2 ||
			CursorClass == SIN_CURSOR_GETITEM1)
		{

			CursorClass = 1;
		}
		return TRUE;
	}

	if(MouseItem.Flag)
	{
		CursorClass = 0;
	}
	else
	{
		CursorClass = 1;

		if(lpCharSelPlayer)
		{
			switch(lpCharSelPlayer->smCharInfo.State)
			{
			case smCHAR_STATE_ENEMY:

				CursorClass = SIN_CURSOR_ATTACK;
				break;
			case smCHAR_STATE_USER:
				break;
			case smCHAR_STATE_NPC:
				CursorClass = SIN_CURSOR_TALK;
				break;
			}
		}

		if(lpSelItem)
		{
			if(MouseButton[0])
				CursorClass = SIN_CURSOR_GETITEM2;
			else
				CursorClass = SIN_CURSOR_GETITEM1;
		}
	}

	return TRUE;
}

int DrawPat3D_Alpha()
{
	int cnt;
	int DispFlag;
	int dx, dz;
	int ap;
	int	RendFalg;
	int BackFrame;
	int cnt2;



	if(lpCurPlayer->PlayVanish > 0)
	{
		ap = lpCurPlayer->DisplayAlpha;
		lpCurPlayer->DisplayAlpha = -160;

		if(lpCurPlayer->PlayVague)
		{
			SetItem2PassTexture(8, smTEXSTATE_FS_SCROLL5);
			lpCurPlayer->DisplayAlpha = -60;
		}
		lpCurPlayer->Draw();

		SetItem2PassTexture(-1);
		lpCurPlayer->DisplayAlpha = ap;
	}

	if((lpCurPlayer->AttackSkil & 0xFF) == SKILL_PLAY_SHADOW_MASTER && lpCurPlayer->MotionInfo->State == CHRMOTION_STATE_SKILL)
	{
		ap = lpCurPlayer->DisplayAlpha;
		BackFrame = lpCurPlayer->frame;
		lpCurPlayer->DisplayAlpha = -140;
		for(cnt2 = 0; cnt2 < 6; cnt2++)
		{
			lpCurPlayer->frame -= 160 * 2;
			if(lpCurPlayer->frame > (lpCurPlayer->MotionInfo->StartFrame * 160))
			{
				lpCurPlayer->Draw();
			}
			lpCurPlayer->DisplayAlpha -= 20;
		}
		lpCurPlayer->frame = BackFrame;
		lpCurPlayer->DisplayAlpha = ap;
	}

	for(cnt = 0; cnt < OTHER_PLAYER_MAX; cnt++)
	{
		if(chrOtherPlayer[cnt].Flag)
		{
			RendFalg = 0;
			if(chrOtherPlayer[cnt].DisplayAlpha != 0 || chrOtherPlayer[cnt].RendAlpha != 0 || chrOtherPlayer[cnt].AttackIce != 0 || chrOtherPlayer[cnt].RenderLatter || chrOtherPlayer[cnt].PlayVanish)
			{
				DispFlag = TRUE;

				if(DispCharLimitMode && chrOtherPlayer[cnt].AttackIce == 0)
				{

					dx = (lpCurPlayer->pX - chrOtherPlayer[cnt].pX) >> FLOATNS;
					dz = (lpCurPlayer->pZ - chrOtherPlayer[cnt].pZ) >> FLOATNS;
					if((dx*dx + dz*dz) > DISP_CHAR_NEAR_DIST) DispFlag = FALSE;
				}

				if(chrOtherPlayer[cnt].PlayVanish > 0 && lpCurPlayer->OnStageField >= 0 && StageField[lpCurPlayer->OnStageField]->FieldCode == rsCASTLE_FIELD)
				{

					if(lpCharSelPlayer != &chrOtherPlayer[cnt])
					{
						dx = (lpCurPlayer->pX - chrOtherPlayer[cnt].pX) >> FLOATNS;
						dz = (lpCurPlayer->pZ - chrOtherPlayer[cnt].pZ) >> FLOATNS;
						if((dx*dx + dz*dz) > DIST_TRANSLEVEL_NEAR) DispFlag = FALSE;
					}
				}

				if(DispFlag)
				{


					if(lpCharSelPlayer && lpCharSelPlayer == &chrOtherPlayer[cnt])
					{
						DWORD OutLineColor;
						switch(lpCharSelPlayer->smCharInfo.State)
						{
						case smCHAR_STATE_USER:
							OutLineColor = D3DCOLOR_RGBA(255, 255, 255, 255);
							break;

						case smCHAR_STATE_NPC:
							OutLineColor = D3DCOLOR_RGBA(0, 255, 100, 255);
							break;

						default:
							OutLineColor = D3DCOLOR_RGBA(255, 30, 0, 255);
							break;
						}

						int ix = (lpCharSelPlayer->pX - smCHAR_CameraX) >> FLOATNS;
						int iy = (lpCharSelPlayer->pY - smCHAR_CameraY) >> FLOATNS;
						int iz = (lpCharSelPlayer->pZ - smCHAR_CameraZ) >> FLOATNS;

						int Dist = (int)sqrt(ix*ix + iy*iy + iz*iz);
						smRender.SetD3D_OutLine(1, Dist, 1.4f, 0, OutLineColor);
					}


					ap = chrOtherPlayer[cnt].DisplayAlpha;

					if(chrOtherPlayer[cnt].PlayVanish > 0)
					{
						chrOtherPlayer[cnt].DisplayAlpha = -160;
						if(chrOtherPlayer[cnt].PlayVague) SetItem2PassTexture(8, smTEXSTATE_FS_SCROLL5);
					}

					if(chrOtherPlayer[cnt].RendAlpha && ap > chrOtherPlayer[cnt].RendAlpha)
					{

						chrOtherPlayer[cnt].DisplayAlpha = chrOtherPlayer[cnt].RendAlpha;

						if(chrOtherPlayer[cnt].MotionInfo->State >= 0x100 &&
							(chrOtherPlayer[cnt].smCharInfo.dwCharSoundCode == snCHAR_SOUND_ILLUSIONKNIGHT || chrOtherPlayer[cnt].PlayVanish > 0))
						{
							chrOtherPlayer[cnt].DisplayAlpha = ap;
						}

						chrOtherPlayer[cnt].Draw();
						chrOtherPlayer[cnt].DisplayAlpha = ap;

					}
					else
					{
						chrOtherPlayer[cnt].Draw();
					}
					RendFalg++;

					SetItem2PassTexture(-1);
					chrOtherPlayer[cnt].DisplayAlpha = ap;


					smRender.SetD3D_OutLine(0);


				}
			}

			if(RendFalg == 0)
			{
				if((chrOtherPlayer[cnt].AttackSkil & 0xFF) == SKILL_PLAY_SHADOW_MASTER && chrOtherPlayer[cnt].MotionInfo->State == CHRMOTION_STATE_SKILL)
				{
					ap = chrOtherPlayer[cnt].DisplayAlpha;
					BackFrame = chrOtherPlayer[cnt].frame;
					chrOtherPlayer[cnt].DisplayAlpha = -160;
					for(cnt2 = 0; cnt2 < 4; cnt2++)
					{
						chrOtherPlayer[cnt].frame -= 160 * 4;
						if(chrOtherPlayer[cnt].frame > (chrOtherPlayer[cnt].MotionInfo->StartFrame * 160))
						{
							chrOtherPlayer[cnt].Draw();
						}
					}
					chrOtherPlayer[cnt].frame = BackFrame;
					chrOtherPlayer[cnt].DisplayAlpha = ap;
				}
			}
		}
	}

	return TRUE;
}


int DrawPatShadow(int x, int y, int z, int ax, int ay, int az)
{
	int cnt;

	smCHAR_SetCameraPosi(x, y, z, ax, ay, az);



	cnt = 0;

	lpCurPlayer->DrawShadow();



	if(lpCurPlayer->OnStageField >= 0)
	{
		if(StageField[lpCurPlayer->OnStageField]->State == FIELD_STATE_ICE)
			lpCurPlayer->DrawIceFoot();
	}


	for(cnt = 0; cnt < OTHER_PLAYER_MAX; cnt++)
	{
		if(chrOtherPlayer[cnt].Flag && chrOtherPlayer[cnt].DisplayAlpha == 0 && chrOtherPlayer[cnt].RendAlpha == 0)
		{
			chrOtherPlayer[cnt].DrawShadow();
		}
	}



	return TRUE;
}

extern BYTE VRKeyBuff[256];

int Draw2DCounter = 0;

int DrawPat2D(int x, int y, int z, int ax, int ay, int az)
{





	int cnt;

	if(lpCurPlayer->Pattern == 0) return FALSE;

	smCHAR_SetCameraPosi(x, y, z, ax, ay, az);
	smRender.SetMaterialGroup(smMaterialGroup);

	for(cnt = 0; cnt < OTHER_PLAYER_MAX; cnt++)
	{
		if(chrOtherPlayer[cnt].Flag)
		{
			chrOtherPlayer[cnt].DrawMotionBlur();
		}
	}

	lpCurPlayer->DrawMotionBlur();

	DrawParticle(x, y, z, ax, ay, az);





	Draw2DCounter++;


	DrawLightImage();

	DrawWeather(ax);


	return TRUE;

}

extern int ViewPointLen;

extern DWORD smBackColor;

int BGFace[2][3] = {
	{ 0 ,  1 , 2 },
	{ 1 ,  3 , 2 }
};


LPDIRECT3DTEXTURE9 *hBGTexture;
LPDIRECT3DTEXTURE9 *hBGTexture2;
smOBJ3D *smBackGround;

LPDIRECT3DTEXTURE9 *BackVideoSurface;
int BackVideoFrame;
int LastFrame;

int BGMatNum[2];


#include "systems/rendering/background/legacy/pbackground.h"

smBACKGROUND	smBackMesh[2];
smBACKGROUND	*lpCurrentBG;


int InitBackGround()
{




	lpCurrentBG = 0;

	ChangeSky(0);

	return TRUE;
}

int CloseBackGround()
{

	smBackMesh[1].Close();
	smBackMesh[0].Close();
	return TRUE;

}

int BgFrame = 0;

int BGMain()
{
	smBackMesh[0].Main();
	smBackMesh[1].Main();

	if(lpCurrentBG && lpCurrentBG->FadeShowCount == 0)
	{
		if(&smBackMesh[0] != lpCurrentBG && smBackMesh[0].smObjBG)
			smBackMesh[0].Close();

		if(&smBackMesh[1] != lpCurrentBG && smBackMesh[1].smObjBG)
			smBackMesh[1].Close();
	}

	return TRUE;
}

int ChangeBG(int BgNum)
{

	if(!lpCurrentBG)
	{
		smBackMesh[0].Init(BgNum);
		lpCurrentBG = &smBackMesh[0];
		if(smBackMesh[0].lpMaterialGroup)
			smBackMesh[0].lpMaterialGroup->ReadTextures();

		smBackMesh[0].FadeShowCount = 0;

		return TRUE;
	}

	if(lpCurrentBG->BackGroundNum == BgNum) return FALSE;

	if(&smBackMesh[0] != lpCurrentBG && !smBackMesh[0].smObjBG)
	{
		smBackMesh[0].Init(BgNum);
		lpCurrentBG = &smBackMesh[0];
		if(smBackMesh[0].lpMaterialGroup)
			smBackMesh[0].lpMaterialGroup->ReadTextures();
		return TRUE;
	}

	if(&smBackMesh[1] != lpCurrentBG && !smBackMesh[1].smObjBG)
	{
		smBackMesh[1].Init(BgNum);
		lpCurrentBG = &smBackMesh[1];
		if(smBackMesh[1].lpMaterialGroup)
			smBackMesh[1].lpMaterialGroup->ReadTextures();
		return TRUE;
	}

	return FALSE;
}

int DrawBG(int x, int y, int z, int ax, int ay, int az)
{


	if(StageField[lpCurPlayer->OnStageField]->State == FIELD_STATE_DUNGEON) return TRUE;
	if(StageField[lpCurPlayer->OnStageField]->State == FIELD_STATE_ROOM) return TRUE;

	d3ddev->SetRenderState(D3DRS_ZENABLE, FALSE);

	if(smBackMesh[0].FadeShowCount > smBackMesh[1].FadeShowCount)
	{
		smBackMesh[0].Draw(x, y, z, ax, ay, az);
		smBackMesh[1].Draw(x, y, z, ax, ay, az);
	}
	else
	{
		smBackMesh[1].Draw(x, y, z, ax, ay, az);
		smBackMesh[0].Draw(x, y, z, ax, ay, az);
	}

	d3ddev->SetRenderState(D3DRS_ZENABLE, TRUE);

	return TRUE;
}


RECT	PartyTradeButtonRect[4] = {
	{	00,0, 20, 20	},
	{	25,0, 20, 20	},
	{	50,0, 20, 20	},
	{	75,0, 20, 20	}
};

RECT	PartyTradeToolTipRect = { 1,18,70,28 };

POINT	PartyButtonPoint = { -1, -1 };


int GetPartyTradeButtonPos(int x, int y)
{
	int cnt, sx;
	int AdjustX;
	int MaxCnt;

	int wx, wy;
	if(PartyButtonPoint.x < 0) return -1;

	wx = PartyButtonPoint.x;
	wy = PartyButtonPoint.y;

	if(EachTradeButtonMode)
	{
		AdjustX = -10; MaxCnt = 4;
	}
	else
	{
		AdjustX = 0; MaxCnt = 3;
	}

	for(cnt = 0; cnt < MaxCnt; cnt++)
	{
		sx = PartyTradeButtonRect[cnt].left + AdjustX + wx;
		if(sx<x && (sx + PartyTradeButtonRect[cnt].right)>x &&
			PartyTradeButtonRect[cnt].top + wy<y && (PartyTradeButtonRect[cnt].top + wy + PartyTradeButtonRect[cnt].bottom)>y)
		{

			return cnt;

		}
	}

	return -1;
}


int DisplayPartyTradeButton()
{

	int SelNum;
	int cnt;
	int AdjustX;
	int MaxCnt;

	int wx, wy;
	if(PartyButtonPoint.x < 0) return -1;
	wx = PartyButtonPoint.x;
	wy = PartyButtonPoint.y;

	if(EachTradeButtonMode)
	{
		AdjustX = -10; MaxCnt = 4;
	}
	else
	{
		AdjustX = 0; MaxCnt = 3;
	}

	SelNum = GetPartyTradeButtonPos(pCursorPos.x, pCursorPos.y);

	for(cnt = 0; cnt < MaxCnt; cnt++)
	{
		if(SelNum == cnt)
		{
			if(lpDDS_ParTradeButton[cnt][1])
			{
				DrawSprite(PartyTradeButtonRect[cnt].left + AdjustX + wx, PartyTradeButtonRect[cnt].top + wy,
					lpDDS_ParTradeButton[cnt][1], 0, 0, PartyTradeButtonRect[cnt].right, PartyTradeButtonRect[cnt].bottom);
			}
		}
		else
		{
			if(lpDDS_ParTradeButton[cnt][0])
			{
				DrawSprite(PartyTradeButtonRect[cnt].left + AdjustX + wx, PartyTradeButtonRect[cnt].top + wy,
					lpDDS_ParTradeButton[cnt][0], 0, 0, PartyTradeButtonRect[cnt].right, PartyTradeButtonRect[cnt].bottom);
			}
		}
	}

	if(SelNum >= 0)
	{
		if(lpDDS_ParTradeToolTip[SelNum])
		{
			DrawSprite(PartyTradeButtonRect[SelNum].left - 60 + AdjustX + wx, PartyTradeToolTipRect.top + wy,
				lpDDS_ParTradeToolTip[SelNum], 0, 0, PartyTradeToolTipRect.right, PartyTradeToolTipRect.bottom);
		}
	}

	return TRUE;
}



smCHAR	chrEachPlayer;
smCHAR *chrEachMaster = 0;
int		EachTradeButton = 0;
int		EachTradeButtonMode = 0;
RECT	RectTradeButton;

POINT3D EachCameraPos;
int	DispEachMode = 0;



int FindBipPoint(smPAT3D *lpPat, char *szObjNodeName, int frame, int *nX, int *nY, int *nZ);


int OpenEachPlayer(smCHAR *lpChar)
{
	// SUMMIT_OPEN_EACH_TRACE_V14
	SummitCharacterSelectionTrace("EACH",
		"OPEN attempt char=%p flag=%d display=%d loading=%d pattern=%p serial=%lu trade=%d master=%p eachFlag=%d eachPattern=%p",
		(void *)lpChar,
		lpChar ? lpChar->Flag : -1,
		lpChar ? lpChar->DisplayFlag : -1,
		lpChar ? lpChar->PatLoading : -1,
		lpChar ? (void *)lpChar->Pattern : NULL,
		lpChar ? static_cast<unsigned long>(lpChar->dwObjectSerial) : 0ul,
		EachTradeButton, (void *)chrEachMaster, chrEachPlayer.Flag, (void *)chrEachPlayer.Pattern);

	if(!lpChar)
	{
		SummitCharacterSelectionTrace("EACH", "OPEN reject reason=null");
		return FALSE;
	}
	if(!lpChar->Flag)
	{
		SummitCharacterSelectionTrace("EACH", "OPEN reject reason=flag0 char=%p", (void *)lpChar);
		return FALSE;
	}
	if(!lpChar->DisplayFlag)
	{
		SummitCharacterSelectionTrace("EACH",
			"OPEN reject reason=display0 char=%p loading=%d pattern=%p",
			(void *)lpChar, lpChar->PatLoading, (void *)lpChar->Pattern);
		return FALSE;
	}
	if(EachTradeButton)
	{
		SummitCharacterSelectionTrace("EACH", "OPEN reject reason=trade_button char=%p", (void *)lpChar);
		return FALSE;
	}

	memcpy(&chrEachPlayer, lpChar, sizeof(smCHAR));
	chrEachPlayer.SetMotionFromCode(CHRMOTION_STATE_STAND);
	chrEachPlayer.ActionPattern = 101;
	chrEachMaster = lpChar;
	chrEachPlayer.Flag = TRUE;

	// SUMMIT_EACH_CAMERA_STABLE_LATCH_V15_2
	// Async Pattern publish can expose the new model before Bip01 Head
	// is ready to write coordinates. Probe with impossible sentinels;
	// on a transient miss, keep the last real head camera for the same height.
	const int summitFallbackCameraX = 0;
	const int summitFallbackCameraY = chrEachPlayer.PatHeight - 10 * fONE;
	const int summitFallbackCameraZ = 0;

	static bool summitHaveLastGoodEachCamera = false;
	static int summitLastGoodEachCameraX = 0;
	static int summitLastGoodEachCameraY = 0;
	static int summitLastGoodEachCameraZ = 0;
	static int summitLastGoodEachCameraPatHeight = 0;

	// These values are deliberately outside the accepted camera range.
	// If FindBipPoint leaves any component untouched, validation below catches it.
	const int summitCameraProbeX = 0x3F3F3F3F;
	const int summitCameraProbeY = 0x3E3E3E3E;
	const int summitCameraProbeZ = 0x3D3D3D3D;

	EachCameraPos.x = summitCameraProbeX;
	EachCameraPos.y = summitCameraProbeY;
	EachCameraPos.z = summitCameraProbeZ;

	FindBipPoint(chrEachPlayer.AnimPattern, "Bip01 Head", chrEachPlayer.frame,
		&EachCameraPos.x, &EachCameraPos.y, &EachCameraPos.z);

	const long long summitCameraLimit = 134217728LL;
	const bool summitInvalidEachCamera =
		static_cast<long long>(EachCameraPos.x) < -summitCameraLimit ||
		static_cast<long long>(EachCameraPos.x) >  summitCameraLimit ||
		static_cast<long long>(EachCameraPos.y) < -summitCameraLimit ||
		static_cast<long long>(EachCameraPos.y) >  summitCameraLimit ||
		static_cast<long long>(EachCameraPos.z) < -summitCameraLimit ||
		static_cast<long long>(EachCameraPos.z) >  summitCameraLimit;

	if(!summitInvalidEachCamera)
	{
		summitHaveLastGoodEachCamera = true;
		summitLastGoodEachCameraX = EachCameraPos.x;
		summitLastGoodEachCameraY = EachCameraPos.y;
		summitLastGoodEachCameraZ = EachCameraPos.z;
		summitLastGoodEachCameraPatHeight = chrEachPlayer.PatHeight;
		SummitCharacterSelectionTrace("EACH",
			"CAMERA real_after_bip x=%d y=%d z=%d height=%d",
			EachCameraPos.x, EachCameraPos.y, EachCameraPos.z, chrEachPlayer.PatHeight);
	}
	else if(summitHaveLastGoodEachCamera &&
		summitLastGoodEachCameraPatHeight == chrEachPlayer.PatHeight)
	{
		SummitCharacterSelectionTrace("EACH",
			"CAMERA transient_after_bip probe=%d,%d,%d -> reuse_last_good=%d,%d,%d height=%d",
			EachCameraPos.x, EachCameraPos.y, EachCameraPos.z,
			summitLastGoodEachCameraX, summitLastGoodEachCameraY, summitLastGoodEachCameraZ,
			chrEachPlayer.PatHeight);

		EachCameraPos.x = summitLastGoodEachCameraX;
		EachCameraPos.y = summitLastGoodEachCameraY;
		EachCameraPos.z = summitLastGoodEachCameraZ;
	}
	else
	{
		SummitCharacterSelectionTrace("EACH",
			"CAMERA invalid_after_bip probe=%d,%d,%d -> fallback=%d,%d,%d height=%d",
			EachCameraPos.x, EachCameraPos.y, EachCameraPos.z,
			summitFallbackCameraX, summitFallbackCameraY, summitFallbackCameraZ,
			chrEachPlayer.PatHeight);

		EachCameraPos.x = summitFallbackCameraX;
		EachCameraPos.y = summitFallbackCameraY;
		EachCameraPos.z = summitFallbackCameraZ;
	}

	if(lpChar->smCharInfo.State == smCHAR_STATE_ENEMY || lpChar->smCharInfo.State == smCHAR_STATE_NPC)
	{
		EachCameraPos.y += lpChar->smCharInfo.ArrowPosi[0] * fONE;
		EachCameraPos.z += lpChar->smCharInfo.ArrowPosi[1] * fONE;
	}

	SummitCharacterSelectionTrace("EACH",
		"OPEN success master=%p serial=%lu pattern=%p eachPattern=%p camera=%d,%d,%d",
		(void *)chrEachMaster,
		static_cast<unsigned long>(chrEachMaster ? chrEachMaster->dwObjectSerial : 0),
		chrEachMaster ? (void *)chrEachMaster->Pattern : NULL,
		(void *)chrEachPlayer.Pattern,
		EachCameraPos.x, EachCameraPos.y, EachCameraPos.z);
	return TRUE;
}


int CloseEachPlayer()
{
	// SUMMIT_CLOSE_EACH_TRACE_V14
	SummitCharacterSelectionTrace("EACH",
		"CLOSE master=%p masterFlag=%d masterDisplay=%d masterLoading=%d masterPattern=%p masterSerial=%lu eachFlag=%d eachPattern=%p eachSerial=%lu",
		(void *)chrEachMaster,
		chrEachMaster ? chrEachMaster->Flag : -1,
		chrEachMaster ? chrEachMaster->DisplayFlag : -1,
		chrEachMaster ? chrEachMaster->PatLoading : -1,
		chrEachMaster ? (void *)chrEachMaster->Pattern : NULL,
		chrEachMaster ? static_cast<unsigned long>(chrEachMaster->dwObjectSerial) : 0ul,
		chrEachPlayer.Flag, (void *)chrEachPlayer.Pattern,
		static_cast<unsigned long>(chrEachPlayer.dwObjectSerial));

	chrEachPlayer.Flag = 0;
	chrEachMaster = 0;
	EachTradeButton = 0;

	return TRUE;
}


int EachPlayerMain()
{
	// SUMMIT_FACE_PREVIEW_ASYNC_REBIND_V12B2
	// Character face/body patterns are streamed asynchronously. If the selected
	// master publishes a new Pattern, refresh the copied central preview instead
	// of allowing the legacy mismatch guard below to close it.
	if(chrEachMaster && chrEachMaster->Flag &&
		chrEachMaster->dwObjectSerial == chrEachPlayer.dwObjectSerial &&
		chrEachMaster->Pattern != chrEachPlayer.Pattern && !EachTradeButton)
	{
		smCHAR *changedMaster = chrEachMaster;
		OpenEachPlayer(changedMaster);
		return TRUE;
	}

	// SUMMIT_EACH_MAIN_CLOSE_REASON_V14
	if(!chrEachMaster || !chrEachMaster->Flag ||
		chrEachMaster->dwObjectSerial != chrEachPlayer.dwObjectSerial ||
		chrEachMaster->Pattern != chrEachPlayer.Pattern)
	{
		SummitCharacterSelectionTrace("EACH",
			"MAIN close_reason master=%p masterFlag=%d masterDisplay=%d masterLoading=%d masterPattern=%p masterSerial=%lu eachFlag=%d eachPattern=%p eachSerial=%lu serialMismatch=%d patternMismatch=%d",
			(void *)chrEachMaster,
			chrEachMaster ? chrEachMaster->Flag : -1,
			chrEachMaster ? chrEachMaster->DisplayFlag : -1,
			chrEachMaster ? chrEachMaster->PatLoading : -1,
			chrEachMaster ? (void *)chrEachMaster->Pattern : NULL,
			chrEachMaster ? static_cast<unsigned long>(chrEachMaster->dwObjectSerial) : 0ul,
			chrEachPlayer.Flag, (void *)chrEachPlayer.Pattern,
			static_cast<unsigned long>(chrEachPlayer.dwObjectSerial),
			chrEachMaster ? (chrEachMaster->dwObjectSerial != chrEachPlayer.dwObjectSerial) : -1,
			chrEachMaster ? (chrEachMaster->Pattern != chrEachPlayer.Pattern) : -1);
		CloseEachPlayer();
		return TRUE;
	}

	if(chrEachPlayer.Flag)
	{



		if(DispEachMode)
		{
			if(chrEachPlayer.TalkSoundSum && chrEachPlayer.TalkSoundSum != g_PlayTalkSoundSum)
			{
				PlayTalkSound();
				chrEachPlayer.AutoChangeTalkMotion(TALK_MOTION_FILE);
			}
			else if(chrEachPlayer.TalkPattern && chrEachPlayer.TalkFrame == 0)
			{
				if(IsPlayTalkSound())
					chrEachPlayer.AutoChangeTalkMotion(TALK_MOTION_FILE);
				else
					chrEachPlayer.AutoChangeTalkMotion(FACIAL_MOTION_FILE);
			}
		}
		else
		{
			if(IsPlayTalkSound())
				StopTalkSound();

			chrEachPlayer.TalkSoundSum = 0;
			g_PlayTalkSoundSum = 0;
		}


		chrEachPlayer.Main();


		if(chrEachMaster && chrEachMaster->smCharInfo.State == smCHAR_STATE_NPC)
		{
			if(chrEachMaster->MotionInfo->State == CHRMOTION_STATE_ATTACK && chrEachPlayer.MotionInfo->State == CHRMOTION_STATE_STAND)
				chrEachPlayer.SetMotionFromCode(CHRMOTION_STATE_ATTACK);
		}

		if(chrEachMaster->MotionInfo->State == CHRMOTION_STATE_DEAD && chrEachMaster->FrameCounter > 0x50)
			CloseEachPlayer();

	}

	return TRUE;
}

int DrawClanMark_EachBox(LPDIRECT3DTEXTURE9 clanMark, int x, int y)
{
	RECT ddRect;

	ddRect.left = 0;
	ddRect.right = 16;
	ddRect.top = 0;
	ddRect.bottom = 16;

	if(clanMark)
	   DrawSprite(x, y, clanMark, 0, 0, 16, 16, true);

	return TRUE;
}

#include "systems/ui/targeting/cTarget.h"

int DrawEachPlayer(float fx, float fy, int SizeMode)
{
	// SUMMIT_DRAW_EACH_STATE_TRACE_V14
	{
		static smCHAR *s_lastDrawMaster = reinterpret_cast<smCHAR *>(~static_cast<std::uintptr_t>(0));
		static int s_lastDrawMasterFlag = -10000;
		static int s_lastDrawEachFlag = -10000;
		static smPAT3D *s_lastDrawPattern = reinterpret_cast<smPAT3D *>(~static_cast<std::uintptr_t>(0));
		if(s_lastDrawMaster != chrEachMaster ||
			s_lastDrawMasterFlag != (chrEachMaster ? chrEachMaster->Flag : -1) ||
			s_lastDrawEachFlag != chrEachPlayer.Flag ||
			s_lastDrawPattern != chrEachPlayer.Pattern)
		{
			SummitCharacterSelectionTrace("EACH",
				"DRAW state master=%p masterFlag=%d masterDisplay=%d masterLoading=%d masterPattern=%p eachFlag=%d eachPattern=%p sizeMode=%d fx=%.3f fy=%.3f",
				(void *)chrEachMaster,
				chrEachMaster ? chrEachMaster->Flag : -1,
				chrEachMaster ? chrEachMaster->DisplayFlag : -1,
				chrEachMaster ? chrEachMaster->PatLoading : -1,
				chrEachMaster ? (void *)chrEachMaster->Pattern : NULL,
				chrEachPlayer.Flag, (void *)chrEachPlayer.Pattern,
				SizeMode, fx, fy);
			s_lastDrawMaster = chrEachMaster;
			s_lastDrawMasterFlag = chrEachMaster ? chrEachMaster->Flag : -1;
			s_lastDrawEachFlag = chrEachPlayer.Flag;
			s_lastDrawPattern = chrEachPlayer.Pattern;
		}
	}

	int x, y, z, w, h;
	int mx, my, he;
	POINT3D angle;

	int dist, len;
	HDC	hdc = NULL;
	DWORD	dwColor;
	int		ObjSizeBack;
	int		ModeFlag;
	int		ClanX, ClanY, ClanLen;

	if (chrEachPlayer.Flag && chrEachMaster && chrEachMaster->Flag)
	{
		chrEachPlayer.pX = 0;
		chrEachPlayer.pY = 0;
		chrEachPlayer.pZ = 0;

		chrEachPlayer.Angle.x = 0;
		chrEachPlayer.Angle.y = 0;
		chrEachPlayer.Angle.z = 0;



		if (!SizeMode)
		{
			cTarget.Draw(chrEachMaster);
		}
		else
		{
			mx = MidX;
			my = MidY;

			MidX = (int)((float)WinSizeX * fx);
			MidY = (int)((float)WinSizeY * fy);

			int chrHeight;

			if (chrEachPlayer.smCharInfo.State <= TRUE)
				chrHeight = chrEachPlayer.PatHeight - 8 * fONE;
			else
				chrHeight = chrEachPlayer.PatHeight;

			h = (int)((float)WinSizeY / 2);
			w = (int)((float)WinSizeY * 1.34f / 4);

			w += 20;
			h += 20;

			smRender.SMSHIFT_PERSPECTIVE_WIDTH = RENDCLIP_DEFAULT_SHIFT_PERSPECTIVE_WIDTH * 3;
			smRender.SMMULT_PERSPECTIVE_HEIGHT = (int)(RENDCLIP_DEFAULT_MULT_PERSPECTIVE_HEIGHT + 25.6f) * 2;

			dist = 30 * fONE;
			he = -1 * fONE;

			dist += EachCameraPos.y;


			if (MatEachBox >= 0)
			{

				dsDrawTexImage(MatEachBox, MidX - (w >> 1), MidY - (h >> 1), w, h, 255, 0);//Background da animação do npc

				if (!SizeMode)
				{
					if (EachTradeButton)
					{

#ifdef	_NEW_PARTRADE


						PartyButtonPoint.y = MidY + (h >> 1);

						if (EachTradeButtonMode)
						{
							dsDrawTexImage(MatEachMenuBox, MidX - 60, MidY + (h >> 1) - 6, 120, 50, 255, 0);
							PartyButtonPoint.x = MidX - 46;
						}
						else
						{
							dsDrawTexImage(MatEachMenuBox, MidX - 50, MidY + (h >> 1) - 6, 100, 50, 255, 0);
							PartyButtonPoint.x = MidX - 36;
						}




#else

						dsDrawTexImage(MatEachMenuBox, MidX - 50, MidY + 56, 100, 60, 255, 0);

						RectTradeButton.left = MidX - 32;
						RectTradeButton.right = MidX + 32;
						RectTradeButton.top = MidY + 62;
						RectTradeButton.bottom = MidY + 62 + 32;

						if (pCursorPos.x > RectTradeButton.left && pCursorPos.x<RectTradeButton.right &&
							pCursorPos.y>RectTradeButton.top && pCursorPos.y < RectTradeButton.bottom)
						{
							dsDrawTexImage(MatEachTradeButton[1], MidX - 32, MidY + 62, 64, 32, 255, 0);
							EachTradeButton = 2;
						}
						else
						{
							dsDrawTexImage(MatEachTradeButton[0], MidX - 32, MidY + 62, 64, 32, 255, 0);
							EachTradeButton = 1;
						}
#endif
					}
					else
					{
					}

				}

			}

			d3ddev->Clear(0, 0, 2, smBackColor, 1.0f, 0);

			smCHAR_SetCameraPosi(EachCameraPos.x, EachCameraPos.y + he, dist + EachCameraPos.z, 0, ANGLE_180, 0);

			if (chrEachMaster->MotionInfo->State >= 0x100)
				ModeFlag = TRUE;
			else
				ModeFlag = FALSE;


			if (ModeFlag && chrEachMaster->smCharInfo.State == smCHAR_STATE_NPC && chrEachPlayer.MotionInfo->State == CHRMOTION_STATE_ATTACK)
				ModeFlag = FALSE;


			if (ModeFlag)
			{
				x = chrEachMaster->pX;
				y = chrEachMaster->pY;
				z = chrEachMaster->pZ;
				memcpy(&angle, &chrEachMaster->Angle, sizeof(POINT3D));

				chrEachMaster->pX = 0;
				chrEachMaster->pY = 0;
				chrEachMaster->pZ = 0;
				ZeroMemory(&chrEachMaster->Angle, sizeof(POINT3D));


				ObjSizeBack = chrEachMaster->smCharInfo.wPlayClass[1];
				chrEachMaster->smCharInfo.wPlayClass[1] = 0;

				chrEachMaster->Draw();

				if (ObjSizeBack)
					chrEachMaster->smCharInfo.wPlayClass[1] = ObjSizeBack;

				if (!SizeMode && chrEachMaster->smCharInfo.State && !EachTradeButton)
				{

					chrEachMaster->DrawStateBar(MidX - (w >> 1) + 5, MidY - (h >> 1) + h + 2);

				}

				chrEachMaster->pX = x;
				chrEachMaster->pY = y;
				chrEachMaster->pZ = z;

				memcpy(&chrEachMaster->Angle, &angle, sizeof(POINT3D));
			}
			else
			{
				chrEachPlayer.AttackAnger = chrEachMaster->AttackAnger;

				ObjSizeBack = chrEachPlayer.smCharInfo.wPlayClass[1];
				chrEachPlayer.smCharInfo.wPlayClass[1] = 0;

				chrEachPlayer.Draw();

				chrEachPlayer.smCharInfo.wPlayClass[1] = ObjSizeBack;

				if (!SizeMode && chrEachMaster->smCharInfo.State)
				{

					chrEachMaster->DrawStateBar(MidX - (w >> 1) + 5, MidY - (h >> 1) + h + 2);

				}
			}

			if (SizeMode)
			{

				if (chrEachPlayer.szChatMessage[0])
				{
					dist = lstrlen(chrEachPlayer.szChatMessage);
					if (dist > 34) dist = 34;
					dist >>= 1;
					dist *= 8;
					dist -= 8;

					DrawMessage(MidX - dist, MidY - (h >> 1) + h + 8, chrEachPlayer.szChatMessage, 36, BOX_ONE);
				}
				else
				{
					if (chrEachMaster->szChatMessage[0])
						strcpy_s(chrEachPlayer.szChatMessage, chrEachMaster->szChatMessage);
				}


				len = lstrlen(chrEachPlayer.smCharInfo.szName);
				dist = len >> 1;
				dist *= 8;
				dist -= 8;

				SIZE sizecheck;
				sizecheck.cx = 0;
				sizecheck.cy = 0;
				GetFontTextExtentPoint(chrEachPlayer.smCharInfo.szName, lstrlen(chrEachPlayer.smCharInfo.szName), &sizecheck);
				dist = sizecheck.cx >> 1;

				SetFontTextColor32(RGB(0, 0, 0));
				dsTextLineOut32(MidX - dist + 1, MidY - (h >> 1) + h - 33, chrEachPlayer.smCharInfo.szName, len);
				SetFontTextColor32(RGB(255, 255, 180));
				dsTextLineOut32(MidX - dist, MidY - (h >> 1) + h - 32, chrEachPlayer.smCharInfo.szName, len);


			}
			else
			{
				if (!EachTradeButton)
				{
					if (lpCurPlayer->OnStageField >= 0 &&
						StageField[lpCurPlayer->OnStageField]->State == FIELD_STATE_CASTLE)
					{
						if (chrEachMaster->smCharInfo.State == smCHAR_STATE_USER || chrEachMaster->PkMode_CharState == smCHAR_STATE_USER)
							chaSiege.ShowPickUserKillPoint(MidX - 32, MidY + (h >> 1) + 10, chrEachMaster->sBlessCastle_Damage[1]);
					}
				}

				if (chrEachPlayer.smCharInfo.ClassClan && chrEachPlayer.ClanInfoNum >= 0)
				{
					ClanLen = lstrlen(ClanInfo[chrEachPlayer.ClanInfoNum].ClanInfoHeader.ClanName);

					ClanX = MidX - 26;
					ClanY = MidY - (h >> 1) + 8;

					DrawClanMark_EachBox(ClanInfo[chrEachPlayer.ClanInfoNum].hClanMark, ClanX - 18, ClanY);

					if (ClanLen < 12)
						ClanX += (12 - ClanLen) << 1;

					ClanY += 2;

					if (pCursorPos.x > (MidX - (w >> 1)) && pCursorPos.y < (MidY + (h >> 1)) && ClanInfo[chrEachPlayer.ClanInfoNum].ClanInfoHeader.ClanReport[0])
						DrawClanMessage((MidX - (w >> 1)), 14, ClanInfo[chrEachPlayer.ClanInfoNum].ClanInfoHeader.ClanReport, 42, RGB(255, 255, 255));
				}
				else
					ClanLen = 0;

				if (chrEachPlayer.smCharInfo.State == smCHAR_STATE_ENEMY && !chrEachPlayer.smCharInfo.ClassClan)
				{
					len = 0;
					switch (chrEachPlayer.smCharInfo.Brood)
					{
					case smCHAR_MONSTER_MUTANT:
						len = 1;
						break;
					case smCHAR_MONSTER_UNDEAD:
						len = 2;
						break;
					case smCHAR_MONSTER_DEMON:
						len = 3;
						break;
					case smCHAR_MONSTER_MECHANIC:
						len = 4;
						break;

					case smCHAR_MONSTER_USER:
						len = -1;
						break;
					}
					if (len >= 0)
						dsDrawTexImage(MatMonBroodImage[len], MidX + (w / 2) - 24, MidY - (h / 2) + 8, 16, 16, 255, 0);
				}


				len = lstrlen(chrEachPlayer.smCharInfo.szName);
				dist = len >> 1;
				dist *= 8;
				dist -= 8;

				SIZE sizecheck;
				sizecheck.cx = 0;
				sizecheck.cy = 0;
				GetFontTextExtentPoint(chrEachPlayer.smCharInfo.szName, lstrlen(chrEachPlayer.smCharInfo.szName), &sizecheck);
				dist = sizecheck.cx >> 1;

				SetFontTextColor32(RGB(0, 0, 0));
				dsTextLineOut32(MidX - dist + 1, MidY - (h >> 1) + h - 17, chrEachPlayer.smCharInfo.szName, len);

				dwColor = RGB(255, 255, 255);

				switch (chrEachPlayer.smCharInfo.State)
				{
				case TRUE:
					dwColor = RGB(255, 160, 120);
					break;
				case FALSE:
					dwColor = RGB(255, 255, 180);
					break;
				}

				SetFontTextColor32(dwColor);

				dsTextLineOut32(MidX - dist, MidY - (h >> 1) + h - 16, chrEachPlayer.smCharInfo.szName, len);

				if (ClanLen)
				{
					SetFontTextColor32((0, 0, 0));
					dsTextLineOut32(ClanX + 1, ClanY + 1, ClanInfo[chrEachPlayer.ClanInfoNum].ClanInfoHeader.ClanName, ClanLen);
					SetFontTextColor32((150, 255, 200));
					dsTextLineOut32(ClanX, ClanY, ClanInfo[chrEachPlayer.ClanInfoNum].ClanInfoHeader.ClanName, ClanLen);
				}

				if (smConfig.getSecurity() >= AccountTypes::SEC_MODERATOR && cInterFace.sInterFlags.MapOnFlag)
				{
					dwColor = RGB(255, 255, 255);
					SetFontTextColor32(dwColor);

#ifdef GAME_GM
					char szBuff[256];

					int ly = SETTINGSHANDLER->getHeight() / 5 + 15;

					wsprintf(szBuff, "N\355vel: %d", chrEachPlayer.smCharInfo.Level);


					dsTextLineOut32(MidX - 50, ly, szBuff, lstrlen(szBuff));
					ly += 16;
					wsprintf(szBuff, "Ataque: %d-%d", chrEachPlayer.smCharInfo.Attack_Damage[0], chrEachPlayer.smCharInfo.Attack_Damage[1]);
					dsTextLineOut32(MidX - 50, ly, szBuff, lstrlen(szBuff));
					ly += 16;
					wsprintf(szBuff, "Defesa: %d", chrEachPlayer.smCharInfo.Defence);
					dsTextLineOut32(MidX - 50, ly, szBuff, lstrlen(szBuff));
					ly += 16;
					wsprintf(szBuff, "Absor\347\343o: %d", chrEachPlayer.smCharInfo.Absorption);
					dsTextLineOut32(MidX - 50, ly, szBuff, lstrlen(szBuff));
					ly += 16;
					wsprintf(szBuff, "Dinheiro: %d", chrEachPlayer.smCharInfo.Money);
					dsTextLineOut32(MidX - 50, ly, szBuff, lstrlen(szBuff));
					ly += 16;
					wsprintf(szBuff, "EXP: %d", chrEachPlayer.smCharInfo.Exp);
					dsTextLineOut32(MidX - 50, ly, szBuff, lstrlen(szBuff));
					ly += 16;
					wsprintf(szBuff, "Precis\343o: %d", sinGetAccuracy(chrEachPlayer.smCharInfo.Level, chrEachPlayer.smCharInfo.Defence));
					dsTextLineOut32(MidX - 50, ly, szBuff, lstrlen(szBuff));
					ly += 16;
					wsprintf(szBuff, "Cr\355tico: %d", sinGetCritical(chrEachPlayer.smCharInfo.Level));
					dsTextLineOut32(MidX - 50, ly, szBuff, lstrlen(szBuff));
					ly += 16;
					wsprintf(szBuff, "Atualiza\347\343o: %d", chrEachPlayer.smCharInfo.bUpdateInfo[0]);
					dsTextLineOut32(MidX - 50, ly, szBuff, lstrlen(szBuff));
					ly += 16;
					wsprintf(szBuff, "Cl\343: %d(%d)", chrEachPlayer.smCharInfo.ClassClan, chrEachMaster->ClanInfoNum);
					dsTextLineOut32(MidX - 50, ly, szBuff, lstrlen(szBuff));
					ly += 16;

					if (DisplayDebug)
					{

						wsprintf(szBuff, "Frame   :%d", chrEachMaster->frame);
						dsTextLineOut32(MidX - 100, ly, szBuff, lstrlen(szBuff));
						ly += 16;

						wsprintf(szBuff, "RcvMoton:%d,%d", (chrEachMaster->PlayBuffCnt & PLAYBUFF_MASK), (chrEachMaster->PlayBuffPosi_End & PLAYBUFF_MASK));
						dsTextLineOut32(MidX - 100, ly, szBuff, lstrlen(szBuff));
						ly += 16;

						wsprintf(szBuff, "User Code :%d", chrEachMaster->dwObjectSerial);
						dsTextLineOut32(MidX - 100, ly, szBuff, lstrlen(szBuff));
						ly += 16;
					}
#endif
				}

			}



			MidX = mx;
			MidY = my;

			smRender.SMSHIFT_PERSPECTIVE_WIDTH = 0;
			smRender.SMMULT_PERSPECTIVE_HEIGHT = RENDCLIP_DEFAULT_MULT_PERSPECTIVE_HEIGHT;
		}
	}

	if (chrEachMaster && chrEachMaster->ActionPattern == 99)
	{
		if ((chrEachMaster->dwLastTransTime + DISPLAY_TRANS_TIME_OVER) < dwPlayTime)
		{
			CloseEachPlayer();
			return TRUE;
		}
	}

	if (lpCharMsTrace)
	{
		if (chrEachMaster != lpCharMsTrace)
		{
			EachTradeButton = 0;
			OpenEachPlayer(lpCharMsTrace);
			DispEachMode = 0;
		}
	}
	else
	{
		if (lpCharSelPlayer && (chrEachMaster != lpCharSelPlayer) && (chrEachMaster != &chrPartyPlayer) && !DispEachMode)
		{
			OpenEachPlayer(lpCharSelPlayer);
			DispEachMode = 0;
		}
	}

	if (EachTradeButton && chrEachMaster)
	{

		if (GetTradeDistance(chrEachMaster) == FALSE)
			EachTradeButton = 0;
	}

	return FALSE;
}


int FindBipPoint(smPAT3D *lpPat, char *szObjNodeName, int frame, int *nX, int *nY, int *nZ)
{
	smOBJ3D	*lpObj = lpPat->GetObjectFromName(szObjNodeName);
	if(!lpObj) return FALSE;


	smMATRIX *mWorld;
	POINT3D angle;

	ZeroMemory(&angle, sizeof(POINT3D));
	angle.y = (-angle.y + ANGLE_180)&ANGCLIP;

	mWorld = &lpObj->mWorld;

	AnimObjectTree(lpObj, frame, angle.x, angle.y, angle.z);

	*nX = mWorld->_41;
	*nZ = mWorld->_42;
	*nY = mWorld->_43;

	return TRUE;
}


int Record_ClinetLogFile(char *szMessage)
{
	HANDLE	hFile;
	DWORD	dwAcess;
	DWORD	FileLength;
	SYSTEMTIME	st;
	int len;
	char	szFileName[128];
	char	szBuff[256];

	GetLocalTime(&st);
	wsprintf(szFileName, "game.log", st.wMonth, st.wDay);

	hFile = CreateFile("game.log", GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE) return FALSE;

	wsprintf(szBuff, "%d-%d (%d:%d:%d) - %s", st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, szMessage);
	len = lstrlen(szBuff);
	if(szBuff[len - 1] != '\n')
	{
		szBuff[len] = '\r';
		szBuff[len + 1] = '\n';
		len += 2;
	}

	FileLength = GetFileSize(hFile, NULL);
	SetFilePointer(hFile, FileLength, NULL, FILE_BEGIN);
	WriteFile(hFile, szBuff, len, &dwAcess, NULL);
	CloseHandle(hFile);

	return TRUE;
}


#include "systems/media/video/ParkDXPlayer.h"

int ParkPlayInit = 0;
int ParkPlayMode = 0;
int ParkPlaying = 0;
RECT ParkPlayRect;


int Init_ParkPlayer()
{

	ParkPlayInit = TRUE;
	ParkDXPlay.InitPlayer();
	ParkPlayMode = 0;
	ParkPlaying = 0;
	ParkPlayRect.left = 0;
	ParkPlayRect.top = 0;
	ParkPlayRect.right = WinSizeX;
	ParkPlayRect.bottom = WinSizeY;

	return TRUE;



}


int Play_ParkPlayer(char *szPath, int x, int y, int w, int h, int PlayMode)
{
	if(ParkPlaying)
		Stop_ParkPlayer();

	Init_ParkPlayer();

	if(ParkDXPlay.LoadPlayerFile(hwnd, szPath))
	{
		ParkDXPlay.PlaySize(x, y, w, h);
		ParkDXPlay.PlayPlayer();
		ParkPlayMode = PlayMode;
		ParkPlaying = TRUE;

		ParkPlayRect.left = x;
		ParkPlayRect.top = y;
		ParkPlayRect.right = x + w;
		ParkPlayRect.bottom = y + h;

		return TRUE;
	}

	return FALSE;
}


int Stop_ParkPlayer()
{
	ParkDXPlay.StopPlayer();
	ParkPlayMode = 0;
	ParkPlaying = FALSE;

	return TRUE;
}





