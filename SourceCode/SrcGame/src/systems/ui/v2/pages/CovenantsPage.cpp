#include "systems/ui/v2/pages/CovenantsPage.h"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include "systems/ui/v2/rendering/IUIRenderer.h"
#include "systems/ui/v2/theme/UITheme.h"
#include "systems/ui/v2/widgets/Button.h"

namespace ui::v2
{
namespace
{
std::string CovenantName(int id, const std::string& key)
{
    switch (id)
    {
        case 1: return "Hunters";
        case 2: return "Gatherers";
        case 3: return "Artisans";
        case 4: return "Fortune";
        case 5: return "Ascendants";
        default: return key.empty() ? "Covenant" : key;
    }
}

std::string CovenantSpecialization(int id)
{
    switch (id)
    {
        case 1: return "Hunting / monstros especiais";
        case 2: return "Mining / Gathering / recursos";
        case 3: return "Aging / Mixing / Craft";
        case 4: return "Loot geral com risco aumentado";
        case 5: return "Progressao / XP com menos loot";
        default: return {};
    }
}

std::string Humanize(std::string value)
{
    bool upper = true;
    for (char& c : value)
    {
        if (c == '_' || c == '.')
        {
            c = ' ';
            upper = true;
        }
        else if (upper)
        {
            c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
            upper = false;
        }
    }
    return value;
}

std::string RankName(int covenantId, const std::string& key)
{
    if (key.empty()) return "Nao aderiu";
    if (covenantId == 1)
    {
        if (key == "tracker") return "Tracker";
        if (key == "trophy_refining") return "Trophy Refining";
        if (key == "hunters_mark") return "Hunter's Mark";
    }
    if (covenantId == 2)
    {
        if (key == "surveyor") return "Surveyor";
        if (key == "prosperous_harvest") return "Prosperous Harvest";
        if (key == "refined_materials") return "Refined Materials";
    }
    if (covenantId == 3)
    {
        if (key == "reforge") return "Reforge";
        if (key == "forgemaster") return "Forgemaster";
        if (key == "master_artisan") return "Master Artisan";
    }
    if (covenantId == 4)
    {
        if (key == "fortune") return "Fortune";
        if (key == "fortune_roll") return "Fortune Roll";
        if (key == "tempt_fate") return "Tempt Fate";
    }
    if (covenantId == 5)
    {
        if (key == "disciple") return "Disciple";
        if (key == "training") return "Training";
        if (key == "ascension") return "Ascension";
    }
    return Humanize(key);
}

std::string ModifierLabel(const std::string& key)
{
    if (key == "hunt.drop_chance") return "Drop em Hunt";
    if (key == "hunt.trophy_chance") return "Chance de Trophy";
    if (key == "hunt.trophy_refining_enabled") return "Trophy Refining";
    if (key == "hunt.mark_enabled") return "Hunter's Mark";
    if (key == "combat.damage_taken") return "Dano recebido";
    if (key == "gathering.surveyor_enabled") return "Surveyor";
    if (key == "gathering.extra_yield_chance") return "Coleta adicional";
    if (key == "gathering.special_material_chance") return "Material especial";
    if (key == "gathering.refined_material_chance") return "Material refinado";
    if (key == "gathering.yield") return "Rendimento de coleta";
    if (key == "crafting.reforge_enabled") return "Reforge";
    if (key == "crafting.forgemaster_enabled") return "Forgemaster";
    if (key == "crafting.master_artisan_enabled") return "Master Artisan";
    if (key == "vendor.blacksmith_sell_value") return "Venda ao Blacksmith";
    if (key == "monster.drop_chance") return "Drop de monstros";
    if (key == "loot.extra_roll_chance") return "Rolagem extra de loot";
    if (key == "fortune.tempt_fate_enabled") return "Tempt Fate";
    if (key == "xp.monster_gain") return "XP de monstros";
    if (key == "ascendant.focused_training_enabled") return "Focused Training";
    return Humanize(key);
}

std::string ModifierText(const CovenantUiModifier& modifier)
{
    std::ostringstream out;
    out << ModifierLabel(modifier.statKey) << ": ";
    if (modifier.operation == 0)
    {
        if (modifier.valueBasisPoints == 10000)
            out << "desbloqueado";
        else
            out << std::fixed << std::setprecision(2) << modifier.valueBasisPoints / 10000.0;
    }
    else
    {
        const double percent = modifier.valueBasisPoints / 100.0;
        if (percent >= 0.0) out << "+";
        out << std::fixed << std::setprecision((modifier.valueBasisPoints % 100) ? 2 : 0) << percent << "%";
    }
    return out.str();
}

std::string PowerName(const std::string& key)
{
    if (key == "hunters_mark") return "Hunter's Mark";
    if (key == "tempt_fate") return "Tempt Fate";
    if (key == "focused_training") return "Focused Training";
    return Humanize(key);
}

const char* ArtisanModeName(int mode)
{
    switch (mode)
    {
        case 1: return "Reforge";
        case 2: return "Forgemaster";
        case 3: return "Master";
        default: return "Off";
    }
}

std::string InventoryDisplay(std::string key)
{
    if (key.rfind("sigil:", 0) == 0)
        return std::string("Sigil: ") + Humanize(key.substr(6));
    if (key.rfind("refined_trophy:", 0) == 0)
        return std::string("Trophy refinado: ") + Humanize(key.substr(15));
    if (key.rfind("trophy:", 0) == 0)
        return std::string("Trophy: ") + Humanize(key.substr(7));
    return Humanize(key);
}

void DrawBorder(IUIRenderer& r, const Rect& rect, Color color)
{
    r.DrawRect({rect.x, rect.y, rect.width, 1}, color);
    r.DrawRect({rect.x, rect.y + rect.height - 1, rect.width, 1}, color);
    r.DrawRect({rect.x, rect.y, 1, rect.height}, color);
    r.DrawRect({rect.x + rect.width - 1, rect.y, 1, rect.height}, color);
}
}

CovenantsPage::CovenantsPage(ICovenantGateway& gateway, UIId id)
    : Container(id), gateway_(gateway)
{
    BuildControls();
    Layout();
}

void CovenantsPage::BuildControls()
{
    static const char* names[CovenantUiMaxCovenants] = {
        "Hunters", "Gatherers", "Artisans", "Fortune", "Ascendants"
    };
    for (std::size_t i = 0; i < covenantButtons_.size(); ++i)
    {
        auto& button = AddChild<Button>(700 + static_cast<UIId>(i), names[i]);
        const int covenantId = static_cast<int>(i) + 1;
        button.SetOnClick([this, covenantId](const UIEvent&) { gateway_.SelectCovenant(covenantId); });
        covenantButtons_[i] = &button;
    }

    auto& primary = AddChild<Button>(710, "Entrar");
    primary.SetOnClick([this](const UIEvent&)
    {
        const CovenantUiEntry* entry = SelectedEntry();
        if (!entry) return;
        gateway_.Execute(entry->joined ? CovenantUiAction::Activate : CovenantUiAction::Join);
    });
    primaryButton_ = &primary;

    auto& power = AddChild<Button>(711, "Poder");
    power.SetOnClick([this](const UIEvent&) { gateway_.Execute(CovenantUiAction::UsePower); });
    powerButton_ = &power;

    auto& artisan = AddChild<Button>(712, "Artisan: Off");
    artisan.SetOnClick([this](const UIEvent&)
    {
        const auto& s = gateway_.Snapshot();
        if (s.artisanMaxMode <= 0) return;
        const int next = (s.artisanMode + 1) % (s.artisanMaxMode + 1);
        gateway_.Execute(CovenantUiAction::SetArtisanMode, next);
    });
    artisanModeButton_ = &artisan;

    auto& refine = AddChild<Button>(713, "Refinar Trophy");
    refine.SetOnClick([this](const UIEvent&)
    {
        const std::string key = FirstRefinableTrophy();
        if (!key.empty()) gateway_.Execute(CovenantUiAction::RefineTrophy, 0, key);
    });
    refineButton_ = &refine;

    auto& mine = AddChild<Button>(715, "Minerar proximo");
    mine.SetOnClick([this](const UIEvent&) { gateway_.Execute(CovenantUiAction::HarvestMining); });
    mineButton_ = &mine;

    auto& gather = AddChild<Button>(716, "Coletar proximo");
    gather.SetOnClick([this](const UIEvent&) { gateway_.Execute(CovenantUiAction::HarvestGathering); });
    gatherButton_ = &gather;

    auto& refresh = AddChild<Button>(714, "Atualizar");
    refresh.SetOnClick([this](const UIEvent&) { ForceRefresh(); });
    refreshButton_ = &refresh;
}

void CovenantsPage::Layout()
{
    for (std::size_t i = 0; i < covenantButtons_.size(); ++i)
        covenantButtons_[i]->SetBounds({16, 46 + static_cast<int>(i) * 42, 170, 34});

    const int actionY = std::max(396, Bounds().height - 44);
    primaryButton_->SetBounds({202, actionY, 142, 30});
    powerButton_->SetBounds({352, actionY, 142, 30});
    artisanModeButton_->SetBounds({502, actionY, 158, 30});
    refineButton_->SetBounds({502, actionY - 38, 158, 30});
    mineButton_->SetBounds({202, actionY - 38, 142, 30});
    gatherButton_->SetBounds({352, actionY - 38, 142, 30});
    refreshButton_->SetBounds({666, actionY, 92, 30});
}

void CovenantsPage::ForceRefresh()
{
    refreshAccumulator_ = 0.0f;
    gateway_.RequestRefresh();
}

const CovenantUiEntry* CovenantsPage::SelectedEntry() const noexcept
{
    const auto& snapshot = gateway_.Snapshot();
    if (!snapshot.valid) return nullptr;
    for (int i = 0; i < snapshot.entryCount; ++i)
        if (snapshot.entries[static_cast<std::size_t>(i)].covenantId == snapshot.selectedCovenantId)
            return &snapshot.entries[static_cast<std::size_t>(i)];
    return nullptr;
}

std::string CovenantsPage::FirstRefinableTrophy() const
{
    const auto& snapshot = gateway_.Snapshot();
    for (int i = 0; i < snapshot.inventoryCount; ++i)
    {
        const auto& item = snapshot.inventory[static_cast<std::size_t>(i)];
        if (item.quantity > 0 && item.key.rfind("trophy:", 0) == 0)
            return item.key;
    }
    return {};
}

void CovenantsPage::RefreshControls()
{
    const auto& snapshot = gateway_.Snapshot();
    const CovenantUiEntry* selected = SelectedEntry();

    for (std::size_t i = 0; i < covenantButtons_.size(); ++i)
    {
        Button* button = covenantButtons_[i];
        const int id = static_cast<int>(i) + 1;
        const CovenantUiEntry* entry = nullptr;
        for (int j = 0; j < snapshot.entryCount; ++j)
            if (snapshot.entries[static_cast<std::size_t>(j)].covenantId == id)
                entry = &snapshot.entries[static_cast<std::size_t>(j)];
        std::string text = CovenantName(id, entry ? entry->key : std::string());
        if (entry)
        {
            if (entry->active) text += "  [ATIVA]";
            else if (entry->joined) text += "  [OK]";
            if (entry->mastered) text += " *";
        }
        button->SetText(text);
        button->SetSelected(snapshot.selectedCovenantId == id);
        button->SetEnabled(snapshot.valid);
    }

    primaryButton_->SetVisible(selected != nullptr);
    if (!selected)
    {
        primaryButton_->SetEnabled(false);
        powerButton_->SetVisible(false);
        artisanModeButton_->SetVisible(false);
        refineButton_->SetVisible(false);
        mineButton_->SetVisible(false);
        gatherButton_->SetVisible(false);
        return;
    }

    if (!selected->joined)
    {
        primaryButton_->SetText("Entrar na Covenant");
        primaryButton_->SetEnabled(selected->canJoin);
    }
    else if (!selected->active)
    {
        primaryButton_->SetText("Ativar Covenant");
        primaryButton_->SetEnabled(snapshot.switchCooldownUntil <= snapshot.serverNow);
    }
    else
    {
        primaryButton_->SetText("Covenant ativa");
        primaryButton_->SetEnabled(false);
    }

    powerButton_->SetVisible(!snapshot.power.key.empty());
    if (!snapshot.power.key.empty())
    {
        if (snapshot.power.activeUntil > snapshot.serverNow)
            powerButton_->SetText(PowerName(snapshot.power.key) + " ATIVO");
        else if (snapshot.power.cooldownUntil > snapshot.serverNow)
            powerButton_->SetText("Cooldown " + std::to_string(snapshot.power.cooldownUntil - snapshot.serverNow) + "s");
        else
            powerButton_->SetText("Usar " + PowerName(snapshot.power.key));
        powerButton_->SetEnabled(selected->active && snapshot.power.available && snapshot.power.cooldownUntil <= snapshot.serverNow);
    }

    const bool artisan = selected->covenantId == 3;
    artisanModeButton_->SetVisible(artisan);
    artisanModeButton_->SetText(std::string("Artisan: ") + ArtisanModeName(snapshot.artisanMode));
    artisanModeButton_->SetEnabled(artisan && selected->active && snapshot.artisanMaxMode > 0);

    const bool canRefine = selected->covenantId == 1 && selected->active && selected->rankId >= 2 && !FirstRefinableTrophy().empty();
    refineButton_->SetVisible(selected->covenantId == 1 && selected->rankId >= 2);
    refineButton_->SetEnabled(canRefine);

    const bool gathererPage = selected->covenantId == 2;
    mineButton_->SetVisible(gathererPage);
    gatherButton_->SetVisible(gathererPage);
    mineButton_->SetEnabled(gathererPage);
    gatherButton_->SetEnabled(gathererPage);
}

void CovenantsPage::Update(float deltaSeconds)
{
    refreshAccumulator_ += deltaSeconds;
    if (refreshAccumulator_ >= 5.0f)
    {
        refreshAccumulator_ = 0.0f;
        gateway_.RequestRefresh();
    }
    RefreshControls();
    Layout();
    Container::Update(deltaSeconds);
}

void CovenantsPage::OnRender(UIRenderContext& context)
{
    const Rect root = GlobalBounds();
    auto& r = context.Renderer();
    r.DrawRect(root, UITheme::WindowBg);

    const Rect left{root.x + 8, root.y + 8, 186, root.height - 16};
    const Rect detail{root.x + 198, root.y + 8, root.width - 206, root.height - 16};
    r.DrawRect(left, UITheme::Surface);
    r.DrawRect(detail, UITheme::Surface);
    DrawBorder(r, left, UITheme::BorderSoft);
    DrawBorder(r, detail, UITheme::BorderSoft);

    r.DrawTextUtf8(FontHandle{}, "COVENANTS", {left.x + 10, left.y + 8, left.width - 20, 24},
                   TextStyle{UITheme::Accent, true, TextAlign::Left});

    const auto& snapshot = gateway_.Snapshot();
    if (!snapshot.valid)
    {
        r.DrawTextUtf8(FontHandle{}, "Aguardando estado do servidor...",
                       {detail.x + 18, detail.y + 18, detail.width - 36, 22},
                       TextStyle{UITheme::TextMuted, false, TextAlign::Left});
        return;
    }

    const CovenantUiEntry* selected = SelectedEntry();
    if (!selected)
        return;

    int y = detail.y + 16;
    const std::string title = CovenantName(selected->covenantId, selected->key);
    r.DrawTextUtf8(FontHandle{}, title, {detail.x + 18, y, 280, 26},
                   TextStyle{UITheme::Text, true, TextAlign::Left});
    r.DrawTextUtf8(FontHandle{}, CovenantSpecialization(selected->covenantId),
                   {detail.x + 18, y + 27, detail.width - 36, 20},
                   TextStyle{UITheme::TextMuted, false, TextAlign::Left});
    y += 56;

    std::string state = selected->active ? "ATIVA" : selected->joined ? "ADERIDA / INATIVA" : "NAO ADERIDA";
    if (selected->mastered) state += "  |  MASTERY";
    r.DrawTextUtf8(FontHandle{}, state, {detail.x + 18, y, 260, 20},
                   TextStyle{selected->active ? UITheme::Accent : UITheme::TextMuted, true, TextAlign::Left});

    if (selected->joined)
    {
        r.DrawTextUtf8(FontHandle{}, "Rank: " + RankName(selected->covenantId, selected->rankKey),
                       {detail.x + 18, y + 24, 300, 20}, TextStyle{UITheme::Text, true, TextAlign::Left});
        const int maxRep = selected->nextRankReputation > 0 ? selected->nextRankReputation : std::max(1, selected->reputation);
        // Rank effects are rendered in the right column. Keep reputation
        // progress entirely in the left column so modifier text can never be
        // painted over the bar.
        const int repBarWidth = std::clamp(detail.width / 2 - 30, 40, 300);
        const int fill = repBarWidth * std::min(selected->reputation, maxRep) / std::max(1, maxRep);
        const Rect bar{detail.x + 18, y + 49, repBarWidth, 16};
        r.DrawRect(bar, UITheme::Slot);
        if (fill > 0) r.DrawRect({bar.x, bar.y, fill, bar.height}, UITheme::AccentMuted);
        DrawBorder(r, bar, UITheme::BorderSoft);
        const std::string rep = selected->nextRankReputation > 0 ?
            std::to_string(selected->reputation) + " / " + std::to_string(selected->nextRankReputation) :
            std::to_string(selected->reputation) + " / MAX";
        r.DrawTextUtf8(FontHandle{}, rep, bar, TextStyle{UITheme::Text, true, TextAlign::Center});
    }
    else
    {
        r.DrawTextUtf8(FontHandle{}, selected->canJoin ? "Disponivel para adesao" : "Requisitos de adesao nao atendidos",
                       {detail.x + 18, y + 28, detail.width - 36, 20},
                       TextStyle{selected->canJoin ? UITheme::Text : UITheme::Disabled, false, TextAlign::Left});
    }

    y += 80;
    r.DrawTextUtf8(FontHandle{}, "TIERS", {detail.x + 18, y, 100, 20},
                   TextStyle{UITheme::TextMuted, true, TextAlign::Left});
    y += 22;
    for (int i = 0; i < snapshot.rankCount; ++i)
    {
        const auto& rank = snapshot.ranks[static_cast<std::size_t>(i)];
        const bool unlocked = selected->joined && selected->rankId >= rank.rankId;
        std::string line = std::string(unlocked ? "[x] " : "[ ] ") + RankName(selected->covenantId, rank.key) +
                           "  (" + std::to_string(rank.minReputation) + " rep)";
        r.DrawTextUtf8(FontHandle{}, line, {detail.x + 18, y, detail.width / 2 - 18, 19},
                       TextStyle{unlocked ? UITheme::Text : UITheme::TextMuted, false, TextAlign::Left});
        y += 20;
    }

    const int rightX = detail.x + detail.width / 2 + 6;
    int rightY = detail.y + 72;
    r.DrawTextUtf8(FontHandle{}, "EFEITOS DO RANK", {rightX, rightY, detail.width / 2 - 22, 20},
                   TextStyle{UITheme::TextMuted, true, TextAlign::Left});
    rightY += 23;
    for (int i = 0; i < snapshot.modifierCount && i < 6; ++i)
    {
        const auto& modifier = snapshot.modifiers[static_cast<std::size_t>(i)];
        r.DrawTextUtf8(FontHandle{}, ModifierText(modifier),
                       {rightX, rightY, detail.width / 2 - 22, 18},
                       TextStyle{modifier.valueBasisPoints < 0 ? Color{231, 135, 135, 255} : UITheme::Text,
                                 false, TextAlign::Left});
        rightY += 19;
    }

    y += 8;
    r.DrawTextUtf8(FontHandle{}, "CHALLENGES", {detail.x + 18, y, 150, 20},
                   TextStyle{UITheme::TextMuted, true, TextAlign::Left});
    y += 22;
    if (!selected->joined)
    {
        r.DrawTextUtf8(FontHandle{}, "Entre na Covenant para iniciar os desafios.",
                       {detail.x + 18, y, detail.width - 36, 19}, TextStyle{UITheme::TextMuted, false, TextAlign::Left});
    }
    else
    {
        for (int i = 0; i < snapshot.challengeCount; ++i)
        {
            const auto& challenge = snapshot.challenges[static_cast<std::size_t>(i)];
            std::string line = std::string(challenge.completed ? "[x] " : "[ ] ") + Humanize(challenge.key) +
                "  " + std::to_string(std::min(challenge.progress, challenge.targetCount)) + "/" +
                std::to_string(challenge.targetCount) + "  +" + std::to_string(challenge.reputationReward) + " rep";
            r.DrawTextUtf8(FontHandle{}, line, {detail.x + 18, y, detail.width - 36, 18},
                           TextStyle{challenge.completed ? UITheme::Accent : UITheme::Text, false, TextAlign::Left});
            y += 19;
        }
    }

    y += 7;
    if (selected->covenantId == 2 && selected->joined)
    {
        r.DrawTextUtf8(FontHandle{}, "SURVEYOR", {detail.x + 18, y, 120, 20},
                       TextStyle{UITheme::TextMuted, true, TextAlign::Left});
        y += 21;
        if (snapshot.surveyAreaCount == 0)
            r.DrawTextUtf8(FontHandle{}, "Nenhum node registrado no Mining registry.", {detail.x + 18, y, detail.width - 36, 18},
                           TextStyle{UITheme::TextMuted, false, TextAlign::Left});
        for (int i = 0; i < snapshot.surveyAreaCount && i < 3; ++i)
        {
            const auto& area = snapshot.surveyAreas[static_cast<std::size_t>(i)];
            r.DrawTextUtf8(FontHandle{}, Humanize(area.resourceKey) + " | area " + std::to_string(area.area) +
                           " | " + std::to_string(area.nodeCount) + " deposits",
                           {detail.x + 18, y, detail.width - 36, 18}, TextStyle{UITheme::Text, false, TextAlign::Left});
            y += 18;
        }
        y += 5;
        r.DrawTextUtf8(FontHandle{}, "RECURSOS COLETADOS", {detail.x + 18, y, 180, 20},
                       TextStyle{UITheme::TextMuted, true, TextAlign::Left});
        y += 21;
        if (snapshot.resourceCount == 0)
            r.DrawTextUtf8(FontHandle{}, "Nenhum recurso coletado ainda.", {detail.x + 18, y, detail.width - 36, 18},
                           TextStyle{UITheme::TextMuted, false, TextAlign::Left});
        for (int i = 0; i < snapshot.resourceCount && i < 3; ++i)
        {
            const auto& item = snapshot.resources[static_cast<std::size_t>(i)];
            r.DrawTextUtf8(FontHandle{}, Humanize(item.key) + " x" + std::to_string(item.quantity),
                           {detail.x + 18, y, detail.width - 36, 18}, TextStyle{UITheme::Text, false, TextAlign::Left});
            y += 18;
        }
    }
    else
    {
        r.DrawTextUtf8(FontHandle{}, "INVENTARIO COVENANT", {detail.x + 18, y, 190, 20},
                       TextStyle{UITheme::TextMuted, true, TextAlign::Left});
        y += 21;
        if (snapshot.inventoryCount == 0)
            r.DrawTextUtf8(FontHandle{}, selected->joined ? "Vazio" : "Disponivel apos adesao",
                           {detail.x + 18, y, 260, 18}, TextStyle{UITheme::TextMuted, false, TextAlign::Left});
        for (int i = 0; i < snapshot.inventoryCount && i < 3; ++i)
        {
            const auto& item = snapshot.inventory[static_cast<std::size_t>(i)];
            r.DrawTextUtf8(FontHandle{}, InventoryDisplay(item.key) + " x" + std::to_string(item.quantity),
                           {detail.x + 18, y, detail.width - 36, 18}, TextStyle{UITheme::Text, false, TextAlign::Left});
            y += 18;
        }
    }

    if (!snapshot.lastMessage.empty())
    {
        const bool ok = snapshot.lastResult > 0;
        r.DrawTextUtf8(FontHandle{}, snapshot.lastMessage,
                       {detail.x + 18, detail.y + detail.height - 76, detail.width - 36, 22},
                       TextStyle{ok ? Color{125, 210, 158, 255} : Color{231, 135, 135, 255}, true, TextAlign::Left});
    }

    if (snapshot.switchCooldownUntil > snapshot.serverNow && !selected->active)
    {
        r.DrawTextUtf8(FontHandle{}, "Troca disponivel em " +
                       std::to_string(snapshot.switchCooldownUntil - snapshot.serverNow) + "s",
                       {detail.x + 18, detail.y + detail.height - 52, 260, 18},
                       TextStyle{UITheme::TextMuted, false, TextAlign::Left});
    }
}
}
