#pragma once

#include "systems/ui/v2/core/Window.h"
#include "systems/ui/v2/viewmodels/AttributesViewModel.h"
#include "systems/ui/v2/viewmodels/InventoryViewModel.h"
#include "systems/ui/v2/viewmodels/SkillsViewModel.h"
#include <array>
#include <functional>

namespace ui::v2
{
class Button;

enum class HudShortcutAction
{
    Attributes,
    Skills,
    Inventory,
    Party,
    Quests,
    System,
    RunWalk,
    Camera,
    Map,
    Chat
};
// Permanent replacement for the legacy bottom HUD chrome. The original
// vital/skill/consumable block keeps its screen position and a compact V2
// shortcut band above it restores the useful controls removed with the legacy
// menu.
class PlayerHud final : public Window
{
public:
    PlayerHud(ICharacterStatsGateway& statsGateway,
              ISkillGateway& skillGateway,
              IInventoryGateway& inventoryGateway,
              UIId id = InvalidUIId);

    using ShortcutHandler = std::function<void(HudShortcutAction)>;
    void SetOnShortcut(ShortcutHandler handler) { onShortcut_ = std::move(handler); }
    void SetViewport(int width, int height);
    void SetLegacyControlState(bool running, int cameraMode, bool mapVisible);
    void Update(float deltaSeconds) override;
    UIEventResult OnEvent(const UIEvent& event) override;

protected:
    void OnRender(UIRenderContext& context) override;

private:
    [[nodiscard]] Rect VitalsRect() const noexcept;
    [[nodiscard]] Rect SkillRect(bool right) const noexcept;
    [[nodiscard]] Rect PotionRect(int ordinal) const noexcept;
    [[nodiscard]] const SkillEntrySnapshot* ActiveSkill(bool right) const noexcept;
    [[nodiscard]] const InventoryItemSnapshot* Potion(int ordinal) const noexcept;
    void DrawVitals(UIRenderContext& context);
    void DrawSkill(UIRenderContext& context, bool right);
    void DrawPotion(UIRenderContext& context, int ordinal);
    void BuildShortcuts();
    void LayoutShortcuts();

    AttributesViewModel stats_;
    SkillsViewModel skills_;
    InventoryViewModel inventory_;
    float refreshAccumulator_ = 0.0f;
    std::array<Button*, 10> shortcutButtons_{};
    ShortcutHandler onShortcut_;
};
}
