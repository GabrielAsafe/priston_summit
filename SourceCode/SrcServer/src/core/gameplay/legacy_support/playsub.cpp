#include "items.h"
#include "content/v1/skin_reform/content/SkinReformRuntime.h"
#include <cstring>
#include "systems/network/protocol/server/opcodes.h"


#include "systems/rendering/legacy_3d/smd3d.h"

#include "systems/network/transport/legacy/smwsock.h"
#include "systems/network/v2/legacy/LegacyBackendChannelSender.h"
#include "systems/gameplay/character/core/character.h"
#include "systems/media/video/legacy_avi/avictrl.h"
#include "core/server/gameplay_runtime/playmain.h"
#include "systems/audio/playback/legacy/Dxwav.h"
#include "core/data/legacy_loader/fileread.h"

#include "systems/rendering/effects/particle_legacy/particle.h"
#include "systems/network/server_gameplay/netplay.h"

#include "core/server/legacy_sin_runtime/sinLinkHeader.h"
#include "systems/presentation/opening/legacy_runtime/HoLinkHeader.h"

#include "systems/world/field/field.h"
#include "systems/audio/effects/legacy/effectsnd.h"

#include "systems/persistence/character_record/record.h"
#include "core/gameplay/legacy_support/playsub.h"
#include "platform/windows/resources/legacy_runtime/resource.h"
#include "systems/rendering/legacy_2d/drawsub.h"
#include "systems/localization/config/language.h"

#include "systems/social/clan/legacy/tjclanDEF.h"
#include "systems/social/clan/legacy/tjclan.h"
#include "systems/social/clan/legacy/cE_CViewClanInfo.h"

#include "core/server/legacy_runtime/onserver.h"

#include "systems/gameplay/events/bellatra/ui/BellatraFontEffect.h"
#include "systems/localization/messages/TextMessage.h"



#include "systems/audio/download/legacy_http/WinIntThread.h"
#include "systems/ui/map/full_zoom/FullZoomMap.h"
#include "systems/rendering/font/FontImage.h"






int rsCheckHardCoreEventArea(int x, int y, int z);
extern POINT3D	rsHardCoreEventOpenPos[7][3];



int	CodeXorExp = 0;
int	CodeXorExp_High = 0;
int	CodeXorLife = 0;
int	CodeXorMana = 0;
int	CodeXorStamina = 0;
int	CodeXorMoney = 0;


#define	PLAYSUB_CHECKRESULT			                                                               1884018394

DWORD	dwPlaySub_CheckResult = PLAYSUB_CHECKRESULT;

static char *szArmorDirectory = "char//tmABCD//";
static char *szNpcDirectory = "char//npc//";

#include "core/gameplay/legacy_support/playmodel.h"
#include "gameplay/classes/v2/services/ClassService.h"

static bool LegacyPlaySubDataChannelAvailable()
{
	return gameplay::network::legacy::ServerLegacyBackendChannelSender().Available(
		gameplay::network::BackendChannel::Data);
}

static int LegacyPlaySubSendDataPacket(const void* data, int size, unsigned int opcode)
{
	return gameplay::network::legacy::ServerLegacyBackendChannelSender().Send(
		gameplay::network::BackendChannel::Data,
		gameplay::network::PacketView(data, static_cast<std::size_t>(size), opcode)).Succeeded() ? TRUE : FALSE;
}

#include "gameplay/legacy_support/PlaySubClassModelLegacyCommon.inl"

int	PlayerArmor = 0;

#define TEMP_DYNPAT_MAX		2048

smDPAT	*DynTempPat[TEMP_DYNPAT_MAX];

int DynTempPatCnt = 0;

int	PlayUsed_Resurrection = 0;


DWORD	dwSafeCodeSum;

BYTE	dwAsmJmpCode = 0xE0;

DWORD	dwStartFuncPoint = 0;
DWORD	dwFuncLen = 0;
DWORD	dwFuncSafeMemCount;
DWORD	dwFuncSafeMemCode;


DWORD	dwFuncChkTime = 0;
DWORD	dwFuncSafeCode = 0;


char	*lpFuncBackupBuff = 0;



DIRECTDRAWSURFACE	lpDDS_CodeSaveBuff = 0;

extern char *szBangImage;

DWORD	*lpCodeVram = 0;


#include "gameplay/legacy_support/PlaySubStartupModelLegacyCommon.inl"

int LoadOftenItem()
{
	char szBuff[256];

	for (int cnt = 0; cnt < MAX_ITEM; cnt++)
	{
		if (sItem[cnt].DorpItem == NULL)
		{
			continue;
		}

		wsprintf(szBuff, "Image//sinImage//Items//DropItem//it%s.ASE", sItem[cnt].DorpItem);

#ifdef _CHECK_MESH
		LoadOftenMeshPatternCheckSum(szBuff);

#else
		LoadOftenMeshPattern(szBuff);
#endif
		if (sItem[cnt].CODE == (sinGG1 | sin01) || sItem[cnt].CODE == (sinGG1 | sin02))
		{
			continue;
		}
	}

#ifdef _CHECK_MESH
	Send_OftPatSumToDataServer();
#endif

	return TRUE;
}


#include "gameplay/legacy_support/PlaySubMeshTextureSwapLegacyCommon.inl"


int SetPlayerArmor(int armor)
{
	if(PlayerArmor == armor)
		return FALSE;
	if(!lpCurPlayer || lpCurPlayer->PatLoading || !lpCurPlayer->smCharInfo.szModelName2[0])
		return FALSE;
	if(armor == -1)
		armor = 0;

	gameplay::classes::ArmorVisualVariant variant = gameplay::classes::ArmorVisualVariant::Normal;
	if(lpCurPlayer->smCharInfo.SizeLevel > 0x1000 && lpCurPlayer->smCharInfo.SizeLevel <= 0x1002)
		variant = gameplay::classes::ArmorVisualVariant::Comic;

	std::string armorPath;
	if(!TryResolveClassArmorPath(lpCurPlayer->smCharInfo.JOB_CODE, variant, armor, armorPath))
		return FALSE;

	lpCurPlayer->PatLoading = TRUE;
	AddLoaderPattern(lpCurPlayer, const_cast<char *>(armorPath.c_str()), lpCurPlayer->smCharInfo.szModelName2);
	PlayerArmor = armor;
	return TRUE;
}


int SetChrArmor(smCHAR *lpChar, int ItemNum)
{
	if(!lpChar || !lpChar->DisplayFlag || lpChar->wStickItems[smPLAYBUFF_ITEM_BODY] == ItemNum)
		return FALSE;
	if(lpChar->PatLoading || !lpChar->smCharInfo.szModelName2[0])
		return FALSE;

	int armor = 0;
	if(ItemNum >= 0)
	{
		const int rawArmor = static_cast<int>((sItem[ItemNum].CODE & sinITEM_MASK3) >> 8);
		armor = rawArmor > 29 ? rawArmor - 17 : rawArmor;
	}

	gameplay::classes::ArmorVisualVariant variant = gameplay::classes::ArmorVisualVariant::Normal;
	if(lpChar->smCharInfo.SizeLevel > 0x1000 && lpChar->smCharInfo.SizeLevel <= 0x1002)
		variant = gameplay::classes::ArmorVisualVariant::Comic;

	std::string armorPath;
	if(!TryResolveClassArmorPath(lpChar->smCharInfo.JOB_CODE, variant, armor, armorPath))
		return FALSE;

	lpChar->PatLoading = TRUE;
	AddLoaderPattern(lpChar, const_cast<char *>(armorPath.c_str()), lpChar->smCharInfo.szModelName2);
	return TRUE;
}


#include "gameplay/legacy_support/PlaySubHairSkinLegacyCommon.inl"

int ChangeModelSkin(smCHAR *lpChar, int ComicArmor, int ComicHeadNum, char *szArmor, char *szHead)
{
	if(!lpChar)
		return FALSE;

	const char *armorFile = szArmor ? szArmor : lpChar->smCharInfo.szModelName;
	const char *headFile = szHead ? szHead : lpChar->smCharInfo.szModelName2;
	if(!armorFile || !armorFile[0] || !headFile || !headFile[0])
		return FALSE;

	const char *armorBaseName = armorFile;
	for(const char *cursor = armorFile; *cursor; ++cursor)
	{
		if(*cursor == '/' || *cursor == '\\')
			armorBaseName = cursor + 1;
	}
	if(!armorBaseName[0])
		return FALSE;

	gameplay::classes::ArmorVisualVariant currentVariant = gameplay::classes::ArmorVisualVariant::Normal;
	int armorIndex = -1;
	if(!TryFindClassArmorModel(lpChar->smCharInfo.JOB_CODE, armorBaseName, currentVariant, armorIndex))
		return FALSE;

	gameplay::classes::ArmorVisualVariant targetVariant = currentVariant;
	if(ComicArmor)
		targetVariant = gameplay::classes::ArmorVisualVariant::Comic;
	else if(currentVariant == gameplay::classes::ArmorVisualVariant::Comic)
		targetVariant = gameplay::classes::ArmorVisualVariant::Normal;

	std::string armorPath;
	if(!TryResolveClassArmorPath(lpChar->smCharInfo.JOB_CODE, targetVariant, armorIndex, armorPath))
		return FALSE;

	char **lpSelFace = 0;
	char *lpSelFaceName = 0;
	const int findFaceValue = FindFaceSkin(lpChar, const_cast<char *>(headFile), &lpSelFace, &lpSelFaceName);
	if(findFaceValue < 0)
		return FALSE;
	const int hairCode = findFaceValue & 0xFF;
	const int faceCode = findFaceValue >> 8;

	if(CheckHairSkinCode(lpChar->smCharInfo.ModelNameCode2) == FALSE)
		lpChar->smCharInfo.ModelNameCode2 = GetHairSkinCode(faceCode, hairCode);

	char *resolvedHeadModel = ResolveClassHeadModel(lpChar->smCharInfo.JOB_CODE, ComicHeadNum, hairCode);
	if(!resolvedHeadModel || !resolvedHeadModel[0])
		return FALSE;

	lpChar->PatLoading = TRUE;
	AddLoaderPattern(lpChar, const_cast<char *>(armorPath.c_str()), resolvedHeadModel);
	return TRUE;
}


