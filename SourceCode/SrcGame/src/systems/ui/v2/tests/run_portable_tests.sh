#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../../../../../.." && pwd)"
SRC="$ROOT/SrcGame/src"
CXX="${CXX:-g++}"
OUT="${TMPDIR:-/tmp}/priston_ui_v2_tests"
mkdir -p "$OUT"

COMMON=(
  "$SRC/systems/ui/v2/core/Widget.cpp"
  "$SRC/systems/ui/v2/core/Container.cpp"
  "$SRC/systems/ui/v2/core/Window.cpp"
  "$SRC/systems/ui/v2/core/UIRuntime.cpp"
  "$SRC/systems/ui/v2/input/UIFocusManager.cpp"
  "$SRC/systems/ui/v2/input/UIInputRouter.cpp"
  "$SRC/systems/ui/v2/input/UIPointerCapture.cpp"
  "$SRC/systems/ui/v2/windows/UIWindowManager.cpp"
  "$SRC/systems/ui/v2/windows/CharacterWindow.cpp"
  "$SRC/systems/ui/v2/widgets/Button.cpp"
  "$SRC/systems/ui/v2/widgets/ConfirmationOverlay.cpp"
  "$SRC/systems/ui/v2/widgets/NumericInputOverlay.cpp"
  "$SRC/systems/ui/v2/widgets/Label.cpp"
  "$SRC/systems/ui/v2/widgets/Panel.cpp"
  "$SRC/systems/ui/v2/widgets/ProgressBar.cpp"
  "$SRC/systems/ui/v2/widgets/Separator.cpp"
  "$SRC/systems/ui/v2/panels/ContextPanel.cpp"
  "$SRC/systems/ui/v2/pages/AttributesPage.cpp"
  "$SRC/systems/ui/v2/pages/SkillsPage.cpp"
  "$SRC/systems/ui/v2/pages/InventoryPage.cpp"
  "$SRC/systems/ui/v2/pages/CompanionsPage.cpp"
  "$SRC/systems/ui/v2/hud/PlayerHud.cpp"
  "$SRC/systems/ui/v2/hud/CompanionPartyHud.cpp"
  "$SRC/systems/ui/v2/quests/QuestJournalWindow.cpp"
  "$SRC/systems/ui/v2/quests/QuestTrackerHud.cpp"
)

FLAGS=(-std=c++17 -Wall -Wextra -pedantic -I "$SRC")

"$CXX" "${FLAGS[@]}" "${COMMON[@]}" \
  "$SRC/systems/ui/v2/tests/ui_v2_core_smoke.cpp" \
  -o "$OUT/ui_v2_core_smoke"
"$OUT/ui_v2_core_smoke"
echo "UI V2 core smoke: PASS"

"$CXX" "${FLAGS[@]}" "${COMMON[@]}" \
  "$SRC/systems/ui/v2/tests/ui_v2_v1_smoke.cpp" \
  -o "$OUT/ui_v2_v1_smoke"
"$OUT/ui_v2_v1_smoke"
echo "UI V2 v1 integration smoke: PASS"

"$CXX" "${FLAGS[@]}" "${COMMON[@]}" \
  "$SRC/systems/ui/v2/tests/ui_v2_quest_smoke.cpp" \
  -o "$OUT/ui_v2_quest_smoke"
"$OUT/ui_v2_quest_smoke"
echo "UI V2 quest journal/tracker smoke: PASS"

# Core/UI-domain code must stay independent from the legacy monolith and DX9.
if grep -R -n -E 'sinLinkHeader|<windows.h>|<d3d9.h>' \
    "$SRC/systems/ui/v2/core" \
    "$SRC/systems/ui/v2/input" \
    "$SRC/systems/ui/v2/widgets" \
    "$SRC/systems/ui/v2/windows" \
    "$SRC/systems/ui/v2/pages" \
    "$SRC/systems/ui/v2/panels" \
    "$SRC/systems/ui/v2/hud" \
    "$SRC/systems/ui/v2/quests" \
    "$SRC/systems/ui/v2/domain" \
    "$SRC/systems/ui/v2/viewmodels"; then
  echo "UI V2 dependency boundary: FAIL" >&2
  exit 1
fi

echo "UI V2 dependency boundary: PASS"

