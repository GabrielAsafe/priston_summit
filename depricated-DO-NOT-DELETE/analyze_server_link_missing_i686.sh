#!/usr/bin/env bash
set -u

echo "=== EXTRAINDO UNDEFINED SYMBOLS DO SERVIDOR ==="

grep -oE "undefined reference to \`[^']+'" link_server_i686.log \
| sed "s/undefined reference to \`//; s/'$//" \
| sort -u > server_undefined_symbols.txt

wc -l server_undefined_symbols.txt

echo
echo "=== TOP 200 SYMBOLS ==="
head -200 server_undefined_symbols.txt

echo
echo "=== PROCURANDO DEFINICOES EM SrcServer/src e SrcGame/src ==="

rm -f server_symbol_def_candidates.txt

while IFS= read -r sym; do
  clean="$(echo "$sym" | sed 's/(.*//; s/::.*//; s/^_//')"
  [ -z "$clean" ] && continue

  echo "===== $sym =====" >> server_symbol_def_candidates.txt

  grep -RIn \
    --include='*.cpp' --include='*.c' --include='*.h' --include='*.hpp' --include='*.CPP' \
    "$clean" SrcServer/src SrcGame/src 2>/dev/null \
    | grep -Eiv 'build_|dist_|lixo_|backup|Backup|\.bak|bak_|corrupt|LuaSrc/Lua/(lua|luac)|ltests' \
    | head -20 >> server_symbol_def_candidates.txt

done < server_undefined_symbols.txt

echo
echo "=== CANDIDATOS GERADOS ==="
wc -l server_symbol_def_candidates.txt

echo
echo "=== RESUMO POR ARQUIVO CANDIDATO ==="

grep -E '^Src(Game|Server)/src/' server_symbol_def_candidates.txt \
| cut -d: -f1 \
| sort \
| uniq -c \
| sort -nr \
| head -80 > server_symbol_candidate_files_ranked.txt

cat server_symbol_candidate_files_ranked.txt

echo
echo "Arquivos:"
echo "  server_undefined_symbols.txt"
echo "  server_symbol_def_candidates.txt"
echo "  server_symbol_candidate_files_ranked.txt"
