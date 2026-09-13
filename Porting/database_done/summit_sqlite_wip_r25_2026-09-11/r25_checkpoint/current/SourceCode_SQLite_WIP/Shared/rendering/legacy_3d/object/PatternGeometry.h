#pragma once

namespace summit
{
namespace rendering
{
namespace legacy3d
{
namespace pattern_geometry
{

inline bool HasValidObjects(const smPAT3D &pattern)
{
	if(pattern.nObj3d < 0 || pattern.nObj3d > pattern_lifecycle::ObjectCapacity)
		return false;
	for(int index = 0; index < pattern.nObj3d; ++index)
		if(!pattern.obj3d[index])
			return false;
	return true;
}

inline bool ReformTransforms(smPAT3D &pattern)
{
	if(!HasValidObjects(pattern))
		return false;
	for(int index = 0; index < pattern.nObj3d; ++index)
	{
		smOBJ3D &object = *pattern.obj3d[index];
		if(!object_normals::BuildGeometryNormals(object) || !object_reform::ReformTransform(object))
			return false;
	}
	return true;
}

inline bool ReformPhysique(smPAT3D &pattern)
{
	if(!pattern.TmParent || !HasValidObjects(pattern))
		return false;
	for(int index = 0; index < pattern.nObj3d; ++index)
	{
		smOBJ3D &object = *pattern.obj3d[index];
		if(!object_normals::BuildGeometryNormals(object) || !object_reform::ReformPhysique(object))
			return false;
	}
	pattern.MaxFrame = pattern.TmParent->MaxFrame;
	return true;
}

inline bool ClearNormals(smPAT3D &pattern)
{
	if(!HasValidObjects(pattern))
		return false;
	for(int index = 0; index < pattern.nObj3d; ++index)
		pattern.obj3d[index]->ZeroNormals();
	return true;
}

inline bool SetPosition(smPAT3D &pattern, const POINT3D *point, const POINT3D *angle)
{
	if(!point || !angle || !HasValidObjects(pattern))
		return false;
	pattern.Posi = *point;
	pattern.Angle = *angle;
	for(int index = 0; index < pattern.nObj3d; ++index)
		pattern.obj3d[index]->SetPosi(point, angle);
	return true;
}

inline bool SetFixedPosition(smPAT3D &pattern)
{
	if(!HasValidObjects(pattern) || pattern.nObj3d == 0)
		return false;

	pattern.Posi.x = pattern.obj3d[0]->Tm._41;
	pattern.Posi.y = pattern.obj3d[0]->Tm._43;
	pattern.Posi.z = pattern.obj3d[0]->Tm._42;
	pattern.Angle.x = pattern.Angle.y = pattern.Angle.z = 0;

	POINT3D angle = { 0, 0, 0 };
	for(int index = 0; index < pattern.nObj3d; ++index)
	{
		POINT3D position = {
			pattern.obj3d[index]->Tm._41,
			pattern.obj3d[index]->Tm._43,
			pattern.obj3d[index]->Tm._42
		};
		pattern.obj3d[index]->SetPosi(&position, &angle);
	}
	return true;
}

} // namespace pattern_geometry
} // namespace legacy3d
} // namespace rendering
} // namespace summit
