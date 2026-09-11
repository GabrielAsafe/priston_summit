int SetServerConfig(char *decode, char *lpPoint)
{
	
	int flag;
	char *p = lpPoint;
	char *pb;
	char strBuff[128];
	int cnt;

	flag = 0;

	



	if(lstrcmp(decode, "\x2A\xBC\xAD\xB9\xF6\xC4\xDA\xB5\xE5\xB9\xF8\xC8\xA3") == 0 ||
		lstrcmpi(decode, "*SERVER_CODE") == 0)
	{
		pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		rsServerConfig.ServerCode = atoi(strBuff);
		flag++;
	}

	if(!flag && (lstrcmp(decode, "\x2A\xC0\xDA\xB5\xBF\xC4\xB3\xB8\xAF\xC5\xCD\xBB\xE7\xBF\xEB") == 0 ||
		lstrcmpi(decode, "*SERVER_AUTOPLAY") == 0))
	{
		rsServerConfig.AutoPlayer = TRUE;
		flag++;
	}

	if(!flag && (lstrcmp(decode, "\x2A\xC0\xFA\xC0\xE5\xB9\xF6\xC6\xDB\xBB\xE7\xBF\xEB") == 0 ||
		lstrcmpi(decode, "*SERVER_RECORD_MEM") == 0))
	{
		rsServerConfig.UseRecorMemory = TRUE;
		flag++;
	}
	

	if(!flag && lstrcmp(decode, "\x2A\xB5\xBF\xB7\xE1\xB1\xDD\xC1\xF6") == 0)
	{
		rsServerConfig.DisableParty = TRUE;
		flag++;
	}
	if(!flag && (lstrcmp(decode, "\x2A\xC1\xA2\xBC\xD3\xC1\xA6\xC7\xD1\xC0\xCE\xBF\xF8") == 0 ||
		lstrcmpi(decode, "*MAX_USERS") == 0))
	{
		pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		rsServerConfig.ConnectUserMax = atoi(strBuff);
		flag++;
	}

	if(!flag && (lstrcmp(decode, "\x2A\xC5\xD7\xBD\xBA\xC6\xAE\xBC\xAD\xB9\xF6") == 0 ||
		lstrcmpi(decode, "*TEST_SERVER") == 0))
	{
		rsServerConfig.TestSeverMode = TRUE;
		pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		if(strBuff[0])
		{
			rsServerConfig.TestSeverMode = atoi(strBuff);
		}
		flag++;
	}

	if(!flag && (lstrcmp(decode, "\x2A\xC1\xA2\xBC\xD3\xB0\xA1\xB4\xC9\xC4\xDA\xB5\xE5") == 0 ||
		lstrcmpi(decode, "*CONNECT_CLIENT_CODE") == 0))
	{
		for(cnt = 0; cnt < 32; cnt++)
		{
			p = local_GetWord(strBuff, p);
			if(!strBuff[0]) break;

			rsServerConfig.dwClientCheckSum[rsServerConfig.ClientCheckSumCount] = atoi(strBuff);
			rsServerConfig.ClientCheckSumCount++;
		}
		flag++;
	}

	if(!flag && (lstrcmp(decode, "\x2A\xC4\xDA\xB5\xE5\xC1\xA2\xBC\xD3\xC1\xBE\xB7\xE1") == 0 ||
		lstrcmpi(decode, "*DISCONNECT_CLIENT_BADCODE") == 0))
	{
		rsServerConfig.ClientCheckOut = TRUE;
		flag++;
	}

	if(!flag && (lstrcmp(decode, "\x2A\xB3\xBB\xBA\xCE\xBA\xB8\xC8\xA3\xC4\xDA\xB5\xE5") == 0 ||
		lstrcmpi(decode, "*CLSAFE_CODE") == 0))
	{

		for(cnt = 0; cnt < 32; cnt++)
		{
			p = local_GetWord(strBuff, p);
			if(!strBuff[0]) break;

			rsServerConfig.dwSafeClientCode[rsServerConfig.SafeClientCodeCount] = atoi(strBuff);
			rsServerConfig.SafeClientCodeCount++;
		}
		flag++;



	}
	if(!flag && (lstrcmp(decode, "\x2A\xBA\xB8\xC8\xA3\xC4\xDA\xB5\xE5\xBF\xC0\xB7\xF9\xC1\xBE\xB7\xE1") == 0 ||
		lstrcmpi(decode, "*DISCONNECT_CLSAFE_CODE") == 0))
	{
		rsServerConfig.ClientCodeCheckOut = TRUE;
		flag++;
	}


	if(!flag && (lstrcmp(decode, "\x2A\xC1\xA6\xC7\xD1\xB9\xF6\xC0\xFC") == 0 ||
		lstrcmpi(decode, "*VERSION") == 0))
	{
		pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		rsServerConfig.LimitVersion = atoi(strBuff);

		flag++;
	}


	if(!flag && (lstrcmp(decode, "\x2A\xBC\xAD\xB9\xF6\xC0\xCC\xB8\xA7") == 0 ||
		lstrcmpi(decode, "*SERVER_NAME") == 0))
	{
		pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		strcpy_s(rsServerConfig.szServerName, strBuff);
		flag++;
	}

	if(!flag && (lstrcmp(decode, "\x2A\x50\x4B\xC7\xE3\xB0\xA1") == 0 ||
		lstrcmpi(decode, "*ENABLE_PK") == 0))
	{

		pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		rsServerConfig.Enable_PK = atoi(strBuff);
		if(rsServerConfig.Enable_PK == 0) rsServerConfig.Enable_PK = TRUE;

		flag++;
	}

	if(!flag && (lstrcmp(decode, "\x2A\x50\x4B\xC7\xCA\xB5\xE5") == 0 ||
		lstrcmpi(decode, "*PK_FIELD") == 0))
	{

		pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		rsServerConfig.Enable_PKField = atoi(strBuff);
		if(rsServerConfig.Enable_PKField == 0) rsServerConfig.Enable_PKField = 46;

		flag++;
	}
	if (!flag && (lstrcmp(decode, "\x2A\x50\x4B\xC7\xCA\xB5\xE5") == 0 ||
		lstrcmpi(decode, "*PK_FIELD1") == 0))
	{

		pb = p; p = local_GetWord(strBuff, p); if (strBuff[0] == 34)p = GetString(strBuff, pb);
		rsServerConfig.Enable_PKField1 = atoi(strBuff);
		if (rsServerConfig.Enable_PKField1 == 0) rsServerConfig.Enable_PKField1 = 47;

		flag++;
	}
	if (!flag && (lstrcmp(decode, "\x2A\x50\x4B\xC7\xCA\xB5\xE5") == 0 ||
		lstrcmpi(decode, "*PK_FIELD2") == 0))
	{

		pb = p; p = local_GetWord(strBuff, p); if (strBuff[0] == 34)p = GetString(strBuff, pb);
		rsServerConfig.Enable_PKField2 = atoi(strBuff);
		if (rsServerConfig.Enable_PKField2 == 0) rsServerConfig.Enable_PKField2 = 50;

		flag++;
	}

	if (!flag && (lstrcmp(decode, "\x2A\x50\x4B\xC7\xCA\xB5\xE5") == 0 ||
		lstrcmpi(decode, "*PK_FIELD3") == 0))
	{

		pb = p; p = local_GetWord(strBuff, p); if (strBuff[0] == 34)p = GetString(strBuff, pb);
		rsServerConfig.Enable_PKField3 = atoi(strBuff);
		if (rsServerConfig.Enable_PKField3 == 0) rsServerConfig.Enable_PKField3 = 37;

		flag++;
	}


	if(!flag && (lstrcmp(decode, "\x2A\x50\x4B\xB8\xF0\xB5\xE7\xC7\xCA\xB5\xE5") == 0 || lstrcmpi(decode, "*PK_FIELD_ALL") == 0))
	{
		pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		rsServerConfig.Enable_PKField_All = atoi(strBuff);
		flag++;
	}


	if(!flag && (lstrcmp(decode, "\x2A\xB9\xB0\xBE\xE0\xB0\xA8\xBD\xC3") == 0 ||
		lstrcmpi(decode, "*POTION_MONITOR") == 0))
	{

		pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		if(strBuff[0])
			rsServerConfig.PotionMonitor = atoi(strBuff);
		else
			rsServerConfig.PotionMonitor = 1;

		flag++;
	}

	if(!flag && (lstrcmp(decode, "\x2A\xBA\xB8\xBE\xC8\xB0\xCB\xBB\xE7") == 0 ||
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


	if(!flag && (lstrcmp(decode, "\x2A\xBA\xB8\xBE\xC8\xC1\xA6\xB0\xC5") == 0 ||
		lstrcmpi(decode, "*REMOVE_PROTECT_TIME") == 0))
	{
		pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		rsServerConfig.RemoveNProtectTime = atoi(strBuff);
		flag++;
	}


	if(!flag && (lstrcmp(decode, "\x2A\xBA\xED\xB7\xB0\xB4\xDC\xB0\xE8") == 0 ||
		lstrcmpi(decode, "*BLOCK_LEVEL") == 0))
	{
		pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		rsServerConfig.BlockLevel = atoi(strBuff);
		flag++;
	}

	if(!flag && (lstrcmp(decode, "\x2A\xBA\xED\xB7\xB0\xBE\xC6\xBF\xF4") == 0 ||
		lstrcmpi(decode, "*BLOCK_LOGOUT") == 0))
	{
		rsServerConfig.BlockLevel_LogOut = TRUE;
		flag++;
	}

	if(!flag && (lstrcmp(decode, "\x2A\xBE\xC6\xC0\xCC\xC5\xDB\xBF\xC0\xB7\xF9\xC1\xBE\xB7\xE1") == 0 ||
		lstrcmpi(decode, "*DISCONNECT_ITEM_ERROR") == 0))
	{
		pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		if(strBuff[0])
			rsServerConfig.ItemCodeOut = atoi(strBuff);
		else
			rsServerConfig.ItemCodeOut = 1;

		flag++;
	}

	if(!flag && (lstrcmp(decode, "\x2A\xC6\xE3\xBC\xC7\xB0\xCB\xBB\xE7") == 0 ||
		lstrcmpi(decode, "*CLIENT_CHECK_FUNC") == 0))
	{
		rsServerConfig.ClientFuncCheckMode = 1;
		pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		rsServerConfig.ClientFuncCheckMode = atoi(strBuff);
		flag++;
	}

	if(!flag && (lstrcmp(decode, "\x2A\xBC\xDB\xC6\xED\xC0\xCC\xBA\xA5\xC6\xAE") == 0 ||
		lstrcmpi(decode, "*EVENT_MANDOO") == 0))
	{
		rsServerConfig.Event_Mandoo = TRUE;;
		flag++;
	}
	if(!flag && (lstrcmp(decode, "\x2A\xC7\xCF\xB5\xE5\xC4\xDA\xBE\xEE\xC0\xCC\xBA\xA5\xC6\xAE") == 0 ||
		lstrcmpi(decode, "*EVENT_HARDCORE") == 0))
	{

		p = local_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_HardCore = atoi(strBuff);
		else
			rsServerConfig.Event_HardCore = 1;

		flag++;
	}
#ifdef	_LANGUAGE_JAPANESE 
	if(!flag && (lstrcmp(decode, "\x2A\xB0\xF8\xB0\xDD\xB7\xC2\xC8\xB0\xC0\xCE\xC0\xCC\xBA\xA5\xC6\xAE") == 0 ||
		lstrcmpi(decode, "*EVENT_DAMAGEOFF") == 0))
	{
		p = local_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_DamageOff = atoi(strBuff);
		else
			rsServerConfig.Event_DamageOff = 1;
		flag++;
	}
#else
	if(!flag && (lstrcmpi(decode, "*EVENT_DAMAGEOFF1") == 0))
	{
		p = local_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_DamageOff1 = atoi(strBuff);
		else
			rsServerConfig.Event_DamageOff1 = 1;
		flag++;
	}
	if(!flag && (lstrcmpi(decode, "*EVENT_DAMAGEOFF2") == 0))
	{
		p = local_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_DamageOff2 = atoi(strBuff);
		else
			rsServerConfig.Event_DamageOff2 = 1;
		flag++;
	}
#endif

	if(!flag && (lstrcmp(decode, "\x2A\xB9\xE6\xBE\xEE\xB7\xC2\xC7\xD2\xC0\xCE\xC0\xCC\xBA\xA5\xC6\xAE") == 0 ||
		lstrcmpi(decode, "*EVENT_DEFENSEOFF") == 0))
	{
		p = local_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_DefenseOff = atoi(strBuff);
		else
			rsServerConfig.Event_DefenseOff = 1;
		flag++;
	}

	if(!flag && (lstrcmp(decode, "\x2A\xBE\xEE\xB8\xB0\xC0\xCC\xC0\xCC\xBA\xA5\xC6\xAE") == 0 ||
		lstrcmpi(decode, "*EVENT_CHILD") == 0))
	{

		p = local_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_Child = atoi(strBuff);
		else
			rsServerConfig.Event_Child = 1;

		flag++;
	}


	if(!flag && (lstrcmp(decode, "\x2A\xC5\xA9\xB8\xAE\xBD\xBA\xC5\xBB\xC0\xCC\xBA\xA5\xC6\xAE") == 0 ||
		lstrcmpi(decode, "*EVENT_CRISTAL") == 0))
	{

		p = local_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_Cristal = atoi(strBuff);
		else
			rsServerConfig.Event_Cristal = 1;

		flag++;
	}
	if(!flag && (lstrcmp(decode, "\x2A\xB9\xB0\xBE\xE0\xC0\xCC\xBA\xA5\xC6\xAE") == 0 ||
		lstrcmpi(decode, "*EVENT_POTION") == 0))
	{

		p = local_GetWord(strBuff, p);
		rsServerConfig.Event_Potion = atoi(strBuff);
		flag++;
	}
	if(!flag && (lstrcmp(decode, "\x2A\xBA\xB0\xC6\xF7\xC0\xCE\xC6\xAE\xC0\xCC\xBA\xA5\xC6\xAE") == 0 ||
		lstrcmpi(decode, "*EVENT_STARPOINT") == 0))
	{

		p = local_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_StarPointTicket = atoi(strBuff);
		else
			rsServerConfig.Event_StarPointTicket = 40;

		flag++;
	}

	if(!flag && (lstrcmp(decode, "\x2A\xBB\xFD\xC0\xCF\xC0\xCC\xBA\xA5\xC6\xAE") == 0 ||
		lstrcmpi(decode, "*EVENT_BIRTHDAY") == 0))
	{

		p = local_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_ComicBirthDay = atoi(strBuff);
		else
			rsServerConfig.Event_ComicBirthDay = 1;

		flag++;
	}

	if(!flag && (lstrcmp(decode, "\x2A\xC6\xDB\xC1\xF1\xC0\xCC\xBA\xA5\xC6\xAE") == 0 ||
		lstrcmpi(decode, "*EVENT_PUZZLE") == 0))
	{

		p = local_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_Puzzle = atoi(strBuff);
		else
			rsServerConfig.Event_Puzzle = 200;

		flag++;
	}

	if(!flag && (lstrcmp(decode, "\x2A\xB9\xD9\xBA\xA7\xBB\xD4\xC0\xCC\xBA\xA5\xC6\xAE") == 0 ||
		lstrcmpi(decode, "*EVENT_BABELHORN") == 0))
	{

		p = local_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_BabelHorn = atoi(strBuff);
		else
			rsServerConfig.Event_BabelHorn = 4;

		if(rsServerConfig.Event_BabelHorn > 10) rsServerConfig.Event_BabelHorn = 10;

		flag++;
	}

	if(!flag && (lstrcmp(decode, "\x2A\xB1\xB8\xB9\xCC\xC8\xA3\xC0\xCC\xBA\xA5\xC6\xAE") == 0 ||
		lstrcmpi(decode, "*EVENT_NINEFOX") == 0))
	{

		p = local_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_NineFox = atoi(strBuff);
		else
			rsServerConfig.Event_NineFox = 20;

		flag++;
	}


	if(!flag && (lstrcmp(decode, "\x2A\xC5\xA9\xB8\xAE\xBD\xBA\xB8\xB6\xBD\xBA\xC0\xCC\xBA\xA5\xC6\xAE") == 0 ||
		lstrcmpi(decode, "*EVENT_CHRISTMAS") == 0))
	{
		p = local_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.EVENT_Christmas = atoi(strBuff);
		else
			rsServerConfig.EVENT_Christmas = 100;

		flag++;
	}
	if(!flag && (lstrcmp(decode, "\x2A\xB9\xDF\xB7\xBB\xC5\xB8\xC0\xCE\xB5\xA5\xC0\xCC\x5F\xC0\xCC\xBA\xA5\xC6\xAE") == 0 ||
		lstrcmpi(decode, "*EVENT_VALENTINEDAY") == 0))
	{

		p = local_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_ValentineDay = atoi(strBuff);
		else
			rsServerConfig.Event_ValentineDay = 100;

		flag++;
	}


	if(!flag && (lstrcmp(decode, "\x2A\xC4\xBF\xC7\xC3\xB8\xB5\x5F\xC0\xCC\xBA\xA5\xC6\xAE") == 0 ||
		lstrcmpi(decode, "*EVENT_COUPLERING") == 0))
	{
		p = local_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_CoupleRing = atoi(strBuff);
		else
			rsServerConfig.Event_CoupleRing = 100;

		flag++;
	}


	if(!flag && (lstrcmp(decode, "\x2A\xBC\xF6\xB9\xDA\x5F\xC0\xCC\xBA\xA5\xC6\xAE") == 0 ||
		lstrcmpi(decode, "*EVENT_WATERMELON") == 0))
	{
		p = local_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_Watermelon = atoi(strBuff);
		else
			rsServerConfig.Event_Watermelon = 100;

		flag++;
	}


	if(!flag && (lstrcmp(decode, "\x2A\xB9\xDF\xB7\xBB\xC5\xB8\xC0\xCE\x5F\xC0\xCC\xBA\xA5\xC6\xAE") == 0 ||
		lstrcmpi(decode, "*EVENT_VALENTINE") == 0))
	{
		p = local_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_Valentine = atoi(strBuff);
		else
			rsServerConfig.Event_Valentine = 100;

		flag++;
	}



	if(!flag && (lstrcmp(decode, "\x2A\xC4\xB5\xB5\xF0\xB5\xA5\xC0\xCC\xC1\xEE\x5F\xC0\xCC\xBA\xA5\xC6\xAE") == 0 ||
		lstrcmpi(decode, "*EVENT_CANDYDAYS") == 0))
	{

		p = local_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_Candydays = atoi(strBuff);
		else
			rsServerConfig.Event_Candydays = 100;

		flag++;
	}


	if(!flag && (lstrcmp(decode, "\x2A\xB9\xE3\xC7\xCF\xB4\xC3\xBC\xD2\xBF\xF8\x5F\xC0\xCC\xBA\xA5\xC6\xAE") == 0 ||
		lstrcmpi(decode, "*EVENT_STAR") == 0))
	{
		p = local_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_Star = atoi(strBuff);
		else
			rsServerConfig.Event_Star = 100;

		flag++;
	}


	if(!flag && (lstrcmp(decode, "\x2A\xBE\xCB\xC6\xC4\xBA\xAA\xC1\xB6\xC7\xD5\x5F\xC0\xCC\xBA\xA5\xC6\xAE") == 0 ||
		lstrcmpi(decode, "*EVENT_PRISTON_ALPHABET") == 0))
	{
		p = local_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_PristonAlphabet = atoi(strBuff);
		else
			rsServerConfig.Event_PristonAlphabet = 100;
		flag++;
	}



	if(!flag && (lstrcmp(decode, "\x2A\xC8\xAD\xC0\xCC\xC6\xAE\xC4\xDC\xC6\xAE\xB6\xF3\xBD\xBA\xC6\xAE\x5F\xC0\xCC\xBA\xA5\xC6\xAE") == 0 ||
		lstrcmpi(decode, "*EVENT_WHITECONTRAST") == 0))
	{
		p = local_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_WhiteContrast = atoi(strBuff);
		else
			rsServerConfig.Event_WhiteContrast = 100;

		flag++;
	}


	if(!flag && (lstrcmp(decode, "\x2A\xB4\xEB\xC0\xE5\xC0\xE5\xC0\xCC\xC0\xC7\xC8\xA5\x5F\xC0\xCC\xBA\xA5\xC6\xAE") == 0 ||
		lstrcmpi(decode, "*EVENT_BLACKSMITH") == 0))
	{
		p = local_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_BlackSmith = atoi(strBuff);
		else
			rsServerConfig.Event_BlackSmith = 100;

		flag++;
	}


	if(!flag && (lstrcmp(decode, "\x2A\xB8\xC5\xC1\xF6\xC4\xC3\xB1\xD7\xB8\xB0\x5F\xC0\xCC\xBA\xA5\xC6\xAE") == 0 ||
		lstrcmpi(decode, "*EVENT_MAGICALGREEN") == 0))
	{
		p = local_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_MagicalGreen = atoi(strBuff);
		else
			rsServerConfig.Event_MagicalGreen = 100;

		flag++;
	}


	if(!flag && (lstrcmp(decode, "\x2A\xC4\xAB\xB6\xF3\xC0\xC7\xB4\xAB\xB9\xB0\x5F\xC0\xCC\xBA\xA5\xC6\xAE") == 0 ||
		lstrcmpi(decode, "*EVENT_TEAROFKARA") == 0))
	{

		p = local_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_TearOfKara = atoi(strBuff);
		else
			rsServerConfig.Event_TearOfKara = 100;

		flag++;
	}


	if(!flag && (lstrcmp(decode, "\x2A\xC8\xAD\xC0\xCC\xC6\xAE\xB5\xA5\xC0\xCC\x5F\xC0\xCC\xBA\xA5\xC6\xAE") == 0 ||
		lstrcmpi(decode, "*EVENT_WHITEDAY") == 0))
	{

		p = local_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_WhiteDay = atoi(strBuff);
		else
			rsServerConfig.Event_WhiteDay = 100;

		flag++;
	}

	if(!flag && (lstrcmp(decode, "\x2A\xB8\xF0\xB8\xAE\xC7\xC1\xC0\xC7\xBD\xA9\xC5\xD2\x5F\xC0\xCC\xBA\xA5\xC6\xAE") == 0 ||
		lstrcmpi(decode, "*EVENT_MORIFSHELTOM") == 0))
	{

		p = local_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_MorifSheltom = atoi(strBuff);
		else
			rsServerConfig.Event_MorifSheltom = 1;

		flag++;
	}

	if(!flag && (lstrcmp(decode, "\x2A\xBF\xF9\xB5\xE5\xC4\xC5\x5F\xC0\xCC\xBA\xA5\xC6\xAE") == 0 ||
		lstrcmpi(decode, "*EVENT_WORLDCUP") == 0))
	{

		p = local_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_WorldCup = atoi(strBuff);
		else
			rsServerConfig.Event_WorldCup = 5;

		flag++;
	}


	if(!flag && (lstrcmp(decode, "\x2A\xBC\xF6\xB9\xDA\xB8\xF3\xBD\xBA\xC5\xCD\x5F\xC0\xCC\xBA\xA5\xC6\xAE") == 0 ||
		lstrcmpi(decode, "*EVENT_WATERMELONMONSTER") == 0))
	{

		p = local_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_WarerMelonMonster = atoi(strBuff);
		else
			rsServerConfig.Event_WarerMelonMonster = 1;

		flag++;
	}



	if(!flag && (lstrcmp(decode, "\x2A\xB8\xF4\xB8\xAE\xBF\xCD\xB4\xC1\xB4\xEB\x5F\xC0\xCC\xBA\xA5\xC6\xAE") == 0 ||
		lstrcmpi(decode, "*EVENT_MOLLYWOLF") == 0))
	{

		p = local_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_MollyWolf = atoi(strBuff);
		else
			rsServerConfig.Event_MollyWolf = 9;

		flag++;
	}


	if(!flag && (lstrcmp(decode, "\x2A\xBA\xB9\xB3\xAF\x5F\xC0\xCC\xBA\xA5\xC6\xAE") == 0 || lstrcmpi(decode, "*EVENT_CHICHEN_DAY") == 0))
	{
		p = local_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_Chichen_Day = atoi(strBuff);
		else
			rsServerConfig.Event_Chichen_Day = 1;

		flag++;
	}

	if(!flag && (lstrcmp(decode, "\x2A\xB0\xED\xBD\xBA\xC6\xAE\x5F\xC0\xCC\xBA\xA5\xC6\xAE") == 0 ||
		lstrcmpi(decode, "*EVENT_GHOST") == 0))
	{

		p = local_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_Ghost = atoi(strBuff);
		else
			rsServerConfig.Event_Ghost = 2;

		flag++;
	}

	if(!flag && (lstrcmp(decode, "\x2A\xBB\xEA\xC5\xB8\xB0\xED\xBA\xED\xB8\xB0\x5F\xC0\xCC\xBA\xA5\xC6\xAE") == 0 ||
		lstrcmpi(decode, "*EVENT_SANTAGOBLIN") == 0))
	{

		p = local_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_SantaGoblin = atoi(strBuff);
		else
			rsServerConfig.Event_SantaGoblin = 5;

		flag++;
	}

	if(!flag && (lstrcmp(decode, "\x2A\xBB\xEF\xB0\xE8\xC5\xC1\x5F\xC0\xCC\xBA\xA5\xC6\xAE") == 0 ||
		lstrcmpi(decode, "*EVENT_SUMMER_CHICKEN") == 0))
	{

		p = local_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_SummerChicken = atoi(strBuff);
		else
			rsServerConfig.Event_SummerChicken = 5;

		flag++;
	}

	if(!flag && (lstrcmp(decode, "\x2A\xC7\xC7\xBD\xC3\xB9\xE6\xC6\xEA\x5F\xC0\xCC\xBA\xA5\xC6\xAE") == 0 ||
		lstrcmpi(decode, "*EVENT_PCBANG_PET") == 0))
	{

		p = local_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_PCBangPet = atoi(strBuff);
		else
			rsServerConfig.Event_PCBangPet = 1;

		flag++;
	}

	if(!flag && (lstrcmp(decode, "\x2A\xB0\xE6\xC7\xE8\xC4\xA1\xBE\xF7\x5F\xC0\xCC\xBA\xA5\xC6\xAE") == 0 ||
		lstrcmpi(decode, "*EVENT_EXPUP") == 0))
	{

		p = local_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.Event_ExpUp = atoi(strBuff);
		else
			rsServerConfig.Event_ExpUp = 120;



		//if (rsServerConfig.Event_ExpUp >= 100 && rsServerConfig.Event_ExpUp < 200)
		//{
		//	rsServerConfig.Event_ExpUp = 100;
		//}
		//else if (rsServerConfig.Event_ExpUp >= 200 && rsServerConfig.Event_ExpUp < 300)
		//{
		//	rsServerConfig.Event_ExpUp = 200;
		//}
		//else if (rsServerConfig.Event_ExpUp >= 300 && rsServerConfig.Event_ExpUp < 400)
		//{
		//	rsServerConfig.Event_ExpUp = 300;
		//}
		//else if (rsServerConfig.Event_ExpUp >= 400)
		//{
		//	rsServerConfig.Event_ExpUp = 400;
		//}
		//else
		//{
		//	rsServerConfig.Event_ExpUp = 100;
		//}

		flag++;
	}

#ifdef _LANGUAGE_ARGENTINA

	if(!flag && (lstrcmp(decode, "\x2A\xB0\xE6\xC7\xE8\xC4\xA1\xBE\xF7\x5F\xC0\xCC\xBA\xA5\xC6\xAE\x5F\xB6\xF3\xC6\xBE") == 0 || lstrcmpi(decode, "*EVENT_EXPUP_LATIN") == 0))
	{

		p = local_GetWord(strBuff, p);
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




	if(!flag && (lstrcmp(decode, "\x2A\xBC\xB1\xB9\xB0\xBB\xF3\xC0\xDA\x5F\xC0\xCC\xBA\xA5\xC6\xAE") == 0 ||
		lstrcmpi(decode, "*EVENT_HALLOWEEN") == 0))
	{
		p = local_GetWord(strBuff, p);
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



	if(!flag && (lstrcmp(decode, "\x2A\xBD\xC3\xB0\xA3\xB0\xE6\xC7\xE8\xC4\xA1") == 0 || lstrcmpi(decode, "*EXP_GAMETIME") == 0))
	{
		p = local_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.ExpGameTimeMode = atoi(strBuff);
		else
			rsServerConfig.ExpGameTimeMode = 1;

		flag++;
	}


	if(!flag && (lstrcmp(decode, "\x2A\xBA\xED\xB7\xB9\xBD\xBA\xC4\xB3\xBD\xBD") == 0 ||
		lstrcmpi(decode, "*EVENT_BLESSCASTLE") == 0))
	{

		p = local_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.BlessCastleMode = atoi(strBuff);
		else
			rsServerConfig.BlessCastleMode = 1;

		flag++;
	}
	if(!flag && (lstrcmp(decode, "\x2A\xB0\xF8\xBC\xBA\xBD\xC3\xB0\xA3") == 0 ||
		lstrcmpi(decode, "*EVENT_SIEGE_TIME") == 0))
	{

		p = local_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.sBlessCastle_StartTime[0] = atoi(strBuff);
		p = local_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.sBlessCastle_StartTime[1] = atoi(strBuff);

		flag++;
	}

	if(!flag && (lstrcmp(decode, "\x2A\xC0\xCC\xBA\xA5\xC6\xAE\xBA\xF1\xC6\xAE\xBC\xB3\xC1\xA4") == 0 ||
		lstrcmpi(decode, "*EVENT_BITMASK") == 0))
	{

		p = local_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.EventPlay_BitMask = atoi(strBuff);

		flag++;
	}

	if(!flag && (lstrcmp(decode, "\x2A\xBD\xC5\xB1\xD4\xC0\xAF\xC0\xFA\xBC\xB1\xB9\xB0") == 0 ||
		lstrcmpi(decode, "*DEFAULT_ITEM_USER") == 0))
	{

		p = local_GetWord(strBuff, p);
		if(strBuff[0])
			rsServerConfig.StartUser_PresentItem = atoi(strBuff);
		else
			rsServerConfig.StartUser_PresentItem = TRUE;

		flag++;
	}


	if(!flag && (lstrcmp(decode, "\x2A\xC0\xFC\xBE\xF7\xC0\xDA\xB1\xE2\xB7\xCF") == 0 ||
		lstrcmpi(decode, "*RECORD_JOBCHANGER") == 0))
	{
		rsServerConfig.Record_JobChanger = TRUE;;
		flag++;
	}

	if(!flag && (lstrcmp(decode, "\x2A\xBF\xB5\xB1\xB8\xBA\xED\xB7\xB0\xC7\xE3\xB0\xA1") == 0 ||
		lstrcmpi(decode, "*ENABLE_BLOCK_FOREVER") == 0))
	{
		rsServerConfig.ForeverBlockMode = TRUE;
		flag++;
	}

	if(!flag && (lstrcmp(decode, "\x2A\xB0\xE6\xC7\xE8\xC4\xA1\xBF\xC0\xC2\xF7\xB9\xFC\xC0\xA7") == 0 ||
		lstrcmpi(decode, "*PERMIT_EXP") == 0))
	{
		pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		rsServerConfig.Permit_Exp = atoi(strBuff);
		flag++;
	}
	if(!flag && (lstrcmp(decode, "\x2A\xB5\xB7\xBF\xC0\xC2\xF7\xB9\xFC\xC0\xA7") == 0 ||
		lstrcmpi(decode, "*PERMIT_MONEY") == 0))
	{
		pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		rsServerConfig.Permit_Money = atoi(strBuff);
		flag++;
	}

	if(!flag && (lstrcmp(decode, "\x2A\xC7\xC1\xB7\xCE\xBC\xBC\xBD\xBA\xBD\xC3\xB0\xA3\xB0\xA8\xBD\xC3") == 0 ||
		lstrcmpi(decode, "*CLIENT_CHECK_PROCESS_TIME") == 0))
	{
		pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		rsServerConfig.ClientProcessTimeChk = atoi(strBuff);
		flag++;
	}
	if(!flag && (lstrcmp(decode, "\x2A\xC7\xC1\xB7\xCE\xBC\xBC\xBD\xBA\xBD\xC3\xB0\xA3\xC1\xBE\xB7\xE1") == 0 ||
		lstrcmpi(decode, "*CLIENT_PROCESS_TIME_OUT") == 0))
	{
		rsServerConfig.ClientProcessTimeChkOut = TRUE;
		flag++;
	}
	if(!flag && (lstrcmp(decode, "\x2A\xB9\xB0\xBE\xE0\xB0\xB9\xBC\xF6\xC8\xAE\xC0\xCE") == 0 ||
		lstrcmpi(decode, "*CHECK_POTION_COUNT") == 0))
	{
		pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		rsServerConfig.Dis_PotionError = atoi(strBuff);
		flag++;
	}
	if(!flag && (lstrcmp(decode, "\x2A\xBC\xAD\xB9\xF6\xB0\xE6\xC7\xE8\xC4\xA1\xBB\xE7\xBF\xEB") == 0 ||
		lstrcmpi(decode, "*ENABLE_SERVER_EXP") == 0))
	{
		pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);

		if(strBuff[0]) rsServerConfig.ExpFixMode = atoi(strBuff);
		else rsServerConfig.ExpFixMode = 50;

		flag++;
	}

	if(!flag && (lstrcmp(decode, "\x2A\xBA\xED\xB7\xB0\xC0\xAF\xC0\xFA\xB0\xE6\xB7\xCE") == 0 ||
		lstrcmpi(decode, "*BLOCK_USER_PATH") == 0))
	{
		pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		strcpy_s(rsServerConfig.szBlockUserPath, strBuff);
		flag++;
	}

	if(!flag && (lstrcmp(decode, "\x2A\xBB\xE7\xBF\xEB\xBD\xC3\xB0\xA3\xC7\xA5\xBD\xC3") == 0 ||
		lstrcmpi(decode, "*DISP_BILLING_TIME") == 0))
	{

		pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		if(strBuff[0])
			rsServerConfig.DispBillingTime = atoi(strBuff);
		else
			rsServerConfig.DispBillingTime = 1;

		flag++;
	}

	//if(!flag && (lstrcmp(decode, "*로그인금지") == 0 ||
	//	lstrcmpi(decode, "*DISABLE_LOGIN") == 0))
	//{

	//	pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
	//	if(strBuff[0])
	//		rsServerConfig.Disable_LogIn = atoi(strBuff);
	//	else
	//		rsServerConfig.Disable_LogIn = 1;
	//	flag++;
	//}

	if(!flag &&	lstrcmpi(decode, "*ALLOWED_LOGIN") == 0)
	{
		pb = p; 
		p = local_GetWord(strBuff, p);

		if(strBuff[0] == 34)
			p = GetString(strBuff, pb);

		if(strBuff[0])
			rsServerConfig.Disable_LogIn = atoi(strBuff) ? 0 : 1;
		else
			rsServerConfig.Disable_LogIn = 1;

		flag++;
	}

	if(!flag && (lstrcmp(decode, "\x2A\xB7\xCE\xB1\xD7\xC5\xA5\xB1\xDD\xC1\xF6") == 0 ||
		lstrcmpi(decode, "*DISABLE_LOGQUE") == 0))
	{
		rsServerConfig.Disable_LogQue = TRUE;
		flag++;
	}
	if(!flag && (lstrcmp(decode, "\x2A\xB7\xCE\xB1\xD7\xBE\xC6\xBF\xF4\xC8\xAE\xC0\xCE\xB1\xDD\xC1\xF6") == 0 ||
		lstrcmpi(decode, "*DISABLE_CHECK_LOGOUT") == 0))
	{
		rsServerConfig.Disable_CheckLogOut = TRUE;
		flag++;
	}

	//if(!flag && (lstrcmp(decode, "*로그인차단") == 0 ||
	//	lstrcmpi(decode, "*DISABLE_LOGINX") == 0))
	//{
	//	rsServerConfig.Disable_LogIn2 = TRUE;
	//	flag++;
	//}


	if(!flag && (lstrcmp(decode, "\x2A\xB9\xAB\xB7\xE1\xB7\xB9\xBA\xA7") == 0 ||
		lstrcmpi(decode, "*FREE_LEVEL") == 0))
	{
		pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		rsServerConfig.FreeLevel = atoi(strBuff);

		pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		if(strBuff[0])
			strcpy_s(rsServerConfig.szFreeGpCode, strBuff);

		flag++;
	}

	if(!flag && (lstrcmp(decode, "\x2A\xBC\xB1\xB9\xB0\xB7\xB9\xBA\xA7") == 0 ||
		lstrcmpi(decode, "*PRESENT_LEVEL") == 0))
	{
		pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		rsServerConfig.PresentLevel = atoi(strBuff);
		flag++;
	}

	if(!flag && (lstrcmp(decode, "\x2A\xB5\xBF\xB7\xE1\xBE\xC6\xC0\xCC\xC5\xDB\xC3\xE2\xB7\xC2") == 0 ||
		lstrcmpi(decode, "*ENABLE_PARTYITEM") == 0))
	{

		rsServerConfig.Enable_PartyItem = TRUE;
		flag++;
	}

	if(!flag && (lstrcmp(decode, "\x2A\xC7\xCA\xB5\xE5\xC1\xF6\xC7\xFC\xB0\xCB\xBB\xE7") == 0 ||
		lstrcmpi(decode, "*CHECK_PLAYFIELD") == 0))
	{

		rsServerConfig.CheckPlayField = TRUE;
		flag++;
	}


	if(!flag && (lstrcmp(decode, "\x2A\xB8\xF0\xB5\xA8\xC6\xC4\xC0\xCF\xB0\xCB\xBB\xE7") == 0 ||
		lstrcmpi(decode, "*CHECK_MODELFILE") == 0))
	{

		rsServerConfig.CheckModelFile = TRUE;
		flag++;
	}


	if(!flag && (lstrcmp(decode, "\x2A\xC1\xA2\xBC\xD3\xC1\xA6\xC7\xD1\x49\x50") == 0 ||
		lstrcmpi(decode, "*DISABLE_IP") == 0))
	{

		if(rsServerConfig.DisableIP_Count < DISABLE_IP_MAX)
		{
			p = local_GetWord(strBuff, p);
			rsServerConfig.DisableIP[rsServerConfig.DisableIP_Count][0] = GetSwapIPCode(strBuff);
			p = local_GetWord(strBuff, p);
			rsServerConfig.DisableIP[rsServerConfig.DisableIP_Count][1] = GetSwapIPCode(strBuff);
			rsServerConfig.DisableIP_Count++;
		}

		flag++;
	}

	if(!flag && (lstrcmp(decode, "\x2A\xC1\xA2\xBC\xD3\xC7\xE3\xB0\xA1\x49\x50") == 0 ||
		lstrcmpi(decode, "*ENABLE_IP") == 0))
	{

		if(rsServerConfig.EnableIP_Count < ENABLE_IP_MAX)
		{
			p = local_GetWord(strBuff, p);
			rsServerConfig.EnableIP[rsServerConfig.EnableIP_Count][0] = GetSwapIPCode(strBuff);
			p = local_GetWord(strBuff, p);
			rsServerConfig.EnableIP[rsServerConfig.EnableIP_Count][1] = GetSwapIPCode(strBuff);
			rsServerConfig.EnableIP_Count++;
		}

		flag++;
	}
	 
	//AQUI TESTE
	/*if(!flag && (lstrcmp(decode, "*관리접속IP") == 0 ||
		lstrcmpi(decode, "*ADMINISTRATOR_IP") == 0))
	{

		if(rsServerConfig.AdminIP_Count < ADMIN_IP_MAX)
		{
			p = local_GetWord(strBuff, p);
			rsServerConfig.AdminIP[rsServerConfig.AdminIP_Count][0] = GetSwapIPCode(strBuff);
			p = local_GetWord(strBuff, p);
			rsServerConfig.AdminIP[rsServerConfig.AdminIP_Count][1] = GetSwapIPCode(strBuff);
			rsServerConfig.AdminIP_Count++;
		}

		flag++;
	}*/

	if(!flag && (lstrcmp(decode, "\x2A\xBC\xAD\xB9\xF6\xC0\xCE\xBA\xA5\xC5\xE4\xB8\xAE") == 0 ||
		lstrcmpi(decode, "*SERVER_INVENTORY") == 0))
	{

		rsServerConfig.ServerInventoryMode = TRUE;
		flag++;
	}

	if(!flag && (lstrcmp(decode, "\x2A\xB9\xE9\xBE\xF7\xC6\xFA\xB4\xF5\xC0\xCC\xB8\xA7") == 0 ||
		lstrcmpi(decode, "*BACKUP_PATH") == 0))
	{
		pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		strcpy_s(rsServerConfig.szBackupPath, strBuff);
		flag++;
	}

	if(!flag && (lstrcmp(decode, "\x2A\xBD\xBA\xC5\xB3\xC6\xF7\xC0\xCE\xC6\xAE\xB0\xA8\xBC\xD2\xBA\xD2\xB0\xA1") == 0 ||
		lstrcmpi(decode, "*DISABLE_DEC_SKILLPOINT") == 0))
	{

		rsServerConfig.Disable_DecSkillPoint = TRUE;
		flag++;
	}

	if(!flag && (lstrcmp(decode, "\x2A\xC4\xF9\xBD\xBA\xC6\xAE\xB1\xDD\xC1\xF6") == 0 ||
		lstrcmpi(decode, "*DISABLE_QUEST") == 0))
	{

		pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		rsServerConfig.Disable_Quest = atoi(strBuff);
		flag++;
	}

	if(!flag && (lstrcmp(decode, "\x2A\xBD\xC3\xC0\xDB\xC7\xCA\xB5\xE5\xB0\xED\xC1\xA4") == 0 ||
		lstrcmpi(decode, "*START_FIELD") == 0))
	{

		pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		if(strBuff[0])
			rsServerConfig.FixedStartField = atoi(strBuff);
		flag++;
	}

	if(!flag && (lstrcmp(decode, "\x2A\xBE\xC6\xC0\xCC\xC5\xDB\xB9\xF6\xB8\xB2\xB1\xDD\xC1\xF6") == 0 ||
		lstrcmpi(decode, "*DISABLE_THROWITEM") == 0))
	{

		pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		if(strBuff[0])
			rsServerConfig.DisableThrowItem = atoi(strBuff);
		else
			rsServerConfig.DisableThrowItem = 1;

		flag++;
	}



	if(!flag && (lstrcmp(decode, "\x2A\xC5\xA9\xB7\xB9\xC0\xCC\xC1\xF6\xC6\xD0\xC5\xB6") == 0 ||
		lstrcmpi(decode, "*CRAZY_PACKET") == 0))
	{

		rsServerConfig.CrazyPacketMode = TRUE;
		flag++;
	}

	if(!flag && (lstrcmp(decode, "\x2A\xC5\xEB\xBA\xB8\xC1\xD6\xBC\xD2") == 0 ||
		lstrcmpi(decode, "*NOTICE_URL") == 0))
	{
		pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		strcpy_s(rsServerConfig.szNoticeURL, strBuff);
		flag++;
	}

	if(!flag && (lstrcmp(decode, "\x2A\xB0\xFC\xB8\xAE\xB8\xED\xB7\xC9\xB1\xE2\xB7\xCF") == 0 ||
		lstrcmpi(decode, "*REC_ADMIN_CMD") == 0))
	{
		pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		if(strBuff[0])
			rsServerConfig.RecordAdminCommand = atoi(strBuff);
		else
			rsServerConfig.RecordAdminCommand = 2;

		flag++;
	}




	if(!flag && (lstrcmp(decode, "\x2A\xC1\xF6\xBF\xAA\xBC\xAD\xB9\xF6") == 0 ||
		lstrcmpi(decode, "*AREA_SERVER") == 0))
	{

		for(cnt = 0; cnt < 32; cnt++)
		{
			if(rsServerConfig.AreaServer.IP_Count >= SERVER_MAP_MAX) break;

			pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
			if(!strBuff[0]) break;

			rsServerConfig.AreaServer.dwIP[rsServerConfig.AreaServer.IP_Count] = inet_addr(strBuff);

			rsServerConfig.AreaServer.IP_Count++;
			rsServerConfig.AreaServer.size = sizeof(TRANS_SEVER_MAP) - (sizeof(DWORD)*(SERVER_MAP_MAX - rsServerConfig.AreaServer.IP_Count));
			rsServerConfig.AreaServer.code = OpCode::OPCODE_AREA_SERVER_MAP;
		}
		flag++;
	}

	if(!flag && (lstrcmp(decode, "\x2A\xC5\xE4\xB3\xCA\xB8\xD5\xC6\xAE\xB0\xE8\xC1\xA4\xBC\xAD\xB9\xF6") == 0 ||
		lstrcmpi(decode, "*TT_DATA_SERVER") == 0))
	{

		cnt = rsServerConfig.TT_DataServer_Count;

		if(cnt >= TT_DATA_SERVER_MAX) goto skip_t;

		pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		if(!strBuff[0]) goto skip_t;
		strcpy_s(rsServerConfig.TT_DataServer[cnt].szServerID, strBuff);

		pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		if(!strBuff[0]) goto skip_t;
		strcpy_s(rsServerConfig.TT_DataServer[cnt].szServerPath, strBuff);

		rsServerConfig.TT_DataServer_Count++;
	skip_t:
		flag++;
	}


	return flag;
}


int SetServerSelectList(char *decode, char *lpPoint)
{
	int flag;
	char *p = lpPoint;
	char *pb;
	char strBuff[128];
	int cnt;

	flag = 0;

	if(TransServerList.ServerCount >= TRANS_SERVER_MAX) return FALSE;

	if(lstrcmp(decode, "*접속서버정보") == 0 ||
		lstrcmpi(decode, "*GAME_SERVER") == 0)
	{
		cnt = TransServerList.ServerCount;

		pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		if(strBuff[0])
		{
			strcpy_s(TransServerList.ServerInfo[cnt].szServerName, strBuff);
		}

		pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		if(strBuff[0])
		{
			strcpy_s(TransServerList.ServerInfo[cnt].szServerIp1, strBuff);
			TransServerList.ServerInfo[cnt].dwServerPort1 = SERVER_TCP_PORT;
		}

		pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		if(strBuff[0])
		{
			strcpy_s(TransServerList.ServerInfo[cnt].szServerIp2, strBuff);
			TransServerList.ServerInfo[cnt].dwServerPort2 = SERVER_TCP_PORT;
		}

		pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		if(strBuff[0])
		{
			strcpy_s(TransServerList.ServerInfo[cnt].szServerIp3, strBuff);
			TransServerList.ServerInfo[cnt].dwServerPort3 = SERVER_TCP_PORT;
		}

		TransServerList.ServerCount++;
		flag++;
	}

	if(!flag && (lstrcmp(decode, "*클랜서버정보") == 0 ||
		lstrcmpi(decode, "*CLAN_SERVER") == 0))
	{

		cnt = TransServerList.ServerCount + TransServerList.ClanServerCount;

		strcpy_s(TransServerList.ServerInfo[cnt].szServerName, "Clan");

		pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		if(strBuff[0])
			strcpy_s(TransServerList.ServerInfo[cnt].szServerIp1, strBuff);

		pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		if(strBuff[0])
			TransServerList.ServerInfo[cnt].dwServerPort1 = atoi(strBuff);

		pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		if(strBuff[0])
			strcpy_s(TransServerList.ServerInfo[cnt].szServerIp2, strBuff);

		pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		if(strBuff[0])
			TransServerList.ServerInfo[cnt].dwServerPort2 = atoi(strBuff);

		pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		if(strBuff[0])
			strcpy_s(TransServerList.ServerInfo[cnt].szServerIp3, strBuff);

		pb = p; p = local_GetWord(strBuff, p); if(strBuff[0] == 34)p = GetString(strBuff, pb);
		if(strBuff[0])
			TransServerList.ServerInfo[cnt].dwServerPort3 = atoi(strBuff);

		TransServerList.ClanServerCount++;
		flag++;
	}


	return flag;
}



