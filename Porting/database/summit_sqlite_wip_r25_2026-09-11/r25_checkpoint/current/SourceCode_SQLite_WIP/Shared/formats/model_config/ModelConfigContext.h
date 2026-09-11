#pragma once

#include <cstdint>

#include "formats/model_config/ModelMotionConfig.h"

namespace summit
{
namespace formats
{
namespace model_config
{

struct ModelConfigContext
{
	char *const *commands;
	const char *motionWord;
	const char *talkMotionWord;
	const char *repeatWord;
	MotionKeywords motionKeywords;
	TalkMotionKeywords talkMotionKeywords;
	int (*resolveItemCategory)(const char *name);
	std::uint32_t (*resolveJobMask)(const char *name);
	int (*resolveSkillIndex)(const char *name);
};

} // namespace model_config
} // namespace formats
} // namespace summit
