#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "formats/legacy_config/LegacyLineCursor.h"

namespace summit
{
namespace formats
{
namespace character_config
{

enum class ParseResult
{
	NotHandled,
	Handled,
	Error
};

inline bool IsToken(const char *command, const char *expected)
{
	return legacy_config::Equals(command, expected);
}

inline int ReadInt(legacy_config::LineCursor &cursor, int fallback = 0)
{
	int value = fallback;
	cursor.NextInt(&value);
	return value;
}

inline float ReadFloat(legacy_config::LineCursor &cursor, float fallback = 0.0f)
{
	char value[64];
	if(!cursor.NextToken(value, sizeof(value)))
		return fallback;
	return static_cast<float>(std::atof(value));
}

template <std::size_t N>
inline bool CopyText(char (&destination)[N], const char *source)
{
	if(!source)
	{
		destination[0] = 0;
		return false;
	}
	const std::size_t length = std::strlen(source);
	if(length >= N)
		return false;
	std::memcpy(destination, source, length + 1);
	return true;
}

inline bool CopyText(char *destination, std::size_t capacity, const char *source)
{
	if(!destination || capacity == 0 || !source)
		return false;
	const std::size_t length = std::strlen(source);
	if(length >= capacity)
		return false;
	std::memcpy(destination, source, length + 1);
	return true;
}

inline void *LegacyCodePointer(DWORD code)
{
	return reinterpret_cast<void *>(static_cast<std::uintptr_t>(code));
}

} // namespace character_config
} // namespace formats
} // namespace summit
