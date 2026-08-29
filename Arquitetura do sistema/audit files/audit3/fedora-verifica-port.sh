#!/usr/bin/env bash
set -euo pipefail

ROOT="${1:-.}"
PDB="${2:-Game.pdb}"
EXE="${3:-dist_mingw_client_i686/Game.exe}"
OUT="${4:-audit-vs-port}"
mkdir -p "$OUT"

printf '== PDB histórico ==\n' > "$OUT/resumo.txt"
if [[ -f "$PDB" ]]; then
  file "$PDB" >> "$OUT/resumo.txt"
  strings -a "$PDB" | grep -E 'SrcGame\\src\\Debug\\[^\\]+\.obj' \
    | sed -E 's#.*SrcGame\\src\\Debug\\([^\\]+\.obj).*#\1#' \
    | sort -fu > "$OUT/pdb-objects.txt" || true
  strings -a "$PDB" | grep -E '/BASE:|/DYNAMICBASE:|/FIXED|/SAFESEH:' \
    | sort -u > "$OUT/pdb-link-flags.txt" || true
  strings -a "$PDB" | grep -Ei '(^|[\\/])(Lua|Lualib|atls|winspool|wsock32)\.lib' \
    | sort -u > "$OUT/pdb-interesting-libs.txt" || true
fi

printf '\n== Fontes atuais suspeitas ==\n' >> "$OUT/resumo.txt"
for f in \
  SrcGame/src/client_server_stubs.cpp \
  SrcGame/src/sinbaram/skill_missing_globals_mingw.cpp \
  SrcGame/src/Server/ClientFuncPos.cpp \
  SrcGame/src/Server/OnSever.cpp \
  SrcGame/src/Server/PartySever.cpp \
  SrcGame/src/Server/Svr_Damge.cpp \
  SrcGame/src/Server/svrEventSpawn.cpp \
  modules/Helper/Utils/utils_exception.cpp \
  modules/Helper/Utils/utils_filesystem.cpp \
  modules/Helper/Utils/utils_logging.cpp \
  modules/Helper/Utils/utils_string.cpp \
  modules/Helper/Utils/utils_testing.cpp
 do
  if [[ -f "$ROOT/$f" ]]; then echo "PRESENT $f"; else echo "MISSING $f"; fi
 done | tee "$OUT/suspect-files.txt" >> "$OUT/resumo.txt"

# Procura sinais de stubs/no-op e condicionais criadas durante o port.
grep -RnaE --include='*.cpp' --include='*.h' \
  '(client_server_stubs|skill_missing_globals_mingw|TODO|FIXME|stub|no.?op|#if.*_CLIENT|#if.*_SERVER|#ifdef[[:space:]]+_MBCS)' \
  "$ROOT/SrcGame/src" "$ROOT/modules" 2>/dev/null \
  > "$OUT/compat-and-stubs.txt" || true

# Endereços absolutos potencialmente relacionados ao image base antigo.
grep -RnaE --include='*.cpp' --include='*.h' \
  '0x00[0-9A-Fa-f]{6}|0x0[0-9A-Fa-f]{7}' \
  "$ROOT/SrcGame/src/Hook" "$ROOT/SrcGame/src/Security" "$ROOT/SrcGame/src/AntiCheat" \
  "$ROOT/SrcGame/src/TJBOY/Ygy" "$ROOT/SrcGame/src" 2>/dev/null \
  > "$OUT/absolute-addresses.txt" || true

if [[ -f "$ROOT/$EXE" ]]; then
  OBJDUMP="${OBJDUMP:-i686-w64-mingw32-objdump}"
  if command -v "$OBJDUMP" >/dev/null 2>&1; then
    "$OBJDUMP" -x "$ROOT/$EXE" > "$OUT/current-game-objdump.txt"
    grep -Ei 'ImageBase|DllCharacteristics|DYNAMIC_BASE|HIGH_ENTROPY|reloc' \
      "$OUT/current-game-objdump.txt" > "$OUT/current-image-layout.txt" || true
  fi
fi

printf '\nRelatórios em %s\n' "$OUT"
