#include "systems/gameplay/character/core/CharacterModelStreamingV3.h"

#include <algorithm>
#include <atomic>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <memory>
#include <mutex>
#include <new>
#include <string>
#include <unordered_map>
#include <vector>

#include "opcodes.h"
#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/network/transport/legacy/smwsock.h"
#include "systems/gameplay/character/core/character.h"
#include "core/data/legacy_loader/fileread.h"

#include "core/concurrency/JobSystem.h"
#include "core/concurrency/SourceAssetLock.h"
#include "systems/rendering/legacy_3d/smRead3d.h"

extern smPATTERN smPattern;
extern smPATTERN smBipPattern;
extern int smDPAT_Delete(smDPAT *dPat);
extern int AddModelDecode(char *iniFile, smMODELINFO *Info);
extern char *GetFaceHairSkin(smCHAR *lpChar);
extern char *GetDefaultFaceHairSkin(smCHAR *lpChar);
extern int AssetLoadV2PrewarmMaterialGroupStep(smMATERIAL_GROUP *group, DWORD ownerMask,
    int MipMap, std::uint32_t maxAttempts, int requestPriority);

namespace
{

enum SummitModelTargetKind
{
    SUMMIT_MODEL_TARGET_CHARACTER,
    SUMMIT_MODEL_TARGET_ITEM,
    SUMMIT_MODEL_TARGET_TOOL
};

struct SummitPreparedModel
{
    SummitPreparedModel()
        : body(0), motion(0), talk(0), talkSharesMotion(false), success(false)
    {
        ZeroMemory(&modelInfo, sizeof(modelInfo));
    }

    std::string sourceName;
    smMODELINFO modelInfo;
    smPAT3D *body;
    smPAT3D *motion;
    smPAT3D *talk;
    bool talkSharesMotion;
    bool success;
};

struct SummitModelTarget
{
    SummitModelTarget()
        : kind(SUMMIT_MODEL_TARGET_CHARACTER), character(0), item(0), generation(0),
          itemCode(0), toolPosition(0)
    {
    }

    SummitModelTargetKind kind;
    smCHAR *character;
    scITEM *item;
    std::uint64_t generation;
    DWORD itemCode;
    int toolPosition;
};

struct SummitModelRequest
{
    SummitModelRequest()
        : id(0), targetKind(SUMMIT_MODEL_TARGET_CHARACTER), dataServerCompanion(false), success(false), cancelled(false)
    {
    }

    std::uint64_t id;
    SummitModelTargetKind targetKind;
    std::string key;
    std::string primaryName;
    std::string requestedSecondary;
    std::string faceHair;
    std::string defaultFaceHair;
    bool dataServerCompanion;
    std::vector<SummitModelTarget> targets;
    SummitPreparedModel primary;
    SummitPreparedModel secondary;
    bool success;
    std::atomic<bool> cancelled;
};

struct SummitModelStreamingState
{
    SummitModelStreamingState() : requestSerial(1u), shuttingDown(false) {}

    std::mutex mutex;
    std::unordered_map<smCHAR *, std::uint64_t> characterGenerations;
    std::unordered_map<scITEM *, std::uint64_t> itemGenerations;
    std::unordered_map<smCHAR *, std::uint64_t> rightToolGenerations;
    std::unordered_map<smCHAR *, std::uint64_t> leftToolGenerations;
    std::unordered_map<std::string, std::weak_ptr<SummitModelRequest> > pendingByKey;
    std::deque<std::shared_ptr<SummitModelRequest> > completed;
    std::deque<smPAT3D *> deferredPatternDeletes;
    std::atomic<std::uint64_t> requestSerial;
    std::atomic<bool> shuttingDown;
};

static SummitModelStreamingState &SummitModelState()
{
    // Intentionally process-lifetime. Legacy global smCHAR/scITEM objects can
    // call cancellation from constructors/destructors across translation units.
    static SummitModelStreamingState *state = new SummitModelStreamingState();
    return *state;
}

static std::recursive_mutex &SummitPatternMutexStorage()
{
    static std::recursive_mutex *mutex = new std::recursive_mutex();
    return *mutex;
}

static std::string SummitLower(std::string value)
{
    for (std::size_t i = 0; i < value.size(); ++i)
        value[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(value[i])));
    return value;
}

