#include <cstring>
#include "systems/network/protocol/server/opcodes.h"
#include "core/server/legacy_sin_runtime/sinLinkHeader.h"
#include "modules/File/File.h"
#include "core/server/legacy_runtime/onserver.h"
#include "systems/progression/battle_pass/BattlePass.h"

#include <time.h>

int	rsRegist_ItemSecCode(rsPLAYINFO* lpPlayInfo, TRANS_ITEMINFO* lpTransItemInfo, int NewItem);
int CreateItem(psITEM* lpsItem, char* dwCode, int JobCode);
int	rsAddInvenItem(rsPLAYINFO* lpPlayInfo, DWORD dwCode, DWORD dwHead, DWORD dwChkSum);
int	rsSendMoneyToUser(rsPLAYINFO* lpPlayInfo, int Money);
DWORD GetServerTime();

//ENVIA AS INFORMAÇÕES DAS RECOMPENSAS DO PASSE AO CLIENTE
void BattlePass::SendItemToClient(rsPLAYINFO* Player)
{
	if (SQL::GetInstance()->IsConnected())
	{
		int i = 0;
		char szQuery[512];
		BP_PACK packet;
		ZeroMemory(&packet, sizeof(BP_PACK));
		packet.size = sizeof(BP_PACK);
		packet.code = BP_PACKET_SEND_ITEM_GAME;
		SQL::GetInstance()->EnterSqlSection();
		wsprintf(szQuery, "SELECT * FROM [BattlePass].[dbo].[BP_Item]");
		if (SQL::GetInstance()->Execute(szQuery))
		{
			while (SQL::GetInstance()->Fetch())
			{
				SQL::GetInstance()->GetDataValue(1, &packet.item[i].Tag);
				SQL::GetInstance()->GetDataValue(2, packet.item[i].itCode);
				SQL::GetInstance()->GetDataValue(3, packet.item[i].Name);
				SQL::GetInstance()->GetDataValue(4, &packet.item[i].Level_Req);
				SQL::GetInstance()->GetDataValue(5, &packet.item[i].Item_Qtd);

				for (int cnt2 = 0; cnt2 < MAX_ITEM; cnt2++)
				{
					if (strcmp(sItem[cnt2].LastCategory, packet.item[i].itCode) == 0)
					{
						packet.item[i].w = sItem[cnt2].w;
						packet.item[i].h = sItem[cnt2].h;
					}
				}

				i++;

				if (i > 69) break;
			}

			Player->lpsmSock->Send((char*)&packet, packet.size, true);
		}
		SQL::GetInstance()->LeaveSqlSection();
	}
}

//ENVIA AS INFORMAÇÕES DE EXPERIÊNCIA E NIVEL DO PASSE AO CLIENTE
void BattlePass::SendBPInfoToGame(rsPLAYINFO* Player)
{
	if (SQL::GetInstance()->IsConnected())
	{
		BP_INFO packet;
		char szQuery[512];
		SQL::GetInstance()->EnterSqlSection();
		wsprintf(szQuery, "SELECT * FROM [BattlePass].[dbo].[BP_Info] WHERE [ID]='%s'", Player->getAccount());
		if (SQL::GetInstance()->Execute(szQuery))
		{
			if (SQL::GetInstance()->Fetch())
			{
				SQL::GetInstance()->GetDataValue(2, &packet.exp);
				SQL::GetInstance()->GetDataValue(3, &packet.lvl);
				SQL::GetInstance()->GetDataValue(4, &packet.type);
				for (int i = 0; i < sizeof(packet.itemResg); i++) {
					SQL::GetInstance()->GetDataValue(5 + i, &packet.itemResg[i]);
				}
				packet.size = sizeof(BP_INFO);
				packet.code = BP_PACKET_SEND_BPINFO_GAME;
				if (Player->lpsmSock)
					Player->lpsmSock->Send((char*)&packet, packet.size, TRUE);
			}
			/*else
			{
			//DEEEEEEU EERRROOOOOOOO SAAAAAAH DESGRACAAAAAAAAAAAAAAAAA
				packet.size = sizeof(BP_INFO);
				packet.code = BP_PACKET_SEND_BPINFO_GAME;
				packet.exp = 0;
				packet.lvl = 0;
				for (int i = 0; i < sizeof(packet.itemResg); i++) {
					packet.itemResg[i] = 1;
				}
				if (Player->lpsmSock)
					Player->lpsmSock->Send((char *)&packet, packet.size, TRUE);
			}*/
		}
		SQL::GetInstance()->LeaveSqlSection();
	}
}


