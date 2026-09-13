#pragma once

namespace
{

bool ValidateBoneMergeRequest(char *meshFile, char **fileList, int fileCount)
{
	if(!meshFile || !meshFile[0] || !fileList)
		return false;
	if(fileCount <= 0 || fileCount > OBJ_FRAME_SEARCH_MAX)
		return false;

	for(int index = 0; index < fileCount; ++index)
	{
		if(!fileList[index] || !fileList[index][0])
			return false;
	}
	return true;
}

bool LoadBonePatterns(char **fileList, int fileCount, bool normalBone,
	std::vector<std::unique_ptr<smPAT3D> > &patterns)
{
	patterns.clear();
	patterns.reserve(fileCount);

	for(int index = 0; index < fileCount; ++index)
	{
		smPhysique = 0;
		smPAT3D *loaded = normalBone
			? smASE_ReadBone(fileList[index])
			: smASE_TalkReadBone(fileList[index]);
		if(!loaded)
		{
			smPhysique = 0;
			return false;
		}
		patterns.emplace_back(loaded);
	}

	// Each legacy loader publishes the object it just returned through this
	// global. The merge owns those temporary patterns, so keeping that pointer
	// would leave a dangling dependency when the temporaries are destroyed.
	smPhysique = 0;
	return true;
}

void InitializeObjectAnimationFrames(smOBJ3D *object)
{
	for(int index = 0; index < OBJ_FRAME_SEARCH_MAX; ++index)
	{
		object->TmRotFrame[index].StartFrame = -1;
		object->TmRotFrame[index].EndFrame = -1;
		object->TmRotFrame[index].PosNum = -1;
		object->TmRotFrame[index].PosCnt = 0;

		object->TmPosFrame[index].StartFrame = -1;
		object->TmPosFrame[index].EndFrame = -1;
		object->TmPosFrame[index].PosNum = -1;
		object->TmPosFrame[index].PosCnt = 0;

		object->TmScaleFrame[index].StartFrame = -1;
		object->TmScaleFrame[index].EndFrame = -1;
		object->TmScaleFrame[index].PosNum = -1;
		object->TmScaleFrame[index].PosCnt = 0;
	}
}

smOBJ3D *FindLegacyObjectByName(smPAT3D *pattern, const char *name)
{
	if(!pattern || !name || !name[0])
		return 0;

	// GetObjectFromName predates const-correctness and does not mutate the name.
	// Keep that legacy API contained at this compatibility boundary.
	return pattern->GetObjectFromName(const_cast<char *>(name));
}

void CopyObjectBaseTransform(smOBJ3D *destination, const smOBJ3D *source)
{
	strcpy_s(destination->NodeName, source->NodeName);
	strcpy_s(destination->NodeParent, source->NodeParent);

	destination->qx = source->qx;
	destination->qy = source->qy;
	destination->qz = source->qz;
	destination->qw = source->qw;
	destination->sx = source->sx;
	destination->sy = source->sy;
	destination->sz = source->sz;
	destination->px = source->px;
	destination->py = source->py;
	destination->pz = source->pz;

	destination->Tm = source->Tm;
	destination->TmInvert = source->TmInvert;
	destination->TmRotate = source->TmRotate;
}

smOBJ3D *FindOrCreateMergedBoneObject(smPAT3D *mergedPattern, const smOBJ3D *source)
{
	if(!mergedPattern || !source)
		return 0;

	if(smOBJ3D *existing = FindLegacyObjectByName(mergedPattern, source->NodeName))
		return existing;

	std::unique_ptr<smOBJ3D> object(new smOBJ3D(0, 0));
	CopyObjectBaseTransform(object.get(), source);
	InitializeObjectAnimationFrames(object.get());
	object->TmRotCnt = 0;
	object->TmPosCnt = 0;
	object->TmScaleCnt = 0;

	smOBJ3D *created = object.get();
	mergedPattern->AddObject(object.release());
	return created;
}

bool AddTrackCount(int currentCount, int addedCount, int *result)
{
	if(!result || currentCount < 0 || addedCount < 0 || currentCount > INT_MAX - addedCount)
		return false;
	*result = currentCount + addedCount;
	return true;
}

bool ReserveObjectAnimationRange(smOBJ3D *destination, const smOBJ3D *source, int animationIndex)
{
	if(!destination || !source || !summit::io::IndexInRange(animationIndex, OBJ_FRAME_SEARCH_MAX))
		return false;

	int nextRotationCount = 0;
	int nextPositionCount = 0;
	int nextScaleCount = 0;
	if(!AddTrackCount(destination->TmRotCnt, source->TmRotCnt, &nextRotationCount) ||
		!AddTrackCount(destination->TmPosCnt, source->TmPosCnt, &nextPositionCount) ||
		!AddTrackCount(destination->TmScaleCnt, source->TmScaleCnt, &nextScaleCount))
		return false;

	destination->TmRotFrame[animationIndex].PosNum = destination->TmRotCnt;
	destination->TmRotFrame[animationIndex].PosCnt = source->TmRotCnt;
	destination->TmPosFrame[animationIndex].PosNum = destination->TmPosCnt;
	destination->TmPosFrame[animationIndex].PosCnt = source->TmPosCnt;
	destination->TmScaleFrame[animationIndex].PosNum = destination->TmScaleCnt;
	destination->TmScaleFrame[animationIndex].PosCnt = source->TmScaleCnt;

	destination->TmRotCnt = nextRotationCount;
	destination->TmPosCnt = nextPositionCount;
	destination->TmScaleCnt = nextScaleCount;
	return true;
}

bool BuildMergedBoneLayout(const std::vector<std::unique_ptr<smPAT3D> > &patterns,
	smPAT3D *mergedPattern, smFRAME_POS *frames)
{
	if(!mergedPattern || !frames || patterns.empty() || patterns.size() > OBJ_FRAME_SEARCH_MAX)
		return false;

	ZeroMemory(frames, sizeof(smFRAME_POS) * OBJ_FRAME_SEARCH_MAX);
	int nextFrame = 0;

	for(std::size_t animationIndex = 0; animationIndex < patterns.size(); ++animationIndex)
	{
		const smPAT3D *sourcePattern = patterns[animationIndex].get();
		if(!sourcePattern || sourcePattern->MaxFrame < 0 || nextFrame > INT_MAX - sourcePattern->MaxFrame)
			return false;

		for(int objectIndex = 0; objectIndex < sourcePattern->nObj3d; ++objectIndex)
		{
			const smOBJ3D *sourceObject = sourcePattern->obj3d[objectIndex];
			if(!sourceObject)
				continue;

			smOBJ3D *destinationObject = FindOrCreateMergedBoneObject(mergedPattern, sourceObject);
			if(!destinationObject ||
				!ReserveObjectAnimationRange(destinationObject, sourceObject, static_cast<int>(animationIndex)))
				return false;
		}

		frames[animationIndex].StartFrame = nextFrame;
		nextFrame += sourcePattern->MaxFrame;
		frames[animationIndex].EndFrame = nextFrame;
	}

	ZeroMemory(mergedPattern->TmFrame, sizeof(mergedPattern->TmFrame));
	for(std::size_t index = 0; index < patterns.size(); ++index)
		mergedPattern->TmFrame[index] = frames[index];
	mergedPattern->TmFrameCnt = static_cast<int>(patterns.size());
	mergedPattern->MaxFrame = nextFrame;
	return true;
}

template <typename T>
std::unique_ptr<T[]> AllocateTrack(int count)
{
	return count > 0 ? std::unique_ptr<T[]>(new T[count]) : std::unique_ptr<T[]>();
}

bool AllocateMergedBoneTracks(smPAT3D *pattern, int animationCount, const smFRAME_POS *frames)
{
	if(!pattern || !frames || animationCount <= 0 || animationCount > OBJ_FRAME_SEARCH_MAX)
		return false;

	for(int objectIndex = 0; objectIndex < pattern->nObj3d; ++objectIndex)
	{
		smOBJ3D *object = pattern->obj3d[objectIndex];
		if(!object || object->TmRotCnt < 0 || object->TmPosCnt < 0 || object->TmScaleCnt < 0)
			return false;

		std::unique_ptr<smTM_ROT[]> rotations = AllocateTrack<smTM_ROT>(object->TmRotCnt);
		std::unique_ptr<smTM_POS[]> positions = AllocateTrack<smTM_POS>(object->TmPosCnt);
		std::unique_ptr<smTM_SCALE[]> scales = AllocateTrack<smTM_SCALE>(object->TmScaleCnt);
		std::unique_ptr<smFMATRIX[]> previousRotations = AllocateTrack<smFMATRIX>(object->TmRotCnt);

		object->TmRot = rotations.release();
		object->TmPos = positions.release();
		object->TmScale = scales.release();
		object->TmPrevRot = previousRotations.release();
		object->TmFrameCnt = animationCount;

		for(int animationIndex = 0; animationIndex < animationCount; ++animationIndex)
		{
			object->TmRotFrame[animationIndex].StartFrame = frames[animationIndex].StartFrame;
			object->TmRotFrame[animationIndex].EndFrame = frames[animationIndex].EndFrame;
			object->TmPosFrame[animationIndex].StartFrame = frames[animationIndex].StartFrame;
			object->TmPosFrame[animationIndex].EndFrame = frames[animationIndex].EndFrame;
			object->TmScaleFrame[animationIndex].StartFrame = frames[animationIndex].StartFrame;
			object->TmScaleFrame[animationIndex].EndFrame = frames[animationIndex].EndFrame;
		}
	}
	return true;
}

bool AddFrameOffset(int frame, int offset, int *result)
{
	if(!result)
		return false;
	const long long adjusted = static_cast<long long>(frame) + static_cast<long long>(offset);
	if(adjusted < INT_MIN || adjusted > INT_MAX)
		return false;
	*result = static_cast<int>(adjusted);
	return true;
}

bool CopyRotationTrack(smOBJ3D *destination, const smOBJ3D *source, int animationIndex, int frameOffset)
{
	const int first = destination->TmRotFrame[animationIndex].PosNum;
	if(source->TmRotCnt > 0 && (!source->TmRot || !source->TmPrevRot || !destination->TmRot || !destination->TmPrevRot))
		return false;
	if(first < 0 || first > destination->TmRotCnt || source->TmRotCnt > destination->TmRotCnt - first)
		return false;

	for(int index = 0; index < source->TmRotCnt; ++index)
	{
		const int destinationIndex = first + index;
		destination->TmRot[destinationIndex] = source->TmRot[index];
		destination->TmPrevRot[destinationIndex] = source->TmPrevRot[index];
		if(!AddFrameOffset(destination->TmRot[destinationIndex].frame, frameOffset,
			&destination->TmRot[destinationIndex].frame))
			return false;
	}
	return true;
}

bool CopyPositionTrack(smOBJ3D *destination, const smOBJ3D *source, int animationIndex, int frameOffset)
{
	const int first = destination->TmPosFrame[animationIndex].PosNum;
	if(source->TmPosCnt > 0 && (!source->TmPos || !destination->TmPos))
		return false;
	if(first < 0 || first > destination->TmPosCnt || source->TmPosCnt > destination->TmPosCnt - first)
		return false;

	for(int index = 0; index < source->TmPosCnt; ++index)
	{
		const int destinationIndex = first + index;
		destination->TmPos[destinationIndex] = source->TmPos[index];
		if(!AddFrameOffset(destination->TmPos[destinationIndex].frame, frameOffset,
			&destination->TmPos[destinationIndex].frame))
			return false;
	}
	return true;
}

bool CopyScaleTrack(smOBJ3D *destination, const smOBJ3D *source, int animationIndex, int frameOffset)
{
	const int first = destination->TmScaleFrame[animationIndex].PosNum;
	if(source->TmScaleCnt > 0 && (!source->TmScale || !destination->TmScale))
		return false;
	if(first < 0 || first > destination->TmScaleCnt || source->TmScaleCnt > destination->TmScaleCnt - first)
		return false;

	for(int index = 0; index < source->TmScaleCnt; ++index)
	{
		const int destinationIndex = first + index;
		destination->TmScale[destinationIndex] = source->TmScale[index];
		if(!AddFrameOffset(destination->TmScale[destinationIndex].frame, frameOffset,
			&destination->TmScale[destinationIndex].frame))
			return false;
	}
	return true;
}

bool CopyMergedBoneTracks(const std::vector<std::unique_ptr<smPAT3D> > &patterns,
	smPAT3D *mergedPattern, const smFRAME_POS *frames)
{
	for(std::size_t animationIndex = 0; animationIndex < patterns.size(); ++animationIndex)
	{
		const smPAT3D *sourcePattern = patterns[animationIndex].get();
		for(int objectIndex = 0; objectIndex < sourcePattern->nObj3d; ++objectIndex)
		{
			const smOBJ3D *sourceObject = sourcePattern->obj3d[objectIndex];
			if(!sourceObject)
				continue;

			smOBJ3D *destinationObject = FindLegacyObjectByName(mergedPattern, sourceObject->NodeName);
			if(!destinationObject)
				return false;

			const int frameOffset = frames[animationIndex].StartFrame;
			if(!CopyRotationTrack(destinationObject, sourceObject, static_cast<int>(animationIndex), frameOffset) ||
				!CopyPositionTrack(destinationObject, sourceObject, static_cast<int>(animationIndex), frameOffset) ||
				!CopyScaleTrack(destinationObject, sourceObject, static_cast<int>(animationIndex), frameOffset))
				return false;
		}
	}
	return true;
}

bool SaveMergedBone(char *meshFile, smPAT3D *pattern)
{
	if(!meshFile || !pattern)
		return false;
	char *outputFile = ChangeFileExt(meshFile, szFileModelBip);
	return outputFile && outputFile[0] && pattern->SaveFile(outputFile);
}

} // anonymous namespace

smPAT3D* smASE_MergeBone(char *szMeshFile, char **FileList, int FileCnt, int ReadType)
{
	if(!ValidateBoneMergeRequest(szMeshFile, FileList, FileCnt))
		return FALSE;

	std::vector<std::unique_ptr<smPAT3D> > sourcePatterns;
	if(!LoadBonePatterns(FileList, FileCnt, ReadType != 0, sourcePatterns))
		return FALSE;

	std::unique_ptr<smPAT3D> mergedPattern(new smPAT3D);
	smFRAME_POS frames[OBJ_FRAME_SEARCH_MAX];
	if(!BuildMergedBoneLayout(sourcePatterns, mergedPattern.get(), frames) ||
		!AllocateMergedBoneTracks(mergedPattern.get(), FileCnt, frames) ||
		!CopyMergedBoneTracks(sourcePatterns, mergedPattern.get(), frames))
		return FALSE;

	mergedPattern->LinkObject();
	if(!SaveMergedBone(szMeshFile, mergedPattern.get()))
		return FALSE;

	smPhysique = 0;
	return mergedPattern.release();
}


