#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'USAGE'
Uso:
  context.sh STRING FILE [BEFORE] [AFTER]

Mostra matches com contexto independente antes/depois.
Padrão: BEFORE=10 AFTER=20.
Força tratamento como texto (-a), útil em source antigo/encoding legado.

Exemplo:
  context.sh 'S_Reduce|M_Reduce' SrcGame/src/sinbaram/HaPremiumItem.cpp 5 12
USAGE
}

[[ $# -ge 2 ]] || { usage >&2; exit 2; }
pattern=$1
file=$2
before=${3:-10}
after=${4:-20}

grep -a -n -E "-B${before}" "-A${after}" -- "$pattern" "$file" || true
