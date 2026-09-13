#include "rendering/legacy_3d/stage/StageStorage.h"
#include "rendering/legacy_3d/stage/StageMeshBuilder.h"
#include "rendering/legacy_3d/stage/StageAreaBuilder.h"
#include "rendering/legacy_3d/stage/StageAreaQuery.h"
#include "rendering/legacy_3d/stage/StageHeightQuery.h"
#include "rendering/legacy_3d/stage/StageSurfaceQuery.h"
#include "rendering/legacy_3d/stage/StageMovementQuery.h"
#include "rendering/legacy_3d/stage/StageMovementProbe.h"
#include "rendering/legacy_3d/stage/StageRenderStamp.h"
#include "rendering/legacy_3d/stage/StageVisibleFaces.h"
#include "rendering/legacy_3d/stage/StageDrawBounds.h"
#include "rendering/legacy_3d/stage/StageLighting.h"
#include "rendering/legacy_3d/stage/StagePointLighting.h"
#include "rendering/legacy_3d/stage/StageDynamicLights.h"
#include "rendering/legacy_3d/stage/StageCollisionGeometry.h"
#include "rendering/legacy_3d/stage/LegacyStageSerialization.h"

int Stage_StepHeight = 10 * fONE;

int smStage_WaterChk;

static POINT3D ResolveLegacyStageMoveOffset(int x, int y, int z, const POINT3D &angle)
{
	GetMoveLocation(x, y, z, angle.x, angle.y, angle.z);
	POINT3D offset;
	offset.x = GeoResult_X;
	offset.y = GeoResult_Y;
	offset.z = GeoResult_Z;
	return offset;
}

class smCHAR;

extern smCHAR *CheckOtherPlayPosi(int x, int y, int z);

namespace
{

void SetLegacyStageCameraAngles(int angX, int angY, int angZ)
{
	CameraAngX = GetRadian(angX);
	CameraAngY = GetRadian(angY);
	CameraAngZ = GetRadian(angZ);
}

void PrepareLegacyStageView(int x, int y, int z, int angX, int angY, int angZ)
{
	CameraX = x >> FLOATNS;
	CameraY = (y >> FLOATNS) - 1500;
	CameraZ = z >> FLOATNS;
	if(CameraY < 400)
		CameraY = 400;

	SetLegacyStageCameraAngles(angX, angY, angZ);
	if(CameraAngX > ANGLE_90)
		CameraAngX = ANGLE_360 - CameraAngX;
	SetViewLength();
	SetViewRadian();
}

void SetLegacyStageRenderCamera(int x, int y, int z, int angX, int angY, int angZ,
	smEMATRIX *rotationMatrix)
{
	if(rotationMatrix)
		smRender.SetCameraPosi(x, y, z, rotationMatrix);
	else
		smRender.SetCameraPosi(x, y, z, angX, angY, angZ);
	SetLegacyStageCameraAngles(angX, angY, angZ);
}

} // anonymous namespace

smSTAGE3D::smSTAGE3D()
{
	summit::rendering::legacy3d::stage_storage::ResetStorageState(*this);
	VectLight.x = fONE;
	VectLight.y = -fONE;
	VectLight.z = fONE / 2;
	Bright = DEFAULT_BRIGHT;
	Contrast = DEFAULT_CONTRAST;
}


smSTAGE3D::smSTAGE3D(int nv, int nf)
{
	summit::rendering::legacy3d::stage_storage::ResetStorageState(*this);
	VectLight.x = fONE;
	VectLight.y = -fONE;
	VectLight.z = fONE / 2;
	Bright = DEFAULT_BRIGHT;
	Contrast = DEFAULT_CONTRAST;
	Init(nv, nf);
}


smSTAGE3D::~smSTAGE3D()
{
	Close();
}


int smSTAGE3D::GetPlaneProduct(smSTAGE_FACE *face, POINT3D *p)
{
	if(!face || !p)
		return 0;
	const smSTAGE_VERTEX *vertices[3] = { 0, 0, 0 };
	if(!summit::rendering::legacy3d::stage_geometry::FaceVertices(*this, *face, vertices))
		return 0;
	const std::int64_t value = summit::rendering::legacy3d::stage_collision::FacePlaneProduct(
		*face, *vertices[0], *p);
	if(value > std::numeric_limits<int>::max())
		return std::numeric_limits<int>::max();
	if(value < std::numeric_limits<int>::min())
		return std::numeric_limits<int>::min();
	return static_cast<int>(value);
};


