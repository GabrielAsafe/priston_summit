#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
python3 "$ROOT/tools/validate_personal_shop_session_v2_authority.py"
python3 "$ROOT/tools/validate_personal_shop_session_boundary_v1.py"
TMP="$(mktemp -d)"; trap 'rm -rf "$TMP"' EXIT
"${CXX:-g++}" -std=gnu++14 -Wall -Wextra -Werror -pedantic -I"$ROOT/Shared" \
  "$ROOT/Shared/gameplay/commerce/v2/tests/personal_shop_session_smoke.cpp" \
  -o "$TMP/personal_shop_session_smoke"
"$TMP/personal_shop_session_smoke"
echo "Personal shop session v2 authority: PASS"
