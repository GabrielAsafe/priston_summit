int smPAT3D::RenderD3D(POINT3D *camera, POINT3D *angle, smPAT3D *TalkPattern, SMotionStEndInfo *lpFrameInfo)
{
	return summit::rendering::legacy3d::pattern_render::RenderAllObjects(
		*this, camera, angle, TalkPattern, lpFrameInfo);
}

int smPAT3D::ChoiseRenderD3D(POINT3D *camera, POINT3D *angle, char **szNodeList, int ListCnt, smPAT3D *TalkPattern, SMotionStEndInfo *lpFrameInfo)
{
	return summit::rendering::legacy3d::pattern_render::RenderSelectedObjects(
		*this, camera, angle, szNodeList, ListCnt, TalkPattern, lpFrameInfo);
}


int smPAT3D::RenderD3D(POINT3D *camera, POINT3D *angle, SMotionStEndInfo *lpFrameInfo)
{
	return summit::rendering::legacy3d::pattern_render::RenderAllObjects(
		*this, camera, angle, 0, lpFrameInfo);
}


int smPAT3D::ChoiseRenderD3D(POINT3D *camera, POINT3D *angle, char **szNodeList, int ListCnt, SMotionStEndInfo *lpFrameInfo)
{
	return summit::rendering::legacy3d::pattern_render::RenderSelectedObjects(
		*this, camera, angle, szNodeList, ListCnt, 0, lpFrameInfo);
}

int smPAT3D::RenderD3dOpening(POINT3D *camera, POINT3D *angle)
{
	return summit::rendering::legacy3d::pattern_render::RenderOpening(*this, camera, angle);
}


int smPAT3D::SetFrame(int frame, smPAT3D *TalkPattern, SMotionStEndInfo *lpFrameInfo)
{
	return summit::rendering::legacy3d::pattern_animation::SetFrame(
		*this, frame, TalkPattern, lpFrameInfo) ? TRUE : FALSE;
}

int smPAT3D::TmTalkAnimation(int frame)
{
	return summit::rendering::legacy3d::pattern_animation::AnimateTalk(*this, frame) ? TRUE : FALSE;
}

int smPAT3D::TmAnimation(int frame, int ax, int ay, int az, smPAT3D *TalkPattern, SMotionStEndInfo *lpFrameInfo)
{
	return summit::rendering::legacy3d::pattern_animation::Animate(
		*this, frame, ax, ay, az, TalkPattern, lpFrameInfo) ? TRUE : FALSE;
}


int smPAT3D::SetFrame(int frame, SMotionStEndInfo *lpFrameInfo)
{
	return summit::rendering::legacy3d::pattern_animation::SetFrame(
		*this, frame, 0, lpFrameInfo) ? TRUE : FALSE;
}


int smPAT3D::TmAnimation(int frame, int ax, int ay, int az, SMotionStEndInfo *lpFrameInfo)
{
	return summit::rendering::legacy3d::pattern_animation::Animate(
		*this, frame, ax, ay, az, 0, lpFrameInfo) ? TRUE : FALSE;
}


int smPAT3D::LinkParentObject(smPAT3D *patParent, smOBJ3D *objParent)
{
	return summit::rendering::legacy3d::pattern_animation::LinkExternalParent(
		*this, patParent, objParent) ? TRUE : FALSE;
}


static const char szSMDFileHeader[] = "SMD Model data Ver 0.62";



int smPAT3D::SaveFile(char *szFile)
{
	return summit::rendering::legacy3d::pattern_serialization::SavePattern(
		*this, szFile, szSMDFileHeader) ? TRUE : FALSE;
}


int smPAT3D::LoadFile(char *szFile, char *szNodeName)
{
	return summit::rendering::legacy3d::pattern_serialization::LoadPattern(
		*this, szFile, szNodeName, szSMDFileHeader, smASE_GetPhysique()) ? TRUE : FALSE;
}


int smFindPatFile(char *szfile, char *FileExt)
{
	char *fileName = smFindFile(szfile, FileExt);
	return fileName && summit::rendering::legacy3d::pattern_serialization::HasValidPatternHeader(
		fileName, szSMDFileHeader) ? TRUE : FALSE;
}
