#pragma once

#include "ObjectViewMath.h"

namespace summit
{
namespace rendering
{
namespace legacy3d
{
namespace object_render
{

inline bool PrepareObject(smOBJ3D &object, const POINT3D *camera, const POINT3D *angle,
	bool fixedObjectPosition, bool requireVisibility)
{
	if(!camera || !angle)
		return false;

	if(requireVisibility)
	{
		int x = 0;
		int y = 0;
		int z = 0;
		if(!object_view::RelativeViewPosition(object.Posi, *camera, FLOATNS, x, y, z) ||
			object.TestRotate(x, y, z, &object.Angle, angle) != TRUE)
			return false;
	}

	POINT3D resolvedCamera = {};
	if(!object_view::ResolveCameraPosition(object.Posi, *camera, fixedObjectPosition, resolvedCamera))
		return false;
	object.CameraPosi = resolvedCamera;

	if(object.WorldForm() != TRUE)
		return false;

	smRender.SetCameraPosi(object.CameraPosi.x, object.CameraPosi.y, object.CameraPosi.z,
		angle->x, angle->y, angle->z);
	return true;
}

inline bool SubmitGeometry(smOBJ3D &object, DWORD head)
{
	if(object.nVertex < 0 || object.nVertex > object.WorldVertexCapacity ||
		(object.nVertex > 0 && !object.WorldVertex) ||
		object.nFace < 0 || object.nFace > object.MaxFace ||
		(object.nFace > 0 && !object.Face))
		return false;

	for(int index = 0; index < object.nVertex; ++index)
		smRender.AddObjectVertex(&object.WorldVertex[index], head);
	for(int index = 0; index < object.nFace; ++index)
		smRender.AddObjectFace(&object.Face[index]);
	return true;
}

inline bool ExecuteRender(smOBJ3D &object, DWORD head)
{
	smRender.Init();
	if(!SubmitGeometry(object, head))
		return false;
	smRender.SetClipStates(object.ClipStates);
	smRender.ClipRendFace();
	smRender.GeomVertex2D(object.ColorEffect);
	smRender.RenderD3D();
	return true;
}

} // namespace object_render
} // namespace legacy3d
} // namespace rendering
} // namespace summit
