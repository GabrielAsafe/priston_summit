#include "systems/ui/v2/windows/CharacterWindow.h"

#include <algorithm>
#include "systems/ui/v2/pages/AttributesPage.h"
#include "systems/ui/v2/pages/InventoryPage.h"
#include "systems/ui/v2/pages/CompanionsPage.h"
#include "systems/ui/v2/pages/CovenantsPage.h"
#include "systems/ui/v2/pages/CraftingPage392.h" // SUMMIT392_C_INCLUDE
#include "systems/ui/v2/pages/SkillsPage.h"
#include "systems/ui/v2/theme/UITheme.h"
#include "systems/ui/v2/widgets/Button.h"

namespace ui::v2
{
CharacterWindow::CharacterWindow(ICharacterStatsGateway& statsGateway,
                                 ISkillGateway& skillGateway,
                                 IInventoryGateway& inventoryGateway,
                                 IContextGateway& contextGateway,
                                 ICompanionGateway& companionGateway,
                                 ICovenantGateway& covenantGateway,
                                 UIId id)
    : Window(id, "Personagem")
{
    SetBounds({24, 24, 790, 560});
    SetTitleBarHeight(34);
    SetMovable(true);
    SetClosable(true);

    auto& attributes = AddChild<AttributesPage>(statsGateway, 510);
    attributesPage_ = &attributes;
    auto& skills = AddChild<SkillsPage>(skillGateway, 511);
    skillsPage_ = &skills;
    auto& inventory = AddChild<InventoryPage>(inventoryGateway, contextGateway, 512);
    inventoryPage_ = &inventory;
    auto& companions = AddChild<CompanionsPage>(companionGateway, 513);
    companionsPage_ = &companions;
    auto& covenants = AddChild<CovenantsPage>(covenantGateway, 514);
    covenantsPage_ = &covenants;
    auto& crafting = AddChild<CraftingPage392>(515); // SUMMIT392_C_CREATE
    craftingPage_ = &crafting;
    crafting.SetInventoryGateway(&inventoryGateway); // SUMMIT410A: real client backpack

    BuildChrome();
    Layout();
    SetActiveTab(CharacterTab::Attributes, false);
}

// SUMMIT462A: expose only the active lookup to the existing legacy key pump.
bool CharacterWindow::HasCraftingSearch462() const {
    return activeTab_==CharacterTab::Crafting && craftingPage_ &&
           craftingPage_->IsVisible() && craftingPage_->IsSearchActive462();
}
bool CharacterWindow::HandleCraftingSearchKey462(std::uint32_t key) {
    return HasCraftingSearch462() && craftingPage_->HandleSearchKey462(key);
}
void CharacterWindow::AcceptCraftingLive407(const std::string& frame) {
    if(craftingPage_)craftingPage_->AcceptLive407(frame);
}

void CharacterWindow::SetCraftingContext(std::function<std::string()> characterName,
                                        std::function<bool(const std::string&)> sender)
{
    if (!craftingPage_) return; // SUMMIT392_C_CONTEXT
    craftingPage_->SetCharacterName(std::move(characterName));
    craftingPage_->SetSender(std::move(sender));
}

void CharacterWindow::BuildChrome()
{
    auto& attributes = AddChild<Button>(500, "Atributos");
    attributes.SetOnClick([this](const UIEvent&) { SetActiveTab(CharacterTab::Attributes); });
    attributesTab_ = &attributes;

    auto& skills = AddChild<Button>(501, "Habilidades");
    skills.SetOnClick([this](const UIEvent&) { SetActiveTab(CharacterTab::Skills); });
    skillsTab_ = &skills;

    auto& inventory = AddChild<Button>(502, "Inventario");
    inventory.SetOnClick([this](const UIEvent&) { SetActiveTab(CharacterTab::Inventory); });
    inventoryTab_ = &inventory;

    auto& companions = AddChild<Button>(504, "Companheiros");
    companions.SetOnClick([this](const UIEvent&) { SetActiveTab(CharacterTab::Companions); });
    companionsTab_ = &companions;

    auto& covenants = AddChild<Button>(505, "Pactos");
    covenants.SetOnClick([this](const UIEvent&) { SetActiveTab(CharacterTab::Covenants); });
    covenantsTab_ = &covenants;
    auto& crafting = AddChild<Button>(506, "Crafting"); // SUMMIT392_C_TAB
    crafting.SetOnClick([this](const UIEvent&) { SetActiveTab(CharacterTab::Crafting); });
    craftingTab_ = &crafting;

    auto& close = AddChild<Button>(503, "X");
    close.SetOnClick([this](const UIEvent&)
    {
        PrepareClose();
        if (onClose_)
            onClose_();
    });
    closeButton_ = &close;
}

void CharacterWindow::Layout()
{
    const Rect bounds = Bounds();
    constexpr int top = 38;
    // SUMMIT392_C_LAYOUT: six tabs, same CharacterWindow chrome.
    attributesTab_->SetBounds({14, top, 108, 30});
    skillsTab_->SetBounds({128, top, 108, 30});
    inventoryTab_->SetBounds({242, top, 108, 30});
    companionsTab_->SetBounds({356, top, 115, 30});
    covenantsTab_->SetBounds({477, top, 108, 30});
    craftingTab_->SetBounds({591, top, 111, 30});
    closeButton_->SetBounds({bounds.width - 42, 5, 30, 24});

    const Rect pageBounds{8, 74, bounds.width - 16, bounds.height - 82};
    attributesPage_->SetBounds(pageBounds);
    skillsPage_->SetBounds(pageBounds);
    inventoryPage_->SetBounds(pageBounds);
    companionsPage_->SetBounds(pageBounds);
    covenantsPage_->SetBounds(pageBounds);
    craftingPage_->SetBounds(pageBounds); // SUMMIT392_C_BOUNDS
}

void CharacterWindow::SetActiveTab(CharacterTab tab, bool notify)
{
    if (activeTab_ == CharacterTab::Inventory && tab != CharacterTab::Inventory && inventoryPage_)
        inventoryPage_->CancelOutstandingDrag();

    if (tab == CharacterTab::Companions && companionsPage_)
        companionsPage_->ForceRefresh();
    if (tab == CharacterTab::Covenants && covenantsPage_)
        covenantsPage_->ForceRefresh();
    if (tab == CharacterTab::Crafting && craftingPage_) // SUMMIT392_C_REFRESH
        craftingPage_->ForceRefresh();

    activeTab_ = tab;
    attributesPage_->SetVisible(tab == CharacterTab::Attributes);
    skillsPage_->SetVisible(tab == CharacterTab::Skills);
    inventoryPage_->SetVisible(tab == CharacterTab::Inventory);
    companionsPage_->SetVisible(tab == CharacterTab::Companions);
    covenantsPage_->SetVisible(tab == CharacterTab::Covenants);
    craftingPage_->SetVisible(tab == CharacterTab::Crafting); // SUMMIT392_C_VISIBLE

    attributesTab_->SetSelected(tab == CharacterTab::Attributes);
    skillsTab_->SetSelected(tab == CharacterTab::Skills);
    inventoryTab_->SetSelected(tab == CharacterTab::Inventory);
    companionsTab_->SetSelected(tab == CharacterTab::Companions);
    covenantsTab_->SetSelected(tab == CharacterTab::Covenants);
    craftingTab_->SetSelected(tab == CharacterTab::Crafting); // SUMMIT392_C_SELECTED

    if (notify && onTabChanged_)
        onTabChanged_(tab);
}

void CharacterWindow::PrepareClose()
{
    if (inventoryPage_)
        inventoryPage_->CancelOutstandingDrag();
}

bool CharacterWindow::HandleSkillShortcut(int shortcut)
{
    return activeTab_ == CharacterTab::Skills && skillsPage_ && skillsPage_->HandleShortcut(shortcut);
}

bool CharacterWindow::HasArmedSkillShortcutTarget() const noexcept
{
    return activeTab_ == CharacterTab::Skills && skillsPage_ && skillsPage_->HasArmedShortcutTarget();
}

bool CharacterWindow::HasModalInput() const noexcept
{
    return inventoryPage_ && inventoryPage_->HasModalInput();
}

bool CharacterWindow::HandleModalKey(std::uint32_t key)
{
    return inventoryPage_ && inventoryPage_->HasModalInput() ? inventoryPage_->HandleModalKey(key) : false;
}

void CharacterWindow::SetViewport(int width, int height)
{
    Rect bounds = Bounds();
    bounds.width = std::min(790, std::max(760, width - 20));
    bounds.height = std::min(560, std::max(540, height - 20));
    bounds.x = std::max(0, (width - bounds.width) / 2);
    bounds.y = std::max(0, (height - bounds.height) / 2);
    SetBounds(bounds);
    Layout();
}
}
