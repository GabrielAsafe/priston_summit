#define CE_UPDATECLDATA_CPP

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

#include <stdio.h>
#include "systems/social/clan/legacy/tjclan.h"
#include "systems/social/clan/legacy/cE_CViewClanInfo.h"
#include "systems/security/integrity/auth.h"
#include "systems/security/integrity/isaocheck.h"
#include "systems/social/clan/legacy/cE_Updatecldata.h"
#include "systems/audio/voice/legacy_ygy/YgyHeader.h"




CUpdateCldata::CUpdateCldata()
{
}

CUpdateCldata::~CUpdateCldata()
{
}


void CUpdateCldata::Updatecldata()
{
	static int cnt = 0;
	extern char updateis[256];
	wsprintf(updateis, "Updatecldata : %d ", cnt++);

	WebDB.Resume();
	WebDB.Updatcldata(cldata.szID, cldata.gserver, cldata.ChaName);
}



void g_UpdateCldata_Close()
{
	g_UpdateCldata.Close();




#ifdef VOICE_CHAT
	if(bCheckClan)
	{
		if(cldata.intClanMark)
		{

			g_CtrlProcess.SetSelectCha(cldata.szID, cldata.ChaName,
									   cldata.name, (DWORD)cldata.intClanMark, cldata.myPosition);
		}
		else
		{

			g_CtrlProcess.SetSelectCha(cldata.szID, cldata.ChaName,
									   NULL, 0, cldata.myPosition);
		}

	}
#endif


}

extern int downClanSu;
void CUpdateCldata::Close()
{
	if(OpenFlag == 0 && downClanSu == 0)
	{
		WebDB.Suspend();
	}
}


