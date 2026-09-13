#pragma once

#include <cmath>
#include <cstdlib>
#include <limits>

#include "ObjectStorage.h"

namespace summit
{
namespace rendering
{
namespace legacy3d
{
namespace object_mesh
{

inline bool HasVertexCapacity(const smOBJ3D &object)
{
	return object.Vertex && object.nVertex >= 0 && object.nVertex < object.MaxVertex;
}

inline bool IsVertexIndex(const smOBJ3D &object, int index)
{
	return index >= 0 && index < object.nVertex;
}

inline bool HasFaceCapacity(const smOBJ3D &object)
{
	return object.Face && object.nFace >= 0 && object.nFace < object.MaxFace;
}

inline void UpdateBounds(smOBJ3D &object, int x, int y, int z)
{
	const int absX = std::abs(x);
	const int absY = std::abs(y);
	const int absZ = std::abs(z);
	if(object.maxX < absX) object.maxX = absX;
	if(object.maxZ < absY) object.maxZ = absY;
	if(object.maxY < absZ) object.maxY = absZ;

	const long long scaledX = static_cast<long long>(x >> FLOATNS);
	const long long scaledY = static_cast<long long>(y >> FLOATNS);
	const long long scaledZ = static_cast<long long>(z >> FLOATNS);
	const long long squared = scaledX * scaledX + scaledY * scaledY + scaledZ * scaledZ;
	const int bound = squared > std::numeric_limits<int>::max()
		? std::numeric_limits<int>::max()
		: static_cast<int>(squared);
	if(object.dBound < bound)
		object.dBound = bound;
	if(object.nVertex == object.MaxVertex)
		object.Bound = static_cast<int>(std::sqrt(static_cast<long double>(object.dBound)));
}

inline int AddVertex(smOBJ3D &object, int x, int y, int z)
{
	if(!HasVertexCapacity(object))
		return -1;

	smVERTEX &vertex = object.Vertex[object.nVertex];
	vertex.x = x;
	vertex.y = y;
	vertex.z = z;
	const int addedVertex = object.nVertex++;
	UpdateBounds(object, x, y, z);
	return addedVertex;
}

inline int AddFace(smOBJ3D &object, int a, int b, int c)
{
	if(!HasFaceCapacity(object) || !IsVertexIndex(object, a) || !IsVertexIndex(object, b) || !IsVertexIndex(object, c))
		return -1;

	smFACE &face = object.Face[object.nFace];
	face.v[0] = static_cast<WORD>(a);
	face.v[1] = static_cast<WORD>(b);
	face.v[2] = static_cast<WORD>(c);
	face.v[3] = 0;
	face.lpTexLink = 0;
	return object.nFace++;
}

inline bool SetFaceMaterial(smOBJ3D &object, int faceIndex, int materialIndex)
{
	if(!object.Face || faceIndex < 0 || faceIndex >= object.nFace || materialIndex < 0 || materialIndex > 0xFFFF)
		return false;
	object.Face[faceIndex].v[3] = static_cast<WORD>(materialIndex);
	return true;
}

inline bool FindTextureTail(smOBJ3D &object, int faceIndex, smTEXLINK *&tail)
{
	tail = object.Face[faceIndex].lpTexLink;
	if(!tail)
		return true;

	int existingLinks = 1;
	while(tail->NextTex && existingLinks < object_storage::TextureLinksPerFaceCapacity)
	{
		tail = tail->NextTex;
		++existingLinks;
	}
	return !tail->NextTex && existingLinks < object_storage::TextureLinksPerFaceCapacity;
}

inline int AddTextureLink(smOBJ3D &object, int faceIndex, DWORD *texture,
	const smFTPOINT &t1, const smFTPOINT &t2, const smFTPOINT &t3)
{
	if(!object.TexLink || !object.Face || faceIndex < 0 || faceIndex >= object.nFace ||
		object.nTexLink < 0 || object.nTexLink >= object_storage::TextureLinkCapacity(object))
		return -1;

	smTEXLINK *tail = 0;
	if(!FindTextureTail(object, faceIndex, tail))
		return -1;

	smTEXLINK &link = object.TexLink[object.nTexLink];
	link.u[0] = t1.u; link.v[0] = t1.v;
	link.u[1] = t2.u; link.v[1] = t2.v;
	link.u[2] = t3.u; link.v[2] = t3.v;
	link.hTexture = texture;
	link.NextTex = 0;

	if(tail)
		tail->NextTex = &link;
	else
		object.Face[faceIndex].lpTexLink = &link;
	return object.nTexLink++;
}

inline smFTPOINT ToFloatTexturePoint(const smTPOINT &point)
{
	smFTPOINT result;
	result.u = static_cast<float>(point.u) / 256.0f;
	result.v = static_cast<float>(point.v) / 256.0f;
	return result;
}

inline int AddTextureLink(smOBJ3D &object, int faceIndex, DWORD *texture,
	const smTPOINT &t1, const smTPOINT &t2, const smTPOINT &t3)
{
	const smFTPOINT f1 = ToFloatTexturePoint(t1);
	const smFTPOINT f2 = ToFloatTexturePoint(t2);
	const smFTPOINT f3 = ToFloatTexturePoint(t3);
	return AddTextureLink(object, faceIndex, texture, f1, f2, f3);
}

inline int AddTexturedFace(smOBJ3D &object, int a, int b, int c,
	const smTPOINT &t1, const smTPOINT &t2, const smTPOINT &t3)
{
	const int face = AddFace(object, a, b, c);
	if(face < 0)
		return -1;
	if(AddTextureLink(object, face, 0, t1, t2, t3) < 0)
	{
		--object.nFace;
		return -1;
	}
	return face;
}

} // namespace object_mesh
} // namespace legacy3d
} // namespace rendering
} // namespace summit
