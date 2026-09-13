#pragma once

#include "rendering/legacy_3d/stage/StageHeightQuery.h"

namespace summit
{
namespace rendering
{
namespace legacy3d
{
namespace stage_surface
{

struct SurfaceHit
{
	SurfaceHit() : face(0), height(CLIP_OUT) {}
	smSTAGE_FACE *face;
	int height;
};

inline const smMATERIAL *MaterialForFace(const smSTAGE3D &stage, const smSTAGE_FACE &face)
{
	if(!stage.smMaterial || !stage.smMaterialGroup)
		return 0;
	const DWORD materialIndex = face.Vertex[3];
	if(materialIndex >= stage.smMaterialGroup->MaterialCount)
		return 0;
	return &stage.smMaterial[materialIndex];
}

inline bool HasMaterialState(const smSTAGE3D &stage, const smSTAGE_FACE &face, DWORD flags)
{
	const smMATERIAL *const material = MaterialForFace(stage, face);
	return material && (material->MeshState & flags) == flags;
}

inline bool IsSolidSurface(const smSTAGE3D &stage, const smSTAGE_FACE &face)
{
	return HasMaterialState(stage, face, SMMAT_STAT_CHECK_FACE);
}

template <typename Visitor>
inline bool VisitSolidFaces(const smSTAGE3D &stage, smSTAGE_FACE *const *faces, int faceCount,
	Visitor visitor)
{
	if(!faces || faceCount <= 0)
		return true;
	for(int index = 0; index < faceCount; ++index)
	{
		smSTAGE_FACE *const face = faces[index];
		if(!face || !IsSolidSurface(stage, *face))
			continue;
		if(!visitor(*face))
			return false;
	}
	return true;
}

inline int HighestWalkableHeight(const smSTAGE3D &stage, smSTAGE_FACE *const *faces,
	int faceCount, int x, int y, int z, int maximumRise)
{
	int height = CLIP_OUT;
	VisitSolidFaces(stage, faces, faceCount, [&](smSTAGE_FACE &face) {
		const int candidate = summit::rendering::legacy3d::stage_height::FaceHeightAtXZ(
			stage, face, x, z);
		if(candidate != CLIP_OUT && candidate - y < maximumRise && candidate > height)
			height = candidate;
		return true;
	});
	return height;
}

struct EmptyHeightResult
{
	EmptyHeightResult() : occupied(false), highest(CLIP_OUT) {}
	bool occupied;
	int highest;
};

inline EmptyHeightResult QueryEmptyHeight(const smSTAGE3D &stage,
	smSTAGE_FACE *const *faces, int faceCount, int x, int y, int z, int objectHeight)
{
	EmptyHeightResult result;
	VisitSolidFaces(stage, faces, faceCount, [&](smSTAGE_FACE &face) {
		const int candidate = summit::rendering::legacy3d::stage_height::FaceHeightAtXZ(
			stage, face, x, z);
		if(candidate == CLIP_OUT)
			return true;
		if(candidate > result.highest)
			result.highest = candidate;
		if(candidate < y + objectHeight && candidate > y)
			result.occupied = true;
		return true;
	});
	return result;
}

inline int ClosestSolidSurfaceDistance(const smSTAGE3D &stage,
	smSTAGE_FACE *const *faces, int faceCount, int x, int y, int z)
{
	int closest = 1000000;
	VisitSolidFaces(stage, faces, faceCount, [&](smSTAGE_FACE &face) {
		const int candidate = summit::rendering::legacy3d::stage_height::FaceHeightAtXZ(
			stage, face, x, z);
		if(candidate == CLIP_OUT)
			return true;
		const long long delta = static_cast<long long>(candidate) - y;
		const long long distance = delta < 0 ? -delta : delta;
		if(distance < closest)
			closest = static_cast<int>(distance);
		return true;
	});
	return closest == 1000000 ? CLIP_OUT : closest;
}

inline bool FindHighestIceSurface(smSTAGE3D &stage, smSTAGE_FACE *const *faces, int faceCount,
	int x, int y, int z, int maximumAbove, SurfaceHit &hit)
{
	hit = SurfaceHit();
	if(!faces || faceCount <= 0)
		return false;
	for(int index = 0; index < faceCount; ++index)
	{
		smSTAGE_FACE *const face = faces[index];
		if(!face || !HasMaterialState(stage, *face,
			SMMAT_STAT_CHECK_FACE | sMATS_SCRIPT_CHECK_ICE))
			continue;

		const int candidate = summit::rendering::legacy3d::stage_height::FaceHeightAtXZ(
			stage, *face, x, z);
		if(candidate == CLIP_OUT || candidate - y >= maximumAbove)
			continue;
		if(!hit.face || candidate > hit.height)
		{
			hit.face = face;
			hit.height = candidate;
		}
	}
	return hit.face != 0;
}

} // namespace stage_surface
} // namespace legacy3d
} // namespace rendering
} // namespace summit
