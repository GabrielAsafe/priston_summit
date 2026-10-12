#!/usr/bin/env python3
"""Read-only, fail-closed readiness check for a *single* native model canary.

Never deploys, copies, moves, cooks, logs or modifies Git/assets.  The current
267 gate and an exact runtime/verifier Game.exe match are needed for READY.
Historical provenance may be inspected for *planning* after a new build.
"""
from __future__ import annotations

import argparse
import hashlib
from pathlib import Path
import shlex
import sys


def _digest(path: Path) -> str:
    h = hashlib.sha256()
    with path.open('rb') as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b''):
            h.update(chunk)
    return h.hexdigest()


def summarize(rows: dict[str, dict], client: Path, inspect_target, strict_target, hash_fn):
    """Evaluate target files without ever selecting a casefold alias for writing."""
    grouped = {}
    count = {'SAME': 0, 'DIFFERENT': 0, 'MISSING': 0,
             'AMBIGUOUS': 0, 'CASE_VARIANT': 0, 'UNSAFE': 0}
    for rel, entry in sorted(rows.items()):
        source = entry['source']
        target, variant, collisions = inspect_target(client, rel)
        if collisions:
            state = 'AMBIGUOUS'
        elif variant:
            state = 'CASE_VARIANT'
        else:
            try:
                exact = strict_target(client, rel)
                if target is not None and exact != target:
                    raise ValueError('historical and strict resolution disagree')
                if target is None and exact.is_file():
                    raise ValueError('historical lookup missed existing target')
            except (ValueError, OSError):
                state = 'UNSAFE'
            else:
                state = ('SAME' if hash_fn(exact) == entry['sha256'] else 'DIFFERENT') \
                    if exact.is_file() else 'MISSING'
        count[state] += 1
        grouped.setdefault(source, []).append((rel, state))
    return count, grouped


def candidates(grouped: dict[str, list[tuple[str, str]]]):
    """Only offer sources whose *every* cache has a safe, exact-spelling target."""
    good = []
    for source, caches in grouped.items():
        if any(state in ('AMBIGUOUS', 'CASE_VARIANT', 'UNSAFE') for _, state in caches):
            continue
        different = sum(state == 'DIFFERENT' for _, state in caches)
        missing = sum(state == 'MISSING' for _, state in caches)
        if not different and not missing:
            continue
        # Existing differing caches are more useful for a reversible A/B test.
        # Smaller source groups minimize the number of overwritten/added files.
        good.append((source, different, missing, len(caches)))
    return sorted(good, key=lambda item: (-int(item[1] > 0), item[3], -item[1], item[0]))


def examine(client: Path, server: Path, stage: Path, executable: Path, selected: str | None):
    # Imports are deliberately postponed: pure planning helpers are testable
    # without Wine, C++ source, or real client files.
    from cook_native_models_259 import no_symlink_ancestors
    from cook_native_models_incremental_263 import _previous
    from deploy_native_model_stage_278 import (
        _entry_target, _historical_inspect_target, _manifest_rows,
    )
    from verify_archived_native_stage_279 import verify_archived_readonly
    from verify_native_model_stage_267 import verify_staging

    for path in (client, server, stage, executable, client / 'Game.exe'):
        no_symlink_ancestors(path)
    client = client.resolve(strict=True)
    server = server.resolve(strict=True)
    stage = stage.resolve(strict=True)
    executable = executable.resolve(strict=True)
    live = client / 'Game.exe'
    if not live.is_file() or not executable.is_file():
        raise ValueError('Game.exe missing')
    # Revalidate *all* 134-type ASE/cache pairs and historical SHA provenance.
    history = verify_archived_readonly(client, server, stage, executable)
    manifest, _ = _previous(stage)
    rows = _manifest_rows(manifest)
    counts, grouped = summarize(rows, client, _historical_inspect_target,
                                _entry_target, _digest)
    options = candidates(grouped)

    identity_drift = history['identity_changed']
    runtime_matches_verifier = _digest(live) == _digest(executable)
    gate = False
    gate_reason = 'identity differs from verified staging'
    if not identity_drift:
        # A historical PASS is never sufficient to create an install plan.
        result = verify_staging(client, server, stage, allow_partial=False, client_exe=executable)
        gate = (result['complete'] and result['missing'] == 0 and
                result['verified'] == result['expected'])
        if not gate:
            raise ValueError('current gate 267 did not pass complete catalog')
        gate_reason = 'PASS'
    print('CANARY_285=READONLY staged=%d same=%d different=%d missing=%d ambiguous=%d case_variants=%d unsafe=%d identity_changed=%s gate267=%s runtime_exe_matches=%d client_writes=0' % (
        len(rows), counts['SAME'], counts['DIFFERENT'], counts['MISSING'],
        counts['AMBIGUOUS'], counts['CASE_VARIANT'], counts['UNSAFE'],
        ','.join(identity_drift) or 'none', gate_reason.replace(' ', '_'),
        runtime_matches_verifier))

    for source, different, missing, total in options[:10]:
        print('CANARY_CANDIDATE source=%s different=%d missing=%d artifacts=%d' % (
            source, different, missing, total))

    ready = gate and runtime_matches_verifier
    if selected is not None:
        matched = next((v for v in options if v[0] == selected), None)
        if matched is None:
            raise ValueError('source is absent, already identical, or has unsafe/case-variant targets: ' + selected)
        if not ready:
            print('CANARY_285=BLOCKED identity/current gate or gameplay Game.exe mismatch; no plan generated')
        else:
            # Existing 278 plan independently re-runs 267; nothing in this tool
            # authorizes apply. Use an *unoccupied* output path for that plan.
            argv = [sys.executable, 'SourceCode/tools/deploy_native_model_stage_278.py', 'plan',
                    '--client', str(client), '--server', str(server),
                    '--client-exe', str(executable), '--out', str(stage),
                    '--source', selected, '--plan', './bugfixes/SUMMIT_CANARIO_PLAN_NOVO.json']
            print('CANARY_285=READY_TO_PLAN command=' + shlex.join(argv))
    elif not ready:
        print('CANARY_285=BLOCKED_FOR_DEPLOYMENT (read-only candidates; no plan generated)')
    else:
        print('CANARY_285=READY_TO_SELECT (use --source with one candidate; no plan generated)')


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--client', type=Path, required=True)
    parser.add_argument('--server', type=Path, required=True)
    parser.add_argument('--client-exe', type=Path, required=True)
    parser.add_argument('--out', type=Path, required=True)
    parser.add_argument('--source', help='Explicit ASE to inspect as a single visual canary')
    args = parser.parse_args()
    try:
        examine(args.client, args.server, args.out, args.client_exe, args.source)
        return 0
    except (ValueError, OSError, TypeError, KeyError) as exc:
        print('CANARY_285=FAIL_CLOSED ' + str(exc), file=sys.stderr)
        return 2


if __name__ == '__main__':
    raise SystemExit(main())
