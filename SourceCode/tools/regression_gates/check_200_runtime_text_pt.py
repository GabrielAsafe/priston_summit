#!/usr/bin/env python3
from pathlib import Path
import base64, json, re, subprocess, sys
root=Path(__file__).resolve().parents[2]
errors=[]
resolver=(root/'Shared/content/v1/runtime/RuntimeContentPath.h').read_text(encoding='utf-8')
text_rt=(root/'Shared/content/v1/text_resource/content/TextResourceRuntime.h').read_text(encoding='utf-8')
loc_rt=(root/'Shared/content/v1/localization/content/LocalizationRuntime.h').read_text(encoding='utf-8')
for token in ('..\\\\SourceCode\\\\Content\\\\packs\\\\', 'RuntimeRegularFileExists', 'RuntimeCurrentDirectory'):
    if token not in resolver: errors.append('runtime content resolver missing '+token)
if 'if(configured && Try(configured)) return;' not in text_rt:
    errors.append('text-resource bootstrap still stops after invalid configured/default path')
if 'if(configured && Try(configured)) return;' not in loc_rt:
    errors.append('localization bootstrap still stops after invalid configured/default path')
for rel in (
 'SrcGame/src/systems/ui/v2/legacy/LegacyNpcDialogWindow.cpp',
 'SrcGame/src/systems/ui/v2/legacy/LegacyMessageWindow.cpp',
 'SrcGame/src/systems/ui/v2/legacy/LegacyNotificationHud.cpp',
 'SrcGame/src/systems/ui/v2/legacy/LegacySkillNpcWindow.cpp'):
    s=(root/rel).read_text(encoding='utf-8')
    if 'MultiByteToWideChar(1252' not in s:
        errors.append(rel+': legacy Portuguese bridge is not explicitly CP1252')

resources=root/'Content/canonical/text/resources'
core=[
 resources/'cliente/image/Sinimage/help/Help.sin.json',
 resources/'cliente/image/Sinimage/PetData/PetMessage.txt.json',
]
english_sentence=re.compile(r'\b(the|you|your|welcome|click|press|party|quest|skill|inventory|monster|weapon|character|friend|trade)\b',re.I)
foreign=re.compile(r'[\u3040-\u30ff\u3400-\u9fff\uac00-\ud7af\ufffd]')
for path in core:
    doc=json.loads(path.read_text(encoding='utf-8'))
    lines=doc.get('semantic',{}).get('lines',[])
    text='\n'.join(lines)
    # Section identifiers are stable runtime keys and intentionally remain English.
    prose='\n'.join(x for x in lines if not x.startswith('*') and x.strip()!='end')
    hits=sorted({m.group(0).lower() for m in english_sentence.finditer(prose)})
    if hits: errors.append(f'{path.relative_to(root)}: English prose residue {hits}')
    if foreign.search(prose): errors.append(f'{path.relative_to(root)}: foreign-script/mojibake residue')
    try: expected=('\r\n'.join(lines)+'\r\n').encode('cp1252')
    except UnicodeEncodeError as exc:
        errors.append(f'{path.relative_to(root)}: not CP1252-compatible: {exc}'); continue
    actual=base64.b64decode(doc.get('compatibility',{}).get('legacy_payload_base64',''))
    if actual != expected: errors.append(f'{path.relative_to(root)}: compatibility payload differs from semantic PT lines')

sync=root/'tools/content_port/sync_linked_text_resources_pt.py'
proc=subprocess.run([sys.executable,str(sync),'--check'],cwd=root,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
if proc.returncode != 0:
    errors.append('linked resource PT sync is not clean: '+proc.stdout.strip())

pack=root/'Content/packs/text_resources.sres'
if not pack.is_file() or pack.stat().st_size == 0: errors.append('compiled text_resources.sres missing/empty')
if errors:
    for e in errors: print('ERROR:',e,file=sys.stderr)
    sys.exit(1)
print('PATCH 200 gate: OK — runtime pack discovery, CP1252 bridge, Pet/Guide PT and linked entity text are synchronized')
