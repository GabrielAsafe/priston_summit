#define CLAN_EITI_CPP
#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>

#include "tjclanDEF.h"

#ifdef USE_PROSTONTALE		

#include "compat/legacy/sin/sinLinkHeader.h"
#include "systems/presentation/opening/legacy_runtime/HoLinkHeader.h"
#include "systems/localization/legacy_catalog/language.h"
#include "systems/localization/legacy/ho_text/HoTextFile.h"
#include "cE_CViewClanInfo.h"
#endif

#include "tjclan.h"
#include "clan_Enti.h"
#include "cE_report.h"

#include "systems/security/integrity/auth.h"
#include "systems/security/integrity/isaocheck.h"

#include "social/clan/ClanEntity_legacy_client_body.inl"
