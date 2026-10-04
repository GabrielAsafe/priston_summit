#include "gameplay/commerce/v2/AgingResultLogPolicy.h"

// AMBER ownership extraction from the frozen OnServer monolith.
// Byte-equivalent Aging kernels remain in the same server translation unit.
// New orchestration belongs behind IAgingService.

int rsRecvAgingUpgradeItem(rsPLAYINFO	*lpPlayInfo, TRANS_ITEMINFO *lpTransItemInfo)
{
	sITEMINFO sItemInfo;

	if (CheckItemForm(&lpTransItemInfo->Item) == FALSE) return FALSE;


	if (rsDeleteInvenItem(lpPlayInfo, lpTransItemInfo->Item.CODE, lpTransItemInfo->Item.ItemHeader.Head, lpTransItemInfo->Item.ItemHeader.dwChkSum) < 0)
		return FALSE;


	memcpy(&sItemInfo, &lpTransItemInfo->Item, sizeof(sITEMINFO));

	lpTransItemInfo->Item.dwCreateTime = 0;
	sinSetAgingItemIncreState(&lpTransItemInfo->Item);


	rsAddInvenItem(lpPlayInfo, lpTransItemInfo->Item.CODE, lpTransItemInfo->Item.ItemHeader.Head, lpTransItemInfo->Item.ItemHeader.dwChkSum);


	rsSaveAgingItemToHDD(&lpTransItemInfo->Item);


	RecordAgingItem(lpPlayInfo, &sItemInfo, &lpTransItemInfo->Item, TRUE);


	rsRecord_ItemLog_Change(lpPlayInfo, sItemInfo.CODE, sItemInfo.ItemHeader.Head, sItemInfo.ItemHeader.dwChkSum,
		lpTransItemInfo->Item.CODE, lpTransItemInfo->Item.ItemHeader.Head, lpTransItemInfo->Item.ItemHeader.dwChkSum, ITEMLOG_AGINGUP);


	return lpPlayInfo->lpsmSock->Send((char *)lpTransItemInfo, lpTransItemInfo->size, TRUE);
}

