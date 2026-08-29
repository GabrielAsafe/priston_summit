#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'USAGE'
Uso:
  new_bugfix.sh NNN nome_curto [BUGFIX_ROOT]

Cria o esqueleto padrão do bugfix e grava o commit base.
BUGFIX_ROOT padrão: bugfixes

Exemplo:
  new_bugfix.sh 044 stamina_reduce_missing_icon_asset
USAGE
}

[[ $# -ge 2 ]] || { usage >&2; exit 2; }
num=$1
name=$2
root=${3:-bugfixes}
dir="$root/${num}_${name}"

if [[ -e "$dir" ]]; then
  echo "Já existe: $dir" >&2
  exit 1
fi

mkdir -p "$dir/before" "$dir/after"
git rev-parse HEAD > "$dir/base_commit.txt"
cat > "$dir/README.md" <<EOF2
# ${num} - ${name}

## Sintoma

## Escopo

## Hipóteses investigadas

## Causa provada

## Evidências

## Fix

## Proveniência

## Validação

## Resultado
EOF2

printf 'Criado: %s\n' "$dir"
