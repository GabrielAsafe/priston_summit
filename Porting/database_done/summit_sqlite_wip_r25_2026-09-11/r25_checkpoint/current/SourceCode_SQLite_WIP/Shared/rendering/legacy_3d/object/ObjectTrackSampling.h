#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace summit
{
namespace rendering
{
namespace legacy3d
{
namespace object_track
{

template <typename Track>
inline int TrackOffset(const Track *allKeys, int totalKeys, const Track *firstKey)
{
	if(!allKeys || !firstKey || totalKeys <= 0)
		return -1;

	const std::uintptr_t begin = reinterpret_cast<std::uintptr_t>(allKeys);
	const std::uintptr_t current = reinterpret_cast<std::uintptr_t>(firstKey);
	const std::uintptr_t end = begin + (sizeof(Track) * static_cast<std::uintptr_t>(totalKeys));
	if(current < begin || current >= end)
		return -1;

	const std::uintptr_t byteOffset = current - begin;
	if((byteOffset % sizeof(Track)) != 0)
		return -1;
	return static_cast<int>(byteOffset / sizeof(Track));
}

template <typename Track>
inline int RemainingTrackKeys(const Track *allKeys, int totalKeys, const Track *firstKey)
{
	const int offset = TrackOffset(allKeys, totalKeys, firstKey);
	return offset >= 0 ? totalKeys - offset : 0;
}

inline int LimitTrackKeysToFrame(const smFRAME_POS *frames, int frameCount, int trackOffset, int availableKeys)
{
	if(!frames || frameCount <= 0 || trackOffset < 0 || availableKeys <= 0)
		return availableKeys;
	for(int index = 0; index < frameCount; ++index)
	{
		if(frames[index].PosNum != trackOffset || frames[index].PosCnt <= 0)
			continue;
		return frames[index].PosCnt < availableKeys ? frames[index].PosCnt : availableKeys;
	}
	return availableKeys;
}

template <typename Track>
inline int FindInterpolationPair(const Track *keys, int keyCount, int frame)
{
	if(!keys || keyCount < 2 || keys[0].frame > frame)
		return -1;
	for(int index = 0; index + 1 < keyCount; ++index)
	{
		if(keys[index].frame <= frame && keys[index + 1].frame > frame)
			return index;
	}
	return -1;
}

inline int FindFrameTrackOffset(const smFRAME_POS *frames, int frameCount, int frame)
{
	if(!frames || frameCount <= 0)
		return -1;
	for(int index = 0; index < frameCount; ++index)
	{
		const smFRAME_POS &entry = frames[index];
		if(entry.PosCnt > 0 && entry.StartFrame <= frame && entry.EndFrame > frame)
			return entry.PosNum;
	}
	return -1;
}

inline int SampleRotation(const smOBJ3D &object, smFMATRIX &matrix, smFMATRIX *previousRotation,
	smTM_ROT *rotation, int frame, SMotionStEndInfo *frameInfo)
{
	const int trackOffset = TrackOffset(object.TmRot, object.TmRotCnt, rotation);
	const int keyCount = LimitTrackKeysToFrame(object.TmRotFrame, object.TmFrameCnt, trackOffset,
		RemainingTrackKeys(object.TmRot, object.TmRotCnt, rotation));
	const int pair = FindInterpolationPair(rotation, keyCount, frame);
	if(pair < 0 || !previousRotation)
		return frame;

	const int startFrame = rotation[pair].frame;
	const int endFrame = rotation[pair + 1].frame;
	if(frameInfo && endFrame > static_cast<int>(frameInfo->EndFrame))
	{
		std::memcpy(&matrix, &previousRotation[pair], sizeof(smFMATRIX));
		return startFrame;
	}

	const float alpha = static_cast<float>(frame - startFrame) / static_cast<float>(endFrame - startFrame);
	float qx = 0.0f, qy = 0.0f, qz = 0.0f, qw = 0.0f;
	D3DMath_QuaternionSlerp(qx, qy, qz, qw, 0, 0, 0, 0,
		rotation[pair + 1].x, rotation[pair + 1].y, rotation[pair + 1].z, rotation[pair + 1].w, alpha);
	smFMatrixFromQuaternion(matrix, qx, qy, qz, qw);
	smFMatrixMult(matrix, previousRotation[pair], matrix);
	return startFrame;
}

inline int SamplePosition(const smOBJ3D &object, float &x, float &y, float &z, smTM_POS *position, int frame)
{
	const int trackOffset = TrackOffset(object.TmPos, object.TmPosCnt, position);
	const int keyCount = LimitTrackKeysToFrame(object.TmPosFrame, object.TmFrameCnt, trackOffset,
		RemainingTrackKeys(object.TmPos, object.TmPosCnt, position));
	const int pair = FindInterpolationPair(position, keyCount, frame);
	if(pair < 0)
		return frame;

	const int startFrame = position[pair].frame;
	const int endFrame = position[pair + 1].frame;
	const float alpha = static_cast<float>(frame - startFrame) / static_cast<float>(endFrame - startFrame);
	x = position[pair].x + ((position[pair + 1].x - position[pair].x) * alpha);
	y = position[pair].y + ((position[pair + 1].y - position[pair].y) * alpha);
	z = position[pair].z + ((position[pair + 1].z - position[pair].z) * alpha);
	return startFrame;
}

inline int SampleScale(const smOBJ3D &object, int &x, int &y, int &z, smTM_SCALE *scale, int frame)
{
	const int trackOffset = TrackOffset(object.TmScale, object.TmScaleCnt, scale);
	const int keyCount = LimitTrackKeysToFrame(object.TmScaleFrame, object.TmFrameCnt, trackOffset,
		RemainingTrackKeys(object.TmScale, object.TmScaleCnt, scale));
	const int pair = FindInterpolationPair(scale, keyCount, frame);
	if(pair < 0)
		return frame;

	const int startFrame = scale[pair].frame;
	const int endFrame = scale[pair + 1].frame;
	const int alpha = ((frame - startFrame) << 8) / (endFrame - startFrame);
	x = scale[pair].x + (((scale[pair + 1].x - scale[pair].x) * alpha) >> 8);
	y = scale[pair].y + (((scale[pair + 1].y - scale[pair].y) * alpha) >> 8);
	z = scale[pair].z + (((scale[pair + 1].z - scale[pair].z) * alpha) >> 8);
	return startFrame;
}

} // namespace object_track
} // namespace legacy3d
} // namespace rendering
} // namespace summit
