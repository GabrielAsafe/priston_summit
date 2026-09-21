#include "systems/ui/v2/hud/CompanionPartyHud.h"

#include <algorithm>
#include <string>

#include "systems/ui/v2/rendering/IUIRenderer.h"
#include "systems/ui/v2/theme/UITheme.h"

namespace ui::v2
{
namespace
{
constexpr int HudWidth = 286;
constexpr int RowHeight = 58;
constexpr int RowGap = 4;
constexpr int HudHeight = 4 * RowHeight + 3 * RowGap;

const char* ModeName(CompanionUiMode mode)
{
    switch (mode)
    {
        case CompanionUiMode::Follow: return "SEG";
        case CompanionUiMode::Passive: return "PAS";
        case CompanionUiMode::Aggressive:
        default: return "AGR";
    }
}

const char* TacticName(CompanionUiTactic tactic)
{
    switch (tactic)
    {
        case CompanionUiTactic::OwnerFocus: return "FOCO";
        case CompanionUiTactic::Defensive: return "DEF";
        case CompanionUiTactic::HoldPosition: return "POS";
        case CompanionUiTactic::Balanced:
        default: return "BAL";
    }
}

void DrawBorder(IUIRenderer& renderer, const Rect& rect)
{
    renderer.DrawRect({rect.x, rect.y, rect.width, 1}, UITheme::BorderSoft);
    renderer.DrawRect({rect.x, rect.y + rect.height - 1, rect.width, 1}, UITheme::BorderSoft);
    renderer.DrawRect({rect.x, rect.y, 1, rect.height}, UITheme::BorderSoft);
    renderer.DrawRect({rect.x + rect.width - 1, rect.y, 1, rect.height}, UITheme::BorderSoft);
}

void DrawBar(IUIRenderer& renderer, const Rect& rect, int current, int maximum, Color fill)
{
    renderer.DrawRect(rect, UITheme::Slot);
    const int safeMax = std::max(1, maximum);
    const int clamped = std::clamp(current, 0, safeMax);
    const int fillWidth = rect.width * clamped / safeMax;
    if (fillWidth > 0)
        renderer.DrawRect({rect.x, rect.y, fillWidth, rect.height}, fill);
    DrawBorder(renderer, rect);
}
}

CompanionPartyHud::CompanionPartyHud(ICompanionGateway& gateway, UIId id)
    : Window(id, {}), gateway_(gateway)
{
    SetMovable(false);
    SetClosable(false);
    SetTitleBarHeight(0);
    SetBounds({12, 120, HudWidth, HudHeight});
}

void CompanionPartyHud::SetViewport(int width, int height)
{
    const int x = 12;
    const int y = std::max(80, (height - HudHeight) / 2 - 40);
    SetBounds({x, y, std::min(HudWidth, std::max(230, width / 3)), HudHeight});
}

Rect CompanionPartyHud::RowRect(int index) const noexcept
{
    return {0, index * (RowHeight + RowGap), Bounds().width, RowHeight};
}

UIEventResult CompanionPartyHud::OnEvent(const UIEvent& event)
{
    if (event.type != UIEventType::PointerDown)
        return UIEventResult::Ignored;

    const auto& snapshot = gateway_.RuntimeSnapshot();
    if (!snapshot.valid)
        return UIEventResult::Ignored;

    const Rect root = GlobalBounds();
    const Point local{event.pointer.x - root.x, event.pointer.y - root.y};
    for (int i = 0; i < snapshot.entryCount; ++i)
    {
        if (!RowRect(i).Contains(local))
            continue;

        const auto& entry = snapshot.entries[static_cast<std::size_t>(i)];
        if (entry.name.empty())
            return UIEventResult::Ignored;

        if (event.button == MouseButton::Left)
        {
            const int nextMode = (static_cast<int>(entry.mode) + 1) % 3;
            gateway_.Execute(CompanionUiAction::SetMode, entry.name, nextMode);
            return UIEventResult::Handled;
        }
        if (event.button == MouseButton::Middle)
        {
            gateway_.Execute(CompanionUiAction::SetTactic, entry.name,
                static_cast<int>(CompanionUiTactic::OwnerFocus));
            return UIEventResult::Handled;
        }
        if (event.button == MouseButton::Right)
        {
            gateway_.Execute(CompanionUiAction::Dismiss, entry.name);
            return UIEventResult::Handled;
        }
    }
    return UIEventResult::Ignored;
}

void CompanionPartyHud::OnRender(UIRenderContext& context)
{
    const auto& snapshot = gateway_.RuntimeSnapshot();
    if (!snapshot.valid || snapshot.entryCount <= 0)
        return;

    auto& renderer = context.Renderer();
    const Rect root = GlobalBounds();
    for (int i = 0; i < snapshot.entryCount; ++i)
    {
        const auto& entry = snapshot.entries[static_cast<std::size_t>(i)];
        Rect row = RowRect(i);
        row.x += root.x;
        row.y += root.y;

        renderer.DrawRect(row, UITheme::Surface);
        DrawBorder(renderer, row);

        const std::string title = entry.name + "  Lv " + std::to_string(entry.level) +
            "  [" + ModeName(entry.mode) + "/" + TacticName(entry.tactic) + "]";
        renderer.DrawTextUtf8(FontHandle{}, title,
            {row.x + 7, row.y + 4, row.width - 14, 16},
            TextStyle{entry.dead ? UITheme::TextMuted : UITheme::Text, true, TextAlign::Left});

        DrawBar(renderer, {row.x + 7, row.y + 23, row.width - 14, 10},
            entry.life, entry.maxLife, Color{174, 72, 82, 255});
        DrawBar(renderer, {row.x + 7, row.y + 36, row.width - 14, 8},
            entry.mana, entry.maxMana, Color{75, 113, 177, 255});

        std::string state;
        if (entry.dead)
            state = "Morto";
        else if (entry.casting)
            state = "Casting";
        else if (!entry.targetName.empty())
            state = "Alvo: " + entry.targetName;
        else
            state = "Sem alvo";
        renderer.DrawTextUtf8(FontHandle{}, state,
            {row.x + 7, row.y + 45, row.width - 14, 11},
            TextStyle{UITheme::TextMuted, false, TextAlign::Left});
    }
}
}
