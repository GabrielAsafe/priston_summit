#pragma once

#include "io/LegacyWin32Handle.h"

namespace summit
{
namespace io
{
namespace legacy_win32
{

inline bool SeekAbsolute(HANDLE file, DWORD offset)
{
	if(file == INVALID_HANDLE_VALUE)
		return false;
	::SetLastError(NO_ERROR);
	const DWORD result = ::SetFilePointer(file, static_cast<LONG>(offset), NULL, FILE_BEGIN);
	return result != INVALID_SET_FILE_POINTER || ::GetLastError() == NO_ERROR;
}

inline bool Tell32(HANDLE file, DWORD &offset)
{
	if(file == INVALID_HANDLE_VALUE)
		return false;
	::SetLastError(NO_ERROR);
	const DWORD result = ::SetFilePointer(file, 0, NULL, FILE_CURRENT);
	if(result == INVALID_SET_FILE_POINTER && ::GetLastError() != NO_ERROR)
		return false;
	offset = result;
	return true;
}

inline bool FileSize32(HANDLE file, DWORD &size)
{
	if(file == INVALID_HANDLE_VALUE)
		return false;
	DWORD high = 0;
	::SetLastError(NO_ERROR);
	const DWORD low = ::GetFileSize(file, &high);
	if(high != 0)
		return false;
	if(low == INVALID_FILE_SIZE && ::GetLastError() != NO_ERROR)
		return false;
	size = low;
	return true;
}

} // namespace legacy_win32
} // namespace io
} // namespace summit
