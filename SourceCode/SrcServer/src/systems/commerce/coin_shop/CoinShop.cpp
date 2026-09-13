#include <cstring>
#include "systems/network/protocol/server/opcodes.h"
#include "systems/commerce/coin_shop/CoinShop.h"

cCOINSHOP cCoinShop;

cCOINSHOP::cCOINSHOP()
{
	
}


cCOINSHOP::~cCOINSHOP()
{
	for (int i = 0; i < 7; i++)
	{
		if (vItemShop[i].size())
		{
			for (auto& itemShop : vItemShop[i])
			{
				if (itemShop)
				{
					delete itemShop;
					itemShop = 0;
				}
			}

			vItemShop[i].clear();
		}
	}
}

void cCOINSHOP::LoadItems()
{
	std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_CoinShopDB);

	for (int i = 0; i < 7; i++)
	{
		if (vItemShop[i].size())
		{
			for (auto& itemShop : vItemShop[i])
			{
				if (itemShop)
				{
					delete itemShop;
					itemShop = 0;
				}
			}

			vItemShop[i].clear();
		}
	}


	if (db && db->Open())
	{
		if (db->Prepare("SELECT * FROM ItemList WHERE ItemSection > 0 AND ItemSection < 8"))
		{
			if (db->Execute(false))
			{
				while (db->NextRow())
				{
					sITEMSHOP* pItemShop = new sITEMSHOP;
					int Section = 0;

					db->GetData(pItemShop->szItemCode, 32, 1, PARAMTYPE_String);
					db->GetData(&Section, 4, 2, PARAMTYPE_Integer);
					db->GetData(&pItemShop->Price, 4, 3, PARAMTYPE_Integer);

					if (Section > 0 && Section <= 7)
						vItemShop[Section - 1].push_back(pItemShop);
				}
			}
		}
		db->Close();
	}
}


#define	DEF_ITEM_MAX		2000

extern int DefaultItemCount;
extern sDEF_ITEMINFO DefaultItems[DEF_ITEM_MAX];

struct psITEM
{
	int		state;
	int		Serial;
	sITEMINFO	ItemInfo;
};

int	rsRegist_ItemSecCode(rsPLAYINFO* lpPlayInfo, TRANS_ITEMINFO* lpTransItemInfo, int NewItem);
int CreateItem(psITEM* lpsItem, char* dwCode, int JobCode);
int	rsAddInvenItem(rsPLAYINFO* lpPlayInfo, DWORD dwCode, DWORD dwHead, DWORD dwChkSum);

void cCOINSHOP::RecvGetItemShop(rsPLAYINFO* lpPlayInfo, smTRANS_COMMAND * lpTransCommand)
{
	if (lpTransCommand->WParam <= 0 || lpTransCommand->WParam > 9 || !lpPlayInfo->lpsmSock)
		return;



	int Section = lpTransCommand->WParam - 1;
	int cnt = 0;
	int itemCount = 0;
	DWORD dwCode = 0;
	sDEF_ITEMINFO* lpDefItem = 0;
	smTRANS_SHOPDATA TransShopData;

	if (vItemShop[Section].size())
	{
		TransShopData.code = OpCode::OPCODE_GET_SHOPITEM;
		TransShopData.size = sizeof(smTRANS_SHOPDATA);
		TransShopData.Section = lpTransCommand->WParam;

		for (auto& itemShop : vItemShop[Section])
		{
			if (itemShop && itemShop->szItemCode[0])
			{
				for (cnt = 0; cnt < MAX_ITEM; cnt++)
				{
					if (!lstrcmpi(itemShop->szItemCode, sItem[cnt].LastCategory))
					{
						dwCode = sItem[cnt].CODE;
						break;
					}
				}

				if (dwCode)
				{
					for (cnt = 0; cnt < DefaultItemCount; cnt++) {
						if (!DefaultItems[cnt].Item.UniqueItem && dwCode == DefaultItems[cnt].Item.CODE) {
							lpDefItem = &DefaultItems[cnt];
							break;
						}
					}

					memcpy(&TransShopData.sCompItem[itemCount], lpDefItem->DefCompressData, lpDefItem->DefCompressDataLen);
					TransShopData.sCompItem[itemCount].Price = itemShop->Price;
					itemCount++;

					if (itemCount >= 15)
					{
						TransShopData.Count = itemCount;
						lpPlayInfo->lpsmSock->Send((char*)&TransShopData, TransShopData.size, TRUE);
						ZeroMemory(TransShopData.sCompItem, sizeof(TransShopData.sCompItem));
						itemCount = 0;
						TransShopData.Count = 0;
					}
				}
			}
		}

		if (itemCount > 0)
		{
			TransShopData.Count = itemCount;
			lpPlayInfo->lpsmSock->Send((char*)&TransShopData, TransShopData.size, TRUE);
		}
	}
}


