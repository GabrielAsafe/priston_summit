#include "systems/ui/v2/social/SocialWindow.h"

#include <algorithm>
#include <cstring>

#include "compat/legacy/sin/sinLinkHeader.h"
#include "opcodes.h"
#include "systems/network/v2/legacy/LegacyClientCommandSender.h"
#include "systems/presentation/opening/legacy_runtime/HoLinkHeader.h"
#include "systems/social/party/ui/cPartyWindow.h"
#include "systems/ui/v2/rendering/IUIRenderer.h"
#include "systems/ui/v2/theme/UITheme.h"
#include "systems/ui/v2/widgets/Button.h"
#include "systems/ui/v2/widgets/TextLayout.h"

extern int WhisperPartyPlayer(char* szName);

namespace ui::v2
{
namespace
{
constexpr int RowHeight = 48;

void Border(IUIRenderer& r, const Rect& rect, Color color)
{
    r.DrawRect({rect.x, rect.y, rect.width, 1}, color);
    r.DrawRect({rect.x, rect.y + rect.height - 1, rect.width, 1}, color);
    r.DrawRect({rect.x, rect.y, 1, rect.height}, color);
    r.DrawRect({rect.x + rect.width - 1, rect.y, 1, rect.height}, color);
}

void DrawRatioBar(IUIRenderer& r, const Rect& rect, int current, int maximum, Color fill)
{
    r.DrawRect(rect, UITheme::Slot);
    Border(r, rect, UITheme::BorderSoft);
    if (maximum <= 0 || current <= 0) return;
    const int width = std::clamp((rect.width - 2) * current / maximum, 0, rect.width - 2);
    if (width > 0) r.DrawRect({rect.x + 1, rect.y + 1, width, std::max(1, rect.height - 2)}, fill);
}
}

SocialWindow::SocialWindow(UIId id) : Window(id, "Social")
{
    SetBounds({170, 80, 500, 430});
    SetTitleBarHeight(34);
    SetMovable(true);
    SetClosable(true);

    auto& party = AddChild<Button>(id == InvalidUIId ? InvalidUIId : id + 1, "Grupo");
    party.SetOnClick([this](const UIEvent&) { SetTab(Tab::Party); }); partyTab_ = &party;
    auto& friends = AddChild<Button>(id == InvalidUIId ? InvalidUIId : id + 2, "Amigos");
    friends.SetOnClick([this](const UIEvent&) { SetTab(Tab::Friends); }); friendsTab_ = &friends;
    auto& leave = AddChild<Button>(id == InvalidUIId ? InvalidUIId : id + 3, "Sair do grupo");
    leave.SetOnClick([this](const UIEvent&) { SendPartyCommandForSelection(3); }); leaveButton_ = &leave;
    auto& delegate = AddChild<Button>(id == InvalidUIId ? InvalidUIId : id + 4, "Delegar lider");
    delegate.SetOnClick([this](const UIEvent&) { SendPartyCommandForSelection(0); }); delegateButton_ = &delegate;
    auto& remove = AddChild<Button>(id == InvalidUIId ? InvalidUIId : id + 5, "Remover");
    remove.SetOnClick([this](const UIEvent&) { SendPartyCommandForSelection(2); }); removeButton_ = &remove;
    auto& whisper = AddChild<Button>(id == InvalidUIId ? InvalidUIId : id + 7, "Sussurrar");
    whisper.SetOnClick([this](const UIEvent&) { WhisperSelectedFriend(); }); whisperButton_ = &whisper;
    auto& close = AddChild<Button>(id == InvalidUIId ? InvalidUIId : id + 6, "X");
    close.SetOnClick([this](const UIEvent&) { if (onClose_) onClose_(); }); closeButton_ = &close;

    Layout(); Refresh();
}

void SocialWindow::Layout()
{
    const Rect b = Bounds();
    partyTab_->SetBounds({14, 40, 104, 30});
    friendsTab_->SetBounds({124, 40, 104, 30});
    closeButton_->SetBounds({b.width - 42, 5, 30, 24});
    leaveButton_->SetBounds({14, b.height - 44, 126, 28});
    delegateButton_->SetBounds({146, b.height - 44, 126, 28});
    removeButton_->SetBounds({278, b.height - 44, 100, 28});
    whisperButton_->SetBounds({14, b.height - 44, 126, 28});
    partyTab_->SetSelected(tab_ == Tab::Party);
    friendsTab_->SetSelected(tab_ == Tab::Friends);
}

void SocialWindow::SetViewport(int width, int height)
{
    Rect b = Bounds();
    b.width = std::min(560, std::max(460, width - 40));
    b.height = std::min(520, std::max(390, height - 60));
    b.x = std::max(0, (width - b.width) / 2);
    b.y = std::max(0, (height - b.height) / 2);
    SetBounds(b); Layout();
}

void SocialWindow::SetTab(Tab tab)
{
    tab_ = tab;
    selectedParty_ = -1;
    selectedFriend_ = -1;
    friendScroll_ = 0;
    Refresh(); Layout();
}

void SocialWindow::RefreshFriends()
{
    friends_.clear();
    char id[256] = {0};
    for (int i = 0; i < 100; ++i)
    {
        std::memset(id, 0, sizeof(id));
        if (!InterfaceParty.chat_IDs(1, i, id)) continue;
        if (id[0]) friends_.push_back({id});
    }
}

void SocialWindow::Refresh()
{
    RefreshFriends();
    if (selectedParty_ >= cParty.MemberCount) selectedParty_ = -1;
    if (selectedFriend_ >= static_cast<int>(friends_.size())) selectedFriend_ = -1;
    RefreshActions();
}

void SocialWindow::RefreshActions()
{
    const bool party = tab_ == Tab::Party;
    const bool inParty = cParty.MemberCount > 0;
    leaveButton_->SetVisible(party && inParty);
    leaveButton_->SetEnabled(party && inParty);
    const bool selectedOther = party && cParty.LeaderFlag && selectedParty_ > 0 && selectedParty_ < cParty.MemberCount;
    delegateButton_->SetVisible(selectedOther);
    delegateButton_->SetEnabled(selectedOther);
    removeButton_->SetVisible(selectedOther);
    removeButton_->SetEnabled(selectedOther);

    const bool selectedFriend = !party && selectedFriend_ >= 0 &&
                                selectedFriend_ < static_cast<int>(friends_.size());
    whisperButton_->SetVisible(selectedFriend);
    whisperButton_->SetEnabled(selectedFriend);
}

void SocialWindow::SendPartyCommandForSelection(int command)
{
    if (cParty.MemberCount <= 0) return;
    int index = selectedParty_;
    if (command == 3)
    {
        index = 0;
        for (int i = 0; i < cParty.MemberCount; ++i)
            if (lpCurPlayer && cParty.Members[i].dwObjectSerial == lpCurPlayer->dwObjectSerial) { index = i; break; }
    }
    if (index < 0 || index >= cParty.MemberCount) return;
    smTRANS_COMMAND packet = {};
    packet.size = sizeof(packet);
    packet.code = OpCode::OPCODE_PARTY_COMMAND;
    packet.WParam = cParty.Members[index].dwObjectSerial;
    packet.LParam = command;
    (void)gameplay::network::legacy::ClientLegacyCommandSender().Send(
        gameplay::network::PacketView(&packet, packet.size, static_cast<std::uint32_t>(packet.code)));
}

void SocialWindow::WhisperSelectedFriend()
{
    if (selectedFriend_ < 0 || selectedFriend_ >= static_cast<int>(friends_.size()))
        return;

    std::vector<char> name(friends_[selectedFriend_].name.begin(), friends_[selectedFriend_].name.end());
    name.push_back('\0');
    WhisperPartyPlayer(name.data());
}

UIEventResult SocialWindow::OnEvent(const UIEvent& event)
{
    const Rect root = GlobalBounds();
    const Rect list{root.x + 14, root.y + 80, root.width - 28, root.height - 136};
    if (event.type == UIEventType::Wheel && tab_ == Tab::Friends && list.Contains(event.pointer))
    {
        const int maxScroll = std::max(0, static_cast<int>(friends_.size()) * 30 - list.height + 8);
        const int dir = event.wheelDelta > 0 ? -1 : (event.wheelDelta < 0 ? 1 : 0);
        friendScroll_ = std::clamp(friendScroll_ + dir * 60, 0, maxScroll);
        return UIEventResult::Handled;
    }
    if (event.type == UIEventType::PointerDown && event.button == MouseButton::Left && list.Contains(event.pointer))
    {
        if (tab_ == Tab::Party)
        {
            const int index = (event.pointer.y - list.y) / RowHeight;
            selectedParty_ = (index >= 0 && index < cParty.MemberCount) ? index : -1;
        }
        else
        {
            const int index = (event.pointer.y - list.y + friendScroll_) / 30;
            selectedFriend_ = (index >= 0 && index < static_cast<int>(friends_.size())) ? index : -1;
        }
        RefreshActions();
        return UIEventResult::Handled;
    }
    return Window::OnEvent(event);
}

void SocialWindow::OnRender(UIRenderContext& context)
{
    Window::OnRender(context);
    Refresh();
    const Rect root = GlobalBounds();
    const Rect list{root.x + 14, root.y + 80, root.width - 28, root.height - 136};
    auto& r = context.Renderer();
    r.DrawRect(list, UITheme::Surface);
    Border(r, list, UITheme::BorderSoft);
    r.PushClip({list.x + 1, list.y + 1, list.width - 2, list.height - 2});
    if (tab_ == Tab::Party)
    {
        if (cParty.MemberCount <= 0)
            r.DrawTextUtf8(FontHandle{}, "Voce nao esta em um grupo.", {list.x + 12, list.y + 12, list.width - 24, 20},
                           TextStyle{UITheme::TextMuted, false, TextAlign::Left});
        for (int i = 0; i < cParty.MemberCount; ++i)
        {
            const auto& member = cParty.Members[i];
            const Rect row{list.x + 5, list.y + 5 + i * RowHeight, list.width - 10, RowHeight - 4};
            r.DrawRect(row, i == selectedParty_ ? UITheme::SurfacePressed : UITheme::Slot);
            if (i == 0) r.DrawRect({row.x, row.y, 3, row.height}, UITheme::Accent);
            const std::string name = std::string(member.szName) + "  Nv." + std::to_string(member.Level);
            r.DrawTextUtf8(FontHandle{}, EllipsizeText(name, row.width - 24), {row.x + 10, row.y + 5, row.width - 20, 18},
                           TextStyle{UITheme::Text, i == 0, TextAlign::Left});
            DrawRatioBar(r, {row.x + 10, row.y + 26, (row.width - 30) / 2, 8}, member.Life[0], member.Life[1], UITheme::Positive);
            DrawRatioBar(r, {row.x + 20 + (row.width - 30) / 2, row.y + 26, (row.width - 30) / 2, 8}, member.Mana[0], member.Mana[1], UITheme::Accent);
        }
    }
    else
    {
        if (friends_.empty())
            r.DrawTextUtf8(FontHandle{}, "Nenhum amigo registrado.", {list.x + 12, list.y + 12, list.width - 24, 20},
                           TextStyle{UITheme::TextMuted, false, TextAlign::Left});
        int y = list.y + 5 - friendScroll_;
        for (std::size_t i = 0; i < friends_.size(); ++i, y += 30)
        {
            if (y + 28 < list.y || y > list.y + list.height) continue;
            const Rect row{list.x + 5, y, list.width - 10, 27};
            r.DrawRect(row, static_cast<int>(i) == selectedFriend_ ? UITheme::SurfacePressed : UITheme::Slot);
            r.DrawTextUtf8(FontHandle{}, EllipsizeText(friends_[i].name, row.width - 20), {row.x + 10, row.y + 5, row.width - 20, 18},
                           TextStyle{UITheme::Text, false, TextAlign::Left});
        }
    }
    r.PopClip();
}
}
