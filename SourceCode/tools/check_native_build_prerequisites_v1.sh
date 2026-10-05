#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
CXX="${CXX:-i686-w64-mingw32-g++}"
WINDRES="${WINDRES:-i686-w64-mingw32-windres}"
missing_tools=()
missing_paths=()
for tool in "$CXX" "$WINDRES" parallel sha256sum; do
    command -v "$tool" >/dev/null 2>&1 || missing_tools+=("$tool")
done
required_paths=(
    build_client_mingw_i686_multicore.sh
    build_server_mingw_i686_j8.sh
    link_client_mingw_i686.sh
    link_server_mingw_i686.sh
    SrcGame/src
    SrcServer/src
    Shared
    Shared2
    modules
    Dependency
    Dependency/DirectX9/Include
    "Dependency/DirectXSDK/Microsoft DirectX SDK (June 2010)/Include"
    SrcGame/src/platform/windows/resources/Script1.rc
    SrcGame/src/platform/windows/resources/resource.h
    SrcServer/src/platform/windows/resources/Script1.rc
    SrcServer/src/platform/windows/resources/resource.h
)
for path in "${required_paths[@]}"; do
    [[ -e "$path" ]] || missing_paths+=("$path")
done
if ((${#missing_tools[@]} || ${#missing_paths[@]})); then
    if ((${#missing_tools[@]})); then
        printf 'NATIVE_BUILD_BLOCKED: missing tools:' >&2
        printf ' %s' "${missing_tools[@]}" >&2
        printf '\n' >&2
    fi
    if ((${#missing_paths[@]})); then
        printf 'NATIVE_BUILD_BLOCKED: missing source/dependency paths:' >&2
        printf ' [%s]' "${missing_paths[@]}" >&2
        printf '\n' >&2
    fi
    exit 2
fi
printf 'NATIVE_BUILD_PREREQUISITES=PASS CXX=%s WINDRES=%s\n' "$(command -v "$CXX")" "$(command -v "$WINDRES")"
