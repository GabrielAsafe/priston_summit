#include <algorithm>
#include <cassert>
#include <memory>
#include <string>
#include <vector>

#include "systems/ui/v2/core/UIRuntime.h"
#include "systems/ui/v2/domain/IQuestGateway.h"
#include "systems/ui/v2/quests/QuestJournalWindow.h"
#include "systems/ui/v2/quests/QuestTrackerHud.h"
#include "systems/ui/v2/rendering/IUIRenderer.h"

using namespace ui::v2;

class FakeQuestRenderer final : public IUIRenderer
{
public:
    void Begin() override { ++begins; }
    void End() override { ++ends; }
    void DrawImage(TextureHandle, const Rect&, const Rect*) override {}
    void DrawRect(const Rect&, Color) override { ++rects; }
    void DrawTextUtf8(FontHandle, std::string_view text, const Rect&, const TextStyle&) override
    {
        texts.emplace_back(text);
    }
    void PushClip(const Rect&) override {}
    void PopClip() override {}

    bool HasText(const std::string& text) const
    {
        return std::find(texts.begin(), texts.end(), text) != texts.end();
    }

    void ClearText() { texts.clear(); }

    int begins = 0;
    int ends = 0;
    int rects = 0;
    std::vector<std::string> texts;
};

class FakeQuestGateway final : public IQuestGateway
{
public:
    FakeQuestGateway()
    {
        QuestSectionSnapshot quests;
        quests.id = 9;
        quests.title = "Quests";

        QuestEntrySnapshot available;
        available.id = 101;
        available.kind = 1;
        available.sectionId = 9;
        available.title = "Primeira Jornada";
        available.name = "Primeira Jornada";
        available.description = "Fale com o guarda, explore a area e conclua a tarefa inicial para testar o diario V2.";
        available.levelMin = 1;
        available.levelMax = 10;
        available.active = true;
        available.objectives.push_back({"Guardas", "Ricarten", 0, 1, false});
        available.rewards.push_back({"Pocao", 2});
        available.rewardGold = 1000;
        quests.quests.push_back(available);

        QuestSectionSnapshot emptyDaily;
        emptyDaily.id = 8;
        emptyDaily.title = "Diarias";

        QuestSectionSnapshot progress;
        progress.id = 10;
        progress.title = "Em andamento";
        progress.progressGroup = true;

        QuestEntrySnapshot running;
        running.id = 202;
        running.kind = 2;
        running.sectionId = 10;
        running.title = "Cacada aos Lobos";
        running.name = "Cacada aos Lobos";
        running.description = "Derrote lobos proximos a cidade.";
        running.active = true;
        running.objectives.push_back({"Lobos", "Floresta", 2, 5, false});
        running.rewardExp = 2500;
        progress.quests.push_back(running);

        QuestSectionSnapshot finalized;
        finalized.id = 11;
        finalized.title = "Finalizadas";
        finalized.progressGroup = true;
        finalized.finalizedGroup = true;

        snapshot.valid = true;
        snapshot.sections = {quests, emptyDaily, progress, finalized};
    }

    QuestJournalSnapshot GetSnapshot() const override
    {
        QuestJournalSnapshot out = snapshot;
        out.selectedSectionId = selectedSection;
        out.selectedQuestId = selectedQuest;
        out.trackedSectionId = trackedSection;
        out.trackedQuestId = trackedQuest;
        for (auto& section : out.sections)
            for (auto& quest : section.quests)
            {
                quest.selected = section.id == selectedSection && quest.id == selectedQuest;
                quest.tracked = section.id == trackedSection && quest.id == trackedQuest;
            }
        return out;
    }

    QuestEntrySnapshot GetTrackedQuest() const override
    {
        if (!trackedQuest)
            return {};
        for (const auto& section : snapshot.sections)
            if (section.id == trackedSection)
                for (auto quest : section.quests)
                    if (quest.id == trackedQuest)
                    {
                        quest.tracked = true;
                        return quest;
                    }
        return {};
    }

    bool SelectQuest(int sectionId, int questId) override
    {
        for (const auto& section : snapshot.sections)
            if (section.id == sectionId)
                for (const auto& quest : section.quests)
                    if (quest.id == questId)
                    {
                        selectedSection = sectionId;
                        selectedQuest = questId;
                        ++selects;
                        return true;
                    }
        return false;
    }

    bool RequestAcceptSelected() override
    {
        if (selectedSection != 9 || selectedQuest != 101)
            return false;
        ++accepts;
        return true;
    }

    bool RequestCancelSelected() override
    {
        if (selectedSection != 10 || selectedQuest != 202)
            return false;
        ++cancels;
        return true;
    }

    bool ClaimSelectedReward() override
    {
        ++rewards;
        return true;
    }

    bool TrackSelected() override
    {
        if (selectedSection != 10 || selectedQuest != 202)
            return false;
        trackedSection = selectedSection;
        trackedQuest = selectedQuest;
        ++tracks;
        return true;
    }

