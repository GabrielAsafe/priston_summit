#pragma once

#include <algorithm>

#include "rendering/legacy_3d/stage/StageAreaStorage.h"

namespace summit
{
namespace rendering
{
namespace legacy3d
{
namespace stage_visible_faces
{

template <typename Visitor>
inline bool VisitVisibleFaces(smSTAGE3D &stage, int mapTop, int mapBottom,
	const int *mapLeft, const int *mapRight, Visitor visitor)
{
	if(mapBottom < mapTop || !mapLeft || !mapRight || stage.nFace < 0 ||
		(stage.nFace > 0 && !stage.Face))
		return false;

	for(int row = mapTop; row < mapBottom; ++row)
	{
		int left = mapLeft[row & 0xFF];
		int right = mapRight[row & 0xFF];
		if(left > right)
			std::swap(left, right);

		for(int column = left; column < right; ++column)
		{
			summit::rendering::legacy3d::stage_area_storage::AreaListView area;
			if(!summit::rendering::legacy3d::stage_area_storage::ReadAreaList(
				stage, column, row, area))
				return false;
			if(!area.data)
				continue;

			for(DWORD item = 1; item <= area.faceCount; ++item)
			{
				smSTAGE_FACE &face = stage.Face[area.data[item]];
				if(face.sum == stage.SumCount)
					continue;
				if(!visitor(face))
					return false;
			}
		}
	}
	return true;
}

} // namespace stage_visible_faces
} // namespace legacy3d
} // namespace rendering
} // namespace summit
