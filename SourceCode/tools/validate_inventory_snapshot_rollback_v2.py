#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
HEADER = ROOT / "Shared/gameplay/inventory/v2/InventorySnapshotRollback.h"
SMOKE = ROOT / "Shared/gameplay/inventory/v2/tests/inventory_snapshot_rollback_smoke.cpp"
RUNNER = ROOT / "Shared/gameplay/inventory/v2/tests/run_inventory_snapshot_rollback_tests.sh"

checks = {}
checks["rollback header exists"] = HEADER.is_file()
checks["portable smoke exists"] = SMOKE.is_file()
checks["portable runner exists"] = RUNNER.is_file()

header = HEADER.read_text(errors="replace") if HEADER.exists() else ""
smoke = SMOKE.read_text(errors="replace") if SMOKE.exists() else ""

checks["captures inventory through query"] = "Capture" in header and "IInventoryQuery" in header
checks["removes outputs introduced after snapshot"] = "commands_.Remove" in header
checks["re-adds missing inputs"] = "commands_.Add" in header
checks["explicit rollback failure codes"] = all(token in header for token in (
    "SnapshotUnavailable", "CurrentSnapshotUnavailable", "RemoveFailed", "AddFailed"))
checks["smoke covers remove failure"] = "RemoveFailed" in smoke
checks["smoke covers add failure"] = "AddFailed" in smoke

failed = [name for name, ok in checks.items() if not ok]
for name, ok in checks.items():
    print(("PASS" if ok else "FAIL") + ": " + name)
if failed:
    raise SystemExit("FAIL: " + ", ".join(failed))
print(f"PASS: inventory snapshot rollback v2 ({len(checks)}/{len(checks)})")
