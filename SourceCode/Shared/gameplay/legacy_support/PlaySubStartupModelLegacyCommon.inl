int	PlaySub_StartFunc()
{
	return FALSE;
}


int Code_VRamBuffOpen()
{

	lpCodeVram = new DWORD[256];

	return (DWORD)lpCodeVram;

}

int Code_VramClear()
{
	lpCodeVram[0] = 777;
	lpCodeVram[1] = 0;
	lpCodeVram[2] = 777;
	lpCodeVram[3] = 0;

	return TRUE;
}

int Code_VramSave(DWORD Param1, DWORD Param2, DWORD Param3)
{
	DWORD	dwTime;

	if (lpCodeVram)
	{
		dwTime = GetCurrentTime();

		lpCodeVram[3] = dwTime;
		lpCodeVram[0] = Param1 ^ dwTime;
		lpCodeVram[1] = Param2 ^ dwTime;
		lpCodeVram[2] = Param3 ^ dwTime;
	}

	return TRUE;
}

int Code_VramLoad(DWORD *Param1, DWORD *Param2, DWORD *Param3)
{
	*Param1 = lpCodeVram[0] ^ lpCodeVram[3];
	*Param2 = lpCodeVram[1] ^ lpCodeVram[3];
	*Param3 = lpCodeVram[2] ^ lpCodeVram[3];

	return TRUE;
}

int	Mix_CodeVram()
{
	DWORD	dwTime;
	DWORD	dwNewTime;


	dwTime = (dwPlayTime >> 4) & 0xFF;
	dwNewTime = (dwTime << 24) | (dwTime << 16) | (dwTime << 8) | dwTime;

	dwTime = lpCodeVram[3];

	lpCodeVram[0] = (lpCodeVram[0] ^ dwTime) ^ dwNewTime;
	lpCodeVram[1] = (lpCodeVram[1] ^ dwTime) ^ dwNewTime;
	lpCodeVram[2] = (lpCodeVram[2] ^ dwTime) ^ dwNewTime;
	lpCodeVram[3] = dwNewTime;

	return TRUE;
}





DWORD Check_CodeSafe(DWORD dwStartFunc)
{
	BYTE	*lpFunc;
	DWORD	dwCode;
	DWORD	*lpdwFunc;
	BYTE	JmpCode = 9;
	DWORD	dwHeaderPos = 0x30;











	dwCode = 1697348954;

	dwFuncChkTime = GetCurrentTime();
	DWORD FnCode = dwCode ^ ((dwFuncChkTime << 16) | (dwFuncChkTime & 0xFFFF));

	dwFuncSafeCode = FnCode;
	dwFuncSafeMemCount = 0;
	dwFuncSafeMemCode = 0;

	Code_VramSave(FnCode, dwFuncChkTime, dwCode);

	lpFunc = 0;
	lpdwFunc = 0;

	return dwCode;
}


DWORD Check_CodeSafe2()
{

	DWORD	*lpdwFunc;
	DWORD	dwCode;
	int cnt;

	lpdwFunc = (DWORD *)(dwStartFuncPoint ^ (dwFuncLen << 8));

	if (dwFuncLen > 200000)
		return FALSE;

	for (cnt = 0; cnt < 32; cnt++)
	{


		dwFuncSafeMemCode = dwFuncSafeMemCode + (lpdwFunc[dwFuncSafeMemCount++] * ((dwFuncSafeMemCount & 0x3) + 1));

		if (dwFuncSafeMemCount >= dwFuncLen)
		{
			dwFuncChkTime = GetCurrentTime();
			dwCode = dwFuncSafeMemCode ^ ((dwFuncChkTime << 16) | (dwFuncChkTime & 0xFFFF));
			dwFuncSafeCode = dwCode;

			Code_VramSave(dwCode, dwFuncChkTime, dwFuncSafeMemCode);



			dwFuncSafeMemCount = 0;
			dwFuncSafeMemCode = 0;
			lpdwFunc = 0;

			return TRUE;
		}
	}

	lpdwFunc = 0;
	return FALSE;
}


DWORD	GetSafeCode()
{
	return dwFuncSafeCode ^ ((dwFuncChkTime << 16) | (dwFuncChkTime & 0xFFFF));
}


DWORD	GetSafeCode2()
{
	DWORD	dwCode, dwFnChkTime, dwFnSafeCode;
	DWORD	dwOriginal;

	Code_VramLoad(&dwCode, &dwFnChkTime, &dwFnSafeCode);

	dwOriginal = dwCode ^ ((dwFnChkTime << 16) | (dwFnChkTime & 0xFFFF));

	if ((dwOriginal^dwFnSafeCode) == 0) return dwOriginal;
	return FALSE;
}


int	LoadOftenMeshPattern(char *szFileName)
{
	DynTempPat[DynTempPatCnt] = smPattern.LoadCharactor(szFileName);
	if (DynTempPat[DynTempPatCnt])
	{
		DynTempPat[DynTempPatCnt]->UseCount = 99;
		DynTempPatCnt++;
		return TRUE;
	}

	return FALSE;
}

#ifdef _CHECK_MESH

static DWORD dwOftPatSum = 0;

