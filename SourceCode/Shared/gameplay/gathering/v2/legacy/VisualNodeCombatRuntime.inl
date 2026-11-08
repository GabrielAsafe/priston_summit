// Summit experimental attackable gathering proxies V2, included after OpenMonsterFromName.
// This hook deliberately edits only the spawned instance, not the monster template.
// Source policy: SourceCode/Content/canonical/professions_v2/nodes3d.experimental.json.
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include "gameplay/world/v2/WorldPosition.h"
#include "gameplay/gathering/v2/legacy/VisualNodeCombatConfig.generated.h"
#include "systems/gameplay/gathering/v2/GatheringRuntime.h"
#include "systems/gameplay/mining/v2/MiningInteractionRuntime.h"
#include "systems/gameplay/mining/v2/MiningRuntime.h"
#include "systems/gameplay/fishing/v2/FishingRuntime.h"
#include "systems/gameplay/fishing/v2/FishingService.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"

// The original functions are defined elsewhere in this same legacy translation unit.
smCHAR* rsOpenTier5SummonFromCanonicalName(const char*, int, int, int, rsPLAYINFO*);
int rsPutItemChecked(rsPLAYINFO*, DWORD);

struct SummitVisualCombatSlot
{
    std::uint32_t serial;
    std::uint32_t lastConsumedSerial;
    std::uint32_t retryAt;
    SummitVisualCombatSlot() : serial(0), lastConsumedSerial(0), retryAt(0) {}
};

static std::vector<SummitVisualCombatSlot>& SummitVisualCombatSlots()
{
    static std::vector<SummitVisualCombatSlot> slots(
        summit_professions::kVisualNodeCount);
    return slots;
}

bool SummitVisualNodeIsManagedSerial(std::uint32_t serial)
{
    if (serial == 0) return false;
    const std::vector<SummitVisualCombatSlot>& slots = SummitVisualCombatSlots();
    for (std::size_t i = 0; i < slots.size(); ++i)
        if (slots[i].serial == serial || slots[i].lastConsumedSerial == serial)
            return true;
    return false;
}

static bool SummitVisualCombatNodeMatches(
    const summit_professions::VisualNodeSpec& spec)
{
    gameplay::gathering::GatheringNodeDefinition node;
    gameplay::gathering::ResourceDefinition resource;
    bool found = false;
    switch (spec.kind)
    {
    case summit_professions::kMining:
        found = gameplay::mining::ServerMiningService().Gathering().GetNode(spec.nodeId, node) &&
                gameplay::mining::ServerMiningService().Gathering().GetResource(node.resourceId, resource);
        break;
    case summit_professions::kFishing:
        found = gameplay::fishing::ServerFishingService().Gathering().GetNode(spec.nodeId, node) &&
                gameplay::fishing::ServerFishingService().Gathering().GetResource(node.resourceId, resource);
        break;
    default:
        found = gameplay::gathering::ServerGatheringService().GetNode(spec.nodeId, node) &&
                gameplay::gathering::ServerGatheringService().GetResource(node.resourceId, resource);
        break;
    }
    return found && node.area == spec.map && node.x == spec.x && node.y == spec.y &&
           node.z == spec.z && resource.key == spec.resourceKey;
}

// V3: reserve a special entity in the exact node stage; no legacy spawn point
// required. No global spawn table, monster template or character is modified.
static const char* SummitVisualModelFileName(const char* text)
{
    const char* name = text;
    for (const char* at=text; at && *at; ++at)
        if (*at=='/' || *at=='\\') name=at+1;
    return name;
}

