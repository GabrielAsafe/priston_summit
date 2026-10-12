#!/usr/bin/env python3
"""Read-only provenance audit of a complete 263 staging tree after binary changes.

This is NOT a current 267 gate and NEVER authorizes plan/apply/deployment.
A complete, self-consistent historical staging tree can still be compared with
client caches even if Game.exe/Server.exe/tool identities have since changed.
"""
from __future__ import annotations

import json
from pathlib import Path
import tempfile

from audit_model_caches_245 import audit as census_audit
from audit_model_roles_246 import audit as roles_audit, _safe_relative
from cook_native_models_259 import no_symlink_ancestors, select_client_executable
from cook_native_models_incremental_263 import FORMAT, _artifact_valid, _hash, _previous

IDENTITY_KEYS = ('server_sha256', 'client_sha256', 'converter_tool_sha256')


def _real_client_and_out(client: Path, server: Path, out: Path, executable: Path):
    for path in (client, server, out, executable):
        no_symlink_ancestors(path)
    client = client.resolve(strict=True)
    server = server.resolve(strict=True)
    out = out.resolve(strict=True)
    if (not client.is_dir() or not (client / 'Game.exe').is_file() or
            not server.is_file() or server.name.casefold() != 'server.exe' or
            not out.is_dir()):
        raise ValueError('valid client, Server.exe and external staging required')
    if (out == client or client in out.parents or out == server.parent or
            server.parent in out.parents or any(p.casefold() == 'canonical' for p in out.parts)):
        raise ValueError('staging must remain outside client/server/canonical')
    for p in client.rglob('*'):
        if p.is_symlink() and p.suffix.lower() == '.ase':
            raise ValueError('symlinked ASE in client')
    return client, server, out


def verify_archived_readonly(client: Path, server: Path, out: Path,
                             client_exe: Path | None = None) -> dict:
    """Verify all bytes and current ASE roles; report runtime identity drift.

    This does not re-do native readbacks. Previously recorded readback flags
    are checked against hashes and original staging provenance, but a changed
    executable requires current gate 267 again before any deployment.
    """
    executable = select_client_executable(client, client_exe)
    client, server, out = _real_client_and_out(client, server, out, executable)
    manifest, staged = _previous(out)  # refuses extras, symlinks, duplicate/unsafe paths
    if manifest.get('format') != FORMAT or manifest.get('scope') != 'all':
        raise ValueError('only full incremental staging permitted in archival comparison')
    if manifest.get('complete') is not True or manifest.get('count') != len(staged) or not staged:
        raise ValueError('archival staging not complete/consistent')
    recorded = manifest.get('identity')
    if (not isinstance(recorded, dict) or set(recorded) != set(IDENTITY_KEYS) or
            any(not isinstance(recorded[k], str) or len(recorded[k]) != 64 or
                any(x not in '0123456789abcdef' for x in recorded[k]) for k in IDENTITY_KEYS)):
        raise ValueError('invalid recorded staging identity')

    with tempfile.TemporaryDirectory(prefix='.summit279_readonly_') as tmp:
        census_path = Path(tmp) / 'census.json'
        roles_path = Path(tmp) / 'roles.json'
        census = census_audit(client, census_path)
        roles = roles_audit(client, census_path, roles_path)
    if roles['summary'].get('unclassified') or roles['summary'].get('legacy_cache_path_over_63'):
        raise ValueError('invalid live ASE roles or paths')
    digests = {entry['source']: entry['source_sha256'] for entry in census['entries']}
    expected = set()
    for item in roles['entries']:
        source = _safe_relative(item['source']).as_posix()
        kinds = item.get('required_cache_types', [])
        if not kinds or sorted(kinds) not in (['smd'], ['smb'], ['smb', 'smd']):
            raise ValueError('unexpected cache roles for ' + source)
        for extension in kinds:
            if (source, extension) in expected:
                raise ValueError('duplicate expected role')
            expected.add((source, extension))
    if set(staged) != expected:
        raise ValueError('archival catalog coverage differs from current ASE roles')
    for (source, extension), row in staged.items():
        if source not in digests or not _artifact_valid(out, row, digests[source]):
            raise ValueError('staged cache corrupt or source changed: ' + source)

    current = {
        'server_sha256': _hash(server),
        'client_sha256': _hash(executable),
        'converter_tool_sha256': _hash(Path(__file__).with_name('cook_native_models_259.py')),
    }
    changed = [key for key in IDENTITY_KEYS if current[key] != recorded[key]]
    return {
        'verified': len(staged), 'expected': len(expected), 'missing': 0,
        'complete': True, 'identity_changed': changed,
        'previous_identity': dict(recorded), 'current_identity': current,
        'read_only': True, 'deployment_allowed': False,
    }
