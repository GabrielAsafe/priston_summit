#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <vector>

namespace summit
{
namespace io
{
namespace legacy_binary
{



inline bool ReadSerializedField(const std::vector<unsigned char> &raw, const void *layoutStart,
	const void *fieldAddress, void *value, std::size_t valueBytes)
{
	if(!layoutStart || !fieldAddress || !value)
		return false;
	const std::uintptr_t base = reinterpret_cast<std::uintptr_t>(layoutStart);
	const std::uintptr_t field = reinterpret_cast<std::uintptr_t>(fieldAddress);
	if(field < base)
		return false;
	const std::size_t offset = static_cast<std::size_t>(field - base);
	if(offset > raw.size() || valueBytes > raw.size() - offset)
		return false;
	std::memcpy(value, &raw[offset], valueBytes);
	return true;
}

template <typename T>
inline bool ReadSerializedField(const std::vector<unsigned char> &raw, const void *layoutStart,
	const T *fieldAddress, T &value)
{
	return ReadSerializedField(raw, layoutStart, fieldAddress, &value, sizeof(value));
}

inline bool CheckedByteCount(std::size_t elementSize, int count, DWORD &bytes)
{
	if(count < 0)
		return false;
	const std::size_t itemCount = static_cast<std::size_t>(count);
	if(itemCount != 0 && elementSize > std::numeric_limits<std::size_t>::max() / itemCount)
		return false;
	const std::size_t total = elementSize * itemCount;
	if(total > static_cast<std::size_t>(std::numeric_limits<DWORD>::max()))
		return false;
	bytes = static_cast<DWORD>(total);
	return true;
}

inline bool ReadExact(HANDLE file, void *destination, DWORD bytes)
{
	if(bytes == 0)
		return true;
	if(!destination || file == INVALID_HANDLE_VALUE)
		return false;
	DWORD read = 0;
	return ::ReadFile(file, destination, bytes, &read, NULL) != FALSE && read == bytes;
}

inline bool WriteExact(HANDLE file, const void *source, DWORD bytes)
{
	if(bytes == 0)
		return true;
	if(!source || file == INVALID_HANDLE_VALUE)
		return false;
	DWORD written = 0;
	return ::WriteFile(file, source, bytes, &written, NULL) != FALSE && written == bytes;
}

} // namespace legacy_binary
} // namespace io
} // namespace summit
