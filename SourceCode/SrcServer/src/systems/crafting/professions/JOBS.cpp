#include <cstring>
#include "systems/network/protocol/server/opcodes.h"
#include "core/server/legacy_sin_runtime/sinLinkHeader.h"
#include "modules/File/File.h"
#include "core/server/legacy_runtime/onserver.h"
#include "systems/crafting/professions/JOBS.h"
#include "systems/gameplay/progression/v2/ProgressionTriggerRuntime.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"
#include "systems/inventory/v2/legacy/LegacyInventoryRuntime.h"
#include "systems/network/v2/legacy/LegacyPacketSender.h"

#include <time.h>


int	rsRegist_ItemSecCode(rsPLAYINFO* lpPlayInfo, TRANS_ITEMINFO* lpTransItemInfo, int NewItem);
int CreateItem(psITEM* lpsItem, char* dwCode, int JobCode);
int	rsSendMoneyToUser(rsPLAYINFO* lpPlayInfo, int Money);
DWORD GetServerTime();



namespace
{
bool SummitJobsSend(rsPLAYINFO* recipient, const void* data, int size)
{
	if(!recipient || !data || size <= 0)
		return false;
	return gameplay::network::legacy::ServerLegacyPacketSender().Send(
		gameplay::PlayerId(static_cast<std::uint32_t>(recipient->dwObjectSerial)),
		gameplay::network::PacketView(data, static_cast<std::size_t>(size), 0u)).Succeeded();
}

bool SummitJobsAddInventoryItem(rsPLAYINFO* recipient, const TRANS_ITEMINFO& item)
{
	if(!recipient)
		return false;
	const gameplay::PlayerId playerId =
		gameplay::player::legacy::ServerLegacyOnlinePlayerRegistry().IdOf(recipient);
	if(!playerId.Valid())
		return false;
	const gameplay::inventory::InventoryMutationResult result =
		gameplay::inventory::legacy::ServerLegacyInventoryRuntime().Add(
			playerId,
			gameplay::inventory::ItemInstanceId(
				gameplay::ItemId(static_cast<std::uint32_t>(item.Item.CODE)),
				static_cast<std::uint32_t>(item.Item.ItemHeader.Head),
				static_cast<std::uint32_t>(item.Item.ItemHeader.dwChkSum)));
	return result.code == gameplay::inventory::InventoryMutationCode::Applied;
}
}


//ENVIA AS INFORMAÇÕES DE EXPERIÊNCIA E NIVEL 
void JOBS::SendJOBInfoToGame(rsPLAYINFO* Player)
{
	if (SQL::GetInstance()->IsConnected())
	{
		JOBS_INFO packet;
		char szQuery[512];
		SQL::GetInstance()->EnterSqlSection();
		wsprintf(szQuery, "SELECT * FROM [JOBS].[dbo].[Jobs_Info] WHERE [ID]='%s'", Player->getAccount());
		if (SQL::GetInstance()->Execute(szQuery))
		{
			if (SQL::GetInstance()->Fetch())
			{
				SQL::GetInstance()->GetDataValue(3, &packet.Job);
				SQL::GetInstance()->GetDataValue(4, &packet.VTNClvl);
				SQL::GetInstance()->GetDataValue(5, &packet.exp);
				SQL::GetInstance()->GetDataValue(6, &packet.m_a);
				SQL::GetInstance()->GetDataValue(7, &packet.m_f);
				SQL::GetInstance()->GetDataValue(8, &packet.m_l);
				SQL::GetInstance()->GetDataValue(9, &packet.m_m);
				SQL::GetInstance()->GetDataValue(10, &packet.m_r);


				packet.size = sizeof(JOBS_INFO);
				packet.code = JOBS_PACKET_SEND_BPINFO_GAME;
				if (Player->lpsmSock)
					SummitJobsSend(Player, &packet, packet.size);
			}
			/*else
			{
				packet.size = sizeof(JOBS_INFO);
				packet.code = JOBS_PACKET_SEND_BPINFO_GAME;
				packet.exp = 0;
				packet.VTNClvl = 0;
				if (Player->lpsmSock)
					SummitJobsSend(Player, &packet, packet.size);
			}*/
		}
		SQL::GetInstance()->LeaveSqlSection();
	}
}

//ENVIA O MERCADO
void JOBS::SendJOBmarketToGame(rsPLAYINFO* Player)
{	
	if (SQL::GetInstance()->IsConnected())
	{
		int i = 0;
		char szQuery[512];
		JOBS_PACK packet;
		ZeroMemory(&packet, sizeof(JOBS_PACK));
		packet.size = sizeof(JOBS_PACK);
		packet.code = JOBS_PACKET_SEND_MARKET;
		SQL::GetInstance()->EnterSqlSection();
		wsprintf(szQuery, "SELECT * FROM [JOBS].[dbo].[Jobs_shop] Order By mine_type Asc, mine_qtd Asc, mine_price Asc");
		if (SQL::GetInstance()->Execute(szQuery))
		{
			while (SQL::GetInstance()->Fetch())
			{
				{			
					SQL::GetInstance()->GetDataValue(1, packet.item[i].ID);
					SQL::GetInstance()->GetDataValue(2, packet.item[i].Name);
					SQL::GetInstance()->GetDataValue(3, &packet.item[i].mine_type);
					SQL::GetInstance()->GetDataValue(4, &packet.item[i].mine_qtd);
					SQL::GetInstance()->GetDataValue(5, &packet.item[i].mine_price);
					SQL::GetInstance()->GetDataValue(6, &packet.item[i].checksum);
				}

				i++;

				if (i > 69) break;
			}

			for (int cnt = 0; cnt < CONNECTMAX; cnt++)
			{
				if (rsPlayInfo[cnt].lpsmSock)
				{
					SummitJobsSend(&rsPlayInfo[cnt], &packet, packet.size);
				}
			}
		}
		SQL::GetInstance()->LeaveSqlSection();
	}
}