static std::uint32_t SummitModelCommitBudget()
{
    static const std::uint32_t configured = []() -> std::uint32_t
    {
        const char *value = std::getenv("SUMMIT_MODEL_COMMITS_PER_FRAME");
        if (!value || !*value)
            return 2u;
        const unsigned long parsed = std::strtoul(value, 0, 10);
        if (parsed < 1ul) return 1u;
        if (parsed > 8ul) return 8u;
        return static_cast<std::uint32_t>(parsed);
    }();
    return configured;
}

static std::uint32_t SummitModelTexturePrewarm()
{
    static const std::uint32_t configured = []() -> std::uint32_t
    {
        const char *value = std::getenv("SUMMIT_MODEL_TEXTURE_PREWARM");
        if (!value || !*value)
            return 8u;
        const unsigned long parsed = std::strtoul(value, 0, 10);
        if (parsed > 32ul) return 32u;
        return static_cast<std::uint32_t>(parsed);
    }();
    return configured;
}

static void SummitDeletePatternOnOwningThread(smPAT3D *pattern)
{
    if (!pattern)
        return;

    if (!summit::core::concurrency::JobSystem::Instance().IsWorkerThread())
    {
        delete pattern;
        return;
    }

    // smPAT3D destruction releases material/texture handles and can ultimately
    // touch D3D resources. Workers never perform that teardown; they only hand
    // ownership back to the game thread.
    std::lock_guard<std::mutex> lock(SummitModelState().mutex);
    SummitModelState().deferredPatternDeletes.push_back(pattern);
}

static void SummitDrainDeferredPatternDeletes()
{
    std::deque<smPAT3D *> pending;
    {
        std::lock_guard<std::mutex> lock(SummitModelState().mutex);
        pending.swap(SummitModelState().deferredPatternDeletes);
    }
    while (!pending.empty())
    {
        delete pending.front();
        pending.pop_front();
    }
}

static void SummitDestroyPreparedModel(SummitPreparedModel &model)
{
    if (model.body)
    {
        SummitDeletePatternOnOwningThread(model.body);
        model.body = 0;
    }
    if (model.motion)
    {
        SummitDeletePatternOnOwningThread(model.motion);
        model.motion = 0;
    }
    if (model.talk && !model.talkSharesMotion)
    {
        SummitDeletePatternOnOwningThread(model.talk);
        model.talk = 0;
    }
    model.talk = 0;
    model.success = false;
}

static bool SummitIsModelConfigSource(const std::string &name)
{
    if (name.size() < 4)
        return false;
    const std::string ext = SummitLower(name.substr(name.size() - 4));
    return ext == ".ini";
}

static bool SummitDecodeModelInfo(const std::string &source, smMODELINFO &info, bool allowRepair)
{
    summit::core::concurrency::SourceAssetLock sourceLock(source.c_str());
    ZeroMemory(&info, sizeof(info));
    if (smModelDecode(const_cast<char *>(source.c_str()), &info))
        return true;

    if (!allowRepair || !SummitIsModelConfigSource(source))
        return false;

    smMODELINFO repaired;
    ZeroMemory(&repaired, sizeof(repaired));
    if (!AddModelDecode(const_cast<char *>(source.c_str()), &repaired))
        return false;

    ZeroMemory(&info, sizeof(info));
    return smModelDecode(const_cast<char *>(source.c_str()), &info) != FALSE;
}

