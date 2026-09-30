#!/usr/bin/env python3
from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys
import tarfile
import tempfile
import zipfile
from collections import defaultdict
from pathlib import Path, PurePosixPath

TARGETS = [
    'forever-fall-01.ase.spm',
    'forever-fall-02.ase.spm',
    'forever-fall-03.ase.spm',
    'forever-fall-04.ase.spm',
]
SOURCE_PRIORITY = [
    ('PT_EU', 'servidor_cliente_PT_EU'),
    ('ExMachina', 'ExMachina'),
    ('priston-main_src', 'priston-main_src'),
]


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def sha256_file(path: Path) -> str:
    h = hashlib.sha256()
    with path.open('rb') as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b''):
            h.update(chunk)
    return h.hexdigest()


def candidate_score(rel: str) -> int:
    s = rel.replace('\\', '/').casefold()
    score = 0
    if '/gameserver/field/' in '/' + s:
        score += 100
    if '/server/gameserver/field/' in '/' + s:
        score += 40
    if '/servidor/gameserver/field/' in '/' + s:
        score += 40
    return score


def scan_direct(archive_root: Path):
    found = {t: [] for t in TARGETS}
    for rank, (label, folder) in enumerate(SOURCE_PRIORITY):
        src = archive_root / folder
        if not src.is_dir():
            continue
        for base, dirs, files in os.walk(src):
            dirs[:] = [d for d in dirs if d not in {'.git', '.svn', 'build', 'Build', 'obj'}]
            for fn in files:
                key = fn.casefold()
                if key not in found:
                    continue
                p = Path(base) / fn
                rel = p.relative_to(src).as_posix()
                found[key].append({
                    'kind': 'file', 'label': label, 'rank': rank,
                    'where': str(p), 'rel': rel,
                    'score': candidate_score(rel),
                    'size': p.stat().st_size, 'sha256': sha256_file(p),
                    'path': p, 'source_root': src,
                })
    return found


def scan_archives(archive_root: Path, found):
    # Archive fallback is only needed when no preferred direct copy exists.
    missing = {t for t, rows in found.items() if not rows}
    if not missing:
        return
    for p in sorted(archive_root.rglob('*'), key=lambda x: str(x).casefold()):
        if not p.is_file():
            continue
        lower = p.name.casefold()
        try:
            if lower.endswith('.zip'):
                with zipfile.ZipFile(p) as z:
                    for info in z.infolist():
                        target = Path(info.filename.replace('\\', '/')).name.casefold()
                        if target not in missing:
                            continue
                        data = z.read(info)
                        found[target].append({
                            'kind': 'zip', 'label': 'ARCHIVE', 'rank': 50,
                            'where': f'{p}::{info.filename}', 'rel': info.filename,
                            'score': candidate_score(info.filename),
                            'size': len(data), 'sha256': sha256_bytes(data),
                            'archive': p, 'member': info.filename,
                        })
            elif lower.endswith(('.tar', '.tar.gz', '.tgz', '.tar.xz', '.txz', '.tar.bz2', '.tbz2')):
                with tarfile.open(p, 'r:*') as t:
                    for m in t.getmembers():
                        if not m.isfile():
                            continue
                        target = Path(m.name.replace('\\', '/')).name.casefold()
                        if target not in missing:
                            continue
                        f = t.extractfile(m)
                        data = f.read() if f else b''
                        found[target].append({
                            'kind': 'tar', 'label': 'ARCHIVE', 'rank': 50,
                            'where': f'{p}::{m.name}', 'rel': m.name,
                            'score': candidate_score(m.name),
                            'size': len(data), 'sha256': sha256_bytes(data),
                            'archive': p, 'member': m.name,
                        })
        except Exception:
            continue


def choose_candidates(found):
    chosen = {}
    for target in TARGETS:
        rows = found[target]
        if not rows:
            raise SystemExit(f'MISSING_SOURCE={target}')
        rows.sort(key=lambda r: (r['rank'], -r['score'], r['where'].casefold()))
        chosen[target] = rows[0]
    return chosen


