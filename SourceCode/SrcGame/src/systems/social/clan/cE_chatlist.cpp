
#define CE_CHATLIST_CPP
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <math.h>

#include "tjclanDEF.h"

#ifdef USE_PROSTONTALE

#include "compat/legacy/sin/sinLinkHeader.h"
#include "systems/presentation/opening/legacy_runtime/HoLinkHeader.h"
#include "systems/localization/legacy_catalog/language.h"
#include "systems/localization/legacy/ho_text/HoTextFile.h"

#endif

#include "tjclan.h"

#include "tjScroll.h"
#include "cE_chatlist.h"




#ifdef PARKMENU
#include "systems/audio/voice/MicVolume.h"
#include "systems/localization/legacy_catalog/Korean/K_VolText.h"
#endif




#include "cE_report.h"
#include "cE_CViewClanInfo.h"
#include "systems/security/integrity/auth.h"
#include "systems/security/integrity/isaocheck.h"

#include "systems/audio/voice/legacy_client/Process.h"

#ifndef VOICE_CHAT
#define VOICE_CHAT
#endif

extern BOOL bCheckClan;

struct LegacyClanChatListPolicy
{
	using ClanMarkSurface = LPDIRECT3DTEXTURE9;

	static void BeginScrollTextureDraw() {}
	static void EndScrollTextureDraw() {}
	static int LargeClanMarkSize() { return 32; }
};

#include "social/clan/ClanChatListLegacyCommon.inl"
