#include "opcodes.h"
#include "systems/rendering/legacy_3d/smd3d.h"

#include "systems/network/transport/legacy/smwsock.h"
#include "systems/gameplay/character/core/character.h"
#include "systems/media/video/legacy_avi/avictrl.h"
#include "core/gameplay/runtime/playmain.h"
#include "systems/audio/playback/legacy/Dxwav.h"
#include "core/data/legacy_loader/fileread.h"

#include "systems/rendering/effects/particle/legacy/particle.h"
#include "systems/network/client/netplay.h"

#include "compat/legacy/sin/sinLinkHeader.h"
#include "systems/presentation/opening/legacy_runtime/HoLinkHeader.h"

#include "systems/world/field/field.h"
#include "systems/audio/effects/legacy/effectsnd.h"

#include "systems/persistence/character_record/record.h"
#include "core/gameplay/legacy_support/playsub.h"
#include "platform/windows/resources/resource.h"
#include "systems/rendering/2d/legacy/drawsub.h"
#include "systems/localization/legacy_catalog/language.h"

#include "systems/social/clan/tjclanDEF.h"
#include "systems/social/clan/tjclan.h"
#include "systems/social/clan/cE_CViewClanInfo.h"

#include "compat/client_server/server_legacy/onserver.h"

#include "systems/gameplay/events/bellatra/ui/BellatraFontEffect.h"
#include "systems/localization/messages/TextMessage.h"



#include "systems/audio/download/legacy_http/WinIntThread.h"
#include "systems/ui/map/full_zoom/FullZoomMap.h"
#include "systems/rendering/font/FontImage.h"
#include "systems/combat/damage_display/DrawDamage.h"
#include "systems/configuration/client/Settings.h"

#include "systems/social/party/ui/cPartyWindow.h"



extern HINSTANCE hinst;



int rsCheckHardCoreEventArea(int x, int y, int z);
extern POINT3D	rsHardCoreEventOpenPos[7][3];



int	CodeXorExp = 0;
int	CodeXorExp_High = 0;
int	CodeXorLife = 0;
int	CodeXorMana = 0;
int	CodeXorStamina = 0;
int	CodeXorMoney = 0;

extern INT64 ExpLevelTable[];
INT64 GetXorExp64(smCHAR_INFO *smCharInfo);


#define	PLAYSUB_CHECKRESULT			                                                               1884018394

DWORD	dwPlaySub_CheckResult = PLAYSUB_CHECKRESULT;

static char *szArmorDirectory = "char//tmABCD//";
static char *szNpcDirectory = "char//npc//";

#include "core/gameplay/legacy_support/playmodel.h"

int ArmorJobNum[9] = { 0, 1, 0, 3, 2, 5, 4, 7, 6 };

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



LPDIRECT3DTEXTURE9	lpDDS_CodeSaveBuff = 0;

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

		wsprintf(szBuff, "image//Sinimage//Items//DropItem//it%s.ASE", sItem[cnt].DorpItem);

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


int SetPlayerArmor(int nItem_LowCode, int nItem_HiCode)
{
	char szbuff[256];

	if (PlayerArmor == (nItem_HiCode | nItem_LowCode))
		return FALSE;

	if (lpCurPlayer->PatLoading)
		return FALSE;

	if (!lpCurPlayer->smCharInfo.szModelName2[0])
		return FALSE;

	if (nItem_LowCode == -1)
		nItem_LowCode = 0;

	int jobNum = lpCurPlayer->smCharInfo.JOB_CODE;
	if (nItem_LowCode > ARMOR_FILE_MAX - 1)
		nItem_LowCode = ARMOR_FILE_MAX - 1;
	if (jobNum > MAX_JOBNUM)
		jobNum = 0;

	strcpy_s(szbuff, szArmorDirectory);
	if (nItem_HiCode == sinCA5 || nItem_HiCode == sinCA6)
	{
		lstrcat(szbuff, szArmorNumFile_New[ArmorJobNum[jobNum]][nItem_LowCode]);
	}
	else if (nItem_HiCode == sinDA3 || nItem_HiCode == sinDA4)
	{
		lstrcat(szbuff, szArmorNumFile_New_2[ArmorJobNum[jobNum]][nItem_LowCode]);
	}
	else
	{
		if (lpCurPlayer->smCharInfo.SizeLevel > 0x1000 && lpCurPlayer->smCharInfo.SizeLevel <= 0x1002)
			lstrcat(szbuff, szArmorNumFile_Comic[ArmorJobNum[jobNum]][nItem_LowCode]);
		else
			lstrcat(szbuff, szArmorNumFile[ArmorJobNum[jobNum]][nItem_LowCode]);
	}

	lpCurPlayer->PatLoading = TRUE;
	AddLoaderPattern(lpCurPlayer, szbuff, lpCurPlayer->smCharInfo.szModelName2);

	PlayerArmor = (nItem_HiCode | nItem_LowCode);

	return TRUE;
}


int SetChrArmor(smCHAR *lpChar, int ItemNum)
{
	char szbuff[256];
	int armor;
	int jobNum;

	if (!lpChar->DisplayFlag || lpChar->wStickItems[smPLAYBUFF_ITEM_BODY] == ItemNum) return FALSE;
	if (lpChar->PatLoading) return FALSE;
	if (!lpChar->smCharInfo.szModelName2[0]) return FALSE;

	jobNum = lpChar->smCharInfo.JOB_CODE;
	if (jobNum > MAX_JOBNUM) jobNum = 0;

	if (ItemNum >= 0)
	{
		if (((sItem[ItemNum].CODE&sinITEM_MASK3) >> 8) > 25)
			armor = (((sItem[ItemNum].CODE&sinITEM_MASK3) >> 8) - 16);
		else
			armor = (sItem[ItemNum].CODE&sinITEM_MASK3) >> 8;
	}
	else
		armor = 0;

	lstrcpy(szbuff, szArmorDirectory);

	if ((sItem[ItemNum].CODE&sinITEM_MASK2) == sinCA5 || (sItem[ItemNum].CODE&sinITEM_MASK2) == sinCA6)
	{
		lstrcat(szbuff, szArmorNumFile_New[ArmorJobNum[jobNum]][armor]);
	}
	else if ((sItem[ItemNum].CODE&sinITEM_MASK2) == sinDA3 || (sItem[ItemNum].CODE&sinITEM_MASK2) == sinDA4)
	{
		lstrcat(szbuff, szArmorNumFile_New_2[ArmorJobNum[jobNum]][armor]);
	}
	else
	{
		if (lpCurPlayer->smCharInfo.SizeLevel > 0x1000 && lpCurPlayer->smCharInfo.SizeLevel <= 0x1002)
			lstrcat(szbuff, szArmorNumFile_Comic[ArmorJobNum[jobNum]][armor]);
		else
			lstrcat(szbuff, szArmorNumFile[ArmorJobNum[jobNum]][armor]);
	}

	lpChar->PatLoading = TRUE;
	AddLoaderPattern(lpChar, szbuff, lpChar->smCharInfo.szModelName2);

	return TRUE;
}