static smCHAR* SummitOpenVisualNodeInArea(
    const summit_professions::VisualNodeSpec& spec, const std::size_t slotIndex,
    const int fx, const int fy,
    const int fz, const char** failure)
{
    if (spec.map < 0 || spec.map >= StageAreaCount || !StageArea)
    { *failure = "stage-index-out-of-range"; return NULL; }
    STG_AREA& area = StageArea[spec.map];
    if (!area.lpStage)
    { *failure = "stage-has-no-geometry"; return NULL; }
    int templateIndex = -1;
    const DWORD canonicalCode = GetSpeedSum(spec.monsterTemplate);
    for (int i = 0; i < chrMonsterCnt; ++i)
    {
        if (lstrcmp(chrMonsterList[i].szName, spec.monsterTemplate) == 0 ||
            chrMonInfoList[i].dwAutoCharCode == canonicalCode ||
            lstrcmpi(SummitVisualModelFileName(chrMonsterList[i].szModelName),
                     SummitVisualModelFileName(summit_professions::kVisualModelPaths[slotIndex])) == 0)
        { templateIndex = i; break; }
    }
    if (templateIndex < 0)
    { *failure = "monster-template-not-loaded"; return NULL; }
    const smCHAR_INFO& definition = chrMonsterList[templateIndex];
    if (!definition.lpMonInfo)
    { *failure = "monster-template-missing-info"; return NULL; }

    // STG_AREA::OpenMonster requires SetStartPosChar(), which returns FALSE
    // on StarArena 58 because this test stage has no legacy monster respawn
    // points. That failure is NOT a shortage of monster slots. Reuse the
    // engine's special-entity allocation path (as companion spawning does),
    // without adding regular spawn points or changing global monster limits.
    if (area.FindEmptyMonTable() < 0)
    { *failure = "area-entity-table-full"; return NULL; }
    smCHAR* monster = srFindEmptyChar();
    if (!monster)
    { *failure = "global-character-pool-full"; return NULL; }

    monster->Init();
    monster->lpStage = area.lpStage;
    monster->SetPosi(fx, 0, fz, 0, 0, 0);
    monster->pY = fy;
    monster->PosiAround.x = fx;
    monster->PosiAround.y = fy;
    monster->PosiAround.z = fz;
    monster->OpenStartPostion = 0; // valid accounting slot; no new stage spawn point
    monster->lpExt1 = (void*)&area.Head;
    monster->dwObjectSerial = GetNewObjectSerial();
    monster->ActionPattern = 5;
    memcpy(&monster->smCharInfo, &definition, sizeof(smCHAR_INFO));
    memcpy(&monster->smMonsterInfo, definition.lpMonInfo, sizeof(smCHAR_MONSTER_INFO));
    monster->smCharInfo.StatePoint = definition.lpMonInfo->ClassCode;
    monster->AttackUserFlag = 0;
    monster->lpExt2 = 0;
    monster->lpExpAttackPlayInfo = 0;
    monster->ExpAttackLife = 0;
    monster->SetLoadPattern(monster->smCharInfo.szModelName);
    monster->Flag = TRUE;
    if (!area.AddMonTable(monster))
    {
        // AddMonTable increments start-point accounting even on failure.
        if (area.StartPointMonCount[monster->OpenStartPostion] > 0)
            --area.StartPointMonCount[monster->OpenStartPostion];
        monster->Close();
        *failure = "area-entity-table-registration-failed";
        return NULL;
    }
    ++area.MonsterCount;
    monster->ReopenCount = REOPEN_COUNT_NUM;
    monster->dwLastTransTime = dwPlayServTime;
    if (!monster->lpAttackDamageList)
    {
        monster->lpAttackDamageList = new ATTACK_DAMAGE_LIST[ATTACK_DAMAGE_LIST_MAX];
        if (monster->lpAttackDamageList)
            ZeroMemory(monster->lpAttackDamageList,
                       sizeof(ATTACK_DAMAGE_LIST) * ATTACK_DAMAGE_LIST_MAX);
    }
    monster->smMonsterInfo.FallItemCount = 0; // normal template drops stay untouched
    monster->smCharInfo.Exp = 0;              // no combat XP for the visual proxy
    ++rsMonOpenCount;
    monster->DistAroundDbl = 0;
    monster->SetMotionFromCode(CHRMOTION_STATE_STAND);
    monster->TargetMoveCount = 0;
    // Keep the instance eligible for autoplay network snapshots. The V3
    // SPAWN line only verified server-side storage, never client visibility.
    monster->dwLastTransTime = dwPlayServTime;
    monster->NearPlayCount = 256;
    monster->SavePlayBuff2();
    return monster;
}

