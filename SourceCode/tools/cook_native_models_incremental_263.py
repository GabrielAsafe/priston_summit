#!/usr/bin/env python3
"""Incremental, offline-only ASE -> native SMD/SMB staging.

Server.exe does the conversion, Game.exe reopens each new cache; Python manages
SHA-256 invalidation and all-or-nothing publication. Never modifies the client,
server, canonical content or Git index. No gameplay hooks and no permanent logs.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
import subprocess
from pathlib import Path, PurePosixPath
import shutil
import sys
import tempfile
import uuid

from audit_model_roles_246 import audit, _safe_relative
from audit_model_caches_245 import audit as census_audit
from cook_native_models_259 import prepare, no_symlink_ancestors, MAGIC, select_client_executable, probe_client_verifier

FORMAT = 'SUMMIT_NATIVE_MODEL_INCREMENTAL_263_V1'


def _hash(path: Path) -> str:
    h = hashlib.sha256()
    with path.open('rb') as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b''):
            h.update(chunk)
    return h.hexdigest()


def _safe_artifact(rel: str) -> PurePosixPath:
    if (not isinstance(rel, str) or not rel or '\\' in rel or '\x00' in rel or
            ':' in rel or '//' in rel or any(part in ('', '.', '..') for part in rel.split('/'))):
        raise ValueError('unsafe native cache path')
    p = PurePosixPath(rel)
    if p.is_absolute() or p.suffix.lower() not in ('.smd', '.smb'):
        raise ValueError('invalid native cache extension')
    return p


def _revalidate_unchanged(client: Path, source_hashes: dict[str, str]):
    for name, digest in source_hashes.items():
        p = client.joinpath(*_safe_relative(name).parts)
        if p.is_symlink() or not p.is_file() or _hash(p) != digest:
            raise ValueError('ASE modified during incremental conversion: ' + name)


def _previous(out: Path) -> tuple[dict, dict[tuple[str, str], dict]]:
    if not out.exists():
        return {}, {}
    if not out.is_dir() or out.is_symlink():
        raise ValueError('existing output must be a real directory')
    manifest_path = out / 'manifest.json'
    if manifest_path.is_symlink() or not manifest_path.is_file():
        raise ValueError('existing output missing safe manifest')
    old = json.loads(manifest_path.read_text(encoding='utf-8'))
    if old.get('format') != FORMAT or not isinstance(old.get('artifacts'), list):
        raise ValueError('existing output manifest is not a 263 incremental manifest')
    seen: dict[tuple[str, str], dict] = {}
    paths = {'manifest.json'}
    for item in old['artifacts']:
        if not isinstance(item, dict):
            raise ValueError('invalid manifest artifact')
        source = _safe_relative(item['source']).as_posix()
        ext = item['type']
        rel = _safe_artifact(item['cache'])
        if ext not in ('smd', 'smb') or rel != _safe_relative(source).with_suffix('.' + ext):
            raise ValueError('manifest artifact path/type mismatch')
        if (source, ext) in seen or rel.as_posix() in paths:
            raise ValueError('duplicate artifact in manifest')
        if not all(isinstance(item.get(k), str) and len(item[k]) == 64
                   and all(c in '0123456789abcdef' for c in item[k])
                   for k in ('source_sha256', 'sha256')):
            raise ValueError('malformed SHA-256 in manifest')
        seen[(source, ext)] = item
        paths.add(rel.as_posix())
        src = out
        for segment in rel.parts:
            src = src / segment
            if src.is_symlink():
                raise ValueError('symlink within cached artifacts')
    for p in out.rglob('*'):
        if p.is_symlink():
            raise ValueError('symlink inside existing output')
        if p.is_file() and p.relative_to(out).as_posix() not in paths:
            raise ValueError('unexpected extra file in existing output')
    return old, seen


def _artifact_valid(out: Path, row: dict, source_sha: str) -> bool:
    if row.get('source_sha256') != source_sha or row.get('native_readback') is not True or row.get('client_readback') is not True:
        return False
    p = out.joinpath(*_safe_artifact(row['cache']).parts)
    if not p.is_file() or p.is_symlink() or not 64 <= p.stat().st_size <= 512 * 1024 * 1024:
        return False
    with p.open('rb') as f:
        if f.read(len(MAGIC)) != MAGIC:
            return False
    return _hash(p) == row.get('sha256') and p.stat().st_size == row.get('bytes')



def _prepare_one_source_with_timeout_retry(
    client: Path, server: Path, census: Path, roles: Path,
    source_dest: Path, wine: str, src: str, executable: Path,
    client_exe: Path | None,
) -> tuple[dict, Path, bool]:
    """Retry *only* an OS-level timeout, once, in a separate private job.

    A timed-out Wine process may still have outstanding activity. Never read
    from or reuse its working directory; the retry has a different destination
    and prepare() independently copies the original ASE and verifies SMD/SMB.
    Non-timeout failures, including parser/readback errors, remain fail-closed.
    """
    def call(dest: Path) -> dict:
        if client_exe is None:
            return prepare(client, server, census, roles, dest, wine, src)
        return prepare(client, server, census, roles, dest, wine, src, executable, False)

    try:
        return call(source_dest), source_dest, False
    except ValueError as exc:
        # cook_native_models_259.prepare wraps subprocess.TimeoutExpired in
        # ValueError. Do not match arbitrary user-controlled error messages.
        if not isinstance(exc.__cause__, subprocess.TimeoutExpired):
            raise
        if source_dest.exists() or source_dest.is_symlink():
            raise ValueError('timed-out native job unexpectedly published an output; retry refused') from exc
        retry_dest = source_dest.with_name(source_dest.name + '_retry277')
        if retry_dest.exists() or retry_dest.is_symlink():
            raise ValueError('offline retry scratch path occupied') from exc
        # A second timeout propagates immediately. No unbounded retries and no
        # change to the native 180-second per-invocation timeout.
        return call(retry_dest), retry_dest, True


def prepare_incremental(client: Path, server: Path, census: Path, roles: Path, out: Path,
                        wine: str = 'wine', pilot_source: str | None = None, client_exe: Path | None = None) -> dict:
    for p in (client, server, census, roles, out):
        no_symlink_ancestors(p)
    executable = select_client_executable(client, client_exe)
    client = client.resolve(strict=True)
    server = server.resolve(strict=True)
    census = census.resolve(strict=True)
    roles = roles.resolve(strict=True)
    out = out.absolute()
    if not client.is_dir() or not server.is_file() or server.name.casefold() != 'server.exe':
        raise ValueError('client/Game.exe and Server.exe are required')
    if not wine or any(c.isspace() for c in wine):
        raise ValueError('invalid Wine runner')
    if any(part.casefold() == 'canonical' for part in out.parts):
        raise ValueError('canonical output forbidden')
    if not out.parent.is_dir():
        raise ValueError('output parent missing')
    parent = out.parent.resolve(strict=True)
    if parent == client or client in parent.parents or parent == server.parent or server.parent in parent.parents:
        raise ValueError('output must be outside client and server')
    if any(part.casefold() == 'canonical' for part in out.parts):
        raise ValueError('canonical output forbidden')
    # Game + Server binaries and the orchestrator comprise the cache ABI/algorithm identity.
    identity = {
        'server_sha256': _hash(server),
        'client_sha256': _hash(executable),
        'converter_tool_sha256': _hash(Path(__file__).with_name('cook_native_models_259.py')),
    }
    given = json.loads(roles.read_text(encoding='utf-8'))
    if given.get('format') != 'SUMMIT_ASE_ROLE_AUDIT_246_V1':
        raise ValueError('unexpected role report')
    old, old_items = _previous(out)
    if pilot_source is not None:
        pilot_source = _safe_relative(pilot_source).as_posix()
    with tempfile.TemporaryDirectory(prefix='.summit263_', dir=parent) as directory:
        scratch = Path(directory)
        current = audit(client, census, scratch / 'roles_verified.json')
        if current.get('entries') != given.get('entries') or current.get('summary') != given.get('summary'):
            raise ValueError('stale or modified role report')
        hashes = {row['source']: row['source_sha256']
                  for row in json.loads(census.read_text(encoding='utf-8'))['entries']}
        tasks: list[tuple[str, str]] = []
        for entry in current['entries']:
            src = _safe_relative(entry['source']).as_posix()
            exts = entry.get('required_cache_types')
            if not isinstance(exts, list) or sorted(exts) not in (['smd'], ['smb'], ['smb', 'smd']):
                raise ValueError('unclassified/invalid ASE: ' + src)
            for ext in exts:
                tasks.append((src, ext))
        if pilot_source is not None and not any(src == pilot_source for src, _ in tasks):
            raise ValueError('pilot source absent from role audit')
        if old and old.get('identity') != identity and pilot_source is not None:
            raise ValueError('converter version changed; full pass required before pilot update')
        prior_identity_ok = old.get('identity') == identity
        publish = scratch / 'publish'
        publish.mkdir()
        by_source: dict[str, list[str]] = {}
        fresh: list[dict] = []
        converted = 0
        reused = 0
        recovered_timeouts = 0
        # Verify every entry, including old entries not selected by --pilot-source.
        # If an old asset is invalid or stale, it remains omitted until converted;
        # a full pass is required for complete catalog coverage.
        for source, ext in tasks:
            prev = old_items.get((source, ext)) if prior_identity_ok else None
            if prev and _artifact_valid(out, prev, hashes[source]):
                raw = out.joinpath(*_safe_artifact(prev['cache']).parts)
                dest = publish.joinpath(*_safe_artifact(prev['cache']).parts)
                dest.parent.mkdir(parents=True, exist_ok=True)
                shutil.copyfile(raw, dest)
                if _hash(dest) != prev['sha256']:
                    raise ValueError('existing cache modified during copy: ' + source)
                fresh.append(dict(prev))
                reused += 1
            elif pilot_source is None or source == pilot_source:
                by_source.setdefault(source, []).append(ext)
            elif prev is not None:
                # A pilot must not silently drop other, previously published
                # caches whose contents have become stale or corrupted.
                raise ValueError('another published cache needs repair; full pass required')
        # One short offline-entry probe per incremental run, not one per ASE.
        if by_source and client_exe is not None:
            probe_client_verifier(executable, wine, scratch)
        for src in by_source:
            source_dest = scratch / 'generated' / ('job_' + str(len(fresh) + converted))
            source_dest.parent.mkdir(exist_ok=True)
            result, source_dest, recovered = _prepare_one_source_with_timeout_retry(
                client, server, census, roles, source_dest, wine, src,
                executable, client_exe)
            recovered_timeouts += int(recovered)
            for row in result['artifacts']:
                rel = _safe_artifact(row['cache'])
                if (row['source'], row['type']) not in tasks or row['source'] != src:
                    raise ValueError('native tool produced unexpected model')
                from_path = source_dest.joinpath(*rel.parts)
                to_path = publish.joinpath(*rel.parts)
                if to_path.exists():
                    # A given ASE requiring both types is converted in a single call.
                    raise ValueError('duplicate generated artifact')
                to_path.parent.mkdir(parents=True, exist_ok=True)
                shutil.copyfile(from_path, to_path)
                if _hash(to_path) != row['sha256'] or not _artifact_valid(publish, row, hashes[src]):
                    raise ValueError('generated artifact changed before staging')
                fresh.append(dict(row))
                converted += 1
        _revalidate_unchanged(client, hashes)
        fresh.sort(key=lambda row: (row['source'], row['type']))
        report = {'format': FORMAT, 'identity': identity,
                  'scope': 'pilot' if pilot_source is not None else 'all',
                  'artifacts': fresh,
                  'count': len(fresh), 'converted': converted, 'reused': reused,
                  'complete': len(fresh) == len(tasks),
                  'notice': 'Offline staging only. Manual deployment requires native Wine and visual verification.'}
        if old and old.get('identity') == identity and old.get('artifacts') == fresh and converted == 0:
            # Real no-op: no output writes and no timestamp changes.
            return report
        (publish / 'manifest.json').write_text(json.dumps(report, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
        if not out.exists():
            os.rename(publish, out)
        else:
            # Keep the original tree recoverable until the new tree is installed.
            backup = parent / ('.' + out.name + '.backup263_' + uuid.uuid4().hex)
            os.rename(out, backup)
            try:
                os.rename(publish, out)
            except BaseException:
                if not out.exists():
                    os.rename(backup, out)
                raise
            shutil.rmtree(backup)
        return report


def main():
    p = argparse.ArgumentParser(description=__doc__)
    for opt in ('client', 'server', 'out'):
        p.add_argument('--' + opt, type=Path, required=True)
    p.add_argument('--census', type=Path, help='Existing audit 245 JSON; must match --roles')
    p.add_argument('--roles', type=Path, help='Existing audit 246 JSON; must match --census')
    p.add_argument('--wine', default='wine')
    p.add_argument('--pilot-source')
    p.add_argument('--client-exe', type=Path, help='Use a freshly built Game.exe; never copies it into --client')
    args = p.parse_args()
    try:
        if (args.census is None) != (args.roles is None):
            raise ValueError('supply both --census and --roles, or neither')
        if args.census is not None:
            result = prepare_incremental(args.client, args.server, args.census, args.roles,
                                         args.out, args.wine, args.pilot_source, args.client_exe)
        else:
            # Refresh audits automatically each invocation; no stale audit files
            # and no side effects inside client, server or SourceCode/Content.
            with tempfile.TemporaryDirectory(prefix='.summit263_audits_') as tmp:
                census = Path(tmp) / 'census.json'
                roles = Path(tmp) / 'roles.json'
                census_audit(args.client, census)
                audit(args.client, census, roles)
                result = prepare_incremental(args.client, args.server, census, roles,
                                             args.out, args.wine, args.pilot_source, args.client_exe)
    except (ValueError, OSError, TypeError, KeyError, json.JSONDecodeError) as exc:
        print('MODEL_COOK_263=FAIL_CLOSED ' + str(exc), file=sys.stderr)
        return 2
    print('MODEL_COOK_263=PASS converted={converted} reused={reused} total={count} complete={complete} no_client_writes=1'.format(**result))
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
