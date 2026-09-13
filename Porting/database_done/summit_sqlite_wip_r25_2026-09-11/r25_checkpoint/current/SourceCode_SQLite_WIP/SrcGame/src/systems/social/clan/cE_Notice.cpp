#define CE_NOTICE_CPP
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
#include "cE_report.h"
#include "cE_CViewClanInfo.h"
#include "systems/security/integrity/auth.h"
#include "systems/security/integrity/isaocheck.h"
#include "cE_Notice.h"

struct LegacyClanNoticePolicy
{
	static void BeginTextureDraw() {}
	static void EndTextureDraw() {}
};

#include "social/clan/ClanNoticeLegacyCommon.inl"
