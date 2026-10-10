#!/usr/bin/env python3
from pathlib import Path
import hashlib
import json
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]
CANON = ROOT / 'Content/canonical/stage_spawns'
TEXT = ROOT / 'Content/canonical/text'
TOOLS = ROOT / 'tools/content_port'

expected = {
    'GameServer/Field/slab.ase.spm': {
        'actors': {
            'monster.slab_114_t_u00e9cnico_invasor',
            'monster.slab_114_t_u00e9cnico_do_laborat_u00f3rio',
            'monster.slab_114_engenheiro_do_laborat_u00f3rio',
            'monster.slab_115_soldado_invasor',
            'monster.slab_115_bombardeiro_invasor',
            'monster.slab_116_guardi_u00e3o_do_laborat_u00f3rio',
            'monster.slab_116_elite_invasor',
        },
        'bosses': {'monster.130_draxos'},
    },
    'GameServer/Field/ancientw.ase.spm': {
        'actors': {
            'monster.slab_118_t_u00e9cnico_zumbi',
            'monster.slab_118_soldado_zumbi',
            'monster.slab_119_bombardeiro_zumbi',
            'monster.slab_119_elite_zumbi',
            'monster.120_nihil',
            'monster.acero',
            'monster.121_chalybs',
        },
        'bosses': {'monster.chefe_121_greedy'},
    },
    'GameServer/Field/seaa.ase.spm': {
        'actors': {
            'monster.119_gurkob', 'monster.119_dey', 'monster.120_koon',
            'monster.121_faugn', 'monster.122_shogoth', 'monster.123_arhdyra',
            'monster.124_sathla',
        },
        'bosses': {'monster.130_yagditha'},
    },
}

index = json.loads((CANON / 'index.json').read_text(encoding='utf-8'))
by = {e['logical_path'].replace('\\','/').casefold(): e for e in index.get('documents', [])}
errors=[]

def doc(logical):
    entry=by.get(logical.casefold())
    if not entry:
        errors.append('missing index entry: '+logical)
        return None
    p=CANON/entry['json_path']
    if not p.is_file():
        errors.append('missing spawn doc: '+str(p.relative_to(ROOT)))
        return None
    return json.loads(p.read_text(encoding='utf-8'))

for logical, wanted in expected.items():
    d=doc(logical)
    if not d: continue
    actors={str(x.get('monster_id') or '') for x in d.get('actors') or []}
    bosses={str(x.get('master_monster_id') or '') for x in d.get('bosses') or []}
    if actors != wanted['actors']:
        errors.append(f'{logical}: actor roster mismatch expected={sorted(wanted["actors"])} actual={sorted(actors)}')
    if bosses != wanted['bosses']:
        errors.append(f'{logical}: boss roster mismatch expected={sorted(wanted["bosses"])} actual={sorted(bosses)}')

for logical in (
    'GameServer/Field/stemple.ase.spm',
    'GameServer/Field/ruin-arenasf.ase.spm',
    'GameServer/Field/lost3.ase.spm',
    'GameServer/Field/dark_boss.ase.spm',
):
    d=doc(logical)
    if d and not ((d.get('actors') or []) or (d.get('bosses') or [])):
        errors.append(logical+': reconstructed spawn must be non-empty')

recovery_path=CANON/'recovery_patch_207.json'
if not recovery_path.is_file():
    errors.append('missing recovery_patch_207.json')
else:
    recovery=json.loads(recovery_path.read_text(encoding='utf-8'))
    if recovery.get('patch') != 207:
        errors.append('recovery metadata patch must be 207')
    blocked=' '.join(recovery.get('blocked_exact_parity') or []).casefold()
    for token in ('field49','field50','field51','field52','eragon'):
        if token not in blocked:
            errors.append('recovery metadata must preserve exact-parity warning: '+token)

coverage=subprocess.run(
    [sys.executable, str(ROOT/'tools/validate_target_map_spawn_coverage_v1.py')],
    text=True, capture_output=True,
)
if coverage.returncode != 0:
    errors.append('target map spawn coverage failed:\n'+coverage.stdout+coverage.stderr)
if 'ERAGON_BOSS_PARITY=BLOCKED_MISSING_CANONICAL_DEFINITION' not in coverage.stdout:
    errors.append('field52 Eragon exact-boss parity warning missing')

# Checked-in field_runtime must be exactly reproducible from canonical sources.
with tempfile.TemporaryDirectory(prefix='summit_patch207_') as td:
    rebuilt=Path(td)/'field_runtime.sfld'
    subprocess.run([
        sys.executable, str(TOOLS/'compile_field_runtime.py'),
        '--canonical-spawns', str(CANON),
        '--canonical-start-points', str(ROOT/'Content/canonical/stage_start_points'),
        '--canonical-fixed-npcs', str(ROOT/'Content/canonical/fixed_npcs'),
        '--canonical-text', str(TEXT),
        str(rebuilt),
    ], check=True, stdout=subprocess.DEVNULL)
    checked=ROOT/'Content/packs/field_runtime.sfld'
    if not checked.is_file() or rebuilt.read_bytes() != checked.read_bytes():
        def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest() if p.is_file() else '<missing>'
        errors.append(f'field_runtime drift checked={sha(checked)} rebuilt={sha(rebuilt)}')

if errors:
    print('\n'.join('ERROR: '+e for e in errors))
    raise SystemExit(1)
print('PATCH 207 gate: OK — fields 46-52 have functional canonical spawn coverage; exact private/variant parity remains explicitly documented')
