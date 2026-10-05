#include "systems/ui/v2/social/PartyStatusHud.h"

#include <algorithm>
#include <string>

#include "systems/social/party/ui/cPartyWindow.h"
#include "systems/ui/v2/rendering/IUIRenderer.h"
#include "systems/ui/v2/theme/UITheme.h"
#include "systems/ui/v2/widgets/TextLayout.h"

namespace ui::v2
{
namespace
{
constexpr int RowHeight = 45;
constexpr int HudWidth = 220;

void Border(IUIRenderer& r, const Rect& rect)
{
    r.DrawRect({rect.x, rect.y, rect.width, 1}, UITheme::BorderSoft);
    r.DrawRect({rect.x, rect.y + rect.height - 1, rect.width, 1}, UITheme::BorderSoft);
    r.DrawRect({rect.x, rect.y, 1, rect.height}, UITheme::BorderSoft);
    r.DrawRect({rect.x + rect.width - 1, rect.y, 1, rect.height}, UITheme::BorderSoft);
}

void Bar(IUIRenderer& r, const Rect& rect, int value, int maximum, Color fill)
{
    r.DrawRect(rect, UITheme::Slot);
    Border(r, rect);
    if (maximum <= 0 || value <= 0) return;
    const int width = std::clamp((rect.width - 2) * value / maximum, 0, rect.width - 2);
    if (width > 0)
        r.DrawRect({rect.x + 1, rect.y + 1, width, std::max(1, rect.height - 2)}, fill);
}
}

PartyStatusHud::PartyStatusHud(UIId id) : Window(id, {})
{
    SetTitleBarHeight(0);
    SetMovable(false);
    SetClosable(false);
    SetBounds({10, 150, HudWidth, RowHeight * 6});
}

void PartyStatusHud::SetViewport(int width, int height)
{
    const int maxHeight = RowHeight * 6;
    SetBounds({10, std::max(84, (height - maxHeight) / 2), std::min(HudWidth, std::max(190, width / 4)), maxHeight});
}

void PartyStatusHud::OnRender(UIRenderContext& context)
{
    if (cParty.MemberCount <= 0) return;
    auto& r = context.Renderer();
    const Rect root = GlobalBounds();
    const int count = std::clamp(cParty.MemberCount, 0, 6);
    for (int i = 0; i < count; ++i)
    {
        const auto& member = cParty.Members[i];
        const Rect row{root.x, root.y + i * RowHeight, root.width, RowHeight - 3};
        r.DrawRect(row, UITheme::Surface);
        Border(r, row);
        if (i == 0) r.DrawRect({row.x, row.y, 3, row.height}, UITheme::Accent);

        const std::string name = std::string(member.szName) + "  Nv. " + std::to_string(member.Level);
        r.DrawTextUtf8(FontHandle{}, EllipsizeText(name, row.width - 18),
                       {row.x + 8, row.y + 4, row.width - 16, 17},
                       TextStyle{UITheme::Text, i == 0, TextAlign::Left});
        Bar(r, {row.x + 8, row.y + 24, row.width - 16, 7}, member.Life[0], member.Life[1], UITheme::Positive);
        Bar(r, {row.x + 8, row.y + 33, row.width - 16, 5}, member.Mana[0], member.Mana[1], UITheme::Accent);
    }
}
}
