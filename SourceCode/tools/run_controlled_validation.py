#!/usr/bin/env python3
"""Run the controlled Summit SQLite WIP validation suite cross-platform."""
from __future__ import annotations

import argparse
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
VALIDATORS = [
    "validate_backup_catalog_completion.py",
    "validate_billingdb_sqlite_contract.py",
    "validate_build_scripts.py",
    "validate_build_source_graph.py",
    "validate_clan_mark_storage.py",
    "validate_clan_sqlite_contract.py",
    "validate_clan_tag_storage.py",
    "validate_compiler_adaptations.py",
    "validate_database_dll_gameplay_contract.py",
    "validate_gameplay_entitlement_boundary.py",
    "validate_local_gameplay_entitlement.py",
    "validate_local_audit_sinks.py",
    "validate_notice_sqlite_contract.py",
    "validate_port_structure_presence.py",
    "validate_server_http_surface.py",
    "validate_sod2_sqlite_contract.py",
    "validate_sod_record_local_sink.py",
    "validate_sqlite_local_login.py",
    "validate_sqlite_local_session.py",
    "validate_sqlite_migrations.py",
    "validate_sqlite_query_coverage.py",
    "validate_sqlite_transactions.py",
    "validate_structural_exact_includes.py",
    "validate_structural_macro_equivalence.py",
    "validate_structural_macro_equivalence_strict.py",
    "validate_structural_policy_equivalence.py",
    "validate_structural_policy_equivalence_strict.py",
    "validate_structural_shared_include_resolution.py",
    "validate_structural_shared_proof.py",
    "validate_structured_safe_parity.py",
    "validate_structured_safe_strict_preprocessor.py",
    "validate_active_nonbaseline_graph.py",
    "validate_active_shared_graph.py",
    "validate_local_auth_policy.py",
    "validate_database_dll_local_only.py",
    "validate_game_masters_sqlite.py",
    "validate_sqlite_local_admin.py",
    "validate_sqlite_local_readiness.py",
    "validate_sqlite_backup_cutover.py",
    "validate_windows_preflight_kit.py",
]


def run_one(name: str) -> tuple[str, int, str]:
    path = ROOT / name
    proc = subprocess.run([sys.executable, str(path)], cwd=ROOT, text=True, capture_output=True)
    body = proc.stdout
    if proc.stderr:
        body += proc.stderr
    return name, proc.returncode, body


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--output", type=Path, help="also write complete suite output to this file")
    ap.add_argument("--jobs", type=int, default=1, help="parallel validators (default: 1; use 4-8 for preflight)")
    args = ap.parse_args()
    if args.jobs < 1 or args.jobs > 32:
        ap.error("--jobs must be 1..32")

    if args.jobs == 1:
        results = [run_one(name) for name in VALIDATORS]
    else:
        with ThreadPoolExecutor(max_workers=args.jobs) as pool:
            futures = {name: pool.submit(run_one, name) for name in VALIDATORS}
            results = [futures[name].result() for name in VALIDATORS]

    chunks: list[str] = []
    passed = 0
    failed = 0
    for name, returncode, body in results:
        header = f"===== {name} =====\n"
        outcome = "PASS" if returncode == 0 else f"FAIL (exit {returncode})"
        text = header + body.rstrip() + f"\nRESULT: {outcome}\n\n"
        print(text, end="")
        chunks.append(text)
        if returncode == 0:
            passed += 1
        else:
            failed += 1
    summary = f"SUMMARY: {passed} PASS / {failed} FAIL / {len(VALIDATORS)} TOTAL\n"
    print(summary, end="")
    chunks.append(summary)
    if args.output:
        out = args.output if args.output.is_absolute() else ROOT / args.output
        out.parent.mkdir(parents=True, exist_ok=True)
        out.write_text("".join(chunks), encoding="utf-8")
    return 0 if failed == 0 else 1


if __name__ == "__main__":
    raise SystemExit(main())
