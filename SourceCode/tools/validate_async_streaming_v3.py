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

def require_if_present(rel, needle, label=None):
    p = ROOT / rel
    if not p.exists():
        print('SKIP  ' + (label or rel) + f' ({rel} omitted from source snapshot)')
        return
    require(rel, needle, label)

# Scheduler / build
require('SrcGame/src/core/concurrency/JobSystem.cpp', 'std::thread::hardware_concurrency()', 'JobSystem scales from hardware concurrency')
require('SrcGame/src/core/concurrency/JobSystem.cpp', 'SUMMIT_WORKER_THREADS', 'JobSystem worker override exists')
require_if_present('build_client_mingw_i686_multicore.sh', '-pthread', 'compile enables pthread')
require_if_present('link_client_mingw_i686.sh', '-pthread', 'link enables pthread')

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
require(play, 'return gStageBootstrapPending;', 'bootstrap barrier remains active through local visual readiness')
require(play, 'SummitPrepareBootstrapVisualSet', 'bootstrap prioritizes spawn-local stage textures')
require(play, 'SUMMIT_STAGE_BOOTSTRAP_LOCAL_TEXTURES', 'bootstrap local texture set is bounded/tunable')
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


require(play, 'return gStageBootstrapPending;', 'bootstrap barrier spans the full first-world readiness contract')
require(play, 'gStageBootstrapSpawnResolved', 'bootstrap tracks an explicit resolved spawn surface')
require(play, 'SummitResolveBootstrapSpawnSurface', 'bootstrap resolves spawn Y from stage geometry')
require(play, 'const int legacyHeight = stage.GetHeight(x, z);', 'bootstrap spawn Y reuses legacy GetHeight semantics')
require(play, '!gStageBootstrapSpawnResolved', 'bootstrap fails closed while spawn surface is unresolved')
require(play, 'floorHeight, floorFace', 'spawn visual selection shares the resolved floor surface')
require('Shared/rendering/legacy_3d/material/smTexture_legacy_client.inl', 'prepared->priority = priority;', 'texture GPU completion preserves request priority')
require('Shared/rendering/legacy_3d/material/smTexture_legacy_client.inl', 'SummitPreparedTextureQueue.insert(position, prepared);', 'GPU upload queue lets HIGH demand overtake LOW prewarm')
require('Shared/rendering/legacy_3d/material/smTexture_legacy_client.inl', 'SummitTextureAdmissionLimit(priority)', 'HIGH texture demand has reserved worker admission')
require(play, 'if (request->field && !bootstrapPrimary)', 'bootstrap queues critical stage textures before global field-map prewarm')

# Stage/prewarm has one frame-loop owner; FieldMain must not double-pump.
win_text = text('SrcGame/src/platform/windows/application/Winmain.cpp')
playd3d = win_text.find('void PlayD3D()')
first_gate = win_text.find('if (IsStageBootstrapBlockingGameplayAsync())', playd3d)
stage_pump = win_text.find('PumpStageStreamingV3();', playd3d)
prewarm_pump = win_text.find('PumpStageTexturePrewarm();', playd3d)
ok = playd3d >= 0 and first_gate >= 0 and stage_pump >= 0 and prewarm_pump >= 0 and stage_pump < first_gate and prewarm_pump < first_gate
checks.append((ok, 'frame loop advances stage/prewarm before gameplay barrier'))
if not ok: errors.append('PlayD3D must pump stage publication/prewarm before checking the bootstrap gameplay barrier')

field_text = text('Shared/world/field/field_legacy_client.inl')
field_main = field_text.find('int FieldMain(')
field_end = field_text.find('int\tWingWarpGate_Field', field_main)
field_body = field_text[field_main:field_end if field_end >= 0 else field_main + 1500]
ok = 'PumpStageStreamingV3();' not in field_body and 'PumpStageTexturePrewarm();' not in field_body
checks.append((ok, 'FieldMain no longer double-pumps stage streaming'))
if not ok: errors.append('FieldMain must not own stage streaming pumps')

