#pragma once

#include <cstdint>
#include <limits>

#include "ObjectViewMath.h"

int TestScreenView(int x, int y, int z, int Bound, const POINT3D *LocalAngle, const POINT3D *GlobalAngle);
extern int DEBUG_OBJSUM;

namespace summit
{
namespace rendering
{
namespace legacy3d
{
namespace pattern_render
{

static const DWORD MaterialTextureSlotCapacity = 8;

inline bool HasRenderInputs(const smPAT3D &pattern, const POINT3D *camera, const POINT3D *angle)
{
	return camera && angle && pattern.nObj3d >= 0 && pattern.nObj3d <= 128;
}

inline bool IsPatternVisible(smPAT3D &pattern, POINT3D *camera, POINT3D *angle)
{
	if(!HasRenderInputs(pattern, camera, angle))
		return false;
	int x = 0, y = 0, z = 0;
	if(!object_view::RelativeViewPosition(pattern.Posi, *camera, FLOATNS, x, y, z))
		return false;
	return TestScreenView(x, y, z, pattern.Bound, &pattern.Angle, angle) == TRUE;
}


inline bool PreparePatternFrame(smPAT3D &pattern, smPAT3D *talkPattern, SMotionStEndInfo *frameInfo)
{
	return talkPattern
		? pattern.SetFrame(pattern.Frame, talkPattern, frameInfo) != 0
		: pattern.SetFrame(pattern.Frame, frameInfo) != 0;
}

inline bool ReformObjectTextures(smPAT3D &pattern, smOBJ3D &object)
{
	smMATERIAL_GROUP *group = pattern.smMaterialGroup;
	if(!group || !group->smMaterial)
		return true;

	const std::int64_t extent64 = static_cast<std::int64_t>(pattern.Bound) *
		(static_cast<std::int64_t>(1) << (FLOATNS + 1));
	if(pattern.Bound <= 0 || extent64 > std::numeric_limits<int>::max())
		return false;

	POINT3D origin = { 0, 0, 0 };
	POINT3D size = { static_cast<int>(extent64), static_cast<int>(extent64), static_cast<int>(extent64) };
	for(DWORD materialIndex = 0; materialIndex < group->MaterialCount; ++materialIndex)
	{
		smMATERIAL &material = group->smMaterial[materialIndex];
		if(!material.ReformTexture)
			continue;
		const DWORD textureCount = material.TextureCounter < MaterialTextureSlotCapacity
			? material.TextureCounter : MaterialTextureSlotCapacity;
		for(DWORD textureIndex = 0; textureIndex < textureCount; ++textureIndex)
		{
			if(!material.TextureFormState[textureIndex])
				continue;
			if(object.ReformTexture(material.TextureFormState[textureIndex],
				static_cast<int>(materialIndex), static_cast<int>(textureIndex), &origin, &size) != TRUE)
				return false;
			break;
		}
	}
	return true;
}


inline int RenderAllObjects(smPAT3D &pattern, POINT3D *camera, POINT3D *angle,
	smPAT3D *talkPattern, SMotionStEndInfo *frameInfo)
{
	if(!IsPatternVisible(pattern, camera, angle))
		return 0;

	if(pattern.smMaterialGroup)
		smRender.SetMaterialGroup(pattern.smMaterialGroup);
	if(!PreparePatternFrame(pattern, talkPattern, frameInfo))
		return 0;

	int rendered = 0;
	const bool reformTextures = pattern.smMaterialGroup && pattern.smMaterialGroup->ReformTexture;
	for(int index = 0; index < pattern.nObj3d; ++index)
	{
		smOBJ3D *object = pattern.obj3d[index];
		if(!object)
			continue;
		if(reformTextures)
		{
			if(!object->TestRender(camera, angle))
				continue;
			if(!ReformObjectTextures(pattern, *object))
				continue;
			rendered += object->ActionRenderD3D();
		}
		else
		{
			rendered += object->RenderD3D(camera, angle);
		}
	}

	// Preserve the historical counter behavior: the reform-texture fast path
	// returned before incrementing DEBUG_OBJSUM.
	if(!reformTextures)
		++DEBUG_OBJSUM;
	return rendered;
}


inline int RenderOpening(smPAT3D &pattern, POINT3D *camera, POINT3D *angle)
{
	if(!HasRenderInputs(pattern, camera, angle) || !pattern.smMaterialGroup)
		return 0;

	smRender.SetMaterialGroup(pattern.smMaterialGroup);
	if(!PreparePatternFrame(pattern, 0, 0))
		return 0;

	int rendered = 0;
	for(int index = 0; index < pattern.nObj3d; ++index)
	{
		smOBJ3D *object = pattern.obj3d[index];
		if(object)
			rendered += object->RenderD3dOpening(camera, angle);
	}
	++DEBUG_OBJSUM;
	return rendered;
}

inline bool NodeIsSelected(const char *nodeName, char **nodeList, int listCount)
{
	if(!nodeName || !nodeList || listCount <= 0)
		return false;
	for(int index = 0; index < listCount; ++index)
	{
		if(nodeList[index] && lstrcmpi(nodeList[index], nodeName) == 0)
			return true;
	}
	return false;
}

inline int RenderSelectedObjects(smPAT3D &pattern, POINT3D *camera, POINT3D *angle,
	char **nodeList, int listCount, smPAT3D *talkPattern, SMotionStEndInfo *frameInfo)
{
	if(listCount < 0 || (listCount > 0 && !nodeList) || !IsPatternVisible(pattern, camera, angle))
		return 0;
	if(pattern.smMaterialGroup)
		smRender.SetMaterialGroup(pattern.smMaterialGroup);
	if(!PreparePatternFrame(pattern, talkPattern, frameInfo))
		return 0;

	int rendered = 0;
	for(int index = 0; index < pattern.nObj3d; ++index)
	{
		smOBJ3D *object = pattern.obj3d[index];
		if(object && NodeIsSelected(object->NodeName, nodeList, listCount))
			rendered += object->RenderD3D(camera, angle);
	}
	++DEBUG_OBJSUM;
	return rendered;
}

} // namespace pattern_render
} // namespace legacy3d
} // namespace rendering
} // namespace summit
