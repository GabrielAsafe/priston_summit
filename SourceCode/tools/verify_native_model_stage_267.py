#!/usr/bin/env python3
"""Read-only fail-closed gate for the incremental native ASE model staging tree.

Unlike the native readbacks executed during cook, this gate checks the durable
provenance, set coverage, binary hashes and client/server identity *now*.
It does not convert or deploy SMD/SMB, or modify client/server/content.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path
import sys
import tempfile

from audit_model_caches_245 import audit as census_audit
from audit_model_roles_246 import _safe_relative, audit as roles_audit
from cook_native_models_259 import no_symlink_ancestors, select_client_executable
from cook_native_models_incremental_263 import (
    FORMAT, _artifact_valid, _hash, _previous,
)


def verify_staging(client: Path, server: Path, out: Path,
                   allow_partial: bool = False, client_exe: Path | None = None) -> dict:
    for path in (client, server, out):
        no_symlink_ancestors(path)
    executable = select_client_executable(client, client_exe)
    client = client.resolve(strict=True)
    server = server.resolve(strict=True)
    out = out.resolve(strict=True)
    if (not client.is_dir() or not server.is_file() or
            server.name.casefold() != 'server.exe' or not out.is_dir()):
        raise ValueError('client/Game.exe, Server.exe and staging directory required')
    if (out == client or client in out.parents or out == server.parent or
            server.parent in out.parents or any(p.casefold() == 'canonical' for p in out.parts)):
        raise ValueError('staging must remain outside client/server/canonical')
    for p in client.rglob('*'):
        if p.is_symlink() and p.suffix.lower() == '.ase':
            raise ValueError('symlinked ASE in client')

    manifest, staged = _previous(out)
    if manifest.get('format') != FORMAT:
        raise ValueError('unexpected staging manifest version')
    identity = {
        'server_sha256': _hash(server),
        'client_sha256': _hash(executable),
        'converter_tool_sha256': _hash(Path(__file__).with_name('cook_native_models_259.py')),
    }
    if manifest.get('identity') != identity:
        raise ValueError('client/server/converter version differs from verified staging')
    if manifest.get('count') != len(staged) or not isinstance(manifest.get('complete'), bool):
        raise ValueError('manifest count or complete field invalid')
    if manifest.get('scope') not in ('pilot', 'all'):
        raise ValueError('unexpected staging scope')

    # Recompute role census from live ASE content, not solely from manifest.
    # Only temporary reports outside the game tree; no permanent diagnostics.
    with tempfile.TemporaryDirectory(prefix='.summit267_verify_') as temp:
        census_path = Path(temp) / 'census.json'
        role_path = Path(temp) / 'roles.json'
        census = census_audit(client, census_path)
        roles = roles_audit(client, census_path, role_path)
    if roles['summary'].get('unclassified') or roles['summary'].get('legacy_cache_path_over_63'):
        raise ValueError('unclassified or path-too-long ASE in live client')

    digest_by_source = {row['source']: row['source_sha256'] for row in census['entries']}
    expected = set()
    for entry in roles['entries']:
        source = _safe_relative(entry['source']).as_posix()
        cache_types = entry.get('required_cache_types', [])
        if not cache_types or sorted(cache_types) not in (['smd'], ['smb'], ['smb', 'smd']):
            raise ValueError('unexpected ASE roles')
        for ext in cache_types:
            if (source, ext) in expected:
                raise ValueError('duplicate expected cache type')
            expected.add((source, ext))

    if not staged:
        raise ValueError('empty staging manifest')
    if not set(staged).issubset(expected):
        raise ValueError('unexpected cache not required by live ASE catalog')
    for key, row in staged.items():
        source, _ = key
        if not _artifact_valid(out, row, digest_by_source[source]):
            raise ValueError('cache corrupt, source changed or readback not verified: ' + source)

    actual_complete = len(staged) == len(expected)
    if manifest['complete'] is not actual_complete:
        raise ValueError('manifest completeness claim contradicts current catalog')
    if not actual_complete and not allow_partial:
        raise ValueError('partial staging: missing %d of %d expected caches; run full incremental cook' %
                         (len(expected) - len(staged), len(expected)))
    if actual_complete and manifest['scope'] != 'all':
        # Pilot-only manifest is never an acceptable full-catalog deployment gate.
        raise ValueError('full catalog requires scope=all')
    return {
        'verified': len(staged),
        'expected': len(expected),
        'missing': len(expected) - len(staged),
        'complete': actual_complete,
        'read_only': True,
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--client', type=Path, required=True)
    parser.add_argument('--server', type=Path, required=True)
    parser.add_argument('--out', type=Path, required=True)
    parser.add_argument('--client-exe', type=Path, help='The same Game.exe used to verify native caches during cook')
    parser.add_argument('--allow-partial', action='store_true',
                        help='Audit a pilot, but never claim that the catalog is complete')
    args = parser.parse_args()
    try:
        result = verify_staging(args.client, args.server, args.out, args.allow_partial, args.client_exe)
    except (OSError, ValueError, TypeError, KeyError, json.JSONDecodeError) as exc:
        print('NATIVE_STAGE_267=FAIL_CLOSED ' + str(exc), file=sys.stderr)
        return 2
    print('NATIVE_STAGE_267=PASS verified={verified} expected={expected} '
          'missing={missing} complete={complete} no_client_writes=1'.format(**result))
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
