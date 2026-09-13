#pragma once

#include <cerrno>
#include <climits>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <cctype>

namespace summit
{
namespace formats
{
namespace legacy_config
{

// Compatibility helper for the old fileread GetWord routine. Unlike
// LineCursor::NextToken it deliberately truncates overlong tokens while still
// advancing across the complete source token.
template <std::size_t N>
inline char *ReadWordTruncated(char (&output)[N], char *input)
{
	output[0] = 0;

	if(!input)
		return 0;

	while(*input == ' ' || *input == '\t')
		++input;

	std::size_t count = 0;
	while(*input != 0 && *input != ' ' && *input != '\t' && *input != '\n' && *input != '\r')
	{
		if(count + 1 < N)
			output[count++] = *input;
		++input;
	}

	output[count] = 0;
	return input;
}

// Cursor for the whitespace-delimited configuration files used by the legacy
// model/item/character loaders. It deliberately preserves the old separators
// (space and tab only) while making buffer limits and quoted values explicit.
class LineCursor
{
public:
	explicit LineCursor(const char *text)
		: current_(text ? text : "")
	{
	}

	const char *Position() const
	{
		return current_;
	}

	bool Empty()
	{
		SkipWhitespace();
		return IsLineEnd(*current_);
	}

	bool NextToken(char *output, std::size_t capacity)
	{
		if(!output || capacity == 0)
			return false;

		SkipWhitespace();
		if(IsLineEnd(*current_))
		{
			output[0] = 0;
			return false;
		}

		std::size_t length = 0;
		bool overflow = false;
		while(!IsTokenEnd(*current_))
		{
			if(length + 1 < capacity)
				output[length++] = *current_;
			else
				overflow = true;
			++current_;
		}

		output[length] = 0;
		if(overflow)
		{
			output[0] = 0;
			return false;
		}
		return length != 0;
	}

	bool NextValue(char *output, std::size_t capacity)
	{
		if(!output || capacity == 0)
			return false;

		SkipWhitespace();
		if(*current_ != '"')
			return NextToken(output, capacity);

		++current_;
		std::size_t length = 0;
		bool overflow = false;
		while(!IsLineEnd(*current_) && *current_ != '"')
		{
			if(length + 1 < capacity)
				output[length++] = *current_;
			else
				overflow = true;
			++current_;
		}

		if(*current_ != '"')
		{
			output[0] = 0;
			return false;
		}
		++current_;
		output[length] = 0;
		if(overflow)
		{
			output[0] = 0;
			return false;
		}
		return true;
	}

	bool NextInt(int *value)
	{
		char token[64];
		if(!value || !NextToken(token, sizeof(token)))
			return false;

		errno = 0;
		char *end = 0;
		const long parsed = std::strtol(token, &end, 10);
		if(errno == ERANGE || end == token || *end != 0 || parsed < INT_MIN || parsed > INT_MAX)
			return false;
		*value = static_cast<int>(parsed);
		return true;
	}

private:
	static bool IsLineEnd(char ch)
	{
		return ch == 0 || ch == '\r' || ch == '\n';
	}

	static bool IsTokenEnd(char ch)
	{
		return IsLineEnd(ch) || ch == ' ' || ch == '\t';
	}

	void SkipWhitespace()
	{
		while(*current_ == ' ' || *current_ == '\t')
			++current_;
	}

	const char *current_;
};

inline bool Equals(const char *left, const char *right)
{
	return left && right && std::strcmp(left, right) == 0;
}

inline bool EqualsIgnoreCaseAscii(const char *left, const char *right)
{
	if(!left || !right)
		return false;

	while(*left && *right)
	{
		const unsigned char lhs = static_cast<unsigned char>(*left);
		const unsigned char rhs = static_cast<unsigned char>(*right);
		if(std::tolower(lhs) != std::tolower(rhs))
			return false;
		++left;
		++right;
	}

	return *left == 0 && *right == 0;
}

} // namespace legacy_config
} // namespace formats
} // namespace summit
