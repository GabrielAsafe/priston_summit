#include "systems/ui/v2/help/GuideWindow.h"

#include <algorithm>
#include <cctype>
#include <windows.h>

#include "content/v1/text_resource/content/TextResourceRuntime.h"
#include "systems/ui/v2/rendering/IUIRenderer.h"
#include "systems/ui/v2/theme/UITheme.h"
#include "systems/ui/v2/widgets/Button.h"
#include "systems/ui/v2/widgets/TextLayout.h"

namespace ui::v2
{
namespace
{
constexpr int LineHeight = 19;

std::string Cp1252ToUtf8(const std::string& input)
{
    if (input.empty()) return {};
    const int wideLen = MultiByteToWideChar(1252, 0, input.data(), static_cast<int>(input.size()), nullptr, 0);
    if (wideLen <= 0) return input;
    std::vector<wchar_t> wide(static_cast<std::size_t>(wideLen));
    MultiByteToWideChar(1252, 0, input.data(), static_cast<int>(input.size()), wide.data(), wideLen);
    const int utfLen = WideCharToMultiByte(CP_UTF8, 0, wide.data(), wideLen, nullptr, 0, nullptr, nullptr);
    if (utfLen <= 0) return input;
    std::string out(static_cast<std::size_t>(utfLen), '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide.data(), wideLen, out.data(), utfLen, nullptr, nullptr);
    return out;
}

std::string Trim(std::string value)
{
    while (!value.empty() && (value.back() == '\r' || value.back() == '\n' || value.back() == ' ' || value.back() == '\t'))
        value.pop_back();
    std::size_t begin = 0;
    while (begin < value.size() && (value[begin] == ' ' || value[begin] == '\t')) ++begin;
    return value.substr(begin);
}

void Border(IUIRenderer& r, const Rect& rect)
{
    r.DrawRect({rect.x, rect.y, rect.width, 1}, UITheme::BorderSoft);
    r.DrawRect({rect.x, rect.y + rect.height - 1, rect.width, 1}, UITheme::BorderSoft);
    r.DrawRect({rect.x, rect.y, 1, rect.height}, UITheme::BorderSoft);
    r.DrawRect({rect.x + rect.width - 1, rect.y, 1, rect.height}, UITheme::BorderSoft);
}
}

GuideWindow::GuideWindow(UIId id) : Window(id, "Guia do Jogo")
{
    SetBounds({90, 55, 720, 520});
    SetTitleBarHeight(34);
    SetMovable(true);
    SetClosable(true);
    auto& close = AddChild<Button>(id == InvalidUIId ? InvalidUIId : id + 1, "Fechar");
    close.SetOnClick([this](const UIEvent&) { if (onClose_) onClose_(); });
    closeButton_ = &close;
    Layout();
    Reload();
}

void GuideWindow::Layout()
{
    const Rect b = Bounds();
    if (closeButton_) closeButton_->SetBounds({b.width - 104, 5, 90, 25});
}

void GuideWindow::SetViewport(int width, int height)
{
    Rect b = Bounds();
    b.width = std::min(760, std::max(620, width - 30));
    b.height = std::min(600, std::max(460, height - 30));
    b.x = std::max(0, (width - b.width) / 2);
    b.y = std::max(0, (height - b.height) / 2);
    SetBounds(b);
    Layout();
    scroll_ = std::clamp(scroll_, 0, MaxScroll());
}

void GuideWindow::Reload()
{
    lines_.clear();
    std::vector<unsigned char> bytes;
    if (!summit::content::text_resource::LoadTextResourceBytes("image/Sinimage/help/Help.sin", bytes))
    {
        lines_.push_back({"Conteudo do guia indisponivel. Verifique Content/packs/text_resources.sres.", false});
        return;
    }

    std::size_t pos = 0;
    while (pos <= bytes.size())
    {
        std::size_t end = pos;
        while (end < bytes.size() && bytes[end] != '\r' && bytes[end] != '\n') ++end;
        std::string raw(bytes.begin() + pos, bytes.begin() + end);
        raw = Trim(raw);
        if (!raw.empty())
        {
            const bool heading = raw[0] == '*';
            if (heading) raw = Trim(raw.substr(1));

            // The canonical legacy Help.sin is already PT-BR except for two
            // surviving presentation strings. Normalize them at the V2 edge
            // without mutating/re-encoding the canonical legacy payload.
            if (heading && raw == "Keyboard")
                raw = "Teclado";
            if (raw == "(CH's name will be appeared in chat box)")
            {
                if (end >= bytes.size()) break;
                pos = end + 1;
                if (end + 1 < bytes.size() && bytes[end] == '\r' && bytes[end + 1] == '\n') ++pos;
                continue;
            }
            lines_.push_back({Cp1252ToUtf8(raw), heading});
        }
        else
            lines_.push_back({"", false});
        if (end >= bytes.size()) break;
        pos = end + 1;
        if (end + 1 < bytes.size() && bytes[end] == '\r' && bytes[end + 1] == '\n') ++pos;
    }
    scroll_ = 0;
}

int GuideWindow::MaxScroll() const noexcept
{
    const int contentHeight = std::max(1, Bounds().height - 60);
    int visualLines = 0;
    const int width = std::max(120, Bounds().width - 48);
    for (const auto& line : lines_)
    {
        if (line.text.empty()) { ++visualLines; continue; }
        visualLines += std::max<int>(1, static_cast<int>(WrapTextApprox(line.text, width, 40, 7).size()));
    }
    return std::max(0, visualLines * LineHeight - contentHeight + 12);
}

UIEventResult GuideWindow::OnEvent(const UIEvent& event)
{
    if (event.type == UIEventType::Wheel && HitTest(event.pointer))
    {
        const int dir = event.wheelDelta > 0 ? -1 : (event.wheelDelta < 0 ? 1 : 0);
        scroll_ = std::clamp(scroll_ + dir * LineHeight * 3, 0, MaxScroll());
        return UIEventResult::Handled;
    }
    return Window::OnEvent(event);
}

void GuideWindow::OnRender(UIRenderContext& context)
{
    Window::OnRender(context);
    const Rect root = GlobalBounds();
    const Rect body{root.x + 14, root.y + 42, std::max(1, root.width - 28), std::max(1, root.height - 54)};
    auto& r = context.Renderer();
    r.DrawRect(body, UITheme::Surface);
    Border(r, body);
    r.PushClip({body.x + 1, body.y + 1, body.width - 2, body.height - 2});
    int y = body.y + 10 - scroll_;
    const int width = body.width - 24;
    for (const auto& line : lines_)
    {
        if (line.text.empty()) { y += LineHeight / 2; continue; }
        const auto wrapped = WrapTextApprox(line.text, width, 40, 7);
        for (const auto& visual : wrapped)
        {
            if (y + LineHeight >= body.y && y <= body.y + body.height)
                r.DrawTextUtf8(FontHandle{}, visual, {body.x + 12, y, width, LineHeight},
                               TextStyle{line.heading ? UITheme::Accent : UITheme::Text,
                                         line.heading, TextAlign::Left});
            y += line.heading ? LineHeight + 4 : LineHeight;
        }
        if (line.heading) y += 3;
    }
    r.PopClip();
}
}
