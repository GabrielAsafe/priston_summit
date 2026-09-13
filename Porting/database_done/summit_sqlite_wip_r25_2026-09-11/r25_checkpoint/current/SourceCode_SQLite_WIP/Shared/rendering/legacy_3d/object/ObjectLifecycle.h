#pragma once

namespace summit
{
namespace rendering
{
namespace legacy3d
{
namespace object_lifecycle
{

inline void ClearFrameTable(smFRAME_POS *frames, int capacity)
{
	if(!frames || capacity <= 0)
		return;

	for(int index = 0; index < capacity; ++index)
	{
		frames[index].StartFrame = 0;
		frames[index].EndFrame = 0;
		frames[index].PosNum = 0;
		frames[index].PosCnt = 0;
	}
}

inline void ClearVertex(smVERTEX &vertex)
{
	vertex.x = vertex.y = vertex.z = 0;
	vertex.nx = vertex.ny = vertex.nz = 0;
}

inline void InitializeRuntimeState(smOBJ3D &object)
{
	object.Vertex = 0;
	object.Face = 0;
	object.TexLink = 0;
	object.Physique = 0;
	object.WorldVertex = 0;
	object.WorldVertexCapacity = 0;
	ClearVertex(object.ZeroVertex);

	object.maxZ = object.minZ = 0;
	object.maxY = object.minY = 0;
	object.maxX = object.minX = 0;
	object.dBound = 0;
	object.Bound = 0;
	object.MaxVertex = 0;
	object.MaxFace = 0;
	object.nVertex = 0;
	object.nFace = 0;
	object.nTexLink = 0;
	object.ColorEffect = TRUE;
	object.ClipStates = (SMCLIP_NEARZ | SMCLIP_LEFT | SMCLIP_RIGHT | SMCLIP_TOP | SMCLIP_BOTTOM);
	object.Posi.x = object.Posi.y = object.Posi.z = 0;
	object.CameraPosi.x = object.CameraPosi.y = object.CameraPosi.z = 0;
	object.Angle.x = object.Angle.y = object.Angle.z = 0;

	object.NodeName[0] = 0;
	object.NodeParent[0] = 0;
	object.pParent = 0;

	smIdentityMatrix(object.Tm);
	smIdentityMatrix(object.TmInvert);
	smFIdentityMatrix(object.TmResult);
	smIdentityMatrix(object.TmRotate);
	smIdentityMatrix(object.mWorld);
	smIdentityMatrix(object.mLocal);
	object.lFrame = 0;
	object.qx = object.qy = object.qz = 0.0f;
	object.qw = 1.0f;
	object.sx = object.sy = object.sz = fONE;
	object.px = object.py = object.pz = 0;

	object.TmRot = 0;
	object.TmPos = 0;
	object.TmScale = 0;
	object.TmPrevRot = 0;
	object.TmRotCnt = 0;
	object.TmPosCnt = 0;
	object.TmScaleCnt = 0;
	ClearFrameTable(object.TmRotFrame, OBJ_FRAME_SEARCH_MAX);
	ClearFrameTable(object.TmPosFrame, OBJ_FRAME_SEARCH_MAX);
	ClearFrameTable(object.TmScaleFrame, OBJ_FRAME_SEARCH_MAX);
	object.TmFrameCnt = 0;
}

inline void ResetGeometryMetadata(smOBJ3D &object)
{
	object.dBound = 0;
	object.Bound = 0;
	object.maxX = object.maxY = object.maxZ = 0;
	object.minX = object.minY = object.minZ = 0;
	object.ColorEffect = TRUE;
	object.ClipStates = (SMCLIP_NEARZ | SMCLIP_LEFT | SMCLIP_RIGHT | SMCLIP_TOP | SMCLIP_BOTTOM);
	object.Posi.x = object.Posi.y = object.Posi.z = 0;
	object.CameraPosi.x = object.CameraPosi.y = object.CameraPosi.z = 0;
	object.Angle.x = object.Angle.y = object.Angle.z = 0;
	ClearVertex(object.ZeroVertex);
	smIdentityMatrix(object.mLocal);
}

} // namespace object_lifecycle
} // namespace legacy3d
} // namespace rendering
} // namespace summit
