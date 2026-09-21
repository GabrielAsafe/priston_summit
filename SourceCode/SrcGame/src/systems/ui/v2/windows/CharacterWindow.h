#pragma once

#include <functional>
#include <utility>
#include "systems/ui/v2/core/Window.h"

namespace ui::v2
{
class Button;
class AttributesPage;
class SkillsPage;
class InventoryPage;
class ICharacterStatsGateway;
class ISkillGateway;
class IInventoryGateway;
class IContextGateway;
class ICompanionGateway;
class CompanionsPage;

enum class CharacterTab
{
    Attributes,
    Skills,
    Inventory,
    Companions
};

class CharacterWindow final : public Window
{
public:
    using CloseHandler = std::function<void()>;
    using TabChangedHandler = std::function<void(CharacterTab)>;

    CharacterWindow(ICharacterStatsGateway& statsGateway,
                    ISkillGateway& skillGateway,
                    IInventoryGateway& inventoryGateway,
                    IContextGateway& contextGateway,
                    ICompanionGateway& companionGateway,
                    UIId id = InvalidUIId);

    void SetActiveTab(CharacterTab tab, bool notify = true);
    [[nodiscard]] CharacterTab ActiveTab() const noexcept { return activeTab_; }

    void SetOnClose(CloseHandler handler) { onClose_ = std::move(handler); }
    void SetOnTabChanged(TabChangedHandler handler) { onTabChanged_ = std::move(handler); }
    void PrepareClose();
    bool HandleSkillShortcut(int shortcut);
    [[nodiscard]] bool HasArmedSkillShortcutTarget() const noexcept;
    [[nodiscard]] bool HasModalInput() const noexcept;
    bool HandleModalKey(std::uint32_t key);

    void SetViewport(int width, int height);

private:
    void BuildChrome();
    void Layout();

    CharacterTab activeTab_ = CharacterTab::Attributes;
    Button* attributesTab_ = nullptr;
    Button* skillsTab_ = nullptr;
    Button* inventoryTab_ = nullptr;
    Button* companionsTab_ = nullptr;
    Button* closeButton_ = nullptr;
    AttributesPage* attributesPage_ = nullptr;
    SkillsPage* skillsPage_ = nullptr;
    InventoryPage* inventoryPage_ = nullptr;
    CompanionsPage* companionsPage_ = nullptr;
    CloseHandler onClose_;
    TabChangedHandler onTabChanged_;
};
}
