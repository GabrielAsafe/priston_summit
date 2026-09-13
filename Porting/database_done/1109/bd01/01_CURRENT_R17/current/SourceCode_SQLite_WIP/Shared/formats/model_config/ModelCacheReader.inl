#pragma once

#include <cstddef>
#include <cstring>

#include "io/LegacyFile.h"

namespace summit
{
namespace formats
{
namespace model_config
{

typedef bool (*CacheKeywordValidator)(const char *cachePath, DWORD fileSize, smMODELINFO &cached, DWORD &fileCode);

enum CacheLoadResult
{
	CacheMissing,
	CacheLoaded,
	CacheInvalid
};

inline bool HasTerminator(const char *text, std::size_t capacity)
{
	return text && std::memchr(text, 0, capacity) != 0;
}

inline bool ValidateCachedModel(const smMODELINFO &model)
{
	// MotionInfo[] supports the full MOTION_INFO_MAX range.  NPC_MOTION_INFO_MAX
	// only bounds the optional NPC rate table and must not reject player/link
	// caches (legacy character bip caches commonly contain 60-120 motions).
	if(model.MotionCount < CHRMOTION_EXT || model.MotionCount > MOTION_INFO_MAX ||
		model.TalkMotionCount < CHRMOTION_EXT || model.TalkMotionCount > TALK_MOTION_INFO_MAX ||
		model.HighModel.ModelNameCnt < 0 || model.HighModel.ModelNameCnt > 4 ||
		model.DefaultModel.ModelNameCnt < 0 || model.DefaultModel.ModelNameCnt > 4 ||
		model.LowModel.ModelNameCnt < 0 || model.LowModel.ModelNameCnt > 4)
		return false;

	if(!HasTerminator(model.szModelFile, sizeof(model.szModelFile)) ||
		!HasTerminator(model.szMotionFile, sizeof(model.szMotionFile)) ||
		!HasTerminator(model.szSubModelFile, sizeof(model.szSubModelFile)) ||
		!HasTerminator(model.szLinkFile, sizeof(model.szLinkFile)) ||
		!HasTerminator(model.szTalkLinkFile, sizeof(model.szTalkLinkFile)) ||
		!HasTerminator(model.szTalkMotionFile, sizeof(model.szTalkMotionFile)))
		return false;
	return true;
}

inline bool CopyText(char *destination, std::size_t capacity, const char *source)
{
	if(!destination || capacity == 0 || !source)
		return false;
	const std::size_t length = std::strlen(source);
	if(length >= capacity)
		return false;
	std::memcpy(destination, source, length + 1);
	return true;
}

inline void InitializeModelInfo(smMODELINFO &model)
{
	model = smMODELINFO();
	model.MotionCount = CHRMOTION_EXT;
	model.TalkMotionCount = CHRMOTION_EXT;
}

inline bool IsAsePath(const char *path)
{
	if(!path)
		return false;
	const std::size_t length = std::strlen(path);
	if(length < 4 || path[length - 4] != '.')
		return false;
	const char a = path[length - 3], s = path[length - 2], e = path[length - 1];
	return (a == 'a' || a == 'A') && (s == 's' || s == 'S') && (e == 'e' || e == 'E');
}

inline bool MergeLinkedModel(smMODELINFO &destination, smMODELINFO &source, int sourceIndex, DWORD fileCode)
{
	if(!ValidateCachedModel(source))
		return false;

	if(destination.szModelFile[0] && sourceIndex > 0)
	{
		if(source.MotionCount > CHRMOTION_EXT && sourceIndex == 1)
		{
			if(MotionKeyWordDecode(fileCode, &source) == FALSE)
				return false;
			if(!CopyText(destination.szMotionFile, sizeof(destination.szMotionFile), source.szMotionFile))
				return false;
			std::memcpy(destination.MotionInfo, source.MotionInfo, sizeof(source.MotionInfo));
			destination.MotionCount = source.MotionCount;
			std::memcpy(destination.NpcMotionRate, source.NpcMotionRate, sizeof(source.NpcMotionRate));
			std::memcpy(destination.NpcMotionRateCnt, source.NpcMotionRateCnt, sizeof(source.NpcMotionRateCnt));
		}
		else if(source.TalkMotionCount > CHRMOTION_EXT && sourceIndex == 2)
		{
			if(!CopyText(destination.szTalkLinkFile, sizeof(destination.szTalkLinkFile), source.szTalkLinkFile) ||
				!CopyText(destination.szTalkMotionFile, sizeof(destination.szTalkMotionFile), source.szTalkMotionFile))
				return false;
			std::memcpy(destination.TalkMotionInfo, source.TalkMotionInfo, sizeof(source.TalkMotionInfo));
			destination.TalkMotionCount = source.TalkMotionCount;
			std::memcpy(destination.TalkMotionRate, source.TalkMotionRate, sizeof(source.TalkMotionRate));
			std::memcpy(destination.TalkMotionRateCnt, source.TalkMotionRateCnt, sizeof(source.TalkMotionRateCnt));
		}
		return true;
	}

	if(source.MotionCount > CHRMOTION_EXT && MotionKeyWordDecode(fileCode, &source) == FALSE)
		return false;
	destination = source;
	return true;
}

inline CacheLoadResult LoadCachedModel(const char *sourcePath, int sourceIndex, smMODELINFO &destination, CacheKeywordValidator validator)
{
	DWORD fileSize = 0;
	char *cachePath = smFindFile(const_cast<char *>(sourcePath), const_cast<char *>("inx"), &fileSize);
	if(!cachePath || fileSize != sizeof(smMODELINFO))
		return CacheMissing;

	summit::io::LegacyFile cache(cachePath, "rb");
	if(!cache)
		return CacheMissing;

	smMODELINFO cached = smMODELINFO();
	if(cache.Read(&cached, sizeof(cached), 1) != 1 || !ValidateCachedModel(cached))
		return CacheInvalid;

	DWORD fileCode = 0;
	if(!validator || !validator(cachePath, fileSize, cached, fileCode))
		return CacheInvalid;
	return MergeLinkedModel(destination, cached, sourceIndex, fileCode) ? CacheLoaded : CacheInvalid;
}

inline bool DecodeModel(const char *iniFile, smMODELINFO &output, CacheKeywordValidator validator, const ModelConfigContext &context)
{
	if(!iniFile || !*iniFile)
		return false;

	smMODELINFO staged;
	InitializeModelInfo(staged);
	if(IsAsePath(iniFile))
	{
		if(!CopyText(staged.szModelFile, sizeof(staged.szModelFile), iniFile))
			return false;
		output = staged;
		return true;
	}

	for(int sourceIndex = 0; sourceIndex < 3; ++sourceIndex)
	{
		const char *sourcePath = sourceIndex == 0 ? iniFile :
			(sourceIndex == 1 ? staged.szLinkFile : staged.szTalkLinkFile);
		if(!sourcePath[0])
			continue;

		const CacheLoadResult cacheResult = LoadCachedModel(sourcePath, sourceIndex, staged, validator);
		if(cacheResult == CacheInvalid)
			return false;
		if(cacheResult == CacheMissing && !ReadModelConfig(sourcePath, staged, context))
			return false;
	}

	output = staged;
	return true;
}

} // namespace model_config
} // namespace formats
} // namespace summit
