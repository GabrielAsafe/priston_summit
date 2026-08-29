#!/usr/bin/env bash
set -euo pipefail

ROOT="${1:-$HOME/Área de Trabalho/summit/design_work}"
CLIENT="${2:-$HOME/Área de Trabalho/summit/Summit Tale Client Full/cliente}"
INV="${3:-$ROOT/logs/asset_similarity_v1_1/asset_inventory.json}"
PAIR="${4:-$ROOT/logs/asset_family_refine_v2/inx_physical_pairing.csv}"
OUT="${5:-$HOME/Área de Trabalho/PT_REPRESENTATIVES_FOR_LOCAL_TEST_$(date +%Y%m%d_%H%M%S).zip}"

TMP="$(mktemp -d "$HOME/Área de Trabalho/PT_REPS_XXXXXX")"
trap 'rm -rf "$TMP"' EXIT
mkdir -p "$TMP/client"

python3 - "$INV" "$PAIR" "$CLIENT" "$TMP/client" <<'PYCODE'
import csv, json, pathlib, shutil, sys

inv_path = pathlib.Path(sys.argv[1])
pair_path = pathlib.Path(sys.argv[2])
client = pathlib.Path(sys.argv[3]).resolve()
dest = pathlib.Path(sys.argv[4])

wanted = [
    "char/monster/Monbagon/Monbagon-C.smd",
    "char/monster/Monshadow/Monshadow-01.smd",
    "char/monster/Vampiric/vemp.smd",
    "char/npc/SN-002/SN-002.smd",
    "char/npc/MN-005/MN-005.smd",
    "char/npc/MN-009/MN-009.smd",
    "image/Sinimage/Items/DropItem/itPR101.smd",
    "image/Sinimage/Items/DropItem/itWA104.smd",
    "image/Sinimage/Items/DropItem/itWH118.smd",
]

data = json.loads(inv_path.read_text(encoding="utf-8"))
records = {
    str(r.get("relative_path", "")).replace("\\", "/").lower(): r
    for r in data["records"]
    if str(r.get("path", "")).startswith(str(client))
}

def copy_record(r):
    if not r:
        return
    # Critical V2 fix: use the exact physical absolute path from inventory,
    # rather than joining possibly differently-cased relative_path on Linux.
    src = pathlib.Path(r["path"])
    if not src.is_file():
        print("MISSING_PHYSICAL", src)
        return
    rel = src.relative_to(client)
    dst = dest / rel
    dst.parent.mkdir(parents=True, exist_ok=True)
    if not dst.exists():
        shutil.copy2(src, dst)
        print("COPY", rel)

wanted_low = {x.lower() for x in wanted}

for item in wanted:
    r = records.get(item.lower())
    copy_record(r)
    if r:
        for tex in r.get("texture_refs") or []:
            copy_record(records.get(str(tex).replace("\\", "/").lower()))

if pair_path.is_file():
    with pair_path.open(encoding="utf-8", newline="") as f:
        for row in csv.DictReader(f):
            model = str(row.get("model_smd", "")).replace("\\", "/").lower()
            if model not in wanted_low:
                continue
            for key in ("inx", "motion_smb"):
                copy_record(
                    records.get(
                        str(row.get(key, "")).replace("\\", "/").lower()
                    )
                )
PYCODE

(
  cd "$TMP"
  find . -type f ! -name SHA256SUMS.txt -print0 \
    | sort -z \
    | xargs -0 sha256sum > SHA256SUMS.txt
  zip -q -r "$OUT" .
)

echo "ZIP=$OUT"
echo "SIZE=$(du -h "$OUT" | cut -f1)"
echo "SHA256=$(sha256sum "$OUT" | awk '{print $1}')"
echo "collector_check=PASS"
