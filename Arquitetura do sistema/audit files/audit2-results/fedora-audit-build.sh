#!/usr/bin/env bash
set -euo pipefail
ROOT="${1:-.}"
OUT="${2:-audit-build}"
mkdir -p "$OUT"

# Usa apenas utilitários GNU/comuns no Fedora.
# Objetivo: distinguir arquivo presente de arquivo efetivamente listado pelo build.

find "$ROOT" \
  \( -path '*/.git' -o -path '*/bugfixes' -o -path '*/depricated' -o -path '*/Dependency' \) -prune -o \
  -type f \( -name 'compile_commands.json' -o -name 'build.ninja' -o -name 'CMakeLists.txt' \
             -o -name 'Makefile' -o -name 'Makefile.*' -o -name '*.mk' \
             -o -name '*.vcxproj' -o -name '*.vcproj' -o -name '*.sln' \) -print \
  | sort > "$OUT/build-manifests.txt"

: > "$OUT/compiled-candidates.raw.txt"

# compile_commands.json: formato previsível para o campo "file".
while IFS= read -r f; do
  case "$f" in
    *compile_commands.json)
      grep -aoE '"file"[[:space:]]*:[[:space:]]*"[^"]+"' "$f" 2>/dev/null \
        | sed -E 's/^.*"file"[[:space:]]*:[[:space:]]*"([^"]+)".*$/\1/' \
        >> "$OUT/compiled-candidates.raw.txt" || true
      ;;
    *.vcxproj|*.vcproj)
      # Visual Studio: <ClCompile Include="...cpp" />
      grep -aoE '<ClCompile[^>]*Include="[^"]+"' "$f" 2>/dev/null \
        | sed -E 's/^.*Include="([^"]+)".*$/\1/' \
        | tr '\\' '/' >> "$OUT/compiled-candidates.raw.txt" || true
      ;;
    build.ninja|Makefile|Makefile.*|*.mk|CMakeLists.txt)
      # Heurística: extrai tokens que parecem TU C/C++.
      grep -aoE '([^[:space:]"'"'"'(){};:=]+/)*[^[:space:]"'"'"'(){};:=]+\.(c|cc|cpp|cxx)' "$f" 2>/dev/null \
        | tr '\\' '/' >> "$OUT/compiled-candidates.raw.txt" || true
      ;;
  esac
done < "$OUT/build-manifests.txt"

sed -E 's#^\./##; s#/+#/#g' "$OUT/compiled-candidates.raw.txt" \
  | sed '/^[[:space:]]*$/d' | sort -u > "$OUT/compiled-candidates.txt"

# Classifica referências por lado quando o caminho já contém SrcGame/SrcServer.
grep -E '(^|/)SrcGame(/|$)' "$OUT/compiled-candidates.txt" | sort -u > "$OUT/compiled-game.txt" || true
grep -E '(^|/)SrcServer(/|$)' "$OUT/compiled-candidates.txt" | sort -u > "$OUT/compiled-server.txt" || true

# Também encontra binários/objetos já existentes: ótima evidência caso o build tenha sido executado.
find "$ROOT" \
  \( -path '*/.git' -o -path '*/bugfixes' -o -path '*/depricated' -o -path '*/Dependency' \) -prune -o \
  -type f \( -name '*.o' -o -name '*.obj' -o -name '*.a' -o -name '*.so' -o -name '*.dll' -o -name '*.exe' \) -print \
  | sort > "$OUT/build-artifacts.txt"

{
  echo "manifests: $(wc -l < "$OUT/build-manifests.txt")"
  echo "compiled candidates: $(wc -l < "$OUT/compiled-candidates.txt")"
  echo "explicit SrcGame candidates: $(wc -l < "$OUT/compiled-game.txt")"
  echo "explicit SrcServer candidates: $(wc -l < "$OUT/compiled-server.txt")"
  echo "existing build artifacts: $(wc -l < "$OUT/build-artifacts.txt")"
} > "$OUT/summary.txt"

cat "$OUT/summary.txt"
