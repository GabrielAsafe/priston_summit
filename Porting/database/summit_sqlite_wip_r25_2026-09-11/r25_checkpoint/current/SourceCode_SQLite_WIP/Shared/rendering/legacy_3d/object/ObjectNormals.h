#pragma once

#include <cmath>
#include <vector>

namespace summit
{
namespace rendering
{
namespace legacy3d
{
namespace object_normals
{

inline bool ClearGeometryNormals(smOBJ3D &object)
{
	if(object.nVertex < 0 || object.nVertex > object.MaxVertex ||
		(object.nVertex > 0 && !object.Vertex))
		return false;
	for(int index = 0; index < object.nVertex; ++index)
		object.Vertex[index].nx = object.Vertex[index].ny = object.Vertex[index].nz = 0;
	return true;
}

inline bool FaceUsesValidVertices(const smOBJ3D &object, const smFACE &face)
{
	return face.v[0] < object.nVertex && face.v[1] < object.nVertex && face.v[2] < object.nVertex;
}

inline bool AccumulateFaceNormal(smOBJ3D &object, const smFACE &face, std::vector<int> &normalCounts)
{
	if(!FaceUsesValidVertices(object, face))
		return false;

	smVERTEX &p1 = object.Vertex[face.v[0]];
	smVERTEX &p2 = object.Vertex[face.v[1]];
	smVERTEX &p3 = object.Vertex[face.v[2]];

	const double i = static_cast<double>(p2.y - p1.y) * (p3.z - p1.z) -
		static_cast<double>(p2.z - p1.z) * (p3.y - p1.y);
	const double j = static_cast<double>(p2.z - p1.z) * (p3.x - p1.x) -
		static_cast<double>(p2.x - p1.x) * (p3.z - p1.z);
	const double k = static_cast<double>(p2.x - p1.x) * (p3.y - p1.y) -
		static_cast<double>(p2.y - p1.y) * (p3.x - p1.x);
	const double length = std::sqrt(i * i + j * j + k * k);
	if(length <= 0.0 || !std::isfinite(length))
		return true;

	const int nx = static_cast<int>(i / length * 32767.0);
	const int ny = static_cast<int>(j / length * 32767.0);
	const int nz = static_cast<int>(k / length * 32767.0);

	const WORD indices[3] = {face.v[0], face.v[1], face.v[2]};
	for(int index = 0; index < 3; ++index)
	{
		smVERTEX &vertex = object.Vertex[indices[index]];
		vertex.nx += nx;
		vertex.ny += ny;
		vertex.nz += nz;
		++normalCounts[indices[index]];
	}
	return true;
}

inline bool BuildGeometryNormals(smOBJ3D &object)
{
	if(object.nVertex < 0 || object.nFace < 0 || (object.nVertex > 0 && !object.Vertex) ||
		(object.nFace > 0 && !object.Face))
		return false;

	std::vector<int> normalCounts(static_cast<std::size_t>(object.nVertex), 0);
	for(int index = 0; index < object.nVertex; ++index)
		object.Vertex[index].nx = object.Vertex[index].ny = object.Vertex[index].nz = 0;

	for(int index = 0; index < object.nFace; ++index)
	{
		if(!AccumulateFaceNormal(object, object.Face[index], normalCounts))
			return false;
	}

	for(int index = 0; index < object.nVertex; ++index)
	{
		if(!normalCounts[index])
			continue;
		smVERTEX &vertex = object.Vertex[index];
		vertex.nx = vertex.nx / normalCounts[index] + vertex.x;
		vertex.ny = vertex.ny / normalCounts[index] + vertex.y;
		vertex.nz = vertex.nz / normalCounts[index] + vertex.z;
	}
	return true;
}

} // namespace object_normals
} // namespace legacy3d
} // namespace rendering
} // namespace summit