# Merchant V2 must own legacy shop visuals/input immediately, not only after
# CharacterWindow becomes visible on the following frame. Also guard the
# second legacy high-rank sell popup path in ThrowInvenItemToField.
BRIDGE="$SRC/systems/ui/v2/legacy/LegacyUIV2Bridge.cpp"
CONTEXT="$SRC/systems/ui/v2/legacy/LegacyContextGateway.cpp"
INVENTORY="$SRC/systems/inventory/legacy/sinInvenTory.cpp"

grep -Fq 'sinMoveKindInter[SIN_SHOP] > 0' "$BRIDGE"
grep -Fq 'sinMoveKindInter[SIN_CHARSHOP] > 0' "$BRIDGE"
grep -Fq 'sinMoveKindInter[SIN_MYSHOP] > 0' "$BRIDGE"
grep -Fq '(MigratedContextActive() && !RequiresLegacyInventoryUI())' "$BRIDGE"
grep -Fq '!ui::v2::legacy::OwnsLegacyShopUI())' "$INVENTORY"
grep -Fq 'if(ui::v2::legacy::OwnsLegacyShopUI())' "$INVENTORY"
grep -Fq 'sinMoveKindInter[SIN_CHARSHOP] = 0;' "$CONTEXT"

echo "UI V2 merchant ownership regression guards: PASS"


# v1.9 legacy-surface migration guards. Commerce/crafting contexts share
# cTrade.Draw(), while classic NPC/help/message flows must now be owned by
# generic V2 windows before their legacy render/input paths execute.
MAIN="$SRC/core/client/legacy_sin_runtime/sinMain.cpp"
INTERFACE="$SRC/systems/ui/hud/legacy_sin/sinInterFace.cpp"
MESSAGE="$SRC/systems/ui/messages/legacy_sin/sinMessageBox.cpp"
QUEST_GATEWAY="$SRC/systems/ui/v2/legacy/LegacyQuestGateway.cpp"

grep -Fq 'sinMoveKindInter[SIN_WAREHOUSE] > 0' "$BRIDGE"
grep -Fq 'sinMoveKindInter[SIN_CRAFTITEM] > 0' "$BRIDGE"
grep -Fq 'sinMoveKindInter[SIN_AGING] > 0' "$BRIDGE"
grep -Fq 'sinMoveKindInter[SIN_SKIN_CHANGE] > 0' "$BRIDGE"
grep -Fq 'if (!ui::v2::legacy::OwnsLegacyHelpUI())' "$MAIN"
grep -Fq 'if (!ui::v2::legacy::OwnsLegacyMessage3UI())' "$MAIN"
grep -Fq 'if (!ui::v2::legacy::SuppressLegacyMixGuide())' "$MAIN"
grep -Fq 'OwnsLegacySkillNpcUI()' "$INTERFACE"
grep -Fq 'ResolveMessage3ForUIV2' "$MESSAGE"
grep -Fq 'Quests classicas' "$QUEST_GATEWAY"
grep -Fq 'sinQuest_Level30' "$QUEST_GATEWAY"
grep -Fq 'HasActiveSupportedLegacyMessage()' "$BRIDGE"
grep -Fq 'HasActiveSupportedLegacyDialog()' "$BRIDGE"
grep -Fq 'if (SkillNpcFlag) return true;' "$BRIDGE"

echo "UI V2 legacy NPC/trade surface regression guards: PASS"

# v1.9.2 merchant/classic-job regression guards. Merchant selling must be
# immediate in V2 and accidental legacy right-clicks must never create the
# high-rank Message3. Change-job quests must be visible in the classic journal.
if grep -Fq 'Confirmar venda' "$CONTEXT"; then
  echo "UI V2 direct merchant sale regression guard: FAIL" >&2
  exit 1
fi
grep -Fq 'return AcceptInventoryItemResolvedOption(inventoryIndex, 1, false);' "$CONTEXT"
grep -Fq 'OwnsLegacyShopUI() && !uiV2ConfirmedMerchantSale' "$INVENTORY"
grep -Fq 'sinQuest_ChangeJob);' "$QUEST_GATEWAY"
grep -Fq 'sinQuest_ChangeJob2);' "$QUEST_GATEWAY"
grep -Fq 'sinQuest_ChangeJob3);' "$QUEST_GATEWAY"
grep -Fq 'sinQuest_ChangeJob4);' "$QUEST_GATEWAY"

echo "UI V2 direct merchant sale/change-job quest regression guards: PASS"
