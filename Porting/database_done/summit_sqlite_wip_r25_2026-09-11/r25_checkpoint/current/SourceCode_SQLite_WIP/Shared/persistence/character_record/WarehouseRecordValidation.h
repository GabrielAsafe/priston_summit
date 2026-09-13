#pragma once

#include <cstddef>
#include <cstring>

namespace summit
{
namespace persistence
{
namespace character_record
{

inline bool ValidateWarehouseTransferLayout(int storedSize,
	int dataSize,
	const unsigned char *data,
	std::size_t dataCapacity,
	std::size_t bytesRead,
	std::size_t headerSize)
{
	if(!data) return false;
	if(bytesRead < headerSize) return false;
	if(storedSize <= 0 || static_cast<std::size_t>(storedSize) > bytesRead) return false;
	if(dataSize < 0 || static_cast<std::size_t>(dataSize) > dataCapacity) return false;
	if(static_cast<std::size_t>(dataSize) > bytesRead - headerSize) return false;
	if(static_cast<std::size_t>(storedSize) < headerSize + static_cast<std::size_t>(dataSize)) return false;

	if(dataSize)
	{
		if(dataSize < static_cast<int>(sizeof(int))) return false;
		int encodedSize = 0;
		std::memcpy(&encodedSize, data, sizeof(encodedSize));
		if(encodedSize < static_cast<int>(sizeof(int)) || encodedSize > dataSize) return false;
	}

	return true;
}

} // namespace character_record
} // namespace persistence
} // namespace summit
