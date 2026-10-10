#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]
errors=[]

def read(rel):
    p=ROOT/rel
    if not p.is_file():
        errors.append(f'missing file: {rel}')
        return ''
    return p.read_text(encoding='utf-8', errors='replace')

def need(rel, token, label):
    s=read(rel)
    if token not in s:
        errors.append(f'{label}: token missing in {rel}: {token!r}')

def forbid(rel, token, label):
    s=read(rel)
    if token in s:
        errors.append(f'{label}: forbidden token still present in {rel}: {token!r}')

warehouse='SrcGame/src/systems/commerce/legacy/sinTrade.cpp'
companions='SrcGame/src/systems/ui/v2/pages/CompanionsPage.cpp'
hud='SrcGame/src/systems/ui/hud/legacy_sin/sinInterFace.cpp'
skills='Shared/gameplay/skills/sinSkill_legacy_client.inl'
effects='Shared/rendering/effects/SinEffect2LegacyCommon.inl'

need(warehouse, 'TempItem.w = sWareHouse.WareHouseItem[i].w;', 'warehouse uses real width')
need(warehouse, 'TempItem.h = sWareHouse.WareHouseItem[i].h;', 'warehouse uses real height')
forbid(warehouse, 'TempItem.w = ITEMSIZE * 2;', 'warehouse fixed 2x4 width retired')
forbid(warehouse, 'TempItem.h = ITEMSIZE * 4;', 'warehouse fixed 2x4 height retired')

need(companions, 'skillModeButton_->SetVisible(true);', 'Support tab remains visible')
need(companions, 'skillModeButton_->SetEnabled(hasSelected);', 'Support tab stays reachable from tactics')
forbid(companions, 'Regras 1 -> 4; executor valida skills.', 'developer guidance removed from player UI')

# PATCH 213R2: accept either the 212 legacy hover or the 213+ V2-owned hover.
# Reuse the exact three predicates from check_213_status_ui_v2.py.
legacy_hover = '(!suppressMainHud || SkillInfoShowFlag == 2)' in read(hud)
v2_notify = 'SrcGame/src/systems/ui/v2/legacy/LegacyNotificationHud.cpp'
v2_hover = (
    'if(!suppressMainHud && SkillInfoShowFlag && !ShowSkillUpInfo)' in read(hud)
    and 'void RenderStatusEffectHover(' in read(v2_notify)
    and 'RenderStatusEffectHover(context, viewport);' in read(v2_notify)
)
if not (legacy_hover or v2_hover):
    errors.append('timer hover survives V2 HUD suppression: neither 212 legacy nor 213+ V2 hover path is present')
need(skills, 'SummitResolveLegacyContinueSkillMaterial', 'legacy timer icon retry')
need(skills, 'missing_timer_icons.log', 'missing timer diagnostic')
need(skills, 'EXP +15%/r', 'VIP benefit tooltip EXP')
need(skills, 'Drop +5%/r', 'VIP benefit tooltip drop')

need(effects, 'cSINEFFECT2 &effect = cSinEffect2[sinEffect_Zdepth2[i].Index - 1];', 'effect render local reference')
need(effects, 'if(!effect.sinPatMesh)', 'combat effect null-mesh guard')
need(effects, 'effect.Flag = 0;', 'failed effect retired safely')
need(effects, 'if(!effect.sinPatMesh->smMaterialGroup)', 'material-group access happens after mesh guard')

if errors:
    print('PATCH 212 gate: FAIL')
    for e in errors:
        print(' -',e)
    sys.exit(1)
print('PATCH 212 gate: OK — warehouse uses real item footprints, Companion tabs remain stable, timer hover/icon recovery is restored, VIP tooltip text is readable, and null combat meshes cannot dereference smPAT3D+0x288')
