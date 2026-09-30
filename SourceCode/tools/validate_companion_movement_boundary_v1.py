#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
EXECUTOR = ROOT / "SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp"
CONTRACT = ROOT / "Shared/gameplay/world/v2/MovementCommands.h"
ADAPTER = ROOT / "SrcServer/src/systems/gameplay/world/v2/legacy/LegacyMovementAdapter.cpp"


def fail(message: str) -> None:
    print(f"companion movement boundary v1: FAIL: {message}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    for path in (EXECUTOR, CONTRACT, ADAPTER):
        if not path.is_file():
            fail(f"missing {path.relative_to(ROOT)}")

    executor = EXECUTOR.read_text(encoding="utf-8", errors="replace")
    contract = CONTRACT.read_text(encoding="utf-8", errors="replace")
    adapter = ADAPTER.read_text(encoding="utf-8", errors="replace")

    for token in (
        "ServerLegacyMovementAdapter().PushAway(",
        "IsLegacyPushImmuneBoss(target)",
        "snCHAR_SOUND_DEATHKNIGHT",
        "snCHAR_SOUND_BABEL",
        "snCHAR_SOUND_KELVEZU",
        "snCHAR_SOUND_MOKOVA",
    ):
        if token not in executor:
            fail(f"push policy/routing token missing: {token}")

    for forbidden in (
        "target->OverLapPosi2 =",
        "target->MoveAngle(",
        "target->dwNextMotionCode = CHRMOTION_STATE_STAND",
        "target->Angle.y =",
    ):
        if forbidden in executor:
            fail(f"movement mutation still direct in Companion executor: {forbidden}")

    if "target->lpTargetPlayInfo = NULL" not in executor:
        fail("movement-specific target release side effect was lost")

    for token in (
        "class IMovementCommands",
        "MovementCommandResult PushAway(EntityId target, EntityId source, int distance)",
    ):
        if token not in contract:
            fail(f"GREEN movement contract missing: {token}")
    for forbidden in ("smCHAR", "rsPLAYINFO", "DWORD", "windows.h", "character.h", "onserver.h"):
        if forbidden in contract:
            fail(f"GREEN movement contract contains legacy symbol/include: {forbidden}")

    for token in (
        "GetRadian2D(resolvedTarget->pX, resolvedTarget->pZ",
        "resolvedTarget->Angle.y = (towardSource + ANGLE_180) & ANGCLIP",
        "resolvedTarget->OverLapPosi2 = FALSE",
        "resolvedTarget->MoveAngle(distance)",
        "resolvedTarget->Angle.y = oldAngle",
        "resolvedTarget->dwNextMotionCode = CHRMOTION_STATE_STAND",
    ):
        if token not in adapter:
            fail(f"legacy movement parity token missing from adapter: {token}")

    print("companion movement boundary v1: PASS (knockback movement authority routed; immunity policy preserved)")


if __name__ == "__main__":
    main()
