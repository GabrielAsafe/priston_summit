#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[2]
skill=(root/'Shared/gameplay/skills/SkillSub_legacy_client.inl').read_text(encoding='utf-8',errors='replace')
ho=(root/'SrcGame/src/systems/rendering/effects/legacy_ho/core/HoEffect.cpp').read_text(encoding='utf-8',errors='replace')
errors=[]
for token in (
    'int particleID = g_NewParticleMgr.Start((char *)name, pos);',
    'if(particleID >= 0) return;',
    'strcmp(name, "MG5OrbMastery") == 0',
    'fallback = "Skill3MagicianSpiritElemental";',
    'strcmp(name, "R5StoneSkin") == 0',
    'fallback = "Skill3MagicianEngrgyShield";',
):
    if token not in skill: errors.append('missing T5 fallback contract: '+token)
for token in (
    'Skill3MagicianSpiritElemental.part',
    'Skill3MagicianEngrgyShield.part',
):
    if token not in ho: errors.append('fallback particle is not loaded by legacy catalog: '+token)
if errors:
    print('\n'.join('ERROR: '+e for e in errors)); sys.exit(1)
print('PATCH 204 gate: OK — Magician T5 visuals prefer canonical assets and fall back to shipped legacy particles')
