#pragma once

#include <algorithm>
#include <cstddef>
#include <cstring>

#include "formats/legacy_config/LegacyLineCursor.h"
#include "formats/model_config/ModelConfigContext.h"
#include "formats/model_config/ModelMotionConfig.h"
#include "io/LegacyFile.h"
#include "io/LegacyPath.h"

namespace summit
{
namespace formats
{
namespace model_config
{

static const int MotionFileCapacity = 128;
static const int MotionPathCapacity = 64;

class MotionFileList
{
public:
	MotionFileList()
		: count_(0)
	{
		for(int index = 0; index < MotionFileCapacity; ++index)
		{
			paths_[index][0] = 0;
			pointers_[index] = paths_[index];
		}
	}

	bool Add(const char *directory, const char *relativePath)
	{
		if(count_ >= MotionFileCapacity || !summit::io::ConcatenatePathParts(
			directory, relativePath, paths_[count_], sizeof(paths_[count_])))
			return false;
		++count_;
		return true;
	}

	int Count() const { return count_; }
	char **Pointers() { return pointers_; }

private:
	char paths_[MotionFileCapacity][MotionPathCapacity];
	char *pointers_[MotionFileCapacity];
	int count_;
};

inline int FindCommand(const ModelConfigContext &context, const char *command)
{
	if(!command || !context.commands)
		return -1;
	for(int index = 0; context.commands[index][0]; ++index)
		if(std::strcmp(command, context.commands[index]) == 0)
			return index;
	return -1;
}

inline bool CopyModelPath(const char *directory, const char *value, char *output, std::size_t outputSize)
{
	return summit::io::ConcatenatePathParts(directory, value, output, outputSize);
}

inline bool ReadModelGroup(legacy_config::LineCursor &cursor, _MODELGROUP &group)
{
	char value[64];
	for(int index = 0; index < 4; ++index)
	{
		if(cursor.Empty())
			break;
		if(!cursor.NextValue(value, sizeof(value)))
			return false;
		if(group.ModelNameCnt >= 4 || std::strlen(value) >= sizeof(group.szModelName[0]))
			return false;
		std::strcpy(group.szModelName[group.ModelNameCnt++], value);
	}
	return true;
}

inline bool ApplyEquippedWeapons(const ModelConfigContext &context, legacy_config::LineCursor &cursor, smMOTIONINFO &motion)
{
	char value[64];
	while(cursor.NextToken(value, sizeof(value)))
	{
		if(std::strcmp(value, "ALL") == 0)
		{
			motion.ItemCodeCount = 0;
			return true;
		}

		int itemCode = -1;
		if(std::strcmp(value, "NONE") == 0)
			itemCode = 0xFF;
		else if(context.resolveItemCategory)
			itemCode = context.resolveItemCategory(value);

		if(itemCode >= 0)
		{
			if(motion.ItemCodeCount < 0 || motion.ItemCodeCount >= MOTION_TOOL_MAX)
				return false;
			motion.ItemCodeList[motion.ItemCodeCount++] = static_cast<BYTE>(itemCode);
		}
	}
	return true;
}
inline bool ApplyJobs(const ModelConfigContext &context, legacy_config::LineCursor &cursor, smMOTIONINFO &motion)
{
	motion.dwJobCodeBit = 0;
	char value[64];
	while(cursor.NextToken(value, sizeof(value)))
	{
		if(context.resolveJobMask)
			motion.dwJobCodeBit |= context.resolveJobMask(value);
	}
	return true;
}
inline bool ApplyLocation(legacy_config::LineCursor &cursor, smMOTIONINFO &motion)
{
	char value[64] = { 0 };
	int position = 0;
	for(int index = 0; index < 2; ++index)
	{
		if(!cursor.NextToken(value, sizeof(value)))
			break;
		if(std::strcmp(value, "TOWN") == 0) position |= 1;
		if(std::strcmp(value, "FIELD") == 0) position |= 2;
	}
	if(position == 0 || motion.State == CHRMOTION_STATE_SKILL)
		position = 3;
	motion.MapPosition = position;
	return true;
}

inline bool ApplySkills(const ModelConfigContext &context, legacy_config::LineCursor &cursor, smMOTIONINFO &motion)
{
	if(motion.State != CHRMOTION_STATE_SKILL)
		return true;

	int skillCount = 0;
	char value[64];
	while(cursor.NextToken(value, sizeof(value)))
	{
		const int skillIndex = context.resolveSkillIndex ? context.resolveSkillIndex(value) : -1;
		if(skillIndex >= 0)
		{
			if(skillCount >= MOTION_SKIL_MAX - 1)
				return false;
			motion.SkillCodeList[skillCount++] = static_cast<BYTE>(skillIndex);
		}
	}
	motion.SkillCodeList[skillCount] = 0;
	return true;
}
inline void NormalizeNpcMotionRates(smMODELINFO &model)
{
	if(CHRMOTION_EXT >= NPC_MOTION_INFO_MAX || !model.NpcMotionRate[CHRMOTION_EXT])
		return;

	int maxRate = 0;
	int maxIndex = CHRMOTION_EXT;
	int total = 0;
	const int count = std::min<int>(model.MotionCount, NPC_MOTION_INFO_MAX);
	for(int index = CHRMOTION_EXT; index < count; ++index)
	{
		if(maxRate < model.NpcMotionRate[index])
		{
			maxRate = model.NpcMotionRate[index];
			maxIndex = index;
		}
		total += model.NpcMotionRate[index];
	}

	if(total > 100)
	{
		maxRate -= total - 100;
		if(maxRate <= 0)
		{
			model.NpcMotionRate[CHRMOTION_EXT] = 0;
			return;
		}
		model.NpcMotionRate[maxIndex] = maxRate;
	}
	else if(total < 100)
		model.NpcMotionRate[maxIndex] += 100 - total;

	int output = 0;
	for(int index = CHRMOTION_EXT; index < count; ++index)
	{
		for(int repeat = 0; repeat < model.NpcMotionRate[index] && output < 100; ++repeat)
			model.NpcMotionRateCnt[output++] = index;
	}
}

inline void NormalizeTalkMotionRates(smMODELINFO &model)
{
	if(CHRMOTION_EXT >= TALK_MOTION_INFO_MAX || !model.TalkMotionRate[CHRMOTION_EXT])
		return;

	int motionIndex[TALK_MOTION_FILE_MAX][TALK_MOTION_INFO_MAX] = { { 0 } };
	int rates[TALK_MOTION_FILE_MAX][TALK_MOTION_INFO_MAX] = { { 0 } };
	int count[TALK_MOTION_FILE_MAX];
	for(int group = 0; group < TALK_MOTION_FILE_MAX; ++group)
		count[group] = CHRMOTION_EXT;

	const int talkCount = std::min<int>(model.TalkMotionCount, TALK_MOTION_INFO_MAX);
	for(int index = CHRMOTION_EXT; index < talkCount; ++index)
	{
		const int group = model.TalkMotionInfo[index].MotionFrame;
		if(group < 0 || group >= TALK_MOTION_FILE_MAX || count[group] >= TALK_MOTION_INFO_MAX)
			continue;
		motionIndex[group][count[group]] = index;
		rates[group][count[group]] = model.TalkMotionRate[index];
		++count[group];
	}

	for(int group = 0; group < TALK_MOTION_FILE_MAX; ++group)
	{
		int maxRate = 0;
		int maxIndex = CHRMOTION_EXT;
		int total = 0;
		for(int index = CHRMOTION_EXT; index < count[group]; ++index)
		{
			if(maxRate < rates[group][index])
			{
				maxRate = rates[group][index];
				maxIndex = index;
			}
			total += rates[group][index];
		}
		if(total > 100)
		{
			maxRate -= total - 100;
			if(maxRate <= 0)
			{
				model.TalkMotionRate[CHRMOTION_EXT] = 0;
				return;
			}
			rates[group][maxIndex] = maxRate;
		}
		else if(total < 100 && count[group] > CHRMOTION_EXT)
			rates[group][maxIndex] += 100 - total;

		int output = 0;
		for(int index = CHRMOTION_EXT; index < count[group]; ++index)
		{
			for(int repeat = 0; repeat < rates[group][index] && output < 100; ++repeat)
				model.TalkMotionRateCnt[group][output++] = motionIndex[group][index];
		}
	}
}

inline bool MergeMotionFiles(smMODELINFO &model, MotionFileList &files)
{
	if(files.Count() == 0)
		return true;

	smPAT3D *pattern = 0;
	if(smFindPatFile(model.szMotionFile, "smb") == FALSE)
		pattern = smASE_MergeBone(model.szMotionFile, files.Pointers(), files.Count());
	else if(smFindPatFile(model.szTalkMotionFile, "smb") == FALSE)
		pattern = smASE_MergeBone(model.szTalkMotionFile, files.Pointers(), files.Count(), 0);
	delete pattern;
	return true;
}

inline bool WriteModelCache(const char *sourceFile, smMODELINFO &model)
{
	char cachePath[260];
	if(!summit::io::ReplaceFileExtension(sourceFile, "inx", cachePath, sizeof(cachePath)))
		return false;
	const DWORD fileCode = GetSpeedSum(cachePath);
	ModelKeyWordEncode(fileCode, &model);
	const bool encodeMotion = model.MotionCount > CHRMOTION_EXT && model.TalkMotionCount <= CHRMOTION_EXT;
	if(encodeMotion)
		MotionKeyWordEncode(fileCode, &model);

	summit::io::LegacyFile cache(cachePath, "wb");
	const bool written = cache && cache.Write(&model, sizeof(model), 1) == 1;
	if(encodeMotion)
		MotionKeyWordDecode(fileCode, &model);
	return written;
}

inline bool ReadModelConfig(const char *fileName, smMODELINFO &model, const ModelConfigContext &context)
{
	if(!fileName || !*fileName)
		return false;

	char directory[260];
	if(!summit::io::CopyFileDirectory(fileName, directory, sizeof(directory)))
		return false;

	summit::io::LegacyFile file(fileName, "rb");
	if(!file)
		return false;

	MotionFileList motionFiles;
	int lastMotion = -1;
	char lineBuffer[512];
	char command[512];
	char value[64];
	while(file.ReadLine(lineBuffer, sizeof(lineBuffer)))
	{
		legacy_config::LineCursor line(lineBuffer);
		if(!line.NextToken(command, sizeof(command)))
			continue;
		const int commandIndex = FindCommand(context, command);
		if(commandIndex < 0)
		{
			if(command[0] == '*' && context.motionWord && std::strstr(command, context.motionWord))
				AppendMotion(model, command, line.Position(), motionFiles.Count(), context.repeatWord,
					context.motionKeywords, &lastMotion);
			else if(command[0] == '*' && context.talkMotionWord && std::strstr(command, context.talkMotionWord) && motionFiles.Count())
				AppendTalkMotion(model, command, line.Position(), motionFiles.Count() - 1, context.repeatWord,
					context.talkMotionKeywords);
			continue;
		}

		legacy_config::LineCursor arguments(line.Position());
		switch(commandIndex)
		{
		case 0:
			if(!arguments.NextValue(value, sizeof(value)) ||
				!CopyModelPath(directory, value, model.szMotionFile, sizeof(model.szMotionFile))) return false;
			break;
		case 17:
		case 1:
			if(!arguments.NextValue(value, sizeof(value)) || !motionFiles.Add(directory, value)) return false;
			break;
		case 2:
			if(!arguments.NextToken(value, sizeof(value)) ||
				!CopyModelPath(directory, value, model.szModelFile, sizeof(model.szModelFile))) return false;
			break;
		case 3:
		case 9:
		case 10:
			break;
		case 4:
			if(!arguments.NextToken(value, sizeof(value)) ||
				!CopyModelPath(directory, value, model.szLinkFile, sizeof(model.szLinkFile))) return false;
			break;
		case 5:
			if(!arguments.NextValue(value, sizeof(value)) ||
				!CopyModelPath(directory, value, model.szSubModelFile, sizeof(model.szSubModelFile))) return false;
			break;
		case 6: if(!ReadModelGroup(arguments, model.HighModel)) return false; break;
		case 7: if(!ReadModelGroup(arguments, model.DefaultModel)) return false; break;
		case 8: if(!ReadModelGroup(arguments, model.LowModel)) return false; break;
		case 11:
			if(lastMotion >= 0 && lastMotion < static_cast<int>(model.MotionCount) &&
				!ApplyEquippedWeapons(context, arguments, model.MotionInfo[lastMotion])) return false;
			break;
		case 12:
			if(lastMotion >= 0 && lastMotion < static_cast<int>(model.MotionCount) &&
				!ApplyJobs(context, arguments, model.MotionInfo[lastMotion])) return false;
			break;
		case 13:
			if(lastMotion >= 0 && lastMotion < static_cast<int>(model.MotionCount) &&
				!ApplyLocation(arguments, model.MotionInfo[lastMotion])) return false;
			break;
		case 14:
			if(lastMotion >= 0 && lastMotion < static_cast<int>(model.MotionCount) &&
				!ApplySkills(context, arguments, model.MotionInfo[lastMotion])) return false;
			break;
		case 15:
			if(!arguments.NextValue(value, sizeof(value)) ||
				!CopyModelPath(directory, value, model.szTalkLinkFile, sizeof(model.szTalkLinkFile))) return false;
			break;
		case 16:
			if(!arguments.NextValue(value, sizeof(value)) ||
				!CopyModelPath(directory, value, model.szTalkMotionFile, sizeof(model.szTalkMotionFile))) return false;
			break;
		default:
			break;
		}
	}

	NormalizeNpcMotionRates(model);
	NormalizeTalkMotionRates(model);
	MergeMotionFiles(model, motionFiles);
	return WriteModelCache(fileName, model);
}

} // namespace model_config
} // namespace formats
} // namespace summit