#include "gameplay/legacy_support/PlaySubHairModelLegacyCommon.inl"




const int  CONST_AGINGBCOLOR_MAX = MAX_AGING;

static short	AgingBlinkColor[CONST_AGINGBCOLOR_MAX][6] =
{
	{ 100,	 40,	 90,	0,	-1,	0			}, //4
	{ 120,	100,	 10,	0,	-1,	0			}, //5
	{   0,	110,	 30,	0,	-1,	0			}, //6
	{   0,	 50,	140,	0,	 0,	smTEXSTATE_FS_SCROLL5	},//7
	{ 100,	  0,	 90,	0,	 1,	smTEXSTATE_FS_SCROLL6	},//8
	{ 150,	 60,	  0,	0,	 2,	smTEXSTATE_FS_SCROLL6	},//9
	{ 150,	 10,	  0,	0,	 3,	smTEXSTATE_FS_SCROLL8	},//10
	{  20,	220,	190,	0,	 4,	smTEXSTATE_FS_SCROLL8	},//11
	{  10,	220,	 30,	0,	 4,	smTEXSTATE_FS_SCROLL8	},//12
	{ 170,	 40,	170,	0,	 4,	smTEXSTATE_FS_SCROLL9	},//13
	{ 250,	 30,	160,	0,	 4,	smTEXSTATE_FS_SCROLL9	},//14
	{  30,	190,	255,	0,	 4,	smTEXSTATE_FS_SCROLL9	},//15
	{ 250,	130,	 30,	0,	 4,	smTEXSTATE_FS_SCROLL9	},//16
	{ 120,	 30,	 30,	0,	 5,	smTEXSTATE_FS_SCROLL10	},//17
	{ 130,	  0,	255,	0,	 5,	smTEXSTATE_FS_SCROLL10	},//18
	{ 220,	240,	 70,	0,	 5,	smTEXSTATE_FS_SCROLL10	},//19
	{ 240,	240,	240,	0,	 5,	smTEXSTATE_FS_SCROLL10	},//20
	/*{   0,	207,	255,	0,	 5,	smTEXSTATE_FS_SCROLL10},// 21
	{ 255,	216,	  0,	0,	 5,	smTEXSTATE_FS_SCROLL10},// 22
	{   0,	114,	255,	0,	 5,	smTEXSTATE_FS_SCROLL10},// 23
	{ 0,	255,	  0,	0,	 5,	smTEXSTATE_FS_SCROLL10},// 24
	{ 255,	  7,	176,	0,	 5,	smTEXSTATE_FS_SCROLL10}, //25
	{ 255,	  162,	  0,	0,	 5,	smTEXSTATE_FS_SCROLL10}, //26
	{ 204,	  255,	  0,   50,	 5,	smTEXSTATE_FS_SCROLL10}, //27
	{ 156,	  0,	255,	0,	 5,	smTEXSTATE_FS_SCROLL10}, //28
	{  6,	  0,	255,  100,	 5,	smTEXSTATE_FS_SCROLL10}, //29
	{ 255,	  0,	  0,   30,	 5,	smTEXSTATE_FS_SCROLL10}, //30*/
};

static short	CraftBlinkColor[6][6] =
{
{ 13,	  0,	  5,	0,	 9,	smTEXSTATE_FS_SCROLL4 },
{ 13,	  0,	  6,	0,	 5,	smTEXSTATE_FS_SCROLL4 },
{ 13,	  0,	  7,	0,	 6,	smTEXSTATE_FS_SCROLL5 },
{ 13,	  0,	  8,	0,	 7,	smTEXSTATE_FS_SCROLL5 },
{ 13,	  0,	  9,	0,	 8,	smTEXSTATE_FS_SCROLL5 },
{ 13,	  0,	  10,	0,	 9,	smTEXSTATE_FS_SCROLL5 }
};



#include "gameplay/legacy_support/PlaySubItemBlinkLegacyCommon.inl"


int sinSetCharItem(DWORD CODE, DWORD Posi, BOOL SetFlag)
{
	DWORD	dwCode;
	sITEMINFO *lpItemInfo;
	DWORD	dwMotionCode;
	int		FrameCounter;
	int		cnt;


	switch (Posi)
	{
	case INVENTORY_POS_LHAND:
		if (SetFlag)
		{
			lpCurPlayer->SetTool(CODE, hvPOSI_LHAND);

			lpItemInfo = &cInvenTory.InvenItem[sInven[1].ItemIndex - 1].sItemInfo;
			if (!lpItemInfo->ItemName[0])
				lpItemInfo = &cInvenTory.InvenItem[sInven[0].ItemIndex - 1].sItemInfo;


			lpCurPlayer->HvLeftHand.ColorBlink = lpItemInfo->EffectBlink[0];
			memcpy(lpCurPlayer->HvLeftHand.sColors, lpItemInfo->EffectColor, sizeof(short) * 4);
			lpCurPlayer->HvLeftHand.DispEffect = lpItemInfo->DispEffect;
			lpCurPlayer->HvLeftHand.BlinkScale = lpItemInfo->ScaleBlink[0];
			lpCurPlayer->HvLeftHand.EffectKind = 0;



			if (lpItemInfo->ItemKindCode == ITEM_KIND_AGING)
			{
				cnt = lpItemInfo->ItemAgingNum[0];
				if (cnt >= 4 && cnt <= CONST_AGINGBCOLOR_MAX)
				{
					cnt -= 4;
					lpCurPlayer->HvLeftHand.ColorBlink = 9;
					lpCurPlayer->HvLeftHand.sColors[SMC_R] = AgingBlinkColor[cnt][0];
					lpCurPlayer->HvLeftHand.sColors[SMC_G] = AgingBlinkColor[cnt][1];
					lpCurPlayer->HvLeftHand.sColors[SMC_B] = AgingBlinkColor[cnt][2];
					lpCurPlayer->HvLeftHand.sColors[SMC_A] = AgingBlinkColor[cnt][3];
				}
			}

			if (lpItemInfo->ItemKindCode == ITEM_KIND_CRAFT)
			{
				cnt = lpItemInfo->ItemAgingNum[0];
				if (cnt >= 6 && cnt <= 10)
				{
					cnt -= 5;
				}
				else
				{
					if (cnt == 0) cnt = 0;
					else cnt = -1;
				}
				if (cnt >= 0)
				{
					lpCurPlayer->HvLeftHand.ColorBlink = 9;
					lpCurPlayer->HvLeftHand.sColors[SMC_R] = CraftBlinkColor[cnt][0];
					lpCurPlayer->HvLeftHand.sColors[SMC_G] = CraftBlinkColor[cnt][1];
					lpCurPlayer->HvLeftHand.sColors[SMC_B] = CraftBlinkColor[cnt][2];
					lpCurPlayer->HvLeftHand.sColors[SMC_A] = CraftBlinkColor[cnt][3];
				}
			}


			GetItemKindFromBliankColor(&lpCurPlayer->HvLeftHand);


			if (lpCurPlayer->HvLeftHand.DispEffect) AssaParticle_ClassUpWeapon(lpCurPlayer);

		}
		else
			lpCurPlayer->SetTool(0, hvPOSI_LHAND);
		break;

	case INVENTORY_POS_RHAND:
		if (SetFlag)
		{
			lpCurPlayer->SetTool(CODE, hvPOSI_RHAND);

			lpItemInfo = &cInvenTory.InvenItem[sInven[0].ItemIndex - 1].sItemInfo;

			lpCurPlayer->HvRightHand.ColorBlink = lpItemInfo->EffectBlink[0];
			memcpy(lpCurPlayer->HvRightHand.sColors, lpItemInfo->EffectColor, sizeof(short) * 4);
			lpCurPlayer->HvRightHand.DispEffect = lpItemInfo->DispEffect;
			lpCurPlayer->HvRightHand.BlinkScale = lpItemInfo->ScaleBlink[0];
			lpCurPlayer->HvRightHand.EffectKind = 0;



			if (lpItemInfo->ItemKindCode == ITEM_KIND_AGING)
			{
				cnt = lpItemInfo->ItemAgingNum[0];
				if (cnt >= 4 && cnt <= CONST_AGINGBCOLOR_MAX)
				{
					cnt -= 4;
					lpCurPlayer->HvRightHand.ColorBlink = 9;
					lpCurPlayer->HvRightHand.sColors[SMC_R] = AgingBlinkColor[cnt][0];
					lpCurPlayer->HvRightHand.sColors[SMC_G] = AgingBlinkColor[cnt][1];
					lpCurPlayer->HvRightHand.sColors[SMC_B] = AgingBlinkColor[cnt][2];
					lpCurPlayer->HvRightHand.sColors[SMC_A] = AgingBlinkColor[cnt][3];
				}
			}

			if (lpItemInfo->ItemKindCode == ITEM_KIND_CRAFT)
			{

				cnt = lpItemInfo->ItemAgingNum[0];
				if (cnt >= 6 && cnt <= 10)
				{
					cnt -= 5;
				}
				else
				{
					if (cnt == 0) cnt = 0;
					else cnt = -1;
				}
				if (cnt >= 0)
				{
					lpCurPlayer->HvRightHand.ColorBlink = 9;
					lpCurPlayer->HvRightHand.sColors[SMC_R] = CraftBlinkColor[cnt][0];
					lpCurPlayer->HvRightHand.sColors[SMC_G] = CraftBlinkColor[cnt][1];
					lpCurPlayer->HvRightHand.sColors[SMC_B] = CraftBlinkColor[cnt][2];
					lpCurPlayer->HvRightHand.sColors[SMC_A] = CraftBlinkColor[cnt][3];
				}
			}


			GetItemKindFromBliankColor(&lpCurPlayer->HvRightHand);


			if (lpCurPlayer->HvRightHand.DispEffect) AssaParticle_ClassUpWeapon(lpCurPlayer);

		}
		else
			lpCurPlayer->SetTool(0, hvPOSI_RHAND);
		break;

	case INVENTORY_POS_ARMOR:
		dwMotionCode = lpCurPlayer->MotionInfo->State;
		FrameCounter = lpCurPlayer->FrameCounter;

		if (CODE != NULL)
		{

			if (SetFlag)
			{
				dwCode = CODE;


				if ((dwCode & sinITEM_MASK3) >> 8 > 29) // Char Armor
					SetPlayerArmor(((dwCode&sinITEM_MASK3) >> 8) - 17);
				else
					SetPlayerArmor((dwCode & sinITEM_MASK3) >> 8);

				lpCurPlayer->wStickItems[smPLAYBUFF_ITEM_BODY] = GetSinItemNumFromCode(dwCode);
			}
			else
			{
				SetPlayerArmor(0);
				lpCurPlayer->wStickItems[smPLAYBUFF_ITEM_BODY] = -2;
			}
			if (dwMotionCode == CHRMOTION_STATE_DEAD)
			{

				lpCurPlayer->SetMotionFromCode(dwMotionCode);
				lpCurPlayer->frame = (lpCurPlayer->MotionInfo->EndFrame - 1) * 160;
				lpCurPlayer->FrameCounter = FrameCounter;
			}
		}
		else
		{
			SetPlayerArmor(-1);
			lpCurPlayer->wStickItems[smPLAYBUFF_ITEM_BODY] = -2;
		}
		break;
	}
	return TRUE;
}


