#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CXX="${CXX:-g++}"
TMP_ROOT="${TMPDIR:-/tmp}/takeoff_progression_sqlite_tu"
FAKE_WIN="$TMP_ROOT/fake_windows"
mkdir -p "$FAKE_WIN" "$TMP_ROOT/obj"
cat > "$FAKE_WIN/windows.h" <<'EOF'
#pragma once
typedef int BOOL;
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
struct SYSTEMTIME { unsigned short wYear,wMonth,wDayOfWeek,wDay,wHour,wMinute,wSecond,wMilliseconds; };
EOF

COMMON=(-std=gnu++14 -Wall -Wextra -Werror -pedantic -I"$FAKE_WIN" -I"$ROOT/Shared" -I"$ROOT/SrcServer/src")

for source in \
    "$ROOT/SrcServer/src/systems/gameplay/covenants/v2/SQLiteCovenantRepository.cpp" \
    "$ROOT/SrcServer/src/systems/gameplay/covenants/v2/SQLiteCovenantRuntimeRepository.cpp" \
    "$ROOT/SrcServer/src/systems/gameplay/reputation/v2/SQLiteReputationRepository.cpp" \
    "$ROOT/SrcServer/src/systems/gameplay/story/v2/SQLiteStoryRepository.cpp" \
    "$ROOT/SrcServer/src/systems/gameplay/progression/v2/SQLiteProgressionContentLoader.cpp"
do
    object="$TMP_ROOT/obj/$(basename "$source").o"
    "$CXX" "${COMMON[@]}" -c "$source" -o "$object"
done

echo "progression SQLite translation units: PASS"
