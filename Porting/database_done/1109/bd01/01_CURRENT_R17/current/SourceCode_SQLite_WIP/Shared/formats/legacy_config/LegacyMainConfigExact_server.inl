smCONFIG *smConfigDecode(char *iniFile)
{
	smCONFIG *Config = &smConfig;

	srAdminCmdCnt = 0;

	ZeroMemory(Config, sizeof(smCONFIG));
	ZeroMemory(&TransServerList, sizeof(TRANS_SERVER_LIST));
	ZeroMemory(&rsServerConfig, sizeof(rsSERVER_CONFIG));

	Config->StageObjCnt = 0;
	Config->MapBright = DEFAULT_BRIGHT;
	Config->MapContrast = DEFAULT_CONTRAST;
	Config->MapLightVector.x = fONE;
	Config->MapLightVector.y = -fONE;
	Config->MapLightVector.z = fONE / 2;

	rsServerConfig.Enable_PKField = -1;
	rsServerConfig.Enable_PKField1 = -1;
	rsServerConfig.Enable_PKField2 = -1;
	rsServerConfig.Enable_PKField3 = -1;
	rsServerConfig.Enable_PKField_All = 0;

	std::ifstream filein(iniFile, std::ifstream::binary);

	if (filein.fail())
	{
		cout << "Failed to open [" << iniFile << "]" << endl;
		return nullptr;
	}

	for(std::string line; std::getline(filein, line);)
	{
		char *p = local_GetWord(decode, (char*)line.c_str());

#ifdef _W_SERVER
		if(SetServerSelectList(decode, p)) continue;
		if(SetServerConfig(decode, p)) continue;
#endif
	}

	filein.close();

	smMATRIX rX, rY, mVect;
	int vx, vy, vz;

	int MapLightAngY = 128 * 16;
	int MapLightAngX = (128 - 10) * 16;
	MapLightAngX = (MapLightAngX - ANGLE_90) & ANGCLIP;

	int x = 0;
	int y = 0;
	int z = (int)(1.5*fONE);

	smIdentityMatrix(rX);
	smIdentityMatrix(rY);

	smRotateYMatrix(rY, MapLightAngY);
	smRotateXMatrix(rX, MapLightAngX);
	smMatrixMult(mVect, rX, rY);

	vx = x * mVect._11 +
		y * mVect._21 +
		z * mVect._31;

	vy = x * mVect._12 +
		y * mVect._22 +
		z * mVect._32;

	vz = x * mVect._13 +
		y * mVect._23 +
		z * mVect._33;

	vx >>= FLOATNS;
	vy >>= FLOATNS;
	vz >>= FLOATNS;

	Config->MapLightVector.x = vx;
	Config->MapLightVector.y = vy;
	Config->MapLightVector.z = vz;

	Config->MapBright = 130;
	Config->MapContrast = 300;

	return &smConfig;
};


char *GetDirectoryFromFile(char *filename, char *szDirectory, size_t size)
{
	int len;
	int cnt;

	if(!szDirectory)
	{
		return NULL;
	}

	strcpy_s(szDirectory, size, filename);

	len = lstrlen(szDirectory);

	for(cnt = len; cnt >= 0; cnt--)
		if(szDirectory[cnt] == '/' || szDirectory[cnt] == '\\') break;

	szDirectory[cnt + 1] = NULL;

	return szDirectory;
}