#include "gameplay/legacy_support/PlaySubActionItemLegacyCommon.inl"

int GetDeathPenalty()
{
	sITEMINFO	*lpItemInfo;

	int	Penalty = 0;

	if (sInven[4].ItemIndex)
	{
		lpItemInfo = &cInvenTory.InvenItem[sInven[4].ItemIndex - 1].sItemInfo;
		if (lpItemInfo->CODE == (sinOR2 | sin01) && lpItemInfo->ItemKindCode == ITEM_KIND_AGING)
		{
			if (lpItemInfo->ItemAgingNum[0] >= Penalty) Penalty = lpItemInfo->ItemAgingNum[0];
		}
	}

	if (sInven[5].ItemIndex)
	{
		lpItemInfo = &cInvenTory.InvenItem[sInven[5].ItemIndex - 1].sItemInfo;
		if (lpItemInfo->CODE == (sinOR2 | sin01) && lpItemInfo->ItemKindCode == ITEM_KIND_AGING)
		{
			if (lpItemInfo->ItemAgingNum[0] >= Penalty) Penalty = lpItemInfo->ItemAgingNum[0];
		}
	}


	if (lpCurPlayer->OnStageField >= 0 && StageField[lpCurPlayer->OnStageField]->State == FIELD_STATE_CASTLE &&
		StageField[lpCurPlayer->OnStageField]->FieldCode == rsCASTLE_FIELD)
	{
		return 0;
	}


	lpItemInfo = FindItemFromCode(sinBI1 | sin04);
	if (lpItemInfo)
	{
		SendUseItemCodeToServer(lpItemInfo);
		cInvenTory.DeleteInvenItemToServer(lpItemInfo->CODE, lpItemInfo->ItemHeader.Head, lpItemInfo->ItemHeader.dwChkSum);
		PlayUsed_Resurrection++;
		return 0;
	}

#ifndef _DEATH_PANALTY
	if (sBiInfo && (sBiInfo->EventPlay[0] & BIMASK_VIP_USER) != 0) return 100;
#endif


#ifdef _LANGUAGE_THAI_RING
	if (Penalty >= 8) return 1000;
	if (Penalty >= 7) return 700;
	if (Penalty >= 6) return 600;
	if (Penalty >= 5) return 500;
	if (Penalty >= 4) return 400;
	if (Penalty >= 3) return 300;
	if (Penalty >= 2) return 200;
	if (Penalty >= 1) return 100;
#endif
	if (Penalty >= 8) return 100;
	if (Penalty >= 6) return 50;

	if ((sinQuest_levelLog&QUESTBIT_LEVEL_85) != 0 && lpCurPlayer->smCharInfo.Level >= 85)
	{

#ifdef _DEATH_PANALTY 
		return 200;
#endif
		return 50;
	}
#ifdef _DEATH_PANALTY 
	return 110;
#endif


	return 33;
}


INT64 ExpLevelTable[] = {
#include "LevelTable.h"
	- 1 };




INT64 LevelTableSum = 292288043662928391;

#include "gameplay/legacy_support/PlaySubExpXorLegacyCommon.inl"



#include "gameplay/legacy_support/PlaySubExpTableCheckLegacyCommon.inl"


int	CheckLevelExp(int Level, INT64 Exp)
{
	INT64 xp;
	INT64 np;

	if (Level > 0 && Level <= CHAR_LEVEL_MAX)
	{
		xp = ExpLevelTable[Level - 1];
		np = ExpLevelTable[Level];

		if (Exp >= xp && Exp <= np)
		{
			return TRUE;
		}

		return TRUE;

	}
	return FALSE;
}


#include "gameplay/legacy_support/PlaySubExpValueLegacyCommon.inl"



int AddExp(int Exp)
{
	int Level;
	INT64 iexp;
	INT64 ChkSum;
	INT64 exp64;
	int mlevel;


	exp64 = CodeXorCharInfo_Exp();

	mlevel = CHAR_LEVEL_MAX;

	if (exp64 < ExpLevelTable[mlevel]) exp64 += Exp;


	if (exp64 > ExpLevelTable[121] && lpCurPlayer->smCharInfo.ChangeJob < 3)
	{
		CodeXorCharInfo_Exp();
		return TRUE;
	}



	SetExp64(&lpCurPlayer->smCharInfo, exp64);

	CodeXorCharInfo_Exp();


	ReformCharForm();

	Level = lpCurPlayer->smCharInfo.Level;
	iexp = ExpLevelTable[Level];

	if (iexp >= 0 && exp64 >= iexp && lpCurPlayer->smCharInfo.Level < CHAR_LEVEL_MAX)
	{

		ChkSum = CheckLevelTable();
		if (LevelTableSum != ChkSum) return TRUE;

		lpCurPlayer->smCharInfo.Level++;
		lpCurPlayer->smCharInfo.Next_Exp = (int)ExpLevelTable[Level + 1];
		StartEffect(lpCurPlayer->pX, lpCurPlayer->pY + 32 * fONE, lpCurPlayer->pZ, EFFECT_LEVELUP1);
		esPlaySound(7, 400);

		ReformCharForm();


		SendPlayUpdateInfo();
		SaveGameData();
	}

	return TRUE;
}


#include "gameplay/legacy_support/PlaySubExpPenaltyLegacyCommon.inl"


int RainMaterial[4][8];


int InitWeatherSystem() {
	return TRUE;
}


extern int WeatherMode;

int DrawWeather(int ax) {
	return TRUE;
}


#define DYN_REMOVER_MAX		64
#define DYN_REMOVER_MASK	(DYN_REMOVER_MAX-1)

smDPAT *DynRemoveList[DYN_REMOVER_MAX];
int DynRemoveCnt = 0;
int DynRemovePush = 0;

HANDLE	hRemoverThread = 0;

#include "gameplay/legacy_support/PlaySubDynamicPatternLegacyCommon.inl"





struct	sCOMPACT_MAP
{
	smTEXTUREHANDLE	*hTexHandle;
	smTEXTUREHANDLE	*hTexTitleHandle;
	RECT	StageRect;
	sFIELD	*lpField;
	int		rx, ry;

};

sCOMPACT_MAP sCompactMap[4];

int MatArrow;
int MatMapBox;
int	MatPartyPos;
int	MatNpcPos;