static bool SummitPrepareModel(const std::string &source, SummitPreparedModel &out, bool allowRepair)
{
    out.sourceName = source;
    if (source.empty())
        return false;

    if (!SummitDecodeModelInfo(source, out.modelInfo, allowRepair))
        return false;

    smPAT3D *motion = 0;
    smPAT3D *talk = 0;

    smASE_SetPhysique(0);
    if (out.modelInfo.szMotionFile[0])
    {
        {
            summit::core::concurrency::SourceAssetLock motionLock(out.modelInfo.szMotionFile);
            motion = smASE_ReadBone(out.modelInfo.szMotionFile);
        }
        if (!motion)
        {
            smASE_SetPhysique(0);
            return false;
        }
    }

    if (out.modelInfo.szTalkMotionFile[0])
    {
        if (out.modelInfo.szMotionFile[0] &&
            _stricmp(out.modelInfo.szMotionFile, out.modelInfo.szTalkMotionFile) == 0)
        {
            talk = motion;
            out.talkSharesMotion = true;
        }
        else
        {
            {
                summit::core::concurrency::SourceAssetLock talkLock(out.modelInfo.szTalkMotionFile);
                talk = smASE_ReadBone(out.modelInfo.szTalkMotionFile);
            }
            if (!talk)
            {
                if (motion) SummitDeletePatternOnOwningThread(motion);
                smASE_SetPhysique(0);
                return false;
            }
        }
    }

    smASE_SetPhysique(motion);
    smPAT3D *body = 0;
    {
        summit::core::concurrency::SourceAssetLock bodyLock(out.modelInfo.szModelFile);
        body = smASE_Read(out.modelInfo.szModelFile);
    }
    smASE_SetPhysique(0);
    if (!body)
    {
        if (talk && talk != motion) SummitDeletePatternOnOwningThread(talk);
        if (motion) SummitDeletePatternOnOwningThread(motion);
        return false;
    }

    smPAT3D *anim = motion ? motion : body;
    if (anim && anim->TmFrameCnt > 0)
    {
        for (DWORD cnt = 0; cnt < out.modelInfo.MotionCount; ++cnt)
        {
            const int frameIndex = out.modelInfo.MotionInfo[cnt].MotionFrame - 1;
            if (frameIndex < 0 || frameIndex >= anim->TmFrameCnt)
                continue;
            const int start = anim->TmFrame[frameIndex].StartFrame / 160;
            out.modelInfo.MotionInfo[cnt].StartFrame += start;
            out.modelInfo.MotionInfo[cnt].EndFrame += start;
        }
    }

    out.body = body;
    out.motion = motion;
    out.talk = talk;
    out.success = true;
    return true;
}

static std::vector<std::string> SummitSecondaryCandidates(const SummitModelRequest &request)
{
    std::vector<std::string> result;
    const std::string embedded = request.primary.modelInfo.szSubModelFile;

    const auto pushUnique = [&result](const std::string &candidate)
    {
        if (candidate.empty())
            return;
        for (std::size_t i = 0; i < result.size(); ++i)
            if (_stricmp(result[i].c_str(), candidate.c_str()) == 0)
                return;
        result.push_back(candidate);
    };

    if (request.dataServerCompanion)
    {
        pushUnique(request.requestedSecondary);
        pushUnique(embedded);
    }
    else
    {
        pushUnique(embedded);
        pushUnique(request.requestedSecondary);
    }
    pushUnique(request.faceHair);
    pushUnique(request.defaultFaceHair);
    return result;
}

static void SummitPrepareRequestOnWorker(const std::shared_ptr<SummitModelRequest> &request)
{
    if (!request || request->cancelled.load(std::memory_order_acquire) ||
        SummitModelState().shuttingDown.load(std::memory_order_acquire))
        return;

    if (!SummitPrepareModel(request->primaryName, request->primary, false))
    {
        request->success = false;
    }
    else
    {
        request->success = true;
        if (request->targetKind == SUMMIT_MODEL_TARGET_CHARACTER)
        {
            const std::vector<std::string> candidates = SummitSecondaryCandidates(*request);
            for (std::size_t i = 0; i < candidates.size(); ++i)
            {
                if (request->cancelled.load(std::memory_order_acquire))
                    break;
                if (SummitPrepareModel(candidates[i], request->secondary, true))
                    break;
                SummitDestroyPreparedModel(request->secondary);
            }
        }
    }

    if (SummitModelState().shuttingDown.load(std::memory_order_acquire))
    {
        SummitDestroyPreparedModel(request->secondary);
        SummitDestroyPreparedModel(request->primary);
        return;
    }

    std::lock_guard<std::mutex> lock(SummitModelState().mutex);
    SummitModelState().completed.push_back(request);
}

static int SummitFindExactPattern(smPATTERN &cache, const std::string &name)
{
    if (name.empty())
        return -1;
    const DWORD code = GetSpeedSum(name.c_str());
    for (int i = 0; i < DPAT_MAX; ++i)
    {
        smDPAT &entry = cache.DinaPat[i];
        if (!entry.Pat || entry.dwSpeedFindSum != code)
            continue;
        if (_stricmp(entry.szPatName, name.c_str()) == 0)
            return i;
    }
    return -1;
}

static int SummitFindFreePatternSlot(smPATTERN &cache, int avoidA, int avoidB)
{
    for (int i = 0; i < DPAT_MAX; ++i)
    {
        if (i == avoidA || i == avoidB)
            continue;
        if (!cache.DinaPat[i].Pat)
            return i;
    }
    return -1;
}

static void SummitClearPatternEntry(smDPAT &entry)
{
    ZeroMemory(&entry, sizeof(entry));
}

