int	ResetInvenItemCode()
{

	dwTotal_InvenItemCode = GetInvenItemCode();
	dwTotal_InvenItemCode = dwPlayTime ^ dwTotal_InvenItemCode;
	dwTotal_InvenItemPlayTime = dwPlayTime;

	return TRUE;
}


int SubInvenItemCode(sITEMINFO *lpItem)
{
	DWORD	dwCode;

	dwCode = dwTotal_InvenItemCode ^ dwTotal_InvenItemPlayTime;
	dwTotal_InvenItemCode = dwCode - GetItemCheckCode(lpItem);

	dwTotal_InvenItemCode = dwPlayTime ^ dwTotal_InvenItemCode;
	dwTotal_InvenItemPlayTime = dwPlayTime;

	return TRUE;
}



int	CheckInvenItemCode()
{
	DWORD dwCode;
	smTRANS_COMMAND	smTransCommand;

	dwCode = GetInvenItemCode();

	if (dwCode != (dwTotal_InvenItemCode^dwTotal_InvenItemPlayTime))
	{
		smTransCommand.size = sizeof(smTRANS_COMMAND);
		smTransCommand.code = OpCode::OPCODE_SET_BLACKLIST3;
		smTransCommand.WParam = 7010;
		smTransCommand.LParam = dwTotal_InvenItemCode ^ dwTotal_InvenItemPlayTime;
		smTransCommand.SParam = dwCode;
		if (LegacyPlaySubDataChannelAvailable())
		{
			LegacyPlaySubSendDataPacket(&smTransCommand, sizeof(smTRANS_COMMAND), smTransCommand.code);
		}
	}

	dwTotal_InvenItemCode = dwPlayTime ^ dwCode;
	dwTotal_InvenItemPlayTime = dwPlayTime;

	return TRUE;
}


void CheckPlaySubFunc2()
{
	if (dwPlaySub_CheckResult != PLAYSUB_CHECKRESULT)
	{
		SendSetHackUser2(1120, dwPlaySub_CheckResult);
		DisconnectFlag = dwPlayTime;
	}
}

short	sChkLife[2];
short	sChkMana[2];
short	sChkStamina[2];
int		EnergyGraphErrorCount[4] = { 0,0,0,0 };



int CheckEnergyGraphError()
{
	int Life, Mana, Stamina;
	smTRANS_COMMAND	smTransCommand;


	Life = EnergyGraphErrorCount[0] - EnergyGraphErrorCount[3];
	Mana = EnergyGraphErrorCount[1] - EnergyGraphErrorCount[3];
	Stamina = EnergyGraphErrorCount[2] - EnergyGraphErrorCount[3];



	if (Life > 0 || Mana > 0 || Stamina > 0)
	{
		smTransCommand.size = sizeof(smTRANS_COMMAND);
		smTransCommand.code = OpCode::OPCODE_CLIENT_ERROR;
		smTransCommand.WParam = 0;

		smTransCommand.LParam = Life;
		smTransCommand.SParam = Mana;
		smTransCommand.EParam = Stamina;

		if (LegacyPlaySubDataChannelAvailable())
			LegacyPlaySubSendDataPacket(&smTransCommand, smTransCommand.size, smTransCommand.code);
	}

	EnergyGraphErrorCount[3] = lpCurPlayer->pX;
	EnergyGraphErrorCount[0] = EnergyGraphErrorCount[3] + Life;
	EnergyGraphErrorCount[1] = EnergyGraphErrorCount[3] + Life;
	EnergyGraphErrorCount[2] = EnergyGraphErrorCount[3] + Life;

	return TRUE;
}


int	ResetEnergyGraph(int Num)
{
	int	Pos;

	Pos = lpCurPlayer->pZ;

	switch (Num)
	{
	case 0:

		sChkLife[0] = Pos;
		sChkLife[1] = Pos ^ (-1);
		break;

	case 1:

		sChkMana[0] = Pos;
		sChkMana[1] = Pos ^ (-1);
		break;

	case 2:

		sChkStamina[0] = Pos;
		sChkStamina[1] = Pos ^ (-1);
		break;

	case 3:
	case 4:

		sChkLife[0] = Pos;
		sChkLife[1] = Pos ^ (-1);
		sChkMana[0] = Pos;
		sChkMana[1] = Pos ^ (-1);
		sChkStamina[0] = Pos;
		sChkStamina[1] = Pos ^ (-1);
		break;

	}

	return TRUE;
}


int CheckEnergyGraph()
{
	int	Pos;
	int	Grp;
	int	Last_Grp;
	int cnt;

	Pos = lpCurPlayer->pZ;

	Last_Grp = sChkLife[0] ^ sChkLife[1];
	Grp = sinGetLife();
	if (Last_Grp != -1)
	{
		cnt = Grp - Last_Grp;
		if (cnt > 15)
		{
			EnergyGraphErrorCount[0] ++;
		}
	}
	sChkLife[0] = Pos;
	sChkLife[1] = Pos ^ Grp;

	Last_Grp = sChkMana[0] ^ sChkMana[1];
	Grp = sinGetMana();
	if (Last_Grp != -1)
	{
		cnt = Grp - Last_Grp;
		if (cnt > 15)
		{
			EnergyGraphErrorCount[1] ++;
		}
	}
	sChkMana[0] = Pos;
	sChkMana[1] = Pos ^ Grp;

	Last_Grp = sChkStamina[0] ^ sChkStamina[1];
	Grp = sinGetStamina();
	if (Last_Grp != -1)
	{
		cnt = Grp - Last_Grp;
		if (cnt > 50)
		{
			EnergyGraphErrorCount[2] ++;
		}
	}
	sChkStamina[0] = Pos;
	sChkStamina[1] = Pos ^ Grp;

	return TRUE;
}
