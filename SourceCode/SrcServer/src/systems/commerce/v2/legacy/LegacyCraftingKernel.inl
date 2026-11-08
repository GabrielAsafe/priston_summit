// AMBER ownership extraction from the frozen OnServer monolith.
// Byte-equivalent Crafting kernels remain in the same server translation unit.
// New orchestration belongs behind ICraftingService.

#include "systems/commerce/v2/legacy/LegacyWalletAdapter.h"
#include "content/v1/skin_reform/content/SkinReformRuntime.h"

int rsRecvCraftItem(rsPLAYINFO *lpPlayInfo, sCRAFTITEM_SERVER *pCraftItem_Server)
{
	cCRAFTITEM	ccItem;
	sITEMINFO	sItemInfo;
	smTRANS_COMMAND	smTransCommand;
	smTRANS_COMMAND_EX	smTransCommandEx;
	int cnt;
	int	ErrorFlag;
	int Result;
	DWORD ItemCraftType = 0;
	int RandNum = 0;
	int PerfStone = 0;

	ErrorFlag = 0;

	for(cnt = 0; cnt < 12; cnt++)
	{
		if((pCraftItem_Server->SheltomCode[cnt] & sinITEM_MASK2) == sinSE1)
		{
			ItemCraftType = sinSE1;
			break;
		}
	}

	int iCountDevine = 0;
	for(cnt = 0; cnt<12; cnt++)
	{
		if((pCraftItem_Server->SheltomCode[cnt]) == (sinOS1 | sin08))
		{
		   iCountDevine += 1;
		}
	}

	for(cnt = 0; cnt < 12; cnt++)
	{
		switch(pCraftItem_Server->SheltomCode[cnt])
		{
		case sinSE1 | sin05:
			spJobCode = 1;
			break;
		case sinSE1 | sin06:
			spJobCode = 2;
			break;
		case sinSE1 | sin07:
			spJobCode = 4;
			break;
		case sinSE1 | sin08:
			spJobCode = 3;
			break;
		case sinSE1 | sin09:
			spJobCode = 6;
			break;
		case sinSE1 | sin10:
			spJobCode = 5;
			break;
		case sinSE1 | sin11:
			spJobCode = 7;
			break;
		case sinSE1 | sin12:
			spJobCode = 8;
			break;
		}
	}


	if(ItemCraftType == sinSE1)
		rsRecord_ItemLost(lpPlayInfo, pCraftItem_Server->SheltomCode, pCraftItem_Server->Head, pCraftItem_Server->CheckSum, ITEMLOG_LOST_RECON);
	else
		rsRecord_ItemLost(lpPlayInfo, pCraftItem_Server->SheltomCode, pCraftItem_Server->Head, pCraftItem_Server->CheckSum, ITEMLOG_MIXLOST);


	for(cnt = 0; cnt < 12; cnt++)
	{
		if(pCraftItem_Server->SheltomCode[cnt])
		{
			if(rsDeleteInvenItem(lpPlayInfo, pCraftItem_Server->SheltomCode[cnt],
				pCraftItem_Server->Head[cnt], pCraftItem_Server->CheckSum[cnt]) < 0)
			{
				smTransCommandEx.WParam = 8000;
				smTransCommandEx.WxParam = 50;
				smTransCommandEx.LxParam = (int)"*MIXTRUE SHELTON*";
				smTransCommandEx.LParam = pCraftItem_Server->SheltomCode[cnt];
				smTransCommandEx.SParam = pCraftItem_Server->Head[cnt];
				smTransCommandEx.EParam = pCraftItem_Server->CheckSum[cnt];
				RecordHackLogFile(lpPlayInfo, &smTransCommandEx);
				ErrorFlag++;

				pCraftItem_Server->SheltomCode[cnt] = 0;
			}

			SaveThrowItem(lpPlayInfo, pCraftItem_Server->SheltomCode[cnt],
				pCraftItem_Server->Head[cnt], pCraftItem_Server->CheckSum[cnt]);
		}
	}

	memcpy(&sItemInfo, &pCraftItem_Server->DesCraftItem.sItemInfo, sizeof(sITEMINFO));

	CurrencyLog.int64_CraftMoney += sItemInfo.Price;

	if(rsServerConfig.ServerInventoryMode)
	{
		if(lpPlayInfo->ServerMoney < sItemInfo.Price)
		{
			smTransCommand.WParam = 8010;
			smTransCommand.LParam = 50;
			smTransCommand.SParam = lpPlayInfo->ServerMoney;
			smTransCommand.EParam = sItemInfo.Price;
			RecordHackLogFile(lpPlayInfo, &smTransCommand);

			(void)gameplay::commerce::legacy::ServerLegacyWalletAdapter().SetBalance(
				gameplay::player::legacy::ServerLegacyOnlinePlayerRegistry().IdOf(lpPlayInfo),
				0, gameplay::commerce::WalletReason::Crafting);
			ErrorFlag++;
		}
		else
		{
			(void)gameplay::commerce::legacy::ServerLegacyWalletAdapter().Debit(
				gameplay::player::legacy::ServerLegacyOnlinePlayerRegistry().IdOf(lpPlayInfo),
				sItemInfo.Price, gameplay::commerce::WalletReason::Crafting);
			lpPlayInfo->UnsaveMoney = lpPlayInfo->ServerMoney;
		}
	}

	if(CheckItemForm(&sItemInfo) == FALSE || sItemInfo.ItemKindCode != 0 || CheckMixItem(&sItemInfo) == FALSE)
	{
		sCraftItem_Recv.Result = FALSE;
		sCraftItem_Recv.size = 32;

		if(lpPlayInfo && lpPlayInfo->lpsmSock)
			lpPlayInfo->lpsmSock->Send((char *)&sCraftItem_Recv, sCraftItem_Recv.size, TRUE);

		smTransCommand.WParam = 99000;
		smTransCommand.SParam = 0;
		smTransCommand.LParam = 0;
		RecordHackLogFile(lpPlayInfo, &smTransCommand);

		return TRUE;
	}

	if(pCraftItem_Server->A_StoneItemCode)
	{
		if ((pCraftItem_Server->A_StoneItemCode == (sinBI1 | sin90)) &&
			rsDeleteInvenItem(lpPlayInfo, pCraftItem_Server->A_StoneItemCode, pCraftItem_Server->A_StoneHead, pCraftItem_Server->A_StoneCheckSum) >= 0)
		{
			rsRecord_ItemLog2(lpPlayInfo, pCraftItem_Server->A_StoneItemCode, pCraftItem_Server->A_StoneHead, pCraftItem_Server->A_StoneCheckSum, ITEMLOG_LOST_RECON);

			SaveThrowItem(lpPlayInfo, pCraftItem_Server->A_StoneItemCode, pCraftItem_Server->A_StoneHead, pCraftItem_Server->A_StoneCheckSum);

			spJobCode = lpPlayInfo->smCharInfo.JOB_CODE;
			PerfStone = TRUE;
		}
		else
		{
			smTransCommandEx.WParam = 8000;
			smTransCommandEx.WxParam = 70;
			smTransCommandEx.LxParam = (int)"Aging Stone";
			smTransCommandEx.LParam = pCraftItem_Server->A_StoneItemCode;
			smTransCommandEx.SParam = pCraftItem_Server->A_StoneHead;
			smTransCommandEx.EParam = pCraftItem_Server->A_StoneCheckSum;
			RecordHackLogFile(lpPlayInfo, &smTransCommandEx);
			pCraftItem_Server->A_StoneItemCode = 0;
		}
	}
	else
		pCraftItem_Server->A_StoneItemCode = 0;

	ccItem.sinRecvCraftItem(pCraftItem_Server);
	Result = ccItem.MixingItem();

	if(ErrorFlag)
		sCraftItem_Recv.Result = FALSE;


	if(Result == 2000)
	{
		for (cnt = 0; cnt < DefaultItemCount; cnt++)
		{
			if(DefaultItems[cnt].Item.CODE == sCraftItem_Recv.DesCraftItem.sItemInfo.CODE && !DefaultItems[cnt].Item.UniqueItem)
			{
				if(PerfStone)
					CreateDefItemPerf(&sCraftItem_Recv.DesCraftItem.sItemInfo, &DefaultItems[cnt], spJobCode);
				else
					CreateDefItem(&sCraftItem_Recv.DesCraftItem.sItemInfo, &DefaultItems[cnt], spJobCode);

				break;
			}
		}
	}

	// Covenant Artisan enhancement runs only after legacy crafting has produced
	// a valid non-unique output. The normal result remains unchanged when the
	// player lacks the capability or required Covenant material.
	if(sCraftItem_Recv.Result == TRUE)
		(void)SummitApplyArtisanCraftingOutput(lpPlayInfo, &sCraftItem_Recv.DesCraftItem.sItemInfo);

	spJobCode = 0;
	sCraftItem_Recv.code = OpCode::OPCODE_CRAFTITEM;

	if(sCraftItem_Recv.Result==TRUE)
	{

		ReformItem( &sCraftItem_Recv.DesCraftItem.sItemInfo);
		sCraftItem_Recv.size = sizeof(sITEM)+32;


		if(rsDeleteInvenItem(lpPlayInfo, sItemInfo.CODE, sItemInfo.ItemHeader.Head, sItemInfo.ItemHeader.dwChkSum) >= 0)
		{
			rsAddInvenItem(lpPlayInfo, sCraftItem_Recv.DesCraftItem.sItemInfo.CODE,
				sCraftItem_Recv.DesCraftItem.sItemInfo.ItemHeader.Head,
				sCraftItem_Recv.DesCraftItem.sItemInfo.ItemHeader.dwChkSum);


			RecordMixItem(lpPlayInfo, &sItemInfo, &sCraftItem_Recv.DesCraftItem.sItemInfo);


			if(ItemCraftType == sinSE1)
			{
				rsRecord_ItemLog_Change(lpPlayInfo, sItemInfo.CODE, sItemInfo.ItemHeader.Head, sItemInfo.ItemHeader.dwChkSum,
					sCraftItem_Recv.DesCraftItem.sItemInfo.CODE, sCraftItem_Recv.DesCraftItem.sItemInfo.ItemHeader.Head, sCraftItem_Recv.DesCraftItem.sItemInfo.ItemHeader.dwChkSum, ITEMLOG_GET_RECON);
			}
			else
			{
				rsRecord_ItemLog_Change(lpPlayInfo, sItemInfo.CODE, sItemInfo.ItemHeader.Head, sItemInfo.ItemHeader.dwChkSum,
					sCraftItem_Recv.DesCraftItem.sItemInfo.CODE, sCraftItem_Recv.DesCraftItem.sItemInfo.ItemHeader.Head, sCraftItem_Recv.DesCraftItem.sItemInfo.ItemHeader.dwChkSum, ITEMLOG_MIXING);
			}

			rsSaveAgingItemToHDD(&sCraftItem_Recv.DesCraftItem.sItemInfo);

			// A successful authoritative mix/reconstruction advances Artisan
			// Covenant content only after the replacement item was accepted.
			if(lpPlayInfo->getName())
				(void)gameplay::progression::RecordServerCovenantActivity(
					lpPlayInfo->getName(), "crafting.success", 1);
		}
		else
		{
			sCraftItem_Recv.size = 32;
			sCraftItem_Recv.Result = FALSE;

			smTransCommandEx.WParam = 8000;
			smTransCommandEx.WxParam = 50;
			smTransCommandEx.LxParam = (int)sItemInfo.ItemName;
			smTransCommandEx.LParam = sItemInfo.CODE;
			smTransCommandEx.SParam = sItemInfo.ItemHeader.Head;
			smTransCommandEx.EParam = sItemInfo.ItemHeader.dwChkSum;
			RecordHackLogFile(lpPlayInfo, &smTransCommandEx);
		}

		SaveThrowItem(lpPlayInfo, sItemInfo.CODE, sItemInfo.ItemHeader.Head, sItemInfo.ItemHeader.dwChkSum);
	}
	else
		sCraftItem_Recv.size = 32;


	if(lpPlayInfo && lpPlayInfo->lpsmSock)
		lpPlayInfo->lpsmSock->Send((char *)&sCraftItem_Recv, sCraftItem_Recv.size, TRUE);

	return TRUE;
}

