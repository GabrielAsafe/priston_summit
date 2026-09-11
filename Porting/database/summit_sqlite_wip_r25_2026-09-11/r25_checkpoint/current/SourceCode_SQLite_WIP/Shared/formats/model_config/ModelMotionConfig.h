#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <cstring>

#include "formats/legacy_config/LegacyLineCursor.h"

namespace summit
{
namespace formats
{
namespace model_config
{

struct MotionKeywords
{
	const char *repeat;
	const char *run;
	const char *attack;
	const char *dead;
	const char *deadAlternate;
	const char *damage;
	const char *damageAlternate;
	const char *damageAlternate2;
	const char *sometimes;
	const char *fallDown;
	const char *fallStandSmall;
	const char *fallStandLarge;
	const char *stand;
	const char *standAlternate;
	const char *walk;
	const char *restart;
	const char *eat;
	const char *skill;
	const char *warp;
	const char *warpAlternate;
	const char *yahoo;
};

struct TalkMotionKeywords
{
	const char *ar;
	const char *e;
	const char *oh;
	const char *eye;
	const char *blank;
	const char *smile;
	const char *grumble;
	const char *sorrow;
	const char *startled;
	const char *nature;
	const char *special;
};

inline bool Contains(const char *text, const char *needle)
{
	return text && needle && *needle && std::strstr(text, needle) != 0;
}

inline bool ParseParenthesizedRate(const char *token, int *rate)
{
	if(!token || !rate)
		return false;
	const std::size_t length = std::strlen(token);
	if(length <= 2 || length >= 6 || token[0] != '(' || token[length - 1] != ')')
		return false;

	char number[10] = { 0 };
	std::memcpy(number, token + 1, length - 2);
	char *end = 0;
	const long parsed = std::strtol(number, &end, 10);
	if(end == number || *end != 0)
		return false;
	*rate = static_cast<int>(std::max<long>(0, std::min<long>(parsed, 100)));
	return true;
}

inline bool ParseMotionInfo(smMOTIONINFO &motion, const char *text, const char *repeatToken, int *rate)
{
	legacy_config::LineCursor cursor(text);
	int startFrame = 0;
	int endFrame = 0;
	if(!cursor.NextInt(&startFrame) || !cursor.NextInt(&endFrame))
		return false;

	motion.StartFrame = static_cast<DWORD>(startFrame);
	motion.EndFrame = static_cast<DWORD>(endFrame);
	for(int index = 0; index < 4; ++index)
		motion.EventFrame[index] = 0;

	char token[256] = { 0 };
	int eventCount = 0;
	while(cursor.NextToken(token, sizeof(token)))
	{
		if(token[0] < '0' || token[0] > '9' || eventCount >= 4)
			break;
		motion.EventFrame[eventCount++] = static_cast<DWORD>((std::atoi(token) - startFrame) * 160);
	}

	if(repeatToken && std::strcmp(token, repeatToken) == 0)
		motion.Repeat = TRUE;
	if(token[0] != '(' && token[0] >= ' ' && static_cast<unsigned char>(token[0]) <= 0x7F)
		motion.KeyCode = token[0];

	if(rate)
	{
		char rateToken[256] = { 0 };
		if(cursor.NextToken(rateToken, sizeof(rateToken)))
			ParseParenthesizedRate(rateToken, rate);
	}
	return true;
}

inline void ApplyMotionState(smMOTIONINFO &motion, const char *command, const MotionKeywords &words)
{
	motion.State = TRUE;
	if(Contains(command, words.run))
	{
		motion.State = CHRMOTION_STATE_RUN;
		motion.Repeat = TRUE;
	}
	if(Contains(command, words.attack)) motion.State = CHRMOTION_STATE_ATTACK;
	if(Contains(command, words.dead) || Contains(command, words.deadAlternate)) motion.State = CHRMOTION_STATE_DEAD;
	if(Contains(command, words.damage) || Contains(command, words.damageAlternate) || Contains(command, words.damageAlternate2)) motion.State = CHRMOTION_STATE_DAMAGE;
	if(Contains(command, words.sometimes)) motion.State = CHRMOTION_STATE_SOMETIME;
	if(Contains(command, words.fallDown)) motion.State = CHRMOTION_STATE_FALLDOWN;
	if(Contains(command, words.fallStandSmall)) motion.State = CHRMOTION_STATE_FALLSTAND;
	if(Contains(command, words.fallStandLarge)) motion.State = CHRMOTION_STATE_FALLDAMAGE;
	if(Contains(command, words.stand) || Contains(command, words.standAlternate))
	{
		motion.State = CHRMOTION_STATE_STAND;
		motion.Repeat = TRUE;
	}
	if(Contains(command, words.walk))
	{
		motion.State = CHRMOTION_STATE_WALK;
		motion.Repeat = TRUE;
	}
	if(Contains(command, words.restart)) motion.State = CHRMOTION_STATE_RESTART;
	if(Contains(command, words.eat)) motion.State = CHRMOTION_STATE_EAT;
	if(Contains(command, words.skill))
	{
		motion.State = CHRMOTION_STATE_SKILL;
		motion.SkillCodeList[0] = 0;
	}
	if(Contains(command, words.warp) || Contains(command, words.warpAlternate)) motion.State = CHRMOTION_STATE_WARP;
	if(Contains(command, words.yahoo)) motion.State = CHRMOTION_STATE_YAHOO;
	if(motion.State == CHRMOTION_STATE_DEAD && motion.EndFrame >= 8)
		motion.EndFrame -= 8;
}

inline void ApplyTalkMotionState(smMOTIONINFO &motion, const char *command, const TalkMotionKeywords &words)
{
	if(Contains(command, words.ar)) motion.State = CHRMOTION_STATE_TALK_AR;
	else if(Contains(command, words.e)) motion.State = CHRMOTION_STATE_TALK_E;
	else if(Contains(command, words.oh)) motion.State = CHRMOTION_STATE_TALK_OH;
	else if(Contains(command, words.eye)) motion.State = CHRMOTION_STATE_TALK_EYE;
	else if(Contains(command, words.blank)) motion.State = CHRMOTION_STATE_TALK_BLANK;
	else if(Contains(command, words.smile)) motion.State = CHRMOTION_STATE_TALK_SMILE;
	else if(Contains(command, words.grumble)) motion.State = CHRMOTION_STATE_TALK_GRUMBLE;
	else if(Contains(command, words.sorrow)) motion.State = CHRMOTION_STATE_TALK_SORROW;
	else if(Contains(command, words.startled)) motion.State = CHRMOTION_STATE_TALK_STARTLED;
	else if(Contains(command, words.nature)) motion.State = CHRMOTION_STATE_TALK_NATURE;
	else if(Contains(command, words.special)) motion.State = CHRMOTION_STATE_TALK_SPECIAL;
}


inline bool AppendMotion(
	smMODELINFO &model,
	const char *command,
	const char *arguments,
	int motionFrame,
	const char *repeatToken,
	const MotionKeywords &words,
	int *lastMotionIndex)
{
	if(model.MotionCount >= MOTION_INFO_MAX)
		return false;
	const int index = static_cast<int>(model.MotionCount);
	smMOTIONINFO &motion = model.MotionInfo[index];
	// The legacy model format allows far more motions than the 30-entry NPC
	// random-idle rate table.  Parse the motion normally beyond that point,
	// but do not pass an out-of-bounds rate slot.
	int *npcRate = index < NPC_MOTION_INFO_MAX ? &model.NpcMotionRate[index] : 0;
	if(!ParseMotionInfo(motion, arguments, repeatToken, npcRate))
		return false;
	motion.MotionFrame = motionFrame;
	ApplyMotionState(motion, command, words);
	if(lastMotionIndex)
		*lastMotionIndex = index;
	++model.MotionCount;
	return true;
}

inline bool AppendTalkMotion(
	smMODELINFO &model,
	const char *command,
	const char *arguments,
	int motionFrame,
	const char *repeatToken,
	const TalkMotionKeywords &words)
{
	if(model.TalkMotionCount >= TALK_MOTION_INFO_MAX)
		return false;
	const int index = static_cast<int>(model.TalkMotionCount);
	smMOTIONINFO &motion = model.TalkMotionInfo[index];
	if(!ParseMotionInfo(motion, arguments, repeatToken, &model.TalkMotionRate[index]))
		return false;
	motion.MotionFrame = motionFrame;
	ApplyTalkMotionState(motion, command, words);
	++model.TalkMotionCount;
	return true;
}

} // namespace model_config
} // namespace formats
} // namespace summit
