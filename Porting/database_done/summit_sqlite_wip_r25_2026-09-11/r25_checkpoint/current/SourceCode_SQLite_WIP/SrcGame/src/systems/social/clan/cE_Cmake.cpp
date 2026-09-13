#define CE_CMAKE_CPP

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <math.h>

#include "tjclanDEF.h"

#ifdef USE_PROSTONTALE

#include "compat/legacy/sin/sinLinkHeader.h"
#include "systems/presentation/opening/legacy_runtime/HoLinkHeader.h"
#include "systems/localization/legacy_catalog/language.h"
#include "compat/legacy/text/jts.h"
#include "systems/localization/legacy/ho_text/HoTextFile.h"
#endif

#include "tjclan.h"
#include "cE_Cmake.h"
#include "cE_report.h"
#include "cE_CViewClanInfo.h"
#include "systems/security/integrity/auth.h"
#include "systems/security/integrity/isaocheck.h"
#include "systems/audio/voice/legacy_client/Process.h"

#ifndef VOICE_CHAT
#define VOICE_CHAT
#endif

extern BOOL bCheckClan;
#include "systems/social/chat/filter/CurseFilter.h"

#include "social/clan/ClanMakeLegacyCommon.inl"