int rsRecvSmeltingItem(rsPLAYINFO* lpPlayInfo, sSMELTINGITEM_SERVER* pSmeltingItem_Server)
{
	CSmeltingItem ccItem;


	smTRANS_COMMAND_EX	smTransCommandEx;

	int cnt;
	int	ErrorFlag;

	DWORD ItemSmeltingType = 0;

	bool bFirst_Ore = TRUE;
	bool bOre_OK = TRUE;
	DWORD first_Ore = 0;
	DWORD Next_Ore = 0;
	int ore_num = 0;
	int crystal_num = 0;

	bool bFirst_Sheltom = TRUE;
	bool bSheltom_OK = TRUE;
	DWORD first_Sheltom = 0;
	DWORD Next_Sheltom = 0;

	ErrorFlag = 0;


	rsRecord_SmeltingItemLog(lpPlayInfo, pSmeltingItem_Server->SmeltingCode, pSmeltingItem_Server->Head, pSmeltingItem_Server->CheckSum, ITEMLOG_SMELTING_LOST);
	for (cnt = 0; cnt < 3; cnt++)
	{
		if (pSmeltingItem_Server->SmeltingCode[cnt])
		{
			if ((pSmeltingItem_Server->SmeltingCode[cnt] & sinITEM_MASK2) == sinPR1)
			{
				if (ore_num < 3)
				{
					ItemSmeltingType = sinPR1;
					ore_num++;
				}
			}
			else if ((pSmeltingItem_Server->SmeltingCode[cnt] & sinITEM_MASK2) == sinPR2)
			{
				if (crystal_num < 3)
				{
					ItemSmeltingType = sinPR2;
					crystal_num++;
				}

			}
			if (rsDeleteInvenItem(lpPlayInfo, pSmeltingItem_Server->SmeltingCode[cnt],
				pSmeltingItem_Server->Head[cnt], pSmeltingItem_Server->CheckSum[cnt]) < 0)
			{
				if (ItemSmeltingType == sinPR1)
				{

					smTransCommandEx.WParam = 8000;
					smTransCommandEx.WxParam = 50;
					smTransCommandEx.LxParam = (int)"*SMELTING ORE*";
					smTransCommandEx.LParam = pSmeltingItem_Server->SmeltingCode[cnt];
					smTransCommandEx.SParam = pSmeltingItem_Server->Head[cnt];
					smTransCommandEx.EParam = pSmeltingItem_Server->CheckSum[cnt];
					RecordHackLogFile(lpPlayInfo, &smTransCommandEx);
				}
				else if (ItemSmeltingType == sinPR2)
				{

					smTransCommandEx.WParam = 8000;
					smTransCommandEx.WxParam = 50;
					smTransCommandEx.LxParam = (int)"*SMELTING CRYSTAL*";
					smTransCommandEx.LParam = pSmeltingItem_Server->SmeltingCode[cnt];
					smTransCommandEx.SParam = pSmeltingItem_Server->Head[cnt];
					smTransCommandEx.EParam = pSmeltingItem_Server->CheckSum[cnt];
					RecordHackLogFile(lpPlayInfo, &smTransCommandEx);
				}
				ErrorFlag++;

				pSmeltingItem_Server->SmeltingCode[cnt] = 0;
			}
			else
			{
				if (bFirst_Ore)
				{
					first_Ore = pSmeltingItem_Server->SmeltingCode[cnt];
					Next_Ore = pSmeltingItem_Server->SmeltingCode[cnt];
					bFirst_Ore = FALSE;
				}
				else
				{
					Next_Ore = pSmeltingItem_Server->SmeltingCode[cnt];
				}
				if (first_Ore != Next_Ore)
				{
					bOre_OK = FALSE;
					ErrorFlag++;
				}
			}
			SaveThrowItem(lpPlayInfo, pSmeltingItem_Server->SmeltingCode[cnt],
				pSmeltingItem_Server->Head[cnt], pSmeltingItem_Server->CheckSum[cnt]);
		}
	}

	if (ItemSmeltingType == sinPR1 && ore_num != 3)
	{
		bOre_OK = FALSE;
		ErrorFlag++;
	}
	else if (ItemSmeltingType == sinPR2 && crystal_num != 3)
	{
		bOre_OK = FALSE;
		ErrorFlag++;
	}

	if (pSmeltingItem_Server->SmeltingCode[4])
	{
		if (rsDeleteInvenItem(lpPlayInfo, pSmeltingItem_Server->SmeltingCode[4],
			pSmeltingItem_Server->Head[4], pSmeltingItem_Server->CheckSum[4]) < 0)
		{

			smTransCommandEx.WParam = 8000;
			smTransCommandEx.WxParam = 50;
			smTransCommandEx.LxParam = (int)"*SMELTING SHELTON*";
			smTransCommandEx.LParam = pSmeltingItem_Server->SmeltingCode[4];
			smTransCommandEx.SParam = pSmeltingItem_Server->Head[4];
			smTransCommandEx.EParam = pSmeltingItem_Server->CheckSum[4];
			RecordHackLogFile(lpPlayInfo, &smTransCommandEx);
			ErrorFlag++;

			pSmeltingItem_Server->SmeltingCode[4] = 0;
		}
		if (bFirst_Sheltom)
		{
			first_Sheltom = pSmeltingItem_Server->SmeltingCode[4];
			Next_Sheltom = pSmeltingItem_Server->SmeltingCode[4];
			bFirst_Sheltom = FALSE;
		}
		else
		{
			Next_Sheltom = pSmeltingItem_Server->SmeltingCode[4];
		}
		if (first_Sheltom != Next_Sheltom)
		{
			bSheltom_OK = FALSE;
			ErrorFlag++;
		}
		SaveThrowItem(lpPlayInfo, pSmeltingItem_Server->SmeltingCode[4],
			pSmeltingItem_Server->Head[4], pSmeltingItem_Server->CheckSum[4]);
	}
























	DWORD code = 0;
	if (bOre_OK && bSheltom_OK)
	{
		if ((Next_Ore & sinITEM_MASK2) == sinPR1)
		{
			if (Next_Ore == (sinPR1 | sin01) && Next_Sheltom == (sinOS1 | sin06))
			{
				code = (sinPR2 | sin01);
			}
			else if (Next_Ore == (sinPR1 | sin02) && Next_Sheltom == (sinOS1 | sin07))
			{
				code = (sinPR2 | sin02);
			}
			else if (Next_Ore == (sinPR1 | sin03) && Next_Sheltom == (sinOS1 | sin08))
			{
				code = (sinPR2 | sin03);
			}
			else if (Next_Ore == (sinPR1 | sin04) && Next_Sheltom == (sinOS1 | sin09))
			{
				code = (sinPR2 | sin04);
			}
			else if (Next_Ore == (sinPR1 | sin05) && Next_Sheltom == (sinOS1 | sin10))
			{
				code = (sinPR2 | sin05);
			}
			else if (Next_Ore == (sinPR1 | sin06) && Next_Sheltom == (sinOS1 | sin11))
			{
				code = (sinPR2 | sin06);
			}
			else if (Next_Ore == (sinPR1 | sin07) && Next_Sheltom == (sinOS1 | sin12))
			{
				code = (sinPR2 | sin07);
			}
			else if (Next_Ore == (sinPR1 | sin07) && Next_Sheltom == (sinOS1 | sin13))
			{
				code = (sinPR2 | sin08);
			}
		}

		else if ((Next_Ore & sinITEM_MASK2) == sinPR2)
		{
			if (Next_Ore == (sinPR2 | sin01) && Next_Sheltom == (sinOS1 | sin07))
			{
				switch (rand() % 2)
				{
				case 0:
					code = (sinPR3 | sin01);
					break;
				case 1:
					code = (sinPR4 | sin01);
					break;
				}
			}
			else if (Next_Ore == (sinPR2 | sin02) && Next_Sheltom == (sinOS1 | sin08))
			{
				switch (rand() % 2)
				{
				case 0:
					code = (sinPR3 | sin02);
					break;
				case 1:
					code = (sinPR4 | sin02);
					break;
				}
			}
			else if (Next_Ore == (sinPR2 | sin03) && Next_Sheltom == (sinOS1 | sin09))
			{
				switch (rand() % 2)
				{
				case 0:
					code = (sinPR3 | sin03);
					break;
				case 1:
					code = (sinPR4 | sin03);
					break;
				}
			}
			else if (Next_Ore == (sinPR2 | sin04) && Next_Sheltom == (sinOS1 | sin10))
			{
				switch (rand() % 2)
				{
				case 0:
					code = (sinPR3 | sin04);
					break;
				case 1:
					code = (sinPR4 | sin04);
					break;
				}
			}
			else if (Next_Ore == (sinPR2 | sin05) && Next_Sheltom == (sinOS1 | sin11))
			{
				switch (rand() % 2)
				{
				case 0:
					code = (sinPR3 | sin05);
					break;
				case 1:
					code = (sinPR4 | sin05);
					break;
				}
			}
			else if (Next_Ore == (sinPR2 | sin06) && Next_Sheltom == (sinOS1 | sin12))
			{
				switch (rand() % 2)
				{
				case 0:
					code = (sinPR3 | sin06);
					break;
				case 1:
					code = (sinPR4 | sin06);
					break;
				}
			}
			else if (Next_Ore == (sinPR2 | sin07) && Next_Sheltom == (sinOS1 | sin13))
			{
				switch (rand() % 2)
				{
				case 0:
					code = (sinPR3 | sin07);
					break;
				case 1:
					code = (sinPR4 | sin07);
					break;
				}
			}



			else if (Next_Ore == (sinPR2 | sin09) && Next_Sheltom == (sinOS1 | sin03))
			{
				switch (rand() % 2)
				{
				case 0:
					code = (sinPR3 | sin09);
					break;
				case 1:
					code = (sinPR4 | sin09);
					break;
				}
			}


			else if (Next_Ore == (sinPR2 | sin10) && Next_Sheltom == (sinOS1 | sin04))
			{
				switch (rand() % 2)
				{
				case 0:
					code = (sinPR3 | sin10);
					break;
				case 1:
					code = (sinPR4 | sin10);
					break;
				}
			}


			else if (Next_Ore == (sinPR2 | sin11) && Next_Sheltom == (sinOS1 | sin05))
			{
				switch (rand() % 2)
				{
				case 0:
					code = (sinPR3 | sin11);
					break;
				case 1:
					code = (sinPR4 | sin11);
					break;
				}
			}


			else if (Next_Ore == (sinPR2 | sin12) && Next_Sheltom == (sinOS1 | sin03))
			{
				switch (rand() % 2)
				{
				case 0:
					code = (sinPR3 | sin12);
					break;
				case 1:
					code = (sinPR4 | sin12);
					break;
				}
			}

			else if (Next_Ore == (sinPR2 | sin13) && Next_Sheltom == (sinOS1 | sin04))
			{
				switch (rand() % 2)
				{
				case 0:
					code = (sinPR3 | sin13);
					break;
				case 1:
					code = (sinPR4 | sin13);
					break;
				}
			}

			else if (Next_Ore == (sinPR2 | sin14) && Next_Sheltom == (sinOS1 | sin05))
			{
				switch (rand() % 2)
				{
				case 0:
					code = (sinPR3 | sin14);
					break;
				case 1:
					code = (sinPR4 | sin14);
					break;
				}
			}














		}
		else
		{
			ErrorFlag++;
		}

		if (code != 0)
		{
			for (cnt = 0; cnt < DefaultItemCount; cnt++)
			{
				if (DefaultItems[cnt].Item.CODE == code)
				{

					CreateDefItem(&pSmeltingItem_Server->DesSmeltingItem.sItemInfo, &DefaultItems[cnt]);
					break;
				}
			}
		}

	}

	ccItem.sinRecvSmeltingItem(pSmeltingItem_Server);

	if (ErrorFlag)
	{
		sSmeltingItem_Recv.Result = FALSE;
	}
	else
	{
		sSmeltingItem_Recv.Result = TRUE;
	}

	if (sSmeltingItem_Recv.Result == TRUE)
	{
		ReformItem(&sSmeltingItem_Recv.DesSmeltingItem.sItemInfo);
		sSmeltingItem_Recv.size = sizeof(sITEM) + 32;

		rsAddInvenItem(lpPlayInfo, sSmeltingItem_Recv.DesSmeltingItem.sItemInfo.CODE,
			sSmeltingItem_Recv.DesSmeltingItem.sItemInfo.ItemHeader.Head,
			sSmeltingItem_Recv.DesSmeltingItem.sItemInfo.ItemHeader.dwChkSum);

		rsRecord_ItemLog2(lpPlayInfo, sSmeltingItem_Recv.DesSmeltingItem.sItemInfo.CODE, sSmeltingItem_Recv.DesSmeltingItem.sItemInfo.ItemHeader.Head,
			sSmeltingItem_Recv.DesSmeltingItem.sItemInfo.ItemHeader.dwChkSum, ITEMLOG_SMELTING_GET);

		RecordSmeltingItem(lpPlayInfo, &sSmeltingItem_Recv.DesSmeltingItem.sItemInfo);

	}
	else
	{
		sSmeltingItem_Recv.size = 32;
	}

	sSmeltingItem_Recv.code = OpCode::OPCODE_SMELTINGITEM;

	if (lpPlayInfo && lpPlayInfo->lpsmSock)
	{
		lpPlayInfo->lpsmSock->Send((char*)&sSmeltingItem_Recv, sSmeltingItem_Recv.size, TRUE);
	}

	return TRUE;
}

