#pragma once

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <vector>

#include "io/LegacyFile.h"
#include "formats/legacy_config/LegacyLineCursor.h"

namespace summit
{
namespace formats
{
namespace belatra
{

template <std::size_t N>
inline bool CopyText(char (&destination)[N], const char *source)
{
	if(!source)
		return false;
	const std::size_t length = std::strlen(source);
	if(length >= N)
		return false;
	std::memcpy(destination, source, length + 1);
	return true;
}

template <typename MonsterEntry>
inline void ResetMonsterEntry(MonsterEntry &entry)
{
	entry.szName[0] = 0;
	entry.lpCharInfo = 0;
	entry.perCount = 0;
}

template <typename Round>
inline void ResetRequiredRound(Round &round)
{
	ResetMonsterEntry(round.hCoreMonster1);
	ResetMonsterEntry(round.hCoreMonster2);
	ResetMonsterEntry(round.hCoreMonster3);
	ResetMonsterEntry(round.hCoreMonster4);
	ResetMonsterEntry(round.hCoreMonsterBoss);
	round.MaxMonster = 0;
}

template <typename MonsterEntry>
inline bool ReadWeightedMonster(legacy_config::LineCursor &cursor, MonsterEntry &entry)
{
	char name[256];
	int percentage = 0;
	if(!cursor.NextValue(name, sizeof(name)) || !cursor.NextInt(&percentage))
		return false;
	return CopyText(entry.szName, name) && (entry.perCount = percentage, true);
}

template <typename Round>
inline bool ReadRound(legacy_config::LineCursor &cursor, Round &round)
{
	if(!ReadWeightedMonster(cursor, round.hCoreMonster1) ||
	   !ReadWeightedMonster(cursor, round.hCoreMonster2) ||
	   !ReadWeightedMonster(cursor, round.hCoreMonster3) ||
	   !ReadWeightedMonster(cursor, round.hCoreMonster4))
		return false;

	char bossName[256];
	int maxMonster = 0;
	if(!cursor.NextValue(bossName, sizeof(bossName)) || !cursor.NextInt(&maxMonster))
		return false;
	if(!CopyText(round.hCoreMonsterBoss.szName, bossName))
		return false;
	round.hCoreMonsterBoss.perCount = 0;
	round.MaxMonster = maxMonster;
	return true;
}

template <typename Round>
inline bool ReadBelatraConfig(const char *fileName,
	Round *rounds,
	std::size_t roundCapacity,
	std::size_t requiredRounds,
	int *tax,
	char *text,
	std::size_t textCapacity,
	int *scoreDivisor)
{
	if(!fileName || !rounds || roundCapacity == 0 || requiredRounds > roundCapacity ||
	   !tax || !text || textCapacity == 0 || !scoreDivisor)
		return false;

	std::vector<Round> staged(rounds, rounds + roundCapacity);
	for(std::size_t index = 0; index < requiredRounds; ++index)
		ResetRequiredRound(staged[index]);

	int stagedTax = 0;
	int stagedScoreDivisor = 0;
	char stagedText[256] = {};
	std::size_t roundCount = 0;

	io::LegacyFile file(fileName, "rb");
	if(!file)
		return false;

	char line[512];
	while(file.ReadLine(line, sizeof(line)))
	{
		legacy_config::LineCursor cursor(line);
		char command[64];
		if(!cursor.NextToken(command, sizeof(command)))
			continue;

		if(legacy_config::EqualsIgnoreCaseAscii(command, "*BELATRA_TAX"))
		{
			if(!cursor.NextInt(&stagedTax))
				return false;
			continue;
		}
		if(legacy_config::EqualsIgnoreCaseAscii(command, "*BELATRA_TEXT"))
		{
			char value[256];
			if(!cursor.NextValue(value, sizeof(value)) || !CopyText(stagedText, value))
				return false;
			continue;
		}
		if(legacy_config::EqualsIgnoreCaseAscii(command, "*BELATRA_DIVSCORE"))
		{
			if(!cursor.NextInt(&stagedScoreDivisor))
				return false;
			continue;
		}
		if(legacy_config::EqualsIgnoreCaseAscii(command, "*BELATRA_MONSTER"))
		{
			if(roundCount < requiredRounds)
			{
				if(!ReadRound(cursor, staged[roundCount]))
					return false;
				++roundCount;
			}
		}
	}

	if(roundCount < requiredRounds)
		return false;

	if(stagedTax >= 0 && stagedTax < 100)
		*tax = stagedTax;
	if(stagedText[0])
	{
		const std::size_t length = std::strlen(stagedText);
		if(length >= textCapacity)
			return false;
		std::memcpy(text, stagedText, length + 1);
	}
	if(stagedScoreDivisor > 0)
		*scoreDivisor = stagedScoreDivisor;
	std::copy(staged.begin(), staged.end(), rounds);
	return true;
}

} // namespace belatra
} // namespace formats
} // namespace summit
