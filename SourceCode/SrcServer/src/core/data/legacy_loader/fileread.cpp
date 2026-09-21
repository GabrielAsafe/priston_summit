#include "items.h"
#include "compat/server_build/server_build_compat.h"
#include <cstring>
#include "systems/network/protocol/server/opcodes.h"
#include <fstream>
#include "gameplay/classes/v2/services/ClassService.h"
#include "gameplay/classes/v2/legacy/LegacyJobDataView.h"
#include <map>
#include <cstdio>





#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/network/transport/legacy/smwsock.h"
#include "systems/gameplay/character/core/character.h"
#include "core/data/legacy_loader/fileread.h"
#include "systems/world/field/field.h"
#include "systems/audio/effects/legacy/effectsnd.h"
#include "platform/windows/registry/smreg.h"
#include "core/server/gameplay_runtime/playmain.h"
#include "core/gameplay/legacy_support/playsub.h"
#include "systems/localization/config/language.h"
#include "smPacket.h"
#include "core/server/legacy_runtime/onserver.h"

#include <iostream>

using namespace std;


int	NotItemSpecial = 0;

extern	int	quit;

static char decode[512];
static char line[512];

static bool ResolveClassSpecializationBit(const char *token, DWORD &out)
{
	if(!token || !token[0]) return false;
	std::uint32_t classBit = 0u;
	if(!gameplay::classes::DefaultClassService().TryJobBitCodeByContentToken(token, classBit))
		return false;

	out = static_cast<DWORD>(classBit);
	return true;
}

static bool ResolveClassMotionJobMask(const char *token, DWORD &out)
{
	if(!token || !token[0]) return false;
	std::uint32_t motionMask = 0u;
	if(!gameplay::classes::DefaultClassService().TryMotionJobMaskByContentToken(token, motionMask))
		return false;

	out = static_cast<DWORD>(motionMask);
	return true;
}



void ModelKeyWordEncode(DWORD dwFileCode, smMODELINFO *lpMODELINFO);
int  ModelKeyWordDecode(DWORD dwFileCode, smMODELINFO *lpMODELINFO);
void MotionKeyWordEncode(DWORD dwCode, smMODELINFO *lpMODELINFO);
int	 MotionKeyWordDecode(DWORD dwCode, smMODELINFO *lpMODELINFO);


#ifdef	_W_SERVER
#define	SVR_NPC_MSG_MAX		1024
char	szSvr_NpcMsgs[SVR_NPC_MSG_MAX][256];
DWORD	dwSvr_NpcMsgCode[SVR_NPC_MSG_MAX];
#else
#define	SVR_NPC_MSG_MAX		32
char	szSvr_NpcMsgs[SVR_NPC_MSG_MAX][128];
DWORD	dwSvr_NpcMsgCode[SVR_NPC_MSG_MAX];
#endif

int		NpcMsgCount = 0;

char	szShotCutMessage[10][100];




char	*SetNPCMsg(char *szMsg)
{
	char	*lpMsg;
	DWORD	dwCode;
	int		cnt;

	if(NpcMsgCount >= SVR_NPC_MSG_MAX) return NULL;

	dwCode = GetSpeedSum(szMsg);

	for(cnt = 0; cnt < NpcMsgCount; cnt++)
	{
		if(dwCode == dwSvr_NpcMsgCode[cnt] && lstrcmp(szSvr_NpcMsgs[cnt], szMsg) == 0)
		{
			break;
		}
	}

	if(cnt < NpcMsgCount)
	{
		return szSvr_NpcMsgs[cnt];
	}

	dwSvr_NpcMsgCode[NpcMsgCount] = dwCode;
	lpMsg = szSvr_NpcMsgs[NpcMsgCount++];
	strcpy_s(lpMsg, sizeof(szSvr_NpcMsgs[0]), szMsg);

	return lpMsg;
}


#include "formats/model_config/LegacyModelConfigExact_server.inl"
#include "formats/server_config/LegacyServerConfigExact_server.inl"
#include "formats/legacy_config/LegacyMainConfigExact_server.inl"
#include "formats/item_config/LegacyItemDefinitionExact_server.inl"
#include "formats/character_config/LegacyCharacterConfigExact_server.inl"
#include "formats/stage_spawn/LegacyStageSpawnExact_server.inl"
#include "formats/item_config/LegacyItemRuntimeExact_server.inl"
#include "formats/model_config/LegacyModelKeywordExact_server.inl"
#include "formats/belatra/LegacyBelatraExact_server.inl"
#include "formats/security/LegacyCrackerConfigExact_server.inl"
