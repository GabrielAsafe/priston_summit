smPAT3D::smPAT3D()
{
	Head = 0;
	summit::rendering::legacy3d::pattern_lifecycle::InitializeEmptyState(*this);
}

smPAT3D::~smPAT3D()
{
	summit::rendering::legacy3d::pattern_lifecycle::ReleaseOwnedResources(*this);
}

void smPAT3D::Init()
{
	summit::rendering::legacy3d::pattern_lifecycle::Reset(*this);
}

void smPAT3D::Close()
{
	summit::rendering::legacy3d::pattern_lifecycle::Reset(*this);
}

int smPAT3D::AddObject(smOBJ3D *obj)
{
	return summit::rendering::legacy3d::pattern_lifecycle::AddOwnedObject(*this, obj) ? TRUE : FALSE;
}

int smPAT3D::ReformTM()
{
	return summit::rendering::legacy3d::pattern_geometry::ReformTransforms(*this) ? TRUE : FALSE;
}

int smPAT3D::ReformPhysique()
{
	return summit::rendering::legacy3d::pattern_geometry::ReformPhysique(*this) ? TRUE : FALSE;
}

int smPAT3D::LinkObject()
{
	return summit::rendering::legacy3d::pattern_hierarchy::LinkPattern(
		obj3d, nObj3d, TmSort, summit::rendering::legacy3d::pattern_lifecycle::ObjectCapacity) ? TRUE : FALSE;
}

smOBJ3D *smPAT3D::GetObjectFromName(char *name)
{
	const int index = summit::rendering::legacy3d::pattern_hierarchy::FindObjectIndexByName(obj3d, nObj3d, name);
	return index >= 0 ? obj3d[index] : 0;
}

void smPAT3D::ZeroNormals()
{
	summit::rendering::legacy3d::pattern_geometry::ClearNormals(*this);
}

void smPAT3D::SetPosi(const POINT3D *point, const POINT3D *angle)
{
	summit::rendering::legacy3d::pattern_geometry::SetPosition(*this, point, angle);
}

void smPAT3D::SetFixPosi()
{
	summit::rendering::legacy3d::pattern_geometry::SetFixedPosition(*this);
}
