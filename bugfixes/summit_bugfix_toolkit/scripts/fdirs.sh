#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'USAGE'
Uso:
  fdirs.sh STRING [ROOT]

Procura diretórios cujo nome contém STRING, ignorando caixa.
Útil para primeiro localizar alvos em HDs grandes e depois restringir a busca.

Exemplo:
  fdirs.sh Premium /home/gabriel/armazem/priston
USAGE
}

[[ $# -ge 1 ]] || { usage >&2; exit 2; }
term=$1
root=${2:-.}
find "$root" -type d -iname "*${term}*" -print 2>/dev/null || true
