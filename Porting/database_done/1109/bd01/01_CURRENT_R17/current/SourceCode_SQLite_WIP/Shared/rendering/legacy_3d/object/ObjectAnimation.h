#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "ObjectTrackSampling.h"

namespace summit
{
namespace rendering
{
namespace legacy3d
{
namespace object_animation
{

struct TrackSelection
{
	TrackSelection() : rotation(-1), position(-1), scale(-1) {}

	bool HasAny() const
	{
		return rotation >= 0 || position >= 0 || scale >= 0;
	}

	int rotation;
	int position;
	int scale;
};

inline bool TrackStorageIsValid(int count, const void *storage)
{
	return count >= 0 && (count == 0 || storage != 0);
}

inline bool FrameOffsetIsValid(int offset, int count)
{
	return offset < 0 || (offset >= 0 && offset < count);
}

inline bool AnimationStateIsValid(const smOBJ3D &object)
{
	return object.TmFrameCnt >= 0 && object.TmFrameCnt <= OBJ_FRAME_SEARCH_MAX &&
		TrackStorageIsValid(object.TmRotCnt, object.TmRot) &&
		TrackStorageIsValid(object.TmRotCnt, object.TmPrevRot) &&
		TrackStorageIsValid(object.TmPosCnt, object.TmPos) &&
		TrackStorageIsValid(object.TmScaleCnt, object.TmScale);
}

inline bool SelectionIsValid(const smOBJ3D &object, const TrackSelection &selection)
{
	return FrameOffsetIsValid(selection.rotation, object.TmRotCnt) &&
		FrameOffsetIsValid(selection.position, object.TmPosCnt) &&
		FrameOffsetIsValid(selection.scale, object.TmScaleCnt);
}

inline TrackSelection SelectTracks(smOBJ3D &object, int frame)
{
	TrackSelection selection;
	selection.rotation = object.GetTmFrameRot(frame);
	selection.position = object.GetTmFramePos(frame);
	selection.scale = object.GetTmFrameScale(frame);
	return selection;
}

inline void SetObjectTranslation(const smOBJ3D &object, smFMATRIX &matrix)
{
	matrix._41 = static_cast<float>(object.px) / fONE;
	matrix._42 = static_cast<float>(object.py) / fONE;
	matrix._43 = static_cast<float>(object.pz) / fONE;
}

inline bool ApplyFrameTableTransform(smOBJ3D &object, int frame, const TrackSelection &selection,
	SMotionStEndInfo *frameInfo, smFMATRIX &matrix)
{
	if(!SelectionIsValid(object, selection))
		return false;

	if(selection.rotation >= 0)
		object.GetRotFrame(matrix, &object.TmPrevRot[selection.rotation], &object.TmRot[selection.rotation], frame, frameInfo);
	else
		smFMatrixFromMatrix(matrix, object.TmRotate);

	if(selection.scale >= 0)
	{
		smMATRIX scaleMatrix;
		smFMATRIX floatScaleMatrix;
		smIdentityMatrix(scaleMatrix);
		object.GetScaleFrame(scaleMatrix._11, scaleMatrix._22, scaleMatrix._33,
			&object.TmScale[selection.scale], frame);
		smFMatrixFromMatrix(floatScaleMatrix, scaleMatrix);
		smFMatrixMult(matrix, matrix, floatScaleMatrix);
	}

	if(selection.position >= 0)
		object.GetPosFrame(matrix._41, matrix._42, matrix._43, &object.TmPos[selection.position], frame);
	else
		SetObjectTranslation(object, matrix);
	return true;
}


inline bool ApplyContinuousTransform(smOBJ3D &object, int frame, SMotionStEndInfo *frameInfo, smFMATRIX &matrix)
{
	if(!AnimationStateIsValid(object))
		return false;

	if(object.TmRotCnt > 0 && object.TmRot[object.TmRotCnt - 1].frame > frame)
		object.GetRotFrame(matrix, object.TmPrevRot, object.TmRot, frame, frameInfo);
	else
		smFMatrixFromMatrix(matrix, object.TmRotate);

	if(object.TmScaleCnt > 0 && object.TmScale[object.TmScaleCnt - 1].frame > frame)
	{
		smMATRIX scaleMatrix;
		smFMATRIX floatScaleMatrix;
		smIdentityMatrix(scaleMatrix);
		object.GetScaleFrame(scaleMatrix._11, scaleMatrix._22, scaleMatrix._33, object.TmScale, frame);
		smFMatrixFromMatrix(floatScaleMatrix, scaleMatrix);
		smFMatrixMult(matrix, matrix, floatScaleMatrix);
	}

	if(object.TmPosCnt > 0 && object.TmPos[object.TmPosCnt - 1].frame > frame)
		object.GetPosFrame(matrix._41, matrix._42, matrix._43, object.TmPos, frame);
	else
		SetObjectTranslation(object, matrix);
	return true;
}


inline void BuildStaticTransform(smOBJ3D &object, smFMATRIX &matrix)
{
	if(object.pParent)
	{
		smMATRIX relativeTransform;
		smMatrixMult(relativeTransform, object.Tm, object.pParent->TmInvert);
		smFMatrixFromMatrix(matrix, relativeTransform);
		return;
	}
	smFMatrixFromMatrix(matrix, object.Tm);
}

inline bool HasContinuousAnimation(const smOBJ3D &object)
{
	return object.TmRotCnt > 0 || object.TmPosCnt > 0 || object.TmScaleCnt > 0;
}

inline bool BuildObjectTransform(smOBJ3D &object, int frame, SMotionStEndInfo *frameInfo, smFMATRIX &matrix)
{
	if(!AnimationStateIsValid(object))
		return false;
	smFIdentityMatrix(matrix);
	if(object.TmFrameCnt > 0)
	{
		const TrackSelection selection = SelectTracks(object, frame);
		if(selection.HasAny())
			return ApplyFrameTableTransform(object, frame, selection, frameInfo, matrix);
	}
	else if(HasContinuousAnimation(object))
	{
		return ApplyContinuousTransform(object, frame, frameInfo, matrix);
	}

	BuildStaticTransform(object, matrix);
	return true;
}


inline smFMATRIX *FindTalkTransform(smPAT3D *talkPattern, const char *nodeName)
{
	if(!talkPattern || !nodeName || !nodeName[0] || talkPattern->nObj3d < 0 || talkPattern->nObj3d > 128)
		return 0;

	for(int index = 0; index < talkPattern->nObj3d; ++index)
	{
		smOBJ3D *object = talkPattern->obj3d[index];
		if(object && stricmp(object->NodeName, nodeName) == 0)
			return &object->TmResult;
	}
	return 0;
}

inline void ComposeWithParent(smOBJ3D &object, const smFMATRIX &localTransform)
{
	if(object.pParent)
	{
		smFMATRIX mutableLocal = localTransform;
		smFMatrixMult(object.TmResult, mutableLocal, object.pParent->TmResult);
	}
	else
		std::memcpy(&object.TmResult, &localTransform, sizeof(localTransform));
}

inline void BuildWorldTransform(smOBJ3D &object, int angleX, int angleY, int angleZ)
{
	if(angleX == 0 && angleY == 0 && angleZ == 0)
	{
		smIdentityMatrix(object.mLocal);
		smMatrixFromFMatrix(object.mWorld, object.TmResult);
		return;
	}

	smMATRIX rotateX;
	smMATRIX rotateY;
	smMATRIX rotateZ;
	smMATRIX transform;
	smIdentityMatrix(rotateX);
	smIdentityMatrix(rotateY);
	smIdentityMatrix(rotateZ);
	smRotateXMatrix(rotateX, angleX);
	smRotateZMatrix(rotateY, angleY);
	smRotateYMatrix(rotateZ, angleZ);

	if(smRender.m_dwRenderType)
	{
		smMatrixMult(object.mLocal, rotateY, rotateX);
		smMatrixMult(object.mLocal, object.mLocal, rotateZ);
	}
	else
	{
		smMatrixMult(object.mLocal, rotateZ, rotateX);
		smMatrixMult(object.mLocal, object.mLocal, rotateY);
	}

	smMatrixFromFMatrix(transform, object.TmResult);
	smMatrixMult(object.mWorld, transform, object.mLocal);
}

inline bool BuildTalkAnimation(smOBJ3D &object, int frame)
{
	if(!AnimationStateIsValid(object))
		return false;
	smFMATRIX transform;
	smFIdentityMatrix(transform);

	if(object.TmFrameCnt > 0)
	{
		const TrackSelection selection = SelectTracks(object, frame);
		if(selection.HasAny() && !ApplyFrameTableTransform(object, frame, selection, 0, transform))
			return false;
	}

	std::memcpy(&object.TmResult, &transform, sizeof(transform));
	return true;
}


inline bool AnimateObject(smOBJ3D &object, int frame, int angleX, int angleY, int angleZ,
	SMotionStEndInfo *frameInfo, smPAT3D *talkPattern)
{
	if(!AnimationStateIsValid(object))
		return false;
	smFMATRIX localTransform;
	smFMATRIX *talkTransform = FindTalkTransform(talkPattern, object.NodeName);
	if(talkTransform)
		std::memcpy(&localTransform, talkTransform, sizeof(localTransform));
	else if(!BuildObjectTransform(object, frame, frameInfo, localTransform))
		return false;

	ComposeWithParent(object, localTransform);
	BuildWorldTransform(object, angleX, angleY, angleZ);
	return true;
}


} // namespace object_animation
} // namespace legacy3d
} // namespace rendering
} // namespace summit
