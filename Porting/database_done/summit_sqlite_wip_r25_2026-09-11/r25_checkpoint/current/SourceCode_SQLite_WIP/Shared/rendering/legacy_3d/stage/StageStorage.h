#pragma once

#include <climits>
#include <cstring>
#include <memory>
#include <limits>
#include <new>

namespace summit
{
namespace rendering
{
namespace legacy3d
{
namespace stage_storage
{

inline void ClearAreaIndex(smSTAGE3D &stage)
{
	std::memset(stage.StageArea, 0, sizeof(stage.StageArea));
	stage.AreaListCnt = 0;
}

inline void ResetStorageState(smSTAGE3D &stage)
{
	stage.Head = 0;
	stage.AreaList = 0;
	stage.MemMode = 0;
	stage.SumCount = 0;
	stage.CalcSumCount = 1;
	stage.Vertex = 0;
	stage.Face = 0;
	stage.TexLink = 0;
	stage.smLight = 0;
	stage.smMaterialGroup = 0;
	stage.StageObject = 0;
	stage.smMaterial = 0;
	stage.nVertex = 0;
	stage.VertexCapacity = 0;
	stage.nFace = 0;
	stage.FaceCapacity = 0;
	stage.nTexLink = 0;
	stage.TexLinkCapacity = 0;
	stage.nLight = 0;
	stage.LightCapacity = 0;
	stage.nVertColor = 0;
	stage.lpwAreaBuff = 0;
	stage.wAreaSize = 0;
	stage.StageMapRect.top = 0;
	stage.StageMapRect.bottom = 0;
	stage.StageMapRect.left = 0;
	stage.StageMapRect.right = 0;
	ClearAreaIndex(stage);
}

inline void ReleaseOwnedStorage(smSTAGE3D &stage)
{
	delete[] stage.AreaList;
	delete[] stage.smLight;
	delete[] stage.lpwAreaBuff;
	delete[] stage.TexLink;
	delete[] stage.Face;
	delete[] stage.Vertex;
	delete stage.StageObject;
	delete stage.smMaterialGroup;
	ResetStorageState(stage);
}

inline bool ValidateAllocationRequest(int vertexCapacity, int faceCapacity)
{
	const int maxVertexCount = static_cast<int>(std::numeric_limits<WORD>::max()) + 1;
	return vertexCapacity >= 0 && vertexCapacity <= maxVertexCount &&
		faceCapacity >= 0 && faceCapacity <= INT_MAX / 4;
}

inline bool HasConsistentRuntimeStorage(const smSTAGE3D &stage)
{
	return stage.nVertex >= 0 && stage.VertexCapacity >= stage.nVertex &&
		(stage.nVertex == 0 || stage.Vertex != 0) &&
		stage.nFace >= 0 && stage.FaceCapacity >= stage.nFace &&
		(stage.nFace == 0 || stage.Face != 0) &&
		stage.nTexLink >= 0 && stage.TexLinkCapacity >= stage.nTexLink &&
		(stage.nTexLink == 0 || stage.TexLink != 0) &&
		stage.nLight >= 0 && stage.LightCapacity >= stage.nLight &&
		(stage.nLight == 0 || stage.smLight != 0);
}

inline bool InitializeOwnedStorage(smSTAGE3D &stage, int vertexCapacity, int faceCapacity)
{
	if(!ValidateAllocationRequest(vertexCapacity, faceCapacity))
		return false;

	std::unique_ptr<smSTAGE_VERTEX[]> vertices;
	std::unique_ptr<smSTAGE_FACE[]> faces;
	std::unique_ptr<smTEXLINK[]> textureLinks;
	std::unique_ptr<smSTAGE_OBJECT> stageObject(new(std::nothrow) smSTAGE_OBJECT);
	if(!stageObject)
		return false;

	if(vertexCapacity > 0)
	{
		vertices.reset(new(std::nothrow) smSTAGE_VERTEX[static_cast<std::size_t>(vertexCapacity)]);
		if(!vertices)
			return false;
	}
	if(faceCapacity > 0)
	{
		faces.reset(new(std::nothrow) smSTAGE_FACE[static_cast<std::size_t>(faceCapacity)]);
		textureLinks.reset(new(std::nothrow) smTEXLINK[static_cast<std::size_t>(faceCapacity) * 4]);
		if(!faces || !textureLinks)
			return false;
	}

	ReleaseOwnedStorage(stage);
	stage.Vertex = vertices.release();
	stage.Face = faces.release();
	stage.TexLink = textureLinks.release();
	stage.StageObject = stageObject.release();
	stage.VertexCapacity = vertexCapacity;
	stage.FaceCapacity = faceCapacity;
	stage.TexLinkCapacity = faceCapacity * 4;
	return true;
}

} // namespace stage_storage
} // namespace legacy3d
} // namespace rendering
} // namespace summit
