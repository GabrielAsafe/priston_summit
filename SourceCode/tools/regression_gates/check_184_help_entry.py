#!/usr/bin/env python3
from pathlib import Path
import sys
r=Path(__file__).resolve().parents[2]
h=(r/'SrcGame/src/systems/ui/v2/legacy/LegacyNpcDialogWindow.h').read_text(encoding='utf8')
c=(r/'SrcGame/src/systems/ui/v2/legacy/LegacyNpcDialogWindow.cpp').read_text(encoding='utf8')
req=['guideButton_','ActivateGuideHelp()','help.KindFlag == SIN_HELP_KIND_NORMAL','InitGuideHelp();']
missing=[x for x in req if x not in h+c]
if missing: print('PATCH 184 missing: '+', '.join(missing),file=sys.stderr);sys.exit(1)
print('PATCH 184 gate: OK')
