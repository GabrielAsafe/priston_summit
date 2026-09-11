#pragma once

namespace summit
{
namespace formats
{
namespace character_config
{

struct CharacterConfigServices
{
	int (*resolveSizeLevel)(const char *name);
	bool (*resolveSoundCode)(const char *name, DWORD &code);
	bool (*resolveItemCode)(const char *name, DWORD &code);
	char *(*internNpcMessage)(const char *message);
	int (*convertMoveSpeed)(float speed);
	int (*convertPercent)(int percent);
	bool testServerMode;
	bool enableServerNpcExtensions;
};

} // namespace character_config
} // namespace formats
} // namespace summit
