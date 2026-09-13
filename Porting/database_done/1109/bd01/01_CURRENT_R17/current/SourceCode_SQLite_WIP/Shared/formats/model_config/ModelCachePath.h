#pragma once

#include <cstddef>

namespace summit
{
namespace formats
{
namespace model_config
{

inline bool CanonicalizeChecksumPath(const char *input, char *output, std::size_t capacity)
{
	if(!input || !output || capacity == 0)
		return false;

	std::size_t write = 0;
	bool lastWasSeparator = false;
	for(const char *cursor = input; *cursor; ++cursor)
	{
		char ch = *cursor;
		if(ch == '/' || ch == '\\')
		{
			if(lastWasSeparator)
				continue;
			ch = '\\';
			lastWasSeparator = true;
		}
		else
			lastWasSeparator = false;

		if(write + 1 >= capacity)
		{
			output[0] = 0;
			return false;
		}
		output[write++] = ch;
	}
	output[write] = 0;
	return true;
}

} // namespace model_config
} // namespace formats
} // namespace summit
