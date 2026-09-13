#pragma once

#include <cstdint>
#include <limits>
#include <vector>

#include "rendering/legacy_3d/stage/StageGeometryAccess.h"
#include "rendering/legacy_3d/stage/StageSurfaceQuery.h"

namespace summit
{
namespace rendering
{
namespace legacy3d
{
namespace stage_lighting
{

struct NormalAccumulator
{
	NormalAccumulator() : x(0), y(0), z(0), count(0) {}
	std::int64_t x;
	std::int64_t y;
	std::int64_t z;
	int count;
};

inline int ClampToInt(std::int64_t value)
{
	if(value < static_cast<std::int64_t>(std::numeric_limits<int>::min()))
		return std::numeric_limits<int>::min();
	if(value > static_cast<std::int64_t>(std::numeric_limits<int>::max()))
		return std::numeric_limits<int>::max();
	return static_cast<int>(value);
}

inline bool BuildVertexNormals(smSTAGE3D &stage, std::vector<POINT3D> &vertexNormals)
{
	if(stage.nVertex < 0 || stage.nFace < 0 ||
		(stage.nVertex > 0 && !stage.Vertex) || (stage.nFace > 0 && !stage.Face))
		return false;

	std::vector<NormalAccumulator> accumulated(static_cast<std::size_t>(stage.nVertex));
	for(int faceIndex = 0; faceIndex < stage.nFace; ++faceIndex)
	{
		smSTAGE_FACE &face = stage.Face[faceIndex];
		const smSTAGE_VERTEX *vertices[3] = { 0, 0, 0 };
		if(!summit::rendering::legacy3d::stage_geometry::FaceVertices(stage, face, vertices))
			return false;

		POINT3D points[3];
		for(int corner = 0; corner < 3; ++corner)
		{
			points[corner].x = vertices[corner]->x >> FLOATNS;
			points[corner].y = vertices[corner]->y >> FLOATNS;
			points[corner].z = vertices[corner]->z >> FLOATNS;
		}
		POINT3D normal;
		if(!SetNormal(&points[0], &points[1], &points[2], &normal))
			return false;

		face.VectNormal[0] = static_cast<short>(normal.x);
		face.VectNormal[1] = static_cast<short>(normal.y);
		face.VectNormal[2] = static_cast<short>(normal.z);
		for(int corner = 0; corner < 3; ++corner)
		{
			NormalAccumulator &sum = accumulated[face.Vertex[corner]];
			sum.x += normal.x;
			sum.y += normal.y;
			sum.z += normal.z;
			if(sum.count == std::numeric_limits<int>::max())
				return false;
			++sum.count;
		}
	}

	vertexNormals.assign(static_cast<std::size_t>(stage.nVertex), POINT3D());
	for(int vertexIndex = 0; vertexIndex < stage.nVertex; ++vertexIndex)
	{
		const NormalAccumulator &sum = accumulated[vertexIndex];
		if(sum.count <= 0)
			continue;
		vertexNormals[vertexIndex].x = ClampToInt(sum.x / sum.count);
		vertexNormals[vertexIndex].y = ClampToInt(sum.y / sum.count);
		vertexNormals[vertexIndex].z = ClampToInt(sum.z / sum.count);
	}
	return true;
}

inline bool ApplyDirectionalLighting(smSTAGE3D &stage, const std::vector<POINT3D> &normals)
{
	if(stage.Contrast == 0 || normals.size() != static_cast<std::size_t>(stage.nVertex))
		return false;

	for(int vertexIndex = 0; vertexIndex < stage.nVertex; ++vertexIndex)
	{
		const POINT3D &normal = normals[vertexIndex];
		const std::int64_t dot = static_cast<std::int64_t>(normal.x) * stage.VectLight.x +
			static_cast<std::int64_t>(normal.y) * stage.VectLight.y +
			static_cast<std::int64_t>(normal.z) * stage.VectLight.z;
		std::int64_t shade = (dot >> FLOATNS) / stage.Contrast + stage.Bright;
		const std::int64_t red =
			(static_cast<std::int64_t>(stage.Vertex[vertexIndex].sDef_Color[SMC_R]) * shade) >> 8;
		const std::int64_t green =
			(static_cast<std::int64_t>(stage.Vertex[vertexIndex].sDef_Color[SMC_G]) * shade) >> 8;
		const std::int64_t blue =
			(static_cast<std::int64_t>(stage.Vertex[vertexIndex].sDef_Color[SMC_B]) * shade) >> 8;
		stage.Vertex[vertexIndex].sDef_Color[SMC_R] = static_cast<short>(ClampToInt(red));
		stage.Vertex[vertexIndex].sDef_Color[SMC_G] = static_cast<short>(ClampToInt(green));
		stage.Vertex[vertexIndex].sDef_Color[SMC_B] = static_cast<short>(ClampToInt(blue));
		stage.Vertex[vertexIndex].sDef_Color[SMC_A] = 255;
	}
	return true;
}

inline BYTE TransparencyToAlpha(float transparency)
{
	if(transparency <= 0.0f)
		return 255;
	if(transparency >= 1.0f)
		return 0;
	return static_cast<BYTE>(255 - static_cast<int>(transparency * 255.0f));
}

inline void ApplyMaterialTransparency(smSTAGE3D &stage)
{
	if(!stage.Face || !stage.Vertex)
		return;
	for(int faceIndex = 0; faceIndex < stage.nFace; ++faceIndex)
	{
		smSTAGE_FACE &face = stage.Face[faceIndex];
		const smMATERIAL *const material =
			summit::rendering::legacy3d::stage_surface::MaterialForFace(stage, face);
		if(!material || material->Transparency <= 0.0f ||
			!summit::rendering::legacy3d::stage_geometry::IsFaceTopologyValid(stage, face))
			continue;
		const BYTE alpha = TransparencyToAlpha(material->Transparency);
		for(int corner = 0; corner < 3; ++corner)
			stage.Vertex[face.Vertex[corner]].sDef_Color[SMC_A] = alpha;
	}
}

inline bool RebuildStageLighting(smSTAGE3D &stage, bool applyLight)
{
	std::vector<POINT3D> normals;
	if(!BuildVertexNormals(stage, normals))
		return false;
	if(applyLight && !ApplyDirectionalLighting(stage, normals))
		return false;
	ApplyMaterialTransparency(stage);
	return true;
}

} // namespace stage_lighting
} // namespace legacy3d
} // namespace rendering
} // namespace summit