void cCOINSHOP::RecvGetItemCoin(rsPLAYINFO * lpPlayInfo)
{
	std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_CoinShopDB);
	smTRANS_COMMAND TransCommand = {};

	if (db->Open())
	{
		char playerID[32];
		wsprintf(playerID, "%s", lpPlayInfo->getAccount());
		if (db->Prepare("INSERT INTO UserInfo (UserID, UserCoin, LastChange) SELECT ?2, 0, datetime('now','localtime') WHERE NOT EXISTS(SELECT 1 FROM UserInfo WHERE UserID=?1)"))
		{ 
			db->BindInputParameter(playerID, 1, PARAMTYPE_String);
			db->BindInputParameter(playerID, 2, PARAMTYPE_String);
			db->Execute();
		}

		if (db->Prepare("SELECT * FROM UserInfo WHERE UserID=?"))
		{
			db->BindInputParameter(playerID, 1, PARAMTYPE_String);
			if (db->Execute())
			{
				TransCommand.size = sizeof(smTRANS_COMMAND);
				TransCommand.code = OpCode::OPCODE_GET_SHOPCOIN;
				db->GetData(&TransCommand.WParam, 4, 2, PARAMTYPE_Integer);
				lpPlayInfo->lpsmSock->Send((char*)&TransCommand, TransCommand.size, TRUE);
			}
		}

		db->Close();
	}
}