int rsRecvManufactureItem(rsPLAYINFO* lpPlayInfo, SManufactureItem_Server* pManufactureItem_Server)
{
	CManufacture ccItem;

	smTRANS_COMMAND_EX	smTransCommandEx;

	int cnt;
	int	ErrorFlag;
	DWORD ItemSmeltingType = 0;
	bool  bProbability = FALSE;
	ErrorFlag = 0;

	if (pManufactureItem_Server->RuneCode[0])
	{
		if (rsDeleteInvenItem(lpPlayInfo, pManufactureItem_Server->RuneCode[0],
			pManufactureItem_Server->Head[0], pManufactureItem_Server->CheckSum[0]) < 0)
		{

			smTransCommandEx.WParam = 8000;
			smTransCommandEx.WxParam = 50;
			smTransCommandEx.LxParam = (int)"*Manufacture Recipe*";
			smTransCommandEx.LParam = pManufactureItem_Server->RuneCode[0];
			smTransCommandEx.SParam = pManufactureItem_Server->Head[0];
			smTransCommandEx.EParam = pManufactureItem_Server->CheckSum[0];
			RecordHackLogFile(lpPlayInfo, &smTransCommandEx);

			ErrorFlag++;

			pManufactureItem_Server->RuneCode[0] = 0;
		}
		SaveThrowItem(lpPlayInfo, pManufactureItem_Server->RuneCode[0],
			pManufactureItem_Server->Head[0], pManufactureItem_Server->CheckSum[0]);
	}

	for (cnt = 1; cnt < 4; cnt++)
	{
		if (pManufactureItem_Server->RuneCode[cnt])
		{
			if (rsDeleteInvenItem(lpPlayInfo, pManufactureItem_Server->RuneCode[cnt],
				pManufactureItem_Server->Head[cnt], pManufactureItem_Server->CheckSum[cnt]) < 0)
			{

				smTransCommandEx.WParam = 8000;
				smTransCommandEx.WxParam = 50;
				smTransCommandEx.LxParam = (int)"*Manufacture Rune*";
				smTransCommandEx.LParam = pManufactureItem_Server->RuneCode[cnt];
				smTransCommandEx.SParam = pManufactureItem_Server->Head[cnt];
				smTransCommandEx.EParam = pManufactureItem_Server->CheckSum[cnt];
				RecordHackLogFile(lpPlayInfo, &smTransCommandEx);

				ErrorFlag++;

				pManufactureItem_Server->RuneCode[cnt] = 0;
			}
			SaveThrowItem(lpPlayInfo, pManufactureItem_Server->RuneCode[cnt],
				pManufactureItem_Server->Head[cnt], pManufactureItem_Server->CheckSum[cnt]);
		}
	}

	DWORD ResultItemCode = 0;


	if (ErrorFlag == 0)
	{

		ResultItemCode = ManufactureCheckRune(lpPlayInfo, pManufactureItem_Server->RuneCode[0], pManufactureItem_Server->RuneCode);
		if (ResultItemCode != 0)
		{
			srand((UINT)time(NULL));
#ifdef _LANGUAGE_VEITNAM
			if ((rand() % 10) < 4)
#else
			//armadura
			if (ResultItemCode == (sinRESULT | sin01))
			{
				if ((rand() % 100) < 5)
					ResultItemCode = (sinDA3 | sin02);

				else if ((rand() % 100) < 50)
					ResultItemCode = (sinDA3 | sin01);

				else if ((rand() % 100) < 80)
					ResultItemCode = (sinDA1 | sin29);


				for (cnt = 0; cnt < DefaultItemCount; cnt++)
				{
					if (DefaultItems[cnt].Item.CODE == ResultItemCode)
					{

						CreateDefItem(&pManufactureItem_Server->DesManufactureItem.sItemInfo, &DefaultItems[cnt]);
						bProbability = TRUE;
						break;
					}
				}

			}
			else if (ResultItemCode == (sinRESULT | sin02))
			{
				if ((rand() % 100) < 5)
					ResultItemCode = (sinDA4 | sin02);


				else if ((rand() % 100) < 50)
					ResultItemCode = (sinDA4 | sin01);


				else if ((rand() % 100) < 80)
					ResultItemCode = (sinDA2 | sin29);

				for (cnt = 0; cnt < DefaultItemCount; cnt++)
				{
					if (DefaultItems[cnt].Item.CODE == ResultItemCode)
					{

						CreateDefItem(&pManufactureItem_Server->DesManufactureItem.sItemInfo, &DefaultItems[cnt]);
						bProbability = TRUE;
						break;
					}
				}

			}
			else if (ResultItemCode == (sinRESULT | sin03))
			{
				if ((rand() % 100) < 5)
					ResultItemCode = (sinDG1 | sin28);


				else if ((rand() % 100) < 50)
					ResultItemCode = (sinDG1 | sin27);


				else if ((rand() % 100) < 80)
					ResultItemCode = (sinDG1 | sin26);

				for (cnt = 0; cnt < DefaultItemCount; cnt++)
				{
					if (DefaultItems[cnt].Item.CODE == ResultItemCode)
					{

						CreateDefItem(&pManufactureItem_Server->DesManufactureItem.sItemInfo, &DefaultItems[cnt]);
						bProbability = TRUE;
						break;
					}
				}

			}
			else if (ResultItemCode == (sinRESULT | sin04))
			{
				if ((rand() % 100) < 5)
					ResultItemCode = (sinDB1 | sin28);


				else if ((rand() % 100) < 50)
					ResultItemCode = (sinDB1 | sin27);


				else if ((rand() % 100) < 80)
					ResultItemCode = (sinDB1 | sin26);

				for (cnt = 0; cnt < DefaultItemCount; cnt++)
				{
					if (DefaultItems[cnt].Item.CODE == ResultItemCode)
					{

						CreateDefItem(&pManufactureItem_Server->DesManufactureItem.sItemInfo, &DefaultItems[cnt]);
						bProbability = TRUE;
						break;
					}
				}

			}
			else if (ResultItemCode == (sinRESULT | sin05))
			{
				if ((rand() % 100) < 5)
					ResultItemCode = (sinOA2 | sin28);


				else if ((rand() % 100) < 50)
					ResultItemCode = (sinOA2 | sin27);


				else if ((rand() % 100) < 80)
					ResultItemCode = (sinOA2 | sin26);

				for (cnt = 0; cnt < DefaultItemCount; cnt++)
				{
					if (DefaultItems[cnt].Item.CODE == ResultItemCode)
					{

						CreateDefItem(&pManufactureItem_Server->DesManufactureItem.sItemInfo, &DefaultItems[cnt]);
						bProbability = TRUE;
						break;
					}
				}

			}
			else if (ResultItemCode == (sinRESULT | sin06))
			{
				if ((rand() % 100) < 5)
					ResultItemCode = (sinDS1 | sin28);


				else if ((rand() % 100) < 50)
					ResultItemCode = (sinDS1 | sin27);


				else if ((rand() % 100) < 80)
					ResultItemCode = (sinDS1 | sin26);

				for (cnt = 0; cnt < DefaultItemCount; cnt++)
				{
					if (DefaultItems[cnt].Item.CODE == ResultItemCode)
					{

						CreateDefItem(&pManufactureItem_Server->DesManufactureItem.sItemInfo, &DefaultItems[cnt]);
						bProbability = TRUE;
						break;
					}
				}

			}
			else {
				if ((rand() % 100) < 50)
#endif
				{
					for (cnt = 0; cnt < DefaultItemCount; cnt++)
					{
						if (DefaultItems[cnt].Item.CODE == ResultItemCode)
						{

							CreateDefItem(&pManufactureItem_Server->DesManufactureItem.sItemInfo, &DefaultItems[cnt]);
							bProbability = TRUE;
							break;
						}
					}
				}
				else
				{
					ErrorFlag++;
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
	ccItem.sinRecvManufactureItem(pManufactureItem_Server);

	if (bProbability)
	{
		g_sManufactureItem_Recv.Result = TRUE;
	}

	if (ErrorFlag)
	{
		g_sManufactureItem_Recv.Result = FALSE;
	}


	if (g_sManufactureItem_Recv.Result == TRUE)
	{
		(void)SummitApplyArtisanCraftingOutput(
			lpPlayInfo, &g_sManufactureItem_Recv.DesManufactureItem.sItemInfo);
		ReformItem(&g_sManufactureItem_Recv.DesManufactureItem.sItemInfo);
		g_sManufactureItem_Recv.size = sizeof(sITEM) + 32;

		rsRecord_ManufactureItemLog(lpPlayInfo, pManufactureItem_Server->RuneCode, pManufactureItem_Server->Head, pManufactureItem_Server->CheckSum, ITEMLOG_MANUFACTURE_LOST);

		rsAddInvenItem(lpPlayInfo, g_sManufactureItem_Recv.DesManufactureItem.sItemInfo.CODE,
			g_sManufactureItem_Recv.DesManufactureItem.sItemInfo.ItemHeader.Head,
			g_sManufactureItem_Recv.DesManufactureItem.sItemInfo.ItemHeader.dwChkSum);

		rsRecord_ItemLog2(lpPlayInfo, g_sManufactureItem_Recv.DesManufactureItem.sItemInfo.CODE, g_sManufactureItem_Recv.DesManufactureItem.sItemInfo.ItemHeader.Head,
			g_sManufactureItem_Recv.DesManufactureItem.sItemInfo.ItemHeader.dwChkSum, ITEMLOG_MANUFACTURE_GET);


		RecordManufactureItem(lpPlayInfo, &g_sManufactureItem_Recv.DesManufactureItem.sItemInfo);

		// A successful authoritative manufacture advances Artisan Covenant content.
		if(lpPlayInfo->getName())
			(void)gameplay::progression::RecordServerCovenantActivity(
				lpPlayInfo->getName(), "crafting.success", 1);

		TRANS_CHATMESSAGE	TransChatMessage;
		wsprintf(TransChatMessage.szMessage, "Parab\351ns %s Acabouu de Craftar o Item %s", lpPlayInfo->getName(), g_sManufactureItem_Recv.DesManufactureItem.sItemInfo.ItemName);
		TransChatMessage.code = OpCode::OPCODE_WHISPERMESSAGE;
		TransChatMessage.size = 32 + lstrlen(TransChatMessage.szMessage);
		TransChatMessage.dwIP = 6;
		TransChatMessage.dwObjectSerial = 0;

		for (cnt = 0; cnt < CONNECTMAX; cnt++)
		{
			if (rsPlayInfo[cnt].lpsmSock)
			{
				rsPlayInfo[cnt].lpsmSock->Send((char*)&TransChatMessage, TransChatMessage.size, TRUE);
			}
		}


	}
	else
	{
		g_sManufactureItem_Recv.size = 32;
		rsRecord_ManufactureItemLog(lpPlayInfo, pManufactureItem_Server->RuneCode, pManufactureItem_Server->Head, pManufactureItem_Server->CheckSum, ITEMLOG_MANUFACTURE_FAIL);
	}

	g_sManufactureItem_Recv.code = OpCode::OPCODE_MANUFACTURE;

	if (lpPlayInfo && lpPlayInfo->lpsmSock)
	{
		lpPlayInfo->lpsmSock->Send((char*)&g_sManufactureItem_Recv, g_sManufactureItem_Recv.size, TRUE);
	}

	return TRUE;
}

int rsRecvSkinChangeItem(rsPLAYINFO* lpPlayInfo, sSKIN_CHANGE_SERVER* pSkinChangeItem_Server)
{
	cSKINSYSTEM ccItem;
	smTRANS_COMMAND_EX	smTransCommandEx;
	int ErrorFlag = 0;
	DWORD	SkinClass;
	int SkinSuccess = FALSE;

	// Reject unavailable mappings BEFORE rsDeleteInvenItem consumes either item.
	std::uint32_t destination = 0;
	// SUMMIT_SKIN_SERVER_GUARD_20261027: codigo-base valido ANTES de consumir inventario.
	if (!lpPlayInfo || !pSkinChangeItem_Server ||
	    !pSkinChangeItem_Server->DesSkinChangeItem.sItemInfo.CODE ||
	    !summit::content::skin_reform::ResolveSkinReform(
	        pSkinChangeItem_Server->dwSkinChangeStoneItemCode, destination))
	{
		sSkinChange_Recv.Result = FALSE;
		sSkinChange_Recv.size = 32;
		sSkinChange_Recv.code = OpCode::OPCODE_SKIN_CHANGE_ITEM;
		if (lpPlayInfo && lpPlayInfo->lpsmSock)
			lpPlayInfo->lpsmSock->Send((char*)&sSkinChange_Recv, sSkinChange_Recv.size, TRUE);
		return TRUE;
	}

	// Rejeita consumiveis/outros itens como equipamento de base; preserva itens em erro.
	const DWORD originalFamily = pSkinChangeItem_Server->DesSkinChangeItem.sItemInfo.CODE & sinITEM_MASK2;
	const bool equipmentFamily = originalFamily == sinWA1 || originalFamily == sinWH1 ||
	    originalFamily == sinWC1 || originalFamily == sinWM1 || originalFamily == sinWP1 ||
	    originalFamily == sinWT1 || originalFamily == sinWS1 || originalFamily == sinWS2 ||
	    originalFamily == sinDA1 || originalFamily == sinDA2 || originalFamily == sinDA3 ||
	    originalFamily == sinDA4 || originalFamily == sinDS1;
	if (!equipmentFamily)
	{
		sSkinChange_Recv.Result = FALSE;
		sSkinChange_Recv.size = 32;
		sSkinChange_Recv.code = OpCode::OPCODE_SKIN_CHANGE_ITEM;
		if (lpPlayInfo && lpPlayInfo->lpsmSock)
			lpPlayInfo->lpsmSock->Send((char*)&sSkinChange_Recv, sSkinChange_Recv.size, TRUE);
		return TRUE;
	}

	if(pSkinChangeItem_Server->DesSkinChangeItem.sItemInfo.CODE)
	{
		if(rsDeleteInvenItem(lpPlayInfo, pSkinChangeItem_Server->DesSkinChangeItem.sItemInfo.CODE,
			pSkinChangeItem_Server->DesSkinChangeItem.sItemInfo.ItemHeader.Head,
			pSkinChangeItem_Server->DesSkinChangeItem.sItemInfo.ItemHeader.dwChkSum) < 0)
		{

			smTransCommandEx.WParam = 8000;
			smTransCommandEx.WxParam = 50;
			smTransCommandEx.LxParam = (int)"*Skin Change Item*";
			smTransCommandEx.LParam = pSkinChangeItem_Server->DesSkinChangeItem.sItemInfo.CODE;
			smTransCommandEx.SParam = pSkinChangeItem_Server->DesSkinChangeItem.sItemInfo.ItemHeader.Head;
			smTransCommandEx.EParam = pSkinChangeItem_Server->DesSkinChangeItem.sItemInfo.ItemHeader.dwChkSum;
			RecordHackLogFile(lpPlayInfo, &smTransCommandEx);

			ErrorFlag++;

			pSkinChangeItem_Server->DesSkinChangeItem.sItemInfo.CODE = 0;
		}

		SaveThrowItem(lpPlayInfo, pSkinChangeItem_Server->DesSkinChangeItem.sItemInfo.CODE,
			pSkinChangeItem_Server->DesSkinChangeItem.sItemInfo.ItemHeader.Head,
			pSkinChangeItem_Server->DesSkinChangeItem.sItemInfo.ItemHeader.dwChkSum);
	}


	if(pSkinChangeItem_Server->dwSkinChangeStoneItemCode)
	{
		SkinClass = pSkinChangeItem_Server->dwSkinChangeStoneItemCode;

		if (rsDeleteInvenItem(lpPlayInfo, pSkinChangeItem_Server->dwSkinChangeStoneItemCode,
			pSkinChangeItem_Server->dwSkinChangeStoneHead, pSkinChangeItem_Server->dwSkinChangeStoneCheckSum) < 0)
		{

			smTransCommandEx.WParam = 8000;
			smTransCommandEx.WxParam = 50;
			smTransCommandEx.LxParam = (int)"*Skin Change*";
			smTransCommandEx.LParam = pSkinChangeItem_Server->dwSkinChangeStoneItemCode;
			smTransCommandEx.SParam = pSkinChangeItem_Server->dwSkinChangeStoneHead;
			smTransCommandEx.EParam = pSkinChangeItem_Server->dwSkinChangeStoneCheckSum;
			RecordHackLogFile(lpPlayInfo, &smTransCommandEx);

			ErrorFlag++;

			pSkinChangeItem_Server->dwSkinChangeStoneItemCode = 0;
		}

		SaveThrowItem(lpPlayInfo, pSkinChangeItem_Server->dwSkinChangeStoneItemCode,
			pSkinChangeItem_Server->dwSkinChangeStoneHead, pSkinChangeItem_Server->dwSkinChangeStoneCheckSum);
	}


	DWORD dwResultItemCode = 0;
	dwResultItemCode = pSkinChangeItem_Server->DesSkinChangeItem.sItemInfo.CODE;



	DWORD dwSpecialJob = 0;
	dwSpecialJob = pSkinChangeItem_Server->DesSkinChangeItem.sItemInfo.JobCodeMask;

	int	i;
	DWORD dwJobCode = 0;
	JOB_DATA_CODE* pJobDataCode = 0;


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
			SkinSuccess = TRUE;
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

	ccItem.sinRecvSkinChange(pSkinChangeItem_Server);

	if(SkinSuccess)
		sSkinChange_Recv.Result = TRUE;


	if(ErrorFlag)
		sSkinChange_Recv.Result = FALSE;

	if(sSkinChange_Recv.Result == TRUE)
	{
		ReformItemSkin(&sSkinChange_Recv.DesSkinChangeItem.sItemInfo, SkinClass);
		sSkinChange_Recv.size = sizeof(sITEM) + 32;


		//rsRecord_MixtureResetItemLOG(lpPlayInfo, pSkinChangeItem_Server, ITEMLOG_MIXTURE_RESET_LOST);


		rsAddInvenItem(lpPlayInfo, sSkinChange_Recv.DesSkinChangeItem.sItemInfo.CODE,
			sSkinChange_Recv.DesSkinChangeItem.sItemInfo.ItemHeader.Head,
			sSkinChange_Recv.DesSkinChangeItem.sItemInfo.ItemHeader.dwChkSum);


		rsRecord_ItemLog2(lpPlayInfo, sSkinChange_Recv.DesSkinChangeItem.sItemInfo.CODE, sSkinChange_Recv.DesSkinChangeItem.sItemInfo.ItemHeader.Head,
			sSkinChange_Recv.DesSkinChangeItem.sItemInfo.ItemHeader.dwChkSum, ITEMLOG_MIXTURE_RESET_GET);


		RecordMixtureResetItem(lpPlayInfo, &sSkinChange_Recv.DesSkinChangeItem.sItemInfo);
	}
	else
	{
		sSkinChange_Recv.size = 32;
		//rsRecord_MixtureResetItemLOG(lpPlayInfo, pSkinChangeItem_Server, ITEMLOG_MIXTURE_RESET_FAIL);
	}


	sSkinChange_Recv.code = OpCode::OPCODE_SKIN_CHANGE_ITEM;

	if(lpPlayInfo && lpPlayInfo->lpsmSock)
	{
		lpPlayInfo->lpsmSock->Send((char*)&sSkinChange_Recv, sSkinChange_Recv.size, TRUE);
	}


	return TRUE;
}

int rsRecvWingItem(rsPLAYINFO *lpPlayInfo, sCRAFTITEM_SERVER *pCraftItem_Server)
{

	sITEMINFO	sItemInfo;
	smTRANS_COMMAND	smTransCommand;
	smTRANS_COMMAND_EX	smTransCommandEx;

	int cnt;
	int	ErrorFlag;
	int	PotionCount = 0;
	int result;
	int	Money;
	int	ForceMode = 0;

	ErrorFlag = 0;

	if(pCraftItem_Server->DesCraftItem.CODE == 1000)
		ForceMode = TRUE;


	if(ForceMode)
		rsRecord_ItemLost(lpPlayInfo, pCraftItem_Server->SheltomCode, pCraftItem_Server->Head, pCraftItem_Server->CheckSum, ITEMLOG_LOST_ALCHMY);
	else

		rsRecord_ItemLost(lpPlayInfo, pCraftItem_Server->SheltomCode, pCraftItem_Server->Head, pCraftItem_Server->CheckSum, ITEMLOG_WINGLOST);

	for(cnt = 0; cnt < 12; cnt++)
	{

		if(pCraftItem_Server->SheltomCode[cnt])
		{


			if(rsDeleteInvenItem(lpPlayInfo, pCraftItem_Server->SheltomCode[cnt],
				pCraftItem_Server->Head[cnt], pCraftItem_Server->CheckSum[cnt]) < 0)
			{


				smTransCommandEx.WParam = 8000;
				smTransCommandEx.WxParam = 50;
				smTransCommandEx.LxParam = (int)"*WING CRISTAL*";
				smTransCommandEx.LParam = pCraftItem_Server->SheltomCode[cnt];
				smTransCommandEx.SParam = pCraftItem_Server->Head[cnt];
				smTransCommandEx.EParam = pCraftItem_Server->CheckSum[cnt];
				RecordHackLogFile(lpPlayInfo, &smTransCommandEx);
				ErrorFlag++;
			}

			SaveThrowItem(lpPlayInfo, pCraftItem_Server->SheltomCode[cnt],
				pCraftItem_Server->Head[cnt], pCraftItem_Server->CheckSum[cnt]);
		}
	}

	memcpy(&sItemInfo, &pCraftItem_Server->DesCraftItem.sItemInfo, sizeof(sITEMINFO));
	Money = pCraftItem_Server->Money;


	if(sItemInfo.CODE && CheckItemForm(&sItemInfo) == FALSE)
	{


		pCraftItem_Server->Result = FALSE;
		pCraftItem_Server->size = 32;
		if(lpPlayInfo && lpPlayInfo->lpsmSock)
		{
			lpPlayInfo->lpsmSock->Send((char *)pCraftItem_Server, pCraftItem_Server->size, TRUE);
		}


		smTransCommand.WParam = 99000;
		smTransCommand.SParam = 1;
		smTransCommand.LParam = sItemInfo.CODE;
		RecordHackLogFile(lpPlayInfo, &smTransCommand);

		return TRUE;
	}


	rsDeleteInvenItem(lpPlayInfo, pCraftItem_Server->DesCraftItem.sItemInfo.CODE,
		pCraftItem_Server->DesCraftItem.sItemInfo.ItemHeader.Head,
		pCraftItem_Server->DesCraftItem.sItemInfo.ItemHeader.dwChkSum);


	result = 0;

	if(!ErrorFlag)
		result = sinWingItemQuestServer(pCraftItem_Server, &lpPlayInfo->smCharInfo);
	else
		pCraftItem_Server->Result = FALSE;

	if(result == 0x100)
	{

		TRANS_ITEMINFO_GROUP	TransItemGroup;
		TRANS_ITEMINFO_GROUP2	TransItemGroup2;
		int cnt2, ItemCnt, LenSize;

		if(lpPlayInfo->ServerMoney >= Money)
		{
			(void)gameplay::commerce::legacy::ServerLegacyWalletAdapter().Debit(
				gameplay::player::legacy::ServerLegacyOnlinePlayerRegistry().IdOf(lpPlayInfo),
				Money, gameplay::commerce::WalletReason::ForceOrb);
			CurrencyLog.int64_ForceOrbMoney += Money;
		}
		else
		{

			smTransCommandEx.WParam = 8000;
			smTransCommandEx.WxParam = 50;
			smTransCommandEx.LxParam = (int)"*FORCE MONEY ERROR*";
			smTransCommandEx.LParam = (sinGG1 | sin01);
			smTransCommandEx.SParam = Money;
			smTransCommandEx.EParam = lpPlayInfo->ServerMoney;
			RecordHackLogFile(lpPlayInfo, &smTransCommandEx);
			return FALSE;
		}

		ZeroMemory(&TransItemGroup2, sizeof(TRANS_ITEMINFO_GROUP2));

		ItemCnt = 0;

		for(cnt = 0; cnt < 12; cnt++)
		{
			if(pCraftItem_Server->SheltomCode[cnt])
			{
				for(cnt2 = 0; cnt2 < DefaultItemCount; cnt2++)
				{
					if(DefaultItems[cnt2].Item.CODE == pCraftItem_Server->SheltomCode[cnt] && !DefaultItems[cnt2].Item.UniqueItem)
					{
						CreateDefItem(&TransItemGroup.sItemInfo[ItemCnt], &DefaultItems[cnt2]);

						rsAddInvenItem(lpPlayInfo, TransItemGroup.sItemInfo[ItemCnt].CODE,
							TransItemGroup.sItemInfo[ItemCnt].ItemHeader.Head,
							TransItemGroup.sItemInfo[ItemCnt].ItemHeader.dwChkSum);


						rsRecord_ItemLog2(lpPlayInfo,
							TransItemGroup.sItemInfo[ItemCnt].CODE,
							TransItemGroup.sItemInfo[ItemCnt].ItemHeader.Head,
							TransItemGroup.sItemInfo[ItemCnt].ItemHeader.dwChkSum,
							ITEMLOG_GET_ALCHMY);

						ItemCnt++;
						break;
					}
				}
			}
		}

		LenSize = EecodeCompress((BYTE *)TransItemGroup.sItemInfo, (BYTE *)TransItemGroup2.szBuff, sizeof(sITEMINFO)*ItemCnt, sizeof(sITEMINFO)*TRANS_GROUPITEM_MAX);
		TransItemGroup2.ItemCount = ItemCnt;
		TransItemGroup2.Result = pCraftItem_Server->Result;
		TransItemGroup2.size = LenSize + TRANS_GROUPITEM_HEADER_SIZE;
		TransItemGroup2.code = OpCode::OPCODE_ITEM_FORCEORB;

		if(lpPlayInfo && lpPlayInfo->lpsmSock)
		{
			lpPlayInfo->lpsmSock->Send((char *)&TransItemGroup2, TransItemGroup2.size, TRUE);
		}

		return TRUE;
	}

	if(pCraftItem_Server->Result == SIN_WINGRESULT_CRISTAL)
	{

		bSql_EventJoin(lpPlayInfo);
		pCraftItem_Server->Result = TRUE;
	}

	if(pCraftItem_Server->Result == TRUE && pCraftItem_Server->DesCraftItem.sItemInfo.CODE && !ErrorFlag)
	{


		if(pCraftItem_Server->DesCraftItem.sItemInfo.ItemKindCode == ITEM_KIND_QUEST_WEAPON)
		{

			for(cnt = 0; cnt < DefaultItemCount; cnt++)
			{
				if(DefaultItems[cnt].Item.CODE == pCraftItem_Server->DesCraftItem.sItemInfo.CODE && DefaultItems[cnt].Item.UniqueItem == ITEM_UNIQUE_CODE_QUEST)
				{
					CreateDefItem(&pCraftItem_Server->DesCraftItem.sItemInfo, &DefaultItems[cnt]);
					pCraftItem_Server->DesCraftItem.sItemInfo.ItemKindCode = ITEM_KIND_QUEST_WEAPON;
					SetChangeJob3QuestItem(&pCraftItem_Server->DesCraftItem.sItemInfo);
					break;
				}
			}
		}
		else
		{
			for(cnt = 0; cnt < DefaultItemCount; cnt++)
			{
				if(DefaultItems[cnt].Item.CODE == pCraftItem_Server->DesCraftItem.sItemInfo.CODE && !DefaultItems[cnt].Item.UniqueItem)
				{

					if(DefaultItems[cnt].sGenDay[0] && DefaultItems[cnt].sGenDay[1] >= DefaultItems[cnt].sGenDay[0])
					{


						pCraftItem_Server->DesCraftItem.sItemInfo.CODE = (sinPM1 | sin04);
						pCraftItem_Server->DesCraftItem.sItemInfo.PotionCount = 30;


#ifdef	_TEST_SERVER
						if(lpPlayInfo->getSecurity() > AccountTypes::SEC_VIP)
						{
							TRANS_CHATMESSAGE	TransChatMessage;
							wsprintf(TransChatMessage.szMessage, " ߻Ҽ ִ ۼ ʰ߽ϴ(%d/%d)",
								DefaultItems[cnt].sGenDay[1], DefaultItems[cnt].sGenDay[0]);

							TransChatMessage.code = OpCode::OPCODE_WHISPERMESSAGE;
							TransChatMessage.size = 32 + lstrlen(TransChatMessage.szMessage);
							TransChatMessage.dwIP = 0;
							TransChatMessage.dwObjectSerial = 0;

							lpPlayInfo->lpsmSock->Send((char *)&TransChatMessage, TransChatMessage.size, TRUE);
						}
#endif


						for(cnt = 0; cnt < DefaultItemCount; cnt++)
						{
							if(DefaultItems[cnt].Item.CODE == pCraftItem_Server->DesCraftItem.sItemInfo.CODE)
							{
								break;
							}
						}
						if(cnt >= DefaultItemCount)
							return FALSE;
					}

					DefaultItems[cnt].sGenDay[1]++;

					if((DefaultItems[cnt].Item.CODE&sinITEM_MASK1) == (sinPM1&sinITEM_MASK1))
					{

						PotionCount = pCraftItem_Server->DesCraftItem.sItemInfo.PotionCount;
						CreateDefItem(&pCraftItem_Server->DesCraftItem.sItemInfo, &DefaultItems[cnt]);
						pCraftItem_Server->DesCraftItem.sItemInfo.PotionCount = PotionCount;

						rsAddServerPotion(lpPlayInfo, pCraftItem_Server->DesCraftItem.sItemInfo.CODE, PotionCount);
					}
					else
					{
						CreateDefItem(&pCraftItem_Server->DesCraftItem.sItemInfo, &DefaultItems[cnt], -1);
					}
					break;
				}
			}
		}
		if(cnt < DefaultItemCount)
		{



			ReformItem(&pCraftItem_Server->DesCraftItem.sItemInfo);

			rsAddInvenItem(lpPlayInfo, pCraftItem_Server->DesCraftItem.sItemInfo.CODE,
				pCraftItem_Server->DesCraftItem.sItemInfo.ItemHeader.Head,
				pCraftItem_Server->DesCraftItem.sItemInfo.ItemHeader.dwChkSum);


			rsRecord_ItemLog2(lpPlayInfo,
				pCraftItem_Server->DesCraftItem.sItemInfo.CODE,
				pCraftItem_Server->DesCraftItem.sItemInfo.ItemHeader.Head,
				pCraftItem_Server->DesCraftItem.sItemInfo.ItemHeader.dwChkSum,
				ITEMLOG_GETWING);

			pCraftItem_Server->size = sizeof(sCRAFTITEM_SERVER);
			if(lpPlayInfo && lpPlayInfo->lpsmSock)
			{
				lpPlayInfo->lpsmSock->Send((char *)pCraftItem_Server, pCraftItem_Server->size, TRUE);
			}

			return TRUE;
		}
	}




	smTransCommand.WParam = 99000;
	smTransCommand.SParam = 1;
	smTransCommand.LParam = sItemInfo.CODE;
	RecordHackLogFile(lpPlayInfo, &smTransCommand);

	pCraftItem_Server->size = sizeof(sCRAFTITEM_SERVER);;
	pCraftItem_Server->Result = FALSE;

	if(lpPlayInfo && lpPlayInfo->lpsmSock)
	{
		lpPlayInfo->lpsmSock->Send((char *)pCraftItem_Server, pCraftItem_Server->size, TRUE);
	}

	return TRUE;
}

int rsRecvMakeLinkCore(rsPLAYINFO *lpPlayInfo, TRANS_ITEMINFO *lpTransItemInfo)
{
	int len, len2;

	if((!lpPlayInfo->getName() || !lpPlayInfo->getName()[0]) || CheckItemForm(&lpTransItemInfo->Item) == FALSE) return FALSE;


	if(rsDeleteInvenItem(lpPlayInfo, lpTransItemInfo->Item.CODE, lpTransItemInfo->Item.ItemHeader.Head, lpTransItemInfo->Item.ItemHeader.dwChkSum) < 0)
		return FALSE;


	lpTransItemInfo->Item.BackUpKey = lpTransItemInfo->Item.ItemHeader.Head;
	lpTransItemInfo->Item.BackUpChkSum = lpTransItemInfo->Item.ItemHeader.dwChkSum;

	len = lstrlen(lpTransItemInfo->Item.ItemName) + 1;
	len2 = strlen(lpPlayInfo->getName());

	if((len + len2) > 32)
	{
		return FALSE;
	}

	strcpy_s(lpTransItemInfo->Item.ItemName + len, sizeof(lpTransItemInfo->Item.ItemName) - len, lpPlayInfo->getName());

	ReformItem(&lpTransItemInfo->Item);


	rsAddInvenItem(lpPlayInfo, lpTransItemInfo->Item.CODE, lpTransItemInfo->Item.ItemHeader.Head, lpTransItemInfo->Item.ItemHeader.dwChkSum);

	lpPlayInfo->lpsmSock->Send((char *)lpTransItemInfo, lpTransItemInfo->size, TRUE);

	return TRUE;
}

int rsRecvUseLinkCore(rsPLAYINFO *lpPlayInfo, TRANS_ITEMINFO *lpTransItemInfo)
{
	char *szName;
	int len;
	rsPLAYINFO *lpPlayInfo2;
	TRANS_CHATMESSAGE	TransChatMessage;
	smTRANS_COMMAND_EX	smTransCommandEx;
	STG_AREA	*lpStgArea;

	if(!lpPlayInfo->lpsmSock) return FALSE;
	if(CheckItemForm(&lpTransItemInfo->Item) == FALSE) return FALSE;


	lpTransItemInfo->Item.ItemName[31] = 0;
	len = lstrlen(lpTransItemInfo->Item.ItemName);

	szName = &lpTransItemInfo->Item.ItemName[len + 1];
	if(!szName[0]) return FALSE;

	lpPlayInfo2 = FindUserFromName(szName);

	gameplay::social::clan::ClanMembershipSnapshot sourceClan;
	gameplay::social::clan::ClanMembershipSnapshot targetClan;
	const gameplay::social::clan::legacy::LegacyClanQueryAdapter& clanQuery =
		gameplay::social::clan::legacy::ServerLegacyClanQueryAdapter();
	const bool sourceClanKnown = clanQuery.TryGetMembership(
		gameplay::player::legacy::ServerLegacyOnlinePlayerRegistry().IdOf(lpPlayInfo), sourceClan);
	const bool targetClanKnown = lpPlayInfo2 && clanQuery.TryGetMembership(
		gameplay::player::legacy::ServerLegacyOnlinePlayerRegistry().IdOf(lpPlayInfo2), targetClan);

	if(!lpPlayInfo2 || !lpPlayInfo2->dwObjectSerial || !sourceClanKnown || !sourceClan.IsMember() || lpPlayInfo->EventMode || lpPlayInfo2->EventMode ||
		rsCheckHardCoreEventArea(lpPlayInfo2->Position.x, lpPlayInfo2->Position.y, lpPlayInfo2->Position.z) == TRUE ||
		lpPlayInfo->Position.Area == QUEST_ARENA_FIELD || lpPlayInfo2->Position.Area == rsCASTLE_FIELD)
	{

		strcpy_s(TransChatMessage.szMessage, szNotLinkCore);
		TransChatMessage.code = OpCode::OPCODE_MESSAGEBOX;
		TransChatMessage.dwIP = 0;
		TransChatMessage.dwObjectSerial = 0;
		TransChatMessage.size = 32 + lstrlen(TransChatMessage.szMessage);

		lpPlayInfo->lpsmSock->Send((char *)&TransChatMessage, TransChatMessage.size, TRUE);
		return FALSE;
	}

	if(!targetClanKnown || !gameplay::social::clan::SameClan(sourceClan, targetClan))
	{
		strcpy_s(TransChatMessage.szMessage, szNotLinkCore2);
		TransChatMessage.code = OpCode::OPCODE_MESSAGEBOX;
		TransChatMessage.dwIP = 0;
		TransChatMessage.dwObjectSerial = 0;
		TransChatMessage.size = 32 + lstrlen(TransChatMessage.szMessage);

		lpPlayInfo->lpsmSock->Send((char *)&TransChatMessage, TransChatMessage.size, TRUE);
		return FALSE;
	}

	lpStgArea = FindStageField(lpPlayInfo2->Position.x, lpPlayInfo2->Position.z);

	if(lpStgArea)
	{

		if(lpStgArea->lpField)
		{
			if(lpPlayInfo->smCharInfo.Level < FieldLimitLevel_Table[lpStgArea->lpField->FieldCode] ||
				lpStgArea->lpField->FieldCode == QUEST_ARENA_FIELD)
			{

				strcpy_s(TransChatMessage.szMessage, szNotLinkCore3);
				TransChatMessage.code = OpCode::OPCODE_MESSAGEBOX;
				TransChatMessage.dwIP = 0;
				TransChatMessage.dwObjectSerial = 0;
				TransChatMessage.size = 32 + lstrlen(TransChatMessage.szMessage);

				lpPlayInfo->lpsmSock->Send((char *)&TransChatMessage, TransChatMessage.size, TRUE);
				return FALSE;
			}
		}

		smTransCommandEx.code = OpCode::OPCODE_USE_LINKCORE;
		smTransCommandEx.size = sizeof(smTRANS_COMMAND_EX);
		smTransCommandEx.WxParam = lpTransItemInfo->Item.CODE;
		smTransCommandEx.LxParam = lpTransItemInfo->Item.ItemHeader.Head;
		smTransCommandEx.SxParam = lpTransItemInfo->Item.ItemHeader.dwChkSum;
		smTransCommandEx.ExParam = 0;
		smTransCommandEx.WParam = lpPlayInfo2->Position.x;
		smTransCommandEx.LParam = lpPlayInfo2->Position.y;
		smTransCommandEx.SParam = lpPlayInfo2->Position.z;
		smTransCommandEx.EParam = lpStgArea->lpField->FieldCode;

		lpPlayInfo->lpsmSock->Send((char *)&smTransCommandEx, smTransCommandEx.size, TRUE);
		return TRUE;
	}

	return FALSE;
}
