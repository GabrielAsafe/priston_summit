#!/usr/bin/env python3
from __future__ import annotations
import argparse, datetime as dt, hashlib, json, os, pathlib, tempfile

def atomic_json(path, data):
    path = pathlib.Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    fd, tmpname = tempfile.mkstemp(
        prefix=path.name + ".", suffix=".tmp", dir=path.parent
    )
    os.close(fd)
    tmp = pathlib.Path(tmpname)
    try:
        tmp.write_text(
            json.dumps(data, ensure_ascii=False, indent=2) + "\n",
            encoding="utf-8",
        )
        os.replace(tmp, path)
    finally:
        tmp.unlink(missing_ok=True)

def sha256_file(path):
    h = hashlib.sha256()
    with pathlib.Path(path).open("rb") as f:
        for b in iter(lambda: f.read(1024 * 1024), b""):
            h.update(b)
    return h.hexdigest()

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--workspace", required=True)
    ap.add_argument("--client-root", required=True)
    ap.add_argument("--profile", required=True)
    a = ap.parse_args()

    workspace = pathlib.Path(a.workspace).expanduser().resolve()
    client = pathlib.Path(a.client_root).expanduser().resolve()
    pp = pathlib.Path(a.profile).expanduser().resolve()

    if not client.is_dir():
        raise FileNotFoundError(client)
    if not pp.is_file():
        raise FileNotFoundError(pp)

    profile = json.loads(pp.read_text(encoding="utf-8"))
    required = ("profile_id", "roundtrip_target", "components")
    missing = [x for x in required if x not in profile]
    if missing:
        raise ValueError(f"profile missing required fields: {missing}")
    if not isinstance(profile["components"], list) or not profile["components"]:
        raise ValueError("profile components must be a non-empty list")

    if workspace.exists() and any(workspace.iterdir()):
        raise FileExistsError(
            f"workspace already exists and is not empty: {workspace}"
        )
    workspace.mkdir(parents=True, exist_ok=True)

    manifest = workspace / "workspace_manifest.json"
    data = {
        "workspace_version": 2,
        "workspace_kind": "character_asset",
        "game_root": str(client),
        "files": [],
        "bridge_history": [],
        "character_asset": {
            "profile_id": profile["profile_id"],
            "asset_set_id": profile.get("asset_set_id"),
            "display_name": profile.get("display_name"),
            "profile": str(pp),
            "profile_sha256_at_init": sha256_file(pp),
        },
        "created_at": dt.datetime.now(dt.timezone.utc).isoformat(),
        "client_modified": False,
    }
    atomic_json(manifest, data)

    print("PT_CHARACTER_WORKSPACE_INIT_V2")
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
