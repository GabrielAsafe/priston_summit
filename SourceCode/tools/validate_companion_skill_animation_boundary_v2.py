#!/usr/bin/env python3
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
EXECUTOR = ROOT / "SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp"
CONTRACT = ROOT / "Shared/gameplay/skills/v2/SkillAnimationCommands.h"
ADAPTER = ROOT / "SrcServer/src/systems/gameplay/skills/v2/legacy/LegacySkillAnimationAdapter.cpp"


def fail(msg: str) -> None:
    print(f"companion skill animation boundary v2: FAIL: {msg}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    for path in (EXECUTOR, CONTRACT, ADAPTER):
        if not path.is_file():
            fail(f"missing {path.relative_to(ROOT)}")
    executor = EXECUTOR.read_text(encoding="utf-8", errors="replace")
    contract = CONTRACT.read_text(encoding="utf-8", errors="replace")
    adapter = ADAPTER.read_text(encoding="utf-8", errors="replace")

    for token in (
        "ServerLegacySkillAnimationAdapter().ConfigureLoop(",
        "ServerLegacySkillAnimationAdapter().ClearLoop(",
        "chargeLoop.eventFrame2",
        "chargeLoop.loopSpeed",
    ):
        if token not in executor:
            fail(f"missing routed animation token: {token}")

    for pattern in (
        r"caster->MotionLoopSpeed\s*=",
        r"caster->MotionLoop\s*=",
    ):
        if re.search(pattern, executor):
            fail(f"direct companion animation mutation remains: {pattern}")

    for token in (
        "class ISkillAnimationCommands",
        "ConfigureLoop(EntityId caster, int loopSpeed, int loopCount)",
        "ClearLoop(EntityId caster)",
        "int eventFrame2;",
    ):
        if token not in contract:
            fail(f"GREEN skill animation contract missing: {token}")
    for forbidden in ("smCHAR", "DWORD", "MotionInfo", "character.h", "windows.h"):
        if forbidden in contract:
            fail(f"GREEN skill animation contract contains legacy symbol: {forbidden}")

    for token in (
        "resolved->MotionLoopSpeed = loopSpeed",
        "resolved->MotionLoop = loopCount",
        "resolved->MotionInfo ? resolved->MotionInfo->EventFrame[2] : 0",
        "resolved->MotionLoop = 0",
    ):
        if token not in adapter:
            fail(f"legacy animation parity token missing: {token}")

    print("companion skill animation boundary v2: PASS (loop authority routed through skill animation commands)")


if __name__ == "__main__":
    main()
