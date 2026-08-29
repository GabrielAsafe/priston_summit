#include "opcodes.h"
#include "sinbaram/sinLinkHeader.h"
#include "cRestaureHandler.h"
#include "Database/Database.h"
#include "Server/onserver.h"
#include "CoinShop.h"

#ifndef SUMMIT_RESTAURE_LIST_DEFINED
#define SUMMIT_RESTAURE_LIST_DEFINED

struct RESTAURE_ITEM
{
    int CoinPrice;
    sITEM psItem;
};

struct RESTAURE_LIST
{
    int size;
    int code;
    int Count;
    RESTAURE_ITEM ItemList[15];
};

#endif


cRESTAURE cRestaure;

extern int srSetItemFromLogData(psITEM *lpsItem, char *LogFileCode);

void cRESTAURE::InsertAgingFailed(rsPLAYINFO* lpPlayInfo, char * ItemName, int ItemAging, int Head, int dwChkSum)
{
	std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_SecurityDB);

	if (db->Open())
	{
		if (db->Prepare("INSERT INTO AgingFailed VALUES (?,?,?,?,?,?,GETDATE())"))
		{
			char playerID[32];
			char playerName[32];
			lstrcpy(playerID, lpPlayInfo->getAccount());
			lstrcpy(playerName, lpPlayInfo->getName());
			db->BindInputParameter(playerID, 1, PARAMTYPE_String);
			db->BindInputParameter(playerName, 2, PARAMTYPE_String);
			db->BindInputParameter(ItemName, 3, PARAMTYPE_String);
			db->BindInputParameter(&ItemAging, 4, PARAMTYPE_Integer);
			db->BindInputParameter(&Head, 5, PARAMTYPE_Integer);
			db->BindInputParameter(&dwChkSum, 6, PARAMTYPE_Integer);
			db->Execute();
		}
		db->Close();
	}
}

extern int CreateCommandItem(rsPLAYINFO *lpPlayInfo, rsPLAYINFO *lpPlayInfo2, char *szItem);
extern int	rsDeleteInvenItem(rsPLAYINFO *lpPlayInfo, DWORD dwCode, DWORD dwHead, DWORD dwChkSum);

void cRESTAURE::RestaureItem(rsPLAYINFO * lpPlayInfo, DWORD Head, DWORD dwChkSum, int coinType, DWORD StoneCode, DWORD StoneHead, DWORD StoneSum)
{
	std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_SecurityDB);

	if (db->Open())
	{
		if (db->Prepare("SELECT * FROM AgingFailed WHERE PlayerID=? AND PlayerName=? AND Head=? AND ChkSum=?"))
		{
			char playerID[32];
			char playerName[32];
			lstrcpy(playerID, lpPlayInfo->getAccount());
			lstrcpy(playerName, lpPlayInfo->getName());
			db->BindInputParameter(playerID, 1, PARAMTYPE_String);
			db->BindInputParameter(playerName, 2, PARAMTYPE_String);
			db->BindInputParameter(&Head, 3, PARAMTYPE_Integer);
			db->BindInputParameter(&dwChkSum, 4, PARAMTYPE_Integer);
			if (db->Execute())
			{
				char buff[255];
				wsprintf(buff, "%d@%d", Head, dwChkSum);

				psITEM psItem;
				if (srSetItemFromLogData(&psItem, buff))
				{
					bool canRestaure = false;

					int Price = 0;
					
					if (psItem.ItemInfo.ItemAgingNum[0] > 0 && psItem.ItemInfo.ItemAgingNum[0] < 8)
						Price = 100000;
					else if (psItem.ItemInfo.ItemAgingNum[0] >= 9 && psItem.ItemInfo.ItemAgingNum[0] < 12)
						Price = 1000;
					else if (psItem.ItemInfo.ItemAgingNum[0] >= 12 && psItem.ItemInfo.ItemAgingNum[0] < 14)
						Price = 2000;
					else if (psItem.ItemInfo.ItemAgingNum[0] >= 14 && psItem.ItemInfo.ItemAgingNum[0] < 18)
						Price = 3000;
					else if (psItem.ItemInfo.ItemAgingNum[0] >= 18 && psItem.ItemInfo.ItemAgingNum[0] < 21)
						Price = 4000;
					else if (psItem.ItemInfo.ItemAgingNum[0] >= 20 && psItem.ItemInfo.ItemAgingNum[0] < 27)
						Price = 5000;
					else
						Price = 999999999;

					if (Price >= 0 && Price <= cCoinShop.GetPlayerCoin(lpPlayInfo))
					{
						canRestaure = true;
						cCoinShop.AddPlayerCoin(lpPlayInfo, -Price);
					}

					if (canRestaure)
					{
						if (db->Prepare("DELETE FROM AgingFailed WHERE PlayerID=? AND PlayerName=? AND Head=? AND ChkSum=?"))
						{
							db->BindInputParameter(playerID, 1, PARAMTYPE_String);
							db->BindInputParameter(playerName, 2, PARAMTYPE_String);
							db->BindInputParameter(&Head, 3, PARAMTYPE_Integer);
							db->BindInputParameter(&dwChkSum, 4, PARAMTYPE_Integer);
							db->Execute();
						}

						char buff2[255];
						wsprintf(buff2, "@%d@%d", Head, dwChkSum);
						CreateCommandItem(lpPlayInfo, lpPlayInfo, buff2);
					}
				}
			}
		}
		db->Close();
	}

	RecvGetRestaure(lpPlayInfo);
}

