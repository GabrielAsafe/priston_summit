#!/usr/bin/env bash
set -u

ROOT="$(pwd)"
OUT="$ROOT/include_check_report"
mkdir -p "$OUT"

SRC_DIRS=(
  "Shared"
  "SrcGame/src"
  "SrcServer/src"
  "modules"
)

INCLUDE_DIRS=(
  "."
  "Shared"
  "Shared2"
  "SrcGame/src"
  "SrcGame/src/Language"
  "SrcGame/src/sinbaram"
  "SrcGame/src/smLib3d"
  "SrcGame/src/HoBaram"
  "SrcGame/src/HoBaram/NewEffect"
  "SrcGame/src/HoBaram/NewEffect/Script"
  "SrcGame/src/Library/LuaSrc"
  "SrcGame/src/Security"
  "SrcGame/src/Settings"
  "SrcServer/src"
  "SrcServer/src/Language"
  "SrcServer/src/sinbaram"
  "SrcServer/src/smLib3d"
  "SrcServer/src/HoBaram"
  "modules"
  "modules/File"
  "modules/Helper"
  "modules/MD5"
  "Dependency/DirectX9/Include"
)

MINGW_INC="$(x86_64-w64-mingw32-g++ -xc++ -E -v /dev/null 2>&1 \
  | awk '/#include <...> search starts here:/{flag=1;next}/End of search list./{flag=0}flag' \
  | sed 's/^[[:space:]]*//')"

> "$OUT/all_includes_raw.txt"
> "$OUT/all_includes_unique.txt"
> "$OUT/include_ok.txt"
> "$OUT/include_missing.txt"
> "$OUT/include_case_mismatch.txt"

echo "Scanning includes..."

find "${SRC_DIRS[@]}" \
  -type f \( -iname "*.h" -o -iname "*.hpp" -o -iname "*.c" -o -iname "*.cpp" -o -iname "*.cc" -o -iname "*.cxx" \) \
  2>/dev/null \
| while IFS= read -r file; do
    grep -nE '^[[:space:]]*#[[:space:]]*include[[:space:]]*[<"][^">]+[">]' "$file" 2>/dev/null \
    | sed -E "s|^|$file:|"
  done > "$OUT/all_includes_raw.txt"

sed -E 's|^([^:]+):([0-9]+):[[:space:]]*#[[:space:]]*include[[:space:]]*[<"]([^">]+)[">].*$|\3|' \
  "$OUT/all_includes_raw.txt" \
| sort -u > "$OUT/all_includes_unique.txt"

check_one() {
  local inc="$1"
  local found=""
  local case_found=""

  for d in "${INCLUDE_DIRS[@]}"; do
    [[ -f "$ROOT/$d/$inc" ]] && found="$ROOT/$d/$inc" && break
  done

  if [[ -z "$found" ]]; then
    while IFS= read -r d; do
      [[ -n "$d" && -f "$d/$inc" ]] && found="$d/$inc" && break
    done <<< "$MINGW_INC"
  fi

  if [[ -n "$found" ]]; then
    echo "$inc -> $found" >> "$OUT/include_ok.txt"
    return
  fi

  local base
  base="$(basename "$inc")"

  for d in "${INCLUDE_DIRS[@]}"; do
    case_found="$(find "$ROOT/$d" -maxdepth 6 -type f -iname "$base" 2>/dev/null | head -n 1 || true)"
    [[ -n "$case_found" ]] && break
  done

  if [[ -z "$case_found" ]]; then
    while IFS= read -r d; do
      [[ -z "$d" ]] && continue
      case_found="$(find "$d" -maxdepth 4 -type f -iname "$base" 2>/dev/null | head -n 1 || true)"
      [[ -n "$case_found" ]] && break
    done <<< "$MINGW_INC"
  fi

  if [[ -n "$case_found" ]]; then
    echo "$inc -> CASE? $case_found" >> "$OUT/include_case_mismatch.txt"
  else
    echo "$inc" >> "$OUT/include_missing.txt"
  fi
}

while IFS= read -r inc; do
  [[ -z "$inc" ]] && continue
  check_one "$inc"
done < "$OUT/all_includes_unique.txt"

echo
echo "Done."
echo "Reports:"
echo "  $OUT/all_includes_raw.txt"
echo "  $OUT/all_includes_unique.txt"
echo "  $OUT/include_ok.txt"
echo "  $OUT/include_case_mismatch.txt"
echo "  $OUT/include_missing.txt"
echo
echo "Summary:"
echo "  total unique includes: $(wc -l < "$OUT/all_includes_unique.txt")"
echo "  ok:                    $(wc -l < "$OUT/include_ok.txt")"
echo "  case mismatch:         $(wc -l < "$OUT/include_case_mismatch.txt")"
echo "  missing:               $(wc -l < "$OUT/include_missing.txt")"
