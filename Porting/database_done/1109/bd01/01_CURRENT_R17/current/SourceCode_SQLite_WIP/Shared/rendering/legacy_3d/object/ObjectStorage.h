#pragma once

#include <limits>
#include <memory>
#include <new>

namespace summit
{
namespace rendering
{
namespace legacy3d
{
namespace object_storage
{

static const int TextureLinksPerFaceCapacity = 8;

template <typename T>
inline void DeleteOwnedArray(T *&pointer)
{
	delete[] pointer;
	pointer = 0;
}

inline void ReleaseGeometryStorage(smOBJ3D &object)
{
	DeleteOwnedArray(object.WorldVertex);
	object.WorldVertexCapacity = 0;
	DeleteOwnedArray(object.TexLink);
	DeleteOwnedArray(object.Physique);
	DeleteOwnedArray(object.Face);
	DeleteOwnedArray(object.Vertex);
	object.MaxVertex = 0;
	object.MaxFace = 0;
	object.nVertex = 0;
	object.nFace = 0;
	object.nTexLink = 0;
}

inline void ReleaseAnimationStorage(smOBJ3D &object)
{
	DeleteOwnedArray(object.TmScale);
	DeleteOwnedArray(object.TmPos);
	DeleteOwnedArray(object.TmRot);
	DeleteOwnedArray(object.TmPrevRot);
	object.TmScaleCnt = 0;
	object.TmPosCnt = 0;
	object.TmRotCnt = 0;
	object.TmFrameCnt = 0;
}

inline void ReleaseOwnedStorage(smOBJ3D &object)
{
	ReleaseAnimationStorage(object);
	ReleaseGeometryStorage(object);
}

inline bool IsValidGeometryCapacity(int vertexCapacity, int faceCapacity)
{
	return vertexCapacity >= 0 && faceCapacity >= 0 &&
		faceCapacity <= (std::numeric_limits<int>::max() / TextureLinksPerFaceCapacity);
}

inline bool AllocateGeometryStorage(smOBJ3D &object, int vertexCapacity, int faceCapacity, bool withPhysique)
{
	if(!IsValidGeometryCapacity(vertexCapacity, faceCapacity))
		return false;

	std::unique_ptr<smVERTEX[]> vertices(vertexCapacity > 0 ? new(std::nothrow) smVERTEX[vertexCapacity] : 0);
	std::unique_ptr<smVERTEX[]> worldVertices(vertexCapacity > 0 ? new(std::nothrow) smVERTEX[vertexCapacity] : 0);
	std::unique_ptr<smFACE[]> faces(faceCapacity > 0 ? new(std::nothrow) smFACE[faceCapacity] : 0);
	const int textureCapacity = faceCapacity * TextureLinksPerFaceCapacity;
	std::unique_ptr<smTEXLINK[]> textureLinks(textureCapacity > 0 ? new(std::nothrow) smTEXLINK[textureCapacity] : 0);
	std::unique_ptr<smOBJ3D *[]> physique(withPhysique && vertexCapacity > 0
		? new(std::nothrow) smOBJ3D *[vertexCapacity] : 0);

	if((vertexCapacity > 0 && (!vertices || !worldVertices)) ||
		(faceCapacity > 0 && (!faces || !textureLinks)) ||
		(withPhysique && vertexCapacity > 0 && !physique))
		return false;

	if(physique)
		for(int index = 0; index < vertexCapacity; ++index)
			physique[index] = 0;

	ReleaseGeometryStorage(object);
	object.MaxVertex = vertexCapacity;
	object.MaxFace = faceCapacity;
	object.Vertex = vertices.release();
	object.WorldVertex = worldVertices.release();
	object.WorldVertexCapacity = vertexCapacity;
	object.Face = faces.release();
	object.TexLink = textureLinks.release();
	object.Physique = physique.release();
	return true;
}


inline int TextureLinkCapacity(const smOBJ3D &object)
{
	if(object.MaxFace <= 0 || object.MaxFace > (std::numeric_limits<int>::max() / TextureLinksPerFaceCapacity))
		return 0;
	return object.MaxFace * TextureLinksPerFaceCapacity;
}

} // namespace object_storage
} // namespace legacy3d
} // namespace rendering
} // namespace summit
