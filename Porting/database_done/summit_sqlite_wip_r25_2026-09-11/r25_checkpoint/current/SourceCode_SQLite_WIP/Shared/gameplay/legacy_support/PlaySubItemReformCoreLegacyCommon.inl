static DWORD GetRefromItemCode(sITEMINFO *lpItemInfo, int Key)
{

	iFormCode = 0;
	iFromCnt = Key;

	ReformStateCode((char *)&lpItemInfo->CODE, sizeof(DWORD));
	ReformStateCode((char *)lpItemInfo->ItemName, 32);

	ReformStateCode((char *)&lpItemInfo->Weight, sizeof(int));
	ReformStateCode((char *)&lpItemInfo->Price, sizeof(int));

	ReformStateCode((char *)lpItemInfo->Resistance, sizeof(short) * 8);
	ReformStateCode((char *)lpItemInfo->Damage, sizeof(short) * 2);

	ReformStateCode((char *)&lpItemInfo->fAbsorb, sizeof(float));
	ReformStateCode((char *)&lpItemInfo->Defence, sizeof(int));
	ReformStateCode((char *)&lpItemInfo->fBlock_Rating, sizeof(float));

	ReformStateCode((char *)&lpItemInfo->Shooting_Range, sizeof(int));
	ReformStateCode((char *)&lpItemInfo->Attack_Speed, sizeof(int));

	ReformStateCode((char *)&lpItemInfo->Attack_Rating, sizeof(int));
	ReformStateCode((char *)&lpItemInfo->Critical_Hit, sizeof(int));

	ReformStateCode((char *)&lpItemInfo->fSpeed, sizeof(float));
	ReformStateCode((char *)&lpItemInfo->Potion_Space, sizeof(int));

	ReformStateCode((char *)&lpItemInfo->fMagic_Mastery, sizeof(float));
	ReformStateCode((char *)&lpItemInfo->fMana_Regen, sizeof(float));
	ReformStateCode((char *)&lpItemInfo->fLife_Regen, sizeof(float));
	ReformStateCode((char *)&lpItemInfo->fStamina_Regen, sizeof(float));
	ReformStateCode((char *)&lpItemInfo->fIncrease_Life, sizeof(float));
	ReformStateCode((char *)&lpItemInfo->fIncrease_Mana, sizeof(float));
	ReformStateCode((char *)&lpItemInfo->fIncrease_Stamina, sizeof(float));

	ReformStateCode((char *)&lpItemInfo->Level, sizeof(int));
	ReformStateCode((char *)&lpItemInfo->Strength, sizeof(int));
	ReformStateCode((char *)&lpItemInfo->Spirit, sizeof(int));
	ReformStateCode((char *)&lpItemInfo->Talent, sizeof(int));
	ReformStateCode((char *)&lpItemInfo->Dexterity, sizeof(int));
	ReformStateCode((char *)&lpItemInfo->Health, sizeof(int));

	ReformStateCode((char *)lpItemInfo->Mana, sizeof(short) * 2);
	ReformStateCode((char *)lpItemInfo->Life, sizeof(short) * 2);
	ReformStateCode((char *)lpItemInfo->Stamina, sizeof(short) * 2);

	ReformStateCode((char *)&lpItemInfo->Money, sizeof(int));
	ReformStateCode((char *)&lpItemInfo->JobCodeMask, sizeof(DWORD));
	ReformStateCode((char *)&lpItemInfo->JobItem, sizeof(sITEM_SPECIAL));

	ReformStateCode((char *)&lpItemInfo->UniqueItem, sizeof(DWORD));
	ReformStateCode((char *)lpItemInfo->EffectBlink, sizeof(short) * 2);
	ReformStateCode((char *)lpItemInfo->EffectColor, sizeof(short) * 4);
	ReformStateCode((char *)lpItemInfo->ScaleBlink, sizeof(short) * 2);
	ReformStateCode((char *)&lpItemInfo->DispEffect, sizeof(DWORD));

	return iFormCode;
}


#ifdef _W_SERVER

static DWORD rsReformStateCode(char *lpState, int size)
{
	int cnt;

	for (cnt = 0; cnt < size; cnt++)
	{
		iFormCode += ((DWORD)lpState[cnt])*((iFromCnt & 0x7FF) + 1);
		iFromCnt++;
	}

	return iFormCode;
}

