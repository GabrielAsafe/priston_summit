#!/usr/bin/env python3
from __future__ import annotations

import argparse
import hashlib
import json
import subprocess
import sys
from pathlib import Path


def run(*args: object) -> None:
    print('+', ' '.join(str(x) for x in args), flush=True)
    subprocess.run([str(x) for x in args], check=True)




def portable_input_path(path: Path, cwd: Path) -> str:
    resolved = path.resolve()
    try:
        return resolved.relative_to(cwd.resolve()).as_posix()
    except ValueError:
        return resolved.name


def portable_output_path(path: Path, out_root: Path) -> str:
    return path.resolve().relative_to(out_root.resolve()).as_posix()

def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open('rb') as f:
        for block in iter(lambda: f.read(1024 * 1024), b''):
            h.update(block)
    return h.hexdigest()


def main() -> None:
    ap = argparse.ArgumentParser(
        description='Compile all Summit runtime content packs using canonical JSON only'
    )
    ap.add_argument('--canonical-text', type=Path, required=True)
    ap.add_argument('--canonical-spawns', type=Path, required=True)
    ap.add_argument('--canonical-maps', type=Path, required=True)
    ap.add_argument('--out-root', type=Path, required=True)
    args = ap.parse_args()

    here = Path(__file__).resolve().parent
    py = sys.executable
    text = args.canonical_text.resolve()
    spawns = args.canonical_spawns.resolve()
    maps = args.canonical_maps.resolve()
    out = args.out_root.resolve()
    invocation_root = Path.cwd().resolve()
    packs = out / 'packs'
    packs.mkdir(parents=True, exist_ok=True)

    run(py, here / 'validate_canonical_text.py', text)
    run(py, here / 'validate_canonical_schemas.py', '--canonical-text', text, '--canonical-spawns', spawns, '--canonical-maps', maps)

    outputs = {
        'text_resources': packs / 'text_resources.sres',
        'definitions': packs / 'definitions.scfg',
        'localization': packs / 'localization.sloc',
        'gameplay_config': packs / 'gameplay_config.sgcf',
        'stage_spawns': packs / 'stage_spawns.sspb',
        'maps_client': packs / 'maps_client.smap',
        'maps_server': packs / 'maps_server.smap',
    }

    run(py, here / 'compile_text_resources.py', text, outputs['text_resources'])
    run(py, here / 'inspect_text_resources.py', outputs['text_resources'])
    run(py, here / 'compile_definitions.py', text, outputs['definitions'])
    run(py, here / 'inspect_definitions.py', outputs['definitions'])
    run(py, here / 'compile_localization.py', text, outputs['localization'])
    run(py, here / 'inspect_localization.py', outputs['localization'])
    run(py, here / 'compile_gameplay_config.py', text, outputs['gameplay_config'])
    run(py, here / 'inspect_gameplay_config.py', outputs['gameplay_config'])
    run(py, here / 'compile_stage_spawns_canonical.py', spawns, outputs['stage_spawns'])
    run(py, here / 'inspect_stage_spawns.py', outputs['stage_spawns'])

    for profile in ('client', 'server'):
        catalog = maps / f'{profile}.maps.json'
        if not catalog.is_file():
            raise SystemExit(f'missing canonical map catalog: {catalog}')
        output = outputs[f'maps_{profile}']
        run(py, here / 'compile_maps.py', catalog, output)
        run(py, here / 'inspect_maps.py', output)

    manifest = {
        'schema': 'summit.canonical-content-build/v1',
        'inputs': {
            'canonical_text': portable_input_path(text, invocation_root),
            'canonical_spawns': portable_input_path(spawns, invocation_root),
            'canonical_maps': portable_input_path(maps, invocation_root),
        },
        'packs': {
            name: {
                'path': portable_output_path(path, out),
                'bytes': path.stat().st_size,
                'sha256': sha256(path),
            }
            for name, path in outputs.items()
        },
    }
    (out / 'BUILD.json').write_text(
        json.dumps(manifest, indent=2, ensure_ascii=False) + '\n', encoding='utf-8'
    )
    print('CANONICAL_BUILD=PASS')


if __name__ == '__main__':
    main()