# Bootstrap spawn coordinates are captured from the field warp, not from a
# partially initialized character object. Texture readiness distinguishes
# permanent asset failure from work that can still complete.
require(play, 'gStageBootstrapSpawnCoordinatesReady', 'bootstrap captures stable spawn coordinates')
require(play, 'gStageBootstrapSpawnX = smConfig.Posi_Player.x;', 'bootstrap captures warp-selected spawn X')
require(play, 'gStageBootstrapSpawnZ = smConfig.Posi_Player.y;', 'bootstrap captures warp-selected spawn Z')
require(play, 'stage->Head = TRUE;', 'async stage worker publishes legacy Head readiness')
require('SrcGame/src/systems/rendering/assets/v3/AssetStreamingV3.h', 'AssetLoadV3GetTextureReadiness', 'texture streaming exposes terminal readiness')
require(play, 'ASSET_LOAD_V3_TEXTURE_FAILED_PERMANENT', 'bootstrap cannot deadlock on permanently failed floor texture')

# Offline DDS cooker / cooked-texture fast path. Logical asset names stay
# unchanged; the worker resolves cooked\<logical>.dds and the render thread
# imports the precompressed DDS without source decode/recompression.
require(tex, 'smTEXFILETYPE_DDS', 'legacy texture type system recognizes DDS')
require(tex, 'SUMMIT_TEXTURE_COOKED', 'cooked DDS path has an A/B environment switch')
require(tex, 'SUMMIT_TEXTURE_COOKED_ROOT', 'cooked DDS root is configurable')
require(tex, 'SummitBuildCookedTexturePath', 'V3 resolves cooked DDS from logical texture paths')
require(tex, 'cookedStat.st_mtime < sourceStat.st_mtime', 'stale cooked DDS falls back to newer source')
require(tex, 'prepared->cookedDDS = true;', 'worker marks cooked DDS payloads explicitly')
require(tex, 'D3DX_FILTER_NONE', 'cooked DDS fast path avoids runtime resampling filters')
require(tex, 'COOKED_DDS_HIT', 'cooked DDS hit telemetry exists')
require(tex, 'SummitRejectCookedTexture', 'invalid cooked DDS falls back to source on retry')
require('tools/cook_textures_dds.py', '"DXT5" if source.suffix.lower() in ALPHA_EXTENSIONS else "DXT1"', 'offline cooker mirrors DXT1/DXT5 opacity policy')
require('tools/cook_textures_dds.py', 'default=1.0', 'offline cooker covers 512x512-and-larger texture stalls by default')
require('tools/cook_textures_dds.py', '--mipmaps', 'offline cooker can precompute mip chains')
require('tools/cook_textures_dds.py', 'ITEM_ICON_PREFIXES', 'item icons have an explicit cooked-runtime family')
require('tools/cook_textures_dds.py', '--item-icons-only', 'DDS cooker supports focused item-icon cooking')
require('tools/cook_textures_dds.py', '--no-item-icons-required', 'item-icon required policy has an explicit A/B escape hatch')
require('tools/cook_textures_dds.py', 'item-icon-runtime-required-lossless', 'small/non-DXT item icons are cooked losslessly instead of skipped')
require('tools/cook_textures_dds.py', 'probe_dds_header(target)', 'item-icon incremental reuse validates cooked DDS dimensions and format')