static DWORD rsGetRefromItemCode(sITEMINFO *lpItemInfo, int Key)
{

	iFormCode = 0;
	iFromCnt = Key;

	rsReformStateCode((char *)&lpItemInfo->CODE, sizeof(DWORD));
	rsReformStateCode((char *)lpItemInfo->ItemName, 32);

	rsReformStateCode((char *)&lpItemInfo->Weight, sizeof(int));
	rsReformStateCode((char *)&lpItemInfo->Price, sizeof(int));

	rsReformStateCode((char *)lpItemInfo->Resistance, sizeof(short) * 8);
	rsReformStateCode((char *)lpItemInfo->Damage, sizeof(short) * 2);

	rsReformStateCode((char *)&lpItemInfo->fAbsorb, sizeof(float));
	rsReformStateCode((char *)&lpItemInfo->Defence, sizeof(int));
	rsReformStateCode((char *)&lpItemInfo->fBlock_Rating, sizeof(float));

	rsReformStateCode((char *)&lpItemInfo->Shooting_Range, sizeof(int));
	rsReformStateCode((char *)&lpItemInfo->Attack_Speed, sizeof(int));

	rsReformStateCode((char *)&lpItemInfo->Attack_Rating, sizeof(int));
	rsReformStateCode((char *)&lpItemInfo->Critical_Hit, sizeof(int));

	rsReformStateCode((char *)&lpItemInfo->fSpeed, sizeof(float));
	rsReformStateCode((char *)&lpItemInfo->Potion_Space, sizeof(int));

	rsReformStateCode((char *)&lpItemInfo->fMagic_Mastery, sizeof(float));
	rsReformStateCode((char *)&lpItemInfo->fMana_Regen, sizeof(float));
	rsReformStateCode((char *)&lpItemInfo->fLife_Regen, sizeof(float));
	rsReformStateCode((char *)&lpItemInfo->fStamina_Regen, sizeof(float));
	rsReformStateCode((char *)&lpItemInfo->fIncrease_Life, sizeof(float));
	rsReformStateCode((char *)&lpItemInfo->fIncrease_Mana, sizeof(float));
	rsReformStateCode((char *)&lpItemInfo->fIncrease_Stamina, sizeof(float));

	rsReformStateCode((char *)&lpItemInfo->Level, sizeof(int));
	rsReformStateCode((char *)&lpItemInfo->Strength, sizeof(int));
	rsReformStateCode((char *)&lpItemInfo->Spirit, sizeof(int));
	rsReformStateCode((char *)&lpItemInfo->Talent, sizeof(int));
	rsReformStateCode((char *)&lpItemInfo->Dexterity, sizeof(int));
	rsReformStateCode((char *)&lpItemInfo->Health, sizeof(int));

	rsReformStateCode((char *)lpItemInfo->Mana, sizeof(short) * 2);
	rsReformStateCode((char *)lpItemInfo->Life, sizeof(short) * 2);
	rsReformStateCode((char *)lpItemInfo->Stamina, sizeof(short) * 2);

	rsReformStateCode((char *)&lpItemInfo->Money, sizeof(int));
	rsReformStateCode((char *)&lpItemInfo->JobCodeMask, sizeof(DWORD));
	rsReformStateCode((char *)&lpItemInfo->JobItem, sizeof(sITEM_SPECIAL));

	rsReformStateCode((char *)&lpItemInfo->UniqueItem, sizeof(DWORD));
	rsReformStateCode((char *)lpItemInfo->EffectBlink, sizeof(short) * 2);
	rsReformStateCode((char *)lpItemInfo->EffectColor, sizeof(short) * 4);
	rsReformStateCode((char *)lpItemInfo->ScaleBlink, sizeof(short) * 2);
	ReformStateCode((char *)&lpItemInfo->DispEffect, sizeof(DWORD));

	return iFormCode;
}

#ifdef _NEW_ITEM_FORMCODE3
#include "core/gameplay/legacy_support/ItemForm2.h"
#else
#include "core/gameplay/legacy_support/ItemForm.h"
#endif


int	rsGetItemForm_Server(sITEMINFO *lpItemInfo)
{

	return rsGetItem_ServerForm(lpItemInfo);

}


int	rsReformItem_Server(sITEMINFO *lpItemInfo)
{
	DWORD dwCode;


	dwCode = rsGetItemForm_Server(lpItemInfo);

	lpItemInfo->ItemHeader.dwTime = dwCode;

	return TRUE;
}

#else

int	rsGetItemForm_Server(sITEMINFO *lpItemInfo)
{

	return TRUE;
}

int	rsReformItem_Server(sITEMINFO *lpItemInfo)
{

	return TRUE;
}

#endif



