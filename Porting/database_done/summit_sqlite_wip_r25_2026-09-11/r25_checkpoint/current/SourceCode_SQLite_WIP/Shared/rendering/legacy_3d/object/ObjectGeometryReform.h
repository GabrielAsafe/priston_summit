#pragma once

#include <algorithm>
#include <limits>
#include <vector>

namespace summit
{
namespace rendering
{
namespace legacy3d
{
namespace object_reform
{

inline bool ToInt(long long value, int &result)
{
	if(value < std::numeric_limits<int>::min() || value > std::numeric_limits<int>::max())
		return false;
	result = static_cast<int>(value);
	return true;
}

inline bool TransformCoordinate(int x, int y, int z, const smMATRIX &matrix, int &outX, int &outY, int &outZ)
{
	const long long tx = static_cast<long long>(x) * matrix._11 + static_cast<long long>(y) * matrix._21 + static_cast<long long>(z) * matrix._31;
	const long long ty = static_cast<long long>(x) * matrix._12 + static_cast<long long>(y) * matrix._22 + static_cast<long long>(z) * matrix._32;
	const long long tz = static_cast<long long>(x) * matrix._13 + static_cast<long long>(y) * matrix._23 + static_cast<long long>(z) * matrix._33;
	return ToInt((tx >> FLOATNS) + matrix._41, outX) &&
		ToInt((ty >> FLOATNS) + matrix._42, outY) &&
		ToInt((tz >> FLOATNS) + matrix._43, outZ);
}

inline bool TransformVertex(smVERTEX &vertex, const smMATRIX &matrix)
{
	int x = 0, y = 0, z = 0;
	int nx = 0, ny = 0, nz = 0;
	if(!TransformCoordinate(vertex.x, vertex.y, vertex.z, matrix, x, y, z) ||
		!TransformCoordinate(vertex.nx, vertex.ny, vertex.nz, matrix, nx, ny, nz))
		return false;
	vertex.x = x; vertex.y = y; vertex.z = z;
	vertex.nx = nx; vertex.ny = ny; vertex.nz = nz;
	return true;
}

inline bool AverageScale(const smOBJ3D &object, int &scale)
{
	const smOBJ3D *source = object.pParent ? object.pParent : &object;
	const long long sum = static_cast<long long>(source->sx) + source->sy + source->sz;
	const long long average = sum / 3;
	return ToInt(average, scale);
}

inline bool NormalizeRotationScale(smMATRIX &matrix, int scale)
{
	if(scale == 0)
		return true;
	int *entries[] = {
		&matrix._11, &matrix._12, &matrix._13,
		&matrix._21, &matrix._22, &matrix._23,
		&matrix._31, &matrix._32, &matrix._33
	};
	for(std::size_t index = 0; index < sizeof(entries) / sizeof(entries[0]); ++index)
	{
		int value = 0;
		if(!ToInt((static_cast<long long>(*entries[index]) << FLOATNS) / scale, value))
			return false;
		*entries[index] = value;
	}
	return true;
}

inline bool ReformTransform(smOBJ3D &object)
{
	if(object.nVertex < 0 || (object.nVertex > 0 && !object.Vertex))
		return false;

	int scale = 0;
	if(!AverageScale(object, scale))
		return false;
	smMATRIX normalized = object.Tm;
	if(!NormalizeRotationScale(normalized, scale))
		return false;
	smMATRIX inverse;
	smMatrixInvert(inverse, normalized);

	std::vector<smVERTEX> transformed(object.Vertex, object.Vertex + object.nVertex);
	for(std::size_t index = 0; index < transformed.size(); ++index)
	{
		if(!TransformVertex(transformed[index], inverse))
			return false;
	}

	object.Tm = normalized;
	object.TmInvert = inverse;
	std::copy(transformed.begin(), transformed.end(), object.Vertex);
	return true;
}

inline bool ReformPhysique(smOBJ3D &object)
{
	if(object.nVertex < 0 || (object.nVertex > 0 && (!object.Vertex || !object.Physique)))
		return false;

	std::vector<smVERTEX> transformed(object.Vertex, object.Vertex + object.nVertex);
	for(int index = 0; index < object.nVertex; ++index)
	{
		smOBJ3D *bone = object.Physique[index];
		if(!bone)
			return false;
		smMATRIX inverse;
		smMatrixInvert(inverse, bone->Tm);
		if(!TransformVertex(transformed[static_cast<std::size_t>(index)], inverse))
			return false;
	}
	std::copy(transformed.begin(), transformed.end(), object.Vertex);
	return true;
}

} // namespace object_reform
} // namespace legacy3d
} // namespace rendering
} // namespace summit
