#!/usr/bin/env bash
set -u

ROOT="${1:-.}"
cd "$ROOT" || exit 1

OUT="${2:-/tmp/summit_eragon_recovery_scan.txt}"
: > "$OUT"

say() {
    printf '%s\n' "$*" | tee -a "$OUT"
}

say "=== SUMMIT ERAGON / DEATH CAVE RECOVERY SCAN ==="
say "ROOT=$(pwd)"
say "DATE=$(date '+%Y-%m-%d %H:%M:%S %z')"
say

say "=== 1. FILENAMES ==="
find . -type f \
    -not -path './.git/*' \
    \( \
        -iname '*eragon*' -o \
        -iname '*dark_boss*' -o \
        -iname '*death*cave*' -o \
        -iname '*lavagolem*' -o \
        -iname '*lava*golem*' -o \
        -iname '*magmagolem*' -o \
        -iname '*magma*golem*' \
    \) \
    -print 2>/dev/null | sort | tee -a "$OUT"

say
say "=== 2. TEXT REFERENCES ==="
if command -v rg >/dev/null 2>&1; then
    rg -n -i \
      --hidden \
      --glob '!.git/**' \
      --glob '!*.o' --glob '!*.obj' --glob '!*.a' --glob '!*.dll' --glob '!*.exe' \
      --glob '!*.sfld' --glob '!*.scfg' --glob '!*.smap' --glob '!*.sloc' --glob '!*.sres' --glob '!*.sgcf' \
      'Eragon|Death[ _-]*Cave|dark_boss|Lava[ _-]*Golem|Magma[ _-]*Golem|lavagolem|magmagolem' \
      . 2>/dev/null | tee -a "$OUT"
else
    grep -RniI \
      --exclude-dir=.git \
      -E 'Eragon|Death[ _-]*Cave|dark_boss|Lava[ _-]*Golem|Magma[ _-]*Golem|lavagolem|magmagolem' \
      . 2>/dev/null | tee -a "$OUT"
fi

say
say "=== 3. LIKELY MONSTER DIRECTORIES ==="
find . -type d \
  -not -path './.git/*' \
  \( \
    -iname '*monster*' -o \
    -iname '*char*' -o \
    -iname '*model*' \
  \) \
  -print 2>/dev/null \
  | grep -Ei 'cliente|Servidor|SourceCode|backup|migration' \
  | sort -u \
  | head -400 \
  | tee -a "$OUT"

say
say "=== 4. POSSIBLE FIRE / DRAGON MODEL FILES ==="
find . -type f \
  -not -path './.git/*' \
  \( \
    -iname '*.ini' -o -iname '*.txt' -o -iname '*.inf' -o \
    -iname '*.ase' -o -iname '*.smd' -o -iname '*.smb' -o \
    -iname '*.bmp' -o -iname '*.tga' -o -iname '*.dds' \
  \) \
  \( \
    -iname '*dragon*' -o -iname '*drago*' -o \
    -iname '*lava*' -o -iname '*magma*' -o \
    -iname '*erag*' -o -iname '*death*' \
  \) \
  -print 2>/dev/null \
  | sort -u \
  | tee -a "$OUT"

say
say "=== 5. STRINGS INSIDE LIKELY CONFIG / MODEL FILES ==="
if command -v strings >/dev/null 2>&1; then
    find . -type f \
      -not -path './.git/*' \
      \( \
        -iname '*.ini' -o -iname '*.txt' -o -iname '*.inf' -o \
        -iname '*.ase' -o -iname '*.smd' -o -iname '*.smb' -o \
        -iname '*.dat' -o -iname '*.bin' \
      \) \
      \( \
        -path '*cliente*' -o \
        -path '*Servidor*' -o \
        -path '*SourceCode*' -o \
        -path '*.summit_*' \
      \) \
      -print0 2>/dev/null \
    | while IFS= read -r -d '' f; do
        hit="$(strings -a -n 5 "$f" 2>/dev/null | grep -Eim 8 'Eragon|Death[ _-]*Cave|dark_boss|Lava[ _-]*Golem|Magma[ _-]*Golem|lavagolem|magmagolem' || true)"
        if [ -n "$hit" ]; then
            printf '\nFILE=%s\n%s\n' "$f" "$hit"
        fi
      done | tee -a "$OUT"
fi

say
say "=== 6. EXISTING FIELD52 / VIP / QUEST REFERENCES ==="
if command -v rg >/dev/null 2>&1; then
    rg -n -i \
      --glob '!.git/**' \
      'WarpField2\(52\)|FieldCode[[:space:]]*==[[:space:]]*52|field[[:space:]]*==[[:space:]]*52|Lar do Eragon|Eragon Cave|QUESTID_T5|RANK5|Tier 5' \
      SourceCode 2>/dev/null | tee -a "$OUT"
fi

say
say "=== 7. CURRENT FIELD52 CANONICAL ==="
for f in \
  SourceCode/Content/canonical/stage_spawns/spawns/GameServer/Field/dark_boss.ase.spm.json \
  SourceCode/Content/canonical/stage_start_points/points/GameServer/Field/dark_boss.ASE.spp.json
do
    if [ -f "$f" ]; then
        say "--- $f"
        cat "$f" | tee -a "$OUT"
    fi
done

say
say "=== DONE ==="
say "REPORT=$OUT"
