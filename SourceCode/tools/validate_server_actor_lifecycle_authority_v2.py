#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
SERVER = ROOT / "SrcServer/src/systems/gameplay/character/core/character.cpp"
SERVICE = ROOT / "Shared/gameplay/world/v2/ActorLifecycleService.h"


def fail(message: str) -> None:
    print(f"server actor lifecycle authority v2: FAIL: {message}")
    raise SystemExit(1)


text = SERVER.read_text(encoding="utf-8", errors="ignore")
service = SERVICE.read_text(encoding="utf-8", errors="ignore")

if '#include "gameplay/world/v2/ActorLifecycleService.h"' not in text:
    fail("server character runtime does not include ActorLifecycleService")

try:
    main = text[text.index("int smCHAR::Main()"):]
    case10 = main[main.index("\tcase 10:\n"):main.index("\tcase 12:\n")]
    case12 = main[main.index("\tcase 12:\n"):main.index("\tcase 99:\n")]
except ValueError as exc:
    fail(f"cannot isolate smCHAR::Main lifecycle cases: {exc}")

if case10.count("ActorLifecycleService::ResolveAnimationCompletion") != 1:
    fail("case 10/11 must delegate completion exactly once")
if "if(frame > MotionInfo->EndFrame * 160)" in case10 or "if (frame > MotionInfo->EndFrame * 160)" in case10:
    fail("case 10/11 still owns inline end-frame completion")

if case12.count("ActorLifecycleService::AdvanceOneShot") < 2:
    fail("case 12 must delegate both warp one-shot branches")
if case12.count("ActorLifecycleService::ResolveAnimationCompletion") < 1:
    fail("case 12 fallback must delegate animation completion")

# The NPC random-motion branch is deliberately still a legacy shell responsibility;
# only the generic completion/one-shot policy is V2 authority here.
if "smMotionInfo->NpcMotionRate[CHRMOTION_EXT]" not in case12:
    fail("NPC legacy shell branch was unexpectedly removed")

for forbidden in ("smCHAR", "smCHAR_INFO", "rsPLAYINFO", "smWINSOCK"):
    if forbidden in service:
        fail(f"GREEN ActorLifecycleService leaks legacy symbol {forbidden}")

required = (
    "AdvanceOneShot",
    "ResolveAnimationCompletion",
    "ActorAnimationCompletionAction::HoldFinalFrame",
    "ActorAnimationCompletionAction::RepeatFromStart",
    "ActorAnimationCompletionAction::ReturnToStand",
)
for token in required:
    if token not in service:
        fail(f"ActorLifecycleService contract missing {token}")

print("server actor lifecycle authority v2: PASS (server completion/warp policy delegated to GREEN ActorLifecycleService)")
