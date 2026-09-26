#!/usr/bin/env python3
from pathlib import Path
import re, sys
ROOT=Path(__file__).resolve().parents[1]
headers={
 'client': ROOT/'SrcGame/src/systems/combat/skills/legacy_sin/sinSkill_InfoIni.h',
 'server': ROOT/'SrcServer/src/systems/combat/skills/legacy/sinSkill_InfoIni.h',
 'support': ROOT/'SrcServer/src/systems/combat/skills/server_support/sinSkill_InfoIni.h',
}
structs=['MECH_INFO','FIGHTER_INFO','PIKE_INFO','ARCHER_INFO','KINA_INFO','ATA_INFO','SACER_INFO','MAGE_INFO']
expected={'MECH_INFO':2688,'FIGHTER_INFO':2368,'PIKE_INFO':2648,'ARCHER_INFO':3048,'KINA_INFO':2488,'ATA_INFO':2688,'SACER_INFO':2728,'MAGE_INFO':3768}

def body(text,name):
    m=re.search(r'\bstruct\s+'+re.escape(name)+r'\s*\{(.*?)\};',text,re.S)
    if not m: raise AssertionError(f'missing struct {name}')
    return m.group(1)

def fields(text,name):
    b=re.sub(r'//.*','',body(text,name))
    out=[]
    for stmt in b.split(';'):
        stmt=' '.join(stmt.split())
        if not stmt: continue
        m=re.match(r'(int|float|DWORD|BYTE)\s+(.+)$',stmt)
        if not m: continue
        typ,decls=m.groups()
        for decl in decls.split(','):
            decl=decl.strip()
            mm=re.match(r'([A-Za-z_]\w*)((?:\[\d+\])*)$',decl)
            if not mm: raise AssertionError(f'{name}: cannot parse {decl!r}')
            nm,dims=mm.groups(); shape=tuple(map(int,re.findall(r'\[(\d+)\]',dims)))
            out.append((typ,nm,shape))
    return out

def calc_size(fs):
    sizes={'int':4,'float':4,'DWORD':4,'BYTE':1}
    total=0
    for typ,nm,shape in fs:
        n=1
        for d in shape:n*=d
        total+=sizes[typ]*n
    return total

texts={k:p.read_text('latin1') for k,p in headers.items()}
errors=[]
for st in structs:
    ref=fields(texts['client'],st)
    for side in ('server','support'):
        got=fields(texts[side],st)
        if got!=ref:
            # show first mismatch
            at=next((i for i,(a,b) in enumerate(zip(ref,got)) if a!=b),min(len(ref),len(got)))
            errors.append(f'{st}: client != {side} at field {at}: client={ref[at:at+1]} {side}={got[at:at+1]} counts={len(ref)}/{len(got)}')
    sz=calc_size(ref)
    if sz!=expected[st]: errors.append(f'{st}: modeled sizeof {sz}, expected {expected[st]}')

net=(ROOT/'SrcGame/src/systems/network/client/netplay.cpp').read_text('latin1')
for st,var,op in [
 ('MECH_INFO','lpMechInfo','smTRANSCODE_MECH_INFO'),('FIGHTER_INFO','lpFighterInfo','smTRANSCODE_FIGHTER_INFO'),
 ('PIKE_INFO','lpPikeInfo','smTRANSCODE_PIKE_INFO'),('ARCHER_INFO','lpArcherInfo','smTRANSCODE_ARCHER_INFO'),
 ('KINA_INFO','lpKinaInfo','smTRANSCODE_KINA_INFO'),('ATA_INFO','lpAtaInfo','smTRANSCODE_ATA_INFO'),
 ('SACER_INFO','lpSacerInfo','smTRANSCODE_SACER_INFO'),('MAGE_INFO','lpMageInfo','smTRANSCODE_MAGE_INFO')]:
    token=f'if(!SUMMIT_SKILL_INFO_SIZE_OK({var}, {st}, {op})) break;'
    if token not in net: errors.append(f'{st}: missing client packet sizeof guard')

motion=(ROOT/'Shared/gameplay/character/character_legacy_client.inl').read_text('utf-8',errors='ignore')
for token in ['SummitMotionEUCanonicalT5PlayCode(storedSkill, dwJobCodeMask) == requestedT5',
              'SKILL_PLAY_T51 && (atSkill & 0xFF) <= SKILL_PLAY_T54']:
    if token not in motion: errors.append('motion: missing EU canonical second-pass token '+token)

skill=(ROOT/'Shared/gameplay/skills/SkillSub_legacy_client.inl').read_text('latin1')
if 'SummitT5StartLegacyLuaEffectAt("game//scripts//particles//MG5WizardTrance.luac"' in skill:
    errors.append('Wizard Trance still feeds .luac bytecode to legacy Lua loader')
if 'MG5OrbMastery' not in skill: errors.append('Wizard Trance compatible particle fallback missing')

if errors:
    print('FAIL tier5 all-class contract')
    for e in errors: print(' -',e)
    sys.exit(1)
print('PASS tier5 all-class contract')
for st in structs: print(f'{st}={expected[st]} bytes')
print('packet_size_guards=8/8')
print('eu_canonical_motion_fallback=enabled-after-primary-miss')
print('wizard_trance_legacy_luac_loader=disabled')
