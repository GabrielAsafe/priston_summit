#include "systems/ui/v2/legacy/LegacySkillGateway.h"

#include <algorithm>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>
#include "compat/legacy/sin/sinLinkHeader.h"
#include "systems/ui/messages/legacy_sin/sinMsg.h"

extern char szSkillInfoBuff2[5000];

namespace ui::v2::legacy
{
namespace
{
constexpr int MaxSkillRank = 10;
// Legacy sinSkill_Info.h is a data-definition header rather than a safe interface
// header. Keep these stable USECODE values at the compatibility boundary instead
// of including that header into the UI V2 translation unit.
constexpr int SkillUseRight = 0x01000000;
constexpr int SkillUseLeft = 0x02000000;
constexpr int SkillUseAll = 0x03000000;
constexpr int MousePosLeft = 1;
constexpr int MousePosRight = 2;

bool ValidIndex(int index)
{
    return index >= 0 && index < SIN_MAX_USE_SKILL;
}

int PointsForPosition(int position)
{
    if (position <= 13)
        return sinSkill.SkillPoint;
    if (position <= SIN_TIER5_SKILL_START)
        return sinSkill.SkillPoint4;
    return sinSkill.SkillPoint5;
}

std::vector<std::string> SplitLegacyLines(const char* text)
{
    std::vector<std::string> lines;
    if (!text)
        return lines;
    const std::string value(text);
    std::size_t start = 0;
    while (start <= value.size())
    {
        const std::size_t end = value.find("/r", start);
        if (end == std::string::npos)
        {
            if (start < value.size())
                lines.emplace_back(value.substr(start));
            break;
        }
        lines.emplace_back(value.substr(start, end - start));
        start = end + 2;
        if (start == value.size())
            break;
    }
    return lines;
}

bool IsSkillDocLine(int oneBasedLine)
{
    for (int line : SkillDocLineCnt)
        if (line > 0 && line == oneBasedLine)
            return true;
    return false;
}
}

SkillBookSnapshot LegacySkillGateway::GetSnapshot() const
{
    SkillBookSnapshot out;
    if (!sinChar)
        return out;

    out.valid = true;
    out.tier1Points = sinSkill.SkillPoint;
    out.tier4Points = sinSkill.SkillPoint4;
    out.tier5Points = sinSkill.SkillPoint5;
    out.skills.reserve(SIN_MAX_USE_SKILL);

    for (int i = 0; i < SIN_MAX_USE_SKILL; ++i)
    {
        const sSKILL& skill = sinSkill.UseSkill[i];
        if (!skill.Flag && !skill.sSkillName[0] && !skill.Skill_Info.SkillName[0])
            continue;

        SkillEntrySnapshot entry;
        entry.index = i;
        entry.code = skill.CODE;
        if (skill.CODE == SKILL_NORMAL_ATTACK && NormalAttckName)
            entry.name = NormalAttckName;
        else
            entry.name = skill.Skill_Info.SkillName[0] ? skill.Skill_Info.SkillName : skill.sSkillName;
        entry.description = skill.Skill_Info.SkillDoc;
        entry.rank = skill.Point;
        entry.requiredLevel = skill.Skill_Info.RequireLevel;
        entry.shortcut = skill.ShortKey;
        entry.mouseBinding = skill.MousePosi;
        entry.visible = skill.Flag != 0;
        entry.unlocked = skill.Use != 0;
        entry.learned = skill.Point > 0;
        entry.leftSelected = sinSkill.pLeftSkill == &sinSkill.UseSkill[i];
        entry.rightSelected = sinSkill.pRightSkill == &sinSkill.UseSkill[i];
        entry.cooldownGauge = std::clamp(skill.GageLength, 0, 35);
        entry.cooldownReady = entry.cooldownGauge >= 35 || skill.UseSkillFlag != 0;

        const int position = i + 1;
        entry.canUpgrade = SkillMasterFlag == 1 && position > 1 && skill.Flag && skill.Use &&
                           skill.Point < MaxSkillRank && PointsForPosition(position) > 0;

        const int textureIndex = skill.Position > 0 ? skill.Position - 1 : i;
        if (textureIndex >= 0 && textureIndex < SIN_MAX_USE_SKILL)
        {
            // Mirror the old skill window: at the skill master, unavailable or
            // max-rank skills are gray; outside the master, unlearned skills are
            // gray. This keeps the V2 snapshot presentation-only and avoids
            // inventing a second unlock rule in SkillsPage.
            const bool preferNormal = SkillMasterFlag == 1
                ? (skill.Use != 0 && skill.Point < MaxSkillRank)
                : (skill.Point > 0);
            LPDIRECT3DTEXTURE9 texture = preferNormal ? cSkill.lpSkillButton[textureIndex]
                                                      : cSkill.lpSkillButton_Gray[textureIndex];
            if (!texture)
                texture = preferNormal ? cSkill.lpSkillButton_Gray[textureIndex]
                                       : cSkill.lpSkillButton[textureIndex];
            if (texture)
                entry.icon = TextureHandle(reinterpret_cast<std::uintptr_t>(texture));
        }

        out.skills.push_back(std::move(entry));
    }
    return out;
}

std::vector<DetailLineSnapshot> LegacySkillGateway::GetDetails(int skillIndex) const
{
    std::vector<DetailLineSnapshot> out;
    if (!ValidIndex(skillIndex))
        return out;

    sSKILL& skill = sinSkill.UseSkill[skillIndex];
    if (!skill.Flag && !skill.sSkillName[0] && !skill.Skill_Info.SkillName[0])
        return out;

    const int savedShowSkillUpInfo = ShowSkillUpInfo;
    ShowSkillUpInfo = 0;
    cSkill.ShowSkillInfo(&skill, &sinSkill.SkillBox[skillIndex]);
    ShowSkillUpInfo = savedShowSkillUpInfo;

    const auto left = SplitLegacyLines(szSkillInfoBuff);
    const auto right = SplitLegacyLines(szSkillInfoBuff2);
    const std::size_t count = std::max(left.size(), right.size());
    out.reserve(count);

    for (std::size_t i = 0; i < count; ++i)
    {
        DetailLineSnapshot line;
        if (i < left.size()) line.left = left[i];
        if (i < right.size()) line.right = right[i];
        const int oneBased = static_cast<int>(i) + 1;
        if (i == 0)
            line.tone = DetailLineTone::Header;
        else if (NextSkillLevelLine > 0 && oneBased == NextSkillLevelLine)
            line.tone = DetailLineTone::Positive;
        else if (IsSkillDocLine(oneBased))
            line.tone = DetailLineTone::Muted;
        else if (!line.right.empty())
            line.tone = DetailLineTone::Accent;
        else
            line.tone = DetailLineTone::Normal;
        if (!line.left.empty() || !line.right.empty())
            out.push_back(std::move(line));
    }
    return out;
}

bool LegacySkillGateway::RequestUpgrade(int skillIndex)
{
    if (!ValidIndex(skillIndex) || SkillMasterFlag != 1)
        return false;

    sSKILL& skill = sinSkill.UseSkill[skillIndex];
    const int position = skillIndex + 1;
    if (position <= 1 || !skill.Flag || !skill.Use || skill.Point >= MaxSkillRank || PointsForPosition(position) <= 0)
        return false;

    pMasterSkill = &skill;
    cMessageBox.CheckOkMessageBox2(MESSAGE_MASTER_SKILL);
    return true;
}

bool LegacySkillGateway::BindLeft(int skillIndex)
{
    if (!ValidIndex(skillIndex) || SkillMasterFlag != 0)
        return false;
    sSKILL& skill = sinSkill.UseSkill[skillIndex];
    if (!skill.Point || (skill.Skill_Info.USECODE != SkillUseLeft && skill.Skill_Info.USECODE != SkillUseAll))
        return false;
    sinSkill.pLeftSkill = &skill;
    return true;
}

bool LegacySkillGateway::BindRight(int skillIndex)
{
    if (!ValidIndex(skillIndex) || SkillMasterFlag != 0)
        return false;
    sSKILL& skill = sinSkill.UseSkill[skillIndex];
    if (!skill.Point || (skill.Skill_Info.USECODE != SkillUseRight && skill.Skill_Info.USECODE != SkillUseAll))
        return false;
    sinSkill.pRightSkill = &skill;
    return true;
}

bool LegacySkillGateway::AssignShortcut(int skillIndex, int shortcut, bool rightMouse)
{
    if (!ValidIndex(skillIndex) || shortcut < 1 || shortcut > 8 || SkillMasterFlag != 0)
        return false;

    sSKILL& skill = sinSkill.UseSkill[skillIndex];
    if (!skill.Point)
        return false;

    const int useCode = skill.Skill_Info.USECODE;
    if (rightMouse)
    {
        if (useCode != SkillUseRight && useCode != SkillUseAll)
            return false;
    }
    else if (useCode != SkillUseLeft && useCode != SkillUseAll)
    {
        return false;
    }

    skill.ShortKey = shortcut;
    skill.MousePosi = rightMouse ? MousePosRight : MousePosLeft;

    // Preserve the classic invariant: a function key belongs to only one
    // skill. The legacy UI used the same rule after assigning a shortcut.
    for (int i = 0; i < SIN_MAX_USE_SKILL; ++i)
    {
        if (i != skillIndex && sinSkill.UseSkill[i].ShortKey == shortcut)
            sinSkill.UseSkill[i].ShortKey = 0;
    }
    return true;
}
}
