#pragma once

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <limits>

namespace summit
{
namespace formats
{
namespace ase
{

// Lightweight cursor for one textual ASE line.
// It knows only the ASE token separators and quoted-string syntax; it has no
// dependency on renderer/game types and can therefore be shared by client and server.
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

	bool NextToken(char *output, std::size_t capacity)
	{
		if(!output || capacity == 0)
			return false;

		SkipSeparators();

		std::size_t length = 0;
		while(!IsTokenEnd(*current_))
		{
			if(length + 1 >= capacity)
			{
				output[0] = '\0';
				return false;
			}

			output[length++] = *current_++;
		}

		output[length] = '\0';
		return length != 0;
	}

	bool NextQuoted(char *output, std::size_t capacity)
	{
		if(!output || capacity == 0)
			return false;

		while(*current_ && *current_ != '\r' && *current_ != '\n' && *current_ != '"')
			++current_;

		if(*current_ != '"')
		{
			output[0] = '\0';
			return false;
		}

		++current_;
		std::size_t length = 0;
		while(*current_ && *current_ != '\r' && *current_ != '\n' && *current_ != '"')
		{
			if(length + 1 >= capacity)
			{
				output[0] = '\0';
				return false;
			}

			output[length++] = *current_++;
		}

		if(*current_ != '"')
		{
			output[0] = '\0';
			return false;
		}

		++current_;
		output[length] = '\0';
		return true;
	}

	bool NextInt(int *value)
	{
		char token[64];
		if(!value || !NextToken(token, sizeof(token)))
			return false;

		char *end = 0;
		const long parsed = std::strtol(token, &end, 10);
		if(end == token || *end != '\0' ||
			parsed < std::numeric_limits<int>::min() || parsed > std::numeric_limits<int>::max())
			return false;

		*value = static_cast<int>(parsed);
		return true;
	}

	bool NextFloat(float *value)
	{
		char token[64];
		if(!value || !NextToken(token, sizeof(token)))
			return false;

		char *end = 0;
		const double parsed = std::strtod(token, &end);
		if(end == token || *end != '\0')
			return false;

		*value = static_cast<float>(parsed);
		return true;
	}

	bool NextDouble(double *value)
	{
		char token[64];
		if(!value || !NextToken(token, sizeof(token)))
			return false;

		char *end = 0;
		const double parsed = std::strtod(token, &end);
		if(end == token || *end != '\0')
			return false;

		*value = parsed;
		return true;
	}

private:
	static bool IsSeparator(char ch)
	{
		return ch == ' ' || ch == '\t' || ch == ':';
	}

	static bool IsTokenEnd(char ch)
	{
		return ch == '\0' || ch == '\r' || ch == '\n' || IsSeparator(ch);
	}

	void SkipSeparators()
	{
		while(IsSeparator(*current_))
			++current_;
	}

	const char *current_;
};

inline bool TagEquals(const char *tag, const char *expected)
{
	return tag && expected && std::strcmp(tag, expected) == 0;
}

} // namespace ase
} // namespace formats
} // namespace summit