int	LoadOftenMeshPatternCheckSum(char *szFileName)
{
	smPAT3D *lpPat3D;

	DynTempPat[DynTempPatCnt] = smPattern.LoadCharactor(szFileName);
	if (DynTempPat[DynTempPatCnt])
	{
		lpPat3D = DynTempPat[DynTempPatCnt]->Pat;
		int szX = 0; int szN = 0;
		for (int cnt = 0; cnt < lpPat3D->nObj3d; cnt++)
		{
			if (szX < lpPat3D->obj3d[cnt]->maxY)
				szX = lpPat3D->obj3d[cnt]->maxY;
			if (szN < lpPat3D->obj3d[cnt]->minY)
				szN = lpPat3D->obj3d[cnt]->minY;
		}
		DynTempPat[DynTempPatCnt]->UseCount = 99;
		DynTempPatCnt++;

		DWORD dwSpdSum = GetSpeedSum(szFileName);
		DWORD dwSizSum = (szX << 4) | (szN << 1);
		dwOftPatSum += dwSpdSum + (dwSpdSum * dwSizSum) + dwSizSum;
		return TRUE;
	}

	return FALSE;
}


int Send_OftPatSumToDataServer()
{

	smTRANS_COMMAND	smTransCommand;
	smTransCommand.code = OpCode::OPCODE_CHECK_ITEMMESH;
	smTransCommand.size = sizeof(smTRANS_COMMAND);
	smTransCommand.WParam = dwOftPatSum;
	smTransCommand.SParam = 0;
	smTransCommand.LParam = 0;
	smTransCommand.EParam = 0;

	dwOftPatSum = 0;

	if (LegacyPlaySubDataChannelAvailable())
		return LegacyPlaySubSendDataPacket(&smTransCommand, smTransCommand.size, smTransCommand.code);

	return FALSE;
}
#endif

static int LoadDataDrivenClassArmorModels()
{
	int errorCount = 0;
	const std::vector<const gameplay::classes::PlayableClassDefinition*> enabled =
		gameplay::classes::DefaultClassService().EnabledClasses();

	for(std::vector<const gameplay::classes::PlayableClassDefinition*>::const_iterator definition = enabled.begin();
		definition != enabled.end(); ++definition)
	{
		if(!(*definition)->armor.HasDataDrivenModels())
			continue;

		const std::vector<std::string> *groups[] = {
			&(*definition)->armor.normalModels,
			&(*definition)->armor.comicModels,
			&(*definition)->armor.newSetModels,
			&(*definition)->armor.newSet2Models
		};
		for(std::size_t group = 0; group < sizeof(groups) / sizeof(groups[0]); ++group)
		{
			for(std::vector<std::string>::const_iterator model = groups[group]->begin();
				model != groups[group]->end(); ++model)
			{
				if(model->empty())
					continue;
				std::string path = (*definition)->armor.modelRoot;
				path += *model;
				if(LoadOftenMeshPattern(const_cast<char *>(path.c_str())) == FALSE)
					errorCount++;
			}
		}
	}
	return errorCount;
}

static int LoadDataDrivenClassModels(int includeBodies, int includeHeads)
{
	int errorCount = 0;
	const std::vector<const gameplay::classes::PlayableClassDefinition*> enabled =
		gameplay::classes::DefaultClassService().EnabledClasses();

	for(std::vector<const gameplay::classes::PlayableClassDefinition*>::const_iterator definition = enabled.begin();
		definition != enabled.end(); ++definition)
	{
		// The eight historical classes are still preloaded from their canonical
		// legacy matrices. New classes can provide bodies/heads declaratively.
		if((*definition)->appearance.HasDataDrivenModels())
		{
			if(includeBodies)
			{
				for(std::vector<std::string>::const_iterator model = (*definition)->creation.bodyModels.begin();
					model != (*definition)->creation.bodyModels.end(); ++model)
				{
					if(!model->empty() && LoadOftenMeshPattern(const_cast<char *>(model->c_str())) == FALSE)
						errorCount++;
				}
			}

			if(includeHeads)
			{
				for(std::vector<std::vector<std::string> >::const_iterator row = (*definition)->appearance.headModels.begin();
					row != (*definition)->appearance.headModels.end(); ++row)
				{
					for(std::vector<std::string>::const_iterator model = row->begin(); model != row->end(); ++model)
					{
						if(!model->empty() && LoadOftenMeshPattern(const_cast<char *>(model->c_str())) == FALSE)
							errorCount++;
					}
				}
			}
		}
	}

	if(includeBodies)
		errorCount += LoadDataDrivenClassArmorModels();
	return errorCount;
}

