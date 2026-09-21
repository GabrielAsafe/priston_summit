#include "systems/ui/v2/legacy/LegacyMessageWindow.h"

#include <string>
#include <vector>
#include <windows.h>
#include "compat/legacy/sin/sinLinkHeader.h"
#include "systems/ui/messages/legacy_sin/sinMsg.h"
#include "systems/ui/v2/widgets/ConfirmationOverlay.h"

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

std::string JoinMessage(const sMESSAGEBOX3& message)
{
    std::string out;
    if (message.MessageDoc1[0]) out = LegacyToUtf8(message.MessageDoc1);
    if (message.MessageDoc2[0])
    {
        if (!out.empty()) out += " ";
        out += LegacyToUtf8(message.MessageDoc2);
    }
    if (message.RecvMessage[0])
    {
        const std::string recv = LegacyToUtf8(message.RecvMessage);
        if (out.find(recv) == std::string::npos)
        {
            if (!out.empty()) out += "\n";
            out += recv;
        }
    }
    return out.empty() ? "Deseja continuar?" : out;
}
}

LegacyMessageWindow::LegacyMessageWindow(UIId id) : Window(id, "Confirmacao")
{
    SetMovable(false);
    SetClosable(false);
    SetBounds({0, 0, 800, 600});

    auto& confirmation = AddChild<ConfirmationOverlay>(id == InvalidUIId ? InvalidUIId : id + 1);
    confirmation.SetBounds({0, 0, 800, 600});
    confirmation.SetDimBackground(false);
    confirmation.SetZOrder(100);
    confirmation_ = &confirmation;
}

bool LegacyMessageWindow::SupportedKind(int kind)
{
    // Secret-number confirmation owns a Win32 text edit and remains legacy
    // until a text-input replacement is available. All other Message3 entries
    // are binary confirmations and can safely reuse the original resolver.
    return kind >= 0 && kind < MESSAGE3_MAX &&
           kind != MESSAGE_SECRET_NUM &&
           kind != MESSAGE_EVENT_GYUNGFUM;
}

int LegacyMessageWindow::FindActiveKind()
{
    for (int kind = 0; kind < MESSAGE3_MAX; ++kind)
        if (sMessageBox3[kind].Flag && SupportedKind(kind))
            return kind;
    return -1;
}

bool LegacyMessageWindow::HasActiveSupportedLegacyMessage()
{
    return FindActiveKind() >= 0;
}

std::string LegacyMessageWindow::TitleFor(int kind)
{
    switch (kind)
    {
        case MESSAGE_SELL_HIGHRANK_ITEM: return "Confirmar venda";
        case MESSAGE_MASTER_SKILL: return "Mestre de habilidades";
        case MESSAGE_INIT_SKILL_POINT: return "Redistribuir habilidades";
        case MESSAGE_CHANGE_JOB:
        case MESSAGE_CHANGE_JOB4:
        case MESSAGE_CHANGE_JOB4_2: return "Mudanca de classe";
        case MESSAGE_EVENT_SMASHTV:
        case MESSAGE_EVENT_GYUNGFUM:
        case MESSAGE_EVENTITEM_SEND:
        case MESSAGE_FOXEVENTITEM_SEND: return "Evento";
        case MESSAGE_QUEST:
        case MESSAGE_QUEST_CHANGEJOB_MORAYOIN:
        case MESSAGE_QUEST_CHANGEJOB3:
        case MESSAGE_QUEST_LEVEL30:
        case MESSAGE_QUEST_LEVEL55:
        case MESSAGE_QUEST_LEVEL55_2:
        case MESSAGE_QUEST_LEVEL70:
        case MESSAGE_QUEST_LEVEL80:
        case MESSAGE_QUEST_LEVEL85:
        case MESSAGE_QUEST_LEVEL90:
        case MESSAGE_QUEST_LEVEL80_2:
        case MESSAGE_QUEST_LEVEL90_2:
        case MESSAGE_QUEST_100LV:
        case MESSAGE_QUEST_100LVITEMOK:
        case MESSAGE_QUEST_100LVBATTLEOK:
        case MESSAGE_QUEST_START:
        case MESSAGE_QUEST_CANCEL: return "Missao";
        default: return "Confirmacao";
    }
}

std::string LegacyMessageWindow::MessageFor(int kind)
{
    return JoinMessage(sMessageBox3[kind]);
}

void LegacyMessageWindow::Refresh()
{
    const int kind = FindActiveKind();
    if (kind < 0)
    {
        activeKind_ = -1;
        if (confirmation_ && confirmation_->IsOpen()) confirmation_->Hide();
        return;
    }
    if (kind == activeKind_ && confirmation_ && confirmation_->IsOpen())
        return;

    activeKind_ = kind;
    SetTitle(TitleFor(kind));
    if (confirmation_)
    {
        confirmation_->Show(TitleFor(kind), MessageFor(kind),
                            [this]() { Resolve(true); },
                            [this]() { Resolve(false); });
    }
}

void LegacyMessageWindow::Resolve(bool accepted)
{
    const int kind = activeKind_;
    activeKind_ = -1;
    if (kind >= 0)
        cMessageBox.ResolveMessage3ForUIV2(kind, accepted);
    Refresh();
}

void LegacyMessageWindow::Update(float deltaSeconds)
{
    Refresh();
    if (confirmation_)
        confirmation_->SetBounds({0, 0, Bounds().width, Bounds().height});
    Window::Update(deltaSeconds);
}


void LegacyMessageWindow::OnRender(UIRenderContext&)
{
    // This window intentionally owns the full viewport only for modal input.
    // Rendering its Window chrome here used to paint an opaque 800x600 panel
    // behind every migrated Message3 (most visibly the Event Girl skill reset).
    // The child ConfirmationOverlay draws only the compact card.
}

bool LegacyMessageWindow::HasModalInput() const noexcept
{
    return confirmation_ && confirmation_->IsOpen();
}

bool LegacyMessageWindow::HandleModalKey(std::uint32_t key)
{
    return confirmation_ && confirmation_->IsOpen() ? confirmation_->HandleKey(key) : false;
}
}