int InitFieldMap()
{
	ZeroMemory(sCompactMap, sizeof(sCOMPACT_MAP) * 4);
	MatArrow = CreateTextureMaterial("Image//ARROW.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	MatMapBox = CreateTextureMaterial("Image//MapBox.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	MatPartyPos = CreateTextureMaterial("Image//party.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	MatNpcPos = CreateTextureMaterial("Image//npc.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);

	return TRUE;
}


#include "gameplay/legacy_support/PlaySubFieldMapLegacyCommon.inl"

int DrawFieldMap()
{
	int px = 656 + (WinSizeX - 800);
	int py = 426 + (WinSizeY - 600);
	int sz = 0;
	int flCode = 0;
	int	dSpe = 0;

	if (lpCurPlayer->dwTradeMsgCode)
		return FALSE;

	if (lpCurPlayer->OnStageField >= 0 &&
		(StageField[lpCurPlayer->OnStageField]->FieldCode == rsSOD_FIELD ||
			StageField[lpCurPlayer->OnStageField]->FieldCode == QUEST_ARENA_FIELD ||
			StageField[lpCurPlayer->OnStageField]->State == FIELD_STATE_ACTION) ||


		StageField[lpCurPlayer->OnStageField]->FieldCode == 36)
	{
		return FALSE;
	}


	if (lpCurPlayer->OnStageField >= 0 && StageField[lpCurPlayer->OnStageField]->State == FIELD_STATE_DUNGEON)
	{
		FieldMapDrawMode = 1;
	}
	else
	{

		FieldMapDrawMode = 2;
	}



	renderDevice.BeginScene();
	FieldMapViewSize1 = FieldMapDrawSize[FieldMapDrawMode] * 64 * fONE;
	sz = FieldMapViewSize2;

	psSetDefaultRenderState(); // transparency in mini map


	dsDrawColorBox(RGBA_MAKE(0, 0, 0, 128), px, py, 128, 128);


	DrawFieldMap(&sCompactMap[1], px + 1, py + 1, sz);

	DrawFieldMap(&sCompactMap[0], px + 1, py + 1, sz);


	DrawMapNPC(px + 64, py + 64);


	if (InterfaceParty.PartyMemberCount)
		DrawMapPartyPlayer(px + 64, py + 64);



	DrawMapArrow(px + 64, py + 64, lpCurPlayer->Angle.y);


	if (GetFieldMapCursor() == TRUE)
		dSpe = 96;
	else
		dSpe = 0;

	if (MatMapBox >= 0)
	{
		dsDrawTexImage(MatMapBox, px, py, 128, 128, 255, dSpe);
	}

	flCode = 0;
	if (lpCurPlayer->OnStageField >= 0)
	{
		if (StageField[lpCurPlayer->OnStageField] == sCompactMap[0].lpField) flCode = 0;
		if (StageField[lpCurPlayer->OnStageField] == sCompactMap[1].lpField) flCode = 1;
	}

	if (sCompactMap[flCode].hTexTitleHandle)
	{

		psDrawTexImage_Point(sCompactMap[flCode].hTexTitleHandle, (float)px, (float)py - 16, 128, 16, 0, 0, 1.0f, 1.0f, 255, dSpe >> 2);
	}

	renderDevice.EndScene();

	return TRUE;
}



int	DrawMapPartyPlayer(int px, int py)
{
	int cnt;
	int	x, y;
	int	size;
	float sz;
	float sx, sy;
	float fx, fy;
	int dist;

	size = FieldMapViewSize2;
	sz = (float)(FieldMapViewSize2 / 64);
	fx = (float)px;
	fy = (float)py;

	for (cnt = 0; cnt < InterfaceParty.PartyMemberCount; cnt++)
	{
		if (InterfaceParty.PartyMember[cnt].ChrCode != lpCurPlayer->dwObjectSerial)
		{
			x = InterfaceParty.PartyMember[cnt].CharacterPos.x - lpCurPlayer->pX;
			y = InterfaceParty.PartyMember[cnt].CharacterPos.z - lpCurPlayer->pZ;
			if (abs(x) < size && abs(y) < size)
			{
				sx = fx + (((float)x) / sz);
				sy = fy - (((float)y) / sz);

				x >>= FLOATNS;
				y >>= FLOATNS;
				dist = x * x + y * y;
				if (dist < PARTY_GETTING_DIST2)
					dsDrawTexImageFloat(MatPartyPos, sx - 3, sy - 3, 8, 8, 255, 0);
				else
					dsDrawTexImageFloat(MatPartyPos, sx - 3, sy - 3, 8, 8, RGBA_MAKE(255, 0, 0, 255), 0);

			}
		}
	}

	return TRUE;
}



#include "gameplay/legacy_support/PlaySubMapDrawLegacyCommon.inl"

int psSetDefaultRenderState()
{


	renderDevice.SetRenderState(D3DRENDERSTATE_ZENABLE, TRUE);
	renderDevice.SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
	renderDevice.SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_LESSEQUAL);



	renderDevice.SetRenderState(D3DRENDERSTATE_TEXTUREHANDLE, 0);


	renderDevice.SetRenderState(D3DRENDERSTATE_DITHERENABLE, TRUE);

	renderDevice.SetRenderState(D3DRENDERSTATE_TEXTUREADDRESS, D3DTADDRESS_WRAP);
	renderDevice.SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, TRUE);


	renderDevice.SetRenderState(D3DRENDERSTATE_TEXTUREMAG, D3DFILTER_LINEAR);
	renderDevice.SetRenderState(D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_LINEAR);

	renderDevice.SetRenderState(D3DRENDERSTATE_SPECULARENABLE, TRUE);
	renderDevice.SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, FALSE);

	renderDevice.SetRenderState(D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATE);

	renderDevice.SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
	renderDevice.SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
	renderDevice.SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);

	smRender.AlphaTestDepth = 60;

	renderDevice.SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFN_LINEAR);
	renderDevice.SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFN_LINEAR);







	return TRUE;
}

int psSetDefaultRenderState_PointFilter()
{

	renderDevice.SetRenderState(D3DRENDERSTATE_ZENABLE, TRUE);
	renderDevice.SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
	renderDevice.SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_LESSEQUAL);




	renderDevice.SetRenderState(D3DRENDERSTATE_TEXTUREHANDLE, 0);


	renderDevice.SetRenderState(D3DRENDERSTATE_DITHERENABLE, TRUE);

	renderDevice.SetRenderState(D3DRENDERSTATE_TEXTUREADDRESS, D3DTADDRESS_WRAP);
	renderDevice.SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, TRUE);


	renderDevice.SetRenderState(D3DRENDERSTATE_TEXTUREMAG, D3DFILTER_LINEAR);
	renderDevice.SetRenderState(D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_LINEAR);

	renderDevice.SetRenderState(D3DRENDERSTATE_SPECULARENABLE, TRUE);
	renderDevice.SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, FALSE);

	renderDevice.SetRenderState(D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATE);

	renderDevice.SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
	renderDevice.SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
	renderDevice.SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);

	smRender.AlphaTestDepth = 60;

	renderDevice.SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFN_POINT);
	renderDevice.SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_POINT);

	return TRUE;
}


int psDrawTexImage(smTEXTUREHANDLE *hTexHandle, float x, float y, float w, float h,
	float fx, float fy, float fw, float fh, int Transp, int specular)
{

	DRZD3DTLVERTEX tlVertex[4];
	int cnt;

	for (cnt = 0; cnt < 4; cnt++)
	{
		tlVertex[cnt].rhw = 1;
		tlVertex[cnt].color = RGBA_MAKE(255, 255, 255, Transp);
		tlVertex[cnt].specular = RGBA_MAKE(specular, specular, specular, 0);
		tlVertex[cnt].sz = 0;
	}

	tlVertex[0].sx = x;
	tlVertex[0].sy = y;
	tlVertex[0].tu = fx;
	tlVertex[0].tv = fy;

	tlVertex[1].sx = x;
	tlVertex[1].sy = y + h;
	tlVertex[1].tu = fx;
	tlVertex[1].tv = fh;

	tlVertex[2].sx = x + w;
	tlVertex[2].sy = y;
	tlVertex[2].tu = fw;
	tlVertex[2].tv = fy;

	tlVertex[3].sx = x + w;
	tlVertex[3].sy = y + h;
	tlVertex[3].tu = fw;
	tlVertex[3].tv = fh;


	if (renderDevice.IsDevice() != NULL)
	{

		psSetDefaultRenderState();

		renderDevice.SetTexture(0, hTexHandle->lpD3DTexture);

		renderDevice.DrawPrimitive(D3DPT_TRIANGLESTRIP,
			D3DFVF_TLVERTEX,
			tlVertex,
			4,
			NULL);

	}
	return TRUE;
}


int psDrawTexImage_Point(smTEXTUREHANDLE *hTexHandle, float x, float y, float w, float h,
	float fx, float fy, float fw, float fh, int Transp, int specular)
{

	DRZD3DTLVERTEX tlVertex[4];
	int cnt;

	for (cnt = 0; cnt < 4; cnt++)
	{
		tlVertex[cnt].rhw = 1;
		tlVertex[cnt].color = RGBA_MAKE(255, 255, 255, Transp);
		tlVertex[cnt].specular = RGBA_MAKE(specular, specular, specular, 0);
		tlVertex[cnt].sz = 0;
	}

	tlVertex[0].sx = x;
	tlVertex[0].sy = y;
	tlVertex[0].tu = fx;
	tlVertex[0].tv = fy;

	tlVertex[1].sx = x;
	tlVertex[1].sy = y + h;
	tlVertex[1].tu = fx;
	tlVertex[1].tv = fh;

	tlVertex[2].sx = x + w;
	tlVertex[2].sy = y;
	tlVertex[2].tu = fw;
	tlVertex[2].tv = fy;

	tlVertex[3].sx = x + w;
	tlVertex[3].sy = y + h;
	tlVertex[3].tu = fw;
	tlVertex[3].tv = fh;


	if (renderDevice.IsDevice() != NULL)
	{

		psSetDefaultRenderState_PointFilter();

		renderDevice.SetTexture(0, hTexHandle->lpD3DTexture);

		renderDevice.DrawPrimitive(D3DPT_TRIANGLESTRIP,
			D3DFVF_TLVERTEX,
			tlVertex,
			4,
			NULL);

	}
	return TRUE;
}


