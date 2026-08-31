#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv
import hashlib
import os
import shutil
from collections import Counter, defaultdict
from pathlib import Path

DEFAULT_SOURCE = Path("/home/gabriel/Área de Trabalho/priston-official/ptbr-antigo")
DEFAULT_CLIENT_REL = Path("Summit Tale Client Full/cliente")
EVID_REL = Path("bugfixes/058_asset_reconciliation/evidence/ptbr_antigo_missing_sync")

# We want client content, not old executables/runtime state.
EXCLUDED_EXTS = {
    ".exe", ".dll", ".sys", ".drv", ".ocx", ".com", ".bat", ".cmd",
    ".msi", ".msp", ".reg", ".lnk", ".url",
    ".pdb", ".ilk", ".lib", ".a", ".so", ".dylib",
}
EXCLUDED_DIR_NAMES = {
    "logs", "log", "crash", "crashes", "dump", "dumps",
    "screenshots", "screenshot", "capture", "captures",
    "userdata", "user_data", "account", "accounts",
    "temp", "tmp", "__pycache__",
}
EXCLUDED_NAME_MARKERS = (
    "_debug.", "debug.", "_trace.", "trace.",
)

def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()

def excluded(rel: Path) -> tuple[bool, str]:
    parts_cf = [p.casefold() for p in rel.parts[:-1]]
    for p in parts_cf:
        if p in EXCLUDED_DIR_NAMES:
            return True, f"excluded_dir:{p}"

    name_cf = rel.name.casefold()
    if rel.suffix.casefold() in EXCLUDED_EXTS:
        return True, f"excluded_ext:{rel.suffix.casefold()}"
    if name_cf.endswith(".log"):
        return True, "excluded_log"
    if any(m in name_cf for m in EXCLUDED_NAME_MARKERS):
        return True, "excluded_debug_trace"
    return False, ""

def scandir_case_map(directory: Path):
    exact = {}
    folded = defaultdict(list)
    try:
        with os.scandir(directory) as it:
            for ent in it:
                exact[ent.name] = ent.name
                folded[ent.name.casefold()].append(ent.name)
    except FileNotFoundError:
        pass
    return exact, folded

def resolve_parent_case(client: Path, parent_parts: tuple[str, ...]):
    """
    Resolve source parent components against CURRENT client case.
    Existing unique case-insensitive components win, preventing Help/help-style
    shadow directories. Missing components keep source casing.
    """
    cur = client
    resolved = []
    virtual_missing = False

    for part in parent_parts:
        if cur.is_dir():
            exact, folded = scandir_case_map(cur)
            if part in exact:
                chosen = part
            else:
                matches = folded.get(part.casefold(), [])
                if len(matches) == 1:
                    chosen = matches[0]
                elif len(matches) > 1:
                    return None, f"AMBIGUOUS_PARENT_CASE:{cur}/{part}:{matches}"
                else:
                    chosen = part
                    virtual_missing = True
        else:
            chosen = part
            virtual_missing = True

        cur = cur / chosen
        resolved.append(chosen)

    return Path(*resolved) if resolved else Path(), ""

def classify_one(client: Path, rel: Path):
    parent_rel, err = resolve_parent_case(client, rel.parts[:-1])
    if err:
        return {
            "status": "AMBIGUOUS_PARENT_CASE",
            "dest_rel": "",
            "note": err,
        }

    parent = client / parent_rel
    leaf = rel.name

    if parent.is_dir():
        exact, folded = scandir_case_map(parent)
        if leaf in exact:
            dst = parent / leaf
            return {
                "status": "EXISTS_EXACT",
                "dest_rel": dst.relative_to(client).as_posix(),
                "note": "",
            }

        matches = folded.get(leaf.casefold(), [])
        if len(matches) == 1:
            dst = parent / matches[0]
            return {
                "status": "EXISTS_CASE_VARIANT",
                "dest_rel": dst.relative_to(client).as_posix(),
                "note": f"source_leaf={leaf};runtime_leaf={matches[0]}",
            }
        if len(matches) > 1:
            return {
                "status": "AMBIGUOUS_LEAF_CASE",
                "dest_rel": "",
                "note": f"{parent}/{leaf}:{matches}",
            }

    dst = parent / leaf
    return {
        "status": "MISSING_COPY",
        "dest_rel": dst.relative_to(client).as_posix(),
        "note": "",
    }