// The vanilla server only includes a chrAutoPlayer in PLAYDATAGROUP when it
// already has an srTransBuffSize. Send a bounded initial presence packet and
// full PLAYERINFO using the established companion visibility handshake, scoped
// to players physically near these four nodes on their matching test map.
// No edits to monster definitions, other spawns, or client protocol.
struct SummitVisualNodePresenceState
{
    std::uint32_t playerSerial;
    std::uint32_t nodeSerial;
    std::uint32_t lastSendAt;
    unsigned attempts;
    SummitVisualNodePresenceState()
        : playerSerial(0), nodeSerial(0), lastSendAt(0), attempts(0) {}
};

static std::vector<SummitVisualNodePresenceState>& SummitVisualNodePresenceStates()
{
    static std::vector<SummitVisualNodePresenceState> states(
        summit_professions::kVisualNodeCount * static_cast<std::size_t>(CONNECTMAX));
    return states;
}

static void SummitVisualNodeAnnounceNearby(std::uint32_t now)
{
    std::vector<SummitVisualNodePresenceState>& delivered = SummitVisualNodePresenceStates();
    const std::vector<SummitVisualCombatSlot>& slots = SummitVisualCombatSlots();
    int nearPairs = 0;
    for (std::size_t i = 0; i < summit_professions::kVisualNodeCount; ++i)
    {
        const summit_professions::VisualNodeSpec& spec = summit_professions::kVisualNodes[i];
        const std::uint32_t serial = slots[i].serial;
        if (!serial) continue;
        smCHAR* monster = NULL;
        if (StageArea && spec.map >= 0 && spec.map < StageAreaCount)
        {
            STG_AREA& area = StageArea[spec.map];
            for (int j = 0; j < area.MonsterLastPoint; ++j)
            {
                smCHAR* candidate = area.lpCharMonster[j];
                if (candidate && candidate->Flag && candidate->dwObjectSerial == serial)
                { monster = candidate; break; }
            }
        }
        if (!monster) continue;
        for (int j = 0; j < CONNECTMAX; ++j)
        {
            rsPLAYINFO* player = &rsPlayInfo[j];
            if (!player->lpsmSock || !player->dwObjectSerial ||
                player->Position.Area != spec.map) continue;
            const std::int64_t dx = (static_cast<std::int64_t>(monster->pX) - player->Position.x) >> FLOATNS;
            const std::int64_t dz = (static_cast<std::int64_t>(monster->pZ) - player->Position.z) >> FLOATNS;
            if (dx <= -TRANS_VIEW_LIMIT || dx >= TRANS_VIEW_LIMIT ||
                dz <= -TRANS_VIEW_LIMIT || dz >= TRANS_VIEW_LIMIT ||
                dx*dx + dz*dz >= DIST_TRANSLEVEL_HIGH) continue;
            ++nearPairs;

            SummitVisualNodePresenceState& state =
                delivered[i * static_cast<std::size_t>(CONNECTMAX) + static_cast<std::size_t>(j)];
            if (state.playerSerial != player->dwObjectSerial || state.nodeSerial != serial)
            {
                state = SummitVisualNodePresenceState();
                state.playerSerial = player->dwObjectSerial;
                state.nodeSerial = serial;
            }
            if (state.attempts >= 2 ||
                (state.attempts != 0 && static_cast<std::uint32_t>(now - state.lastSendAt) < 6000u))
                continue;
            state.lastSendAt = now;
            ++state.attempts;

            // The legacy PLAYDATAGROUP reader looks for an optional trailing
            // smTRANS_COMMAND after the declared PlayDataCnt entries. Include
            // an explicit zero command, rather than making it read beyond the
            // logical packet length. This is only for managed visual nodes.
            char presenceBuffer[sizeof(smPLAYDATA_GROUP) + sizeof(smPLAYDATA) +
                                sizeof(smTRANS_COMMAND)];
            ZeroMemory(presenceBuffer, sizeof(presenceBuffer));
            smPLAYDATA_GROUP* group = reinterpret_cast<smPLAYDATA_GROUP*>(presenceBuffer);
            smPLAYDATA* presence = reinterpret_cast<smPLAYDATA*>(presenceBuffer + sizeof(smPLAYDATA_GROUP));
            group->size = sizeof(presenceBuffer);
            group->code = OpCode::OPCODE_PLAYDATAGROUP;
            group->PlayDataCnt = 1;
            group->dwSendCode = 0x80000000u | (player->SendPlayGroupCount & 0x00FFFFFFu);
            presence->size = sizeof(smPLAYDATA);
            presence->code = OpCode::OPCODE_PLAYDATA1;
            presence->dwObjectSerial = serial;
            presence->x = monster->pX;
            presence->y = monster->pY;
            presence->z = monster->pZ;
            presence->angle[0] = monster->Angle.x;
            presence->angle[1] = monster->Angle.y;
            presence->angle[2] = monster->Angle.z;
            presence->angle[3] = 0;
            presence->frame = 0;
            presence->dwAutoCharCode = monster->smMonsterInfo.dwAutoCharCode;
            const int presenceSent = SendOnServerLegacySocket(
                player->lpsmSock, presenceBuffer, group->size);

            smTRNAS_PLAYERINFO info;
            ZeroMemory(&info, sizeof(info));
            memcpy(&info.smCharInfo, &monster->smCharInfo, sizeof(smCHAR_INFO));
            info.size = sizeof(smTRNAS_PLAYERINFO);
            info.code = OpCode::OPCODE_PLAYERINFO;
            info.dwObjectSerial = serial;
            info.x = monster->pX;
            info.y = monster->pY;
            info.z = monster->pZ;
            const int infoSent = SendOnServerLegacySocket(
                player->lpsmSock, reinterpret_cast<char*>(&info), info.size);
            monster->NearPlayCount = 256;
            fprintf(stderr,
                "[SUMMIT_NODE_3D] NET_ANNOUNCE node=%u serial=%u player_serial=%u try=%u "
                "presence_sent=%d info_sent=%d autobuff=%d loading=%d code=%u model_path=%s\n",
                static_cast<unsigned int>(spec.nodeId), static_cast<unsigned int>(serial),
                static_cast<unsigned int>(player->dwObjectSerial), state.attempts,
                presenceSent ? 1 : 0, infoSent ? 1 : 0,
                monster->srTransBuffSize, monster->PatLoading,
                static_cast<unsigned int>(monster->smMonsterInfo.dwAutoCharCode),
                monster->smCharInfo.szModelName);
            fflush(stderr);
        }
    }
    // Change-only diagnostics distinguish map/client-position problems from
    // missing or rejected packets. No player names, IPs or account IDs.
    int online = 0;
    int inMap58 = 0;
    for (int j = 0; j < CONNECTMAX; ++j)
    {
        if (!rsPlayInfo[j].lpsmSock || !rsPlayInfo[j].dwObjectSerial) continue;
        ++online;
        if (rsPlayInfo[j].Position.Area == 58) ++inMap58;
    }
    static int previousOnline = -1, previousArea = -1, previousNear = -1;
    if (online != previousOnline || inMap58 != previousArea || nearPairs != previousNear)
    {
        fprintf(stderr,
            "[SUMMIT_NODE_3D] NET_SCAN online=%d area58=%d near_node_pairs=%d\n",
            online, inMap58, nearPairs);
        fflush(stderr);
        previousOnline = online;
        previousArea = inMap58;
        previousNear = nearPairs;
    }
    // V4.1 read-only runtime probe. The previous near=0 message did not
    // distinguish server/client coordinate differences from a node that
    // disappeared from the stage's monster table. Do not alter send gates.
    static std::uint32_t lastProbeAt = 0;
    static bool hadPlayerIn58 = false;
    if (inMap58 && (!hadPlayerIn58 ||
        static_cast<std::uint32_t>(now - lastProbeAt) >= 5000u))
    {
        lastProbeAt = now;
        for (int j = 0; j < CONNECTMAX; ++j)
        {
            rsPLAYINFO* player = &rsPlayInfo[j];
            if (!player->lpsmSock || !player->dwObjectSerial ||
                player->Position.Area != 58) continue;
            const int px = player->Position.x >> FLOATNS;
            const int pz = player->Position.z >> FLOATNS;
            std::size_t closestIndex = 0;
            std::int64_t closestDist2 = -1;
            int liveNodes = 0;
            smCHAR* closestAlive = NULL;
            for (std::size_t i = 0; i < summit_professions::kVisualNodeCount; ++i)
            {
                const summit_professions::VisualNodeSpec& s = summit_professions::kVisualNodes[i];
                if (s.map != 58) continue;
                const std::int64_t ex = static_cast<std::int64_t>(s.x) - px;
                const std::int64_t ez = static_cast<std::int64_t>(s.z) - pz;
                const std::int64_t squared = ex*ex + ez*ez;
                if (closestDist2 < 0 || squared < closestDist2)
                { closestIndex = i; closestDist2 = squared; }
                if (!StageArea || 58 >= StageAreaCount) continue;
                STG_AREA& area = StageArea[58];
                const std::uint32_t wanted = slots[i].serial;
                if (!wanted) continue;
                for (int k = 0; k < area.MonsterLastPoint; ++k)
                {
                    smCHAR* found = area.lpCharMonster[k];
                    if (found && found->Flag && found->dwObjectSerial == wanted)
                    { ++liveNodes; break; }
                }
            }
            const summit_professions::VisualNodeSpec& nearest =
                summit_professions::kVisualNodes[closestIndex];
            if (StageArea && 58 < StageAreaCount)
            {
                STG_AREA& area = StageArea[58];
                const std::uint32_t wanted = slots[closestIndex].serial;
                for (int k = 0; wanted && k < area.MonsterLastPoint; ++k)
                {
                    smCHAR* found = area.lpCharMonster[k];
                    if (found && found->Flag && found->dwObjectSerial == wanted)
                    { closestAlive = found; break; }
                }
            }
            int deltaX = nearest.x - px, deltaZ = nearest.z - pz;
            const char* gate = "NODE_NOT_IN_AREA_TABLE";
            if (closestAlive)
            {
                deltaX = static_cast<int>((static_cast<std::int64_t>(closestAlive->pX) -
                            player->Position.x) >> FLOATNS);
                deltaZ = static_cast<int>((static_cast<std::int64_t>(closestAlive->pZ) -
                            player->Position.z) >> FLOATNS);
                const std::int64_t dist2 = static_cast<std::int64_t>(deltaX)*deltaX +
                                          static_cast<std::int64_t>(deltaZ)*deltaZ;
                gate = (deltaX <= -TRANS_VIEW_LIMIT || deltaX >= TRANS_VIEW_LIMIT ||
                        deltaZ <= -TRANS_VIEW_LIMIT || deltaZ >= TRANS_VIEW_LIMIT)
                     ? "OUTSIDE_AXIS_LIMIT"
                     : (dist2 >= DIST_TRANSLEVEL_HIGH ? "OUTSIDE_DIST2_LIMIT" : "WITHIN_RANGE");
            }
            fprintf(stderr,
                "[SUMMIT_NODE_3D] NET_PROBE area=58 slot=%d player_fixed=%d,%d,%d "
                "player_grid=%d,%d nearest=%u node_grid=%d,%d "
                "live_nodes=%d/%u nearest_live=%d monster_fixed=%d,%d "
                "dx=%d dz=%d axis_limit=%d dist2_limit=%d gate=%s now=%u\n",
                j, player->Position.x, player->Position.y, player->Position.z,
                px, pz, static_cast<unsigned int>(nearest.nodeId), nearest.x, nearest.z,
                liveNodes, static_cast<unsigned int>(summit_professions::kVisualNodeCount),
                closestAlive ? 1 : 0,
                closestAlive ? closestAlive->pX : 0, closestAlive ? closestAlive->pZ : 0,
                deltaX, deltaZ, TRANS_VIEW_LIMIT, DIST_TRANSLEVEL_HIGH, gate,
                static_cast<unsigned int>(now));
            fflush(stderr);
            break; // At most one anonymous diagnostic per interval.
        }
    }
    hadPlayerIn58 = (inMap58 != 0);
}

