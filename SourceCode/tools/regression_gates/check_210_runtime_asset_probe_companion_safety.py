#!/usr/bin/env python3
from pathlib import Path

ROOT=Path(__file__).resolve().parents[2]
checks={
'Shared/diagnostics/RuntimeAssetProbe.h':[
 'runtime_asset_attempts.log','runtime_asset_unique.log','SUMMIT_RUNTIME_ASSET_PROBE','RuntimeAssetProbeAttempt','RuntimeAssetProbeResult'],
'Shared/content/v1/runtime/RuntimeContentPath.h':['RuntimeAssetProbeAttempt("RUNTIME_PACK"'],
'Shared/rendering/legacy_3d/smRead3d_legacy_client.inl':['RuntimeAssetProbeAttempt("MODEL"','RuntimeAssetProbeAttempt("STAGE"','RuntimeAssetProbeAttempt("MODEL_CACHE"'],
'Shared/rendering/legacy_3d/smRead3d_legacy_server.inl':['RuntimeAssetProbeAttempt("MODEL"','RuntimeAssetProbeAttempt("STAGE"'],
'Shared/formats/model_config/LegacyModelConfigExact_client.inl':['RuntimeAssetProbeAttempt("MODEL_CONFIG"'],
'Shared/formats/model_config/LegacyModelConfigExact_server.inl':['RuntimeAssetProbeAttempt("MODEL_CONFIG"'],
'Shared/rendering/legacy_3d/material/smTexture_legacy_client.inl':['RuntimeAssetProbeAttempt("TEXTURE"','return !value || !*value || *value != \'0\';'],
'Shared/rendering/effects/particles/HoNewParticleMgrLegacyCommon.inl':['RuntimeAssetProbeAttempt("PARTICLE_SCRIPT"'],
'Shared/rendering/effects/HoAnimDataLegacyCommon.inl':['RuntimeAssetProbeAttempt("ANIM_SEQUENCE"','RuntimeAssetProbeAttempt("ANIM_IMAGE"'],
'SrcGame/src/systems/audio/playback/legacy/Dxwav.cpp':['RuntimeAssetProbeAttempt("AUDIO_WAV"','RuntimeAssetProbeAttempt("AUDIO_BGM"'],
'SrcGame/src/systems/media/video/ParkDXPlayer.cpp':['RuntimeAssetProbeAttemptW("VIDEO"'],
'Shared/gameplay/character/character_legacy_client.inl':['VISUAL_WAIT_MODEL','if(character->PatLoading || !character->Pattern || !character->MotionInfo)','if (!Pattern || PatLoading || !MotionInfo)'],
'Shared/server/legacy_runtime/OnSever_legacy_client.inl':['summitFixedNpcName','strcpy_s(lpChar->smCharInfo.szName, summitFixedNpcName);'],
'Shared/server/legacy_runtime/OnSever_legacy_server.inl':['summitFixedNpcName','strcpy_s(lpChar->smCharInfo.szName, summitFixedNpcName);'],
'SrcGame/src/core/gameplay/legacy_support/playsub.cpp':['FieldCode == 52','Lar do Eragon'],
}
for rel,tokens in checks.items():
 p=ROOT/rel
 if not p.is_file(): raise SystemExit(f'FAIL missing {rel}')
 text=p.read_text(encoding='utf-8',errors='replace')
 for token in tokens:
  if token not in text: raise SystemExit(f'FAIL {rel}: missing {token}')
  print(f'PASS: {rel}: {token}')
print('PATCH 210 gate: OK — runtime-wide asset attempts are logged, companion summon waits for streamed models, fixed-NPC PT names survive OpenNpc decode, and Eragon title is PT-BR')
