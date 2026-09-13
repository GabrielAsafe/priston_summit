#pragma once

#include <cstring>

namespace summit
{
namespace rendering
{
namespace legacy3d
{
namespace pattern_animation
{

inline bool HasValidObjectOrder(const smPAT3D &pattern)
{
	if(pattern.nObj3d < 0 || pattern.nObj3d > 128)
		return false;
	for(int index = 0; index < pattern.nObj3d; ++index)
	{
		const int objectIndex = static_cast<int>(pattern.TmSort[index]);
		if(objectIndex < 0 || objectIndex >= pattern.nObj3d || !pattern.obj3d[objectIndex])
			return false;
	}
	return true;
}

inline bool AnimationCacheMatches(const smPAT3D &pattern, int frame, int angleX, int angleY, int angleZ,
	const smPAT3D *talkPattern)
{
	return pattern.TmLastFrame == frame && pattern.TmLastAngle.x == angleX &&
		pattern.TmLastAngle.y == angleY && pattern.TmLastAngle.z == angleZ &&
		pattern.TmLastTalkPattern == talkPattern;
}

inline void CommitAnimationCache(smPAT3D &pattern, int frame, int angleX, int angleY, int angleZ,
	smPAT3D *talkPattern)
{
	pattern.TmLastFrame = frame;
	pattern.TmLastAngle.x = angleX;
	pattern.TmLastAngle.y = angleY;
	pattern.TmLastAngle.z = angleZ;
	pattern.TmLastTalkPattern = talkPattern;
}

inline bool Animate(smPAT3D &pattern, int frame, int angleX, int angleY, int angleZ,
	smPAT3D *talkPattern, SMotionStEndInfo *frameInfo)
{
	if(!HasValidObjectOrder(pattern))
		return false;
	if(AnimationCacheMatches(pattern, frame, angleX, angleY, angleZ, talkPattern))
		return true;

	for(int index = 0; index < pattern.nObj3d; ++index)
	{
		smOBJ3D *object = pattern.obj3d[pattern.TmSort[index]];
		const int result = talkPattern
			? object->TmAnimation(frame, angleX, angleY, angleZ, talkPattern, frameInfo)
			: object->TmAnimation(frame, angleX, angleY, angleZ, frameInfo);
		if(!result)
			return false;
	}
	CommitAnimationCache(pattern, frame, angleX, angleY, angleZ, talkPattern);
	return true;
}

inline bool AnimateTalk(smPAT3D &pattern, int frame)
{
	if(!HasValidObjectOrder(pattern))
		return false;
	if(pattern.TmLastTalkFrame == frame)
		return true;

	for(int index = 0; index < pattern.nObj3d; ++index)
	{
		smOBJ3D *object = pattern.obj3d[pattern.TmSort[index]];
		if(!object->TmTalkAnimation(frame))
			return false;
	}
	pattern.TmLastTalkFrame = frame;
	return true;
}

inline bool SyncWorldFromResolvedParents(smPAT3D &pattern)
{
	if(pattern.nObj3d < 0 || pattern.nObj3d > 128)
		return false;
	for(int index = 0; index < pattern.nObj3d; ++index)
	{
		smOBJ3D *object = pattern.obj3d[index];
		if(!object)
			return false;
		if(object->pParent)
			std::memcpy(&object->mWorld, &object->pParent->mWorld, sizeof(smMATRIX));
	}
	return true;
}

inline bool SetFrame(smPAT3D &pattern, int frame, smPAT3D *talkPattern, SMotionStEndInfo *frameInfo)
{
	if(frame < 0)
		return false;
	if(!talkPattern)
	{
		if(pattern.TmParent)
		{
			return Animate(*pattern.TmParent, frame, pattern.Angle.x, pattern.Angle.y, pattern.Angle.z, 0, frameInfo) &&
				SyncWorldFromResolvedParents(pattern);
		}
		return Animate(pattern, frame, pattern.Angle.x, pattern.Angle.y, pattern.Angle.z, 0, frameInfo);
	}

	if(talkPattern->Frame < 0 || !AnimateTalk(*talkPattern, talkPattern->Frame))
		return false;
	if(pattern.TmParent)
		return Animate(*pattern.TmParent, frame, pattern.Angle.x, pattern.Angle.y, pattern.Angle.z, talkPattern, frameInfo);
	return Animate(pattern, frame, pattern.Angle.x, pattern.Angle.y, pattern.Angle.z, talkPattern, frameInfo);
}

inline bool LinkExternalParent(smPAT3D &pattern, smPAT3D *parentPattern, smOBJ3D *parentObject)
{
	if(pattern.nObj3d < 0 || pattern.nObj3d > 128)
		return false;
	for(int index = 0; index < pattern.nObj3d; ++index)
	{
		if(!pattern.obj3d[index])
			return false;
	}
	pattern.TmParent = parentPattern;
	for(int index = 0; index < pattern.nObj3d; ++index)
		pattern.obj3d[index]->pParent = parentObject;
	return true;
}

} // namespace pattern_animation
} // namespace legacy3d
} // namespace rendering
} // namespace summit
