#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv
import hashlib
import shutil
from pathlib import Path

SOURCE = Path("/home/gabriel/Área de Trabalho/priston-official/ptbr-antigo/image/Sinimage/Premium/Vampiric_back_EX.bmp")
TARGET_REL = Path("image/Sinimage/Premium/Vampiric_back_EX.bmp")
EXPECTED_SOURCE_SHA256 = "53b1653e6186a4f7a3151adaed304f4320d64f5bfa2a28b5de31c16f6fa872b2"

def sha256(p: Path) -> str:
    h = hashlib.sha256()
    with p.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()

def main():
    ap = argparse.ArgumentParser(description="Restore exact Vampiric_back_EX.bmp from trusted priston-official.")
    ap.add_argument("repo", nargs="?", default=".")
    ap.add_argument("action", choices=["audit", "apply", "rollback"], nargs="?", default="audit")
    args = ap.parse_args()

    repo = Path(args.repo).expanduser().resolve()
    client = repo / "Summit Tale Client Full" / "cliente"
    target = client / TARGET_REL

    evid = repo / "bugfixes/058_asset_reconciliation/evidence/vampiric_back_ex_exact_restore"
    evid.mkdir(parents=True, exist_ok=True)
    manifest = evid / "manifest.tsv"
    evidence_copy = evid / "Vampiric_back_EX.bmp"

    if args.action == "rollback":
        if not manifest.is_file():
            raise SystemExit("ERRO: manifest ausente.")
        with manifest.open("r", encoding="utf-8", newline="") as f:
            rows = list(csv.DictReader(f, delimiter="\t"))
        if len(rows) != 1:
            raise SystemExit("ERRO: manifest inesperado.")
        r = rows[0]
        if not target.exists():
            print("OK: target já está ausente.")
            return
        cur = sha256(target)
        if cur != r["target_sha256"]:
            raise SystemExit(
                "ERRO: rollback recusado; arquivo mudou após restauração.\n"
                f"atual={cur}\nesperado={r['target_sha256']}"
            )
        target.unlink()
        print(f"OK: rollback removeu somente {target}")
        return

    if not SOURCE.is_file():
        raise SystemExit(f"ERRO: source ausente: {SOURCE}")
    if not client.is_dir():
        raise SystemExit(f"ERRO: client ausente: {client}")

    src_sha = sha256(SOURCE)
    if src_sha != EXPECTED_SOURCE_SHA256:
        raise SystemExit(
            "ERRO: SHA do trusted source mudou.\n"
            f"atual={src_sha}\nesperado={EXPECTED_SOURCE_SHA256}"
        )

    print("=== EXACT ASSET RESTORE 058: Vampiric_back_EX.bmp ===")
    print(f"source={SOURCE}")
    print(f"source_sha256={src_sha}")
    print(f"target={target}")
    print(f"target_exists={int(target.exists())}")

    if target.exists():
        dst_sha = sha256(target)
        print(f"target_sha256={dst_sha}")
        if dst_sha == src_sha:
            print("ALREADY_PRESENT_IDENTICAL=1")
            return
        raise SystemExit("ERRO: target já existe com conteúdo diferente; não sobrescrevo.")

    data = SOURCE.read_bytes()
    evidence_copy.write_bytes(data)
    if sha256(evidence_copy) != src_sha:
        raise SystemExit("ERRO: evidence copy divergiu.")

    print(f"size={len(data)}")
    print("SOURCE_HASH_VALID=YES")
    print("DESTINATION_CONFLICT=NO")

    if args.action == "audit":
        print("APPLY=NOT_RUN")
        return

    target.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(SOURCE, target)

    dst_sha = sha256(target)
    if dst_sha != src_sha:
        target.unlink(missing_ok=True)
        raise SystemExit("ERRO: SHA do runtime divergiu após copy.")

    with manifest.open("w", encoding="utf-8", newline="") as f:
        fields = ["target","source","source_sha256","target_sha256","created"]
        w = csv.DictWriter(f, fieldnames=fields, delimiter="\t", lineterminator="\n")
        w.writeheader()
        w.writerow({
            "target": str(target),
            "source": str(SOURCE),
            "source_sha256": src_sha,
            "target_sha256": dst_sha,
            "created": "1",
        })

    print(f"OK: restaurado SOMENTE {target}")
    print(f"runtime_sha256={dst_sha}")
    print("RUNTIME_TEST=READY")

if __name__ == "__main__":
    main()
