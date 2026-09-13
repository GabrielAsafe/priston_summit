#pragma once

#include <cerrno>
#include <climits>
#include <cstdlib>

namespace summit
{
namespace io
{

inline bool ParseNonNegativeInt(const char *text, int *value)
{
	if(!text || !value || !*text)
		return false;

	errno = 0;
	char *end = nullptr;
	const long parsed = std::strtol(text, &end, 10);
	if(errno == ERANGE || end == text || *end != '\0' || parsed < 0 || parsed > INT_MAX)
		return false;

	*value = static_cast<int>(parsed);
	return true;
}

inline bool IndexInRange(int index, int count)
{
	return index >= 0 && count >= 0 && index < count;
}

}
}
