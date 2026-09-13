#pragma once

#include <cstring>

namespace summit
{
namespace rendering
{
namespace legacy3d
{
namespace stage_serialization
{

// Historical raw smSTAGE3D payload used by SMD files.
// Keep this layout frozen even when runtime-only fields are added to smSTAGE3D.
struct LegacyStageDiskLayout
{
	DWORD Head;
	DWORD *StageArea[MAP_SIZE][MAP_SIZE];
	POINT *AreaList;
	int AreaListCnt;
	int MemMode;
	DWORD SumCount;
	int CalcSumCount;
	smSTAGE_VERTEX *Vertex;
	smSTAGE_FACE *Face;
	smTEXLINK *TexLink;
	smLIGHT3D *smLight;
	smMATERIAL_GROUP *smMaterialGroup;
	smSTAGE_OBJECT *StageObject;
	smMATERIAL *smMaterial;
	int nVertex;
	int nFace;
	int nTexLink;
	int nLight;
	int nVertColor;
	int Contrast;
	int Bright;
	POINT3D VectLight;
	DWORD *lpwAreaBuff;
	int wAreaSize;
	RECT StageMapRect;
};

inline LegacyStageDiskLayout MakeLegacyStageDiskLayout(const smSTAGE3D &stage)
{
	LegacyStageDiskLayout disk;
	std::memset(&disk, 0, sizeof(disk));
	disk.Head = stage.Head;
	std::memcpy(disk.StageArea, stage.StageArea, sizeof(disk.StageArea));
	disk.AreaList = stage.AreaList;
	disk.AreaListCnt = stage.AreaListCnt;
	disk.MemMode = stage.MemMode;
	disk.SumCount = stage.SumCount;
	disk.CalcSumCount = stage.CalcSumCount;
	disk.Vertex = stage.Vertex;
	disk.Face = stage.Face;
	disk.TexLink = stage.TexLink;
	disk.smLight = stage.smLight;
	disk.smMaterialGroup = stage.smMaterialGroup;
	disk.StageObject = stage.StageObject;
	disk.smMaterial = stage.smMaterial;
	disk.nVertex = stage.nVertex;
	disk.nFace = stage.nFace;
	disk.nTexLink = stage.nTexLink;
	disk.nLight = stage.nLight;
	disk.nVertColor = stage.nVertColor;
	disk.Contrast = stage.Contrast;
	disk.Bright = stage.Bright;
	disk.VectLight = stage.VectLight;
	disk.lpwAreaBuff = stage.lpwAreaBuff;
	disk.wAreaSize = stage.wAreaSize;
	disk.StageMapRect = stage.StageMapRect;
	return disk;
}

} // namespace stage_serialization
} // namespace legacy3d
} // namespace rendering
} // namespace summit
