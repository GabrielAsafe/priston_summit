#!/usr/bin/env python3
"""Classify ASE mesh/bone cache needs using the engine's GEOMOBJECT/Bip semantics.

Read-only. This is NOT a converter and does not validate the private SMD/SMB
binary format. Rejects a changed client/census instead of using stale evidence.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path, PurePosixPath
import re
import sys

GEOM = re.compile(r'^\*GEOMOBJECT(?:\s|\{|$)')
NODE = re.compile(r'^\*NODE_NAME\s+"([^"]*)"')
NON_GEOM = re.compile(r'^\*(?:HELPEROBJECT|LIGHTOBJECT|CAMERAOBJECT|SHAPEOBJECT|MATERIAL_LIST|SCENE)(?:\s|\{|$)')


def _digest(path: Path) -> str:
    sha = hashlib.sha256()
    with path.open('rb') as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b''):
            sha.update(chunk)
    return sha.hexdigest()


def _safe_relative(value: str) -> PurePosixPath:
    if not value or '\\' in value or '\x00' in value or ':' in value or '//' in value:
        raise ValueError('invalid source spelling')
    if any(segment in ('.', '..', '') for segment in value.split('/')):
        raise ValueError('unsafe source components')
    p = PurePosixPath(value)
    if p.is_absolute() or any(part in ('..', '.', '') for part in p.parts) or p.suffix.lower() != '.ase':
        raise ValueError('unsafe source path')
    return p


def _nodes(path: Path) -> tuple[int, int, int]:
    mesh = bone = unnamed = 0
    looking = False
    lines_after = 0
    with path.open('r', encoding='cp1252', errors='replace') as stream:
        for line in stream:
            part = line.strip()
            if GEOM.match(part):
                if looking:
                    unnamed += 1
                looking = True
                lines_after = 0
                continue
            if not looking:
                continue
            if NON_GEOM.match(part):
                unnamed += 1
                looking = False
                continue
            match = NODE.match(part)
            if match:
                if match.group(1).startswith('Bip'):
                    bone += 1
                else:
                    mesh += 1
                looking = False
            else:
                lines_after += 1
                if lines_after > 64:
                    unnamed += 1
                    looking = False
    if looking:
        unnamed += 1
    return mesh, bone, unnamed


def audit(client: Path, census: Path, output: Path) -> dict:
    client = client.resolve(strict=True)
    if not client.is_dir() or not (client / 'Game.exe').is_file():
        raise ValueError('wrong client root')
    original_out = output.absolute()
    if original_out.is_symlink() or any(p.is_symlink() for p in original_out.parents):
        raise ValueError('output symlink refused')
    output = output.resolve(strict=False)
    if output == census.resolve(strict=True):
        raise ValueError('output cannot overwrite census')
    if output == client or client in output.parents:
        raise ValueError('output must be outside client, without symlinks')
    report = json.loads(census.read_text(encoding='utf-8'))
    if report.get('format') != 'SUMMIT_MODEL_CACHE_CENSUS_245_V1':
        raise ValueError('unexpected census version')
    entries = report.get('entries')
    if not isinstance(entries, list) or len(entries) != report.get('counts', {}).get('sources_ase'):
        raise ValueError('incomplete census')
    seen = set()
    rows = []
    summary = {'ase': 0, 'mesh_only': 0, 'bone_only': 0, 'both': 0,
               'unclassified': 0, 'needs_smd': 0, 'needs_smb': 0,
               'cache_smd_present_unverified': 0, 'cache_smb_present_unverified': 0,
               'legacy_cache_path_over_63': 0}
    for item in entries:
        if item.get('status') == 'ambiguous_case':
            raise ValueError('case ambiguous ASE source; refuse incomplete role classification')
        rel = _safe_relative(item.get('source', ''))
        spelling = rel.as_posix()
        if spelling.casefold() in seen:
            raise ValueError('source collision')
        seen.add(spelling.casefold())
        source = client.joinpath(*rel.parts)
        cursor = client
        for segment in rel.parts:
            cursor = cursor / segment
            if cursor.is_symlink():
                raise ValueError('source symlink')
        if not source.is_file() or _digest(source) != item.get('source_sha256'):
            raise ValueError(f'source changed or missing: {spelling}')
        cache = item.get('cache', {})
        if not all(t in cache for t in ('smd', 'smb')):
            raise ValueError('incomplete cache entry')
        mesh, bone, unnamed = _nodes(source)
        classification = 'both' if mesh and bone else ('mesh_only' if mesh else ('bone_only' if bone else 'unclassified'))
        if unnamed:
            classification = 'unclassified'
        summary[classification] += 1
        summary['ase'] += 1
        if len(spelling.encode('utf-8')) >= 64:
            summary['legacy_cache_path_over_63'] += 1
        requires = []
        for role, needed, typ in (('smd', bool(mesh), 'geometry'), ('smb', bool(bone), 'bones')):
            status = cache[role].get('status')
            if status not in ('missing', 'stale', 'fresh_by_mtime_only'):
                raise ValueError(f'unsafe or unknown {role} cache status')
            if needed and classification != 'unclassified':
                requires.append(role)
                if status == 'fresh_by_mtime_only':
                    summary[f'cache_{role}_present_unverified'] += 1
                else:
                    summary[f'needs_{role}'] += 1
        rows.append({'source': spelling, 'role': classification, 'mesh_objects': mesh,
                     'bone_objects': bone, 'unnamed_objects': unnamed,
                     'required_cache_types': requires,
                     'smd': cache['smd']['status'], 'smb': cache['smb']['status']})
    result = {'format': 'SUMMIT_ASE_ROLE_AUDIT_246_V1', 'summary': summary,
              'notice': 'Analysis of ASE GEOMOBJECT names is not proof of runtime use or SMD/SMB binary correctness. No files in client modified.',
              'entries': rows}
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(result, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    return result


def main() -> int:
    cli = argparse.ArgumentParser(description=__doc__)
    cli.add_argument('--client', type=Path, required=True)
    cli.add_argument('--census', type=Path, required=True)
    cli.add_argument('--out', type=Path, required=True)
    args = cli.parse_args()
    try:
        result = audit(args.client, args.census, args.out)
    except (OSError, ValueError, KeyError, TypeError, json.JSONDecodeError) as exc:
        print('ROLE_AUDIT_246=FAIL_CLOSED ' + str(exc), file=sys.stderr)
        return 2
    print('ROLE_AUDIT_246=PASS ' + ' '.join(f'{k}={v}' for k, v in result['summary'].items()))
    print('NO_CLIENT_WRITES=1 NO_BINARY_COOK=1')
    return 0

if __name__ == '__main__':
    raise SystemExit(main())