static smDPAT *SummitInstallPreparedModelLocked(SummitPreparedModel &prepared)
{
    if (!prepared.success || !prepared.body || prepared.sourceName.empty())
        return 0;

    const int existingIndex = SummitFindExactPattern(smPattern, prepared.sourceName);
    if (existingIndex >= 0)
    {
        smDPAT *existing = &smPattern.DinaPat[existingIndex];
        ++existing->UseCount;
        SummitDestroyPreparedModel(prepared);
        return existing;
    }

    const int bodySlot = SummitFindFreePatternSlot(smPattern, -1, -1);
    if (bodySlot < 0)
        return 0;

    int motionSlot = -1;
    int talkSlot = -1;
    if (prepared.motion)
    {
        motionSlot = SummitFindFreePatternSlot(smBipPattern, -1, -1);
        if (motionSlot < 0)
            return 0;
    }
    if (prepared.talk)
    {
        if (prepared.talkSharesMotion)
        {
            talkSlot = motionSlot;
        }
        else
        {
            talkSlot = SummitFindFreePatternSlot(smBipPattern, motionSlot, -1);
            if (talkSlot < 0)
                return 0;
        }
    }

    smDPAT *motionEntry = 0;
    smDPAT *talkEntry = 0;

    if (motionSlot >= 0)
    {
        smDPAT &entry = smBipPattern.DinaPat[motionSlot];
        SummitClearPatternEntry(entry);
        entry.Pat = prepared.motion;
        prepared.motion = 0;
        strcpy_s(entry.szPatName, sizeof(entry.szPatName), prepared.modelInfo.szMotionFile);
        entry.dwSpeedFindSum = GetSpeedSum(entry.szPatName);
        entry.UseCount = prepared.talkSharesMotion ? 2 : 1;
        motionEntry = &entry;
        if (prepared.talkSharesMotion)
        {
            prepared.talk = 0;
            talkEntry = &entry;
        }
    }

    if (talkSlot >= 0 && talkSlot != motionSlot)
    {
        smDPAT &entry = smBipPattern.DinaPat[talkSlot];
        SummitClearPatternEntry(entry);
        entry.Pat = prepared.talk;
        prepared.talk = 0;
        strcpy_s(entry.szPatName, sizeof(entry.szPatName), prepared.modelInfo.szTalkMotionFile);
        entry.dwSpeedFindSum = GetSpeedSum(entry.szPatName);
        entry.UseCount = 1;
        talkEntry = &entry;
    }

    smDPAT &body = smPattern.DinaPat[bodySlot];
    SummitClearPatternEntry(body);
    body.Pat = prepared.body;
    prepared.body = 0;
    body.smDinaLink = motionEntry;
    body.lpTalkLink = talkEntry;
    body.lpModelInfo = new(std::nothrow) smMODELINFO;
    if (!body.lpModelInfo)
    {
        if (body.Pat) { delete body.Pat; body.Pat = 0; }
        if (talkEntry && talkEntry != motionEntry) smDPAT_Delete(talkEntry);
        if (motionEntry)
        {
            // A shared talk/motion entry starts at 2 because the body releases
            // both links independently.
            while (motionEntry->UseCount > 0)
                smDPAT_Delete(motionEntry);
        }
        SummitClearPatternEntry(body);
        return 0;
    }

    memcpy(body.lpModelInfo, &prepared.modelInfo, sizeof(smMODELINFO));
    strcpy_s(body.szPatName, sizeof(body.szPatName), prepared.sourceName.c_str());
    body.dwSpeedFindSum = GetSpeedSum(body.szPatName);
    body.UseCount = 1;
    body.LastUsedTime = GetCurrentTime();
    prepared.success = false;
    return &body;
}

static bool SummitCharacterTargetCurrent(const SummitModelTarget &target)
{
    if (!target.character)
        return false;
    std::lock_guard<std::mutex> lock(SummitModelState().mutex);
    const std::unordered_map<smCHAR *, std::uint64_t>::const_iterator it =
        SummitModelState().characterGenerations.find(target.character);
    return it != SummitModelState().characterGenerations.end() && it->second == target.generation;
}

static bool SummitItemTargetCurrent(const SummitModelTarget &target)
{
    if (!target.item)
        return false;
    std::lock_guard<std::mutex> lock(SummitModelState().mutex);
    const std::unordered_map<scITEM *, std::uint64_t>::const_iterator it =
        SummitModelState().itemGenerations.find(target.item);
    return it != SummitModelState().itemGenerations.end() && it->second == target.generation;
}

