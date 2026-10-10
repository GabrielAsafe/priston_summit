#pragma once

namespace ui::v2::legacy
{
void Initialize();
void Shutdown();
void Update();
void Render();
bool HandleLegacyMessage(int message);
bool HandleMouseWheel(int delta);
bool HandleCharacterHotkeys();
bool HandleSkillShortcutHotkeys();
bool HandleQuestHotkey();
bool HandleMainHudHotkeys();
// SUMMIT_PATCH_214_GM_WARP_LIST_V2_DECL
void ToggleGMWarpListWindow();
void ToggleSettingsWindow();
bool OpenGuideWindow();
bool OpenSocialWindow();
bool OpenFriendsWindow();
bool OpenQuestJournalWindow();
[[nodiscard]] bool IsSettingsWindowOpen();
bool HandleModalHotkeys();
void OnResolutionChanged(int width, int height);

// Used by the legacy loop to avoid rendering/dispatching the old window below
// a V2 replacement. Inventory can intentionally return false in contextual
// shop/trade/crafting flows until those windows are migrated as well.
[[nodiscard]] bool OwnsCharacterStatus();
[[nodiscard]] bool OwnsSkills();
[[nodiscard]] bool OwnsInventory();
[[nodiscard]] bool IsCharacterWindowOpen();
[[nodiscard]] bool SuppressLegacyCharacterPanels();
[[nodiscard]] bool SuppressLegacyMainHud();
[[nodiscard]] bool OwnsQuestJournalUI();
[[nodiscard]] bool OwnsLegacyShopUI();
[[nodiscard]] bool OwnsLegacyTradeUI();
[[nodiscard]] bool OwnsLegacyCoinShopUI();
bool OwnsLegacyRestorationUI();
[[nodiscard]] bool OwnsLegacyHelpUI();
[[nodiscard]] bool OwnsLegacyMessage3UI();
[[nodiscard]] bool OwnsLegacyNotificationUI();
[[nodiscard]] bool OwnsLegacySkillNpcUI();
[[nodiscard]] bool SuppressLegacyMixGuide();
bool SuppressLegacyContextEffects();
[[nodiscard]] bool HasMigratedContext();
[[nodiscard]] bool BlocksWorldInputAt(int x, int y);
void ClearLegacyCharacterHelpOverlay();
}
