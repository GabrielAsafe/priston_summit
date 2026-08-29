#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'USAGE'
Uso:
  compare_assets.sh DIR_A DIR_B

Compara apenas os nomes dos arquivos diretamente dentro de dois diretórios.
Saída do comm -3:
- sem tabulação: somente em DIR_A
- com tabulação: somente em DIR_B

Exemplo:
  compare_assets.sh ExMachina/client/image/Sinimage/Premium pristontale/bin/Client/image/Sinimage/Premium
USAGE
}

[[ $# -eq 2 ]] || { usage >&2; exit 2; }
a=$1
b=$2

comm -3 \
  <(find "$a" -maxdepth 1 -type f -printf '%f\n' | LC_ALL=C sort) \
  <(find "$b" -maxdepth 1 -type f -printf '%f\n' | LC_ALL=C sort)