static bool SummitToolTargetCurrent(const SummitModelTarget &target)
{
    if (!target.character)
        return false;
    std::lock_guard<std::mutex> lock(SummitModelState().mutex);
    const std::unordered_map<smCHAR *, std::uint64_t> &generations =
        target.toolPosition == hvPOSI_RHAND ? SummitModelState().rightToolGenerations
                                           : SummitModelState().leftToolGenerations;
    const std::unordered_map<smCHAR *, std::uint64_t>::const_iterator it = generations.find(target.character);
    return it != generations.end() && it->second == target.generation;
}

static void SummitPrewarmPatternTextures(smDPAT *pattern)
{
    if (!pattern || !pattern->Pat || !pattern->Pat->smMaterialGroup)
        return;
    AssetLoadV2PrewarmMaterialGroupStep(pattern->Pat->smMaterialGroup,
        SMASSET_OWNER_CHARACTER, 1, SummitModelTexturePrewarm(), 1);
}

static void SummitCompleteRequestOnMain(const std::shared_ptr<SummitModelRequest> &request)
{
    if (!request)
        return;

    // Freeze the subscriber list at commit time while removing the request from
    // the coalescing table.  AddLoaderPattern/AddLoaderItem may append targets
    // from the game thread while a worker is parsing, so reading targets without
    // this lock would be a vector data race.
    std::vector<SummitModelTarget> targets;
    {
        std::lock_guard<std::mutex> lock(SummitModelState().mutex);
        std::unordered_map<std::string, std::weak_ptr<SummitModelRequest> >::iterator it =
            SummitModelState().pendingByKey.find(request->key);
        if (it != SummitModelState().pendingByKey.end())
        {
            std::shared_ptr<SummitModelRequest> current = it->second.lock();
            if (!current || current->id == request->id)
                SummitModelState().pendingByKey.erase(it);
        }
        targets = request->targets;
    }

    if (request->cancelled.load(std::memory_order_acquire) || !request->success)
    {
        for (std::size_t i = 0; i < targets.size(); ++i)
        {
            const SummitModelTarget &target = targets[i];
            if (target.kind == SUMMIT_MODEL_TARGET_CHARACTER && SummitCharacterTargetCurrent(target))
                target.character->PatLoading = FALSE;
            if (target.kind == SUMMIT_MODEL_TARGET_ITEM && SummitItemTargetCurrent(target))
                target.item->PatLoading = FALSE;
        }
        SummitDestroyPreparedModel(request->secondary);
        SummitDestroyPreparedModel(request->primary);
        return;
    }

    std::vector<SummitModelTarget> valid;
    valid.reserve(targets.size());
    for (std::size_t i = 0; i < targets.size(); ++i)
    {
        const SummitModelTarget &target = targets[i];
        if (target.kind == SUMMIT_MODEL_TARGET_CHARACTER)
        {
            if (SummitCharacterTargetCurrent(target)) valid.push_back(target);
        }
        else if (target.kind == SUMMIT_MODEL_TARGET_ITEM)
        {
            if (SummitItemTargetCurrent(target)) valid.push_back(target);
        }
        else if (SummitToolTargetCurrent(target))
        {
            valid.push_back(target);
        }
    }

    if (valid.empty())
    {
        SummitDestroyPreparedModel(request->secondary);
        SummitDestroyPreparedModel(request->primary);
        return;
    }

    smDPAT *primary = 0;
    smDPAT *secondary = 0;
    {
        std::lock_guard<std::recursive_mutex> lock(SummitPatternMutexStorage());
        primary = SummitInstallPreparedModelLocked(request->primary);
        if (primary && request->secondary.success)
            secondary = SummitInstallPreparedModelLocked(request->secondary);

        if (!primary)
        {
            SummitDestroyPreparedModel(request->secondary);
            SummitDestroyPreparedModel(request->primary);
        }
        else if (request->targetKind != SUMMIT_MODEL_TARGET_TOOL)
        {
            // The install reference is owned by the first direct character/item
            // consumer; every additional consumer receives one extra cache ref.
            for (std::size_t i = 1; i < valid.size(); ++i)
                ++primary->UseCount;
            if (secondary)
            {
                std::size_t characterUsers = 0;
                for (std::size_t i = 0; i < valid.size(); ++i)
                    if (valid[i].kind == SUMMIT_MODEL_TARGET_CHARACTER)
                        ++characterUsers;
                for (std::size_t i = 1; i < characterUsers; ++i)
                    ++secondary->UseCount;
            }
        }
    }

    if (!primary)
    {
        for (std::size_t i = 0; i < valid.size(); ++i)
        {
            if (valid[i].kind == SUMMIT_MODEL_TARGET_CHARACTER)
                valid[i].character->PatLoading = FALSE;
            else
                valid[i].item->PatLoading = FALSE;
        }
        return;
    }

    SummitPrewarmPatternTextures(primary);
    SummitPrewarmPatternTextures(secondary);

    if (request->targetKind == SUMMIT_MODEL_TARGET_TOOL)
    {
        // SetTool() is re-entered only after the model is in the cache. Its
        // fast path acquires a cache reference and therefore performs no I/O.
        for (std::size_t i = 0; i < valid.size(); ++i)
            valid[i].character->SetTool(valid[i].itemCode, valid[i].toolPosition);

        // Tool consumers acquired their own refs through SetTool(); release the
        // temporary install reference held by this prepared bundle.
        smDPAT_Delete(primary);
        return;
    }

    for (std::size_t i = 0; i < valid.size(); ++i)
    {
        SummitModelTarget &target = valid[i];
        if (target.kind == SUMMIT_MODEL_TARGET_ITEM)
        {
            scITEM *item = target.item;
            smDPAT *old = item->lpDinaPattern;
            item->DisplayFlag = 0;
            item->SetPattern(primary->Pat);
            item->lpDinaPattern = primary;
            strcpy_s(item->szModelName, sizeof(item->szModelName), primary->szPatName);
            item->DisplayFlag = TRUE;
            item->PatLoading = FALSE;
            if (old) smDPAT_Delete(old);
            continue;
        }

        smCHAR *character = target.character;
        smDPAT *oldPrimary = character->lpDinaPattern;
        smDPAT *oldSecondary = character->lpDinaPattern2;

        character->DisplayFlag = 0;
        character->SetDinaPattern(primary);
        character->SetMotionFromCode(CHRMOTION_STATE_STAND);
        character->Flag = 1;
        strcpy_s(character->smCharInfo.szModelName, sizeof(character->smCharInfo.szModelName), primary->szPatName);

        if (secondary)
        {
            character->SetDinaPattern2(secondary);
            strcpy_s(character->smCharInfo.szModelName2, sizeof(character->smCharInfo.szModelName2), secondary->szPatName);
        }
        else if (oldSecondary)
        {
            character->SetDinaPattern2(oldSecondary);
            oldSecondary = 0;
        }
        else
        {
            character->SetDinaPattern2(0);
        }

        character->DisplayFlag = TRUE;
        character->PatLoading = FALSE;
        if (oldPrimary) smDPAT_Delete(oldPrimary);
        if (oldSecondary) smDPAT_Delete(oldSecondary);
    }
}

