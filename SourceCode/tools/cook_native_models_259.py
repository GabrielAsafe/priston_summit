#!/usr/bin/env python3
"""Offline experimental ASE -> native SMD/SMB batch preparation.

Fail closed. Invokes Server.exe's exact native parser in an isolated scratch
working directory, checks every artifact, then publishes ONLY to --out.
Never writes to --client, --server or SourceCode/Content/canonical.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

from audit_model_roles_246 import audit, _safe_relative, _digest

MAGIC = b'SMD Model data Ver 0.62'
FORMAT = 'SUMMIT_NATIVE_MODEL_COOK_259_V1'


def _failure_excerpt(stderr: str, stdout: str) -> str:
    """Preserve Wine's first exception/backtrace lines as well as its tail.

    Printed only on a failed *explicit offline* cook; no persistent logs.
    """
    message = (stderr + '\n' + stdout).strip()
    if len(message) <= 2400:
        return message
    return message[:1600] + '\n... [middle omitted] ...\n' + message[-700:]


def no_symlink_ancestors(path: Path):
    cursor = path.absolute()
    while True:
        if cursor.is_symlink():
            raise ValueError('symlink in path')
        if cursor.parent == cursor:
            break
        cursor = cursor.parent


def select_client_executable(client: Path, override: Path | None = None) -> Path:
    """Select a real Game.exe without ever copying it into the asset directory."""
    selected = override if override is not None else client / 'Game.exe'
    no_symlink_ancestors(selected)
    selected = selected.resolve(strict=True)
    if selected.name.casefold() != 'game.exe' or not selected.is_file():
        raise ValueError('client verifier must be a regular Game.exe')
    return selected


def probe_client_verifier(executable: Path, wine: str, working_directory: Path, timeout: int = 12) -> None:
    """Verify the offline CLI works BEFORE handing it a complex model.

    The deliberately nonexistent SMB forces WinMain's early branch to return 2.
    A stale Game.exe cannot silently run its normal startup for 180 seconds.
    """
    binary = executable.read_bytes()
    if not binary.startswith(b'MZ') or b'--summit-verify-model' not in binary:
        raise ValueError('selected Game.exe lacks the offline verifier; use --client-exe with dist_mingw_client_i686/Game.exe')
    probe = '__summit_offline_probe_272_missing__.smb'
    if (working_directory / probe).exists():
        raise ValueError('offline probe path unexpectedly occupied')
    try:
        result = subprocess.run([wine, str(executable), '--summit-verify-model', 'bone', probe],
                                cwd=working_directory, capture_output=True, text=True,
                                errors='replace', timeout=timeout, check=False)
    except subprocess.TimeoutExpired as exc:
        raise ValueError('Game.exe offline verifier did not exit quickly; check which Game.exe is selected') from exc
    except OSError as exc:
        raise ValueError('cannot invoke offline Game.exe: ' + str(exc)) from exc
    if result.returncode != 2:
        raise ValueError('Game.exe offline probe returned rc=%d (expected 2 for missing SMB)' % result.returncode)


def prepare(client: Path, server: Path, census: Path, roles: Path, out: Path, wine: str = 'wine', pilot_source: str | None = None, client_exe: Path | None = None, probe_exe: bool = True) -> dict:
    for x in (client, server, census, roles, out):
        no_symlink_ancestors(x)
    executable = select_client_executable(client, client_exe)
    client = client.resolve(strict=True)
    server = server.resolve(strict=True)
    census = census.resolve(strict=True)
    roles = roles.resolve(strict=True)
    if not server.is_file() or server.name.casefold() != 'server.exe':
        raise ValueError('native server executable required')
    if not client.is_dir():
        raise ValueError('client assets directory required')
    if client_exe is None and not (client / 'Game.exe').is_file():
        raise ValueError('client root missing Game.exe')
    out = out.absolute()
    if out.exists() or out.is_symlink():
        raise ValueError('output directory must not exist')
    if not out.parent.is_dir():
        raise ValueError('output parent must already exist')
    parent = out.parent.resolve(strict=True)
    if parent == client or client in parent.parents or parent == server.parent or server.parent in parent.parents:
        raise ValueError('output must be outside client and server')
    if parent.name.lower() == 'canonical' or any(part.casefold() == 'canonical' for part in parent.parts):
        raise ValueError('do not publish to canonical')
    if not wine or any(c.isspace() for c in wine):
        raise ValueError('invalid runner; supply one executable path')

    given = json.loads(roles.read_text(encoding='utf-8'))
    if given.get('format') != 'SUMMIT_ASE_ROLE_AUDIT_246_V1':
        raise ValueError('unexpected role audit version')
    with tempfile.TemporaryDirectory(prefix='.summit259_', dir=parent) as temp:
        root = Path(temp)
        checked = audit(client, census, root / 'verified_roles.json')
        if given.get('entries') != checked['entries'] or given.get('summary') != checked['summary']:
            raise ValueError('stale or altered roles report')
        hashes = {row['source']: row['source_sha256'] for row in json.loads(census.read_text(encoding='utf-8'))['entries']}
        if pilot_source is not None and pilot_source not in {row['source'] for row in checked['entries']}:
            raise ValueError('pilot source not found in validated roles report')
        job_root = root / 'jobs'
        published = root / 'publish'
        published.mkdir()
        artifacts = []
        # Keep existing offline fake-runner tests unchanged. A selected executable
        # from the build directory must prove its early WinMain entry point first.
        if client_exe is not None and probe_exe:
            probe_client_verifier(executable, wine, root)
        for i, entry in enumerate(checked['entries']):
            if pilot_source is not None and entry['source'] != pilot_source:
                continue
            rel = _safe_relative(entry['source'])
            if (len(rel.as_posix()) > 63 or
                    not re.fullmatch(r'[A-Za-z0-9_./-]+', rel.as_posix())):
                raise ValueError('ASE path unsupported by native server legacy buffer')
            required = entry.get('required_cache_types')
            if entry.get('role') == 'unclassified' or not required:
                raise ValueError('unclassified or unused ASE refused; review manifest')
            if sorted(required) not in (['smd'], ['smb'], ['smb', 'smd']):
                raise ValueError('invalid conversion roles')
            original = client.joinpath(*rel.parts)
            for ext in required:
                role = 'mesh' if ext == 'smd' else 'bone'
                job = job_root / f'{i:04d}_{ext}'
                source = job.joinpath(*rel.parts)
                source.parent.mkdir(parents=True)
                shutil.copyfile(original, source)
                if (_digest(source) != hashes[rel.as_posix()] or
                        _digest(original) != hashes[rel.as_posix()]):
                    raise ValueError('ASE changed during copying')
                command = [wine, str(server), '--summit-cook-model-one', role, rel.as_posix()]
                try:
                    result = subprocess.run(command, cwd=job, capture_output=True, text=True,
                                            errors='replace', timeout=180, check=False)
                except (OSError, subprocess.TimeoutExpired) as exc:
                    raise ValueError(f'native conversion invocation failed: {rel} {role}: {exc}') from exc
                if result.returncode != 0 or 'MODEL_COOK_259=PASS' not in result.stdout.splitlines():
                    # Only show bounded diagnostic output on failure; no regular gameplay logging.
                    detail = _failure_excerpt(result.stderr, result.stdout)
                    raise ValueError(f'native conversion failed: {rel} {role}: rc={result.returncode} {detail}')
                native = source.with_suffix('.' + ext)
                if native.is_symlink() or not native.is_file():
                    raise ValueError('native converter did not produce regular cache')
                data = native.read_bytes()
                if len(data) < 64 or len(data) > 512 * 1024 * 1024 or not data.startswith(MAGIC):
                    raise ValueError('unexpected native SMD/SMB header or size')
                # PATCH 261: reject publication unless the *client* native
                # parser also reopens the binary, without D3D/login startup.
                client_command = [wine, str(executable), '--summit-verify-model',
                                  role, rel.with_suffix('.' + ext).as_posix()]
                try:
                    client_result = subprocess.run(client_command, cwd=job, capture_output=True,
                                                   text=True, errors='replace', timeout=180, check=False)
                except (OSError, subprocess.TimeoutExpired) as exc:
                    raise ValueError(f'client native readback invocation failed: {rel} {role}: {exc}') from exc
                if client_result.returncode != 42:
                    detail = _failure_excerpt(client_result.stderr, client_result.stdout)
                    raise ValueError(f'client native readback failed: {rel} {role}: rc={client_result.returncode} {detail}')
                destination = published.joinpath(*rel.with_suffix('.' + ext).parts)
                destination.parent.mkdir(parents=True, exist_ok=True)
                with destination.open('xb') as file:
                    file.write(data)
                artifacts.append({'source': rel.as_posix(), 'type': ext, 'cache': destination.relative_to(published).as_posix(),
                                  'bytes': len(data), 'sha256': hashlib.sha256(data).hexdigest(),
                                  'source_sha256': hashes[rel.as_posix()], 'native_readback': True, 'client_readback': True})
        if not artifacts:
            raise ValueError('no native conversions requested')
        for entry in checked['entries']:
            rel = _safe_relative(entry['source'])
            if _digest(client.joinpath(*rel.parts)) != hashes[rel.as_posix()]:
                raise ValueError('ASE changed before publication')
        document = {'format': FORMAT, 'converter': 'Server.exe --summit-cook-model-one',
                    'notice': 'Native server and client readbacks succeeded. Visual quality and gameplay compatibility are NOT proven. Publication to client remains manual after visual validation.',
                    'count': len(artifacts), 'scope': 'pilot_only' if pilot_source is not None else 'all_classified_ase', 'artifacts': artifacts}
        (published / 'manifest.json').write_text(json.dumps(document, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
        if out.exists():
            raise ValueError('output appeared during conversion')
        os.rename(published, out)
        return document


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for opt in ('client', 'server', 'census', 'roles', 'out'):
        parser.add_argument('--' + opt, type=Path, required=True)
    parser.add_argument('--wine', default='wine', help='Wine executable, single path (default: wine)')
    parser.add_argument('--client-exe', type=Path, help='Explicit Game.exe to verify caches; client assets stay at --client')
    parser.add_argument('--pilot-source', help='Exact ASE path from the verified role report; process one source first')
    args = parser.parse_args()
    try:
        result = prepare(args.client, args.server, args.census, args.roles, args.out, args.wine, args.pilot_source, args.client_exe)
    except (ValueError, OSError, KeyError, TypeError, json.JSONDecodeError) as exc:
        print('MODEL_COOK_259=FAIL_CLOSED ' + str(exc), file=sys.stderr)
        return 2
    print(f'MODEL_COOK_259=PASS artifacts={result["count"]} native_readback=1 client_readback=1 no_client_writes=1')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
