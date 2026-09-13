#pragma once

#include "formats/character_config/CharacterConfigServices.h"
#include "formats/character_config/CharacterSoundCodeLookup.inl"

namespace summit
{
namespace formats
{
namespace character_config
{

inline int ConvertLegacyPercent8(int percent100)
{
	return (percent100 * 256) / 100;
}

inline int ConvertLegacyMoveSpeed(float speed)
{
	return static_cast<int>((speed - 9) * 16) + fONE;
}

inline int ResolveLegacyCharacterSizeLevel(const char *name)
{
	if(!name)
		return -1;
	for(int index = 0; szCharSizeCodeName[index][0]; ++index)
		if(lstrcmp(szCharSizeCodeName[index], name) == 0)
			return index;
	return -1;
}

inline bool ResolveLegacyCharacterSoundCode(const char *name, DWORD &code)
{
	return ResolveCharacterSoundCodeShared(
		name, code, [](const char *left, const char *right) { return lstrcmpi(left, right); });
}

inline bool ResolveLegacyCharacterItemCode(const char *name, DWORD &code)
{
	if(!name)
		return false;
	for(int index = 0; index < MAX_ITEM; ++index)
	{
		if(lstrcmpi(name, sItem[index].LastCategory) == 0)
		{
			code = sItem[index].CODE;
			return true;
		}
	}
	return false;
}

inline char *InternLegacyCharacterNpcMessage(const char *message)
{
	return message ? SetNPCMsg(const_cast<char *>(message)) : 0;
}

inline CharacterConfigServices MakeLegacyCharacterConfigServices(
	bool testServerMode,
	bool enableServerNpcExtensions)
{
	CharacterConfigServices services = {};
	services.resolveSizeLevel = &ResolveLegacyCharacterSizeLevel;
	services.resolveSoundCode = &ResolveLegacyCharacterSoundCode;
	services.resolveItemCode = &ResolveLegacyCharacterItemCode;
	services.internNpcMessage = &InternLegacyCharacterNpcMessage;
	services.convertMoveSpeed = &ConvertLegacyMoveSpeed;
	services.convertPercent = &ConvertLegacyPercent8;
	services.testServerMode = testServerMode;
	services.enableServerNpcExtensions = enableServerNpcExtensions;
	return services;
}

} // namespace character_config
} // namespace formats
} // namespace summit