void JOBS::RecvJOBsellToClient(rsPLAYINFO* Player, JOBS_RESGATE* infos)
{
	char szQuery[512];
	char jobidsell[32];
	char jobnamesell[32];
	int	 rjob_auxmine;
	int	 rjob_contsellqtd;
	int	 rjob_contsellprice;
	rjob_auxmine = infos->item.job_auxmine;
	rjob_contsellqtd = infos->item.job_contsellqtd;
	rjob_contsellprice = infos->item.job_contsellprice;
	wsprintf(jobidsell, "'%s'", Player->getAccount());
	wsprintf(jobnamesell, "'%s'", Player->getName());
	int rm_a;
	int rm_f;
	int rm_l;
	int rm_m;
	int rm_r;
    int CHECKSUM = GetRandomPos(100000000, 999999999);

	if (SQL::GetInstance()->IsConnected())
	{
		wsprintf(szQuery, "SELECT * FROM [JOBS].[dbo].[Jobs_Info] WHERE [ID]='%s'", Player->getAccount());
		if (SQL::GetInstance()->Execute(szQuery))
		{
			if (SQL::GetInstance()->Fetch())
			{
				SQL::GetInstance()->GetDataValue(6, &rm_a);
				SQL::GetInstance()->GetDataValue(7, &rm_f);
				SQL::GetInstance()->GetDataValue(8, &rm_l);
				SQL::GetInstance()->GetDataValue(9, &rm_m);
				SQL::GetInstance()->GetDataValue(10, &rm_r);
			}

			switch (rjob_auxmine)
			{
			case 1:
				rm_a -= rjob_contsellqtd;
				wsprintf(szQuery, "UPDATE [JOBS].[dbo].[Jobs_Info] SET M_A=%d WHERE[ID] = '%s'", rm_a, Player->getAccount());
				if (SQL::GetInstance()->Execute(szQuery))
				{

				}
				break;
			case 2:
				rm_f -= rjob_contsellqtd;
				wsprintf(szQuery, "UPDATE [JOBS].[dbo].[Jobs_Info] SET M_F=%d WHERE[ID] = '%s'", rm_f, Player->getAccount());
				if (SQL::GetInstance()->Execute(szQuery))
				{

				}
				break;
			case 3:
				rm_l -= rjob_contsellqtd;
				wsprintf(szQuery, "UPDATE [JOBS].[dbo].[Jobs_Info] SET M_L=%d WHERE[ID] = '%s'", rm_l, Player->getAccount());
				if (SQL::GetInstance()->Execute(szQuery))
				{

				}
				break;
			case 4:
				rm_m -= rjob_contsellqtd;
				wsprintf(szQuery, "UPDATE [JOBS].[dbo].[Jobs_Info] SET M_M=%d WHERE[ID] = '%s'", rm_m, Player->getAccount());
				if (SQL::GetInstance()->Execute(szQuery))
				{

				}
				break;
			case 5:
				rm_r -= rjob_contsellqtd;
				wsprintf(szQuery, "UPDATE [JOBS].[dbo].[Jobs_Info] SET M_R=%d WHERE[ID] = '%s'", rm_r, Player->getAccount());
				if (SQL::GetInstance()->Execute(szQuery))
				{

				}
				break;
			}


			wsprintf(szQuery, "INSERT INTO  [JOBS].[dbo].[Jobs_shop] (ID, NICK, MINE_TYPE, MINE_QTD, MINE_PRICE, CHECKSUM) VALUES ('%s', '%s', %d, %d, %d, %d)", Player->getAccount(), Player->getName(), rjob_auxmine, rjob_contsellqtd, rjob_contsellprice, CHECKSUM);
			if (SQL::GetInstance()->Execute(szQuery))
			{

				TRANS_CHATMESSAGE	TransChatMessage;
				wsprintf(TransChatMessage.szMessage, "Voc\352 colocou %d atributos a venda por %d Coins!", rjob_contsellqtd, rjob_contsellprice);
				TransChatMessage.code = OpCode::OPCODE_WHISPERMESSAGE;
				TransChatMessage.size = 32 + lstrlen(TransChatMessage.szMessage);
				TransChatMessage.dwIP = 3;
				TransChatMessage.dwObjectSerial = 0;

				if (Player->lpsmSock)
					SummitJobsSend(Player, &TransChatMessage, TransChatMessage.size);
			}
		}

		SQL::GetInstance()->LeaveSqlSection();

		JOBS::GetInstance()->SendJOBInfoToGame(Player);
		JOBS::GetInstance()->SendJOBmarketToGame(Player);
	}

}

