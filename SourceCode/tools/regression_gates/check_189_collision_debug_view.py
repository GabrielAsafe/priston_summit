#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[2]
impl=(root/'Shared/rendering/legacy_3d/stage/smStage3d_legacy_client.inl').read_text(errors='replace')
head=(root/'Shared/rendering/legacy_3d/stage/smStage3d_legacy_client.h').read_bytes().decode('latin1')
win=(root/'SrcGame/src/platform/windows/application/Winmain.cpp').read_bytes().decode('latin1')
need=[(head,'RenderCollisionDebugGeom'),(impl,'SMMAT_STAT_CHECK_FACE'),(impl,'D3DFILL_WIREFRAME'),(impl,'RenderCollisionDebugGeom();'),(win,'VK_CONTROL] && wParam == VK_F10'),(win,'smStageCollisionDebugView = !smStageCollisionDebugView')]
errors=['missing '+t for s,t in need if t not in s]
if errors:
 print('\n'.join('ERROR: '+x for x in errors));sys.exit(1)
print('PATCH 189 gate: OK (Ctrl+F10)')
