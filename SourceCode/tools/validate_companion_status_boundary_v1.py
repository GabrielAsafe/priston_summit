#!/usr/bin/env python3
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
EXECUTOR = ROOT / "SrcServer/src/systems/gameplay/companions/CompanionSkillExecutor.cpp"
CONTRACT = ROOT / "Shared/gameplay/combat/v2/StatusEffectService.h"
ADAPTER = ROOT / "SrcServer/src/systems/gameplay/combat/v2/legacy/LegacyStatusEffectAdapter.cpp"


def fail(message: str) -> None:
    print(f"companion status boundary v1: FAIL: {message}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    for path in (EXECUTOR, CONTRACT, ADAPTER):
        if not path.is_file():
            fail(f"missing {path.relative_to(ROOT)}")

    executor = EXECUTOR.read_text(encoding="utf-8", errors="replace")
    for token in (
        "ServerLegacyStatusEffectAdapter().ApplyStun(",
        "ServerLegacyStatusEffectAdapter().ApplyPoison(",
        "ServerLegacyStatusEffectAdapter().ApplyHolyMind(",
        "ServerLegacyStatusEffectAdapter().ApplyAssassinEye(",
        "ServerLegacyStatusEffectAdapter().ApplySlow(",
        "ServerLegacyStatusEffectAdapter().ApplyHeavenlyLight(",
    ):
        if token not in executor:
            fail(f"missing routed status operation: {token}")

    # These status families have been fully extracted in this cut.  The only
    # direct residuals intentionally left for the follow-up cleanup are three
    # elemental slow groups and two explicit PlayStunCount = 0 clears.
    for pattern in (
        r"->PlayPoison\s*\[",
        r"->PlayHolyMind\s*\[",
        r"->dwAssassinEyeTime\s*=",
        r"->AssassinEyeParam\s*=",
        r"->PlayHeavenlyLight\s*\[",
    ):
        if re.search(pattern, executor):
            fail(f"extracted status mutation still direct: {pattern}")

    stun_assignments = re.findall(r"->PlayStunCount\s*=\s*([^;]+);", executor)
    if any(rhs.strip() != "0" for rhs in stun_assignments):
        fail(f"non-clear stun assignment still direct: {stun_assignments}")

    residual_slow = re.findall(r"->Play(?:SlowCount|SlowSpeed|Distortion)\s*=", executor)
    if len(residual_slow) not in (0, 9):
        fail(f"unexpected direct slow residual count: {len(residual_slow)} (expected 9 before cleanup or 0 after)")
    residual_clear = re.findall(r"->PlayStunCount\s*=\s*0\s*;", executor)
    if len(residual_clear) not in (0, 2):
        fail(f"unexpected direct stun-clear residual count: {len(residual_clear)} (expected 2 before cleanup or 0 after)")

    contract = CONTRACT.read_text(encoding="utf-8", errors="replace")
    for token in (
        "class IStatusEffectService",
        "ApplyStun",
        "ApplyPoison",
        "ApplyHolyMind",
        "ApplyAssassinEye",
        "ApplySlow",
        "ApplyHeavenlyLight",
    ):
        if token not in contract:
            fail(f"GREEN status contract missing: {token}")
    for forbidden_symbol in (
        "rsPLAYINFO", "smCHAR", "DWORD", "short ", "windows.h", "onserver.h", "character.h"
    ):
        if forbidden_symbol in contract:
            fail(f"GREEN status contract contains legacy symbol/include: {forbidden_symbol}")

    adapter = ADAPTER.read_text(encoding="utf-8", errors="replace")
    for token in (
        "resolved->PlayStunCount = durationTicks",
        "resolved->PlayPoison[1] = static_cast<short>(durationTicks)",
        "resolved->PlayHolyMind[0] = static_cast<short>(damageReduction)",
        "resolved->dwAssassinEyeTime = static_cast<DWORD>(expiresAt)",
        "resolved->PlaySlowCount = effect.durationTicks",
        "resolved->PlayDistortion = effect.distortionTicks",
        "resolved->PlayHeavenlyLight[0] = static_cast<short>(attackRateReduction)",
    ):
        if token not in adapter:
            fail(f"legacy status parity token missing from adapter: {token}")

    print(
        "companion status boundary v1: PASS "
        f"({len(residual_slow)} direct slow writes; {len(residual_clear)} explicit stun clears reserved for cleanup)"
    )


if __name__ == "__main__":
    main()