int smSTAGE3D::GetThroughPlane(smSTAGE_FACE *face, POINT3D *sp, POINT3D *ep)
{
	if(!face || !sp || !ep)
		return FALSE;
	return summit::rendering::legacy3d::stage_collision::SegmentIntersectsFacePlane(
		*this, *face, *sp, *ep) ? TRUE : FALSE;
};


int smSTAGE3D::GetTriangleImact(smSTAGE_FACE *face, smLINE3D *pLines, int LineCnt)
{
	if(!face)
		return FALSE;
	return summit::rendering::legacy3d::stage_collision::AnyLineIntersectsFace(
		*this, *face, pLines, LineCnt) ? TRUE : FALSE;
}


int smSTAGE3D::CheckNextMove(POINT3D *Posi, POINT3D *Angle, POINT3D *MovePosi, int dist, int ObjWidth, int ObjHeight, int CheckOverLap)
{
	if(!Head || !Posi || !Angle || !MovePosi)
		return 0;

	smSTAGE_FACE *faces[summit::rendering::legacy3d::stage_area_query::LegacyFaceBufferCapacity] = { 0 };
	const int verticalRange = ObjHeight + fONE * 16384;
	const int faceCount = summit::rendering::legacy3d::stage_area_query::CollectFacesAroundPoint(
		*this, Posi->x, Posi->z, Posi->y + verticalRange, Posi->y - verticalRange,
		faces, summit::rendering::legacy3d::stage_area_query::LegacyFaceBufferCapacity);
	if(faceCount <= 0)
		return 0;

	POINT3D probeAngles[3] = { *Angle, *Angle, *Angle };
	probeAngles[1].y = (Angle->y - 768) & ANGCLIP;
	probeAngles[2].y = (Angle->y + 768) & ANGCLIP;

	smStage_WaterChk = CLIP_OUT;
	int groundHeight = CLIP_OUT;
	int waterHeight = CLIP_OUT;

	for(int probeIndex = 0; probeIndex < 3; ++probeIndex)
	{
		smLINE3D lines[summit::rendering::legacy3d::stage_movement_probe::ProbeLineCount];
		POINT3D endpoint;
		if(!summit::rendering::legacy3d::stage_movement_probe::BuildMovementProbe(
			*Posi, probeAngles[probeIndex], dist, ObjWidth, ObjHeight,
			lines, summit::rendering::legacy3d::stage_movement_probe::ProbeLineCount,
			endpoint, ResolveLegacyStageMoveOffset))
			return 0;

		const summit::rendering::legacy3d::stage_movement::ProbeEvaluation evaluation =
			summit::rendering::legacy3d::stage_movement::EvaluateProbe(
				*this, faces, faceCount, endpoint, Stage_StepHeight, ObjHeight,
				groundHeight, waterHeight,
				[](const smSTAGE_FACE &, const smMATERIAL &material) {
					return SummitStageMovementMaterialIsWalkable(material);
				},
				[&](smSTAGE_FACE &face) {
					return summit::rendering::legacy3d::stage_collision::AnyLineIntersectsFace(
						*this, face, lines,
						summit::rendering::legacy3d::stage_movement_probe::ProbeLineCount);
				});
		groundHeight = evaluation.groundHeight;
		waterHeight = evaluation.waterHeight;

		if(groundHeight != CLIP_OUT)
		{
			smStage_WaterChk = waterHeight;
			const bool overlapAllowed = !CheckOverLap ||
				(probeIndex > 0 && CheckOtherPlayPosi(endpoint.x, groundHeight, endpoint.z) == NULL);
			if(!evaluation.blocked && overlapAllowed)
			{
				MovePosi->x = endpoint.x;
				MovePosi->y = groundHeight;
				MovePosi->z = endpoint.z;
				return probeIndex + 1;
			}
		}

		if(probeIndex == 0)
			dist >>= 1;
	}
	return 0;
}

int smSTAGE3D::GetFloorHeight(int x, int y, int z, int ObjHeight)
{
	smSTAGE_FACE *faces[summit::rendering::legacy3d::stage_area_query::LegacyFaceBufferCapacity] = { 0 };
	const int verticalRange = ObjHeight + fONE * 16384;
	const int faceCount = summit::rendering::legacy3d::stage_area_query::CollectFacesAroundPoint(
		*this, x, z, y + verticalRange, y - verticalRange, faces, summit::rendering::legacy3d::stage_area_query::LegacyFaceBufferCapacity);
	return summit::rendering::legacy3d::stage_surface::HighestWalkableHeight(
		*this, faces, faceCount, x, y, z, 10 * fONE);
}