int psDrawTexImage2(smTEXTUREHANDLE *hTexHandle, float *x, float *y, float fx, float fy, float fw, float fh, int Transp, int specular)
{

	DRZD3DTLVERTEX tlVertex[4];
	int cnt;

	for (cnt = 0; cnt < 4; cnt++)
	{
		tlVertex[cnt].rhw = 1;
		tlVertex[cnt].color = RGBA_MAKE(255, 255, 255, Transp);
		tlVertex[cnt].specular = RGBA_MAKE(specular, specular, specular, 0);
		tlVertex[cnt].sz = 0;
	}

	tlVertex[0].sx = x[0];
	tlVertex[0].sy = y[0];
	tlVertex[0].tu = fx;
	tlVertex[0].tv = fy;

	tlVertex[1].sx = x[1];
	tlVertex[1].sy = y[1];
	tlVertex[1].tu = fx;
	tlVertex[1].tv = fh;

	tlVertex[2].sx = x[2];
	tlVertex[2].sy = y[2];
	tlVertex[2].tu = fw;
	tlVertex[2].tv = fy;

	tlVertex[3].sx = x[3];
	tlVertex[3].sy = y[3];
	tlVertex[3].tu = fw;
	tlVertex[3].tv = fh;








	if (renderDevice.IsDevice() != NULL)
	{

		psSetDefaultRenderState();

		renderDevice.SetTexture(0, hTexHandle->lpD3DTexture);

		renderDevice.DrawPrimitive(D3DPT_TRIANGLESTRIP,
			D3DFVF_TLVERTEX,
			tlVertex,
			4,
			NULL);
	}
	return TRUE;
}



#include "gameplay/legacy_support/PlaySubCompressionLegacyCommon.inl"

#include "gameplay/legacy_support/PlaySubLegacyItemReformV10Common.inl"

static DWORD GetRefromItemCodeSkin(sITEMINFO* lpItemInfo, int Key, DWORD skin)
{

	iFormCode = 0;
	iFromCnt = Key;
	DWORD itemskin = skin;

	std::uint32_t targetCode = 0;
	if (!summit::content::skin_reform::ResolveSkinReform(itemskin, targetCode))
		return 0; // no mutation for unsupported skins; caller preflights before item consumption
	lpItemInfo->CODE = targetCode;

		lpItemInfo->SpecialItemFlag[2] = ITEM_KIND_SKIN;

	ReformStateCode((char*)&lpItemInfo->CODE, sizeof(DWORD));
	ReformStateCode((char*)lpItemInfo->ItemName, 32);

	ReformStateCode((char*)&lpItemInfo->Weight, sizeof(int));
	ReformStateCode((char*)&lpItemInfo->Price, sizeof(int));

	ReformStateCode((char*)lpItemInfo->Resistance, sizeof(short) * 8);
	ReformStateCode((char*)lpItemInfo->Damage, sizeof(short) * 2);

	ReformStateCode((char*)&lpItemInfo->fAbsorb, sizeof(float));
	ReformStateCode((char*)&lpItemInfo->Defence, sizeof(int));
	ReformStateCode((char*)&lpItemInfo->fBlock_Rating, sizeof(float));

	ReformStateCode((char*)&lpItemInfo->Shooting_Range, sizeof(int));
	ReformStateCode((char*)&lpItemInfo->Attack_Speed, sizeof(int));

	ReformStateCode((char*)&lpItemInfo->Attack_Rating, sizeof(int));
	ReformStateCode((char*)&lpItemInfo->Critical_Hit, sizeof(int));

	ReformStateCode((char*)&lpItemInfo->fSpeed, sizeof(float));
	ReformStateCode((char*)&lpItemInfo->Potion_Space, sizeof(int));

	ReformStateCode((char*)&lpItemInfo->fMagic_Mastery, sizeof(float));
	ReformStateCode((char*)&lpItemInfo->fMana_Regen, sizeof(float));
	ReformStateCode((char*)&lpItemInfo->fLife_Regen, sizeof(float));
	ReformStateCode((char*)&lpItemInfo->fStamina_Regen, sizeof(float));
	ReformStateCode((char*)&lpItemInfo->fIncrease_Life, sizeof(float));
	ReformStateCode((char*)&lpItemInfo->fIncrease_Mana, sizeof(float));
	ReformStateCode((char*)&lpItemInfo->fIncrease_Stamina, sizeof(float));

	ReformStateCode((char*)&lpItemInfo->Level, sizeof(int));
	ReformStateCode((char*)&lpItemInfo->Strength, sizeof(int));
	ReformStateCode((char*)&lpItemInfo->Spirit, sizeof(int));
	ReformStateCode((char*)&lpItemInfo->Talent, sizeof(int));
	ReformStateCode((char*)&lpItemInfo->Dexterity, sizeof(int));
	ReformStateCode((char*)&lpItemInfo->Health, sizeof(int));

	ReformStateCode((char*)lpItemInfo->Mana, sizeof(short) * 2);
	ReformStateCode((char*)lpItemInfo->Life, sizeof(short) * 2);
	ReformStateCode((char*)lpItemInfo->Stamina, sizeof(short) * 2);

	ReformStateCode((char*)&lpItemInfo->Money, sizeof(int));
	ReformStateCode((char*)&lpItemInfo->JobCodeMask, sizeof(DWORD));
	ReformStateCode((char*)&lpItemInfo->JobItem, sizeof(sITEM_SPECIAL));

	ReformStateCode((char*)&lpItemInfo->UniqueItem, sizeof(DWORD));
	ReformStateCode((char*)lpItemInfo->EffectBlink, sizeof(short) * 2);
	ReformStateCode((char*)lpItemInfo->EffectColor, sizeof(short) * 4);
	ReformStateCode((char*)lpItemInfo->ScaleBlink, sizeof(short) * 2);
	ReformStateCode((char*)&lpItemInfo->DispEffect, sizeof(DWORD));

	return iFormCode;
}

#include "gameplay/legacy_support/PlaySubItemReformCoreLegacyCommon.inl"

int ReformItemSkin(sITEMINFO* lpItemInfo, DWORD skin)
{

	DWORD itemskin = skin;
	DWORD dwCode;

#ifdef _W_SERVER
	if (!lpItemInfo) return FALSE;
	std::uint32_t skinDestination = 0;
	if (!summit::content::skin_reform::ResolveSkinReform(skin, skinDestination))
		return FALSE; // no changes to item header when mapping is missing

	extern time_t		tServerTime;

	lpItemInfo->ItemHeader.Head = ItemFormKey + GetCurrentTime();

	dwCode = GetRefromItemCodeSkin(lpItemInfo, lpItemInfo->ItemHeader.Head, itemskin);

	lpItemInfo->ItemHeader.dwChkSum = dwCode;
	lpItemInfo->ItemHeader.dwVersion = ItemFormVersion;
	lpItemInfo->dwCreateTime = (DWORD)tServerTime;

	if (!lpItemInfo->BackUpKey || !lpItemInfo->BackUpChkSum)
	{
		lpItemInfo->BackUpKey = lpItemInfo->ItemHeader.Head;
		lpItemInfo->BackUpChkSum = lpItemInfo->ItemHeader.dwChkSum;
	}

	ReformMixItem(lpItemInfo);
	rsReformItem_Server(lpItemInfo);

	lpItemInfo->Temp0 = lpItemInfo->ItemHeader.dwChkSum + lpItemInfo->CODE;

#else

	if (!lpItemInfo->CODE || lpItemInfo->CODE == (sinGG1 | sin01) || (lpItemInfo->CODE & sinITEM_MASK1) == (sinPM1 & sinITEM_MASK1))
	{
		lpItemInfo->ItemHeader.Head = ItemFormKey + GetCurrentTime();

		dwCode = GetRefromItemCode(lpItemInfo, lpItemInfo->ItemHeader.Head);

		lpItemInfo->ItemHeader.dwChkSum = dwCode;
		lpItemInfo->ItemHeader.dwVersion = ItemFormVersion;
	}
#endif

	ItemFormKey++;

	return TRUE;
}

#include "gameplay/legacy_support/PlaySubItemReformLifecycleLegacyCommon.inl"

int CheckCharForm()
{
	int val;

	val = CheckCharForm(&lpCurPlayer->smCharInfo);

	if (val == FALSE)
	{


		SendSetHackUser(81);

		DisconnectServerCode = 4;
		DisconnectFlag = GetCurrentTime();

	}

	return val;
}


#include "gameplay/legacy_support/PlaySubTradeMixLegacyCommon.inl"

