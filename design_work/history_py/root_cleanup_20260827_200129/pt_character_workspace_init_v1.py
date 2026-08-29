#!/usr/bin/env python3
from __future__ import annotations
import argparse, datetime as dt, hashlib, json, os, pathlib, tempfile


def atomic_json(path, data):
    path = pathlib.Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    fd, tmpname = tempfile.mkstemp(prefix=path.name + ".", suffix=".tmp", dir=path.parent)
    os.close(fd)
    tmp = pathlib.Path(tmpname)
    try:
        tmp.write_text(json.dumps(data, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
        os.replace(tmp, path)
    finally:
        tmp.unlink(missing_ok=True)


def sha256_file(path):
    h = hashlib.sha256()
    with pathlib.Path(path).open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--workspace", required=True)
    p.add_argument("--client-root", required=True)
    p.add_argument("--profile", required=True)
    args = p.parse_args()

    workspace = pathlib.Path(args.workspace).expanduser().resolve()
    client_root = pathlib.Path(args.client_root).expanduser().resolve()
    profile_path = pathlib.Path(args.profile).expanduser().resolve()

    if not client_root.is_dir():
        raise FileNotFoundError(client_root)
    if not profile_path.is_file():
        raise FileNotFoundError(profile_path)

    profile = json.loads(profile_path.read_text(encoding="utf-8"))
    if profile.get("profile_id") != "pikeman_c01_m4":
        raise ValueError("workspace initializer expects the real Pikeman profile")

    manifest = workspace / "workspace_manifest.json"
    if workspace.exists():
        existing = list(workspace.iterdir())
        if existing:
            raise FileExistsError(
                f"workspace already exists and is not empty; refusing to merge: {workspace}"
            )
    workspace.mkdir(parents=True, exist_ok=True)

    data = {
        "workspace_version": 1,
        "workspace_kind": "character_asset",
        "game_root": str(client_root),
        "files": [],
        "bridge_history": [],
        "character_asset": {
            "profile_id": profile["profile_id"],
            "asset_set_id": profile.get("asset_set_id"),
            "display_name": profile.get("display_name"),
            "profile": str(profile_path),
            "profile_sha256_at_init": sha256_file(profile_path),
        },
        "created_at": dt.datetime.now(dt.timezone.utc).isoformat(),
        "client_modified": False,
    }
    atomic_json(manifest, data)

    print("PT_CHARACTER_WORKSPACE_INIT_V1")
    print(f"workspace={workspace}")
    print(f"manifest={manifest}")
    print(f"profile_id={profile['profile_id']}")
    print(f"asset_set_id={profile.get('asset_set_id')}")
    print("registered_files=0")
    print("client_modified=False")
    print("workspace_init_check=PASS")


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        print(f"workspace_init_check=FAIL: {type(exc).__name__}: {exc}")
        raise