int smSTAGE3D::GetFloorHeight2(int x, int y, int z, int ObjHeight)
{
	smSTAGE_FACE *faces[summit::rendering::legacy3d::stage_area_query::LegacyFaceBufferCapacity] = { 0 };
	const int verticalRange = ObjHeight + fONE * 16384;
	const int faceCount = summit::rendering::legacy3d::stage_area_query::CollectFacesAroundPoint(
		*this, x, z, y + verticalRange, y - verticalRange, faces, summit::rendering::legacy3d::stage_area_query::LegacyFaceBufferCapacity);

	int height = CLIP_OUT;
	summit::rendering::legacy3d::stage_surface::VisitSolidFaces(
		*this, faces, faceCount, [&](smSTAGE_FACE &face) {
			const int candidate = summit::rendering::legacy3d::stage_height::FaceHeightAtXZ(
				*this, face, x, z);
			if(candidate != CLIP_OUT && candidate - y > 10 * fONE && height > candidate)
				height = candidate;
			return true;
		});
	return height;
}


int smSTAGE3D::GetEmptyHeight(int x, int y, int z, int ObjHeight)
{
	smSTAGE_FACE *faces[summit::rendering::legacy3d::stage_area_query::LegacyFaceBufferCapacity] = { 0 };
	const int verticalRange = ObjHeight + fONE * 16384;
	const int faceCount = summit::rendering::legacy3d::stage_area_query::CollectFacesAroundPoint(
		*this, x, z, y + verticalRange, y - verticalRange, faces, summit::rendering::legacy3d::stage_area_query::LegacyFaceBufferCapacity);
	const summit::rendering::legacy3d::stage_surface::EmptyHeightResult result =
		summit::rendering::legacy3d::stage_surface::QueryEmptyHeight(
			*this, faces, faceCount, x, y, z, ObjHeight);
	return result.occupied ? result.highest : 0;
}


int smSTAGE3D::CheckFloorFaceHeight(int x, int y, int z, int hSize)
{
	smSTAGE_FACE *faces[summit::rendering::legacy3d::stage_area_query::LegacyFaceBufferCapacity] = { 0 };
	const int verticalRange = fONE * 16384;
	const int faceCount = summit::rendering::legacy3d::stage_area_query::CollectFacesAroundPoint(
		*this, x, z, y + verticalRange, y - verticalRange, faces, summit::rendering::legacy3d::stage_area_query::LegacyFaceBufferCapacity);
	const int distance = summit::rendering::legacy3d::stage_surface::ClosestSolidSurfaceDistance(
		*this, faces, faceCount, x, y, z);
	if(distance != CLIP_OUT && distance <= hSize)
		return 0;
	return distance;
}


int smSTAGE3D::CheckVecImpact(POINT3D *Posi, POINT3D *Angle, int dist)
{
	if(!Head || !Posi || !Angle)
		return 0;

	smSTAGE_FACE *faces[summit::rendering::legacy3d::stage_area_query::LegacyFaceBufferCapacity] = { 0 };
	const int verticalRange = fONE * 64;
	const int faceCount = summit::rendering::legacy3d::stage_area_query::CollectFacesAroundPoint(
		*this, Posi->x, Posi->z, Posi->y + verticalRange, Posi->y - verticalRange,
		faces, summit::rendering::legacy3d::stage_area_query::LegacyFaceBufferCapacity);

	smLINE3D line;
	line.sp = *Posi;
	GetMoveLocation(0, 0, dist, Angle->x, Angle->y, Angle->z);
	line.ep.x = Posi->x + GeoResult_X;
	line.ep.y = Posi->y + GeoResult_Y;
	line.ep.z = Posi->z + GeoResult_Z;

	bool blocked = false;
	summit::rendering::legacy3d::stage_surface::VisitSolidFaces(
		*this, faces, faceCount, [&](smSTAGE_FACE &face) {
			if(GetTriangleImact(&face, &line, 1) == TRUE)
			{
				blocked = true;
				return false;
			}
			return true;
		});
	*Posi = line.ep;
	return blocked ? FALSE : TRUE;
}