def build_inventory(source: Path, client: Path):
    source_files = []
    excluded_rows = []

    for p in source.rglob("*"):
        if p.is_symlink():
            if p.exists() and p.is_file():
                rel = p.relative_to(source)
                excluded_rows.append((rel, "excluded_symlink"))
            continue
        if not p.is_file():
            continue

        rel = p.relative_to(source)
        is_exc, why = excluded(rel)
        if is_exc:
            excluded_rows.append((rel, why))
            continue
        source_files.append((rel, p))

    # Detect source paths that collide case-insensitively.
    source_case = defaultdict(list)
    for rel, p in source_files:
        source_case[rel.as_posix().casefold()].append((rel, p))

    rows = []
    for rel, src in source_files:
        coll = source_case[rel.as_posix().casefold()]
        if len(coll) > 1:
            rows.append({
                "status": "SOURCE_CASE_AMBIGUOUS",
                "source_rel": rel.as_posix(),
                "dest_rel": "",
                "source_path": str(src),
                "source_size": src.stat().st_size,
                "source_sha256": "",
                "dest_sha256": "",
                "note": " | ".join(x[0].as_posix() for x in coll),
            })
            continue

        c = classify_one(client, rel)
        row = {
            "status": c["status"],
            "source_rel": rel.as_posix(),
            "dest_rel": c["dest_rel"],
            "source_path": str(src),
            "source_size": src.stat().st_size,
            "source_sha256": "",
            "dest_sha256": "",
            "note": c["note"],
        }

        # Hash only cases where content comparison or copying matters.
        if c["status"] in {"MISSING_COPY", "EXISTS_EXACT", "EXISTS_CASE_VARIANT"}:
            src_hash = sha256(src)
            row["source_sha256"] = src_hash

            if c["status"] != "MISSING_COPY":
                dst = client / c["dest_rel"]
                if dst.is_file():
                    dst_hash = sha256(dst)
                    row["dest_sha256"] = dst_hash
                    if dst_hash == src_hash:
                        row["status"] += "_SAME"
                    else:
                        row["status"] += "_DIFF"
                else:
                    row["status"] = "DESTINATION_TYPE_CONFLICT"
                    row["note"] = f"destination is not file: {dst}"

        rows.append(row)

    return rows, excluded_rows

