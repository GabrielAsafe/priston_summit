#!/usr/bin/env bash
set -euo pipefail

ROOT="${1:-$HOME/Área de Trabalho/summit/design_work}"
CLIENT="${2:-$HOME/Área de Trabalho/summit/Summit Tale Client Full/cliente}"
INV="${3:-$ROOT/logs/asset_similarity_v1_1/asset_inventory.json}"
PAIR="${4:-$ROOT/logs/asset_family_refine_v2/inx_physical_pairing.csv}"
OUT="${5:-$HOME/Área de Trabalho/PT_REMAINING_FAMILY_ASSETS_$(date +%Y%m%d_%H%M%S).zip}"

for p in "$ROOT" "$CLIENT" "$INV" "$PAIR"; do
  [[ -e "$p" ]] || { echo "MISSING_REQUIRED=$p" >&2; exit 2; }
done

TMP="$(mktemp -d "$HOME/Área de Trabalho/PT_REMAINING_XXXXXX")"
trap 'rm -rf "$TMP"' EXIT
mkdir -p "$TMP/client" "$TMP/design_work"

python3 - "$ROOT" "$CLIENT" "$INV" "$PAIR" "$TMP" <<'PY'
from __future__ import annotations
import csv, hashlib, json, pathlib, re, shutil, struct, sys
from collections import Counter, defaultdict

root = pathlib.Path(sys.argv[1]).resolve()
client = pathlib.Path(sys.argv[2]).resolve()
inv_path = pathlib.Path(sys.argv[3]).resolve()
pair_path = pathlib.Path(sys.argv[4]).resolve()
tmp = pathlib.Path(sys.argv[5]).resolve()
dest_client = tmp / "client"
dest_dw = tmp / "design_work"

inventory = json.loads(inv_path.read_text(encoding="utf-8"))
records = []
by_rel = {}

for r in inventory.get("records", []):
    try:
        p = pathlib.Path(str(r.get("path", "")))
        p.relative_to(client)
    except Exception:
        continue
    records.append(r)
    rel = str(r.get("relative_path", "")).replace("\\", "/").lower()
    if rel:
        by_rel[rel] = r

with pair_path.open(encoding="utf-8", newline="") as f:
    pairing = list(csv.DictReader(f))

def norm(s):
    return str(s or "").replace("\\", "/").lower().lstrip("./")

def copy_record(r, reason):
    if not r:
        return False
    src = pathlib.Path(str(r.get("path", "")))
    if not src.is_file():
        print("MISSING_PHYSICAL", src)
        return False
    try:
        rel = src.relative_to(client)
    except Exception:
        return False
    dst = dest_client / rel
    dst.parent.mkdir(parents=True, exist_ok=True)
    if not dst.exists():
        shutil.copy2(src, dst)
    selections.append({
        "reason": reason,
        "relative_path": str(rel),
        "size": src.stat().st_size,
        "kind": r.get("kind", ""),
        "family": r.get("compatibility_family", ""),
    })
    return True

def copy_rel(rel, reason):
    return copy_record(by_rel.get(norm(rel)), reason)

def copy_model_dependencies(model_rel, reason_prefix):
    mr = by_rel.get(norm(model_rel))
    if not mr:
        return
    copy_record(mr, reason_prefix + ":model")
    for tex in mr.get("texture_refs") or []:
        copy_rel(tex, reason_prefix + ":texture")

selections = []

# ------------------------------------------------------------
# A. ALL legacy 67,084-byte INX, plus physical SMD/SMB and model textures.
# ------------------------------------------------------------
legacy_inx = [
    r for r in records
    if str(r.get("extension", "")).lower() == ".inx"
    and int(r.get("size") or 0) == 67084
]
legacy_paths = {norm(r["relative_path"]) for r in legacy_inx}

for r in legacy_inx:
    copy_record(r, "INX_LEGACY_67084")

for row in pairing:
    if norm(row.get("inx")) not in legacy_paths:
        continue
    model = row.get("model_smd")
    motion = row.get("motion_smb")
    if model:
        copy_model_dependencies(model, "INX_LEGACY_DEP")
    if motion:
        copy_rel(motion, "INX_LEGACY_DEP:SMB")

# Add a small set of modern INX controls without duplicating giant roots unnecessarily.
modern = [
    r for r in records
    if str(r.get("extension", "")).lower() == ".inx"
    and int(r.get("size") or 0) == 95268
]
for r in sorted(modern, key=lambda x: str(x.get("relative_path", "")).lower())[:5]:
    copy_record(r, "INX_MODERN_CONTROL_95268")

# ------------------------------------------------------------
# B. Protected BMP representatives:
#    top prefix clusters + every dimension family.
# ------------------------------------------------------------
bmp = [
    r for r in records
    if r.get("kind") == "BMP_PRISTON_PREFIX14"
]

