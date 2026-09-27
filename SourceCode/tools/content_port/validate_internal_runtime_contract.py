#!/usr/bin/env python3
from __future__ import annotations

import argparse
from pathlib import Path

RUNTIME_FILES = (
    'Shared/content/v1/definition/content/DefinitionRuntime.h',
    'Shared/content/v1/text_resource/content/TextResourceRuntime.h',
    'Shared/content/v1/localization/content/LocalizationRuntime.h',
    'Shared/content/v1/gameplay_config/content/GameplayConfigRuntime.h',
    'Shared/content/v1/stage_spawn/legacy/LegacyStageSpawnAdapter.h',
    'Shared/world/field/field_legacy_client.inl',
    'Shared/world/field/field_legacy_server.inl',
)


def main() -> int:
    ap = argparse.ArgumentParser(description='Validate that compiled-content pack selection is internal to the client/server binaries')
    ap.add_argument('--source-root', type=Path, default=Path('.'))
    args = ap.parse_args()
    root = args.source_root.resolve()
    errors: list[str] = []

    texts: dict[str, str] = {}
    for rel in RUNTIME_FILES:
        path = root / rel
        if not path.is_file():
            errors.append(f'missing runtime file: {rel}')
            continue
        text = path.read_text(encoding='utf-8', errors='surrogateescape')
        texts[rel] = text
        if 'getenv("SUMMIT_' in text or "getenv('SUMMIT_" in text:
            errors.append(f'content activation still reads SUMMIT environment variables: {rel}')

    definition = texts.get('Shared/content/v1/definition/content/DefinitionRuntime.h', '')
    for needle in ('definitions_client.scfg', 'definitions_server.scfg', 'CompiledItemDefinitionsEnabled(){return true;}'):
        if needle not in definition:
            errors.append(f'definition runtime contract missing: {needle}')

    client_field = texts.get('Shared/world/field/field_legacy_client.inl', '')
    server_field = texts.get('Shared/world/field/field_legacy_server.inl', '')
    if 'ResolveRuntimePackPath("maps_client.smap")' not in client_field:
        errors.append('client map runtime does not resolve maps_client.smap internally')
    if 'ResolveRuntimePackPath("maps_server.smap")' not in server_field:
        errors.append('server map runtime does not resolve maps_server.smap internally')

    stage = texts.get('Shared/content/v1/stage_spawn/legacy/LegacyStageSpawnAdapter.h', '')
    if 'ResolveRuntimePackPath("stage_spawns.sspb")' not in stage:
        errors.append('stage-spawn runtime does not resolve stage_spawns.sspb internally')

    expected = {
        'Shared/content/v1/text_resource/content/TextResourceRuntime.h': 'ResolveRuntimePackPath("text_resources.sres")',
        'Shared/content/v1/localization/content/LocalizationRuntime.h': 'ResolveRuntimePackPath("localization.sloc")',
        'Shared/content/v1/gameplay_config/content/GameplayConfigRuntime.h': 'ResolveRuntimePackPath("gameplay_config.sgcf")',
    }
    for rel, needle in expected.items():
        if needle not in texts.get(rel, ''):
            errors.append(f'internal pack path missing in {rel}: {needle}')

    sh = root / 'tools/use_compiled_content.sh'
    if sh.is_file():
        text = sh.read_text(encoding='utf-8', errors='replace')
        if 'export SUMMIT_' in text:
            errors.append('compatibility shell helper still exports content activation variables')
        if 'activation is built into the client/server binaries' not in text:
            errors.append('compatibility shell helper does not declare internal activation')

    resolver = root / 'Shared/content/v1/runtime/RuntimeContentPath.h'
    if not resolver.is_file():
        errors.append('missing standalone runtime resolver')
    else:
        resolver_text = resolver.read_text(encoding='utf-8', errors='replace')
        if 'SourceCode' in resolver_text or '..\\\\Content' in resolver_text:
            errors.append('runtime resolver still contains a source/external fallback')
        if 'Content\\\\packs\\\\' not in resolver_text or 'GetModuleFileNameA' not in resolver_text:
            errors.append('runtime resolver is not executable-relative standalone content')

    print(f'SOURCE_ROOT={root}')
    print(f'RUNTIME_FILES_CHECKED={len(RUNTIME_FILES)}')
    if errors:
        for error in errors:
            print(f'ERROR={error}')
        print('RESULT=FAIL')
        return 1
    print('CONTENT_ACTIVATION_ENV_READS=0')
    print('PACK_RESOLUTION=STANDALONE_EXECUTABLE_CONTENT')
    print('CLIENT_DEFINITION_PACK=definitions_client.scfg')
    print('SERVER_DEFINITION_PACK=definitions_server.scfg')
    print('RESULT=PASS')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
