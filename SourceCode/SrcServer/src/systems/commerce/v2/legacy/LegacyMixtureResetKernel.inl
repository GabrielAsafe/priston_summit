// AMBER ownership extraction from the frozen OnServer monolith.
// Kept as an .inl so the byte-equivalent legacy kernel remains in the same
// server translation unit while source ownership moves out of RED. New
// behavior belongs behind IMixingService, not in this compatibility kernel.

int rsRecvMixtureResetItem(rsPLAYINFO *lpPlayInfo, sMIXTURE_RESET_ITEM_SERVER *pMixtureResetItem_Server)
{
	cMIXTURERESET ccItem;
	smTRANS_COMMAND_EX	smTransCommandEx;
	int ErrorFlag = 0;
	int MixtureResetSuccess = FALSE;


	if(pMixtureResetItem_Server->DesMixtureResetItem.sItemInfo.CODE)
	{
		if(rsDeleteInvenItem(lpPlayInfo, pMixtureResetItem_Server->DesMixtureResetItem.sItemInfo.CODE,
			pMixtureResetItem_Server->DesMixtureResetItem.sItemInfo.ItemHeader.Head,
			pMixtureResetItem_Server->DesMixtureResetItem.sItemInfo.ItemHeader.dwChkSum) < 0)
		{

			smTransCommandEx.WParam = 8000;
			smTransCommandEx.WxParam = 50;
			smTransCommandEx.LxParam = (int)"*Mixture Reset Item*";
			smTransCommandEx.LParam = pMixtureResetItem_Server->DesMixtureResetItem.sItemInfo.CODE;
			smTransCommandEx.SParam = pMixtureResetItem_Server->DesMixtureResetItem.sItemInfo.ItemHeader.Head;
			smTransCommandEx.EParam = pMixtureResetItem_Server->DesMixtureResetItem.sItemInfo.ItemHeader.dwChkSum;
			RecordHackLogFile(lpPlayInfo, &smTransCommandEx);

			ErrorFlag++;

			pMixtureResetItem_Server->DesMixtureResetItem.sItemInfo.CODE = 0;
		}

		SaveThrowItem(lpPlayInfo, pMixtureResetItem_Server->DesMixtureResetItem.sItemInfo.CODE,
			pMixtureResetItem_Server->DesMixtureResetItem.sItemInfo.ItemHeader.Head,
			pMixtureResetItem_Server->DesMixtureResetItem.sItemInfo.ItemHeader.dwChkSum);
	}


	if(pMixtureResetItem_Server->dwMixtureResetStoneItemCode)
	{
		if(rsDeleteInvenItem(lpPlayInfo, pMixtureResetItem_Server->dwMixtureResetStoneItemCode,
			pMixtureResetItem_Server->dwMixtureResetStoneHead, pMixtureResetItem_Server->dwMixtureResetStoneCheckSum) < 0)
		{

			smTransCommandEx.WParam = 8000;
			smTransCommandEx.WxParam = 50;
			smTransCommandEx.LxParam = (int)"*Mixture Reset Stone*";
			smTransCommandEx.LParam = pMixtureResetItem_Server->dwMixtureResetStoneItemCode;
			smTransCommandEx.SParam = pMixtureResetItem_Server->dwMixtureResetStoneHead;
			smTransCommandEx.EParam = pMixtureResetItem_Server->dwMixtureResetStoneCheckSum;
			RecordHackLogFile(lpPlayInfo, &smTransCommandEx);

			ErrorFlag++;

			pMixtureResetItem_Server->dwMixtureResetStoneItemCode = 0;
		}

		SaveThrowItem(lpPlayInfo, pMixtureResetItem_Server->dwMixtureResetStoneItemCode,
			pMixtureResetItem_Server->dwMixtureResetStoneHead, pMixtureResetItem_Server->dwMixtureResetStoneCheckSum);
	}


	DWORD dwResultItemCode = 0;
	dwResultItemCode = pMixtureResetItem_Server->DesMixtureResetItem.sItemInfo.CODE;



	DWORD dwSpecialJob = 0;
	dwSpecialJob = pMixtureResetItem_Server->DesMixtureResetItem.sItemInfo.JobCodeMask;

	int	i;
	DWORD dwJobCode = 0;
	JOB_DATA_CODE *pJobDataCode = 0;


	for(i = 1; i < 9; i++)
	{
		pJobDataCode = GetJobDataCode(i, 0);
		if(pJobDataCode->JobBitCode == dwSpecialJob)
		{
			dwJobCode = pJobDataCode->JobCode;
			break;
		}
	}


	if(i == 9)
	{
		dwJobCode = 0;
		NotItemSpecial = 1;
	}


	if(ErrorFlag == 0)
	{
		if(dwResultItemCode != 0)
		{
			for(int cnt = 0; cnt < DefaultItemCount; cnt++)
			{
				if(DefaultItems[cnt].Item.CODE == dwResultItemCode)
				{
					CreateDefItem(&pMixtureResetItem_Server->DesMixtureResetItem.sItemInfo, &DefaultItems[cnt], dwJobCode);
					MixtureResetSuccess = TRUE;
					break;
				}
			}
		}
		else
		{
			ErrorFlag++;
		}
	}
	else
	{
		ErrorFlag++;
	}


	ccItem.sinRecvMixtureItemReset(pMixtureResetItem_Server);


	if(MixtureResetSuccess)
		sMixtureReset_Recv.Result = TRUE;


	if(ErrorFlag)
		sMixtureReset_Recv.Result = FALSE;

	if(sMixtureReset_Recv.Result == TRUE)
	{
		ReformItem(&sMixtureReset_Recv.DesMixtureResetItem.sItemInfo);
		sMixtureReset_Recv.size = sizeof(sITEM) + 32;


		rsRecord_MixtureResetItemLog(lpPlayInfo, pMixtureResetItem_Server, ITEMLOG_MIXTURE_RESET_LOST);


		rsAddInvenItem(lpPlayInfo, sMixtureReset_Recv.DesMixtureResetItem.sItemInfo.CODE,
			sMixtureReset_Recv.DesMixtureResetItem.sItemInfo.ItemHeader.Head,
			sMixtureReset_Recv.DesMixtureResetItem.sItemInfo.ItemHeader.dwChkSum);


		rsRecord_ItemLog2(lpPlayInfo, sMixtureReset_Recv.DesMixtureResetItem.sItemInfo.CODE, sMixtureReset_Recv.DesMixtureResetItem.sItemInfo.ItemHeader.Head,
			sMixtureReset_Recv.DesMixtureResetItem.sItemInfo.ItemHeader.dwChkSum, ITEMLOG_MIXTURE_RESET_GET);


		RecordMixtureResetItem(lpPlayInfo, &sMixtureReset_Recv.DesMixtureResetItem.sItemInfo);
	}
	else
	{
		sMixtureReset_Recv.size = 32;
		rsRecord_MixtureResetItemLog(lpPlayInfo, pMixtureResetItem_Server, ITEMLOG_MIXTURE_RESET_FAIL);
	}


	sMixtureReset_Recv.code = OpCode::OPCODE_MIXTURE_RESET_ITEM;

	if(lpPlayInfo && lpPlayInfo->lpsmSock)
	{
		lpPlayInfo->lpsmSock->Send((char *)&sMixtureReset_Recv, sMixtureReset_Recv.size, TRUE);
	}


	return TRUE;
}
