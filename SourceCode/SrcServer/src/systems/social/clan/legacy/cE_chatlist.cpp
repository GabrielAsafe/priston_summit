#include <cstring>

#define CE_CHATLIST_CPP
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <math.h>

#include "systems/social/clan/legacy/tjclanDEF.h"

#ifdef USE_PROSTONTALE

#include "core/server/legacy_sin_runtime/sinLinkHeader.h"
#include "systems/presentation/opening/legacy_runtime/HoLinkHeader.h"
#include "systems/localization/config/language.h"
#include "systems/localization/legacy/ho_text/HoTextFile.h"

#endif

#include "systems/social/clan/legacy/tjclan.h"

#include "systems/social/clan/legacy/tjScroll.h"
#include "systems/social/clan/legacy/cE_chatlist.h"




#ifdef PARKMENU
#include "systems/audio/voice/MicVolume.h"
#include "systems/localization/locales/Korean/K_VolText.h"
#endif




#include "systems/social/clan/legacy/cE_report.h"
#include "systems/social/clan/legacy/cE_CViewClanInfo.h"
#include "systems/security/integrity/auth.h"
#include "systems/security/integrity/isaocheck.h"

#include "systems/audio/voice/legacy_ygy/YgyHeader.h"

struct LegacyClanChatListPolicy
{
	using ClanMarkSurface = DIRECTDRAWSURFACE;

	static void BeginScrollTextureDraw()
	{
		renderDevice.BeginScene();
	}

	static void EndScrollTextureDraw()
	{
		renderDevice.EndScene();
	}

	static int LargeClanMarkSize() { return 16; }
};

#include "social/clan/ClanChatListLegacyCommon.inl"
