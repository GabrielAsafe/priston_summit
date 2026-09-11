

#include "systems/commerce/coin_shop/CoinShop.h"
#include "systems/commerce/coin_shop/CoinShop_T.h"
#include "systems/localization/legacy_catalog/language.h"
#include "compat/legacy/sin/sinLinkHeader.h"


cITEM cItem;

sITEM TempItem;
sITEM MouseItem;

sITEMREQUIRE sRequire;
POINT ItemBoxPosi;
POINT ItemBoxSize;
POINT TextSetPosi;


char	szInfoBuff[5000];
char    szInfoBuff2[5000];

int sinShowItemInfoFlag = 0;

int tWeaponClass = 0;
int AgingGageFlag = 0;
int AgingBarLenght = 0;
int NameMixLine = 0;

DWORD sinItemTime = 0;

int NotSell_Item_CODECnt = 0;
int NotSell_Item_MASKCnt = 0;
int NotSell_Item_KINDCnt = 0;

int NotDrow_Item_CODECnt = 0;
int NotDrow_Item_MASKCnt = 0;
int NotDrow_Item_KINDCnt = 0;

int NotSet_Item_CODECnt = 0;
int NotSet_Item_MASKCnt = 0;
int NotSet_Item_KINDCnt = 0;

int Ring1;
int Ring2;
int Amulet1;
int Amulet2;
int Efect1Line;
int Efect1Line2;
int Efect1Line3;
int Efect2Line;
int Efect2Line2;
int Efect2Line3;

int SetBoss = 0;
extern int LValRing;
extern int RValRing;
extern int KelAmulet;
extern int LShyRing;
extern int RShyRing;
extern int TullaAmulet;
extern int SetBonus1;
extern int SetBonus2;

DWORD NotSell_Item_CODE[] = { (sinQT1 | sin07),(sinQT1 | sin08),0 };
DWORD NotSell_Item_MASK[] = { 0 };
DWORD NotSell_Item_KIND[] = { ITEM_KIND_QUEST_WEAPON,0 };

DWORD NotDrow_Item_CODE[] = { (sinQT1 | sin07),(sinQT1 | sin08),0 };
DWORD NotDrow_Item_MASK[] = { 0 };
DWORD NotDrow_Item_KIND[] = { ITEM_KIND_QUEST_WEAPON,0 };

DWORD NotSet_Item_CODE[] = { (sinQT1 | sin07),(sinQT1 | sin08),0 };
DWORD NotSet_Item_MASK[] = { 0 };
DWORD NotSet_Item_KIND[] = { ITEM_KIND_QUEST_WEAPON,0 };


sITEM sItem[MAX_ITEM] = {
#include "item_catalog_recovered.inc"
};

cITEM::cITEM()
{

}

cITEM::~cITEM()
{

}

