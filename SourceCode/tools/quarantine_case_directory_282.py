#!/usr/bin/env python3
"""SUMMIT 283: strictly gated, reversible quarantine of one known duplicate case directory.

Never touches tmABCD, models staging, git index or canonical. The optional operation
relocates ONLY cliente/char/tmabcd into a user-selected backup under bugfixes.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
import stat
import sys
from pathlib import Path

EXPECTED_SMD = frozenset('''
mfbb29.smd mfbb30.smd mfbb31.smd
mfbc23.smd mfbc24.smd mfbc25.smd
mmba29.smd mmba30.smd mmba31.smd
mmbd22.smd mmbd23.smd mmbd24.smd
tfbd26.smd tfbd27.smd tfbd28.smd
tmba25.smd tmba26.smd tmba27.smd
tmbb25.smd tmbb26.smd tmbb27.smd
tmbc25.smd tmbc26.smd tmbc27.smd
'''.split())
EXPECTED_INX = frozenset('''
a032.inx a033.inx a034.inx
b032.inx b033.inx b034.inx
c032.inx c033.inx c034.inx
d032.inx d033.inx d034.inx
ma032.inx ma033.inx ma034.inx
mb032.inx mb033.inx mb034.inx
mc032.inx mc033.inx mc034.inx
md032.inx md033.inx md034.inx
'''.split())
EXPECTED = EXPECTED_SMD | EXPECTED_INX
MANIFEST = 'SUMMIT_QUARANTINE_282.json'

class Refusal(ValueError):
    pass


def real_file_sha(path: Path) -> str:
    """No-follow open and regular-file check, protect against symlink swaps."""
    fd = os.open(path, os.O_RDONLY | getattr(os, 'O_NOFOLLOW', 0) | getattr(os, 'O_NONBLOCK', 0))
    try:
        st = os.fstat(fd)
        if not stat.S_ISREG(st.st_mode):
            raise Refusal(f'not a regular file: {path}')
        h = hashlib.sha256()
        with os.fdopen(fd, 'rb', closefd=False) as f:
            for chunk in iter(lambda: f.read(1024 * 1024), b''):
                h.update(chunk)
        return h.hexdigest()
    finally:
        os.close(fd)


def no_symlink_chain(path: Path, allow_absent_leaf=False):
    for p in reversed((path, *path.parents)):
        if p.is_symlink():
            raise Refusal(f'symlink in path: {p}')
        if p.exists() and p != path and not p.is_dir():
            raise Refusal(f'parent is not a directory: {p}')
    if not allow_absent_leaf and not path.exists():
        raise Refusal(f'missing path: {path}')


def paths(repo: Path, *, expect_lower=True):
    repo = repo.absolute()
    no_symlink_chain(repo)
    if not repo.is_dir() or not (repo / 'SourceCode').is_dir():
        raise Refusal('not a SUMMIT repo (SourceCode missing)')
    root = repo / 'cliente' / 'char'
    upper = root / 'tmABCD'
    lower = root / 'tmabcd'
    for p in (root, upper):
        no_symlink_chain(p)
        if not p.is_dir():
            raise Refusal(f'expected directory: {p}')
    case_variants = sorted(p.name for p in root.iterdir() if p.name.casefold() == 'tmabcd')
    expected = ['tmABCD', 'tmabcd'] if expect_lower else ['tmABCD']
    if case_variants != expected:
        raise Refusal(f'unexpected case-variant set: {case_variants}')
    return repo, upper, lower


def inventory(directory: Path, required: frozenset[str] = EXPECTED) -> dict[str, str]:
    """Only the named regular files; reject hidden extras, subdirs and links."""
    no_symlink_chain(directory)
    if not directory.is_dir():
        raise Refusal(f'not a directory: {directory}')
    entries = list(directory.iterdir())
    actual = {p.name for p in entries}
    if actual != required or len(entries) != len(required):
        raise Refusal(f'wrong lowercase contents: count={len(entries)} '
                      f'extra={sorted(actual - required)} missing={sorted(required - actual)}')
    return {name: real_file_sha(directory / name) for name in sorted(required)}


def inspect(repo: Path) -> tuple[Path, Path, Path, dict[str, str]]:
    repo, upper, lower = paths(repo)
    no_symlink_chain(lower)
    if not lower.is_dir():
        raise Refusal(f'lowercase directory missing: {lower}')
    low = inventory(lower)
    # Case-insensitive leaf ambiguities are not permitted within the upper directory.
    upper_names = [p.name for p in upper.iterdir()]
    issues = []
    for name in sorted(EXPECTED):
        matches = [candidate for candidate in upper_names if candidate.casefold() == name.casefold()]
        if not matches:
            issues.append(f'missing_in_upper:{name}')
            continue
        if len(matches) > 1:
            issues.append(f'ambiguous_in_upper:{name}')
            continue
        # Must be exact physical target, not an upper/lower case alias.
        if matches[0] != name:
            issues.append(f'upper_spelling_differs:{name}->{matches[0]}')
            continue
        sha = real_file_sha(upper / name)
        if sha != low[name]:
            issues.append(f'hash_differs:{name}')
    if issues:
        raise Refusal('not a fully redundant directory: ' + ', '.join(issues))
    return repo, upper, lower, low


def backup_location(repo: Path, arg: str):
    if not arg:
        raise Refusal('--backup-dir is required')
    b = Path(arg)
    if '..' in b.parts or '.' in b.parts:
        raise Refusal('backup directory traversal refused')
    b = (b if b.is_absolute() else repo / b).absolute()
    no_symlink_chain(b, allow_absent_leaf=True)
    parent = repo / 'bugfixes'
    no_symlink_chain(parent)
    if not parent.is_dir():
        raise Refusal('bugfixes directory missing')
    if not b.is_relative_to(parent) or b == parent:
        raise Refusal('backup must be under repo/bugfixes and outside cliente')
    if not b.parent.is_dir():
        raise Refusal('backup parent must already exist')
    return b


def write_manifest_atomic(dest: Path, data: dict):
    raw = (json.dumps(data, sort_keys=True, indent=2) + '\n').encode('utf-8')
    tmp = dest.with_name(dest.name + '.pending')
    if tmp.exists() or tmp.is_symlink():
        raise Refusal('pending manifest already exists')
    fd = os.open(tmp, os.O_CREAT | os.O_EXCL | os.O_WRONLY, 0o600)
    try:
        with os.fdopen(fd, 'wb') as f:
            f.write(raw)
            f.flush()
            os.fsync(f.fileno())
    except Exception:
        try: tmp.unlink()
        except OSError: pass
        raise
    os.replace(tmp, dest)


def apply(repo: Path, backup_dir: str):
    repo, upper, lower, hashes = inspect(repo)
    backup = backup_location(repo, backup_dir)
    if backup.exists() or backup.is_symlink():
        raise Refusal(f'backup already exists: {backup}')
    if os.stat(lower).st_dev != os.stat(backup.parent).st_dev:
        raise Refusal('non-atomic cross-device move refused')
    print(f'CASE282_PREFLIGHT=PASS duplicates={len(hashes)} matching_sha256={len(hashes)} extras=0')
    manifest = {
        'format': 'SUMMIT_CASE_QUARANTINE_283_V2',
        'client_rel': 'cliente/char/tmabcd',
        'preserved_rel': 'cliente/char/tmABCD',
        'files': hashes,
        'status': 'quarantined',
    }
    # Do not overwrite existing backups or ever touch upper.
    backup.mkdir(mode=0o700)
    moved = backup / 'tmabcd'
    try:
        # Persist recovery manifest BEFORE the rename. If interrupted after the move,
        # explicit rollback can still verify and restore the quarantined directory.
        write_manifest_atomic(backup / MANIFEST, manifest)
        # Preflight again before moving: file content could have changed while building manifest.
        _, _, _, again = inspect(repo)
        if again != hashes:
            raise Refusal('source changed since preflight')
        if moved.exists() or moved.is_symlink():
            raise Refusal('quarantine slot unexpectedly occupied')
        os.rename(lower, moved)
        try:
            if inventory(moved) != hashes:
                raise Refusal('quarantine content changed')
        except BaseException:
            # Roll back immediately if we have not successfully committed manifest.
            if moved.is_dir() and not lower.exists() and not lower.is_symlink():
                os.rename(moved, lower)
            raise
    except Exception:
        # A leftover empty backup directory is harmless and outside client.
        raise
    print(f'CASE282_APPLY=PASS moved={len(hashes)} client_upper_untouched=1 git_index_untouched=1')
    print('BACKUP=' + str(backup))


def rollback(repo: Path, backup_dir: str):
    repo, upper, lower = paths(repo, expect_lower=False)
    backup = backup_location(repo, backup_dir)
    no_symlink_chain(backup)
    m = backup / MANIFEST
    no_symlink_chain(m)
    raw = json.loads(m.read_text(encoding='utf-8'))
    if not isinstance(raw, dict) or raw.get('status') != 'quarantined' or raw.get('client_rel') != 'cliente/char/tmabcd' or raw.get('preserved_rel') != 'cliente/char/tmABCD':
        raise Refusal('invalid manifest metadata')
    # Backward-compatible restoration of a backup genuinely created by PATCH 282.
    # New quarantines require all 48 files and a distinct version marker.
    archive_version = raw.get('format')
    if archive_version == 'SUMMIT_CASE_QUARANTINE_282_V1':
        required = EXPECTED_SMD
    elif archive_version == 'SUMMIT_CASE_QUARANTINE_283_V2':
        required = EXPECTED
    else:
        raise Refusal('invalid manifest format')
    saved_hashes = raw.get('files')
    if not isinstance(saved_hashes, dict) or set(saved_hashes) != required or not all(isinstance(v, str) and len(v) == 64 and all(c in '0123456789abcdef' for c in v) for v in saved_hashes.values()):
        raise Refusal('invalid manifest file hashes')
    if lower.exists() or lower.is_symlink():
        raise Refusal('lowercase destination already occupied')
    moved = backup / 'tmabcd'
    if inventory(moved, required) != saved_hashes:
        raise Refusal('quarantined files modified')
    # Before restoring, check upper models still present; unrelated new client files okay.
    for name, h in saved_hashes.items():
        if real_file_sha(upper / name) != h:
            raise Refusal(f'preserved model changed: {name}')
    if os.stat(moved).st_dev != os.stat(lower.parent).st_dev:
        raise Refusal('cross-device rollback refused')
    os.rename(moved, lower)
    print(f'CASE282_ROLLBACK=PASS restored={len(saved_hashes)} git_index_untouched=1')


def main(argv=None):
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('operation', choices=('inspect', 'apply', 'rollback'))
    p.add_argument('--repo', default='.')
    p.add_argument('--backup-dir')
    p.add_argument('--confirm-game-closed', action='store_true')
    p.add_argument('--confirm-rollback', action='store_true')
    a = p.parse_args(argv)
    try:
        repo = Path(a.repo)
        if a.operation == 'inspect':
            _, _, _, hashes = inspect(repo)
            print(f'CASE282_INSPECT=PASS pairs={len(hashes)} identical={len(hashes)} extra=0 client_writes=0')
        elif a.operation == 'apply':
            if not a.confirm_game_closed:
                raise Refusal('apply requires --confirm-game-closed')
            apply(repo, a.backup_dir)
        else:
            if not a.confirm_rollback:
                raise Refusal('rollback requires --confirm-rollback')
            rollback(repo, a.backup_dir)
        return 0
    except (OSError, ValueError, TypeError, json.JSONDecodeError) as exc:
        print(f'CASE282=FAIL_CLOSED {exc}', file=sys.stderr)
        return 2

if __name__ == '__main__':
    raise SystemExit(main())
