#define CLAN_VIEWCLANINFO_CPP
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
#include "cE_report.h"
#include "cE_CJoin.h"
#include "cE_CViewClanInfo.h"


#include "systems/security/integrity/auth.h"
#include "systems/security/integrity/isaocheck.h"

struct LegacyClanViewPolicy
{
	using ClanMarkSurface = LPDIRECT3DTEXTURE9;

	static const char *PrimaryMarkFormat()
	{
		return "%s//%s//%s_%s.%s";
	}

	template <typename Entry>
	static void ReleasePrimaryMark(Entry &entry)
	{
		entry.hClanMark32->Release();
		entry.hClanMark32 = NULL;
	}
};

#include "social/clan/ClanViewInfoLegacyCommon.inl"
