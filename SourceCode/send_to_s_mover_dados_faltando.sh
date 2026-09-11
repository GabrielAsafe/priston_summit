#!/usr/bin/env bash
set -euo pipefail

SELF_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SELF_DIR/.." && pwd)"

FEDORA_SERVER="${SUMMIT_SERVER_ROOT:-$ROOT_DIR/Servidor}"
BUILD_DIR="${SUMMIT_SERVER_BUILD_DIR:-$SELF_DIR/dist_mingw_server_i686}"
MANIFEST="${SUMMIT_SERVER_DEPLOY_MANIFEST:-$SELF_DIR/server_deploy_extra.txt}"

SSH_USER_HOST="${SUMMIT_VM_SSH:-server@127.0.0.1}"
SSH_PORT="${SUMMIT_VM_PORT:-2223}"
WINDOWS_SERVER_ROOT="${SUMMIT_VM_SERVER_ROOT:-C:/SummitTale/Servidor}"

MODE="all"
DRY_RUN=0

usage() {
  cat <<'EOF'
Uso:
  ./send_to_s.sh [--all|--runtime-only|--binary-only] [--dry-run]

Modos:
  --all          envia Server.exe/DLLs + runtime extra do manifesto (default)
  --runtime-only envia somente arquivos/diretórios extras
  --binary-only  envia somente Server.exe e DLLs
  --dry-run      mostra os comandos sem executar
EOF
}

while (($#)); do
  case "$1" in
    --all) MODE="all" ;;
    --runtime-only) MODE="runtime-only" ;;
    --binary-only) MODE="binary-only" ;;
    --dry-run) DRY_RUN=1 ;;
    -h|--help) usage; exit 0 ;;
    *) echo "ERRO: argumento desconhecido: $1" >&2; usage >&2; exit 2 ;;
  esac
  shift
done

log() { printf '%s\n' "$*"; }

run_cmd() {
  if (( DRY_RUN )); then
    printf '[dry-run]'
    printf ' %q' "$@"
    printf '\n'
  else
    "$@"
  fi
}

# Gera um EncodedCommand PowerShell (UTF-16LE + base64).
# Assim nenhum pipe/aspas é reinterpretado pelo cmd.exe usado pelo OpenSSH.
encode_ps() {
  local script="$1"
  python3 - "$script" <<'PY'
import base64, sys
s = sys.argv[1]
print(base64.b64encode(s.encode("utf-16le")).decode("ascii"))
PY
}

remote_mkdir() {
  local win_dir="$1"
  local escaped="${win_dir//\'/\'\'}"
  local ps="[System.IO.Directory]::CreateDirectory('$escaped') > \$null"
  local enc
  enc="$(encode_ps "$ps")"
  run_cmd ssh -p "$SSH_PORT" "$SSH_USER_HOST" \
    powershell.exe -NoProfile -NonInteractive -EncodedCommand "$enc"
}

remote_parent() {
  local rel="$1"
  local parent="${rel%/*}"
  if [[ "$parent" == "$rel" ]]; then
    printf '%s\n' "$WINDOWS_SERVER_ROOT"
  else
    printf '%s/%s\n' "$WINDOWS_SERVER_ROOT" "$parent"
  fi
}

deploy_binary() {
  mkdir -p "$BUILD_DIR"

  local mingw_bin="/usr/i686-w64-mingw32/sys-root/mingw/bin"
  local dll
  for dll in libgcc_s_dw2-1.dll libstdc++-6.dll libwinpthread-1.dll; do
    if [[ -f "$mingw_bin/$dll" ]]; then
      cp -f "$mingw_bin/$dll" "$BUILD_DIR/$dll"
    fi
  done

  local required=(Server.exe libgcc_s_dw2-1.dll libstdc++-6.dll libwinpthread-1.dll)
  local missing=0
  for f in "${required[@]}"; do
    if [[ ! -f "$BUILD_DIR/$f" ]]; then
      echo "ERRO: build ausente: $BUILD_DIR/$f" >&2
      missing=1
    fi
  done
  (( missing == 0 )) || return 1

  log "[deploy] binário/DLLs -> $SSH_USER_HOST:$WINDOWS_SERVER_ROOT"
  remote_mkdir "$WINDOWS_SERVER_ROOT"

  local f
  for f in "${required[@]}"; do
    run_cmd scp -P "$SSH_PORT" "$BUILD_DIR/$f" \
      "$SSH_USER_HOST:$WINDOWS_SERVER_ROOT/$f"
  done
}

deploy_runtime_entry() {
  local rel="$1"
  [[ -n "$rel" ]] || return 0
  [[ "$rel" != \#* ]] || return 0

  local local_path="$FEDORA_SERVER/$rel"
  local parent

  if [[ -f "$local_path" ]]; then
    parent="$(remote_parent "$rel")"
    remote_mkdir "$parent"
    log "[deploy] arquivo: $rel"
    run_cmd scp -P "$SSH_PORT" "$local_path" \
      "$SSH_USER_HOST:$WINDOWS_SERVER_ROOT/$rel"
    return 0
  fi

  local dir_path="${local_path%/}"
  if [[ -d "$dir_path" ]]; then
    local rel_no_slash="${rel%/}"
    local remote_dir_parent
    remote_dir_parent="$(remote_parent "$rel_no_slash")"
    remote_mkdir "$remote_dir_parent"
    log "[deploy] diretório: $rel_no_slash/"
    run_cmd scp -P "$SSH_PORT" -r "$dir_path" \
      "$SSH_USER_HOST:$remote_dir_parent/"
    return 0
  fi

  echo "AVISO: runtime não encontrado no Fedora: $local_path" >&2
}

deploy_runtime() {
  if [[ ! -f "$MANIFEST" ]]; then
    echo "ERRO: manifesto não encontrado: $MANIFEST" >&2
    return 1
  fi

  log "[deploy] runtime Fedora: $FEDORA_SERVER"
  log "[deploy] runtime Windows: $SSH_USER_HOST:$WINDOWS_SERVER_ROOT"

  # Deduplica preservando a primeira ocorrência e ignorando linhas vazias/comentários.
  local tmp
  tmp="$(mktemp)"
  trap 'rm -f "$tmp"' RETURN
  awk '
    {
      sub(/\r$/, "")
      gsub(/^[[:space:]]+|[[:space:]]+$/, "")
    }
    NF && $0 !~ /^#/ && !seen[$0]++ { print }
  ' "$MANIFEST" > "$tmp"

  while IFS= read -r line || [[ -n "$line" ]]; do
    deploy_runtime_entry "$line"
  done < "$tmp"

  rm -f "$tmp"
  trap - RETURN
}

log "[deploy] SourceCode=$SELF_DIR"
log "[deploy] Servidor Fedora=$FEDORA_SERVER"
log "[deploy] Servidor VM=$SSH_USER_HOST:$WINDOWS_SERVER_ROOT"

case "$MODE" in
  all)
    deploy_binary
    deploy_runtime
    ;;
  runtime-only)
    deploy_runtime
    ;;
  binary-only)
    deploy_binary
    ;;
esac

log "[deploy] deploy concluído"
