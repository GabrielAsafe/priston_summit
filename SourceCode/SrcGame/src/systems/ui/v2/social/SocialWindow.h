#pragma once

#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "systems/ui/v2/core/Window.h"

namespace ui::v2
{
class Button;

class SocialWindow final : public Window
{
public:
    using CloseHandler = std::function<void()>;
    enum class Tab { Party, Friends };

    explicit SocialWindow(UIId id = InvalidUIId);
    void SetViewport(int width, int height);
    void SetOnClose(CloseHandler handler) { onClose_ = std::move(handler); }
    void SetTab(Tab tab);
    void Refresh();
    UIEventResult OnEvent(const UIEvent& event) override;

protected:
    void OnRender(UIRenderContext& context) override;

private:
    struct FriendEntry { std::string name; };
    void Layout();
    void RefreshFriends();
    void SendPartyCommandForSelection(int command);
    void WhisperSelectedFriend();
    void RefreshActions();

    Tab tab_ = Tab::Party;
    std::vector<FriendEntry> friends_;
    int selectedParty_ = -1;
    int selectedFriend_ = -1;
    int friendScroll_ = 0;
    Button* partyTab_ = nullptr;
    Button* friendsTab_ = nullptr;
    Button* leaveButton_ = nullptr;
    Button* delegateButton_ = nullptr;
    Button* removeButton_ = nullptr;
    Button* whisperButton_ = nullptr;
    Button* closeButton_ = nullptr;
    CloseHandler onClose_;
};
}
