#include "systems/ui/v2/legacy/LegacyNpcDialogWindow.h"

#include <algorithm>
#include <cstring>
#include <string>
#include <vector>
#include <windows.h>
#include "compat/legacy/sin/sinLinkHeader.h"
#include "systems/ui/v2/rendering/IUIRenderer.h"
#include "systems/ui/v2/theme/UITheme.h"
#include "systems/ui/v2/widgets/Button.h"
#include "systems/ui/v2/widgets/TextLayout.h"

namespace ui::v2::legacy
{
namespace
{
std::string LegacyToUtf8(const char* text)
{
    if (!text || !*text) return {};
    const int wideLen = MultiByteToWideChar(CP_ACP, 0, text, -1, nullptr, 0);
    if (wideLen <= 1) return text;
    std::vector<wchar_t> wide(static_cast<std::size_t>(wideLen));
    MultiByteToWideChar(CP_ACP, 0, text, -1, wide.data(), wideLen);
    const int utfLen = WideCharToMultiByte(CP_UTF8, 0, wide.data(), -1, nullptr, 0, nullptr, nullptr);
    if (utfLen <= 1) return text;
    std::string out(static_cast<std::size_t>(utfLen), '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide.data(), -1, out.data(), utfLen, nullptr, nullptr);
    if (!out.empty() && out.back() == '\0') out.pop_back();
    return out;
}
}

LegacyNpcDialogWindow::LegacyNpcDialogWindow(UIId id) : Window(id, "Dialogo")
{
    SetBounds({190, 100, 520, 390});
    SetMovable(true);
    SetClosable(false);

    auto& primary = AddChild<Button>(id == InvalidUIId ? InvalidUIId : id + 1, "Continuar");
    primary.SetBounds({250, 342, 120, 30});
    primary.SetOnClick([this](const UIEvent&) { ActivatePrimary(); });
    primaryButton_ = &primary;

    auto& close = AddChild<Button>(id == InvalidUIId ? InvalidUIId : id + 2, "Fechar");
    close.SetBounds({380, 342, 120, 30});
    close.SetOnClick([this](const UIEvent&) { CloseDialog(); });
    closeButton_ = &close;
}

bool LegacyNpcDialogWindow::SupportedKind(int kind)
{
    if (kind == SIN_HELP_KIND_INITPOINT)
        return sMessageBox3[MESSAGE_EVENT_GYUNGFUM].Flag != 0;

    switch (kind)
    {
        case SIN_HELP_KIND_SOD:
        case SIN_HELP_KIND_WARPGATE:
        case SIN_HELP_KIND_TELEPORT:
        case SIN_HELP_KIND_C_TELEPORT:
        case SIN_HELP_KIND_RESEARCH:
        case SIN_HELP_KIND_TELEPORT_SCROLL:
        case SIN_HELP_KIND_TELEPORT_MILTER:
        case SIN_HELP_KIND_TRADEBPEXP:
            return false;
        default:
            return kind != 0;
    }
}

int LegacyNpcDialogWindow::FindActiveIndex()
{
    for (int i = 0; i < SIN_MAX_HELP_NUMBER; ++i)
        if (sSinHelp[i].KindFlag && SupportedKind(sSinHelp[i].KindFlag))
            return i;
    return -1;
}

bool LegacyNpcDialogWindow::HasActiveSupportedLegacyDialog()
{
    return FindActiveIndex() >= 0;
}

bool LegacyNpcDialogWindow::HasActiveEventGiftDialog()
{
    if (!sMessageBox3[MESSAGE_EVENT_GYUNGFUM].Flag)
        return false;
    for (int i = 0; i < SIN_MAX_HELP_NUMBER; ++i)
        if (sSinHelp[i].KindFlag == SIN_HELP_KIND_INITPOINT)
            return true;
    return false;
}

const char* LegacyNpcDialogWindow::TitleFor(int kind)
{
    switch (kind)
    {
        case SIN_HELP_KIND_QUEST:
        case SIN_HELP_KIND_QUEST_WING:
        case SIN_HELP_KIND_Q_LEVEL30:
        case SIN_HELP_KIND_Q_LEVEL55:
        case SIN_HELP_KIND_Q_LEVEL55_2:
        case SIN_HELP_KIND_Q_LEVEL70:
        case SIN_HELP_KIND_Q_LEVEL80:
        case SIN_HELP_KIND_Q_LEVEL85:
        case SIN_HELP_KIND_Q_LEVEL90:
        case SIN_HELP_KIND_Q_LEVEL80_2:
        case SIN_HELP_KIND_Q_LEVEL90_2:
            return "Missao";
        case SIN_HELP_KIND_SMASHTV:
        case SIN_HELP_KIND_INITPOINT:
            return "Evento";
        default:
            return "Dialogo NPC";
    }
}

void LegacyNpcDialogWindow::Refresh()
{
    const int next = FindActiveIndex();
    if (next != activeIndex_)
    {
        activeIndex_ = next;
        scrollLine_ = 0;
    }
    if (activeIndex_ < 0)
        return;

    const sSINHELP& help = sSinHelp[activeIndex_];
    SetTitle(TitleFor(help.KindFlag));
    if (primaryButton_)
    {
        const bool eventGift = help.KindFlag == SIN_HELP_KIND_INITPOINT &&
                               sMessageBox3[MESSAGE_EVENT_GYUNGFUM].Flag;
        primaryButton_->SetVisible(eventGift || help.ButtonShowFlag != 0 ||
                                   help.ButtonPosi.x != 0 || help.ButtonPosi.y != 0);
        primaryButton_->SetText(eventGift ? "Confirmar" : "Continuar");
        primaryButton_->SetBounds({250, Bounds().height - 48, 120, 30});
    }
    if (closeButton_)
        closeButton_->SetBounds({380, Bounds().height - 48, 120, 30});
}

void LegacyNpcDialogWindow::ActivatePrimary()
{
    if (activeIndex_ < 0) return;
    sSINHELP& help = sSinHelp[activeIndex_];
    if (help.KindFlag == SIN_HELP_KIND_INITPOINT &&
        sMessageBox3[MESSAGE_EVENT_GYUNGFUM].Flag)
    {
        cMessageBox.ResolveMessage3ForUIV2(MESSAGE_EVENT_GYUNGFUM, true);
        Refresh();
        return;
    }
    const int x = help.ButtonPosi.x ? help.ButtonPosi.x : help.PosiX + 80;
    const int y = help.ButtonPosi.y ? help.ButtonPosi.y : help.PosiY + help.SizeH * 32 - 30;

    // Several legacy NPC handlers inspect the global cursor instead of their
    // x/y parameters. Temporarily emulate a click on the original OK button so
    // the exact quest/event backend executes without exposing the old chrome.
    const POINT savedCursor = pCursorPos;
    pCursorPos.x = x + 24;
    pCursorPos.y = y + 12;
    cSinHelp.LButtonDown(pCursorPos.x, pCursorPos.y);
    pCursorPos = savedCursor;
    Refresh();
}

void LegacyNpcDialogWindow::CloseDialog()
{
    if (activeIndex_ < 0) return;
    if (sSinHelp[activeIndex_].KindFlag == SIN_HELP_KIND_INITPOINT &&
        sMessageBox3[MESSAGE_EVENT_GYUNGFUM].Flag)
    {
        cMessageBox.ResolveMessage3ForUIV2(MESSAGE_EVENT_GYUNGFUM, false);
        Refresh();
        return;
    }
    std::memset(&sSinHelp[activeIndex_], 0, sizeof(sSINHELP));
    HelpBoxKindIndex = 0;
    cSinHelp.NPCTeleportFlag = 0;
    activeIndex_ = -1;
    scrollLine_ = 0;
}

void LegacyNpcDialogWindow::Update(float deltaSeconds)
{
    Refresh();
    Window::Update(deltaSeconds);
}

UIEventResult LegacyNpcDialogWindow::OnEvent(const UIEvent& event)
{
    if (activeIndex_ >= 0 && event.type == UIEventType::Wheel)
    {
        const int lineCount = std::max(0, sSinHelp[activeIndex_].LineCnt);
        const int maxScroll = std::max(0, lineCount - 15);
        if (event.wheelDelta > 0) scrollLine_ = std::max(0, scrollLine_ - 2);
        if (event.wheelDelta < 0) scrollLine_ = std::min(maxScroll, scrollLine_ + 2);
        return UIEventResult::Handled;
    }
    return Window::OnEvent(event);
}

void LegacyNpcDialogWindow::OnRender(UIRenderContext& context)
{
    Window::OnRender(context);
    if (activeIndex_ < 0) return;

    const sSINHELP& help = sSinHelp[activeIndex_];
    const Rect root = GlobalBounds();
    auto& r = context.Renderer();
    const int maxLines = 15;
    int visualLine = 0;
    for (int i = scrollLine_; i < help.LineCnt && visualLine < maxLines; ++i)
    {
        if (!help.szDocBuff[i]) continue;
        const auto wrapped = WrapTextApprox(LegacyToUtf8(help.szDocBuff[i]), root.width - 40, 2, 7);
        for (const auto& line : wrapped)
        {
            if (visualLine >= maxLines) break;
            r.DrawTextUtf8(FontHandle{}, line,
                           {root.x + 20, root.y + 44 + visualLine * 18, root.width - 40, 18},
                           TextStyle{UITheme::Text, false, TextAlign::Left});
            ++visualLine;
        }
    }
    if (help.LineCnt > maxLines)
        r.DrawTextUtf8(FontHandle{}, "Use a roda do mouse para rolar",
                       {root.x + 20, root.y + root.height - 74, root.width - 40, 18},
                       TextStyle{UITheme::TextMuted, false, TextAlign::Left});
}
}
