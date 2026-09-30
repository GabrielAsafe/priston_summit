#include "systems/ui/v2/crafting/MixGuideWindow.h"

#include <algorithm>
#include <string>
#include <vector>

#include "systems/ui/v2/rendering/IUIRenderer.h"
#include "systems/ui/v2/theme/UITheme.h"
#include "systems/ui/v2/widgets/Button.h"
#include "systems/ui/v2/widgets/TextLayout.h"

namespace ui::v2
{
namespace
{
constexpr int RowHeight = 30;
constexpr int LineHeight = 18;

struct VisualLine
{
    std::string text;
    Color color = UITheme::Text;
    bool bold = false;
};

void DrawBorder(IUIRenderer& renderer, const Rect& rect, Color color)
{
    renderer.DrawRect({rect.x, rect.y, rect.width, 1}, color);
    renderer.DrawRect({rect.x, rect.y + rect.height - 1, rect.width, 1}, color);
    renderer.DrawRect({rect.x, rect.y, 1, rect.height}, color);
    renderer.DrawRect({rect.x + rect.width - 1, rect.y, 1, rect.height}, color);
}

void AddWrapped(std::vector<VisualLine>& out, const std::string& value, int width,
                Color color = UITheme::Text, bool bold = false)
{
    if (value.empty()) return;
    const auto lines = WrapTextApprox(value, std::max(80, width), 180, 8);
    for (const auto& line : lines) out.push_back({line, color, bold});
}

std::vector<VisualLine> DetailLines(const MixGuideSnapshot& catalog,
                                    const MixRecipeSnapshot* recipe,
                                    int width)
{
    std::vector<VisualLine> lines;
    if (!recipe)
    {
        lines.push_back({"Selecione uma receita.", UITheme::TextMuted, false});
        return lines;
    }
    lines.push_back({recipe->itemTypeLabel + " - " + recipe->recipeId, UITheme::Accent, true});
    lines.push_back({"Nivel minimo: " + std::to_string(recipe->minimumCharacterLevel) +
                     "  |  Sheltoms: " + std::to_string(recipe->totalSheltoms), UITheme::TextMuted, false});
    lines.push_back({"", UITheme::Text, false});
    lines.push_back({"INGREDIENTES", UITheme::TextMuted, true});
    for (const auto& value : recipe->ingredients) AddWrapped(lines, "- " + value, width);
    if (recipe->ingredients.empty()) lines.push_back({"Sem ingredientes documentados.", UITheme::Disabled, false});
    lines.push_back({"", UITheme::Text, false});
    lines.push_back({"BONUS", UITheme::TextMuted, true});
    for (const auto& value : recipe->bonuses) AddWrapped(lines, "- " + value, width, UITheme::Positive);
    if (recipe->bonuses.empty()) lines.push_back({"Sem bonus documentados.", UITheme::Disabled, false});
    if (!recipe->sourceIngredients.empty() || !recipe->sourceBonuses.empty())
    {
        lines.push_back({"", UITheme::Text, false});
        lines.push_back({"NOTACAO DA FONTE", UITheme::TextMuted, true});
        if (!recipe->sourceIngredients.empty()) AddWrapped(lines, recipe->sourceIngredients, width, UITheme::TextMuted);
        if (!recipe->sourceBonuses.empty()) AddWrapped(lines, recipe->sourceBonuses, width, UITheme::TextMuted);
    }
    if (!catalog.rules.empty())
    {
        lines.push_back({"", UITheme::Text, false});
        lines.push_back({"REGRAS", UITheme::TextMuted, true});
        for (const auto& rule : catalog.rules) AddWrapped(lines, "- " + rule, width, UITheme::TextMuted);
    }
    if (!catalog.sourceUrl.empty())
    {
        lines.push_back({"", UITheme::Text, false});
        lines.push_back({"FONTE", UITheme::TextMuted, true});
        AddWrapped(lines, catalog.sourceUrl, width, UITheme::TextMuted);
        if (!catalog.retrievedDate.empty()) AddWrapped(lines, "Catalogado em " + catalog.retrievedDate, width, UITheme::TextMuted);
    }
    return lines;
}
}

MixGuideWindow::MixGuideWindow(IMixGuideGateway& gateway, UIId id)
    : Window(id, "Tabela de Mix"), gateway_(gateway)
{
    SetBounds({40, 35, 820, 560});
    SetTitleBarHeight(34);
    SetMovable(true);
    SetClosable(true);
    BuildChrome();
    Layout();
    Reload();
}

void MixGuideWindow::BuildChrome()
{
    auto& all = AddChild<Button>(1001, "Todos");
    all.SetOnClick([this](const UIEvent&) { SetCategory(Category::All); }); allButton_ = &all;
    auto& weapon = AddChild<Button>(1002, "Armas");
    weapon.SetOnClick([this](const UIEvent&) { SetCategory(Category::Weapon); }); weaponButton_ = &weapon;
    auto& armor = AddChild<Button>(1003, "Armad./Robes");
    armor.SetOnClick([this](const UIEvent&) { SetCategory(Category::Armor); }); armorButton_ = &armor;
    auto& shield = AddChild<Button>(1004, "Escudos");
    shield.SetOnClick([this](const UIEvent&) { SetCategory(Category::Shield); }); shieldButton_ = &shield;
    auto& orbital = AddChild<Button>(1005, "Orbitais");
    orbital.SetOnClick([this](const UIEvent&) { SetCategory(Category::Orbital); }); orbitalButton_ = &orbital;
    auto& close = AddChild<Button>(1006, "X");
    close.SetOnClick([this](const UIEvent&) { if (onClose_) onClose_(); }); closeButton_ = &close;
}

void MixGuideWindow::Layout()
{
    const Rect b = Bounds();
    int x = 14;
    allButton_->SetBounds({x, 40, 78, 30}); x += 84;
    weaponButton_->SetBounds({x, 40, 88, 30}); x += 94;
    armorButton_->SetBounds({x, 40, 118, 30}); x += 124;
    shieldButton_->SetBounds({x, 40, 92, 30}); x += 98;
    orbitalButton_->SetBounds({x, 40, 92, 30});
    closeButton_->SetBounds({b.width - 42, 5, 30, 24});
    allButton_->SetSelected(category_ == Category::All);
    weaponButton_->SetSelected(category_ == Category::Weapon);
    armorButton_->SetSelected(category_ == Category::Armor);
    shieldButton_->SetSelected(category_ == Category::Shield);
    orbitalButton_->SetSelected(category_ == Category::Orbital);
}

void MixGuideWindow::SetViewport(int width, int height)
{
    Rect b = Bounds();
    b.width = std::min(900, std::max(720, width - 30));
    b.height = std::min(620, std::max(500, height - 30));
    b.x = std::max(0, (width - b.width) / 2);
    b.y = std::max(0, (height - b.height) / 2);
    SetBounds(b); Layout();
    listScroll_ = std::clamp(listScroll_, 0, MaxListScroll());
    detailScroll_ = std::clamp(detailScroll_, 0, MaxDetailScroll());
}

void MixGuideWindow::Reload()
{
    snapshot_ = gateway_.GetSnapshot();
    if (selected_ >= snapshot_.recipes.size()) selected_ = 0;
    listScroll_ = detailScroll_ = 0;
    Layout();
}

void MixGuideWindow::SetCategory(Category category)
{
    category_ = category;
    selected_ = 0;
    for (std::size_t i = 0; i < snapshot_.recipes.size(); ++i)
        if (Visible(snapshot_.recipes[i])) { selected_ = i; break; }
    listScroll_ = detailScroll_ = 0;
    Layout();
}

bool MixGuideWindow::Visible(const MixRecipeSnapshot& r) const
{
    switch (category_)
    {
        case Category::All: return true;
        case Category::Weapon: return r.itemType == "weapon";
        case Category::Armor: return r.itemType == "armor_or_robe";
        case Category::Shield: return r.itemType == "shield";
        case Category::Orbital: return r.itemType == "orbital";
    }
    return true;
}

const MixRecipeSnapshot* MixGuideWindow::Selected() const noexcept
{
    return selected_ < snapshot_.recipes.size() ? &snapshot_.recipes[selected_] : nullptr;
}

int MixGuideWindow::MaxListScroll() const noexcept
{
    int count = 0; for (const auto& r : snapshot_.recipes) if (Visible(r)) ++count;
    const int height = std::max(1, Bounds().height - 100);
    return std::max(0, count * RowHeight - height + 8);
}

int MixGuideWindow::MaxDetailScroll() const noexcept
{
    const int width = std::max(180, Bounds().width - 338 - 26);
    const int height = std::max(1, Bounds().height - 100);
    const auto lines = DetailLines(snapshot_, Selected(), width - 24);
    return std::max(0, static_cast<int>(lines.size()) * LineHeight - height + 20);
}

void MixGuideWindow::BuildVisibleRows()
{
    visibleRows_.clear();
    const Rect root = GlobalBounds();
    const Rect list{root.x + 14, root.y + 82, 306, std::max(1, root.height - 96)};
    int y = list.y + 4 - listScroll_;
    for (std::size_t i = 0; i < snapshot_.recipes.size(); ++i)
    {
        if (!Visible(snapshot_.recipes[i])) continue;
        Rect row{list.x + 4, y, list.width - 12, RowHeight - 2};
        if (row.y + row.height >= list.y && row.y <= list.y + list.height)
            visibleRows_.push_back({i, row});
        y += RowHeight;
    }
}

UIEventResult MixGuideWindow::OnEvent(const UIEvent& event)
{
    BuildVisibleRows();
    const Rect root = GlobalBounds();
    const Rect list{root.x + 14, root.y + 82, 306, std::max(1, root.height - 96)};
    const Rect detail{root.x + 334, root.y + 82, std::max(1, root.width - 348), std::max(1, root.height - 96)};
    if (event.type == UIEventType::Wheel)
    {
        const int dir = event.wheelDelta > 0 ? -1 : (event.wheelDelta < 0 ? 1 : 0);
        if (list.Contains(event.pointer))
        {
            listScroll_ = std::clamp(listScroll_ + dir * RowHeight * 2, 0, MaxListScroll());
            BuildVisibleRows(); return UIEventResult::Handled;
        }
        if (detail.Contains(event.pointer))
        {
            detailScroll_ = std::clamp(detailScroll_ + dir * LineHeight * 3, 0, MaxDetailScroll());
            return UIEventResult::Handled;
        }
    }
    if (event.type == UIEventType::PointerDown && event.button == MouseButton::Left)
        for (const auto& row : visibleRows_)
            if (row.rect.Contains(event.pointer))
            {
                selected_ = row.recipeIndex; detailScroll_ = 0; return UIEventResult::Handled;
            }
    return Window::OnEvent(event);
}

void MixGuideWindow::DrawList(UIRenderContext& context)
{
    const Rect root = GlobalBounds();
    const Rect list{root.x + 14, root.y + 82, 306, std::max(1, root.height - 96)};
    auto& r = context.Renderer(); r.DrawRect(list, UITheme::Surface); DrawBorder(r, list, UITheme::BorderSoft);
    r.PushClip({list.x + 1, list.y + 1, list.width - 2, list.height - 2});
    int y = list.y + 4 - listScroll_;
    for (std::size_t i = 0; i < snapshot_.recipes.size(); ++i)
    {
        const auto& recipe = snapshot_.recipes[i]; if (!Visible(recipe)) continue;
        Rect row{list.x + 4, y, list.width - 12, RowHeight - 2};
        if (row.y + row.height >= list.y && row.y <= list.y + list.height)
        {
            const bool selected = i == selected_;
            r.DrawRect(row, selected ? UITheme::SurfacePressed : UITheme::Slot);
            if (selected) r.DrawRect({row.x, row.y, 3, row.height}, UITheme::Accent);
            const std::string text = recipe.recipeId + "  Lv." + std::to_string(recipe.minimumCharacterLevel);
            r.DrawTextUtf8(FontHandle{}, EllipsizeText(text, row.width - 22, 7),
                           {row.x + 10, row.y + 6, row.width - 18, 18},
                           TextStyle{UITheme::Text, selected, TextAlign::Left});
        }
        y += RowHeight;
    }
    r.PopClip();
}

void MixGuideWindow::DrawDetails(UIRenderContext& context)
{
    const Rect root = GlobalBounds();
    const Rect detail{root.x + 334, root.y + 82, std::max(1, root.width - 348), std::max(1, root.height - 96)};
    auto& r = context.Renderer(); r.DrawRect(detail, UITheme::Surface); DrawBorder(r, detail, UITheme::BorderSoft);
    const Rect content{detail.x + 12, detail.y + 12, detail.width - 24, detail.height - 22};
    const auto lines = DetailLines(snapshot_, Selected(), content.width - 8);
    r.PushClip(content); int y = content.y - detailScroll_;
    for (const auto& line : lines)
    {
        if (y + LineHeight >= content.y && y <= content.y + content.height)
            r.DrawTextUtf8(FontHandle{}, line.text, {content.x, y, content.width - 8, LineHeight},
                           TextStyle{line.color, line.bold, TextAlign::Left});
        y += LineHeight;
    }
    r.PopClip();
}

void MixGuideWindow::OnRender(UIRenderContext& context)
{
    Layout(); Window::OnRender(context); DrawList(context); DrawDetails(context);
}
}
