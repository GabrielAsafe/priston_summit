#pragma once

// Semantic version of the character UI migration layer.
#define UI_V2_VERSION_MAJOR 1
#define UI_V2_VERSION_MINOR 10
#define UI_V2_VERSION_PATCH 0

// UI V2 release switches. The v1.3 character window/HUD are enabled by default;
// define UI_V2_CHARACTER_WINDOW=0 at compile time for an immediate rollback.
#ifndef UI_V2_CHARACTER_WINDOW
#define UI_V2_CHARACTER_WINDOW 1
#endif

// When enabled, the V2 character window replaces the visual/input layer for
// Attributes, Skills and Inventory. Commerce/crafting contexts migrated through
// v1.2 remain inside the Inventory tab through ContextPanel.
#ifndef UI_V2_SUPPRESS_LEGACY_CHARACTER_UI
#define UI_V2_SUPPRESS_LEGACY_CHARACTER_UI 1
#endif

// Developer-only diagnostic window from phase 1.
#ifndef UI_V2_SHOW_BOOTSTRAP_WINDOW
#define UI_V2_SHOW_BOOTSTRAP_WINDOW 0
#endif

// Permanent compact HUD replacing the old bottom chrome.
#ifndef UI_V2_PLAYER_HUD
#define UI_V2_PLAYER_HUD 1
#endif

// Hide the classic HP/MP/STM columns, chat, X/Q/D/S/V/C buttons and the
// map/camera/run controls while the V2 HUD is active.
#ifndef UI_V2_SUPPRESS_LEGACY_MAIN_HUD
#define UI_V2_SUPPRESS_LEGACY_MAIN_HUD 1
#endif

// Compact always-on companion combat HUD. Runtime state is delivered by a
// dedicated low-frequency packet and does not trigger DataServer reads.
#ifndef UI_V2_COMPANION_PARTY_HUD
#define UI_V2_COMPANION_PARTY_HUD 1
#endif

// Quest journal/tracker migration. The legacy cQuestWindow remains the
// network/rules backend while its presentation and pointer input are replaced.
#ifndef UI_V2_QUEST_JOURNAL
#define UI_V2_QUEST_JOURNAL 1
#endif

#ifndef UI_V2_SUPPRESS_LEGACY_QUEST_UI
#define UI_V2_SUPPRESS_LEGACY_QUEST_UI 1
#endif