float GetItemAbsorb(sITEMINFO *lpItemInfo)
{
	DWORD	dwCode = lpItemInfo->CODE&sinITEM_MASK2;
	float	fAborb = lpItemInfo->fAbsorb;



	/*
	if(dwCode == sinDA1)
	{
		switch(lpItemInfo->CODE)
		{

		case (sinDA1 | sin02):
			fAborb += 0.3f;
			break;
		case (sinDA1 | sin03):
			fAborb += 0.6f;
			break;
		case (sinDA1 | sin04):
			fAborb += 1.0f;
			break;
		case (sinDA1 | sin05):
			fAborb += 1.3f;
			break;

		case (sinDA1 | sin06):
			fAborb += 1.6f;
			break;
		case (sinDA1 | sin07):
			fAborb += 1.9f;
			break;
		case (sinDA1 | sin08):
			fAborb += 2.3f;
			break;
		case (sinDA1 | sin09):
			fAborb += 2.6f;
			break;
		case (sinDA1 | sin10):
			fAborb += 3.0f;
			break;

		case (sinDA1 | sin11):
			fAborb += 3.4f;
			break;
		case (sinDA1 | sin12):
			fAborb += 3.7f;
			break;
		case (sinDA1 | sin13):
			fAborb += 4.0f;
			break;
		case (sinDA1 | sin14):
			fAborb += 4.2f;
			break;
		case (sinDA1 | sin15):
			fAborb += 4.5f;
			break;

		case (sinDA1 | sin16):
			fAborb += 4.8f;
			break;
		case (sinDA1 | sin17):
			fAborb += 5.3f;
			break;
		case (sinDA1 | sin18):
			fAborb += 6.0f;
			break;
		case (sinDA1 | sin19):
			fAborb += 7.0f;
			break;
		case (sinDA1 | sin20):
			fAborb += 8.4f;
			break;

		}
	}
	else if(dwCode == sinDA2)
	{
		switch(lpItemInfo->CODE)
		{

		case (sinDA2 | sin02):
			fAborb += 0.3f;
			break;
		case (sinDA2 | sin03):
			fAborb += 0.6f;
			break;
		case (sinDA2 | sin04):
			fAborb += 0.9f;
			break;
		case (sinDA2 | sin05):
			fAborb += 1.2f;
			break;

		case (sinDA2 | sin06):
			fAborb += 1.5f;
			break;
		case (sinDA2 | sin07):
			fAborb += 1.8f;
			break;
		case (sinDA2 | sin08):
			fAborb += 2.1f;
			break;
		case (sinDA2 | sin09):
			fAborb += 2.4f;
			break;
		case (sinDA2 | sin10):
			fAborb += 2.7f;
			break;

		case (sinDA2 | sin11):
			fAborb += 3.0f;
			break;
		case (sinDA2 | sin12):
			fAborb += 3.3f;
			break;
		case (sinDA2 | sin13):
			fAborb += 3.6f;
			break;
		case (sinDA2 | sin14):
			fAborb += 3.9f;
			break;
		case (sinDA2 | sin15):
			fAborb += 4.2f;
			break;

		case (sinDA2 | sin16):
			fAborb += 4.5f;
			break;
		case (sinDA2 | sin17):
			fAborb += 4.9f;
			break;
		case (sinDA2 | sin18):
			fAborb += 5.5f;
			break;
		case (sinDA2 | sin19):
			fAborb += 6.8f;
			break;
		case (sinDA2 | sin20):
			fAborb += 8.0f;
			break;


		}
	}

#ifdef _LANGUAGE_ENGLISH_NEXTUP

	else if(dwCode == sinDB1)
	{
		switch(lpItemInfo->CODE)
		{

		case (sinDB1 | sin01):
			fAborb += 0.0f;
			break;
		case (sinDB1 | sin02):
			fAborb += 0.0f;
			break;
		case (sinDB1 | sin03):
			fAborb += 0.2f;
			break;
		case (sinDB1 | sin04):
			fAborb += 0.3f;
			break;
		case (sinDB1 | sin05):
			fAborb += 0.4f;
			break;

		case (sinDB1 | sin06):
			fAborb += 0.5f;
			break;
		case (sinDB1 | sin07):
			fAborb += 0.6f;
			break;
		case (sinDB1 | sin08):
			fAborb += 0.7f;
			break;
		case (sinDB1 | sin09):
			fAborb += 0.8f;
			break;
		case (sinDB1 | sin10):
			fAborb += 1.1f;
			break;

		case (sinDB1 | sin11):
			fAborb += 1.3f;
			break;
		case (sinDB1 | sin12):
			fAborb += 1.5f;
			break;
		case (sinDB1 | sin13):
			fAborb += 1.7f;
			break;
		case (sinDB1 | sin14):
			fAborb += 1.9f;
			break;
		case (sinDB1 | sin15):
			fAborb += 2.1f;
			break;

		case (sinDB1 | sin16):
			fAborb += 2.3f;
			break;
		case (sinDB1 | sin17):
			fAborb += 2.5f;
			break;
		case (sinDB1 | sin18):
			fAborb += 2.7f;
			break;
		case (sinDB1 | sin19):
			fAborb += 2.9f;
			break;
		case (sinDB1 | sin20):
			fAborb += 3.1f;
			break;

		}
	}
	else if(dwCode == sinDG1)
	{
		switch(lpItemInfo->CODE)
		{

		case (sinDG1 | sin01):
			fAborb += 0.0f;
			break;
		case (sinDG1 | sin02):
			fAborb += 0.0f;
			break;
		case (sinDG1 | sin03):
			fAborb += 0.1f;
			break;
		case (sinDG1 | sin04):
			fAborb += 0.2f;
			break;
		case (sinDG1 | sin05):
			fAborb += 0.3f;
			break;

		case (sinDG1 | sin06):
			fAborb += 0.4f;
			break;
		case (sinDG1 | sin07):
			fAborb += 0.5f;
			break;
		case (sinDG1 | sin08):
			fAborb += 0.6f;
			break;
		case (sinDG1 | sin09):
			fAborb += 0.7f;
			break;
		case (sinDG1 | sin10):
			fAborb += 0.9f;
			break;

		case (sinDG1 | sin11):
			fAborb += 1.1f;
			break;
		case (sinDG1 | sin12):
			fAborb += 1.3f;
			break;
		case (sinDG1 | sin13):
			fAborb += 1.5f;
			break;
		case (sinDG1 | sin14):
			fAborb += 1.7f;
			break;
		case (sinDG1 | sin15):
			fAborb += 1.9f;
			break;

		case (sinDG1 | sin16):
			fAborb += 2.1f;
			break;
		case (sinDG1 | sin17):
			fAborb += 2.3f;
			break;
		case (sinDG1 | sin18):
			fAborb += 2.5f;
			break;
		case (sinDG1 | sin19):
			fAborb += 2.7f;
			break;
		case (sinDG1 | sin20):
			fAborb += 2.9f;
			break;

		}
	}
	else if(dwCode == sinDS1)
	{
		switch(lpItemInfo->CODE)
		{

		case (sinDS1 | sin01):
			fAborb += 0.5f;
			break;
		case (sinDS1 | sin02):
			fAborb += 0.6f;
			break;
		case (sinDS1 | sin03):
			fAborb += 0.7f;
			break;
		case (sinDS1 | sin04):
			fAborb += 0.8f;
			break;
		case (sinDS1 | sin05):
			fAborb += 0.9f;
			break;

		case (sinDS1 | sin06):
			fAborb += 1.0f;
			break;
		case (sinDS1 | sin07):
			fAborb += 1.0f;
			break;
		case (sinDS1 | sin08):
			fAborb += 1.1f;
			break;
		case (sinDS1 | sin09):
			fAborb += 1.3f;
			break;
		case (sinDS1 | sin10):
			fAborb += 1.4f;
			break;

		case (sinDS1 | sin11):
			fAborb += 1.5f;
			break;
		case (sinDS1 | sin12):
			fAborb += 1.6f;
			break;
		case (sinDS1 | sin13):
			fAborb += 1.7f;
			break;
		case (sinDS1 | sin14):
			fAborb += 1.8f;
			break;
		case (sinDS1 | sin15):
			fAborb += 1.9f;
			break;

		case (sinDS1 | sin16):
			fAborb += 2.0f;
			break;
		case (sinDS1 | sin17):
			fAborb += 2.1f;
			break;
		case (sinDS1 | sin18):
			fAborb += 2.2f;
			break;
		case (sinDS1 | sin19):
			fAborb += 2.3f;
			break;
		case (sinDS1 | sin20):
			fAborb += 2.4f;
			break;

		}
	}

#endif
*/
	return	fAborb;
}


#include "gameplay/legacy_support/PlaySubQuitCacheLegacyCommon.inl"

int	RequestPartyPlayers(smCHAR	*lpChar)
{
	int cnt, cnt2;


	if (lpChar)
	{
		lpChar->PartyFlag = FALSE;

		for (cnt = 0; cnt < InterfaceParty.PartyMemberCount; cnt++)
		{
			if (InterfaceParty.PartyMember[cnt].ChrCode == lpChar->dwObjectSerial)
			{
				lpChar->PartyFlag = TRUE;
				return TRUE;
			}
		}
	}
	else
	{

		for (cnt2 = 0; cnt2 < OTHER_PLAYER_MAX; cnt2++)
		{
			if (chrOtherPlayer[cnt2].Flag && chrOtherPlayer[cnt2].dwObjectSerial)
			{
				chrOtherPlayer[cnt2].PartyFlag = FALSE;
				for (cnt = 0; cnt < InterfaceParty.PartyMemberCount; cnt++)
				{
					if (InterfaceParty.PartyMember[cnt].ChrCode == chrOtherPlayer[cnt2].dwObjectSerial)
					{
						chrOtherPlayer[cnt2].PartyFlag = TRUE;
						break;
					}
				}
			}
		}
	}

	return FALSE;
}


#include "gameplay/legacy_support/PlaySubPartyChatLegacyCommon.inl"

#include "gameplay/legacy_support/PlaySubAttackCopyLegacyCommon.inl"

#include "gameplay/legacy_support/PlaySubSetJobFaceLegacyCommon.inl"





#define	_CHANGE_JOB_COUNT_MAX		FACE_MODEL_MAX
#define	_FACE_COUNT_MAX				HEAREPOTION_MAX