//RECEBE AS INFORMAÇÕES DO CLIENTE REFERENTE AO ITEM SELECIONADO PARA RESGATAR 
void BattlePass::RecvItemToClient(rsPLAYINFO* Player, BP_RESGATE* Shop)
{

	time_t mytime;
	mytime = time(NULL);
	struct tm tm = *localtime(&mytime);


	if (SQL::GetInstance()->IsConnected())
	{
		psITEM* lpsItem;
		lpsItem = new psITEM;
		int UserBP_Level = 0;
		int Item_Level_Req = 0;
		int Item_Info_Resgate = 1;
		int Item_Qtd = 0;
		int BP_Info = 0;
		int Item_Lock = 0;
		char CodItem[32];
		char szQuery[512];
		SQL::GetInstance()->EnterSqlSection();
		wsprintf(szQuery, "SELECT * FROM [BattlePass].[dbo].[BP_Info] WHERE [ID]='%s'", Player->getAccount());
		if (SQL::GetInstance()->Execute(szQuery))
		{
			if (SQL::GetInstance()->Fetch())
			{
				if (Shop->item.Tag < 0)
				{
					Shop->item.Tag = 0;
				}
				SQL::GetInstance()->GetDataValue(3, &UserBP_Level);
				SQL::GetInstance()->GetDataValue(4, &BP_Info);
				SQL::GetInstance()->GetDataValue(Shop->item.Tag + 5, &Item_Info_Resgate);
			}
			wsprintf(szQuery, "SELECT * FROM [BattlePass].[dbo].[BP_Item] WHERE [Tag]='%d'", Shop->item.Tag);
			if (SQL::GetInstance()->Execute(szQuery))
			{
				if (SQL::GetInstance()->Fetch())
				{
					SQL::GetInstance()->GetDataValue(2, CodItem);
					SQL::GetInstance()->GetDataValue(4, &Item_Level_Req);
					SQL::GetInstance()->GetDataValue(5, &Item_Qtd);
					SQL::GetInstance()->GetDataValue(6, &Item_Lock);

				}

			}
			//
			//
			if (((Shop->item.Tag % 2 == 0) && BP_Info > 0) || ((Shop->item.Tag % 2 > 0) && BP_Info == 2))
			{
				if (UserBP_Level >= Item_Level_Req)
				{
					if (Item_Info_Resgate == 0)
					{
						wsprintf(szQuery, "UPDATE [BattlePass].[dbo].[BP_Info] SET BP_Resg%d=1 WHERE [ID]='%s'", Shop->item.Tag + 1, Player->getAccount());
						if (SQL::GetInstance()->Execute(szQuery))
						{
							if (strcmp(CodItem, "GG101") == 0)
							{
								rsSendMoneyToUser(Player, Item_Qtd);

								TRANS_CHATMESSAGE	TransChatMessage;
								wsprintf(TransChatMessage.szMessage, "Voc\352 resgatou %d de %s.", Item_Qtd, Shop->item.Name);
								TransChatMessage.code = OpCode::OPCODE_WHISPERMESSAGE;
								TransChatMessage.size = 32 + lstrlen(TransChatMessage.szMessage);
								TransChatMessage.dwIP = 3;
								TransChatMessage.dwObjectSerial = 0;

								if (Player->lpsmSock)
									Player->lpsmSock->Send((char*)&TransChatMessage, TransChatMessage.size, TRUE);

								BattlePass::GetInstance()->SendItemToClient(Player);
								BattlePass::GetInstance()->SendBPInfoToGame(Player);
								SendOpenBattlePass(Player->lpsmSock);

								//Criar log na pasta do server do resgate do BP
								HANDLE	hFile;
								DWORD	dwAcess;
								DWORD	FileLength;
								char	szFileName[128];
								char	szBuff[256];
								char* lpMsg;
								char* HackLogDirectory = "LogFile";

								wsprintf(szFileName, "%s//(BattlePass) %d - %d.log", HackLogDirectory, tm.tm_mon + 1, tm.tm_mday);

								lpMsg = "%d:%d:%d - ID:( %s ) / Name:( %s ) / IP ( %s ) - Resgatou ( %d ) ( %s ) /r/n";
								wsprintf(szBuff, lpMsg,
									tm.tm_hour, tm.tm_min, tm.tm_sec, Player->getAccount(), Player->getName(),
									Player->lpsmSock->szIPAddr, Item_Qtd, Shop->item.Name);

								hFile = CreateFile(szFileName, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
								if (hFile == INVALID_HANDLE_VALUE)
								{
								}
								else {
									FileLength = GetFileSize(hFile, NULL);
									SetFilePointer(hFile, FileLength, NULL, FILE_BEGIN);
									WriteFile(hFile, szBuff, lstrlen(szBuff), &dwAcess, NULL);
									CloseHandle(hFile);
								}

							}
							else if (strcmp(CodItem, "COIN") == 0)
							{
								//Shop::GetInstance()->AddPlayerCoin(Player, Item_Qtd);

								TRANS_CHATMESSAGE	TransChatMessage;
								wsprintf(TransChatMessage.szMessage, "Voc\352 resgatou %d %s.", Item_Qtd, Shop->item.Name);
								TransChatMessage.code = OpCode::OPCODE_WHISPERMESSAGE;
								TransChatMessage.size = 32 + lstrlen(TransChatMessage.szMessage);
								TransChatMessage.dwIP = 3;
								TransChatMessage.dwObjectSerial = 0;

								if (Player->lpsmSock)
									Player->lpsmSock->Send((char*)&TransChatMessage, TransChatMessage.size, TRUE);

								BattlePass::GetInstance()->SendItemToClient(Player);
								BattlePass::GetInstance()->SendBPInfoToGame(Player);
								SendOpenBattlePass(Player->lpsmSock);

								//Criar log na pasta do server do resgate do BP
								HANDLE	hFile;
								DWORD	dwAcess;
								DWORD	FileLength;
								char	szFileName[128];
								char	szBuff[256];
								char* lpMsg;
								char* HackLogDirectory = "LogFile";

								wsprintf(szFileName, "%s//(BattlePass) %d - %d.log", HackLogDirectory, tm.tm_mon + 1, tm.tm_mday);

								lpMsg = "%d:%d:%d - ID:( %s ) / Name:( %s ) / IP ( %s ) - Resgatou ( %d ) ( %s ) /r/n";
								wsprintf(szBuff, lpMsg,
									tm.tm_hour, tm.tm_min, tm.tm_sec, Player->getAccount(), Player->getName(),
									Player->lpsmSock->szIPAddr, Item_Qtd, Shop->item.Name);

								hFile = CreateFile(szFileName, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
								if (hFile == INVALID_HANDLE_VALUE)
								{
								}
								else {
									FileLength = GetFileSize(hFile, NULL);
									SetFilePointer(hFile, FileLength, NULL, FILE_BEGIN);
									WriteFile(hFile, szBuff, lstrlen(szBuff), &dwAcess, NULL);
									CloseHandle(hFile);
								}
							}
							else if (strcmp(CodItem, "SB3H") == 0) //SORTEIO ITENS
							{
								char itemsorteada[512];
								char itemname[512];
								int Sorteioitem = GetRandomPos(0, 100);
								if (Sorteioitem > 90)
								{
									wsprintf(itemsorteada, "bi209");
									wsprintf(itemname, "Ba\372 ouro 3H");
								}
								else if (Sorteioitem > 55 && Sorteioitem <= 90)
								{
									wsprintf(itemsorteada, "bi205");
									wsprintf(itemname, "Ba\372 prata 3H");
								}
								else
								{
									wsprintf(itemsorteada, "bi201");
									wsprintf(itemname, "Ba\372 bronze 3H");
								}

								if (CreateItem(lpsItem, itemsorteada, Player->smCharInfo.JOB_CODE) == TRUE)
								{
									TRANS_ITEMINFO		TransItemInfo;
									ReformItem(&lpsItem->ItemInfo);
									TransItemInfo.code = OpCode::OPCODE_PUTITEM;
									TransItemInfo.size = sizeof(TRANS_ITEMINFO);
									memcpy(&TransItemInfo.Item, &lpsItem->ItemInfo, sizeof(sITEMINFO));
									rsRegist_ItemSecCode(Player, &TransItemInfo, 1);
									rsAddInvenItem(Player, TransItemInfo.Item.CODE, TransItemInfo.Item.ItemHeader.Head, TransItemInfo.Item.ItemHeader.dwChkSum);
									if (Player->lpsmSock)
										Player->lpsmSock->Send((char*)&TransItemInfo, TransItemInfo.size, TRUE);
								}

								TRANS_CHATMESSAGE	TransChatMessage;
								wsprintf(TransChatMessage.szMessage, "Voc\352 sorteou %d %s.", Item_Qtd, itemname);
								TransChatMessage.code = OpCode::OPCODE_WHISPERMESSAGE;
								TransChatMessage.size = 32 + lstrlen(TransChatMessage.szMessage);
								TransChatMessage.dwIP = 3;
								TransChatMessage.dwObjectSerial = 0;

								if (Player->lpsmSock)
									Player->lpsmSock->Send((char*)&TransChatMessage, TransChatMessage.size, TRUE);

								//Criar log na pasta do server do resgate do BP
								HANDLE	hFile;
								DWORD	dwAcess;
								DWORD	FileLength;
								char	szFileName[128];
								char	szBuff[256];
								char* lpMsg;
								char* HackLogDirectory = "LogFile";

								wsprintf(szFileName, "%s//(BattlePass) %d - %d.log", HackLogDirectory, tm.tm_mon + 1, tm.tm_mday);

								lpMsg = "%d:%d:%d - ID:( %s ) / Name:( %s ) / IP ( %s ) - Sorteou ( %d ) ( %s ) /r/n";
								wsprintf(szBuff, lpMsg,
									tm.tm_hour, tm.tm_min, tm.tm_sec, Player->getAccount(), Player->getName(),
									Player->lpsmSock->szIPAddr, Item_Qtd, itemname);

								hFile = CreateFile(szFileName, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
								if (hFile == INVALID_HANDLE_VALUE)
								{
								}
								else {
									FileLength = GetFileSize(hFile, NULL);
									SetFilePointer(hFile, FileLength, NULL, FILE_BEGIN);
									WriteFile(hFile, szBuff, lstrlen(szBuff), &dwAcess, NULL);
									CloseHandle(hFile);
								}
							}
							else if (strcmp(CodItem, "SB1D") == 0) //SORTEIO ITENS
							{
								char itemsorteada[512];
								char itemname[512];
								int Sorteioitem = GetRandomPos(0, 100);
								if (Sorteioitem > 90)
								{
									wsprintf(itemsorteada, "bi210");
									wsprintf(itemname, "Ba\372 ouro 1D");
								}
								else if (Sorteioitem > 55 && Sorteioitem <= 90)
								{
									wsprintf(itemsorteada, "bi206");
									wsprintf(itemname, "Ba\372 prata 1D");
								}
								else
								{
									wsprintf(itemsorteada, "bi202");
									wsprintf(itemname, "Ba\372 bronze 1D");
								}

								if (CreateItem(lpsItem, itemsorteada, Player->smCharInfo.JOB_CODE) == TRUE)
								{
									TRANS_ITEMINFO		TransItemInfo;
									ReformItem(&lpsItem->ItemInfo);
									TransItemInfo.code = OpCode::OPCODE_PUTITEM;
									TransItemInfo.size = sizeof(TRANS_ITEMINFO);
									memcpy(&TransItemInfo.Item, &lpsItem->ItemInfo, sizeof(sITEMINFO));
									rsRegist_ItemSecCode(Player, &TransItemInfo, 1);
									rsAddInvenItem(Player, TransItemInfo.Item.CODE, TransItemInfo.Item.ItemHeader.Head, TransItemInfo.Item.ItemHeader.dwChkSum);
									if (Player->lpsmSock)
										Player->lpsmSock->Send((char*)&TransItemInfo, TransItemInfo.size, TRUE);
								}

								TRANS_CHATMESSAGE	TransChatMessage;
								wsprintf(TransChatMessage.szMessage, "Voc\352 sorteou %d %s.", Item_Qtd, itemname);
								TransChatMessage.code = OpCode::OPCODE_WHISPERMESSAGE;
								TransChatMessage.size = 32 + lstrlen(TransChatMessage.szMessage);
								TransChatMessage.dwIP = 3;
								TransChatMessage.dwObjectSerial = 0;

								if (Player->lpsmSock)
									Player->lpsmSock->Send((char*)&TransChatMessage, TransChatMessage.size, TRUE);

								//Criar log na pasta do server do resgate do BP
								HANDLE	hFile;
								DWORD	dwAcess;
								DWORD	FileLength;
								char	szFileName[128];
								char	szBuff[256];
								char* lpMsg;
								char* HackLogDirectory = "LogFile";

								wsprintf(szFileName, "%s//(BattlePass) %d - %d.log", HackLogDirectory, tm.tm_mon + 1, tm.tm_mday);

								lpMsg = "%d:%d:%d - ID:( %s ) / Name:( %s ) / IP ( %s ) - Sorteou ( %d ) ( %s ) /r/n";
								wsprintf(szBuff, lpMsg,
									tm.tm_hour, tm.tm_min, tm.tm_sec, Player->getAccount(), Player->getName(),
									Player->lpsmSock->szIPAddr, Item_Qtd, itemname);

								hFile = CreateFile(szFileName, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
								if (hFile == INVALID_HANDLE_VALUE)
								{
								}
								else {
									FileLength = GetFileSize(hFile, NULL);
									SetFilePointer(hFile, FileLength, NULL, FILE_BEGIN);
									WriteFile(hFile, szBuff, lstrlen(szBuff), &dwAcess, NULL);
									CloseHandle(hFile);
								}
							}
							else if (strcmp(CodItem, "SB3D") == 0) //SORTEIO ITENS
							{
								char itemsorteada[512];
								char itemname[512];
								int Sorteioitem = GetRandomPos(0, 100);
								if (Sorteioitem > 90)
								{
									wsprintf(itemsorteada, "bi211");
									wsprintf(itemname, "Ba\372 ouro 3D");
								}
								else if (Sorteioitem > 55 && Sorteioitem <= 90)
								{
									wsprintf(itemsorteada, "bi207");
									wsprintf(itemname, "Ba\372 prata 3D");
								}
								else
								{
									wsprintf(itemsorteada, "bi203");
									wsprintf(itemname, "Ba\372 bronze 3D");
								}

								if (CreateItem(lpsItem, itemsorteada, Player->smCharInfo.JOB_CODE) == TRUE)
								{
									TRANS_ITEMINFO		TransItemInfo;
									ReformItem(&lpsItem->ItemInfo);
									TransItemInfo.code = OpCode::OPCODE_PUTITEM;
									TransItemInfo.size = sizeof(TRANS_ITEMINFO);
									memcpy(&TransItemInfo.Item, &lpsItem->ItemInfo, sizeof(sITEMINFO));
									rsRegist_ItemSecCode(Player, &TransItemInfo, 1);
									rsAddInvenItem(Player, TransItemInfo.Item.CODE, TransItemInfo.Item.ItemHeader.Head, TransItemInfo.Item.ItemHeader.dwChkSum);
									if (Player->lpsmSock)
										Player->lpsmSock->Send((char*)&TransItemInfo, TransItemInfo.size, TRUE);
								}

								TRANS_CHATMESSAGE	TransChatMessage;
								wsprintf(TransChatMessage.szMessage, "Voc\352 sorteou %d %s.", Item_Qtd, itemname);
								TransChatMessage.code = OpCode::OPCODE_WHISPERMESSAGE;
								TransChatMessage.size = 32 + lstrlen(TransChatMessage.szMessage);
								TransChatMessage.dwIP = 3;
								TransChatMessage.dwObjectSerial = 0;

								if (Player->lpsmSock)
									Player->lpsmSock->Send((char*)&TransChatMessage, TransChatMessage.size, TRUE);

								//Criar log na pasta do server do resgate do BP
								HANDLE	hFile;
								DWORD	dwAcess;
								DWORD	FileLength;
								char	szFileName[128];
								char	szBuff[256];
								char* lpMsg;
								char* HackLogDirectory = "LogFile";

								wsprintf(szFileName, "%s//(BattlePass) %d - %d.log", HackLogDirectory, tm.tm_mon + 1, tm.tm_mday);

								lpMsg = "%d:%d:%d - ID:( %s ) / Name:( %s ) / IP ( %s ) - Sorteou ( %d ) ( %s ) /r/n";
								wsprintf(szBuff, lpMsg,
									tm.tm_hour, tm.tm_min, tm.tm_sec, Player->getAccount(), Player->getName(),
									Player->lpsmSock->szIPAddr, Item_Qtd, itemname);

								hFile = CreateFile(szFileName, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
								if (hFile == INVALID_HANDLE_VALUE)
								{
								}
								else {
									FileLength = GetFileSize(hFile, NULL);
									SetFilePointer(hFile, FileLength, NULL, FILE_BEGIN);
									WriteFile(hFile, szBuff, lstrlen(szBuff), &dwAcess, NULL);
									CloseHandle(hFile);
								}
							}
							else if (strcmp(CodItem, "SP3H") == 0) //SORTEIO ITENS
							{
								char itemsorteada[512];
								char itemname[512];
								int Sorteioitem = GetRandomPos(0, 100);
								if (Sorteioitem > 90)
								{
									wsprintf(itemsorteada, "bi113");
									wsprintf(itemname, "Xp 3H");
								}
								else if (Sorteioitem > 70 && Sorteioitem <= 90)
								{
									wsprintf(itemsorteada, "bi121");
									wsprintf(itemname, "Drena 3H");
								}
								else if (Sorteioitem > 50 && Sorteioitem <= 70)
								{
									wsprintf(itemsorteada, "bi191");
									wsprintf(itemname, "RES Down 3H");
								}
								else if (Sorteioitem > 40 && Sorteioitem <= 50)
								{
									wsprintf(itemsorteada, "bi112");
									wsprintf(itemname, "Olho 3H");
								}
								else if (Sorteioitem > 20 && Sorteioitem <= 40)
								{
									wsprintf(itemsorteada, "bi119");
									wsprintf(itemname, "Presa 3H");
								}
								else
								{
									wsprintf(itemsorteada, "bi128");
									wsprintf(itemname, "Mana Down 3H");
								}

								if (CreateItem(lpsItem, itemsorteada, Player->smCharInfo.JOB_CODE) == TRUE)
								{
									TRANS_ITEMINFO		TransItemInfo;
									ReformItem(&lpsItem->ItemInfo);
									TransItemInfo.code = OpCode::OPCODE_PUTITEM;
									TransItemInfo.size = sizeof(TRANS_ITEMINFO);
									memcpy(&TransItemInfo.Item, &lpsItem->ItemInfo, sizeof(sITEMINFO));
									rsRegist_ItemSecCode(Player, &TransItemInfo, 1);
									rsAddInvenItem(Player, TransItemInfo.Item.CODE, TransItemInfo.Item.ItemHeader.Head, TransItemInfo.Item.ItemHeader.dwChkSum);
									if (Player->lpsmSock)
										Player->lpsmSock->Send((char*)&TransItemInfo, TransItemInfo.size, TRUE);
								}

								TRANS_CHATMESSAGE	TransChatMessage;
								wsprintf(TransChatMessage.szMessage, "Voc\352 sorteou %d %s.", Item_Qtd, itemname);
								TransChatMessage.code = OpCode::OPCODE_WHISPERMESSAGE;
								TransChatMessage.size = 32 + lstrlen(TransChatMessage.szMessage);
								TransChatMessage.dwIP = 3;
								TransChatMessage.dwObjectSerial = 0;

								if (Player->lpsmSock)
									Player->lpsmSock->Send((char*)&TransChatMessage, TransChatMessage.size, TRUE);

								//Criar log na pasta do server do resgate do BP
								HANDLE	hFile;
								DWORD	dwAcess;
								DWORD	FileLength;
								char	szFileName[128];
								char	szBuff[256];
								char* lpMsg;
								char* HackLogDirectory = "LogFile";

								wsprintf(szFileName, "%s//(BattlePass) %d - %d.log", HackLogDirectory, tm.tm_mon + 1, tm.tm_mday);

								lpMsg = "%d:%d:%d - ID:( %s ) / Name:( %s ) / IP ( %s ) - Sorteou ( %d ) ( %s ) /r/n";
								wsprintf(szBuff, lpMsg,
									tm.tm_hour, tm.tm_min, tm.tm_sec, Player->getAccount(), Player->getName(),
									Player->lpsmSock->szIPAddr, Item_Qtd, itemname);

								hFile = CreateFile(szFileName, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
								if (hFile == INVALID_HANDLE_VALUE)
								{
								}
								else {
									FileLength = GetFileSize(hFile, NULL);
									SetFilePointer(hFile, FileLength, NULL, FILE_BEGIN);
									WriteFile(hFile, szBuff, lstrlen(szBuff), &dwAcess, NULL);
									CloseHandle(hFile);
								}
							}
							else if (strcmp(CodItem, "SP3D") == 0) //SORTEIO ITENS
							{
								char itemsorteada[512];
								char itemname[512];
								int Sorteioitem = GetRandomPos(0, 100);
								if (Sorteioitem > 90)
								{
									wsprintf(itemsorteada, "bi115");
									wsprintf(itemname, "Xp 3D");
								}
								else if (Sorteioitem > 70 && Sorteioitem <= 90)
								{
									wsprintf(itemsorteada, "bi121");
									wsprintf(itemname, "Drena 3D");
								}
								else if (Sorteioitem > 50 && Sorteioitem <= 70)
								{
									wsprintf(itemsorteada, "bi192");
									wsprintf(itemname, "RES Down 3D");
								}
								else if (Sorteioitem > 40 && Sorteioitem <= 50)
								{
									wsprintf(itemsorteada, "bi114");
									wsprintf(itemname, "Olho 3D");
								}
								else if (Sorteioitem > 20 && Sorteioitem <= 40)
								{
									wsprintf(itemsorteada, "bi120");
									wsprintf(itemname, "Presa 3D");
								}
								else
								{
									wsprintf(itemsorteada, "bi129");
									wsprintf(itemname, "Mana Down 3D");
								}

								if (CreateItem(lpsItem, itemsorteada, Player->smCharInfo.JOB_CODE) == TRUE)
								{
									TRANS_ITEMINFO		TransItemInfo;
									ReformItem(&lpsItem->ItemInfo);
									TransItemInfo.code = OpCode::OPCODE_PUTITEM;
									TransItemInfo.size = sizeof(TRANS_ITEMINFO);
									memcpy(&TransItemInfo.Item, &lpsItem->ItemInfo, sizeof(sITEMINFO));
									rsRegist_ItemSecCode(Player, &TransItemInfo, 1);
									rsAddInvenItem(Player, TransItemInfo.Item.CODE, TransItemInfo.Item.ItemHeader.Head, TransItemInfo.Item.ItemHeader.dwChkSum);
									if (Player->lpsmSock)
										Player->lpsmSock->Send((char*)&TransItemInfo, TransItemInfo.size, TRUE);
								}

								TRANS_CHATMESSAGE	TransChatMessage;
								wsprintf(TransChatMessage.szMessage, "Voc\352 sorteou %d %s.", Item_Qtd, itemname);
								TransChatMessage.code = OpCode::OPCODE_WHISPERMESSAGE;
								TransChatMessage.size = 32 + lstrlen(TransChatMessage.szMessage);
								TransChatMessage.dwIP = 3;
								TransChatMessage.dwObjectSerial = 0;

								if (Player->lpsmSock)
									Player->lpsmSock->Send((char*)&TransChatMessage, TransChatMessage.size, TRUE);

								//Criar log na pasta do server do resgate do BP
								HANDLE	hFile;
								DWORD	dwAcess;
								DWORD	FileLength;
								char	szFileName[128];
								char	szBuff[256];
								char* lpMsg;
								char* HackLogDirectory = "LogFile";

								wsprintf(szFileName, "%s//(BattlePass) %d - %d.log", HackLogDirectory, tm.tm_mon + 1, tm.tm_mday);

								lpMsg = "%d:%d:%d - ID:( %s ) / Name:( %s ) / IP ( %s ) - Sorteou ( %d ) ( %s ) /r/n";
								wsprintf(szBuff, lpMsg,
									tm.tm_hour, tm.tm_min, tm.tm_sec, Player->getAccount(), Player->getName(),
									Player->lpsmSock->szIPAddr, Item_Qtd, itemname);

								hFile = CreateFile(szFileName, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
								if (hFile == INVALID_HANDLE_VALUE)
								{
								}
								else {
									FileLength = GetFileSize(hFile, NULL);
									SetFilePointer(hFile, FileLength, NULL, FILE_BEGIN);
									WriteFile(hFile, szBuff, lstrlen(szBuff), &dwAcess, NULL);
									CloseHandle(hFile);
								}
							}
							else if (strcmp(CodItem, "SS5") == 0) //SORTEIO ITENS
							{
								for (int i = 0; i < 5; i++)
								{
									char itemsorteada[512];
									char itemname[512];
									int Sorteioitem = GetRandomPos(0, 100);

									if (Sorteioitem > 97)
									{
										wsprintf(itemsorteada, "os114");
										wsprintf(itemname, "Ordo");
									}
									else if (Sorteioitem > 90 && Sorteioitem <= 97)
									{
										wsprintf(itemsorteada, "os113");
										wsprintf(itemname, "Bellum");
									}
									else if (Sorteioitem > 80 && Sorteioitem <= 90)
									{
										wsprintf(itemsorteada, "os112");
										wsprintf(itemname, "Enigma");
									}
									else if (Sorteioitem > 70 && Sorteioitem <= 80)
									{
										wsprintf(itemsorteada, "os111");
										wsprintf(itemname, "Inferna");
									}
									else if (Sorteioitem > 55 && Sorteioitem <= 70)
									{
										wsprintf(itemsorteada, "os110");
										wsprintf(itemname, "Mirage");
									}
									else if (Sorteioitem > 40 && Sorteioitem <= 55)
									{
										wsprintf(itemsorteada, "os109");
										wsprintf(itemname, "Celesto");
									}
									else if (Sorteioitem > 20 && Sorteioitem <= 40)
									{
										wsprintf(itemsorteada, "os108");
										wsprintf(itemname, "Devine");
									}
									else
									{
										wsprintf(itemsorteada, "os107");
										wsprintf(itemname, "Murky");
									}

									if (CreateItem(lpsItem, itemsorteada, Player->smCharInfo.JOB_CODE) == TRUE)
									{
										TRANS_ITEMINFO		TransItemInfo;
										ReformItem(&lpsItem->ItemInfo);
										TransItemInfo.code = OpCode::OPCODE_PUTITEM;
										TransItemInfo.size = sizeof(TRANS_ITEMINFO);
										memcpy(&TransItemInfo.Item, &lpsItem->ItemInfo, sizeof(sITEMINFO));
										rsRegist_ItemSecCode(Player, &TransItemInfo, 1);
										rsAddInvenItem(Player, TransItemInfo.Item.CODE, TransItemInfo.Item.ItemHeader.Head, TransItemInfo.Item.ItemHeader.dwChkSum);
										if (Player->lpsmSock)
											Player->lpsmSock->Send((char*)&TransItemInfo, TransItemInfo.size, TRUE);
									}

									TRANS_CHATMESSAGE	TransChatMessage;
									wsprintf(TransChatMessage.szMessage, "Voc\352 sorteou %d %s.", Item_Qtd, itemname);
									TransChatMessage.code = OpCode::OPCODE_WHISPERMESSAGE;
									TransChatMessage.size = 32 + lstrlen(TransChatMessage.szMessage);
									TransChatMessage.dwIP = 3;
									TransChatMessage.dwObjectSerial = 0;

									if (Player->lpsmSock)
										Player->lpsmSock->Send((char*)&TransChatMessage, TransChatMessage.size, TRUE);

									//Criar log na pasta do server do resgate do BP
									HANDLE	hFile;
									DWORD	dwAcess;
									DWORD	FileLength;
									char	szFileName[128];
									char	szBuff[256];
									char* lpMsg;
									char* HackLogDirectory = "LogFile";

									wsprintf(szFileName, "%s//(BattlePass) %d - %d.log", HackLogDirectory, tm.tm_mon + 1, tm.tm_mday);

									lpMsg = "%d:%d:%d - ID:( %s ) / Name:( %s ) / IP ( %s ) - Sorteou ( %d ) ( %s ) /r/n";
									wsprintf(szBuff, lpMsg,
										tm.tm_hour, tm.tm_min, tm.tm_sec, Player->getAccount(), Player->getName(),
										Player->lpsmSock->szIPAddr, Item_Qtd, itemname);

									hFile = CreateFile(szFileName, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
									if (hFile == INVALID_HANDLE_VALUE)
									{
									}
									else {
										FileLength = GetFileSize(hFile, NULL);
										SetFilePointer(hFile, FileLength, NULL, FILE_BEGIN);
										WriteFile(hFile, szBuff, lstrlen(szBuff), &dwAcess, NULL);
										CloseHandle(hFile);
									}
								}
							}
							else if (strcmp(CodItem, "SS3") == 0) //SORTEIO ITENS
							{
								for (int i = 0; i < 3; i++)
								{
									char itemsorteada[512];
									char itemname[512];
									int Sorteioitem = GetRandomPos(0, 100);

									if (Sorteioitem > 97)
									{
										wsprintf(itemsorteada, "os114");
										wsprintf(itemname, "Ordo");
									}
									else if (Sorteioitem > 90 && Sorteioitem <= 97)
									{
										wsprintf(itemsorteada, "os113");
										wsprintf(itemname, "Bellum");
									}
									else if (Sorteioitem > 80 && Sorteioitem <= 90)
									{
										wsprintf(itemsorteada, "os112");
										wsprintf(itemname, "Enigma");
									}
									else if (Sorteioitem > 70 && Sorteioitem <= 80)
									{
										wsprintf(itemsorteada, "os111");
										wsprintf(itemname, "Inferna");
									}
									else if (Sorteioitem > 55 && Sorteioitem <= 70)
									{
										wsprintf(itemsorteada, "os110");
										wsprintf(itemname, "Mirage");
									}
									else if (Sorteioitem > 40 && Sorteioitem <= 55)
									{
										wsprintf(itemsorteada, "os109");
										wsprintf(itemname, "Celesto");
									}
									else if (Sorteioitem > 20 && Sorteioitem <= 40)
									{
										wsprintf(itemsorteada, "os108");
										wsprintf(itemname, "Devine");
									}
									else
									{
										wsprintf(itemsorteada, "os107");
										wsprintf(itemname, "Murky");
									}

									if (CreateItem(lpsItem, itemsorteada, Player->smCharInfo.JOB_CODE) == TRUE)
									{
										TRANS_ITEMINFO		TransItemInfo;
										ReformItem(&lpsItem->ItemInfo);
										TransItemInfo.code = OpCode::OPCODE_PUTITEM;
										TransItemInfo.size = sizeof(TRANS_ITEMINFO);
										memcpy(&TransItemInfo.Item, &lpsItem->ItemInfo, sizeof(sITEMINFO));
										rsRegist_ItemSecCode(Player, &TransItemInfo, 1);
										rsAddInvenItem(Player, TransItemInfo.Item.CODE, TransItemInfo.Item.ItemHeader.Head, TransItemInfo.Item.ItemHeader.dwChkSum);
										if (Player->lpsmSock)
											Player->lpsmSock->Send((char*)&TransItemInfo, TransItemInfo.size, TRUE);
									}

									TRANS_CHATMESSAGE	TransChatMessage;
									wsprintf(TransChatMessage.szMessage, "Voc\352 sorteou %d %s.", Item_Qtd, itemname);
									TransChatMessage.code = OpCode::OPCODE_WHISPERMESSAGE;
									TransChatMessage.size = 32 + lstrlen(TransChatMessage.szMessage);
									TransChatMessage.dwIP = 3;
									TransChatMessage.dwObjectSerial = 0;

									if (Player->lpsmSock)
										Player->lpsmSock->Send((char*)&TransChatMessage, TransChatMessage.size, TRUE);

									//Criar log na pasta do server do resgate do BP
									HANDLE	hFile;
									DWORD	dwAcess;
									DWORD	FileLength;
									char	szFileName[128];
									char	szBuff[256];
									char* lpMsg;
									char* HackLogDirectory = "LogFile";

									wsprintf(szFileName, "%s//(BattlePass) %d - %d.log", HackLogDirectory, tm.tm_mon + 1, tm.tm_mday);

									lpMsg = "%d:%d:%d - ID:( %s ) / Name:( %s ) / IP ( %s ) - Sorteou ( %d ) ( %s ) /r/n";
									wsprintf(szBuff, lpMsg,
										tm.tm_hour, tm.tm_min, tm.tm_sec, Player->getAccount(), Player->getName(),
										Player->lpsmSock->szIPAddr, Item_Qtd, itemname);

									hFile = CreateFile(szFileName, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
									if (hFile == INVALID_HANDLE_VALUE)
									{
									}
									else {
										FileLength = GetFileSize(hFile, NULL);
										SetFilePointer(hFile, FileLength, NULL, FILE_BEGIN);
										WriteFile(hFile, szBuff, lstrlen(szBuff), &dwAcess, NULL);
										CloseHandle(hFile);
									}
								}
							}
							else if (strcmp(CodItem, "SF1") == 0) //SORTEIO ITENS
							{
								char itemsorteada[512];
								char itemname[512];
								int Sorteioitem = GetRandomPos(0, 100);

								if (Sorteioitem > 97)
								{
									wsprintf(itemsorteada, "fo111");
									wsprintf(itemname, "Force Inferna");
								}
								else if (Sorteioitem > 90 && Sorteioitem <= 97)
								{
									wsprintf(itemsorteada, "fo110");
									wsprintf(itemname, "Force Mirage");
								}
								else if (Sorteioitem > 80 && Sorteioitem <= 90)
								{
									wsprintf(itemsorteada, "fo109");
									wsprintf(itemname, "Force Celesto");
								}
								else if (Sorteioitem > 70 && Sorteioitem <= 80)
								{
									wsprintf(itemsorteada, "fo111");
									wsprintf(itemname, "Force Inferna");
								}
								else if (Sorteioitem > 55 && Sorteioitem <= 70)
								{
									wsprintf(itemsorteada, "fo110");
									wsprintf(itemname, "Force Mirage");
								}
								else if (Sorteioitem > 40 && Sorteioitem <= 55)
								{
									wsprintf(itemsorteada, "fo109");
									wsprintf(itemname, "Force Celesto");
								}
								else if (Sorteioitem > 20 && Sorteioitem <= 40)
								{
									wsprintf(itemsorteada, "fo108");
									wsprintf(itemname, "Force Devine");
								}
								else
								{
									wsprintf(itemsorteada, "fo107");
									wsprintf(itemname, "Force Murky");
								}

								if (CreateItem(lpsItem, itemsorteada, Player->smCharInfo.JOB_CODE) == TRUE)
								{
									TRANS_ITEMINFO		TransItemInfo;
									ReformItem(&lpsItem->ItemInfo);
									TransItemInfo.code = OpCode::OPCODE_PUTITEM;
									TransItemInfo.size = sizeof(TRANS_ITEMINFO);
									memcpy(&TransItemInfo.Item, &lpsItem->ItemInfo, sizeof(sITEMINFO));
									rsRegist_ItemSecCode(Player, &TransItemInfo, 1);
									rsAddInvenItem(Player, TransItemInfo.Item.CODE, TransItemInfo.Item.ItemHeader.Head, TransItemInfo.Item.ItemHeader.dwChkSum);
									if (Player->lpsmSock)
										Player->lpsmSock->Send((char*)&TransItemInfo, TransItemInfo.size, TRUE);
								}

								TRANS_CHATMESSAGE	TransChatMessage;
								wsprintf(TransChatMessage.szMessage, "Voc\352 sorteou %d %s.", Item_Qtd, itemname);
								TransChatMessage.code = OpCode::OPCODE_WHISPERMESSAGE;
								TransChatMessage.size = 32 + lstrlen(TransChatMessage.szMessage);
								TransChatMessage.dwIP = 3;
								TransChatMessage.dwObjectSerial = 0;

								if (Player->lpsmSock)
									Player->lpsmSock->Send((char*)&TransChatMessage, TransChatMessage.size, TRUE);

								//Criar log na pasta do server do resgate do BP
								HANDLE	hFile;
								DWORD	dwAcess;
								DWORD	FileLength;
								char	szFileName[128];
								char	szBuff[256];
								char* lpMsg;
								char* HackLogDirectory = "LogFile";

								wsprintf(szFileName, "%s//(BattlePass) %d - %d.log", HackLogDirectory, tm.tm_mon + 1, tm.tm_mday);

								lpMsg = "%d:%d:%d - ID:( %s ) / Name:( %s ) / IP ( %s ) - Sorteou ( %d ) ( %s ) /r/n";
								wsprintf(szBuff, lpMsg,
									tm.tm_hour, tm.tm_min, tm.tm_sec, Player->getAccount(), Player->getName(),
									Player->lpsmSock->szIPAddr, Item_Qtd, itemname);

								hFile = CreateFile(szFileName, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
								if (hFile == INVALID_HANDLE_VALUE)
								{
								}
								else {
									FileLength = GetFileSize(hFile, NULL);
									SetFilePointer(hFile, FileLength, NULL, FILE_BEGIN);
									WriteFile(hFile, szBuff, lstrlen(szBuff), &dwAcess, NULL);
									CloseHandle(hFile);
								}
							}
							else
							{
								for (int i = 0; i < Item_Qtd; i++)
								{
									if (Item_Lock == 1) {
										if (CreateItem(lpsItem, CodItem, Shop->item.spec) == TRUE)
										{
											TRANS_ITEMINFO		TransItemInfo;
											//lpsItem->ItemInfo.ItemKindCode = ITEM_KIND_BP_FREE;
											ReformItem(&lpsItem->ItemInfo);
											TransItemInfo.code = OpCode::OPCODE_PUTITEM;
											TransItemInfo.size = sizeof(TRANS_ITEMINFO);
											memcpy(&TransItemInfo.Item, &lpsItem->ItemInfo, sizeof(sITEMINFO));
											rsRegist_ItemSecCode(Player, &TransItemInfo, 1);
											rsAddInvenItem(Player, TransItemInfo.Item.CODE, TransItemInfo.Item.ItemHeader.Head, TransItemInfo.Item.ItemHeader.dwChkSum);
											if (Player->lpsmSock)
												Player->lpsmSock->Send((char*)&TransItemInfo, TransItemInfo.size, TRUE);

											BattlePass::GetInstance()->SendItemToClient(Player);
											BattlePass::GetInstance()->SendBPInfoToGame(Player);
											SendOpenBattlePass(Player->lpsmSock);
										}
									}
									else if (Item_Lock == 0) {
										if (CreateItem(lpsItem, CodItem, Shop->item.spec) == TRUE)
										{
											TRANS_ITEMINFO		TransItemInfo;
											ReformItem(&lpsItem->ItemInfo);
											TransItemInfo.code = OpCode::OPCODE_PUTITEM;
											TransItemInfo.size = sizeof(TRANS_ITEMINFO);
											memcpy(&TransItemInfo.Item, &lpsItem->ItemInfo, sizeof(sITEMINFO));
											rsRegist_ItemSecCode(Player, &TransItemInfo, 1);
											rsAddInvenItem(Player, TransItemInfo.Item.CODE, TransItemInfo.Item.ItemHeader.Head, TransItemInfo.Item.ItemHeader.dwChkSum);
											if (Player->lpsmSock)
												Player->lpsmSock->Send((char*)&TransItemInfo, TransItemInfo.size, TRUE);

											BattlePass::GetInstance()->SendItemToClient(Player);
											BattlePass::GetInstance()->SendBPInfoToGame(Player);
											SendOpenBattlePass(Player->lpsmSock);
										}
									}


									TRANS_CHATMESSAGE	TransChatMessage;
									wsprintf(TransChatMessage.szMessage, "Voc\352 resgatou %d %s.", Item_Qtd, Shop->item.Name);
									TransChatMessage.code = OpCode::OPCODE_WHISPERMESSAGE;
									TransChatMessage.size = 32 + lstrlen(TransChatMessage.szMessage);
									TransChatMessage.dwIP = 3;
									TransChatMessage.dwObjectSerial = 0;

									if (Player->lpsmSock)
										Player->lpsmSock->Send((char*)&TransChatMessage, TransChatMessage.size, TRUE);

									//Criar log na pasta do server do resgate do BP
									HANDLE	hFile;
									DWORD	dwAcess;
									DWORD	FileLength;
									char	szFileName[128];
									char	szBuff[256];
									char* lpMsg;
									char* HackLogDirectory = "LogFile";

									wsprintf(szFileName, "%s//(BattlePass) %d - %d.log", HackLogDirectory, tm.tm_mon + 1, tm.tm_mday);

									lpMsg = "%d:%d:%d - ID:( %s ) / Name:( %s ) / IP ( %s ) - Resgatou ( %d ) ( %s ) /r/n";
									wsprintf(szBuff, lpMsg,
										tm.tm_hour, tm.tm_min, tm.tm_sec, Player->getAccount(), Player->getName(),
										Player->lpsmSock->szIPAddr, Item_Qtd, Shop->item.Name);

									hFile = CreateFile(szFileName, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
									if (hFile == INVALID_HANDLE_VALUE)
									{
									}
									else {
										FileLength = GetFileSize(hFile, NULL);
										SetFilePointer(hFile, FileLength, NULL, FILE_BEGIN);
										WriteFile(hFile, szBuff, lstrlen(szBuff), &dwAcess, NULL);
										CloseHandle(hFile);
									}

								}
							}
						}
					}
					else
					{
						TRANS_CHATMESSAGE	TransChatMessage;
						wsprintf(TransChatMessage.szMessage, "Esse item j\341 foi resgatado.");
						TransChatMessage.code = OpCode::OPCODE_WHISPERMESSAGE;
						TransChatMessage.size = 32 + lstrlen(TransChatMessage.szMessage);
						TransChatMessage.dwIP = 3;
						TransChatMessage.dwObjectSerial = 0;

						if (Player->lpsmSock)
							Player->lpsmSock->Send((char*)&TransChatMessage, TransChatMessage.size, TRUE);
					}

				}
				else
				{
					TRANS_CHATMESSAGE	TransChatMessage;
					wsprintf(TransChatMessage.szMessage, "Seu passe de batalha n\343o tem n\355vel suficientes para resgatar esse item.");
					TransChatMessage.code = OpCode::OPCODE_WHISPERMESSAGE;
					TransChatMessage.size = 32 + lstrlen(TransChatMessage.szMessage);
					TransChatMessage.dwIP = 3;
					TransChatMessage.dwObjectSerial = 0;

					if (Player->lpsmSock)
						Player->lpsmSock->Send((char*)&TransChatMessage, TransChatMessage.size, TRUE);
				}
			}
			else {
				TRANS_CHATMESSAGE	TransChatMessage;
				wsprintf(TransChatMessage.szMessage, "Seu passe de batalha n\343o est\341 habilitado para essa modalidade.");
				TransChatMessage.code = OpCode::OPCODE_WHISPERMESSAGE;
				TransChatMessage.size = 32 + lstrlen(TransChatMessage.szMessage);
				TransChatMessage.dwIP = 3;
				TransChatMessage.dwObjectSerial = 0;

				if (Player->lpsmSock)
					Player->lpsmSock->Send((char*)&TransChatMessage, TransChatMessage.size, TRUE);
			}
		}

		SQL::GetInstance()->LeaveSqlSection();
		delete lpsItem;
	}
}

//ADICIONA EXPERIÊNCIA NO PASSE DE BATALHA (USADO AO MATAR MONSTROS E OUTROS JOGADORES)
void BattlePass::AddExpBattlePass(rsPLAYINFO* Player, int valor)
{
	if (SQL::GetInstance()->IsConnected())
	{
		int UserBP_Exp = 0;
		int UserBP_Level = 0;
		int Userativo = 0;
		char szQuery[512];
		SQL::GetInstance()->EnterSqlSection();
		wsprintf(szQuery, "SELECT * FROM [BattlePass].[dbo].[BP_Info] WHERE [ID]='%s'", Player->getAccount());
		if (SQL::GetInstance()->Execute(szQuery))
		{
			if (SQL::GetInstance()->Fetch())
			{
				SQL::GetInstance()->GetDataValue(2, &UserBP_Exp);
				SQL::GetInstance()->GetDataValue(3, &UserBP_Level);
				SQL::GetInstance()->GetDataValue(4, &Userativo);
			}

			if (Userativo > 0)
			{

				// Calculo feito para saber se o usuário já atingiu à experiência necessária para evoluir o nível do passe de batalha
				UserBP_Exp = UserBP_Exp + valor;

				if ((UserBP_Exp >= 999) && (UserBP_Level < 20))
				{
					UserBP_Exp = UserBP_Exp - 999;
					UserBP_Level++;

					//ENVIA MENSAGEM PARA O JOGADOR ASSIM QUE ELE UPA DE NIVEL
					TRANS_CHATMESSAGE	TransChatMessage;
					wsprintf(TransChatMessage.szMessage, "Parab\351ns voc\352 atingiu o n\355vel %d do seu Passe de Batalha", UserBP_Level);
					TransChatMessage.code = OpCode::OPCODE_WHISPERMESSAGE;
					TransChatMessage.size = 32 + lstrlen(TransChatMessage.szMessage);
					TransChatMessage.dwIP = 3;
					TransChatMessage.dwObjectSerial = 0;

					if (Player->lpsmSock)
						Player->lpsmSock->Send((char*)&TransChatMessage, TransChatMessage.size, TRUE);
				}
				else if (UserBP_Level > 19) {
					UserBP_Exp = 999;
				}

				TRANS_CHATMESSAGE pvp;

				wsprintf(pvp.szMessage, "Passe de Batalha > Exp: %d/999", UserBP_Exp);
				pvp.code = OpCode::OPCODE_WHISPERMESSAGE;
				pvp.size = 32 + lstrlen(pvp.szMessage);
				pvp.dwIP = 9;
				pvp.dwObjectSerial = 0;

				if (Player->lpsmSock)
					Player->lpsmSock->Send((char*)&pvp, pvp.size, TRUE);
				//
				//
				if (UserBP_Exp < 999) {
					wsprintf(szQuery, "UPDATE [BattlePass].[dbo].[BP_Info] SET BP_Exp=%d WHERE [ID]='%s'", UserBP_Exp, Player->getAccount());
					if (SQL::GetInstance()->Execute(szQuery))
					{
					}
					wsprintf(szQuery, "UPDATE [BattlePass].[dbo].[BP_Info] SET BP_Level=%d WHERE [ID]='%s'", UserBP_Level, Player->getAccount());
					if (SQL::GetInstance()->Execute(szQuery))
					{
					}
				}
			}
		}
		SQL::GetInstance()->LeaveSqlSection();
	}
}


