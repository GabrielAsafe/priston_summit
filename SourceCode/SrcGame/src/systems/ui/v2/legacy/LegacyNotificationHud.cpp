#include "systems/ui/v2/legacy/LegacyNotificationHud.h"

#include <algorithm>
#include <string>
#include <vector>
#include <windows.h>

#include "compat/legacy/sin/sinLinkHeader.h"
#include "systems/ui/v2/rendering/IUIRenderer.h"
#include "systems/ui/v2/theme/UITheme.h"
#include "systems/ui/v2/widgets/TextLayout.h"

namespace ui::v2::legacy
{
namespace
{
std::string LegacyToUtf8(const char* text)
{
    if (!text || !*text) return {};
    const int wideLen = MultiByteToWideChar(1252, 0, text, -1, nullptr, 0);
    if (wideLen <= 1) return text;
    std::vector<wchar_t> wide(static_cast<std::size_t>(wideLen));
    MultiByteToWideChar(1252, 0, text, -1, wide.data(), wideLen);
    const int utfLen = WideCharToMultiByte(CP_UTF8, 0, wide.data(), -1, nullptr, 0, nullptr, nullptr);
    if (utfLen <= 1) return text;
    std::string out(static_cast<std::size_t>(utfLen), '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide.data(), -1, out.data(), utfLen, nullptr, nullptr);
    if (!out.empty() && out.back() == '\0') out.pop_back();
    return out;
}

void DrawBorder(IUIRenderer& r, const Rect& rect, Color color)
{
    r.DrawRect({rect.x, rect.y, rect.width, 1}, color);
    r.DrawRect({rect.x, rect.y + rect.height - 1, rect.width, 1}, color);
    r.DrawRect({rect.x, rect.y, 1, rect.height}, color);
    r.DrawRect({rect.x + rect.width - 1, rect.y, 1, rect.height}, color);
}

int TextWidthApprox(const std::string& text)
{
    return static_cast<int>(text.size()) * 7;
}

int DrawNotice(UIRenderContext& context, const Rect& viewport, int y, const std::string& text, bool emphasized)
{
    if (text.empty()) return y;

    auto& r = context.Renderer();
    const int width = std::clamp(TextWidthApprox(text) + 48, 260, std::min(620, std::max(260, viewport.width - 32)));
    const auto lines = WrapTextApprox(text, width - 28, 3, 7);
    const int height = 24 + static_cast<int>(std::max<std::size_t>(1, lines.size())) * 18;
    const Rect box{viewport.x + (viewport.width - width) / 2, y, width, height};

    r.DrawRect(box, UITheme::SurfaceRaised);
    DrawBorder(r, box, emphasized ? UITheme::Accent : UITheme::AccentMuted);
    r.DrawRect({box.x, box.y, box.width, 3}, emphasized ? UITheme::Accent : UITheme::AccentMuted);

    for (std::size_t i = 0; i < lines.size(); ++i)
    {
        r.DrawTextUtf8(FontHandle{}, lines[i],
                       {box.x + 14, box.y + 12 + static_cast<int>(i) * 18, box.width - 28, 18},
                       TextStyle{UITheme::Text, emphasized, TextAlign::Center});
    }
    return box.y + box.height + 8;
}
}

LegacyNotificationHud::LegacyNotificationHud(UIId id) : Window(id, {})
{
    SetBounds({0, 0, 800, 600});
    SetMovable(false);
    SetClosable(false);
    SetHitTestVisible(false);
}

void LegacyNotificationHud::SetViewport(int width, int height)
{
    SetBounds({0, 0, width, height});
}

int LegacyNotificationHud::FindActiveMessage()
{
    for (int i = 0; i < 100; ++i)
        if (sMessageBox[i].Flag)
            return i;
    return -1;
}

int LegacyNotificationHud::FindActiveMessage1()
{
    for (int i = 0; i < 100; ++i)
        if (sMessageBox1[i].Flag)
            return i;
    return -1;
}

bool LegacyNotificationHud::HasActiveLegacyNotification()
{
    return FindActiveMessage() >= 0 || FindActiveMessage1() >= 0 ||
           (cHelpPet.MsgFlag && cHelpPet.szPetBuff[0]) || cHelpPet.HelpMsgFlag;
}

void LegacyNotificationHud::OnRender(UIRenderContext& context)
{
    const Rect viewport = GlobalBounds();
    int y = viewport.y + 10;

    const int message = FindActiveMessage();
    if (message >= 0)
    {
        const char* raw = message == MESSAGE_100LVITEM_OK && sMessageBox[message].MessageDoc2[0]
                        ? sMessageBox[message].MessageDoc2
                        : sMessageBox[message].MessageDoc;
        y = DrawNotice(context, viewport, y, LegacyToUtf8(raw), false);
    }

    const int message1 = FindActiveMessage1();
    if (message1 >= 0)
        y = DrawNotice(context, viewport, y, LegacyToUtf8(sMessageBox1[message1].MessageDoc), true);

    // PATCH 183: pet producers remain legacy, presentation is V2-only.
    if (cHelpPet.MsgFlag && cHelpPet.szPetBuff[0])
        y = DrawNotice(context, viewport, y, LegacyToUtf8(cHelpPet.szPetBuff), false);

    if (cHelpPet.HelpMsgFlag)
    {
        const auto it = cHelpPet.PetMsg_Map.find(cHelpPet.HelpMsgId);
        if (it != cHelpPet.PetMsg_Map.end())
        {
            std::string tutorial;
            for (const std::string* line : it->second)
            {
                if (!line || line->empty()) continue;
                if (!tutorial.empty()) tutorial += " ";
                tutorial += *line;
            }
            DrawNotice(context, viewport, y, LegacyToUtf8(tutorial.c_str()), true);
        }
    }
}
}