# Login / character-selection startup no longer front-loads the selection stage
# and preview models synchronously before the opening screen. The selection
# stage is parsed on the worker pool while the user is on login; preview bodies
# and faces use CharacterModelStreamingV3. Login UI receives lossless cooked DDS.
hologin = 'Shared/authentication/character_selection/HoLogin_legacy_client.inl'
require(hologin, 'SummitQueueHoLoginStagePreload', 'character-selection stage has background preload')
require(hologin, 'JobPriority::Normal', 'selection stage preload runs above background prewarm but below interactive HIGH demand')
require(hologin, 'SummitRequestCharacterModelV3Interactive(&UserCharacter[index]', 'user previews use async model streaming')
require(hologin, '&JobCharacter[targetIndex], chrFile1, chrFile2', 'class previews use async model streaming')
require(hologin, 'Keep the old preview visible', 'face changes do not blank/freeze the current preview')
require('SrcGame/src/systems/presentation/opening/legacy_runtime/HoLinkHeader.cpp', 'lpLogin->BeginPreload();', 'selection stage preloads during opening/login')
require('tools/cook_textures_dds.py', '"game/gui/opening/login/ui/"', 'cooker recognizes login/selection UI roots')
require('tools/cook_textures_dds.py', 'A8R8G8B8', 'login UI cooker has lossless DDS format')
require('tools/cook_textures_dds.py', '--no-login-ui-lossless', 'lossless login UI cooking can be disabled for A/B')
require('SrcGame/src/systems/presentation/opening/HoOpening.cpp', 'SummitOpeningRuntimeTraceFile("NUL")', 'opening hot-path probes are compile-time gated')
forbid('SrcGame/src/systems/presentation/opening/HoOpening.cpp', 'fopen("NUL"', 'opening draw loop has no unconditional NUL fopen')
forbid('SrcGame/src/systems/presentation/opening/legacy_runtime/HoLinkHeader.cpp', 'fopen("NUL"', 'opening wrapper has no unconditional NUL fopen')
require(hologin, 'SummitHoLoginRuntimeTraceFile', 'selection diagnostic files are compile-time gated')
require(hologin, 'SummitHoLoginRuntimeTraceFile(const char *path, const char *mode = \"a\")', 'selection trace wrapper preserves legacy fopen signature')
require(hologin, 'return fopen(path, mode ? mode : \"a\");', 'selection trace wrapper forwards fopen mode only when enabled')
forbid(hologin, 'FILE *f = fopen(', 'selection path has no unconditional diagnostic fopen')
require('SrcGame/src/systems/presentation/opening/HoOpening.cpp', 'SummitQueueOpeningStagePreload', 'opening 3D scene has background preload')
require('SrcGame/src/systems/presentation/opening/HoOpening.cpp', 'JobPriority::Low', 'opening scene preload stays below interactive demand')
forbid('SrcGame/src/systems/presentation/opening/HoOpening.cpp', 'Stage = smSTAGE3D_ReadASE("StartImage//Opening//map//stage.ASE")', 'normal login construction no longer parses opening stage synchronously')
require(hologin, '__summit_login_scene_build__', 'opening and selection legacy scene builders are serialized')

# Selection preview body/head must share the exact same skeleton object. The
# legacy renderer compares TmParent by pointer identity before drawing Pattern2.
require(model, 'motionBorrowedFromPrimary', 'secondary character model can borrow primary skeleton identity')
require(model, '_stricmp(out.modelInfo.szMotionFile, sharedPrimary->modelInfo.szMotionFile) == 0', 'secondary borrows skeleton only for the same motion source')
require(model, 'existing->Pat->TmParent == expectedParent', 'cached secondary is reused only with a compatible skeleton')
require(model, 'needsPairedPrimary', 'cached primary cannot break prepared body/head skeleton pairing')
require(model, 'SummitInstallPreparedModelLocked(request->secondary, primary)', 'secondary installs against the committed primary skeleton')
require(model, 'Even cancellation/shutdown is', 'model worker transfers teardown to owner thread')

# Concurrency fixes selectively ported from the render audit.
require('SrcGame/src/core/concurrency/JobSystem.cpp', 'pendingJobs_.fetch_sub(1, std::memory_order_acq_rel) == 1', 'JobSystem idle transition is detected atomically')
require('SrcGame/src/core/concurrency/JobSystem.cpp', 'std::lock_guard<std::mutex> lock(mutex_);', 'JobSystem idle notification synchronizes with WaitIdle mutex')
require('SrcGame/src/core/concurrency/SourceAssetLock.cpp', 'std::unordered_map<std::string, std::unique_ptr<std::recursive_mutex> > locks;', 'source asset locks are exact-path instead of hash-striped')
require('Shared/rendering/legacy_3d/ase/AseGeometryReader.inl', 'thread_local smPAT3D *smPhysique', 'ASE physique context is thread-local')
require('Shared/rendering/legacy_3d/smRead3d_shared.inl', 'thread_local char DataDirectory', 'legacy model data directory is thread-local')
require('Shared/rendering/legacy_3d/smRead3d_shared.inl', 'std::atomic<int> smMeshReload', 'legacy mesh reload flags are atomic')
require('SrcGame/src/core/data/legacy_loader/fileread.cpp', 'thread_local char decode[512]', 'legacy loader decode scratch is thread-local')
require('Shared/gameplay/character/character_legacy_client.inl', 'SummitCancelCharacterModelV3(this);', 'character destruction invalidates async model generations')