void cCOINSHOP::RecvBuyShopItem(rsPLAYINFO * lpPlayInfo, smTRANS_BUYSHOP * lpTransBuy)
{
	psITEM* lpsItem;
	lpsItem = new psITEM;
	std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_CoinShopDB);

	if (db->Open())
	{
		sITEMSHOP* pItemShop = new sITEMSHOP;
		char playerID[32];
		wsprintf(playerID, "%s", lpPlayInfo->getAccount());
		if (db->Prepare("SELECT * FROM ItemList WHERE szItemCode=?"))
		{
			db->BindInputParameter(lpTransBuy->szItemCode, 1, PARAMTYPE_String);
			if (db->Execute())
			{
				int Price = 0;
				int PlayerCoin = 0;

				db->GetData(&Price, 4, 3, PARAMTYPE_Integer);

				if (Price > 0)
				{
					if (db->Prepare("SELECT * FROM UserInfo WHERE UserID=?"))
					{
						db->BindInputParameter(playerID, 1, PARAMTYPE_String);
						if (db->Execute())
						{
							db->GetData(&PlayerCoin, 4, 2, PARAMTYPE_Integer);

							if (PlayerCoin >= Price)
							{
								PlayerCoin -= Price;
								if (db->Prepare("UPDATE UserInfo SET UserCoin=?, LastChange=GETDATE() WHERE UserID=?"))
								{
									db->BindInputParameter(&PlayerCoin, 1, PARAMTYPE_Integer);
									db->BindInputParameter(playerID, 2, PARAMTYPE_String);

									if (db->Execute(false))
									{
										if (lpTransBuy->szItemCode[0] == 'P' || lpTransBuy->szItemCode[0] == 'p' || lpTransBuy->szItemCode[0] == 'G' || lpTransBuy->szItemCode[0] == 'g')
										{

										}
										else

											if (CreateItem(lpsItem, lpTransBuy->szItemCode, lpPlayInfo->smCharInfo.JOB_CODE) == TRUE)
											{
												TRANS_ITEMINFO		TransItemInfo;
												ReformItem(&lpsItem->ItemInfo);
												TransItemInfo.code = OpCode::OPCODE_PUTITEM;
												TransItemInfo.size = sizeof(TRANS_ITEMINFO);
												memcpy(&TransItemInfo.Item, &lpsItem->ItemInfo, sizeof(sITEMINFO));
												rsRegist_ItemSecCode(lpPlayInfo, &TransItemInfo, 1);
												rsAddInvenItem(lpPlayInfo, TransItemInfo.Item.CODE, TransItemInfo.Item.ItemHeader.Head, TransItemInfo.Item.ItemHeader.dwChkSum);
												if (lpPlayInfo->lpsmSock)
													lpPlayInfo->lpsmSock->Send((char*)&TransItemInfo, TransItemInfo.size, TRUE);
											}

									}

								}

								HANDLE hFile;
								char szFileName[255];
								char szLog[500];
								SYSTEMTIME	SystemTime;

								GetLocalTime(&SystemTime);
								sprintf(szFileName, "COINSHOP_LOG//%d-%d.log", SystemTime.wMonth, SystemTime.wDay);
								CreateDirectory("COINSHOP_LOG", NULL);
								sprintf(szLog, "%d:%d:%d - (ID : %s) (Name : %s) buy (Coin : %d) (CODE : %s)/r/n", SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond,
									playerID, lpPlayInfo->getName(), PlayerCoin, lpTransBuy->szItemCode);

								hFile = CreateFile(szFileName, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
								if (hFile != INVALID_HANDLE_VALUE)
								{
									DWORD dwFileLength = GetFileSize(hFile, NULL);
									DWORD dwAccess;
									SetFilePointer(hFile, dwFileLength, NULL, FILE_BEGIN);
									WriteFile(hFile, szLog, strlen(szLog), &dwAccess, NULL);
									CloseHandle(hFile);
								}
							}
						}
					}
				}
			}
		}
		db->Close();
	}

	RecvGetItemCoin(lpPlayInfo);
}

int cCOINSHOP::GetPlayerCoin(rsPLAYINFO * lpPlayInfo)
{
	std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_CoinShopDB);
	char playerID[32];
	int coin;

	lstrcpy(playerID, lpPlayInfo->getAccount());

	if (db->Open())
	{
		if (db->Prepare("INSERT INTO UserInfo (UserID, UserCoin, LastChange) SELECT ?2, 0, datetime('now','localtime') WHERE NOT EXISTS(SELECT 1 FROM UserInfo WHERE UserID=?1)"))
		{
			db->BindInputParameter(playerID, 1, PARAMTYPE_String);
			db->BindInputParameter(playerID, 2, PARAMTYPE_String);
			db->Execute();
		}

		if (db->Prepare("SELECT * FROM UserInfo WHERE UserID=?"))
		{
			db->BindInputParameter(playerID, 1, PARAMTYPE_String);
			if (db->Execute())
			{
				db->GetData(&coin, 4, 2, PARAMTYPE_Integer);
			}
		}

		db->Close();
	}

	return coin;
}

void cCOINSHOP::AddPlayerCoin(rsPLAYINFO * lpPlayInfo, int coin)
{
	std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_CoinShopDB);
	char playerID[32];

	lstrcpy(playerID, lpPlayInfo->getAccount());

	if (db->Open())
	{
		if (db->Prepare("UPDATE UserInfo SET UserCoin=UserCoin + ? WHERE UserID=?"))
		{
			db->BindInputParameter(&coin, 1, PARAMTYPE_Integer);
			db->BindInputParameter(playerID, 2, PARAMTYPE_String);
			db->Execute();
		}

		db->Close();
	}

	
}


