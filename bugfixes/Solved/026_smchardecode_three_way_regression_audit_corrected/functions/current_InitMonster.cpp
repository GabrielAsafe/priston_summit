int InitMonster();
int	OpenBossTimeMin = 1;

int rsRefreshConfig()
{

#ifdef _W_SERVER

	EnterCriticalSection(&cSerSection);

	bool readWarline();
	readWarline();
	
	if (!smConfigDecode("Configs//hotuk.ini"))
	{
		Sleep(5000);
		quit = 1;

		return FALSE;
	}

	StartUser_PresentItem_Init();

	if (AgingEvento) 
	{
		cout << "Aging Free ON" << endl;
	}
	else 
	{
		cout << "Aging Free OFF" << endl;
	}
	if (bMaintenanceMode) 
	{
		cout << "Modo Manutencao Ativo!" << endl;
	}

	AgingEventoGlobalOn();

	OpenBossTimeMin = boss;

	if(rsServerConfig.LimitVersion)
	{
		Server_LimitVersion = rsServerConfig.LimitVersion;
	}

	if(rsServerConfig.szServerName[0] && lstrlen(rsServerConfig.szServerName) < 16)
	{
		strcpy_s(TransServerList.szServerName, rsServerConfig.szServerName);
	}

	if(rsServerConfig.Permit_Exp)
		Permit_CheckExp = rsServerConfig.Permit_Exp;
	else
		Permit_CheckExp = TOTAL_CHECK_EXP_MAX;

	if(rsServerConfig.Permit_Money)
		Permit_CheckMoney = rsServerConfig.Permit_Money;
	else
		Permit_CheckMoney = TOTAL_CHECK_MONEY_MAX;

	rsServerConfig.ServerInventoryMode = TRUE;
	rsServerConfig.UseRecorMemory = TRUE;
	rsServerConfig.PotionMonitor = TRUE;


	if(rsServerConfig.ConnectUserMax > 0)
		rsConnectUserLimit = rsServerConfig.ConnectUserMax;

	rsResetBackList();

	rsResetClientFuncPos();

	InitCrackFinding();
	
    cCoinShop.LoadItems();
	cCoinShop_T.LoadItems();

	LeaveCriticalSection(&cSerSection);

	rsInit_SerToServer();
	
#endif

	return TRUE;
}
