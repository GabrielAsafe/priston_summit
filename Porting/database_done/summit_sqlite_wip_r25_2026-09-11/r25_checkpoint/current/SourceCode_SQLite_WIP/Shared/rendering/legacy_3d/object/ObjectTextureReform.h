#pragma once

#include <limits>

#include "ObjectStorage.h"

namespace summit
{
namespace rendering
{
namespace legacy3d
{
namespace object_texture
{

inline bool IsSupportedReformType(int type)
{
	return type >= 0 && type <= 4;
}

inline smTEXLINK *FindTextureLayer(smFACE &face, int layer)
{
	if(layer < 0 || layer >= object_storage::TextureLinksPerFaceCapacity)
		return 0;
	smTEXLINK *link = face.lpTexLink;
	for(int index = 0; index < layer && link; ++index)
		link = link->NextTex;
	return link;
}

inline bool LegacyProjectedCoordinate(int coordinate, int offset, int scale, float &result)
{
	if(scale == 0)
		return false;
	const long long translated = static_cast<long long>(coordinate) + offset;
	const long long quotient = (translated * static_cast<long long>(1 << FLOATNS)) / scale;
	if(quotient < std::numeric_limits<int>::min() || quotient > std::numeric_limits<int>::max())
		return false;
	result = static_cast<float>(static_cast<int>(quotient)) / static_cast<float>(fONE) + 0.5f;
	return true;
}

inline bool ReformFaceTexture(smOBJ3D &object, smFACE &face, int layer, const POINT3D &position,
	const POINT3D &size, const smVERTEX *worldVertices, int worldVertexCount)
{
	smTEXLINK *link = FindTextureLayer(face, layer);
	if(!link || !worldVertices || size.x == 0 || size.y == 0)
		return false;

	for(int corner = 0; corner < 3; ++corner)
	{
		const int vertexIndex = face.v[corner];
		if(vertexIndex < 0 || vertexIndex >= object.nVertex || vertexIndex >= worldVertexCount)
			return false;

		float u = 0.0f;
		float v = 0.0f;
		if(!LegacyProjectedCoordinate(worldVertices[vertexIndex].x, position.x, size.x, u) ||
			!LegacyProjectedCoordinate(worldVertices[vertexIndex].y, position.y, size.y, v))
			return false;
		link->u[corner] = -u;
		link->v[corner] = -v;
	}
	return true;
}

inline bool ReformTexture(smOBJ3D &object, int type, int materialIndex, int textureLayer,
	const POINT3D &position, const POINT3D &size, const smVERTEX *worldVertices, int worldVertexCount)
{
	if(!IsSupportedReformType(type))
		return true;
	if(object.nFace < 0 || (object.nFace > 0 && !object.Face) || textureLayer < 0)
		return false;

	for(int index = 0; index < object.nFace; ++index)
	{
		smFACE &face = object.Face[index];
		if(face.v[3] != materialIndex)
			continue;
		if(!ReformFaceTexture(object, face, textureLayer, position, size, worldVertices, worldVertexCount))
			return false;
	}
	return true;
}

} // namespace object_texture
} // namespace legacy3d
} // namespace rendering
} // namespace summit
