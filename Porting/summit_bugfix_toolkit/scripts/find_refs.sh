#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'USAGE'
Uso:
  find_refs.sh STRING [ROOT]

Busca uma string literalmente (-F) em C/C++ e mostra linha.
Boa opção quando STRING contém [, ], *, + ou outros caracteres de regex.

Exemplo:
  find_refs.sh 'UpKeepItemTGAImage[12]' SrcGame/src
USAGE
}

[[ $# -ge 1 ]] || { usage >&2; exit 2; }
term=$1
root=${2:-.}

grep -RniF --binary-files=without-match \
  --include='*.c' --include='*.cpp' --include='*.h' --include='*.hpp' \
  --exclude-dir=.git -- "$term" "$root" 2>/dev/null || true
