// Shared byte-equivalent legacy combat helpers extracted from client/server Svr_Damge.cpp.
// Target-specific combat formulas and server policies remain in the local translation units.

int rsRecvRangeAttackData_Old(rsPLAYINFO *lpPlayInfo, TRANS_SKIL_ATTACKDATA *lpTransSkilAttackData, int Type)
{

	smTRANS_COMMAND		smTransCommand;



	lpPlayInfo->Recv_AttackCount++;
	lpPlayInfo->Recv_AttackDamage += lpTransSkilAttackData->Power;

	if(lpPlayInfo->HideMode && lpPlayInfo->getSecurity() < AccountTypes::SEC_ADMINISTRATOR) return FALSE;

	RecvRangeAttack(lpPlayInfo, lpTransSkilAttackData);

	if(lpTransSkilAttackData->Power >= 1000)
	{
		smTransCommand.WParam = 1830;
		smTransCommand.SParam = lpTransSkilAttackData->Power;
		smTransCommand.LParam = smTransCommand.SParam;
		RecordHackLogFile(lpPlayInfo, &smTransCommand);

		DisconnectUser(lpPlayInfo->lpsmSock);
	}



	return TRUE;
}

int rsRecvRangeAttackUserData_Old(rsPLAYINFO *lpPlayInfo, TRANS_SKIL_ATTACKDATA *lpTransSkilAttackData)
{

	smTRANS_COMMAND		smTransCommand;



	lpPlayInfo->Recv_AttackCount++;
	lpPlayInfo->Recv_AttackDamage += lpTransSkilAttackData->Power;

	if(lpPlayInfo->HideMode && lpPlayInfo->getSecurity() < AccountTypes::SEC_ADMINISTRATOR) return FALSE;

	RecvRangeAttackUser(lpPlayInfo, lpTransSkilAttackData);

	if(lpTransSkilAttackData->Power >= 1000)
	{
		smTransCommand.WParam = 1830;
		smTransCommand.SParam = lpTransSkilAttackData->Power;
		smTransCommand.LParam = smTransCommand.SParam;
		RecordHackLogFile(lpPlayInfo, &smTransCommand);

		DisconnectUser(lpPlayInfo->lpsmSock);
	}

	return TRUE;
}

int	rsCheckAttackRange(int dx, int dy, int dz, smCHAR *lpChar, int dAttackSize)
{
	int x, y, z;

	x = (dx - lpChar->pX) >> FLOATNS;
	y = (dy - lpChar->pY) >> FLOATNS;
	z = (dz - lpChar->pZ) >> FLOATNS;

	if(abs(y) > 100 || (x*x + z*z) > dAttackSize)
		return FALSE;

	return TRUE;
}

int	rsCheckAttackRange(int dx, int dy, int dz, rsPLAYINFO *lpPlayInfo)
{
	int x, y, z;

	x = (dx - lpPlayInfo->Position.x) >> FLOATNS;
	y = (dy - lpPlayInfo->Position.y) >> FLOATNS;
	z = (dz - lpPlayInfo->Position.z) >> FLOATNS;

	if(abs(y) > 100 || (x*x + z*z) > (300 * 300))
		return FALSE;

	return TRUE;
}

int RecvRangeAttack(rsPLAYINFO *lpPlayInfo, TRANS_SKIL_ATTACKDATA *lpTransSkilAttackData)
{

	return TRUE;
}

int RecvRangeAttackUser(rsPLAYINFO *lpPlayInfo, TRANS_SKIL_ATTACKDATA *lpTransSkilAttackData)
{

	int cnt, cnt2;
	TRANS_ATTACKDATA	TransAttackData;

	if(lpPlayInfo->getSecurity() < AccountTypes::SEC_MODERATOR)
	{
		if(!rsServerConfig.Enable_PK) return FALSE;
		if(lpPlayInfo->smCharInfo.Level <= LIMIT_PK_LEVEL) return FALSE;
	}

	if((lpTransSkilAttackData->AttackState & 0xFFFF) == 2)
		return FALSE;


	for(cnt2 = 0; cnt2 < CONNECTMAX; cnt2++)
	{
		if(rsPlayInfo[cnt2].lpsmSock && rsPlayInfo[cnt2].dwObjectSerial)
		{
			for(cnt = 0; cnt < lpTransSkilAttackData->TargetCount; cnt++)
			{
				if(rsPlayInfo[cnt2].dwObjectSerial == lpTransSkilAttackData->dwTarObjectSerial[cnt] &&
				   rsPlayInfo[cnt2].smCharInfo.Level > LIMIT_PK_LEVEL)
				{


					TransAttackData.code = OpCode::OPCODE_ATTACKDATA;
					TransAttackData.size = sizeof(TRANS_ATTACKDATA);

					TransAttackData.x = rsPlayInfo[cnt2].Position.x;
					TransAttackData.y = rsPlayInfo[cnt2].Position.y;
					TransAttackData.z = rsPlayInfo[cnt2].Position.z;


					TransAttackData.AttackState = 1;
					TransAttackData.AttackSize = 32 * fONE;
					TransAttackData.Power = lpTransSkilAttackData->Power;

					TransAttackData.dwDestObjectSerial = lpPlayInfo->dwObjectSerial;
					TransAttackData.dwTarObjectSerial = rsPlayInfo[cnt2].dwObjectSerial;
					TransAttackData.dwChkSum = TransAttackData.Power * 2002 +
						(TransAttackData.x * TransAttackData.y * TransAttackData.z);


					rsPlayInfo[cnt2].Send_AttackCount++;
					rsPlayInfo[cnt2].Send_AttackDamage += lpTransSkilAttackData->Power;

					rsPlayInfo[cnt2].lpsmSock->Send((char *)&TransAttackData, TransAttackData.size, TRUE);

					break;
				}
			}
		}
	}

	return TRUE;
}

