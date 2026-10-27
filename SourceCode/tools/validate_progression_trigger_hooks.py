#!/usr/bin/env python3
"""Guard server-side Story reevaluation hooks and reentrancy coordination."""

from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
QUESTS = ROOT / "SrcServer/src/systems/gameplay/quests/server/cQuestHandler.cpp"
JOBS = ROOT / "SrcServer/src/systems/crafting/professions/JOBS.cpp"
RUNTIME = ROOT / "SrcServer/src/systems/gameplay/progression/v2/ProgressionRuntime.cpp"
TRIGGER = ROOT / "SrcServer/src/systems/gameplay/progression/v2/ProgressionTriggerRuntime.cpp"
GATEWAYS = ROOT / "SrcServer/src/systems/gameplay/progression/v2/LegacyProgressionGateways.cpp"
ADAPTER = ROOT / "SrcServer/src/systems/gameplay/progression/v2/legacy/LegacyPlayerExperienceAdapter.cpp"
SERVICE = ROOT / "Shared/gameplay/progression/v2/PlayerExperienceService.h"


def fail(message: str) -> None:
    print(f"progression trigger hooks validation: FAIL: {message}", file=sys.stderr)
    raise SystemExit(1)


def function_body(text: str, signature: str) -> str:
    start = text.find(signature)
    if start < 0:
        fail(f"missing function {signature}")
    brace = text.find("{", start)
    if brace < 0:
        fail(f"missing body for {signature}")
    depth = 0
    for i in range(brace, len(text)):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                return text[brace : i + 1]
    fail(f"unterminated body for {signature}")
    return ""


def require(body: str, token: str, where: str) -> None:
    if token not in body:
        fail(f"{where} is missing {token}")


