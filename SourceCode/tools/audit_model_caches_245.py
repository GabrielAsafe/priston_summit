#!/usr/bin/env python3
"""Read-only census of legacy ASE -> SMD/SMB caches for SUMMIT.

The engine uses its own 32-bit binary SaveFile layout. This tool deliberately
DOES NOT pretend that a Python-generated SMD/SMB would be compatible.
"""
from __future__ import annotations
import argparse
import hashlib
import json
from pathlib import Path
import sys


def digest(path: Path) -> str:
    h = hashlib.sha256()
    with path.open('rb') as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b''):
            h.update(chunk)
    return h.hexdigest()


def audit(client: Path, out: Path) -> dict:
    client = client.resolve(strict=True)
    out = out.resolve(strict=False)
    if not client.is_dir() or out == client or client in out.parents:
        raise ValueError('report must be outside client tree')
    if out.is_symlink():
        raise ValueError('output symlink not allowed')
    if not (client / 'Game.exe').is_file():
        raise ValueError('Game.exe missing, wrong client root')

    source_paths = []
    collision_groups: dict[str, list[str]] = {}
    for path in client.rglob('*'):
        if path.is_symlink():
            continue
        if not path.is_file() or path.suffix.lower() != '.ase':
            continue
        rel = path.relative_to(client).as_posix()
        source_paths.append((rel, path))
        collision_groups.setdefault(rel.casefold(), []).append(rel)

    entries = []
    summary = {'sources_ase': 0, 'cache_smd_fresh': 0, 'cache_smd_missing': 0,
               'cache_smd_stale': 0, 'cache_smb_fresh': 0, 'cache_smb_missing': 0,
               'cache_smb_stale': 0, 'ambiguous_case': 0}
    collisions = {k for k, v in collision_groups.items() if len(v) > 1}
    for rel, src in sorted(source_paths, key=lambda x: x[0].casefold()):
        summary['sources_ase'] += 1
        if rel.casefold() in collisions:
            summary['ambiguous_case'] += 1
            entries.append({'source': rel, 'status': 'ambiguous_case',
                            'variants': sorted(collision_groups[rel.casefold()])})
            continue
        item = {'source': rel, 'source_bytes': src.stat().st_size,
                'source_sha256': digest(src), 'cache': {}}
        source_time = src.stat().st_mtime_ns
        for extension in ('smd', 'smb'):
            dest = src.with_suffix('.' + extension)
            if dest.is_symlink():
                status = 'unsafe_symlink'
            elif not dest.is_file():
                status = 'missing'
            elif dest.stat().st_size == 0 or dest.stat().st_mtime_ns < source_time:
                status = 'stale'
            else:
                status = 'fresh_by_mtime_only'
            item['cache'][extension] = {'status': status,
                                        'path': dest.relative_to(client).as_posix()}
            key = f'cache_{extension}_' + ('fresh' if status == 'fresh_by_mtime_only' else status)
            if key in summary:
                summary[key] += 1
        entries.append(item)
    report = {'format': 'SUMMIT_MODEL_CACHE_CENSUS_245_V1', 'counts': summary,
              'warning': 'SMD/SMB use private engine format; mtime does not prove binary compatibility. Each ASE may need only one cache type. No conversion/publication performed.',
              'entries': entries}
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(json.dumps(report, indent=2, ensure_ascii=False) + '\n', 'utf-8')
    return report


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--client', required=True, type=Path)
    parser.add_argument('--out', required=True, type=Path)
    args = parser.parse_args()
    try:
        r = audit(args.client, args.out)
    except (OSError, ValueError) as e:
        print(f'MODEL_CENSUS_245=FAIL_CLOSED: {e}', file=sys.stderr)
        return 2
    print('MODEL_CENSUS_245=PASS ' + ' '.join(f'{k}={v}' for k,v in r['counts'].items()))
    print(f'REPORT={args.out.resolve()}')
    print('NO_WRITES_TO_CLIENT=1 NO_BINARY_COOK=1')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
