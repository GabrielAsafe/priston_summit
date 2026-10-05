#!/usr/bin/env python3
from pathlib import Path
import base64
import json
import os
import subprocess
import sys
import tempfile

root = Path(__file__).resolve().parents[2]
canonical = root / 'Content/canonical/text/resources'
header = (root / 'Shared/content/v1/text_resource/content/TextResourceRuntime.h').read_text(encoding='utf-8')
npc_v2 = (root / 'SrcGame/src/systems/ui/v2/legacy/LegacyNpcDialogWindow.cpp').read_text(encoding='utf-8')
msg_v2 = (root / 'SrcGame/src/systems/ui/v2/legacy/LegacyMessageWindow.cpp').read_text(encoding='utf-8')
legacy_main = (root / 'SrcGame/src/core/client/legacy_sin_runtime/sinMain.cpp').read_text(encoding='utf-8')

errors = []
for token in (
    'Content/packs/text_resources.sres',
    'SUMMIT_TEXT_RESOURCE_PACK',
    'CompiledTextResourcesEnabled(){ return ActiveTextResourceSource()!=0; }',
):
    if token not in header:
        errors.append('text-resource bootstrap missing: ' + token)

for token in (
    'case HAQUEST_CODE_ELEMENTARY_A:',
    'case HAQUEST_CODE_ELEMENTARY_G:',
    'return "Missao de iniciante";',
):
    if token not in npc_v2:
        errors.append('UI V2 NPC dialog does not explicitly own elementary quest presentation: ' + token)

for token in ('case MESSAGE_QUEST_ElementaryA:', 'case MESSAGE_QUEST_ElementaryG:'):
    if token not in msg_v2:
        errors.append('UI V2 confirmation does not own elementary quest action: ' + token)

if 'if (!ui::v2::legacy::OwnsLegacyHelpUI())' not in legacy_main:
    errors.append('legacy help rendering is not suppressed when UI V2 owns the dialog')
if 'if (!ui::v2::legacy::OwnsLegacyMessage3UI())' not in legacy_main:
    errors.append('legacy Message3 rendering is not suppressed when UI V2 owns confirmation')

active_names = {
    'haQuesttired_Astart.sin','haQuesttired_Apass1.sin','haQuesttired_Apass.sin','haQuesttired_Aend.sin',
    'haQuestterrified_Bstart_M.sin','haQuestterrified_Bstart_T.sin','haQuestterrified_Bpass_M.sin','haQuestterrified_Bpass_T.sin','haQuestterrified_Bend.sin',
    'haQuestcandy_Cstart_M.sin','haQuestcandy_Cstart_T.sin','haQuestcandy_Cpass.sin','haQuestcandy_Cend.sin',
    'haQuestGoblin_Dstart_M.sin','haQuestGoblin_Dstart_T.sin','haQuestGoblin_Dpass_M.sin','haQuestGoblin_Dpass_T.sin','haQuestGoblin_Dend.sin',
    'haQuestundead_Estart_M.sin','haQuestundead_Estart_T.sin','haQuestundead_Epass_M.sin','haQuestundead_Epass_T.sin','haQuestundead_Eend.sin',
    'haQuestcyclops_Fstart.sin','haQuestcyclops_Fpass.sin','haQuestcyclops_Fend.sin',
    'haQuesthammer_Gstart.sin','haQuesthammer_Gpass.sin','haQuesthammer_Gend.sin','haQuestLevel_Over.sin',
}
found = []
english_tokens = (
    'welcome to ', 'quest subject', 'you have ', 'you are ', 'you can ', 'please ',
    'hunt ', 'go back ', 'warrior', 'warehouse keeper', 'blacksmith ', 'monster',
    'reward', 'potion', 'battle', 'village', 'town', ' goblins ', 'skeleton', 'cyclops',
    'thanks to ', 'the combatants', 'from now', 'may the spirit',
)
for path in canonical.rglob('*.json'):
    basename = path.name[:-5]
    if basename not in active_names:
        continue
    found.append(path)
    doc = json.loads(path.read_text(encoding='utf-8'))
    lines = doc.get('semantic', {}).get('lines')
    if not isinstance(lines, list) or not lines:
        errors.append(f'{path}: missing semantic lines')
        continue
    text = '\n'.join(str(x) for x in lines)
    lower = ' ' + ' '.join(text.lower().split()) + ' '
    hits = sorted({tok.strip() for tok in english_tokens if tok in lower})
    if hits:
        errors.append(f'{path}: English residue {hits}')
    for forbidden in ('�', 'ƒ', 'Ã', 'Â'):
        if forbidden in text:
            errors.append(f'{path}: mojibake marker {forbidden!r}')
    try:
        expected = ('\r\n'.join(lines) + '\r\n').encode('cp1252')
    except UnicodeEncodeError as exc:
        errors.append(f'{path}: dialogue not CP1252-compatible for legacy bridge: {exc}')
        continue
    try:
        raw = base64.b64decode(doc['compatibility']['legacy_payload_base64'], validate=True)
    except Exception as exc:
        errors.append(f'{path}: invalid compatibility payload: {exc}')
        continue
    if raw != expected:
        errors.append(f'{path}: compatibility payload differs from Portuguese canonical lines')

if len(found) != 37:
    errors.append(f'expected 37 elementary quest canonical resources/aliases, found {len(found)}')

# Prove the normal client path has a default compiled-resource bootstrap.
# This checkout uses the newer executable-relative RuntimeContentPath authority,
# which includes windows.h and therefore cannot be host-compiled on this Linux validator.
uses_candidates = 'Content/packs/text_resources.sres' in header
uses_runtime_resolver = 'ResolveRuntimePackPath("text_resources.sres")' in header
if not (uses_candidates or uses_runtime_resolver):
    errors.append('text-resource runtime has no default pack bootstrap')
if uses_runtime_resolver:
    runtime_path = root / 'Shared/content/v1/runtime/RuntimeContentPath.h'
    if not runtime_path.exists():
        errors.append('RuntimeContentPath.h missing for text-resource bootstrap')
    else:
        runtime_text = runtime_path.read_text(encoding='utf-8')
        for token in ('RuntimeExecutableDirectory()', 'ResolveRuntimePackPath'):
            if token not in runtime_text:
                errors.append('runtime content resolver missing: ' + token)
        if 'Content' not in runtime_text or 'packs' not in runtime_text:
            errors.append('runtime content resolver does not target Content/packs')
pack = root / 'Content/packs/text_resources.sres'
if not pack.exists() or pack.stat().st_size == 0:
    errors.append('compiled text_resources.sres missing or empty')

if errors:
    for error in errors:
        print('ERROR:', error, file=sys.stderr)
    sys.exit(1)

print(f'PATCH 194 gate: OK ({len(found)} elementary quest resources in pt-BR; Amanda flow presented by UI V2; text pack auto-bootstrap active)')
