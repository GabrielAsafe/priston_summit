#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
EXECUTOR = ROOT / "SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp"
CONTRACT = ROOT / "Shared/gameplay/skills/v2/runtime/SkillRuntime.h"
ADAPTER = ROOT / "SrcServer/src/systems/gameplay/skills/v2/legacy/LegacySkillRuntimeAdapter.cpp"


def fail(message: str) -> None:
    print(f"companion skill runtime boundary v1: FAIL: {message}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    for path in (EXECUTOR, CONTRACT, ADAPTER):
        if not path.is_file():
            fail(f"missing {path.relative_to(ROOT)}")

    executor = EXECUTOR.read_text(encoding="utf-8", errors="replace")
    if "ServerLegacySkillRuntimeAdapter().Begin(" not in executor:
        fail("BeginCast is not routed through ISkillRuntime")
    for token in ("static DWORD SkillMotionState", "caster->AttackSkil =", "caster->SetMotionFromCode(runtime.lastSkillMotionState)"):
        if token in executor:
            fail(f"legacy cast runtime mutation still direct in CompanionSkillExecutor: {token}")
    for token in ("runtime.lastSkillPlayCode = skillRuntime.playCode", "runtime.lastSkillMotionState = skillRuntime.motionState"):
        if token not in executor:
            fail(f"Companion runtime does not capture service result: {token}")

    contract = CONTRACT.read_text(encoding="utf-8", errors="replace")
    for token in ("class ISkillRuntime", "SkillRuntimeRequest", "SkillRuntimeResult", "virtual SkillRuntimeResult Begin"):
        if token not in contract:
            fail(f"GREEN skill runtime contract missing: {token}")
    for forbidden_symbol in ("smCHAR", "rsPLAYINFO", "DWORD", "windows.h", "onserver.h", "character.h"):
        if forbidden_symbol in contract:
            fail(f"GREEN skill runtime contains legacy symbol/include: {forbidden_symbol}")

    adapter = ADAPTER.read_text(encoding="utf-8", errors="replace")
    for token in (
        "DefaultSkillService().LegacyPlayCode(request.skill)",
        "caster->AttackSkil = encodedPlaySkill",
        "caster->SetMotionFromCode",
        "caster->AttackSkil = 0",
        "CHRMOTION_STATE_ATTACK",
        "CHRMOTION_STATE_SKILL",
    ):
        if token not in adapter:
            fail(f"legacy runtime parity token missing from adapter: {token}")

    print("companion skill runtime boundary v1: PASS")


if __name__ == "__main__":
    main()
