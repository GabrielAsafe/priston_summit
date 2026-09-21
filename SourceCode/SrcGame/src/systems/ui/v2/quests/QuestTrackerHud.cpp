#include "systems/ui/v2/quests/QuestTrackerHud.h"

#include <algorithm>
#include <string>

#include "systems/ui/v2/rendering/IUIRenderer.h"
#include "systems/ui/v2/theme/UITheme.h"
#include "systems/ui/v2/widgets/TextLayout.h"

namespace ui::v2
{
QuestTrackerHud::QuestTrackerHud(IQuestGateway& gateway, UIId id)
    : Window(id, "Quest rastreada"), gateway_(gateway)
{
    SetBounds({0, 80, 300, 116});
    SetTitleBarHeight(28);
    SetMovable(false);
    SetClosable(false);
    SetHitTestVisible(false);
    SetVisible(false);
}

void QuestTrackerHud::SetViewport(int width, int)
{
    Rect bounds = Bounds();
    bounds.x = std::max(8, width - bounds.width - 18);
    bounds.y = 74;
    SetBounds(bounds);
}

void QuestTrackerHud::Update(float deltaSeconds)
{
    refreshAccumulator_ += deltaSeconds;
    if (refreshAccumulator_ >= 0.25f)
    {
        refreshAccumulator_ = 0.0f;
        tracked_ = gateway_.GetTrackedQuest();
        SetVisible(tracked_.id != 0);
        if (tracked_.id != 0)
        {
            Rect bounds = Bounds();
            const int objectiveRows = std::max(1, static_cast<int>(tracked_.objectives.size()));
            bounds.height = std::clamp(62 + objectiveRows * 20, 96, 176);
            SetBounds(bounds);
        }
    }
    Window::Update(deltaSeconds);
}

void QuestTrackerHud::OnRender(UIRenderContext& context)
{
    if (tracked_.id == 0)
        return;

    Window::OnRender(context);
    auto& renderer = context.Renderer();
    const Rect root = GlobalBounds();
    const std::string title = EllipsizeText(tracked_.title.empty() ? tracked_.name : tracked_.title, root.width - 24, 7);
    renderer.DrawTextUtf8(FontHandle{}, title, {root.x + 12, root.y + 34, root.width - 24, 18},
                          TextStyle{UITheme::Accent, true, TextAlign::Left});

    int y = root.y + 56;
    if (tracked_.objectives.empty())
    {
        renderer.DrawTextUtf8(FontHandle{}, "Detalhes em atualizacao...", {root.x + 12, y, root.width - 24, 18},
                              TextStyle{UITheme::TextMuted, false, TextAlign::Left});
        return;
    }

    for (const auto& objective : tracked_.objectives)
    {
        const std::string line = std::to_string(objective.current) + "/" + std::to_string(objective.required) + "  " + objective.name;
        renderer.DrawTextUtf8(FontHandle{}, EllipsizeText(line, root.width - 24, 7),
                              {root.x + 12, y, root.width - 24, 18},
                              TextStyle{objective.complete ? UITheme::Positive : UITheme::Text, false, TextAlign::Left});
        y += 20;
        if (y > root.y + root.height - 20)
            break;
    }
}
}