int smSTAGE3D::CheckVecImpact2(int sx, int sy, int sz, int ex, int ey, int ez)
{
	if(!Head)
		return 0;

	smSTAGE_FACE *faces[summit::rendering::legacy3d::stage_area_query::LegacyFaceBufferCapacity] = { 0 };
	const int verticalRange = fONE * 128;
	const int faceCount = summit::rendering::legacy3d::stage_area_query::CollectFacesAroundPoint(
		*this, sx, sz, sy + verticalRange, sy - verticalRange, faces, summit::rendering::legacy3d::stage_area_query::LegacyFaceBufferCapacity);

	smLINE3D line;
	line.sp.x = sx;
	line.sp.y = sy;
	line.sp.z = sz;
	line.ep.x = ex;
	line.ep.y = ey;
	line.ep.z = ez;

	bool blocked = false;
	summit::rendering::legacy3d::stage_surface::VisitSolidFaces(
		*this, faces, faceCount, [&](smSTAGE_FACE &face) {
			if(GetTriangleImact(&face, &line, 1) == TRUE)
			{
				blocked = true;
				return false;
			}
			return true;
		});
	return blocked ? FALSE : TRUE;
}



void smSTAGE3D::Init(void)
{
	summit::rendering::legacy3d::stage_storage::ReleaseOwnedStorage(*this);
}


int smSTAGE3D::Init(int nv, int nf)
{
	return summit::rendering::legacy3d::stage_storage::InitializeOwnedStorage(*this, nv, nf)
		? TRUE : FALSE;
}


int smSTAGE3D::Close()
{
	summit::rendering::legacy3d::stage_storage::ReleaseOwnedStorage(*this);
	return TRUE;
}


int smSTAGE3D::AddVertex(int x, int y, int z)
{
	return summit::rendering::legacy3d::stage_mesh::AddVertex(*this, x, y, z);
}


int smSTAGE3D::AddFace(int a, int b, int c, int matrial)
{
	return summit::rendering::legacy3d::stage_mesh::AddFace(*this, a, b, c, matrial);
}


int smSTAGE3D::SetFaceMaterial(int FaceNum, int MatNum)
{
	return summit::rendering::legacy3d::stage_mesh::SetFaceMaterial(*this, FaceNum, MatNum)
		? TRUE : FALSE;
}


int smSTAGE3D::SetVertexColor(DWORD NumVertex, BYTE r, BYTE g, BYTE b, BYTE a)
{
	return summit::rendering::legacy3d::stage_mesh::SetVertexColor(*this, NumVertex, r, g, b, a)
		? TRUE : FALSE;
}


int smSTAGE3D::AddTexLink(int FaceNum, DWORD *hTex, smFTPOINT *t1, smFTPOINT *t2, smFTPOINT *t3)
{
	return summit::rendering::legacy3d::stage_mesh::AddTextureLink(
		*this, FaceNum, hTex, t1, t2, t3);
}




int smSTAGE3D::SetVertexShade(int isSetLight)
{
	return summit::rendering::legacy3d::stage_lighting::RebuildStageLighting(
		*this, isSetLight != FALSE) ? TRUE : FALSE;
}


int smSTAGE3D::AddVertexLightRound(POINT3D *LightPos, int r, int g, int b, int Range)
{
	if(!LightPos)
		return FALSE;
	return summit::rendering::legacy3d::stage_point_lighting::ApplyRadialVertexLight(
		*this, *LightPos, r, g, b, Range) ? TRUE : FALSE;
}


int	smSTAGE3D::InitDynLight(int nl)
{
	return summit::rendering::legacy3d::stage_dynamic_lights::Initialize(*this, nl)
		? TRUE : FALSE;
}


int smSTAGE3D::AddDynLight(int type, POINT3D *LightPos, int r, int g, int b, int Range)
{
	if(!LightPos)
		return FALSE;
	return summit::rendering::legacy3d::stage_dynamic_lights::Append(
		*this, type, *LightPos, r, g, b, Range) ? TRUE : FALSE;
}


