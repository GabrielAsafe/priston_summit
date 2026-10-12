#!/usr/bin/env python3
"""Audit effect/particle/skill source images against cooked DDS (READ-ONLY).

The PATCH 243 global cook already targets all known image extensions. This
checker identifies effect assets within that inventory, excluded images, and
non-image scripts/models that DDS cooking cannot replace. On-disk DDS presence
is NOT proof of a COOKED_DDS_HIT in a running game.
"""
from __future__ import annotations

import argparse
from collections import Counter
import json
from pathlib import Path
import sys

import cook_all_missing_243 as global_cook
import cook_textures_dds as texture

FORMAT = 'SUMMIT_EFFECT_COOKED_244_AUDIT_V1'
EFFECT_NAMES = ('effect', 'effects', 'particle', 'particles', 'skill', 'skills',
                'assaeffect', 'spells', 'spell', 'magic', 'magics')
SCRIPT_EXTENSIONS = {'.ini', '.lua', '.txt', '.eff', '.spt', '.ps', '.pfx', '.dat'}
MODEL_EXTENSIONS = {'.ase', '.smd', '.obj', '.3ds'}


def effect_path(path: str) -> bool:
    """Select attack/particle effect families by path *components*."""
    parts = [part.casefold() for part in path.replace('\\', '/').split('/')]
    return any(part in EFFECT_NAMES for part in parts[:-1])


def get_effect_non_images(client: Path) -> list[dict]:
    rows = []
    for base, dirs, names in __import__('os').walk(client, followlinks=False):
        directory = Path(base)
        if directory == client:
            dirs[:] = [d for d in dirs if d.casefold() not in global_cook.EXCLUDED_ROOTS and
                       not (directory / d).is_symlink()]
        else:
            dirs[:] = [d for d in dirs if not (directory / d).is_symlink()]
        for filename in names:
            source = directory / filename
            if source.is_symlink() or not source.is_file():
                continue
            logical = source.relative_to(client).as_posix()
            if not effect_path(logical):
                continue
            extension = source.suffix.casefold()
            if extension in SCRIPT_EXTENSIONS:
                kind = 'effect_script_or_data'
            elif extension in MODEL_EXTENSIONS:
                kind = 'effect_model'
            else:
                continue
            rows.append({'source': logical, 'category': kind})
    return sorted(rows, key=lambda x: x['source'].casefold())


def audit(client: Path) -> dict:
    client = client.resolve(strict=True)
    if not client.is_dir() or (client / 'cooked').is_symlink():
        raise ValueError('client missing or cooked symlink')
    sources, unsafe = global_cook.discover(client)
    plan, exclusions, counts = global_cook.prepare_plan(client)
    missing = {row['source'] for row in plan}
    excluded = {row['source']: row for row in exclusions}
    existing = global_cook.existing_dds(client / 'cooked')
    rows = []
    tally = Counter()
    for rel in sources:
        if not effect_path(rel):
            continue
        profile = texture.cook_profile_for(Path(rel))
        target = texture.cooked_relative_path(rel, profile).as_posix()
        if rel in excluded:
            status = excluded[rel]['reason']
        elif rel in missing:
            status = 'missing_dds'
        else:
            candidates = existing.get(target.casefold(), [])
            if len(candidates) != 1:
                status = 'unverifiable_dds'
            else:
                # prepare_plan checks structural header and freshness globally.
                status = 'dds_present'
        tally[status] += 1
        rows.append({'source': rel, 'dds': target, 'status': status, 'profile': profile})
    nonimage = get_effect_non_images(client)
    nonimage_counts = Counter(row['category'] for row in nonimage)
    return {'format': FORMAT, 'client': str(client),
            'global_243_summary': counts,
            'effect_image_counts': dict(sorted(tally.items())),
            'effect_image_sources': len(rows),
            'effect_images': rows,
            'effect_nonimages_counts': dict(sorted(nonimage_counts.items())),
            'effect_nonimages': nonimage,
            'unsafe_image_paths': [row for row in unsafe if effect_path(row['source'])],
            'scope_note': 'DDS on disk is not proof the game used it; effect scripts and models are not DDS images.'}


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--client', required=True, type=Path)
    parser.add_argument('--out', required=True, type=Path)
    args = parser.parse_args(argv)
    try:
        client = args.client.resolve(strict=True)
        output = args.out.resolve()
        if output == client or client in output.parents or output in client.parents:
            raise ValueError('output report must be outside the client tree')
        result = audit(client)
        output.parent.mkdir(parents=True, exist_ok=True)
        # No client writes. Audit only, not cook/publish.
        global_cook.report_write(output, result)
        counts = result['effect_image_counts']
        print('EFFECT_AUDIT=PASS images={images} present={present} missing={missing} excluded={excluded} nonimages={other}'.format(
            images=result['effect_image_sources'],
            present=counts.get('dds_present', 0),
            missing=counts.get('missing_dds', 0),
            excluded=sum(v for k, v in counts.items() if k not in ('dds_present', 'missing_dds')),
            other=len(result['effect_nonimages'])))
        print(f'REPORT={output}')
        print('NOTE=presence_not_runtime_hit;_models_and_scripts_not_DDS')
        return 0
    except (OSError, ValueError, TypeError) as exc:
        print(f'EFFECT_AUDIT=FAIL_CLOSED {type(exc).__name__}: {exc}', file=sys.stderr)
        return 2


if __name__ == '__main__':
    raise SystemExit(main())
