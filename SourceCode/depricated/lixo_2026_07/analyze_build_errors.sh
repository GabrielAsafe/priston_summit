#!/usr/bin/env bash
set -e

LOG="${1:-build_mingw_client_compile.log}"
OUTDIR="build_error_report"

rm -rf "$OUTDIR"
mkdir -p "$OUTDIR/groups"

grep -niE "error:|fatal error:" "$LOG" > "$OUTDIR/errors_raw.txt" || true

# Remove duplicados exatos
sort -u "$OUTDIR/errors_raw.txt" > "$OUTDIR/errors_unique.txt"

# Remove número da linha do log, mantendo arquivo:linha:erro
sed -E 's/^[0-9]+://' "$OUTDIR/errors_unique.txt" > "$OUTDIR/errors_clean.txt"

# Agrupar por arquivo fonte/header
awk -F: '
{
  file=$1
  gsub("/", "_", file)
  print $0 >> "build_error_report/groups/" file ".txt"
}
' "$OUTDIR/errors_clean.txt"

# Resumo por arquivo
awk -F: '{count[$1]++} END {for (f in count) print count[f], f}' "$OUTDIR/errors_clean.txt" \
  | sort -nr > "$OUTDIR/summary_by_file.txt"

# Resumo por tipo de erro
sed -E 's/^[^:]+:[0-9]+:[0-9]+: error: //' "$OUTDIR/errors_clean.txt" \
  | sed -E "s/'[^']+'/'X'/g" \
  | sort \
  | uniq -c \
  | sort -nr > "$OUTDIR/summary_by_error.txt"

# Grupos manuais úteis
grep -Ei "OpCode|OPCODE_|PACKET_|UPDATE_BATTLE_POINTS|DELETE_ITEM_PREMIUM" "$OUTDIR/errors_clean.txt" > "$OUTDIR/groups/opcodes.txt" || true
grep -Ei "getAccount|getName|rsPLAYINFO" "$OUTDIR/errors_clean.txt" > "$OUTDIR/groups/rsplayinfo.txt" || true
grep -Ei "Lua|lua\.h|lauxlib|lobject|lstate|lzio|ltm" "$OUTDIR/errors_clean.txt" > "$OUTDIR/groups/lua.txt" || true
grep -Ei "SQL|SQLLEN|SQLULEN|ODBC" "$OUTDIR/errors_clean.txt" > "$OUTDIR/groups/sql.txt" || true
grep -Ei "D3D|D3DX|Direct|ddraw|lpD3D|D3DRENDERSTATE" "$OUTDIR/errors_clean.txt" > "$OUTDIR/groups/directx.txt" || true
grep -Ei "LOG_ERROR|Utils_Log|Log::" "$OUTDIR/errors_clean.txt" > "$OUTDIR/groups/logging.txt" || true
grep -Ei "min|max|std::min|std::max" "$OUTDIR/errors_clean.txt" > "$OUTDIR/groups/minmax.txt" || true

echo "Relatório criado em: $OUTDIR"
echo
echo "Resumo por arquivo:"
head -40 "$OUTDIR/summary_by_file.txt"
echo
echo "Resumo por tipo:"
head -40 "$OUTDIR/summary_by_error.txt"


cat > "$OUTDIR/priority_report.txt" <<'EOF'
# Prioridade sugerida

## 1. DirectX / D3DX
Ver groups/directx.txt

## 2. rsPLAYINFO / PrimeItem
Ver groups/rsplayinfo.txt

## 3. Logging
Ver groups/logging.txt

## 4. min/max
Ver groups/minmax.txt

## 5. OpCodes ignorados por agora
Ver groups/opcodes.txt

## 6. Erros por arquivo
Ver summary_by_file.txt
EOF

echo
echo "Prioridade sugerida:"
cat "$OUTDIR/priority_report.txt"
