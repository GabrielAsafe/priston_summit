#define CE_CHIP_CPP
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
#include "cE_chip.h"
#include "tjScroll.h"
#include "cE_report.h"
#include "cE_CViewClanInfo.h"
#include "systems/security/integrity/auth.h"
#include "systems/security/integrity/isaocheck.h"
#include "systems/audio/voice/legacy_client/Process.h"

#ifndef VOICE_CHAT
#define VOICE_CHAT
#endif

extern BOOL bCheckClan;

#if defined(__MINGW32__) || defined(__MINGW64__)
// Compiler-only compatibility overload.  The pre_refactor cE_chip.cpp calls
// SetFontTextColor(HDC, COLORREF), while the available legacy declaration is
// SetFontTextColor(COLORREF).  Keep the original call site/policy unchanged and
// map only the MinGW-only overload to the equivalent Win32 GDI operation.
static inline void SetFontTextColor(HDC dc, COLORREF color)
{
	SetTextColor(dc, color);
}
#endif

struct LegacyClanUiPolicy
{
	using ClanMarkSurface = LPDIRECT3DTEXTURE9;

	static void BeginTextureDraw() {}
	static void EndTextureDraw() {}

	static void SetTextColorCompat(HDC dc, COLORREF color)
	{
		SetFontTextColor(dc, color);
	}
};

#include "social/clan/ClanChipLegacyCommon.inl"
