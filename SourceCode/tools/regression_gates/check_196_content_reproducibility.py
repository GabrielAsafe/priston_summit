#!/usr/bin/env python3
from __future__ import annotations

import hashlib
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

root = Path(__file__).resolve().parents[2]
canonical = root / 'Content/canonical/text'
packs = root / 'Content/packs'
tools = root / 'tools/content_port'


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open('rb') as f:
        for block in iter(lambda: f.read(1024 * 1024), b''):
            h.update(block)
    return h.hexdigest()


def run(*args: object) -> None:
    subprocess.run([str(x) for x in args], check=True, stdout=subprocess.DEVNULL)


with tempfile.TemporaryDirectory(prefix='summit_content_repro_') as td:
    temp = Path(td)
    temp_canonical = temp / 'text'
    temp_packs = temp / 'packs'
    shutil.copytree(canonical, temp_canonical, ignore=shutil.ignore_patterns('localization'))

    run(sys.executable, tools / 'materialize_localization_catalog.py', temp_canonical)
    run(sys.executable, tools / 'compile_localization.py', temp_canonical, temp_packs / 'localization.sloc')
    run(sys.executable, tools / 'compile_text_resources.py', temp_canonical, temp_packs / 'text_resources.sres')
    run(sys.executable, tools / 'compile_definitions.py', temp_canonical, temp_packs / 'definitions.scfg')

    generated = [
        *(f'localization/{name}' for name in ('en.json','pt-BR.json','zh-TW.json','ko.json','bindings.json','index.json','report.json')),
    ]
    errors: list[str] = []
    for rel in generated:
        expected = canonical / rel
        actual = temp_canonical / rel
        if not expected.is_file():
            errors.append(f'missing checked-in generated catalog: {expected.relative_to(root)}')
            continue
        if expected.read_bytes() != actual.read_bytes():
            errors.append(
                f'generated catalog drift: {expected.relative_to(root)} '
                f'checked={sha256(expected)} rebuilt={sha256(actual)}'
            )

    for name in ('localization.sloc','text_resources.sres','definitions.scfg'):
        for suffix in ('', '.report.json'):
            rel = name + suffix
            expected = packs / rel
            actual = temp_packs / rel
            if not expected.is_file():
                errors.append(f'missing checked-in runtime artifact: {expected.relative_to(root)}')
                continue
            if expected.read_bytes() != actual.read_bytes():
                errors.append(
                    f'runtime artifact drift: {expected.relative_to(root)} '
                    f'checked={sha256(expected)} rebuilt={sha256(actual)}'
                )

    if errors:
        print('\n'.join('ERROR: ' + e for e in errors), file=sys.stderr)
        raise SystemExit(1)

print('PATCH 196 gate: OK — canonical content regenerates localization/SLOC/SRES/SCFG byte-for-byte')