int rsRecvAgingItem(rsPLAYINFO *lpPlayInfo, sCRAFTITEM_SERVER *pCraftItem_Server)
{
	cAGING		ccItem;
	sITEMINFO	sItemInfo;
	smTRANS_COMMAND	smTransCommand;
	smTRANS_COMMAND_EX	smTransCommandEx;

	int cnt;
	int	ErrorFlag;
	int Price;
	int	QuestMode;
	int	AgingCnt, AgingLog;






	ErrorFlag = 0;


	rsRecord_ItemLost(lpPlayInfo, pCraftItem_Server->SheltomCode, pCraftItem_Server->Head, pCraftItem_Server->CheckSum, ITEMLOG_AGINGLOST);

	for(cnt = 0; cnt < 12; cnt++)
	{

		if(pCraftItem_Server->SheltomCode[cnt])
		{


			if(rsDeleteInvenItem(lpPlayInfo, pCraftItem_Server->SheltomCode[cnt],
				pCraftItem_Server->Head[cnt], pCraftItem_Server->CheckSum[cnt]) < 0)
			{


				smTransCommandEx.WParam = 8000;
				smTransCommandEx.WxParam = 70;
				smTransCommandEx.LxParam = (int)"*AGING SHELTON*";
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


#ifdef AGING_MONEY_FREE
	Price = 0;
#else
	Price = ((sItemInfo.ItemAgingNum[0] + 1) * sItemInfo.Price) / 2;
#endif

		Price = 0;

	CurrencyLog.int64_AgingMoney += Price;

	if(rsServerConfig.ServerInventoryMode && sItemInfo.ItemKindCode != ITEM_KIND_QUEST_WEAPON)
	{
		if(lpPlayInfo->ServerMoney < Price)
		{
			smTransCommand.WParam = 8010;
			smTransCommand.LParam = 70;
			smTransCommand.SParam = lpPlayInfo->ServerMoney;
			smTransCommand.EParam = Price;
			RecordHackLogFile(lpPlayInfo, &smTransCommand);

			lpPlayInfo->SetServerMoney(0, WHERE_AGINGITEM);
			ErrorFlag++;
		}
		else
		{
			lpPlayInfo->SubServerMoney(Price, WHERE_AGINGITEM);
			lpPlayInfo->UnsaveMoney = lpPlayInfo->ServerMoney;
		}
	}

	if(CheckItemForm(&sItemInfo) == FALSE || CheckMixItem(&sItemInfo) == FALSE)
	{


		sAging_Recv.Result = FALSE;
		sAging_Recv.size = 32;
		if(lpPlayInfo && lpPlayInfo->lpsmSock)
		{
			lpPlayInfo->lpsmSock->Send((char *)&sAging_Recv, sAging_Recv.size, TRUE);
		}


		smTransCommand.WParam = 99000;
		smTransCommand.SParam = 0;
		smTransCommand.LParam = 1;
		RecordHackLogFile(lpPlayInfo, &smTransCommand);

		return TRUE;
	}

	if((sItemInfo.CODE&sinITEM_MASK2) == sinMA1)
		QuestMode = TRUE;
	else
	{
		if(lpPlayInfo->getSecurity() >= AccountTypes::SEC_MODERATOR)
			QuestMode = -1;
		else
			QuestMode = FALSE;
	}


	int pedraDireto = 0;

	if((pCraftItem_Server->A_StoneItemCode == (sinBI2 | sin73)))
	{
		pedraDireto = 1;
	}

	if ((pCraftItem_Server->A_StoneItemCode == (sinBI2 | sin74)))
	{
		pedraDireto = 2;
	}

	if ((pCraftItem_Server->A_StoneItemCode == (sinBI2 | sin75)))
	{
		pedraDireto = 3;
	}

	if ((pCraftItem_Server->A_StoneItemCode == (sinBI2 | sin76)))
	{
		pedraDireto = 4;
	}
	if ((pCraftItem_Server->A_StoneItemCode == (sinBI2 | sin77)))
	{
		pedraDireto = 5;
	}

	if(pCraftItem_Server->A_StoneItemCode)
	{
		if((pCraftItem_Server->A_StoneItemCode == (sinBI1 | sin10) || pCraftItem_Server->A_StoneItemCode == (sinBI1 | sin11) ||
			pCraftItem_Server->A_StoneItemCode == (sinBI1 | sin60) || pCraftItem_Server->A_StoneItemCode == (sinBI1 | sin61) ||
			pCraftItem_Server->A_StoneItemCode == (sinBI1 | sin85)) &&
			rsDeleteInvenItem(lpPlayInfo,
				pCraftItem_Server->A_StoneItemCode,
				pCraftItem_Server->A_StoneHead,
				pCraftItem_Server->A_StoneCheckSum) >= 0)
		{

			rsRecord_ItemLog2(lpPlayInfo, pCraftItem_Server->A_StoneItemCode, pCraftItem_Server->A_StoneHead, pCraftItem_Server->A_StoneCheckSum, ITEMLOG_AGINGLOST);

			SaveThrowItem(lpPlayInfo,
				pCraftItem_Server->A_StoneItemCode, pCraftItem_Server->A_StoneHead, pCraftItem_Server->A_StoneCheckSum);
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
	{
		pCraftItem_Server->A_StoneItemCode = 0;
	}

	ccItem.sinRecvAgingItem(pCraftItem_Server);
	cnt = ccItem.AgingItem(QuestMode);

	//POSSIVEL CORRE√á√ÉO DA FALHA
	if(ErrorFlag) sAging_Recv.Result = TRUE;


	if(rsDeleteInvenItem(lpPlayInfo, sItemInfo.CODE, sItemInfo.ItemHeader.Head, sItemInfo.ItemHeader.dwChkSum) < 0)
	{

		sAging_Recv.size = 32;
		sAging_Recv.Result = FALSE;


		smTransCommandEx.WParam = 8000;
		smTransCommandEx.WxParam = 70;
		smTransCommandEx.LxParam = (int)sItemInfo.ItemName;
		smTransCommandEx.LParam = sItemInfo.CODE;
		smTransCommandEx.SParam = sItemInfo.ItemHeader.Head;
		smTransCommandEx.EParam = sItemInfo.ItemHeader.dwChkSum;
		RecordHackLogFile(lpPlayInfo, &smTransCommandEx);
	}

	sAging_Recv.code = OpCode::OPCODE_AGINGITEM;
	if(sAging_Recv.Result)
	{


		if(QuestMode == TRUE)
		{
			for(cnt = 0; cnt < DefaultItemCount; cnt++)
			{
				if(DefaultItems[cnt].Item.CODE == sAging_Recv.DesCraftItem.sItemInfo.CODE)
				{
					CreateDefItem(&sAging_Recv.DesCraftItem.sItemInfo, &DefaultItems[cnt]);
					break;
				}
			}
		}

		int atual;
		int age;

		switch (pedraDireto)
		{
		case 1:
			atual = sAging_Recv.DesCraftItem.sItemInfo.ItemAgingNum[0];
			age = PedraDireto1 - atual; // PEDRA DO AGING DIRETO

			SetAgingItem(&sAging_Recv.DesCraftItem.sItemInfo, age);
			break;
		case 2:
			atual = sAging_Recv.DesCraftItem.sItemInfo.ItemAgingNum[0];
			age = PedraDireto2 - atual; // PEDRA DO AGING DIRETO

			SetAgingItem(&sAging_Recv.DesCraftItem.sItemInfo, age);
			break;

		case 3:
			atual = sAging_Recv.DesCraftItem.sItemInfo.ItemAgingNum[0];
			age = PedraDireto3 - atual; // PEDRA DO AGING DIRETO

			SetAgingItem(&sAging_Recv.DesCraftItem.sItemInfo, age);
			break;

		case 4:
			atual = sAging_Recv.DesCraftItem.sItemInfo.ItemAgingNum[0];
			age = PedraDireto4 - atual; // PEDRA DO AGING DIRETO

			SetAgingItem(&sAging_Recv.DesCraftItem.sItemInfo, age);
			break;

		case 5:
			atual = sAging_Recv.DesCraftItem.sItemInfo.ItemAgingNum[0];
			age = PedraDireto5 - atual; // PEDRA DO AGING DIRETO

			SetAgingItem(&sAging_Recv.DesCraftItem.sItemInfo, age);
			break;

		case 6:
			atual = sAging_Recv.DesCraftItem.sItemInfo.ItemAgingNum[0];
			age = 22 - atual; // PEDRA DO AGING DIRETO

			SetAgingItem(&sAging_Recv.DesCraftItem.sItemInfo, age);
			break;

		case 7:
			atual = sAging_Recv.DesCraftItem.sItemInfo.ItemAgingNum[0];
			age = 23 - atual; // PEDRA DO AGING DIRETO

			SetAgingItem(&sAging_Recv.DesCraftItem.sItemInfo, age);
			break;

		case 8:
			atual = sAging_Recv.DesCraftItem.sItemInfo.ItemAgingNum[0];
			age = 24 - atual; // PEDRA DO AGING DIRETO

			SetAgingItem(&sAging_Recv.DesCraftItem.sItemInfo, age);
			break;

		case 9:
			atual = sAging_Recv.DesCraftItem.sItemInfo.ItemAgingNum[0];
			age = 25 - atual; // PEDRA DO AGING DIRETO

			SetAgingItem(&sAging_Recv.DesCraftItem.sItemInfo, age);
			break;

		case 10:
			atual = sAging_Recv.DesCraftItem.sItemInfo.ItemAgingNum[0];
			age = 26 - atual; // PEDRA DO AGING DIRETO

			SetAgingItem(&sAging_Recv.DesCraftItem.sItemInfo, age);
			break;
		}

		ReformItem(&sAging_Recv.DesCraftItem.sItemInfo);
		sAging_Recv.size = sizeof(sITEM) + 32;

		rsAddInvenItem(lpPlayInfo, sAging_Recv.DesCraftItem.sItemInfo.CODE,
			sAging_Recv.DesCraftItem.sItemInfo.ItemHeader.Head,
			sAging_Recv.DesCraftItem.sItemInfo.ItemHeader.dwChkSum);

		if (QuestMode != TRUE)
		{

			RecordAgingItem(lpPlayInfo, &sItemInfo, &sAging_Recv.DesCraftItem.sItemInfo, FALSE);

			AgingCnt = sAging_Recv.DesCraftItem.sItemInfo.ItemAgingNum[0] - sItemInfo.ItemAgingNum[0];
			gameplay::commerce::AgingResultLogConfig resultLogConfig;
			resultLogConfig.oneLevelDownLog = ITEMLOG_AGING_1DOWN;
			resultLogConfig.twoLevelsDownLog = ITEMLOG_AGING_2DOWN;
			resultLogConfig.successLog = ITEMLOG_AGING;
			resultLogConfig.twoLevelsUpLog = ITEMLOG_AGING_2UP;
			const gameplay::commerce::AgingResultLogPlan resultLogPlan =
				gameplay::commerce::AgingResultLogPolicy::Plan(
					sAging_Recv.Result,
					sAging_Recv.DesCraftItem.sItemInfo.ItemAgingNum[0],
					pedraDireto > 0, resultLogConfig);
			AgingLog = resultLogPlan.logCode;

			if(resultLogPlan.announce)
			{
				char szMsgAging[256];
				switch(resultLogPlan.announcement)
				{
				case gameplay::commerce::AgingAnnouncementKind::OneLevelDown:
					wsprintf(szMsgAging, "> Moriph: %s falhou no aging (+%d) no item (%s) (-1)!",
						lpPlayInfo->getName(), resultLogPlan.ageNumber, sAging_Recv.DesCraftItem.sItemInfo.ItemName);
					break;
				case gameplay::commerce::AgingAnnouncementKind::TwoLevelsDown:
					wsprintf(szMsgAging, "> Moriph: %s falhou no aging (+%d) no item (%s) (-2)!",
						lpPlayInfo->getName(), resultLogPlan.ageNumber, sAging_Recv.DesCraftItem.sItemInfo.ItemName);
					break;
				case gameplay::commerce::AgingAnnouncementKind::Success:
					wsprintf(szMsgAging, "> Moriph: %s conquistou o nÌvel (+%d) de aging no item (%s). gzzzz!",
						lpPlayInfo->getName(), resultLogPlan.ageNumber, sAging_Recv.DesCraftItem.sItemInfo.ItemName);
					break;
				case gameplay::commerce::AgingAnnouncementKind::TwoLevelsUp:
					wsprintf(szMsgAging, "> Moriph: %s conquistou +2 no nÌvel [+%d] de aging no item (%s). gzzzz!",
						lpPlayInfo->getName(), resultLogPlan.ageNumber, sAging_Recv.DesCraftItem.sItemInfo.ItemName);
					break;
				default:
					break;
				}
				rsSendMessageAll(szMsgAging, 3);
			}


			rsRecord_ItemLog_Change(lpPlayInfo, sItemInfo.CODE, sItemInfo.ItemHeader.Head, sItemInfo.ItemHeader.dwChkSum,
				sAging_Recv.DesCraftItem.sItemInfo.CODE, sAging_Recv.DesCraftItem.sItemInfo.ItemHeader.Head, sAging_Recv.DesCraftItem.sItemInfo.ItemHeader.dwChkSum, AgingLog);


			rsSaveAgingItemToHDD(&sAging_Recv.DesCraftItem.sItemInfo);
		}

		SaveThrowItem(lpPlayInfo, sItemInfo.CODE, sItemInfo.ItemHeader.Head, sItemInfo.ItemHeader.dwChkSum);

	}
	else
	{
		sAging_Recv.size = 32;

		if(QuestMode != TRUE)
		{

		    RecordAgingItem(lpPlayInfo, &sItemInfo, 0, FALSE);


		    rsRecord_ItemLog2(lpPlayInfo, sItemInfo.CODE, sItemInfo.ItemHeader.Head, sItemInfo.ItemHeader.dwChkSum, ITEMLOG_AGINGLOST);
		}
	}

	if(lpPlayInfo && lpPlayInfo->lpsmSock)
	{
	   lpPlayInfo->lpsmSock->Send((char *)&sAging_Recv, sAging_Recv.size, TRUE);
	}

	return TRUE;
}
