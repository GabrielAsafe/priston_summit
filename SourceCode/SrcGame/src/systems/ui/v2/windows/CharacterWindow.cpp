#include "systems/ui/v2/windows/CharacterWindow.h"

#include <algorithm>
#include "systems/ui/v2/pages/AttributesPage.h"
#include "systems/ui/v2/pages/InventoryPage.h"
#include "systems/ui/v2/pages/CompanionsPage.h"
#include "systems/ui/v2/pages/CovenantsPage.h"
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

    BuildChrome();
    Layout();
    SetActiveTab(CharacterTab::Attributes, false);
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

    auto& companions = AddChild<Button>(504, "Companions");
    companions.SetOnClick([this](const UIEvent&) { SetActiveTab(CharacterTab::Companions); });
    companionsTab_ = &companions;

    auto& covenants = AddChild<Button>(505, "Covenants");
    covenants.SetOnClick([this](const UIEvent&) { SetActiveTab(CharacterTab::Covenants); });
    covenantsTab_ = &covenants;

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
    attributesTab_->SetBounds({14, top, 116, 30});
    skillsTab_->SetBounds({136, top, 116, 30});
    inventoryTab_->SetBounds({258, top, 116, 30});
    companionsTab_->SetBounds({380, top, 126, 30});
    covenantsTab_->SetBounds({512, top, 126, 30});
    closeButton_->SetBounds({bounds.width - 42, 5, 30, 24});

    const Rect pageBounds{8, 74, bounds.width - 16, bounds.height - 82};
    attributesPage_->SetBounds(pageBounds);
    skillsPage_->SetBounds(pageBounds);
    inventoryPage_->SetBounds(pageBounds);
    companionsPage_->SetBounds(pageBounds);
    covenantsPage_->SetBounds(pageBounds);
}

void CharacterWindow::SetActiveTab(CharacterTab tab, bool notify)
{
    if (activeTab_ == CharacterTab::Inventory && tab != CharacterTab::Inventory && inventoryPage_)
        inventoryPage_->CancelOutstandingDrag();

    if (tab == CharacterTab::Companions && companionsPage_)
        companionsPage_->ForceRefresh();
    if (tab == CharacterTab::Covenants && covenantsPage_)
        covenantsPage_->ForceRefresh();

    activeTab_ = tab;
    attributesPage_->SetVisible(tab == CharacterTab::Attributes);
    skillsPage_->SetVisible(tab == CharacterTab::Skills);
    inventoryPage_->SetVisible(tab == CharacterTab::Inventory);
    companionsPage_->SetVisible(tab == CharacterTab::Companions);
    covenantsPage_->SetVisible(tab == CharacterTab::Covenants);

    attributesTab_->SetSelected(tab == CharacterTab::Attributes);
    skillsTab_->SetSelected(tab == CharacterTab::Skills);
    inventoryTab_->SetSelected(tab == CharacterTab::Inventory);
    companionsTab_->SetSelected(tab == CharacterTab::Companions);
    covenantsTab_->SetSelected(tab == CharacterTab::Covenants);

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