int	LoadOftenFullPattern()
{
	int cnt, cnt2;
	int	ErrorCount;
	char	szBuff[128];

	ErrorCount = 0;

	// Legacy armor matrices are a compatibility backend, not the source of
	// truth for how many classes exist. Only rows referenced by enabled class
	// definitions are preloaded. New classes can use data-driven armor models.
	ErrorCount += LoadLegacyClassArmorModelSet(
		gameplay::classes::ArmorVisualVariant::Normal, 0, ARMOR_FILE_MAX, FALSE);
	ErrorCount += LoadLegacyClassArmorModelSet(
		gameplay::classes::ArmorVisualVariant::Comic, 0, ARMOR_FILE_MAX, FALSE);


	for (cnt = 0; cnt < FACE_MODEL_MAX; cnt++)
	{
		for (cnt2 = 0; cnt2 < HEAREPOTION_MAX; cnt2++)
		{
			if (LoadOftenMeshPattern(szModel_FighterFaceName[cnt][cnt2]) == FALSE)
				ErrorCount++;
			if (LoadOftenMeshPattern(szModel_MechFaceName[cnt][cnt2]) == FALSE)
				ErrorCount++;
			if (LoadOftenMeshPattern(szModel_ArcherFaceName[cnt][cnt2]) == FALSE)
				ErrorCount++;
			if (LoadOftenMeshPattern(szModel_PikeFaceName[cnt][cnt2]) == FALSE)
				ErrorCount++;
			if (LoadOftenMeshPattern(szModel_KnightFaceName[cnt][cnt2]) == FALSE)
				ErrorCount++;
			if (LoadOftenMeshPattern(szModel_AtalantaFaceName[cnt][cnt2]) == FALSE)
				ErrorCount++;
			if (LoadOftenMeshPattern(szModel_PristessFaceName[cnt][cnt2]) == FALSE)
				ErrorCount++;
			if (LoadOftenMeshPattern(szModel_MagicanFaceName[cnt][cnt2]) == FALSE)
				ErrorCount++;

		}
	}

	ErrorCount += LoadDataDrivenClassModels(TRUE, TRUE);





	HANDLE hFindHandle;
	WIN32_FIND_DATA	fd;

	char	szFindPath[64];
	char	szFilePath[64];

	strcpy_s(szFindPath, szNpcDirectory);
	lstrcat(szFindPath, "*.ini");

	hFindHandle = FindFirstFile(szFindPath, &fd);
	if (hFindHandle != INVALID_HANDLE_VALUE)
	{
		while (1)
		{
			strcpy_s(szFilePath, szNpcDirectory);
			lstrcat(szFilePath, fd.cFileName);

			LoadOftenMeshPattern(szFilePath);


			if (FindNextFile(hFindHandle, &fd) == FALSE) break;
		}
	}
	FindClose(hFindHandle);


	return ErrorCount;
}


int LoadOftenModels()
{

	return TRUE;
}

char *szOftenNPC[] = {

	0
};


int LoadFolder_Ini(char *szPath, char *szWildCard, char *szChgExt, int OftSave);

int Make_CharAll_SMD()
{
	LoadOftenFullPattern();
	LoadFolder_Ini("Char//Npc", "*.ini", 0, 0);
	LoadFolder_Ini("Char//Monster", "*.ini", 0, 0);

	return TRUE;
}


int LoadOftenNpc()
{
#ifdef _W_SERVER
	LoadFolder_Ini("Char//Npc", "*.inx", "ini", TRUE);
#else
	int cnt = 0;
	while (1)
	{
		if (!szOftenNPC[cnt])
			break;
		LoadOftenMeshPattern(szOftenNPC[cnt]);
		cnt++;
	}
#endif

	return TRUE;
}


int LoadOftenFace()
{

	int cnt, cnt2;
	int ErrorCount = 0;

	for (cnt = 0; cnt < 6; cnt++)
	{

		for (cnt2 = 0; cnt2 < HEAREPOTION_MAX; cnt2++)
		{
			if (LoadOftenMeshPattern(szModel_FighterFaceName[cnt][cnt2]) == FALSE)
				ErrorCount++;
			if (LoadOftenMeshPattern(szModel_MechFaceName[cnt][cnt2]) == FALSE)
				ErrorCount++;
			if (LoadOftenMeshPattern(szModel_ArcherFaceName[cnt][cnt2]) == FALSE)
				ErrorCount++;
			if (LoadOftenMeshPattern(szModel_PikeFaceName[cnt][cnt2]) == FALSE)
				ErrorCount++;
			if (LoadOftenMeshPattern(szModel_KnightFaceName[cnt][cnt2]) == FALSE)
				ErrorCount++;
			if (LoadOftenMeshPattern(szModel_AtalantaFaceName[cnt][cnt2]) == FALSE)
				ErrorCount++;
			if (LoadOftenMeshPattern(szModel_PristessFaceName[cnt][cnt2]) == FALSE)
				ErrorCount++;
			if (LoadOftenMeshPattern(szModel_MagicanFaceName[cnt][cnt2]) == FALSE)
				ErrorCount++;

		}
	}

	ErrorCount += LoadDataDrivenClassModels(FALSE, TRUE);

	if (ErrorCount)
		return FALSE;

	return TRUE;

}


int LoadOftenArmor()
{
	const int legacyErrors = LoadLegacyClassArmorModelSet(
		gameplay::classes::ArmorVisualVariant::Normal, 0, 29, TRUE);
	const int dataDrivenErrors = LoadDataDrivenClassArmorModels();
	return legacyErrors == 0 && dataDrivenErrors == 0 ? TRUE : FALSE;
}