def write_reports(evid: Path, rows, excluded_rows):
    evid.mkdir(parents=True, exist_ok=True)

    fields = [
        "status", "source_rel", "dest_rel", "source_path",
        "source_size", "source_sha256", "dest_sha256", "note"
    ]
    with (evid / "inventory.tsv").open("w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(f, fieldnames=fields, delimiter="\t", lineterminator="\n")
        w.writeheader()
        w.writerows(rows)

    missing = [r for r in rows if r["status"] == "MISSING_COPY"]
    with (evid / "missing_to_copy.tsv").open("w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(f, fieldnames=fields, delimiter="\t", lineterminator="\n")
        w.writeheader()
        w.writerows(missing)

    # Focus report for item/armor restoration.
    item_rows = []
    for r in missing:
        low = r["dest_rel"].casefold()
        if (
            "/items/" in f"/{low}"
            or low.startswith("image/sinimage/items/")
            or "/skins/" in f"/{low}"
            or "/defense/" in f"/{low}"
            or low.startswith("char/")
        ):
            item_rows.append(r)

    with (evid / "missing_items_char.tsv").open("w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(f, fieldnames=fields, delimiter="\t", lineterminator="\n")
        w.writeheader()
        w.writerows(item_rows)

    with (evid / "excluded.tsv").open("w", encoding="utf-8", newline="") as f:
        w = csv.writer(f, delimiter="\t", lineterminator="\n")
        w.writerow(["source_rel", "reason"])
        for rel, why in excluded_rows:
            w.writerow([rel.as_posix(), why])

    status_counts = Counter(r["status"] for r in rows)
    top_counts = Counter()
    ext_counts = Counter()
    missing_bytes = 0

    for r in missing:
        rel = Path(r["dest_rel"])
        top = rel.parts[0] if rel.parts else "(root)"
        top_counts[top] += 1
        ext_counts[rel.suffix.casefold() or "(no_ext)"] += 1
        missing_bytes += int(r["source_size"])

    with (evid / "summary.txt").open("w", encoding="utf-8") as f:
        f.write(f"SOURCE_FILES_CONSIDERED={len(rows)}\n")
        f.write(f"EXCLUDED={len(excluded_rows)}\n")
        for k in sorted(status_counts):
            f.write(f"{k}={status_counts[k]}\n")
        f.write(f"MISSING_BYTES={missing_bytes}\n")
        f.write("\nMISSING_BY_TOP_LEVEL\n")
        for k, v in top_counts.most_common():
            f.write(f"{k}\t{v}\n")
        f.write("\nMISSING_BY_EXTENSION\n")
        for k, v in ext_counts.most_common():
            f.write(f"{k}\t{v}\n")

def apply_rows(repo: Path, source: Path, client: Path, evid: Path, rows):
    planned = [r for r in rows if r["status"] == "MISSING_COPY"]
    if not planned:
        print("OK: nenhum arquivo ausente para copiar.")
        return

    manifest_rows = []
    created = []
    manifest = evid / "apply_manifest.tsv"

    try:
        for idx, r in enumerate(planned, 1):
            src = Path(r["source_path"])
            # Re-resolve on each copy so newly-created directories also become
            # canonical for later siblings.
            rel = Path(r["source_rel"])
            c = classify_one(client, rel)
            if c["status"] != "MISSING_COPY":
                # It appeared since audit; do not overwrite.
                continue

            dst = client / c["dest_rel"]
            dst.parent.mkdir(parents=True, exist_ok=True)

            if dst.exists():
                raise RuntimeError(f"destino apareceu antes da cópia: {dst}")

            shutil.copy2(src, dst)
            src_hash = r["source_sha256"] or sha256(src)
            dst_hash = sha256(dst)
            if dst_hash != src_hash:
                raise RuntimeError(f"SHA divergiu: {dst}")

            created.append(dst)
            manifest_rows.append({
                "source_rel": rel.as_posix(),
                "dest_rel": dst.relative_to(client).as_posix(),
                "source_sha256": src_hash,
                "target_sha256": dst_hash,
                "size": dst.stat().st_size,
            })

            if idx % 500 == 0:
                print(f"copied={idx}/{len(planned)}")

    except Exception:
        for p in reversed(created):
            try:
                p.unlink()
            except OSError:
                pass
        # Remove only empty dirs created under client.
        for p in sorted({x.parent for x in created}, key=lambda x: len(x.parts), reverse=True):
            try:
                p.rmdir()
            except OSError:
                pass
        raise

    fields = ["source_rel", "dest_rel", "source_sha256", "target_sha256", "size"]
    with manifest.open("w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(f, fieldnames=fields, delimiter="\t", lineterminator="\n")
        w.writeheader()
        w.writerows(manifest_rows)

    print(f"OK: copiados {len(manifest_rows)} arquivos ausentes; nenhum existente foi sobrescrito.")
    print(f"MANIFEST={manifest}")

def rollback(client: Path, evid: Path):
    manifest = evid / "apply_manifest.tsv"
    if not manifest.is_file():
        raise SystemExit(f"ERRO: manifest ausente: {manifest}")

    with manifest.open("r", encoding="utf-8", newline="") as f:
        rows = list(csv.DictReader(f, delimiter="\t"))

    # Verify every existing created file first. No partial rollback on hash drift.
    for r in rows:
        p = client / r["dest_rel"]
        if not p.exists():
            continue
        if not p.is_file():
            raise SystemExit(f"ERRO: rollback recusado; não é arquivo: {p}")
        cur = sha256(p)
        if cur != r["target_sha256"]:
            raise SystemExit(
                "ERRO: rollback recusado; arquivo criado foi modificado:\n"
                f"{p}\natual={cur}\nesperado={r['target_sha256']}"
            )

    removed = []
    for r in reversed(rows):
        p = client / r["dest_rel"]
        if p.is_file():
            p.unlink()
            removed.append(p)

    # Remove now-empty directories only.
    parents = sorted({p.parent for p in removed}, key=lambda x: len(x.parts), reverse=True)
    for p in parents:
        while p != client:
            try:
                p.rmdir()
            except OSError:
                break
            p = p.parent

    print(f"OK: rollback removeu {len(removed)} arquivos criados pelo sync.")

def main():
    ap = argparse.ArgumentParser(
        description="Sync files missing from Summit client from trusted priston-official ptbr-antigo; never overwrite."
    )
    ap.add_argument("repo", nargs="?", default=".")
    ap.add_argument("action", choices=["audit", "apply", "rollback"], nargs="?", default="audit")
    ap.add_argument("--source", default=str(DEFAULT_SOURCE))
    args = ap.parse_args()

    repo = Path(args.repo).expanduser().resolve()
    source = Path(args.source).expanduser().resolve()
    client = repo / DEFAULT_CLIENT_REL
    evid = repo / EVID_REL

    if not source.is_dir():
        raise SystemExit(f"ERRO: ptbr-antigo não encontrado: {source}")
    if not client.is_dir():
        raise SystemExit(f"ERRO: client Summit não encontrado: {client}")

    if args.action == "rollback":
        rollback(client, evid)
        return

    print("=== PTBR-ANTIGO -> SUMMIT MISSING-ONLY SYNC 058 ===")
    print(f"source={source}")
    print(f"client={client}")
    print("policy=no-overwrite + case-aware + rollback-by-sha256")

    rows, excluded_rows = build_inventory(source, client)
    write_reports(evid, rows, excluded_rows)

    counts = Counter(r["status"] for r in rows)
    print(f"SOURCE_FILES_CONSIDERED={len(rows)}")
    print(f"EXCLUDED={len(excluded_rows)}")
    for k in sorted(counts):
        print(f"{k}={counts[k]}")

    missing = [r for r in rows if r["status"] == "MISSING_COPY"]
    item_missing = [
        r for r in missing
        if (
            r["dest_rel"].casefold().startswith("image/sinimage/items/")
            or r["dest_rel"].casefold().startswith("char/")
        )
    ]
    print(f"MISSING_COPY={len(missing)}")
    print(f"MISSING_ITEMS_OR_CHAR={len(item_missing)}")
    print(f"REPORT={evid / 'summary.txt'}")
    print(f"ITEM_REPORT={evid / 'missing_items_char.tsv'}")

    blockers = [
        r for r in rows
        if r["status"] in {
            "SOURCE_CASE_AMBIGUOUS", "AMBIGUOUS_PARENT_CASE",
            "AMBIGUOUS_LEAF_CASE", "DESTINATION_TYPE_CONFLICT"
        }
    ]
    print(f"BLOCKERS={len(blockers)}")

    if args.action == "audit":
        print("APPLY=NOT_RUN")
        return

    if blockers:
        raise SystemExit("ERRO: há blockers de case/type; nada aplicado. Revise inventory.tsv.")

    apply_rows(repo, source, client, evid, rows)

if __name__ == "__main__":
    main()
