#!/usr/bin/env python3
"""Compare and explicitly deploy verified ASE->SMD/SMB caches, with rollback.

No Wine, model conversion or persistent diagnostics. Every operation (including
apply) re-runs the independent, full catalog 267 gate before trusting staging.
Commands: compare (read only), plan (outside game), apply (explicit), rollback.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path, PurePosixPath
import shutil
import stat
import sys
import tempfile

from cook_native_models_259 import no_symlink_ancestors
from cook_native_models_incremental_263 import _hash, _previous, _safe_artifact
from verify_native_model_stage_267 import verify_staging
from verify_archived_native_stage_279 import verify_archived_readonly

PLAN_FORMAT = 'SUMMIT_NATIVE_DEPLOY_PLAN_278_V1'
BACKUP_FORMAT = 'SUMMIT_NATIVE_DEPLOY_BACKUP_278_V1'


def _fail(message: str):
    raise ValueError(message)


def _safe_dir(p: Path, must_exist: bool = True) -> Path:
    no_symlink_ancestors(p)
    p = p.absolute()
    if must_exist and not p.is_dir():
        _fail('directory missing: ' + str(p))
    return p.resolve(strict=must_exist)


def _protected_path(destination: Path, client: Path, server: Path, stage: Path):
    no_symlink_ancestors(destination)
    p = destination.absolute()
    if (p == client or client in p.parents or p == server.parent or server.parent in p.parents or
            p == stage or stage in p.parents or any(v.casefold() == 'canonical' for v in p.parts)):
        _fail('plan/backup cannot be placed in client, server, staging or canonical')
    if not p.parent.is_dir():
        _fail('output parent must already exist')
    if p.exists() or p.is_symlink():
        _fail('output path already occupied')


def _entry_target(client: Path, rel: str) -> Path:
    parts = _safe_artifact(rel).parts
    cursor = client
    for part in parts:
        if cursor.is_dir():
            if cursor.is_symlink():
                _fail('symlink in target path')
            # On Linux Wine can perform a case-insensitive lookup even when the
            # direct Python path is absent. Never create ambiguous shadow paths.
            matches = [name for name in os.listdir(cursor) if name.casefold() == part.casefold()]
            if matches and (len(matches) != 1 or matches[0] != part):
                _fail('case-insensitive target path collision: ' + str(cursor / part))
        cursor = cursor / part
        if cursor.is_symlink():
            _fail('symlink in target path: ' + str(cursor))
        if cursor.exists() and cursor != client / Path(*parts) and not cursor.is_dir():
            _fail('target parent is not a directory: ' + str(cursor))
    if cursor.exists() and not cursor.is_file():
        _fail('target is not a regular file: ' + str(cursor))
    return cursor


def _historical_existing_target(client: Path, rel: str) -> tuple[Path | None, bool]:
    """Read-only casefold lookup, never used by plan, apply or rollback.

    Each component must have at most ONE case-insensitive match. That unique
    existing spelling is used only to hash the current client's bytes; we do
    not rename, invent, or install a case-insensitive alias. Two matches are
    genuinely ambiguous on Windows/Wine and remain fail-closed.
    """
    parts = _safe_artifact(rel).parts
    cursor = client
    case_variant = False
    for index, part in enumerate(parts):
        if cursor.is_symlink() or not cursor.is_dir():
            _fail('target parent is not a safe directory: ' + str(cursor))
        matches = [entry for entry in os.scandir(cursor)
                   if entry.name.casefold() == part.casefold()]
        if len(matches) > 1:
            _fail('case-insensitive target path collision: ' + str(cursor / part))
        if not matches:
            return None, case_variant
        entry = matches[0]
        if entry.is_symlink():
            _fail('symlink in historical target path: ' + str(cursor / entry.name))
        case_variant |= entry.name != part
        cursor = cursor / entry.name
        if index < len(parts) - 1:
            if not entry.is_dir(follow_symlinks=False):
                _fail('target parent is not a directory: ' + str(cursor))
        elif not entry.is_file(follow_symlinks=False):
            _fail('target is not a regular file: ' + str(cursor))
    return cursor, case_variant


def _historical_inspect_target(client: Path, rel: str) -> tuple[Path | None, bool, list[str]]:
    """Only for archival compare: report genuinely ambiguous casefold paths.

    No candidate inside an ambiguous branch is opened or hashed. Never call
    this helper from plan/apply/rollback; their strict _entry_target remains.
    """
    parts = _safe_artifact(rel).parts
    cursor = client
    case_variant = False
    for index, part in enumerate(parts):
        if cursor.is_symlink() or not cursor.is_dir():
            _fail('target parent is not a safe directory: ' + str(cursor))
        matches = [entry for entry in os.scandir(cursor)
                   if entry.name.casefold() == part.casefold()]
        # Never hide a symlink behind an ambiguity report.
        if any(entry.is_symlink() for entry in matches):
            _fail('symlink in historical target path: ' + str(cursor / part))
        if len(matches) > 1:
            # This is a genuine conflict on Linux and a single ambiguous name
            # under Wine. Present the branch, not an arbitrarily chosen file.
            candidates = sorted((cursor / entry.name).relative_to(client).as_posix()
                                for entry in matches)
            return None, False, candidates
        if not matches:
            return None, case_variant, []
        entry = matches[0]
        case_variant |= entry.name != part
        cursor = cursor / entry.name
        if index < len(parts) - 1:
            if not entry.is_dir(follow_symlinks=False):
                _fail('target parent is not a directory: ' + str(cursor))
        elif not entry.is_file(follow_symlinks=False):
            _fail('target is not a regular file: ' + str(cursor))
    return cursor, case_variant, []


def _atomic_json(path: Path, data: dict):
    no_symlink_ancestors(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    text = json.dumps(data, ensure_ascii=False, indent=2, sort_keys=True) + '\n'
    with tempfile.NamedTemporaryFile(mode='w', encoding='utf-8', dir=path.parent,
                                     prefix='.summit278_', delete=False) as f:
        temp = Path(f.name)
        try:
            f.write(text)
            f.flush()
            os.fsync(f.fileno())
        except BaseException:
            temp.unlink(missing_ok=True)
            raise
    try:
        os.replace(temp, path)
    finally:
        temp.unlink(missing_ok=True)


def _atomic_copy(src: Path, target: Path, expected_sha: str, original: Path | None = None,
                 restore_metadata: bool = False):
    no_symlink_ancestors(src)
    no_symlink_ancestors(target)
    target.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile(dir=target.parent, prefix='.summit278_', delete=False) as f:
        temp = Path(f.name)
    try:
        shutil.copyfile(src, temp)
        if _hash(temp) != expected_sha:
            _fail('hash changed during copy: ' + str(src))
        if original is not None:
            if restore_metadata:
                shutil.copystat(original, temp, follow_symlinks=False)
            else:
                os.chmod(temp, stat.S_IMODE(original.stat().st_mode))
        else:
            os.chmod(temp, 0o644)
        with temp.open('rb') as synchronized:
            os.fsync(synchronized.fileno())
        os.replace(temp, target)
        if _hash(target) != expected_sha:
            _fail('hash changed during atomic publication: ' + str(target))
    finally:
        temp.unlink(missing_ok=True)


def _manifest_rows(manifest: dict) -> dict[str, dict]:
    result = {}
    for row in manifest['artifacts']:
        rel = _safe_artifact(row['cache']).as_posix()
        if rel.casefold() in (v.casefold() for v in result):
            _fail('case-insensitive collision in staging manifest')
        if row.get('sha256') is None or not isinstance(row.get('sha256'), str):
            _fail('missing staged cache digest')
        result[rel] = row
    return result


def _validated(client: Path, server: Path, stage: Path, exe: Path | None):
    for p in (client, server, stage):
        no_symlink_ancestors(p)
    result = verify_staging(client, server, stage, allow_partial=False, client_exe=exe)
    if not result['complete'] or result['missing'] or result['verified'] != result['expected']:
        _fail('staging 267 did not verify complete catalog')
    manifest_path = stage / 'manifest.json'
    manifest, _ = _previous(stage)
    return manifest, _hash(manifest_path), _manifest_rows(manifest)


def compare(client: Path, server: Path, stage: Path, exe: Path | None = None,
            details: bool = False, archival_readonly: bool = False):
    client = _safe_dir(client)
    server = server.resolve(strict=True)
    stage = _safe_dir(stage)
    if archival_readonly:
        # Historical provenance still requires the full 134/134-type hash/ASE
        # audit. It NEVER authorizes plan/apply, whose _validated() path is
        # unchanged and still requires the exact current 267 identity.
        verify_archived_readonly(client, server, stage, exe)
        manifest, _ = _previous(stage)
        rows = _manifest_rows(manifest)
    else:
        _, _, rows = _validated(client, server, stage, exe)
    results = []
    for rel, row in sorted(rows.items()):
        if archival_readonly:
            # The client's Linux filesystem may spell one directory or file
            # differently. Only this READ-ONLY report accepts a unique alias.
            target, case_variant, candidates = _historical_inspect_target(client, rel)
        else:
            target, case_variant, candidates = _entry_target(client, rel), False, []
        if candidates:
            # A complete provenance-verified staging cache can be compared to
            # other client files, but this target has no unique real identity.
            # In particular do NOT select an exact-spelling winner.
            digest = None
            state = 'AMBIGUOUS'
        elif target is not None and target.is_file():
            digest = _hash(target)
            state = 'SAME' if digest == row['sha256'] else 'DIFFERENT'
        else:
            digest = None
            state = 'MISSING'
        results.append({'cache': rel, 'state': state, 'existing_sha256': digest,
                        'staged_sha256': row['sha256'], 'case_variant': case_variant,
                        'ambiguous_candidates': candidates,
                        'existing_path': target.relative_to(client).as_posix() if target is not None else None})
    totals = {kind: sum(i['state'] == kind for i in results) for kind in ('SAME', 'DIFFERENT', 'MISSING')}
    return totals, results


def make_plan(client: Path, server: Path, stage: Path, exe: Path | None,
              plan_file: Path, sources: list[str], all_models: bool):
    if all_models == bool(sources):
        _fail('choose --all OR one or more --source ASE paths')
    client = _safe_dir(client)
    server = server.resolve(strict=True)
    stage = _safe_dir(stage)
    executable = (exe if exe is not None else client / 'Game.exe').resolve(strict=True)
    _protected_path(plan_file, client, server, stage)
    manifest, manifest_sha, rows = _validated(client, server, stage, executable)
    selected = []
    if all_models:
        selected = sorted(rows)
    else:
        required_sources = set()
        for source in sources:
            if (not source.endswith('.ASE') and not source.endswith('.ase')) or '\\' in source:
                _fail('selection must be an ASE source path')
            rel = PurePosixPath(source)
            if rel.is_absolute() or any(x in ('', '.', '.', '..') for x in source.split('/')):
                _fail('unsafe ASE source path')
            required_sources.add(rel.as_posix())
        known = {row['source'] for row in rows.values()}
        if not required_sources <= known:
            _fail('source not in verified manifest: ' + ', '.join(sorted(required_sources - known)))
        selected = sorted(rel for rel, row in rows.items() if row['source'] in required_sources)
    if not selected:
        _fail('no selected caches')
    plans = []
    for rel in selected:
        target = _entry_target(client, rel)
        row = rows[rel]
        plans.append({'cache': rel, 'source': row['source'], 'new_sha256': row['sha256'],
                      'original_sha256': _hash(target) if target.is_file() else None})
    plan = {'format': PLAN_FORMAT, 'client': str(client), 'server': str(server),
            'stage': str(stage), 'client_exe': str(executable),
            'manifest_sha256': manifest_sha, 'identity': manifest['identity'],
            'full_deployment': bool(all_models), 'artifacts': plans}
    # Exclusive create: never quietly replace a previous authorisation.
    with plan_file.open('x', encoding='utf-8') as f:
        f.write(json.dumps(plan, ensure_ascii=False, indent=2, sort_keys=True) + '\n')
    return plan


def _load_plan(path: Path):
    no_symlink_ancestors(path)
    plan = json.loads(path.read_text(encoding='utf-8'))
    if plan.get('format') != PLAN_FORMAT or not isinstance(plan.get('artifacts'), list) or not plan['artifacts']:
        _fail('invalid deployment plan')
    client = _safe_dir(Path(plan['client']))
    server = Path(plan['server']).resolve(strict=True)
    stage = _safe_dir(Path(plan['stage']))
    executable = Path(plan['client_exe']).resolve(strict=True)
    if str(client) != plan['client'] or str(server) != plan['server'] or str(stage) != plan['stage']:
        _fail('plan directory changed')
    manifest, manifest_sha, rows = _validated(client, server, stage, executable)
    if plan.get('manifest_sha256') != manifest_sha or plan.get('identity') != manifest['identity']:
        _fail('staging identity/manifest changed since plan')
    if plan.get('full_deployment') not in (True, False):
        _fail('missing deployment scope')
    if plan['full_deployment'] and len(plan['artifacts']) != len(rows):
        _fail('full deployment selection does not cover the entire verified catalog')
    selected = {}
    for item in plan['artifacts']:
        if not isinstance(item, dict):
            _fail('invalid artifact row')
        rel = _safe_artifact(item['cache']).as_posix()
        if rel in selected or rel not in rows:
            _fail('duplicate/unverified selected cache')
        if item['new_sha256'] != rows[rel]['sha256'] or item['source'] != rows[rel]['source']:
            _fail('plan artifact differs from verified staging')
        old = item['original_sha256']
        if old is not None and (not isinstance(old, str) or len(old) != 64 or any(c not in '0123456789abcdef' for c in old)):
            _fail('invalid original SHA-256')
        selected[rel] = item
    return plan, client, server, stage, selected


def _current_sha(path: Path):
    return _hash(path) if path.is_file() else None


def _preflight_target(client: Path, items: dict[str, dict]):
    for rel, item in items.items():
        target = _entry_target(client, rel)
        if _current_sha(target) != item['original_sha256']:
            _fail('client cache changed since plan: ' + rel)


def _load_backup(backup: Path):
    backup = _safe_dir(backup)
    meta_file = backup / 'receipt.json'
    no_symlink_ancestors(meta_file)
    meta = json.loads(meta_file.read_text(encoding='utf-8'))
    if meta.get('format') != BACKUP_FORMAT or meta.get('state') not in ('READY', 'APPLYING', 'APPLIED', 'ROLLED_BACK', 'ROLLBACK_INCOMPLETE'):
        _fail('invalid backup receipt')
    original = backup / 'plan.json'
    no_symlink_ancestors(original)
    if _hash(original) != meta.get('plan_sha256'):
        _fail('backed-up plan changed')
    plan = json.loads(original.read_text(encoding='utf-8'))
    if plan.get('format') != PLAN_FORMAT:
        _fail('invalid backed-up plan')
    client = _safe_dir(Path(plan['client']))
    server_parent = Path(plan['server']).parent
    stage = Path(plan['stage'])
    if (backup == client or client in backup.parents or backup == server_parent or server_parent in backup.parents
            or backup == stage or stage in backup.parents
            or any(v.casefold() == 'canonical' for v in backup.parts)):
        _fail('backup is in a protected tree')
    selected = {}
    for row in plan['artifacts']:
        rel = _safe_artifact(row['cache']).as_posix()
        if rel in selected:
            _fail('duplicate cache in backup')
        selected[rel] = row
    return meta, plan, client, selected


def rollback(backup: Path) -> dict:
    meta, plan, client, items = _load_backup(backup)
    if meta['state'] == 'ROLLED_BACK':
        return {'restored': 0, 'already': True}
    backup = backup.resolve(strict=True)
    # Preflight all artifacts before any mutation: refuse external edits.
    for rel, item in items.items():
        target = _entry_target(client, rel)
        current = _current_sha(target)
        if current not in (item['original_sha256'], item['new_sha256']):
            _fail('cache externally edited; cannot auto-rollback: ' + rel)
        saved = backup / 'original' / Path(*_safe_artifact(rel).parts)
        if item['original_sha256'] is not None:
            no_symlink_ancestors(saved)
            if not saved.is_file() or _hash(saved) != item['original_sha256']:
                _fail('backup missing or corrupt: ' + rel)
    restored = 0
    try:
        for rel, item in reversed(list(items.items())):
            target = _entry_target(client, rel)
            if _current_sha(target) == item['original_sha256']:
                continue
            if item['original_sha256'] is None:
                target.unlink()
                # Do not remove user directories beyond ones which are empty.
            else:
                saved = backup / 'original' / Path(*_safe_artifact(rel).parts)
                _atomic_copy(saved, target, item['original_sha256'], saved, restore_metadata=True)
            restored += 1
        meta['state'] = 'ROLLED_BACK'
        _atomic_json(backup / 'receipt.json', meta)
    except BaseException:
        meta['state'] = 'ROLLBACK_INCOMPLETE'
        _atomic_json(backup / 'receipt.json', meta)
        raise
    return {'restored': restored, 'already': False}


def apply(plan_file: Path, backup: Path, full_ack: bool) -> dict:
    plan, client, server, stage, items = _load_plan(plan_file)
    if plan['full_deployment'] and not full_ack:
        _fail('full catalog requires explicit --confirm-full-deployment after a visual canary')
    _protected_path(backup, client, server, stage)
    if backup == plan_file or backup in plan_file.parents:
        _fail('backup overlaps plan')
    _preflight_target(client, items)
    backup.mkdir(mode=0o700)
    receipt = {'format': BACKUP_FORMAT, 'state': 'READY', 'plan_sha256': None}
    started = False
    try:
        shutil.copyfile(plan_file, backup / 'plan.json')
        receipt['plan_sha256'] = _hash(backup / 'plan.json')
        for rel, item in items.items():
            if item['original_sha256'] is not None:
                orig = _entry_target(client, rel)
                saved = backup / 'original' / Path(*_safe_artifact(rel).parts)
                saved.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(orig, saved)
                if _hash(saved) != item['original_sha256']:
                    _fail('original changed during backup: ' + rel)
        _preflight_target(client, items)
        _atomic_json(backup / 'receipt.json', receipt)
        receipt['state'] = 'APPLYING'
        _atomic_json(backup / 'receipt.json', receipt)
        started = True
        changed = 0
        for rel, item in items.items():
            target = _entry_target(client, rel)
            if _current_sha(target) != item['original_sha256']:
                _fail('cache changed during deployment: ' + rel)
            staged = stage / Path(*_safe_artifact(rel).parts)
            _atomic_copy(staged, target, item['new_sha256'], target if target.is_file() else None)
            changed += 1
        receipt['state'] = 'APPLIED'
        _atomic_json(backup / 'receipt.json', receipt)
        return {'deployed': changed, 'backup': str(backup)}
    except BaseException as exc:
        if started:
            try:
                rollback(backup)
            except BaseException as restore_exc:
                raise ValueError('apply failed; rollback also failed, preserve backup at %s: %s' % (backup, restore_exc)) from exc
        raise


def main() -> int:
    p = argparse.ArgumentParser(description=__doc__)
    sub = p.add_subparsers(dest='command', required=True)
    for name in ('compare', 'plan'):
        sp = sub.add_parser(name)
        sp.add_argument('--client', required=True, type=Path)
        sp.add_argument('--server', required=True, type=Path)
        sp.add_argument('--out', required=True, type=Path, help='Validated 263 staging')
        sp.add_argument('--client-exe', type=Path)
        if name == 'compare':
            sp.add_argument('--details', action='store_true')
            sp.add_argument('--archival-readonly', action='store_true',
                            help='Compare historically verified caches without authorizing deployment if executable hashes changed')
        else:
            sp.add_argument('--plan', type=Path, required=True)
            group = sp.add_mutually_exclusive_group(required=True)
            group.add_argument('--source', action='append', default=[], help='ASE source; may repeat')
            group.add_argument('--all', action='store_true', help='Full catalog; deploy only after visual canary')
    sp = sub.add_parser('apply')
    sp.add_argument('--plan', type=Path, required=True)
    sp.add_argument('--backup-dir', type=Path, required=True)
    sp.add_argument('--confirm-offline-client', action='store_true')
    sp.add_argument('--confirm-full-deployment', action='store_true')
    sp = sub.add_parser('rollback')
    sp.add_argument('--backup-dir', type=Path, required=True)
    sp.add_argument('--confirm-rollback', action='store_true')
    args = p.parse_args()
    try:
        if args.command == 'compare':
            totals, rows = compare(args.client, args.server, args.out, args.client_exe,
                                   args.details, args.archival_readonly)
            if args.archival_readonly:
                archive = verify_archived_readonly(args.client, args.server, args.out, args.client_exe)
                print('NATIVE_DEPLOY_280=ARCHIVAL_COMPARE total=%d same=%d different=%d missing=%d ambiguous=%d case_variants=%d identity_changed=%s deployment_allowed=0 client_writes=0' %
                      (len(rows), totals['SAME'], totals['DIFFERENT'], totals['MISSING'],
                       sum(r['state'] == 'AMBIGUOUS' for r in rows),
                       sum(bool(r['case_variant']) for r in rows),
                       ','.join(archive['identity_changed']) or 'none'))
            else:
                print('NATIVE_DEPLOY_278=COMPARE total=%d same=%d different=%d missing=%d client_writes=0' %
                      (len(rows), totals['SAME'], totals['DIFFERENT'], totals['MISSING']))
            if args.details:
                for row in rows:
                    if args.archival_readonly and row['state'] == 'AMBIGUOUS':
                        print('%-9s %s candidates=%s' %
                              (row['state'], row['cache'], ','.join(row['ambiguous_candidates'])))
                    elif args.archival_readonly and row['case_variant']:
                        print('%-9s %s case_variant=%s' %
                              (row['state'], row['cache'], row['existing_path'] or 'parent-only'))
                    else:
                        print('%-9s %s' % (row['state'], row['cache']))
        elif args.command == 'plan':
            plan = make_plan(args.client, args.server, args.out, args.client_exe,
                             args.plan, args.source, args.all)
            print('NATIVE_DEPLOY_278=PLAN selected=%d full=%s client_writes=0 file=%s' %
                  (len(plan['artifacts']), plan['full_deployment'], args.plan))
        elif args.command == 'apply':
            if not args.confirm_offline_client:
                _fail('stop Game.exe and provide --confirm-offline-client')
            result = apply(args.plan, args.backup_dir, args.confirm_full_deployment)
            print('NATIVE_DEPLOY_278=APPLIED count=%d backup=%s' % (result['deployed'], result['backup']))
        else:
            if not args.confirm_rollback:
                _fail('explicit --confirm-rollback required')
            result = rollback(args.backup_dir)
            print('NATIVE_DEPLOY_278=ROLLED_BACK restored=%d already=%s' %
                  (result['restored'], result['already']))
        return 0
    except (OSError, ValueError, TypeError, KeyError, json.JSONDecodeError) as exc:
        print('NATIVE_DEPLOY_278=FAIL_CLOSED ' + str(exc), file=sys.stderr)
        return 2


if __name__ == '__main__':
    raise SystemExit(main())
