int smPlayWindowMessage()
{
	MSG msg;

	MessageLoading = TRUE;

	while (1)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{

			TranslateMessage(&msg);
			if (msg.message == WM_SYSKEYDOWN || msg.message == WM_SYSKEYUP) continue;
			DispatchMessage(&msg);
		}
		else
			break;
	}

	MessageLoading = FALSE;
	return TRUE;
}


static DWORD iFormCode;
static DWORD iFromCnt;

static DWORD ReformStateCode(char *lpState, int size)
{
	int cnt;

	for (cnt = 0; cnt < size; cnt++)
	{
		iFormCode += ((DWORD)lpState[cnt])*((iFromCnt & 0x1FFF) + 1);
		iFromCnt++;
	}

	return iFormCode;
}

static DWORD CompareStateCode(char *lpState1, char *lpState2, int size)
{
	int cnt;

	for (cnt = 0; cnt < size; cnt++)
	{
		if (lpState1[cnt] != lpState2[cnt])
			return 1;
	}

	return 0;
}


static DWORD GetRefromItemCode_Old(sITEMINFO *lpItemInfo, int Key)
{

	iFormCode = 0;
	iFromCnt = Key;

	ReformStateCode((char *)&lpItemInfo->CODE, sizeof(DWORD));
	ReformStateCode((char *)lpItemInfo->ItemName, 32);

	ReformStateCode((char *)&lpItemInfo->Weight, sizeof(int));
	ReformStateCode((char *)&lpItemInfo->Price, sizeof(int));

	ReformStateCode((char *)lpItemInfo->Resistance, sizeof(short) * 8);
	ReformStateCode((char *)lpItemInfo->Damage, sizeof(short) * 2);



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

	return iFormCode;
}

static DWORD GetRefromItemCode_V10(sITEMINFO *lpItemInfo, int Key)
{

	iFormCode = 0;
	iFromCnt = Key;

	ReformStateCode((char *)&lpItemInfo->CODE, sizeof(DWORD));
	ReformStateCode((char *)lpItemInfo->ItemName, 32);

	ReformStateCode((char *)&lpItemInfo->Weight, sizeof(int));
	ReformStateCode((char *)&lpItemInfo->Price, sizeof(int));

	ReformStateCode((char *)lpItemInfo->Resistance, sizeof(short) * 8);
	ReformStateCode((char *)lpItemInfo->Damage, sizeof(short) * 2);

	ReformStateCode((char *)&lpItemInfo->Defence, sizeof(int));

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

	return iFormCode;
}

static int CompareReformItemCode_V10(sITEMINFO *lpItemInfo1, sITEMINFO *lpItemInfo2)
{
	int code = 0;

	code += CompareStateCode((char *)&lpItemInfo1->CODE, (char *)&lpItemInfo2->CODE, sizeof(DWORD));
	code += CompareStateCode((char *)lpItemInfo1->ItemName, (char *)lpItemInfo2->ItemName, 32);

	code += CompareStateCode((char *)&lpItemInfo1->Weight, (char *)&lpItemInfo2->Weight, sizeof(int));
	code += CompareStateCode((char *)&lpItemInfo1->Price, (char *)&lpItemInfo2->Price, sizeof(int));

	code += CompareStateCode((char *)lpItemInfo1->Resistance, (char *)lpItemInfo2->Resistance, sizeof(short) * 8);
	code += CompareStateCode((char *)lpItemInfo1->Damage, (char *)lpItemInfo2->Damage, sizeof(short) * 2);

	code += CompareStateCode((char *)&lpItemInfo1->Defence, (char *)&lpItemInfo2->Defence, sizeof(int));

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

	return code;
}
