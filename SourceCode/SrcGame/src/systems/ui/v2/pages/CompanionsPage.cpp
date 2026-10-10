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

const char* CustomConditionName(int condition)
{
    switch(condition)
    {
        case 1: return "Self HP";
        case 2: return "Owner HP";
        case 3: return "Enemies";
        case 4: return "Owner target";
        case 5: return "Party threat";
        case 0:
        default: return "Always";
    }
}

const char* CustomActionName(int action)
{
    switch(action)
    {
        case 0: return "Owner target";
        case 1: return "Defend";
        case 2: return "Nearest";
        case 3: return "Retreat";
        case 4: return "Hold";
        default: return "Nearest";
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

std::string SkillDisplay(const CompanionUiSkillSlot& skill)
{
    if (!skill.useSlot || !skill.skillCode)
        return "Desligado";
    std::string text = skill.name.empty() ? ("Skill " + std::to_string(skill.useSlot)) : skill.name;
    if (skill.point > 0)
        text += " L" + std::to_string(skill.point);
    return text;
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

    auto& equipmentMode = AddChild<Button>(655, "Equipamento");
    equipmentMode.SetOnClick([this](const UIEvent&)
    {
        tacticsView_ = false;
        skillsView_ = false;
        buffSkillsView_ = false;
        RefreshControls();
    });
    equipmentModeButton_ = &equipmentMode;

    auto& detailMode = AddChild<Button>(652, "Skills");
    detailMode.SetOnClick([this](const UIEvent&)
    {
        tacticsView_ = false;
        skillsView_ = true;
        buffSkillsView_ = false;
        RefreshControls();
    });
    detailModeButton_ = &detailMode;

    auto& skillMode = AddChild<Button>(653, "Suporte");
    skillMode.SetOnClick([this](const UIEvent&)
    {
        tacticsView_ = false;
        skillsView_ = true;
        buffSkillsView_ = true;
        RefreshControls();
    });
    skillModeButton_ = &skillMode;

    auto& tacticsMode = AddChild<Button>(654, "Taticas custom");
    tacticsMode.SetOnClick([this](const UIEvent&)
    {
        tacticsView_ = !tacticsView_;
        if (tacticsView_) skillsView_ = false;
        RefreshControls();
    });
    tacticsModeButton_ = &tacticsMode;

    auto& customToggle = AddChild<Button>(719, "Custom OFF");
    customToggle.SetOnClick([this](const UIEvent&)
    {
        ExecuteForSelected(CompanionUiAction::ToggleCustomTactics);
    });
    customTacticsToggleButton_ = &customToggle;

    for (std::size_t slot = 0; slot < 4; ++slot)
    {
        auto& activePrev = AddChild<Button>(660 + static_cast<UIId>(slot) * 2, "<");
        activePrev.SetOnClick([this, slot](const UIEvent&)
        {
            ExecuteForSelected(CompanionUiAction::CycleActiveSkill, -static_cast<int>(slot + 1));
        });
        activeSkillPrevButtons_[slot] = &activePrev;

        auto& activeNext = AddChild<Button>(661 + static_cast<UIId>(slot) * 2, ">");
        activeNext.SetOnClick([this, slot](const UIEvent&)
        {
            ExecuteForSelected(CompanionUiAction::CycleActiveSkill, static_cast<int>(slot + 1));
        });
        activeSkillNextButtons_[slot] = &activeNext;

        auto& minResource = AddChild<Button>(700 + static_cast<UIId>(slot) * 3, "R0");
        minResource.SetOnClick([this, slot](const UIEvent&)
        {
            ExecuteForSelected(CompanionUiAction::CycleActiveMinResource, static_cast<int>(slot + 1));
        });
        activeMinResourceButtons_[slot] = &minResource;

        auto& minTargets = AddChild<Button>(701 + static_cast<UIId>(slot) * 3, "N1");
        minTargets.SetOnClick([this, slot](const UIEvent&)
        {
            ExecuteForSelected(CompanionUiAction::CycleActiveMinTargets, static_cast<int>(slot + 1));
        });
        activeMinTargetsButtons_[slot] = &minTargets;

        auto& targetMode = AddChild<Button>(702 + static_cast<UIId>(slot) * 3, "TA");
        targetMode.SetOnClick([this, slot](const UIEvent&)
        {
            ExecuteForSelected(CompanionUiAction::CycleActiveTargetMode, static_cast<int>(slot + 1));
        });
        activeTargetModeButtons_[slot] = &targetMode;

        auto& buffPrev = AddChild<Button>(670 + static_cast<UIId>(slot) * 4, "<");
        buffPrev.SetOnClick([this, slot](const UIEvent&)
        {
            ExecuteForSelected(CompanionUiAction::CycleBuffSkill, -static_cast<int>(slot + 1));
        });
        buffSkillPrevButtons_[slot] = &buffPrev;

        auto& buffNext = AddChild<Button>(671 + static_cast<UIId>(slot) * 4, ">");
        buffNext.SetOnClick([this, slot](const UIEvent&)
        {
            ExecuteForSelected(CompanionUiAction::CycleBuffSkill, static_cast<int>(slot + 1));
        });
        buffSkillNextButtons_[slot] = &buffNext;

        auto& intervalPrev = AddChild<Button>(672 + static_cast<UIId>(slot) * 4, "-");
        intervalPrev.SetOnClick([this, slot](const UIEvent&)
        {
            ExecuteForSelected(CompanionUiAction::AdjustBuffInterval, -static_cast<int>(slot + 1));
        });
        buffIntervalPrevButtons_[slot] = &intervalPrev;

        auto& intervalNext = AddChild<Button>(673 + static_cast<UIId>(slot) * 4, "+");
        intervalNext.SetOnClick([this, slot](const UIEvent&)
        {
            ExecuteForSelected(CompanionUiAction::AdjustBuffInterval, static_cast<int>(slot + 1));
        });
        buffIntervalNextButtons_[slot] = &intervalNext;
    }

    for (std::size_t slot = 0; slot < 4; ++slot)
    {
        const int value = static_cast<int>(slot + 1);
        auto& enabled = AddChild<Button>(720 + static_cast<UIId>(slot) * 6, "ON");
        enabled.SetOnClick([this, value](const UIEvent&) { ExecuteForSelected(CompanionUiAction::ToggleTacticRule, value); });
        tacticRuleEnabledButtons_[slot] = &enabled;

        auto& condition = AddChild<Button>(721 + static_cast<UIId>(slot) * 6, "Always");
        condition.SetOnClick([this, value](const UIEvent&) { ExecuteForSelected(CompanionUiAction::CycleTacticCondition, value); });
        tacticRuleConditionButtons_[slot] = &condition;

        auto& action = AddChild<Button>(722 + static_cast<UIId>(slot) * 6, "Nearest");
        action.SetOnClick([this, value](const UIEvent&) { ExecuteForSelected(CompanionUiAction::CycleTacticAction, value); });
        tacticRuleActionButtons_[slot] = &action;

        auto& threshold = AddChild<Button>(723 + static_cast<UIId>(slot) * 6, "-");
        threshold.SetOnClick([this, value](const UIEvent&) { ExecuteForSelected(CompanionUiAction::CycleTacticThreshold, value); });
        tacticRuleThresholdButtons_[slot] = &threshold;

        auto& up = AddChild<Button>(724 + static_cast<UIId>(slot) * 6, "^");
        up.SetOnClick([this, value](const UIEvent&) { ExecuteForSelected(CompanionUiAction::MoveTacticRule, -value); });
        tacticRuleUpButtons_[slot] = &up;

        auto& down = AddChild<Button>(725 + static_cast<UIId>(slot) * 6, "v");
        down.SetOnClick([this, value](const UIEvent&) { ExecuteForSelected(CompanionUiAction::MoveTacticRule, value); });
        tacticRuleDownButtons_[slot] = &down;
    }

    auto& resetActive = AddChild<Button>(690, "Ativas: auto V1");
    resetActive.SetOnClick([this](const UIEvent&) { ExecuteForSelected(CompanionUiAction::ResetActiveSkills); });
    resetActiveSkillsButton_ = &resetActive;

    auto& resetBuffs = AddChild<Button>(691, "Desligar suporte");
    resetBuffs.SetOnClick([this](const UIEvent&) { ExecuteForSelected(CompanionUiAction::ResetBuffs); });
    resetBuffsButton_ = &resetBuffs;

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
    equipmentModeButton_->SetBounds({292, 246, 100, 28});
    tacticsModeButton_->SetBounds({398, 246, 100, 28});
    skillModeButton_->SetBounds({504, 246, 100, 28});
    detailModeButton_->SetBounds({610, 246, 114, 28});
    customTacticsToggleButton_->SetBounds({292, 282, 120, 26});

    for (std::size_t slot = 0; slot < 4; ++slot)
    {
        const int y = 292 + static_cast<int>(slot) * 29;
        // Active and buff controls share the same wide row geometry, but only
        // one group is visible at a time. This gives long skill names room.
        activeSkillPrevButtons_[slot]->SetBounds({292, y, 30, 24});
        activeMinResourceButtons_[slot]->SetBounds({558, y, 38, 24});
        activeMinTargetsButtons_[slot]->SetBounds({600, y, 38, 24});
        activeTargetModeButtons_[slot]->SetBounds({642, y, 38, 24});
        activeSkillNextButtons_[slot]->SetBounds({684, y, 30, 24});

        buffSkillPrevButtons_[slot]->SetBounds({292, y, 30, 24});
        buffSkillNextButtons_[slot]->SetBounds({566, y, 30, 24});
        buffIntervalPrevButtons_[slot]->SetBounds({650, y, 28, 24});
        buffIntervalNextButtons_[slot]->SetBounds({686, y, 28, 24});
    }
    for (std::size_t slot = 0; slot < 4; ++slot)
    {
        const int y = 316 + static_cast<int>(slot) * 29;
        tacticRuleEnabledButtons_[slot]->SetBounds({292, y, 38, 24});
        tacticRuleConditionButtons_[slot]->SetBounds({334, y, 112, 24});
        tacticRuleActionButtons_[slot]->SetBounds({450, y, 100, 24});
        tacticRuleThresholdButtons_[slot]->SetBounds({554, y, 44, 24});
        tacticRuleUpButtons_[slot]->SetBounds({602, y, 30, 24});
        tacticRuleDownButtons_[slot]->SetBounds({636, y, 30, 24});
    }
    resetActiveSkillsButton_->SetBounds({292, 410, 220, 28});
    resetBuffsButton_->SetBounds({292, 410, 220, 28});
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
    const bool equipmentView = !skillsView_ && !tacticsView_;
    equipmentModeButton_->SetEnabled(hasSelected);
    equipmentModeButton_->SetSelected(hasSelected && equipmentView);
    equipmentModeButton_->SetText("Equipamento");
    detailModeButton_->SetEnabled(hasSelected);
    detailModeButton_->SetSelected(hasSelected && skillsView_ && !tacticsView_ && !buffSkillsView_);
    detailModeButton_->SetText("Skills");
    // Keep all top-level tabs stable. Selecting Tactics must not make the
    // Support tab disappear; clicking Support already leaves tacticsView_.
    skillModeButton_->SetVisible(true);
    skillModeButton_->SetEnabled(hasSelected);
    skillModeButton_->SetSelected(hasSelected && skillsView_ && !tacticsView_ && buffSkillsView_);
    skillModeButton_->SetText("Suporte");
    tacticsModeButton_->SetEnabled(hasSelected);
    tacticsModeButton_->SetSelected(tacticsView_);
    tacticsModeButton_->SetText("Taticas custom");

    rightPrevButton_->SetVisible(equipmentView);
    rightNextButton_->SetVisible(equipmentView);
    leftPrevButton_->SetVisible(equipmentView);
    leftNextButton_->SetVisible(equipmentView);
    resetEquipmentButton_->SetVisible(equipmentView);
    resyncButton_->SetVisible(equipmentView);
    rightPrevButton_->SetEnabled(hasSelected && equipmentView);
    rightNextButton_->SetEnabled(hasSelected && equipmentView);
    leftPrevButton_->SetEnabled(hasSelected && equipmentView);
    leftNextButton_->SetEnabled(hasSelected && equipmentView);
    resetEquipmentButton_->SetEnabled(hasSelected && equipmentView);
    resyncButton_->SetEnabled(hasSelected && equipmentView);

    customTacticsToggleButton_->SetVisible(tacticsView_);
    customTacticsToggleButton_->SetEnabled(hasSelected && tacticsView_);
    customTacticsToggleButton_->SetSelected(snapshot.customTacticsEnabled);
    customTacticsToggleButton_->SetText(snapshot.customTacticsEnabled ? "Custom ON" : "Custom OFF");
    for(std::size_t ruleIndex = 0; ruleIndex < 4; ++ruleIndex)
    {
        const auto& rule = snapshot.tacticRules[ruleIndex];
        Button* controls[] = { tacticRuleEnabledButtons_[ruleIndex], tacticRuleConditionButtons_[ruleIndex],
            tacticRuleActionButtons_[ruleIndex], tacticRuleThresholdButtons_[ruleIndex],
            tacticRuleUpButtons_[ruleIndex], tacticRuleDownButtons_[ruleIndex] };
        for(Button* control : controls)
        {
            control->SetVisible(tacticsView_);
            control->SetEnabled(hasSelected && tacticsView_);
        }
        tacticRuleEnabledButtons_[ruleIndex]->SetSelected(rule.enabled);
        tacticRuleEnabledButtons_[ruleIndex]->SetText(rule.enabled ? "ON" : "OFF");
        tacticRuleConditionButtons_[ruleIndex]->SetText(CustomConditionName(rule.condition));
        tacticRuleActionButtons_[ruleIndex]->SetText(CustomActionName(rule.action));
        tacticRuleThresholdButtons_[ruleIndex]->SetText(rule.threshold > 0 ? std::to_string(rule.threshold) : "--");
        tacticRuleUpButtons_[ruleIndex]->SetEnabled(hasSelected && tacticsView_ && ruleIndex > 0);
        tacticRuleDownButtons_[ruleIndex]->SetEnabled(hasSelected && tacticsView_ && ruleIndex + 1 < 4);
    }

    for (std::size_t slot = 0; slot < 4; ++slot)
    {
        const bool buffConfigured = snapshot.valid && snapshot.buffSkills[slot].useSlot != 0;
        const bool activeView = skillsView_ && !tacticsView_ && !buffSkillsView_;
        const bool buffView = skillsView_ && !tacticsView_ && buffSkillsView_;
        activeSkillPrevButtons_[slot]->SetVisible(activeView);
        activeSkillNextButtons_[slot]->SetVisible(activeView);
        activeMinResourceButtons_[slot]->SetVisible(activeView);
        activeMinTargetsButtons_[slot]->SetVisible(activeView);
        activeTargetModeButtons_[slot]->SetVisible(activeView);
        buffSkillPrevButtons_[slot]->SetVisible(buffView);
        buffSkillNextButtons_[slot]->SetVisible(buffView);
        buffIntervalPrevButtons_[slot]->SetVisible(buffView);
        buffIntervalNextButtons_[slot]->SetVisible(buffView);
        activeSkillPrevButtons_[slot]->SetEnabled(hasSelected && activeView);
        activeSkillNextButtons_[slot]->SetEnabled(hasSelected && activeView);
        const bool activeConfigured = snapshot.valid && snapshot.activeSkills[slot].useSlot != 0;
        activeMinResourceButtons_[slot]->SetEnabled(hasSelected && activeView && activeConfigured);
        activeMinTargetsButtons_[slot]->SetEnabled(hasSelected && activeView && activeConfigured);
        activeTargetModeButtons_[slot]->SetEnabled(hasSelected && activeView && activeConfigured);
        if(snapshot.valid)
        {
            const auto& policy = snapshot.activeSkills[slot];
            activeMinResourceButtons_[slot]->SetText("R" + std::to_string(policy.minResourcePercent));
            activeMinTargetsButtons_[slot]->SetText("N" + std::to_string(policy.minimumTargets));
            activeTargetModeButtons_[slot]->SetText(policy.targetMode == 1 ? "TO" : (policy.targetMode == 2 ? "TN" : "TA"));
        }
        buffSkillPrevButtons_[slot]->SetEnabled(hasSelected && buffView);
        buffSkillNextButtons_[slot]->SetEnabled(hasSelected && buffView);
        buffIntervalPrevButtons_[slot]->SetEnabled(hasSelected && buffView && buffConfigured);
        buffIntervalNextButtons_[slot]->SetEnabled(hasSelected && buffView && buffConfigured);
    }
    resetActiveSkillsButton_->SetVisible(skillsView_ && !tacticsView_ && !buffSkillsView_);
    resetBuffsButton_->SetVisible(skillsView_ && !tacticsView_ && buffSkillsView_);
    resetActiveSkillsButton_->SetEnabled(hasSelected && skillsView_ && !tacticsView_ && !buffSkillsView_);
    resetBuffsButton_->SetEnabled(hasSelected && skillsView_ && !tacticsView_ && buffSkillsView_);

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
    // PATCH 213: the custom-tactics grid owns the detail area. Do not leave
    // the behavior summary visually underneath its controls.
    if (selected && !tacticsView_)
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
    r.DrawRect({root.x + 274, detailY, root.width - 298, 192}, UITheme::Surface);

    if (tacticsView_)
    {
        // The mode buttons and Custom ON/OFF already communicate the view.
        // Do not render implementation guidance in the control row: it looked
        // like stray text under the tabs and collided with localized labels.
        if(!selected)
        {
            r.DrawTextUtf8(FontHandle{}, "Selecione outro personagem da conta.",
                           {root.x + 292, detailY + 72, 390, 24},
                           TextStyle{UITheme::TextMuted, false, TextAlign::Left});
        }
    }
    else if (skillsView_)
    {
        if (!buffSkillsView_)
        {
            r.DrawTextUtf8(FontHandle{}, "SKILLS ATIVAS", {root.x + 292, detailY + 8, 220, 20},
                           TextStyle{UITheme::TextMuted, true, TextAlign::Left});
            const std::string activeMode = snapshot.skillLoadoutActive ? "CUSTOM" : "AUTO V1";
            r.DrawTextUtf8(FontHandle{}, activeMode, {root.x + 600, detailY + 8, 104, 20},
                           TextStyle{snapshot.skillLoadoutActive ? UITheme::Accent : UITheme::TextMuted, true, TextAlign::Right});

            if (selected)
            {
                for (std::size_t slot = 0; slot < 4; ++slot)
                {
                    const int y = detailY + 46 + static_cast<int>(slot) * 29;
                    const auto& active = snapshot.activeSkills[slot];
                    r.DrawTextUtf8(FontHandle{},
                                   std::to_string(slot + 1) + ". " + SkillDisplay(active),
                                   {root.x + 330, y, 218, 22},
                                   TextStyle{UITheme::Text, false, TextAlign::Left});
                }
            }
            else
            {
                r.DrawTextUtf8(FontHandle{}, "Selecione outro personagem da conta.",
                               {root.x + 292, detailY + 72, 390, 24},
                               TextStyle{UITheme::TextMuted, false, TextAlign::Left});
            }
        }
        else
        {
            r.DrawTextUtf8(FontHandle{}, "SUPORTE / RECAST", {root.x + 292, detailY + 8, 240, 20},
                           TextStyle{UITheme::TextMuted, true, TextAlign::Left});
            const std::string buffMode = snapshot.buffLoadoutActive ? "ON" : "OFF";
            r.DrawTextUtf8(FontHandle{}, buffMode, {root.x + 600, detailY + 8, 104, 20},
                           TextStyle{snapshot.buffLoadoutActive ? UITheme::Accent : UITheme::TextMuted, true, TextAlign::Right});

            if (selected)
            {
                for (std::size_t slot = 0; slot < 4; ++slot)
                {
                    const int y = detailY + 46 + static_cast<int>(slot) * 29;
                    const auto& buff = snapshot.buffSkills[slot];
                    r.DrawTextUtf8(FontHandle{},
                                   std::to_string(slot + 1) + ". " + SkillDisplay(buff),
                                   {root.x + 330, y, 226, 22},
                                   TextStyle{UITheme::Text, false, TextAlign::Left});

                    const std::string interval = (buff.useSlot && buff.intervalSeconds > 0)
                        ? std::to_string(buff.intervalSeconds) + "s"
                        : std::string("--");
                    r.DrawTextUtf8(FontHandle{}, interval,
                                   {root.x + 602, y, 44, 22},
                                   TextStyle{UITheme::Accent, false, TextAlign::Right});
                }
            }
            else
            {
                r.DrawTextUtf8(FontHandle{}, "Selecione outro personagem da conta.",
                               {root.x + 292, detailY + 72, 390, 24},
                               TextStyle{UITheme::TextMuted, false, TextAlign::Left});
            }
        }
    }
    else
    {
        r.DrawTextUtf8(FontHandle{}, "EQUIPAMENTO DO COMPANION", {root.x + 292, detailY + 12, 300, 20},
                       TextStyle{UITheme::TextMuted, true, TextAlign::Left});
        if (selected)
        {
            r.DrawTextUtf8(FontHandle{}, selected->name + " - Lv " + std::to_string(selected->level),
                           {root.x + 292, detailY + 34, 280, 22},
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
    }

    const std::string result = ResultText(snapshot.lastResult);
    if (!result.empty())
        r.DrawTextUtf8(FontHandle{}, result, {root.x + 22, root.y + root.height - 30, root.width - 44, 20},
                       TextStyle{snapshot.lastResult > 0 ? UITheme::Accent : UITheme::TextMuted, false, TextAlign::Left});
}
}
