#!/usr/bin/env python3
from pathlib import Path
import re, sys

ROOT = Path(__file__).resolve().parents[1]
errors = []
checks = []

def text(rel):
    p = ROOT / rel
    if not p.exists():
        errors.append(f"missing: {rel}")
        return ""
    return p.read_text(encoding='utf-8', errors='replace')

def require(rel, needle, label=None):
    s = text(rel)
    ok = needle in s
    checks.append((ok, label or f"{rel}: {needle}"))
    if not ok: errors.append(label or f"missing contract {needle!r} in {rel}")

def forbid(rel, needle, label=None):
    s = text(rel)
    ok = needle not in s
    checks.append((ok, label or f"{rel}: no {needle}"))
    if not ok: errors.append(label or f"forbidden contract {needle!r} in {rel}")

# Scheduler / build
require('SrcGame/src/core/concurrency/JobSystem.cpp', 'std::thread::hardware_concurrency()', 'JobSystem scales from hardware concurrency')
require('SrcGame/src/core/concurrency/JobSystem.cpp', 'SUMMIT_WORKER_THREADS', 'JobSystem worker override exists')
require('build_client_mingw_i686_multicore.sh', '-pthread', 'compile enables pthread')
require('link_client_mingw_i686.sh', '-pthread', 'link enables pthread')

# Texture split CPU worker -> GPU pump
tex = 'Shared/rendering/legacy_3d/material/smTexture_legacy_client.inl'
require(tex, 'AssetLoadV3RequestTexture', 'async texture request API')
require(tex, 'AssetLoadV3PumpTextureUploads', 'render-thread texture completion pump')
require(tex, 'SUMMIT_TEXTURE_STAGING_BUDGET_MB', 'bounded texture staging memory')
require(tex, 'SUMMIT_TEXTURE_UPLOAD_BUDGET_US', 'bounded GPU upload time')
require(tex, 'AssetLoadV3ResetAsyncState', 'texture async session reset')

# Stage path must enqueue worker work, not invoke the old faux loader.
play = 'SrcGame/src/core/gameplay/runtime/playmain.cpp'
require(play, 'RequestStageFromFieldAsync', 'stage async request API')
require(play, 'PumpStageStreamingV3', 'stage completion pump')
require(play, 'SummitCapturePendingFieldTransitionSetPosi', 'warp position is deferred until stage readiness')
require(play, 'JobPriority::Low', 'adjacent stage uses background priority')
require(play, 'JobPriority::High', 'needed stage/asset work has high priority')
require(play, 'SummitFinalizeBootstrapPlayerPlacement', 'bootstrap snaps player only after collision geometry publication')
require(play, 'IsStageBootstrapBlockingGameplayAsync', 'bootstrap exposes an explicit gameplay barrier')
require('SrcGame/src/platform/windows/application/Winmain.cpp', 'PumpStageStreamingV3();', 'stage completions pump even while gameplay is gated')
require('SrcGame/src/platform/windows/application/Winmain.cpp', 'Never draw the player (or any world-space effect)', 'world render is gated until primary geometry exists')


# Bootstrap invariant: logical field metadata must remain valid while geometry is async.
require(play, 'Speculative neighbour requests must not race the primary bootstrap', 'adjacent preload waits for primary geometry')
require(play, 'Keep OnStageField bound to that metadata until geometry is published', 'bootstrap preserves logical field in InitPat3D')
require('Shared/gameplay/character/character_legacy_client.inl',
        'Do not erase the logical\n\t// field or simulate a fall to the -80 fallback plane while no stage exists.',
        'movement preserves logical field while geometry is pending')

# Gameplay model and equipment paths are request based.
model = 'SrcGame/src/systems/gameplay/character/core/CharacterModelStreamingV3.cpp'
require(model, 'SummitRequestCharacterModelV3', 'character model async request API')
require(model, 'SummitRequestToolModelV3', 'tool model async request API')
require(model, 'SummitPumpModelStreamingV3', 'model completion pump')
require(model, 'SummitResetModelStreamingV3', 'model session reset')
require('Shared/gameplay/character/character_legacy_client.inl', 'SummitAcquireCachedModelV3', 'SetTool uses cache-only fast path')

# Sky is no longer parsed synchronously by the active implementation.
sky = 'SrcGame/src/systems/rendering/sky/legacy/HoSky.cpp'
require(sky, 'SummitRequestSkyModelV3', 'sky schedules background mesh loads')
forbid(sky, 'smASE_Read(', 'active sky implementation has no synchronous ASE read')
require('SrcGame/src/platform/windows/application/Winmain.cpp', 'SummitPumpSkyStreamingV3', 'sky completions pumped on owner thread')

# Parser scratch must be thread-local before parallel parse.
require('Shared/rendering/legacy_3d/smRead3d_legacy_client.inl', 'thread_local', 'ASE parser scratch isolated per worker')
require('Shared/formats/legacy_loader/fileread_legacy_client.inl', 'thread_local', 'legacy file parser scratch isolated per worker')

# Shutdown/session ordering: drain workers before releasing resource owners.
win = text('SrcGame/src/platform/windows/application/Winmain.cpp')
pos_wait = win.find('JobSystem::Instance().WaitIdle()')
pos_close = win.find('ClosePat3D()')
ok = pos_wait >= 0 and pos_close >= 0 and pos_wait < pos_close
checks.append((ok, 'GameClose drains workers before pattern teardown'))
if not ok: errors.append('GameClose must WaitIdle before ClosePat3D')

for ok, label in checks:
    print(('PASS' if ok else 'FAIL') + '  ' + label)
print(f"RESULT={'PASS' if not errors else 'FAIL'} checks={len(checks)} errors={len(errors)}")
if errors:
    for e in errors:
        print('ERROR ' + e)
    sys.exit(1)
