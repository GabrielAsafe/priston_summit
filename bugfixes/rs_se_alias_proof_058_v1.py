#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv
import hashlib
import re
from pathlib import Path

PAIRS = [
    ("RS102", "SE106"),
    ("RS103", "SE107"),
    ("RS104", "SE108"),
    ("RS105", "SE109"),
    ("RS106", "SE110"),
    ("RS107", "SE111"),
    ("RS108", "SE112"),
]

ASSET_ROOTS = [
    ("Summit-current", None),
    ("ExMachina", Path("/home/gabriel/armazem/priston/ExMachina")),
    ("pristontale-1-master", Path("/home/gabriel/armazem/priston/pristontale-1-master")),
    ("priston-official", Path("/home/gabriel/Área de Trabalho/priston-official")),
]

IMAGE_EXTS = {".bmp", ".tga", ".dds", ".png", ".jpg", ".jpeg"}
QUOTED_RE = re.compile(r'["\']([^"\']+)["\']')

def sha256(p: Path) -> str:
    h = hashlib.sha256()
    with p.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()

def read_text(p: Path) -> str:
    data = p.read_bytes()
    for enc in ("utf-8", "cp1252", "latin1"):
        try:
            return data.decode(enc)
        except UnicodeDecodeError:
            pass
    return data.decode("latin1", errors="replace")

def normalized_semantic_lines(text: str):
    out = []
    for raw in text.splitlines():
        s = raw.strip()
        if not s:
            continue
        # Preserve all content; normalize whitespace only.
        s = " ".join(s.split())
        out.append(s)
    return out

def quoted_values(text: str):
    vals = []
    for m in QUOTED_RE.finditer(text):
        v = m.group(1).strip()
        if v not in vals:
            vals.append(v)
    return vals

def find_assets_for_stem(root: Path, stem: str):
    out = []
    stem_cf = stem.casefold()
    for p in root.rglob("*"):
        if not p.is_file() or p.is_symlink():
            continue
        if p.suffix.casefold() not in IMAGE_EXTS:
            continue
        if p.stem.casefold() in {stem_cf, ("it" + stem).casefold()}:
            out.append(p)
    return out

def main():
    ap = argparse.ArgumentParser(description="Prove RS<->SE OpenItem aliases without creating/converting images.")
    ap.add_argument("repo", nargs="?", default=".")
    args = ap.parse_args()

    repo = Path(args.repo).expanduser().resolve()
    client = repo / "Summit Tale Client Full" / "cliente"
    openitem = repo / "Servidor" / "GameServer" / "OpenItem"
    outdir = repo / "bugfixes/058_asset_reconciliation/evidence/rs_se_alias_proof"
    outdir.mkdir(parents=True, exist_ok=True)

    rows = []
    asset_rows = []

    for rs, se in PAIRS:
        rsf = openitem / f"{rs}.txt"
        sef = openitem / f"{se}.txt"

        if not rsf.is_file() or not sef.is_file():
            rows.append({
                "rs": rs,
                "se": se,
                "rs_file": str(rsf),
                "se_file": str(sef),
                "rs_sha256": sha256(rsf) if rsf.is_file() else "",
                "se_sha256": sha256(sef) if sef.is_file() else "",
                "byte_identical": "NO",
                "semantic_identical": "NO",
                "rs_quoted": "",
                "se_quoted": "",
                "first_diff": "MISSING_FILE",
            })
            continue

        rsb = rsf.read_bytes()
        seb = sef.read_bytes()
        rst = read_text(rsf)
        set_ = read_text(sef)
        rsn = normalized_semantic_lines(rst)
        sen = normalized_semantic_lines(set_)

        first_diff = ""
        maxlen = max(len(rsn), len(sen))
        for i in range(maxlen):
            a = rsn[i] if i < len(rsn) else "<EOF>"
            b = sen[i] if i < len(sen) else "<EOF>"
            if a != b:
                first_diff = f"line={i+1} RS={a!r} SE={b!r}"
                break

        rows.append({
            "rs": rs,
            "se": se,
            "rs_file": str(rsf.relative_to(repo)),
            "se_file": str(sef.relative_to(repo)),
            "rs_sha256": sha256(rsf),
            "se_sha256": sha256(sef),
            "byte_identical": "YES" if rsb == seb else "NO",
            "semantic_identical": "YES" if rsn == sen else "NO",
            "rs_quoted": ";".join(quoted_values(rst)),
            "se_quoted": ";".join(quoted_values(set_)),
            "first_diff": first_diff,
        })

        for root_name, root in ASSET_ROOTS:
            if root_name == "Summit-current":
                root = client
            if not root or not root.is_dir():
                continue

            for role, stem in (("RS", rs), ("SE", se)):
                for p in find_assets_for_stem(root, stem):
                    try:
                        rel = p.relative_to(root).as_posix()
                    except ValueError:
                        rel = str(p)
                    asset_rows.append({
                        "rs": rs,
                        "se": se,
                        "role": role,
                        "asset_code": stem,
                        "source_root": root_name,
                        "source_rel": rel,
                        "source_path": str(p),
                        "ext": p.suffix,
                        "size": p.stat().st_size,
                        "sha256": sha256(p),
                    })

    fields = [
        "rs","se","rs_file","se_file","rs_sha256","se_sha256",
        "byte_identical","semantic_identical","rs_quoted","se_quoted","first_diff"
    ]
    with (outdir / "pair_compare.tsv").open("w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(f, fieldnames=fields, delimiter="\t", lineterminator="\n")
        w.writeheader()
        w.writerows(rows)

    afields = [
        "rs","se","role","asset_code","source_root","source_rel",
        "source_path","ext","size","sha256"
    ]
    with (outdir / "pair_assets.tsv").open("w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(f, fieldnames=afields, delimiter="\t", lineterminator="\n")
        w.writeheader()
        w.writerows(asset_rows)

    print("=== RS <-> SE ALIAS PROOF 058 ===")
    print("policy=AUDIT_ONLY; NO_IMAGE_GENERATION; NO_CONVERSION; NO_COPY")
    for r in rows:
        print(
            f"{r['rs']} <- {r['se']} "
            f"byte_identical={r['byte_identical']} "
            f"semantic_identical={r['semantic_identical']}"
        )
        print(f"  RS quoted: {r['rs_quoted']}")
        print(f"  SE quoted: {r['se_quoted']}")
        if r["first_diff"]:
            print(f"  first_diff: {r['first_diff']}")

        aa = [x for x in asset_rows if x["rs"] == r["rs"]]
        if aa:
            for x in aa:
                print(
                    f"  ASSET {x['role']} {x['source_root']:20s} "
                    f"{x['source_rel']} sha256={x['sha256']}"
                )
        else:
            print("  ASSET: none")

    print(f"OUT={outdir}")

if __name__ == "__main__":
    main()