# Worker-side release never destroys D3D resources or recycles the texture slot.
require(tex, 'SummitTextureDeletePending[MAX_TEXTURE]', 'texture slots expose deferred owner-thread deletion state')
require(tex, 'SummitDeferredTextureDeletes.push_back(textureIndex);', 'worker texture deletion is deferred')
require(tex, '!SummitTextureDeletePending[cnt].load', 'pending texture deletion prevents slot reuse ABA')
require(tex, 'SummitDrainDeferredTextureDeletes();', 'render thread drains deferred texture deletions')
require(tex, 'SummitAssetRegistry.MarkReleased(static_cast<std::size_t>(index));', 'deferred texture cleanup releases registry ownership')
require(tex, 'cooperative == D3DERR_DEVICELOST || cooperative == D3DERR_DEVICENOTRESET', 'device-lost texture uploads remain queued for retry')

# D3D9 lifecycle remains owner-thread centric; no mandatory global D3D lock.
require('SrcGame/src/systems/rendering/legacy_3d/smDsx.h', 'BOOL BeginRender', 'BeginRender reports scene-start failure')
require('SrcGame/src/systems/rendering/legacy_3d/smDsx.cpp', 'gD3DSceneBegun', 'BeginScene/EndScene pairing is explicit')
require('SrcGame/src/systems/rendering/legacy_3d/smDsx.cpp', 'if (FAILED(resetResult))', 'failed device Reset cannot run reset callbacks')
require('SrcGame/src/systems/rendering/legacy_3d/smDsx.cpp', 'if (FontRank) { FontRank->Release();', 'FontRank participates in device teardown')

# Memory-safety guards kept from the audit without adopting serialization rewrites.
require('Shared/rendering/legacy_3d/SmRend3dLegacyCommon.h', 'CanReserveRenderSpace', 'legacy render buffers have capacity reservation guards')
require('Shared/rendering/effects/legacy_new/HoEffectViewLegacyCommon.inl', 'CanReserveRenderSpace', 'effect view checks legacy render capacity')
require('Shared/rendering/effects/particles/HoParticleLegacyCommon.inl', 'CanReserveRenderSpace', 'particle path checks legacy render capacity')
require('SrcGame/src/systems/rendering/effects/legacy_ho/core/HoPhysics.h', 'index < 0 || index >= MAX_PHYSICS_OBJECT_MAX', 'physics manager rejects invalid indices')
require('SrcGame/src/systems/rendering/effects/legacy_ho/core/HoPhysics.cpp', 'length <= 0.0001f', 'physics destination normalization guards zero length')
require('Shared/rendering/effects/assa/AssaParticleLegacyCommon.inl', 'sizeof(CharList[0]) * safeCount', 'effect target memcpy is bounded by pointer-array capacity')
require('SrcGame/src/systems/rendering/sky/legacy/HoSky.cpp', 'DrawIndex < -1 || DrawIndex >= static_cast<int>(SkyDrawInfoList.size())', 'sky draw index validates disabled and invalid states')