void cRESTAURE::RecvGetRestaure(rsPLAYINFO * lpPlayInfo)
{
	std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_SecurityDB);
	RESTAURE_LIST restaureList = {};

	cCoinShop.RecvGetItemCoin(lpPlayInfo);

	restaureList.code = OpCode::OPCODE_RESTAURE_LIST;
	restaureList.size = sizeof(RESTAURE_LIST);

	if (db->Open())
	{
		if (db->Prepare("SELECT Head, ChkSum FROM AgingFailed WHERE PlayerID=? AND PlayerName=? ORDER BY Date DESC"))
		{
			char playerID[32];
			char playerName[32];
			lstrcpy(playerID, lpPlayInfo->getAccount());
			lstrcpy(playerName, lpPlayInfo->getName());
			db->BindInputParameter(playerID, 1, PARAMTYPE_String);
			db->BindInputParameter(playerName, 2, PARAMTYPE_String);
			if (db->Execute(false))
			{
				while (db->NextRow())
				{
					DWORD Head, ChkSum;
					psITEM psItem;

					db->GetData(&Head, 4, 1, PARAMTYPE_Integer);
					db->GetData(&ChkSum, 4, 2, PARAMTYPE_Integer);

					char buff[500];
					wsprintf(buff, "%d@%d", Head, ChkSum);

					if (srSetItemFromLogData(&psItem, buff))
					{
						memcpy(
							&restaureList.ItemList[restaureList.Count].psItem.sItemInfo,
							&psItem.ItemInfo,
							sizeof(sITEMINFO)
										);

						restaureList.ItemList[restaureList.Count].psItem.sItemInfo.ItemHeader.dwTime = 0;
						restaureList.ItemList[restaureList.Count].psItem.sItemInfo.ItemHeader.dwVersion = 0;
						restaureList.ItemList[restaureList.Count].psItem.sItemInfo.dwCreateTime = 0;

						if (psItem.ItemInfo.ItemAgingNum[0] > 0 && psItem.ItemInfo.ItemAgingNum[0] < 8)
							restaureList.ItemList[restaureList.Count].CoinPrice = 0;
						else if (psItem.ItemInfo.ItemAgingNum[0] >= 9 && psItem.ItemInfo.ItemAgingNum[0] < 12)
							restaureList.ItemList[restaureList.Count].CoinPrice = 6000;
						else if (psItem.ItemInfo.ItemAgingNum[0] >= 12 && psItem.ItemInfo.ItemAgingNum[0] < 14)
							restaureList.ItemList[restaureList.Count].CoinPrice = 8000;
						else if (psItem.ItemInfo.ItemAgingNum[0] >= 14 && psItem.ItemInfo.ItemAgingNum[0] < 18)
							restaureList.ItemList[restaureList.Count].CoinPrice = 18000;
						else if(psItem.ItemInfo.ItemAgingNum[0] >= 18 && psItem.ItemInfo.ItemAgingNum[0] < 22)
							restaureList.ItemList[restaureList.Count].CoinPrice = 18000;
						else if(psItem.ItemInfo.ItemAgingNum[0] >= 22 && psItem.ItemInfo.ItemAgingNum[0] < 27)
							restaureList.ItemList[restaureList.Count].CoinPrice = 1300;
						else
							restaureList.ItemList[restaureList.Count].CoinPrice = 999999999;

						restaureList.Count++;

						if (restaureList.Count >= 15)
						{
							lpPlayInfo->lpsmSock->Send((char*)&restaureList, restaureList.size, TRUE);

							restaureList.Count = 0;
							ZeroMemory(&restaureList.ItemList, sizeof(sITEMINFO) * 15);
						}
					}
				}
			}
		}
		db->Close();
	}

	lpPlayInfo->lpsmSock->Send((char*)&restaureList, restaureList.size, TRUE);
}


