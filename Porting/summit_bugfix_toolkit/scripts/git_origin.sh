#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'USAGE'
Uso:
  git_origin.sh STRING [PATH]

Executa duas buscas históricas:
1) git log -S (mudança na quantidade da string)
2) git log -G (linhas de diff que casam com regex)

PATH é opcional.

Exemplo:
  git_origin.sh S_Reduce SrcGame/src/sinbaram/HaPremiumItem.cpp
USAGE
}

[[ $# -ge 1 ]] || { usage >&2; exit 2; }
term=$1
path=${2:-}

printf '%s\n' '=== git log -S ==='
if [[ -n "$path" ]]; then
  git log --all --oneline -S"$term" -- "$path"
else
  git log --all --oneline -S"$term"
fi

printf '\n%s\n' '=== git log -G ==='
if [[ -n "$path" ]]; then
  git log --all --oneline -G"$term" -- "$path"
else
  git log --all --oneline -G"$term"
fi
