#ifdef _W_SERVER


int SetServerConfig(char *decode, char *lpPoint)
{

	int flag;
	char *p = lpPoint;
	char *pb;
	char strBuff[128];
	int cnt;

	flag = 0;

	if(lstrcmp(decode, "*�����ڵ��ȣ") == 0 ||
		lstrcmpi(decode, "*SERVER_CODE") == 0)
	{
		pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		rsServerConfig.ServerCode = atoi(strBuff);
		flag++;
	}

	if(!flag && (lstrcmp(decode, "*�ڵ�ĳ���ͻ��") == 0 ||
		lstrcmpi(decode, "*SERVER_AUTOPLAY") == 0))
	{
		rsServerConfig.AutoPlayer = TRUE;
		flag++;
	}

	if(!flag && (lstrcmp(decode, "*������ۻ��") == 0 ||
		lstrcmpi(decode, "*SERVER_RECORD_MEM") == 0))
	{
		rsServerConfig.UseRecorMemory = TRUE;
		flag++;
	}


	if(!flag && lstrcmp(decode, "*�������") == 0)
	{
		rsServerConfig.DisableParty = TRUE;
		flag++;
	}
	if(!flag && (lstrcmp(decode, "*���������ο�") == 0 ||
		lstrcmpi(decode, "*MAX_USERS") == 0))
	{
		pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		rsServerConfig.ConnectUserMax = atoi(strBuff);
		flag++;
	}

	if(!flag && (lstrcmp(decode, "*�׽�Ʈ����") == 0 ||
		lstrcmpi(decode, "*TEST_SERVER") == 0))
	{
		rsServerConfig.TestSeverMode = TRUE;
		pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		if(strBuff[0])
		{
			rsServerConfig.TestSeverMode = atoi(strBuff);
		}
		flag++;
	}

	if(!flag && (lstrcmp(decode, "*���Ӱ����ڵ�") == 0 ||
		lstrcmpi(decode, "*CONNECT_CLIENT_CODE") == 0))
	{
		for(cnt = 0; cnt < 32; cnt++)
		{
			p = fileread_GetWord(strBuff, p);
			if(!strBuff[0]) break;

			rsServerConfig.dwClientCheckSum[rsServerConfig.ClientCheckSumCount] = atoi(strBuff);
			rsServerConfig.ClientCheckSumCount++;
		}
		flag++;
	}

	if(!flag && (lstrcmp(decode, "*�ڵ���������") == 0 ||
		lstrcmpi(decode, "*DISCONNECT_CLIENT_BADCODE") == 0))
	{
		rsServerConfig.ClientCheckOut = TRUE;
		flag++;
	}

	if(!flag && (lstrcmp(decode, "*���κ�ȣ�ڵ�") == 0 ||
		lstrcmpi(decode, "*CLSAFE_CODE") == 0))
	{

		for(cnt = 0; cnt < 32; cnt++)
		{
			p = fileread_GetWord(strBuff, p);
			if(!strBuff[0]) break;

			rsServerConfig.dwSafeClientCode[rsServerConfig.SafeClientCodeCount] = atoi(strBuff);
			rsServerConfig.SafeClientCodeCount++;
		}
		flag++;



	}
	if(!flag && (lstrcmp(decode, "*��ȣ�ڵ��������") == 0 ||
		lstrcmpi(decode, "*DISCONNECT_CLSAFE_CODE") == 0))
	{
		rsServerConfig.ClientCodeCheckOut = TRUE;
		flag++;
	}


	if(!flag && (lstrcmp(decode, "*���ѹ���") == 0 ||
		lstrcmpi(decode, "*VERSION") == 0))
	{
		pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		rsServerConfig.LimitVersion = atoi(strBuff);

		flag++;
	}


	if(!flag && (lstrcmp(decode, "*������IP�ּ�") == 0 ||
		lstrcmpi(decode, "*ADMIN_IP") == 0))
	{
		pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		strcpy_s(rsServerConfig.szAdminIP, strBuff);
		flag++;
	}

	if(!flag && (lstrcmp(decode, "*���������Ӿ�ȣ") == 0 ||
		lstrcmpi(decode, "*ADMIN_LOGIN") == 0))
	{
		pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		strcpy_s(rsServerConfig.szAdminLogPassword, strBuff);
		flag++;
	}

	if(!flag && (lstrcmp(decode, "*�����̸�") == 0 ||
		lstrcmpi(decode, "*SERVER_NAME") == 0))
	{
		pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		strcpy_s(rsServerConfig.szServerName, strBuff);
		flag++;
	}

	if(!flag && (lstrcmp(decode, "*PK�㰡") == 0 ||
		lstrcmpi(decode, "*ENABLE_PK") == 0))
	{

		pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		rsServerConfig.Enable_PK = atoi(strBuff);
		if(rsServerConfig.Enable_PK == 0) rsServerConfig.Enable_PK = TRUE;

		flag++;
	}

	if(!flag && (lstrcmp(decode, "*PK�ʵ�") == 0 ||
		lstrcmpi(decode, "*PK_FIELD") == 0))
	{

		pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		rsServerConfig.Enable_PKField = atoi(strBuff);
		if(rsServerConfig.Enable_PKField == 0) rsServerConfig.Enable_PKField = 34;

		flag++;
	}


	if(!flag && (lstrcmp(decode, "*PK����ʵ�") == 0 || lstrcmpi(decode, "*PK_FIELD_ALL") == 0))
	{
		pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		rsServerConfig.Enable_PKField_All = atoi(strBuff);
		flag++;
	}


	if(!flag && (lstrcmp(decode, "*���న��") == 0 ||
		lstrcmpi(decode, "*POTION_MONITOR") == 0))
	{

		pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		if(strBuff[0])
			rsServerConfig.PotionMonitor = atoi(strBuff);
		else
			rsServerConfig.PotionMonitor = 1;

		flag++;
	}

	if(!flag && (lstrcmp(decode, "*���Ȱ˻�") == 0 ||
		lstrcmpi(decode, "*CL_PROTECT") == 0))
	{

		rsServerConfig.CheckProtect = TRUE;
		flag++;
	}


#ifdef LOG_THE_PK

	if(!flag && (lstrcmpi(decode, "*PK_LOG") == 0))
	{
		rsServerConfig.bPK_LOG = true;
		flag++;
	}
#endif


	if(!flag && (lstrcmp(decode, "*��������") == 0 ||
		lstrcmpi(decode, "*REMOVE_PROTECT_TIME") == 0))
	{
		pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		rsServerConfig.RemoveNProtectTime = atoi(strBuff);
		flag++;
	}


	if(!flag && (lstrcmp(decode, "*����ܰ�") == 0 ||
		lstrcmpi(decode, "*BLOCK_LEVEL") == 0))
	{
		pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		rsServerConfig.BlockLevel = atoi(strBuff);
		flag++;
	}

	if(!flag && (lstrcmp(decode, "*����ƿ�") == 0 ||
		lstrcmpi(decode, "*BLOCK_LOGOUT") == 0))
	{
		rsServerConfig.BlockLevel_LogOut = TRUE;
		flag++;
	}

	if(!flag && (lstrcmp(decode, "*�����ۿ�������") == 0 ||
		lstrcmpi(decode, "*DISCONNECT_ITEM_ERROR") == 0))
	{
		pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		if(strBuff[0])
			rsServerConfig.ItemCodeOut = atoi(strBuff);
		else
			rsServerConfig.ItemCodeOut = 1;

		flag++;
	}

	if(!flag && (lstrcmp(decode, "*��ǰ˻�") == 0 ||
		lstrcmpi(decode, "*CLIENT_CHECK_FUNC") == 0))
	{
		rsServerConfig.ClientFuncCheckMode = 1;
		pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		rsServerConfig.ClientFuncCheckMode = atoi(strBuff);
		flag++;
	}

	if(!flag && (lstrcmp(decode, "*�����̺�Ʈ") == 0 ||
		lstrcmpi(decode, "*EVENT_MANDOO") == 0))
	{
		rsServerConfig.Event_Mandoo = TRUE;;
		flag++;
	}
	if(!flag && (lstrcmp(decode, "*�ϵ��ھ��̺�Ʈ") == 0 ||
		lstrcmpi(decode, "*EVENT_HARDCORE") == 0))
	{

		p = fileread_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_HardCore = atoi(strBuff);
		else
			rsServerConfig.Event_HardCore = 1;

		flag++;
	}
#ifdef	_LANGUAGE_JAPANESE 
	if(!flag && (lstrcmp(decode, "*���ݷ�Ȱ���̺�Ʈ") == 0 ||
		lstrcmpi(decode, "*EVENT_DAMAGEOFF") == 0))
	{
		p = fileread_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_DamageOff = atoi(strBuff);
		else
			rsServerConfig.Event_DamageOff = 1;
		flag++;
	}
#else
	if(!flag && (lstrcmpi(decode, "*EVENT_DAMAGEOFF1") == 0))
	{
		p = fileread_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_DamageOff1 = atoi(strBuff);
		else
			rsServerConfig.Event_DamageOff1 = 1;
		flag++;
	}
	if(!flag && (lstrcmpi(decode, "*EVENT_DAMAGEOFF2") == 0))
	{
		p = fileread_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_DamageOff2 = atoi(strBuff);
		else
			rsServerConfig.Event_DamageOff2 = 1;
		flag++;
	}
#endif

	if(!flag && (lstrcmp(decode, "*���������̺�Ʈ") == 0 ||
		lstrcmpi(decode, "*EVENT_DEFENSEOFF") == 0))
	{
		p = fileread_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_DefenseOff = atoi(strBuff);
		else
			rsServerConfig.Event_DefenseOff = 1;
		flag++;
	}

	if(!flag && (lstrcmp(decode, "*����̺�Ʈ") == 0 ||
		lstrcmpi(decode, "*EVENT_CHILD") == 0))
	{

		p = fileread_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_Child = atoi(strBuff);
		else
			rsServerConfig.Event_Child = 1;

		flag++;
	}


	if(!flag && (lstrcmp(decode, "*ũ����Ż�̺�Ʈ") == 0 ||
		lstrcmpi(decode, "*EVENT_CRISTAL") == 0))
	{

		p = fileread_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_Cristal = atoi(strBuff);
		else
			rsServerConfig.Event_Cristal = 1;

		flag++;
	}
	if(!flag && (lstrcmp(decode, "*�����̺�Ʈ") == 0 ||
		lstrcmpi(decode, "*EVENT_POTION") == 0))
	{

		p = fileread_GetWord(strBuff, p);
		rsServerConfig.Event_Potion = atoi(strBuff);
		flag++;
	}
	if(!flag && (lstrcmp(decode, "*������Ʈ�̺�Ʈ") == 0 ||
		lstrcmpi(decode, "*EVENT_STARPOINT") == 0))
	{

		p = fileread_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_StarPointTicket = atoi(strBuff);
		else
			rsServerConfig.Event_StarPointTicket = 40;

		flag++;
	}

	if(!flag && (lstrcmp(decode, "*�����̺�Ʈ") == 0 ||
		lstrcmpi(decode, "*EVENT_BIRTHDAY") == 0))
	{

		p = fileread_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_ComicBirthDay = atoi(strBuff);
		else
			rsServerConfig.Event_ComicBirthDay = 1;

		flag++;
	}

	if(!flag && (lstrcmp(decode, "*�����̺�Ʈ") == 0 ||
		lstrcmpi(decode, "*EVENT_PUZZLE") == 0))
	{

		p = fileread_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_Puzzle = atoi(strBuff);
		else
			rsServerConfig.Event_Puzzle = 200;

		flag++;
	}

	if(!flag && (lstrcmp(decode, "*�ٺ����̺�Ʈ") == 0 ||
		lstrcmpi(decode, "*EVENT_BABELHORN") == 0))
	{

		p = fileread_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_BabelHorn = atoi(strBuff);
		else
			rsServerConfig.Event_BabelHorn = 4;

		if(rsServerConfig.Event_BabelHorn > 10) rsServerConfig.Event_BabelHorn = 10;

		flag++;
	}

	if(!flag && (lstrcmp(decode, "*����ȣ�̺�Ʈ") == 0 ||
		lstrcmpi(decode, "*EVENT_NINEFOX") == 0))
	{

		p = fileread_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_NineFox = atoi(strBuff);
		else
			rsServerConfig.Event_NineFox = 20;

		flag++;
	}


	if(!flag && (lstrcmp(decode, "*ũ���������̺�Ʈ") == 0 ||
		lstrcmpi(decode, "*EVENT_CHRISTMAS") == 0))
	{
		p = fileread_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.EVENT_Christmas = atoi(strBuff);
		else
			rsServerConfig.EVENT_Christmas = 100;

		flag++;
	}
	if(!flag && (lstrcmp(decode, "*�߷�Ÿ�ε���_�̺�Ʈ") == 0 ||
		lstrcmpi(decode, "*EVENT_VALENTINEDAY") == 0))
	{

		p = fileread_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_ValentineDay = atoi(strBuff);
		else
			rsServerConfig.Event_ValentineDay = 100;

		flag++;
	}


	if(!flag && (lstrcmp(decode, "*Ŀ�ø�_�̺�Ʈ") == 0 ||
		lstrcmpi(decode, "*EVENT_COUPLERING") == 0))
	{
		p = fileread_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_CoupleRing = atoi(strBuff);
		else
			rsServerConfig.Event_CoupleRing = 100;

		flag++;
	}


	if(!flag && (lstrcmp(decode, "*����_�̺�Ʈ") == 0 ||
		lstrcmpi(decode, "*EVENT_WATERMELON") == 0))
	{
		p = fileread_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_Watermelon = atoi(strBuff);
		else
			rsServerConfig.Event_Watermelon = 100;

		flag++;
	}


	if(!flag && (lstrcmp(decode, "*�߷�Ÿ��_�̺�Ʈ") == 0 ||
		lstrcmpi(decode, "*EVENT_VALENTINE") == 0))
	{
		p = fileread_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_Valentine = atoi(strBuff);
		else
			rsServerConfig.Event_Valentine = 100;

		flag++;
	}



	if(!flag && (lstrcmp(decode, "*ĵ������_�̺�Ʈ") == 0 ||
		lstrcmpi(decode, "*EVENT_CANDYDAYS") == 0))
	{

		p = fileread_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_Candydays = atoi(strBuff);
		else
			rsServerConfig.Event_Candydays = 100;

		flag++;
	}


	if(!flag && (lstrcmp(decode, "*���ϴüҿ�_�̺�Ʈ") == 0 ||
		lstrcmpi(decode, "*EVENT_STAR") == 0))
	{
		p = fileread_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_Star = atoi(strBuff);
		else
			rsServerConfig.Event_Star = 100;

		flag++;
	}


	if(!flag && (lstrcmp(decode, "*���ĺ�����_�̺�Ʈ") == 0 ||
		lstrcmpi(decode, "*EVENT_PRISTON_ALPHABET") == 0))
	{
		p = fileread_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_PristonAlphabet = atoi(strBuff);
		else
			rsServerConfig.Event_PristonAlphabet = 100;
		flag++;
	}



	if(!flag && (lstrcmp(decode, "*ȭ��Ʈ��Ʈ��Ʈ_�̺�Ʈ") == 0 ||
		lstrcmpi(decode, "*EVENT_WHITECONTRAST") == 0))
	{
		p = fileread_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_WhiteContrast = atoi(strBuff);
		else
			rsServerConfig.Event_WhiteContrast = 100;

		flag++;
	}


	if(!flag && (lstrcmp(decode, "*����������ȥ_�̺�Ʈ") == 0 ||
		lstrcmpi(decode, "*EVENT_BLACKSMITH") == 0))
	{
		p = fileread_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_BlackSmith = atoi(strBuff);
		else
			rsServerConfig.Event_BlackSmith = 100;

		flag++;
	}


	if(!flag && (lstrcmp(decode, "*�����ñ׸�_�̺�Ʈ") == 0 ||
		lstrcmpi(decode, "*EVENT_MAGICALGREEN") == 0))
	{
		p = fileread_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_MagicalGreen = atoi(strBuff);
		else
			rsServerConfig.Event_MagicalGreen = 100;

		flag++;
	}


	if(!flag && (lstrcmp(decode, "*ī���Ǵ���_�̺�Ʈ") == 0 ||
		lstrcmpi(decode, "*EVENT_TEAROFKARA") == 0))
	{

		p = fileread_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_TearOfKara = atoi(strBuff);
		else
			rsServerConfig.Event_TearOfKara = 100;

		flag++;
	}


	if(!flag && (lstrcmp(decode, "*ȭ��Ʈ����_�̺�Ʈ") == 0 ||
		lstrcmpi(decode, "*EVENT_WHITEDAY") == 0))
	{

		p = fileread_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_WhiteDay = atoi(strBuff);
		else
			rsServerConfig.Event_WhiteDay = 100;

		flag++;
	}

	if(!flag && (lstrcmp(decode, "*�����ǽ���_�̺�Ʈ") == 0 ||
		lstrcmpi(decode, "*EVENT_MORIFSHELTOM") == 0))
	{

		p = fileread_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_MorifSheltom = atoi(strBuff);
		else
			rsServerConfig.Event_MorifSheltom = 1;

		flag++;
	}

	if(!flag && (lstrcmp(decode, "*������_�̺�Ʈ") == 0 ||
		lstrcmpi(decode, "*EVENT_WORLDCUP") == 0))
	{

		p = fileread_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_WorldCup = atoi(strBuff);
		else
			rsServerConfig.Event_WorldCup = 5;

		flag++;
	}


	if(!flag && (lstrcmp(decode, "*���ڸ���_�̺�Ʈ") == 0 ||
		lstrcmpi(decode, "*EVENT_WATERMELONMONSTER") == 0))
	{

		p = fileread_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_WarerMelonMonster = atoi(strBuff);
		else
			rsServerConfig.Event_WarerMelonMonster = 1;

		flag++;
	}



	if(!flag && (lstrcmp(decode, "*����ʹ���_�̺�Ʈ") == 0 ||
		lstrcmpi(decode, "*EVENT_MOLLYWOLF") == 0))
	{

		p = fileread_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_MollyWolf = atoi(strBuff);
		else
			rsServerConfig.Event_MollyWolf = 9;

		flag++;
	}


	if(!flag && (lstrcmp(decode, "*����_�̺�Ʈ") == 0 || lstrcmpi(decode, "*EVENT_CHICHEN_DAY") == 0))
	{
		p = fileread_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_Chichen_Day = atoi(strBuff);
		else
			rsServerConfig.Event_Chichen_Day = 1;

		flag++;
	}

	if(!flag && (lstrcmp(decode, "*���Ʈ_�̺�Ʈ") == 0 ||
		lstrcmpi(decode, "*EVENT_GHOST") == 0))
	{

		p = fileread_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_Ghost = atoi(strBuff);
		else
			rsServerConfig.Event_Ghost = 2;

		flag++;
	}

	if(!flag && (lstrcmp(decode, "*��Ÿ����_�̺�Ʈ") == 0 ||
		lstrcmpi(decode, "*EVENT_SANTAGOBLIN") == 0))
	{

		p = fileread_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_SantaGoblin = atoi(strBuff);
		else
			rsServerConfig.Event_SantaGoblin = 5;

		flag++;
	}

	if(!flag && (lstrcmp(decode, "*�����_�̺�Ʈ") == 0 ||
		lstrcmpi(decode, "*EVENT_SUMMER_CHICKEN") == 0))
	{

		p = fileread_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_SummerChicken = atoi(strBuff);
		else
			rsServerConfig.Event_SummerChicken = 5;

		flag++;
	}

	if(!flag && (lstrcmp(decode, "*�ǽù���_�̺�Ʈ") == 0 ||
		lstrcmpi(decode, "*EVENT_PCBANG_PET") == 0))
	{

		p = fileread_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_PCBangPet = atoi(strBuff);
		else
			rsServerConfig.Event_PCBangPet = 1;

		flag++;
	}

	if(!flag && (lstrcmp(decode, "*����ġ��_�̺�Ʈ") == 0 ||
		lstrcmpi(decode, "*EVENT_EXPUP") == 0))
	{

		p = fileread_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_ExpUp = atoi(strBuff);
		else
			rsServerConfig.Event_ExpUp = 120;



		if(rsServerConfig.Event_ExpUp >= 100 && rsServerConfig.Event_ExpUp < 200)
		{
			rsServerConfig.Event_ExpUp = 100;
		}
		else if(rsServerConfig.Event_ExpUp >= 200 && rsServerConfig.Event_ExpUp < 300)
		{
			rsServerConfig.Event_ExpUp = 200;
		}
		else if(rsServerConfig.Event_ExpUp >= 300 && rsServerConfig.Event_ExpUp < 400)
		{
			rsServerConfig.Event_ExpUp = 300;
		}
		else if(rsServerConfig.Event_ExpUp >= 400)
		{
			rsServerConfig.Event_ExpUp = 400;
		}
		else
		{
			rsServerConfig.Event_ExpUp = 100;
		}

		flag++;
	}

#ifdef _LANGUAGE_ARGENTINA

	if(!flag && (lstrcmp(decode, "*����ġ��_�̺�Ʈ_��ƾ") == 0 || lstrcmpi(decode, "*EVENT_EXPUP_LATIN") == 0))
	{

		p = fileread_GetWord(strBuff, p);
		if(strBuff[0])
		{
			rsServerConfig.Event_ExpUp_latin = atoi(strBuff);
		}
		else
		{
			rsServerConfig.Event_ExpUp_latin = 100;
		}

		flag++;
	}

#endif




	if(!flag && (lstrcmp(decode, "*��������_�̺�Ʈ") == 0 ||
		lstrcmpi(decode, "*EVENT_HALLOWEEN") == 0))
	{
		p = fileread_GetWord(strBuff, p);
		if(strBuff[0])
		{
			rsServerConfig.Event_GiftBox = atoi(strBuff);
		}
		else
		{
			rsServerConfig.Event_GiftBox = 0;
		}

		flag++;
	}



	if(!flag && (lstrcmp(decode, "*�ð�����ġ") == 0 || lstrcmpi(decode, "*EXP_GAMETIME") == 0))
	{
		p = fileread_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.ExpGameTimeMode = atoi(strBuff);
		else
			rsServerConfig.ExpGameTimeMode = 1;

		flag++;
	}


	if(!flag && (lstrcmp(decode, "*�����ĳ��") == 0 ||
		lstrcmpi(decode, "*EVENT_BLESSCASTLE") == 0))
	{

		p = fileread_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.BlessCastleMode = atoi(strBuff);
		else
			rsServerConfig.BlessCastleMode = 1;

		flag++;
	}
	if(!flag && (lstrcmp(decode, "*�����ð�") == 0 ||
		lstrcmpi(decode, "*EVENT_SIEGE_TIME") == 0))
	{

		p = fileread_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.sBlessCastle_StartTime[0] = atoi(strBuff);
		p = fileread_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.sBlessCastle_StartTime[1] = atoi(strBuff);

		flag++;
	}

	if(!flag && (lstrcmp(decode, "*�̺�Ʈ��Ʈ����") == 0 ||
		lstrcmpi(decode, "*EVENT_BITMASK") == 0))
	{

		p = fileread_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.EventPlay_BitMask = atoi(strBuff);

		flag++;
	}

	if(!flag && (lstrcmp(decode, "*�ű���������") == 0 ||
		lstrcmpi(decode, "*DEFAULT_ITEM_USER") == 0))
	{

		p = fileread_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.StartUser_PresentItem = atoi(strBuff);
		else
			rsServerConfig.StartUser_PresentItem = TRUE;

		flag++;
	}


	if(!flag && (lstrcmp(decode, "*�����ڱ��") == 0 ||
		lstrcmpi(decode, "*RECORD_JOBCHANGER") == 0))
	{
		rsServerConfig.Record_JobChanger = TRUE;;
		flag++;
	}

	if(!flag && (lstrcmp(decode, "*��������㰡") == 0 ||
		lstrcmpi(decode, "*ENABLE_BLOCK_FOREVER") == 0))
	{
		rsServerConfig.ForeverBlockMode = TRUE;
		flag++;
	}

	if(!flag && (lstrcmp(decode, "*����ġ��������") == 0 ||
		lstrcmpi(decode, "*PERMIT_EXP") == 0))
	{
		pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		rsServerConfig.Permit_Exp = atoi(strBuff);
		flag++;
	}
	if(!flag && (lstrcmp(decode, "*����������") == 0 ||
		lstrcmpi(decode, "*PERMIT_MONEY") == 0))
	{
		pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		rsServerConfig.Permit_Money = atoi(strBuff);
		flag++;
	}

	if(!flag && (lstrcmp(decode, "*���μ����ð�����") == 0 ||
		lstrcmpi(decode, "*CLIENT_CHECK_PROCESS_TIME") == 0))
	{
		pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		rsServerConfig.ClientProcessTimeChk = atoi(strBuff);
		flag++;
	}
	if(!flag && (lstrcmp(decode, "*���μ����ð�����") == 0 ||
		lstrcmpi(decode, "*CLIENT_PROCESS_TIME_OUT") == 0))
	{
		rsServerConfig.ClientProcessTimeChkOut = TRUE;
		flag++;
	}
	if(!flag && (lstrcmp(decode, "*���హ��Ȯ��") == 0 ||
		lstrcmpi(decode, "*CHECK_POTION_COUNT") == 0))
	{
		pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		rsServerConfig.Dis_PotionError = atoi(strBuff);
		flag++;
	}
	if(!flag && (lstrcmp(decode, "*��������ġ���") == 0 ||
		lstrcmpi(decode, "*ENABLE_SERVER_EXP") == 0))
	{
		pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);

		if(strBuff[0]) rsServerConfig.ExpFixMode = atoi(strBuff);
		else rsServerConfig.ExpFixMode = 50;

		flag++;
	}

	if(!flag && (lstrcmp(decode, "*����������") == 0 ||
		lstrcmpi(decode, "*BLOCK_USER_PATH") == 0))
	{
		pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		strcpy_s(rsServerConfig.szBlockUserPath, strBuff);
		flag++;
	}

	if(!flag && (lstrcmp(decode, "*���ð�ǥ��") == 0 ||
		lstrcmpi(decode, "*DISP_BILLING_TIME") == 0))
	{

		pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		if(strBuff[0])
			rsServerConfig.DispBillingTime = atoi(strBuff);
		else
			rsServerConfig.DispBillingTime = 1;

		flag++;
	}

	//if(!flag && (lstrcmp(decode, "*�α��α���") == 0 ||
	//	lstrcmpi(decode, "*DISABLE_LOGIN") == 0))
	//{

	//	pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
	//	if(strBuff[0])
	//		rsServerConfig.Disable_LogIn = atoi(strBuff);
	//	else
	//		rsServerConfig.Disable_LogIn = 1;
	//	flag++;
	//}

	if(!flag &&	lstrcmpi(decode, "*ALLOWED_LOGIN") == 0)
	{
		pb = p; 
		p = fileread_GetWord(strBuff, p); 

		if(strBuff[0] == 34)
			p = GetString(strBuff, pb);

		if(strBuff[0])
			rsServerConfig.allowed_login = atoi(strBuff);
		else
			rsServerConfig.allowed_login = 0;

		flag++;
	}

	if(!flag && (lstrcmp(decode, "*�α�ť����") == 0 ||
		lstrcmpi(decode, "*DISABLE_LOGQUE") == 0))
	{
		rsServerConfig.Disable_LogQue = TRUE;
		flag++;
	}
	if(!flag && (lstrcmp(decode, "*�α׾ƿ�Ȯ�α���") == 0 ||
		lstrcmpi(decode, "*DISABLE_CHECK_LOGOUT") == 0))
	{
		rsServerConfig.Disable_CheckLogOut = TRUE;
		flag++;
	}

	//if(!flag && (lstrcmp(decode, "*�α�������") == 0 ||
	//	lstrcmpi(decode, "*DISABLE_LOGINX") == 0))
	//{
	//	rsServerConfig.Disable_LogIn2 = TRUE;
	//	flag++;
	//}


	if(!flag && (lstrcmp(decode, "*���᷹��") == 0 ||
		lstrcmpi(decode, "*FREE_LEVEL") == 0))
	{
		pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		rsServerConfig.FreeLevel = atoi(strBuff);

		pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		if(strBuff[0])
			strcpy_s(rsServerConfig.szFreeGpCode, strBuff);

		flag++;
	}

	if(!flag && (lstrcmp(decode, "*��������") == 0 ||
		lstrcmpi(decode, "*PRESENT_LEVEL") == 0))
	{
		pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		rsServerConfig.PresentLevel = atoi(strBuff);
		flag++;
	}

	if(!flag && (lstrcmp(decode, "*������������") == 0 ||
		lstrcmpi(decode, "*ENABLE_PARTYITEM") == 0))
	{

		rsServerConfig.Enable_PartyItem = TRUE;
		flag++;
	}

	if(!flag && (lstrcmp(decode, "*�ʵ������˻�") == 0 ||
		lstrcmpi(decode, "*CHECK_PLAYFIELD") == 0))
	{

		rsServerConfig.CheckPlayField = TRUE;
		flag++;
	}


	if(!flag && (lstrcmp(decode, "*�����ϰ˻�") == 0 ||
		lstrcmpi(decode, "*CHECK_MODELFILE") == 0))
	{

		rsServerConfig.CheckModelFile = TRUE;
		flag++;
	}


	if(!flag && (lstrcmp(decode, "*��������IP") == 0 ||
		lstrcmpi(decode, "*DISABLE_IP") == 0))
	{

		if(rsServerConfig.DisableIP_Count < DISABLE_IP_MAX)
		{
			p = fileread_GetWord(strBuff, p);
			rsServerConfig.DisableIP[rsServerConfig.DisableIP_Count][0] = GetSwapIPCode(strBuff);
			p = fileread_GetWord(strBuff, p);
			rsServerConfig.DisableIP[rsServerConfig.DisableIP_Count][1] = GetSwapIPCode(strBuff);
			rsServerConfig.DisableIP_Count++;
		}

		flag++;
	}

	if(!flag && (lstrcmp(decode, "*�����㰡IP") == 0 ||
		lstrcmpi(decode, "*ENABLE_IP") == 0))
	{

		if(rsServerConfig.EnableIP_Count < ENABLE_IP_MAX)
		{
			p = fileread_GetWord(strBuff, p);
			rsServerConfig.EnableIP[rsServerConfig.EnableIP_Count][0] = GetSwapIPCode(strBuff);
			p = fileread_GetWord(strBuff, p);
			rsServerConfig.EnableIP[rsServerConfig.EnableIP_Count][1] = GetSwapIPCode(strBuff);
			rsServerConfig.EnableIP_Count++;
		}

		flag++;
	}

	if(!flag && (lstrcmp(decode, "*��������IP") == 0 ||
		lstrcmpi(decode, "*ADMINISTRATOR_IP") == 0))
	{

		if(rsServerConfig.AdminIP_Count < ADMIN_IP_MAX)
		{
			p = fileread_GetWord(strBuff, p);
			rsServerConfig.AdminIP[rsServerConfig.AdminIP_Count][0] = GetSwapIPCode(strBuff);
			p = fileread_GetWord(strBuff, p);
			rsServerConfig.AdminIP[rsServerConfig.AdminIP_Count][1] = GetSwapIPCode(strBuff);
			rsServerConfig.AdminIP_Count++;
		}

		flag++;
	}

	if(!flag && (lstrcmp(decode, "*�ý���IP") == 0 ||
		lstrcmpi(decode, "*SYSTEM_IP") == 0))
	{

		if(rsServerConfig.SystemIP_Count < ADMIN_IP_MAX)
		{
			p = fileread_GetWord(strBuff, p);
			rsServerConfig.SystemIP[rsServerConfig.SystemIP_Count][0] = GetSwapIPCode(strBuff);
			p = fileread_GetWord(strBuff, p);
			rsServerConfig.SystemIP[rsServerConfig.SystemIP_Count][1] = GetSwapIPCode(strBuff);
			rsServerConfig.SystemIP_Count++;
		}

		flag++;
	}

	if(!flag && (lstrcmp(decode, "*�α��μ���IP") == 0 ||
		lstrcmpi(decode, "*LOGIN_SERVER_IP") == 0))
	{

		if(rsServerConfig.SystemIP_Count < LOGIN_IP_MAX)
		{
			p = fileread_GetWord(strBuff, p);
			rsServerConfig.LoginServerIP[rsServerConfig.LoginServerIP_Count][0] = GetSwapIPCode(strBuff);
			p = fileread_GetWord(strBuff, p);
			rsServerConfig.LoginServerIP[rsServerConfig.LoginServerIP_Count][1] = GetSwapIPCode(strBuff);
			rsServerConfig.LoginServerIP_Count++;
		}

		flag++;
	}

	if(!flag && (lstrcmp(decode, "*�����κ��丮") == 0 ||
		lstrcmpi(decode, "*SERVER_INVENTORY") == 0))
	{

		rsServerConfig.ServerInventoryMode = TRUE;
		flag++;
	}

	if(!flag && (lstrcmp(decode, "*��������̸�") == 0 ||
		lstrcmpi(decode, "*BACKUP_PATH") == 0))
	{
		pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		strcpy_s(rsServerConfig.szBackupPath, strBuff);
		flag++;
	}

	if(!flag && (lstrcmp(decode, "*��ų����Ʈ���ҺҰ�") == 0 ||
		lstrcmpi(decode, "*DISABLE_DEC_SKILLPOINT") == 0))
	{

		rsServerConfig.Disable_DecSkillPoint = TRUE;
		flag++;
	}

	if(!flag && (lstrcmp(decode, "*����Ʈ����") == 0 ||
		lstrcmpi(decode, "*DISABLE_QUEST") == 0))
	{

		pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		rsServerConfig.Disable_Quest = atoi(strBuff);
		flag++;
	}

	if(!flag && (lstrcmp(decode, "*�����ʵ����") == 0 ||
		lstrcmpi(decode, "*START_FIELD") == 0))
	{

		pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		if(strBuff[0])
			rsServerConfig.FixedStartField = atoi(strBuff);
		flag++;
	}

	if(!flag && (lstrcmp(decode, "*�����۹�������") == 0 ||
		lstrcmpi(decode, "*DISABLE_THROWITEM") == 0))
	{

		pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		if(strBuff[0])
			rsServerConfig.DisableThrowItem = atoi(strBuff);
		else
			rsServerConfig.DisableThrowItem = 1;

		flag++;
	}



	if(!flag && (lstrcmp(decode, "*ũ��������Ŷ") == 0 ||
		lstrcmpi(decode, "*CRAZY_PACKET") == 0))
	{

		rsServerConfig.CrazyPacketMode = TRUE;
		flag++;
	}

	if(!flag && (lstrcmp(decode, "*�뺸�ּ�") == 0 ||
		lstrcmpi(decode, "*NOTICE_URL") == 0))
	{
		pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		strcpy_s(rsServerConfig.szNoticeURL, strBuff);
		flag++;
	}

	if(!flag && (lstrcmp(decode, "*������ɱ��") == 0 ||
		lstrcmpi(decode, "*REC_ADMIN_CMD") == 0))
	{
		pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		if(strBuff[0])
			rsServerConfig.RecordAdminCommand = atoi(strBuff);
		else
			rsServerConfig.RecordAdminCommand = 2;

		flag++;
	}


	if(!flag && (lstrcmp(decode, "*��������IP") == 0 ||
		lstrcmpi(decode, "*SERVER_LINK_IP") == 0))
	{
		pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		strcpy_s(rsServerConfig.szLinkServerIP, strBuff);
		flag++;
	}


	if(!flag && (lstrcmp(decode, "*��������") == 0 ||
		lstrcmpi(decode, "*AREA_SERVER") == 0))
	{

		for(cnt = 0; cnt < 32; cnt++)
		{
			if(rsServerConfig.AreaServer.IP_Count >= SERVER_MAP_MAX) break;

			pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
			if(!strBuff[0]) break;

			rsServerConfig.AreaServer.dwIP[rsServerConfig.AreaServer.IP_Count] = inet_addr(strBuff);

			rsServerConfig.AreaServer.IP_Count++;
			rsServerConfig.AreaServer.size = sizeof(TRANS_SEVER_MAP) - (sizeof(DWORD)*(SERVER_MAP_MAX - rsServerConfig.AreaServer.IP_Count));
			rsServerConfig.AreaServer.code = OpCode::OPCODE_AREA_SERVER_MAP;
		}
		flag++;
	}

	if(!flag && (lstrcmp(decode, "*��ʸ�Ʈ��������") == 0 ||
		lstrcmpi(decode, "*TT_DATA_SERVER") == 0))
	{

		cnt = rsServerConfig.TT_DataServer_Count;

		if(cnt >= TT_DATA_SERVER_MAX) goto skip_t;

		pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		if(!strBuff[0]) goto skip_t;
		strcpy_s(rsServerConfig.TT_DataServer[cnt].szServerID, strBuff);

		pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		if(!strBuff[0]) goto skip_t;
		strcpy_s(rsServerConfig.TT_DataServer[cnt].szServerPath, strBuff);

		rsServerConfig.TT_DataServer_Count++;
	skip_t:
		flag++;
	}


	return flag;
}