// # SUMMIT386A GMSP JSON GM XYZ
#include "gameplay/gathering/v2/legacy/GmSpawnRuntime386.inl"

static void SummitVisualNodeCombatTick(std::uint32_t now)
{
    SummitGMTick386(now);
    static bool bootLogged = false;
    if (!bootLogged)
    {
        fprintf(stderr, "[SUMMIT_NODE_3D] TICK_ACTIVE count=%u stages=%d now=%u\n",
            static_cast<unsigned int>(summit_professions::kVisualNodeCount),
            StageAreaCount, static_cast<unsigned int>(now));
        fflush(stderr);
        bootLogged = true;
    }
    std::vector<SummitVisualCombatSlot>& slots = SummitVisualCombatSlots();
    for (std::size_t i = 0; i < summit_professions::kVisualNodeCount; ++i)
    {
        const summit_professions::VisualNodeSpec& spec = summit_professions::kVisualNodes[i];
        SummitVisualCombatSlot& slot = slots[i];
        if (slot.serial)
        {
            // Normal monsters can be culled after a long period without nearby
            // players. That is legitimate for ambient monsters, but a logical
            // profession node must remain discoverable after someone logs in.
            // Verify the actual area table, not just the remembered serial.
            smCHAR* living = NULL;
            if (StageArea && spec.map >= 0 && spec.map < StageAreaCount)
            {
                STG_AREA& area = StageArea[spec.map];
                for (int j = 0; j < area.MonsterLastPoint; ++j)
                {
                    smCHAR* candidate = area.lpCharMonster[j];
                    if (candidate && candidate->Flag &&
                        candidate->dwObjectSerial == slot.serial)
                    { living = candidate; break; }
                }
            }
            if (living)
            {
                // Keep ONLY these managed gathering instances alive. In the
                // legacy srAutoCharMain an ActionPattern=5 character is culled
                // once dwLastTransTime is older than 5 minutes. NearPlayCount
                // alone does NOT prevent that independent timeout. Refresh the
                // timestamp while the logical node is still registered, but
                // never when killed (Kill already clears slot.serial).
                living->NearPlayCount = 256;
                living->ReopenCount = REOPEN_COUNT_NUM;
                living->dwLastTransTime = dwPlayServTime;
                continue;
            }
            // A stale serial is not a valid spawn. Recover immediately, while
            // preserving the post-kill cooldown (Kill already clears serial).
            fprintf(stderr,
                "[SUMMIT_NODE_3D] LOST node=%u serial=%u reason=missing-from-stage-table\n",
                static_cast<unsigned int>(spec.nodeId),
                static_cast<unsigned int>(slot.serial));
            fflush(stderr);
            slot.serial = 0;
            slot.retryAt = now;
        }
        if (now < slot.retryAt)
            continue;
        if (!spec.physicalOnly && !SummitVisualCombatNodeMatches(spec))
        {
            fprintf(stderr, "[SUMMIT_NODE_3D] WAIT node=%u reason=sqlite-resource-or-coordinates-mismatch map=%d xyz=%d,%d,%d\n",
                static_cast<unsigned int>(spec.nodeId), spec.map, spec.x, spec.y, spec.z);
            slot.retryAt = now + 15000u;
            continue;
        }
        const int fx = gameplay::WorldGridToFixed(spec.x);
        const int fy = gameplay::WorldGridToFixed(spec.y);
        const int fz = gameplay::WorldGridToFixed(spec.z);
        const char* failure = "unknown";
        smCHAR* monster = SummitOpenVisualNodeInArea(spec, i, fx, fy, fz, &failure);
        if (!monster || !monster->dwObjectSerial)
        {
            fprintf(stderr, "[SUMMIT_NODE_3D] SPAWN_FAIL node=%u map=%d template=%s reason=%s\n",
                static_cast<unsigned int>(spec.nodeId), spec.map,
                spec.monsterTemplate, monster ? "zero-serial" : failure);
            slot.retryAt = now + 15000u;
            continue;
        }
        monster->smMonsterInfo.GetExp = 0;
        monster->smCharInfo.Level = 1;
        monster->smCharInfo.Life[0] = 30;
        monster->smCharInfo.Life[1] = 30;
        monster->smCharInfo.Move_Speed = 0;
        monster->smCharInfo.Attack_Damage[0] = 0;
        monster->smCharInfo.Attack_Damage[1] = 0;
        monster->smCharInfo.Defence = 0;
        slot.lastConsumedSerial = 0;
        slot.serial = static_cast<std::uint32_t>(monster->dwObjectSerial);
        fprintf(stderr, "[SUMMIT_NODE_3D] SPAWN node=%u serial=%u map=%d xyz=%d,%d,%d model=%s\n",
            static_cast<unsigned int>(spec.nodeId),
            static_cast<unsigned int>(slot.serial), spec.map,
            spec.x, spec.y, spec.z, spec.monsterTemplate);
        fflush(stderr);
    }
    // Includes players who arrived after startup; never announces to other
    // maps or outside the normal high-detail viewport.
    SummitVisualNodeAnnounceNearby(now);
}