void cITEM::Init()
{
	for (int cnt = 0; cnt < MAX_ITEM; cnt++)
	{
		sItem[cnt].sItemInfo.CODE = sItem[cnt].CODE;
	}

	MatItemInfoBox = CreateTextureMaterial("Image//SinImage//Items//ItemInfoBox.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);

	MatItemInfoBox_TopLeft = CreateTextureMaterial("image//Sinimage//Items//ItemInfoBox//BoxTopLeft.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	MatItemInfoBox_TopRight = CreateTextureMaterial("image//Sinimage//Items//ItemInfoBox//BoxTopRight.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	MatItemInfoBox_TopCenter = CreateTextureMaterial("image//Sinimage//Items//ItemInfoBox//BoxTopCenter.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);

	MatItemInfoBox_BottomLeft = CreateTextureMaterial("image//Sinimage//Items//ItemInfoBox//BoxBottomLeft.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	MatItemInfoBox_BottomRight = CreateTextureMaterial("image//Sinimage//Items//ItemInfoBox//BoxBottomRight.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	MatItemInfoBox_BottomCenter = CreateTextureMaterial("image//Sinimage//Items//ItemInfoBox//BoxBottomCenter.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);

	MatItemInfoBox_Left = CreateTextureMaterial("image//Sinimage//Items//ItemInfoBox//BoxLeft.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	MatItemInfoBox_Right = CreateTextureMaterial("image//Sinimage//Items//ItemInfoBox//BoxRight.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	MatItemInfoBox_Center = CreateTextureMaterial("image//Sinimage//Items//ItemInfoBox//BoxCenter.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);

	Load();


	int CheckCount = 0;
	while (1)
	{

		if (NotSell_Item_CODE[NotSell_Item_CODECnt])
			NotSell_Item_CODECnt++;
		else
			CheckCount++;

		if (NotSell_Item_MASK[NotSell_Item_MASKCnt])
			NotSell_Item_MASKCnt++;
		else
			CheckCount++;

		if (NotSell_Item_KIND[NotSell_Item_KINDCnt])
			NotSell_Item_KINDCnt++;
		else
			CheckCount++;


		if (NotDrow_Item_CODE[NotDrow_Item_CODECnt])
			NotDrow_Item_CODECnt++;
		else
			CheckCount++;

		if (NotDrow_Item_MASK[NotDrow_Item_MASKCnt])
			NotDrow_Item_MASKCnt++;
		else
			CheckCount++;

		if (NotDrow_Item_KIND[NotDrow_Item_KINDCnt])
			NotDrow_Item_KINDCnt++;
		else
			CheckCount++;


		if (NotSet_Item_CODE[NotSet_Item_CODECnt])
			NotSet_Item_CODECnt++;
		else
			CheckCount++;

		if (NotSet_Item_MASK[NotSet_Item_MASKCnt])
			NotSet_Item_MASKCnt++;
		else
			CheckCount++;

		if (NotSet_Item_KIND[NotSet_Item_KINDCnt])
			NotSet_Item_KINDCnt++;
		else
			CheckCount++;

		if (CheckCount >= 9)
			break;

	}

	memset(g_Manufacture_ItemInfo, 0, sizeof(SManufacture_ResultItemInfo));
}

void cITEM::Load()
{
	AgingBar = LoadDibSurfaceOffscreen("image//Sinimage//shopall//Aging//Bar.bmp");
	AgingGage = LoadDibSurfaceOffscreen("image//Sinimage//shopall//Aging//Bar_.bmp");
	AgingBar2 = LoadDibSurfaceOffscreen("image//Sinimage//shopall//Aging//Aging.bmp");
}

void cITEM::Release()
{
	if (AgingBar)
	{
		AgingBar->Release();
		AgingBar = 0;
	}
	if (AgingGage)
	{
		AgingGage->Release();
		AgingGage = 0;
	}
	if (AgingBar2)
	{
		AgingBar2->Release();
		AgingBar2 = 0;
	}
}

void cITEM::Draw()
{
	int t = 0, i = 0;
	char TempBuff[64];
	int len = 0, len2 = 0;
	int tSetTextXposi = 0;
	int x, y;
	len = lstrlen(szInfoBuff);

	memset(&TempBuff, 0, sizeof(TempBuff));
	for (i = 0; i < len; i++)
	{
		if (szInfoBuff[i] == '/' && i + 1 < len && szInfoBuff[i + 1] == 'r')
		{
			memcpy(TempBuff, szInfoBuff, i);
			break;
		}

	}
	len2 = lstrlen(TempBuff);
	tSetTextXposi = (int)(ItemBoxPosi.x + (((ItemBoxSize.x * 16) / 2) - ((len2 * 5.5) / 2)) + (len2 * 5.5) + 15);

	if (MouseItem.Flag)
	{
		if (!MouseItem.lpItem)
			dsDrawColorBox(D3DCOLOR_RGBA(125, 125, 255, 125), MouseItem.x, MouseItem.y, MouseItem.w, MouseItem.h);

		x = pCursorPos.x - (MouseItem.w / 2);
		y = pCursorPos.y - (MouseItem.h / 2);

		dsDrawOffsetArray = dsARRAY_TOP;
		DrawSprite(x, y, MouseItem.lpItem, 0, 0, MouseItem.w, MouseItem.h);
		if ((MouseItem.sItemInfo.CODE & sinITEM_MASK2) == sinPZ1 || (MouseItem.sItemInfo.CODE & sinITEM_MASK2) == sinPZ2)
		{
			if (MouseItem.sItemInfo.PotionCount == 2)
			{
				if (MouseItem.w == 44)
					DrawSprite(x, y, cInvenTory.lpPuzzlewidth, 0, 0, 44, 22);
				if (MouseItem.w == 22)
					DrawSprite(x, y, cInvenTory.lpPuzzleHeight, 0, 0, 22, 44);
			}
		}

		if (cMyShop.OpenFlag)
			dsDrawColorBox(D3DCOLOR_RGBA(0, 0, 128, 125), x, y, MouseItem.w, MouseItem.h);
	}

	if (sinShowItemInfoFlag)
	{


		if (sinShowItemInfoFlag == 1)dsDrawOffsetArray = dsARRAY_TOP;
		else dsDrawOffsetArray = dsARRAY_BOTTOM;
		for (i = 0; i < ItemBoxSize.x; i++)
		{
			for (int j = 0; j < ItemBoxSize.y; j++)
			{
				if (i == 0 && j == 0)
					dsDrawTexImage(MatItemInfoBox_TopLeft, ItemBoxPosi.x + (i * 16), ItemBoxPosi.y + (j * 16), 16, 16, 255);
				if (j == 0 && i != 0 && i + 1 < ItemBoxSize.x)
					dsDrawTexImage(MatItemInfoBox_TopCenter, ItemBoxPosi.x + (i * 16), ItemBoxPosi.y + (j * 16), 16, 16, 255);
				if (j == 0 && i + 1 == ItemBoxSize.x)
					dsDrawTexImage(MatItemInfoBox_TopRight, ItemBoxPosi.x + (i * 16), ItemBoxPosi.y + (j * 16), 16, 16, 255);
				if (i == 0 && j != 0 && j + 1 != ItemBoxSize.y)
					dsDrawTexImage(MatItemInfoBox_Left, ItemBoxPosi.x + (i * 16), ItemBoxPosi.y + (j * 16), 16, 16, 255);
				if (i != 0 && j != 0 && i + 1 != ItemBoxSize.x && j + 1 != ItemBoxSize.y)
					dsDrawTexImage(MatItemInfoBox_Center, ItemBoxPosi.x + (i * 16), ItemBoxPosi.y + (j * 16), 16, 16, 255);
				if (i + 1 == ItemBoxSize.x && j != 0 && j + 1 != ItemBoxSize.y)
					dsDrawTexImage(MatItemInfoBox_Right, ItemBoxPosi.x + (i * 16), ItemBoxPosi.y + (j * 16), 16, 16, 255);
				if (i == 0 && j + 1 == ItemBoxSize.y)
					dsDrawTexImage(MatItemInfoBox_BottomLeft, ItemBoxPosi.x + (i * 16), ItemBoxPosi.y + (j * 16), 16, 16, 255);
				if (i != 0 && j + 1 == ItemBoxSize.y && i + 1 != ItemBoxSize.x)
					dsDrawTexImage(MatItemInfoBox_BottomCenter, ItemBoxPosi.x + (i * 16), ItemBoxPosi.y + (j * 16), 16, 16, 255);
				if (j + 1 == ItemBoxSize.y && i + 1 == ItemBoxSize.x)
					dsDrawTexImage(MatItemInfoBox_BottomRight, ItemBoxPosi.x + (i * 16), ItemBoxPosi.y + (j * 16), 16, 16, 255);
			}
		}

		if (tWeaponClass == 1)
			DrawSprite(tSetTextXposi, ItemBoxPosi.y + 13, cInvenTory.lpShowWeaponClass[0], 0, 0, 18, 16);

		if (tWeaponClass == 2)
			DrawSprite(tSetTextXposi, ItemBoxPosi.y + 13, cInvenTory.lpShowWeaponClass[1], 0, 0, 18, 16);

		if (tWeaponClass == 3)
			DrawSprite(tSetTextXposi, ItemBoxPosi.y + 13, cInvenTory.lpShowWeaponClass[2], 0, 0, 18, 16);

		if (tWeaponClass == 4)
			DrawSprite(tSetTextXposi, ItemBoxPosi.y + 13, cInvenTory.lpShowWeaponClass[3], 0, 0, 18, 16);

		if (AgingGageFlag)
		{
			if (AgingGageFlag == 1)
				DrawSprite(ItemBoxPosi.x + 43, ItemBoxPosi.y + 35, AgingBar, 0, 0, 130, 19);
			if (AgingGageFlag == 2)
				DrawSprite(ItemBoxPosi.x + 14 + 83, ItemBoxPosi.y + 35, AgingBar2, 0, 0, 23, 19);
			if (AgingBarLenght)
				DrawSprite(ItemBoxPosi.x + 45, ItemBoxPosi.y + 42, AgingGage, 0, 0, AgingBarLenght, 4);
		}
	}
}

void cITEM::Main()
{

	sinShowItemInfoFlag = 0;
	if (MouseItem.Flag)
	{
		if (!sinMessageBoxShowFlag)
		{
			MouseItem.x = pCursorPos.x - (MouseItem.w / 2);
			MouseItem.y = pCursorPos.y - (MouseItem.h / 2);
		}

	}
	if (cMyShop.OpenFlag)
	{
		if (MyShopMouseItem.Flag)
		{
			MyShopMouseItem.x = pCursorPos.x - (MyShopMouseItem.w / 2);
			MyShopMouseItem.y = pCursorPos.y - (MyShopMouseItem.h / 2);
		}
	}

	CheckItemTable();

}

void cITEM::GetMousePos(int* MouseX, int* MouseY)
{
	if (MouseItem.Flag)
	{
		if (!sinMessageBoxShowFlag)
		{
			*MouseX = pCursorPos.x - (MouseItem.w / 2);
			*MouseY = pCursorPos.y - (MouseItem.h / 2);
		}
	}
	if (cMyShop.OpenFlag)
	{
		if (MyShopMouseItem.Flag)
		{
			*MouseX = pCursorPos.x - (MyShopMouseItem.w / 2);
			*MouseY = pCursorPos.y - (MyShopMouseItem.h / 2);
		}
	}

}

void cITEM::Close()
{

}

void cITEM::LButtonDown(int x, int y)
{

}

void cITEM::LButtonUp(int x, int y)
{

}

void cITEM::RButtonDown(int x, int y)
{
	if (!MouseItem.Flag)
		CursorClass = 1;
}

void cITEM::RButtonUp(int x, int y)
{

}

void cITEM::KeyDown()
{

}

int sinLineCount = 0;
int RedLine[10] = { 0,0,0,0,0,0,0,0,0,0 };
int SpecialItemLine = 0;
int CountSpecialName = 0;
int   ItemInfoCol = 0;
int UniFlag = 0;
DWORD sinItemKindFlag = 0;
sITEMINFO sInfo;
sITEMPRICE sinItemPrice;


int RequireLine[10] = { 0,0,0,0,0,0,0,0,0 };
int MixItemLine[10] = { 0,0,0,0,0,0,0,0,0,0 };
int AgingItemLine[10] = { 0,0,0,0,0,0,0,0,0,0 };
int AgingLevel4 = 0;
int QuestItemLine = 0;
int MyShopItemPriceLine = 0;
int MyShopItemPriceLineRED = 0;
int MyShopItemPriceLineGREEN = 0;
int ItemPerfLine = 0;
int ItemSkinLine = 0;
int ItemBoss1 = 0;
int ItemBoss2 = 0;
int ItemBossLine = 0;
int ItemBossFlag = 0;
bool bItemInfoPerf = false;
int trajeM = 0, trajeF = 0;

struct MixTypeColor
{
	int	iMixType;
	int	iRow;
};

MixTypeColor sMixTypeColor = { 0 };

int cITEM::ShowItemInfo(sITEM* pItem, int Flag, int Index)
{
	int   cnt = 0;
	int   BlockRate = 0;
	int   sinSpeed = 0;
	int   sinAbsorb = 0;
	int   sinLeftSpot = 0;
	int   sinRightSpot = 0;
	int   sinRegen = 0;

	char	szTemp[64];
	char    szTemp2[64];
	tWeaponClass = 0;
	AgingGageFlag = 0;
	AgingBarLenght = 0;
	AgingLevel4 = 0;
	QuestItemLine = 0;

	int AgingCnt = 0;

	UniFlag = 0;
	sinItemKindFlag = 0;


	ItemInfoCol = 0;
	SpecialItemLine = 0;


	if (pItem->Class == ITEM_CLASS_WEAPON_ONE)
		tWeaponClass = 1;

	if (pItem->Class == ITEM_CLASS_WEAPON_TWO)
		tWeaponClass = 2;

	if ((pItem->sItemInfo.CODE & sinITEM_MASK2) == sinDA1)
		tWeaponClass = 3;
	if ((pItem->sItemInfo.CODE & sinITEM_MASK2) == sinDA3)
		tWeaponClass = 3;
	if ((pItem->sItemInfo.CODE & sinITEM_MASK2) == sinCA1)
		tWeaponClass = 3;
	if ((pItem->sItemInfo.CODE & sinITEM_MASK2) == sinCA5)
		tWeaponClass = 3;
	if ((pItem->sItemInfo.CODE & sinITEM_MASK2) == sinDA2)
		tWeaponClass = 4;
	if ((pItem->sItemInfo.CODE & sinITEM_MASK2) == sinDA4)
		tWeaponClass = 4;
	if ((pItem->sItemInfo.CODE & sinITEM_MASK2) == sinCA2)
		tWeaponClass = 4;
	if ((pItem->sItemInfo.CODE & sinITEM_MASK2) == sinCA6)
		tWeaponClass = 4;
	if ((pItem->sItemInfo.CODE & sinITEM_MASK2) == sinCA7)
		tWeaponClass = 4;

	if ((pItem->sItemInfo.CODE & sinITEM_MASK2) == sinOM1)
		tWeaponClass = 4;


	for (int t = 0; t < 10; t++)
	{
		RedLine[t] = 0;
		RequireLine[t] = 0;
	}
	for (int t = 0; t < 10; t++)
	{
		MixItemLine[t] = 0;
		AgingItemLine[t] = 0;

	}


	sRequire.rDexterity = 0;
	sRequire.rHealth = 0;
	sRequire.rLevel = 0;
	sRequire.rSpirit = 0;
	sRequire.rStrength = 0;
	sRequire.rTalent = 0;

	memset(&szTemp, 0, sizeof(szTemp));
	memset(&szTemp2, 0, sizeof(szTemp2));
	memset(&szInfoBuff, 0, sizeof(szInfoBuff));
	memset(&szInfoBuff2, 0, sizeof(szInfoBuff2));

	bItemInfoPerf = false;

	if (cCoinShop.isOpen() || cCoinShop_T.isOpen() || pItem->sItemInfo.SpecialItemFlag[2] != ITEM_KIND_SKIN && !bSendGetItemInfoPerf && (
		(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinWA1 || (pItem->sItemInfo.CODE & sinITEM_MASK2) == sinWC1 ||
		(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinWH1 || (pItem->sItemInfo.CODE & sinITEM_MASK2) == sinWM1 ||
		(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinWP1 || (pItem->sItemInfo.CODE & sinITEM_MASK2) == sinWS1 ||
		(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinWS2 || (pItem->sItemInfo.CODE & sinITEM_MASK2) == sinWT1 ||

		(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinDA1 || (pItem->sItemInfo.CODE & sinITEM_MASK2) == sinDB1 ||
		(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinDG1 || (pItem->sItemInfo.CODE & sinITEM_MASK2) == sinDS1 ||
		(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinDA2 || (pItem->sItemInfo.CODE & sinITEM_MASK2) == sinDA3 ||
		(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinDA4 ||

		(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinOA1 || (pItem->sItemInfo.CODE & sinITEM_MASK2) == sinOA2 ||
		(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinOM1 || (pItem->sItemInfo.CODE & sinITEM_MASK2) == sinOR1 ||
		(pItem->sItemInfo.CODE & sinITEM_MASK2) == sinOR2 || (pItem->sItemInfo.CODE & sinITEM_MASK2) == sinOS1))
	{
		std::unordered_map<DWORD, sITEMINFO>::iterator it = mapItemInfo.find(pItem->sItemInfo.CODE);

		if (it == mapItemInfo.end())
		{
			SendGetItemInfoPerf(pItem->sItemInfo.CODE);
			bSendGetItemInfoPerf = true;
		}
		else
		{
			memcpy(&sInfo, &it->second, sizeof(sITEMINFO));
			bItemInfoPerf = true;
		}
	}

	if (!bItemInfoPerf)
	{
		if (pItem->Class == ITEM_CLASS_WEAPON_TWO && pItem->ItemPosition)
			memcpy(&sInfo, &cInvenTory.InvenItem[sInven[0].ItemIndex - 1].sItemInfo, sizeof(sITEMINFO));
		else
			memcpy(&sInfo, &pItem->sItemInfo, sizeof(sITEMINFO));
	}


#ifdef	_LANGUAGE_CHINESE 
	{
		if (sInfo.CODE == (sinWA1 | sin20))
		{
			strcpy(sInfo.ItemName, ha100LVQuestItemName[0]);
		}

		if (sInfo.CODE == (sinWC1 | sin20))
		{
			strcpy(sInfo.ItemName, ha100LVQuestItemName[1]);
		}

		if (sInfo.CODE == (sinWH1 | sin21))
		{
			strcpy(sInfo.ItemName, ha100LVQuestItemName[2]);
		}

		if (sInfo.CODE == (sinWM1 | sin21))
		{
			strcpy(sInfo.ItemName, ha100LVQuestItemName[7]);
		}

		if (sInfo.CODE == (sinWP1 | sin21))
		{
			strcpy(sInfo.ItemName, ha100LVQuestItemName[3]);
		}

		if (sInfo.CODE == (sinWS1 | sin22))
		{
			strcpy(sInfo.ItemName, ha100LVQuestItemName[4]);
		}

		if (sInfo.CODE == (sinWS2 | sin23))
		{
			strcpy(sInfo.ItemName, ha100LVQuestItemName[5]);
		}

		if (sInfo.CODE == (sinWT1 | sin21))
		{
			strcpy(sInfo.ItemName, ha100LVQuestItemName[6]);
		}

	}
#endif
#ifdef	_LANGUAGE_ENGLISH 
	{
		if (sInfo.CODE == (sinWA1 | sin20))
		{
			strcpy(sInfo.ItemName, ha100LVQuestItemName[0]);
		}

		if (sInfo.CODE == (sinWC1 | sin20))
		{
			strcpy(sInfo.ItemName, ha100LVQuestItemName[1]);
		}

		if (sInfo.CODE == (sinWH1 | sin21))
		{
			strcpy(sInfo.ItemName, ha100LVQuestItemName[2]);
		}

		if (sInfo.CODE == (sinWM1 | sin21))
		{
			strcpy(sInfo.ItemName, ha100LVQuestItemName[7]);
		}

		if (sInfo.CODE == (sinWP1 | sin21))
		{
			strcpy(sInfo.ItemName, ha100LVQuestItemName[3]);
		}

		if (sInfo.CODE == (sinWS1 | sin22))
		{
			strcpy(sInfo.ItemName, ha100LVQuestItemName[4]);
		}

		if (sInfo.CODE == (sinWS2 | sin23))
		{
			strcpy(sInfo.ItemName, ha100LVQuestItemName[5]);
		}

		if (sInfo.CODE == (sinWT1 | sin21))
		{
			strcpy(sInfo.ItemName, ha100LVQuestItemName[6]);
		}



	}
#endif
#ifdef	_LANGUAGE_VEITNAM 
	{
		if (sInfo.CODE == (sinWA1 | sin20))
		{
			strcpy(sInfo.ItemName, ha100LVQuestItemName[0]);
		}

		if (sInfo.CODE == (sinWC1 | sin20))
		{
			strcpy(sInfo.ItemName, ha100LVQuestItemName[1]);
		}

		if (sInfo.CODE == (sinWH1 | sin21))
		{
			strcpy(sInfo.ItemName, ha100LVQuestItemName[2]);
		}

		if (sInfo.CODE == (sinWM1 | sin21))
		{
			strcpy(sInfo.ItemName, ha100LVQuestItemName[7]);
		}

		if (sInfo.CODE == (sinWP1 | sin21))
		{
			strcpy(sInfo.ItemName, ha100LVQuestItemName[3]);
		}

		if (sInfo.CODE == (sinWS1 | sin22))
		{
			strcpy(sInfo.ItemName, ha100LVQuestItemName[4]);
		}

		if (sInfo.CODE == (sinWS2 | sin23))
		{
			strcpy(sInfo.ItemName, ha100LVQuestItemName[5]);
		}

		if (sInfo.CODE == (sinWT1 | sin21))
		{
			strcpy(sInfo.ItemName, ha100LVQuestItemName[6]);
		}

	}
#endif

	if (sInfo.UniqueItem)
		UniFlag = 1;
	if (sInfo.ItemKindCode)
		sinItemKindFlag = sInfo.ItemKindCode;


	if (sInfo.CODE == (sinQT1 | sin06))
	{
		sinItemKindFlag = 100;
	}



	//Nome do Item
	if (tWeaponClass)
		wsprintf(szInfoBuff, "%s%s/r", sInfo.ItemName, "");
	else
		wsprintf(szInfoBuff, "%s/r", sInfo.ItemName);


	if ((sInfo.CODE & sinITEM_MASK2) == sinPZ1 || (sInfo.CODE & sinITEM_MASK2) == sinPZ2)
	{
		if (sInfo.PotionCount <= 1)
		{
			wsprintf(szInfoBuff, "%s/r", sinLuckyBox);

		}
	}


	strcpy_s(szInfoBuff2, "/r");
	ItemInfoCol++;


	if (sInfo.ItemKindCode == ITEM_KIND_AGING)
	{
		if (sInfo.ItemAgingCount[1]) AgingGageFlag = 1;
		else AgingGageFlag = 2;

		lstrcat(szInfoBuff, "/r/r");
		lstrcat(szInfoBuff2, "/r/r");
		ItemInfoCol++;
		ItemInfoCol++;
		if (sInfo.ItemAgingCount[0])
		{
			AgingBarLenght = (int)((float)((float)125 * (((float)sInfo.ItemAgingCount[0] / (float)sInfo.ItemAgingCount[1]))));


		}
		AgingLevel4 = sInfo.ItemAgingNum[0];

	}
	if (sInfo.ItemKindCode == ITEM_KIND_QUEST_WEAPON && sInfo.ItemAgingNum[0] < 4 &&
		(sInfo.CODE & sinITEM_MASK2) != sinOR2)
	{
		lstrcat(szInfoBuff, "/r");
		lstrcat(szInfoBuff2, "/r");
		ItemInfoCol++;

		wsprintf(szTemp, "%s:%d/%d  /r", QuestMonsterName[sInfo.ItemAgingNum[1]], sInfo.ItemAgingCount[1] - sInfo.ItemAgingCount[0], sInfo.ItemAgingCount[1]);
		lstrcat(szInfoBuff, szTemp);
		strcpy_s(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		QuestItemLine = ItemInfoCol;

		lstrcat(szInfoBuff, "/r");
		lstrcat(szInfoBuff2, "/r");
		ItemInfoCol++;


	}

	//Tipo do Mix no Item
	if (sInfo.ItemKindCode == ITEM_KIND_CRAFT && pItem->sItemInfo.iMixType > 0)
	{
		char* pszMixName = 0;

		switch (pItem->sItemInfo.iMixType)
		{
		case 1:
			pszMixName = "Mix Iniciante";
			break;
		case 2:
			pszMixName = "Mix Devine";
			break;
		case 3:
			pszMixName = "Mix Celesto";
			break;
		case 4:
			pszMixName = "Mix Bellum";
			break;
		case 5:
			pszMixName = "Mix Bellum";
			break;
		}

		if (pszMixName[0] != 0 && lstrlen(pszMixName) > 0)
		{
			wsprintf(szTemp, "%s/r/r", pszMixName);
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r/r");
			lstrcat(szInfoBuff2, szTemp2);

			sMixTypeColor.iMixType = pItem->sItemInfo.iMixType;
			sMixTypeColor.iRow = ItemInfoCol;

			ItemInfoCol++;
			ItemInfoCol++;
		}
	}

	if (pItem->CODE == (sinEC1 | sin01))
	{
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		strcpy_s(szTemp, TownName[0]);
		lstrcat(szInfoBuff, szTemp);
		strcpy_s(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

	}

	if (pItem->CODE == (sinEC1 | sin02))
	{
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		strcpy_s(szTemp, TownName[1]);
		lstrcat(szInfoBuff, szTemp);
		strcpy_s(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

	}
	if (pItem->CODE == (sinEC1 | sin04))
	{
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		strcpy_s(szTemp, TownName[2]);
		lstrcat(szInfoBuff, szTemp);
		strcpy_s(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

	}
	if (pItem->CODE == (sinEC1 | sin03))
	{
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		strcpy_s(szTemp, TownName[3]);
		lstrcat(szInfoBuff, szTemp);
		strcpy_s(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}
	if (pItem->CODE == (sinEC1 | sin06))
	{
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		strcpy_s(szTemp, TownName[4]);
		lstrcat(szInfoBuff, szTemp);
		strcpy_s(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}
	if (pItem->CODE == (sinEC1 | sin07))
	{
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		strcpy_s(szTemp, TownName[5]);
		lstrcat(szInfoBuff, szTemp);
		strcpy_s(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}
	trajeM = 0;
	//descri��o dos itens
	if (pItem->CODE == (sinCA1 | sin31) || pItem->CODE == (sinCA1 | sin35) || pItem->CODE == (sinCA1 | sin43) ||
		pItem->CODE == (sinCA1 | sin54) || pItem->CODE == (sinCA1 | sin66) || pItem->CODE == (sinCA1 | sin67) ||
		pItem->CODE == (sinCA1 | sin85) || pItem->CODE == (sinCA1 | sin86) || pItem->CODE == (sinCA1 | sin89) ||
		pItem->CODE == (sinCA1 | sin90) || pItem->CODE == (sinCA1 | sin93) || pItem->CODE == (sinCA1 | sin94) ||
		pItem->CODE == (sinCA2 | sin32) || pItem->CODE == (sinCA2 | sin35) || pItem->CODE == (sinCA2 | sin43) ||
		pItem->CODE == (sinCA2 | sin54) || pItem->CODE == (sinCA2 | sin66) || pItem->CODE == (sinCA2 | sin67) ||
		pItem->CODE == (sinCA2 | sin85) || pItem->CODE == (sinCA2 | sin86) || pItem->CODE == (sinCA2 | sin89) ||
		pItem->CODE == (sinCA2 | sin90) || pItem->CODE == (sinCA2 | sin93) || pItem->CODE == (sinCA2 | sin94) ||

		pItem->CODE == (sinCA5 | sin01) || pItem->CODE == (sinCA5 | sin03) || pItem->CODE == (sinCA5 | sin04) ||
		pItem->CODE == (sinCA5 | sin07) || pItem->CODE == (sinCA5 | sin09) || pItem->CODE == (sinCA5 | sin10) ||
		pItem->CODE == (sinCA5 | sin15) || pItem->CODE == (sinCA5 | sin17) || pItem->CODE == (sinCA5 | sin18) ||
		pItem->CODE == (sinCA5 | sin21) || pItem->CODE == (sinCA5 | sin22) || pItem->CODE == (sinCA5 | sin25) ||
		pItem->CODE == (sinCA5 | sin26) || pItem->CODE == (sinCA5 | sin31) || pItem->CODE == (sinCA5 | sin32) ||
		pItem->CODE == (sinCA5 | sin35) || pItem->CODE == (sinCA5 | sin36) || pItem->CODE == (sinCA5 | sin39) ||
		pItem->CODE == (sinCA5 | sin40) || pItem->CODE == (sinCA5 | sin43) || pItem->CODE == (sinCA5 | sin44) ||
		pItem->CODE == (sinCA5 | sin47) ||

		pItem->CODE == (sinCA5 | sin65) || pItem->CODE == (sinCA5 | sin67) || pItem->CODE == (sinCA5 | sin69) ||
		pItem->CODE == (sinCA5 | sin71) || pItem->CODE == (sinCA5 | sin73) || pItem->CODE == (sinCA5 | sin75) ||
		pItem->CODE == (sinCA5 | sin77) || pItem->CODE == (sinCA5 | sin80) || pItem->CODE == (sinCA5 | sin82) ||
		pItem->CODE == (sinCA5 | sin84) || pItem->CODE == (sinCA5 | sin86) ||

		pItem->CODE == (sinCA6 | sin01) || pItem->CODE == (sinCA6 | sin03) || pItem->CODE == (sinCA6 | sin04) ||
		pItem->CODE == (sinCA6 | sin07) || pItem->CODE == (sinCA6 | sin09) || pItem->CODE == (sinCA6 | sin10) ||
		pItem->CODE == (sinCA6 | sin15) || pItem->CODE == (sinCA6 | sin17) || pItem->CODE == (sinCA6 | sin18) ||
		pItem->CODE == (sinCA6 | sin21) || pItem->CODE == (sinCA6 | sin22) || pItem->CODE == (sinCA6 | sin25) ||
		pItem->CODE == (sinCA6 | sin26) || pItem->CODE == (sinCA6 | sin31) || pItem->CODE == (sinCA6 | sin32) ||
		pItem->CODE == (sinCA6 | sin35) || pItem->CODE == (sinCA6 | sin36) || pItem->CODE == (sinCA6 | sin39) ||
		pItem->CODE == (sinCA6 | sin40) || pItem->CODE == (sinCA6 | sin43) || pItem->CODE == (sinCA6 | sin44) ||
		pItem->CODE == (sinCA6 | sin47) ||


		pItem->CODE == (sinCA6 | sin65) || pItem->CODE == (sinCA6 | sin67) || pItem->CODE == (sinCA6 | sin69) ||
		pItem->CODE == (sinCA6 | sin71) || pItem->CODE == (sinCA6 | sin73) || pItem->CODE == (sinCA6 | sin75) ||
		pItem->CODE == (sinCA6 | sin77) || pItem->CODE == (sinCA6 | sin80) || pItem->CODE == (sinCA6 | sin82) ||
		pItem->CODE == (sinCA6 | sin84) || pItem->CODE == (sinCA6 | sin86)) {

		wsprintfA(szTemp, "%s/r", "Traje Masculino");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);

		ItemInfoCol++;
		trajeM = ItemInfoCol;

		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	trajeF = 0;

	if (pItem->CODE == (sinCA5 | sin02) || pItem->CODE == (sinCA5 | sin05) || pItem->CODE == (sinCA5 | sin06) ||
		pItem->CODE == (sinCA5 | sin08) || pItem->CODE == (sinCA5 | sin11) || pItem->CODE == (sinCA5 | sin11) ||
		pItem->CODE == (sinCA5 | sin16) || pItem->CODE == (sinCA5 | sin19) || pItem->CODE == (sinCA5 | sin20) ||
		pItem->CODE == (sinCA5 | sin23) || pItem->CODE == (sinCA5 | sin24) || pItem->CODE == (sinCA5 | sin27) ||
		pItem->CODE == (sinCA5 | sin28) || pItem->CODE == (sinCA5 | sin33) || pItem->CODE == (sinCA5 | sin34) ||
		pItem->CODE == (sinCA5 | sin37) || pItem->CODE == (sinCA5 | sin38) || pItem->CODE == (sinCA2 | sin41) ||
		pItem->CODE == (sinCA5 | sin42) || pItem->CODE == (sinCA5 | sin45) || pItem->CODE == (sinCA2 | sin46) ||
		pItem->CODE == (sinCA2 | sin48) || pItem->CODE == (sinCA5 | sin41) ||

		pItem->CODE == (sinCA2 | sin96) || pItem->CODE == (sinCA2 | sin91) || pItem->CODE == (sinCA2 | sin92) ||
		pItem->CODE == (sinCA2 | sin87) || pItem->CODE == (sinCA2 | sin88) || pItem->CODE == (sinCA2 | sin68) ||
		pItem->CODE == (sinCA2 | sin69) || pItem->CODE == (sinCA2 | sin55) || pItem->CODE == (sinCA2 | sin34) ||
		pItem->CODE == (sinCA2 | sin37) || pItem->CODE == (sinCA2 | sin45) || pItem->CODE == (sinCA1 | sin33) ||
		pItem->CODE == (sinCA1 | sin37) || pItem->CODE == (sinCA1 | sin45) || pItem->CODE == (sinCA1 | sin55) ||
		pItem->CODE == (sinCA1 | sin91) || pItem->CODE == (sinCA1 | sin92) || pItem->CODE == (sinCA1 | sin87) ||
		pItem->CODE == (sinCA1 | sin88) || pItem->CODE == (sinCA1 | sin68) || pItem->CODE == (sinCA1 | sin69) ||
		pItem->CODE == (sinCA1 | sin95) || pItem->CODE == (sinCA1 | sin96) ||

		pItem->CODE == (sinCA6 | sin02) || pItem->CODE == (sinCA6 | sin05) || pItem->CODE == (sinCA6 | sin06) ||
		pItem->CODE == (sinCA6 | sin08) || pItem->CODE == (sinCA6 | sin11) || pItem->CODE == (sinCA6 | sin11) ||
		pItem->CODE == (sinCA6 | sin16) || pItem->CODE == (sinCA6 | sin19) || pItem->CODE == (sinCA6 | sin20) ||
		pItem->CODE == (sinCA6 | sin23) || pItem->CODE == (sinCA6 | sin24) || pItem->CODE == (sinCA6 | sin27) ||
		pItem->CODE == (sinCA6 | sin28) || pItem->CODE == (sinCA6 | sin33) || pItem->CODE == (sinCA6 | sin34) ||
		pItem->CODE == (sinCA6 | sin37) || pItem->CODE == (sinCA6 | sin38) || pItem->CODE == (sinCA6 | sin41) ||
		pItem->CODE == (sinCA6 | sin42) || pItem->CODE == (sinCA6 | sin45) || pItem->CODE == (sinCA6 | sin46) ||
		pItem->CODE == (sinCA6 | sin48) ||

		pItem->CODE == (sinCA5 | sin66) || pItem->CODE == (sinCA5 | sin68) || pItem->CODE == (sinCA5 | sin70) ||
		pItem->CODE == (sinCA5 | sin72) || pItem->CODE == (sinCA5 | sin74) || pItem->CODE == (sinCA5 | sin76) ||
		pItem->CODE == (sinCA5 | sin78) || pItem->CODE == (sinCA5 | sin81) || pItem->CODE == (sinCA5 | sin83) ||
		pItem->CODE == (sinCA5 | sin85) || pItem->CODE == (sinCA5 | sin87) ||

		pItem->CODE == (sinCA6 | sin66) || pItem->CODE == (sinCA6 | sin68) || pItem->CODE == (sinCA6 | sin70) ||
		pItem->CODE == (sinCA6 | sin72) || pItem->CODE == (sinCA6 | sin74) || pItem->CODE == (sinCA6 | sin76) ||
		pItem->CODE == (sinCA6 | sin78) || pItem->CODE == (sinCA6 | sin81) || pItem->CODE == (sinCA6 | sin83) ||
		pItem->CODE == (sinCA6 | sin85) || pItem->CODE == (sinCA6 | sin87)) {

		wsprintfA(szTemp, "%s/r", "Traje Feminino");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);

		ItemInfoCol++;
		trajeF = ItemInfoCol;

		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinGB1 | sin01)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Vale 100k ao ser/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "vendido no NPC./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinGB1 | sin02)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Vale 500k ao ser/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "vendido no NPC./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinGB1 | sin03)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Vale 250k ao ser/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "vendido no NPC./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinGB1 | sin04)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Vale 25kk ao ser/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "vendido no NPC./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI1 | sin08)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Use para teletransporta-se /r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "para v\341rios mapas./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBC1 | sin21) || pItem->CODE == (sinBC1 | sin22) || pItem->CODE == (sinBC1 | sin23)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Regenera\347\343o de HP em 15%/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBC1 | sin24) || pItem->CODE == (sinBC1 | sin25) || pItem->CODE == (sinBC1 | sin26)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Regenera\347\343o de MP em 15%/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBC1 | sin27) || pItem->CODE == (sinBC1 | sin28) || pItem->CODE == (sinBC1 | sin29)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Regenera\347\343o de RES em 15%/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI1 | sin45)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Redistribui os pontos de for\347a/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI1 | sin46)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Redistribui os pontos de intelig\352ncia/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI1 | sin47)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Redistribui os pontos de Talento/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI1 | sin48)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Redistribui os pontos de Agilidade/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI1 | sin49)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Redistribui os pontos de Vitalidade/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI1 | sin39)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Treina em 100% todas as habilidades/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "da primeira coroa/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI1 | sin40)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Treina em 100% todas as habilidades/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "da segunda coroa/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI1 | sin41)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Treina em 100% todas as habilidades/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "da terceira coroa/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if ((sInfo.CODE & sinITEM_MASK2) == sinGP1) {
		if (pItem->CODE == (sinGP1 | sin09)) {
			lstrcpy(szTemp, "/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;

			lstrcpy(szTemp, "Invoca um monstro aliado/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "para auxiliar nas batalhas./r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "Poss\355veis monstros: Hopi,/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "Duende Macabro, Decoy, Bargon,/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "Decapitador, Figon, Rei Hopi, Hulk./r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
		}
		else if (pItem->CODE == (sinGP1 | sin17)) {
			lstrcpy(szTemp, "/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;

			lstrcpy(szTemp, "Invoca um monstro aliado/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "para auxiliar nas batalhas./r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "Poss\355veis monstros:/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "Arqueiro do Mal, Patrulheiro do Mal,/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "Guerreiro do Mal, Cavaleiro do Mal./r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
		}
		else if (pItem->CODE == (sinGP1 | sin18)) {
			lstrcpy(szTemp, "/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;

			lstrcpy(szTemp, "Invoca um monstro aliado/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "para auxiliar nas batalhas./r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "Poss\355veis monstros:/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "Naz, Duende Guerreiro,/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "Sereia Profana, M\341quina-D./r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
		}
		else if (pItem->CODE == (sinGP1 | sin19)) {
			lstrcpy(szTemp, "/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;

			lstrcpy(szTemp, "Invoca um monstro aliado/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "para auxiliar nas batalhas./r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "Poss\355veis monstros:/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "Ratoo, Grotesco,/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "Punhos de A\347o, Gelo./r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
		}
		else if (pItem->CODE == (sinGP1 | sin20)) {
			lstrcpy(szTemp, "/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;

			lstrcpy(szTemp, "Invoca um monstro aliado/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "para auxiliar nas batalhas./r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "Poss\355veis monstros:/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "Omicron, Aranha Rainha,/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "Montanha, Omega./r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
		}
		else if (pItem->CODE == (sinGP1 | sin23)) {
			lstrcpy(szTemp, "/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;

			lstrcpy(szTemp, "Invoca um monstro aliado/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "para auxiliar nas batalhas,/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "Poss\355veis monstros:/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "Quimera, P\340ssaro Demon\355aco,/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "Cavaleiro Sanguin\341rio, Seto/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
		}
		else if (pItem->CODE == (sinGP1 | sin24)) {
			lstrcpy(szTemp, "/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;

			lstrcpy(szTemp, "Invoca um monstro aliado/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "para auxiliar nas batalhas,/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "Poss\355veis monstros:/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "Olho de Gelo, Golem de Gelo,/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "Tartaruga Blindada, Valento,/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "Verme de Fogo, Mokova,/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "Guardi\343o do Templo/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
		}
		else if (pItem->CODE == (sinGP1 | sin25)) {
			lstrcpy(szTemp, "/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "Invoca um monstro aliado/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "para auxiliar nas batalhas,/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "Poss\355veis monstros:/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "Naz das Cinzas, Gorgon,/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "Arqueira das Cinzas,/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "Guardi\343o do Templo/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
		}
		else if (pItem->CODE == (sinGP1 | sin26)) {
			lstrcpy(szTemp, "/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;

			lstrcpy(szTemp, "Invoca um monstro aliado/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "para auxiliar nas batalhas,/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "Poss\355veis monstros:/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "PS, AS, PRS, MG, ATA, FS/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "KS e MS das Cinzas/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;

		}
		else if (pItem->CODE == (sinGP1 | sin27)) {
			lstrcpy(szTemp, "/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;

			lstrcpy(szTemp, "Invoca um monstro aliado/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "para auxiliar nas batalhas,/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "Poss\355veis monstros:/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "Golem de Metal, Wolverine,/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "Chama Elemental/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;

		}
		else if (pItem->CODE == (sinGP1 | sin35)) {
			lstrcpy(szTemp, "/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;

			lstrcpy(szTemp, "Invoca o Boss F\372ria/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "para auxiliar nas batalhas/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
		}
		else if (pItem->CODE == (sinGP1 | sin36)) {
			lstrcpy(szTemp, "/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;

			lstrcpy(szTemp, "Invoca o Boss Babel/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "para auxiliar nas batalhas/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
		}
		else if (pItem->CODE == (sinGP1 | sin37)) {
			lstrcpy(szTemp, "/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;

			lstrcpy(szTemp, "Invoca o Boss Valento/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "para auxiliar nas batalhas/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
		}
		else if (pItem->CODE == (sinGP1 | sin38)) {
			lstrcpy(szTemp, "/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;

			lstrcpy(szTemp, "Invoca o Boss Kelvezu/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "para auxiliar nas batalhas/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
		}
		else if (pItem->CODE == (sinGP1 | sin39)) {
			lstrcpy(szTemp, "/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;

			lstrcpy(szTemp, "Invoca o Boss Mokova/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "para auxiliar nas batalhas/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
		}
		else if (pItem->CODE == (sinGP1 | sin40)) {
			lstrcpy(szTemp, "/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;

			lstrcpy(szTemp, "Invoca o Boss Rob\364 Mutante/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "para auxiliar nas batalhas/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
		}
		else if (pItem->CODE == (sinGP1 | sin41)) {
			lstrcpy(szTemp, "/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;

			lstrcpy(szTemp, "Invoca o Boss DevilShy/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "para auxiliar nas batalhas/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
		}
		else {
			lstrcpy(szTemp, "/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;

			lstrcpy(szTemp, "Invoca um monstro aliado/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			lstrcpy(szTemp, "para auxiliar nas batalhas/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
		}
	}

	if ((sInfo.CODE & sinITEM_MASK2) == sinQW1) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Aumenta o alcance do Portal./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinSE1 | sin01)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Item usado no processo/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "de Re-Especializa\347\343o./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "Utilize 2 Pedra./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinSE1 | sin02)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Item usado no processo/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "de Re-Especializa\347\343o./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "Utilize 2 Pedras./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinSE1 | sin03)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Item usado no processo/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "de Re-Especializa\347\343o./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "Utilize 2 Pedras./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinSE1 | sin04)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Item usado no processo/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "de Re-Especializa\347\343o./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "Utilize 2 Pedras./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinSE1 | sin13)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Item usado no processo/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "de Re-Especializa\347\343o./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "Utilize 2 Pedras./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI1 | sin89)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Item usado no processo/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "de revers\343o do mix ou aging./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI1 | sin90)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Item usado no processo/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "de Re-Especializa\347\343o para obter/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "status 100% perfeitos./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (((sInfo.CODE & sinITEM_MASK2) == sinQT1) || ((sInfo.CODE & sinITEM_MASK2) == sinMA1) || ((sInfo.CODE & sinITEM_MASK2) == sinMA2)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "Item de Desafio./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinQT1 | sin06)) {
		lstrcpy(szTemp, "Usado para o desafio Nv. 40./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinQT1 | sin09) || pItem->CODE == (sinQT1 | sin10) || pItem->CODE == (sinQT1 | sin11)) {
		lstrcpy(szTemp, "Usado para o desafio Nv. 80./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinQT1 | sin07)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "Adiciona 40 pontos de Vida./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinQT1 | sin08)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "Reduz para 2%% a penalidade/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "de Experi\352ncia ao morrer./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinCH1 | sin01)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Usa a Habilidade Vida Virtual/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "da Sacerdotisa no N\355vel 4./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "Apenas personagens Masculinos/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "podem usar este item./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinCH1 | sin02)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Usa a Habilidade Vida Virtual/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "da Sacerdotisa no N\355vel 8./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "Apenas personagens Masculinos/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "podem usar este item./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinCH1 | sin03)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Usa a Habilidade Vida Virtual/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "da Sacerdotisa no N\355vel 4./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "Apenas personagens Femininos/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "podem usar este item./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinCH1 | sin04)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Usa a Habilidade Vida Virtual/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "da Sacerdotisa no N\355vel 8./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "Apenas personagens Femininos/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "podem usar este item./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI1 | sin10)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Reduz a chance de/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "Falha do processo de/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI1 | sin11)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Reduz a chance de/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "Quebra do processo de/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI1 | sin60)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Impede o item de/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "quebrar no processo de/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "Aging em 100%./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI1 | sin61)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Impede o item de/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "falhar no processo de/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "Aging em 100%./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI1 | sin85)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Concede 100% de sucesso/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "no processo de/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "Aging do item./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI1 | sin16)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Muda o cabelo do personagem./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "Tipo de Cabelo: A/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI1 | sin17)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Muda o cabelo do personagem./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "Tipo de Cabelo: B/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI1 | sin18)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Muda o cabelo do personagem./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "Tipo de Cabelo: C/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI2 | sin13)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Muda o cabelo do personagem./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "Tipo de Cabelo: D/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI2 | sin14)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Muda o cabelo do personagem./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "Tipo de Cabelo: E/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI2 | sin15)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Muda o cabelo do personagem./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "Tipo de Cabelo: F/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI2 | sin16)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Muda o cabelo do personagem./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "Tipo de Cabelo: G/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI2 | sin17)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Muda o cabelo do personagem./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "Tipo de Cabelo: H/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI2 | sin18)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Muda o cabelo do personagem./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "Tipo de Cabelo: I/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI2 | sin19)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Muda o cabelo do personagem./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "Tipo de Cabelo: J/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI2 | sin20)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Muda o cabelo do personagem./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "Tipo de Cabelo: K/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI2 | sin21)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Muda o cabelo do personagem./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "Tipo de Cabelo: L/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI2 | sin22)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Muda o cabelo do personagem./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "Tipo de Cabelo: M/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI2 | sin40)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Muda o cabelo do personagem./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "Tipo de Cabelo: N/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI2 | sin41)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Muda o cabelo do personagem./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "Tipo de Cabelo: O/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI2 | sin42)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Muda o cabelo do personagem./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "Tipo de Cabelo: P/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI2 | sin43)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Muda o cabelo do personagem./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "Tipo de Cabelo: Q/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI2 | sin44)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Muda o cabelo do personagem./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "Tipo de Cabelo: R/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI2 | sin47)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Muda o cabelo do personagem./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "Tipo de Cabelo: S/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI2 | sin48)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Muda o cabelo do personagem./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "Tipo de Cabelo: T/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI2 | sin49)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Muda o cabelo do personagem./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "Tipo de Cabelo: U/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI2 | sin98)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Use para upar 1 Level./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI3 | sin01)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Pr\352mio para quem sobreviveu/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "ao Hell's Gate./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI3 | sin02)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Set de Sheltons necess\341rios para/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "a realiza\347\343o do processo/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "de Aging do n\355vel +10./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "O Pacote possui:/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Sparks/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Raidents/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Transparos/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Murkis/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Devines/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Celestos/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "01 Pedra de Aging/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "01 Ouro M\341gico/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Os Sheltons podem vir com/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Especializa\347\343o aleat\363ria/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "ou sem Especializa\347\343o./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI3 | sin03)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Set de Sheltons necess\341rios para/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "a realiza\347\343o do processo/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "de Aging do n\355vel +11./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "O Pacote possui:/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Raidents/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Transparos/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Murkis/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Devines/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Celestos/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "01 Mirage/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "01 Pedra de Aging/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "01 Ouro M\341gico/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Os Sheltons podem vir com/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Especializa\347\343o aleat\363ria/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "ou sem Especializa\347\343o./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI3 | sin04)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Set de Sheltons necess\341rios para/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "a realiza\347\343o do processo/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "de Aging do n\355vel +12./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "O Pacote possui:/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Raidents/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Transparos/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Murkis/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Devines/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Celestos/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Mirages/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "01 Pedra de Aging/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "01 Ouro M\341gico/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Os Sheltons podem vir com/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Especializa\347\343o aleat\363ria/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "ou sem Especializa\347\343o./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI3 | sin05)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Set de Sheltons necess\341rios para/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "a realiza\347\343o do processo/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "de Aging do n\355vel +13./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "O Pacote possui:/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Transparos/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Murkis/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Devines/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Celestos/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Mirages/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "01 Inferna/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "01 Pedra de Aging/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "01 Ouro M\341gico/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Os Sheltons podem vir com/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Especializa\347\343o aleat\363ria/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "ou sem Especializa\347\343o./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI3 | sin06)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Set de Sheltons necess\341rios para/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "a realiza\347\343o do processo/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "de Aging do n\355vel +14./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "O Pacote possui:/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Transparos/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Murkis/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Devines/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Celestos/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Mirages/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Infernas/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "01 Pedra de Aging/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "01 Ouro M\341gico/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Os Sheltons podem vir com/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Especializa\347\343o aleat\363ria/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "ou sem Especializa\347\343o./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI3 | sin07)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Set de Sheltons necess\341rios para/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "a realiza\347\343o do processo/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "de Aging do n\355vel +15./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "O Pacote possui:/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Murkis/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Devines/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Celestos/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Mirages/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Infernas/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "01 Enigma/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "01 Pedra de Aging/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "01 Ouro M\341gico/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Os Sheltons podem vir com/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Especializa\347\343o aleat\363ria/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "ou sem Especializa\347\343o./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI3 | sin08)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Set de Sheltons necess\341rios para/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "a realiza\347\343o do processo/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "de Aging do n\355vel +16./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "O Pacote possui:/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Murkis/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Devines/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Celestos/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Mirages/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Infernas/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Enigmas/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "01 Pedra de Aging/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "01 Ouro M\341gico/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Os Sheltons podem vir com/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Especializa\347\343o aleat\363ria/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "ou sem Especializa\347\343o./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI3 | sin09)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Set de Sheltons necess\341rios para/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "a realiza\347\343o do processo/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "de Aging do n\355vel +17./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "O Pacote possui:/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Devines/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Celestos/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Mirages/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Infernas/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Enigmas/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "01 Bellum/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "01 Pedra de Aging/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "01 Ouro M\341gico/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Os Sheltons podem vir com/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Especializa\347\343o aleat\363ria/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "ou sem Especializa\347\343o./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI3 | sin10)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Set de Sheltons necess\341rios para/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "a realiza\347\343o do processo/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "de Aging do n\355vel +18./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "O Pacote possui:/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Devines/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Celestos/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Mirages/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Infernas/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Enigmas/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Bellums/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "01 Pedra de Aging/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "01 Ouro M\341gico/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Os Sheltons podem vir com/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Especializa\347\343o aleat\363ria/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "ou sem Especializa\347\343o./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI3 | sin11)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Set de Sheltons necess\341rios para/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "a realiza\347\343o do processo/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "de Aging do n\355vel +19./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "O Pacote possui:/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Celestos/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Mirages/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Infernas/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Enigmas/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Bellums/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "01 Ordo/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "01 Pedra de Aging/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "01 Ouro M\341gico/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Os Sheltons podem vir com/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Especializa\347\343o aleat\363ria/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "ou sem Especializa\347\343o./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI3 | sin12)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Set de Sheltons necess\341rios para/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "a realiza\347\343o do processo/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "de Aging do n\355vel +20./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "O Pacote possui:/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Celestos/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Mirages/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Infernas/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Enigmas/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Bellums/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "02 Ordos/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "01 Pedra de Aging/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "01 Ouro M\341gico/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Os Sheltons podem vir com/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Especializa\347\343o aleat\363ria/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "ou sem Especializa\347\343o./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI3 | sin13)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Caixa de itens premiuns Open-beta/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI3 | sin14)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Pacote de itens premiuns 12H/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "O Pacote possui:/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "01 Olho m\341gico 50% 12H/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "01 Po\347\343o de experi\352ncia 30% 12H/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "01 Presa de Vampiro 12H/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "01 Drena Almas 12H/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "01 Po\347\343o Redut. de MANA 30% 12H/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "01 Po\347\343o Redut. de RES 30% 12H/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "01 F\352nix 12H/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI3 | sin15)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Pacote de itens premiuns 1D/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "O Pacote possui:/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "01 Olho m\341gico 50% 1D/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "01 Po\347\343o de experi\352ncia 50% 1D/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "01 Presa de Vampiro 1D/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "01 Drena Almas 1D/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "01 Po\347\343o Redut. de MANA 30% 1D/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "01 Po\347\343o Redut. de RES 30% 1D/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "01 F\352nix 1D/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI3 | sin16)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "premia\347\343o pra os que ajudou na beta/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI3 | sin30)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Abre O Bau Em qualquer Lugar/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI3 | sin31)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Abre O Bau Em qualquer Lugar/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}



	if (pItem->CODE == (sinBI3 | sin17) || pItem->CODE == (sinBI3 | sin18) || pItem->CODE == (sinBI3 | sin19))
	{
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Vip/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "Com varios beneficios/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}



	if (pItem->CODE == (sinBI3 | sin20) || pItem->CODE == (sinBI3 | sin21) || pItem->CODE == (sinBI3 | sin22))
	{
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Gold Looter/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "Mate os Monstro e colete o ouro./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}


	if (pItem->CODE == (sinBI1 | sin23)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Lan\347a fogos de artif\355cio no/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "c\341u como forma de comemora\347\343o./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI1 | sin36)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Matura uma Arma que passou/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "pelo processo de Aging./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI1 | sin37)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Matura um Escudo ou Orbital/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "que passou pelo processo/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "de Aging./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI1 | sin38)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Matura uma Armadura ou Roup\343o/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "que passou pelo processo/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "de Aging./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI2 | sin90)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Abre o respectivo NPC de onde/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "estiver para realizar o processo/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI2 | sin91)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Abre o respectivo NPC de onde/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "estiver para realizar o processo/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI2 | sin92)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Abre o respectivo NPC de onde/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "estiver para realizar o processo/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI2 | sin93)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Abre o respectivo NPC de onde/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "estiver para realizar o processo/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI2 | sin94)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Abre o respectivo NPC de onde/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "estiver para realizar o processo/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI2 | sin95)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Abre o respectivo NPC de onde/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "estiver para realizar o processo/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI2 | sin96)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Abre o respectivo NPC de onde/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "estiver para realizar o processo/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI1 | sin42)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Abre uma loja para que o/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "personagem possa vender itens./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI1 | sin62)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Matura um Bracelete que passou/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "pelo processo de Aging./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI1 | sin63)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Matura uma Luva que passou/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "pelo processo de Aging./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI1 | sin64)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Matura uma Bota que passou/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "pelo processo de Aging./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI1 | sin95)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "O personagem fica com uma/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "express\343o de Felicidade./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI1 | sin96)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "O personagem fica com uma/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "express\343o Amorosa./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI1 | sin97)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "O personagem fica com uma/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "express\343o de Tristeza./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI1 | sin98)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "O personagem fica com uma/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "express\343o de Fofura./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI2 | sin45)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "O personagem fica com uma/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "m\341scara de halloween./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI2 | sin46)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "O personagem fica com um/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "ch\341peu de Carneiro./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI2 | sin50)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "O personagem fica com uma/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "Cartola na cabe\347a./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}


	if (pItem->CODE == (sinBI2 | sin51)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "O personagem fica com um/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "chap\351u de Bola./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}


	if (pItem->CODE == (sinBI2 | sin52)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "O personagem fica com um/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "ch\341peu de Girafa./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI2 | sin53)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "O personagem fica com uma/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "touca de Natal./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI2 | sin73)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Leva o item direto ao/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "n\355vel de Aging +10./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI2 | sin74)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Leva o item direto ao/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "n\355vel de Aging +15./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI2 | sin75)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Leva o item direto ao/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "n\355vel de Aging +18./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI2 | sin76)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Leva o item direto ao/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "n\355vel de Aging +20./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}
	if (pItem->CODE == (sinBI2 | sin77)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Leva o item direto ao/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "n\355vel de Aging +28./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}


	if (pItem->CODE == (sinBI1 | sin09)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Deixa o personagem inchado e/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "aumenta 15% do poder de ATQ./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI1 | sin12) || pItem->CODE == (sinBI1 | sin14) || pItem->CODE == (sinBI1 | sin78)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Aumenta 50% na chanche de/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "dropar mais itens de um monstro./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI1 | sin13) || pItem->CODE == (sinBI1 | sin15)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Aumenta em 30% a Experi\352ncia/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "adquirida de um monstro./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI1 | sin19) || pItem->CODE == (sinBI1 | sin20) || pItem->CODE == (sinBI1 | sin81)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Converte % do dano em HP/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI1 | sin21) || pItem->CODE == (sinBI1 | sin22) || pItem->CODE == (sinBI1 | sin82)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Converte % do dano em MP/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI1 | sin56) || pItem->CODE == (sinBI1 | sin57) || pItem->CODE == (sinBI1 | sin58)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Adiciona +10 de Regen de STM/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI1 | sin24) || pItem->CODE == (sinBI1 | sin25) || pItem->CODE == (sinBI1 | sin79)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Aumenta em 50% a Experi\352ncia/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "adquirida de um monstro./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI1 | sin28) || pItem->CODE == (sinBI1 | sin29) || pItem->CODE == (sinBI1 | sin83)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Reduz o uso de MP em 30%/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI1 | sin32) || pItem->CODE == (sinBI1 | sin33) ||
		pItem->CODE == (sinBI1 | sin34) || pItem->CODE == (sinBI1 | sin51) || pItem->CODE == (sinBI1 | sin77)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Invoca uma F\352nix para/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "ajudar nas batalhas./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Aumenta 10% na chanche de/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "dropar mais itens de um monstro./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Aumenta 10% no exp/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "de um monstro./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI1 | sin43) || pItem->CODE == (sinBI1 | sin44) || pItem->CODE == (sinBI1 | sin50) || pItem->CODE == (sinBI1 | sin80)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Aumenta em 100% a Experi\352ncia/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "adquirida de um monstro./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}



	if (pItem->CODE == (sinBI2 | sin01) || pItem->CODE == (sinBI2 | sin02) || pItem->CODE == (sinBI2 | sin03) || pItem->CODE == (sinBI2 | sin04)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Aumenta 20% na chanche de/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "dropar mais itens de um monstro./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Aumenta em 10% a Experi\352ncia/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "adquirida de um monstro./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}
	if (pItem->CODE == (sinBI2 | sin05) || pItem->CODE == (sinBI2 | sin06) || pItem->CODE == (sinBI2 | sin07) || pItem->CODE == (sinBI2 | sin08)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Aumenta 30% na chanche de/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "dropar mais itens de um monstro./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Aumenta em 20% a Experi\352ncia/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "adquirida de um monstro./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}
	if (pItem->CODE == (sinBI2 | sin09) || pItem->CODE == (sinBI2 | sin10) || pItem->CODE == (sinBI2 | sin11) || pItem->CODE == (sinBI2 | sin12)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Aumenta 40% na chanche de/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "dropar mais itens de um monstro./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Aumenta em 30% a Experi\352ncia/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "adquirida de um monstro./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}
	if (pItem->CODE == (sinBI2 | sin23) || pItem->CODE == (sinBI2 | sin24) || pItem->CODE == (sinBI2 | sin25)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Aumenta 50% na chanche de/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "dropar mais itens de um monstro./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Aumenta em 40% a Experi\352ncia/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "adquirida de um monstro./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI2 | sin26) || pItem->CODE == (sinBI2 | sin27) || pItem->CODE == (sinBI2 | sin28)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Reduz o uso de MP em 10%/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Aumenta em 10% a Experi\352ncia/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "adquirida de um monstro./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}
	if (pItem->CODE == (sinBI2 | sin29) || pItem->CODE == (sinBI2 | sin30) || pItem->CODE == (sinBI2 | sin31)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Reduz o uso de MP em 20%/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Aumenta em 20% a Experi\352ncia/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "adquirida de um monstro./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}
	if (pItem->CODE == (sinBI2 | sin32) || pItem->CODE == (sinBI2 | sin33) || pItem->CODE == (sinBI2 | sin34)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Reduz o uso de MP em 30%/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Aumenta em 30% a Experi\352ncia/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "adquirida de um monstro./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}
	if (pItem->CODE == (sinBI2 | sin35) || pItem->CODE == (sinBI2 | sin36) || pItem->CODE == (sinBI2 | sin37)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Reduz o uso de MP em 40%/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Aumenta em 40% a Experi\352ncia/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "adquirida de um monstro./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI1 | sin52) || pItem->CODE == (sinBI1 | sin65) ||
		pItem->CODE == (sinBI1 | sin69) || pItem->CODE == (sinBI1 | sin73))
	{
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Invoca um pequeno drag\343o/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "com o poder de fogo para /r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "ajudar nas batalhas./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI1 | sin53) || pItem->CODE == (sinBI1 | sin66) ||
		pItem->CODE == (sinBI1 | sin70) || pItem->CODE == (sinBI1 | sin74))
	{
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Invoca um pequeno drag\343o/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "com o poder de gelo para /r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "ajudar nas batalhas./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI1 | sin54) || pItem->CODE == (sinBI1 | sin67) ||
		pItem->CODE == (sinBI1 | sin71) || pItem->CODE == (sinBI1 | sin75))
	{
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Invoca um pequeno drag\343o/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "com o poder de raio para /r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "ajudar nas batalhas./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI1 | sin55) || pItem->CODE == (sinBI1 | sin68) ||
		pItem->CODE == (sinBI1 | sin72) || pItem->CODE == (sinBI1 | sin76))
	{
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Invoca um pequeno drag\343o/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "com o poder de cura para /r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "ajudar nas batalhas./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI1 | sin84)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Aumenta +50 do peso total no/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "personagem permanentemente./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Limite de uso: 5 vezes/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI1 | sin86) || pItem->CODE == (sinBI1 | sin87) || pItem->CODE == (sinBI1 | sin88)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Converte % do dano em HP/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI1 | sin91) || pItem->CODE == (sinBI1 | sin92)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Reduz o uso de STM em 30%/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if ((sInfo.CODE & sinITEM_MASK2) == sinPR1 ||
		(sInfo.CODE & sinITEM_MASK2) == sinPR2) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Item usado no/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "processo de Fundi\347\343o./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if ((sInfo.CODE & sinITEM_MASK2) == sinPR3 ||
		(sInfo.CODE & sinITEM_MASK2) == sinPR4) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Item usado no/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "processo de Cria\347\343o./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (((sInfo.CODE & sinITEM_MASK2) == sinWR1) ||
		((sInfo.CODE & sinITEM_MASK2) == sinDR1)) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Item usado no/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		lstrcpy(szTemp, "processo de Cria\347\343o./r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinBI2 | sin83))
	{
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Ressuscita o personagem morto/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->CODE == (sinGP1 | sin10) || pItem->CODE == (sinGP1 | sin22))
	{
		strcpy_s(szTemp, ClanCristalName);
		lstrcat(szInfoBuff, szTemp);
		strcpy_s(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		for (cnt = 0; cnt < 10; cnt++)
		{
			if (!MixItemLine[cnt])
			{
				MixItemLine[cnt] = ItemInfoCol;
				break;
			}
		}
	}


	int len = lstrlen(pItem->sItemInfo.ItemName);
	char szLinkName[24];
	if (pItem->CODE == (sinEC1 | sin05))
	{
		strcpy_s(szTemp, LinkCoreName);
		lstrcat(szInfoBuff, szTemp);
		strcpy_s(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		if (pItem->sItemInfo.ItemName[len + 1] != NULL)
		{
			lstrcat(szInfoBuff, "/r");
			lstrcat(szInfoBuff2, "/r");
			ItemInfoCol++;

			strcpy_s(szLinkName, &pItem->sItemInfo.ItemName[len + 1]);
			strcpy_s(szTemp, szLinkName);
			lstrcat(szInfoBuff, DesLinkCore);
			lstrcat(szInfoBuff, szTemp);
			strcpy_s(szTemp2, "/r");
			lstrcat(szInfoBuff, szTemp2);
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;

			for (cnt = 0; cnt < 10; cnt++)
			{
				if (!MixItemLine[cnt])
				{
					MixItemLine[cnt] = ItemInfoCol;
					break;
				}

			}

		}
	}

	ItemPerfLine = 0;
	ItemSkinLine = 0;

	if (sInfo.SpecialItemFlag[1] == ITEM_KIND_PERFECT)
	{
		lstrcat(szInfoBuff, "/r");
		lstrcat(szInfoBuff2, "/r");
		ItemInfoCol++;

		wsprintf(szTemp, "Item Perfeito/r");
		lstrcat(szInfoBuff, szTemp);
		strcpy_s(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		ItemPerfLine = ItemInfoCol;
	}

	if (sInfo.SpecialItemFlag[2] == ITEM_KIND_SKIN)
	{
		lstrcat(szInfoBuff, "/r");
		lstrcat(szInfoBuff2, "/r");
		ItemInfoCol++;

		wsprintf(szTemp, "Skin Aplicada/r");
		lstrcat(szInfoBuff, szTemp);
		strcpy_s(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		ItemSkinLine = ItemInfoCol;
	}

	if (sInfo.Damage[0] || sInfo.Damage[1])
	{
		strcpy_s(szTemp, sinAbilityName[0]);
		wsprintf(szTemp2, "%d-%d/r", sInfo.Damage[0], sInfo.Damage[1]);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		if ((sInfo.ItemKindMask & SIN_ADD_DAMAGE_MIN) != 0 ||
			(sInfo.ItemKindMask & SIN_ADD_DAMAGE_MAX) != 0)
		{
			for (cnt = 0; cnt < 10; cnt++)
			{
				if (!MixItemLine[cnt])
				{
					MixItemLine[cnt] = ItemInfoCol;
					break;
				}

			}
		}

		if (sInfo.ItemAgingNum[0])
		{
			if ((sInfo.CODE & sinITEM_MASK2) == sinWA1 ||
				(sInfo.CODE & sinITEM_MASK2) == sinWC1 ||
				(sInfo.CODE & sinITEM_MASK2) == sinWH1 ||
				(sInfo.CODE & sinITEM_MASK2) == sinWM1 ||
				(sInfo.CODE & sinITEM_MASK2) == sinWP1 ||
				(sInfo.CODE & sinITEM_MASK2) == sinWS1 ||
				(sInfo.CODE & sinITEM_MASK2) == sinWS2 ||
				(sInfo.CODE & sinITEM_MASK2) == sinWT1)

				AgingItemLine[AgingCnt++] = ItemInfoCol;

		}

	}

	if (sInfo.Attack_Speed)
	{
		strcpy_s(szTemp, sinAbilityName[1]);
		if (CheckQuestItemDownFlag && sInfo.ItemKindCode == ITEM_KIND_QUEST_WEAPON)
		{
			wsprintf(szTemp2, "%d/r", sInfo.Attack_Speed - 2);
			RedLine[9] = ItemInfoCol + 1;
		}
		else
		{
			wsprintf(szTemp2, "%d/r", sInfo.Attack_Speed);
		}
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		if ((sInfo.ItemKindMask & SIN_ADD_ATTACK_SPEED) != 0)
		{
			for (cnt = 0; cnt < 10; cnt++)
			{
				if (!MixItemLine[cnt])
				{
					MixItemLine[cnt] = ItemInfoCol;
					break;
				}

			}
		}

	}

	if (sInfo.Shooting_Range)
	{
		strcpy_s(szTemp, sinAbilityName[2]);
		wsprintf(szTemp2, "%d/r", sInfo.Shooting_Range);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;





	}
	if (sInfo.Critical_Hit)
	{
		strcpy_s(szTemp, sinAbilityName[3]);
		wsprintf(szTemp2, "%d%s/r", sInfo.Critical_Hit, "%");
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		if ((sInfo.ItemKindMask & SIN_ADD_CRITICAL) != 0)
		{
			for (cnt = 0; cnt < 10; cnt++)
			{
				if (!MixItemLine[cnt])
				{
					MixItemLine[cnt] = ItemInfoCol;
					break;
				}
			}
		}

		if (sInfo.ItemAgingNum[0])
		{
			if ((sInfo.CODE & sinITEM_MASK2) == sinWC1 ||
				(sInfo.CODE & sinITEM_MASK2) == sinWP1 ||
				(sInfo.CODE & sinITEM_MASK2) == sinWS1 ||
				(sInfo.CODE & sinITEM_MASK2) == sinWT1 ||
				(sInfo.CODE & sinITEM_MASK2) == sinWS2)
				AgingItemLine[AgingCnt++] = ItemInfoCol;

		}
	}


	if (sInfo.Defence)
	{
		strcpy_s(szTemp, sinAbilityName[4]);
		wsprintf(szTemp2, "%d/r", sInfo.Defence);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		if ((sInfo.ItemKindMask & SIN_ADD_DEFENCE) != 0)
		{
			for (cnt = 0; cnt < 10; cnt++)
			{
				if (!MixItemLine[cnt])
				{
					MixItemLine[cnt] = ItemInfoCol;
					break;
				}
			}
		}


		if (sInfo.ItemAgingNum[0])
		{
			if ((sInfo.CODE & sinITEM_MASK2) == sinOM1 ||
				(sInfo.CODE & sinITEM_MASK2) == sinDA1 ||
				(sInfo.CODE & sinITEM_MASK2) == sinDA2 ||
				(sInfo.CODE & sinITEM_MASK2) == sinDA3 ||
				(sInfo.CODE & sinITEM_MASK2) == sinDA4 ||
				(sInfo.CODE & sinITEM_MASK2) == sinDB1 ||
				(sInfo.CODE & sinITEM_MASK2) == sinDG1 ||
				(sInfo.CODE & sinITEM_MASK2) == sinOA2)
			{
				AgingItemLine[AgingCnt++] = ItemInfoCol;
			}

		}

	}

	if (sInfo.Attack_Rating)
	{
		strcpy_s(szTemp, sinAbilityName[5]);
		wsprintf(szTemp2, "%d/r", sInfo.Attack_Rating);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		if ((sInfo.ItemKindMask & SIN_ADD_ATTACK_RATE) != 0)
		{
			for (cnt = 0; cnt < 10; cnt++)
			{
				if (!MixItemLine[cnt])
				{
					MixItemLine[cnt] = ItemInfoCol;
					break;
				}
			}
		}

		if (sInfo.ItemAgingNum[0])
		{
			if ((sInfo.CODE & sinITEM_MASK2) == sinWA1 ||
				(sInfo.CODE & sinITEM_MASK2) == sinWC1 ||
				(sInfo.CODE & sinITEM_MASK2) == sinWM1 ||
				(sInfo.CODE & sinITEM_MASK2) == sinWH1 ||
				(sInfo.CODE & sinITEM_MASK2) == sinWP1 ||
				(sInfo.CODE & sinITEM_MASK2) == sinWS2 ||
				(sInfo.CODE & sinITEM_MASK2) == sinOA2)
			{
				AgingItemLine[AgingCnt++] = ItemInfoCol;
			}
		}
	}

	if (sInfo.fAbsorb)
	{

		sinAbsorb = (int)(GetItemAbsorb(&sInfo) * 10.001f);
		sinRightSpot = sinAbsorb % 10;
		sinLeftSpot = (sinAbsorb - sinRightSpot) / 10;
		strcpy_s(szTemp, sinAbilityName[6]);
		wsprintf(szTemp2, "%d.%d/r", sinLeftSpot, sinRightSpot);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		if ((sInfo.ItemKindMask & SIN_ADD_ABSORB) != 0)
		{
			for (cnt = 0; cnt < 10; cnt++)
			{
				if (!MixItemLine[cnt])
				{
					MixItemLine[cnt] = ItemInfoCol;
					break;
				}
			}

		}
		if (sInfo.ItemAgingNum[0])
		{
			if ((sInfo.CODE & sinITEM_MASK2) == sinOM1 ||
				(sInfo.CODE & sinITEM_MASK2) == sinDA1 ||
				(sInfo.CODE & sinITEM_MASK2) == sinDS1 ||
				(sInfo.CODE & sinITEM_MASK2) == sinDA2 ||
				(sInfo.CODE & sinITEM_MASK2) == sinDA3 ||
				(sInfo.CODE & sinITEM_MASK2) == sinDA4 ||
				(sInfo.CODE & sinITEM_MASK2) == sinDB1 ||
				(sInfo.CODE & sinITEM_MASK2) == sinDG1)
			{
				AgingItemLine[AgingCnt++] = ItemInfoCol;
			}
		}
	}

	if (sInfo.fBlock_Rating)
	{
		BlockRate = (int)(sInfo.fBlock_Rating);
		strcpy_s(szTemp, sinAbilityName[7]);
		wsprintf(szTemp2, "%d%s/r", BlockRate, "%");
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		if ((sInfo.ItemKindMask & SIN_ADD_BLOCK_RATE) != 0)
		{
			for (cnt = 0; cnt < 10; cnt++)
			{
				if (!MixItemLine[cnt])
				{
					MixItemLine[cnt] = ItemInfoCol;
					break;
				}

			}
		}


		if (sInfo.ItemAgingNum[0])
		{
			if ((sInfo.CODE & sinITEM_MASK2) == sinDS1)
				AgingItemLine[AgingCnt++] = ItemInfoCol;

		}


	}


	if (sInfo.fSpeed)
	{
		sinSpeed = (int)(sInfo.fSpeed * 10.001f);
		sinRightSpot = sinSpeed % 10;
		sinLeftSpot = (sinSpeed - sinRightSpot) / 10;
		strcpy_s(szTemp, sinAbilityName[8]);
		wsprintf(szTemp2, "%d.%d/r", sinLeftSpot, sinRightSpot);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		if ((sInfo.ItemKindMask & SIN_ADD_MOVE_SPEED) != 0)
		{
			for (cnt = 0; cnt < 10; cnt++)
			{
				if (!MixItemLine[cnt])
				{
					MixItemLine[cnt] = ItemInfoCol;
					break;
				}
			}

		}

	}

	if (sInfo.Durability[0] || sInfo.Durability[1])
	{
		strcpy_s(szTemp, sinAbilityName[9]);
		wsprintf(szTemp2, "%d/%d/r", sInfo.Durability[0], sInfo.Durability[1]);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		if (sInfo.ItemAgingNum[0])
		{
			if ((sInfo.CODE & sinITEM_MASK2) == sinWA1 ||
				(sInfo.CODE & sinITEM_MASK2) == sinWC1 ||
				(sInfo.CODE & sinITEM_MASK2) == sinWH1 ||
				(sInfo.CODE & sinITEM_MASK2) == sinWM1 ||
				(sInfo.CODE & sinITEM_MASK2) == sinWS1 ||
				(sInfo.CODE & sinITEM_MASK2) == sinWS2 ||
				(sInfo.CODE & sinITEM_MASK2) == sinWT1 ||
				(sInfo.CODE & sinITEM_MASK2) == sinDS1 ||
				(sInfo.CODE & sinITEM_MASK2) == sinOM1 ||
				(sInfo.CODE & sinITEM_MASK2) == sinDA1 ||
				(sInfo.CODE & sinITEM_MASK2) == sinDA2 ||
				(sInfo.CODE & sinITEM_MASK2) == sinDA3 ||
				(sInfo.CODE & sinITEM_MASK2) == sinDA4 ||
				(sInfo.CODE & sinITEM_MASK2) == sinDB1 ||
				(sInfo.CODE & sinITEM_MASK2) == sinDG1 ||
				(sInfo.CODE & sinITEM_MASK2) == sinOA2)
			{
				AgingItemLine[AgingCnt++] = ItemInfoCol;
			}

		}

	}


	if (sInfo.Mana[0] || sInfo.Mana[1])
	{
		strcpy_s(szTemp, sinAbilityName[10]);
		wsprintf(szTemp2, "%d-%d/r", sInfo.Mana[0], sInfo.Mana[1]);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

	}
	if (sInfo.Life[0] || sInfo.Life[1])
	{
		strcpy_s(szTemp, sinAbilityName[11]);
		wsprintf(szTemp2, "%d-%d/r", sInfo.Life[0], sInfo.Life[1]);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

	}
	if (sInfo.Stamina[0] || sInfo.Stamina[1])
	{
		strcpy_s(szTemp, sinAbilityName[12]);
		wsprintf(szTemp2, "%d-%d/r", sInfo.Stamina[0], sInfo.Stamina[1]);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

	}


	if (sInfo.Resistance[0])
	{
		strcpy_s(szTemp, sinAbilityName[13]);
		wsprintf(szTemp2, "%d/r", sInfo.Resistance[0]);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		if ((sInfo.ItemKindMask & SIN_ADD_BIO) != 0)
		{
			for (cnt = 0; cnt < 10; cnt++)
			{
				if (!MixItemLine[cnt])
				{
					MixItemLine[cnt] = ItemInfoCol;
					break;
				}
			}
		}
	}
	if (sInfo.Resistance[1])
	{
		strcpy_s(szTemp, sinAbilityName[14]);
		wsprintf(szTemp2, "%d/r", sInfo.Resistance[1]);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

	}
	if (sInfo.Resistance[2])
	{
		strcpy_s(szTemp, sinAbilityName[15]);
		wsprintf(szTemp2, "%d/r", sInfo.Resistance[2]);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		if ((sInfo.ItemKindMask & SIN_ADD_FIRE) != 0)
		{
			for (cnt = 0; cnt < 10; cnt++)
			{
				if (!MixItemLine[cnt])
				{
					MixItemLine[cnt] = ItemInfoCol;
					break;
				}
			}

		}

	}

	if (sInfo.Resistance[3])
	{
		strcpy_s(szTemp, sinAbilityName[16]);
		wsprintf(szTemp2, "%d/r", sInfo.Resistance[3]);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		if ((sInfo.ItemKindMask & SIN_ADD_ICE) != 0)
		{
			for (cnt = 0; cnt < 10; cnt++)
			{
				if (!MixItemLine[cnt])
				{
					MixItemLine[cnt] = ItemInfoCol;
					break;
				}
			}
		}
	}
	if (sInfo.Resistance[4])
	{
		strcpy_s(szTemp, sinAbilityName[17]);
		wsprintf(szTemp2, "%d/r", sInfo.Resistance[4]);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		if ((sInfo.ItemKindMask & SIN_ADD_LIGHTNING) != 0)
		{
			for (cnt = 0; cnt < 10; cnt++)
			{
				if (!MixItemLine[cnt])
				{
					MixItemLine[cnt] = ItemInfoCol;
					break;
				}
			}
		}
	}
	if (sInfo.Resistance[5])
	{
		strcpy_s(szTemp, sinAbilityName[18]);
		wsprintf(szTemp2, "%d/r", sInfo.Resistance[5]);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		if ((sInfo.ItemKindMask & SIN_ADD_POISON) != 0)
		{
			for (cnt = 0; cnt < 10; cnt++)
			{
				if (!MixItemLine[cnt])
				{
					MixItemLine[cnt] = ItemInfoCol;
					break;
				}
			}
		}
	}
	if (sInfo.Resistance[6])
	{
		strcpy_s(szTemp, sinAbilityName[19]);
		wsprintf(szTemp2, "%d/r", sInfo.Resistance[6]);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

	}
	if (sInfo.Resistance[7])
	{
		strcpy_s(szTemp, sinAbilityName[20]);
		wsprintf(szTemp2, "%d/r", sInfo.Resistance[7]);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

	}

	if (sInfo.fLife_Regen && (sInfo.CODE & sinITEM_MASK2) != sinFO1)
	{
		sinRegen = (int)(sInfo.fLife_Regen * 10.001f);
		sinRightSpot = sinRegen % 10;
		sinLeftSpot = (sinRegen - sinRightSpot) / 10;
		strcpy_s(szTemp, sinAbilityName[21]);
		wsprintf(szTemp2, "%d.%d/r", sinLeftSpot, sinRightSpot);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		if ((sInfo.ItemKindMask & SIN_ADD_LIFEREGEN) != 0)
		{
			for (cnt = 0; cnt < 10; cnt++)
			{
				if (!MixItemLine[cnt])
				{
					MixItemLine[cnt] = ItemInfoCol;
					break;
				}
			}
		}

	}

	if (sInfo.fMana_Regen && (sInfo.CODE & sinITEM_MASK2) != sinFO1)
	{
		sinRegen = (int)(sInfo.fMana_Regen * 10.001f);
		sinRightSpot = sinRegen % 10;
		sinLeftSpot = (sinRegen - sinRightSpot) / 10;
		strcpy_s(szTemp, sinAbilityName[22]);
		wsprintf(szTemp2, "%d.%d/r", sinLeftSpot, sinRightSpot);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		if ((sInfo.ItemKindMask & SIN_ADD_MANAREGEN) != 0)
		{
			for (cnt = 0; cnt < 10; cnt++)
			{
				if (!MixItemLine[cnt])
				{
					MixItemLine[cnt] = ItemInfoCol;
					break;
				}
			}
		}

	}

	if (sInfo.fStamina_Regen && (sInfo.CODE & sinITEM_MASK2) != sinFO1)
	{
		sinRegen = (int)(sInfo.fStamina_Regen * 10.001f);
		sinRightSpot = sinRegen % 10;
		sinLeftSpot = (sinRegen - sinRightSpot) / 10;
		strcpy_s(szTemp, sinAbilityName[23]);
		wsprintf(szTemp2, "%d.%d/r", sinLeftSpot, sinRightSpot);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		if ((sInfo.ItemKindMask & SIN_ADD_STAMINAREGEN) != 0)
		{
			for (cnt = 0; cnt < 10; cnt++)
			{
				if (!MixItemLine[cnt])
				{
					MixItemLine[cnt] = ItemInfoCol;
					break;
				}
			}
		}

	}
	int AddState;
	if (sInfo.fIncrease_Life)
	{
		AddState = (int)sInfo.fIncrease_Life;
		strcpy_s(szTemp, sinAbilityName[24]);
		wsprintf(szTemp2, "%d/r", AddState);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		if ((sInfo.ItemKindMask & SIN_ADD_LIFE) != 0)
		{
			for (cnt = 0; cnt < 10; cnt++)
			{
				if (!MixItemLine[cnt])
				{
					MixItemLine[cnt] = ItemInfoCol;
					break;
				}
			}
		}
	}
	if (sInfo.fIncrease_Mana)
	{
		AddState = (int)sInfo.fIncrease_Mana;
		strcpy_s(szTemp, sinAbilityName[25]);
		wsprintf(szTemp2, "%d/r", AddState);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		if ((sInfo.ItemKindMask & SIN_ADD_MANA) != 0)
		{
			for (cnt = 0; cnt < 10; cnt++)
			{
				if (!MixItemLine[cnt])
				{
					MixItemLine[cnt] = ItemInfoCol;
					break;
				}
			}
		}


		if (sInfo.ItemAgingNum[0])
		{
			if ((sInfo.CODE & sinITEM_MASK2) == sinWM1 ||
				(sInfo.CODE & sinITEM_MASK2) == sinWS1 ||
				(sInfo.CODE & sinITEM_MASK2) == sinWT1)
			{
				AgingItemLine[AgingCnt++] = ItemInfoCol;
			}

		}

	}
	if (sInfo.fIncrease_Stamina)
	{
		AddState = (int)sInfo.fIncrease_Stamina;
		strcpy_s(szTemp, sinAbilityName[26]);
		wsprintf(szTemp2, "%d/r", AddState);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		if ((sInfo.ItemKindMask & SIN_ADD_STAMINA) != 0)
		{
			for (cnt = 0; cnt < 10; cnt++)
			{
				if (!MixItemLine[cnt])
				{
					MixItemLine[cnt] = ItemInfoCol;
					break;
				}
			}
		}

	}
	if (sInfo.Potion_Space)
	{
		strcpy_s(szTemp, sinAbilityName[27]);
		wsprintf(szTemp2, "%d/r", sInfo.Potion_Space);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

	}

	lstrcpy(szTemp2, "  /r");
	lstrcat(szInfoBuff, szTemp2);
	lstrcat(szInfoBuff2, szTemp2);
	ItemInfoCol++;

	if (pItem->CODE == (sinOR1 | sin25) || pItem->CODE == (sinOA1 | sin25))
	{
		ItemBoss1 = 1;
		lstrcpy(szTemp2, "  /r");
		lstrcat(szInfoBuff, szTemp2);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		wsprintf(szTemp, "Set Boss #1/r");
		lstrcat(szInfoBuff, szTemp);
		strcpy_s(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		ItemBossLine = ItemInfoCol;
		wsprintf(szTemp, "Anillo Valento (%d/2)/r", LValRing + RValRing);
		lstrcat(szInfoBuff, szTemp);
		strcpy_s(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		Ring1 = ItemInfoCol;
		wsprintf(szTemp, "Amuleto Kelvezu (%d/1)/r", KelAmulet);
		lstrcat(szInfoBuff, szTemp);
		strcpy_s(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		Amulet1 = ItemInfoCol;
		lstrcat(szInfoBuff, "/r");
		lstrcat(szInfoBuff2, "/r");
		ItemInfoCol++;
		wsprintf(szTemp, "Set B\364nus #1/r");
		lstrcat(szInfoBuff, szTemp);
		strcpy_s(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		Efect1Line = ItemInfoCol;
		wsprintf(szTemp, "2 - %d Defensa/r", SetBoss1Bonus1);
		lstrcat(szInfoBuff, szTemp);
		strcpy_s(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		Efect1Line2 = ItemInfoCol;
		wsprintf(szTemp, "3 - %d Vida/r", SetBoss1Bonus2);
		lstrcat(szInfoBuff, szTemp);
		strcpy_s(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		Efect1Line3 = ItemInfoCol;
	}
	else
		ItemBoss1 = 0;

	if (pItem->CODE == (sinOR1 | sin26) || pItem->CODE == (sinOA1 | sin26))
	{
		ItemBoss2 = 1;
		lstrcpy(szTemp2, "  /r");
		lstrcat(szInfoBuff, szTemp2);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		wsprintf(szTemp, "Set Boss #2/r");
		lstrcat(szInfoBuff, szTemp);
		strcpy_s(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		ItemBossLine = ItemInfoCol;
		wsprintf(szTemp, "Anel Shy (%d/2)/r", LShyRing + RShyRing);
		lstrcat(szInfoBuff, szTemp);
		strcpy_s(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		Ring2 = ItemInfoCol;
		wsprintf(szTemp, "Amuleto Tulla (%d/1)/r", TullaAmulet);
		lstrcat(szInfoBuff, szTemp);
		strcpy_s(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		Amulet2 = ItemInfoCol;
		lstrcat(szInfoBuff, "/r");
		lstrcat(szInfoBuff2, "/r");
		ItemInfoCol++;
		wsprintf(szTemp, "Set B\364nus #2/r");
		lstrcat(szInfoBuff, szTemp);
		strcpy_s(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		Efect2Line = ItemInfoCol;
		wsprintf(szTemp, "2 - %d Defensa/r", SetBoss2Bonus1);
		lstrcat(szInfoBuff, szTemp);
		strcpy_s(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		Efect2Line2 = ItemInfoCol;
		wsprintf(szTemp, "3 - %d Vida/r", SetBoss2Bonus2);
		lstrcat(szInfoBuff, szTemp);
		strcpy_s(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		Efect2Line3 = ItemInfoCol;
	}

	int i = 0;
	if (sInfo.Level)
	{
		strcpy_s(szTemp, sinAbilityName[28]);
		if ((sInfo.CODE & sinITEM_MASK2) == sinFO1)
		{
			lstrcat(szInfoBuff, "/r");
			lstrcat(szInfoBuff2, "/r");
			ItemInfoCol++;

			for (i = 0; i < 16; i++)
			{
				if (((pItem->CODE & sinITEM_MASK3) == SheltomCode2[i]))
				{
					if (i <= 3)
					{
						wsprintf(szTemp2, "%d-%d/r", ForceOrbUseLevel[i][0], ForceOrbUseLevel[i][1]);
					}
					else
					{
						wsprintf(szTemp2, "%d+/r", ForceOrbUseLevel[i][0]);
					}
					break;
				}
				else if (((pItem->CODE & sinITEM_MASK3) == MagicSheltomCode[i]))
				{
					if (i <= 3)
					{
						wsprintf(szTemp2, "%d-%d/r", MagicForceOrbUseLevel[i][0], MagicForceOrbUseLevel[i][1]);
					}
					else
					{
						wsprintf(szTemp2, "%d+/r", MagicForceOrbUseLevel[i][0]);
					}
					break;
				}
				else if (((pItem->CODE & sinITEM_MASK3) == BillingMagicSheltomCode[i]))
				{
					wsprintf(szTemp2, "%d+/r", BillingMagicForceOrbUseLevel[i][0]);
					break;
				}
			}
			lstrcat(szInfoBuff, szTemp);
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			RequireLine[0] = ItemInfoCol;
			if (sinChar->Level < ForceOrbUseLevel[i][0] ||
				sinChar->Level > ForceOrbUseLevel[i][1])
			{
				sRequire.rLevel = 1;
				RedLine[0] = ItemInfoCol;
			}

		}
		else
		{
			wsprintf(szTemp2, "%d/r", sInfo.Level);
			lstrcat(szInfoBuff, szTemp);
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			RequireLine[0] = ItemInfoCol;
			if (sInfo.Level > sinChar->Level)
			{
				sRequire.rLevel = 1;
				RedLine[0] = ItemInfoCol;
			}
		}
	}

	if ((sInfo.CODE & sinITEM_MASK2) == sinSK1 || (sInfo.CODE & sinITEM_MASK2) == sinSK2) {
		lstrcpy(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		lstrcpy(szTemp, "Use para trocar a skin/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcpy(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if ((sInfo.CODE & sinITEM_MASK2) == sinFO1)
	{
		for (i = 0; i < 16; i++)
		{
			if ((pItem->CODE & sinITEM_MASK3) == SheltomCode2[i])
			{
				strcpy_s(szTemp, sinAddDamageItem);
				wsprintf(szTemp2, "%d/r", ForceOrbDamage[i]);
				lstrcat(szInfoBuff, szTemp);
				lstrcat(szInfoBuff2, szTemp2);
				ItemInfoCol++;

				if (i > 5 && i < 10)
				{
					strcpy_s(szTemp, AddPercentDamage3);
					wsprintf(szTemp2, "%d%s/r", 10, "%");
					lstrcat(szInfoBuff, szTemp);
					lstrcat(szInfoBuff2, szTemp2);
					ItemInfoCol++;
				}
				else if (i == 10)
				{
					strcpy_s(szTemp, AddPercentDamage3);
					wsprintf(szTemp2, "%d%s/r", 15, "%");
					lstrcat(szInfoBuff, szTemp);
					lstrcat(szInfoBuff2, szTemp2);
					ItemInfoCol++;
				}
				else if (i == 11)
				{
					strcpy_s(szTemp, AddPercentDamage3);
					wsprintf(szTemp2, "%d%s/r", 20, "%");
					lstrcat(szInfoBuff, szTemp);
					lstrcat(szInfoBuff2, szTemp2);
					ItemInfoCol++;
				}
				else if (i == 12)
				{
					strcpy_s(szTemp, AddPercentDamage3);
					wsprintf(szTemp2, "%d%s/r", 20, "%");
					lstrcat(szInfoBuff, szTemp);
					lstrcat(szInfoBuff2, szTemp2);
					ItemInfoCol++;
				}
				else if (i == 13)
				{
					strcpy_s(szTemp, AddPercentDamage3);
					wsprintf(szTemp2, "%d%s/r", 25, "%");
					lstrcat(szInfoBuff, szTemp);
					lstrcat(szInfoBuff2, szTemp2);
					ItemInfoCol++;
				}
				else if (i == 14)
				{
					strcpy_s(szTemp, AddPercentDamage3);
					wsprintf(szTemp2, "%d%s/r", 30, "%");
					lstrcat(szInfoBuff, szTemp);
					lstrcat(szInfoBuff2, szTemp2);
					ItemInfoCol++;
				}
				else if (i == 15)
				{
					strcpy_s(szTemp, AddPercentDamage3);
					wsprintf(szTemp2, " %d%s/r", 40, "%");
					lstrcat(szInfoBuff, szTemp);
					lstrcat(szInfoBuff2, szTemp2);
					ItemInfoCol++;
				}
				strcpy_s(szTemp, "Dura\347\343o: /r");
				wsprintf(szTemp2, "%02d:%02d:%02d/r", (ForceOrbUseTime[i] / 60 / 60) % 24, (ForceOrbUseTime[i] / 60) % 60, (ForceOrbUseTime[i]) % 60);
				//wsprintf(szTemp2, "%d%s/r", ForceOrbUseTime[i], SecName);
				lstrcat(szInfoBuff, szTemp);
				lstrcat(szInfoBuff2, szTemp2);
				ItemInfoCol++;
				break;
			}
			else if ((pItem->CODE & sinITEM_MASK3) == MagicSheltomCode[i])
			{
				strcpy_s(szTemp, sinAddDamageItem);
				wsprintf(szTemp2, "%d/r", MagicForceOrbDamage[i]);
				lstrcat(szInfoBuff, szTemp);
				lstrcat(szInfoBuff2, szTemp2);
				ItemInfoCol++;


				if (i > 5 && i < 10)
				{
					strcpy_s(szTemp, AddPercentDamage3);
					wsprintf(szTemp2, "%d%s/r", 10, "%");
					lstrcat(szInfoBuff, szTemp);
					lstrcat(szInfoBuff2, szTemp2);
					ItemInfoCol++;
				}
				else if (i == 10)
				{
					strcpy_s(szTemp, AddPercentDamage3);
					wsprintf(szTemp2, "%d%s/r", 15, "%");
					lstrcat(szInfoBuff, szTemp);
					lstrcat(szInfoBuff2, szTemp2);
					ItemInfoCol++;
				}
				else if (i == 11)
				{
					strcpy_s(szTemp, AddPercentDamage3);
					wsprintf(szTemp2, "%d%s/r", 20, "%");
					lstrcat(szInfoBuff, szTemp);
					lstrcat(szInfoBuff2, szTemp2);
					ItemInfoCol++;
				}

				strcpy_s(szTemp, "Dura\347\343o: /r");
				wsprintf(szTemp2, "%02d:%02d:%02d/r", (MagicForceOrbUseTime[i] / 60 / 60) % 24, (MagicForceOrbUseTime[i] / 60) % 60, (MagicForceOrbUseTime[i]) % 60);
				/*wsprintf(szTemp2, "%d%s/r", MagicForceOrbUseTime[i], SecName);*/
				lstrcat(szInfoBuff, szTemp);
				lstrcat(szInfoBuff2, szTemp2);
				ItemInfoCol++;
				break;
			}
			else if ((pItem->CODE & sinITEM_MASK3) == BillingMagicSheltomCode[i])
			{

				strcpy_s(szTemp, AddPercentDamage3);
				wsprintf(szTemp2, "%d%s/r", 15, "%");
				lstrcat(szInfoBuff, szTemp);
				lstrcat(szInfoBuff2, szTemp2);
				ItemInfoCol++;

				strcpy_s(szTemp, "Dura\347\343o: /r");
				wsprintf(szTemp2, "%02d:%02d:%02d/r", (BillingMagicForceOrbUseTime[i] / 60 / 60) % 24, (BillingMagicForceOrbUseTime[i] / 60) % 60, (BillingMagicForceOrbUseTime[i]) % 60);
				/*wsprintf(szTemp2, "%d%s/r", BillingMagicForceOrbUseTime[i], SecName);*/
				lstrcat(szInfoBuff, szTemp);
				lstrcat(szInfoBuff2, szTemp2);
				ItemInfoCol++;
				break;
			}
		}
	}



	if (pItem->sItemInfo.CODE == (sinDA1 | sin48) || pItem->sItemInfo.CODE == (sinDA2 | sin48))
	{
		DWORD NowTime = GetPlayTime_T();
		DWORD Time = (60 * 60 * 24 * 14) - (NowTime - pItem->sItemInfo.dwCreateTime);
		strcpy_s(szTemp, sinContinueTimeItem);
		wsprintf(szTemp2, "%02d:%02d:%02d:%02d/r", (Time / 60 / 60) / 24, (Time / 60 / 60) % 24, (Time / 60) % 60, (Time) % 60);

		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->sItemInfo.CODE == (sinOR2 | sin06) || pItem->sItemInfo.CODE == (sinOR2 | sin07) ||
		pItem->sItemInfo.CODE == (sinOR2 | sin08) || pItem->sItemInfo.CODE == (sinOR2 | sin09) ||
		pItem->sItemInfo.CODE == (sinOR2 | sin10) || pItem->sItemInfo.CODE == (sinOR2 | sin11) ||
		pItem->sItemInfo.CODE == (sinOR2 | sin12) || pItem->sItemInfo.CODE == (sinOR2 | sin13) ||
		pItem->sItemInfo.CODE == (sinOR2 | sin14) || pItem->sItemInfo.CODE == (sinOR2 | sin15) ||
		pItem->sItemInfo.CODE == (sinOR2 | sin16) || pItem->sItemInfo.CODE == (sinOR2 | sin17) ||
		pItem->sItemInfo.CODE == (sinOR2 | sin18) || pItem->sItemInfo.CODE == (sinOR2 | sin19) ||
		pItem->sItemInfo.CODE == (sinOR2 | sin20) || pItem->sItemInfo.CODE == (sinOR2 | sin21) ||
		pItem->sItemInfo.CODE == (sinOR2 | sin22) || pItem->sItemInfo.CODE == (sinOR2 | sin23) ||
		pItem->sItemInfo.CODE == (sinOR2 | sin24) || pItem->sItemInfo.CODE == (sinOR2 | sin25))
	{
		DWORD NowTime = GetPlayTime_T();
		DWORD Time = (60 * 60 * 24 * 5) - (NowTime - pItem->sItemInfo.dwCreateTime);
		strcpy_s(szTemp, sinContinueTimeItem);
		wsprintf(szTemp2, "%02d:%02d:%02d:%02d/r", (Time / 60 / 60) / 24, (Time / 60 / 60) % 24, (Time / 60) % 60, (Time) % 60);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->sItemInfo.CODE == (sinOR2 | sin27) || pItem->sItemInfo.CODE == (sinOA1 | sin32))
	{
		DWORD NowTime = GetPlayTime_T();
		DWORD Time = (60 * 60 * 24 * 3) - (NowTime - pItem->sItemInfo.dwCreateTime);

		strcpy_s(szTemp, sinContinueTimeItem);

		if (DeleteEventItem_TimeOut(&pItem->sItemInfo) == TRUE)
			wsprintf(szTemp2, "%s/r", ExpireItem);
		else
			wsprintf(szTemp2, " %02d:%02d:%02d:%02d/r", (Time / 60 / 60) / 24, (Time / 60 / 60) % 24, (Time / 60) % 60, (Time) % 60);

		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->sItemInfo.CODE == (sinOR2 | sin28) || pItem->sItemInfo.CODE == (sinOA1 | sin33))
	{
		DWORD NowTime = GetPlayTime_T();
		DWORD Time = (60 * 60 * 24 * 7) - (NowTime - pItem->sItemInfo.dwCreateTime);

		strcpy_s(szTemp, sinContinueTimeItem);

		if (DeleteEventItem_TimeOut(&pItem->sItemInfo) == TRUE)
			wsprintf(szTemp2, "%s/r", ExpireItem);
		else
			wsprintf(szTemp2, "%02d:%02d:%02d:%02d/r", (Time / 60 / 60) / 24, (Time / 60 / 60) % 24, (Time / 60) % 60, (Time) % 60);



		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}


	if (pItem->sItemInfo.CODE == (sinOR2 | sin29) || pItem->sItemInfo.CODE == (sinOA1 | sin34))
	{
		DWORD NowTime = GetPlayTime_T();
		DWORD Time = (60 * 60) - (NowTime - pItem->sItemInfo.dwCreateTime);

		strcpy_s(szTemp, sinContinueTimeItem);

		if (DeleteEventItem_TimeOut(&pItem->sItemInfo) == TRUE)
			wsprintf(szTemp2, "%s/r", ExpireItem);
		else

			wsprintf(szTemp2, "%02d:%02d:%02d/r", (Time / 60 / 60) % 24, (Time / 60) % 60, (Time) % 60);

		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}


	if (pItem->sItemInfo.CODE == (sinOR2 | sin30) || pItem->sItemInfo.CODE == (sinOR2 | sin53) ||
		pItem->sItemInfo.CODE == (sinOR2 | sin55) || pItem->sItemInfo.CODE == (sinOA1 | sin53) ||
		pItem->sItemInfo.CODE == (sinOA1 | sin55) || pItem->sItemInfo.CODE == (sinDS1 | sin32) ||
		pItem->sItemInfo.CODE == (sinOA1 | sin35))
	{
		DWORD NowTime = GetPlayTime_T();
		DWORD Time = (60 * 60 * 24) - (NowTime - pItem->sItemInfo.dwCreateTime);

		strcpy_s(szTemp, sinContinueTimeItem);

		if (DeleteEventItem_TimeOut(&pItem->sItemInfo) == TRUE)
			wsprintf(szTemp2, "%s/r", ExpireItem);
		else

			wsprintf(szTemp2, "%02d:%02d:%02d:%02d/r", (Time / 60 / 60) / 24, (Time / 60 / 60) % 24, (Time / 60) % 60, (Time) % 60);


		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}


	if (pItem->sItemInfo.CODE == (sinOR2 | sin33))
	{
		DWORD NowTime = GetPlayTime_T();
		DWORD Time = (60 * 60 * 24 * 7) - (NowTime - pItem->sItemInfo.dwCreateTime);

		strcpy_s(szTemp, sinContinueTimeItem);

		if (DeleteEventItem_TimeOut(&pItem->sItemInfo) == TRUE)
			wsprintf(szTemp2, "%s/r", ExpireItem);
		else
			wsprintf(szTemp2, "%02d:%02d:%02d:%02d/r", (Time / 60 / 60) / 24, (Time / 60 / 60) % 24, (Time / 60) % 60, (Time) % 60);



		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}


	if (pItem->sItemInfo.CODE == (sinOA1 | sin36))
	{
		DWORD NowTime = GetPlayTime_T();
		DWORD Time = (60 * 60 * 24 * 7) - (NowTime - pItem->sItemInfo.dwCreateTime);
		strcpy_s(szTemp, sinContinueTimeItem);

		if (DeleteEventItem_TimeOut(&pItem->sItemInfo) == TRUE)
			wsprintf(szTemp2, "%s/r", ExpireItem);

		else
			wsprintf(szTemp2, "%02d:%02d:%02d:%02d/r", (Time / 60 / 60) / 24, (Time / 60 / 60) % 24, (Time / 60) % 60, (Time) % 60);

		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->sItemInfo.CODE == (sinOA1 | sin37))
	{
		DWORD NowTime = GetPlayTime_T();
		DWORD Time = (60 * 60 * 24 * 7) - (NowTime - pItem->sItemInfo.dwCreateTime);

		strcpy_s(szTemp, sinContinueTimeItem);

		if (DeleteEventItem_TimeOut(&pItem->sItemInfo) == TRUE)
			wsprintf(szTemp2, "%s/r", ExpireItem);
		else
			wsprintf(szTemp2, "%02d:%02d:%02d:%02d/r", (Time / 60 / 60) / 24, (Time / 60 / 60) % 24, (Time / 60) % 60, (Time) % 60);



		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->sItemInfo.CODE == (sinDB1 | sin33))
	{
		DWORD NowTime = GetPlayTime_T();
		DWORD Time = (60 * 60 * 24 * 1) - (NowTime - pItem->sItemInfo.dwCreateTime);
		strcpy_s(szTemp, sinContinueTimeItem);
		if (DeleteEventItem_TimeOut(&pItem->sItemInfo) == TRUE)
			wsprintf(szTemp2, "%s/r", ExpireItem);
		else
			wsprintf(szTemp2, "%02d:%02d:%02d:%02d/r", (Time / 60 / 60) / 24, (Time / 60 / 60) % 24, (Time / 60) % 60, (Time) % 60);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}


	if (pItem->sItemInfo.CODE == (sinDB1 | sin34))
	{
		DWORD NowTime = GetPlayTime_T();
		DWORD Time = (60 * 60) - (NowTime - pItem->sItemInfo.dwCreateTime);
		strcpy_s(szTemp, sinContinueTimeItem);
		if (DeleteEventItem_TimeOut(&pItem->sItemInfo) == TRUE)
			wsprintf(szTemp2, "%s/r", ExpireItem);
		else
			wsprintf(szTemp2, "%02d:%02d:%02d:%02d/r", (Time / 60 / 60) / 24, (Time / 60 / 60) % 24, (Time / 60) % 60, (Time) % 60);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}



	if (pItem->sItemInfo.CODE == (sinOA2 | sin33) || pItem->sItemInfo.CODE == (sinDS1 | sin32))
	{
		DWORD NowTime = GetPlayTime_T();
		DWORD Time = (60 * 60 * 24 * 1) - (NowTime - pItem->sItemInfo.dwCreateTime);
		strcpy_s(szTemp, sinContinueTimeItem);
		if (DeleteEventItem_TimeOut(&pItem->sItemInfo) == TRUE)
			wsprintf(szTemp2, "%s/r", ExpireItem);
		else
			wsprintf(szTemp2, "%02d:%02d:%02d:%02d/r", (Time / 60 / 60) / 24, (Time / 60 / 60) % 24, (Time / 60) % 60, (Time) % 60);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}


	if (pItem->sItemInfo.CODE == (sinOA2 | sin34))
	{
		DWORD NowTime = GetPlayTime_T();
		DWORD Time = (60 * 60) - (NowTime - pItem->sItemInfo.dwCreateTime);
		strcpy_s(szTemp, sinContinueTimeItem);
		if (DeleteEventItem_TimeOut(&pItem->sItemInfo) == TRUE)
			wsprintf(szTemp2, "%s/r", ExpireItem);
		else
			wsprintf(szTemp2, "%02d:%02d:%02d:%02d/r", (Time / 60 / 60) / 24, (Time / 60 / 60) % 24, (Time / 60) % 60, (Time) % 60);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}


	if (pItem->CODE == (sinBI2 | sin83))
	{
		DWORD NowTime = GetPlayTime_T();
		DWORD Time = (60 * 60 * 3) - (NowTime - pItem->sItemInfo.dwCreateTime);
		strcpy_s(szTemp, sinContinueTimeItem);
		if (DeleteEventItem_TimeOut(&pItem->sItemInfo) == TRUE)
			wsprintf(szTemp2, "%s/r", ExpireItem);
		else
			wsprintf(szTemp2, "%02d:%02d:%02d:%02d/r", (Time / 60 / 60) / 24, (Time / 60 / 60) % 24, (Time / 60) % 60, (Time) % 60);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->sItemInfo.CODE == (sinOA1 | sin39) || pItem->sItemInfo.CODE == (sinOA1 | sin40) ||
		pItem->sItemInfo.CODE == (sinOA1 | sin41) || pItem->sItemInfo.CODE == (sinOA1 | sin42))
	{
		DWORD NowTime = GetPlayTime_T();
		DWORD Time = (60 * 60 * 24 * 14) - (NowTime - pItem->sItemInfo.dwCreateTime);

		strcpy_s(szTemp, sinContinueTimeItem);

		if (DeleteEventItem_TimeOut(&pItem->sItemInfo) == TRUE)
			wsprintf(szTemp2, "%s/r", ExpireItem);
		else
			wsprintf(szTemp2, "%02d:%02d:%02d:%02d/r", (Time / 60 / 60) / 24, (Time / 60 / 60) % 24, (Time / 60) % 60, (Time) % 60);



		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->sItemInfo.CODE == (sinOR2 | sin34) || pItem->sItemInfo.CODE == (sinOR2 | sin51) ||
		pItem->sItemInfo.CODE == (sinOA1 | sin51) || pItem->sItemInfo.CODE == (sinDS1 | sin33) ||
		pItem->sItemInfo.CODE == (sinOR2 | sin35) || pItem->sItemInfo.CODE == (sinOA1 | sin38))
	{
		DWORD NowTime = GetPlayTime_T();
		DWORD Time = (60 * 60 * 24 * 14) - (NowTime - pItem->sItemInfo.dwCreateTime);

		strcpy_s(szTemp, sinContinueTimeItem);

		if (DeleteEventItem_TimeOut(&pItem->sItemInfo) == TRUE)
			wsprintf(szTemp2, "%s/r", ExpireItem);
		else
			wsprintf(szTemp2, "%02d:%02d:%02d:%02d/r", (Time / 60 / 60) / 24, (Time / 60 / 60) % 24, (Time / 60) % 60, (Time) % 60);



		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->sItemInfo.CODE == (sinOR2 | sin50) || pItem->sItemInfo.CODE == (sinOR2 | sin54) ||
		pItem->sItemInfo.CODE == (sinOR2 | sin56) || pItem->sItemInfo.CODE == (sinOA1 | sin50) ||
		pItem->sItemInfo.CODE == (sinOA1 | sin54) || pItem->sItemInfo.CODE == (sinOA1 | sin56) ||
		pItem->sItemInfo.CODE == (sinDS1 | sin32) || pItem->sItemInfo.CODE == (sinOA2 | sin31))
	{
		DWORD NowTime = GetPlayTime_T();
		DWORD Time = (60 * 60 * 24 * 7) - (NowTime - pItem->sItemInfo.dwCreateTime);
		strcpy_s(szTemp, sinContinueTimeItem);
		if (DeleteEventItem_TimeOut(&pItem->sItemInfo) == TRUE)
			wsprintf(szTemp2, "%s/r", ExpireItem);
		else
			wsprintf(szTemp2, "%02d:%02d:%02d:%02d/r", (Time / 60 / 60) / 24, (Time / 60 / 60) % 24, (Time / 60) % 60, (Time) % 60);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->sItemInfo.CODE == (sinOR2 | sin50) || pItem->sItemInfo.CODE == (sinOR2 | sin54) ||
		pItem->sItemInfo.CODE == (sinOR2 | sin56) || pItem->sItemInfo.CODE == (sinOA1 | sin50) ||
		pItem->sItemInfo.CODE == (sinOA1 | sin54) || pItem->sItemInfo.CODE == (sinOA1 | sin56) ||
		pItem->sItemInfo.CODE == (sinDS1 | sin32) || pItem->sItemInfo.CODE == (sinOA2 | sin31))
	{
		DWORD NowTime = GetPlayTime_T();
		DWORD Time = (60 * 60 * 24 * 7) - (NowTime - pItem->sItemInfo.dwCreateTime);
		strcpy_s(szTemp, sinContinueTimeItem);
		if (DeleteEventItem_TimeOut(&pItem->sItemInfo) == TRUE)
			wsprintf(szTemp2, "%s/r", ExpireItem);
		else
			wsprintf(szTemp2, "%02d:%02d:%02d:%02d/r", (Time / 60 / 60) / 24, (Time / 60 / 60) % 24, (Time / 60) % 60, (Time) % 60);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	/*if (pItem->sItemInfo.CODE == (sinOA2 | sin32) || pItem->sItemInfo.CODE == (sinOR2 | sin52) ||
		pItem->sItemInfo.CODE == (sinOA1 | sin52) || pItem->sItemInfo.CODE == (sinDB1 | sin32))
	{
		DWORD NowTime = GetPlayTime_T();
		DWORD Time = (60 * 60 * 24 * 30) - (NowTime - pItem->sItemInfo.dwCreateTime);
		strcpy_s(szTemp, sinContinueTimeItem);
		if (DeleteEventItem_TimeOut(&pItem->sItemInfo) == TRUE)
			wsprintf(szTemp2, "%s/r", ExpireItem);
		else
			wsprintf(szTemp2, "%02d:%02d:%02d:%02d/r", (Time / 60 / 60) / 24, (Time / 60 / 60) % 24, (Time / 60) % 60, (Time) % 60);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}*/

	if (pItem->sItemInfo.CODE == (sinDA1 | sin31) || pItem->sItemInfo.CODE == (sinDA2 | sin31) ||
		pItem->sItemInfo.CODE == (sinDA1 | sin33) || pItem->sItemInfo.CODE == (sinDA2 | sin33) ||
		pItem->sItemInfo.CODE == (sinDA1 | sin35) || pItem->sItemInfo.CODE == (sinDA2 | sin35) ||
		pItem->sItemInfo.CODE == (sinDA1 | sin37) || pItem->sItemInfo.CODE == (sinDA2 | sin37) ||
		pItem->sItemInfo.CODE == (sinDA1 | sin39) || pItem->sItemInfo.CODE == (sinDA2 | sin39) ||
		pItem->sItemInfo.CODE == (sinDA1 | sin41) || pItem->sItemInfo.CODE == (sinDA2 | sin41) ||
		pItem->sItemInfo.CODE == (sinDA1 | sin43) || pItem->sItemInfo.CODE == (sinDA2 | sin43) ||
		pItem->sItemInfo.CODE == (sinDA1 | sin45) || pItem->sItemInfo.CODE == (sinDA2 | sin45) ||
		pItem->sItemInfo.CODE == (sinDB1 | sin31) || pItem->sItemInfo.CODE == (sinOA2 | sin31) ||
		pItem->sItemInfo.CODE == (sinOR2 | sin31) || pItem->sItemInfo.CODE == (sinOR2 | sin32)) //Desativado tempo dos brincos
	{
		DWORD NowTime = GetPlayTime_T();
		DWORD Time = (60 * 60 * 24 * 7) - (NowTime - pItem->sItemInfo.dwCreateTime);
		strcpy_s(szTemp, sinContinueTimeItem);
		if (DeleteEventItem_TimeOut(&pItem->sItemInfo) == TRUE)
			wsprintf(szTemp2, "%s/r", ExpireItem);
		else
			wsprintf(szTemp2, "%02d:%02d:%02d:%02d/r", (Time / 60 / 60) / 24, (Time / 60 / 60) % 24, (Time / 60) % 60, (Time) % 60);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

	}

	/*//Trajes 1 dia
	if (pItem->CODE == (sinCA1 | sin31) || pItem->CODE == (sinCA1 | sin33) ||
		pItem->CODE == (sinCA2 | sin31) || pItem->CODE == (sinCA2 | sin33) ||
		pItem->CODE == (sinCA5 | sin01) || pItem->CODE == (sinCA5 | sin02) ||
		pItem->CODE == (sinCA5 | sin03) || pItem->CODE == (sinCA5 | sin05) ||
		pItem->CODE == (sinCA5 | sin07) || pItem->CODE == (sinCA5 | sin08) ||
		pItem->CODE == (sinCA5 | sin47) || pItem->CODE == (sinCA5 | sin48) ||

		pItem->CODE == (sinCA5 | sin82) || pItem->CODE == (sinCA5 | sin83) ||
		pItem->CODE == (sinCA5 | sin84) || pItem->CODE == (sinCA5 | sin85) ||

		pItem->CODE == (sinCA6 | sin01) || pItem->CODE == (sinCA6 | sin02) ||
		pItem->CODE == (sinCA6 | sin03) || pItem->CODE == (sinCA6 | sin05) ||
		pItem->CODE == (sinCA6 | sin07) || pItem->CODE == (sinCA6 | sin08) ||
		pItem->CODE == (sinCA6 | sin47) || pItem->CODE == (sinCA6 | sin48) ||

		pItem->CODE == (sinCA6 | sin82) || pItem->CODE == (sinCA6 | sin83) ||
		pItem->CODE == (sinCA6 | sin84) || pItem->CODE == (sinCA6 | sin85))

	{
		DWORD NowTime = GetPlayTime_T();
		DWORD Time = (60 * 60 * 24 * 1) - (NowTime - pItem->sItemInfo.dwCreateTime);
		strcpy_s(szTemp, sinContinueTimeItem);
		if (DeleteEventItem_TimeOut(&pItem->sItemInfo) == TRUE)
			wsprintf(szTemp2, "%s/r", ExpireItem);
		else
			wsprintf(szTemp2, "%02d:%02d:%02d:%02d/r", (Time / 60 / 60) / 24, (Time / 60 / 60) % 24, (Time / 60) % 60, (Time) % 60);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	//Trajes 3 dias
	if (pItem->CODE == (sinCA5 | sin49) || pItem->CODE == (sinCA5 | sin50) ||
		pItem->CODE == (sinCA5 | sin51) || pItem->CODE == (sinCA5 | sin52) ||
		pItem->CODE == (sinCA5 | sin53) || pItem->CODE == (sinCA5 | sin54) ||
		pItem->CODE == (sinCA5 | sin55) || pItem->CODE == (sinCA5 | sin56) ||
		pItem->CODE == (sinCA5 | sin57) || pItem->CODE == (sinCA5 | sin58) ||
		pItem->CODE == (sinCA5 | sin59) || pItem->CODE == (sinCA5 | sin60) ||
		pItem->CODE == (sinCA5 | sin61) || pItem->CODE == (sinCA5 | sin62) ||
		pItem->CODE == (sinCA5 | sin63) || pItem->CODE == (sinCA5 | sin64) ||

		pItem->CODE == (sinCA6 | sin49) || pItem->CODE == (sinCA6 | sin50) ||
		pItem->CODE == (sinCA6 | sin51) || pItem->CODE == (sinCA6 | sin52) ||
		pItem->CODE == (sinCA6 | sin53) || pItem->CODE == (sinCA6 | sin54) ||
		pItem->CODE == (sinCA6 | sin55) || pItem->CODE == (sinCA6 | sin56) ||
		pItem->CODE == (sinCA6 | sin57) || pItem->CODE == (sinCA6 | sin58) ||
		pItem->CODE == (sinCA6 | sin59) || pItem->CODE == (sinCA6 | sin60) ||
		pItem->CODE == (sinCA6 | sin61) || pItem->CODE == (sinCA6 | sin62) ||
		pItem->CODE == (sinCA6 | sin63) || pItem->CODE == (sinCA6 | sin64))

	{
		DWORD NowTime = GetPlayTime_T();
		DWORD Time = (60 * 60 * 24 * 3) - (NowTime - pItem->sItemInfo.dwCreateTime);

		strcpy_s(szTemp, sinContinueTimeItem);

		if (DeleteEventItem_TimeOut(&pItem->sItemInfo) == TRUE)
			wsprintf(szTemp2, "%s/r", ExpireItem);
		else
			wsprintf(szTemp2, "%02d:%02d:%02d:%02d/r", (Time / 60 / 60) / 24, (Time / 60 / 60) % 24, (Time / 60) % 60, (Time) % 60);

		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	/*////Trajes 7 dias
	if (pItem->CODE == (sinCA1 | sin35) || pItem->CODE == (sinCA1 | sin37) ||
		pItem->CODE == (sinCA1 | sin01) || pItem->CODE == (sinCA1 | sin02) ||
		pItem->CODE == (sinCA1 | sin05) || pItem->CODE == (sinCA1 | sin06) ||
		pItem->CODE == (sinCA1 | sin09) || pItem->CODE == (sinCA1 | sin10) ||
		pItem->CODE == (sinCA1 | sin13) || pItem->CODE == (sinCA1 | sin14) ||
		pItem->CODE == (sinCA1 | sin43) || pItem->CODE == (sinCA1 | sin45) ||
		pItem->CODE == (sinCA1 | sin54) || pItem->CODE == (sinCA1 | sin55) ||
		pItem->CODE == (sinCA1 | sin66) || pItem->CODE == (sinCA1 | sin68) ||
		pItem->CODE == (sinCA1 | sin85) || pItem->CODE == (sinCA1 | sin87) ||
		pItem->CODE == (sinCA1 | sin89) || pItem->CODE == (sinCA1 | sin91) ||
		pItem->CODE == (sinCA1 | sin93) || pItem->CODE == (sinCA1 | sin95) ||

		pItem->CODE == (sinCA2 | sin35) || pItem->CODE == (sinCA2 | sin37) ||
		pItem->CODE == (sinCA2 | sin43) || pItem->CODE == (sinCA2 | sin45) ||
		pItem->CODE == (sinCA2 | sin54) || pItem->CODE == (sinCA2 | sin55) ||
		pItem->CODE == (sinCA2 | sin66) || pItem->CODE == (sinCA2 | sin68) ||
		pItem->CODE == (sinCA2 | sin85) || pItem->CODE == (sinCA2 | sin87) ||
		pItem->CODE == (sinCA2 | sin89) || pItem->CODE == (sinCA2 | sin91) ||
		pItem->CODE == (sinCA2 | sin93) || pItem->CODE == (sinCA2 | sin95) ||

		pItem->CODE == (sinCA5 | sin04) || pItem->CODE == (sinCA5 | sin06) ||
		pItem->CODE == (sinCA5 | sin07) || pItem->CODE == (sinCA5 | sin08) ||
		pItem->CODE == (sinCA5 | sin09) || pItem->CODE == (sinCA5 | sin11) ||
		pItem->CODE == (sinCA5 | sin15) || pItem->CODE == (sinCA5 | sin16) ||
		pItem->CODE == (sinCA5 | sin17) || pItem->CODE == (sinCA5 | sin19) ||
		pItem->CODE == (sinCA5 | sin22) || pItem->CODE == (sinCA5 | sin24) ||
		pItem->CODE == (sinCA5 | sin25) || pItem->CODE == (sinCA5 | sin27) ||
		pItem->CODE == (sinCA5 | sin31) || pItem->CODE == (sinCA5 | sin33) ||
		pItem->CODE == (sinCA5 | sin35) || pItem->CODE == (sinCA5 | sin37) ||
		pItem->CODE == (sinCA5 | sin39) || pItem->CODE == (sinCA5 | sin41) ||
		pItem->CODE == (sinCA5 | sin43) || pItem->CODE == (sinCA5 | sin45) ||
		pItem->CODE == (sinCA5 | sin75) || pItem->CODE == (sinCA5 | sin76) ||
		pItem->CODE == (sinCA5 | sin86) || pItem->CODE == (sinCA5 | sin87) ||

		pItem->CODE == (sinCA6 | sin04) || pItem->CODE == (sinCA6 | sin06) ||
		pItem->CODE == (sinCA6 | sin07) || pItem->CODE == (sinCA6 | sin08) ||
		pItem->CODE == (sinCA6 | sin09) || pItem->CODE == (sinCA6 | sin11) ||
		pItem->CODE == (sinCA6 | sin15) || pItem->CODE == (sinCA6 | sin16) ||
		pItem->CODE == (sinCA6 | sin17) || pItem->CODE == (sinCA6 | sin19) ||
		pItem->CODE == (sinCA6 | sin22) || pItem->CODE == (sinCA6 | sin24) ||
		pItem->CODE == (sinCA6 | sin25) || pItem->CODE == (sinCA6 | sin27) ||
		pItem->CODE == (sinCA6 | sin31) || pItem->CODE == (sinCA6 | sin33) ||
		pItem->CODE == (sinCA6 | sin35) || pItem->CODE == (sinCA6 | sin37) ||
		pItem->CODE == (sinCA6 | sin39) || pItem->CODE == (sinCA6 | sin41) ||
		pItem->CODE == (sinCA6 | sin43) || pItem->CODE == (sinCA6 | sin45) ||

		pItem->CODE == (sinCA7 | sin01) || pItem->CODE == (sinCA7 | sin02) ||
		pItem->CODE == (sinCA7 | sin05) || pItem->CODE == (sinCA7 | sin06) ||
		pItem->CODE == (sinCA7 | sin09) || pItem->CODE == (sinCA7 | sin10) ||
		pItem->CODE == (sinCA7 | sin13) || pItem->CODE == (sinCA7 | sin14) ||

		pItem->CODE == (sinCA6 | sin75) || pItem->CODE == (sinCA6 | sin76) ||
		pItem->CODE == (sinCA6 | sin86) || pItem->CODE == (sinCA6 | sin87))

	{
		DWORD NowTime = GetPlayTime_T();
		DWORD Time = (60 * 60 * 24 * 7) - (NowTime - pItem->sItemInfo.dwCreateTime);
		strcpy_s(szTemp, sinContinueTimeItem);
		if (DeleteEventItem_TimeOut(&pItem->sItemInfo) == TRUE)
			wsprintf(szTemp2, "%s/r", ExpireItem);
		else
			wsprintf(szTemp2, "%02d:%02d:%02d:%02d/r", (Time / 60 / 60) / 24, (Time / 60 / 60) % 24, (Time / 60) % 60, (Time) % 60);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}


	/*//Trajes 30 dias
	if (pItem->CODE == (sinCA5 | sin10) || pItem->CODE == (sinCA5 | sin12) ||
		pItem->CODE == (sinCA5 | sin18) || pItem->CODE == (sinCA5 | sin20) ||
		pItem->CODE == (sinCA5 | sin21) || pItem->CODE == (sinCA5 | sin23) ||
		pItem->CODE == (sinCA5 | sin26) || pItem->CODE == (sinCA5 | sin28) ||
		pItem->CODE == (sinCA5 | sin32) || pItem->CODE == (sinCA5 | sin34) ||
		pItem->CODE == (sinCA5 | sin36) || pItem->CODE == (sinCA5 | sin38) ||
		pItem->CODE == (sinCA5 | sin40) || pItem->CODE == (sinCA5 | sin42) ||
		pItem->CODE == (sinCA5 | sin44) || pItem->CODE == (sinCA5 | sin46) ||

		pItem->CODE == (sinCA5 | sin65) || pItem->CODE == (sinCA5 | sin66) ||
		pItem->CODE == (sinCA5 | sin67) || pItem->CODE == (sinCA5 | sin68) ||
		pItem->CODE == (sinCA5 | sin69) || pItem->CODE == (sinCA5 | sin70) ||
		pItem->CODE == (sinCA5 | sin71) || pItem->CODE == (sinCA5 | sin72) ||
		pItem->CODE == (sinCA5 | sin73) || pItem->CODE == (sinCA5 | sin74) ||
		pItem->CODE == (sinCA5 | sin77) || pItem->CODE == (sinCA5 | sin78) ||

		pItem->CODE == (sinCA6 | sin10) || pItem->CODE == (sinCA6 | sin12) ||
		pItem->CODE == (sinCA6 | sin18) || pItem->CODE == (sinCA6 | sin20) ||
		pItem->CODE == (sinCA6 | sin21) || pItem->CODE == (sinCA6 | sin23) ||
		pItem->CODE == (sinCA6 | sin26) || pItem->CODE == (sinCA6 | sin28) ||
		pItem->CODE == (sinCA6 | sin32) || pItem->CODE == (sinCA6 | sin34) ||
		pItem->CODE == (sinCA6 | sin36) || pItem->CODE == (sinCA6 | sin38) ||
		pItem->CODE == (sinCA6 | sin40) || pItem->CODE == (sinCA6 | sin42) ||
		pItem->CODE == (sinCA6 | sin44) || pItem->CODE == (sinCA6 | sin46) ||

		pItem->CODE == (sinCA6 | sin65) || pItem->CODE == (sinCA6 | sin66) ||
		pItem->CODE == (sinCA6 | sin67) || pItem->CODE == (sinCA6 | sin68) ||
		pItem->CODE == (sinCA6 | sin69) || pItem->CODE == (sinCA6 | sin70) ||
		pItem->CODE == (sinCA6 | sin71) || pItem->CODE == (sinCA6 | sin72) ||
		pItem->CODE == (sinCA6 | sin73) || pItem->CODE == (sinCA6 | sin74) ||
		pItem->CODE == (sinCA6 | sin77) || pItem->CODE == (sinCA6 | sin78) ||
		pItem->CODE == (sinOE1 | sin01) || pItem->CODE == (sinBI2 | sin83))

	{
		DWORD NowTime = GetPlayTime_T();
		DWORD Time = (60 * 60 * 24 * 30) - (NowTime - pItem->sItemInfo.dwCreateTime);
		strcpy_s(szTemp, sinContinueTimeItem);
		if (DeleteEventItem_TimeOut(&pItem->sItemInfo) == TRUE)
			wsprintf(szTemp2, "%s/r", ExpireItem);
		else
			wsprintf(szTemp2, "%02d:%02d:%02d:%02d/r", (Time / 60 / 60) / 24, (Time / 60 / 60) % 24, (Time / 60) % 60, (Time) % 60);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}*/

	if (pItem->sItemInfo.CODE == (sinDA1 | sin32) || pItem->sItemInfo.CODE == (sinDA2 | sin32) ||
		pItem->sItemInfo.CODE == (sinDA1 | sin34) || pItem->sItemInfo.CODE == (sinDA2 | sin34) ||
		pItem->sItemInfo.CODE == (sinDA1 | sin36) || pItem->sItemInfo.CODE == (sinDA2 | sin36) ||
		pItem->sItemInfo.CODE == (sinDA1 | sin38) || pItem->sItemInfo.CODE == (sinDA2 | sin38) ||
		pItem->sItemInfo.CODE == (sinDA1 | sin40) || pItem->sItemInfo.CODE == (sinDA2 | sin40) ||
		pItem->sItemInfo.CODE == (sinDA1 | sin42) || pItem->sItemInfo.CODE == (sinDA2 | sin42) ||
		pItem->sItemInfo.CODE == (sinDA1 | sin44) || pItem->sItemInfo.CODE == (sinDA2 | sin44) ||
		pItem->sItemInfo.CODE == (sinDA1 | sin46) || pItem->sItemInfo.CODE == (sinDA2 | sin46) ||
		pItem->sItemInfo.CODE == (sinDA1 | sin54) || pItem->sItemInfo.CODE == (sinDA2 | sin54) ||
		pItem->sItemInfo.CODE == (sinDA1 | sin55) || pItem->sItemInfo.CODE == (sinDA2 | sin55) ||
		pItem->sItemInfo.CODE == (sinSP1 | sin34))
	{
		DWORD NowTime = GetPlayTime_T();
		DWORD Time = (60 * 60 * 24 * 30) - (NowTime - pItem->sItemInfo.dwCreateTime);
		strcpy_s(szTemp, sinContinueTimeItem);
		if (DeleteEventItem_TimeOut(&pItem->sItemInfo) == TRUE)
			wsprintf(szTemp2, "%s/r", ExpireItem);
		else
			wsprintf(szTemp2, "%02d:%02d:%02d:%02d/r", (Time / 60 / 60) / 24, (Time / 60 / 60) % 24, (Time / 60) % 60, (Time) % 60);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if ((sInfo.CODE & sinITEM_MASK2) == sinBI1)
	{
		for (int i = 0; i < 7; i++)
		{
			if (pItem->CODE == dwPremiumItemCode[i])
			{
				strcpy_s(szTemp, PremiumItemDoc[i][0]);
				wsprintf(szTemp2, "/r");
				lstrcat(szInfoBuff, szTemp);
				lstrcat(szInfoBuff2, szTemp2);
				ItemInfoCol++;

				strcpy_s(szTemp, PremiumItemDoc[i][1]);
				wsprintf(szTemp2, "/r");
				lstrcat(szInfoBuff, szTemp);
				lstrcat(szInfoBuff2, szTemp2);
				ItemInfoCol++;
				break;
			}
		}
	}

	if ((sInfo.CODE & sinITEM_MASK2) == sinSE1)
	{
		for (int i = 0; i < MAX_SEEL_COUNT; i++)
		{
			if (pItem->CODE == sReconItem[i].dwCODE)
			{
				strcpy_s(szTemp, sinAbilityName[28]);
				wsprintf(szTemp2, "%d-%d/r", sReconItem[i].iLevel[0], sReconItem[i].iLevel[1]);
				lstrcat(szInfoBuff, szTemp);
				lstrcat(szInfoBuff2, szTemp2);
				ItemInfoCol++;
			}
		}
	}


	if ((sInfo.CODE & sinITEM_MASK2) == sinBC1)
	{
		for (i = 0; i < 16; i++)
		{
			if ((pItem->CODE & sinITEM_MASK3) == dwCastlItemCODE[i])
			{

				if (i == 5 || i == 6 || i == 7)
				{
					strcpy_s(szTemp, CastlItemInfo[5]);
					wsprintf(szTemp2, "/r");
					lstrcat(szInfoBuff, szTemp);
					lstrcat(szInfoBuff2, szTemp2);
					ItemInfoCol++;

					strcpy_s(szTemp, CastlItemInfo2[5]);
					wsprintf(szTemp2, "/r");
					lstrcat(szInfoBuff, szTemp);
					lstrcat(szInfoBuff2, szTemp2);
					ItemInfoCol++;

					wsprintf(szTemp, AttributeTower[3], 60);
					wsprintf(szTemp2, "/r");
					lstrcat(szInfoBuff, szTemp);
					lstrcat(szInfoBuff2, szTemp2);
					ItemInfoCol++;

					strcpy_s(szTemp, AttributeTower[i - 5]);
					wsprintf(szTemp2, "/r");
					lstrcat(szInfoBuff, szTemp);
					lstrcat(szInfoBuff2, szTemp2);
					ItemInfoCol++;
					RedLine[1] = ItemInfoCol;

					break;
				}
				else if (i > 7)
				{
					strcpy_s(szTemp, CastlItemInfo[6]);
					wsprintf(szTemp2, "/r");
					lstrcat(szInfoBuff, szTemp);
					lstrcat(szInfoBuff2, szTemp2);
					ItemInfoCol++;

					strcpy_s(szTemp, CastlItemInfo2[6]);
					wsprintf(szTemp2, "/r");
					lstrcat(szInfoBuff, szTemp);
					lstrcat(szInfoBuff2, szTemp2);
					ItemInfoCol++;

					wsprintf(szTemp, AttributeTower[3], 60);
					wsprintf(szTemp2, "/r");
					lstrcat(szInfoBuff, szTemp);
					lstrcat(szInfoBuff2, szTemp2);
					ItemInfoCol++;

					strcpy_s(szTemp, CharClassTarget[i - 8]);
					wsprintf(szTemp2, "/r");
					lstrcat(szInfoBuff, szTemp);
					lstrcat(szInfoBuff2, szTemp2);
					ItemInfoCol++;
					RedLine[1] = ItemInfoCol;

					break;
				}
				else
				{
					strcpy_s(szTemp, CastlItemInfo[i]);
					wsprintf(szTemp2, "/r");
					lstrcat(szInfoBuff, szTemp);
					lstrcat(szInfoBuff2, szTemp2);
					ItemInfoCol++;

					strcpy_s(szTemp, CastlItemInfo2[i]);
					wsprintf(szTemp2, "/r");
					lstrcat(szInfoBuff, szTemp);
					lstrcat(szInfoBuff2, szTemp2);
					ItemInfoCol++;

					if (i < 3)
					{

						wsprintf(szTemp, AttributeTower[3], 30);
						wsprintf(szTemp2, "/r");
						lstrcat(szInfoBuff, szTemp);
						lstrcat(szInfoBuff2, szTemp2);
						ItemInfoCol++;
					}
					break;
				}
			}
		}

	}


	if ((sInfo.CODE & sinITEM_MASK2) == sinBC1)
	{
		for (i = 0; i < 9; i++)
		{
			if ((pItem->CODE & sinITEM_MASK3) == dwBoosterItemCode[i])
			{
				int n = 0;
				/*strcpy_s(szTemp, BoosterItemInfo[m]);
				wsprintf(szTemp2, "/r");
				lstrcat(szInfoBuff, szTemp);
				lstrcat(szInfoBuff2, szTemp2);
				ItemInfoCol++;

				strcpy_s(szTemp, BoosterItemInfo2[m]);
				wsprintf(szTemp2, "/r");
				lstrcat(szInfoBuff, szTemp);
				lstrcat(szInfoBuff2, szTemp2);
				ItemInfoCol++;*/

				if (i == 0 || i == 3 || i == 6) n = 0;
				if (i == 1 || i == 4 || i == 7) n = 1;
				if (i == 2 || i == 5 || i == 8) n = 2;
				wsprintf(szTemp, AttributeTower[3], BoosterItem_UseTime[n]);
				wsprintf(szTemp2, "/r");
				lstrcat(szInfoBuff, szTemp);
				lstrcat(szInfoBuff2, szTemp2);
				ItemInfoCol++;

				break;
			}

		}

	}

	if (sInfo.Strength)
	{
		strcpy_s(szTemp, sinAbilityName[29]);
		wsprintf(szTemp2, "%d/r", sInfo.Strength);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		RequireLine[1] = ItemInfoCol;
		if (sInfo.Strength > sinChar->Strength)
		{
			sRequire.rStrength = 1;
			RedLine[1] = ItemInfoCol;

		}

	}

	if (sInfo.Spirit && (sInfo.CODE & sinITEM_MASK2) != sinFO1)
	{
		strcpy_s(szTemp, sinAbilityName[30]);
		wsprintf(szTemp2, "%d/r", sInfo.Spirit);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		RequireLine[2] = ItemInfoCol;
		if (sInfo.Spirit > sinChar->Spirit)
		{
			sRequire.rSpirit = 1;
			RedLine[2] = ItemInfoCol;

		}

	}

	if (sInfo.Talent)
	{
		strcpy_s(szTemp, sinAbilityName[31]);
		wsprintf(szTemp2, "%d/r", sInfo.Talent);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		RequireLine[3] = ItemInfoCol;
		if (sInfo.Talent > sinChar->Talent)
		{
			sRequire.rTalent = 1;
			RedLine[3] = ItemInfoCol;

		}
	}

	if (sInfo.Dexterity)
	{
		strcpy_s(szTemp, sinAbilityName[32]);
		wsprintf(szTemp2, "%d/r", sInfo.Dexterity);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		RequireLine[4] = ItemInfoCol;
		if (sInfo.Dexterity > sinChar->Dexterity)
		{
			sRequire.rDexterity = 1;
			RedLine[4] = ItemInfoCol;

		}

	}
	if (sInfo.Health)
	{
		strcpy_s(szTemp, sinAbilityName[33]);
		wsprintf(szTemp2, "%d/r", sInfo.Health);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		RequireLine[5] = ItemInfoCol;
		if (sInfo.Health > sinChar->Health)
		{
			sRequire.rHealth = 1;
			RedLine[5] = ItemInfoCol;
		}

	}

	ShowItemInfo2(pItem, Flag, Index);
	return TRUE;
}

int cITEM::ShowItemInfo2(sITEM* pItem, int Flag, int Index)
{
	int   BlockRate = 0;
	int   sinSpeed = 0;
	int   sinAbsorb = 0;
	int   sinLeftSpot = 0;
	int   sinRightSpot = 0;
	int sinMagic_Mastery;

	int   sinRegen = 0;

	char	szTemp[64];
	char    szTemp2[64];

	sRequire.rDexterity = 0;
	sRequire.rHealth = 0;
	sRequire.rLevel = 0;
	sRequire.rBattleLevel = 0;
	sRequire.rSpirit = 0;
	sRequire.rStrength = 0;
	sRequire.rTalent = 0;

	memset(&szTemp, 0, sizeof(szTemp));
	memset(&szTemp2, 0, sizeof(szTemp2));



	if (sInfo.JobCodeMask && (sInfo.CODE & sinITEM_MASK2) != sinFO1)
	{
		SearchSpecialItemJob(sInfo.JobCodeMask);

	}

	if (sInfo.JobItem.Add_Attack_Speed)
	{
		strcpy_s(szTemp, sinSpecialName[0]);
		wsprintf(szTemp2, "%d/r", sInfo.JobItem.Add_Attack_Speed);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

	}
	if (sInfo.JobItem.Add_Critical_Hit)
	{
		strcpy_s(szTemp, sinSpecialName[1]);
		wsprintf(szTemp2, "%d%s/r", sInfo.JobItem.Add_Critical_Hit, "%");
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

	}
	if (sInfo.JobItem.Add_Defence)
	{
		strcpy_s(szTemp, sinSpecialName[2]);
		wsprintf(szTemp2, "%d/r", sInfo.JobItem.Add_Defence);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

	}
	if (sInfo.JobItem.Add_fAbsorb)
	{
		sinAbsorb = (int)(sInfo.JobItem.Add_fAbsorb * 10.001f);
		sinRightSpot = sinAbsorb % 10;
		sinLeftSpot = (sinAbsorb - sinRightSpot) / 10;
		strcpy_s(szTemp, sinSpecialName[3]);
		wsprintf(szTemp2, "%d.%d/r", sinLeftSpot, sinRightSpot);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

	}
	if (sInfo.JobItem.Add_fBlock_Rating)
	{
		BlockRate = (int)(sInfo.JobItem.Add_fBlock_Rating);
		strcpy_s(szTemp, sinSpecialName[4]);
		wsprintf(szTemp2, "%d%s/r", BlockRate, "%");
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

	}

	if (sInfo.JobItem.Add_fMagic_Mastery)
	{
		sinMagic_Mastery = (int)(sInfo.JobItem.Add_fMagic_Mastery);
		strcpy_s(szTemp, sinSpecialName[5]);
		wsprintf(szTemp2, "%d/r", sinMagic_Mastery);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

	}

	if (sInfo.JobItem.Add_fSpeed)
	{
		sinSpeed = (int)(sInfo.JobItem.Add_fSpeed * 10.001f);
		sinRightSpot = sinSpeed % 10;
		sinLeftSpot = (sinSpeed - sinRightSpot) / 10;
		strcpy_s(szTemp, sinSpecialName[6]);
		wsprintf(szTemp2, "%d.%d/r", sinLeftSpot, sinRightSpot);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

	}

	if (sInfo.JobItem.Add_Resistance[0])
	{
		strcpy_s(szTemp, sinSpecialName[7]);
		wsprintf(szTemp2, "%d/r", sInfo.JobItem.Add_Resistance[0]);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}
	if (sInfo.JobItem.Add_Resistance[1])
	{
		strcpy_s(szTemp, sinSpecialName[8]);
		wsprintf(szTemp2, "%d/r", sInfo.JobItem.Add_Resistance[1]);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (sInfo.JobItem.Add_Resistance[2])
	{
		strcpy_s(szTemp, sinSpecialName[9]);
		wsprintf(szTemp2, "%d/r", sInfo.JobItem.Add_Resistance[2]);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

	}
	if (sInfo.JobItem.Add_Resistance[3])
	{
		strcpy_s(szTemp, sinSpecialName[10]);
		wsprintf(szTemp2, "%d/r", sInfo.JobItem.Add_Resistance[3]);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

	}
	if (sInfo.JobItem.Add_Resistance[4])
	{
		strcpy_s(szTemp, sinSpecialName[11]);
		wsprintf(szTemp2, "%d/r", sInfo.JobItem.Add_Resistance[4]);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

	}
	if (sInfo.JobItem.Add_Resistance[5])
	{
		strcpy_s(szTemp, sinSpecialName[12]);
		wsprintf(szTemp2, "%d/r", sInfo.JobItem.Add_Resistance[5]);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

	}
	if (sInfo.JobItem.Add_Resistance[6])
	{
		strcpy_s(szTemp, sinSpecialName[13]);
		wsprintf(szTemp2, "%d/r", sInfo.JobItem.Add_Resistance[6]);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

	}
	if (sInfo.JobItem.Add_Resistance[7])
	{
		strcpy_s(szTemp, sinSpecialName[14]);
		wsprintf(szTemp2, "%d/r", sInfo.JobItem.Add_Resistance[7]);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

	}
	if (sInfo.JobItem.Lev_Damage[1])
	{
		strcpy_s(szTemp, sinSpecialName[15]);
		wsprintf(szTemp2, "LV/%d/r", sInfo.JobItem.Lev_Damage[1]);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

	}

	if (sInfo.JobItem.Lev_Attack_Rating)
	{
		strcpy_s(szTemp, sinSpecialName[16]);
		wsprintf(szTemp2, "LV/%d/r", sInfo.JobItem.Lev_Attack_Rating);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

	}

	if (sInfo.JobItem.Add_Shooting_Range)
	{
		strcpy_s(szTemp, sinSpecialName[17]);
		wsprintf(szTemp2, "%d/r", sInfo.JobItem.Add_Shooting_Range);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

	}

	if (sInfo.JobItem.Lev_Attack_Resistance[0])
	{
		strcpy_s(szTemp, sinSpecialName[18]);
		wsprintf(szTemp2, "%d/r", sInfo.JobItem.Lev_Attack_Resistance[0]);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

	}
	if (sInfo.JobItem.Lev_Attack_Resistance[1])
	{
		strcpy_s(szTemp, sinSpecialName[19]);
		wsprintf(szTemp2, "%d/r", sInfo.JobItem.Lev_Attack_Resistance[1]);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

	}
	if (sInfo.JobItem.Lev_Attack_Resistance[2])
	{
		strcpy_s(szTemp, sinSpecialName[20]);
		wsprintf(szTemp2, "%d/r", sInfo.JobItem.Lev_Attack_Resistance[2]);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

	}
	if (sInfo.JobItem.Lev_Attack_Resistance[3])
	{
		strcpy_s(szTemp, sinSpecialName[21]);
		wsprintf(szTemp2, "%d/r", sInfo.JobItem.Lev_Attack_Resistance[3]);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

	}
	if (sInfo.JobItem.Lev_Attack_Resistance[4])
	{
		wsprintf(szTemp, sinSpecialName[22]);
		wsprintf(szTemp2, "%d/r", sInfo.JobItem.Lev_Attack_Resistance[4]);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

	}

	if (sInfo.JobItem.Lev_Attack_Resistance[5])
	{
		strcpy_s(szTemp, sinSpecialName[23]);
		wsprintf(szTemp2, "%d/r", sInfo.JobItem.Lev_Attack_Resistance[5]);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;


	}
	if (sInfo.JobItem.Lev_Attack_Resistance[6])
	{
		strcpy_s(szTemp, sinSpecialName[24]);
		wsprintf(szTemp2, "%d/r", sInfo.JobItem.Lev_Attack_Resistance[6]);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

	}
	if (sInfo.JobItem.Lev_Attack_Resistance[7])
	{
		strcpy_s(szTemp, sinSpecialName[25]);
		wsprintf(szTemp2, "%d/r", sInfo.JobItem.Lev_Attack_Resistance[7]);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

	}
	if (sInfo.JobItem.Lev_Life)
	{
		strcpy_s(szTemp, sinSpecialName[26]);
		wsprintf(szTemp2, "LV/%d/r", sInfo.JobItem.Lev_Life);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

	}
	if (sInfo.JobItem.Lev_Mana)
	{
		strcpy_s(szTemp, sinSpecialName[27]);
		wsprintf(szTemp2, "LV/%d/r", sInfo.JobItem.Lev_Mana);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

	}
	int sinPer_Regen;
	if (sInfo.JobItem.Per_Life_Regen && (sInfo.CODE & sinITEM_MASK2) != sinFO1)
	{
		sinPer_Regen = (int)(sInfo.JobItem.Per_Life_Regen * 10.001f);
		sinRightSpot = sinPer_Regen % 10;
		sinLeftSpot = (sinPer_Regen - sinRightSpot) / 10;

		strcpy_s(szTemp, sinSpecialName[28]);
		wsprintf(szTemp2, "%d.%d/r", sinLeftSpot, sinRightSpot);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

	}
	if (sInfo.JobItem.Per_Mana_Regen && (sInfo.CODE & sinITEM_MASK2) != sinFO1)
	{
		sinPer_Regen = (int)(sInfo.JobItem.Per_Mana_Regen * 10.001f);
		sinRightSpot = sinPer_Regen % 10;
		sinLeftSpot = (sinPer_Regen - sinRightSpot) / 10;
		strcpy_s(szTemp, sinSpecialName[29]);
		wsprintf(szTemp2, "%d.%d/r", sinLeftSpot, sinRightSpot);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

	}

	if (sInfo.JobItem.Per_Stamina_Regen && (sInfo.CODE & sinITEM_MASK2) != sinFO1)
	{
		sinPer_Regen = (int)(sInfo.JobItem.Per_Stamina_Regen * 10.001f);
		sinRightSpot = sinPer_Regen % 10;
		sinLeftSpot = (sinPer_Regen - sinRightSpot) / 10;
		strcpy_s(szTemp, sinSpecialName[30]);
		wsprintf(szTemp2, "%d.%d/r", sinLeftSpot, sinRightSpot);
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

	}


	char szTemp7[64];
	memset(szTemp7, 0, sizeof(szTemp7));
	if (cShop.OpenFlag)
	{
		strcpy_s(szTemp, "/r");
		strcpy_s(szTemp2, "/r");
		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		if (sInfo.Price && pItem->CODE != (sinGF1 | sin01) && pItem->CODE != (sinGF1 | sin02) && ((sInfo.CODE & sinITEM_MASK2) != sinGB1))
		{
			if (Flag == 2)
			{
				sinItemPrice = cInvenTory.GetInvenItemPrice(pItem);
				pItem->SellPrice = sinItemPrice.SellPrice;
				strcpy_s(szTemp, sinSpecialName[31]);

				NumLineComa(pItem->SellPrice, szTemp7);
				strcpy_s(szTemp2, szTemp7);
				lstrcat(szTemp2, "/r");

			}
			if (Flag == 1)
			{
				strcpy_s(szTemp, sinSpecialName[32]);

#ifdef HASIEGE_MODE

				NumLineComa(cShop.haShopItemPrice(sInfo.Price), szTemp7);
#else
				NumLineComa(sInfo.Price, szTemp7);
#endif


				strcpy_s(szTemp2, szTemp7);
				lstrcat(szTemp2, "/r");




			}

			lstrcat(szInfoBuff, szTemp);
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
		}

	}
	MyShopItemPriceLine = 0;
	MyShopItemPriceLineRED = 0;
	MyShopItemPriceLineGREEN = 0;
	if (cCharShop.OpenFlag)
	{
		if (Flag == 3)
		{
			strcpy_s(szTemp, "/r");
			strcpy_s(szTemp2, "/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;

			strcpy_s(szTemp, MyShopItemSell5);
			NumLineComa(MyShopItemSellMoney2, szTemp7);
			strcpy_s(szTemp2, szTemp7);
			lstrcat(szTemp2, "    /r");

			lstrcat(szInfoBuff, szTemp);
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;

			if (sinChar->Money < MyShopItemSellMoney2) {
				MyShopItemPriceLineRED = ItemInfoCol;
			}
			else
			{
				MyShopItemPriceLine = ItemInfoCol;
			}


		}


		else if (Flag == 4)
		{
			lstrcpy(szTemp, "/r");
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;

			lstrcpy(szTemp, "Coins: /r");
			NumLineComa(MyShopItemSellMoney2, szTemp7);
			lstrcpy(szTemp2, szTemp7);
			lstrcat(szTemp2, "    /r");

			lstrcat(szInfoBuff, szTemp);
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			MyShopItemPriceLine = ItemInfoCol;
		}
	}

	if (cMyShop.OpenFlag)
	{
		if (Flag == 3)
		{
			strcpy_s(szTemp, "/r");
			strcpy_s(szTemp2, "/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;

			strcpy_s(szTemp, MyShopItemSell5);
			NumLineComa(MyShopItemSellMoney2, szTemp7);
			strcpy_s(szTemp2, szTemp7);
			lstrcat(szTemp2, "    /r");

			lstrcat(szInfoBuff, szTemp);
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;

			if (sinChar->Money < MyShopItemSellMoney2) {
				MyShopItemPriceLineGREEN = ItemInfoCol;
			}
			else
			{
				MyShopItemPriceLine = ItemInfoCol;
			}
		}

		else if (Flag == 4)
		{
			lstrcpy(szTemp, "/r");
			lstrcpy(szTemp2, "/r");
			lstrcat(szInfoBuff, szTemp);
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;

			lstrcpy(szTemp, "Coins: /r");
			NumLineComa(MyShopItemSellMoney2, szTemp7);
			lstrcpy(szTemp2, szTemp7);
			lstrcat(szTemp2, "    /r");

			lstrcat(szInfoBuff, szTemp);
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			MyShopItemPriceLine = ItemInfoCol;
		}
	}

	if (pItem->CODE == (sinGF1 | sin01))
	{
		lstrcat(szInfoBuff, "/r");
		lstrcat(szInfoBuff2, "/r");
		ItemInfoCol++;

		wsprintf(szTemp, sinGold, sInfo.Price);
		lstrcat(szTemp, "/r");
		lstrcat(szInfoBuff, szTemp);
		strcpy_s(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (((sInfo.CODE & sinITEM_MASK2) == sinGB1))
	{
		strcpy_s(szTemp, sinSpecialName[31]);

#ifdef HASIEGE_MODE
		NumLineComa(cShop.haShopItemPrice(sInfo.Price), szTemp7);
#else
		NumLineComa(sInfo.Price, szTemp7);
#endif

		strcpy_s(szTemp2, szTemp7);
		lstrcat(szTemp2, "/r");

		lstrcat(szInfoBuff, szTemp);
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
	}

	if (pItem->sItemInfo.SpecialItemFlag[0] == CHECK_COPY_ITEM)
	{
		lstrcat(szInfoBuff, "/r");
		lstrcat(szInfoBuff2, "/r");
		ItemInfoCol++;

		strcpy_s(szTemp, sinCopyItem5);
		lstrcat(szInfoBuff, szTemp);
		strcpy_s(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		RedLine[6] = ItemInfoCol;

	}

	if (pItem->sItemInfo.SpecialItemFlag[0] == CHECK_GIVE_ITEM)
	{
		lstrcat(szInfoBuff, "/r");
		lstrcat(szInfoBuff2, "/r");
		ItemInfoCol++;

		strcpy_s(szTemp, sinGiveItem5);
		lstrcat(szInfoBuff, szTemp);
		strcpy_s(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		RedLine[7] = ItemInfoCol;

	}

	if (cItem.GetItemLimitTime(pItem) == FALSE)
	{

		lstrcat(szInfoBuff, "/r");
		lstrcat(szInfoBuff2, "/r");
		ItemInfoCol++;

		strcpy_s(szTemp, sinItemLimitTimeOverMsg);
		lstrcat(szInfoBuff, szTemp);
		strcpy_s(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;
		RedLine[8] = ItemInfoCol;

	}

	if (MyShopItemIndex[Index])
	{
		if (Flag == 2)
		{
			lstrcat(szInfoBuff, "/r");
			lstrcat(szInfoBuff2, "/r");
			ItemInfoCol++;

			wsprintf(szTemp, "%s", NowMyShopSell);
			lstrcat(szInfoBuff, szTemp);
			strcpy_s(szTemp2, "/r");
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;

			RedLine[9] = ItemInfoCol;
		}
	}

	if (smConfig.getSecurity() >= AccountTypes::SEC_MODERATOR && VRKeyBuff[VK_CONTROL])
	{
		wsprintf(szTemp, "Code: %s/r", pItem->LastCategory);
		lstrcat(szInfoBuff, szTemp);
		strcpy_s(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		wsprintf(szTemp, "Checksum: %d/r", pItem->sItemInfo.ItemHeader.dwChkSum);
		lstrcat(szInfoBuff, szTemp);
		strcpy_s(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

		wsprintf(szTemp, "Head: %d/r", pItem->sItemInfo.ItemHeader.Head);
		lstrcat(szInfoBuff, szTemp);
		strcpy_s(szTemp2, "/r");
		lstrcat(szInfoBuff2, szTemp2);
		ItemInfoCol++;

	}

	GetInfoBoxSize(pItem, ItemInfoCol);

	sinLineCount = ItemInfoCol;

	return TRUE;

}


int cITEM::GetInfoBoxSize(sITEM* pItem, int Col)
{
	int ty, tx;

	ItemBoxSize.x = 13;
	ItemBoxSize.y = Col + 1;

	ItemBoxPosi.x = (pItem->x + (pItem->w / 2)) - ((ItemBoxSize.x * 16) / 2);
	ItemBoxPosi.y = pItem->y - (ItemBoxSize.y * 16);


	ty = ItemBoxPosi.y + (ItemBoxSize.y * 16);
	tx = ItemBoxPosi.x + (ItemBoxSize.x * 16);


	if (ItemBoxPosi.y < 0)
		ItemBoxPosi.y = 0;
	if (ItemBoxPosi.x < 0)
		ItemBoxPosi.x = 0;

	if (ty > 600)
		ItemBoxPosi.y -= (ty - 600);
	if (tx > 800)
		ItemBoxPosi.x -= (tx - 800);

	return TRUE;
}

int cITEM::DrawItemText()
{
	if (!sinShowItemInfoFlag)return FALSE;
	if (sinShowItemInfoFlag == 1)dsDrawOffsetArray = dsARRAY_TOP;
	else dsDrawOffsetArray = dsARRAY_BOTTOM;
	HDC	hdc = NULL;


	int i, Count = 0;
	int len = 0, len2 = 0;

	int TempLen = 0;
	int TempLen2 = 0;

	int Templen = 0;
	int TemplenNum = 0;

	int ImsiLen = 0;


	int Textlen = 0;
	int Textlen2 = 0;

	int SetTextXposi = 0;
	int SetTextXposi2 = 0;


	int CutLen = 0;
	int CutLen2 = 0;

	int j = 0;


	char* pItemInfo[40];
	char* pItemInfo2[40];



	int  BlodCheckFlag = 0;

	len = lstrlen(szInfoBuff);
	len2 = lstrlen(szInfoBuff2);




	char szInfoBuffBack[5000] = { 0 };
	char szInfoBuff2Back[5000] = { 0 };
	char szAgingNum[32] = { 0 };

	int  TempLen3 = 0;

	lstrcpy(szInfoBuffBack, szInfoBuff);
	lstrcpy(szInfoBuff2Back, szInfoBuff2);

	for (i = 0; i < len; i++) {
		for (j = 0; j < len2; j++) {
			if (szInfoBuffBack[i] == '/' && i + 1 < len && szInfoBuffBack[i + 1] == 'r') {
				if (szInfoBuff2Back[j] == '/' && j + 1 < len2 && szInfoBuff2Back[j + 1] == 'r') {
					pItemInfo[Count] = &szInfoBuffBack[TempLen];
					pItemInfo2[Count] = &szInfoBuff2Back[TempLen2];

					szInfoBuffBack[i] = 0;
					szInfoBuff2Back[j] = 0;

					TempLen = i + 2;
					TempLen2 = j + 2;

					Textlen = lstrlen(pItemInfo[Count]);
					Textlen2 = lstrlen(pItemInfo2[Count]);

					SetTextXposi = (ItemBoxPosi.x + (158 / 2)) - (Textlen * 6);
					SetTextXposi2 = (ItemBoxPosi.x + (158 / 2));



					SetTextXposi += 20;
					SetTextXposi2 += 17;


					if (*pItemInfo2[Count] == ' ' || Textlen2 < 1) {
						SetTextXposi = (ItemBoxPosi.x + (158 / 2)) - (Textlen * 3);
					}

					char text[100] = { 0 };
					SIZE textsizecheck = { 0 };

					strcat_s(text, sizeof(text), pItemInfo[Count]);
					strcat_s(text, sizeof(text), pItemInfo2[Count]);


					// ALINHAMENTO DE ITEM BOX OROGINAL
					GetFontTextExtentPoint(text, strlen(text), &textsizecheck);

					SetTextXposi = ItemBoxPosi.x + (((ItemBoxSize.x * 13) - textsizecheck.cx) / 1);

					GetFontTextExtentPoint(pItemInfo[Count], strlen(pItemInfo[Count]), &textsizecheck);
					SetTextXposi2 = SetTextXposi + textsizecheck.cx;


					// CORES DA ITEM BOX
					SetFontTextColor(RGB(255, 255, 255));

					if (Count + 1 == 1)
					{

						if (UniFlag)
						{
							SetFontTextColor(RGB(255, 255, 255));
						}
						else
						{
							SetFontTextColor(RGB(255, 255, 255));

							if (sinItemKindFlag)
							{
								switch (sinItemKindFlag)
								{
								case ITEM_KIND_CRAFT:
								{
									SetFontTextColor(RGB(255, 110, 255));
								}
								break;

								case ITEM_KIND_AGING:
								{
									SetFontTextColor(RGB(100, 230, 255));
								}
								break;

								case ITEM_KIND_QUEST_WEAPON:
								{
									SetFontTextColor(RGB(165, 157, 140));
								}
								break;

								case 100:
								{
									SetFontTextColor(RGB(255, 100, 29));
								}
								break;

								}
							}
						}

						dsTextLineOut(ItemBoxPosi.x + 15, ItemBoxPosi.y + 27 + ((Count - 1) * 14), pItemInfo[Count], Textlen);

						// Desativa aqui

						if (AgingGageFlag) {

							SetFontTextColor(RGB(100, 100, 100));

							wsprintf(szAgingNum, "+%d", AgingLevel4);

							if (AgingLevel4 < 10)
								dsTextLineOut(ItemBoxPosi.x + 104, ItemBoxPosi.y + 36, szAgingNum, lstrlen(szAgingNum));
							else
								dsTextLineOut(ItemBoxPosi.x + 100, ItemBoxPosi.y + 36, szAgingNum, lstrlen(szAgingNum));

						}

						BlodCheckFlag = 1;
					}

					SetFontTextColor(RGB(255, 240, 215)); // Status do item sem mix  255, 245, 240 

					for (j = 0; j < 10; j++)
					{
						int perfectItemId = 2;

						if (AgingGageFlag && AgingGageFlag == 1)
						{
							perfectItemId = 4;
						}

						else if (Count + 1 == MixItemLine[j])
						{
							SetFontTextColor(RGB(255, 110, 255)); // Colour mixed item
						}

						if (AgingGageFlag)
						{
							if (Count + 1 == AgingItemLine[j])
							{
								SetFontTextColor(RGB(100, 230, 255)); // Cor do atributo Aging

							}

						}

					}

					if (SpecialItemLine)
					{
						if (SpecialItemLine <= Count + 2)
						{
							SetFontTextColor(RGB(255, 220, 0)); // Spec adicional

						}
						if (SpecialItemLine == Count + 2 || (SpecialItemLine + (CountSpecialName - 1)) == Count + 2)
						{
							SetFontTextColor(RGB(255, 220, 0)); // Spec colour adicional ( ex: Pike, Archer, Magician )
							dsTextLineOut(ItemBoxPosi.x + 15, ItemBoxPosi.y + 28 + ((Count - 1) * 14), pItemInfo[Count], Textlen);
							BlodCheckFlag = TRUE;
						}

					}

					for (j = 0; j < 10; j++)
					{
						if (Count + 1 == RequireLine[j])
						{
							if (Count + 1 != RedLine[j])
							{
								SetFontTextColor(RGB(125, 120, 100)); // Required status true
								dsTextLineOut(ItemBoxPosi.x + 15, ItemBoxPosi.y + 28 + ((Count - 1) * 14), pItemInfo[Count], Textlen);
								dsTextLineOut(SetTextXposi2, ItemBoxPosi.y + 28 + ((Count - 1) * 14), pItemInfo2[Count], Textlen2);
								BlodCheckFlag = TRUE;
							}
						}

						if (Count + 1 == RedLine[j])
						{
							SetFontTextColor(RGB(255, 0, 0)); // Required status false
							dsTextLineOut(ItemBoxPosi.x + 15, ItemBoxPosi.y + 28 + ((Count - 1) * 14), pItemInfo[Count], Textlen);
							dsTextLineOut(SetTextXposi2, ItemBoxPosi.y + 28 + ((Count - 1) * 14), pItemInfo2[Count], Textlen2);
							BlodCheckFlag = TRUE;
						}

					}

					if (cShop.OpenFlag)
					{
						if (Count == sinLineCount - 1)
							SetFontTextColor(RGB(247, 243, 193));
					}


					if (Count + 1 == QuestItemLine)
					{

						SetFontTextColor(RGB(255, 132, 50));

					}

					if (Count + 1 == MyShopItemPriceLine)
					{

						SetFontTextColor(RGB(65, 177, 255));
					}

					if (Count + 1 == MyShopItemPriceLineRED)
					{

						SetFontTextColor(RGB(255, 0, 0));
					}

					if (Count + 1 == MyShopItemPriceLineGREEN)
					{

						SetFontTextColor(RGB(0, 255, 0));
					}

					if (Count + 1 == ItemPerfLine)
					{

						SetFontTextColor(RGB(0, 191, 255));

						dsTextLineOut(ItemBoxPosi.x + 15, ItemBoxPosi.y + 12 + ((Count - 1) * 14), pItemInfo[Count], Textlen);


						BlodCheckFlag = TRUE;
					}

					if (Count + 1 == ItemSkinLine)
					{
						SetFontTextColor(RGB(210, 187, 113));

						dsTextLineOut(ItemBoxPosi.x + 15, ItemBoxPosi.y + 12 + ((Count - 1) * 14), pItemInfo[Count], Textlen);

						BlodCheckFlag = TRUE;
					}

					if (!BlodCheckFlag)
					{
						dsTextLineOut(SetTextXposi2, ItemBoxPosi.y + 28 + ((Count - 1) * 14), pItemInfo2[Count], Textlen2);
						COLORREF colour = GetTextColor(hdc);
						SetFontTextColor(RGB(255, 255, 255));

						//Aplicar cor no nome do mix
						if (sMixTypeColor.iRow == Count)
						{
							switch (sMixTypeColor.iMixType)
							{
							case 1:
								SetTextColor(hdc, RGB(112, 219, 219));
								break;
							case 2:
								SetTextColor(hdc, RGB(190, 30, 150));
								break;
							case 3:
								SetTextColor(hdc, RGB(212, 189, 32));
								break;
							case 4:
								SetTextColor(hdc, RGB(0, 255, 153));
								break;
							case 5:
								SetTextColor(hdc, RGB(200, 200, 200));
								break;
							}
						}

						if (Count + 1 == trajeM)
						{

							SetFontTextColor(RGB(40, 122, 210));
						}

						if (Count + 1 == trajeF)
						{

							SetFontTextColor(RGB(240, 120, 176));
						}

						if (Count + 1 == NameMixLine && sinItemKindFlag == 1)
						{
							SetFontTextColor(RGB(200, 200, 200));
						}

						//Texto : Set Boss #
						if (Count + 1 == ItemBossLine && ItemBoss1 == 1)
						{
							SetTextColor(0, RGB(0, 102, 102));
						}
						//Texto : Valento Ring
						if (Count + 1 == Ring1 && ItemBoss1 == 1)
						{
							if (LValRing + RValRing == 2)
								SetTextColor(0, RGB(0, 102, 102));
							else
								SetTextColor(0, RGB(0, 102, 102));
						}
						//Texto : Kelvezu Amulet
						if (Count + 1 == Amulet1 && ItemBoss1 == 1)
						{
							if (KelAmulet == 1)
								SetTextColor(0, RGB(0, 102, 102));
							else
								SetTextColor(0, RGB(0, 102, 102));
						}
						// Texto : Efeito Bonus
						if (Count + 1 == Efect1Line && ItemBoss1 == 1)
						{
							SetTextColor(0, RGB(0, 102, 102));
						}
						//Texto : Bonus 1
						if (Count + 1 == Efect1Line2 && ItemBoss1 == 1)
						{
							if (SetBonus1 >= 2)
								SetTextColor(0, RGB(0, 102, 102));
							else
								SetTextColor(0, RGB(0, 102, 102));
						}
						//Texto : Bonus 2
						if (Count + 1 == Efect1Line3 && ItemBoss1 == 1)
						{
							if (SetBonus1 >= 3)
								SetTextColor(0, RGB(0, 102, 102));
							else
								SetTextColor(0, RGB(0, 102, 102));
						}
						//Texto : Set Boss #2
						if (Count + 1 == ItemBossLine && ItemBoss2 == 1)
						{
							SetTextColor(0, RGB(0, 102, 102));
						}
						//Texto : Shy Ring
						if (Count + 1 == Ring2 && ItemBoss2 == 1)
						{
							if (LShyRing + RShyRing == 2)
								SetTextColor(0, RGB(0, 102, 102));
							else
								SetTextColor(0, RGB(0, 102, 102));
						}
						//Texto : Tulla Amulet
						if (Count + 1 == Amulet2 && ItemBoss2 == 1)
						{
							if (TullaAmulet == 1)
								SetTextColor(0, RGB(0, 102, 102));
							else
								SetTextColor(0, RGB(0, 102, 102));
						}
						// Texto : Efeito Bonus
						if (Count + 1 == Efect2Line && ItemBoss2 == 1)
						{
							SetTextColor(0, RGB(0, 102, 102));
						}
						//Texto : Bonus 1
						if (Count + 1 == Efect2Line2 && ItemBoss2 == 1)
						{
							if (SetBonus2 >= 2)
								SetTextColor(0, RGB(0, 102, 102));
							else
								SetTextColor(0, RGB(0, 102, 102));
						}
						//Texto : Bonus 2
						if (Count + 1 == Efect2Line3 && ItemBoss2 == 1)
						{
							if (SetBonus2 >= 3)
								SetTextColor(0, RGB(0, 102, 102));
							else
								SetTextColor(0, RGB(0, 102, 102));
						}

						dsTextLineOut(ItemBoxPosi.x + 15, ItemBoxPosi.y + 28 + ((Count - 1) * 14), pItemInfo[Count], Textlen);
						SetFontTextColor(colour);
					}

					BlodCheckFlag = 0;

					Count++;
					break;
				}
			}

		}

		BlodCheckFlag = 0;
	}


	return TRUE;

}

int cITEM::SearchSpecialItemJob(DWORD SpecialJob_CODE)
{


	int cnt;

	char	szTemp[64];
	char    szTemp2[64];
	CountSpecialName = 0;

	cnt = 0;

	lstrcpy(szTemp2, "  /r");
	lstrcat(szInfoBuff, szTemp2);
	lstrcat(szInfoBuff2, szTemp2);
	ItemInfoCol++;


	while (1)
	{
		if (JobDataBase[cnt].JobBitCode == 0)
			break;



		if (JobDataBase[cnt].JobBitCode & SpecialJob_CODE)
		{
			wsprintf(szTemp, SpecialName3, JobDataBase[cnt].szName2);
			lstrcpy(szTemp2, "  /r");
			lstrcat(szInfoBuff, szTemp);
			lstrcat(szInfoBuff2, szTemp2);
			ItemInfoCol++;
			SpecialItemLine = ItemInfoCol + 1;

			CountSpecialName++;
			SpecialItemLine -= (CountSpecialName - 1);

		}
		cnt++;
	}
	return FALSE;
}

int ItemTableCheckDelayTime = 0;

int cITEM::CheckItemTable()
{
	ItemTableCheckDelayTime++;
	if (ItemTableCheckDelayTime < 70 * 10)return FALSE;
	ItemTableCheckDelayTime = 0;


	DWORD CheckSumItemDataADD = 0;

	for (int j = 0; j < INVENTORY_MAX_POS; j++)
	{
		if (sInven[j].Position)
		{
			CheckSumItemDataADD += sInven[j].Position * j;
		}
	}

	for (int i = 0; i < MAX_ITEM; i++)
	{
		if (sItem[i].CODE)
		{
			CheckSumItemDataADD += sItem[i].CODE * i;
			CheckSumItemDataADD += sItem[i].h * i;
			CheckSumItemDataADD += sItem[i].w * i;
			CheckSumItemDataADD += sItem[i].Class * i;
			CheckSumItemDataADD += sItem[i].ItemPosition * i;
		}
	}
	const DWORD CheckSumItemData = 3977515816;

	// Checksum item table here




	return TRUE;
}


int cITEM::GetItemLimitTime(sITEM* pItem)
{


	return TRUE;
}



