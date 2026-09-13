#pragma once

#include <algorithm>
#include <climits>
#include <cstddef>
#include <cstring>
#include <memory>
#include <new>
#include <vector>

#include "rendering/legacy_3d/stage/StageGeometryAccess.h"

namespace summit
{
namespace rendering
{
namespace legacy3d
{
namespace stage_area
{

static const std::size_t MaxCellsPerFace = 4096;

inline bool FacePoints(const smSTAGE3D &stage, int faceIndex, POINT3D (&points)[3])
{
	if(!stage.Face || faceIndex < 0 || faceIndex >= stage.nFace)
		return false;
	const smSTAGE_VERTEX *vertices[3] = { 0, 0, 0 };
	if(!summit::rendering::legacy3d::stage_geometry::FaceVertices(
		stage, stage.Face[faceIndex], vertices))
		return false;
	for(int corner = 0; corner < 3; ++corner)
	{
		points[corner].x = vertices[corner]->x >> FLOATNS;
		points[corner].y = vertices[corner]->y >> FLOATNS;
		points[corner].z = vertices[corner]->z >> FLOATNS;
	}
	return true;
}

template <typename Visitor>
inline bool VisitCoveredCells(const POINT3D &p1, const POINT3D &p2, const POINT3D &p3, Visitor visitor)
{
	const POINT3D *points[3] = { &p1, &p2, &p3 };
	int left = points[0]->x;
	int right = points[0]->x;
	int top = points[0]->z;
	int bottom = points[0]->z;
	for(int index = 1; index < 3; ++index)
	{
		left = std::min(left, points[index]->x);
		right = std::max(right, points[index]->x);
		top = std::min(top, points[index]->z);
		bottom = std::max(bottom, points[index]->z);
	}

	left >>= 6;
	right = (right >> 6) + 1;
	top >>= 6;
	bottom = (bottom >> 6) + 1;
	if(right < left || bottom < top)
		return false;

	const long long width = static_cast<long long>(right) - left;
	const long long height = static_cast<long long>(bottom) - top;
	if(width < 0 || height < 0 ||
		width > static_cast<long long>(MaxCellsPerFace) ||
		height > static_cast<long long>(MaxCellsPerFace) ||
		width * height > static_cast<long long>(MaxCellsPerFace))
		return false;

	for(int x = left; x < right; ++x)
	{
		for(int y = top; y < bottom; ++y)
		{
			if(!visitor(x & 0xFF, y & 0xFF))
				return false;
		}
	}
	return true;
}

inline bool CollectCoveredCells(const POINT3D &p1, const POINT3D &p2, const POINT3D &p3,
	POINT *cells, std::size_t capacity, int &cellCount)
{
	cellCount = 0;
	if(!cells || capacity == 0)
		return false;
	return VisitCoveredCells(p1, p2, p3, [&](int x, int y) {
		if(static_cast<std::size_t>(cellCount) >= capacity)
			return false;
		cells[cellCount].x = x;
		cells[cellCount].y = y;
		++cellCount;
		return true;
	});
}

inline bool BuildAreaIndex(smSTAGE3D &stage)
{
	if(stage.nFace < 0 || stage.nVertex < 0 || (stage.nFace > 0 && (!stage.Face || !stage.Vertex)))
		return false;

	const std::size_t gridSize = static_cast<std::size_t>(MAP_SIZE) * MAP_SIZE;
	std::vector<DWORD> counts(gridSize, 0);
	POINT3D points[3];
	for(int faceIndex = 0; faceIndex < stage.nFace; ++faceIndex)
	{
		if(!FacePoints(stage, faceIndex, points))
			return false;
		if(!VisitCoveredCells(points[0], points[1], points[2], [&](int x, int y) {
			DWORD &count = counts[static_cast<std::size_t>(y) * MAP_SIZE + x];
			if(count == static_cast<DWORD>(INT_MAX))
				return false;
			++count;
			return true;
		}))
			return false;
	}

	std::vector<int> offsets(gridSize, -1);
	std::size_t requiredWords = 0;
	for(std::size_t cell = 0; cell < gridSize; ++cell)
	{
		if(counts[cell] == 0)
			continue;
		const std::size_t cellWords = static_cast<std::size_t>(counts[cell]) + 1;
		if(requiredWords > static_cast<std::size_t>(INT_MAX) - cellWords)
			return false;
		offsets[cell] = static_cast<int>(requiredWords);
		requiredWords += cellWords;
	}

	std::unique_ptr<DWORD[]> areaBuffer;
	if(requiredWords > 0)
	{
		areaBuffer.reset(new(std::nothrow) DWORD[requiredWords]());
		if(!areaBuffer)
			return false;
	}
	std::vector<DWORD> written(gridSize, 0);
	for(int faceIndex = 0; faceIndex < stage.nFace; ++faceIndex)
	{
		if(!FacePoints(stage, faceIndex, points))
			return false;
		if(!VisitCoveredCells(points[0], points[1], points[2], [&](int x, int y) {
			const std::size_t cell = static_cast<std::size_t>(y) * MAP_SIZE + x;
			if(offsets[cell] < 0 || written[cell] >= counts[cell])
				return false;
			DWORD *list = areaBuffer.get() + offsets[cell];
			list[++written[cell]] = static_cast<DWORD>(faceIndex);
			list[0] = written[cell];
			return true;
		}))
			return false;
	}

	delete[] stage.lpwAreaBuff;
	stage.lpwAreaBuff = areaBuffer.release();
	stage.wAreaSize = static_cast<int>(requiredWords);
	std::memset(stage.StageArea, 0, sizeof(stage.StageArea));
	for(int y = 0; y < MAP_SIZE; ++y)
	{
		for(int x = 0; x < MAP_SIZE; ++x)
		{
			const std::size_t cell = static_cast<std::size_t>(y) * MAP_SIZE + x;
			if(offsets[cell] >= 0)
				stage.StageArea[x][y] = stage.lpwAreaBuff + offsets[cell];
		}
	}
	return true;
}

} // namespace stage_area
} // namespace legacy3d
} // namespace rendering
} // namespace summit
