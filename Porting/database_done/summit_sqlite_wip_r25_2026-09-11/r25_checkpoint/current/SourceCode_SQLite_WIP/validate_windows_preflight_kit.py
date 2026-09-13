#!/usr/bin/env python3
"""Static contract for the R25 Windows preflight/runtime evidence kit."""
from pathlib import Path

ROOT = Path(__file__).resolve().parent
PS = ROOT / "tools" / "windows_sqlite_preflight.ps1"
RUNNER = ROOT / "tools" / "run_controlled_validation.py"
SMOKE = ROOT / "WINDOWS_RUNTIME_SMOKE_R25.md"
CUTOVER = ROOT / "LOCAL_ONLY_CUTOVER_SAFETY_R25.md"


def main() -> int:
    ps = PS.read_text(encoding="utf-8")
    runner = RUNNER.read_text(encoding="utf-8")
    smoke = SMOKE.read_text(encoding="utf-8")
    cutover = CUTOVER.read_text(encoding="utf-8")

    for token in [
        "run_controlled_validation.py", "audit-local-only", "prepare-local-only",
        "PRE-FLIGHT PASS", "legacy_billing_fallback=1",
    ]:
        assert token in ps, token
    # Preflight must remain non-mutating with respect to provider policy.
    assert "set-billing-fallback" not in ps

    for token in ["VALIDATORS = [", "--jobs", "--output", "ThreadPoolExecutor"]:
        assert token in runner, token
    assert '"validate_sqlite_backup_cutover.py"' in runner
    assert '"validate_windows_preflight_kit.py"' in runner

    for token in [
        "legacy_billing_fallback=1", "set-billing-fallback off",
        "Database.dll.disabled", "wrong local password", "duplicate simultaneous login",
        "Crash/recovery", "R6.1", "pre-cutover SQLite backup",
    ]:
        assert token in smoke, token
    for token in ["backup API", "zero active account sessions", "non-mutating", "user_version=32"]:
        assert token in cutover, token

    print("Windows preflight kit PASS: non-mutating preflight, canonical runner, rollback backup and strong Database.dll independence smoke contract verified")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