static int CompareReformItemCode(sITEMINFO *lpItemInfo1, sITEMINFO *lpItemInfo2)
{

	int code;

	code = 0;

	code += CompareStateCode((char *)&lpItemInfo1->CODE, (char *)&lpItemInfo2->CODE, sizeof(DWORD));
	code += CompareStateCode((char *)lpItemInfo1->ItemName, (char *)lpItemInfo2->ItemName, 32);

	code += CompareStateCode((char *)&lpItemInfo1->Weight, (char *)&lpItemInfo2->Weight, sizeof(int));
	code += CompareStateCode((char *)&lpItemInfo1->Price, (char *)&lpItemInfo2->Price, sizeof(int));

	code += CompareStateCode((char *)lpItemInfo1->Resistance, (char *)lpItemInfo2->Resistance, sizeof(short) * 8);
	code += CompareStateCode((char *)lpItemInfo1->Damage, (char *)lpItemInfo2->Damage, sizeof(short) * 2);

	code += CompareStateCode((char *)&lpItemInfo1->fAbsorb, (char *)&lpItemInfo2->fAbsorb, sizeof(float));
	code += CompareStateCode((char *)&lpItemInfo1->Defence, (char *)&lpItemInfo2->Defence, sizeof(int));
	code += CompareStateCode((char *)&lpItemInfo1->fBlock_Rating, (char *)&lpItemInfo2->fBlock_Rating, sizeof(float));

	code += CompareStateCode((char *)&lpItemInfo1->Shooting_Range, (char *)&lpItemInfo2->Shooting_Range, sizeof(int));
	code += CompareStateCode((char *)&lpItemInfo1->Attack_Speed, (char *)&lpItemInfo2->Attack_Speed, sizeof(int));

	code += CompareStateCode((char *)&lpItemInfo1->Attack_Rating, (char *)&lpItemInfo2->Attack_Rating, sizeof(int));
	code += CompareStateCode((char *)&lpItemInfo1->Critical_Hit, (char *)&lpItemInfo2->Critical_Hit, sizeof(int));

	code += CompareStateCode((char *)&lpItemInfo1->fSpeed, (char *)&lpItemInfo2->fSpeed, sizeof(float));
	code += CompareStateCode((char *)&lpItemInfo1->Potion_Space, (char *)&lpItemInfo2->Potion_Space, sizeof(int));

	code += CompareStateCode((char *)&lpItemInfo1->fMagic_Mastery, (char *)&lpItemInfo2->fMagic_Mastery, sizeof(float));
	code += CompareStateCode((char *)&lpItemInfo1->fMana_Regen, (char *)&lpItemInfo2->fMana_Regen, sizeof(float));
	code += CompareStateCode((char *)&lpItemInfo1->fLife_Regen, (char *)&lpItemInfo2->fLife_Regen, sizeof(float));
	code += CompareStateCode((char *)&lpItemInfo1->fStamina_Regen, (char *)&lpItemInfo2->fStamina_Regen, sizeof(float));
	code += CompareStateCode((char *)&lpItemInfo1->fIncrease_Life, (char *)&lpItemInfo2->fIncrease_Life, sizeof(float));
	code += CompareStateCode((char *)&lpItemInfo1->fIncrease_Mana, (char *)&lpItemInfo2->fIncrease_Mana, sizeof(float));
	code += CompareStateCode((char *)&lpItemInfo1->fIncrease_Stamina, (char *)&lpItemInfo2->fIncrease_Stamina, sizeof(float));

	code += CompareStateCode((char *)&lpItemInfo1->Level, (char *)&lpItemInfo2->Level, sizeof(int));
	code += CompareStateCode((char *)&lpItemInfo1->Strength, (char *)&lpItemInfo2->Strength, sizeof(int));
	code += CompareStateCode((char *)&lpItemInfo1->Spirit, (char *)&lpItemInfo2->Spirit, sizeof(int));
	code += CompareStateCode((char *)&lpItemInfo1->Talent, (char *)&lpItemInfo2->Talent, sizeof(int));
	code += CompareStateCode((char *)&lpItemInfo1->Dexterity, (char *)&lpItemInfo2->Dexterity, sizeof(int));
	code += CompareStateCode((char *)&lpItemInfo1->Health, (char *)&lpItemInfo2->Health, sizeof(int));

	code += CompareStateCode((char *)lpItemInfo1->Mana, (char *)lpItemInfo2->Mana, sizeof(short) * 2);
	code += CompareStateCode((char *)lpItemInfo1->Life, (char *)lpItemInfo2->Life, sizeof(short) * 2);
	code += CompareStateCode((char *)lpItemInfo1->Stamina, (char *)lpItemInfo2->Stamina, sizeof(short) * 2);

	code += CompareStateCode((char *)&lpItemInfo1->Money, (char *)&lpItemInfo2->Money, sizeof(int));
	code += CompareStateCode((char *)&lpItemInfo1->JobCodeMask, (char *)&lpItemInfo2->JobCodeMask, sizeof(DWORD));
	code += CompareStateCode((char *)&lpItemInfo1->JobItem, (char *)&lpItemInfo2->JobItem, sizeof(sITEM_SPECIAL));

	code += CompareStateCode((char *)&lpItemInfo1->UniqueItem, (char *)&lpItemInfo2->UniqueItem, sizeof(DWORD));
	code += CompareStateCode((char *)lpItemInfo1->EffectBlink, (char *)lpItemInfo2->EffectBlink, sizeof(short) * 2);
	code += CompareStateCode((char *)lpItemInfo1->EffectColor, (char *)lpItemInfo2->EffectColor, sizeof(short) * 4);
	code += CompareStateCode((char *)lpItemInfo1->ScaleBlink, (char *)lpItemInfo2->ScaleBlink, sizeof(short) * 2);
	code += CompareStateCode((char *)&lpItemInfo1->DispEffect, (char *)&lpItemInfo2->DispEffect, sizeof(DWORD));

	return code;
}


