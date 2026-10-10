#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[2]
checks=[
 ('playmain include probe','SrcGame/src/core/gameplay/runtime/playmain.cpp','#include "diagnostics/RuntimeAssetProbe.h"'),
 ('bypass helper','SrcGame/src/core/gameplay/runtime/playmain.cpp','SummitEragonDiagnosticBypassTick'),
 ('bypass opt in','SrcGame/src/core/gameplay/runtime/playmain.cpp','SUMMIT_ERAGON_DIAG_BYPASS'),
 ('bypass F11','SrcGame/src/core/gameplay/runtime/playmain.cpp','VK_F11'),
 ('bypass field52 only','SrcGame/src/core/gameplay/runtime/playmain.cpp','StageField[slot]->FieldCode != 52'),
 ('bypass x','SrcGame/src/core/gameplay/runtime/playmain.cpp','198280 * fONE'),
 ('bypass z','SrcGame/src/core/gameplay/runtime/playmain.cpp','242864 * fONE'),
 ('bypass probe event','SrcGame/src/core/gameplay/runtime/playmain.cpp','ERAGON_BYPASS_F11'),
 ('summary category filter','tools/runtime_asset_probe_summary.py',"--category"),
 ('summary role filter','tools/runtime_asset_probe_summary.py',"--role"),
 ('summary timeline','tools/runtime_asset_probe_summary.py',"--timeline"),
]
failed=[]
for label,rel,token in checks:
    p=root/rel
    text=p.read_text(errors='ignore') if p.exists() else ''
    if token not in text: failed.append(label)
    else: print('PASS:',label)
if failed:
    print('PATCH 211 gate: FAIL — '+', '.join(failed),file=sys.stderr); sys.exit(1)
print(f'PATCH 211 gate: OK — Eragon diagnostic bypass is opt-in/field-52-only and asset probe analysis is filterable ({len(checks)} contracts)')
