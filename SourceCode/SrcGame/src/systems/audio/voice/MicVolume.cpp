#include <algorithm>
#define MICVOL_CPP

#include <windows.h>
#include "MicVolume.h"

struct LegacyMicVolumePolicy
{
	static int MaxControlStep(DWORD steps)
	{
		return (int)std::min<DWORD>(32767, steps - 1);
	}
};

#include "audio/voice/MicVolumeLegacyCommon.inl"