static bool SummitVisualNodeCombatKill(smCHAR* monster, rsPLAYINFO* killer,
                                       std::uint32_t now)
{
    if (!monster) return false;
    const std::uint32_t serial = static_cast<std::uint32_t>(monster->dwObjectSerial);
    std::vector<SummitVisualCombatSlot>& slots = SummitVisualCombatSlots();
    for (std::size_t i = 0; i < summit_professions::kVisualNodeCount; ++i)
    {
        SummitVisualCombatSlot& slot = slots[i];
        if (slot.lastConsumedSerial == serial && serial != 0)
            return true; // duplicate callback: never award or run legacy drops twice.
        if (serial == 0 || slot.serial != serial)
            continue;

        const summit_professions::VisualNodeSpec& spec = summit_professions::kVisualNodes[i];
        slot.serial = 0;
        slot.lastConsumedSerial = serial;
        slot.retryAt = now + spec.respawnMs;
        if (!killer || !killer->lpsmSock)
            return true;
        const gameplay::PlayerId player =
            gameplay::player::legacy::ServerLegacyOnlinePlayerRegistry().IdOf(killer);
        if (!player.Valid())
            return true;

        // SUMMIT417A_BASE_NODES: only registered node serials enter this branch.
        // Consume the death serial above; never credit SQLite or generic monster loot.
        if (spec.physicalOnly)
        {
            const unsigned int roll = static_cast<unsigned int>(rand()) % 10000u;
            if (roll < spec.itemChancePer10000)
            {
                const int granted = rsPutItemChecked(killer, spec.legacyItemCode);
                fprintf(stderr, "[SUMMIT417] BASE_DROP node=%u item=%s numeric=0x%08X physical=%d sqlite=0\n",
                    static_cast<unsigned int>(spec.nodeId), spec.itemTextCode,
                    static_cast<unsigned int>(spec.legacyItemCode), granted ? 1 : 0);
                fflush(stderr);
            }
            return true;
        }
        gameplay::gathering::HarvestResult harvest;
        const std::uint32_t roll = static_cast<std::uint32_t>(rand());
        if (spec.kind == summit_professions::kMining)
            harvest = gameplay::mining::HarvestMiningNodeForPlayer(player, spec.nodeId, now, roll);
        else if (spec.kind == summit_professions::kFishing)
            harvest = gameplay::fishing::FishNodeForPlayer(player, spec.nodeId, now, roll);
        else
            harvest = gameplay::gathering::HarvestGatheringNodeForPlayer(player, spec.nodeId, now, roll);

        // Harvest validates player, area, distance, profession/level and node cooldown.
        // Never drop an item after rejected harvesting.
        if (harvest.Succeeded())
        {
            const std::uint32_t chanceRoll = static_cast<std::uint32_t>(rand()) % 10000u;
            if (chanceRoll < spec.itemChancePer10000)
            {
                // Sends a real catalog item via the existing server item authority.
                // This path is not a transactional crafting grant; test inventory/relog.
                const int granted = rsPutItemChecked(killer, spec.legacyItemCode);
                fprintf(stdout, "[SUMMIT_NODE_3D] REWARD node=%u resource=%s amount=%d code=%s sent=%d\n",
                    static_cast<unsigned int>(spec.nodeId), harvest.resourceKey.c_str(),
                    harvest.amount, spec.itemTextCode, granted ? 1 : 0);
            }
        }
        else
        {
            fprintf(stdout, "[SUMMIT_NODE_3D] REJECT node=%u status=%d\n",
                static_cast<unsigned int>(spec.nodeId), static_cast<int>(harvest.status));
        }
        // Suppress all normal drops of the reused monster for *this instance*.
        return true;
    }
    return false;
}
