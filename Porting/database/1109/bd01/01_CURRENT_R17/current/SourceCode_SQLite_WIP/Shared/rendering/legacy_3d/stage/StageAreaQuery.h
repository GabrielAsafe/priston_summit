#pragma once

#include <cstddef>

#include "rendering/legacy_3d/stage/StageGeometryAccess.h"
#include "rendering/legacy_3d/stage/StageAreaStorage.h"
#include "rendering/legacy_3d/stage/StageQueryStamp.h"

namespace summit
{
namespace rendering
{
namespace legacy3d
{
namespace stage_area_query
{

static const std::size_t LegacyFaceBufferCapacity = 1000;

inline bool FaceOverlapsVerticalRange(const smSTAGE3D &stage, const smSTAGE_FACE &face,
	int top, int bottom)
{
	if(!summit::rendering::legacy3d::stage_geometry::IsFaceTopologyValid(stage, face))
		return false;

	for(int corner = 0; corner < 3; ++corner)
	{
		const int y = stage.Vertex[face.Vertex[corner]].y;
		if(y < top && y > bottom)
			return true;
	}
	return false;
}


template <typename Visitor>
inline bool VisitUniqueFacesInCell(smSTAGE3D &stage, int areaX, int areaZ, int token,
	Visitor visitor)
{
	if(!stage.Face || stage.nFace <= 0)
		return true;
	summit::rendering::legacy3d::stage_area_storage::AreaListView area;
	if(!summit::rendering::legacy3d::stage_area_storage::ReadAreaList(stage, areaX, areaZ, area))
		return false;
	if(!area.data)
		return true;

	for(DWORD item = 1; item <= area.faceCount; ++item)
	{
		const DWORD faceIndex = area.data[item];
		smSTAGE_FACE &face = stage.Face[faceIndex];
		if(!summit::rendering::legacy3d::stage_query::MarkFaceOnce(face, token))
			continue;
		if(!visitor(face))
			return false;
	}
	return true;
}

inline int CollectFaces(smSTAGE3D &stage, int x, int z, int width, int height,
	int top, int bottom, smSTAGE_FACE **output, std::size_t outputCapacity)
{
	if(!output || outputCapacity == 0 || !stage.Face || stage.nFace <= 0 ||
		width <= 0 || height <= 0)
		return 0;

	const int token = summit::rendering::legacy3d::stage_query::BeginFaceQuery(stage);
	const int startX = (x >> (FLOATNS + 6)) & 0xFF;
	const int startZ = (z >> (FLOATNS + 6)) & 0xFF;
	const int endX = startX + (width >> (FLOATNS + 6));
	const int endZ = startZ + (height >> (FLOATNS + 6));

	std::size_t outputCount = 0;
	for(int cellX = startX; cellX < endX; ++cellX)
	{
		for(int cellZ = startZ; cellZ < endZ; ++cellZ)
		{
			summit::rendering::legacy3d::stage_area_storage::AreaListView area;
			if(!summit::rendering::legacy3d::stage_area_storage::ReadAreaList(stage, cellX, cellZ, area))
				return static_cast<int>(outputCount);
			if(!area.data)
				continue;

			for(DWORD item = 1; item <= area.faceCount; ++item)
			{
				const DWORD faceIndex = area.data[item];

				smSTAGE_FACE &face = stage.Face[faceIndex];
				if(!summit::rendering::legacy3d::stage_query::MarkFaceOnce(face, token))
					continue;
				if(!FaceOverlapsVerticalRange(stage, face, top, bottom))
					continue;
				if(outputCount >= outputCapacity)
					return static_cast<int>(outputCount);
				output[outputCount++] = &face;
			}
		}
	}
	return static_cast<int>(outputCount);
}

inline int CollectFacesAroundPoint(smSTAGE3D &stage, int x, int z, int top, int bottom,
	smSTAGE_FACE **output, std::size_t outputCapacity)
{
	const int cellSpan = (1 << FLOATNS) * 64;
	return CollectFaces(stage, x - cellSpan, z - cellSpan, cellSpan * 2, cellSpan * 2,
		top, bottom, output, outputCapacity);
}

} // namespace stage_area_query
} // namespace legacy3d
} // namespace rendering
} // namespace summit