def main() -> None:
    for path in (QUESTS, JOBS, RUNTIME, TRIGGER, GATEWAYS, ADAPTER, SERVICE):
        if not path.is_file():
            fail(f"missing {path.relative_to(ROOT)}")

    quests = QUESTS.read_text(encoding="utf-8")
    for signature in (
        "void cQUESTHANDLER::StartQuest",
        "void cQUESTHANDLER::CancelQuest",
        "void cQUESTHANDLER::GetReward",
    ):
        require(function_body(quests, signature),
                "NotifyProgressionIfQuestStateChanged",
                signature)

    jobs = JOBS.read_text(encoding="utf-8")
    for signature in (
        "void JOBS::RecvJOBItemToClient",
        "void JOBS::AddExpJOBS",
    ):
        require(function_body(jobs, signature),
                "NotifyServerProgressionPlayerStateChanged",
                signature)

    gateways = GATEWAYS.read_text(encoding="utf-8")
    grant = function_body(gateways, "rewards::RewardResult LegacyProgressionRewardGateway::Grant")
    # XP authority is the V2 service; only its legacy adapter may invoke AddExp.
    require(grant, "PlayerExperienceService::Grant(", "LegacyProgressionRewardGateway::Grant")
    require(grant, "legacy::ServerLegacyPlayerExperienceAdapter()", "LegacyProgressionRewardGateway::Grant")
    require(grant, "PlayerExperienceGrantRequest(", "LegacyProgressionRewardGateway::Grant")
    require(grant, "PlayerExperienceSource::ProgressionReward", "LegacyProgressionRewardGateway::Grant")
    require(grant, "if(!experience.Granted())", "LegacyProgressionRewardGateway::Grant")
    require(grant, "NotifyServerProgressionPlayerStateChanged(playerId);",
            "LegacyProgressionRewardGateway::Grant XP path")
    if "player->AddExp" in grant:
        fail("LegacyProgressionRewardGateway::Grant bypasses PlayerExperienceService")
    if grant.count("PlayerExperienceService::Grant(") != 1:
        fail("LegacyProgressionRewardGateway::Grant must call XP service exactly once")

    adapter = ADAPTER.read_text(encoding="utf-8")
    mutate = function_body(adapter, "PlayerExperienceGrantResult LegacyPlayerExperienceAdapter::Grant")
    require(mutate, "player->AddExp(", "LegacyPlayerExperienceAdapter::Grant")
    require(mutate, "PlayerExperienceGrantCode::Granted", "LegacyPlayerExperienceAdapter::Grant")
    if mutate.count("player->AddExp(") != 1:
        fail("LegacyPlayerExperienceAdapter::Grant must mutate XP exactly once")

    service = SERVICE.read_text(encoding="utf-8")
    service_grant = function_body(service, "static PlayerExperienceGrantResult Grant")
    require(service_grant, "request.amount <= 0", "PlayerExperienceService::Grant")
    require(service_grant, "port.Grant(request)", "PlayerExperienceService::Grant")

    runtime = RUNTIME.read_text(encoding="utf-8")
    process = function_body(runtime, "ProgressionPlayerRunReport ProcessServerProgressionPlayer")
    require(process, "IsServerProgressionRuntimeReady()", "ProcessServerProgressionPlayer")
    require(process, "PlayerProcessingCoordinator()", "ProcessServerProgressionPlayer")
    require(process, "coordinator.Request", "ProcessServerProgressionPlayer")
    require(process, "PlayerProcessingSnapshots()", "ProcessServerProgressionPlayer")
    require(process, "std::lock_guard<std::recursive_mutex>", "ProcessServerProgressionPlayer")

    initialize = function_body(runtime, "bool InitializeServerProgressionRuntime")
    require(initialize, "ServerProgressionRuntimeState::CatalogLoaded", "InitializeServerProgressionRuntime")
    finalize = function_body(runtime, "bool FinalizeServerProgressionRuntimeAfterItems")
    require(finalize, "ServerProgressionRuntimeState::CatalogLoaded", "FinalizeServerProgressionRuntimeAfterItems")
    require(finalize, "ServerProgressionRuntimeState::Ready", "FinalizeServerProgressionRuntimeAfterItems")
    require(finalize, "RegisterServerProgressionEventTriggers();", "FinalizeServerProgressionRuntimeAfterItems")

    trigger = TRIGGER.read_text(encoding="utf-8")
    snapshot = function_body(trigger, "bool ProcessSnapshot")
    require(snapshot, "IsServerProgressionRuntimeReady()", "ProcessSnapshot")
    require(snapshot, "ProcessServerProgressionPlayer", "ProcessSnapshot")

    online = function_body(trigger, "bool ProcessServerProgressionOnlinePlayer")
    require(online, "ServerLegacyPlayerAdapter().TryGet(playerId", "ProcessServerProgressionOnlinePlayer")
    require(online, "ProcessSnapshot", "ProcessServerProgressionOnlinePlayer")
    notify = function_body(trigger, "void NotifyServerProgressionPlayerStateChanged")
    require(notify, "ProcessServerProgressionOnlinePlayer", "NotifyServerProgressionPlayerStateChanged")

    world = function_body(trigger, "void NotifyServerProgressionWorldStateChanged")
    require(world, "WorldProcessingCoordinator().Request", "NotifyServerProgressionWorldStateChanged")
    require(world, "ServerLegacyOnlinePlayerRegistry().OnlineHandles()", "NotifyServerProgressionWorldStateChanged")
    require(world, "ServerLegacyPlayerAdapter().TryGet", "NotifyServerProgressionWorldStateChanged")
    require(world, "ProcessSnapshot", "NotifyServerProgressionWorldStateChanged")
    registration = function_body(trigger, "void RegisterServerProgressionEventTriggers")
    require(registration, "StoryWorldFlagChangedEvent", "RegisterServerProgressionEventTriggers")
    require(registration, "CovenantJoinedEvent", "RegisterServerProgressionEventTriggers")
    require(registration, "CovenantLeftEvent", "RegisterServerProgressionEventTriggers")
    require(registration, "CovenantReputationChangedEvent", "RegisterServerProgressionEventTriggers")
    require(registration, "CovenantRankChangedEvent", "RegisterServerProgressionEventTriggers")
    require(registration, "reputation::ReputationChangedEvent", "RegisterServerProgressionEventTriggers")


    print("progression trigger hooks validation: PASS")


if __name__ == "__main__":
    main()
