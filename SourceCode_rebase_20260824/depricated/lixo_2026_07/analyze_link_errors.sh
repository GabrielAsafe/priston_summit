#!/usr/bin/env bash
set -u

LOG="build_mingw_client_link.log"
OUTDIR="build_link_error_report"

rm -rf "$OUTDIR"
mkdir -p "$OUTDIR/groups"

if [ ! -f "$LOG" ]; then
    echo "Log não encontrado: $LOG"
    exit 1
fi

grep -niE "undefined reference|cannot find|multiple definition|ld returned|collect2|error:" "$LOG" \
  > "$OUTDIR/link_errors_clean.txt" || true

grep -oP "undefined reference to \`[^']+'" "$LOG" \
  | sed "s/undefined reference to \`//" \
  | sort | uniq -c | sort -nr \
  > "$OUTDIR/undefined_symbols.txt" || true

grep -ni "undefined reference" "$LOG" > "$OUTDIR/groups/undefined_reference.txt" || true
grep -ni "multiple definition" "$LOG" > "$OUTDIR/groups/multiple_definition.txt" || true
grep -ni "cannot find" "$LOG" > "$OUTDIR/groups/cannot_find_library.txt" || true

echo "Relatório criado em: $OUTDIR"
echo
echo "Resumo de símbolos undefined:"
cat "$OUTDIR/undefined_symbols.txt" | head -100
echo
echo "Primeiros erros:"
cat "$OUTDIR/link_errors_clean.txt" | head -160
