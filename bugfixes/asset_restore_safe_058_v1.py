#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv
import hashlib
import shutil
from pathlib import Path

def sha256(p: Path) -> str:
    h = hashlib.sha256()
    with p.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()

def main():
    ap = argparse.ArgumentParser(description="Restore only SAFE_FULL_PATH_COPY assets from fresh v3 audit.")
    ap.add_argument("repo", nargs="?", default=".")
    ap.add_argument("action", choices=["audit", "apply", "rollback"], nargs="?", default="audit")
    ap.add_argument("--plan", default="bugfixes/058_asset_reconciliation/evidence/trusted_asset_hunt_v3/safe_full_path_copy.tsv")
    args = ap.parse_args()

    repo = Path(args.repo).expanduser().resolve()
    client = repo / "Summit Tale Client Full" / "cliente"
    plan = (repo / args.plan).resolve()
    evid = repo / "bugfixes" / "058_asset_reconciliation" / "evidence" / "safe_restore_apply"
    manifest = evid / "manifest.tsv"

    if args.action == "rollback":
        if not manifest.is_file():
            raise SystemExit("ERRO: manifest de apply não encontrado.")
        removed = 0
        with manifest.open("r", encoding="utf-8", newline="") as f:
            for r in csv.DictReader(f, delimiter="\t"):
                if r["action"] != "RESTORED":
                    continue
                dst = client / r["runtime_destination"]
                if dst.is_file() and sha256(dst) == r["source_sha256"]:
                    dst.unlink()
                    removed += 1
        print(f"OK: rollback removeu {removed} assets restaurados.")
        return

    if not client.is_dir():
        raise SystemExit(f"ERRO: cliente ausente: {client}")
    if not plan.is_file():
        raise SystemExit(f"ERRO: rode a v3 novamente; plano ausente: {plan}")

    rows = []
    conflicts = []
    with plan.open("r", encoding="utf-8", newline="") as f:
        for r in csv.DictReader(f, delimiter="\t"):
            if r.get("classification") != "SAFE_FULL_PATH_COPY":
                continue
            src = Path(r["source_path"])
            dst = client / r["runtime_destination"]
            expected = r["source_sha256"]
            if not src.is_file():
                action = "SOURCE_MISSING"
                conflicts.append((r["target_rel"], action))
            else:
                actual_src = sha256(src)
                if actual_src != expected:
                    action = "SOURCE_HASH_CHANGED"
                    conflicts.append((r["target_rel"], action))
                elif dst.is_file():
                    dh = sha256(dst)
                    if dh == expected:
                        action = "ALREADY_PRESENT_IDENTICAL"
                    else:
                        action = "DESTINATION_CONFLICT"
                        conflicts.append((r["target_rel"], action))
                else:
                    action = "RESTORE"
            rows.append({
                "action": action,
                "target_rel": r["target_rel"],
                "runtime_destination": r["runtime_destination"],
                "source_path": r["source_path"],
                "source_sha256": expected,
                "risk": r.get("risk",""),
                "references": r.get("references",""),
            })

    print("=== SAFE ASSET RESTORE 058 ===")
    for key in ["RESTORE","ALREADY_PRESENT_IDENTICAL","SOURCE_MISSING","SOURCE_HASH_CHANGED","DESTINATION_CONFLICT"]:
        print(f"{key}={sum(r['action']==key for r in rows)}")

    evid.mkdir(parents=True, exist_ok=True)
    with (evid / "audit.tsv").open("w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(f, fieldnames=["action","target_rel","runtime_destination","source_path","source_sha256","risk","references"],
                           delimiter="\t", lineterminator="\n")
        w.writeheader(); w.writerows(rows)

    if args.action == "audit":
        return

    if conflicts:
        for target, why in conflicts[:20]:
            print(f"CONFLICT: {target}: {why}")
        raise SystemExit("ERRO: conflito detectado; nada aplicado.")

    applied = []
    for r in rows:
        if r["action"] == "RESTORE":
            src = Path(r["source_path"])
            dst = client / r["runtime_destination"]
            dst.parent.mkdir(parents=True, exist_ok=True)
            # Never overwrite.
            if dst.exists():
                raise SystemExit(f"ERRO: destino apareceu durante apply: {dst}")
            shutil.copy2(src, dst)
            if sha256(dst) != r["source_sha256"]:
                dst.unlink(missing_ok=True)
                raise SystemExit(f"ERRO: SHA-256 pós-cópia divergente: {dst}")
            r = dict(r)
            r["action"] = "RESTORED"
        applied.append(r)

    with manifest.open("w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(f, fieldnames=["action","target_rel","runtime_destination","source_path","source_sha256","risk","references"],
                           delimiter="\t", lineterminator="\n")
        w.writeheader(); w.writerows(applied)

    print(f"OK: restaurados {sum(r['action']=='RESTORED' for r in applied)} assets; nenhum existente foi sobrescrito.")

if __name__ == "__main__":
    main()
