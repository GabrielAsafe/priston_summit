#!/usr/bin/env python3
"""Close the static authority audit for the two largest server legacy monoliths.

This is an orchestration gate: each authority family remains owned by its focused
validator, while this gate makes the closure criterion explicit and prevents the
frozen preflight from silently omitting one of those families.
"""
from __future__ import annotations

from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
TOOLS = ROOT / "tools"

# High-authority families reachable from OnServer / smCHAR and the adjacent
# persistence/network shells. Presentation/timer/hydration state is intentionally
# not treated as gameplay authority here; exact shell budgets remain enforced by
# the focused ownership validators.
VALIDATORS = (
    "validate_server_life_mutation_ownership_v1.py",
    "validate_server_resource_mutation_ownership_v1.py",
    "validate_server_status_effect_mutation_ownership_v1.py",
    "validate_inventory_mutation_ownership_v3.py",
    "validate_player_experience_ownership_v1.py",
    "validate_wallet_mutation_ownership_v2.py",
    "validate_coin_wallet_mutation_ownership_v1.py",
    "validate_onserver_quest_read_boundary_v1.py",
    "validate_onserver_quest_spoiler_boundary_v1.py",
    "validate_character_record_quest_read_boundary_v1.py",
    "validate_character_main_targeting_authority_v2.py",
    "validate_onserver_socket_transport_retired_v3.py",
    "validate_character_transport_retired_v3.py",
    "validate_red_direct_socket_ownership_v3.py",
)

DOC = ROOT / "docs/architecture/LEGACY_MONOLITH_AUTHORITY_CLOSURE_V1.md"
PREFLIGHT = TOOLS / "run_frozen_legacy_preflight.sh"


def fail(message: str) -> None:
    print(f"legacy monolith authority residual v1: FAIL: {message}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    if not DOC.is_file():
        fail("missing authority closure classification document")

    doc = DOC.read_text(encoding="utf-8", errors="replace")
    required_doc_tokens = (
        "AUTHORITY_RESIDUAL=ZERO_STATIC",
        "Shell legítimo",
        "Life / lethal / healing",
        "Mana / Stamina",
        "Status effects",
        "Inventory",
        "Experience",
        "Wallet / SCoins / TCoin",
        "Quest reads",
        "Transport / packet routing",
        "Target selection",
    )
    for token in required_doc_tokens:
        if token not in doc:
            fail(f"classification document missing token: {token}")

    preflight = PREFLIGHT.read_text(encoding="utf-8", errors="replace")
    if "validate_legacy_monolith_authority_residual_v1.py" not in preflight:
        fail("aggregate monolith gate is not wired into frozen preflight")

    for name in VALIDATORS:
        path = TOOLS / name
        if not path.is_file():
            fail(f"missing component validator: {name}")
        result = subprocess.run(
            [sys.executable, str(path)],
            cwd=ROOT,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
        )
        if result.stdout:
            print(result.stdout, end="" if result.stdout.endswith("\n") else "\n")
        if result.returncode != 0:
            fail(f"component validator failed: {name}")

    print(
        "legacy monolith authority residual v1: PASS "
        f"(families={len(VALIDATORS)}, unclassified_high_authority=0; shell state remains explicitly budgeted)"
    )


if __name__ == "__main__":
    main()