int GetCritical(rsPLAYINFO *lpPlayInfo, smCHAR *lpChar, int Critical_Hit)
{
	int Result;

	Result = Critical_Hit + (((lpPlayInfo->smCharInfo.Level - lpChar->smCharInfo.Level) * 25) / 100);



	if(lpChar->dwAssassinEyeTime && Critical_Hit > 0)
	{
		if(lpChar->dwAssassinEyeTime > dwPlayServTime)
		{
			Result -= lpChar->AssassinEyeParam;
		}
		else
		{
			lpChar->dwAssassinEyeTime = 0;
			lpChar->AssassinEyeParam = 0;
		}
	}



	if(Result > 70)
		Result = 70;

	return Result;
}

int GetCritical2(rsPLAYINFO *lpPlayInfo, rsPLAYINFO *lpPlayInfo2, int Critical_Hit)
{
	int Result;

	Result = Critical_Hit + (((lpPlayInfo->smCharInfo.Level - lpPlayInfo2->smCharInfo.Level) * 25) / 100);
	if(Result > 70)
		Result = 70;

	return Result;
}

int	rsRecordCharDamage(rsPLAYINFO *lpPlayInfo, smCHAR *lpChar, int Damage)
{
	int cnt;
	int EmptyNum = -1;
	DWORD	dwClanCode;

	if(lpChar->lpAttackDamageList_BlessCastle)
	{

		if(lpPlayInfo->dwClanCode && lpPlayInfo->Position.Area == rsCASTLE_FIELD)
		{

			lpPlayInfo->sBlessCastle_Damage[0] += Damage;

			for(cnt = 0; cnt < ATTACK_DAMAGE_LIST_MAX; cnt++)
			{
				if(lpChar->lpAttackDamageList_BlessCastle[cnt].dwUserCode)
				{
					dwClanCode = GetClanCode(lpChar->lpAttackDamageList_BlessCastle[cnt].dwUserCode);
					if(dwClanCode == lpPlayInfo->dwClanCode)
					{
						lpChar->lpAttackDamageList_BlessCastle[cnt].DamageCount += Damage;
						lpChar->lpAttackDamageList_BlessCastle[cnt].Count++;
						return TRUE;
					}
				}
				if(EmptyNum < 0 && lpChar->lpAttackDamageList_BlessCastle[cnt].dwUserCode == 0)
				{
					EmptyNum = cnt;
				}

			}

			if(EmptyNum >= 0)
			{
				lpChar->lpAttackDamageList_BlessCastle[EmptyNum].dwUserCode = lpPlayInfo->smCharInfo.ClassClan;
				lpChar->lpAttackDamageList_BlessCastle[EmptyNum].DamageCount = Damage;
				lpChar->lpAttackDamageList_BlessCastle[EmptyNum].Count++;

				return TRUE;
			}
		}
		return TRUE;
	}

	if(!lpChar->lpAttackDamageList) return FALSE;

	if(lpChar->smCharInfo.dwCharSoundCode == snCHAR_SOUND_CASTLE_TOWER_B)
	{

		if(lpPlayInfo->dwClanCode && lpPlayInfo->Position.Area == rsCASTLE_FIELD)
		{

			lpPlayInfo->sBlessCastle_Damage[0] += Damage;

			for(cnt = 0; cnt < ATTACK_DAMAGE_LIST_MAX; cnt++)
			{
				if(lpChar->lpAttackDamageList[cnt].dwUserCode)
				{
					dwClanCode = GetClanCode(lpChar->lpAttackDamageList[cnt].dwUserCode);
					if(dwClanCode == lpPlayInfo->dwClanCode)
					{
						lpChar->lpAttackDamageList[cnt].DamageCount += Damage;
						lpChar->lpAttackDamageList[cnt].Count++;
						return TRUE;
					}
				}
				if(EmptyNum < 0 && lpChar->lpAttackDamageList[cnt].dwUserCode == 0)
				{
					EmptyNum = cnt;
				}

			}

			if(EmptyNum >= 0)
			{
				lpChar->lpAttackDamageList[EmptyNum].dwUserCode = lpPlayInfo->smCharInfo.ClassClan;
				lpChar->lpAttackDamageList[EmptyNum].DamageCount = Damage;
				lpChar->lpAttackDamageList[EmptyNum].Count++;

				return TRUE;
			}
		}
	}
	else
	{

		for(cnt = 0; cnt < ATTACK_DAMAGE_LIST_MAX; cnt++)
		{
			if(lpChar->lpAttackDamageList[cnt].dwUserCode == lpPlayInfo->dwObjectSerial)
			{
				lpChar->lpAttackDamageList[cnt].DamageCount += Damage;
				lpChar->lpAttackDamageList[cnt].Count++;
				return TRUE;
			}
			if(EmptyNum < 0 && lpChar->lpAttackDamageList[cnt].dwUserCode == 0)
			{
				EmptyNum = cnt;
			}
		}

		if(EmptyNum >= 0)
		{
			lpChar->lpAttackDamageList[EmptyNum].dwUserCode = lpPlayInfo->dwObjectSerial;
			lpChar->lpAttackDamageList[EmptyNum].DamageCount = Damage;
			lpChar->lpAttackDamageList[EmptyNum].Count++;

			return TRUE;
		}
	}

	return FALSE;
}