static DWORD GetRefromTradeCode(sTRADE *lpTrade, int Key)
{

	int cnt;

	iFormCode = 0;
	iFromCnt = Key;

	for (cnt = 0; cnt < MAX_TRADE_ITEM; cnt++)
	{
		if (lpTrade->TradeItem[cnt].Flag)
		{
			ReformStateCode((char *)&lpTrade->TradeItem[cnt].sItemInfo.CODE, sizeof(DWORD));
			ReformStateCode((char *)&lpTrade->TradeItem[cnt].sItemInfo.ItemHeader.dwChkSum, sizeof(DWORD));
		}
	}
	ReformStateCode((char *)&lpTrade->Money, sizeof(int));

	return iFormCode;
}

static DWORD GetRefromCharState(smCHAR_INFO *lpCharInfo, int Key)
{

	iFormCode = 0;
	iFromCnt = Key;



	ReformStateCode((char *)&lpCharInfo->szModelName2, 64);

	ReformStateCode((char *)&lpCharInfo->Level, sizeof(int));
	ReformStateCode((char *)&lpCharInfo->Strength, sizeof(int));
	ReformStateCode((char *)&lpCharInfo->Spirit, sizeof(int));
	ReformStateCode((char *)&lpCharInfo->Talent, sizeof(int));
	ReformStateCode((char *)&lpCharInfo->Dexterity, sizeof(int));
	ReformStateCode((char *)&lpCharInfo->Health, sizeof(int));
	ReformStateCode((char *)&lpCharInfo->Level, sizeof(int));


	ReformStateCode((char *)&lpCharInfo->Accuracy, sizeof(int));
	ReformStateCode((char *)&lpCharInfo->Attack_Rating, sizeof(int));
	ReformStateCode((char *)lpCharInfo->Attack_Damage, sizeof(int) * 2);
	ReformStateCode((char *)&lpCharInfo->Attack_Speed, sizeof(int));
	ReformStateCode((char *)&lpCharInfo->Shooting_Range, sizeof(int));
	ReformStateCode((char *)&lpCharInfo->Critical_Hit, sizeof(int));

	ReformStateCode((char *)&lpCharInfo->Defence, sizeof(int));
	ReformStateCode((char *)&lpCharInfo->Chance_Block, sizeof(int));
	ReformStateCode((char *)&lpCharInfo->Absorption, sizeof(int));
	ReformStateCode((char *)&lpCharInfo->Move_Speed, sizeof(int));
	ReformStateCode((char *)&lpCharInfo->Sight, sizeof(int));
	ReformStateCode((char *)lpCharInfo->Weight, sizeof(short) * 2);

	ReformStateCode((char *)lpCharInfo->Resistance, sizeof(short) * 8);
	ReformStateCode((char *)lpCharInfo->Attack_Resistance, sizeof(short) * 8);

	ReformStateCode((char *)lpCharInfo->Life, sizeof(short) * 2);
	ReformStateCode((char *)lpCharInfo->Mana, sizeof(short) * 2);
	ReformStateCode((char *)lpCharInfo->Stamina, sizeof(short) * 2);

	ReformStateCode((char *)&lpCharInfo->Exp, sizeof(int));
	ReformStateCode((char *)&lpCharInfo->Next_Exp, sizeof(int));
	ReformStateCode((char *)&lpCharInfo->Money, sizeof(int));
	ReformStateCode((char *)&lpCharInfo->StatePoint, sizeof(int));

#ifdef _MODE_EXP64
	ReformStateCode((char *)&lpCharInfo->Exp_High, sizeof(int));
#endif

	return iFormCode;
}


static int ItemFormVersion = 12;
static int ItemFormKey = 0;
