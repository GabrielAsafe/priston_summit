#!/usr/bin/env bash
set -euo pipefail

ROOT=${1:-.}
OUT=${2:-audit2}
mkdir -p "$OUT"

GAME="$ROOT/SourceCode/SrcGame/src"
SERVER="$ROOT/SourceCode/SrcServer/src"

# Inventário físico canônico.
find "$GAME" -type f \( -name '*.c' -o -name '*.cc' -o -name '*.cpp' -o -name '*.cxx' -o -name '*.h' -o -name '*.hh' -o -name '*.hpp' \) -print | sort > "$OUT/files-game.txt"
find "$SERVER" -type f \( -name '*.c' -o -name '*.cc' -o -name '*.cpp' -o -name '*.cxx' -o -name '*.h' -o -name '*.hh' -o -name '*.hpp' \) -print | sort > "$OUT/files-server.txt"

# Includes com arquivo chamador + linha. -a evita perder fontes detectados como binários.
for spec in "game:$GAME" "server:$SERVER"; do
  side=${spec%%:*}
  tree=${spec#*:}
  grep -RnaH -a --include='*.c' --include='*.cc' --include='*.cpp' --include='*.cxx' --include='*.h' --include='*.hh' --include='*.hpp' \
    '^[[:space:]]*#include' "$tree" > "$OUT/includes-$side.txt" || true
done

# Resolve includes locais ("...") para caminhos reais quando possível.
# Procura primeiro relativo ao arquivo chamador; depois nas raízes src de Game/Server.
: > "$OUT/include-edges.tsv"
while IFS= read -r row; do
  src=${row%%:*}
  rest=${row#*:}; line=${rest%%:*}; text=${rest#*:}
  inc=$(printf '%s\n' "$text" | sed -nE 's/^[[:space:]]*#include[[:space:]]*"([^"]+)".*/\1/p')
  [ -n "$inc" ] || continue
  srcdir=$(dirname "$src")
  resolved=''
  for candidate in "$srcdir/$inc" "$GAME/$inc" "$SERVER/$inc" "$ROOT/SourceCode/Shared/$inc"; do
    if [ -e "$candidate" ]; then
      resolved=$(realpath -m --relative-to="$ROOT" "$candidate")
      break
    fi
  done
  [ -n "$resolved" ] || resolved="UNRESOLVED:$inc"
  printf '%s\t%s\t%s\t%s\n' "$src" "$line" "$inc" "$resolved" >> "$OUT/include-edges.tsv"
done < <(cat "$OUT/includes-game.txt" "$OUT/includes-server.txt")

# Hubs por caminho já resolvido.
cut -f4 "$OUT/include-edges.tsv" | sort | uniq -c | sort -nr > "$OUT/include-hubs.txt"

# OpCodes com origem.
grep -RnaH -aE --include='*.c' --include='*.cc' --include='*.cpp' --include='*.cxx' --include='*.h' --include='*.hh' --include='*.hpp' \
  '(OpCode::[A-Za-z0-9_]+|OPCODE_[A-Za-z0-9_]+)' "$GAME" "$SERVER" > "$OUT/opcodes.txt" || true

grep -oE '(OPCODE_[A-Za-z0-9_]+)' "$OUT/opcodes.txt" | sort -u > "$OUT/opcode-names.txt"

# Database IDs e usos.
grep -RnaH -aE --include='*.c' --include='*.cc' --include='*.cpp' --include='*.cxx' --include='*.h' --include='*.hh' --include='*.hpp' \
  'DATABASEID_[A-Za-z0-9_]+' "$SERVER" > "$OUT/databases.txt" || true

# Lifecycle.
for spec in "game:$GAME" "server:$SERVER"; do
  side=${spec%%:*}
  tree=${spec#*:}
  grep -RnaH -aE --include='*.c' --include='*.cc' --include='*.cpp' --include='*.cxx' --include='*.h' --include='*.hh' --include='*.hpp' \
    '\b(Init|Initialize|Start|Load|Create|Open|Run|Main|Update|Tick|Process|Draw|Close|Shutdown|Destroy)[[:space:]]*\(' \
    "$tree" > "$OUT/lifecycle-$side.txt" || true
done

# Possíveis arquivos de composição/startup.
grep -RnaH -aE --include='*.cpp' \
  '(openDatabase|WinMain[[:space:]]*\(|main[[:space:]]*\(|\.Init[[:space:]]*\(|->Init[[:space:]]*\()' \
  "$GAME" "$SERVER" > "$OUT/startup-candidates.txt" || true

printf 'Gerado em %s\n' "$OUT"
printf 'Próximo foco: include-hubs.txt, startup-candidates.txt, opcode-names.txt e arquivos de build.\n'
