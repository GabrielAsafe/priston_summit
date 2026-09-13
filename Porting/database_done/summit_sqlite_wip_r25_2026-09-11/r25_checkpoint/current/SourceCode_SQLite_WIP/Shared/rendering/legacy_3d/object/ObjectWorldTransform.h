#pragma once

#include <cstdint>
#include <limits>

namespace summit
{
namespace rendering
{
namespace legacy3d
{
namespace object_world
{

inline bool ToInt(std::int64_t value, int &result)
{
	if(value < std::numeric_limits<int>::min() || value > std::numeric_limits<int>::max())
		return false;
	result = static_cast<int>(value);
	return true;
}

inline bool TransformLegacyVector(const smMATRIX &matrix,
	int sourceX, int sourceY, int sourceZ, int &resultX, int &resultY, int &resultZ)
{
	const std::int64_t rotatedX = static_cast<std::int64_t>(sourceX) * matrix._11 +
		static_cast<std::int64_t>(sourceY) * matrix._21 + static_cast<std::int64_t>(sourceZ) * matrix._31;
	const std::int64_t rotatedY = static_cast<std::int64_t>(sourceX) * matrix._12 +
		static_cast<std::int64_t>(sourceY) * matrix._22 + static_cast<std::int64_t>(sourceZ) * matrix._32;
	const std::int64_t rotatedZ = static_cast<std::int64_t>(sourceX) * matrix._13 +
		static_cast<std::int64_t>(sourceY) * matrix._23 + static_cast<std::int64_t>(sourceZ) * matrix._33;

	return ToInt((rotatedX >> FLOATNS) + matrix._41, resultX) &&
		ToInt((rotatedZ >> FLOATNS) + matrix._43, resultY) &&
		ToInt((rotatedY >> FLOATNS) + matrix._42, resultZ);
}


inline bool TransformVertex(const smOBJ3D &object, bool useWorldNormalMatrix, int vertexIndex, smVERTEX &output)
{
	if(vertexIndex < 0 || vertexIndex >= object.nVertex || !object.Vertex)
		return false;

	const smVERTEX &source = object.Vertex[vertexIndex];
	const smOBJ3D *physiqueObject = 0;
	if(object.Physique)
	{
		physiqueObject = object.Physique[vertexIndex];
		if(!physiqueObject)
			return false;
	}

	const smMATRIX &positionMatrix = physiqueObject ? physiqueObject->mWorld : object.mWorld;
	const smMATRIX &normalMatrix = physiqueObject
		? (useWorldNormalMatrix ? physiqueObject->mWorld : physiqueObject->mLocal)
		: (useWorldNormalMatrix ? object.mWorld : object.mLocal);

	smVERTEX transformed = output;
	if(!TransformLegacyVector(positionMatrix, source.x, source.y, source.z,
		transformed.x, transformed.y, transformed.z) ||
		!TransformLegacyVector(normalMatrix, source.nx, source.ny, source.nz,
		transformed.nx, transformed.ny, transformed.nz))
		return false;
	output = transformed;
	return true;
}

inline bool BuildWorldVertices(const smOBJ3D &object, bool useWorldNormalMatrix,
	smVERTEX *output, int outputCapacity)
{
	if(object.nVertex < 0 || object.nVertex > outputCapacity || (object.nVertex > 0 && !output))
		return false;

	for(int index = 0; index < object.nVertex; ++index)
	{
		if(!TransformVertex(object, useWorldNormalMatrix, index, output[index]))
			return false;
	}
	return true;
}

} // namespace object_world
} // namespace legacy3d
} // namespace rendering
} // namespace summit