int smSTAGE3D::CheckFaceIceFoot(POINT3D *Pos, POINT3D *Angle, int CheckHeight)
{
	if(!Pos || !Angle)
		return 0;

	smSTAGE_FACE *faces[summit::rendering::legacy3d::stage_area_query::LegacyFaceBufferCapacity] = { 0 };
	const int verticalRange = CheckHeight + fONE * 16384;
	const int faceCount = summit::rendering::legacy3d::stage_area_query::CollectFacesAroundPoint(
		*this, Pos->x, Pos->z, Pos->y + verticalRange, Pos->y - verticalRange,
		faces, summit::rendering::legacy3d::stage_area_query::LegacyFaceBufferCapacity);

	summit::rendering::legacy3d::stage_surface::SurfaceHit hit;
	if(!summit::rendering::legacy3d::stage_surface::FindHighestIceSurface(
		*this, faces, faceCount, Pos->x, Pos->y, Pos->z, 10 * fONE, hit))
		return 0;

	Angle->x = GetRadian2D(0, -1024, hit.face->VectNormal[2], hit.face->VectNormal[1]);
	Angle->z = GetRadian2D(0, -1024, hit.face->VectNormal[0], hit.face->VectNormal[1]);
	Pos->y = hit.height + fONE;
	return 1;
}







int smSTAGE3D::MakeAreaFaceList(int x, int z, int width, int height, int top, int bottom)
{
	smSTAGE_FACE *faces[summit::rendering::legacy3d::stage_area_query::LegacyFaceBufferCapacity] = { 0 };
	return summit::rendering::legacy3d::stage_area_query::CollectFaces(
		*this, x, z, width, height, top, bottom, faces, summit::rendering::legacy3d::stage_area_query::LegacyFaceBufferCapacity);
}


int smSTAGE3D::GetPolyHeight(smSTAGE_FACE *face, int x, int z)
{
	if(!face)
		return CLIP_OUT;
	return summit::rendering::legacy3d::stage_height::FaceHeightAtXZ(*this, *face, x, z);
}


int smSTAGE3D::GetAreaHeight(int ax, int az, int x, int z)
{
	return summit::rendering::legacy3d::stage_height::HighestHeightInCell(
		*this, ax, az, x, z,
		[this](const smSTAGE_FACE &face) {
			return summit::rendering::legacy3d::stage_surface::HasMaterialState(
				*this, face, SMMAT_STAT_CHECK_FACE);
		},
		-200 * fONE);
}


int smSTAGE3D::GetAreaHeight2(int ax, int az, int x, int z)
{
	return summit::rendering::legacy3d::stage_height::HighestHeightInCell(
		*this, ax, az, x, z,
		[](const smSTAGE_FACE &) { return true; },
		-200 * fONE);
}


int smSTAGE3D::GetHeight(int x, int z)
{
	if(!Head)
		return 0;
	return GetAreaHeight(x >> (6 + FLOATNS), z >> (6 + FLOATNS), x, z);
}


int smSTAGE3D::GetHeight2(int x, int z)
{
	if(!Head)
		return 0;
	return GetAreaHeight2(x >> (6 + FLOATNS), z >> (6 + FLOATNS), x, z);
}


int smSTAGE3D::CheckSolid(int sx, int sy, int sz, int dx, int dy, int dz)
{
	POINT3D start = { sx, sy, sz };
	POINT3D destination = { dx, dy, dz };
	const int token = summit::rendering::legacy3d::stage_query::BeginFaceQuery(*this);

	auto cellHasSolidFace = [&](int worldX, int worldZ) {
		bool hit = false;
		const int areaX = worldX >> (6 + FLOATNS);
		const int areaZ = worldZ >> (6 + FLOATNS);
		summit::rendering::legacy3d::stage_area_query::VisitUniqueFacesInCell(
			*this, areaX, areaZ, token, [&](smSTAGE_FACE &face) {
				if(!summit::rendering::legacy3d::stage_surface::HasMaterialState(
					*this, face, SMMAT_STAT_CHECK_FACE))
					return true;

				smLINE3D line;
				line.sp = start;
				line.ep = destination;
				if(summit::rendering::legacy3d::stage_collision::LineIntersectsFace(
					*this, face, line))
				{
					hit = true;
					return false;
				}
				return true;
			});
		return hit;
	};

	return (cellHasSolidFace(dx, dz) || cellHasSolidFace(sx, sz)) ? TRUE : FALSE;
}


void smSTAGE3D::clearStageArea()
{
	for(int y = 0; y < MAP_SIZE; y++)
	{
		for(int x = 0; x < MAP_SIZE; x++)
		{
			StageArea[x][y] = 0;
		}
	}
}


int smSTAGE3D::getPolyAreas(POINT3D *ip1, POINT3D *ip2, POINT3D *ip3)
{
	if(!ip1 || !ip2 || !ip3)
		return FALSE;
	if(!AreaList)
	{
		AreaList = new(std::nothrow) POINT[summit::rendering::legacy3d::stage_area::MaxCellsPerFace];
		if(!AreaList)
			return FALSE;
	}
	return summit::rendering::legacy3d::stage_area::CollectCoveredCells(
		*ip1, *ip2, *ip3, AreaList, summit::rendering::legacy3d::stage_area::MaxCellsPerFace,
		AreaListCnt) ? TRUE : FALSE;
}