# Login/selection startup V4: DDS generation is automatic for interactive UI,
# global texture sweeps are removed, and AssaEffect does not rescan recursively
# a second time when moving from opening to character selection.
require_if_present('build_client_mingw_i686_multicore.sh', 'SUMMIT_AUTO_COOK_DDS', 'client build has automatic DDS cooking switch')
require_if_present('build_client_mingw_i686_multicore.sh', 'AUTO_COOK_DDS="${SUMMIT_AUTO_COOK_DDS:-0}"', 'DDS auto-cook is opt-in after login UI cooked regression')
require('tools/cook_textures_dds.py', '--login-ui-only', 'DDS cooker supports focused login UI cooking')
require('tools/cook_textures_dds.py', 'Fast incremental path', 'DDS cooker skips decode for up-to-date cooked UI')
require('Shared/rendering/legacy_3d/material/smTexture_legacy_client.inl', 'SUMMIT_TEXTURE_COOKED_UI', 'cooked login UI has an explicit runtime A/B opt-in')
require('Shared/rendering/legacy_3d/material/smTexture_legacy_client.inl', 'SummitCookedTextureRuntimeEligible(prepared->path)', 'runtime bypasses cooked DDS for incompatible interactive UI')
require('Shared/rendering/legacy_3d/material/smTexture_legacy_client.inl', '"startimage/login/"', 'interactive UI cooked bypass covers login roots')
require(tex, 'SummitIsInteractiveLoginTexture', 'login UI textures receive explicit interactive admission')
require(tex, 'AssetLoadV3RequestTexture(texture, 0, 0);', 'login UI textures request HIGH V3 residency')
require(tex, 'SummitInteractiveTextureRetries', 'interactive UI demand survives bounded admission')
require(tex, 'SummitPumpInteractiveTextureRetries();', 'interactive UI retries are pumped every render frame')
require(tex, 'readiness == ASSET_LOAD_V3_TEXTURE_RETRYABLE && admitted < 4u', 'interactive UI retry admission is frame-bounded')
require(tex, 'entry.generation = SummitTextureAsyncGenerations[index].load', 'interactive UI retry queue is generation-safe')
hologin_text = text(hologin)
startup_sweeps_removed = hologin_text.count('\tReadTextures();') == 0
checks.append((startup_sweeps_removed, 'selection init does not trigger broad global texture sweep'))
if not startup_sweeps_removed: errors.append('HoLogin startup must not add broad ReadTextures sweeps')
require(hologin, 'SMASSET_OWNER_UI, 1, 2u, 1', 'selection stage prewarms incrementally per frame')
require('SrcGame/src/systems/rendering/legacy_3d/smTexture.h', 'AssetLoadV2PrewarmMaterialGroupStep', 'incremental material prewarm is declared in the public texture API')
require('SrcGame/src/systems/rendering/effects/legacy_sin/AssaEffect.cpp', 'static bool catalogReady = false;', 'Assa effect resource catalog is process-lifetime cached')
require('SrcGame/src/systems/rendering/effects/legacy_sin/AssaEffect.cpp', 'if (!catalogReady)', 'selection skips duplicate Assa effect recursive rescan')
forbid('SrcGame/src/systems/rendering/effects/legacy_sin/AssaEffect.cpp', 'fopen("LOGS\\mingw_assa_effect_init.txt"', 'Assa effect init has no unconditional trace file I/O')
forbid('Shared/rendering/effects/legacy_sin/AssaUtil_legacy_client.inl', 'mingw_loadassares.txt', 'Assa resource scan has no unconditional verbose trace I/O')