static std::string SummitMakeCharacterKey(const char *primaryName, const char *secondaryName,
    const char *faceHair, const char *defaultFaceHair, bool companion)
{
    std::string key = "C|";
    key += SummitLower(primaryName ? primaryName : "");
    key += "|";
    key += SummitLower(secondaryName ? secondaryName : "");
    key += "|";
    key += SummitLower(faceHair ? faceHair : "");
    key += "|";
    key += SummitLower(defaultFaceHair ? defaultFaceHair : "");
    key += companion ? "|1" : "|0";
    return key;
}

static std::string SummitMakeItemKey(const char *modelName)
{
    return std::string("I|") + SummitLower(modelName ? modelName : "");
}

static std::string SummitMakeToolKey(const char *modelName)
{
    return std::string("T|") + SummitLower(modelName ? modelName : "");
}

static int SummitQueueRequest(const std::shared_ptr<SummitModelRequest> &request,
    const SummitModelTarget &target)
{
    if (!request)
        return FALSE;

    {
        std::lock_guard<std::mutex> lock(SummitModelState().mutex);
        std::unordered_map<std::string, std::weak_ptr<SummitModelRequest> >::iterator existingIt =
            SummitModelState().pendingByKey.find(request->key);
        if (existingIt != SummitModelState().pendingByKey.end())
        {
            std::shared_ptr<SummitModelRequest> existing = existingIt->second.lock();
            if (existing && !existing->cancelled.load(std::memory_order_acquire))
            {
                existing->targets.push_back(target);
                return TRUE;
            }
            SummitModelState().pendingByKey.erase(existingIt);
        }

        request->targets.push_back(target);
        SummitModelState().pendingByKey[request->key] = request;
    }

    if (!summit::core::concurrency::JobSystem::Instance().Submit(
        [request]() { SummitPrepareRequestOnWorker(request); },
        request->targetKind == SUMMIT_MODEL_TARGET_TOOL
            ? summit::core::concurrency::JobPriority::High
            : summit::core::concurrency::JobPriority::Normal))
    {
        std::lock_guard<std::mutex> lock(SummitModelState().mutex);
        std::unordered_map<std::string, std::weak_ptr<SummitModelRequest> >::iterator it =
            SummitModelState().pendingByKey.find(request->key);
        if (it != SummitModelState().pendingByKey.end())
        {
            std::shared_ptr<SummitModelRequest> current = it->second.lock();
            if (!current || current->id == request->id)
                SummitModelState().pendingByKey.erase(it);
        }
        return FALSE;
    }
    return TRUE;
}

} // anonymous namespace

