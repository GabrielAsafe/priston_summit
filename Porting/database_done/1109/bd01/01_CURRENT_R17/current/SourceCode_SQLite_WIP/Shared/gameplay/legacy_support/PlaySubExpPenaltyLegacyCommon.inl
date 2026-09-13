int CheckFieldContExp()
{
	int Level;
	INT64 LowExp;
	INT64 HighExp;
	INT64 exp;
	INT64 exp64;

	Level = lpCurPlayer->smCharInfo.Level;
	if (Level >= 11)
	{
		LowExp = ExpLevelTable[Level - 1];
		HighExp = ExpLevelTable[Level];
		exp = (HighExp - LowExp) / 100;

		exp64 = GetXorExp64(&lpCurPlayer->smCharInfo);

		exp = exp64 - exp;
		if (exp < LowExp)
		{
			return FALSE;
		}
	}
	else
	{
		if (lpCurPlayer->OnStageField >= 0 && StageField[lpCurPlayer->OnStageField]->State != FIELD_STATE_FOREST)
		{

			return FALSE;
		}
	}

	return TRUE;
}



INT64 GetNextExp(int Level)
{
	return ExpLevelTable[Level];
}


int DeadPlayerExp(smCHAR *lpChar)
{
	int Level;
	INT64 LowExp;
	INT64 HighExp;
	INT64 exp;
	INT64 ep;
	INT64 exp64;
	int Money;
	int ExpLower = 0;
	int	Penalty = GetDeathPenalty();

	Level = lpChar->smCharInfo.Level;
	if (Level >= 11)
	{
		LowExp = ExpLevelTable[Level - 1];
		HighExp = ExpLevelTable[Level];
		if (Penalty)
			exp = (HighExp - LowExp) / Penalty;
		else
			exp = 0;

		exp64 = CodeXorCharInfo_Exp();
		if (exp64 >= LowExp)
		{
			ep = exp64;
			exp64 -= exp;
			if (exp64 < LowExp)
			{
				exp64 = LowExp;
				ExpLower = TRUE;
			}
			SetExp64(&lpCurPlayer->smCharInfo, exp64);
			ep = ep - exp64;
			SetTotalSubExp((int)ep);
		}
		CodeXorCharInfo_Exp();


		if (Penalty)
			Money = lpChar->smCharInfo.Money / Penalty;
		else
			Money = 0;

		lpChar->smCharInfo.Money -= Money;
		if (lpChar->smCharInfo.Money < 0)
		{
			Money = -Money;
			lpChar->smCharInfo.Money = 0;
		}
		SetTotalSubMoney(Money);


		ReformCharForm();
		ResetInvenMoney();
	}

	return ExpLower;
}


int ContGameExp(smCHAR *lpChar)
{
	int Level;
	INT64 LowExp;
	INT64 HighExp;
	INT64 exp;
	INT64 ep;
	INT64 exp64;
	int Money;

	Level = lpChar->smCharInfo.Level;
	if (Level >= 11)
	{
		LowExp = ExpLevelTable[Level - 1];
		HighExp = ExpLevelTable[Level];
		exp = (HighExp - LowExp) / 100;


		exp64 = CodeXorCharInfo_Exp();

		if (exp64 > LowExp)
		{
			ep = exp64;
			exp64 -= exp;
			if (exp64 < LowExp)
				exp64 = LowExp;

			SetExp64(&lpCurPlayer->smCharInfo, exp64);

			ep = ep - exp64;
			SetTotalSubExp((int)ep);
		}

		CodeXorCharInfo_Exp();


		Money = lpChar->smCharInfo.Money / 100;
		lpChar->smCharInfo.Money -= Money;
		if (lpChar->smCharInfo.Money < 0)
		{
			Money = -Money;
			lpChar->smCharInfo.Money = 0;
		}

		SetTotalSubMoney(Money);


		ReformCharForm();
		ResetInvenMoney();
	}

	return TRUE;
}




int GetPlayDefence(smCHAR_INFO *lpAttackCharInfo, smCHAR_INFO *lpDefenceCharInfo)
{
	int defence;

	float ar, def, dLv, aLv;
	int weight;


	ar = (float)lpAttackCharInfo->Attack_Rating;
	aLv = (float)lpAttackCharInfo->Level;
	def = (float)lpDefenceCharInfo->Defence;
	dLv = (float)lpDefenceCharInfo->Level;
	weight = lpDefenceCharInfo->Weight[0];
	defence = (int)(((((ar / (ar + (def / 2))) - ((dLv - aLv) / 50))) * 100)) - (weight / 10000);

	if (defence > 95) defence = 95;

	return defence;
}
