#pragma once

#include <string> // SUMMIT392_H_STRING
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
class ICovenantGateway;
class CompanionsPage;
class CovenantsPage;
class CraftingPage392; // SUMMIT392_H_PAGE

enum class CharacterTab
{
    Attributes,
    Skills,
    Inventory,
    Companions,
    Covenants,
    Crafting // SUMMIT392_H_ENUM
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
                    ICovenantGateway& covenantGateway,
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

    // SUMMIT462A: scoped Crafting lookup key routing; never consumes chat keys
    // unless Crafting is open and its search dialog is active.
    bool HasCraftingSearch462() const;
    bool HandleCraftingSearchKey462(std::uint32_t key);
    void AcceptCraftingLive407(const std::string& frame);
    void SetCraftingContext(std::function<std::string()> characterName,
                            std::function<bool(const std::string&)> sender); // SUMMIT392_H_CONTEXT
    void SetViewport(int width, int height);

private:
    void BuildChrome();
    void Layout();

    CharacterTab activeTab_ = CharacterTab::Attributes;
    Button* attributesTab_ = nullptr;
    Button* skillsTab_ = nullptr;
    Button* inventoryTab_ = nullptr;
    Button* companionsTab_ = nullptr;
    Button* covenantsTab_ = nullptr;
    Button* craftingTab_ = nullptr; // SUMMIT392_H_BUTTON
    Button* closeButton_ = nullptr;
    AttributesPage* attributesPage_ = nullptr;
    SkillsPage* skillsPage_ = nullptr;
    InventoryPage* inventoryPage_ = nullptr;
    CompanionsPage* companionsPage_ = nullptr;
    CovenantsPage* covenantsPage_ = nullptr;
    CraftingPage392* craftingPage_ = nullptr; // SUMMIT392_H_POINTER
    CloseHandler onClose_;
    TabChangedHandler onTabChanged_;
};
}