void JOBS::RecvJOBbuyToClient(rsPLAYINFO* Player, JOBS_PACK* Buy)
{
	if (SQL::GetInstance()->IsConnected())
	{
		int BuyerCoin = 0;
		int SellerCoin = 0;
		int Price = 0;
		int Qtd = 0;
		int Type = 0;
		int brm_a;
		int brm_f;
		int brm_l;
		int brm_m;
		int brm_r;
		char szQuery[512];
		char ID[512];
		char NICK[512];
		SQL::GetInstance()->EnterSqlSection();
		sprintf_s(szQuery, sizeof(szQuery), "SELECT * FROM [CoinShopDB].[dbo].[Usuario] WHERE [Login]='%s'", Player->getAccount());
		if (SQL::GetInstance()->Execute(szQuery))
		{
			if (SQL::GetInstance()->Fetch())
				SQL::GetInstance()->GetDataValue(6, &BuyerCoin);
			sprintf_s(szQuery, sizeof(szQuery), "SELECT * FROM [JOBS].[dbo].[Jobs_Shop] WHERE [CHECKSUM]='%d'", Buy->item->checksum);
			if (SQL::GetInstance()->Execute(szQuery))
			{
				if (SQL::GetInstance()->Fetch())
				SQL::GetInstance()->GetDataValue(1, ID);
				SQL::GetInstance()->GetDataValue(2, NICK);
				SQL::GetInstance()->GetDataValue(3, &Type);
				SQL::GetInstance()->GetDataValue(4, &Qtd);
				SQL::GetInstance()->GetDataValue(5, &Price);
			}
			sprintf_s(szQuery, sizeof(szQuery), "SELECT * FROM [PAINEL_NET].[dbo].[Usuario] WHERE [Login]='%s'", ID);
			if (SQL::GetInstance()->Execute(szQuery))
			{
				if (SQL::GetInstance()->Fetch())
					SQL::GetInstance()->GetDataValue(6, &SellerCoin);
			}
			wsprintf(szQuery, "SELECT * FROM [JOBS].[dbo].[Jobs_Info] WHERE [ID]='%s'", Player->getAccount());
			if (SQL::GetInstance()->Execute(szQuery))
			{
				if (SQL::GetInstance()->Fetch())
				{
					SQL::GetInstance()->GetDataValue(6, &brm_a);
					SQL::GetInstance()->GetDataValue(7, &brm_f);
					SQL::GetInstance()->GetDataValue(8, &brm_l);
					SQL::GetInstance()->GetDataValue(9, &brm_m);
					SQL::GetInstance()->GetDataValue(10, &brm_r);
				}
			}

			if (lstrcmpi(Player->getAccount(), ID) == 0)
			{
				Price = 0;
			}
			
			
			if (BuyerCoin >= Price)
				{
					BuyerCoin -= Price;
					SellerCoin += Price;
					sprintf_s(szQuery, sizeof(szQuery), "UPDATE [PAINEL_NET].[dbo].[Usuario] SET Coins=%d WHERE [Login]='%s'", BuyerCoin, Player->getAccount());
					if (SQL::GetInstance()->Execute(szQuery))
					{
						sprintf_s(szQuery, sizeof(szQuery), "UPDATE [PAINEL_NET].[dbo].[Usuario] SET Coins=%d WHERE [Login]='%s'", SellerCoin, ID);
						if (SQL::GetInstance()->Execute(szQuery))
						{
							sprintf_s(szQuery, sizeof(szQuery), "DELETE FROM [JOBS].[dbo].[Jobs_Shop] WHERE [CHECKSUM]='%d'", Buy->item->checksum);
							if (SQL::GetInstance()->Execute(szQuery))
							{

								switch (Type)
								{
								case 1:
									brm_a += Qtd;
									wsprintf(szQuery, "UPDATE [JOBS].[dbo].[Jobs_Info] SET M_A=%d WHERE[ID] = '%s'", brm_a, Player->getAccount());
									if (SQL::GetInstance()->Execute(szQuery))
									{

									}
									break;
								case 2:
									brm_f += Qtd;
									wsprintf(szQuery, "UPDATE [JOBS].[dbo].[Jobs_Info] SET M_F=%d WHERE[ID] = '%s'", brm_f, Player->getAccount());
									if (SQL::GetInstance()->Execute(szQuery))
									{

									}
									break;
								case 3:
									brm_l += Qtd;
									wsprintf(szQuery, "UPDATE [JOBS].[dbo].[Jobs_Info] SET M_L=%d WHERE[ID] = '%s'", brm_l, Player->getAccount());
									if (SQL::GetInstance()->Execute(szQuery))
									{

									}
									break;
								case 4:
									brm_m += Qtd;
									wsprintf(szQuery, "UPDATE [JOBS].[dbo].[Jobs_Info] SET M_M=%d WHERE[ID] = '%s'", brm_m, Player->getAccount());
									if (SQL::GetInstance()->Execute(szQuery))
									{

									}
									break;
								case 5:
									brm_r += Qtd;
									wsprintf(szQuery, "UPDATE [JOBS].[dbo].[Jobs_Info] SET M_R=%d WHERE[ID] = '%s'", brm_r, Player->getAccount());
									if (SQL::GetInstance()->Execute(szQuery))
									{

									}
									break;
								}


								TRANS_CHATMESSAGE	TransChatMessage;
								sprintf_s(TransChatMessage.szMessage, sizeof(TransChatMessage.szMessage), "Voc\352 comprou %d atributos de '%s', agora voc\352 tem %d de Coins.", Qtd, NICK, BuyerCoin);
								TransChatMessage.code = OpCode::OPCODE_WHISPERMESSAGE;
								TransChatMessage.size = 32 + lstrlen(TransChatMessage.szMessage);
								TransChatMessage.dwIP = 3;
								TransChatMessage.dwObjectSerial = 0;


								if (Player->lpsmSock)
									SummitJobsSend(Player, &TransChatMessage, TransChatMessage.size);
							}
						}
					}

				}
				else
				{
					TRANS_CHATMESSAGE	TransChatMessage;
					sprintf_s(TransChatMessage.szMessage, sizeof(TransChatMessage.szMessage), "Voc\352 n\343o possui cr\351ditos suficientes para comprar esses atributos.");
					TransChatMessage.code = OpCode::OPCODE_WHISPERMESSAGE;
					TransChatMessage.size = 32 + lstrlen(TransChatMessage.szMessage);
					TransChatMessage.dwIP = 3;
					TransChatMessage.dwObjectSerial = 0;

					if (Player->lpsmSock)
						SummitJobsSend(Player, &TransChatMessage, TransChatMessage.size);
				}
				SQL::GetInstance()->LeaveSqlSection();
				JOBS::GetInstance()->SendJOBInfoToGame(Player);
				JOBS::GetInstance()->SendJOBmarketToGame(Player);
				
		}
	}
}


