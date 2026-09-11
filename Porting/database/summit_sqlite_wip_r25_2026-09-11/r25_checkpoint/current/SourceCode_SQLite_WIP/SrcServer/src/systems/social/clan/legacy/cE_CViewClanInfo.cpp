#include <cstring>
#define CLAN_VIEWCLANINFO_CPP
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
#include "systems/social/clan/legacy/cE_report.h"
#include "systems/social/clan/legacy/cE_CJoin.h"
#include "systems/social/clan/legacy/cE_CViewClanInfo.h"


#include "systems/security/integrity/auth.h"
#include "systems/security/integrity/isaocheck.h"

struct LegacyClanViewPolicy
{
	using ClanMarkSurface = DIRECTDRAWSURFACE;

	static const char *PrimaryMarkFormat()
	{
		return "%s//%s//%s_%s_16.%s";
	}

	template <typename Entry>
	static void ReleasePrimaryMark(Entry &entry)
	{
		entry.hClanMark->Release();
		entry.hClanMark = NULL;
	}
};

#include "social/clan/ClanViewInfoLegacyCommon.inl"
