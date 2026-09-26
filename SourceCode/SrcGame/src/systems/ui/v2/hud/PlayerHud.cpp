#include "systems/ui/v2/hud/PlayerHud.h"

#include <algorithm>
#include <string>
#include "systems/ui/v2/rendering/IUIRenderer.h"
#include "systems/ui/v2/theme/UITheme.h"
#include "systems/ui/v2/widgets/TextLayout.h"
#include "systems/ui/v2/widgets/Button.h"

namespace ui::v2
{
namespace
{
constexpr int HudWidth = 558;
constexpr int ShortcutBand = 58;
constexpr int HudHeight = 82 + ShortcutBand;

void DrawBorder(IUIRenderer& r, const Rect& rect, Color color)
{
    r.DrawRect({rect.x, rect.y, rect.width, 1}, color);
    r.DrawRect({rect.x, rect.y + rect.height - 1, rect.width, 1}, color);
    r.DrawRect({rect.x, rect.y, 1, rect.height}, color);
    r.DrawRect({rect.x + rect.width - 1, rect.y, 1, rect.height}, color);
}

void DrawBar(IUIRenderer& r, const Rect& rect, int current, int maximum, Color fill, const char* label)
{
    r.DrawRect(rect, UITheme::Slot);
    const int safeMax = std::max(1, maximum);
    const int clamped = std::clamp(current, 0, safeMax);
    const int fillWidth = rect.width * clamped / safeMax;
    if (fillWidth > 0)
        r.DrawRect({rect.x, rect.y, fillWidth, rect.height}, fill);
    DrawBorder(r, rect, UITheme::BorderSoft);

    const std::string text = std::string(label) + "  " + std::to_string(current) + "/" + std::to_string(maximum);
    r.DrawTextUtf8(FontHandle{}, text, rect, TextStyle{UITheme::Text, true, TextAlign::Center});
}
}

PlayerHud::PlayerHud(ICharacterStatsGateway& statsGateway,
                     ISkillGateway& skillGateway,
                     IInventoryGateway& inventoryGateway,
                     UIId id)
    : Window(id, {}), stats_(statsGateway), skills_(skillGateway), inventory_(inventoryGateway)
{
    SetMovable(false);
    SetClosable(false);
    SetTitleBarHeight(0);
    SetBounds({0, 0, HudWidth, HudHeight});
    BuildShortcuts();
    stats_.Refresh();
    skills_.Refresh();
    inventory_.Refresh();
}

void PlayerHud::SetViewport(int width, int height)
{
    const int x = std::max(8, (width - HudWidth) / 2);
    const int y = std::max(8, height - HudHeight - 8);
    SetBounds({x, y, std::min(HudWidth, std::max(320, width - 16)), HudHeight});
    LayoutShortcuts();
}

void PlayerHud::BuildShortcuts()
{
    static const char* labels[10] = {"C Atrib", "S Skills", "V Inv", "D Grupo", "Q Quest",
                                     "X Sistema", "R Correr", "Z Camera", "Tab Mapa", "Enter Chat"};
    for (int i = 0; i < 10; ++i)
    {
        auto& button = AddChild<Button>(150 + i, labels[i]);
        button.SetOnClick([this, i](const UIEvent&)
        {
            if (onShortcut_) onShortcut_(static_cast<HudShortcutAction>(i));
        });
        shortcutButtons_[static_cast<std::size_t>(i)] = &button;
    }
    LayoutShortcuts();
}

void PlayerHud::LayoutShortcuts()
{
    const int gap = 4;
    const int left = 8;
    const int available = std::max(250, Bounds().width - left * 2);
    const int buttonWidth = std::max(44, (available - gap * 4) / 5);
    for (int i = 0; i < 10; ++i)
    {
        if (!shortcutButtons_[static_cast<std::size_t>(i)]) continue;
        const int row = i / 5;
        const int col = i % 5;
        shortcutButtons_[static_cast<std::size_t>(i)]->SetBounds({left + col * (buttonWidth + gap), 5 + row * 26, buttonWidth, 22});
    }
}

void PlayerHud::SetLegacyControlState(bool running, int cameraMode, bool mapVisible)
{
    if (shortcutButtons_[6])
        shortcutButtons_[6]->SetText(running ? "R Andar" : "R Correr");

    if (shortcutButtons_[7])
    {
        const char* cameraText = cameraMode == 1 ? "Z Cam Auto" :
                                 cameraMode == 2 ? "Z Cam Fixa" : "Z Cam Manual";
        shortcutButtons_[7]->SetText(cameraText);
    }

    if (shortcutButtons_[8])
        shortcutButtons_[8]->SetText(mapVisible ? "Tab Fechar mapa" : "Tab Abrir mapa");
}

void PlayerHud::Update(float deltaSeconds)
{
    refreshAccumulator_ += deltaSeconds;
    if (refreshAccumulator_ >= 0.10f)
    {
        refreshAccumulator_ = 0.0f;
        stats_.Refresh();
        skills_.Refresh();
        inventory_.Refresh();
    }
    Window::Update(deltaSeconds);
}

Rect PlayerHud::VitalsRect() const noexcept
{
    return {8, 8 + ShortcutBand, 248, 66};
}

Rect PlayerHud::SkillRect(bool right) const noexcept
{
    return {right ? 318 : 264, 8 + ShortcutBand, 48, 66};
}

Rect PlayerHud::PotionRect(int ordinal) const noexcept
{
    return {382 + ordinal * 58, 8 + ShortcutBand, 52, 66};
}

const SkillEntrySnapshot* PlayerHud::ActiveSkill(bool right) const noexcept
{
    for (const auto& skill : skills_.Snapshot().skills)
        if (right ? skill.rightSelected : skill.leftSelected)
            return &skill;
    return nullptr;
}

const InventoryItemSnapshot* PlayerHud::Potion(int ordinal) const noexcept
{
    const int slot = 11 + ordinal;
    for (const auto& item : inventory_.Snapshot().items)
        if (item.equipmentSlot == slot)
            return &item;
    return nullptr;
}

UIEventResult PlayerHud::OnEvent(const UIEvent& event)
{
    const UIEventResult shortcut = Container::OnEvent(event);
    if (shortcut != UIEventResult::Ignored)
        return shortcut;
    if (event.type == UIEventType::PointerDown && event.button == MouseButton::Left)
    {
        const Rect root = GlobalBounds();
        const Point local{event.pointer.x - root.x, event.pointer.y - root.y};
        for (int i = 0; i < 3; ++i)
        {
            if (!PotionRect(i).Contains(local))
                continue;
            if (const auto* potion = Potion(i))
                inventory_.Activate(potion->index);
            return UIEventResult::Handled;
        }
    }
    // The status/skill regions are presentation-only for now. Returning
    // Ignored lets normal world clicks pass through those transparent areas.
    return UIEventResult::Ignored;
}

void PlayerHud::DrawVitals(UIRenderContext& context)
{
    auto& r = context.Renderer();
    const Rect root = GlobalBounds();
    const Rect local = VitalsRect();
    const Rect box{root.x + local.x, root.y + local.y, local.width, local.height};
    const auto& s = stats_.Snapshot();

    r.DrawRect(box, UITheme::Surface);
    DrawBorder(r, box, UITheme::BorderSoft);
    if (!s.valid)
        return;

    DrawBar(r, {box.x + 6, box.y + 5, box.width - 12, 16}, s.lifeCurrent, s.lifeMax,
            Color{174, 72, 82, 255}, "HP");
    DrawBar(r, {box.x + 6, box.y + 25, box.width - 12, 16}, s.manaCurrent, s.manaMax,
            Color{75, 113, 177, 255}, "MP");
    DrawBar(r, {box.x + 6, box.y + 45, box.width - 12, 16}, s.staminaCurrent, s.staminaMax,
            Color{111, 151, 78, 255}, "RES");
}

void PlayerHud::DrawSkill(UIRenderContext& context, bool right)
{
    auto& r = context.Renderer();
    const Rect root = GlobalBounds();
    Rect rect = SkillRect(right);
    rect.x += root.x;
    rect.y += root.y;
    r.DrawRect(rect, UITheme::Surface);
    DrawBorder(r, rect, UITheme::BorderSoft);

    const auto* skill = ActiveSkill(right);
    const Rect icon{rect.x + 6, rect.y + 5, 36, 36};
    r.DrawRect(icon, UITheme::Slot);
    if (skill && skill->icon.IsValid())
        r.DrawImage(skill->icon, icon, nullptr);
    if (skill && skill->cooldownGauge < 35)
    {
        const int remaining = 35 - std::clamp(skill->cooldownGauge, 0, 35);
        const int cover = std::max(1, icon.height * remaining / 35);
        r.DrawRect({icon.x, icon.y, icon.width, cover}, Color{8, 10, 14, 190});
        const int progress = icon.width * std::clamp(skill->cooldownGauge, 0, 35) / 35;
        if (progress > 0)
            r.DrawRect({icon.x, icon.y + icon.height - 3, progress, 3}, UITheme::Accent);
    }
    // Keep the bottom HUD intentionally compact: the icon communicates the
    // selected skill and L/R indicates the mouse binding. The full skill name
    // belongs in the CharacterWindow tooltip/details surface, not over the
    // small HUD slot.
    r.DrawTextUtf8(FontHandle{}, right ? "R" : "L", {rect.x + 3, rect.y + 45, rect.width - 6, 16},
                   TextStyle{UITheme::Accent, true, TextAlign::Center});
}

void PlayerHud::DrawPotion(UIRenderContext& context, int ordinal)
{
    auto& r = context.Renderer();
    const Rect root = GlobalBounds();
    Rect rect = PotionRect(ordinal);
    rect.x += root.x;
    rect.y += root.y;
    r.DrawRect(rect, UITheme::Surface);
    DrawBorder(r, rect, UITheme::BorderSoft);

    const auto* item = Potion(ordinal);
    const Rect icon{rect.x + 8, rect.y + 5, 36, 36};
    r.DrawRect(icon, UITheme::Slot);
    if (item && item->texture.IsValid())
        r.DrawImage(item->texture, icon, nullptr);
    if (item && item->potionCount > 0)
        r.DrawTextUtf8(FontHandle{}, std::to_string(item->potionCount),
                       {rect.x + 3, rect.y + 28, rect.width - 6, 16},
                       TextStyle{UITheme::Text, true, TextAlign::Right});
    r.DrawTextUtf8(FontHandle{}, "P" + std::to_string(ordinal + 1),
                   {rect.x + 3, rect.y + 47, rect.width - 6, 14},
                   TextStyle{UITheme::TextMuted, true, TextAlign::Center});
}

void PlayerHud::OnRender(UIRenderContext& context)
{
    const Rect root = GlobalBounds();
    auto& r = context.Renderer();
    r.DrawRect(root, Color{13, 16, 22, 225});
    DrawBorder(r, root, UITheme::BorderSoft);
    DrawVitals(context);
    DrawSkill(context, false);
    DrawSkill(context, true);
    for (int i = 0; i < 3; ++i)
        DrawPotion(context, i);
}
}