def materialize(candidate: dict, destination: Path) -> None:
    destination.parent.mkdir(parents=True, exist_ok=True)
    if candidate['kind'] == 'file':
        shutil.copy2(candidate['path'], destination)
        return
    if candidate['kind'] == 'zip':
        with zipfile.ZipFile(candidate['archive']) as z:
            destination.write_bytes(z.read(candidate['member']))
        return
    if candidate['kind'] == 'tar':
        with tarfile.open(candidate['archive'], 'r:*') as t:
            f = t.extractfile(candidate['member'])
            if not f:
                raise RuntimeError(f"cannot extract {candidate['where']}")
            destination.write_bytes(f.read())
        return
    raise RuntimeError(f"unsupported candidate kind: {candidate['kind']}")


def run(*args: object) -> None:
    print('+', ' '.join(str(x) for x in args), flush=True)
    subprocess.run([str(x) for x in args], check=True)


def load_index(root: Path):
    return json.loads((root / 'index.json').read_text(encoding='utf-8'))['documents']


def spawn_docs(canonical: Path):
    result = {}
    for e in load_index(canonical):
        logical = e['logical_path'].casefold()
        name = Path(logical).name
        if name in TARGETS:
            d = json.loads((canonical / e['json_path']).read_text(encoding='utf-8'))
            result[name] = (e, d)
    return result


def spawn_counts(canonical: Path):
    result = {}
    for target, (e, d) in spawn_docs(canonical).items():
        actors = d.get('actors', [])
        bosses = d.get('bosses', [])
        result[target] = {
            'actors': len(actors),
            'bosses': len(bosses),
            'actor_weight_sum': sum(int(x.get('weight', 0)) for x in actors),
            'resolved_actors': sum(bool(x.get('monster_id')) for x in actors),
            'resolved_boss_refs': sum(bool(x.get('master_monster_id')) + bool(x.get('slave_monster_id')) for x in bosses),
            'json_path': e['json_path'],
        }
    return result


def norm_model(value: object) -> str:
    return str(value or '').replace('\\', '/').strip().casefold()


def is_top_level_monster(doc: dict) -> bool:
    p = PurePosixPath(str(doc.get('source', {}).get('legacy_path', '')).replace('\\', '/'))
    parts = [x.casefold() for x in p.parts]
    return len(parts) == 4 and parts[:3] == ['servidor', 'gameserver', 'monster'] and p.suffix.casefold() == '.inf'


def canonical_monster_indexes(canonical_text: Path):
    by_sig = defaultdict(list)
    by_model = defaultdict(list)
    by_id = {}
    for e in load_index(canonical_text):
        if e.get('kind') != 'monster_definition':
            continue
        d = json.loads((canonical_text / e['json_path']).read_text(encoding='utf-8'))
        if not is_top_level_monster(d):
            continue
        f = d.get('semantic', {}).get('fields', {})
        model = norm_model(f.get('model_file'))
        if not model:
            continue
        try:
            level = int(f.get('level'))
        except (TypeError, ValueError):
            level = None
        row = {'id': d.get('id', ''), 'name': str(f.get('name', '')), 'model': model, 'level': level}
        by_model[model].append(row)
        by_sig[(model, level)].append(row)
        if row['id']:
            by_id[row['id']] = row
    return by_sig, by_model, by_id


def base_monster_score(row: dict, expected_level: int | None) -> tuple[int, str]:
    mid = row.get('id', '')
    score = 0
    if re.match(r'^monster\.\d+_', mid):
        score += 100
    bad_markers = ('.r1', '.r2', '.r3', '.r4', '.r5', '.r6', '.r7', '.r8', '.sod_', '.new_sod_', '.chefe_', '.mini_', '.desafio_')
    if any(x in mid for x in bad_markers):
        score -= 80
    if expected_level is not None and row.get('level') == expected_level:
        score += 30
    return (-score, mid)