#include "gameplay/legacy_support/PlaySubHairSkinLegacyCommon.inl"

int	ChangeModelSkin(smCHAR *lpChar, int ComicArmor, int ComicHeadNum, char *szArmor, char *szHead)
{
	char szArmorName[32];
	char szHeadName[32];
	char szbuff1[256];
	char szbuff2[256];

	int len, cnt;
	int jobNum;
	int SelCount;
	int FindArmor, FindHead;
	char *lpArmorFile;
	char *lpHeadFile;


	char **lpSelFace = 0;
	char *lpSelFaceName = 0;

	jobNum = lpChar->smCharInfo.JOB_CODE;
	if (jobNum > MAX_JOBNUM) jobNum = 0;

	if (szArmor) lpArmorFile = szArmor;
	else lpArmorFile = lpChar->smCharInfo.szModelName;

	if (szHead) lpHeadFile = szHead;
	else lpHeadFile = lpChar->smCharInfo.szModelName2;

	szArmorName[0] = 0;
	szHeadName[0] = 0;


	len = lstrlen(lpArmorFile);
	for (cnt = len - 1; cnt >= 0; cnt--)
	{
		if (lpArmorFile[cnt] == '/' || lpArmorFile[cnt] == '\\')
		{
			strcpy_s(szArmorName, &lpArmorFile[cnt + 1]);
			break;
		}
	}

	strcpy_s(szHeadName, lpHeadFile);


	if (!szArmorName[0] || !szHeadName[0])
		return FALSE;

	SelCount = ComicHeadNum;

	FindArmor = 0;
	FindHead = 0;

	for (cnt = 0; cnt < ARMOR_FILE_MAX; cnt++)
	{
		if (lstrcmpi(szArmorName, szArmorNumFile[ArmorJobNum[jobNum]][cnt]) == 0)
		{
			if (ComicArmor)
				strcpy_s(szArmorName, szArmorNumFile_Comic[ArmorJobNum[jobNum]][cnt]);
			else
				strcpy_s(szArmorName, szArmorNumFile[ArmorJobNum[jobNum]][cnt]);

			FindArmor++;
			break;
		}
	}

	if (!FindArmor)
	{
		for (cnt = 0; cnt < ARMOR_FILE_MAX; cnt++)
		{
			if (lstrcmpi(szArmorName, szArmorNumFile_Comic[ArmorJobNum[jobNum]][cnt]) == 0)
			{
				if (ComicArmor)
					strcpy_s(szArmorName, szArmorNumFile_Comic[ArmorJobNum[jobNum]][cnt]);
				else
					strcpy_s(szArmorName, szArmorNumFile[ArmorJobNum[jobNum]][cnt]);

				FindArmor++;
				break;
			}
		}
	}

	if (!FindArmor)
	{
		for (cnt = 0; cnt < ARMOR_FILE_MAX; cnt++)
		{
			if (lstrcmpi(szArmorName, szArmorNumFile_New[ArmorJobNum[jobNum]][cnt]) == 0)
			{
				if (ComicArmor)
					lstrcpy(szArmorName, szArmorNumFile_Comic[ArmorJobNum[jobNum]][cnt]);
				else
					lstrcpy(szArmorName, szArmorNumFile_New[ArmorJobNum[jobNum]][cnt]);

				FindArmor++;
				break;
			}
		}
	}

	if (!FindArmor)
	{
		for (cnt = 0; cnt < ARMOR_FILE_MAX; cnt++)
		{
			if (lstrcmpi(szArmorName, szArmorNumFile_New_2[ArmorJobNum[jobNum]][cnt]) == 0)
			{
				if (ComicArmor)
					lstrcpy(szArmorName, szArmorNumFile_Comic[ArmorJobNum[jobNum]][cnt]);
				else
					lstrcpy(szArmorName, szArmorNumFile_New_2[ArmorJobNum[jobNum]][cnt]);

				FindArmor++;
				break;
			}
		}
	}


	int FindFaceValue = FindFaceSkin(lpChar, szHeadName, &lpSelFace, &lpSelFaceName);
	if (FindFaceValue < 0) return FALSE;
	int HairCode = FindFaceValue & 0xFF;
	int FaceCode = FindFaceValue >> 8;


	if (CheckHairSkinCode(lpChar->smCharInfo.ModelNameCode2) == FALSE)
	{
		lpChar->smCharInfo.ModelNameCode2 = GetHairSkinCode(FaceCode, HairCode);
	}

	switch (jobNum)
	{
	case JOBCODE_FIGHTER:
		strcpy_s(szHeadName, szModel_FighterFaceName[SelCount][HairCode]);
		break;
	case JOBCODE_MECHANICIAN:
		strcpy_s(szHeadName, szModel_MechFaceName[SelCount][HairCode]);
		break;
	case JOBCODE_ARCHER:
		strcpy_s(szHeadName, szModel_ArcherFaceName[SelCount][HairCode]);
		break;
	case JOBCODE_PIKEMAN:
		strcpy_s(szHeadName, szModel_PikeFaceName[SelCount][HairCode]);
		break;
	case JOBCODE_ATALANTA:
		strcpy_s(szHeadName, szModel_AtalantaFaceName[SelCount][HairCode]);
		break;
	case JOBCODE_KNIGHT:
		strcpy_s(szHeadName, szModel_KnightFaceName[SelCount][HairCode]);
		break;
	case JOBCODE_MAGICIAN:
		strcpy_s(szHeadName, szModel_MagicanFaceName[SelCount][HairCode]);
		break;
	case JOBCODE_PRIESTESS:
		strcpy_s(szHeadName, szModel_PristessFaceName[SelCount][HairCode]);
		break;
	}

	strcpy_s(szbuff1, szArmorDirectory);
	lstrcat(szbuff1, szArmorName);
	strcpy_s(szbuff2, szHeadName);

	lpChar->PatLoading = TRUE;
	AddLoaderPattern(lpChar, szbuff1, szbuff2);

	return TRUE;
}

