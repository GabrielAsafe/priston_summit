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

    # Status application/clear authority is fully extracted. Direct writes in
    # the executor are no longer accepted as a transitional state.
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
    if residual_slow:
        fail(f"direct slow writes returned: {len(residual_slow)}")
    residual_clear = re.findall(r"->PlayStunCount\s*=\s*0\s*;", executor)
    if residual_clear:
        fail(f"direct stun-clear writes returned: {len(residual_clear)}")

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
        "effect.timerMode == SlowTimerMode::CountUpFromNegative ? -effect.durationTicks : effect.durationTicks",
        "resolved->PlayDistortion = effect.distortionTicks",
        "resolved->PlayHeavenlyLight[0] = static_cast<short>(attackRateReduction)",
    ):
        if token not in adapter:
            fail(f"legacy status parity token missing from adapter: {token}")

    print(
        "companion status boundary v1: PASS "
        "(executor direct status writes=0)"
    )


if __name__ == "__main__":
    main()