def choose_canonical_monster(rows: list[dict], expected_level: int | None):
    if not rows:
        return None
    ordered = sorted(rows, key=lambda r: base_monster_score(r, expected_level))
    if len(ordered) == 1:
        return ordered[0]
    # Only accept the winner when its heuristic score is strictly stronger.
    a = base_monster_score(ordered[0], expected_level)[0]
    b = base_monster_score(ordered[1], expected_level)[0]
    if a < b:
        return ordered[0]
    # Common case: a normal base monster and an R/SoD clone share the signature.
    base = [r for r in ordered if re.match(r'^monster\.\d+_', r.get('id', ''))]
    if len(base) == 1:
        return base[0]
    return None


def decode_cp949(raw: bytes) -> str:
    for enc in ('cp949', 'euc_kr'):
        try:
            return raw.decode(enc)
        except UnicodeDecodeError:
            pass
    try:
        return raw.decode('utf-8')
    except UnicodeDecodeError:
        return raw.decode('latin-1', errors='replace')


def scan_source_monster_aliases(source_roots: list[Path], source_code: Path, canonical_text: Path):
    tools = source_code / 'tools/content_port'
    sys.path.insert(0, str(tools))
    from legacy_text import lex_bytes  # type: ignore
    from source_tokens import load_source_token_maps  # type: ignore

    token_map = load_source_token_maps(source_code)['character']
    by_sig, by_model, by_id = canonical_monster_indexes(canonical_text)
    aliases = {}
    conflicts = defaultdict(list)
    scanned = 0
    matched = 0

    def parse_inf(path: Path):
        name_raw = None
        model = ''
        level = None
        try:
            data = path.read_bytes()
        except OSError:
            return None
        for line in data.splitlines():
            toks = lex_bytes(line)
            if len(toks) < 2 or not toks[0].raw.startswith(b'*'):
                continue
            key = token_map.get(toks[0].raw)
            if key == 'name':
                name_raw = toks[1].raw
            elif key == 'model_file':
                model = norm_model(toks[1].text)
            elif key == 'level':
                try:
                    level = int(toks[1].text)
                except (TypeError, ValueError):
                    pass
        if not name_raw or not model:
            return None
        return name_raw, model, level

    seen_paths = set()
    for root in source_roots:
        if not root.is_dir():
            continue
        for p in root.rglob('*'):
            if not p.is_file() or p.suffix.casefold() != '.inf':
                continue
            s = '/' + p.as_posix().casefold()
            if '/gameserver/monster/' not in s:
                continue
            key_path = str(p.resolve())
            if key_path in seen_paths:
                continue
            seen_paths.add(key_path)
            scanned += 1
            parsed = parse_inf(p)
            if not parsed:
                continue
            raw_name, model, level = parsed
            rows = by_sig.get((model, level), []) or by_model.get(model, [])
            chosen = choose_canonical_monster(rows, level)
            if not chosen:
                continue
            matched += 1
            mid = chosen['id']
            previous = aliases.get(raw_name)
            if previous and previous['id'] != mid:
                conflicts[raw_name].extend([previous, chosen])
                aliases.pop(raw_name, None)
                continue
            if raw_name not in conflicts:
                aliases[raw_name] = {
                    'id': mid,
                    'canonical_name': chosen.get('name', ''),
                    'source_name': decode_cp949(raw_name),
                    'model': model,
                    'level': level,
                    'source_file': str(p),
                }

    return aliases, conflicts, by_id, {'source_monster_files_scanned': scanned, 'source_monsters_matched': matched, 'aliases': len(aliases), 'conflicts': len(conflicts)}


