#include "content/v1/text_resource/content/TextResourceRuntime.h"
#include "opcodes.h"
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
#include "core/gameplay/runtime/playmain.h"
#include "core/gameplay/legacy_support/playsub.h"
#include "systems/localization/legacy_catalog/language.h"

#include "compat/client_server/server_legacy/onserver.h"
#include "systems/configuration/client/Settings.h"

extern void LeIniStr(char* Section, char* Key, char* szFileIni, char* Var1);

int	NotItemSpecial = 0;

extern	int	quit;

static thread_local char decode[512];
static thread_local char line[512];

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


#include "formats/model_config/LegacyModelConfigExact_client.inl"
#include "formats/server_config/LegacyServerConfigExact_client.inl"
#include "formats/legacy_config/LegacyMainConfigExact_client.inl"
extern char *szCharSizeCodeName[];
#include "content/v1/definition/legacy/CompiledDefinitionAdapter.inl"
#include "formats/item_config/LegacyItemDefinitionExact_client.inl"
#include "formats/character_config/LegacyCharacterConfigExact_client.inl"
#include "formats/stage_spawn/LegacyStageSpawnExact_client.inl"
#include "formats/item_config/LegacyItemRuntimeExact_client.inl"
#include "formats/model_config/LegacyModelKeywordExact_client.inl"
#include "formats/belatra/LegacyBelatraExact_client.inl"
#include "formats/security/LegacyCrackerConfigExact_client.inl"
