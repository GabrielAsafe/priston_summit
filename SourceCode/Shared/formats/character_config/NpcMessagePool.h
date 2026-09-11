#pragma once

#include <cstddef>

namespace summit
{
namespace formats
{
namespace character_config
{

template <typename Code, std::size_t MaxMessages, std::size_t MessageCapacity,
	typename HashFn, typename EqualFn, typename CopyFn>
inline char *InternNpcMessage(
	const char *message,
	char (&messages)[MaxMessages][MessageCapacity],
	Code (&codes)[MaxMessages],
	int &count,
	HashFn hash,
	EqualFn equal,
	CopyFn copy)
{
	if(count >= static_cast<int>(MaxMessages))
		return 0;

	const Code code = hash(message);
	int index = 0;
	for(; index < count; ++index)
	{
		if(code == codes[index] && equal(messages[index], message))
			break;
	}

	if(index < count)
		return messages[index];

	codes[count] = code;
	char *stored = messages[count++];
	copy(stored, MessageCapacity, message);
	return stored;
}

} // namespace character_config
} // namespace formats
} // namespace summit
