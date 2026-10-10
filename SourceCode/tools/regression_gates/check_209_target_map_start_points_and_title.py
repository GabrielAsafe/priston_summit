#!/usr/bin/env python3
from pathlib import Path
import json
root=Path(__file__).resolve().parents[2]
base=root/'Content/canonical/stage_start_points/points/GameServer/Field'
checks=[('Stemple.ASE.spp.json',49),('ruin-arenaSF.ASE.spp.json',50)]
for name,field in checks:
 d=json.loads((base/name).read_text(encoding='utf-8'))
 pts=d.get('points',[])
 if len(pts)<16: raise SystemExit(f'FAIL: field {field}: start-point capacity still too small ({len(pts)})')
 if len({p['slot'] for p in pts})!=len(pts): raise SystemExit(f'FAIL: field {field}: duplicate slots')
 if not all(p.get('state',0)>0 for p in pts): raise SystemExit(f'FAIL: field {field}: disabled fallback point')
 print(f'PASS: field {field}: start_points={len(pts)}')
text=(root/'SrcGame/src/core/gameplay/legacy_support/playsub.cpp').read_text(errors='replace')
for token in ['FieldCode == 49','"Abismo do Mar 2"','legacy Korean asset']:
 if token not in text: raise SystemExit('FAIL: field 49 PT minimap title override missing: '+token)
print('PATCH 209 gate: OK — fields 49/50 have functional spawn-point capacity and field 49 bypasses the legacy Korean minimap title')
