#!/usr/bin/env python3
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]

def read(rel):
    return (ROOT / rel).read_text(errors='ignore')

def require(rel, *tokens):
    text = read(rel)
    missing = [t for t in tokens if t not in text]
    if missing:
        raise SystemExit(f"FAIL {rel}: missing {missing}")

require('Shared/rendering/assets/v2/AssetLoadRegistry.h',
        'ASSET_UNLOADED', 'ASSET_LOADING', 'ASSET_LOADED',
        'ASSET_FAILED_TRANSIENT', 'ASSET_FAILED_PERMANENT', 'ASSET_WAITING_DEVICE', 'ASSET_EVICTED',
        'maxTransientFailures = 3u', 'residentBudgetBytes = 1024ull * 1024ull * 1024ull',
        'maxRuntimeDimension = 4096u', 'maxExternalRuntimeDimension = 2048u',
        'globalPrewarmLimit = 32u', 'scopedPrewarmLimit = 32u',
        'MarkWaitingForDevice', 'SelectEvictionCandidate', 'ComputeTargetDimensions')

require('Shared/rendering/legacy_3d/material/smTexture_legacy_client.inl',
        'SUMMIT_TEXTURE_BUDGET_MB', 'SUMMIT_TEXTURE_MAX_DIM',
        'SUMMIT_EXTERNAL_TEXTURE_MAX_DIM', 'SUMMIT_TEXTURE_PREWARM_LIMIT',
        'SUMMIT_TEXTURE_SCOPED_PREWARM_LIMIT',
        'AssetLoadV2GetResidentTexture', 'AssetLoadV2ReadMaterialGroup',
        'AssetLoadV2TrimToBudget', 'AssetLoadV2ReleaseMaterial',
        'IMPLAUSIBLE_DIMENSIONS', 'OFFSCREEN_IMPLAUSIBLE_DIMENSIONS',
        'DECODE_BUFFER_OOM_RETRY_LIMIT', 'new (std::nothrow) char[iSize]',
        'D3DFMT_DXT1', 'D3DFMT_DXT5', 'if (MipMap == true)',
        'return smTEXFILETYPE_PNG;', 'prewarmLimit', 'scopedPrewarmLimit',
        'WAIT_DEVICE', 'D3DERR_DEVICELOST', 'D3DERR_DEVICENOTRESET',
        'BUDGET_OVERSHOOT')

link_script = ROOT / 'link_client_mingw_i686.sh'
if link_script.is_file():
    require('link_client_mingw_i686.sh', '-Wl,--large-address-aware')
else:
    print('Asset Load System V2 native-link check: SKIP (link_client_mingw_i686.sh omitted from source snapshot)')
require('SrcGame/src/core/gameplay/runtime/playmain.cpp',
        'RequestStageFromFieldAsync(lpField, lpSecondField)',
        'void PumpStageTexturePrewarm()',
        'AssetLoadV2PrewarmMaterialGroupStep(',
        'AssetLoadV3SetLoadingBoost(true)')
require('Shared/rendering/effects/legacy_new/HoEffectResMgrLegacyCommon.inl',
        '#ifndef _SERVER', 'AssetLoadV2ReleaseMaterial', 'AssetLoadV2ReadMaterialGroup')
require('Shared/rendering/effects/legacy_sin/AssaUtil_legacy_client.inl',
        'AssetLoadV2ReleaseMaterial', 'AssaResCount = 0;', 'AssetLoadV2DumpTelemetry("ASSARES_CLEAR")')
require('Shared/rendering/legacy_3d/SmStageObjectLegacyCommon.inl',
        'ZeroMemory(mObj, sizeof(mObj));',
        'mObj[cnt].BipPattern && mObj[cnt].BipPattern != mObj[cnt].Pattern',
        'mObj[nObj].BipPattern = 0;')

# Stage loaders must no longer perform a process-wide ReadTextures() inside the
# two stage object prewarm blocks.
play = read('SrcGame/src/core/gameplay/runtime/playmain.cpp')
for marker in ('smSTAGE3D *LoadStageFromField(', 'smSTAGE3D *LoadStageFromField2('):
    start = play.find(marker)
    if start < 0:
        raise SystemExit(f'FAIL playmain: missing {marker}')
    end = play.find('\n}', start)
    block = play[start:end]
    if re.search(r'(?m)^\s*ReadTextures\s*\(\s*\)\s*;', block):
        raise SystemExit(f'FAIL playmain: global ReadTextures remains in {marker}')

# Production client code must not bypass demand residency by dereferencing
# smTEXTUREHANDLE::lpD3DTexture directly. Internal loader implementation and
# comments in the renderer are allowed.
violations = []
for base in (ROOT / 'SrcGame', ROOT / 'Shared'):
    for path in base.rglob('*'):
        if not path.is_file():
            continue
        rel = path.relative_to(ROOT).as_posix()
        low = rel.lower()
        if 'legacy_oracle/' in rel or 'server' in low:
            continue
        if rel == 'Shared/rendering/legacy_3d/material/smTexture_legacy_client.inl':
            continue
        try:
            lines = path.read_text(errors='ignore').splitlines()
        except OSError:
            continue
        for lineno, line in enumerate(lines, 1):
            if 'lpD3DTexture' not in line:
                continue
            stripped = line.strip()
            if rel.endswith('/smTexture.h') or rel.endswith('/smType.h'):
                continue
            if stripped.startswith('//'):
                continue
            violations.append(f'{rel}:{lineno}:{stripped}')

if violations:
    print('FAIL direct legacy texture residency bypasses:', file=sys.stderr)
    print('\n'.join(violations[:80]), file=sys.stderr)
    raise SystemExit(1)

print('Asset Load System V2 contracts: PASS')
