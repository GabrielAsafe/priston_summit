#!/usr/bin/env python3
"""Create reproducible full and incremental runtime packages for Summit.

The checkpoint stores SHA-256 for the deployed cliente/ and Servidor/ trees.
Delta archives contain only added/modified files plus a deletion manifest.
No third-party Python packages are required.
"""
from __future__ import annotations

import argparse
import fnmatch
import hashlib
import json
import os
from pathlib import Path
import shutil
import sys
import tempfile
from datetime import datetime, timezone
import zipfile

SCHEMA = "summit.runtime-checkpoint/v1"
DELTA_SCHEMA = "summit.runtime-delta/v1"

DEFAULT_EXCLUDES = [
    "cliente/LOGS/**",
    "Servidor/LOGS/**",
    "**/*.log",
    "**/*.dmp",
    "**/*.tmp",
    "cliente/LoadItemImage_debug.txt",
    # Legacy duplicate left by the old map-pack path. The active pack is
    # cliente/Content/packs/maps_client.smap.
    "cliente/content/maps/maps_client.smap",
]

RUNTIME_ROOTS = ("cliente", "Servidor")


def utc_now() -> str:
    return datetime.now(timezone.utc).replace(microsecond=0).isoformat().replace("+00:00", "Z")


def sha256_file(path: Path, chunk: int = 1024 * 1024) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        while True:
            b = f.read(chunk)
            if not b:
                break
            h.update(b)
    return h.hexdigest()


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def norm_rel(path: Path) -> str:
    return path.as_posix()


def excluded(rel: str, patterns: list[str]) -> bool:
    rel = rel.replace("\\", "/")
    for pat in patterns:
        if fnmatch.fnmatchcase(rel, pat):
            return True
        # pathlib-style directory wildcard convenience
        if pat.endswith("/**") and (rel == pat[:-3] or rel.startswith(pat[:-2])):
            return True
    return False


def scan(repo: Path, patterns: list[str]) -> dict[str, dict]:
    out: dict[str, dict] = {}
    for root_name in RUNTIME_ROOTS:
        root = repo / root_name
        if not root.is_dir():
            raise SystemExit(f"runtime root missing: {root}")
        for p in sorted(root.rglob("*")):
            if not p.is_file() and not p.is_symlink():
                continue
            rel = norm_rel(p.relative_to(repo))
            if excluded(rel, patterns):
                continue
            if p.is_symlink():
                target = os.readlink(p)
                data = target.encode("utf-8", "surrogateescape")
                out[rel] = {
                    "type": "symlink",
                    "target": target,
                    "size": len(data),
                    "sha256": sha256_bytes(data),
                }
            else:
                st = p.stat()
                out[rel] = {
                    "type": "file",
                    "size": st.st_size,
                    "sha256": sha256_file(p),
                }
    return out


def canonical_json(obj) -> bytes:
    return (json.dumps(obj, ensure_ascii=False, sort_keys=True, indent=2) + "\n").encode("utf-8")


def checkpoint_object(repo: Path, files: dict[str, dict], patterns: list[str]) -> dict:
    body = {
        "schema": SCHEMA,
        "created_at": utc_now(),
        "repo_layout": {"client": "cliente", "server": "Servidor"},
        "excludes": patterns,
        "files": files,
    }
    body["snapshot_sha256"] = sha256_bytes(canonical_json(body["files"]))
    body["file_count"] = len(files)
    body["total_bytes"] = sum(v.get("size", 0) for v in files.values())
    return body


def load_checkpoint(path: Path) -> dict:
    try:
        obj = json.loads(path.read_text(encoding="utf-8"))
    except FileNotFoundError:
        raise SystemExit(f"checkpoint not found: {path}")
    if obj.get("schema") != SCHEMA:
        raise SystemExit(f"unsupported checkpoint schema: {obj.get('schema')!r}")
    return obj


def write_checkpoint(path: Path, obj: dict) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    tmp = path.with_suffix(path.suffix + ".tmp")
    tmp.write_bytes(canonical_json(obj))
    tmp.replace(path)


def compare(base: dict[str, dict], cur: dict[str, dict]):
    added = sorted(k for k in cur.keys() - base.keys())
    removed = sorted(k for k in base.keys() - cur.keys())
    modified = sorted(
        k for k in cur.keys() & base.keys()
        if cur[k].get("sha256") != base[k].get("sha256") or cur[k].get("type") != base[k].get("type")
    )
    return added, modified, removed


def zip_write_path(zf: zipfile.ZipFile, repo: Path, rel: str, info: dict):
    p = repo / rel
    if info.get("type") == "symlink":
        zi = zipfile.ZipInfo(rel)
        zi.create_system = 3
        zi.external_attr = 0o120777 << 16
        zf.writestr(zi, info["target"])
    else:
        zf.write(p, rel)


def create_full(repo: Path, output: Path, files: dict[str, dict], patterns: list[str]):
    manifest = {
        "schema": "summit.runtime-full/v1",
        "created_at": utc_now(),
        "snapshot_sha256": sha256_bytes(canonical_json(files)),
        "file_count": len(files),
        "total_bytes": sum(v.get("size", 0) for v in files.values()),
        "excludes": patterns,
        "files": files,
    }
    output.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(output, "w", compression=zipfile.ZIP_DEFLATED, compresslevel=9) as zf:
        for rel, info in files.items():
            zip_write_path(zf, repo, rel, info)
        zf.writestr("RUNTIME_PACKAGE_MANIFEST.json", canonical_json(manifest))
    return manifest


