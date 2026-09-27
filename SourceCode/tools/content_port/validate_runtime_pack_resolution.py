#!/usr/bin/env python3
from __future__ import annotations

import argparse
import hashlib
from pathlib import Path

COMMON_PACKS = (
    'stage_spawns.sspb',
    'text_resources.sres',
    'localization.sloc',
    'gameplay_config.sgcf',
)
ROLE_PACKS = {
    'client': ('definitions_client.scfg', 'maps_client.smap') + COMMON_PACKS,
    'server': ('definitions_server.scfg', 'maps_server.smap') + COMMON_PACKS,
}


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open('rb') as handle:
        for block in iter(lambda: handle.read(1024 * 1024), b''):
            h.update(block)
    return h.hexdigest()


def main() -> int:
    ap = argparse.ArgumentParser(description='Validate standalone executable-relative Summit runtime content deployment')
    ap.add_argument('--repo-root', type=Path, default=Path('.'), help='summit repository root containing SourceCode/, cliente/, Servidor/')
    args = ap.parse_args()
    repo = args.repo_root.resolve()
    source_packs = repo / 'SourceCode' / 'Content' / 'packs'
    errors: list[str] = []

    print(f'REPO_ROOT={repo}')
    print(f'SOURCE_PACK_ROOT={source_packs}')

    for role, runtime_root in (('client', repo / 'cliente'), ('server', repo / 'Servidor')):
        pack_root = runtime_root / 'Content' / 'packs'
        print(f'ROLE={role} RUNTIME_PACK_ROOT={pack_root}')
        for name in ROLE_PACKS[role]:
            source = source_packs / name
            deployed = pack_root / name
            source_exists = source.is_file()
            deployed_exists = deployed.is_file()
            same = source_exists and deployed_exists and sha256(source) == sha256(deployed)
            print(f'PACK role={role} name={name} SOURCE_EXISTS={int(source_exists)} DEPLOYED_EXISTS={int(deployed_exists)} HASH_MATCH={int(same)}')
            if not source_exists:
                errors.append(f'missing source pack: {source}')
            elif not deployed_exists:
                errors.append(f'missing standalone deployed pack: {deployed}')
            elif not same:
                errors.append(f'deployed pack differs from source pack: {deployed}')

    helper = repo / 'SourceCode' / 'Shared' / 'content' / 'v1' / 'runtime' / 'RuntimeContentPath.h'
    if not helper.is_file():
        errors.append(f'missing runtime resolver: {helper}')
    else:
        text = helper.read_text(encoding='utf-8', errors='replace')
        for needle in ('GetModuleFileNameA', 'Content\\\\packs\\\\'):
            if needle not in text:
                errors.append(f'runtime resolver missing standalone contract: {needle}')
        forbidden = ('SourceCode', '..\\\\Content', 'GetCurrentDirectory', 'SUMMIT_')
        for needle in forbidden:
            if needle in text:
                errors.append(f'runtime resolver contains forbidden external/source fallback: {needle}')

    if errors:
        for error in errors:
            print(f'ERROR={error}')
        print('RESULT=FAIL')
        return 1

    print('PACK_RESOLUTION=STANDALONE_EXECUTABLE_CONTENT')
    print('CLIENT_RUNTIME_ROOT=cliente/Content/packs')
    print('SERVER_RUNTIME_ROOT=Servidor/Content/packs')
    print('RESULT=PASS')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
