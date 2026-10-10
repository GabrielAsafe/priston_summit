#!/usr/bin/env python3
from pathlib import Path
import json
import sys

ROOT = Path(__file__).resolve().parents[1]
CANON = ROOT / 'Content/canonical/stage_spawns'
TEXT = ROOT / 'Content/canonical/text'
TARGETS = {
    46: 'slab.ase.spm',
    47: 'ancientw.ase.spm',
    48: 'seaa.ase.spm',
    49: 'stemple.ase.spm',
    50: 'ruin-arenasf.ase.spm',
    51: 'lost3.ase.spm',
    52: 'dark_boss.ase.spm',
}

def load_monster_ids():
    index = json.loads((TEXT / 'index.json').read_text(encoding='utf-8'))
    return {
        str(entry.get('id'))
        for entry in index.get('documents', [])
        if entry.get('kind') == 'monster_definition' and entry.get('id')
    }

idx = json.loads((CANON / 'index.json').read_text(encoding='utf-8'))
by = {str(e['logical_path']).replace('\\', '/').casefold(): e for e in idx.get('documents', [])}
monster_ids = load_monster_ids()
blocked = []
for field, name in TARGETS.items():
    logical = ('GameServer/Field/' + name).casefold()
    entry = by.get(logical)
    if not entry:
        print(f'FIELD={field} FILE={name} STATUS=MISSING_CANONICAL_DOCUMENT')
        blocked.append(field)
        continue
    doc = json.loads((CANON / entry['json_path']).read_text(encoding='utf-8'))
    actors = doc.get('actors') or []
    bosses = doc.get('bosses') or []
    missing_ids = []
    for actor in actors:
        monster_id = str(actor.get('monster_id') or '')
        if not monster_id or monster_id not in monster_ids:
            missing_ids.append(monster_id or '<empty>')
    for boss in bosses:
        master = str(boss.get('master_monster_id') or '')
        if not master or master not in monster_ids:
            missing_ids.append(master or '<empty>')
        for slave in boss.get('slaves') or []:
            monster_id = str(slave.get('monster_id') or '')
            if not monster_id or monster_id not in monster_ids:
                missing_ids.append(monster_id or '<empty>')
    status = 'PASS' if (actors or bosses) and not missing_ids else 'EMPTY_OR_UNRESOLVED'
    print(
        f'FIELD={field} FILE={name} ACTORS={len(actors)} BOSSES={len(bosses)} '
        f'UNRESOLVED={len(missing_ids)} STATUS={status}'
    )
    if missing_ids:
        print('  MISSING_IDS=' + ','.join(sorted(set(missing_ids))))
    if status != 'PASS':
        blocked.append(field)

if blocked:
    print('TARGET_MAP_SPAWN_COVERAGE=BLOCKED fields=' + ','.join(map(str, blocked)))
    sys.exit(2)

# The private field 52 is functionally populated, but exact boss parity remains
# explicitly unresolved until an authoritative Eragon monster definition/assets
# are restored. This is a parity warning, not an empty-map failure.
field52 = by.get('gameserver/field/dark_boss.ase.spm')
if field52:
    doc52 = json.loads((CANON / field52['json_path']).read_text(encoding='utf-8'))
    if not (doc52.get('bosses') or []):
        print('FIELD=52 ERAGON_BOSS_PARITY=BLOCKED_MISSING_CANONICAL_DEFINITION')

print('TARGET_MAP_SPAWN_COVERAGE=PASS fields=46-52')
