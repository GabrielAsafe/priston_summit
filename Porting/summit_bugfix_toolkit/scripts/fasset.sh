#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'USAGE'
Uso:
  fasset.sh STRING [ROOT] [MAXDEPTH]

Procura STRING no nome de arquivos, case-insensitive.
STRING é tratado como fragmento literal do nome.
ROOT padrão: .
MAXDEPTH é opcional.

Exemplos:
  fasset.sh S_Reduce /home/gabriel/armazem/priston
  fasset.sh reduce ./image/Sinimage/Premium 1
USAGE
}

[[ $# -ge 1 ]] || { usage >&2; exit 2; }
term=$1
root=${2:-.}
maxdepth=${3:-}

args=("$root")
if [[ -n "$maxdepth" ]]; then
  args+=(-maxdepth "$maxdepth")
fi
args+=(-type f -iname "*${term}*" -print)
find "${args[@]}" 2>/dev/null || true