def patch_unresolved_ids(recovered: Path, aliases: dict, by_id: dict):
    patched = []
    unresolved = []
    docs = spawn_docs(recovered)
    for target in TARGETS:
        if target not in docs:
            continue
        e, d = docs[target]
        changed = False
        for actor in d.get('actors', []):
            raw_hex = actor.get('legacy_name_bytes_hex', '')
            raw = bytes.fromhex(raw_hex) if raw_hex else b''
            if not actor.get('monster_id') and raw in aliases:
                a = aliases[raw]
                actor['monster_id'] = a['id']
                actor['resolution'] = 'source_monster_signature'
                actor['legacy_name'] = a['source_name']
                actor['canonical_name'] = a['canonical_name']
                changed = True
                patched.append({'spawn': target, 'role': 'actor', 'source_name': a['source_name'], 'monster_id': a['id'], 'canonical_name': a['canonical_name'], 'weight': int(actor.get('weight', 0))})
            if not actor.get('monster_id'):
                unresolved.append({'spawn': target, 'role': 'actor', 'raw_hex': raw_hex, 'cp949_name': decode_cp949(raw), 'weight': int(actor.get('weight', 0))})
        for boss in d.get('bosses', []):
            for role, id_key, name_hex_key, display_key in (
                ('boss_master', 'master_monster_id', 'master_legacy_name_bytes_hex', 'master_legacy_name'),
                ('boss_slave', 'slave_monster_id', 'slave_legacy_name_bytes_hex', 'slave_legacy_name'),
            ):
                raw_hex = boss.get(name_hex_key, '')
                raw = bytes.fromhex(raw_hex) if raw_hex else b''
                if not boss.get(id_key) and raw in aliases:
                    a = aliases[raw]
                    boss[id_key] = a['id']
                    boss[role.replace('boss_', '') + '_resolution'] = 'source_monster_signature'
                    boss[display_key] = a['source_name']
                    changed = True
                    patched.append({'spawn': target, 'role': role, 'source_name': a['source_name'], 'monster_id': a['id'], 'canonical_name': a['canonical_name']})
                if not boss.get(id_key):
                    unresolved.append({'spawn': target, 'role': role, 'raw_hex': raw_hex, 'cp949_name': decode_cp949(raw)})
        if changed:
            (recovered / e['json_path']).write_text(json.dumps(d, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    return patched, unresolved


def main() -> int:
    here = Path(__file__).resolve()
    source_default = here.parents[1]
    ap = argparse.ArgumentParser(description='Recover Forever Fall SPMs and resolve Korean spawn names through source monster model/level signatures.')
    ap.add_argument('--archive-root', type=Path, default=Path.home() / 'armazem/priston')
    ap.add_argument('--source-code', type=Path, default=source_default)
    ap.add_argument('--canonical-root', type=Path, required=True, help='Canonical root containing stage_spawns/ and text/.')
    ap.add_argument('--apply', action='store_true', help='Install recovered canonical JSON and rebuild field_runtime.sfld after all monster IDs resolve.')
    ap.add_argument('--report', type=Path, default=None)
    args = ap.parse_args()

    source = args.source_code.expanduser().resolve()
    archive_root = args.archive_root.expanduser().resolve()
    canonical_root = args.canonical_root.expanduser().resolve()
    canonical = canonical_root / 'stage_spawns'
    canonical_text = canonical_root / 'text'
    pack = source / 'Content/packs/field_runtime.sfld'
    tools = source / 'tools/content_port'
    report_path = (args.report.expanduser().resolve() if args.report else source / 'Content/forever_fall_recovery_report.json')

    if not archive_root.is_dir():
        raise SystemExit(f'ARCHIVE_ROOT_NOT_FOUND={archive_root}')
    print(f'SOURCE_CODE={source}')
    print(f'CANONICAL_ROOT={canonical_root}')
    print(f'STAGE_SPAWN_CANONICAL={canonical}')
    print(f'PACK_OUTPUT={pack}')
    for required in (
        canonical / 'index.json', canonical_text / 'index.json',
        canonical_root / 'stage_start_points/index.json', canonical_root / 'fixed_npcs/index.json',
        tools / 'import_corpus.py', tools / 'canonicalize_stage_spawns.py',
        tools / 'compile_field_runtime.py', tools / 'inspect_field_runtime.py',
        tools / 'legacy_text.py', tools / 'source_tokens.py',
    ):
        if not required.exists():
            raise SystemExit(f'MISSING_REQUIRED={required}')

    found = scan_direct(archive_root)
    scan_archives(archive_root, found)
    chosen = choose_candidates(found)

    source_roots = []
    for _label, folder in SOURCE_PRIORITY:
        root = archive_root / folder
        if root.is_dir():
            source_roots.append(root)
    aliases, alias_conflicts, by_id, alias_stats = scan_source_monster_aliases(source_roots, source, canonical_text)
    print('MONSTER_ALIAS_SCAN=' + json.dumps(alias_stats, ensure_ascii=False, sort_keys=True))

    with tempfile.TemporaryDirectory(prefix='summit_forever_fall_') as td:
        tmp = Path(td)
        corpus = tmp / 'corpus'
        field = corpus / 'Servidor/GameServer/Field'
        for target, candidate in chosen.items():
            materialize(candidate, field / target)
            print(f"SELECTED_SOURCE target={target} label={candidate['label']} sha256={candidate['sha256']} where={candidate['where']}")

        shadow = tmp / 'shadow'
        recovered = tmp / 'recovered_canonical'
        merged = tmp / 'merged_canonical'
        candidate_pack = tmp / 'field_runtime.sfld'

        run(sys.executable, tools / 'import_corpus.py', '--corpus', corpus, '--source-root', source, '--out', shadow)
        run(sys.executable, tools / 'canonicalize_stage_spawns.py', '--shadow', shadow, '--canonical-text', canonical_text, '--out', recovered)

        recovered_counts_before_alias = spawn_counts(recovered)
        patched_aliases, unresolved = patch_unresolved_ids(recovered, aliases, by_id)
        recovered_counts = spawn_counts(recovered)

        missing_recovered = [t for t in TARGETS if t not in recovered_counts]
        empty_recovered = [t for t, c in recovered_counts.items() if c['actors'] + c['bosses'] == 0]
        if missing_recovered:
            raise SystemExit('RECOVERED_CANONICAL_MISSING=' + ','.join(missing_recovered))
        if empty_recovered:
            raise SystemExit('RECOVERED_CANONICAL_EMPTY=' + ','.join(empty_recovered))

        print('FOREVER_FALL_RESOLVED_ROSTER_BEGIN')
        for target in TARGETS:
            _e, doc = spawn_docs(recovered)[target]
            for actor in doc.get('actors', []):
                raw = bytes.fromhex(actor.get('legacy_name_bytes_hex', '')) if actor.get('legacy_name_bytes_hex') else b''
                mid = actor.get('monster_id', '')
                cname = by_id.get(mid, {}).get('name', actor.get('canonical_name', ''))
                print(f"ROSTER map={target} weight={int(actor.get('weight',0))} source_name={decode_cp949(raw)!r} monster_id={mid!r} canonical_name={cname!r} resolution={actor.get('resolution','')}")
            for boss in doc.get('bosses', []):
                print(f"BOSS map={target} master={boss.get('master_monster_id','')!r} slave={boss.get('slave_monster_id','')!r} count={boss.get('slave_count',0)} hours={boss.get('open_hours',[])}")
        print('FOREVER_FALL_RESOLVED_ROSTER_END')

        if unresolved:
            for row in unresolved:
                print('UNRESOLVED_MONSTER ' + json.dumps(row, ensure_ascii=False, sort_keys=True))
            raise SystemExit(f'UNRESOLVED_MONSTER_IDS={len(unresolved)}')

        shutil.copytree(canonical, merged)
        existing = {e['logical_path'].casefold(): e for e in load_index(merged)}
        recovered_index = load_index(recovered)
        replacements = []
        for e in recovered_index:
            logical = e['logical_path'].casefold()
            if logical not in existing:
                raise SystemExit(f'CANONICAL_TARGET_NOT_FOUND={e["logical_path"]}')
            dst_entry = existing[logical]
            src_doc = recovered / e['json_path']
            dst_doc = merged / dst_entry['json_path']
            shutil.copy2(src_doc, dst_doc)
            replacements.append({'logical_path': e['logical_path'], 'json_path': dst_entry['json_path']})

        run(
            sys.executable, tools / 'compile_field_runtime.py',
            '--canonical-spawns', merged,
            '--canonical-start-points', canonical_root / 'stage_start_points',
            '--canonical-fixed-npcs', canonical_root / 'fixed_npcs',
            '--canonical-text', canonical_text,
            candidate_pack,
        )
        run(sys.executable, tools / 'inspect_field_runtime.py', candidate_pack)

        merged_counts = spawn_counts(merged)
        still_empty = [t for t in TARGETS if t not in merged_counts or merged_counts[t]['actors'] + merged_counts[t]['bosses'] == 0]
        not_resolved = [t for t in TARGETS if merged_counts[t]['resolved_actors'] != merged_counts[t]['actors']]
        if still_empty:
            raise SystemExit('MERGED_CANONICAL_EMPTY=' + ','.join(still_empty))
        if not_resolved:
            raise SystemExit('MERGED_CANONICAL_UNRESOLVED=' + ','.join(not_resolved))

        report = {
            'schema': 'summit.forever-fall-recovery/v2',
            'archive_root': str(archive_root),
            'source_code': str(source),
            'canonical_root': str(canonical_root),
            'source_priority': [x[0] for x in SOURCE_PRIORITY],
            'selected_sources': {
                target: {k: v for k, v in candidate.items() if k not in {'path', 'archive', 'source_root'}}
                for target, candidate in chosen.items()
            },
            'monster_alias_stats': alias_stats,
            'monster_alias_conflicts': {
                raw.hex(): [{'id': x.get('id',''), 'name': x.get('name','')} for x in rows]
                for raw, rows in alias_conflicts.items()
            },
            'patched_aliases': patched_aliases,
            'before': spawn_counts(canonical),
            'recovered_before_alias_resolution': recovered_counts_before_alias,
            'after': merged_counts,
            'candidate_pack': {'bytes': candidate_pack.stat().st_size, 'sha256': sha256_file(candidate_pack)},
            'replacements': replacements,
            'applied': bool(args.apply),
        }

        if args.apply:
            actual_index = {e['logical_path'].casefold(): e for e in load_index(canonical)}
            for e in recovered_index:
                dst_entry = actual_index[e['logical_path'].casefold()]
                shutil.copy2(recovered / e['json_path'], canonical / dst_entry['json_path'])
            pack.parent.mkdir(parents=True, exist_ok=True)
            run(
                sys.executable, tools / 'compile_field_runtime.py',
                '--canonical-spawns', canonical,
                '--canonical-start-points', canonical_root / 'stage_start_points',
                '--canonical-fixed-npcs', canonical_root / 'fixed_npcs',
                '--canonical-text', canonical_text,
                pack,
            )
            run(sys.executable, tools / 'inspect_field_runtime.py', pack)
            report['installed_pack'] = {'path': str(pack), 'bytes': pack.stat().st_size, 'sha256': sha256_file(pack)}

        report_path.parent.mkdir(parents=True, exist_ok=True)
        report_path.write_text(json.dumps(report, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
        print('FOREVER_FALL_RECOVERY=' + ('APPLIED' if args.apply else 'VALIDATED_DRY_RUN'))
        for target in TARGETS:
            c = merged_counts[target]
            print(f"{target}: actors={c['actors']} resolved={c['resolved_actors']} bosses={c['bosses']} weight_sum={c['actor_weight_sum']}")
        print(f'PACK_SHA256={sha256_file(candidate_pack)}')
        print(f'REPORT={report_path}')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
