int LoadFolder_Ini(char *szPath, char *szWildCard, char *szChgExt, int OftSave)
{
	HANDLE hFindHandle;
	WIN32_FIND_DATA	fd;
	char	szFindPath[64];
	char	szFilePath[64];
	char	*lpPath;

	HANDLE hFindHandle2;
	WIN32_FIND_DATA	fd2;
	char	szFindPath2[64];

	smDPAT	*lpDPatTest[65536];
	int		DPatCount = 0;
	int		cnt;

	strcpy_s(szFindPath, szPath);
	lstrcat(szFindPath, "//*.*");

	hFindHandle = FindFirstFile(szFindPath, &fd);
	if (hFindHandle != INVALID_HANDLE_VALUE)
	{
		while (1)
		{
			if ((fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) != 0)
			{
				wsprintf(szFindPath2, "%s//%s//%s", szPath, fd.cFileName, szWildCard);

				hFindHandle2 = FindFirstFile(szFindPath2, &fd2);
				if (hFindHandle2 != INVALID_HANDLE_VALUE)
				{
					while (1)
					{
						wsprintf(szFilePath, "%s//%s//%s", szPath, fd.cFileName, fd2.cFileName);

						if (OftSave)
						{


							if (szChgExt)
								lpPath = ChangeFileExt(szFilePath, szChgExt);
							else
								lpPath = 0;

							if (!lpPath)
								lpPath = szFilePath;

							strcpy_s(szFilePath, lpPath);

							LoadOftenMeshPattern(szFilePath);
						}
						else
						{

							lpDPatTest[DPatCount] = smPattern.LoadCharactor(szFilePath);
							if (lpDPatTest[DPatCount]) DPatCount++;
							else
							{
								DPatCount = DPatCount;
							}
						}

						if (FindNextFile(hFindHandle2, &fd2) == FALSE)
							break;
					}
					FindClose(hFindHandle2);
				}
			}


			if (FindNextFile(hFindHandle, &fd) == FALSE)
				break;
		}
		FindClose(hFindHandle);
	}

	for (cnt = 0; cnt < DPatCount; cnt++)
	{
		smDPAT_Delete(lpDPatTest[cnt]);
	}

	return TRUE;
}

sSOD_SCORE	sSodScore;
int			SodScoreFlag = 0;
extern SBL_SetFontEffect SoD_SetFontEffect;
