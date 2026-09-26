#pragma once

#include "rendering/legacy_3d/stage/StageSurfaceQuery.h"

namespace summit
{
namespace rendering
{
namespace legacy3d
{
namespace stage_movement
{

struct SurfaceScanResult
{
	SurfaceScanResult()
		: groundHeight(CLIP_OUT), waterHeight(CLIP_OUT), lastWalkableFace(0) {}
	int groundHeight;
	int waterHeight;
	smSTAGE_FACE *lastWalkableFace;
};

template <typename WalkablePredicate>
inline SurfaceScanResult ScanSurfaces(const smSTAGE3D &stage, smSTAGE_FACE *const *faces,
	int faceCount, int x, int y, int z, int maximumRise, int initialGroundHeight,
	int initialWaterHeight, WalkablePredicate isWalkable)
{
	SurfaceScanResult result;
	result.groundHeight = initialGroundHeight;
	result.waterHeight = initialWaterHeight;
	if(!faces || faceCount <= 0)
		return result;

	for(int index = 0; index < faceCount; ++index)
	{
		smSTAGE_FACE *const face = faces[index];
		if(!face)
			continue;
		const smMATERIAL *const material =
			summit::rendering::legacy3d::stage_surface::MaterialForFace(stage, *face);
		if(!material)
			continue;

		if(isWalkable(*face, *material))
		{
			const int candidate = summit::rendering::legacy3d::stage_height::FaceHeightAtXZ(
				stage, *face, x, z);
			if(candidate == CLIP_OUT)
				continue;
			if(static_cast<long long>(candidate) - y < maximumRise && candidate > result.groundHeight)
				result.groundHeight = candidate;
			result.lastWalkableFace = face;
			continue;
		}

		if(material->Transparency > 0.1 || (material->MeshState & sMATS_SCRIPT_ORG_WATER))
		{
			const int candidate = summit::rendering::legacy3d::stage_height::FaceHeightAtXZ(
				stage, *face, x, z);
			if(candidate != CLIP_OUT && candidate > result.waterHeight)
				result.waterHeight = candidate;
		}
	}
	return result;
}

inline bool WaterTransitionBlocksMove(int groundHeight, int waterHeight, int objectHeight)
{
	if(groundHeight == CLIP_OUT || waterHeight == CLIP_OUT)
		return false;
	const long long fixedOne = (1LL << FLOATNS);
	const long long threshold = static_cast<long long>(groundHeight) +
		(objectHeight >> 1) + 10 * fixedOne;
	return static_cast<long long>(waterHeight) > threshold &&
		static_cast<long long>(waterHeight) < threshold + 5 * fixedOne;
}

template <typename Intersector>
inline bool HasSolidImpact(const smSTAGE3D &stage, smSTAGE_FACE *const *faces, int faceCount,
	Intersector intersects)
{
	bool blocked = false;
	summit::rendering::legacy3d::stage_surface::VisitSolidFaces(
		stage, faces, faceCount, [&](smSTAGE_FACE &face) {
			if(intersects(face))
			{
				blocked = true;
				return false;
			}
			return true;
		});
	return blocked;
}

struct ProbeEvaluation
{
	ProbeEvaluation()
		: groundHeight(CLIP_OUT), waterHeight(CLIP_OUT), lastWalkableFace(0), blocked(false) {}
	int groundHeight;
	int waterHeight;
	smSTAGE_FACE *lastWalkableFace;
	bool blocked;
};

template <typename WalkablePredicate, typename Intersector>
inline ProbeEvaluation EvaluateProbe(const smSTAGE3D &stage, smSTAGE_FACE *const *faces,
	int faceCount, const POINT3D &endpoint, int maximumRise, int objectHeight,
	int initialGroundHeight, int initialWaterHeight,
	WalkablePredicate isWalkable, Intersector intersects)
{
	ProbeEvaluation evaluation;
	const SurfaceScanResult scan = ScanSurfaces(
		stage, faces, faceCount, endpoint.x, endpoint.y, endpoint.z, maximumRise,
		initialGroundHeight, initialWaterHeight, isWalkable);
	evaluation.groundHeight = scan.groundHeight;
	evaluation.waterHeight = scan.waterHeight;
	evaluation.lastWalkableFace = scan.lastWalkableFace;
	if(evaluation.groundHeight == CLIP_OUT)
		return evaluation;

	evaluation.blocked = WaterTransitionBlocksMove(
		evaluation.groundHeight, evaluation.waterHeight, objectHeight);
	if(!evaluation.blocked)
		evaluation.blocked = HasSolidImpact(stage, faces, faceCount, intersects);
	return evaluation;
}

} // namespace stage_movement
} // namespace legacy3d
} // namespace rendering
} // namespace summit
