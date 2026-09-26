#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$SCRIPT_DIR"

# Sobe até encontrar a verdadeira raiz SourceCode. Não concatena "SourceCode" duas vezes.
while [[ "$ROOT" != "/" ]]; do
  if [[ -d "$ROOT/SrcGame/src" && -d "$ROOT/Shared" ]]; then
    break
  fi
  if [[ -d "$ROOT/SourceCode/SrcGame/src" && -d "$ROOT/SourceCode/Shared" ]]; then
    ROOT="$ROOT/SourceCode"
    break
  fi
  ROOT="$(dirname "$ROOT")"
done

if [[ ! -d "$ROOT/SrcGame/src" || ! -d "$ROOT/Shared" ]]; then
  echo "ERROR: nao foi possivel localizar a raiz SourceCode a partir de $SCRIPT_DIR" >&2
  exit 2
fi

SRC="$ROOT/SrcGame/src"
OUT="${TMPDIR:-/tmp}/priston_covenant_ui_tests"
CXX="${CXX:-g++}"
mkdir -p "$OUT"

"$CXX" -std=c++17 -Wall -Wextra -pedantic -I "$SRC" \
  "$SRC/systems/ui/v2/core/Widget.cpp" \
  "$SRC/systems/ui/v2/core/Container.cpp" \
  "$SRC/systems/ui/v2/widgets/Button.cpp" \
  "$SRC/systems/ui/v2/pages/CovenantsPage.cpp" \
  "$SRC/systems/ui/v2/tests/ui_v2_covenant_smoke.cpp" \
  -o "$OUT/ui_v2_covenant_smoke"
"$OUT/ui_v2_covenant_smoke"
echo "UI V2 Covenant smoke: PASS"

cat > "$OUT/protocol_size.cpp" <<'CPP'
#include "gameplay/covenants/v2/CovenantUIProtocol.h"
int main(){ return sizeof(gameplay::covenants::protocol::CovenantUIStatePacket) < 4096 ? 0 : 1; }
CPP
"$CXX" -std=c++17 -I "$ROOT/Shared" "$OUT/protocol_size.cpp" -o "$OUT/protocol_size"
"$OUT/protocol_size"
echo "Covenant UI protocol size: PASS"

CLIENT_NET="$SRC/systems/network/client/netplay.cpp"
SERVER_NET="$ROOT/Shared/server/legacy_runtime/OnSever_legacy_server.inl"
BRIDGE="$SRC/systems/ui/v2/legacy/LegacyUIV2Bridge.cpp"

grep -Fq 'OPCODE_COVENANT_UI_STATE' "$CLIENT_NET"
grep -Fq 'ApplyLegacyCovenantStatePacket' "$CLIENT_NET"
grep -Fq 'OPCODE_COVENANT_UI_ACTION' "$SERVER_NET"
grep -Fq 'HandleCovenantUIAction' "$SERVER_NET"
grep -Fq "sinGetKeyClick('N')" "$BRIDGE"
grep -Fq 'CharacterTab::Covenants' "$BRIDGE"
echo "Covenant UI network/hotkey wiring: PASS"
