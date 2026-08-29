#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'USAGE'
Uso:
  sgrep.sh STRING [ROOT] [CONTEXT]

Busca STRING em arquivos de source comuns.
- STRING pode ser regex estendida.
- ROOT padrão: .
- CONTEXT padrão: 0. Se > 0, usa -C.

Exemplos:
  sgrep.sh 'STAMINA_REDUCE_P|S_Reduce' SrcGame/src
  sgrep.sh 'MatIcon' SrcGame/src 8
USAGE
}

[[ $# -ge 1 ]] || { usage >&2; exit 2; }
pattern=$1
root=${2:-.}
context=${3:-0}

args=(-RniE --binary-files=without-match
  --include='*.c' --include='*.cc' --include='*.cpp' --include='*.cxx'
  --include='*.h' --include='*.hh' --include='*.hpp'
  --include='*.ini' --include='*.txt'
  --exclude-dir=.git
  --exclude-dir=build
  --exclude-dir=Debug
  --exclude-dir=Release)

if [[ "$context" =~ ^[0-9]+$ ]] && (( context > 0 )); then
  args+=("-C${context}")
fi

grep "${args[@]}" -- "$pattern" "$root" 2>/dev/null || true
