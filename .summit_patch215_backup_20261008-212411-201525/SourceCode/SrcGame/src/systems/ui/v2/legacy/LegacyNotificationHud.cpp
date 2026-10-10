#include "systems/ui/v2/legacy/LegacyNotificationHud.h"

#include <algorithm>
#include <cstring>
#include <cstdio>
#include <string>
#include <vector>
#include <windows.h>

#include "compat/legacy/sin/sinLinkHeader.h"
#include "systems/combat/skills/legacy_sin/sinSkill.h"
#include "systems/commerce/premium/HaPremiumItem.h"
#include "systems/ui/v2/rendering/IUIRenderer.h"
#include "systems/ui/v2/theme/UITheme.h"
#include "systems/ui/v2/widgets/TextLayout.h"

namespace ui::v2::legacy
{
namespace
{
std::string LegacyToUtf8(const char* text)
{
    if (!text || !*text) return {};
    const int wideLen = MultiByteToWideChar(1252, 0, text, -1, nullptr, 0);
    if (wideLen <= 1) return text;
    std::vector<wchar_t> wide(static_cast<std::size_t>(wideLen));
    MultiByteToWideChar(1252, 0, text, -1, wide.data(), wideLen);
    const int utfLen = WideCharToMultiByte(CP_UTF8, 0, wide.data(), -1, nullptr, 0, nullptr, nullptr);
    if (utfLen <= 1) return text;
    std::string out(static_cast<std::size_t>(utfLen), '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide.data(), -1, out.data(), utfLen, nullptr, nullptr);
    if (!out.empty() && out.back() == '\0') out.pop_back();
    return out;
}

void DrawBorder(IUIRenderer& r, const Rect& rect, Color color)
{
    r.DrawRect({rect.x, rect.y, rect.width, 1}, color);
    r.DrawRect({rect.x, rect.y + rect.height - 1, rect.width, 1}, color);
    r.DrawRect({rect.x, rect.y, 1, rect.height}, color);
    r.DrawRect({rect.x + rect.width - 1, rect.y, 1, rect.height}, color);
}

int TextWidthApprox(const std::string& text)
{
    return static_cast<int>(text.size()) * 7;
}

int DrawNotice(UIRenderContext& context, const Rect& viewport, int y, const std::string& text, bool emphasized)
{
    if (text.empty()) return y;

    auto& r = context.Renderer();
    const int width = std::clamp(TextWidthApprox(text) + 48, 260, std::min(620, std::max(260, viewport.width - 32)));
    const auto lines = WrapTextApprox(text, width - 28, 3, 7);
    const int height = 24 + static_cast<int>(std::max<std::size_t>(1, lines.size())) * 18;
    const Rect box{viewport.x + (viewport.width - width) / 2, y, width, height};

    r.DrawRect(box, UITheme::SurfaceRaised);
    DrawBorder(r, box, emphasized ? UITheme::Accent : UITheme::AccentMuted);
    r.DrawRect({box.x, box.y, box.width, 3}, emphasized ? UITheme::Accent : UITheme::AccentMuted);

    for (std::size_t i = 0; i < lines.size(); ++i)
    {
        r.DrawTextUtf8(FontHandle{}, lines[i],
                       {box.x + 14, box.y + 12 + static_cast<int>(i) * 18, box.width - 28, 18},
                       TextStyle{UITheme::Text, emphasized, TextAlign::Center});
    }
    return box.y + box.height + 8;
}

std::string CleanLegacyText(const char* text)
{
    std::string out = LegacyToUtf8(text);
    std::size_t pos = 0;
    while ((pos = out.find("/r", pos)) != std::string::npos)
        out.replace(pos, 2, " ");
    while ((pos = out.find("  ")) != std::string::npos)
        out.replace(pos, 2, " ");
    return out;
}

std::string FormatRemaining(unsigned long seconds)
{
    char buffer[64] = {0};
    if (seconds >= 3600)
        std::snprintf(buffer, sizeof(buffer), "%luh %02lum", seconds / 3600, (seconds % 3600) / 60);
    else if (seconds >= 60)
        std::snprintf(buffer, sizeof(buffer), "%lum %02lus", seconds / 60, seconds % 60);
    else
        std::snprintf(buffer, sizeof(buffer), "%lus", seconds);
    return buffer;
}

void DrawStatusTooltip(UIRenderContext& context, const Rect& viewport,
                       const std::string& title, const std::vector<std::string>& details)
{
    auto& r = context.Renderer();
    int width = std::max(180, TextWidthApprox(title) + 30);
    for (const auto& line : details)
        width = std::max(width, TextWidthApprox(line) + 30);
    width = std::min(width, std::max(180, viewport.width - 24));

    const int height = 34 + static_cast<int>(details.size()) * 18;
    int x = static_cast<int>(pRealCursorPos.x) + 18;
    int y = static_cast<int>(pRealCursorPos.y) + 18;
    if (x + width > viewport.x + viewport.width - 8)
        x = static_cast<int>(pRealCursorPos.x) - width - 12;
    if (y + height > viewport.y + viewport.height - 8)
        y = static_cast<int>(pRealCursorPos.y) - height - 12;
    x = std::max(viewport.x + 8, x);
    y = std::max(viewport.y + 8, y);

    const Rect box{x, y, width, height};
    r.DrawRect(box, UITheme::SurfaceRaised);
    DrawBorder(r, box, UITheme::Accent);
    r.DrawRect({box.x, box.y, box.width, 3}, UITheme::Accent);
    r.DrawTextUtf8(FontHandle{}, title, {box.x + 12, box.y + 9, box.width - 24, 18},
                   TextStyle{UITheme::Text, true, TextAlign::Left});
    for (std::size_t i = 0; i < details.size(); ++i)
    {
        r.DrawTextUtf8(FontHandle{}, details[i],
                       {box.x + 12, box.y + 30 + static_cast<int>(i) * 18, box.width - 24, 17},
                       TextStyle{UITheme::TextMuted, false, TextAlign::Left});
    }
}

const char* PremiumIconPath(int kind)
{
    switch (kind)
    {
        case nsPremiumItem::THIRD_EYES: return "image//Sinimage//Premium//ThirdEyes.tga";
        case nsPremiumItem::EXPUP_POTION: return "image//Sinimage//Premium//ExpUpPotion.tga";
        case nsPremiumItem::VAMPIRIC_CUS: return "image//Sinimage//Premium//Vampiric.tga";
        case nsPremiumItem::MANA_RECHAR_P: return "image//Sinimage//Premium//M_Rechar.tga";
        case nsPremiumItem::MIGHT_OF_AWELL: return "image//Sinimage//Premium//M_Might.tga";
        case nsPremiumItem::MANA_REDUCE_P: return "image//Sinimage//Premium//M_Reduce.tga";
        case nsPremiumItem::PHENIX_PET: return "image//Sinimage//Premium//M_Phoenix.TGA";
        case nsPremiumItem::HELP_PET_TERRY: return "image//Sinimage//Premium//Pet_terry-pose.tga";
        case nsPremiumItem::HELP_PET_NEPSYS: return "image//Sinimage//Premium//Pet_nepsys-pose.tga";
        case nsPremiumItem::HELP_PET_IO: return "image//Sinimage//Premium//Pet_io-pose.tga";
        case nsPremiumItem::HELP_PET_MUTE: return "image//Sinimage//Premium//Pet-mute-pose.tga";
        case nsPremiumItem::VAMPIRIC_CUS_EX: return "image//Sinimage//Premium//Vampiric_EX.tga";
        case nsPremiumItem::STAMINA_REDUCE_P: return "image//Sinimage//Premium//S_Reduce.tga";
        case nsPremiumItem::BIGHEAD1:
        case nsPremiumItem::BIGHEAD2:
        case nsPremiumItem::BIGHEAD3:
        case nsPremiumItem::BIGHEAD4: return "image//Sinimage//Premium//BigHead.tga";
        case nsPremiumItem::STAMINA_RECHAR_P: return "image//Sinimage//Premium//S_Rechar.tga";
        case nsPremiumItem::CARAVAN: return "image//Sinimage//Premium//Caravan.tga";
        case nsPremiumItem::GoldLooter: return "image//Sinimage//Premium//GoldLooter.tga";
        case nsPremiumItem::HP_BOOSTER: return "image//Sinimage//Premium//HPBooster.png";
        case nsPremiumItem::MP_BOOSTER: return "image//Sinimage//Premium//MPBooster.png";
        case nsPremiumItem::RES_BOOSTER: return "image//Sinimage//Premium//RESBooster.png";
        default: return nullptr;
    }
}

const char* PremiumTitle(int kind)
{
    switch (kind)
    {
        case nsPremiumItem::THIRD_EYES: return "Terceiro Olho";
        case nsPremiumItem::EXPUP_POTION: return "Bônus de EXP";
        case nsPremiumItem::VAMPIRIC_CUS: return "Presa de Vampiro";
        case nsPremiumItem::MANA_RECHAR_P: return "Recarga de Mana";
        case nsPremiumItem::MIGHT_OF_AWELL: return "Poder de Awell";
        case nsPremiumItem::MANA_REDUCE_P: return "Redução de Mana";
        case nsPremiumItem::PHENIX_PET: return "Fênix";
        case nsPremiumItem::HELP_PET_TERRY: return "Terry";
        case nsPremiumItem::HELP_PET_NEPSYS: return "Nepsys";
        case nsPremiumItem::HELP_PET_IO: return "Eor";
        case nsPremiumItem::HELP_PET_MUTE: return "Mut";
        case nsPremiumItem::VAMPIRIC_CUS_EX: return "Super Presa de Vampiro";
        case nsPremiumItem::STAMINA_REDUCE_P: return "Redução de Stamina";
        case nsPremiumItem::BIGHEAD1:
        case nsPremiumItem::BIGHEAD2:
        case nsPremiumItem::BIGHEAD3:
        case nsPremiumItem::BIGHEAD4: return "Big Head";
        case nsPremiumItem::STAMINA_RECHAR_P: return "Recarga de Stamina";
        case nsPremiumItem::CARAVAN: return "Caravana";
        case nsPremiumItem::GoldLooter: return "Coletor de Ouro";
        case nsPremiumItem::HP_BOOSTER: return "Booster de HP";
        case nsPremiumItem::MP_BOOSTER: return "Booster de MP";
        case nsPremiumItem::RES_BOOSTER: return "Booster de RES";
        default: return nullptr;
    }
}

std::string PremiumDescription(const sUpKeepItem& item)
{
    char text[160] = {0};
    switch (item.TGAImageNumber)
    {
        case nsPremiumItem::THIRD_EYES: std::snprintf(text, sizeof(text), "Drop +%d%%", item.Per); break;
        case nsPremiumItem::EXPUP_POTION: std::snprintf(text, sizeof(text), "EXP +%d%%", item.Per); break;
        case nsPremiumItem::VAMPIRIC_CUS: std::snprintf(text, sizeof(text), "Converte parte do dano em HP."); break;
        case nsPremiumItem::MANA_RECHAR_P: std::snprintf(text, sizeof(text), "Converte parte do dano em MP."); break;
        case nsPremiumItem::MIGHT_OF_AWELL: std::snprintf(text, sizeof(text), "Poder de Awell ativo."); break;
        case nsPremiumItem::MANA_REDUCE_P: std::snprintf(text, sizeof(text), "Uso de MP -%d%%", item.Per); break;
        case nsPremiumItem::PHENIX_PET: std::snprintf(text, sizeof(text), "Fênix ativa: ajuda em batalhas."); break;
        case nsPremiumItem::HELP_PET_TERRY: std::snprintf(text, sizeof(text), "Terry ativo: ajuda em batalhas."); break;
        case nsPremiumItem::HELP_PET_NEPSYS: std::snprintf(text, sizeof(text), "Nepsys ativo: ajuda em batalhas."); break;
        case nsPremiumItem::HELP_PET_IO: std::snprintf(text, sizeof(text), "Eor ativo: ajuda em batalhas."); break;
        case nsPremiumItem::HELP_PET_MUTE: std::snprintf(text, sizeof(text), "Mut ativo: ajuda em batalhas."); break;
        case nsPremiumItem::VAMPIRIC_CUS_EX: std::snprintf(text, sizeof(text), "Super Presa de Vampiro ativa."); break;
        case nsPremiumItem::STAMINA_REDUCE_P: std::snprintf(text, sizeof(text), "Uso de SP -%d%%", item.Per); break;
        case nsPremiumItem::BIGHEAD1:
        case nsPremiumItem::BIGHEAD2:
        case nsPremiumItem::BIGHEAD3:
        case nsPremiumItem::BIGHEAD4: std::snprintf(text, sizeof(text), "Poder de ataque +10%%"); break;
        case nsPremiumItem::STAMINA_RECHAR_P: std::snprintf(text, sizeof(text), "Regeneração de stamina ativa."); break;
        case nsPremiumItem::CARAVAN: std::snprintf(text, sizeof(text), "Caravana ativa."); break;
        case nsPremiumItem::GoldLooter: std::snprintf(text, sizeof(text), "Coleta automática de ouro ativa."); break;
        case nsPremiumItem::HP_BOOSTER: std::snprintf(text, sizeof(text), "HP +15%%"); break;
        case nsPremiumItem::MP_BOOSTER: std::snprintf(text, sizeof(text), "MP +15%%"); break;
        case nsPremiumItem::RES_BOOSTER: std::snprintf(text, sizeof(text), "RES +15%%"); break;
        default: std::snprintf(text, sizeof(text), "Efeito premium ativo."); break;
    }
    return text;
}

const char* SkillExpectedIconPath(DWORD code, char* buffer, std::size_t size)
{
    for (int i = 0; i < SIN_MAX_SKILL; ++i)
    {
        if (sSkill[i].CODE != code || !sSkill[i].IconName[0])
            continue;
        std::snprintf(buffer, size, "image//Sinimage//skill//keep//%s", sSkill[i].IconName);
        return buffer;
    }
    return nullptr;
}

void AppendMissingStatusLog(const char* type, unsigned long code, const char* name, const char* path, int material)
{
    CreateDirectoryA("LOGS", nullptr);
    FILE* file = std::fopen("LOGS/missing_status_icons.log", "at");
    if (!file)
        file = std::fopen("missing_status_icons.log", "at");
    if (!file)
        return;
    std::fprintf(file, "TYPE=%s CODE=0x%08lX MATERIAL=%d NAME=%s PATH=%s\n",
                 type ? type : "UNKNOWN", code, material,
                 name ? name : "", path ? path : "<unknown>");
    std::fclose(file);
}

bool SkillIconMissing(const sSKILL& skill)
{
    // Material state is authoritative. A valid texture can come from a cooked
    // runtime source and therefore have no loose-file path on disk.
    return !skill.MatIcon;
}

void LogMissingSkillOnce(const sSKILL& skill)
{
    static DWORD logged[64] = {0};
    for (DWORD code : logged)
        if (code == skill.CODE)
            return;
    for (DWORD& code : logged)
    {
        if (!code)
        {
            code = skill.CODE;
            break;
        }
    }
    char path[260] = {0};
    const char* expected = SkillExpectedIconPath(skill.CODE, path, sizeof(path));
    const char* name = skill.Skill_Info.SkillName[0] ? skill.Skill_Info.SkillName : skill.sSkillName;
    AppendMissingStatusLog("SKILL", static_cast<unsigned long>(skill.CODE), name, expected, skill.MatIcon);
}

bool PremiumIconMissing(const sUpKeepItem& item)
{
    const int index = item.TGAImageNumber - 1;
    if (index < 0 || index >= SINUPKEEPITEM_MAX)
        return true;
    return !chaPremiumitem.UpKeepItemTGAImage[index];
}

void LogMissingPremiumOnce(const sUpKeepItem& item)
{
    static bool logged[SINUPKEEPITEM_MAX + 1] = {false};
    const int kind = item.TGAImageNumber;
    if (kind >= 0 && kind <= SINUPKEEPITEM_MAX && logged[kind])
        return;
    if (kind >= 0 && kind <= SINUPKEEPITEM_MAX)
        logged[kind] = true;
    const int index = kind - 1;
    const int material = (index >= 0 && index < SINUPKEEPITEM_MAX) ? chaPremiumitem.UpKeepItemTGAImage[index] : 0;
    AppendMissingStatusLog("PREMIUM", static_cast<unsigned long>(kind), item.Id, PremiumIconPath(kind), material);
}

bool CursorInside(int x, int y, int width = 32, int height = 32)
{
    const int rx = static_cast<int>(pRealCursorPos.x);
    const int ry = static_cast<int>(pRealCursorPos.y);
    const int lx = static_cast<int>(pCursorPos.x);
    const int ly = static_cast<int>(pCursorPos.y);
    return (rx >= x && rx < x + width && ry >= y && ry < y + height) ||
           (lx >= x && lx < x + width && ly >= y && ly < y + height);
}

void RenderStatusEffectHover(UIRenderContext& context, const Rect& viewport)
{
    auto& r = context.Renderer();

    for (int i = 0; i < MAX_CONTINUE_SKILL; ++i)
    {
        const sSKILL& skill = TempContinueSkill[i];
        if (!skill.Flag)
            continue;
        const int x = 10 + skill.IconPosiX;
        const int y = 10;
        const bool missing = SkillIconMissing(skill);
        if (missing)
        {
            LogMissingSkillOnce(skill);
            r.DrawRect({x, y, 32, 32}, UITheme::SurfaceRaised);
            DrawBorder(r, {x, y, 32, 32}, UITheme::Danger);
            r.DrawTextUtf8(FontHandle{}, "?", {x, y + 6, 32, 20},
                           TextStyle{UITheme::Danger, true, TextAlign::Center});
        }
        if (!CursorInside(x, y))
            continue;

        std::string title = skill.CODE == PLAYERVIP ? "VIP" :
                            CleanLegacyText(skill.Skill_Info.SkillName[0] ? skill.Skill_Info.SkillName : skill.sSkillName);
        if (title.empty()) title = "Efeito ativo";
        std::vector<std::string> details;
        if (skill.CODE == PLAYERVIP)
        {
            details.emplace_back("EXP +15% | Drop +5%");
            details.emplace_back("Use /comandos para os atalhos VIP.");
        }
        else
        {
            const std::string doc = CleanLegacyText(skill.Skill_Info.SkillDoc);
            if (!doc.empty()) details.push_back(doc);
        }
        if (skill.UseTime > 0)
        {
            const long ticks = static_cast<long>(skill.UseTime) * 70L - static_cast<long>(skill.CheckTime);
            const unsigned long seconds = ticks > 0 ? static_cast<unsigned long>((ticks + 69) / 70) : 0;
            details.push_back("Tempo restante: " + FormatRemaining(seconds));
        }
        if (missing)
            details.emplace_back("Ícone não resolvido; consulte LOGS/missing_status_icons.log");
        DrawStatusTooltip(context, viewport, title, details);
        return;
    }

    for (int i = 0; i < SINUPKEEPITEM_MAX; ++i)
    {
        const sUpKeepItem& item = chaPremiumitem.UpKeepItem[i];
        if (!item.Visible)
            continue;
        const bool missing = PremiumIconMissing(item);
        if (missing)
        {
            LogMissingPremiumOnce(item);
            r.DrawRect({item.PosiX, item.PosiY, 32, 32}, UITheme::SurfaceRaised);
            DrawBorder(r, {item.PosiX, item.PosiY, 32, 32}, UITheme::Danger);
            r.DrawTextUtf8(FontHandle{}, "?", {item.PosiX, item.PosiY + 6, 32, 20},
                           TextStyle{UITheme::Danger, true, TextAlign::Center});
        }
        if (!CursorInside(item.PosiX, item.PosiY, 30, 30))
            continue;

        const char* semanticTitle = PremiumTitle(item.TGAImageNumber);
        std::string title = semanticTitle ? semanticTitle : CleanLegacyText(item.Id);
        if (title.empty()) title = "Premium";
        std::vector<std::string> details;
        details.push_back(PremiumDescription(item));
        if (item.IconTime[1] >= item.IconTime[0])
            details.push_back("Tempo restante: " + FormatRemaining(item.IconTime[1] - item.IconTime[0]));
        if (missing)
            details.emplace_back("Ícone não resolvido; consulte LOGS/missing_status_icons.log");
        DrawStatusTooltip(context, viewport, title, details);
        return;
    }
}

}

LegacyNotificationHud::LegacyNotificationHud(UIId id) : Window(id, {})
{
    SetBounds({0, 0, 800, 600});
    SetMovable(false);
    SetClosable(false);
    SetHitTestVisible(false);
}

void LegacyNotificationHud::SetViewport(int width, int height)
{
    SetBounds({0, 0, width, height});
}

int LegacyNotificationHud::FindActiveMessage()
{
    for (int i = 0; i < 100; ++i)
        if (sMessageBox[i].Flag)
            return i;
    return -1;
}

int LegacyNotificationHud::FindActiveMessage1()
{
    for (int i = 0; i < 100; ++i)
        if (sMessageBox1[i].Flag)
            return i;
    return -1;
}

bool LegacyNotificationHud::HasActiveLegacyNotification()
{
    return FindActiveMessage() >= 0 || FindActiveMessage1() >= 0 ||
           (cHelpPet.MsgFlag && cHelpPet.szPetBuff[0]) || cHelpPet.HelpMsgFlag;
}

void LegacyNotificationHud::OnRender(UIRenderContext& context)
{
    const Rect viewport = GlobalBounds();
    int y = viewport.y + 10;

    const int message = FindActiveMessage();
    if (message >= 0)
    {
        const char* raw = message == MESSAGE_100LVITEM_OK && sMessageBox[message].MessageDoc2[0]
                        ? sMessageBox[message].MessageDoc2
                        : sMessageBox[message].MessageDoc;
        y = DrawNotice(context, viewport, y, LegacyToUtf8(raw), false);
    }

    const int message1 = FindActiveMessage1();
    if (message1 >= 0)
        y = DrawNotice(context, viewport, y, LegacyToUtf8(sMessageBox1[message1].MessageDoc), true);

    // PATCH 183: pet producers remain legacy, presentation is V2-only.
    if (cHelpPet.MsgFlag && cHelpPet.szPetBuff[0])
        y = DrawNotice(context, viewport, y, LegacyToUtf8(cHelpPet.szPetBuff), false);

    if (cHelpPet.HelpMsgFlag)
    {
        const auto it = cHelpPet.PetMsg_Map.find(cHelpPet.HelpMsgId);
        if (it != cHelpPet.PetMsg_Map.end())
        {
            std::string tutorial;
            for (const std::string* line : it->second)
            {
                if (!line || line->empty()) continue;
                if (!tutorial.empty()) tutorial += " ";
                tutorial += *line;
            }
            DrawNotice(context, viewport, y, LegacyToUtf8(tutorial.c_str()), true);
        }
    }

    // PATCH 213: status-effect hover is a V2 surface. This covers continuous
    // skills (including VIP) and premium/Fenix timers while keeping legacy
    // producers and gameplay state authoritative.
    RenderStatusEffectHover(context, viewport);
}
}
