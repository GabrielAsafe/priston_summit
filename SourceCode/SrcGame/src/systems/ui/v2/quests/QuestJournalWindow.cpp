#include "systems/ui/v2/quests/QuestJournalWindow.h"

#include <algorithm>
#include <string>
#include <vector>

#include "systems/ui/v2/rendering/IUIRenderer.h"
#include "systems/ui/v2/theme/UITheme.h"
#include "systems/ui/v2/widgets/Button.h"
#include "systems/ui/v2/widgets/ConfirmationOverlay.h"
#include "systems/ui/v2/widgets/TextLayout.h"

namespace ui::v2
{
namespace
{
constexpr int SectionHeaderHeight = 24;
constexpr int QuestRowHeight = 28;
constexpr int DetailLineHeight = 18;

struct DetailVisualLine
{
    std::string text;
    Color color = UITheme::Text;
    bool bold = false;
};

void DrawBorder(IUIRenderer& renderer, const Rect& rect, Color color)
{
    renderer.DrawRect({rect.x, rect.y, rect.width, 1}, color);
    renderer.DrawRect({rect.x, rect.y + rect.height - 1, rect.width, 1}, color);
    renderer.DrawRect({rect.x, rect.y, 1, rect.height}, color);
    renderer.DrawRect({rect.x + rect.width - 1, rect.y, 1, rect.height}, color);
}

void AddWrapped(std::vector<DetailVisualLine>& lines, const std::string& text, int width, Color color = UITheme::Text, bool bold = false)
{
    if (text.empty())
        return;
    const auto wrapped = WrapTextApprox(text, std::max(80, width), 160, 8);
    for (const auto& line : wrapped)
        lines.push_back({line, color, bold});
}

void AddStringList(std::vector<DetailVisualLine>& lines, const char* heading,
                   const std::vector<std::string>& values, int width)
{
    if (values.empty())
        return;
    lines.push_back({heading, UITheme::TextMuted, true});
    for (const auto& value : values)
        AddWrapped(lines, "- " + value, width);
}

std::vector<DetailVisualLine> BuildDetailLines(const QuestEntrySnapshot& quest, int width, QuestDetailTab tab)
{
    std::vector<DetailVisualLine> lines;
    const int textWidth = std::max(80, width);
    const std::string heading = !quest.name.empty() ? quest.name : (!quest.title.empty() ? quest.title : "Quest");
    lines.push_back({heading, UITheme::Accent, true});

    auto addMeta = [&]()
    {
        std::string meta;
        if (quest.levelMin > 0)
        {
            meta = "Nivel " + std::to_string(quest.levelMin);
            if (quest.levelMax > 0 && quest.levelMax != quest.levelMin)
                meta += "-" + std::to_string(quest.levelMax);
        }
        if (quest.party)
            meta += (meta.empty() ? "" : "  |  ") + std::string("Grupo");
        else if (!meta.empty())
            meta += "  |  Solo";
        if (quest.codex.available && quest.codex.repeatable)
            meta += (meta.empty() ? "" : "  |  ") + std::string("Repetivel");
        if (!meta.empty())
            lines.push_back({meta, UITheme::TextMuted, false});
    };

    if (tab == QuestDetailTab::Summary)
    {
        addMeta();
        if (!quest.description.empty())
        {
            lines.push_back({"", UITheme::Text, false});
            AddWrapped(lines, quest.description, textWidth);
        }
        if (quest.codex.available)
        {
            lines.push_back({"", UITheme::Text, false});
            lines.push_back({"CODEX PT-BR", UITheme::TextMuted, true});
            if (!quest.codex.type.empty())
                AddWrapped(lines, "Tipo: " + quest.codex.type, textWidth);
            if (!quest.codex.category.empty())
                AddWrapped(lines, "Categoria: " + quest.codex.category, textWidth);
            if (!quest.codex.classes.empty())
                AddWrapped(lines, "Classes: " + [&]() { std::string x; for (const auto& v : quest.codex.classes) { if (!x.empty()) x += ", "; x += v; } return x; }(), textWidth);
            if (!quest.codex.factions.empty())
                AddWrapped(lines, "Faccoes: " + [&]() { std::string x; for (const auto& v : quest.codex.factions) { if (!x.empty()) x += ", "; x += v; } return x; }(), textWidth);
            if (!quest.codex.prerequisites.empty())
            {
                lines.push_back({"", UITheme::Text, false});
                AddStringList(lines, "PRE-REQUISITOS", quest.codex.prerequisites, textWidth);
            }
        }
    }
    else if (tab == QuestDetailTab::Lore)
    {
        if (!quest.codex.available)
        {
            lines.push_back({"", UITheme::Text, false});
            lines.push_back({"Esta quest ainda nao possui uma entrada vinculada no Codex.", UITheme::TextMuted, false});
        }
        else
        {
            lines.push_back({"", UITheme::Text, false});
            if (!quest.codex.lore.empty())
                AddWrapped(lines, quest.codex.lore, textWidth);
            else
                AddWrapped(lines, "O guia fornecido nao separa um texto narrativo proprio. Esta aba mostra o contexto documentado sem inventar lore adicional.", textWidth, UITheme::TextMuted);
            if (!quest.codex.stages.empty())
            {
                lines.push_back({"", UITheme::Text, false});
                AddStringList(lines, "ETAPAS", quest.codex.stages, textWidth);
            }
            if (!quest.codex.prerequisites.empty())
            {
                lines.push_back({"", UITheme::Text, false});
                AddStringList(lines, "PRE-REQUISITOS", quest.codex.prerequisites, textWidth);
            }
            if (!quest.codex.notes.empty())
            {
                lines.push_back({"", UITheme::Text, false});
                AddStringList(lines, "NOTAS", quest.codex.notes, textWidth);
            }
            if (!quest.codex.source.empty())
            {
                lines.push_back({"", UITheme::Text, false});
                lines.push_back({"FONTE", UITheme::TextMuted, true});
                AddWrapped(lines, quest.codex.source, textWidth, UITheme::TextMuted);
            }
        }
    }
    else if (tab == QuestDetailTab::Objectives)
    {
        if (!quest.objectives.empty())
        {
            lines.push_back({"", UITheme::Text, false});
            lines.push_back({"PROGRESSO ATUAL", UITheme::TextMuted, true});
            for (const auto& objective : quest.objectives)
            {
                std::string value;
                if (objective.required > 0)
                    value = std::to_string(objective.current) + "/" + std::to_string(objective.required) + "  ";
                value += objective.name;
                if (!objective.location.empty())
                    value += " - " + objective.location;
                const Color color = objective.complete ? UITheme::Positive : UITheme::Text;
                AddWrapped(lines, value, textWidth, color);
            }
        }
        if (quest.codex.available && !quest.codex.objectives.empty())
        {
            lines.push_back({"", UITheme::Text, false});
            AddStringList(lines, "OBJETIVOS DO GUIA", quest.codex.objectives, textWidth);
        }
        if (quest.codex.available && !quest.codex.questItems.empty())
        {
            lines.push_back({"", UITheme::Text, false});
            AddStringList(lines, "ITENS DA QUEST", quest.codex.questItems, textWidth);
        }
        if (quest.objectives.empty() && (!quest.codex.available || quest.codex.objectives.empty()))
        {
            lines.push_back({"", UITheme::Text, false});
            lines.push_back({"Nenhum objetivo documentado para esta entrada.", UITheme::TextMuted, false});
        }
    }
    else if (tab == QuestDetailTab::Npcs)
    {
        lines.push_back({"", UITheme::Text, false});
        if (quest.codex.available && !quest.codex.npcs.empty())
            AddStringList(lines, "PERSONAGENS / NPCS", quest.codex.npcs, textWidth);
        else
            lines.push_back({"Nenhum NPC documentado no Codex para esta quest.", UITheme::TextMuted, false});
    }
    else if (tab == QuestDetailTab::Maps)
    {
        lines.push_back({"", UITheme::Text, false});
        if (quest.codex.available && !quest.codex.locations.empty())
            AddStringList(lines, "MAPAS / LOCAIS", quest.codex.locations, textWidth);
        else
            lines.push_back({"Nenhum mapa documentado no Codex para esta quest.", UITheme::TextMuted, false});
    }
    else if (tab == QuestDetailTab::Rewards)
    {
        if (!quest.rewards.empty() || quest.rewardGold || quest.rewardExp || quest.rewardClanExp || quest.rewardCoin || quest.rewardTCoin)
        {
            lines.push_back({"", UITheme::Text, false});
            lines.push_back({"RECOMPENSA DO RUNTIME", UITheme::TextMuted, true});
            for (const auto& reward : quest.rewards)
                lines.push_back({std::to_string(reward.count) + "x " + reward.name, UITheme::Text, false});
            if (quest.rewardGold) lines.push_back({std::to_string(quest.rewardGold) + " Gold", UITheme::Text, false});
            if (quest.rewardExp) lines.push_back({std::to_string(quest.rewardExp) + " EXP", UITheme::Text, false});
            if (quest.rewardClanExp) lines.push_back({std::to_string(quest.rewardClanExp) + " EXP Clan", UITheme::Text, false});
            if (quest.rewardCoin) lines.push_back({std::to_string(quest.rewardCoin) + " Coin", UITheme::Text, false});
            if (quest.rewardTCoin) lines.push_back({std::to_string(quest.rewardTCoin) + " TCoin", UITheme::Text, false});
        }
        if (quest.codex.available && !quest.codex.rewards.empty())
        {
            lines.push_back({"", UITheme::Text, false});
            AddStringList(lines, "RECOMPENSA DOCUMENTADA", quest.codex.rewards, textWidth);
        }
        if (quest.rewards.empty() && !quest.rewardGold && !quest.rewardExp && !quest.rewardClanExp && !quest.rewardCoin && !quest.rewardTCoin &&
            (!quest.codex.available || quest.codex.rewards.empty()))
        {
            lines.push_back({"", UITheme::Text, false});
            lines.push_back({"Nenhuma recompensa documentada para esta entrada.", UITheme::TextMuted, false});
        }
    }

    return lines;
}
}

QuestJournalWindow::QuestJournalWindow(IQuestGateway& gateway, UIId id)
    : Window(id, "Diario de Quests"), gateway_(gateway)
{
    SetBounds({40, 35, 760, 530});
    SetTitleBarHeight(34);
    SetMovable(true);
    SetClosable(true);
    BuildChrome();
    Layout();
    Refresh();
}

void QuestJournalWindow::BuildChrome()
{
    auto& available = AddChild<Button>(900, "Disponiveis");
    available.SetOnClick([this](const UIEvent&) { SetTab(JournalTab::Available); });
    availableTab_ = &available;

    auto& progress = AddChild<Button>(901, "Progresso");
    progress.SetOnClick([this](const UIEvent&) { SetTab(JournalTab::Progress); });
    progressTab_ = &progress;

    auto& codex = AddChild<Button>(908, "Codex");
    codex.SetOnClick([this](const UIEvent&) { SetTab(JournalTab::Codex); });
    codexTab_ = &codex;

    auto& summary = AddChild<Button>(910, "Resumo");
    summary.SetOnClick([this](const UIEvent&) { SetDetailTab(QuestDetailTab::Summary); });
    summaryTab_ = &summary;
    auto& lore = AddChild<Button>(911, "Lore");
    lore.SetOnClick([this](const UIEvent&) { SetDetailTab(QuestDetailTab::Lore); });
    loreTab_ = &lore;
    auto& objectives = AddChild<Button>(912, "Objetivos");
    objectives.SetOnClick([this](const UIEvent&) { SetDetailTab(QuestDetailTab::Objectives); });
    objectivesTab_ = &objectives;
    auto& npcs = AddChild<Button>(913, "NPCs");
    npcs.SetOnClick([this](const UIEvent&) { SetDetailTab(QuestDetailTab::Npcs); });
    npcsTab_ = &npcs;
    auto& maps = AddChild<Button>(914, "Mapas");
    maps.SetOnClick([this](const UIEvent&) { SetDetailTab(QuestDetailTab::Maps); });
    mapsTab_ = &maps;
    auto& rewards = AddChild<Button>(915, "Recomp.");
    rewards.SetOnClick([this](const UIEvent&) { SetDetailTab(QuestDetailTab::Rewards); });
    rewardsTab_ = &rewards;

    auto& accept = AddChild<Button>(902, "Aceitar");
    accept.SetOnClick([this](const UIEvent&)
    {
        const QuestEntrySnapshot* selected = SelectedQuest();
        if (!selected || !confirmation_) return;
        confirmation_->Show("Aceitar quest", "Aceitar " + (selected->title.empty() ? std::string("esta quest") : selected->title) + "?", [this]()
        {
            gateway_.RequestAcceptSelected();
            Refresh();
        });
    });
    acceptButton_ = &accept;

    auto& cancel = AddChild<Button>(903, "Cancelar");
    cancel.SetOnClick([this](const UIEvent&)
    {
        const QuestEntrySnapshot* selected = SelectedQuest();
        if (!selected || !confirmation_) return;
        confirmation_->Show("Cancelar quest", "Cancelar " + (selected->title.empty() ? std::string("esta quest") : selected->title) + "?", [this]()
        {
            gateway_.RequestCancelSelected();
            Refresh();
        });
    });
    cancelButton_ = &cancel;

    auto& reward = AddChild<Button>(904, "Receber recompensa");
    reward.SetOnClick([this](const UIEvent&) { gateway_.ClaimSelectedReward(); Refresh(); });
    rewardButton_ = &reward;

    auto& track = AddChild<Button>(905, "Rastrear");
    track.SetOnClick([this](const UIEvent&) { gateway_.TrackSelected(); Refresh(); });
    trackButton_ = &track;

    auto& close = AddChild<Button>(906, "X");
    close.SetOnClick([this](const UIEvent&) { if (onClose_) onClose_(); });
    closeButton_ = &close;

    auto& confirmation = AddChild<ConfirmationOverlay>(907);
    confirmation.SetZOrder(100);
    confirmation_ = &confirmation;
}

void QuestJournalWindow::Layout()
{
    const Rect bounds = Bounds();
    availableTab_->SetBounds({14, 40, 112, 30});
    progressTab_->SetBounds({132, 40, 112, 30});
    codexTab_->SetBounds({250, 40, 92, 30});
    closeButton_->SetBounds({bounds.width - 42, 5, 30, 24});

    const int detailX = 338;
    const int detailY = 86;
    const int detailGap = 4;
    const int detailAvailable = std::max(300, bounds.width - detailX - 22);
    const int detailButtonWidth = std::max(48, (detailAvailable - detailGap * 5) / 6);
    summaryTab_->SetBounds({detailX, detailY, detailButtonWidth, 24});
    loreTab_->SetBounds({detailX + (detailButtonWidth + detailGap), detailY, detailButtonWidth, 24});
    objectivesTab_->SetBounds({detailX + 2 * (detailButtonWidth + detailGap), detailY, detailButtonWidth, 24});
    npcsTab_->SetBounds({detailX + 3 * (detailButtonWidth + detailGap), detailY, detailButtonWidth, 24});
    mapsTab_->SetBounds({detailX + 4 * (detailButtonWidth + detailGap), detailY, detailButtonWidth, 24});
    rewardsTab_->SetBounds({detailX + 5 * (detailButtonWidth + detailGap), detailY, detailButtonWidth, 24});

    const int actionY = bounds.height - 42;
    acceptButton_->SetBounds({bounds.width - 432, actionY, 100, 28});
    cancelButton_->SetBounds({bounds.width - 326, actionY, 100, 28});
    rewardButton_->SetBounds({bounds.width - 220, actionY, 150, 28});
    trackButton_->SetBounds({bounds.width - 64 - 96, 40, 96, 30});
    if (confirmation_) confirmation_->SetBounds({0, 0, bounds.width, bounds.height});
}

void QuestJournalWindow::SetViewport(int width, int height)
{
    Rect bounds = Bounds();
    bounds.width = std::min(820, std::max(700, width - 30));
    bounds.height = std::min(570, std::max(500, height - 30));
    bounds.x = std::max(0, (width - bounds.width) / 2);
    bounds.y = std::max(0, (height - bounds.height) / 2);
    SetBounds(bounds);
    Layout();
    listScroll_ = std::clamp(listScroll_, 0, MaxListScroll());
    detailsScroll_ = std::clamp(detailsScroll_, 0, MaxDetailsScroll());
}

void QuestJournalWindow::SetTab(JournalTab tab)
{
    tab_ = tab;
    listScroll_ = 0;
    detailsScroll_ = 0;
    detailTab_ = QuestDetailTab::Summary;
    availableTab_->SetSelected(tab == JournalTab::Available);
    progressTab_->SetSelected(tab == JournalTab::Progress);
    codexTab_->SetSelected(tab == JournalTab::Codex);
    RefreshButtons();
}

void QuestJournalWindow::SetDetailTab(QuestDetailTab tab)
{
    detailTab_ = tab;
    detailsScroll_ = 0;
    RefreshButtons();
}

bool QuestJournalWindow::SectionVisible(const QuestSectionSnapshot& section) const noexcept
{
    if (tab_ == JournalTab::Codex)
        return section.codexGroup;
    if (section.codexGroup)
        return false;
    const bool progress = section.progressGroup || section.finalizedGroup;
    return tab_ == JournalTab::Progress ? progress : !progress;
}

void QuestJournalWindow::Refresh()
{
    snapshot_ = gateway_.GetSnapshot();
    listScroll_ = std::clamp(listScroll_, 0, MaxListScroll());
    detailsScroll_ = std::clamp(detailsScroll_, 0, MaxDetailsScroll());
    BuildVisibleRows();
    RefreshButtons();
}

void QuestJournalWindow::Update(float deltaSeconds)
{
    refreshAccumulator_ += deltaSeconds;
    if (refreshAccumulator_ >= 0.20f)
    {
        refreshAccumulator_ = 0.0f;
        Refresh();
    }
    Window::Update(deltaSeconds);
}

const QuestEntrySnapshot* QuestJournalWindow::SelectedQuest() const noexcept
{
    for (const auto& section : snapshot_.sections)
        if (section.id == snapshot_.selectedSectionId && SectionVisible(section))
            for (const auto& quest : section.quests)
                if (quest.id == snapshot_.selectedQuestId)
                    return &quest;
    return nullptr;
}

void QuestJournalWindow::RefreshButtons()
{
    availableTab_->SetSelected(tab_ == JournalTab::Available);
    progressTab_->SetSelected(tab_ == JournalTab::Progress);
    codexTab_->SetSelected(tab_ == JournalTab::Codex);

    const QuestEntrySnapshot* selected = SelectedQuest();
    const bool hasSelected = selected != nullptr;
    const bool inCodex = tab_ == JournalTab::Codex || (hasSelected && selected->sectionId == 0x7020);
    const bool inProgress = hasSelected && selected->sectionId == 10;
    const bool finalized = hasSelected && selected->sectionId == 11;

    acceptButton_->SetVisible(hasSelected && !inCodex && !inProgress && !finalized);
    acceptButton_->SetEnabled(hasSelected && !inCodex && selected->active && !inProgress && !finalized);
    cancelButton_->SetVisible(!inCodex && inProgress && !selected->complete);
    cancelButton_->SetEnabled(!inCodex && inProgress && !selected->complete);
    rewardButton_->SetVisible(!inCodex && inProgress && selected->complete);
    rewardButton_->SetEnabled(!inCodex && inProgress && selected->complete);
    trackButton_->SetVisible(!inCodex && inProgress);
    trackButton_->SetEnabled(!inCodex && inProgress);
    trackButton_->SetText(inProgress && selected && selected->tracked ? "Rastreando" : "Rastrear");

    Button* detailButtons[] = {summaryTab_, loreTab_, objectivesTab_, npcsTab_, mapsTab_, rewardsTab_};
    for (Button* button : detailButtons)
        if (button) button->SetVisible(hasSelected);
    if (summaryTab_) summaryTab_->SetSelected(detailTab_ == QuestDetailTab::Summary);
    if (loreTab_) loreTab_->SetSelected(detailTab_ == QuestDetailTab::Lore);
    if (objectivesTab_) objectivesTab_->SetSelected(detailTab_ == QuestDetailTab::Objectives);
    if (npcsTab_) npcsTab_->SetSelected(detailTab_ == QuestDetailTab::Npcs);
    if (mapsTab_) mapsTab_->SetSelected(detailTab_ == QuestDetailTab::Maps);
    if (rewardsTab_) rewardsTab_->SetSelected(detailTab_ == QuestDetailTab::Rewards);
}

int QuestJournalWindow::MaxListScroll() const noexcept
{
    const int listHeight = std::max(1, Bounds().height - 126);
    int contentHeight = 0;
    for (const auto& section : snapshot_.sections)
    {
        if (!SectionVisible(section))
            continue;
        contentHeight += SectionHeaderHeight + std::max(1, static_cast<int>(section.quests.size())) * QuestRowHeight + 6;
    }
    return std::max(0, contentHeight - listHeight);
}

int QuestJournalWindow::MaxDetailsScroll() const noexcept
{
    const QuestEntrySnapshot* selected = SelectedQuest();
    if (!selected)
        return 0;
    const int detailsWidth = std::max(160, Bounds().width - 334 - 30);
    const int detailsHeight = std::max(1, Bounds().height - 158);
    const auto lines = BuildDetailLines(*selected, detailsWidth - 26, detailTab_);
    return std::max(0, static_cast<int>(lines.size()) * DetailLineHeight - detailsHeight + 18);
}

void QuestJournalWindow::BuildVisibleRows()
{
    visibleRows_.clear();
    const Rect root = GlobalBounds();
    const Rect list{root.x + 14, root.y + 82, 306, std::max(1, root.height - 126)};
    int y = list.y - listScroll_;

    for (const auto& section : snapshot_.sections)
    {
        if (!SectionVisible(section))
            continue;
        y += SectionHeaderHeight;
        if (section.quests.empty())
        {
            y += QuestRowHeight;
        }
        for (const auto& quest : section.quests)
        {
            Rect row{list.x + 4, y, list.width - 12, QuestRowHeight - 2};
            if (row.y + row.height >= list.y && row.y <= list.y + list.height)
                visibleRows_.push_back({section.id, quest.id, row});
            y += QuestRowHeight;
        }
        y += 6;
    }
}

bool QuestJournalWindow::HasModalInput() const noexcept
{
    return confirmation_ && confirmation_->IsOpen();
}

bool QuestJournalWindow::HandleModalKey(std::uint32_t key)
{
    return confirmation_ && confirmation_->IsOpen() ? confirmation_->HandleKey(key) : false;
}

UIEventResult QuestJournalWindow::OnEvent(const UIEvent& event)
{
    if (confirmation_ && confirmation_->IsOpen())
        return confirmation_->OnEvent(event);
    BuildVisibleRows();
    const Rect root = GlobalBounds();
    const Rect list{root.x + 14, root.y + 82, 306, std::max(1, root.height - 126)};
    const Rect details{root.x + 334, root.y + 82, std::max(1, root.width - 348), std::max(1, root.height - 126)};

    if (event.type == UIEventType::Wheel)
    {
        const int direction = event.wheelDelta > 0 ? -1 : (event.wheelDelta < 0 ? 1 : 0);
        if (list.Contains(event.pointer))
        {
            listScroll_ = std::clamp(listScroll_ + direction * QuestRowHeight * 2, 0, MaxListScroll());
            BuildVisibleRows();
            return UIEventResult::Handled;
        }
        if (details.Contains(event.pointer))
        {
            detailsScroll_ = std::clamp(detailsScroll_ + direction * DetailLineHeight * 3, 0, MaxDetailsScroll());
            return UIEventResult::Handled;
        }
    }

    if (event.type == UIEventType::PointerDown && event.button == MouseButton::Left)
    {
        for (const auto& row : visibleRows_)
        {
            if (row.rect.Contains(event.pointer))
            {
                if (gateway_.SelectQuest(row.sectionId, row.questId))
                {
                    detailsScroll_ = 0;
                    detailTab_ = QuestDetailTab::Summary;
                    Refresh();
                }
                return UIEventResult::Handled;
            }
        }
    }

    return Window::OnEvent(event);
}

void QuestJournalWindow::DrawList(UIRenderContext& context)
{
    const Rect root = GlobalBounds();
    const Rect list{root.x + 14, root.y + 82, 306, std::max(1, root.height - 126)};
    auto& renderer = context.Renderer();
    renderer.DrawRect(list, UITheme::Surface);
    DrawBorder(renderer, list, UITheme::BorderSoft);

    renderer.PushClip({list.x + 1, list.y + 1, list.width - 2, list.height - 2});
    int y = list.y - listScroll_;
    for (const auto& section : snapshot_.sections)
    {
        if (!SectionVisible(section))
            continue;

        const Rect header{list.x + 1, y, list.width - 2, SectionHeaderHeight};
        renderer.DrawRect(header, UITheme::SurfaceRaised);
        renderer.DrawTextUtf8(FontHandle{}, section.title,
                              {header.x + 8, header.y + 3, header.width - 16, 18},
                              TextStyle{UITheme::TextMuted, true, TextAlign::Left});
        y += SectionHeaderHeight;

        if (section.quests.empty())
        {
            renderer.DrawTextUtf8(FontHandle{}, "Nenhuma quest.",
                                  {list.x + 12, y + 3, list.width - 24, 18},
                                  TextStyle{UITheme::Disabled, false, TextAlign::Left});
            y += QuestRowHeight;
        }
        else
        {
            for (const auto& quest : section.quests)
            {
                const Rect row{list.x + 4, y, list.width - 12, QuestRowHeight - 2};
                if (row.y + row.height >= list.y && row.y <= list.y + list.height)
                {
                    const bool selected = snapshot_.selectedSectionId == section.id && snapshot_.selectedQuestId == quest.id;
                    renderer.DrawRect(row, selected ? UITheme::SurfacePressed : UITheme::Slot);
                    if (selected)
                        renderer.DrawRect({row.x, row.y, 3, row.height}, UITheme::Accent);
                    if (quest.tracked)
                        renderer.DrawTextUtf8(FontHandle{}, "*", {row.x + 6, row.y + 5, 12, 18},
                                              TextStyle{UITheme::Accent, true, TextAlign::Left});
                    const std::string title = EllipsizeText(quest.title.empty() ? "Quest" : quest.title, row.width - 30, 7);
                    renderer.DrawTextUtf8(FontHandle{}, title,
                                          {row.x + 18, row.y + 5, row.width - 24, 18},
                                          TextStyle{quest.active ? UITheme::Text : UITheme::Disabled, selected, TextAlign::Left});
                }
                y += QuestRowHeight;
            }
        }
        y += 6;
    }
    renderer.PopClip();

    const int maxScroll = MaxListScroll();
    if (maxScroll > 0)
    {
        const Rect track{list.x + list.width - 5, list.y + 3, 2, list.height - 6};
        renderer.DrawRect(track, UITheme::BorderSoft);
        const int thumbHeight = std::max(24, list.height * list.height / (list.height + maxScroll));
        const int travel = std::max(0, track.height - thumbHeight);
        const int thumbY = track.y + listScroll_ * travel / maxScroll;
        renderer.DrawRect({track.x - 1, thumbY, 4, thumbHeight}, UITheme::AccentMuted);
    }
}

void QuestJournalWindow::DrawDetails(UIRenderContext& context)
{
    const Rect root = GlobalBounds();
    const Rect details{root.x + 334, root.y + 82, std::max(1, root.width - 348), std::max(1, root.height - 126)};
    auto& renderer = context.Renderer();
    renderer.DrawRect(details, UITheme::Surface);
    DrawBorder(renderer, details, UITheme::BorderSoft);

    const QuestEntrySnapshot* selected = SelectedQuest();
    if (!selected)
    {
        renderer.DrawTextUtf8(FontHandle{}, "Selecione uma quest para ver os detalhes.",
                              {details.x + 14, details.y + 16, details.width - 28, 22},
                              TextStyle{UITheme::TextMuted, false, TextAlign::Left});
        return;
    }

    const Rect content{details.x + 12, details.y + 40, details.width - 24, details.height - 48};
    const auto lines = BuildDetailLines(*selected, content.width - 8, detailTab_);
    renderer.PushClip(content);
    int y = content.y - detailsScroll_;
    for (const auto& line : lines)
    {
        if (y + DetailLineHeight >= content.y && y <= content.y + content.height)
            renderer.DrawTextUtf8(FontHandle{}, line.text, {content.x, y, content.width - 8, DetailLineHeight},
                                  TextStyle{line.color, line.bold, TextAlign::Left});
        y += DetailLineHeight;
    }
    renderer.PopClip();

    const int maxScroll = MaxDetailsScroll();
    if (maxScroll > 0)
    {
        const Rect track{details.x + details.width - 6, details.y + 40, 2, details.height - 48};
        renderer.DrawRect(track, UITheme::BorderSoft);
        const int thumbHeight = std::max(24, details.height * details.height / (details.height + maxScroll));
        const int travel = std::max(0, track.height - thumbHeight);
        const int thumbY = track.y + detailsScroll_ * travel / maxScroll;
        renderer.DrawRect({track.x - 1, thumbY, 4, thumbHeight}, UITheme::AccentMuted);
    }
}

void QuestJournalWindow::OnRender(UIRenderContext& context)
{
    Layout();
    Window::OnRender(context);
    DrawList(context);
    DrawDetails(context);
}
}