int	CheckUserModel(smCHAR_INFO *lpCharInfo)
{
	if(!lpCharInfo)
		return FALSE;

	lpCharInfo->szModelName[31] = 0;
	lpCharInfo->szModelName2[31] = 0;

	const int len = lstrlen(lpCharInfo->szModelName);
	if(len < 4 || len >= 64)
		return FALSE;

	// JOB_CODE 0 is the historical pre-class/loading state. Keep its old
	// namespace rule. Real classes are validated by their class-owned model
	// roots, which allows a new class to live outside char/tmABCD.
	if(lpCharInfo->JOB_CODE == 0)
	{
		char directory[64];
		strcpy_s(directory, lpCharInfo->szModelName);
		for(int cnt = len - 1; cnt >= 0; --cnt)
		{
			if(directory[cnt] == '/' || directory[cnt] == '\\')
			{
				directory[cnt + 1] = 0;
				break;
			}
		}
		return lstrcmpi(directory, "char//tmABCD//") == 0 &&
			lstrcmpi(&lpCharInfo->szModelName[len - 3], "ini") == 0;
	}

	const gameplay::classes::ClassService &classService = gameplay::classes::DefaultClassService();
	if(!classService.IsBodyModelPathAllowed(lpCharInfo->JOB_CODE, lpCharInfo->szModelName))
		return FALSE;

	if(lpCharInfo->szModelName2[0])
	{
		int faceCode = 0;
		int hairCode = 0;
		return FindClassHeadModel(lpCharInfo->JOB_CODE, lpCharInfo->szModelName2, faceCode, hairCode);
	}

	char *defaultHead = ResolveDefaultClassHeadModel(lpCharInfo->JOB_CODE, 0);
	if(!defaultHead || !defaultHead[0])
		return FALSE;
	strcpy_s(lpCharInfo->szModelName2, defaultHead);
	return TRUE;
}



#include "gameplay/legacy_support/PlaySubChangeJobFaceLegacyCommon.inl"




#define	TIME_EFFECT_BUFF_MAX	16

struct sTIME_EFFECT_BUFF
{
	DWORD	dwCode;
	DWORD	dwTime;
	int		x, y, z;
	smCHAR	*lpChar;
};

sTIME_EFFECT_BUFF TimeEffectBuff[TIME_EFFECT_BUFF_MAX];


#include "gameplay/legacy_support/PlaySubTimeEffectInitLegacyCommon.inl"


int AddTimeEffect(DWORD EffectCode, DWORD dwTime, int x, int y, int z, smCHAR *lpChar)
{
	int cnt;
	for (cnt = 0; cnt < TIME_EFFECT_BUFF_MAX; cnt++)
	{
		if (!TimeEffectBuff[cnt].dwCode)
		{
			TimeEffectBuff[cnt].dwCode = EffectCode;
			TimeEffectBuff[cnt].dwTime = dwTime;
			TimeEffectBuff[cnt].x = x;
			TimeEffectBuff[cnt].y = y;
			TimeEffectBuff[cnt].z = z;
			TimeEffectBuff[cnt].lpChar = lpChar;
			return TRUE;
		}
	}
	return FALSE;
}

int AddTimeEffect(DWORD EffectCode, DWORD dwTime, int x, int y, int z)
{
	return AddTimeEffect(EffectCode, dwTime, x, y, z, 0);
}


int TimeEffectMain()
{
	int cnt;
	for (cnt = 0; cnt < TIME_EFFECT_BUFF_MAX; cnt++)
	{
		if (TimeEffectBuff[cnt].dwCode && TimeEffectBuff[cnt].dwTime < dwPlayTime)
		{

			switch (TimeEffectBuff[cnt].dwCode)
			{
			case SKILL_RAGE_OF_ZECRAM_HIT2:

				StartSkill(TimeEffectBuff[cnt].x, TimeEffectBuff[cnt].y, TimeEffectBuff[cnt].z, 0, 0, 0, SKILL_RAGE_OF_ZECRAM_HIT2);
				SkillPlaySound(SKILL_SOUND_SKILL_RAGE_OF_ZECRAM2, TimeEffectBuff[cnt].x, TimeEffectBuff[cnt].y, TimeEffectBuff[cnt].z);
				SetDynLight(TimeEffectBuff[cnt].x, TimeEffectBuff[cnt].y, TimeEffectBuff[cnt].z, 100, 0, 0, 0, 500);
				break;

			case OpCode::OPCODE_SUCCESS_EVATION:

				cInterFace.SetStringEffect(HA_EVASION);
				break;

			case SKILL_EFFECT_FIREFLOWER:
				if (TimeEffectBuff[cnt].lpChar && TimeEffectBuff[cnt].lpChar->Flag)
				{
					ParkAssaParticle_FireCracker(TimeEffectBuff[cnt].lpChar, TimeEffectBuff[cnt].x);
					esPlaySound(23 + (rand() % 3), GetDistVolume(TimeEffectBuff[cnt].lpChar->pX, TimeEffectBuff[cnt].lpChar->pY, TimeEffectBuff[cnt].lpChar->pZ));
				}
				break;
			}

			TimeEffectBuff[cnt].dwCode = 0;
		}
	}
	return TRUE;
}

#include "gameplay/legacy_support/PlaySubRotPlayerLegacyCommon.inl"

int	PlaySub_EndFunc()
{


	return TRUE;
}




int	Check_PlaySubReleaseFunc()
{
	DWORD	dwStartFunc;
	DWORD	dwEndFunc;
	DWORD	*lpStartFunc;
	int		cnt, len;
	DWORD	dwChkSum;

	dwStartFunc = (DWORD)Code_VRamBuffOpen;
	dwEndFunc = (DWORD)Check_PlaySubReleaseFunc;
	len = (dwEndFunc - dwStartFunc) >> 2;

	dwChkSum = 0;

	lpStartFunc = (DWORD *)dwStartFunc;

	if (len < 0) return FALSE;

	for (cnt = 0; cnt < len; cnt++)
	{
		dwChkSum += (lpStartFunc[cnt] * ((cnt & 0x3FF) + 1));
	}

	dwPlaySub_CheckResult = dwChkSum;

	if (PLAYSUB_CHECKRESULT != dwChkSum)
	{

		return dwChkSum;
	}

	return NULL;
}



#include "gameplay/legacy_support/PlaySubPotionIntegrityLegacyCommon.inl"


int	IncPotionCount(sCHK_POTION_COUNT *ChkPotion, sITEMINFO *lpItem)
{
	int	PotionCount;

	PotionCount = lpItem->PotionCount;

	if (!PotionCount) return FALSE;

	if (lpItem->CODE == (sinPL1 | sin01))
		ChkPotion->Pl101 += PotionCount;
	if (lpItem->CODE == (sinPL1 | sin02))
		ChkPotion->Pl102 += PotionCount;
	if (lpItem->CODE == (sinPL1 | sin03))
		ChkPotion->Pl103 += PotionCount;
	if (lpItem->CODE == (sinPL1 | sin04))
		ChkPotion->Pl104 += PotionCount;
	if (lpItem->CODE == (sinPL1 | sin05))
		ChkPotion->Pl105 += PotionCount;

	if (lpItem->CODE == (sinPM1 | sin01))
		ChkPotion->Pm101 += PotionCount;
	if (lpItem->CODE == (sinPM1 | sin02))
		ChkPotion->Pm102 += PotionCount;
	if (lpItem->CODE == (sinPM1 | sin03))
		ChkPotion->Pm103 += PotionCount;
	if (lpItem->CODE == (sinPM1 | sin04))
		ChkPotion->Pm104 += PotionCount;
	if (lpItem->CODE == (sinPM1 | sin05))
		ChkPotion->Pm105 += PotionCount;

	if (lpItem->CODE == (sinPS1 | sin01))
		ChkPotion->Ps101 += PotionCount;
	if (lpItem->CODE == (sinPS1 | sin02))
		ChkPotion->Ps102 += PotionCount;
	if (lpItem->CODE == (sinPS1 | sin03))
		ChkPotion->Ps103 += PotionCount;
	if (lpItem->CODE == (sinPS1 | sin04))
		ChkPotion->Ps104 += PotionCount;
	if (lpItem->CODE == (sinPS1 | sin05))
		ChkPotion->Ps105 += PotionCount;
	return TRUE;
}



#include "gameplay/legacy_support/PlaySubPotionCountLegacyCommon.inl"



DWORD GetInvenItemCode()
{
	int cnt;

	DWORD	dwCode;


	dwCode = 0;

	for (cnt = 0; cnt < INVENTORY_MAXITEM; cnt++)
	{
		if (cInvenTory.InvenItem[cnt].Flag)
		{
			dwCode += GetItemCheckCode(&cInvenTory.InvenItem[cnt].sItemInfo);
		}
	}

	for (cnt = 0; cnt < INVENTORY_MAXITEM; cnt++)
	{
		if (cInvenTory.InvenItemTemp[cnt].Flag)
		{
			dwCode += GetItemCheckCode(&cInvenTory.InvenItemTemp[cnt].sItemInfo);
		}
	}

	if (cTrade.OpenFlag)
	{
		for (cnt = 0; cnt < MAX_TRADE_ITEM; cnt++)
		{
			if (sTrade.TradeItem[cnt].Flag)
			{
				dwCode += GetItemCheckCode(&sTrade.TradeItem[cnt].sItemInfo);
			}
		}
	}


	if (MouseItem.Flag)
	{
		dwCode += GetItemCheckCode(&MouseItem.sItemInfo);
	}

	if (cWareHouse.OpenFlag)
	{
		for (cnt = 0; cnt < 100; cnt++)
		{
			if (sWareHouse.WareHouseItem[cnt].Flag)
			{
				dwCode += GetItemCheckCode(&sWareHouse.WareHouseItem[cnt].sItemInfo);
			}
		}
	}

	if (cCraftItem.OpenFlag)
	{
		for (cnt = 0; cnt < 15; cnt++)
		{
			if (sCraftItem.CraftItem[cnt].Flag)
			{
				dwCode += GetItemCheckCode(&sCraftItem.CraftItem[cnt].sItemInfo);
			}
		}
	}

	if (cAging.OpenFlag)
	{
		for (cnt = 0; cnt < 15; cnt++)
		{
			if (sAgingItem.AgingItem[cnt].Flag)
			{
				dwCode += GetItemCheckCode(&sAgingItem.AgingItem[cnt].sItemInfo);
			}
		}
	}


	if (SmeltingItem.OpenFlag)
	{
		for (cnt = 0; cnt < 5; cnt++)
		{
			if (S_smeltingItem.SmeltingItem[cnt].Flag)
			{
				dwCode += GetItemCheckCode(&S_smeltingItem.SmeltingItem[cnt].sItemInfo);
			}
		}
	}


	if (ManufactureItem.m_OpenFlag)
	{
		for (cnt = 0; cnt < 4; cnt++)
		{
			if (g_sManufactureItem.ManufactureItem[cnt].Flag)
			{
				dwCode += GetItemCheckCode(&g_sManufactureItem.ManufactureItem[cnt].sItemInfo);
			}
		}
	}



	if (cMixtureReset.OpenFlag)
	{
		for (cnt = 0; cnt < 2; cnt++)
		{
			if (sMixtureResetItem.MixtureResetItem[cnt].Flag)
			{
				dwCode += GetItemCheckCode(&sMixtureResetItem.MixtureResetItem[cnt].sItemInfo);
			}
		}
	}

	return dwCode;
}


