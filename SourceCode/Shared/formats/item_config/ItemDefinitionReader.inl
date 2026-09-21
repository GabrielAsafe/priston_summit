#pragma once

#include <cstddef>
#include <cstdlib>
#include <cstring>

#include "formats/legacy_config/LegacyLineCursor.h"
#include "io/LegacyFile.h"
#include "io/LegacyPath.h"
#include "gameplay/classes/v2/services/ClassService.h"

namespace summit
{
namespace formats
{
namespace item_config
{

inline bool TokenIs(const char *token, const char *expected)
{
	return token && expected && std::strcmp(token, expected) == 0;
}

inline bool TokenIsEither(const char *token, const char *first, const char *second)
{
	return TokenIs(token, first) || TokenIs(token, second);
}

inline int ReadInt(legacy_config::LineCursor &cursor)
{
	char value[64] = { 0 };
	return cursor.NextToken(value, sizeof(value)) ? std::atoi(value) : 0;
}

inline float ReadFloat(legacy_config::LineCursor &cursor)
{
	char value[64] = { 0 };
	return cursor.NextToken(value, sizeof(value)) ? static_cast<float>(std::atof(value)) : 0.0f;
}

template <typename T>
inline void ReadIntegerPair(legacy_config::LineCursor &cursor, T (&values)[2])
{
	values[0] = static_cast<T>(ReadInt(cursor));
	values[1] = static_cast<T>(ReadInt(cursor));
}

template <typename T>
inline void ReadFloatPair(legacy_config::LineCursor &cursor, T (&values)[2])
{
	values[0] = static_cast<T>(ReadFloat(cursor));
	values[1] = static_cast<T>(ReadFloat(cursor));
}

template <std::size_t Capacity>
inline bool CopyText(char (&destination)[Capacity], const char *source)
{
	if(!source)
		return false;
	const std::size_t length = std::strlen(source);
	if(length >= Capacity)
		return false;
	std::memcpy(destination, source, length + 1);
	return true;
}

// CP949 tokens used by the historical OpenItem files. Keeping bytes explicit
// avoids source-encoding differences between the client and server projects.
namespace token
{
static const char Name[] = "\x2A\xC0\xCC\xB8\xA7";
static const char Code[] = "\x2A\xC4\xDA\xB5\xE5";
static const char Unique[] = "\x2A\xC0\xAF\xB4\xCF\xC5\xA9";
static const char UniqueColor[] = "\x2A\xC0\xAF\xB4\xCF\xC5\xA9\xBB\xF6\xBB\xF3";
static const char DisplayEffect[] = "\x2A\xC0\xCC\xC6\xE5\xC6\xAE\xBC\xB3\xC1\xA4";
static const char Durability[] = "\x2A\xB3\xBB\xB1\xB8\xB7\xC2";
static const char Weight[] = "\x2A\xB9\xAB\xB0\xD4";
static const char Price[] = "\x2A\xB0\xA1\xB0\xDD";
static const char Bionic[] = "\x2A\xBB\xFD\xC3\xBC";
static const char Earth[] = "\x2A\xB4\xEB\xC0\xDA\xBF\xAC";
static const char Fire[] = "\x2A\xBA\xD2";
static const char Ice[] = "\x2A\xB3\xC3\xB1\xE2";
static const char Lightning[] = "\x2A\xB9\xF8\xB0\xB3";
static const char Poison[] = "\x2A\xB5\xB6";
static const char Water[] = "\x2A\xB9\xB0";
static const char Wind[] = "\x2A\xB9\xD9\xB6\xF7";
static const char AttackDamage[] = "\x2A\xB0\xF8\xB0\xDD\xB7\xC2";
static const char ShootingRange[] = "\x2A\xBB\xE7\xC1\xA4\xB0\xC5\xB8\xAE";
static const char AttackSpeed[] = "\x2A\xB0\xF8\xB0\xDD\xBC\xD3\xB5\xB5";
static const char AttackRating[] = "\x2A\xB8\xED\xC1\xDF\xB7\xC2";
static const char Critical[] = "\x2A\xC5\xA9\xB8\xAE\xC6\xBC\xC4\xC3";
static const char Absorb[] = "\x2A\xC8\xED\xBC\xF6\xB7\xC2";
static const char Defence[] = "\x2A\xB9\xE6\xBE\xEE\xB7\xC2";
static const char Block[] = "\x2A\xBA\xED\xB7\xB0\xC0\xB2";
static const char MoveSpeed[] = "\x2A\xC0\xCC\xB5\xBF\xBC\xD3\xB5\xB5";
static const char PotionSpace[] = "\x2A\xBA\xB8\xC0\xAF\xB0\xF8\xB0\xA3";
static const char MagicMastery[] = "\x2A\xB8\xB6\xB9\xFD\xBC\xF7\xB7\xC3\xB5\xB5";
static const char ManaRegen[] = "\x2A\xB8\xB6\xB3\xAA\xC0\xE7\xBB\xFD";
static const char ManaRegenAlt[] = "\x2A\xB1\xE2\xB7\xC2\xC0\xE7\xBB\xFD";
static const char LifeRegen[] = "\x2A\xB6\xF3\xC0\xCC\xC7\xC1\xC0\xE7\xBB\xFD";
static const char LifeRegenAlt[] = "\x2A\xBB\xFD\xB8\xED\xB7\xC2\xC0\xE7\xBB\xFD";
static const char StaminaRegen[] = "\x2A\xBD\xBA\xC5\xD7\xB9\xCC\xB3\xAA\xC0\xE7\xBB\xFD";
static const char StaminaRegenAlt[] = "\x2A\xB1\xD9\xB7\xC2\xC0\xE7\xBB\xFD";
static const char ManaAdd[] = "\x2A\xB8\xB6\xB3\xAA\xC3\xDF\xB0\xA1";
static const char ManaAddAlt[] = "\x2A\xB1\xE2\xB7\xC2\xC3\xDF\xB0\xA1";
static const char LifeAdd[] = "\x2A\xB6\xF3\xC0\xCC\xC7\xC1\xC3\xDF\xB0\xA1";
static const char LifeAddAlt[] = "\x2A\xBB\xFD\xB8\xED\xB7\xC2\xC3\xDF\xB0\xA1";
static const char StaminaAdd[] = "\x2A\xBD\xBA\xC5\xD7\xB9\xCC\xB3\xAA\xC3\xDF\xB0\xA1";
static const char StaminaAddAlt[] = "\x2A\xB1\xD9\xB7\xC2\xC3\xDF\xB0\xA1";
static const char Level[] = "\x2A\xB7\xB9\xBA\xA7";
static const char Strength[] = "\x2A\xC8\xFB";
static const char Spirit[] = "\x2A\xC1\xA4\xBD\xC5\xB7\xC2";
static const char Talent[] = "\x2A\xC0\xE7\xB4\xC9";
static const char Dexterity[] = "\x2A\xB9\xCE\xC3\xB8\xBC\xBA";
static const char Health[] = "\x2A\xB0\xC7\xB0\xAD";
static const char Stamina[] = "\x2A\xBD\xBA\xC5\xD7\xB9\xCC\xB3\xCA\xBB\xF3\xBD\xC2";
static const char StaminaAlt[] = "\x2A\xB1\xD9\xB7\xC2\xBB\xF3\xBD\xC2";
static const char Mana[] = "\x2A\xB8\xB6\xB3\xAA\xBB\xF3\xBD\xC2";
static const char ManaAlt[] = "\x2A\xB1\xE2\xB7\xC2\xBB\xF3\xBD\xC2";
static const char Life[] = "\x2A\xB6\xF3\xC0\xCC\xC7\xC1\xBB\xF3\xBD\xC2";
static const char LifeAlt[] = "\x2A\xBB\xFD\xB8\xED\xB7\xC2\xBB\xF3\xBD\xC2";
static const char Job[] = "\x2A\x2A\xC6\xAF\xC8\xAD";
static const char JobRandom[] = "\x2A\x2A\xC6\xAF\xC8\xAD\xB7\xA3\xB4\xFD";
static const char SpecialAbsorb[] = "\x2A\x2A\xC8\xED\xBC\xF6\xB7\xC2";
static const char SpecialDefence[] = "\x2A\x2A\xB9\xE6\xBE\xEE\xB7\xC2";
static const char SpecialMoveSpeed[] = "\x2A\x2A\xC0\xCC\xB5\xBF\xBC\xD3\xB5\xB5";
static const char SpecialBlock[] = "\x2A\x2A\xBA\xED\xB7\xB0\xC0\xB2";
static const char SpecialAttackSpeed[] = "\x2A\x2A\xB0\xF8\xB0\xDD\xBC\xD3\xB5\xB5";
static const char SpecialCritical[] = "\x2A\x2A\xC5\xA9\xB8\xAE\xC6\xBC\xC4\xC3";
static const char SpecialRange[] = "\x2A\x2A\xBB\xE7\xC1\xA4\xB0\xC5\xB8\xAE";
static const char SpecialMagic[] = "\x2A\x2A\xB8\xB6\xB9\xFD\xBC\xF7\xB7\xC3\xB5\xB5";
static const char SpecialManaAdd[] = "\x2A\x2A\xB8\xB6\xB3\xAA\xC3\xDF\xB0\xA1";
static const char SpecialManaAddAlt[] = "\x2A\x2A\xB1\xE2\xB7\xC2\xC3\xDF\xB0\xA1";
static const char SpecialLifeAdd[] = "\x2A\x2A\xB6\xF3\xC0\xCC\xC7\xC1\xC3\xDF\xB0\xA1";
static const char SpecialLifeAddAlt[] = "\x2A\x2A\xBB\xFD\xB8\xED\xB7\xC2\xC3\xDF\xB0\xA1";
static const char SpecialAttackRating[] = "\x2A\x2A\xB8\xED\xC1\xDF\xB7\xC2";
static const char SpecialDamage[] = "\x2A\x2A\xB0\xF8\xB0\xDD\xB7\xC2";
static const char SpecialManaRegen[] = "\x2A\x2A\xB8\xB6\xB3\xAA\xC0\xE7\xBB\xFD";
static const char SpecialManaRegenAlt[] = "\x2A\x2A\xB1\xE2\xB7\xC2\xC0\xE7\xBB\xFD";
static const char SpecialLifeRegen[] = "\x2A\x2A\xB6\xF3\xC0\xCC\xC7\xC1\xC0\xE7\xBB\xFD";
static const char SpecialLifeRegenAlt[] = "\x2A\x2A\xBB\xFD\xB8\xED\xB7\xC2\xC0\xE7\xBB\xFD";
static const char SpecialStaminaRegen[] = "\x2A\x2A\xBD\xBA\xC5\xD7\xB9\xCC\xB3\xAA\xC0\xE7\xBB\xFD";
static const char SpecialStaminaRegenAlt[] = "\x2A\x2A\xB1\xD9\xB7\xC2\xC0\xE7\xBB\xFD";
static const char GenerationLimit[] = "\x2A\xB9\xDF\xBB\xFD\xC1\xA6\xC7\xD1";
static const char LinkedFile[] = "\x2A\xBF\xAC\xB0\xE1\xC6\xC4\xC0\xCF";
}

inline bool HandleIdentityAndVisual(const char *command, legacy_config::LineCursor &cursor, sDEF_ITEMINFO &definition)
{
	sITEMINFO &item = definition.Item;
	char value[128] = { 0 };
	if(TokenIs(command, token::Name))
	{
		if(cursor.NextValue(value, sizeof(value))) CopyText(item.ItemName, value);
		return true;
	}
#ifdef _LANGUAGE_BRAZIL
	if(lstrcmpi(command, "*J_NAME") == 0)
	{
		if(cursor.NextValue(value, sizeof(value))) CopyText(item.ItemName, value);
		return true;
	}
#endif
	if(TokenIs(command, token::Code))
	{
		if(cursor.NextValue(value, sizeof(value)))
			for(int index = 0; index < MAX_ITEM; ++index)
				if(lstrcmpi(value, sItem[index].LastCategory) == 0) { item.CODE = sItem[index].CODE; break; }
		return true;
	}
	if(TokenIs(command, token::Unique))
	{
		item.UniqueItem = cursor.Empty() ? TRUE : ReadInt(cursor);
		return true;
	}
	if(TokenIs(command, token::UniqueColor))
	{
		for(int index = 0; index < 4; ++index) item.EffectColor[index] = ReadInt(cursor);
		item.EffectBlink[0] = ReadInt(cursor);
		if(!cursor.Empty()) item.ScaleBlink[0] = static_cast<int>(ReadFloat(cursor) * fONE);
		return true;
	}
	if(TokenIs(command, token::DisplayEffect)) { item.DispEffect = ReadInt(cursor); return true; }
	if(TokenIs(command, token::Durability)) { ReadIntegerPair(cursor, definition.sDurability); return true; }
	if(TokenIs(command, token::Weight)) { item.Weight = ReadInt(cursor); return true; }
	if(TokenIs(command, token::Price)) { item.Price = ReadInt(cursor); return true; }
	return false;
}

inline bool HandleCombatStats(const char *command, legacy_config::LineCursor &cursor, sDEF_ITEMINFO &definition)
{
	sITEMINFO &item = definition.Item;
	if(TokenIs(command, token::Bionic)) { ReadIntegerPair(cursor, definition.sResistance[sITEMINFO_BIONIC]); return true; }
	if(TokenIs(command, token::Earth)) { ReadIntegerPair(cursor, definition.sResistance[sITEMINFO_EARTH]); return true; }
	if(TokenIs(command, token::Fire)) { ReadIntegerPair(cursor, definition.sResistance[sITEMINFO_FIRE]); return true; }
	if(TokenIs(command, token::Ice)) { ReadIntegerPair(cursor, definition.sResistance[sITEMINFO_ICE]); return true; }
	if(TokenIs(command, token::Lightning)) { ReadIntegerPair(cursor, definition.sResistance[sITEMINFO_LIGHTING]); return true; }
	if(TokenIs(command, token::Poison)) { ReadIntegerPair(cursor, definition.sResistance[sITEMINFO_POISON]); return true; }
	if(TokenIs(command, token::Water)) { ReadIntegerPair(cursor, definition.sResistance[sITEMINFO_WATER]); return true; }
	if(TokenIs(command, token::Wind)) { ReadIntegerPair(cursor, definition.sResistance[sITEMINFO_WIND]); return true; }
	if(TokenIs(command, token::AttackDamage)) { for(int index=0; index<4; ++index) definition.sDamage[index]=static_cast<short>(ReadInt(cursor)); return true; }
	if(TokenIs(command, token::ShootingRange)) { item.Shooting_Range=ReadInt(cursor); return true; }
	if(TokenIs(command, token::AttackSpeed)) { item.Attack_Speed=ReadInt(cursor); return true; }
	if(TokenIs(command, token::AttackRating)) { ReadIntegerPair(cursor, definition.sAttack_Rating); return true; }
	if(TokenIs(command, token::Critical)) { item.Critical_Hit=ReadInt(cursor); return true; }
	if(TokenIs(command, token::Absorb)) { ReadFloatPair(cursor, definition.fAbsorb); return true; }
	if(TokenIs(command, token::Defence)) { ReadIntegerPair(cursor, definition.sDefence); return true; }
	if(TokenIs(command, token::Block)) { ReadFloatPair(cursor, definition.fBlock_Rating); return true; }
	if(TokenIs(command, token::MoveSpeed)) { ReadFloatPair(cursor, definition.fSpeed); return true; }
	if(TokenIs(command, token::PotionSpace)) { item.Potion_Space=ReadInt(cursor); return true; }
	if(TokenIs(command, token::MagicMastery)) { item.fMagic_Mastery=ReadFloat(cursor); return true; }
	return false;
}

inline bool HandleResourcesAndRequirements(const char *command, legacy_config::LineCursor &cursor, sDEF_ITEMINFO &definition)
{
	sITEMINFO &item = definition.Item;
	if(TokenIsEither(command, token::ManaRegen, token::ManaRegenAlt)) { ReadFloatPair(cursor, definition.fMana_Regen); return true; }
	if(TokenIsEither(command, token::LifeRegen, token::LifeRegenAlt)) { ReadFloatPair(cursor, definition.fLife_Regen); return true; }
	if(TokenIsEither(command, token::StaminaRegen, token::StaminaRegenAlt)) { ReadFloatPair(cursor, definition.fStamina_Regen); return true; }
	if(TokenIsEither(command, token::ManaAdd, token::ManaAddAlt)) { ReadIntegerPair(cursor, definition.Increase_Mana); return true; }
	if(TokenIsEither(command, token::LifeAdd, token::LifeAddAlt)) { ReadIntegerPair(cursor, definition.Increase_Life); return true; }
	if(TokenIsEither(command, token::StaminaAdd, token::StaminaAddAlt)) { ReadIntegerPair(cursor, definition.Increase_Stamina); return true; }
	if(TokenIs(command, token::Level)) { item.Level=ReadInt(cursor); return true; }
	if(TokenIs(command, token::Strength)) { item.Strength=ReadInt(cursor); return true; }
	if(TokenIs(command, token::Spirit)) { item.Spirit=ReadInt(cursor); return true; }
	if(TokenIs(command, token::Talent)) { item.Talent=ReadInt(cursor); return true; }
	if(TokenIs(command, token::Dexterity)) { item.Dexterity=ReadInt(cursor); return true; }
	if(TokenIs(command, token::Health)) { item.Health=ReadInt(cursor); return true; }
	if(TokenIsEither(command, token::Stamina, token::StaminaAlt)) { ReadIntegerPair(cursor, item.Stamina); return true; }
	if(TokenIsEither(command, token::Mana, token::ManaAlt)) { ReadIntegerPair(cursor, item.Mana); return true; }
	if(TokenIsEither(command, token::Life, token::LifeAlt)) { ReadIntegerPair(cursor, item.Life); return true; }
	return false;
}

inline bool ResolveJobBitCode(const char *value, DWORD &out)
{
	if(!value || !*value)
		return false;

	std::uint32_t resolved = 0u;
	if(!gameplay::classes::DefaultClassService().TryJobBitCodeByContentToken(value, resolved))
		return false;

	out = static_cast<DWORD>(resolved);
	return true;
}

inline bool ApplyJobList(legacy_config::LineCursor &cursor, sITEMINFO &item)
{
	char value[64];
	while(cursor.NextToken(value, sizeof(value)))
	{
		DWORD jobBitCode = 0;
		if(ResolveJobBitCode(value, jobBitCode))
			item.JobCodeMask = jobBitCode;
	}
	return true;
}

inline bool ApplyRandomJobList(legacy_config::LineCursor &cursor, sDEF_ITEMINFO &definition)
{
	char value[64];
	while(cursor.NextToken(value, sizeof(value)))
	{
		DWORD jobBitCode = 0;
		if(ResolveJobBitCode(value, jobBitCode) &&
		   definition.JobBitCodeRandomCount < SPECIAL_JOB_RANDOM_MAX)
		{
			definition.dwJobBitCode_Random[definition.JobBitCodeRandomCount++] = jobBitCode;
		}
	}
	return true;
}

inline bool HandleJobSpecialization(const char *command, legacy_config::LineCursor &cursor, sDEF_ITEMINFO &definition)
{
	sITEMINFO &item=definition.Item;
	if(TokenIs(command,token::Job)) return ApplyJobList(cursor,item);
	if(TokenIs(command,token::JobRandom)) return ApplyRandomJobList(cursor,definition);
	if(TokenIs(command,token::SpecialAbsorb)) { ReadFloatPair(cursor,definition.fSpecial_Absorb); return true; }
	if(TokenIs(command,token::SpecialDefence)) { ReadIntegerPair(cursor,definition.sSpecial_Defence); return true; }
	if(TokenIs(command,token::SpecialMoveSpeed)) { ReadFloatPair(cursor,definition.fSpecial_fSpeed); return true; }
	if(TokenIs(command,token::SpecialBlock)) { item.JobItem.Add_fBlock_Rating=ReadFloat(cursor); return true; }
	if(TokenIs(command,token::SpecialAttackSpeed)) { item.JobItem.Add_Attack_Speed=ReadInt(cursor); return true; }
	if(TokenIs(command,token::SpecialCritical)) { item.JobItem.Add_Critical_Hit=ReadInt(cursor); return true; }
	if(TokenIs(command,token::SpecialRange)) { item.JobItem.Add_Shooting_Range=ReadInt(cursor); return true; }
	if(TokenIs(command,token::SpecialMagic)) { ReadFloatPair(cursor,definition.fSpecial_Magic_Mastery); return true; }
	if(TokenIsEither(command,token::SpecialManaAdd,token::SpecialManaAddAlt)) { item.JobItem.Lev_Mana=ReadInt(cursor); return true; }
	if(TokenIsEither(command,token::SpecialLifeAdd,token::SpecialLifeAddAlt)) { item.JobItem.Lev_Life=ReadInt(cursor); return true; }
	if(TokenIs(command,token::SpecialAttackRating)) { ReadIntegerPair(cursor,definition.Lev_Attack_Rating); return true; }
	if(TokenIs(command,token::SpecialDamage)) { ReadIntegerPair(cursor,item.JobItem.Lev_Damage); return true; }
	if(TokenIsEither(command,token::SpecialManaRegen,token::SpecialManaRegenAlt)) { ReadFloatPair(cursor,definition.fSpecial_Mana_Regen); return true; }
	if(TokenIsEither(command,token::SpecialLifeRegen,token::SpecialLifeRegenAlt)) { item.JobItem.Per_Life_Regen=ReadFloat(cursor); return true; }
	if(TokenIsEither(command,token::SpecialStaminaRegen,token::SpecialStaminaRegenAlt)) { item.JobItem.Per_Stamina_Regen=ReadFloat(cursor); return true; }
	return false;
}

inline bool ReadItemDefinition(const char *sourceFile, sDEF_ITEMINFO &output)
{
	if(!sourceFile || !*sourceFile)
		return false;

	sDEF_ITEMINFO staged = sDEF_ITEMINFO();
	summit::io::LegacyFile file(sourceFile,"rb");
	if(!file)
		return false;

	char lineBuffer[512];
	char command[128];
	while(file.ReadLine(lineBuffer,sizeof(lineBuffer)))
	{
		legacy_config::LineCursor line(lineBuffer);
		if(!line.NextToken(command,sizeof(command)))
			continue;
		legacy_config::LineCursor values(line.Position());
		if(HandleIdentityAndVisual(command,values,staged)) continue;
		if(HandleCombatStats(command,values,staged)) continue;
		if(HandleResourcesAndRequirements(command,values,staged)) continue;
		if(HandleJobSpecialization(command,values,staged)) continue;
		if(TokenIs(command,token::GenerationLimit)) { staged.sGenDay[0]=ReadInt(values); continue; }
		if(TokenIs(command,token::LinkedFile))
		{
			char relative[128];
			char nextFile[260];
			if(values.NextValue(relative,sizeof(relative)) && summit::io::BuildPathFromFileDirectory(
				sourceFile,relative,nextFile,sizeof(nextFile)))
				file.Open(nextFile,"rb");
		}
	}

	if(staged.Item.EffectBlink[0])
		staged.Item.EffectBlink[1]=staged.Item.EffectColor[0]+staged.Item.EffectColor[1]+
			staged.Item.EffectColor[2]+staged.Item.EffectColor[3]+static_cast<short>(staged.Item.DispEffect);
	output=staged;
	return true;
}

} // namespace item_config
} // namespace formats
} // namespace summit
