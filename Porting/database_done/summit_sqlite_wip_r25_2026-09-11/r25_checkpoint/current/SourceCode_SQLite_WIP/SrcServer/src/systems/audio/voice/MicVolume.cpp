#include <algorithm>
using std::min;
using std::max;
#define MICVOL_CPP

#include <windows.h>
#include "systems/audio/voice/MicVolume.h"

struct LegacyMicVolumePolicy
{
	static int MaxControlStep(DWORD steps)
	{
		return (int)std::min((long)32767, (long)(steps - 1));
	}
};

#include "audio/voice/MicVolumeLegacyCommon.inl"