std::recursive_mutex &SummitPatternCacheMutexV3()
{
    return SummitPatternMutexStorage();
}

smDPAT *SummitAcquireCachedModelV3(const char *modelName)
{
    if (!modelName || !modelName[0])
        return 0;
    std::lock_guard<std::recursive_mutex> lock(SummitPatternMutexStorage());
    const int index = SummitFindExactPattern(smPattern, modelName);
    if (index < 0)
        return 0;
    smDPAT *entry = &smPattern.DinaPat[index];
    ++entry->UseCount;
    entry->LastUsedTime = GetCurrentTime();
    return entry;
}

int SummitRequestCharacterModelV3(smCHAR *character, const char *primaryName, const char *secondaryName)
{
    if (!character || !primaryName || !primaryName[0])
        return FALSE;

    const char *face = GetFaceHairSkin(character);
    const char *fallback = GetDefaultFaceHairSkin(character);
    const bool companion =
        character->smCharInfo.State == smCHAR_STATE_USER &&
        character->smCharInfo.Brood == smCHAR_MONSTER_USER &&
        character->smCharInfo.dwTemp[smCOMPANION_NET_TEMP_MAGIC] == smCOMPANION_NET_MAGIC;

    SummitModelTarget target;
    target.kind = SUMMIT_MODEL_TARGET_CHARACTER;
    target.character = character;
    {
        std::lock_guard<std::mutex> lock(SummitModelState().mutex);
        target.generation = ++SummitModelState().characterGenerations[character];
    }

    character->PatLoading = TRUE;

    std::shared_ptr<SummitModelRequest> request(new SummitModelRequest());
    request->id = SummitModelState().requestSerial.fetch_add(1u, std::memory_order_acq_rel);
    request->targetKind = SUMMIT_MODEL_TARGET_CHARACTER;
    request->primaryName = primaryName;
    request->requestedSecondary = secondaryName ? secondaryName : "";
    request->faceHair = face ? face : "";
    request->defaultFaceHair = fallback ? fallback : "";
    request->dataServerCompanion = companion;
    request->key = SummitMakeCharacterKey(primaryName, secondaryName,
        request->faceHair.c_str(), request->defaultFaceHair.c_str(), companion);
    const int queued = SummitQueueRequest(request, target);
    if (!queued)
        character->PatLoading = FALSE;
    return queued;
}

int SummitRequestItemModelV3(scITEM *item, const char *modelName)
{
    if (!item || !modelName || !modelName[0])
        return FALSE;

    SummitModelTarget target;
    target.kind = SUMMIT_MODEL_TARGET_ITEM;
    target.item = item;
    {
        std::lock_guard<std::mutex> lock(SummitModelState().mutex);
        target.generation = ++SummitModelState().itemGenerations[item];
    }

    item->PatLoading = TRUE;

    std::shared_ptr<SummitModelRequest> request(new SummitModelRequest());
    request->id = SummitModelState().requestSerial.fetch_add(1u, std::memory_order_acq_rel);
    request->targetKind = SUMMIT_MODEL_TARGET_ITEM;
    request->primaryName = modelName;
    request->key = SummitMakeItemKey(modelName);
    const int queued = SummitQueueRequest(request, target);
    if (!queued)
        item->PatLoading = FALSE;
    return queued;
}

