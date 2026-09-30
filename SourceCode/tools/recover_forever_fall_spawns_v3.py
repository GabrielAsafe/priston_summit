#!/usr/bin/env python3
from __future__ import annotations

import argparse
import hashlib
import json
import os
import shutil
import subprocess
import sys
import tarfile
import tempfile
import zipfile
from pathlib import Path

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
            dirs[:] = [d for d in dirs if d not in {'.git', '.svn', 'build', 'Build', 'obj', 'bin'}]
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
                    'path': p,
                })
    return found


def scan_archives(archive_root: Path, found):
    # Archive fallback only for targets still missing from direct preferred sources.
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
            # Recovery should keep scanning other archives; report only selected inputs.
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


def spawn_counts(canonical: Path):
    result = {}
    for e in load_index(canonical):
        logical = e['logical_path'].casefold()
        if Path(logical).name in TARGETS:
            d = json.loads((canonical / e['json_path']).read_text(encoding='utf-8'))
            result[Path(logical).name] = {
                'actors': len(d.get('actors', [])),
                'bosses': len(d.get('bosses', [])),
                'actor_weight_sum': sum(int(x.get('weight', 0)) for x in d.get('actors', [])),
                'json_path': e['json_path'],
            }
    return result


def main() -> int:
    here = Path(__file__).resolve()
    source = here.parents[1]
    ap = argparse.ArgumentParser(description='Recover Forever Fall SPMs, canonicalize them and rebuild SSPB safely.')
    ap.add_argument('--archive-root', type=Path, default=Path.home() / 'armazem/priston')
    ap.add_argument('--source-code', type=Path, default=source)
    ap.add_argument('--canonical-root', type=Path, default=None, help='Canonical root containing stage_spawns/ and text/. Defaults to SourceCode/Content/canonical when present.')
    ap.add_argument('--apply', action='store_true', help='Deprecated: standalone SSPB install is disabled; use v4 for field_runtime.sfld.')
    ap.add_argument('--report', type=Path, default=None)
    args = ap.parse_args()
    if args.apply:
        raise SystemExit('APPLY_DISABLED_OBSOLETE_STAGE_SPAWN_PACK: use recover_forever_fall_spawns_v4.py with field_runtime.sfld')

    source = args.source_code.expanduser().resolve()
    archive_root = args.archive_root.expanduser().resolve()
    default_canonical_root = source / 'Content/canonical'
    canonical_root = (args.canonical_root.expanduser().resolve() if args.canonical_root else default_canonical_root)
    canonical = canonical_root / 'stage_spawns'
    canonical_text = canonical_root / 'text'
    pack = source / 'Content/packs/stage_spawns.sspb'
    tools = source / 'tools/content_port'
    report_path = (args.report.expanduser().resolve() if args.report else source / 'Content/forever_fall_recovery_report.json')

    if not archive_root.is_dir():
        raise SystemExit(f'ARCHIVE_ROOT_NOT_FOUND={archive_root}')
    if not canonical_root.is_dir():
        raise SystemExit(f'CANONICAL_ROOT_NOT_FOUND={canonical_root}')
    print(f'SOURCE_CODE={source}')
    print(f'CANONICAL_ROOT={canonical_root}')
    print(f'STAGE_SPAWN_CANONICAL={canonical}')
    print(f'PACK_OUTPUT={pack}')
    for required in (canonical / 'index.json', canonical_text / 'index.json', tools / 'import_corpus.py', tools / 'canonicalize_stage_spawns.py', tools / 'compile_stage_spawns_canonical.py', tools / 'inspect_stage_spawns.py'):
        if not required.exists():
            raise SystemExit(f'MISSING_REQUIRED={required}')

    found = scan_direct(archive_root)
    scan_archives(archive_root, found)
    chosen = choose_candidates(found)

    with tempfile.TemporaryDirectory(prefix='summit_forever_fall_') as td:
        tmp = Path(td)
        corpus = tmp / 'corpus'
        field = corpus / 'Servidor/GameServer/Field'
        for target, candidate in chosen.items():
            materialize(candidate, field / target)

        shadow = tmp / 'shadow'
        recovered = tmp / 'recovered_canonical'
        merged = tmp / 'merged_canonical'
        candidate_pack = tmp / 'stage_spawns.sspb'

        run(sys.executable, tools / 'import_corpus.py', '--corpus', corpus, '--source-root', source, '--out', shadow)
        run(sys.executable, tools / 'canonicalize_stage_spawns.py', '--shadow', shadow, '--canonical-text', canonical_text, '--out', recovered)

        recovered_counts = spawn_counts(recovered)
        missing_recovered = [t for t in TARGETS if t not in recovered_counts]
        empty_recovered = [t for t, c in recovered_counts.items() if c['actors'] + c['bosses'] == 0]
        if missing_recovered:
            raise SystemExit('RECOVERED_CANONICAL_MISSING=' + ','.join(missing_recovered))
        if empty_recovered:
            raise SystemExit('RECOVERED_CANONICAL_EMPTY=' + ','.join(empty_recovered))

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

        run(sys.executable, tools / 'compile_stage_spawns_canonical.py', merged, candidate_pack)
        run(sys.executable, tools / 'inspect_stage_spawns.py', candidate_pack)

        merged_counts = spawn_counts(merged)
        still_empty = [t for t in TARGETS if t not in merged_counts or merged_counts[t]['actors'] + merged_counts[t]['bosses'] == 0]
        if still_empty:
            raise SystemExit('MERGED_CANONICAL_EMPTY=' + ','.join(still_empty))

        report = {
            'schema': 'summit.forever-fall-recovery/v1',
            'archive_root': str(archive_root),
            'source_code': str(source),
            'canonical_root': str(canonical_root),
            'source_priority': [x[0] for x in SOURCE_PRIORITY],
            'selected_sources': {
                target: {k: v for k, v in candidate.items() if k not in {'path', 'archive'}}
                for target, candidate in chosen.items()
            },
            'before': spawn_counts(canonical),
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
            shutil.copy2(candidate_pack, pack)
            report['installed_pack'] = {'path': str(pack), 'bytes': pack.stat().st_size, 'sha256': sha256_file(pack)}

        report_path.parent.mkdir(parents=True, exist_ok=True)
        report_path.write_text(json.dumps(report, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
        print('FOREVER_FALL_RECOVERY=' + ('APPLIED' if args.apply else 'VALIDATED_DRY_RUN'))
        for target in TARGETS:
            c = merged_counts[target]
            print(f"{target}: actors={c['actors']} bosses={c['bosses']} weight_sum={c['actor_weight_sum']}")
        print(f'REPORT={report_path}')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
