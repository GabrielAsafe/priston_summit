#!/usr/bin/env python3
from __future__ import annotations
import argparse, datetime as dt, hashlib, json, os, pathlib, re, shutil, tempfile


def sha256_file(path):
    h = hashlib.sha256()
    with pathlib.Path(path).open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def normalize_rel(value):
    s = str(value).replace("\\", "/")
    s = re.sub(r"/+", "/", s).strip("/")
    p = pathlib.PurePosixPath(s)
    if not s or p.is_absolute() or ".." in p.parts:
        raise ValueError(f"unsafe relative path: {value!r}")
    return p.as_posix()


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


def safe_client_path(root, rel):
    p = (root / pathlib.PurePosixPath(normalize_rel(rel))).resolve()
    p.relative_to(root.resolve())
    return p


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--profile", required=True)
    p.add_argument("--client-root", required=True)
    args = p.parse_args()

    profile_path = pathlib.Path(args.profile).expanduser().resolve()
    client_root = pathlib.Path(args.client_root).expanduser().resolve()
    if not profile_path.is_file():
        raise FileNotFoundError(profile_path)
    if not client_root.is_dir():
        raise FileNotFoundError(client_root)

    data = json.loads(profile_path.read_text(encoding="utf-8"))
    if data.get("profile_id") != "pikeman_c01_m4":
        raise ValueError(f"unexpected profile_id={data.get('profile_id')!r}")

    original_sha = sha256_file(profile_path)
    stamp = dt.datetime.now().strftime("%Y%m%d_%H%M%S_%f")
    backup = profile_path.with_name(profile_path.stem + f".backup_{stamp}.json")
    shutil.copy2(profile_path, backup)
    if sha256_file(backup) != original_sha:
        raise RuntimeError("profile backup verification failed")

    changes = []
    if data.get("roundtrip_target"):
        old = data["roundtrip_target"]
        new = normalize_rel(old)
        if old != new:
            changes.append(("roundtrip_target", old, new))
            data["roundtrip_target"] = new

    verified = []
    for comp in data.get("components", []):
        old = comp.get("roundtrip_target")
        if not old:
            continue
        new = normalize_rel(old)
        if old != new:
            changes.append((f"component:{comp.get('role')}", old, new))
            comp["roundtrip_target"] = new

        cp = safe_client_path(client_root, new)
        if comp.get("required", False) and not cp.is_file():
            raise FileNotFoundError(f"required component missing: {new} -> {cp}")
        if cp.is_file():
            comp["verified_client_size"] = cp.stat().st_size
            comp["verified_client_sha256"] = sha256_file(cp)
            verified.append((comp.get("role"), new, cp.stat().st_size, comp["verified_client_sha256"]))

    # Verify the three primary baselines remain exactly the bootstrap values.
    primary = [
        ("model", data.get("model_smd"), data.get("model_smd_sha256")),
        ("skeleton", data.get("skeleton_smb"), data.get("skeleton_smb_sha256")),
        ("model_info", data.get("model_info_inx"), data.get("model_info_inx_sha256")),
    ]
    root = profile_path.parents[1]
    for role, local_name, expected_sha in primary:
        if not local_name or not expected_sha:
            raise ValueError(f"missing primary baseline metadata for {role}")
        local = (root / local_name).resolve()
        if not local.is_file():
            raise FileNotFoundError(local)
        if sha256_file(local) != expected_sha:
            raise RuntimeError(f"{role} local baseline SHA mismatch")

    data.setdefault("bootstrap_evidence", {})["profile_path_normalization_check"] = "PASS"
    data["bootstrap_evidence"]["profile_path_fix_timestamp"] = dt.datetime.now(dt.timezone.utc).isoformat()
    atomic_json(profile_path, data)

    new_sha = sha256_file(profile_path)
    print("PT_PIKEMAN_PROFILE_FIX_V1")
    print(f"profile={profile_path}")
    print(f"backup={backup}")
    print(f"old_profile_sha256={original_sha}")
    print(f"new_profile_sha256={new_sha}")
    print(f"change_count={len(changes)}")
    for key, old, new in changes:
        print(f"FIX {key}: {old!r} -> {new!r}")
    for role, target, size, sha in verified:
        print(f"VERIFY role={role} target={target} size={size} sha256={sha}")
    print("client_modified=False")
    print("profile_path_fix_check=PASS")


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        print(f"profile_path_fix_check=FAIL: {type(exc).__name__}: {exc}")
        raise
