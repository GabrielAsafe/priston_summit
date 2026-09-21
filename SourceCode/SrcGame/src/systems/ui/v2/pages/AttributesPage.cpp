#include "systems/ui/v2/pages/AttributesPage.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>
#include "systems/ui/v2/rendering/IUIRenderer.h"
#include "systems/ui/v2/theme/UITheme.h"
#include "systems/ui/v2/widgets/Button.h"
#include "systems/ui/v2/widgets/TextLayout.h"

namespace ui::v2
{
namespace
{
constexpr std::array<const char*, 5> AttributeNames = {{"Forca", "Espirito", "Talento", "Destreza", "Vida"}};
constexpr std::array<const char*, 8> ResistanceNames = {{"Organico", "Terra", "Fogo", "Gelo", "Raio", "Veneno", "Agua", "Vento"}};

int AttributeValue(const CharacterStatsSnapshot& s, std::size_t index)
{
    switch (static_cast<CharacterAttribute>(index))
    {
        case CharacterAttribute::Strength: return s.strength;
        case CharacterAttribute::Spirit: return s.spirit;
        case CharacterAttribute::Talent: return s.talent;
        case CharacterAttribute::Dexterity: return s.dexterity;
        case CharacterAttribute::Health: return s.health;
    }
    return 0;
}

std::string Delta(float value)
{
    std::ostringstream stream;
    stream << '+';
    if (std::fabs(value - std::round(value)) < 0.001f)
        stream << static_cast<int>(std::round(value));
    else
        stream << std::fixed << std::setprecision(value < 1.0f ? 2 : 1) << value;
    return stream.str();
}

std::string ImpactText(const AttributeImpactSnapshot& impact)
{
    std::string result;
    const auto add = [&result](const std::string& text)
    {
        if (!result.empty()) result += "  ";
        result += text;
    };
    if (impact.life > 0.0f) add("Vida " + Delta(impact.life));
    if (impact.mana > 0.0f) add("Mana " + Delta(impact.mana));
    if (impact.stamina > 0.0f) add("Sta " + Delta(impact.stamina));
    if (impact.attackRating > 0.0f) add("Ataque " + Delta(impact.attackRating));
    if (impact.defense > 0.0f) add("Def " + Delta(impact.defense));
    if (impact.maxWeight > 0.0f) add("Peso " + Delta(impact.maxWeight));
    if (impact.affectsDamage) add("Dano*");
    if (impact.affectsAbsorption) add("Absorcao");
    if (impact.affectsMoveSpeed) add("Velocidade");
    return result;
}

void DrawMetric(IUIRenderer& renderer, const Rect& rect, const char* label, const std::string& value)
{
    renderer.DrawTextUtf8(FontHandle{}, label, {rect.x, rect.y, rect.width * 2 / 3, rect.height},
                          TextStyle{UITheme::TextMuted, false, TextAlign::Left});
    renderer.DrawTextUtf8(FontHandle{}, value, {rect.x, rect.y, rect.width, rect.height},
                          TextStyle{UITheme::Text, true, TextAlign::Right});
}

void DrawResource(IUIRenderer& renderer, int x, int y, int width, const char* label, int current, int maximum, Color fill)
{
    renderer.DrawTextUtf8(FontHandle{}, label, {x, y, 75, 20}, TextStyle{UITheme::TextMuted, false, TextAlign::Left});
    const Rect bar{x + 78, y + 3, width - 78, 13};
    renderer.DrawRect(bar, UITheme::Slot);
    if (maximum > 0)
    {
        const float ratio = std::clamp(static_cast<float>(current) / static_cast<float>(maximum), 0.0f, 1.0f);
        renderer.DrawRect({bar.x, bar.y, static_cast<int>(bar.width * ratio), bar.height}, fill);
    }
    renderer.DrawTextUtf8(FontHandle{}, std::to_string(current) + " / " + std::to_string(maximum), bar,
                          TextStyle{UITheme::Text, false, TextAlign::Center});
}
}

AttributesPage::AttributesPage(ICharacterStatsGateway& gateway, UIId id)
    : Container(id), viewModel_(gateway)
{
    BuildControls();
    viewModel_.Refresh();
}

void AttributesPage::BuildControls()
{
    for (std::size_t i = 0; i < buttons_.size(); ++i)
    {
        auto& one = AddChild<Button>(100 + static_cast<UIId>(i) * 2, "+1");
        one.SetOnClick([this, i](const UIEvent&) { viewModel_.Add(static_cast<CharacterAttribute>(i), 1); });
        buttons_[i].plusOne = &one;

        auto& five = AddChild<Button>(101 + static_cast<UIId>(i) * 2, "+5");
        five.SetOnClick([this, i](const UIEvent&) { viewModel_.Add(static_cast<CharacterAttribute>(i), 5); });
        buttons_[i].plusTen = &five;
    }

    auto& undo = AddChild<Button>(120, "Desfazer ultimo ponto");
    undo.SetOnClick([this](const UIEvent&) { viewModel_.Undo(); });
    undoButton_ = &undo;
    LayoutControls();
}

void AttributesPage::LayoutControls()
{
    constexpr int rowStart = 72;
    constexpr int rowHeight = 62;
    for (std::size_t i = 0; i < buttons_.size(); ++i)
    {
        const int y = rowStart + static_cast<int>(i) * rowHeight;
        buttons_[i].plusOne->SetBounds({286, y + 10, 52, 30});
        buttons_[i].plusTen->SetBounds({344, y + 10, 58, 30});
    }
    if (undoButton_)
        undoButton_->SetBounds({24, 414, 190, 30});
}

void AttributesPage::Update(float deltaSeconds)
{
    refreshAccumulator_ += deltaSeconds;
    if (refreshAccumulator_ >= 0.12f)
    {
        refreshAccumulator_ = 0.0f;
        viewModel_.Refresh();
    }

    const bool hasPoints = viewModel_.Snapshot().availablePoints > 0;
    for (auto& row : buttons_)
    {
        row.plusOne->SetEnabled(hasPoints);
        row.plusTen->SetEnabled(viewModel_.Snapshot().availablePoints >= 5);
    }
    Container::Update(deltaSeconds);
}

void AttributesPage::OnRender(UIRenderContext& context)
{
    const Rect root = GlobalBounds();
    auto& r = context.Renderer();
    r.DrawRect(root, UITheme::WindowBg);

    const auto& s = viewModel_.Snapshot();
    if (!s.valid)
    {
        r.DrawTextUtf8(FontHandle{}, "Dados do personagem indisponiveis.", {root.x + 24, root.y + 24, root.width - 48, 30},
                       TextStyle{UITheme::TextMuted, false, TextAlign::Left});
        return;
    }

    r.DrawTextUtf8(FontHandle{}, EllipsizeText(s.name.empty() ? "Personagem" : s.name, 210),
                   {root.x + 24, root.y + 18, 210, 28}, TextStyle{UITheme::Text, true, TextAlign::Left});
    r.DrawTextUtf8(FontHandle{}, EllipsizeText(s.jobName + " - Nivel " + std::to_string(s.level), 220),
                   {root.x + 240, root.y + 18, 220, 28}, TextStyle{UITheme::TextMuted, false, TextAlign::Left});
    r.DrawTextUtf8(FontHandle{}, "Pontos disponiveis: " + std::to_string(s.availablePoints),
                   {root.x + root.width - 250, root.y + 18, 220, 28},
                   TextStyle{s.availablePoints > 0 ? UITheme::Accent : UITheme::TextMuted, true, TextAlign::Right});

    r.DrawRect({root.x + 20, root.y + 54, 392, 1}, UITheme::BorderSoft);
    r.DrawTextUtf8(FontHandle{}, "ATRIBUTOS / POR PONTO", {root.x + 24, root.y + 58, 250, 22},
                   TextStyle{UITheme::TextMuted, true, TextAlign::Left});

    constexpr int rowStart = 72;
    constexpr int rowHeight = 62;
    for (std::size_t i = 0; i < AttributeNames.size(); ++i)
    {
        const int y = root.y + rowStart + static_cast<int>(i) * rowHeight;
        r.DrawRect({root.x + 20, y, 392, 57}, i % 2 == 0 ? UITheme::Surface : UITheme::SurfaceRaised);
        r.DrawTextUtf8(FontHandle{}, AttributeNames[i], {root.x + 34, y + 5, 145, 22},
                       TextStyle{UITheme::TextMuted, false, TextAlign::Left});
        r.DrawTextUtf8(FontHandle{}, std::to_string(AttributeValue(s, i)), {root.x + 178, y + 5, 84, 22},
                       TextStyle{UITheme::Text, true, TextAlign::Right});
        const auto impactLines = WrapTextApprox(ImpactText(s.attributeImpacts[i]), 246, 2, 7);
        for (std::size_t line = 0; line < impactLines.size(); ++line)
            r.DrawTextUtf8(FontHandle{}, impactLines[line], {root.x + 34, y + 27 + static_cast<int>(line) * 16, 246, 16},
                           TextStyle{UITheme::TextMuted, false, TextAlign::Left});
    }
    r.DrawTextUtf8(FontHandle{}, "* Dano depende da classe e da arma equipada.", {root.x + 24, root.y + 384, 388, 18},
                   TextStyle{UITheme::TextMuted, false, TextAlign::Left});

    const int rightX = root.x + 438;
    const int rightW = root.width - 462;
    r.DrawTextUtf8(FontHandle{}, "RECURSOS", {rightX, root.y + 58, rightW, 22},
                   TextStyle{UITheme::TextMuted, true, TextAlign::Left});
    DrawResource(r, rightX, root.y + 84, rightW, "Vida", s.lifeCurrent, s.lifeMax, Color{164, 72, 79, 255});
    DrawResource(r, rightX, root.y + 108, rightW, "Mana", s.manaCurrent, s.manaMax, Color{74, 116, 178, 255});
    DrawResource(r, rightX, root.y + 132, rightW, "Stamina", s.staminaCurrent, s.staminaMax, Color{115, 153, 85, 255});

    r.DrawTextUtf8(FontHandle{}, "COMBATE", {rightX, root.y + 170, rightW, 22},
                   TextStyle{UITheme::TextMuted, true, TextAlign::Left});
    const int half = (rightW - 12) / 2;
    DrawMetric(r, {rightX, root.y + 196, half, 20}, "Dano", std::to_string(s.damageMin) + "-" + std::to_string(s.damageMax));
    DrawMetric(r, {rightX + half + 12, root.y + 196, half, 20}, "Ataque", std::to_string(s.attackRating));
    DrawMetric(r, {rightX, root.y + 220, half, 20}, "Defesa", std::to_string(s.defense));
    DrawMetric(r, {rightX + half + 12, root.y + 220, half, 20}, "Absorcao", std::to_string(s.absorption));
    DrawMetric(r, {rightX, root.y + 244, half, 20}, "Critico", std::to_string(s.critical) + "%");
    DrawMetric(r, {rightX + half + 12, root.y + 244, half, 20}, "Bloqueio", std::to_string(s.block) + "%");
    DrawMetric(r, {rightX, root.y + 268, half, 20}, "Veloc.", std::to_string(s.moveSpeed));
    DrawMetric(r, {rightX + half + 12, root.y + 268, half, 20}, "Peso", std::to_string(s.weightCurrent) + "/" + std::to_string(s.weightMax));

    r.DrawTextUtf8(FontHandle{}, "RESISTENCIAS", {rightX, root.y + 310, rightW, 22},
                   TextStyle{UITheme::TextMuted, true, TextAlign::Left});
    for (std::size_t i = 0; i < ResistanceNames.size(); ++i)
    {
        const int column = static_cast<int>(i % 2);
        const int row = static_cast<int>(i / 2);
        const int x = rightX + column * (half + 12);
        DrawMetric(r, {x, root.y + 338 + row * 24, half, 20}, ResistanceNames[i], std::to_string(s.resistances[i]));
    }
}
}
