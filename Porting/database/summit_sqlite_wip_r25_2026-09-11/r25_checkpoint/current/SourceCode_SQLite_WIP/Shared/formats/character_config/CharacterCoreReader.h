#pragma once

#include "formats/character_config/CharacterConfigCommon.h"
#include "formats/character_config/CharacterConfigServices.h"
#include "formats/character_config/CharacterConfigTokens.h"

namespace summit
{
namespace formats
{
namespace character_config
{

inline ParseResult HandleCharacterCore(
	const char *command,
	legacy_config::LineCursor &cursor,
	smCHAR_INFO &character,
	smCHAR_MONSTER_INFO *monster,
	char *dialogMessage,
	std::size_t dialogCapacity,
	const CharacterConfigServices &services)
{
	char value[256] = { 0 };

	if(IsToken(command, token::Name))
	{
		if(!cursor.NextValue(value, sizeof(value)) || !CopyText(character.szName, value))
			return ParseResult::Error;
		return ParseResult::Handled;
	}
#ifdef _LANGUAGE_BRAZIL
	if(lstrcmpi(command, "*J_NAME") == 0)
	{
		if(!cursor.NextValue(value, sizeof(value)))
			return ParseResult::Error;
		if(monster)
		{
			if(!CopyText(monster->szName, value)) return ParseResult::Error;
		}
		else if(!CopyText(character.szName, value))
			return ParseResult::Error;
		return ParseResult::Handled;
	}
#endif
	if(IsToken(command, token::ModelFile))
	{
		if(!cursor.NextValue(value, sizeof(value)) || !CopyText(character.szModelName, value))
			return ParseResult::Error;
		return ParseResult::Handled;
	}
	if(IsToken(command, token::Dialog))
	{
		if(!dialogMessage)
			return ParseResult::NotHandled;
		if(!cursor.NextValue(value, sizeof(value)) || !CopyText(dialogMessage, dialogCapacity, value))
			return ParseResult::Error;
		if(monster && monster->NpcMsgCount < NPC_MESSAGE_MAX && services.internNpcMessage)
			monster->lpNpcMessage[monster->NpcMsgCount++] = services.internNpcMessage(dialogMessage);
		return ParseResult::Handled;
	}
#ifdef _LANGUAGE_BRAZIL
	if(lstrcmpi(command, "*J_CHAT") == 0)
	{
		if(!dialogMessage)
			return ParseResult::NotHandled;
		if(!cursor.NextValue(value, sizeof(value)) || !CopyText(dialogMessage, dialogCapacity, value))
			return ParseResult::Error;
		if(monster && monster->NpcMsgCount < NPC_MESSAGE_MAX && services.internNpcMessage)
			monster->lpNpcMessage[monster->NpcMsgCount++] = services.internNpcMessage(dialogMessage);
		return ParseResult::Handled;
	}
#endif
	if(IsToken(command, token::State))
	{
		if(!cursor.NextValue(value, sizeof(value))) return ParseResult::Error;
		character.State = IsToken(value, token::Enemy) ? TRUE : FALSE;
		return ParseResult::Handled;
	}
	if(IsToken(command, token::Size))
	{
		if(!cursor.NextToken(value, sizeof(value))) return ParseResult::Error;
		character.SizeLevel = services.resolveSizeLevel ? services.resolveSizeLevel(value) : -1;
		return ParseResult::Handled;
	}
	if(IsToken(command, token::Sound) || IsToken(command, token::EffectSound))
	{
		if(!cursor.NextToken(value, sizeof(value))) return ParseResult::Error;
		if(services.resolveSoundCode)
		{
			DWORD code=0;
			if(services.resolveSoundCode(value,code)) character.dwCharSoundCode=code;
		}
		return ParseResult::Handled;
	}
	if(IsToken(command, token::Level)) { character.Level = ReadInt(cursor); return ParseResult::Handled; }
	if(IsToken(command, token::Boss)) { character.wPlayClass[0] = MONSTER_CLASS_BOSS; return ParseResult::Handled; }
	if(IsToken(command, token::Class)) { character.wPlayClass[0] = static_cast<WORD>(ReadInt(cursor)); return ParseResult::Handled; }
	if(IsToken(command, token::ModelScale))
	{
		character.wPlayClass[1] = static_cast<short>(ReadFloat(cursor) * fONE);
		if(character.wPlayClass[1] == fONE) character.wPlayClass[1] = 0;
		return ParseResult::Handled;
	}
	if(IsToken(command, token::MoveSpeed))
	{
		const float speed = ReadFloat(cursor);
		character.Move_Speed = services.convertMoveSpeed ? services.convertMoveSpeed(speed) : static_cast<int>(speed);
		return ParseResult::Handled;
	}
	if(IsToken(command, token::AttackDamage))
	{
		character.Attack_Damage[0] = ReadInt(cursor);
		character.Attack_Damage[1] = ReadInt(cursor);
		return ParseResult::Handled;
	}
	if(IsToken(command, token::AttackSpeed))
	{
		character.Attack_Speed = static_cast<int>(ReadFloat(cursor) * fONE);
		// The historical format accepts two values and the second one wins, even when empty.
		character.Attack_Speed = static_cast<int>(ReadFloat(cursor) * fONE);
		return ParseResult::Handled;
	}
	if(IsToken(command, token::AttackRange)) { character.Shooting_Range = ReadInt(cursor) * fONE; return ParseResult::Handled; }
	if(IsToken(command, token::AttackRating)) { character.Attack_Rating = ReadInt(cursor); return ParseResult::Handled; }
	if(IsToken(command, token::Defence)) { character.Defence = ReadInt(cursor); return ParseResult::Handled; }
	if(IsToken(command, token::Absorption)) { character.Absorption = ReadInt(cursor); return ParseResult::Handled; }
	if(IsToken(command, token::Block)) { character.Chance_Block = ReadInt(cursor); return ParseResult::Handled; }
	if(IsToken(command, token::Life) || IsToken(command, token::LifeAlt))
	{
		character.Life[0] = static_cast<short>(ReadInt(cursor));
		character.Life[1] = character.Life[0];
		return ParseResult::Handled;
	}
	if(IsToken(command, token::Bionic)) { character.Resistance[sITEMINFO_BIONIC] = static_cast<short>(ReadInt(cursor)); return ParseResult::Handled; }
	if(IsToken(command, token::Water)) { character.Resistance[sITEMINFO_WATER] = static_cast<short>(ReadInt(cursor)); return ParseResult::Handled; }
	if(IsToken(command, token::Lightning)) { character.Resistance[sITEMINFO_LIGHTING] = static_cast<short>(ReadInt(cursor)); return ParseResult::Handled; }
	if(IsToken(command, token::Ice)) { character.Resistance[sITEMINFO_ICE] = static_cast<short>(ReadInt(cursor)); return ParseResult::Handled; }
	if(IsToken(command, token::Wind)) { character.Resistance[sITEMINFO_WIND] = static_cast<short>(ReadInt(cursor)); return ParseResult::Handled; }
	if(IsToken(command, token::Earth)) { character.Resistance[sITEMINFO_EARTH] = static_cast<short>(ReadInt(cursor)); return ParseResult::Handled; }
	if(IsToken(command, token::Fire)) { character.Resistance[sITEMINFO_FIRE] = static_cast<short>(ReadInt(cursor)); return ParseResult::Handled; }
	if(IsToken(command, token::Poison)) { character.Resistance[sITEMINFO_POISON] = static_cast<short>(ReadFloat(cursor)); return ParseResult::Handled; }
	if(IsToken(command, token::Sight))
	{
		character.Sight = ReadInt(cursor);
		if(monster) monster->Real_Sight = character.Sight;
		character.Sight *= character.Sight;
		return ParseResult::Handled;
	}
	if(IsToken(command, token::ArrowCorrection))
	{
		if(!cursor.Empty()) character.ArrowPosi[0] = static_cast<short>(ReadInt(cursor));
		if(!cursor.Empty()) character.ArrowPosi[1] = static_cast<short>(ReadInt(cursor));
		return ParseResult::Handled;
	}

	return ParseResult::NotHandled;
}

} // namespace character_config
} // namespace formats
} // namespace summit