def create_delta(repo: Path, output: Path, cp: dict, cur: dict[str, dict], patterns: list[str]):
    base = cp["files"]
    added, modified, removed = compare(base, cur)
    changed = added + modified
    manifest = {
        "schema": DELTA_SCHEMA,
        "created_at": utc_now(),
        "base_snapshot_sha256": cp.get("snapshot_sha256"),
        "target_snapshot_sha256": sha256_bytes(canonical_json(cur)),
        "excludes": patterns,
        "added": {k: cur[k] for k in added},
        "modified": {k: cur[k] for k in modified},
        "removed": removed,
        "changed_file_count": len(changed),
        "removed_file_count": len(removed),
        "changed_bytes": sum(cur[k].get("size", 0) for k in changed),
    }
    output.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(output, "w", compression=zipfile.ZIP_DEFLATED, compresslevel=9) as zf:
        for rel in changed:
            zip_write_path(zf, repo, rel, cur[rel])
        zf.writestr("RUNTIME_DELTA_MANIFEST.json", canonical_json(manifest))
    return manifest


def cmd_checkpoint(args):
    repo = Path(args.repo_root).resolve()
    patterns = DEFAULT_EXCLUDES + args.exclude
    files = scan(repo, patterns)
    obj = checkpoint_object(repo, files, patterns)
    out = Path(args.checkpoint).resolve()
    write_checkpoint(out, obj)
    print(f"CHECKPOINT={out}")
    print(f"FILES={obj['file_count']}")
    print(f"BYTES={obj['total_bytes']}")
    print(f"SNAPSHOT_SHA256={obj['snapshot_sha256']}")
    print("RESULT=PASS")


def cmd_status(args):
    repo = Path(args.repo_root).resolve()
    cp = load_checkpoint(Path(args.checkpoint).resolve())
    patterns = cp.get("excludes", DEFAULT_EXCLUDES) + args.exclude
    cur = scan(repo, patterns)
    added, modified, removed = compare(cp["files"], cur)
    for k in added:
        print(f"ADDED {k}")
    for k in modified:
        print(f"MODIFIED {k}")
    for k in removed:
        print(f"REMOVED {k}")
    print(f"ADDED_COUNT={len(added)}")
    print(f"MODIFIED_COUNT={len(modified)}")
    print(f"REMOVED_COUNT={len(removed)}")
    print(f"CHANGED_BYTES={sum(cur[k]['size'] for k in added + modified)}")
    print("RESULT=PASS")


def cmd_full(args):
    repo = Path(args.repo_root).resolve()
    patterns = DEFAULT_EXCLUDES + args.exclude
    files = scan(repo, patterns)
    output = Path(args.output).resolve()
    man = create_full(repo, output, files, patterns)
    print(f"PACKAGE={output}")
    print(f"FILES={man['file_count']}")
    print(f"BYTES={man['total_bytes']}")
    print(f"SNAPSHOT_SHA256={man['snapshot_sha256']}")
    print(f"PACKAGE_SHA256={sha256_file(output)}")
    if args.checkpoint:
        cp = checkpoint_object(repo, files, patterns)
        write_checkpoint(Path(args.checkpoint).resolve(), cp)
        print(f"CHECKPOINT={Path(args.checkpoint).resolve()}")
    print("RESULT=PASS")


def cmd_delta(args):
    repo = Path(args.repo_root).resolve()
    cp_path = Path(args.checkpoint).resolve()
    cp = load_checkpoint(cp_path)
    patterns = cp.get("excludes", DEFAULT_EXCLUDES) + args.exclude
    cur = scan(repo, patterns)
    output = Path(args.output).resolve()
    man = create_delta(repo, output, cp, cur, patterns)
    print(f"PACKAGE={output}")
    print(f"ADDED_COUNT={len(man['added'])}")
    print(f"MODIFIED_COUNT={len(man['modified'])}")
    print(f"REMOVED_COUNT={len(man['removed'])}")
    print(f"CHANGED_BYTES={man['changed_bytes']}")
    print(f"BASE_SNAPSHOT_SHA256={man['base_snapshot_sha256']}")
    print(f"TARGET_SNAPSHOT_SHA256={man['target_snapshot_sha256']}")
    print(f"PACKAGE_SHA256={sha256_file(output)}")
    if args.advance_checkpoint:
        obj = checkpoint_object(repo, cur, patterns)
        write_checkpoint(cp_path, obj)
        print(f"CHECKPOINT_ADVANCED={cp_path}")
    print("RESULT=PASS")


def build_parser():
    p = argparse.ArgumentParser(description="Summit runtime full/delta packager")
    sub = p.add_subparsers(dest="cmd", required=True)

    def common(sp):
        sp.add_argument("--repo-root", default=".")
        sp.add_argument("--exclude", action="append", default=[], help="additional repo-relative glob to exclude")

    s = sub.add_parser("checkpoint", help="record current runtime tree as baseline")
    common(s)
    s.add_argument("--checkpoint", required=True)
    s.set_defaults(func=cmd_checkpoint)

    s = sub.add_parser("status", help="show changes since a checkpoint")
    common(s)
    s.add_argument("--checkpoint", required=True)
    s.set_defaults(func=cmd_status)

    s = sub.add_parser("full", help="create a full runtime package")
    common(s)
    s.add_argument("--output", required=True)
    s.add_argument("--checkpoint", help="also write/replace checkpoint with this package snapshot")
    s.set_defaults(func=cmd_full)

    s = sub.add_parser("delta", help="create a package containing only changes since checkpoint")
    common(s)
    s.add_argument("--checkpoint", required=True)
    s.add_argument("--output", required=True)
    s.add_argument("--advance-checkpoint", action="store_true", help="advance baseline only after package creation succeeds")
    s.set_defaults(func=cmd_delta)
    return p


def main():
    args = build_parser().parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
