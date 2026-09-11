#pragma once

#include <cstddef>

namespace summit
{
namespace security
{
namespace integrity
{

inline bool ValidateClientFuncPosLayout(int storedSize,
	int funcCount,
	std::size_t structureSize,
	int maxFunctions)
{
	if(storedSize != static_cast<int>(structureSize)) return false;
	if(funcCount < 0 || funcCount > maxFunctions) return false;
	return true;
}

} // namespace integrity
} // namespace security
} // namespace summit