void ResetHairModel()
{
	char szDefaultHeadModel[260] = { 0 };

	char c[2] = { 0 };
	int r = (int)lpCurPlayer->smCharInfo.ChangeJob;
	if (r > 0)
		c[0] = ('a' + (r - 1));

	char cl[2] = { 0 };
	if (r == 2)
		cl[0] = '_';
	else
		cl[0] = '-';

	switch (lpCurPlayer->smCharInfo.JOB_CODE)
	{
	case 1:
		wsprintf(szDefaultHeadModel, "char//tmABCD//tmh%sB01%s.inf", cl, c);
		break;
	case 2:
		wsprintf(szDefaultHeadModel, "char//tmABCD//tmh%sA01%s.inf", cl, c);
		break;
	case 3:
		wsprintf(szDefaultHeadModel, "char//tmABCD//tfh%sD01%s.inf", cl, c);
		break;
	case 4:
		wsprintf(szDefaultHeadModel, "char//tmABCD//tmh%sC01%s.inf", cl, c);
		break;
	case 5:
		wsprintf(szDefaultHeadModel, "char//tmABCD//Mfh%sB01%s.inf", cl, c);
		break;
	case 6:
		wsprintf(szDefaultHeadModel, "char//tmABCD//Mmh%sA01%s.inf", cl, c);
		break;
	case 7:
		wsprintf(szDefaultHeadModel, "char//tmABCD//Mmh%sD01%s.inf", cl, c);
		break;
	case 8:
		wsprintf(szDefaultHeadModel, "char//tmABCD//Mfh%sC01%s.inf", cl, c);
		break;
	}

	char **lpSelFace = 0;
	char *lpSelFaceName = 0;

	int FindFaceValue = FindFaceSkin(lpCurPlayer, szDefaultHeadModel, &lpSelFace, &lpSelFaceName);

	if (FindFaceValue < 0)
		return;

	int HairCode = FindFaceValue & 0xFF;
	int FaceCode = FindFaceValue >> 8;

	lpCurPlayer->smCharInfo.ModelNameCode2 = GetHairSkinCode(FaceCode, HairCode);

	lstrcpy(lpCurPlayer->smCharInfo.szModelName2, szDefaultHeadModel);

	lpCurPlayer->PatLoading = TRUE;
	AddLoaderPattern(lpCurPlayer, lpCurPlayer->smCharInfo.szModelName, szDefaultHeadModel);
	lpCurPlayer->DisplayTools = TRUE;

	ReformCharForm();

	lpCurPlayer->smCharInfo.bUpdateInfo[0]++;
	SendSetObjectSerial(lpCurPlayer->dwObjectSerial);

	StartSkill(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ, 0, 0, 0, SKILL_UP1);
	SkillPlaySound(SKILL_SOUND_LEARN, lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ);

	SaveGameData();
}


#include "gameplay/legacy_support/PlaySubHairModelLegacyCommon.inl"




const int  CONST_AGINGBCOLOR_MAX = MAX_AGING;

