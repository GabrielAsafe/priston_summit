#pragma once

#include <cstddef>
#include <cstring>

#include "io/LegacyFile.h"
#include "formats/legacy_config/LegacyLineCursor.h"

namespace summit
{
namespace formats
{
namespace security
{

template <std::size_t N>
inline bool CopyCrackerText(char (&destination)[N], const char *source)
{
	if(!source)
		return false;
	const std::size_t length = std::strlen(source);
	if(length >= N)
		return false;
	std::memcpy(destination, source, length + 1);
	return true;
}

inline bool ReadValueOrEmpty(legacy_config::LineCursor &cursor, char *output, std::size_t capacity)
{
	if(cursor.Empty())
	{
		if(!output || capacity == 0)
			return false;
		output[0] = 0;
		return true;
	}
	return cursor.NextValue(output, capacity);
}

template <typename Packet>
inline int ReadCrackerConfig(const char *fileName, Packet &packet, int opcode)
{
	if(!fileName)
		return 0;

	Packet staged = packet;
	int count = 0;
	io::LegacyFile file(fileName, "rb");
	if(!file)
		return 0;

	char line[512];
	while(file.ReadLine(line, sizeof(line)))
	{
		legacy_config::LineCursor cursor(line);
		char command[64];
		if(!cursor.NextToken(command, sizeof(command)) ||
		   !legacy_config::EqualsIgnoreCaseAscii(command, "*CRACK"))
			continue;

		if(count >= FIND_CRACK_LIST_MAX)
			break;

		char name1[64];
		char name2[64];
		char className[64];
		if(!ReadValueOrEmpty(cursor, name1, sizeof(name1)) ||
		   !ReadValueOrEmpty(cursor, name2, sizeof(name2)) ||
		   !ReadValueOrEmpty(cursor, className, sizeof(className)) ||
		   !CopyCrackerText(staged.CrackName[count].szName1, name1) ||
		   !CopyCrackerText(staged.CrackName[count].szName2, name2) ||
		   !CopyCrackerText(staged.CrackName[count].szClass, className))
			return 0;
		++count;
	}

	staged.code = opcode;
	staged.size = static_cast<int>(sizeof(staged.CrackName[0])) * count + 16;
	staged.FindCounter = count;
	packet = staged;
	return count;
}

} // namespace security
} // namespace formats
} // namespace summit