int	rsRecord_PKDamage(rsPLAYINFO *lpPlayInfo, rsPLAYINFO *lpPlayInfo2, int Damage)
{
	int cnt;
	ATTACK_DAMAGE_LIST *lpAttackDamageList = 0;
	DWORD dwClanCode;
	int EmptyNum = -1;


	if(!lpPlayInfo2->dwClanCode || lpPlayInfo2->dwClanCode == lpPlayInfo->dwClanCode)
	{
		if(lpPlayInfo2->dwClanCode != rsBlessCastle.dwMasterClan && lpPlayInfo->dwClanCode != rsBlessCastle.dwMasterClan)
		{

			return FALSE;
		}
	}

	if(rsBlessCastle.lpAttackDamageList)
	{
		lpAttackDamageList = rsBlessCastle.lpAttackDamageList;
	}
	else if(rsBlessCastle.lpChar_ValhallaTower)
	{

		if(rsBlessCastle.lpChar_ValhallaTower->Flag &&
		   rsBlessCastle.lpChar_ValhallaTower->smCharInfo.dwCharSoundCode == snCHAR_SOUND_CASTLE_TOWER_B &&
		   rsBlessCastle.lpChar_ValhallaTower->smCharInfo.Life[0] > 0)
		{
			lpAttackDamageList = rsBlessCastle.lpChar_ValhallaTower->lpAttackDamageList;
		}
	}

	if(!lpAttackDamageList) return FALSE;

	Damage -= lpPlayInfo->smCharInfo.Absorption;
	Damage /= PK_SCORE_DIVIDE;
	if(Damage <= 0)
	{
		Damage = 1;
	}

	lpPlayInfo->sBlessCastle_Damage[0] += Damage;


	if(lpPlayInfo->dwClanCode && lpPlayInfo->Position.Area == rsCASTLE_FIELD)
	{

		for(cnt = 0; cnt < ATTACK_DAMAGE_LIST_MAX; cnt++)
		{
			if(lpAttackDamageList[cnt].dwUserCode)
			{
				dwClanCode = GetClanCode(lpAttackDamageList[cnt].dwUserCode);
				if(dwClanCode == lpPlayInfo->dwClanCode)
				{
					lpAttackDamageList[cnt].DamageCount += Damage;
					lpAttackDamageList[cnt].Count++;
					return TRUE;
				}
			}
			if(EmptyNum < 0 && lpAttackDamageList[cnt].dwUserCode == 0)
			{
				EmptyNum = cnt;
			}
		}
		if(EmptyNum >= 0)
		{
			lpAttackDamageList[EmptyNum].dwUserCode = lpPlayInfo->smCharInfo.ClassClan;
			lpAttackDamageList[EmptyNum].DamageCount = Damage;
			lpAttackDamageList[EmptyNum].Count++;

			return TRUE;
		}
	}

	return TRUE;
}
