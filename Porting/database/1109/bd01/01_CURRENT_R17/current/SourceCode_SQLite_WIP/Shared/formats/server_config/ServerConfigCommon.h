#pragma once

#include <cerrno>
#include <climits>
#include <cstddef>
#include <cstdlib>
#include <cstring>

#include "formats/legacy_config/LegacyLineCursor.h"

namespace summit
{
namespace formats
{
namespace server_config
{

enum class ParseResult
{
	NotHandled = 0,
	Handled = 1,
	Error = -1
};

inline bool Matches(const char *command, const char *legacyToken, const char *asciiToken)
{
	return command && ((legacyToken && std::strcmp(command, legacyToken) == 0) ||
		(asciiToken && legacy_config::EqualsIgnoreCaseAscii(command, asciiToken)));
}

inline bool ParseInteger(const char *text, int *value)
{
	if(!text || !value || !text[0])
		return false;
	errno = 0;
	char *end = 0;
	const long parsed = std::strtol(text, &end, 10);
	if(errno == ERANGE || end == text || *end != 0 || parsed < INT_MIN || parsed > INT_MAX)
		return false;
	*value = static_cast<int>(parsed);
	return true;
}

inline bool ReadInteger(legacy_config::LineCursor &cursor, int *value)
{
	char tokenValue[64];
	return cursor.NextValue(tokenValue, sizeof(tokenValue)) && ParseInteger(tokenValue, value);
}

inline bool ReadOptionalInteger(legacy_config::LineCursor &cursor, int defaultValue, int *value)
{
	if(!value)
		return false;
	if(cursor.Empty())
	{
		*value = defaultValue;
		return true;
	}
	return ReadInteger(cursor, value);
}


inline bool ReadTokenOrEmpty(legacy_config::LineCursor &cursor, char *output, std::size_t capacity)
{
	if(!output || capacity == 0)
		return false;
	if(cursor.Empty())
	{
		output[0] = 0;
		return true;
	}
	return cursor.NextToken(output, capacity);
}

inline bool ReadValueOrEmpty(legacy_config::LineCursor &cursor, char *output, std::size_t capacity)
{
	if(!output || capacity == 0)
		return false;
	if(cursor.Empty())
	{
		output[0] = 0;
		return true;
	}
	return cursor.NextValue(output, capacity);
}

template <typename Integer>
inline bool ReadAtoiOrDefault(legacy_config::LineCursor &cursor, int defaultValue, Integer &destination)
{
	if(cursor.Empty())
	{
		destination = static_cast<Integer>(defaultValue);
		return true;
	}
	char text[128];
	if(!cursor.NextValue(text, sizeof(text)))
		return false;
	destination = static_cast<Integer>(std::atoi(text));
	return true;
}

template <typename Integer>
inline bool ReadAtoiIfPresent(legacy_config::LineCursor &cursor, Integer &destination)
{
	if(cursor.Empty())
		return true;
	char text[128];
	if(!cursor.NextValue(text, sizeof(text)))
		return false;
	destination = static_cast<Integer>(std::atoi(text));
	return true;
}

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

} // namespace server_config
} // namespace formats
} // namespace summit
