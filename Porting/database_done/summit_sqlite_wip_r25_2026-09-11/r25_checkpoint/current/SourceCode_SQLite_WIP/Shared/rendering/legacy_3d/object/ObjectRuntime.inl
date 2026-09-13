int smOBJ3D::TestRotate(int x, int y, int z, const POINT3D *LocalAngle, const POINT3D *GlobalAngle)
{
#if SUMMIT_LEGACY_OBJECT_VISIBILITY
	(void)LocalAngle;
	if(!GlobalAngle)
		return FALSE;

	const summit::rendering::legacy3d::object_visibility::Viewport viewport = {
		MidX, MidY, viewdistZ, ViewPointLen, smScreenWidth, smScreenHeight
	};
	return summit::rendering::legacy3d::object_visibility::ProjectSphere(
		x, y, z, Bound, GlobalAngle->x, GlobalAngle->y, GlobalAngle->z,
		GetSin, GetCos, ANGCLIP, FLOATDS, FLOATNS, viewport) ? TRUE : FALSE;
#else
	(void)x; (void)y; (void)z; (void)LocalAngle; (void)GlobalAngle;
	return FALSE;
#endif
}



int smOBJ3D::BeginRender()
{
	return summit::rendering::legacy3d::object_render::SubmitGeometry(*this, Head) ? TRUE : FALSE;
}



int smOBJ3D::GetRotFrame(smFMATRIX &gmat, smFMATRIX *PrevRot, smTM_ROT *tmRot, int frame, SMotionStEndInfo *lpFrameInfo)
{
	return summit::rendering::legacy3d::object_track::SampleRotation(*this, gmat, PrevRot, tmRot, frame, lpFrameInfo);
}



int smOBJ3D::GetPosFrame(float &x, float &y, float &z, smTM_POS *tmPos, int frame)
{
	return summit::rendering::legacy3d::object_track::SamplePosition(*this, x, y, z, tmPos, frame);
}


int smOBJ3D::GetScaleFrame(int &x, int &y, int &z, smTM_SCALE *tmScale, int frame)
{
	return summit::rendering::legacy3d::object_track::SampleScale(*this, x, y, z, tmScale, frame);
}


int smOBJ3D::GetTmFramePos(int frame)
{
	return summit::rendering::legacy3d::object_track::FindFrameTrackOffset(TmPosFrame, TmFrameCnt, frame);
}


int smOBJ3D::GetTmFrameScale(int frame)
{
	return summit::rendering::legacy3d::object_track::FindFrameTrackOffset(TmScaleFrame, TmFrameCnt, frame);
}


int smOBJ3D::GetTmFrameRot(int frame)
{
	return summit::rendering::legacy3d::object_track::FindFrameTrackOffset(TmRotFrame, TmFrameCnt, frame);
}


int smOBJ3D::TmTalkAnimation(int frame)
{
	return summit::rendering::legacy3d::object_animation::BuildTalkAnimation(*this, frame) ? TRUE : FALSE;
}


int smOBJ3D::TmAnimation(int frame, int ax, int ay, int az, smPAT3D *TalkPattern, SMotionStEndInfo *lpFrameInfo)
{
	return summit::rendering::legacy3d::object_animation::AnimateObject(
		*this, frame, ax, ay, az, lpFrameInfo, TalkPattern) ? TRUE : FALSE;
}


int smOBJ3D::TmAnimation(int frame, int ax, int ay, int az, SMotionStEndInfo *lpFrameInfo)
{
	return summit::rendering::legacy3d::object_animation::AnimateObject(
		*this, frame, ax, ay, az, lpFrameInfo, 0) ? TRUE : FALSE;
}


int smOBJ3D::WorldForm()
{
	const bool useWorldNormalMatrix = (Head & OBJ_HEAD_TYPE_NEW_NORMAL) != 0;
	return summit::rendering::legacy3d::object_world::BuildWorldVertices(
		*this, useWorldNormalMatrix, WorldVertex, WorldVertexCapacity) ? TRUE : FALSE;
}


void smOBJ3D::SetPosi(const POINT3D *point, const POINT3D *angle)
{
	if(!point || !angle)
		return;
	Posi = *point;
	Angle = *angle;
}


int smOBJ3D::TestRender(POINT3D *camera, POINT3D *angle)
{
	return summit::rendering::legacy3d::object_render::PrepareObject(
		*this, camera, angle, smObjFixPos != 0, true) ? TRUE : FALSE;
}


int smOBJ3D::ActionRenderD3D()
{
	return summit::rendering::legacy3d::object_render::ExecuteRender(*this, Head) ? TRUE : FALSE;
}


int smOBJ3D::RenderD3D(POINT3D *camera, POINT3D *angle)
{
	if(!summit::rendering::legacy3d::object_render::PrepareObject(
		*this, camera, angle, smObjFixPos != 0, true))
		return FALSE;
	return ActionRenderD3D();
}


int smOBJ3D::RenderD3dOpening(POINT3D *camera, POINT3D *angle)
{
	if(!summit::rendering::legacy3d::object_render::PrepareObject(
		*this, camera, angle, smObjFixPos != 0, false))
		return FALSE;
	return ActionRenderD3D();
}


int smOBJ3D::GetSaveSize()
{
	return summit::rendering::legacy3d::object_serialization::SerializedObjectSize(*this);
}


int smOBJ3D::SaveFile(HANDLE hFile)
{
	Head = 0x41424344;
	Head |= OBJ_HEAD_TYPE_NEW_NORMAL;
	return summit::rendering::legacy3d::object_serialization::SaveObject(hFile, *this, Head);
}


int smOBJ3D::LoadFile(HANDLE hFile, smPAT3D *PatPhysique)
{
	DWORD loadedHead = 0;
	const int bytes = summit::rendering::legacy3d::object_serialization::LoadObject(
		hFile, *this, PatPhysique, &loadedHead);
	if(bytes > 0)
		Head = loadedHead;
	return bytes;
}
