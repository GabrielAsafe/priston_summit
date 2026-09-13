#pragma once

#include <algorithm>

namespace summit
{
namespace rendering
{
namespace legacy3d
{
namespace pattern_lifecycle
{

static const int ObjectCapacity = 128;

inline void InitializeEmptyState(smPAT3D &pattern)
{
	for(int index = 0; index < ObjectCapacity; ++index)
	{
		pattern.obj3d[index] = 0;
		pattern.TmSort[index] = static_cast<BYTE>(index);
	}
	pattern.TmParent = 0;
	pattern.smMaterialGroup = 0;
	pattern.MaxFrame = 0;
	pattern.Frame = 0;
	pattern.SizeWidth = 0;
	pattern.SizeHeight = 0;
	pattern.nObj3d = 0;
	pattern.Posi.x = pattern.Posi.y = pattern.Posi.z = 0;
	pattern.Angle.x = pattern.Angle.y = pattern.Angle.z = 0;
	pattern.CameraPosi.x = pattern.CameraPosi.y = pattern.CameraPosi.z = 0;
	pattern.dBound = 0;
	pattern.Bound = 0;
	for(int index = 0; index < OBJ_FRAME_SEARCH_MAX; ++index)
	{
		pattern.TmFrame[index].StartFrame = 0;
		pattern.TmFrame[index].EndFrame = 0;
		pattern.TmFrame[index].PosNum = 0;
		pattern.TmFrame[index].PosCnt = 0;
	}
	pattern.TmFrameCnt = 0;
	pattern.TmLastFrame = -1;
	pattern.TmLastAngle.x = pattern.TmLastAngle.y = pattern.TmLastAngle.z = -1;
	pattern.TmLastTalkPattern = 0;
	pattern.TmLastTalkFrame = -1;
}

inline void ReleaseOwnedResources(smPAT3D &pattern)
{
	const int count = pattern.nObj3d < 0 ? 0 : std::min(pattern.nObj3d, ObjectCapacity);
	for(int index = count - 1; index >= 0; --index)
	{
		smOBJ3D *object = pattern.obj3d[index];
		if(!object)
			continue;
		// Guard against accidental duplicate ownership in a corrupted pattern.
		for(int duplicate = 0; duplicate < index; ++duplicate)
			if(pattern.obj3d[duplicate] == object)
				pattern.obj3d[duplicate] = 0;
		delete object;
		pattern.obj3d[index] = 0;
	}
	delete pattern.smMaterialGroup;
	pattern.smMaterialGroup = 0;
}

inline void Reset(smPAT3D &pattern)
{
	ReleaseOwnedResources(pattern);
	InitializeEmptyState(pattern);
}


inline void SwapRuntimeState(smPAT3D &left, smPAT3D &right)
{
	using std::swap;
	for(int index = 0; index < ObjectCapacity; ++index)
	{
		swap(left.obj3d[index], right.obj3d[index]);
		swap(left.TmSort[index], right.TmSort[index]);
	}
	swap(left.TmParent, right.TmParent);
	swap(left.smMaterialGroup, right.smMaterialGroup);
	swap(left.MaxFrame, right.MaxFrame);
	swap(left.Frame, right.Frame);
	swap(left.SizeWidth, right.SizeWidth);
	swap(left.SizeHeight, right.SizeHeight);
	swap(left.nObj3d, right.nObj3d);
	swap(left.Posi, right.Posi);
	swap(left.Angle, right.Angle);
	swap(left.CameraPosi, right.CameraPosi);
	swap(left.dBound, right.dBound);
	swap(left.Bound, right.Bound);
	for(int index = 0; index < OBJ_FRAME_SEARCH_MAX; ++index)
		swap(left.TmFrame[index], right.TmFrame[index]);
	swap(left.TmFrameCnt, right.TmFrameCnt);
	swap(left.TmLastFrame, right.TmLastFrame);
	swap(left.TmLastAngle, right.TmLastAngle);
	swap(left.TmLastTalkPattern, right.TmLastTalkPattern);
	swap(left.TmLastTalkFrame, right.TmLastTalkFrame);
}

inline bool ContainsObject(const smPAT3D &pattern, const smOBJ3D *object)
{
	if(!object || pattern.nObj3d < 0 || pattern.nObj3d > ObjectCapacity)
		return false;
	for(int index = 0; index < pattern.nObj3d; ++index)
		if(pattern.obj3d[index] == object)
			return true;
	return false;
}

inline int LastAnimationFrame(const smOBJ3D &object)
{
	int frame = 0;
	if(object.TmRotCnt > 0 && object.TmRot)
		frame = std::max(frame, object.TmRot[object.TmRotCnt - 1].frame);
	if(object.TmPosCnt > 0 && object.TmPos)
		frame = std::max(frame, object.TmPos[object.TmPosCnt - 1].frame);
	if(object.TmScaleCnt > 0 && object.TmScale)
		frame = std::max(frame, object.TmScale[object.TmScaleCnt - 1].frame);
	return frame;
}

inline bool AddOwnedObject(smPAT3D &pattern, smOBJ3D *object)
{
	if(!object || pattern.nObj3d < 0 || pattern.nObj3d >= ObjectCapacity || ContainsObject(pattern, object))
		return false;

	pattern.obj3d[pattern.nObj3d++] = object;
	pattern.MaxFrame = std::max(pattern.MaxFrame, LastAnimationFrame(*object));
	pattern.SizeWidth = std::max(pattern.SizeWidth, std::max(object->maxX, object->maxZ));
	pattern.SizeHeight = std::max(pattern.SizeHeight, object->maxY);
	if(pattern.Bound < object->Bound)
	{
		pattern.Bound = object->Bound;
		pattern.dBound = object->dBound;
	}
	return true;
}

} // namespace pattern_lifecycle
} // namespace legacy3d
} // namespace rendering
} // namespace summit
