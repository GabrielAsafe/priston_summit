#!/usr/bin/env python3
from pathlib import Path
import re, sys
root=Path(__file__).resolve().parents[2]
errors=[]
for rel in ('Shared/rendering/legacy_3d/stage/smStage3d_legacy_client.inl','Shared/rendering/legacy_3d/stage/smStage3d_legacy_server.inl'):
    p=root/rel; text=p.read_text(encoding='utf-8',errors='replace')
    m=re.search(r'(?m)^int\s+smSTAGE3D::LoadFile\(char \*szFile\)\s*\{',text)
    if not m: errors.append(f'{rel}: active LoadFile missing'); continue
    ob=text.find('{',m.start()); d=0; end=None
    for i in range(ob,len(text)):
        if text[i]=='{': d+=1
        elif text[i]=='}':
            d-=1
            if d==0: end=i+1; break
    body=text[m.start():end]
    if 'stage_serialization::LoadStage' not in body: errors.append(f'{rel}: canonical loader missing')
    for bad in ('ReadFile(hFile, &Head','TexLink - lpOldTexLink','NextTex - lpOldTexLink'):
        if bad in body: errors.append(f'{rel}: unsafe pattern remains: {bad}')
if errors: print('\n'.join(errors),file=sys.stderr); sys.exit(1)
print('PATCH 178 gate: OK')