#else


int SetServerConfig(char *decode, char *lpPoint)
{

	return NULL;
}

#endif



int SetServerSelectList(char *decode, char *lpPoint)
{
	int flag;
	char *p = lpPoint;
	char *pb;
	char strBuff[128];
	int cnt;

	flag = 0;

	if(TransServerList.ServerCount >= TRANS_SERVER_MAX) return FALSE;

	if(lstrcmp(decode, "*���Ӽ�������") == 0 ||
		lstrcmpi(decode, "*GAME_SERVER") == 0)
	{
		cnt = TransServerList.ServerCount;

		pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		if(strBuff[0])
		{
			strcpy_s(TransServerList.ServerInfo[cnt].szServerName, strBuff);
		}

		pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		if(strBuff[0])
		{
			strcpy_s(TransServerList.ServerInfo[cnt].szServerIp1, strBuff);
			TransServerList.ServerInfo[cnt].dwServerPort1 = TCP_SERVPORT;
		}

		pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		if(strBuff[0])
		{
			strcpy_s(TransServerList.ServerInfo[cnt].szServerIp2, strBuff);
			TransServerList.ServerInfo[cnt].dwServerPort2 = TCP_SERVPORT;
		}

		pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		if(strBuff[0])
		{
			strcpy_s(TransServerList.ServerInfo[cnt].szServerIp3, strBuff);
			TransServerList.ServerInfo[cnt].dwServerPort3 = TCP_SERVPORT;
		}

		TransServerList.ServerCount++;
		flag++;
	}

	if(!flag && (lstrcmp(decode, "*Ŭ����������") == 0 ||
		lstrcmpi(decode, "*CLAN_SERVER") == 0))
	{

		cnt = TransServerList.ServerCount + TransServerList.ClanServerCount;

		strcpy_s(TransServerList.ServerInfo[cnt].szServerName, "Clan");

		pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		if(strBuff[0])
			strcpy_s(TransServerList.ServerInfo[cnt].szServerIp1, strBuff);

		pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		if(strBuff[0])
			TransServerList.ServerInfo[cnt].dwServerPort1 = atoi(strBuff);

		pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		if(strBuff[0])
			strcpy_s(TransServerList.ServerInfo[cnt].szServerIp2, strBuff);

		pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		if(strBuff[0])
			TransServerList.ServerInfo[cnt].dwServerPort2 = atoi(strBuff);

		pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		if(strBuff[0])
			strcpy_s(TransServerList.ServerInfo[cnt].szServerIp3, strBuff);

		pb = p; p = fileread_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		if(strBuff[0])
			TransServerList.ServerInfo[cnt].dwServerPort3 = atoi(strBuff);

		TransServerList.ClanServerCount++;
		flag++;
	}


	return flag;
}



