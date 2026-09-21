#pragma once

#include <cstdint>
#include <cstring>

#include "formats/model_config/ModelConfigContext.h"
#include "gameplay/classes/v2/services/ClassService.h"

namespace summit
{
namespace formats
{
namespace model_config
{

inline int ResolveLegacyItemCategory(const char *name)
{
	for(int index = 0; index < MAX_ITEM; ++index)
		if(std::strcmp(name, sItem[index].LastCategory) == 0)
			return index;
	return -1;
}

inline std::uint32_t ResolveLegacyJobMask(const char *name)
{
	std::uint32_t mask = 0u;
	return name && gameplay::classes::DefaultClassService().TryMotionJobMaskByContentToken(name, mask)
		? mask : 0u;
}

inline int ResolveLegacySkillIndex(const char *name)
{
	for(int index = 0; SkillDataCode[index].szSkillName[0]; ++index)
		if(lstrcmpi(SkillDataCode[index].szSkillName, name) == 0)
			return index;
	return -1;
}

inline ModelConfigContext MakeLegacyModelConfigContext(
	char *const *commands,
	const char *motionWord,
	const char *talkMotionWord,
	const char *repeatWord,
	const MotionKeywords &motionKeywords,
	const TalkMotionKeywords &talkMotionKeywords)
{
	return ModelConfigContext{
		commands,
		motionWord,
		talkMotionWord,
		repeatWord,
		motionKeywords,
		talkMotionKeywords,
		&ResolveLegacyItemCategory,
		&ResolveLegacyJobMask,
		&ResolveLegacySkillIndex
	};
}

} // namespace model_config
} // namespace formats
} // namespace summit
