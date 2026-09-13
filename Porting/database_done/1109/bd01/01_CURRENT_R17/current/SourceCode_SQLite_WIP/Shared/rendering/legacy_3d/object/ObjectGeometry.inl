smOBJ3D::smOBJ3D()
{
	Head = 0;
	summit::rendering::legacy3d::object_lifecycle::InitializeRuntimeState(*this);
}


smOBJ3D::~smOBJ3D()
{
	summit::rendering::legacy3d::object_storage::ReleaseOwnedStorage(*this);
}

smOBJ3D::smOBJ3D(int nv, int nf) : smOBJ3D()
{
	Init(nv, nf, 0);
}




void smOBJ3D::Init(int nv, int nf, int pFlag)
{
	using namespace summit::rendering::legacy3d::object_storage;
	if(!AllocateGeometryStorage(*this, nv, nf, pFlag != 0))
		return;

	summit::rendering::legacy3d::object_lifecycle::ResetGeometryMetadata(*this);
}


int smOBJ3D::AddVertex(int x, int y, int z)
{
	return summit::rendering::legacy3d::object_mesh::AddVertex(*this, x, y, z);
}


int smOBJ3D::AddFace(int a, int b, int c)
{
	return summit::rendering::legacy3d::object_mesh::AddFace(*this, a, b, c);
}


int smOBJ3D::SetFaceMaterial(int FaceNum, int MatNum)
{
	return summit::rendering::legacy3d::object_mesh::SetFaceMaterial(*this, FaceNum, MatNum) ? TRUE : FALSE;
}


int smOBJ3D::AddTexLink(int FaceNum, DWORD *hTex, smFTPOINT *t1, smFTPOINT *t2, smFTPOINT *t3)
{
	if(!t1 || !t2 || !t3)
		return -1;
	return summit::rendering::legacy3d::object_mesh::AddTextureLink(*this, FaceNum, hTex, *t1, *t2, *t3);
}


int smOBJ3D::AddTexLinkTP(int FaceNum, DWORD *hTex, smTPOINT *t1, smTPOINT *t2, smTPOINT *t3)
{
	if(!t1 || !t2 || !t3)
		return -1;
	return summit::rendering::legacy3d::object_mesh::AddTextureLink(*this, FaceNum, hTex, *t1, *t2, *t3);
}


int smOBJ3D::AddFace(int a, int b, int c, smTPOINT *t1, smTPOINT *t2, smTPOINT *t3)
{
	if(!t1 || !t2 || !t3)
		return -1;
	return summit::rendering::legacy3d::object_mesh::AddTexturedFace(*this, a, b, c, *t1, *t2, *t3);
}


int smOBJ3D::ReformTexture(int Type, int MatNum, int MatCnt, POINT3D *pos, POINT3D *size)
{
	if(!pos || !size)
		return FALSE;
	return summit::rendering::legacy3d::object_texture::ReformTexture(
		*this, Type, MatNum, MatCnt, *pos, *size, WorldVertex, WorldVertexCapacity) ? TRUE : FALSE;
}


void smOBJ3D::ZeroNormals()
{
	if(summit::rendering::legacy3d::object_normals::ClearGeometryNormals(*this))
		Head &= (~OBJ_HEAD_TYPE_NEW_NORMAL);
}



void smOBJ3D::SetGNormals()
{
	summit::rendering::legacy3d::object_normals::BuildGeometryNormals(*this);
}


void smOBJ3D::SortMatrial()
{
	summit::rendering::legacy3d::object_compaction::SortFacesByMaterial(*this);
}


void smOBJ3D::SortVertex()
{
	summit::rendering::legacy3d::object_compaction::CompactVertices(*this);
}


void smOBJ3D::ReformTM()
{
	summit::rendering::legacy3d::object_reform::ReformTransform(*this);
}


void smOBJ3D::ReformPhysique()
{
	summit::rendering::legacy3d::object_reform::ReformPhysique(*this);
}
