#!/usr/bin/env python3
from pathlib import Path
import sys
p=Path(__file__).resolve().parents[2]/'SrcGame/src/systems/ui/v2/panels/ContextPanel.cpp'
s=p.read_text()
errors=[]
if 'WrapTextApprox(info, root.width - 24, 3, 7)' not in s: errors.append('hover detail is not wrapped')
if 'EllipsizeText(info, root.width - 24)' in s: errors.append('old cropped hover detail still active')
if 'root.height - 202' not in s: errors.append('status/footer spacing not reserved')
if errors:
 print('\n'.join('ERROR: '+e for e in errors));sys.exit(1)
print('PATCH 188 gate: OK')
