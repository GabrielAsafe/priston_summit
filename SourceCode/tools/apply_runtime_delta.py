#!/usr/bin/env python3
"""Apply a Summit runtime delta ZIP to a standalone installation.

Run from the install root containing cliente/ and Servidor/ or provide --repo-root.
The script validates hashes of added/modified files after extraction and honors
removed paths from RUNTIME_DELTA_MANIFEST.json.
"""
from __future__ import annotations
import argparse, hashlib, json, os
from pathlib import Path
import tempfile, zipfile

SCHEMA = "summit.runtime-delta/v1"

def sha256_file(p: Path) -> str:
    h = hashlib.sha256()
    with p.open('rb') as f:
        for b in iter(lambda: f.read(1024*1024), b''):
            h.update(b)
    return h.hexdigest()

def safe_target(root: Path, rel: str) -> Path:
    p = (root / rel).resolve()
    try:
        p.relative_to(root.resolve())
    except ValueError:
        raise SystemExit(f"unsafe path in delta: {rel}")
    return p

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--repo-root', default='.')
    ap.add_argument('--delta', required=True)
    args = ap.parse_args()
    root = Path(args.repo_root).resolve()
    dz = Path(args.delta).resolve()
    with zipfile.ZipFile(dz, 'r') as zf:
        try:
            man = json.loads(zf.read('RUNTIME_DELTA_MANIFEST.json').decode('utf-8'))
        except KeyError:
            raise SystemExit('delta manifest missing')
        if man.get('schema') != SCHEMA:
            raise SystemExit(f"unsupported delta schema: {man.get('schema')!r}")
        changed = dict(man.get('added', {}))
        changed.update(man.get('modified', {}))
        # Extract changed files explicitly; never extract arbitrary zip entries.
        for rel, info in changed.items():
            target = safe_target(root, rel)
            target.parent.mkdir(parents=True, exist_ok=True)
            data = zf.read(rel)
            tmp = target.with_name(target.name + '.summit-delta.tmp')
            tmp.write_bytes(data)
            if hashlib.sha256(data).hexdigest() != info['sha256']:
                tmp.unlink(missing_ok=True)
                raise SystemExit(f"hash mismatch in archive: {rel}")
            tmp.replace(target)
        for rel in man.get('removed', []):
            p = safe_target(root, rel)
            if p.is_file() or p.is_symlink():
                p.unlink()
        for rel, info in changed.items():
            p = safe_target(root, rel)
            if not p.is_file():
                raise SystemExit(f"applied file missing: {rel}")
            if sha256_file(p) != info['sha256']:
                raise SystemExit(f"post-apply hash mismatch: {rel}")
    print(f"DELTA={dz}")
    print(f"APPLIED={len(changed)}")
    print(f"REMOVED={len(man.get('removed', []))}")
    print('RESULT=PASS')

if __name__ == '__main__':
    main()
