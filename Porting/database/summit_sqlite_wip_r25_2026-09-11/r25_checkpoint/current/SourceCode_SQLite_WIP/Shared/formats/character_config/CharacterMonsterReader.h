#pragma once

#include <algorithm>

#include "formats/character_config/CharacterConfigCommon.h"
#include "formats/character_config/CharacterConfigServices.h"
#include "formats/character_config/CharacterConfigTokens.h"

namespace summit
{
namespace formats
{
namespace character_config
{

inline bool AppendDrop(smCHAR_MONSTER_INFO &monster, DWORD code, int percentage, short minPrice = 0, short maxPrice = 0)
{
	if(monster.FallItemCount < 0 || monster.FallItemCount >= FALLITEM_MAX)
		return false;
	_FALLITEM &item = monster.FallItems[monster.FallItemCount++];
	item.dwItemCode = code;
	item.Percentage = percentage;
	item.sPrice[0] = minPrice;
	item.sPrice[1] = maxPrice;
	return true;
}

inline bool ResolveCodes(
	legacy_config::LineCursor &cursor,
	const CharacterConfigServices &services,
	const char *first,
	DWORD *codes,
	int capacity,
	int &count)
{
	count = 0;
	char value[128] = { 0 };
	if(first && *first)
		CopyText(value, first);

	for(;;)
	{
		if(value[0])
		{
			DWORD code = 0;
			if(services.resolveItemCode && services.resolveItemCode(value, code))
			{
				if(count >= capacity) return false;
				codes[count++] = code;
			}
		}
		value[0] = 0;
		if(!cursor.NextToken(value, sizeof(value))) break;
	}
	return true;
}

inline ParseResult HandleMonsterDrops(
	const char *command,
	legacy_config::LineCursor &cursor,
	smCHAR_MONSTER_INFO &monster,
	const CharacterConfigServices &services)
{
	char value[128] = { 0 };
	if(IsToken(command, token::AllSeeItem)) { monster.AllSeeItem = TRUE; return ParseResult::Handled; }
	if(IsToken(command, token::FallItemMax)) { monster.FallItemMax = ReadInt(cursor); return ParseResult::Handled; }
	if(IsToken(command, token::ExtraFallItem))
	{
		const int percentage = ReadInt(cursor);
		if(!cursor.NextToken(value, sizeof(value))) return ParseResult::Handled;
		if(monster.FallItemPlusCount < 0 || monster.FallItemPlusCount >= FALLITEM2_MAX)
			return ParseResult::Error;
		DWORD code = 0;
		if(services.resolveItemCode && services.resolveItemCode(value, code))
		{
			_FALLITEM &item = monster.FallItems_Plus[monster.FallItemPlusCount++];
			item.dwItemCode = code;
			item.Percentage = percentage;
			item.sPrice[0] = 0;
			item.sPrice[1] = 0;
		}
		return ParseResult::Handled;
	}
	if(IsToken(command, token::FallItem))
	{
		int percentage = ReadInt(cursor);
		monster.FallItemPerMax += percentage;
		if(!cursor.NextToken(value, sizeof(value))) return ParseResult::Handled;

		if(IsToken(value, token::NoItem))
		{
			if(services.testServerMode)
			{
				monster.FallItemPerMax -= percentage;
				percentage /= 2;
				monster.FallItemPerMax += percentage;
			}
			return AppendDrop(monster, 0, percentage) ? ParseResult::Handled : ParseResult::Error;
		}
		if(IsToken(value, token::Money))
		{
			const short minimum = static_cast<short>(ReadInt(cursor));
			const short maximum = static_cast<short>(ReadInt(cursor));
			return AppendDrop(monster, sinGG1 | sin01, percentage, minimum, maximum) ? ParseResult::Handled : ParseResult::Error;
		}

		DWORD codes[32];
		int count = 0;
		if(!ResolveCodes(cursor, services, value, codes, 32, count))
			return ParseResult::Error;
		if(count == 0)
			return ParseResult::Handled;
		if(monster.FallItemCount < 0 || count > FALLITEM_MAX - monster.FallItemCount)
			return ParseResult::Error;
		percentage /= count;
		for(int index = 0; index < count; ++index)
			if(!AppendDrop(monster, codes[index], percentage)) return ParseResult::Error;
		return ParseResult::Handled;
	}
	return ParseResult::NotHandled;
}

inline ParseResult HandleMonsterBehavior(
	const char *command,
	legacy_config::LineCursor &cursor,
	smCHAR_INFO &character,
	smCHAR_MONSTER_INFO &monster,
	const CharacterConfigServices &services)
{
	char value[256] = { 0 };
	if(IsToken(command, token::EventModel))
	{
		if(!cursor.NextValue(value, sizeof(value)) || !CopyText(character.szModelName2 + 1, sizeof(character.szModelName2) - 1, value))
			return ParseResult::Error;
		monster.UseEventModel = TRUE;
		return ParseResult::Handled;
	}
	if(IsToken(command, token::SkillDamage)) { monster.SkillDamage[0]=static_cast<short>(ReadInt(cursor)); monster.SkillDamage[1]=static_cast<short>(ReadInt(cursor)); return ParseResult::Handled; }
	if(IsToken(command, token::SkillDistance)) { monster.SkillDistance=ReadInt(cursor); return ParseResult::Handled; }
	if(IsToken(command, token::SkillRange)) { monster.SkillRange=ReadInt(cursor); return ParseResult::Handled; }
	if(IsToken(command, token::SkillRating)) { monster.SkillRating=ReadInt(cursor); return ParseResult::Handled; }
	if(IsToken(command, token::SkillCurse)) { monster.SkillCurse=ReadInt(cursor); return ParseResult::Handled; }
	if(IsToken(command, token::MoveType)) { ReadInt(cursor); return ParseResult::Handled; }
	if(IsToken(command, token::MoveRange)) { monster.MoveRange=static_cast<int>(ReadFloat(cursor)*fONE); return ParseResult::Handled; }
	if(IsToken(command, token::ActiveHour))
	{
		if(!cursor.NextToken(value,sizeof(value))) return ParseResult::Error;
		monster.ActiveHour=0;
		if(lstrcmpi(value,token::Day)==0) monster.ActiveHour=1;
		if(lstrcmpi(value,token::Night)==0) monster.ActiveHour=-1;
		return ParseResult::Handled;
	}
	if(IsToken(command, token::GenerateGroup)) { monster.GenerateGroup[0]=ReadInt(cursor); monster.GenerateGroup[1]=ReadInt(cursor); return ParseResult::Handled; }
	if(IsToken(command, token::IQ)) { monster.IQ=ReadInt(cursor); return ParseResult::Handled; }
	if(IsToken(command, token::ClassCode)) { monster.ClassCode=ReadInt(cursor); return ParseResult::Handled; }
	if(IsToken(command, token::StunPercent) || IsToken(command, token::StunRate)) { monster.DamageStunPers=ReadInt(cursor); return ParseResult::Handled; }
	if(IsToken(command, token::Nature))
	{
		monster.Nature=smCHAR_MONSTER_NATURAL;
		if(cursor.NextToken(value,sizeof(value)))
		{
			if(lstrcmpi(value,"good")==0) monster.Nature=smCHAR_MONSTER_GOOD;
			if(lstrcmpi(value,"evil")==0) monster.Nature=smCHAR_MONSTER_EVIL;
		}
		return ParseResult::Handled;
	}
	if(IsToken(command, token::EventCode)) { monster.EventCode=ReadInt(cursor); return ParseResult::Handled; }
	if(IsToken(command, token::EventInfo)) { monster.EventInfo=ReadInt(cursor); return ParseResult::Handled; }
	if(IsToken(command, token::EventItem))
	{
		if(cursor.NextToken(value,sizeof(value)) && services.resolveItemCode)
		{
			DWORD code=0; if(services.resolveItemCode(value,code)) monster.dwEvnetItem=code;
		}
		return ParseResult::Handled;
	}
	if(IsToken(command, token::SpecialAttackRate))
	{
		const int percentage=ReadInt(cursor);
		monster.SpAttackPercetage=services.convertPercent ? services.convertPercent(percentage) : percentage;
		return ParseResult::Handled;
	}
	if(IsToken(command, token::Undead))
	{
		cursor.NextToken(value,sizeof(value));
		if(IsToken(value,token::Present) || IsToken(value,token::PresentAlt))
		{
			monster.Undead=TRUE; character.Brood=smCHAR_MONSTER_UNDEAD;
		}
		else
		{
			monster.Undead=FALSE; character.Brood=smCHAR_MONSTER_NORMAL;
		}
		return ParseResult::Handled;
	}
	if(IsToken(command, token::MonsterRace))
	{
		if(cursor.NextToken(value,sizeof(value)) && value[0])
		{
			character.Brood=smCHAR_MONSTER_NORMAL;
			if(IsToken(value,token::RaceUndead)) character.Brood=smCHAR_MONSTER_UNDEAD;
			else if(IsToken(value,token::RaceMutant)) character.Brood=smCHAR_MONSTER_MUTANT;
			else if(IsToken(value,token::RaceDemon)) character.Brood=smCHAR_MONSTER_DEMON;
			else if(IsToken(value,token::RaceMechanic)) character.Brood=smCHAR_MONSTER_MECHANIC;
		}
		return ParseResult::Handled;
	}
	if(IsToken(command, token::Experience)) { monster.GetExp=ReadInt(cursor); character.Exp=monster.GetExp; return ParseResult::Handled; }
	if(IsToken(command, token::PotionCount)) { monster.PotionCount=ReadInt(cursor); return ParseResult::Handled; }
	if(IsToken(command, token::PotionPercent)) { monster.PotionPercent=ReadInt(cursor); return ParseResult::Handled; }
	return HandleMonsterDrops(command,cursor,monster,services);
}

} // namespace character_config
} // namespace formats
} // namespace summit
