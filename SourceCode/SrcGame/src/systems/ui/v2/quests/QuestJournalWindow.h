#pragma once

#include <functional>
#include <utility>
#include <vector>

#include "systems/ui/v2/core/Window.h"
#include "systems/ui/v2/domain/IQuestGateway.h"

namespace ui::v2
{
class Button;
class ConfirmationOverlay;

class QuestJournalWindow final : public Window
{
public:
    using CloseHandler = std::function<void()>;

    explicit QuestJournalWindow(IQuestGateway& gateway, UIId id = InvalidUIId);

    void Update(float deltaSeconds) override;
    UIEventResult OnEvent(const UIEvent& event) override;
    void SetViewport(int width, int height);
    void SetOnClose(CloseHandler handler) { onClose_ = std::move(handler); }
    [[nodiscard]] bool HasModalInput() const noexcept;
    bool HandleModalKey(std::uint32_t key);

protected:
    void OnRender(UIRenderContext& context) override;

private:
    enum class JournalTab
    {
        Available,
        Progress
    };

    struct VisibleQuestRow
    {
        int sectionId = 0;
        int questId = 0;
        Rect rect{};
    };

    void BuildChrome();
    void Layout();
    void Refresh();
    void RefreshButtons();
    void SetTab(JournalTab tab);
    [[nodiscard]] const QuestEntrySnapshot* SelectedQuest() const noexcept;
    [[nodiscard]] int MaxListScroll() const noexcept;
    [[nodiscard]] int MaxDetailsScroll() const noexcept;
    void BuildVisibleRows();
    void DrawList(UIRenderContext& context);
    void DrawDetails(UIRenderContext& context);

    IQuestGateway& gateway_;
    QuestJournalSnapshot snapshot_{};
    JournalTab tab_ = JournalTab::Available;
    Button* availableTab_ = nullptr;
    Button* progressTab_ = nullptr;
    Button* acceptButton_ = nullptr;
    Button* cancelButton_ = nullptr;
    Button* rewardButton_ = nullptr;
    Button* trackButton_ = nullptr;
    Button* closeButton_ = nullptr;
    ConfirmationOverlay* confirmation_ = nullptr;
    CloseHandler onClose_;
    float refreshAccumulator_ = 0.0f;
    int listScroll_ = 0;
    int detailsScroll_ = 0;
    std::vector<VisibleQuestRow> visibleRows_;
};
}