int smSTAGE3D::SetupPolyAreas()
{
	return summit::rendering::legacy3d::stage_area::BuildAreaIndex(*this) ? TRUE : FALSE;
}


int smSTAGE3D::RenderGeom()
{
	if(!summit::rendering::legacy3d::stage_storage::HasConsistentRuntimeStorage(*this))
		return FALSE;
	if(smMaterialGroup)
		smRender.SetMaterialGroup(smMaterialGroup);

	smRender.InitStageMesh(Vertex, SumCount);
	const bool submitted = summit::rendering::legacy3d::stage_visible_faces::VisitVisibleFaces(
		*this, MapPosiTop, MapPosiBot, MapPosiLeft, MapPosiRight,
		[](smSTAGE_FACE &face) {
			smRender.AddStageFace(&face);
			return true;
		});
	if(!submitted)
		return FALSE;

	smRender.ClipRendFace();
	smRender.GeomVertex2D();
	return TRUE;
}

int smSTAGE3D::DrawStage(int x, int y, int z, int angX, int angY, int angZ, smEMATRIX *eRotMatrix)
{
	if(!Head)
		return FALSE;
	if(!summit::rendering::legacy3d::stage_render_stamp::BeginRenderPass(*this))
		return FALSE;
	if(!summit::rendering::legacy3d::stage_draw_bounds::InsideOuterBounds(*this, x, z))
		return FALSE;

	PrepareLegacyStageView(x, y, z, angX, angY, angZ);
	if(!MakeMapTable())
		return FALSE;
	SetLegacyStageRenderCamera(x, y, z, angX, angY, angZ, eRotMatrix);
	if(!RenderGeom())
		return FALSE;
	smRender.RenderD3D();
	if(StageObject)
		StageObject->Draw(x, y, z, angX, angY, angZ);

	// Preserve the legacy return contract: DrawStage historically returns FALSE even after drawing.
	return FALSE;
}

int smSTAGE3D::DrawStage2(int x, int y, int z, int angX, int angY, int angZ, smEMATRIX *eRotMatrix)
{
	if(!Head)
		return FALSE;
	if(!summit::rendering::legacy3d::stage_render_stamp::BeginRenderPass(*this))
		return FALSE;
	if(!summit::rendering::legacy3d::stage_draw_bounds::InsideSecondaryBounds(*this, x, z))
		return FALSE;

	PrepareLegacyStageView(x, y, z, angX, angY, angZ);
	if(!MakeMapTable())
		return FALSE;
	SetLegacyStageRenderCamera(x, y, z, angX, angY, angZ, eRotMatrix);
	if(!RenderGeom())
		return FALSE;
	smRender.RenderD3D();
	return TRUE;
}

int smSTAGE3D::DrawOpeningStage(int x, int y, int z, int angX, int angY, int angZ, int FrameStep)
{
	if(!Head)
		return FALSE;
	if(!summit::rendering::legacy3d::stage_render_stamp::BeginRenderPass(*this))
		return FALSE;
	if(!summit::rendering::legacy3d::stage_draw_bounds::InsideOuterBounds(*this, x, z))
		return FALSE;

	PrepareLegacyStageView(x, y, z, angX, angY, angZ);
	if(!MakeMapTable())
		return FALSE;
	SetLegacyStageRenderCamera(x, y, z, angX, angY, angZ, 0);
	if(!RenderGeom())
		return FALSE;
	smRender.RenderD3D();
	if(StageObject)
		StageObject->DrawOpening(x, y, z, angX, angY, angZ, FrameStep);

	// Preserve the legacy return contract.
	return FALSE;
}



static const char *szSMDFileHeader = "SMD Stage data Ver 0.72";

int smSTAGE3D::SaveFile(char *szFile)
{
	return summit::rendering::legacy3d::stage_serialization::SaveStage(
		*this, szFile, szSMDFileHeader) ? TRUE : FALSE;
}

int smSTAGE3D::LoadFile(char *szFile)
{
	return summit::rendering::legacy3d::stage_serialization::LoadStage(
		*this, szFile, szSMDFileHeader) ? TRUE : FALSE;
}
