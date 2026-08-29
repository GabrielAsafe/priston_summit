#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'USAGE'
Uso:
  probe_asset.sh FILE [FILE ...]

Mostra formato, tamanho/hash e, quando disponível, informações do ImageMagick.
Não modifica os arquivos.
USAGE
}

[[ $# -ge 1 ]] || { usage >&2; exit 2; }

for f in "$@"; do
  echo "===== $f ====="
  if [[ ! -e "$f" ]]; then
    echo "MISSING"
    echo
    continue
  fi
  file -- "$f"
  stat -c 'size=%s bytes | mode=%A | mtime=%y' -- "$f" 2>/dev/null || true
  sha256sum -- "$f"
  if command -v identify >/dev/null 2>&1; then
    identify -- "$f" 2>/dev/null || true
  elif command -v magick >/dev/null 2>&1; then
    magick identify -- "$f" 2>/dev/null || true
  fi
  echo
 done
