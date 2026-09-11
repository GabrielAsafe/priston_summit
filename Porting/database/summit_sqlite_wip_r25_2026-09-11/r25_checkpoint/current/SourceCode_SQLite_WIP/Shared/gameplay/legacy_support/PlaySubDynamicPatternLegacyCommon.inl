DWORD WINAPI RemoveDynaPatternThreadProc(void *pInfo)
{
	HANDLE	hThread;

	int Counter = 0;


	int RemoveCnt;

	hThread = GetCurrentThread();

	while (1)
	{
		if (quit) break;

		if ((DynRemovePush - DynRemoveCnt) > 32)
		{

			EnterCriticalSection(&cLoadSection);
			EnterCriticalSection(&cDrawSection);

			while (1)
			{
				RemoveCnt = DynRemoveCnt & DYN_REMOVER_MASK;
				smDPAT_Delete(DynRemoveList[RemoveCnt]);
				DynRemoveCnt++;
				if (DynRemovePush <= DynRemoveCnt) break;
			}


			LeaveCriticalSection(&cDrawSection);
			LeaveCriticalSection(&cLoadSection);
		}
		else
			SuspendThread(hThread);

		Counter++;
	}

	ExitThread(TRUE);
	return TRUE;
}

int RemoveDynPatSub()
{
	int RemoveCnt;

	while (1)
	{
		if ((DynRemovePush - DynRemoveCnt) > 32)
		{
			while (1)
			{
				RemoveCnt = DynRemoveCnt & DYN_REMOVER_MASK;
				smDPAT_Delete(DynRemoveList[RemoveCnt]);
				DynRemoveCnt++;
				if (DynRemovePush <= DynRemoveCnt) break;
			}
		}
		else
			break;
	}

	return TRUE;
}



int AddRemoveDynPattern(smDPAT *lpDinaPattern)
{



	if (ServerMode)
	{
		if (lpDinaPattern->UseCount > 1)
			smDPAT_Delete(lpDinaPattern);

		return TRUE;
	}


	DynRemoveList[DynRemovePush&DYN_REMOVER_MASK] = lpDinaPattern;
	DynRemovePush++;
	RemoveDynPatSub();

	return DynRemovePush;
}
