#pragma once

#include <cstddef>
#include <cstdint>

namespace summit
{
namespace rendering
{
namespace legacy3d
{
namespace stage_area_storage
{

struct AreaListView
{
	AreaListView() : data(0), faceCount(0), wordCount(0), offset(0) {}
	const DWORD *data;
	DWORD faceCount;
	std::size_t wordCount;
	std::size_t offset;
};

inline bool ResolveAreaOffset(const smSTAGE3D &stage, const DWORD *list, std::size_t &offset)
{
	if(!list || !stage.lpwAreaBuff || stage.wAreaSize <= 0)
		return false;
	const std::uintptr_t base = reinterpret_cast<std::uintptr_t>(stage.lpwAreaBuff);
	const std::uintptr_t value = reinterpret_cast<std::uintptr_t>(list);
	if(value < base)
		return false;
	const std::uintptr_t delta = value - base;
	if(delta % sizeof(DWORD) != 0)
		return false;
	offset = static_cast<std::size_t>(delta / sizeof(DWORD));
	return offset < static_cast<std::size_t>(stage.wAreaSize);
}

inline bool ReadAreaList(const smSTAGE3D &stage, int areaX, int areaZ, AreaListView &view)
{
	view = AreaListView();
	const DWORD *const list = stage.StageArea[areaX & 0xFF][areaZ & 0xFF];
	if(!list)
		return true;

	std::size_t offset = 0;
	if(!ResolveAreaOffset(stage, list, offset) || stage.nFace < 0)
		return false;
	const std::size_t remaining = static_cast<std::size_t>(stage.wAreaSize) - offset;
	if(remaining == 0)
		return false;
	const DWORD faceCount = list[0];
	if(faceCount > static_cast<DWORD>(stage.nFace))
		return false;
	const std::size_t wordCount = static_cast<std::size_t>(faceCount) + 1;
	if(wordCount > remaining)
		return false;
	for(std::size_t item = 1; item < wordCount; ++item)
	{
		if(list[item] >= static_cast<DWORD>(stage.nFace))
			return false;
	}
	view.data = list;
	view.faceCount = faceCount;
	view.wordCount = wordCount;
	view.offset = offset;
	return true;
}

} // namespace stage_area_storage
} // namespace legacy3d
} // namespace rendering
} // namespace summit