int SummitRequestToolModelV3(smCHAR *character, unsigned long itemCode, int toolPosition, const char *modelName)
{
    if (!character || !modelName || !modelName[0] ||
        (toolPosition != hvPOSI_RHAND && toolPosition != hvPOSI_LHAND))
        return FALSE;

    SummitModelTarget target;
    target.kind = SUMMIT_MODEL_TARGET_TOOL;
    target.character = character;
    target.itemCode = static_cast<DWORD>(itemCode);
    target.toolPosition = toolPosition;
    {
        std::lock_guard<std::mutex> lock(SummitModelState().mutex);
        std::unordered_map<smCHAR *, std::uint64_t> &generations =
            toolPosition == hvPOSI_RHAND ? SummitModelState().rightToolGenerations
                                        : SummitModelState().leftToolGenerations;
        target.generation = ++generations[character];
    }

    std::shared_ptr<SummitModelRequest> request(new SummitModelRequest());
    request->id = SummitModelState().requestSerial.fetch_add(1u, std::memory_order_acq_rel);
    request->targetKind = SUMMIT_MODEL_TARGET_TOOL;
    request->primaryName = modelName;
    request->key = SummitMakeToolKey(modelName);
    return SummitQueueRequest(request, target);
}

void SummitCancelCharacterModelV3(smCHAR *character)
{
    if (!character)
        return;
    std::lock_guard<std::mutex> lock(SummitModelState().mutex);
    ++SummitModelState().characterGenerations[character];
    ++SummitModelState().rightToolGenerations[character];
    ++SummitModelState().leftToolGenerations[character];
    character->PatLoading = FALSE;
}

void SummitCancelItemModelV3(scITEM *item)
{
    if (!item)
        return;
    std::lock_guard<std::mutex> lock(SummitModelState().mutex);
    ++SummitModelState().itemGenerations[item];
    item->PatLoading = FALSE;
}

void SummitCancelToolModelV3(smCHAR *character, int toolPosition)
{
    if (!character)
        return;
    std::lock_guard<std::mutex> lock(SummitModelState().mutex);
    if (toolPosition == hvPOSI_RHAND)
        ++SummitModelState().rightToolGenerations[character];
    else if (toolPosition == hvPOSI_LHAND)
        ++SummitModelState().leftToolGenerations[character];
}

void SummitPumpModelStreamingV3()
{
    SummitDrainDeferredPatternDeletes();

    const std::uint32_t budget = SummitModelCommitBudget();
    for (std::uint32_t i = 0; i < budget; ++i)
    {
        std::shared_ptr<SummitModelRequest> request;
        {
            std::lock_guard<std::mutex> lock(SummitModelState().mutex);
            if (SummitModelState().completed.empty())
                break;
            request = SummitModelState().completed.front();
            SummitModelState().completed.pop_front();
        }
        SummitCompleteRequestOnMain(request);
    }
}

static void SummitClearModelStreamingState(bool permanentShutdown)
{
    SummitModelState().shuttingDown.store(true, std::memory_order_release);

    std::deque<std::shared_ptr<SummitModelRequest> > completed;
    {
        std::lock_guard<std::mutex> lock(SummitModelState().mutex);
        for (std::unordered_map<std::string, std::weak_ptr<SummitModelRequest> >::iterator it =
            SummitModelState().pendingByKey.begin(); it != SummitModelState().pendingByKey.end(); ++it)
        {
            std::shared_ptr<SummitModelRequest> request = it->second.lock();
            if (request)
                request->cancelled.store(true, std::memory_order_release);
        }
        SummitModelState().pendingByKey.clear();
        completed.swap(SummitModelState().completed);
        SummitModelState().characterGenerations.clear();
        SummitModelState().itemGenerations.clear();
        SummitModelState().rightToolGenerations.clear();
        SummitModelState().leftToolGenerations.clear();
    }

    // The caller drains the JobSystem before a session reset, so all prepared
    // model objects are now exclusively owned by this queue/main thread.
    SummitDrainDeferredPatternDeletes();
    while (!completed.empty())
    {
        SummitDestroyPreparedModel(completed.front()->secondary);
        SummitDestroyPreparedModel(completed.front()->primary);
        completed.pop_front();
    }

    if (!permanentShutdown)
        SummitModelState().shuttingDown.store(false, std::memory_order_release);
}

void SummitResetModelStreamingV3()
{
    SummitClearModelStreamingState(false);
}

void SummitShutdownModelStreamingV3()
{
    SummitClearModelStreamingState(true);
}
