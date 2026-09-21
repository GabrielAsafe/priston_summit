#include "systems/ui/v2/pages/CompanionsPage.h"

#include <algorithm>
#include <cstdio>
#include <string>

#include "systems/ui/v2/rendering/IUIRenderer.h"
#include "systems/ui/v2/theme/UITheme.h"
#include "systems/ui/v2/widgets/Button.h"
#include "systems/ui/v2/widgets/TextLayout.h"

namespace ui::v2
{
namespace
{
std::string ResultText(int result)
{
    switch (result)
    {
        case 0: return {};
        case 1: return "Operacao concluida.";
        case -2: return "Esse personagem nao pode ser usado agora.";
        case -3: return "Falha ao ler/gravar o DAT do personagem.";
        case -4: return "Esse personagem ja esta online.";
        case -5: return "Esse companion ja esta invocado.";
        case -6: return "Limite de companions ativos atingido.";
        case -7: return "Falha ao criar a entidade do companion.";
        case -8: return "Nenhum personagem elegivel encontrado.";
        case -9: return "Nao ha espaco para guardar o loadout desta sessao.";
        case -10: return "Acao invalida para esta versao da UI.";
        case -11: return "Falha ao persistir o perfil do companion.";
        default: return "Resultado do servidor: " + std::to_string(result);
    }
}

const char* TacticName(CompanionUiTactic tactic)
{
    switch (tactic)
    {
        case CompanionUiTactic::OwnerFocus: return "Foco no dono";
        case CompanionUiTactic::Defensive: return "Defensivo";
        case CompanionUiTactic::HoldPosition: return "Manter posicao";
        case CompanionUiTactic::Balanced:
        default: return "Balanceado";
    }
}

std::string ItemDisplay(const std::string& name, std::uint32_t code)
{
    if (!code)
        return "Nenhum";
    if (!name.empty())
        return name;
    char buffer[32] = {};
    std::snprintf(buffer, sizeof(buffer), "0x%08X", static_cast<unsigned>(code));
    return buffer;
}
}

CompanionsPage::CompanionsPage(ICompanionGateway& gateway, UIId id)
    : Container(id), gateway_(gateway)
{
    BuildControls();
    ForceRefresh();
}

void CompanionsPage::BuildControls()
{
    for (std::size_t i = 0; i < characterButtons_.size(); ++i)
    {
        auto& button = AddChild<Button>(600 + static_cast<UIId>(i), "-");
        button.SetOnClick([this, i](const UIEvent&) { SelectVisibleRow(i); });
        characterButtons_[i] = &button;
    }

    auto& summonDismiss = AddChild<Button>(620, "Invocar");
    summonDismiss.SetOnClick([this](const UIEvent&)
    {
        const CompanionUiEntry* selected = SelectedEntry();
        if (!selected) return;
        ExecuteForSelected(selected->active ? CompanionUiAction::Dismiss : CompanionUiAction::Summon);
    });
    summonDismissButton_ = &summonDismiss;

    auto& dismissAll = AddChild<Button>(621, "Dispensar todos");
    dismissAll.SetOnClick([this](const UIEvent&) { gateway_.Execute(CompanionUiAction::DismissAll); });
    dismissAllButton_ = &dismissAll;

    auto& refresh = AddChild<Button>(622, "Atualizar");
    refresh.SetOnClick([this](const UIEvent&) { ForceRefresh(); });
    refreshButton_ = &refresh;

    auto& aggressive = AddChild<Button>(630, "Agressivo");
    aggressive.SetOnClick([this](const UIEvent&) { ExecuteForSelected(CompanionUiAction::SetMode, 0); });
    aggressiveButton_ = &aggressive;
    auto& follow = AddChild<Button>(631, "Seguir");
    follow.SetOnClick([this](const UIEvent&) { ExecuteForSelected(CompanionUiAction::SetMode, 1); });
    followButton_ = &follow;
    auto& passive = AddChild<Button>(632, "Passivo");
    passive.SetOnClick([this](const UIEvent&) { ExecuteForSelected(CompanionUiAction::SetMode, 2); });
    passiveButton_ = &passive;

    auto& tacticPrev = AddChild<Button>(633, "<");
    tacticPrev.SetOnClick([this](const UIEvent&)
    {
        const CompanionUiEntry* selected = SelectedEntry();
        if (!selected) return;
        int value = static_cast<int>(selected->tactic) - 1;
        if (value < 0) value = 3;
        ExecuteForSelected(CompanionUiAction::SetTactic, value);
    });
    tacticPrevButton_ = &tacticPrev;
    auto& tacticNext = AddChild<Button>(634, ">");
    tacticNext.SetOnClick([this](const UIEvent&)
    {
        const CompanionUiEntry* selected = SelectedEntry();
        if (!selected) return;
        int value = (static_cast<int>(selected->tactic) + 1) % 4;
        ExecuteForSelected(CompanionUiAction::SetTactic, value);
    });
    tacticNextButton_ = &tacticNext;

    auto& retreatPrev = AddChild<Button>(635, "-");
    retreatPrev.SetOnClick([this](const UIEvent&)
    {
        const CompanionUiEntry* selected = SelectedEntry();
        if (!selected) return;
        ExecuteForSelected(CompanionUiAction::SetRetreatHp, std::max(0, selected->retreatHpPercent - 5));
    });
    retreatPrevButton_ = &retreatPrev;
    auto& retreatNext = AddChild<Button>(636, "+");
    retreatNext.SetOnClick([this](const UIEvent&)
    {
        const CompanionUiEntry* selected = SelectedEntry();
        if (!selected) return;
        ExecuteForSelected(CompanionUiAction::SetRetreatHp, std::min(90, selected->retreatHpPercent + 5));
    });
    retreatNextButton_ = &retreatNext;

    auto& rangePrev = AddChild<Button>(637, "-");
    rangePrev.SetOnClick([this](const UIEvent&)
    {
        const CompanionUiEntry* selected = SelectedEntry();
        if (!selected) return;
        ExecuteForSelected(CompanionUiAction::SetPreferredRange, std::max(0, selected->preferredRange - 25));
    });
    rangePrevButton_ = &rangePrev;
    auto& rangeNext = AddChild<Button>(638, "+");
    rangeNext.SetOnClick([this](const UIEvent&)
    {
        const CompanionUiEntry* selected = SelectedEntry();
        if (!selected) return;
        ExecuteForSelected(CompanionUiAction::SetPreferredRange, std::min(600, selected->preferredRange + 25));
    });
    rangeNextButton_ = &rangeNext;

    auto& rightPrev = AddChild<Button>(640, "<");
    rightPrev.SetOnClick([this](const UIEvent&) { ExecuteForSelected(CompanionUiAction::EquipRightPrev); });
    rightPrevButton_ = &rightPrev;
    auto& rightNext = AddChild<Button>(641, ">");
    rightNext.SetOnClick([this](const UIEvent&) { ExecuteForSelected(CompanionUiAction::EquipRightNext); });
    rightNextButton_ = &rightNext;
    auto& leftPrev = AddChild<Button>(642, "<");
    leftPrev.SetOnClick([this](const UIEvent&) { ExecuteForSelected(CompanionUiAction::EquipLeftPrev); });
    leftPrevButton_ = &leftPrev;
    auto& leftNext = AddChild<Button>(643, ">");
    leftNext.SetOnClick([this](const UIEvent&) { ExecuteForSelected(CompanionUiAction::EquipLeftNext); });
    leftNextButton_ = &leftNext;

    auto& reset = AddChild<Button>(650, "Equipamento do DAT");
    reset.SetOnClick([this](const UIEvent&) { ExecuteForSelected(CompanionUiAction::ResetEquipment); });
    resetEquipmentButton_ = &reset;

    auto& resync = AddChild<Button>(651, "Recarregar DAT");
    resync.SetOnClick([this](const UIEvent&) { ExecuteForSelected(CompanionUiAction::ResyncData); });
    resyncButton_ = &resync;

    LayoutControls();
}

void CompanionsPage::LayoutControls()
{
    for (std::size_t i = 0; i < characterButtons_.size(); ++i)
        characterButtons_[i]->SetBounds({22, 74 + static_cast<int>(i) * 48, 236, 38});

    summonDismissButton_->SetBounds({282, 74, 150, 34});
    dismissAllButton_->SetBounds({440, 74, 150, 34});
    refreshButton_->SetBounds({598, 74, 126, 34});

    aggressiveButton_->SetBounds({282, 132, 136, 32});
    followButton_->SetBounds({426, 132, 136, 32});
    passiveButton_->SetBounds({570, 132, 136, 32});

    tacticPrevButton_->SetBounds({282, 172, 36, 30});
    tacticNextButton_->SetBounds({688, 172, 36, 30});
    retreatPrevButton_->SetBounds({282, 208, 36, 28});
    retreatNextButton_->SetBounds({446, 208, 36, 28});
    rangePrevButton_->SetBounds({496, 208, 36, 28});
    rangeNextButton_->SetBounds({688, 208, 36, 28});

    rightPrevButton_->SetBounds({282, 326, 36, 30});
    rightNextButton_->SetBounds({688, 326, 36, 30});
    leftPrevButton_->SetBounds({282, 360, 36, 30});
    leftNextButton_->SetBounds({688, 360, 36, 30});

    resetEquipmentButton_->SetBounds({282, 404, 190, 32});
    resyncButton_->SetBounds({482, 404, 170, 32});
}

const CompanionUiEntry* CompanionsPage::SelectedEntry() const
{
    const auto& snapshot = gateway_.Snapshot();
    if (!snapshot.valid)
        return nullptr;
    for (int i = 0; i < snapshot.entryCount; ++i)
    {
        const auto& entry = snapshot.entries[static_cast<std::size_t>(i)];
        if (entry.selected && !entry.currentCharacter)
            return &entry;
    }
    return nullptr;
}

void CompanionsPage::SelectVisibleRow(std::size_t row)
{
    const auto& snapshot = gateway_.Snapshot();
    if (!snapshot.valid || row >= static_cast<std::size_t>(snapshot.entryCount))
        return;
    const auto& entry = snapshot.entries[row];
    if (entry.name.empty() || entry.currentCharacter)
        return;
    gateway_.Execute(CompanionUiAction::Select, entry.name);
}

void CompanionsPage::ExecuteForSelected(CompanionUiAction action, int value)
{
    const CompanionUiEntry* selected = SelectedEntry();
    if (!selected)
        return;
    gateway_.Execute(action, selected->name, value);
}

void CompanionsPage::ForceRefresh()
{
    refreshAccumulator_ = 0.0f;
    gateway_.RequestRefresh();
}

void CompanionsPage::RefreshControls()
{
    const auto& snapshot = gateway_.Snapshot();
    for (std::size_t row = 0; row < characterButtons_.size(); ++row)
    {
        Button* button = characterButtons_[row];
        if (!snapshot.valid || row >= static_cast<std::size_t>(snapshot.entryCount))
        {
            button->SetText("-");
            button->SetEnabled(false);
            button->SetSelected(false);
            continue;
        }

        const auto& entry = snapshot.entries[row];
        std::string text = entry.name.empty() ? "-" : entry.name;
        if (entry.currentCharacter)
            text += "  [em uso]";
        else if (entry.active)
        {
            text += entry.dead ? "  [morto]" : "  [ativo]";
            if (entry.maxLife > 0)
                text += " HP " + std::to_string(entry.life) + "/" + std::to_string(entry.maxLife);
        }
        if (entry.level > 0)
            text += "  Lv " + std::to_string(entry.level);
        button->SetText(text);
        button->SetEnabled(!entry.currentCharacter && !entry.name.empty());
        button->SetSelected(entry.selected);
    }

    const CompanionUiEntry* selected = SelectedEntry();
    const bool hasSelected = selected != nullptr;
    summonDismissButton_->SetEnabled(hasSelected);
    summonDismissButton_->SetText(hasSelected && selected->active ? "Dispensar" : "Invocar");
    dismissAllButton_->SetEnabled(snapshot.valid && snapshot.activeCount > 0);
    rightPrevButton_->SetEnabled(hasSelected);
    rightNextButton_->SetEnabled(hasSelected);
    leftPrevButton_->SetEnabled(hasSelected);
    leftNextButton_->SetEnabled(hasSelected);
    resetEquipmentButton_->SetEnabled(hasSelected);
    resyncButton_->SetEnabled(hasSelected);

    aggressiveButton_->SetEnabled(hasSelected);
    followButton_->SetEnabled(hasSelected);
    passiveButton_->SetEnabled(hasSelected);
    tacticPrevButton_->SetEnabled(hasSelected);
    tacticNextButton_->SetEnabled(hasSelected);
    retreatPrevButton_->SetEnabled(hasSelected);
    retreatNextButton_->SetEnabled(hasSelected);
    rangePrevButton_->SetEnabled(hasSelected);
    rangeNextButton_->SetEnabled(hasSelected);

    aggressiveButton_->SetSelected(hasSelected && selected->mode == CompanionUiMode::Aggressive);
    followButton_->SetSelected(hasSelected && selected->mode == CompanionUiMode::Follow);
    passiveButton_->SetSelected(hasSelected && selected->mode == CompanionUiMode::Passive);
}

void CompanionsPage::Update(float deltaSeconds)
{
    refreshAccumulator_ += deltaSeconds;
    if (refreshAccumulator_ >= 30.0f)
    {
        refreshAccumulator_ = 0.0f;
        gateway_.RequestRefresh();
    }
    RefreshControls();
    Container::Update(deltaSeconds);
}

void CompanionsPage::OnRender(UIRenderContext& context)
{
    const Rect root = GlobalBounds();
    auto& r = context.Renderer();
    r.DrawRect(root, UITheme::WindowBg);

    const auto& snapshot = gateway_.Snapshot();
    r.DrawTextUtf8(FontHandle{}, "COMPANIONS DA CONTA", {root.x + 22, root.y + 20, 300, 24},
                   TextStyle{UITheme::Text, true, TextAlign::Left});

    if (!snapshot.valid)
    {
        r.DrawTextUtf8(FontHandle{}, "Aguardando estado do servidor...", {root.x + 22, root.y + 48, 360, 22},
                       TextStyle{UITheme::TextMuted, false, TextAlign::Left});
        return;
    }

    r.DrawTextUtf8(FontHandle{}, "Ativos: " + std::to_string(snapshot.activeCount) + "/" + std::to_string(snapshot.maxActive),
                   {root.x + root.width - 220, root.y + 20, 190, 24},
                   TextStyle{UITheme::Accent, true, TextAlign::Right});

    r.DrawTextUtf8(FontHandle{}, "Personagens", {root.x + 22, root.y + 50, 236, 20},
                   TextStyle{UITheme::TextMuted, true, TextAlign::Left});
    r.DrawTextUtf8(FontHandle{}, "Acao", {root.x + 282, root.y + 50, 180, 20},
                   TextStyle{UITheme::TextMuted, true, TextAlign::Left});
    r.DrawTextUtf8(FontHandle{}, "Comportamento", {root.x + 282, root.y + 112, 260, 20},
                   TextStyle{UITheme::TextMuted, true, TextAlign::Left});

    const CompanionUiEntry* selected = SelectedEntry();
    if (selected)
    {
        r.DrawTextUtf8(FontHandle{}, std::string("Tatica: ") + TacticName(selected->tactic),
                       {root.x + 330, root.y + 176, 350, 22},
                       TextStyle{UITheme::Text, false, TextAlign::Left});
        r.DrawTextUtf8(FontHandle{}, "Recuar HP: " + std::to_string(selected->retreatHpPercent) + "%",
                       {root.x + 326, root.y + 212, 120, 20},
                       TextStyle{UITheme::TextMuted, false, TextAlign::Left});
        const std::string rangeText = selected->preferredRange > 0 ?
            std::to_string(selected->preferredRange) : std::string("auto");
        r.DrawTextUtf8(FontHandle{}, "Range: " + rangeText,
                       {root.x + 540, root.y + 212, 144, 20},
                       TextStyle{UITheme::TextMuted, false, TextAlign::Left});
    }

    const int detailY = root.y + 246;
    r.DrawRect({root.x + 274, detailY, root.width - 298, 146}, UITheme::Surface);
    r.DrawTextUtf8(FontHandle{}, "EQUIPAMENTO DO COMPANION", {root.x + 292, detailY + 12, 310, 20},
                   TextStyle{UITheme::TextMuted, true, TextAlign::Left});

    if (selected)
    {
        r.DrawTextUtf8(FontHandle{}, selected->name + " - Lv " + std::to_string(selected->level),
                       {root.x + 292, detailY + 34, 380, 22},
                       TextStyle{UITheme::Text, true, TextAlign::Left});

        std::string status = selected->active ? (selected->dead ? "Morto" : "Ativo") : "Offline";
        if (selected->maxLife > 0)
            status += "  HP " + std::to_string(selected->life) + "/" + std::to_string(selected->maxLife);
        if (selected->maxMana > 0)
            status += "  MP " + std::to_string(selected->mana) + "/" + std::to_string(selected->maxMana);
        r.DrawTextUtf8(FontHandle{}, status, {root.x + 292, detailY + 56, 390, 20},
                       TextStyle{UITheme::TextMuted, false, TextAlign::Left});

        if (selected->active && !selected->targetName.empty())
            r.DrawTextUtf8(FontHandle{}, "Alvo: " + selected->targetName, {root.x + 292, detailY + 76, 390, 20},
                           TextStyle{UITheme::Accent, false, TextAlign::Left});

        r.DrawTextUtf8(FontHandle{}, "Mao direita: " + ItemDisplay(selected->rightItemName, selected->rightItemCode),
                       {root.x + 330, detailY + 80, 350, 24},
                       TextStyle{UITheme::Text, false, TextAlign::Left});
        r.DrawTextUtf8(FontHandle{}, "Mao esquerda: " + ItemDisplay(selected->leftItemName, selected->leftItemCode),
                       {root.x + 330, detailY + 112, 350, 24},
                       TextStyle{UITheme::Text, false, TextAlign::Left});
    }
    else
    {
        r.DrawTextUtf8(FontHandle{}, "Selecione outro personagem da conta.",
                       {root.x + 292, detailY + 52, 390, 24},
                       TextStyle{UITheme::TextMuted, false, TextAlign::Left});
    }

    const std::string result = ResultText(snapshot.lastResult);
    if (!result.empty())
        r.DrawTextUtf8(FontHandle{}, result, {root.x + 22, root.y + root.height - 30, root.width - 44, 20},
                       TextStyle{snapshot.lastResult > 0 ? UITheme::Accent : UITheme::TextMuted, false, TextAlign::Left});
}
}