//RECEBE AS INFORMAÇÕES DO CLIENTE REFERENTE AO ITEM
void JOBS::RecvJOBItemToClient(rsPLAYINFO* Player, JOBS_RESGATE* item)
{

	time_t mytime;
	mytime = time(NULL);
	struct tm tm = *localtime(&mytime);
	

	if (SQL::GetInstance()->IsConnected())
	{
		psITEM* lpsItem;
		lpsItem = new psITEM;
		
		int Item_Level_Req = 0;
		char CodItem1[32];
		char CodItem2[32];
		char Codname1[32];
		char Codname2[32];
		char szQuery[512];
		
		int exp;
		int VTNClvl;
		int Job;
		int m_a;
		int m_f;
		int m_l;
		int m_m;
		int m_r;
		int cm_a;
		int cm_f;
		int cm_l;
		int cm_m;
		int cm_r;
		int c_all = 50;
		
	
		char name[32];
		int job_code = 0;
		int forgejob_code = 0;
		bool professionStateChanged = false;

		wsprintf(name, "'%s'", item->item.Name);
		job_code = item->item.job_code;
		forgejob_code = item->item.Forje_code;

		if (job_code > 0)
		{
			if (SQL::GetInstance()->IsConnected())
			{
				char szQuery[512];
				SQL::GetInstance()->EnterSqlSection();
				wsprintf(szQuery, "INSERT INTO  [JOBS].[dbo].[Jobs_Info] (ID, NICK, JOB, job_Level, Job_xp, M_A, M_F, M_L, M_M, M_R) VALUES ('%s', 0, %d, 1, 0, 0, 0, 0, 0, 0)", Player->getAccount(), job_code);
				if (SQL::GetInstance()->Execute(szQuery)) {
					professionStateChanged = true;
				}
				SQL::GetInstance()->LeaveSqlSection();
			}
		}
		


		SQL::GetInstance()->EnterSqlSection();
		wsprintf(szQuery, "SELECT * FROM [JOBS].[dbo].[Jobs_Info] WHERE [ID]='%s'", Player->getAccount());
		if (SQL::GetInstance()->Execute(szQuery))
		{
			if (SQL::GetInstance()->Fetch())
			{
				SQL::GetInstance()->GetDataValue(3, &Job);
				SQL::GetInstance()->GetDataValue(4, &VTNClvl);
				SQL::GetInstance()->GetDataValue(5, &exp);
				SQL::GetInstance()->GetDataValue(6, &m_a);
				SQL::GetInstance()->GetDataValue(7, &m_f);
				SQL::GetInstance()->GetDataValue(8, &m_l);
				SQL::GetInstance()->GetDataValue(9, &m_m);
				SQL::GetInstance()->GetDataValue(10, &m_r);
			}

			if (forgejob_code == 1)
			{
				switch (Job)
				{
				case 1:
					cm_a = c_all * 4;
					cm_f = c_all;
					cm_l = c_all;
					cm_m = c_all;
					cm_r = c_all;

					wsprintf(CodItem1, "wm118");
					wsprintf(CodItem2, "wm118");

					wsprintf(Codname1, "Cajado 80b");
					wsprintf(Codname2, "Cajado 80b");

					break;
				case 2:
					cm_a = c_all;
					cm_f = c_all * 4;
					cm_l = c_all;
					cm_m = c_all;
					cm_r = c_all;

					wsprintf(CodItem1, "ws220");
					wsprintf(CodItem2, "ws220");

					wsprintf(Codname1, "Espada 80b");
					wsprintf(Codname2, "Espada 80b");

					break;
				case 3:
					cm_a = c_all;
					cm_f = c_all;
					cm_l = c_all * 4;
					cm_m = c_all;
					cm_r = c_all;

					wsprintf(CodItem1, "wa117");
					wsprintf(CodItem2, "ws119");

					wsprintf(Codname1, "Machado 80b");
					wsprintf(Codname2, "Arco 80b");

					break;
				case 4:
					cm_a = c_all;
					cm_f = c_all;
					cm_l = c_all;
					cm_m = c_all * 4;
					cm_r = c_all;

					wsprintf(CodItem1, "wh118");
					wsprintf(CodItem2, "wp118");

					wsprintf(Codname1, "Martelo 80b");
					wsprintf(Codname2, "Foice 80b");

					break;
				case 5:
					cm_a = c_all;
					cm_f = c_all;
					cm_l = c_all;
					cm_m = c_all;
					cm_r = c_all * 4;

					wsprintf(CodItem1, "wc117");
					wsprintf(CodItem2, "wt118");

					wsprintf(Codname1, "Garra 80b");
					wsprintf(Codname2, "Lan\347a 80b");

					break;
				}
				if (m_a >= cm_a && m_f >= cm_f && m_l >= cm_l && m_m >= cm_m && m_r >= cm_r)
				{
					m_a = m_a - cm_a;
					m_f = m_f - cm_f;
					m_l = m_l - cm_l;
					m_m = m_m - cm_m;
					m_r = m_r - cm_r;

					int chanceforja = 70;
					int sorteioforja = GetRandomPos(0, 100);
					if (VTNClvl == 20)
					{
						chanceforja = 40;
					}
					else if (VTNClvl >= 5)
					{
						chanceforja = 55;
					}

				
					wsprintf(szQuery, "UPDATE [JOBS].[dbo].[Jobs_Info] SET M_A=%d WHERE[ID] = '%s'", m_a, Player->getAccount());
					if (SQL::GetInstance()->Execute(szQuery))
					{
						wsprintf(szQuery, "UPDATE [JOBS].[dbo].[Jobs_Info] SET M_F=%d WHERE[ID] = '%s'", m_f, Player->getAccount());
						if (SQL::GetInstance()->Execute(szQuery))
						{
							wsprintf(szQuery, "UPDATE [JOBS].[dbo].[Jobs_Info] SET M_L=%d WHERE[ID] = '%s'", m_l, Player->getAccount());
							if (SQL::GetInstance()->Execute(szQuery))
							{
								wsprintf(szQuery, "UPDATE [JOBS].[dbo].[Jobs_Info] SET M_M=%d WHERE[ID] = '%s'", m_m, Player->getAccount());
								if (SQL::GetInstance()->Execute(szQuery))
								{
									wsprintf(szQuery, "UPDATE [JOBS].[dbo].[Jobs_Info] SET M_R=%d WHERE[ID] = '%s'", m_r, Player->getAccount());
									if (SQL::GetInstance()->Execute(szQuery))
									{
										if (sorteioforja >= chanceforja)
										{


										char itemname[512];
										char itemsor[512];
										int Sorteioitem = GetRandomPos(0, 100);

										if (Sorteioitem > 50)
										{
											wsprintf(itemsor, "%s", CodItem1);
											wsprintf(itemname, "%s", Codname1);
										}
										else
										{
											wsprintf(itemsor, "%s", CodItem2);
											wsprintf(itemname, "%s", Codname2);
										}

										if (CreateItem(lpsItem, itemsor, 0) == TRUE)
										{
											TRANS_ITEMINFO		TransItemInfo;
											//ReformItem(&lpsItem->ItemInfo);
											TransItemInfo.code = OpCode::OPCODE_PUTITEM;
											TransItemInfo.size = sizeof(TRANS_ITEMINFO);
											memcpy(&TransItemInfo.Item, &lpsItem->ItemInfo, sizeof(sITEMINFO));
											rsRegist_ItemSecCode(Player, &TransItemInfo, 1);
											if (SummitJobsAddInventoryItem(Player, TransItemInfo) && Player->lpsmSock)
												SummitJobsSend(Player, &TransItemInfo, TransItemInfo.size);
										}

										TRANS_CHATMESSAGE	TransChatMessage;
										wsprintf(TransChatMessage.szMessage, "Voc\352 Forjou 1 %s.", itemname);
										TransChatMessage.code = OpCode::OPCODE_WHISPERMESSAGE;
										TransChatMessage.size = 32 + lstrlen(TransChatMessage.szMessage);
										TransChatMessage.dwIP = 3;
										TransChatMessage.dwObjectSerial = 0;

										if (Player->lpsmSock)
											SummitJobsSend(Player, &TransChatMessage, TransChatMessage.size);

										//Criar log na pasta do server do resgate do BP
										HANDLE	hFile;
										DWORD	dwAcess;
										DWORD	FileLength;
										char	szFileName[128];
										char	szBuff[256];
										char* lpMsg;
										char* HackLogDirectory = "LogFile";

										wsprintf(szFileName, "%s//(FORJA) %d - %d.log", HackLogDirectory, tm.tm_mon + 1, tm.tm_mday);

										lpMsg = "%d:%d:%d - ID:( %s ) / Name:( %s ) / IP ( %s ) - Forjou ( %d ) ( %s ) /r/n";
										wsprintf(szBuff, lpMsg,
											tm.tm_hour, tm.tm_min, tm.tm_sec, Player->getAccount(), Player->getName(),
											Player->lpsmSock->szIPAddr, 1, itemname);

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
										TRANS_CHATMESSAGE	TransChatMessage;
										wsprintf(TransChatMessage.szMessage, "Voc\352 Falhou no processo de Forja!");
										TransChatMessage.code = OpCode::OPCODE_WHISPERMESSAGE;
										TransChatMessage.size = 32 + lstrlen(TransChatMessage.szMessage);
										TransChatMessage.dwIP = 3;
										TransChatMessage.dwObjectSerial = 0;

										if (Player->lpsmSock)
											SummitJobsSend(Player, &TransChatMessage, TransChatMessage.size);

										//Criar log na pasta do server do resgate do BP
										HANDLE	hFile;
										DWORD	dwAcess;
										DWORD	FileLength;
										char	szFileName[128];
										char	szBuff[256];
										char* lpMsg;
										char* HackLogDirectory = "LogFile";

										wsprintf(szFileName, "%s//(FORJA) %d - %d.log", HackLogDirectory, tm.tm_mon + 1, tm.tm_mday);

										lpMsg = "%d:%d:%d - ID:( %s ) / Name:( %s ) / IP ( %s ) - Tentou Forjar ( %d ) ( %s ) /r/n";
										wsprintf(szBuff, lpMsg,
											tm.tm_hour, tm.tm_min, tm.tm_sec, Player->getAccount(), Player->getName(),
											Player->lpsmSock->szIPAddr, 1, "item");

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

					}
				}
					



				}

				else
				{
					TRANS_CHATMESSAGE	TransChatMessage;
					wsprintf(TransChatMessage.szMessage, "Voc\352 n\343o tem materiais suficientes para realizar a Forja!");
					TransChatMessage.code = OpCode::OPCODE_WHISPERMESSAGE;
					TransChatMessage.size = 32 + lstrlen(TransChatMessage.szMessage);
					TransChatMessage.dwIP = 3;
					TransChatMessage.dwObjectSerial = 0;

					if (Player->lpsmSock)
						SummitJobsSend(Player, &TransChatMessage, TransChatMessage.size);
				}


				}
				else if (forgejob_code == 2)
				{
				switch (Job)
				{
				case 1:
					cm_a = c_all * 8;
					cm_f = c_all;
					cm_l = c_all;
					cm_m = c_all;
					cm_r = c_all;

					wsprintf(CodItem1, "wm119");
					wsprintf(CodItem2, "wm119");

					wsprintf(Codname1, "Cajado 80c");
					wsprintf(Codname2, "Cajado 80c");

					break;
				case 2:
					cm_a = c_all;
					cm_f = c_all * 8;
					cm_l = c_all;
					cm_m = c_all;
					cm_r = c_all;

					wsprintf(CodItem1, "ws221");
					wsprintf(CodItem2, "ws221");

					wsprintf(Codname1, "Espada 80c");
					wsprintf(Codname2, "Espada 80c");

					break;
				case 3:
					cm_a = c_all;
					cm_f = c_all;
					cm_l = c_all * 8;
					cm_m = c_all;
					cm_r = c_all;

					wsprintf(CodItem1, "wa118");
					wsprintf(CodItem2, "ws120");

					wsprintf(Codname1, "Machado 80c");
					wsprintf(Codname2, "Arco 80c");

					break;
				case 4:
					cm_a = c_all;
					cm_f = c_all;
					cm_l = c_all;
					cm_m = c_all * 8;
					cm_r = c_all;

					wsprintf(CodItem1, "wh119");
					wsprintf(CodItem2, "wp119");

					wsprintf(Codname1, "Martelo 80c");
					wsprintf(Codname2, "Foice 80c");

					break;
				case 5:
					cm_a = c_all;
					cm_f = c_all;
					cm_l = c_all;
					cm_m = c_all;
					cm_r = c_all * 8;

					wsprintf(CodItem1, "wc118");
					wsprintf(CodItem2, "wt119");

					wsprintf(Codname1, "Garra 80c");
					wsprintf(Codname2, "Lan\347a 80c");

					break;
				}
					if (m_a >= cm_a && m_f >= cm_f && m_l >= cm_l && m_m >= cm_m && m_r >= cm_r)
					{
						m_a = m_a - cm_a;
						m_f = m_f - cm_f;
						m_l = m_l - cm_l;
						m_m = m_m - cm_m;
						m_r = m_r - cm_r;

						int chanceforja = 80;
						int sorteioforja = GetRandomPos(0, 100);
						if (VTNClvl == 20)
						{
							chanceforja = 60;
						}
						else if (VTNClvl >= 5)
						{
							chanceforja = 70;
						}

					
							wsprintf(szQuery, "UPDATE [JOBS].[dbo].[Jobs_Info] SET M_A=%d WHERE[ID] = '%s'", m_a, Player->getAccount());
							if (SQL::GetInstance()->Execute(szQuery))
							{
								wsprintf(szQuery, "UPDATE [JOBS].[dbo].[Jobs_Info] SET M_F=%d WHERE[ID] = '%s'", m_f, Player->getAccount());
								if (SQL::GetInstance()->Execute(szQuery))
								{
									wsprintf(szQuery, "UPDATE [JOBS].[dbo].[Jobs_Info] SET M_L=%d WHERE[ID] = '%s'", m_l, Player->getAccount());
									if (SQL::GetInstance()->Execute(szQuery))
									{
										wsprintf(szQuery, "UPDATE [JOBS].[dbo].[Jobs_Info] SET M_M=%d WHERE[ID] = '%s'", m_m, Player->getAccount());
										if (SQL::GetInstance()->Execute(szQuery))
										{
											wsprintf(szQuery, "UPDATE [JOBS].[dbo].[Jobs_Info] SET M_R=%d WHERE[ID] = '%s'", m_r, Player->getAccount());
											if (SQL::GetInstance()->Execute(szQuery))
											{
												if (sorteioforja >= chanceforja)
												{


												char itemname[512];
												char itemsor[512];
												int Sorteioitem = GetRandomPos(0, 100);

												if (Sorteioitem > 50)
												{
													wsprintf(itemsor, "%s", CodItem1);
													wsprintf(itemname, "%s", Codname1);
												}
												else
												{
													wsprintf(itemsor, "%s", CodItem2);
													wsprintf(itemname, "%s", Codname2);
												}

												if (CreateItem(lpsItem, itemsor, 0) == TRUE)
												{
													TRANS_ITEMINFO		TransItemInfo;
													//ReformItem(&lpsItem->ItemInfo);
													TransItemInfo.code = OpCode::OPCODE_PUTITEM;
													TransItemInfo.size = sizeof(TRANS_ITEMINFO);
													memcpy(&TransItemInfo.Item, &lpsItem->ItemInfo, sizeof(sITEMINFO));
													rsRegist_ItemSecCode(Player, &TransItemInfo, 1);
													if (SummitJobsAddInventoryItem(Player, TransItemInfo) && Player->lpsmSock)
														SummitJobsSend(Player, &TransItemInfo, TransItemInfo.size);
												}

												TRANS_CHATMESSAGE	TransChatMessage;
												wsprintf(TransChatMessage.szMessage, "Voc\352 Forjou 1 %s.", itemname);
												TransChatMessage.code = OpCode::OPCODE_WHISPERMESSAGE;
												TransChatMessage.size = 32 + lstrlen(TransChatMessage.szMessage);
												TransChatMessage.dwIP = 3;
												TransChatMessage.dwObjectSerial = 0;

												if (Player->lpsmSock)
													SummitJobsSend(Player, &TransChatMessage, TransChatMessage.size);

												//Criar log na pasta do server do resgate do BP
												HANDLE	hFile;
												DWORD	dwAcess;
												DWORD	FileLength;
												char	szFileName[128];
												char	szBuff[256];
												char* lpMsg;
												char* HackLogDirectory = "LogFile";

												wsprintf(szFileName, "%s//(FORJA) %d - %d.log", HackLogDirectory, tm.tm_mon + 1, tm.tm_mday);

												lpMsg = "%d:%d:%d - ID:( %s ) / Name:( %s ) / IP ( %s ) - Forjou ( %d ) ( %s ) /r/n";
												wsprintf(szBuff, lpMsg,
													tm.tm_hour, tm.tm_min, tm.tm_sec, Player->getAccount(), Player->getName(),
													Player->lpsmSock->szIPAddr, 1, itemname);

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
												TRANS_CHATMESSAGE	TransChatMessage;
												wsprintf(TransChatMessage.szMessage, "Voc\352 Falhou no processo de Forja!");
												TransChatMessage.code = OpCode::OPCODE_WHISPERMESSAGE;
												TransChatMessage.size = 32 + lstrlen(TransChatMessage.szMessage);
												TransChatMessage.dwIP = 3;
												TransChatMessage.dwObjectSerial = 0;

												if (Player->lpsmSock)
													SummitJobsSend(Player, &TransChatMessage, TransChatMessage.size);

												//Criar log na pasta do server do resgate do BP
												HANDLE	hFile;
												DWORD	dwAcess;
												DWORD	FileLength;
												char	szFileName[128];
												char	szBuff[256];
												char* lpMsg;
												char* HackLogDirectory = "LogFile";

												wsprintf(szFileName, "%s//(FORJA) %d - %d.log", HackLogDirectory, tm.tm_mon + 1, tm.tm_mday);

												lpMsg = "%d:%d:%d - ID:( %s ) / Name:( %s ) / IP ( %s ) - Tentou Forjar ( %d ) ( %s ) /r/n";
												wsprintf(szBuff, lpMsg,
													tm.tm_hour, tm.tm_min, tm.tm_sec, Player->getAccount(), Player->getName(),
													Player->lpsmSock->szIPAddr, 1, "item");

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

							}
						}
						



					}

					else
					{
						TRANS_CHATMESSAGE	TransChatMessage;
						wsprintf(TransChatMessage.szMessage, "Voc\352 n\343o tem materiais suficientes para realizar a Forja!");
						TransChatMessage.code = OpCode::OPCODE_WHISPERMESSAGE;
						TransChatMessage.size = 32 + lstrlen(TransChatMessage.szMessage);
						TransChatMessage.dwIP = 3;
						TransChatMessage.dwObjectSerial = 0;

						if (Player->lpsmSock)
							SummitJobsSend(Player, &TransChatMessage, TransChatMessage.size);
					}


				}
				else if (forgejob_code == 3)
				{
				switch (Job)
				{
				case 1:
					cm_a = c_all * 15;
					cm_f = c_all;
					cm_l = c_all;
					cm_m = c_all;
					cm_r = c_all;

					wsprintf(CodItem1, "wm120");
					wsprintf(CodItem2, "wm120");

					wsprintf(Codname1, "Cajado 80d");
					wsprintf(Codname2, "Cajado 80d");

					break;
				case 2:
					cm_a = c_all;
					cm_f = c_all * 15;
					cm_l = c_all;
					cm_m = c_all;
					cm_r = c_all;

					wsprintf(CodItem1, "ws222");
					wsprintf(CodItem2, "ws222");

					wsprintf(Codname1, "Espada 80d");
					wsprintf(Codname2, "Espada 80d");

					break;
				case 3:
					cm_a = c_all;
					cm_f = c_all;
					cm_l = c_all * 15;
					cm_m = c_all;
					cm_r = c_all;

					wsprintf(CodItem1, "wa119");
					wsprintf(CodItem2, "ws121");

					wsprintf(Codname1, "Machado 80d");
					wsprintf(Codname2, "Arco 80d");

					break;
				case 4:
					cm_a = c_all;
					cm_f = c_all;
					cm_l = c_all;
					cm_m = c_all * 15;
					cm_r = c_all;

					wsprintf(CodItem1, "wh120");
					wsprintf(CodItem2, "wp120");

					wsprintf(Codname1, "Martelo 80d");
					wsprintf(Codname2, "Foice 80d");

					break;
				case 5:
					cm_a = c_all;
					cm_f = c_all;
					cm_l = c_all;
					cm_m = c_all;
					cm_r = c_all * 15;

					wsprintf(CodItem1, "wc119");
					wsprintf(CodItem2, "wt120");

					wsprintf(Codname1, "Garra 80d");
					wsprintf(Codname2, "Lan\347a 80d");

					break;
				}
					if (m_a >= cm_a && m_f >= cm_f && m_l >= cm_l && m_m >= cm_m && m_r >= cm_r)
					{
						m_a = m_a - cm_a;
						m_f = m_f - cm_f;
						m_l = m_l - cm_l;
						m_m = m_m - cm_m;
						m_r = m_r - cm_r;

						int chanceforja = 90;
						int sorteioforja = GetRandomPos(0, 100);
						if (VTNClvl == 20)
						{
							chanceforja = 70;
						}
						else if (VTNClvl >= 5)
						{
							chanceforja = 80;
						}


							wsprintf(szQuery, "UPDATE [JOBS].[dbo].[Jobs_Info] SET M_A=%d WHERE[ID] = '%s'", m_a, Player->getAccount());
							if (SQL::GetInstance()->Execute(szQuery))
							{
								wsprintf(szQuery, "UPDATE [JOBS].[dbo].[Jobs_Info] SET M_F=%d WHERE[ID] = '%s'", m_f, Player->getAccount());
								if (SQL::GetInstance()->Execute(szQuery))
								{
									wsprintf(szQuery, "UPDATE [JOBS].[dbo].[Jobs_Info] SET M_L=%d WHERE[ID] = '%s'", m_l, Player->getAccount());
									if (SQL::GetInstance()->Execute(szQuery))
									{
										wsprintf(szQuery, "UPDATE [JOBS].[dbo].[Jobs_Info] SET M_M=%d WHERE[ID] = '%s'", m_m, Player->getAccount());
										if (SQL::GetInstance()->Execute(szQuery))
										{
											wsprintf(szQuery, "UPDATE [JOBS].[dbo].[Jobs_Info] SET M_R=%d WHERE[ID] = '%s'", m_r, Player->getAccount());
											if (SQL::GetInstance()->Execute(szQuery))
											{
												if (sorteioforja >= chanceforja)
												{


												char itemname[512];
												char itemsor[512];
												int Sorteioitem = GetRandomPos(0, 100);

												if (Sorteioitem > 50)
												{
													wsprintf(itemsor, "%s", CodItem1);
													wsprintf(itemname, "%s", Codname1);
												}
												else
												{
													wsprintf(itemsor, "%s", CodItem2);
													wsprintf(itemname, "%s", Codname2);
												}

												if (CreateItem(lpsItem, itemsor, 0) == TRUE)
												{
													TRANS_ITEMINFO		TransItemInfo;
													//ReformItem(&lpsItem->ItemInfo);
													TransItemInfo.code = OpCode::OPCODE_PUTITEM;
													TransItemInfo.size = sizeof(TRANS_ITEMINFO);
													memcpy(&TransItemInfo.Item, &lpsItem->ItemInfo, sizeof(sITEMINFO));
													rsRegist_ItemSecCode(Player, &TransItemInfo, 1);
													if (SummitJobsAddInventoryItem(Player, TransItemInfo) && Player->lpsmSock)
														SummitJobsSend(Player, &TransItemInfo, TransItemInfo.size);
												}

												TRANS_CHATMESSAGE	TransChatMessage;
												wsprintf(TransChatMessage.szMessage, "Voc\352 Forjou 1 '%s'", itemname );
												TransChatMessage.code = OpCode::OPCODE_WHISPERMESSAGE;
												TransChatMessage.size = 32 + lstrlen(TransChatMessage.szMessage);
												TransChatMessage.dwIP = 3;
												TransChatMessage.dwObjectSerial = 0;

												if (Player->lpsmSock)
													SummitJobsSend(Player, &TransChatMessage, TransChatMessage.size);

												//Criar log na pasta do server do resgate do BP
												HANDLE	hFile;
												DWORD	dwAcess;
												DWORD	FileLength;
												char	szFileName[128];
												char	szBuff[256];
												char* lpMsg;
												char* HackLogDirectory = "LogFile";

												wsprintf(szFileName, "%s//(FORJA) %d - %d.log", HackLogDirectory, tm.tm_mon + 1, tm.tm_mday);

												lpMsg = "%d:%d:%d - ID:( %s ) / Name:( %s ) / IP ( %s ) - Forjou ( %d ) ( %s ) /r/n";
												wsprintf(szBuff, lpMsg,
													tm.tm_hour, tm.tm_min, tm.tm_sec, Player->getAccount(), Player->getName(),
													Player->lpsmSock->szIPAddr, 1, itemname);

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
												TRANS_CHATMESSAGE	TransChatMessage;
												wsprintf(TransChatMessage.szMessage, "Voc\352 Falhou no processo de Forja!");
												TransChatMessage.code = OpCode::OPCODE_WHISPERMESSAGE;
												TransChatMessage.size = 32 + lstrlen(TransChatMessage.szMessage);
												TransChatMessage.dwIP = 3;
												TransChatMessage.dwObjectSerial = 0;

												if (Player->lpsmSock)
													SummitJobsSend(Player, &TransChatMessage, TransChatMessage.size);

												//Criar log na pasta do server do resgate do BP
												HANDLE	hFile;
												DWORD	dwAcess;
												DWORD	FileLength;
												char	szFileName[128];
												char	szBuff[256];
												char* lpMsg;
												char* HackLogDirectory = "LogFile";

												wsprintf(szFileName, "%s//(FORJA) %d - %d.log", HackLogDirectory, tm.tm_mon + 1, tm.tm_mday);

												lpMsg = "%d:%d:%d - ID:( %s ) / Name:( %s ) / IP ( %s ) - Tentou Forjar ( %d ) ( %s ) /r/n";
												wsprintf(szBuff, lpMsg,
													tm.tm_hour, tm.tm_min, tm.tm_sec, Player->getAccount(), Player->getName(),
													Player->lpsmSock->szIPAddr, 1, "item");

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

							}
						}
						



					}

					else
					{
						TRANS_CHATMESSAGE	TransChatMessage;
						wsprintf(TransChatMessage.szMessage, "Voc\352 n\343o tem materiais suficientes para realizar a Forja!");
						TransChatMessage.code = OpCode::OPCODE_WHISPERMESSAGE;
						TransChatMessage.size = 32 + lstrlen(TransChatMessage.szMessage);
						TransChatMessage.dwIP = 3;
						TransChatMessage.dwObjectSerial = 0;

						if (Player->lpsmSock)
							SummitJobsSend(Player, &TransChatMessage, TransChatMessage.size);
					}


				}
		}

		SQL::GetInstance()->LeaveSqlSection();
		JOBS::GetInstance()->SendJOBInfoToGame(Player);
		delete lpsItem;

		if (professionStateChanged)
			gameplay::progression::NotifyServerProgressionPlayerStateChanged(
                gameplay::player::legacy::ServerLegacyOnlinePlayerRegistry().IdOf(Player));
	}
}


//ADICIONA EXPERIÊNCIA NA JOB
void JOBS::AddExpJOBS(rsPLAYINFO* Player, int valor)
{
	if (SQL::GetInstance()->IsConnected())
	{
		bool professionStateChanged = false;
		int JOBUser_Exp = 0;
		int JOB_LVL = 0;
		char szQuery[512];
		SQL::GetInstance()->EnterSqlSection();
		wsprintf(szQuery, "SELECT * FROM [JOBS].[dbo].[Jobs_Info] WHERE [ID]='%s'", Player->getAccount());
		if (SQL::GetInstance()->Execute(szQuery))
		{
			if (SQL::GetInstance()->Fetch())
			{
				SQL::GetInstance()->GetDataValue(4, &JOB_LVL);
				SQL::GetInstance()->GetDataValue(5, &JOBUser_Exp);
			}

			// Calculo feito para saber se o usuário já atingiu à experiência necessária para evoluir o nível do passe de batalha
			JOBUser_Exp = JOBUser_Exp + valor;

			if ((JOBUser_Exp >= 999) && (JOB_LVL < 20))
			{
				JOBUser_Exp = JOBUser_Exp - 999;
				JOB_LVL++;

				//ENVIA MENSAGEM PARA O JOGADOR ASSIM QUE ELE UPA DE NIVEL
				TRANS_CHATMESSAGE	TransChatMessage;
				wsprintf(TransChatMessage.szMessage, "Parab\351ns voc\352 atingiu o n\355vel %d em Especializa\347\343o", JOB_LVL);
				TransChatMessage.code = OpCode::OPCODE_WHISPERMESSAGE;
				TransChatMessage.size = 32 + lstrlen(TransChatMessage.szMessage);
				TransChatMessage.dwIP = 3;
				TransChatMessage.dwObjectSerial = 0;

				if (Player->lpsmSock)
					SummitJobsSend(Player, &TransChatMessage, TransChatMessage.size);
			}
			else if (JOB_LVL >= 20) {
				JOBUser_Exp = 999;
			}

			TRANS_CHATMESSAGE pvp;

			wsprintf(pvp.szMessage, "Especializa\347\343o > Exp %d", JOBUser_Exp);
			pvp.code = OpCode::OPCODE_WHISPERMESSAGE;
			pvp.size = 32 + lstrlen(pvp.szMessage);
			pvp.dwIP = 9;
			pvp.dwObjectSerial = 0;

			if (Player->lpsmSock)
				SummitJobsSend(Player, &pvp, pvp.size);
			//
			//
			if (JOBUser_Exp < 999) {
				wsprintf(szQuery, "UPDATE [JOBS].[dbo].[Jobs_Info] SET Job_xp=%d WHERE[ID] = '%s'", JOBUser_Exp, Player->getAccount());
				if (SQL::GetInstance()->Execute(szQuery))
				{
					professionStateChanged = true;
				}
				wsprintf(szQuery, "UPDATE [JOBS].[dbo].[Jobs_Info] SET Job_Level=%d WHERE[ID] = '%s'", JOB_LVL, Player->getAccount());
				if (SQL::GetInstance()->Execute(szQuery))
				{
					professionStateChanged = true;
				}
			}
		}
		SQL::GetInstance()->LeaveSqlSection();

		if (professionStateChanged)
			gameplay::progression::NotifyServerProgressionPlayerStateChanged(
                gameplay::player::legacy::ServerLegacyOnlinePlayerRegistry().IdOf(Player));
	}
}

//ADICIONA MINERIOS
void JOBS::AddminJOBS(rsPLAYINFO* Player, int valor, int type)
{
	if (SQL::GetInstance()->IsConnected())
	{
		int M_A = 0;
		int M_F = 0;
		int M_L = 0;
		int M_M = 0;
		int M_R = 0;
		char szQuery[512];
		SQL::GetInstance()->EnterSqlSection();
		wsprintf(szQuery, "SELECT * FROM [JOBS].[dbo].[Jobs_Info] WHERE [ID]='%s'", Player->getAccount());
		if (SQL::GetInstance()->Execute(szQuery))
		{
			if (SQL::GetInstance()->Fetch())
			{
				SQL::GetInstance()->GetDataValue(6, &M_A);
				SQL::GetInstance()->GetDataValue(7, &M_F);
				SQL::GetInstance()->GetDataValue(8, &M_L);
				SQL::GetInstance()->GetDataValue(9, &M_M);
				SQL::GetInstance()->GetDataValue(10, &M_R);
			}

			int Min = 0;
			char sznamemin[512];
			char sznamemin2[512];
			// ADD min no min coerente
			switch (type)
			{
			case 1:
				M_A = M_A + valor;
				wsprintf(sznamemin, "Essencias");
				Min = M_A;
				wsprintf(szQuery, "UPDATE [JOBS].[dbo].[Jobs_Info] SET M_A=%d WHERE[ID] = '%s'", Min, Player->getAccount());
				if (SQL::GetInstance()->Execute(szQuery))
				{
				}
				break;
			case 2:
				M_F = M_F + valor;
				wsprintf(sznamemin, "Ferros");
				Min = M_F;
				wsprintf(szQuery, "UPDATE [JOBS].[dbo].[Jobs_Info] SET M_F=%d WHERE[ID] = '%s'", Min, Player->getAccount());
				if (SQL::GetInstance()->Execute(szQuery))
				{
				}
				break;
			case 3:
				M_L = M_L + valor;
				wsprintf(sznamemin, "Madeiras");
				Min = M_L;
				wsprintf(szQuery, "UPDATE [JOBS].[dbo].[Jobs_Info] SET M_L=%d WHERE[ID] = '%s'", Min, Player->getAccount());
				if (SQL::GetInstance()->Execute(szQuery))
				{
				}
				break;
			case 4:
				M_M = M_M + valor;
				wsprintf(sznamemin, "Minerios");
				Min = M_M;
				wsprintf(szQuery, "UPDATE [JOBS].[dbo].[Jobs_Info] SET M_M=%d WHERE[ID] = '%s'", Min, Player->getAccount());
				if (SQL::GetInstance()->Execute(szQuery))
				{
				}
				break;
			case 5:
				M_R = M_R + valor;
				wsprintf(sznamemin, "Energias");
				Min = M_R;
				wsprintf(szQuery, "UPDATE [JOBS].[dbo].[Jobs_Info] SET M_R=%d WHERE[ID] = '%s'", Min, Player->getAccount());
				if (SQL::GetInstance()->Execute(szQuery))
				{
				}
				break;
			
			}
			

			TRANS_CHATMESSAGE pvp;

			wsprintf(pvp.szMessage, "Materiais > %s: %d", sznamemin, Min);
			pvp.code = OpCode::OPCODE_WHISPERMESSAGE;
			pvp.size = 32 + lstrlen(pvp.szMessage);
			pvp.dwIP = 9;
			pvp.dwObjectSerial = 0;

			if (Player->lpsmSock)
				SummitJobsSend(Player, &pvp, pvp.size);
			//
			//

			
		}
		SQL::GetInstance()->LeaveSqlSection();
	}
}