    mutable QuestJournalSnapshot snapshot;
    int selectedSection = 0;
    int selectedQuest = 0;
    int trackedSection = 0;
    int trackedQuest = 0;
    int selects = 0;
    int accepts = 0;
    int cancels = 0;
    int rewards = 0;
    int tracks = 0;
};

static void Click(UIRuntime& runtime, int x, int y)
{
    UIEvent down;
    down.type = UIEventType::PointerDown;
    down.button = MouseButton::Left;
    down.pointer = {x, y};
    UIEvent up = down;
    up.type = UIEventType::PointerUp;
    assert(runtime.HandleEvent(down));
    assert(runtime.HandleEvent(up));
}

int main()
{
    FakeQuestGateway gateway;
    auto renderer = std::make_unique<FakeQuestRenderer>();
    auto* rawRenderer = renderer.get();

    UIRuntime runtime;
    assert(runtime.Initialize(std::move(renderer)));

    auto journal = std::make_unique<QuestJournalWindow>(gateway, 100);
    auto* rawJournal = journal.get();
    rawJournal->SetViewport(1024, 768);
    constexpr WindowId JournalId = 1000;
    assert(runtime.Windows().Register(JournalId, std::move(journal), UIWindowLayer::NormalWindow));
    assert(runtime.Windows().Open(JournalId));

    auto tracker = std::make_unique<QuestTrackerHud>(gateway, 101);
    tracker->SetViewport(1024, 768);
    constexpr WindowId TrackerId = 1001;
    assert(runtime.Windows().Register(TrackerId, std::move(tracker), UIWindowLayer::Hud));
    assert(runtime.Windows().Open(TrackerId));

    runtime.Update(0.30f);
    runtime.Render();
    assert(rawRenderer->begins == 1 && rawRenderer->ends == 1 && rawRenderer->rects > 5);
    assert(rawRenderer->HasText("Diario de Quests"));
    assert(rawRenderer->HasText("Disponiveis"));
    assert(rawRenderer->HasText("Quests"));
    assert(rawRenderer->HasText("Primeira Jornada"));
    assert(rawRenderer->HasText("Nenhuma quest."));

    // SetViewport(1024x768) yields an 820x570 window at (102,99). The first
    // available quest starts below the section header at local y=106.
    Click(runtime, 130, 218);
    assert(gateway.selects == 1);
    assert(gateway.selectedSection == 9 && gateway.selectedQuest == 101);
    rawRenderer->ClearText();
    runtime.Render();
    assert(rawRenderer->HasText("OBJETIVOS"));
    assert(rawRenderer->HasText("RECOMPENSAS"));

    // Accept action goes through IQuestGateway; the V2 does not mutate the
    // underlying quest object directly.
    Click(runtime, 102 + 388 + 50, 99 + 528 + 14);
    assert(rawJournal->HasModalInput());
    assert(gateway.accepts == 0);
    assert(rawJournal->HandleModalKey(13));
    assert(!rawJournal->HasModalInput());
    assert(gateway.accepts == 1);

    // Switch to in-progress quests, select one and track it.
    Click(runtime, 102 + 148 + 64, 99 + 40 + 15);
    rawRenderer->ClearText();
    runtime.Render();
    assert(rawRenderer->HasText("Em andamento"));
    assert(rawRenderer->HasText("Cacada aos Lobos"));
    assert(rawRenderer->HasText("Finalizadas"));

    Click(runtime, 130, 218);
    assert(gateway.selectedSection == 10 && gateway.selectedQuest == 202);
    Click(runtime, 102 + 660 + 48, 99 + 40 + 15);
    assert(gateway.tracks == 1 && gateway.trackedQuest == 202);

    // Tracker refreshes independently of the journal and remains available
    // when the journal itself is closed by the shell.
    runtime.Update(0.30f);
    rawRenderer->ClearText();
    runtime.Render();
    assert(rawRenderer->HasText("Quest rastreada"));
    assert(rawRenderer->HasText("Cacada aos Lobos"));
    assert(rawRenderer->HasText("2/5  Lobos"));

    // Both list and detail panes own wheel input so the world never receives
    // the scroll while the journal is under the pointer.
    UIEvent listWheel;
    listWheel.type = UIEventType::Wheel;
    listWheel.pointer = {130, 260};
    listWheel.wheelDelta = -120;
    assert(runtime.HandleEvent(listWheel));

    UIEvent detailWheel = listWheel;
    detailWheel.pointer = {500, 300};
    assert(runtime.HandleEvent(detailWheel));

    // Closing the V2 window must not destroy the independent tracked state.
    assert(runtime.Windows().Close(JournalId));
    runtime.Update(0.30f);
    rawRenderer->ClearText();
    runtime.Render();
    assert(!rawRenderer->HasText("Diario de Quests"));
    assert(rawRenderer->HasText("Quest rastreada"));
    assert(rawRenderer->HasText("2/5  Lobos"));

    runtime.Shutdown();
    return 0;
}