DWORD	dwTotal_InvenItemCode;
DWORD	dwTotal_InvenItemPlayTime;



#include "gameplay/legacy_support/PlaySubInventoryIntegrityLegacyCommon.inl"

sITEMINFO *FindInvenItem(DWORD dwItemCode, DWORD dwHead, DWORD dwChkSum)
{
	int cnt;
	sITEMINFO *lpItem;

	if ((dwItemCode&sinITEM_MASK1) == (sinPM1&sinITEM_MASK1)) return NULL;

	for (cnt = 0; cnt < INVENTORY_MAXITEM; cnt++)
	{
		if (cInvenTory.InvenItem[cnt].Flag)
		{
			lpItem = &cInvenTory.InvenItem[cnt].sItemInfo;
			if (lpItem->CODE == dwItemCode && lpItem->ItemHeader.Head == dwHead && lpItem->ItemHeader.dwChkSum == dwChkSum)
			{
				return lpItem;
			}
		}
	}

	for (cnt = 0; cnt < INVENTORY_MAXITEM; cnt++)
	{
		if (cInvenTory.InvenItemTemp[cnt].Flag)
		{
			lpItem = &cInvenTory.InvenItemTemp[cnt].sItemInfo;
			if (lpItem->CODE == dwItemCode && lpItem->ItemHeader.Head == dwHead && lpItem->ItemHeader.dwChkSum == dwChkSum)
			{
				return lpItem;
			}
		}
	}

	if (cTrade.OpenFlag)
	{
		for (cnt = 0; cnt < MAX_TRADE_ITEM; cnt++)
		{
			if (sTrade.TradeItem[cnt].Flag)
			{
				lpItem = &sTrade.TradeItem[cnt].sItemInfo;
				if (lpItem->CODE == dwItemCode && lpItem->ItemHeader.Head == dwHead && lpItem->ItemHeader.dwChkSum == dwChkSum)
				{
					return lpItem;
				}
			}
		}
	}

	if (MouseItem.Flag)
	{
		lpItem = &MouseItem.sItemInfo;
		if (lpItem->CODE == dwItemCode && lpItem->ItemHeader.Head == dwHead && lpItem->ItemHeader.dwChkSum == dwChkSum)
		{
			return lpItem;
		}
	}

	if (cWareHouse.OpenFlag)
	{
		for (cnt = 0; cnt < 100; cnt++)
		{
			if (sWareHouse.WareHouseItem[cnt].Flag)
			{
				lpItem = &sWareHouse.WareHouseItem[cnt].sItemInfo;
				if (lpItem->CODE == dwItemCode && lpItem->ItemHeader.Head == dwHead && lpItem->ItemHeader.dwChkSum == dwChkSum)
				{
					return lpItem;
				}
			}
		}
	}

	if (cCraftItem.OpenFlag)
	{
		for (cnt = 0; cnt < 15; cnt++)
		{
			if (sCraftItem.CraftItem[cnt].Flag)
			{
				lpItem = &sCraftItem.CraftItem[cnt].sItemInfo;
				if (lpItem->CODE == dwItemCode && lpItem->ItemHeader.Head == dwHead && lpItem->ItemHeader.dwChkSum == dwChkSum)
				{
					return lpItem;
				}
			}
		}
	}

	if (cAging.OpenFlag)
	{
		for (cnt = 0; cnt < 15; cnt++)
		{
			if (sAgingItem.AgingItem[cnt].Flag)
			{
				lpItem = &sAgingItem.AgingItem[cnt].sItemInfo;
				if (lpItem->CODE == dwItemCode && lpItem->ItemHeader.Head == dwHead && lpItem->ItemHeader.dwChkSum == dwChkSum)
				{
					return lpItem;
				}
			}
		}
	}

	if (SmeltingItem.OpenFlag)
	{
		for (cnt = 0; cnt < 5; cnt++)
		{
			if (S_smeltingItem.SmeltingItem[cnt].Flag)
			{
				lpItem = &S_smeltingItem.SmeltingItem[cnt].sItemInfo;
				if (lpItem->CODE == dwItemCode && lpItem->ItemHeader.Head == dwHead && lpItem->ItemHeader.dwChkSum == dwChkSum)
				{
					return lpItem;
				}
			}
		}
	}


	if (ManufactureItem.m_OpenFlag)
	{
		for (cnt = 0; cnt < 4; cnt++)
		{
			if (g_sManufactureItem.ManufactureItem[cnt].Flag)
			{
				lpItem = &g_sManufactureItem.ManufactureItem[cnt].sItemInfo;
				if (lpItem->CODE == dwItemCode && lpItem->ItemHeader.Head == dwHead && lpItem->ItemHeader.dwChkSum == dwChkSum)
				{
					return lpItem;
				}
			}
		}
	}


	if (cMixtureReset.OpenFlag)
	{
		for (cnt = 0; cnt < 2; cnt++)
		{
			if (sMixtureResetItem.MixtureResetItem[cnt].Flag)
			{
				lpItem = &sMixtureResetItem.MixtureResetItem[cnt].sItemInfo;
				if (lpItem->CODE == dwItemCode && lpItem->ItemHeader.Head == dwHead && lpItem->ItemHeader.dwChkSum == dwChkSum)
					return lpItem;
			}
		}
	}

	return NULL;
}


#include "gameplay/legacy_support/PlaySubInventoryTimingLegacyCommon.inl"

#include "gameplay/legacy_support/PlaySubFolderLoadLegacyCommon.inl"

#include "gameplay/legacy_support/PlaySubSodHelpersLegacyCommon.inl"

#include "gameplay/legacy_support/PlaySubSodMainLegacyCommon.inl"

int	DisplaySodScore()
{
	char szBuff[256];
	int x, y;
	int cnt, cnt2;
	DWORD	dwColor;
	DWORD tM, tS;
	int len;
	int RnkCnt = 0;

	if (!SodScoreFlag)
	{
		SodScoreFlag = TRUE;
		ZeroMemory(&sSodScore, sizeof(sSOD_SCORE));
	}

	psSetDefaultRenderState();

	if (SOD_StringCount)
	{
		len = lstrlen(szSOD_String);
		if ((SOD_StringCount & 0x8) == 0)
		{
			DrawFontImage(szSOD_String, MidX - len * 16, 330,
				RGBA_MAKE(bSOD_StringColor[0], bSOD_StringColor[1], bSOD_StringColor[2], 255), 2.0f);
		}
	}

	if (sSodScore.dwDispTime > dwPlayTime && rsCheckHardCoreEventArea(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ) == TRUE)
	{

		x = (WinSizeX / 2) - 80; y = 8;

		wsprintf(szBuff, "ROUND %d", sSodScore.Round);
		DrawFontImage(szBuff, x - 40, y, RGBA_MAKE(80, 255, 40, 255), 0.9f);

		if (sSodScore.dwPlayTime)
		{
			tM = (dwPlayTime - sSodScore.dwPlayTime) / 1000;
			tS = tM % 60;
			tM = tM / 60;
			if (tM > 99 || tM < 0) tM = 99;
		}
		else
		{
			tM = 0;
			tS = 0;
		}

		wsprintf(szBuff, "PlayTime %d:%d", tM, tS);
		DrawFontImage(szBuff, x + 160, y + 2, RGBA_MAKE(80, 160, 255, 255), 0.7f);

		y += 24;

		wsprintf(szBuff, "Score %d", sSodScore.Score);

		if (sSodScore.ScoreEffectCount)
			DrawFontImage(szBuff, x - 40, y + 8, RGBA_MAKE(255, 128, 64, 255 - sSodScore.ScoreEffectCount), 0.9f);
		else
			DrawFontImage(szBuff, x - 40, y + 8, RGBA_MAKE(255, 255, 0, 255), 0.9f);


		SodTeamSort();

		for (cnt = 0; cnt < 4; cnt++)
		{
			cnt2 = TeamScoreSort[cnt];

			if (sSodScore.MyTeam == cnt2)
				dwColor = RGBA_MAKE(255, 255, 0, 255);
			else
				dwColor = RGBA_MAKE(255, 128, 0, 255);

			DrawSpecialFontImage(TeamMarkSort[cnt2], x + 160, y, RGBA_MAKE(255, 255, 255, 255), 0.7f);

			wsprintf(szBuff, "Team %d", sSodScore.TeamScore[cnt2]);
			DrawFontImage(szBuff, x + 190, y, dwColor, 0.7f);
			y += 18;
		}
	}

	return FALSE;
}








