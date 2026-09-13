#pragma once

#include <climits>
#include <cstddef>
#include <cstring>

#include "formats/legacy_config/LegacyLineCursor.h"
#include "io/LegacyFile.h"

namespace summit
{
namespace formats
{
namespace stage_spawn
{

namespace token
{
static const char Actor[]="\x2A\xC3\xE2\xBF\xAC\xC0\xDA";
static const char BossActor[]="\x2A\xC3\xE2\xBF\xAC\xC0\xDA\xB5\xCE\xB8\xF1";
static const char MaxActorPosition[]="\x2A\xC3\xD6\xB4\xEB\xB5\xBF\xBD\xC3\xC3\xE2\xC7\xF6\xBC\xF6";
static const char Delay[]="\x2A\xC3\xE2\xC7\xF6\xB0\xA3\xB0\xDD";
static const char MaxActor[]="\x2A\xC3\xE2\xC7\xF6\xBC\xF6";
}

inline bool TokenEquals(const char *command,const char *cp949,const char *ascii)
{
	return command && ((cp949 && std::strcmp(command,cp949)==0) || (ascii && lstrcmpi(command,ascii)==0));
}

inline smCHAR_INFO *FindCharacter(smCHAR_INFO *characters,int count,const char *name)
{
	if(!characters || !name || count <= 0) return 0;
	for(int index=0; index<count; ++index)
		if(std::strcmp(characters[index].szName,name)==0) return &characters[index];
	return 0;
}

template <std::size_t N>
inline bool CopyName(char (&destination)[N],const char *source)
{
	if(!source) return false;
	const std::size_t length=std::strlen(source);
	if(length >= N) return false;
	std::memcpy(destination,source,length+1);
	return true;
}

inline bool ReadActor(
	legacy_config::LineCursor &cursor,
	rsSTG_MONSTER_LIST &list,
	smCHAR_INFO *characters,
	int characterCount,
	int &monsterCount,
	int &percentageTotal)
{
	if(monsterCount >= rsSTG_MONSTER_MAX) return true;
	char name[64];
	if(!cursor.NextValue(name,sizeof(name))) return false;
	rsSTG_MONSTER &entry=list.rsMonster[monsterCount];
	// Preserve the legacy observable slot contents even when the actor name is unknown.
	if(!CopyName(entry.szMonsterName,name)) return false;
	smCHAR_INFO *character=FindCharacter(characters,characterCount,name);
	if(!character) return true;
	int percentage=0;
	if(!cursor.NextInt(&percentage)) percentage=0;
	entry.lpCharInfo=character;
	entry.NumOpenStart=percentageTotal;
	entry.OpenPercentage=percentage;
	percentageTotal+=percentage;
	++monsterCount;
	return true;
}

inline bool ReadBossActor(
	legacy_config::LineCursor &cursor,
	rsSTG_MONSTER_LIST &list,
	smCHAR_INFO *characters,
	int characterCount)
{
	if(list.BossMonsterCount >= rsBOSS_MONSTER_MAX) return true;
	sBOSS_MONSTER &boss=list.sBossMonsters[list.BossMonsterCount];
	char name[64];
	if(!cursor.NextValue(name,sizeof(name))) return false;
	boss.MasterMonster=FindCharacter(characters,characterCount,name);
	if(!cursor.NextValue(name,sizeof(name))) return false;
	boss.SlaveMonster=FindCharacter(characters,characterCount,name);
	boss.SlaveCount=0;
	cursor.NextInt(&boss.SlaveCount);
	while(!cursor.Empty())
	{
		int hour=0;
		if(!cursor.NextInt(&hour)) break;
		if(boss.OpenTimeCount >= static_cast<int>(sizeof(boss.bOpenTime)/sizeof(boss.bOpenTime[0])))
			return false;
		boss.bOpenTime[boss.OpenTimeCount++]=static_cast<BYTE>(hour);
	}
	++list.BossMonsterCount;
	return true;
}

inline bool ReadDelay(legacy_config::LineCursor &cursor,rsSTG_MONSTER_LIST &list)
{
	int shift=0;
	if(!cursor.NextInt(&shift)) shift=0;
	if(shift < 0 || shift >= static_cast<int>(sizeof(int)*CHAR_BIT)-1) return false;
	list.OpenInterval=(1 << shift);
	if(list.OpenInterval > 1) --list.OpenInterval;
	if(!cursor.Empty())
	{
		int seconds=0;
		if(!cursor.NextInt(&seconds)) return false;
		if(seconds < 0 || seconds > static_cast<int>(UINT_MAX/1000U)) return false;
		list.dwIntervalTime=static_cast<DWORD>(seconds)*1000U;
	}
	return true;
}

inline bool ReadStageMonsterConfig(
	const char *sourceFile,
	rsSTG_MONSTER_LIST &output,
	smCHAR_INFO *characters,
	int characterCount)
{
	if(!sourceFile || !*sourceFile || characterCount < 0 || (characterCount > 0 && !characters)) return false;
	rsSTG_MONSTER_LIST staged={};
	staged.OpenInterval=0x7F;
	staged.OpenLimit=3;
	summit::io::LegacyFile file(sourceFile,"rb");
	if(!file) return false;
	int monsterCount=0;
	int percentageTotal=0;
	char lineBuffer[256];
	char command[64];
	while(file.ReadLine(lineBuffer,sizeof(lineBuffer)))
	{
		legacy_config::LineCursor line(lineBuffer);
		if(!line.NextToken(command,sizeof(command))) continue;
		legacy_config::LineCursor values(line.Position());
		if(TokenEquals(command,token::Actor,"*ACTOR"))
		{
			if(!ReadActor(values,staged,characters,characterCount,monsterCount,percentageTotal)) return false;
			continue;
		}
		if(TokenEquals(command,token::BossActor,"*BOSS_ACTOR"))
		{
			if(!ReadBossActor(values,staged,characters,characterCount)) return false;
			continue;
		}
		if(TokenEquals(command,token::MaxActorPosition,"*MAX_ACTOR_POS"))
		{
			staged.LimitMax=0; values.NextInt(&staged.LimitMax); continue;
		}
		if(TokenEquals(command,token::Delay,"*DELAY"))
		{
			if(!ReadDelay(values,staged)) return false;
			continue;
		}
		if(TokenEquals(command,token::MaxActor,"*MAX_ACTOR"))
		{
			staged.OpenLimit=0; values.NextInt(&staged.OpenLimit); continue;
		}
	}
	staged.Counter=monsterCount;
	staged.PecetageCount=percentageTotal;
	output=staged;
	return true;
}

} // namespace stage_spawn
} // namespace formats
} // namespace summit