# Predictive preload V6 rebased on stabilization V5: first-frame login UI is
# deterministic, the future selection scene/models advance while login is visible,
# and common effect textures are warmed before first use.
require('SrcGame/src/systems/rendering/legacy_3d/smTexture.h', 'AssetLoadV2RequestTexture', 'texture API exposes explicit predictive async request')
require('SrcGame/src/systems/rendering/legacy_3d/smTexture.h', 'AssetLoadV2LoadTextureNow', 'texture API exposes exact owner-thread first-use fallback')
require(tex, 'SummitTextureAsyncGenerations[index].fetch_add', 'exact texture fallback invalidates stale async generation')
require('SrcGame/src/systems/presentation/opening/HoOpening.cpp', 'SummitLoadOpeningMaterialNow(Back);', 'login background is resident before first frame')
require('SrcGame/src/systems/presentation/opening/HoOpening.cpp', 'SummitLoadOpeningMaterialNow(PristonLogoImage);', 'login logo/chrome is resident before first frame')
forbid('SrcGame/src/systems/presentation/opening/HoOpening.cpp', 'AssetLoadV3LoadOpeningUiNow();', 'opening no longer depends on broad first-frame UI sweep')
forbid('SrcGame/src/systems/presentation/opening/HoOpening.cpp', '\tReadTextures();', 'opening no longer starts a broad global texture sweep')
require('SrcGame/src/systems/presentation/opening/HoOpening.cpp', 'SMASSET_OWNER_UI, 1, 2u, 1', 'opening 3D scene keeps bounded incremental prewarm')
require(hologin, 'JobPriority::Normal', 'selection stage preload has enough priority to finish before transition')
require(hologin, 'void HoLogin::PumpOpeningPreload()', 'selection exposes an opening-time preload pump')
require(hologin, 'SMASSET_OWNER_UI, 1, 2u, 2', 'selection environment textures prewarm during login')
require('Shared/authentication/character_selection/HoLoginLegacyShared.h', 'PumpOpeningPreload', 'selection opening preload pump is public to opening runtime')
require(model, 'SummitRequestCharacterModelV3Interactive', 'model streaming exposes an interactive HIGH-priority request')
require(model, 'JobPriority::High, true', 'interactive character previews use the HIGH worker lane')
require(model, 'const int texturePriority = request->interactive ? 0 : 1;', 'interactive model textures keep HIGH priority through prewarm')
require(hologin, 'BeginUserCharacterPreload();', 'selection starts user preview loading before legacy init completes')
require(hologin, 'SummitRequestCharacterModelV3Interactive(&UserCharacter[index]', 'existing user previews use interactive model streaming')
require(hologin, 'SummitRequestCharacterModelV3Interactive(\n\t\t\t&JobCharacter[targetIndex]', 'class previews use interactive model streaming')
require('SrcGame/src/systems/presentation/opening/legacy_runtime/HoLinkHeader.cpp', 'lpLogin->BeginPreload();', 'selection stage parsing begins before opening initialization completes')
require('SrcGame/src/systems/presentation/opening/legacy_runtime/HoLinkHeader.cpp', 'lpLogin->PumpOpeningPreload();', 'opening frames advance future selection residency')
require('SrcGame/src/systems/presentation/opening/legacy_runtime/HoLinkHeader.cpp', 'lpLogin->BeginUserCharacterPreload();', 'server character list immediately starts preview preload')
require('SrcGame/src/systems/presentation/opening/legacy_runtime/HoLinkHeader.cpp', 'AssetLoadV3SetLoadingBoost(false);', 'login loading boost is released before gameplay')
require('SrcGame/src/systems/rendering/effects/legacy_sin/AssaUtil.h', 'SummitPrewarmAssaEffectTexturesStep', 'effect catalog exposes progressive texture prewarm')
require('Shared/rendering/effects/legacy_sin/AssaUtil_legacy_client.inl', 'AssetLoadV2RequestTexture(material.smTexture[0], SMASSET_OWNER_EFFECT', 'effect catalog queues exact textures ahead of use')
require('Shared/rendering/effects/legacy_sin/AssaUtil_legacy_client.inl', 'AssetLoadV2LoadTextureNow(material.smTexture[0], SMASSET_OWNER_EFFECT', 'first-use effect texture falls back synchronously instead of white')
require('SrcGame/src/systems/presentation/opening/legacy_runtime/HoLinkHeader.cpp', 'SummitPrewarmAssaEffectTexturesStep(2, 2);', 'opening/login frames prewarm effect textures')
require('SrcGame/src/platform/windows/application/Winmain.cpp', 'SummitPrewarmAssaEffectTexturesStep(1, 2);', 'gameplay continues unfinished effect prewarm')

for ok, label in checks:
    print(('PASS' if ok else 'FAIL') + '  ' + label)
print(f"RESULT={'PASS' if not errors else 'FAIL'} checks={len(checks)} errors={len(errors)}")
if errors:
    for e in errors:
        print('ERROR ' + e)
    sys.exit(1)