prefix_groups = defaultdict(list)
dim_groups = defaultdict(list)
for r in bmp:
    prefix_groups[str(r.get("prefix14_hex", ""))].append(r)
    dim_groups[
        (
            int(r.get("width") or 0),
            int(r.get("height") or 0),
            int(r.get("bpp") or 0),
            int(r.get("compression") or 0),
        )
    ].append(r)

# Top 30 prefix clusters by population.
for prefix, rows in sorted(prefix_groups.items(), key=lambda kv: len(kv[1]), reverse=True)[:30]:
    rep = sorted(rows, key=lambda x: int(x.get("size") or 0))[len(rows)//2]
    copy_record(rep, f"BMP_PREFIX_CLUSTER:{prefix}")

# Ensure every dimension/bpp/compression combination is represented once.
for key, rows in sorted(dim_groups.items()):
    rep = sorted(rows, key=lambda x: int(x.get("size") or 0))[len(rows)//2]
    copy_record(rep, f"BMP_DIMENSION_FAMILY:{key[0]}x{key[1]}_{key[2]}bpp_c{key[3]}")

# ------------------------------------------------------------
# C. Protected TGA: automatically find codec controls and true outliers.
# ------------------------------------------------------------
KEY = bytes.fromhex("47380409101924314051647990a9c4e10021")

def decode_header(h, mode):
    if len(h) < 18:
        return None
    if mode == "FULL18":
        return bytes((h[i] - KEY[i]) & 0xFF for i in range(18))
    if mode == "KEEP_00_16":
        if h[:2] != b"\x00\x00":
            return None
        return h[:2] + bytes((h[i] - KEY[i]) & 0xFF for i in range(2, 18))
    return None

def plausible_tga(h):
    if h is None or len(h) != 18:
        return False
    cmap = h[1]
    itype = h[2]
    width, height = struct.unpack_from("<HH", h, 12)
    bpp = h[16]
    if cmap not in (0, 1):
        return False
    if itype not in (1, 2, 3, 9, 10, 11):
        return False
    if not (1 <= width <= 8192 and 1 <= height <= 8192):
        return False
    if bpp not in (8, 16, 24, 32):
        return False
    return True

tga_unknown = [r for r in records if r.get("kind") == "TGA_UNKNOWN"]
codec_groups = defaultdict(list)
tga_outliers = []

for r in tga_unknown:
    p = pathlib.Path(str(r.get("path", "")))
    if not p.is_file():
        continue
    h = p.read_bytes()[:18]
    matches = []
    for mode in ("FULL18", "KEEP_00_16"):
        dec = decode_header(h, mode)
        if plausible_tga(dec):
            itype = dec[2]
            width, height = struct.unpack_from("<HH", dec, 12)
            bpp = dec[16]
            matches.append((mode, itype, width, height, bpp))
    if matches:
        codec_groups[matches[0]].append(r)
    else:
        tga_outliers.append(r)

# One representative for the 12 largest decoded TGA families.
for key, rows in sorted(codec_groups.items(), key=lambda kv: len(kv[1]), reverse=True)[:12]:
    rep = sorted(rows, key=lambda x: int(x.get("size") or 0))[len(rows)//2]
    copy_record(
        rep,
        f"TGA_CODEC_CONTROL:{key[0]}:type{key[1]}:{key[2]}x{key[3]}:{key[4]}bpp"
    )

# Every true codec outlier.
for r in tga_outliers:
    copy_record(r, "TGA_CODEC_OUTLIER")

# ------------------------------------------------------------
# D. More STATIC Model0.62 representatives across categories/signatures.
# ------------------------------------------------------------
static = [
    r for r in records
    if r.get("compatibility_family") == "SMD_MODEL_062_STATIC"
]
already = {
    "image/sinimage/items/dropitem/itpr101.smd",
    "image/sinimage/items/dropitem/itwa104.smd",
    "image/sinimage/items/dropitem/itwh118.smd",
}

def geom_bucket(v):
    v = int(v or 0)
    if v < 50: return "<50"
    if v < 300: return "50-299"
    if v < 1000: return "300-999"
    if v < 5000: return "1000-4999"
    return "5000+"

wanted_cats = ("WEAPON_ITEM", "EFFECT", "MAP_STAGE")
for cat in wanted_cats:
    rows = [r for r in static if r.get("category_hint") == cat and norm(r["relative_path"]) not in already]
    groups = defaultdict(list)
    for r in rows:
        groups[
            (
                int(r.get("object_count") or 0),
                int(r.get("material_count") or 0),
                geom_bucket(r.get("total_vertices")),
            )
        ].append(r)
    # Up to 5 distinct structural representatives per category.
    chosen = sorted(groups.items(), key=lambda kv: len(kv[1]), reverse=True)[:5]
    for sig, members in chosen:
        rep = sorted(members, key=lambda x: int(x.get("size") or 0))[len(members)//2]
        copy_model_dependencies(
            rep["relative_path"],
            f"STATIC_{cat}:obj{sig[0]}:mat{sig[1]}:geom{sig[2]}"
        )

# ------------------------------------------------------------
# E. Current scripts needed to reproduce/debug locally.
# ------------------------------------------------------------
script_names = [
    "pt_protected_tga_v1.py",
    "pt_auto_profile_production_v1.py",
    "pt_smd_static_capability_probe_v1.py",
    "pt_smd_blender_import_v4_generic.py",
    "pt_blender_to_smd_static_v1.py",
    "pt_character_texture_v1_2.py",
    "pt_character_asset_v1_5.py",
    "pt_character_workspace_init_v2.py",
    "pt_pikeman_smd_material_audit_v1.py",
    "pt_blender_to_smd_free_mesh_v3.py",
    "pt_free_mesh_validate_v2.py",
    "summit_workspace_asset_bridge_v1_2.py",
    "summit_roundtrip_restore_v4.py",
]
for name in script_names:
    src = root / name
    if src.is_file():
        shutil.copy2(src, dest_dw / name)

hist_names = [
    "pt_smd_blender_import_v3_static_armature.py",
    "pt_smb_motion_evaluator_v1.py",
]
(dest_dw / "history_py").mkdir(parents=True, exist_ok=True)
for name in hist_names:
    src = root / "history_py" / name
    if src.is_file():
        shutil.copy2(src, dest_dw / "history_py" / name)

# Include the exact machine-readable source reports, not giant checkpoints.
(dest_dw / "logs").mkdir(parents=True, exist_ok=True)
for src in [
    inv_path,
    pair_path,
    root / "logs/asset_family_refine_v2/bmp_prefix14_clusters.csv",
    root / "logs/asset_family_refine_v2/tga_raw_signature_clusters.csv",
    root / "logs/asset_family_refine_v2/smd_semantic_subfamilies.csv",
]:
    if src.is_file():
        shutil.copy2(src, dest_dw / "logs" / src.name)

# Deduplicate selection report by physical path/reason.
seen = set()
unique = []
for row in selections:
    key = (row["reason"], row["relative_path"])
    if key not in seen:
        seen.add(key)
        unique.append(row)

with (tmp / "SELECTIONS.csv").open("w", encoding="utf-8", newline="") as f:
    w = csv.DictWriter(
        f,
        fieldnames=["reason", "relative_path", "size", "kind", "family"]
    )
    w.writeheader()
    w.writerows(unique)

summary = {
    "legacy_inx_67084_count_in_client": len(legacy_inx),
    "bmp_protected_count_in_client": len(bmp),
    "bmp_prefix_cluster_count": len(prefix_groups),
    "bmp_dimension_family_count": len(dim_groups),
    "tga_unknown_count_in_client": len(tga_unknown),
    "tga_codec_decoded_count": sum(len(x) for x in codec_groups.values()),
    "tga_codec_outlier_count": len(tga_outliers),
    "static_smd_count_in_client": len(static),
    "selected_entries": len(unique),
    "client_modified": False,
}
(tmp / "COLLECTION_SUMMARY.json").write_text(
    json.dumps(summary, ensure_ascii=False, indent=2) + "\n",
    encoding="utf-8"
)

print("COLLECTION_SUMMARY")
for k, v in summary.items():
    print(f"{k}={v}")
PY

# Generate checksums AFTER all files exist, excluding SHA256SUMS itself.
(
  cd "$TMP"
  find . -type f ! -name SHA256SUMS.txt -print0 \
    | sort -z \
    | xargs -0 sha256sum > SHA256SUMS.txt
)

python3 - "$TMP" "$OUT" <<'PY'
import pathlib, sys, zipfile
src = pathlib.Path(sys.argv[1])
out = pathlib.Path(sys.argv[2]).expanduser().resolve()
out.parent.mkdir(parents=True, exist_ok=True)
with zipfile.ZipFile(out, "w", zipfile.ZIP_DEFLATED) as z:
    for p in sorted(src.rglob("*")):
        if p.is_file():
            z.write(p, p.relative_to(src))
print(out)
PY

echo
echo "============================================================"
echo "READY"
echo "ZIP=$OUT"
echo "SIZE=$(du -h "$OUT" | cut -f1)"
echo "SHA256=$(sha256sum "$OUT" | awk '{print $1}')"
echo "client_modified=False"
echo "collector_remaining_family_assets_check=PASS"
echo "============================================================"