static short	AgingBlinkColor[CONST_AGINGBCOLOR_MAX][6] =
{
{ 100,	 40,	 90,	0,	-1,	0 },//4
{ 120,	100,	 10,	0,	-1,	0 },//5
{ 0,	110,	 30,	0,	-1,	0 },//6
{ 0,	 50,	140,	0,	 0,	smTEXSTATE_FS_SCROLL5 },//7
{ 100,	  0,	 90,	0,	 1,	smTEXSTATE_FS_SCROLL6 },//8
{ 150,	 60,	  0,	0,	 2,	smTEXSTATE_FS_SCROLL6 },//9
{ 150,	 10,	  0,	0,	 3,	smTEXSTATE_FS_SCROLL8 },//10
{ 20,	220,	190,	0,	 4,	smTEXSTATE_FS_SCROLL8 },//11
{ 10,	220,	 30,	0,	 4,	smTEXSTATE_FS_SCROLL8 },//12
{ 170,	 40,	170,	0,	 4,	smTEXSTATE_FS_SCROLL9 },//13
{ 250,	 30,	160,	0,	 4,	smTEXSTATE_FS_SCROLL9 },//14
{ 30,	190,	255,	0,	 4,	smTEXSTATE_FS_SCROLL9 },//15
{ 250,	130,	 30,	0,	 4,	smTEXSTATE_FS_SCROLL9 },//16
{ 120,	 30,	 30,	0,	 4,	smTEXSTATE_FS_SCROLL10 },//17
{ 130,	  0,	255,	0,	 4,	smTEXSTATE_FS_SCROLL10 },//18
{ 220,	240,	 70,	0,	 4,	smTEXSTATE_FS_SCROLL10 },//19
{ 240,	240,	240,	0,	 9,	smTEXSTATE_FS_SCROLL10 },//20
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

				int Armor = ((dwCode&sinITEM_MASK3) >> 8);

				if (Armor > 25)
					Armor -= 16;

				SetPlayerArmor(Armor, (dwCode&sinITEM_MASK2));

				lpCurPlayer->wStickItems[smPLAYBUFF_ITEM_BODY] = GetSinItemNumFromCode(dwCode);
			}
			else
			{
				SetPlayerArmor(0, -1);
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
			SetPlayerArmor(-1, -1);
			lpCurPlayer->wStickItems[smPLAYBUFF_ITEM_BODY] = -2;
		}
		break;

	case INVENTORY_POS_COSTUME:

		if (cInvenTory.CostumeFlag)
		{
			dwMotionCode = lpCurPlayer->MotionInfo->State;
			FrameCounter = lpCurPlayer->FrameCounter;

			if (CODE != NULL)
			{
				if (SetFlag)
				{
					dwCode = CODE;

					int Armor = ((dwCode&sinITEM_MASK3) >> 8);

					if (Armor > 25)
						Armor -= 16;

					SetPlayerArmor(Armor, (dwCode&sinITEM_MASK2));

					lpCurPlayer->wStickItems[smPLAYBUFF_ITEM_BODY] = GetSinItemNumFromCode(dwCode);
				}
				else
				{
					SetPlayerArmor(0, -1);
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
				SetPlayerArmor(-1, -1);
				lpCurPlayer->wStickItems[smPLAYBUFF_ITEM_BODY] = -2;
			}
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

	if (lpCurPlayer->OnStageField >= 0 && StageField[lpCurPlayer->OnStageField]->FieldCode == 17)
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

	lpItemInfo = FindItemFromCode(sinBI2 | sin83);
	if (lpItemInfo)
	{
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


INT64 LevelTableSum = 23766931515015039;

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

	}
	return FALSE;
}


#include "gameplay/legacy_support/PlaySubExpValueLegacyCommon.inl"

bool bcanadd = false;

void setexp()
{
	bcanadd = true;
}

extern int NumLineComa64(INT64 Num, char* pBuff);

int AddExp(INT64 Exp)
{
	if (!bcanadd)
		return FALSE;


	INT64 exp64 = CodeXorCharInfo_Exp();
	/*int mlevel = CHAR_LEVEL_MAX;*/

	bool flag = false;
	// DEFINE O LEVEL MÁXIMO PARA GANHAR EXP NO CASO, 110.
	int mlevel = 118 - 1; //sergio

	if (exp64 < ExpLevelTable[mlevel])
	{
		exp64 += Exp;
		flag = true;
	}

	if (flag)
	{
		char szBuf[64] = { 0 };
		char szTemp[32] = { 0 };

		NumLineComa64(Exp, szTemp);

		wsprintf(szBuf, "> Ganhou %s de experi\352ncia!", szTemp);
		AddChatBuff(szBuf, 9);
	}

	SetExp64(&lpCurPlayer->smCharInfo, exp64);
	CodeXorCharInfo_Exp();
	ReformCharForm();

	int Level = lpCurPlayer->smCharInfo.Level;
	INT64 iexp = ExpLevelTable[Level];

	/*if(LevelTableSum != CheckLevelTable())
	{
		bcanadd = false;
		Log::getInstance()->output(LOG_DEBUG, "SetDisconnectFlag: 33");
		DisconnectFlag = GetCurrentTime();
		return FALSE;
	}*/

	while (iexp >= 0 && exp64 >= iexp && lpCurPlayer->smCharInfo.Level < CHAR_LEVEL_MAX)
	{

		lpCurPlayer->smCharInfo.Level++;
		lpCurPlayer->smCharInfo.Next_Exp = (int)ExpLevelTable[Level + 1];
		StartEffect(lpCurPlayer->pX, lpCurPlayer->pY + 32 * fONE, lpCurPlayer->pZ, EFFECT_LEVELUP1);
		esPlaySound(7, 400);
		ReformCharForm();

		SendPlayUpdateInfo();
		SaveGameData();

		Level = lpCurPlayer->smCharInfo.Level;
		iexp = ExpLevelTable[Level];

		extern cCHARSTATUS cCharStatus;
		cCharStatus.CheckLevel();
	}

	bcanadd = false;

	return TRUE;
}


#include "gameplay/legacy_support/PlaySubExpPenaltyLegacyCommon.inl"


int RainMaterial[4][8];


int InitWeatherSystem()
{
	if (SETTINGSHANDLER->getTextureQuality() >= 2)
	{
		return FALSE;
	}

	RainMaterial[0][0] = CreateTextureMaterial("game//sky//rain//ll.bmp", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ADDCOLOR);

	RainMaterial[1][0] = CreateTextureMaterial("game//sky//rain//45rain01.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);
	RainMaterial[1][1] = CreateTextureMaterial("game//sky//rain//45rain02.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);
	RainMaterial[1][2] = CreateTextureMaterial("game//sky//rain//45rain03.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);
	RainMaterial[1][3] = CreateTextureMaterial("game//sky//rain//45rain04.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);
	RainMaterial[1][4] = CreateTextureMaterial("game//sky//rain//45rain05.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);
	RainMaterial[1][5] = CreateTextureMaterial("game//sky//rain//45rain06.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);

	RainMaterial[2][0] = CreateTextureMaterial("game//sky//rain//80rain01.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);
	RainMaterial[2][1] = CreateTextureMaterial("game//sky//rain//80rain02.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);
	RainMaterial[2][2] = CreateTextureMaterial("game//sky//rain//80rain03.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);
	RainMaterial[2][3] = CreateTextureMaterial("game//sky//rain//80rain04.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);
	RainMaterial[2][4] = CreateTextureMaterial("game//sky//rain//80rain05.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);
	RainMaterial[2][5] = CreateTextureMaterial("game//sky//rain//80rain06.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);

	RainMaterial[3][0] = CreateTextureMaterial("game//sky//rain//90rain01.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);
	RainMaterial[3][1] = CreateTextureMaterial("game//sky//rain//90rain02.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);
	RainMaterial[3][2] = CreateTextureMaterial("game//sky//rain//90rain03.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);
	RainMaterial[3][3] = CreateTextureMaterial("game//sky//rain//90rain04.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);
	RainMaterial[3][4] = CreateTextureMaterial("game//sky//rain//90rain05.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);
	RainMaterial[3][5] = CreateTextureMaterial("game//sky//rain//90rain06.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);

	return TRUE;
}


extern int WeatherMode;

int DrawWeather(int ax)
{
	if (SETTINGSHANDLER->getTextureQuality() >= 2)
	{
		return FALSE;
	}

	smFACE2D face2d;

	int tm;
	float fx;
	int RainMode;

	if (ax < ANGLE_45 - 256) RainMode = 0;
	else
	{
		if (ax > ANGLE_45 + 256) RainMode = 3;
		else
		{
			if (ax > ANGLE_45) RainMode = 2;
			else RainMode = 1;
		}
	}

	if (WeatherMode == 2)
	{

		if (lpCurPlayer->OnStageField >= 0 &&
			(StageField[lpCurPlayer->OnStageField]->State == FIELD_STATE_DUNGEON ||
				StageField[lpCurPlayer->OnStageField]->State == FIELD_STATE_ROOM))
			return TRUE;






		smRender.Init();

		switch (RainMode)
		{
		case 0:
			tm = timeGetTime();
			tm = (tm >> 2) & 0x3FF;
			fx = ((float)tm) / 256;

			tm = timeGetTime();
			tm = (tm) & 0x3FF;
			fx = ((float)tm) / 256;

			face2d.TexRect.left = fx;
			face2d.TexRect.right = fx + 3;
			face2d.TexRect.top = fx;
			face2d.TexRect.bottom = fx + 0.5f;
			face2d.width = WinSizeX;
			face2d.height = WinSizeY;
			face2d.Transparency = 255;
			face2d.r = 255; face2d.g = 255; face2d.b = 255;
			face2d.x = 0;
			face2d.z = 0;
			face2d.y = 0;
			face2d.MatNum = RainMaterial[RainMode][0];
			smRender.AddFace2D_Direct(&face2d);

			break;

		case 1:
			tm = timeGetTime() >> 6;

			face2d.TexRect.left = 1;
			face2d.TexRect.right = 0;
			face2d.TexRect.top = 0;
			face2d.TexRect.bottom = 1;
			face2d.width = WinSizeX / 2;
			face2d.height = WinSizeY;
			face2d.Transparency = 160;
			face2d.r = 255; face2d.g = 255; face2d.b = 255;
			face2d.x = 0;
			face2d.z = 0;
			face2d.y = 0;
			face2d.MatNum = RainMaterial[RainMode][tm % 6];
			smRender.AddFace2D_Direct(&face2d);

			face2d.TexRect.left = 0;
			face2d.TexRect.right = 1;
			face2d.TexRect.top = 0;
			face2d.TexRect.bottom = 1;
			face2d.width = WinSizeX / 2;
			face2d.height = WinSizeY;
			face2d.Transparency = 160;
			face2d.r = 255; face2d.g = 255; face2d.b = 255;
			face2d.x = WinSizeX / 2;
			face2d.z = 0;
			face2d.y = 0;
			face2d.MatNum = RainMaterial[RainMode][(tm + 3) % 6];
			smRender.AddFace2D_Direct(&face2d);
			break;

		case 2:

			tm = timeGetTime() >> 6;

			face2d.TexRect.left = 1;
			face2d.TexRect.right = 0;
			face2d.TexRect.top = 0;
			face2d.TexRect.bottom = 1;
			face2d.width = WinSizeX / 2;
			face2d.height = WinSizeY;
			face2d.Transparency = 128;
			face2d.r = 255; face2d.g = 255; face2d.b = 255;
			face2d.x = 0;
			face2d.z = 0;
			face2d.y = 0;
			face2d.MatNum = RainMaterial[RainMode][tm % 6];
			smRender.AddFace2D_Direct(&face2d);

			face2d.TexRect.left = 0;
			face2d.TexRect.right = 1;
			face2d.TexRect.top = 0;
			face2d.TexRect.bottom = 1;
			face2d.width = WinSizeX / 2;
			face2d.height = WinSizeY;
			face2d.Transparency = 128;
			face2d.r = 255; face2d.g = 255; face2d.b = 255;
			face2d.x = WinSizeX / 2;
			face2d.z = 0;
			face2d.y = 0;
			face2d.MatNum = RainMaterial[RainMode][(tm + 3) % 6];
			smRender.AddFace2D_Direct(&face2d);
			break;

		case 3:
			tm = timeGetTime() >> 6;

			face2d.TexRect.left = 0;
			face2d.TexRect.right = 1;
			face2d.TexRect.top = 0;
			face2d.TexRect.bottom = 1;
			face2d.width = WinSizeX / 2;
			face2d.height = WinSizeY / 2;
			face2d.Transparency = 128;
			face2d.r = 255; face2d.g = 255; face2d.b = 255;
			face2d.x = 0;
			face2d.z = 0;
			face2d.y = 0;
			face2d.MatNum = RainMaterial[RainMode][tm % 6];
			smRender.AddFace2D_Direct(&face2d);

			face2d.TexRect.left = 1;
			face2d.TexRect.right = 0;
			face2d.TexRect.top = 0;
			face2d.TexRect.bottom = 1;
			face2d.width = WinSizeX / 2;
			face2d.height = WinSizeY / 2;
			face2d.Transparency = 128;
			face2d.r = 255; face2d.g = 255; face2d.b = 255;
			face2d.x = WinSizeX / 2;
			face2d.z = 0;
			face2d.y = 0;
			face2d.MatNum = RainMaterial[RainMode][(tm + 3) % 6];
			smRender.AddFace2D_Direct(&face2d);


			face2d.TexRect.left = 0;
			face2d.TexRect.right = 1;
			face2d.TexRect.top = 1;
			face2d.TexRect.bottom = 0;
			face2d.width = WinSizeX / 2;
			face2d.height = WinSizeY / 2;
			face2d.Transparency = 128;
			face2d.r = 255; face2d.g = 255; face2d.b = 255;
			face2d.x = 0;
			face2d.z = 0;
			face2d.y = WinSizeY >> 1;
			face2d.MatNum = RainMaterial[RainMode][(tm + 1) % 6];
			smRender.AddFace2D_Direct(&face2d);

			face2d.TexRect.left = 1;
			face2d.TexRect.right = 0;
			face2d.TexRect.top = 1;
			face2d.TexRect.bottom = 0;
			face2d.width = WinSizeX / 2;
			face2d.height = WinSizeY / 2;
			face2d.Transparency = 128;
			face2d.r = 255; face2d.g = 255; face2d.b = 255;
			face2d.x = WinSizeX / 2;
			face2d.z = 0;
			face2d.y = WinSizeY >> 1;
			face2d.MatNum = RainMaterial[RainMode][(tm + 4) % 6];
			smRender.AddFace2D_Direct(&face2d);

		}

		smRender.SetClipStates(0);
		smRender.ClipRendFace();
		smRender.RenderD3D();
	}

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
int	MatBossTime;
int	BPbar;
int	BPbox;
int	BPhud;
int Bpf;
int Bpp;
int bpinfo;


int InitFieldMap()
{
	ZeroMemory(sCompactMap, sizeof(sCOMPACT_MAP) * 4);
	MatArrow = CreateTextureMaterial("image//ARROW.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	MatBossTime = CreateTextureMaterial("game//boss//nextboss.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	MatMapBox = CreateTextureMaterial("image//MapBox.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	MatPartyPos = CreateTextureMaterial("image//party.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	MatNpcPos = CreateTextureMaterial("image//npc.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	BPbar = CreateTextureMaterial("game//images//battlepass//EXP_BAR_mini.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	BPhud = CreateTextureMaterial("game//images//battlepass//hud_mini.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	BPbox = CreateTextureMaterial("game//images//battlepass//box_mini.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	//Bpf = CreateTextureMaterial("game//images//shop//BPF.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	//Bpp = CreateTextureMaterial("game//images//shop//BPP.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	bpinfo = CreateTextureMaterial("game//images//battlepass//Balao_Info_BP.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);

	return TRUE;
}


#include "gameplay/legacy_support/PlaySubFieldMapLegacyCommon.inl"

int DrawFieldMap()
{ 
	SYSTEMTIME systemTime;
	GetLocalTime(&systemTime);
	extern int szBossTime;
	char szBuff2[32] = { 0 };
	char szBuff3[32] = { 0 };

	int px = 656 + (WinSizeX - 800);
	int py = 426 + (WinSizeY - 600);
	int sz = 0;
	int flCode = 0;
	int	dSpe = 0;
	extern int szBossTime;
	char szBuff[32] = { 0 };
	HDC	hdc = NULL;

	if (MatBossTime >= 0)
	{
		wsprintf(szBuff, "Boss Time: xx:%d", szBossTime);
		dsDrawTexImage(MatBossTime, px - 5, py - 40, 130, 23, 255, dSpe);
		SetFontTextColor(RGB(255, 0, 0));
		dsTextLineOut(px + 23, py - 36, szBuff, lstrlen(szBuff));
	}

	// Reuse the lateral Battle Pass artwork as the character EXP bar.
	// The old horizontal EXP texture remains disabled by FIX 073.
	if (BPbar >= 0 && lpCurPlayer)
	{
		int expPercent10 = 0; // tenths of a percent: 0..1000
		int level = lpCurPlayer->smCharInfo.Level;
		INT64 currentExp = GetXorExp64(&lpCurPlayer->smCharInfo);

		if (level > 0 && level <= CHAR_LEVEL_MAX)
		{
			INT64 levelStart = ExpLevelTable[level - 1];
			INT64 levelEnd = ExpLevelTable[level];
			INT64 levelSpan = levelEnd - levelStart;

			if (levelSpan > 0)
			{
				INT64 levelExp = currentExp - levelStart;
				if (levelExp < 0) levelExp = 0;
				if (levelExp > levelSpan) levelExp = levelSpan;
				expPercent10 = (int)(((double)levelExp * 1000.0) / (double)levelSpan);
			}
			else
			{
				expPercent10 = 1000;
			}
		}

		if (expPercent10 < 0) expPercent10 = 0;
		if (expPercent10 > 1000) expPercent10 = 1000;

		int expBarHeight = (132 * expPercent10) / 1000;

		dsDrawTexImage(BPhud, px - 44, py - 20, 22, 136, 255, dSpe);
		if (expBarHeight > 0)
			dsDrawTexImage(BPbar, px - 40, py + 114, 16, -expBarHeight, 255, dSpe);

		wsprintf(szBuff, "%d.%d%%", expPercent10 / 10, expPercent10 % 10);
		SetFontTextColor(RGB(255, 255, 255));
		dsTextLineOut(px - 47, py + 120, szBuff, lstrlen(szBuff));

		if (pCursorPos.x > px - 48 && pCursorPos.x < px - 20 && pCursorPos.y > py - 20 && pCursorPos.y < py + 116)
		{
			dsDrawTexImage(bpinfo, px - 90, py + 108, 74, 32, 255, dSpe);
			wsprintf(szBuff2, "EXP: %d.%d%%", expPercent10 / 10, expPercent10 % 10);
			wsprintf(szBuff3, "Nivel: %d", level);
			dsTextLineOut(px - 84, py + 110, szBuff2, lstrlen(szBuff2));
			dsTextLineOut(px - 84, py + 123, szBuff3, lstrlen(szBuff3));
		}
	}

	if (lpCurPlayer->dwTradeMsgCode)
		return FALSE;


	if (lpCurPlayer->OnStageField >= 0 &&
		(StageField[lpCurPlayer->OnStageField]->FieldCode == rsSOD_FIELD || StageField[lpCurPlayer->OnStageField]->FieldCode == QUEST_ARENA_FIELD ||
			StageField[lpCurPlayer->OnStageField]->State == FIELD_STATE_ACTION) || StageField[lpCurPlayer->OnStageField]->FieldCode == 36)
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




	FieldMapViewSize1 = FieldMapDrawSize[FieldMapDrawMode] * 64 * fONE;
	sz = FieldMapViewSize2;

	psSetDefaultRenderState(); // transparency in mini map


	dsDrawColorBox(D3DCOLOR_RGBA(0, 0, 0, 128), px, py, 128, 128);


	DrawFieldMap(&sCompactMap[1], px + 1, py + 1, sz);

	DrawFieldMap(&sCompactMap[0], px + 1, py + 1, sz);


	DrawMapNPC(px + 64, py + 64);


	if (cParty.GetMemberCount())
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



	return TRUE;
}



int	DrawMapPartyPlayer(int px, int py)
{
	return cParty.DrawMapPartyPlayers(px, py);

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
					dsDrawTexImageFloat(MatPartyPos, sx - 3, sy - 3, 8, 8, D3DCOLOR_RGBA(255, 0, 0, 255), 0);

			}
		}
	}

	return TRUE;
}



#include "gameplay/legacy_support/PlaySubMapDrawLegacyCommon.inl"

int psSetDefaultRenderState()
{



	return TRUE;
}

int psSetDefaultRenderState_PointFilter()
{


	return TRUE;
}


int psDrawTexImage(smTEXTUREHANDLE *hTexHandle, float x, float y, float w, float h,
	float fx, float fy, float fw, float fh, int Transp, int specular)
{
	D3DTLVERTEX tlVertex[4];
	int cnt;
	for (cnt = 0; cnt < 4; cnt++)

	{
		tlVertex[cnt].rhw = 1;
		tlVertex[cnt].color = D3DCOLOR_RGBA(255, 255, 255, Transp);
		tlVertex[cnt].specular = D3DCOLOR_RGBA(specular, specular, specular, 0);
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
	MagFilter = true;
	DrawTexture(hTexHandle->lpD3DTexture, tlVertex, true);
	MagFilter = false;
	return TRUE;
}

int psDrawTexImage_Point(smTEXTUREHANDLE *hTexHandle, float x, float y, float w, float h,
	float fx, float fy, float fw, float fh, int Transp, int specular)
{
	D3DTLVERTEX tlVertex[4];
	int cnt;
	for (cnt = 0; cnt < 4; cnt++)

	{
		tlVertex[cnt].rhw = 1;
		tlVertex[cnt].color = D3DCOLOR_RGBA(255, 255, 255, Transp);
		tlVertex[cnt].specular = D3DCOLOR_RGBA(specular, specular, specular, 0);
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
	DrawTexture(hTexHandle->lpD3DTexture, tlVertex, true);
	return TRUE;
}


int psDrawTexImage2(smTEXTUREHANDLE *hTexHandle, float *x, float *y, float fx, float fy, float fw, float fh, int Transp, int specular)
{
	D3DTLVERTEX tlVertex[4];
	int cnt;
	for (cnt = 0; cnt < 4; cnt++)

	{
		tlVertex[cnt].rhw = 1;
		tlVertex[cnt].color = D3DCOLOR_RGBA(255, 255, 255, Transp);
		tlVertex[cnt].specular = D3DCOLOR_RGBA(specular, specular, specular, 0);
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
	DrawTexture(hTexHandle->lpD3DTexture, tlVertex, true);
	return TRUE;
}



#include "gameplay/legacy_support/PlaySubCompressionLegacyCommon.inl"

#include "gameplay/legacy_support/PlaySubLegacyItemReformV10Common.inl"

#include "gameplay/legacy_support/PlaySubItemReformCoreLegacyCommon.inl"

#include "gameplay/legacy_support/PlaySubItemReformLifecycleLegacyCommon.inl"

#ifndef _GU
#define _GU

#include "systems/security/anticheat/AntiCheat.h"

#endif

extern AntiCheat * pcAntiCheat;

int CheckCharForm()
{
	int val;

	val = CheckCharForm(&lpCurPlayer->smCharInfo);

	pcAntiCheat->CheckThread();

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




	/*if (dwCode == sinDA1)
	{
		switch (lpItemInfo->CODE)
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
	else if (dwCode == sinDA2)
	{
		switch (lpItemInfo->CODE)
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
	else if (dwCode == sinDA3)
	{
		switch (lpItemInfo->CODE)
		{

		case (sinDA3 | sin01):
			fAborb += 0.3f;
			break;
		case (sinDA3 | sin02):
			fAborb += 0.6f;
			break;
		case (sinDA3 | sin03):
			fAborb += 1.0f;
			break;
		}
	}
	else if (dwCode == sinDA4)
	{
		switch (lpItemInfo->CODE)
		{

		case (sinDA4 | sin01):
			fAborb += 0.3f;
			break;
		case (sinDA4 | sin02):
			fAborb += 0.6f;
			break;
		case (sinDA4 | sin03):
			fAborb += 0.9f;
			break;
		}
	}

#ifdef _LANGUAGE_ENGLISH_NEXTUP

	else if (dwCode == sinDB1)
	{
		switch (lpItemInfo->CODE)
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
	else if (dwCode == sinDG1)
	{
		switch (lpItemInfo->CODE)
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
	else if (dwCode == sinDS1)
	{
		switch (lpItemInfo->CODE)
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

#endif*/
	return	fAborb;
}


#include "gameplay/legacy_support/PlaySubQuitCacheLegacyCommon.inl"

int	RequestPartyPlayers(smCHAR	*lpChar)
{
	return cParty.RequestPartyPlayers(lpChar);

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
	int cnt, cnt2, len;
	char	szBuff[64];
	char	*lpBuff;


	lpCharInfo->szModelName[31] = 0;
	lpCharInfo->szModelName2[31] = 0;

	len = lstrlen(lpCharInfo->szModelName);
	strcpy_s(szBuff, lpCharInfo->szModelName);
	for (cnt = len - 1; cnt >= 0; cnt--)
	{
		if (szBuff[cnt] == '/' || szBuff[cnt] == '\\')
		{
			szBuff[cnt + 1] = 0;
			break;
		}
	}

	if (lstrcmpi(szBuff, "char//tmABCD//") != 0) return FALSE;
	lpBuff = &lpCharInfo->szModelName[len - 3];
	if (lstrcmpi(lpBuff, "ini") != 0) return FALSE;


	switch (lpCharInfo->JOB_CODE)
	{
	case 0:
		return TRUE;

	case JOBCODE_FIGHTER:
		for (cnt = 0; cnt < _CHANGE_JOB_COUNT_MAX; cnt++)
		{
			for (cnt2 = 0; cnt2 < _FACE_COUNT_MAX; cnt2++)
			{
				if (lstrcmpi(szModel_FighterFaceName[cnt][cnt2], lpCharInfo->szModelName2) == 0)
				{
					return TRUE;
				}
			}
		}
		if (!lpCharInfo->szModelName2[0])
		{

			strcpy_s(lpCharInfo->szModelName2, szModel_FighterFaceName[0][0]);
			return TRUE;
		}
		break;
	case JOBCODE_MECHANICIAN:
		for (cnt = 0; cnt < _CHANGE_JOB_COUNT_MAX; cnt++)
		{
			for (cnt2 = 0; cnt2 < _FACE_COUNT_MAX; cnt2++)
			{
				if (lstrcmpi(szModel_MechFaceName[cnt][cnt2], lpCharInfo->szModelName2) == 0)
				{
					return TRUE;
				}
			}
		}
		if (!lpCharInfo->szModelName2[0])
		{

			strcpy_s(lpCharInfo->szModelName2, szModel_MechFaceName[0][0]);
			return TRUE;
		}
		break;
	case JOBCODE_ARCHER:
		for (cnt = 0; cnt < _CHANGE_JOB_COUNT_MAX; cnt++)
		{
			for (cnt2 = 0; cnt2 < _FACE_COUNT_MAX; cnt2++)
			{
				if (lstrcmpi(szModel_ArcherFaceName[cnt][cnt2], lpCharInfo->szModelName2) == 0)
				{
					return TRUE;
				}
			}
		}
		if (!lpCharInfo->szModelName2[0])
		{

			strcpy_s(lpCharInfo->szModelName2, szModel_ArcherFaceName[0][0]);
			return TRUE;
		}

		break;
	case JOBCODE_PIKEMAN:
		for (cnt = 0; cnt < _CHANGE_JOB_COUNT_MAX; cnt++)
		{
			for (cnt2 = 0; cnt2 < _FACE_COUNT_MAX; cnt2++)
			{
				if (lstrcmpi(szModel_PikeFaceName[cnt][cnt2], lpCharInfo->szModelName2) == 0)
				{
					return TRUE;
				}
			}
		}
		if (!lpCharInfo->szModelName2[0])
		{

			strcpy_s(lpCharInfo->szModelName2, szModel_PikeFaceName[0][0]);
			return TRUE;
		}
		break;
	case JOBCODE_ATALANTA:
		for (cnt = 0; cnt < _CHANGE_JOB_COUNT_MAX; cnt++)
		{
			for (cnt2 = 0; cnt2 < _FACE_COUNT_MAX; cnt2++)
			{
				if (lstrcmpi(szModel_AtalantaFaceName[cnt][cnt2], lpCharInfo->szModelName2) == 0)
				{
					return TRUE;
				}
			}
		}
		if (!lpCharInfo->szModelName2[0])
		{

			strcpy_s(lpCharInfo->szModelName2, szModel_AtalantaFaceName[0][0]);
			return TRUE;
		}
		break;
	case JOBCODE_KNIGHT:
		for (cnt = 0; cnt < _CHANGE_JOB_COUNT_MAX; cnt++)
		{
			for (cnt2 = 0; cnt2 < _FACE_COUNT_MAX; cnt2++)
			{
				if (lstrcmpi(szModel_KnightFaceName[cnt][cnt2], lpCharInfo->szModelName2) == 0)
				{
					return TRUE;
				}
			}
		}
		if (!lpCharInfo->szModelName2[0])
		{

			strcpy_s(lpCharInfo->szModelName2, szModel_KnightFaceName[0][0]);
			return TRUE;
		}
		break;
	case JOBCODE_MAGICIAN:
		for (cnt = 0; cnt < _CHANGE_JOB_COUNT_MAX; cnt++)
		{
			for (cnt2 = 0; cnt2 < _FACE_COUNT_MAX; cnt2++)
			{
				if (lstrcmpi(szModel_MagicanFaceName[cnt][cnt2], lpCharInfo->szModelName2) == 0)
				{
					return TRUE;
				}
			}
		}
		if (!lpCharInfo->szModelName2[0])
		{

			strcpy_s(lpCharInfo->szModelName2, szModel_MagicanFaceName[0][0]);
			return TRUE;
		}
		break;
	case JOBCODE_PRIESTESS:
		for (cnt = 0; cnt < _CHANGE_JOB_COUNT_MAX; cnt++)
		{
			for (cnt2 = 0; cnt2 < _FACE_COUNT_MAX; cnt2++)
			{
				if (lstrcmpi(szModel_PristessFaceName[cnt][cnt2], lpCharInfo->szModelName2) == 0)
				{
					return TRUE;
				}
			}
		}
		if (!lpCharInfo->szModelName2[0])
		{

			strcpy_s(lpCharInfo->szModelName2, szModel_PristessFaceName[0][0]);
			return TRUE;
		}
		break;
	}

	return FALSE;
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

			case OpCode::OPCODE_SUCCESS_DEFENSE:

				cInterFace.SetStringEffect(HA_DEFANCE);
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
	int	PotionCount = 0;

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
	// sin05 does not exist in current sCHK_POTION_COUNT structure
	// if (lpItem->CODE == (sinPL1 | sin05))
	// 	ChkPotion->Pl105 += PotionCount;


	if (lpItem->CODE == (sinPM1 | sin01))
		ChkPotion->Pm101 += PotionCount;
	if (lpItem->CODE == (sinPM1 | sin02))
		ChkPotion->Pm102 += PotionCount;
	if (lpItem->CODE == (sinPM1 | sin03))
		ChkPotion->Pm103 += PotionCount;
	if (lpItem->CODE == (sinPM1 | sin04))
		ChkPotion->Pm104 += PotionCount;
	// sin05 does not exist in current sCHK_POTION_COUNT structure
	// if (lpItem->CODE == (sinPM1 | sin05))
	// 	ChkPotion->Pm105 += PotionCount;


	if (lpItem->CODE == (sinPS1 | sin01))
		ChkPotion->Ps101 += PotionCount;
	if (lpItem->CODE == (sinPS1 | sin02))
		ChkPotion->Ps102 += PotionCount;
	if (lpItem->CODE == (sinPS1 | sin03))
		ChkPotion->Ps103 += PotionCount;
	if (lpItem->CODE == (sinPS1 | sin04))
		ChkPotion->Ps104 += PotionCount;
	// sin05 does not exist in current sCHK_POTION_COUNT structure
	// if (lpItem->CODE == (sinPS1 | sin05))
	// 	ChkPotion->Ps105 += PotionCount;

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

	if (cSkinChange.OpenFlag)
	{
		for (cnt = 0; cnt < 2; cnt++)
		{
			if (sSkinChangeItem.SkinChangeItem[cnt].Flag)
			{
				dwCode += GetItemCheckCode(&sSkinChangeItem.SkinChangeItem[cnt].sItemInfo);
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

	if (cSkinChange.OpenFlag)
	{
		for (cnt = 0; cnt < 2; cnt++)
		{
			if (sSkinChangeItem.SkinChangeItem[cnt].Flag)
			{
				lpItem = &sSkinChangeItem.SkinChangeItem[cnt].sItemInfo;
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
				D3DCOLOR_RGBA(bSOD_StringColor[0], bSOD_StringColor[1], bSOD_StringColor[2], 255), 2.0f);
		}
	}

	if (sSodScore.dwDispTime > dwPlayTime && rsCheckHardCoreEventArea(lpCurPlayer->pX, lpCurPlayer->pY, lpCurPlayer->pZ) == TRUE)
	{

		x = (WinSizeX / 2) - 80; y = 8;

		wsprintf(szBuff, "ROUND %02d", sSodScore.Round);
		DrawFontImage(szBuff, x - 40, y, D3DCOLOR_RGBA(80, 255, 40, 255), 0.9f);

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

		wsprintf(szBuff, "PlayTime %02d:%02d", tM, tS);
		DrawFontImage(szBuff, x + 160, y + 2, D3DCOLOR_RGBA(80, 160, 255, 255), 0.7f);

		y += 24;

		wsprintf(szBuff, "Score %d", sSodScore.Score);

		if (sSodScore.ScoreEffectCount)
			DrawFontImage(szBuff, x - 40, y + 8, D3DCOLOR_RGBA(255, 128, 64, 255 - sSodScore.ScoreEffectCount), 0.9f);
		else
			DrawFontImage(szBuff, x - 40, y + 8, D3DCOLOR_RGBA(255, 255, 0, 255), 0.9f);


		SodTeamSort();

		for (cnt = 0; cnt < 4; cnt++)
		{
			cnt2 = TeamScoreSort[cnt];

			if (sSodScore.MyTeam == cnt2)
				dwColor = D3DCOLOR_RGBA(255, 255, 0, 255);
			else
				dwColor = D3DCOLOR_RGBA(255, 128, 0, 255);

			DrawSpecialFontImage(TeamMarkSort[cnt2], x + 160, y, D3DCOLOR_RGBA(255, 255, 255, 255), 0.7f);

			wsprintf(szBuff, "Team %d", sSodScore.TeamScore[cnt2]);
			DrawFontImage(szBuff, x + 190, y, dwColor, 0.7f);
			y += 18;
		}
	}

	return FALSE;
}












