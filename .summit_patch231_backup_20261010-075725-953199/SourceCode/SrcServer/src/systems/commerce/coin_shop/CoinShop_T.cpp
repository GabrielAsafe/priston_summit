#include <cstring>
#include "systems/network/protocol/server/opcodes.h"
#include "systems/commerce/coin_shop/CoinShop_T.h"
#include "content/v1/commerce/coin_shop/content/CoinShopRuntime.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"
#include "systems/inventory/v2/legacy/LegacyInventoryRuntime.h"
#include "systems/network/v2/legacy/LegacyPacketSender.h"
#include "gameplay/commerce/v2/PremiumPurchaseTransactionService.h"
#include "gameplay/commerce/v2/PremiumPurchaseReplayGuard.h"
#include "systems/commerce/v2/legacy/SQLitePremiumPurchaseReplayStore.h"
#include "systems/commerce/v2/legacy/LegacyTimeCreditWalletAdapter.h"

namespace
{
bool SummitCoinShopTSend(rsPLAYINFO *recipient, const void *data, int size)
{
	if(!recipient || !data || size <= 0)
		return false;
	const gameplay::PlayerId playerId =
		gameplay::player::legacy::ServerLegacyOnlinePlayerRegistry().IdOf(recipient);
	return gameplay::network::legacy::ServerLegacyPacketSender().Send(
		playerId, gameplay::network::PacketView(data, static_cast<std::size_t>(size), 0u)).Succeeded();
}

gameplay::commerce::IPremiumPurchaseReplayStore& SummitTimeShopReplayGuard()
{
	static gameplay::commerce::legacy::SQLitePremiumPurchaseReplayStore store;
	return store;
}

bool SummitTimeShopCatalogPrice(const char* code, int& price, bool& canonicalAvailable)
{
	price = 0;
	canonicalAvailable = false;
	const summit::content::commerce::coin_shop::CompiledCoinShopSource* catalog =
		summit::content::commerce::coin_shop::ActiveCoinShopSource();
	if(!catalog || !catalog->IsValid())
		return false;
	canonicalAvailable = true;
	summit::content::commerce::coin_shop::CoinShopCatalogEntry entry;
	if(!catalog->Find(summit::content::commerce::coin_shop::COIN_SHOP_KIND_TIME, code, entry))
		return false;
	price = entry.price;
	return price > 0;
}
}

cCOINSHOP_T cCoinShop_T;

namespace
{
	bool QuestOnlyWingCode(const char* code)
	{
		return code && (!lstrcmpi(code, "QW103") || !lstrcmpi(code, "QW106"));
	}
}

cCOINSHOP_T::cCOINSHOP_T()
{
}

cCOINSHOP_T::~cCOINSHOP_T()
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

void cCOINSHOP_T::LoadItems()
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

	const summit::content::commerce::coin_shop::CompiledCoinShopSource* catalog =
		summit::content::commerce::coin_shop::ActiveCoinShopSource();
	if (catalog && catalog->IsValid())
	{
		std::vector<summit::content::commerce::coin_shop::CoinShopCatalogEntry> entries;
		if (catalog->Entries(summit::content::commerce::coin_shop::COIN_SHOP_KIND_TIME, entries))
		{
			for (std::size_t i = 0; i < entries.size(); ++i)
			{
				const summit::content::commerce::coin_shop::CoinShopCatalogEntry& entry = entries[i];
				sITEMSHOP* pItemShop = new sITEMSHOP;
				ZeroMemory(pItemShop, sizeof(sITEMSHOP));
				strcpy_s(pItemShop->szItemCode, sizeof(pItemShop->szItemCode), entry.code.c_str());
				pItemShop->Price = entry.price;
				if (entry.section >= 1 && entry.section <= 7)
					vItemShop[entry.section - 1].push_back(pItemShop);
				else
					delete pItemShop;
			}
		}
		return; // Valid canonical pack is authoritative, including an intentionally empty catalog.
	}

	// Rollback compatibility only: if the compiled canonical pack is missing or
	// invalid, keep the legacy static DB catalog readable. Wallet state remains DB-backed.
	if (db && db->Open())
	{
		if (db->Prepare("SELECT * FROM ItemList_T WHERE ItemSection > 0 AND ItemSection < 8"))
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

					if (QuestOnlyWingCode(pItemShop->szItemCode))
					{
						delete pItemShop;
						continue;
					}

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

void cCOINSHOP_T::RecvGetItemShop(rsPLAYINFO* lpPlayInfo, smTRANS_COMMAND * lpTransCommand)
{
	if (lpTransCommand->WParam <= 0 || lpTransCommand->WParam > 7 || !lpPlayInfo->lpsmSock)
		return;



	int Section = lpTransCommand->WParam - 1;
	int cnt = 0;
	int itemCount = 0;
	DWORD dwCode = 0;
	sDEF_ITEMINFO* lpDefItem = 0;
	smTRANS_SHOPDATA TransShopData = {};

	if (vItemShop[Section].size())
	{
		TransShopData.code = OpCode::OPCODE_GET_SHOPITEM_T;
		TransShopData.size = sizeof(smTRANS_SHOPDATA);
		TransShopData.Section = lpTransCommand->WParam;

		for (auto& itemShop : vItemShop[Section])
		{
			dwCode = 0;
			lpDefItem = 0;
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

					if (!lpDefItem)
						continue;

					memcpy(&TransShopData.sCompItem[itemCount], lpDefItem->DefCompressData, lpDefItem->DefCompressDataLen);
					TransShopData.sCompItem[itemCount].Price = itemShop->Price;
					itemCount++;

					if (itemCount >= 15)
					{
						TransShopData.Count = itemCount;
						SummitCoinShopTSend(lpPlayInfo, &TransShopData, TransShopData.size);
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
			SummitCoinShopTSend(lpPlayInfo, &TransShopData, TransShopData.size);
		}
	}
}

void cCOINSHOP_T::RecvGetItemCoin(rsPLAYINFO * lpPlayInfo)
{
	std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_CoinShopDB);
	smTRANS_COMMAND TransCommand = {};

	if (db->Open())
	{
		char playerID[32];
		wsprintf(playerID, "%s", lpPlayInfo->getAccount());
		if (db->Prepare("INSERT INTO UserInfo_T (UserID, UserMinutos, LastChange) SELECT ?2, 0, datetime('now','localtime') WHERE NOT EXISTS(SELECT 1 FROM UserInfo_T WHERE UserID=?1)"))
		{
			db->BindInputParameter(playerID, 1, PARAMTYPE_String);
			db->BindInputParameter(playerID, 2, PARAMTYPE_String);
			db->Execute();
		}

		if (db->Prepare("SELECT * FROM UserInfo_T WHERE UserID=?"))
		{
			db->BindInputParameter(playerID, 1, PARAMTYPE_String);
			if (db->Execute())
			{
				TransCommand.size = sizeof(smTRANS_COMMAND);
				TransCommand.code = OpCode::OPCODE_GET_SHOPCOIN_T;
				db->GetData(&TransCommand.WParam, 4, 2, PARAMTYPE_Integer);
				SummitCoinShopTSend(lpPlayInfo, &TransCommand, TransCommand.size);
			}
		}

		db->Close();
	}
}

namespace
{
class LegacyTimeShopGrantAdapter final : public gameplay::commerce::IPremiumPurchaseGrant
{
public:
	bool Grant(const gameplay::commerce::PremiumPurchaseRequest& request) override
	{
		rsPLAYINFO* player = gameplay::player::legacy::ServerLegacyOnlinePlayerRegistry().Resolve(request.player);
		if(!player || request.itemCode.empty())
			return false;

		const char first = request.itemCode[0];
		if(first == 'P' || first == 'p' || first == 'G' || first == 'g')
			return true; // Preserve the historical no-op grant path for these codes.

		psITEM item = {};
		if(CreateItem(&item, const_cast<char*>(request.itemCode.c_str()), player->smCharInfo.JOB_CODE) != TRUE)
			return false;

		TRANS_ITEMINFO packet = {};
		ReformItem(&item.ItemInfo);
		packet.code = OpCode::OPCODE_PUTITEM;
		packet.size = sizeof(TRANS_ITEMINFO);
		memcpy(&packet.Item, &item.ItemInfo, sizeof(sITEMINFO));
		rsRegist_ItemSecCode(player, &packet, 1);
		const gameplay::inventory::InventoryMutationResult inventoryResult =
			gameplay::inventory::legacy::ServerLegacyInventoryRuntime().Add(
				request.player,
				gameplay::inventory::ItemInstanceId(
					gameplay::ItemId(static_cast<std::uint32_t>(packet.Item.CODE)),
					static_cast<std::uint32_t>(packet.Item.ItemHeader.Head),
					static_cast<std::uint32_t>(packet.Item.ItemHeader.dwChkSum)));
		if(inventoryResult.code != gameplay::inventory::InventoryMutationCode::Applied)
			return false;
		if(player->lpsmSock)
			(void)SummitCoinShopTSend(player, &packet, packet.size);
		return true;
	}
};
}

void cCOINSHOP_T::RecvBuyShopItem(rsPLAYINFO * lpPlayInfo, smTRANS_BUYSHOP * lpTransBuy)
{
	if(!lpPlayInfo || !lpTransBuy)
		return;
	if(lpTransBuy->ItemIndex < 0)
		return;

	gameplay::commerce::PremiumPurchaseOperation operation;
	operation.purchaserKey = lpPlayInfo->getAccount() ? lpPlayInfo->getAccount() : "";
	operation.scope = gameplay::commerce::PremiumPurchaseScope::TimeShop;
	operation.requestId = static_cast<std::uint32_t>(lpTransBuy->ItemIndex);
	operation.itemCode = lpTransBuy->szItemCode;
	const gameplay::commerce::PremiumPurchaseReplayResult replay =
		SummitTimeShopReplayGuard().Reserve(operation);
	if(!replay.MayExecute())
	{
		RecvGetItemCoin(lpPlayInfo);
		return;
	}

	int price = 0;
	bool canonicalCatalogAvailable = false;
	(void)SummitTimeShopCatalogPrice(lpTransBuy->szItemCode, price, canonicalCatalogAvailable);
	if(!canonicalCatalogAvailable && !QuestOnlyWingCode(lpTransBuy->szItemCode))
	{
		std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_CoinShopDB);
		if(db && db->Open())
		{
			if(db->Prepare("SELECT * FROM ItemList_T WHERE szItemCode=?"))
			{
				db->BindInputParameter(lpTransBuy->szItemCode, 1, PARAMTYPE_String);
				if(db->Execute())
					db->GetData(&price, 4, 3, PARAMTYPE_Integer);
			}
			db->Close();
		}
	}

	if(price > 0)
	{
		const gameplay::PlayerId playerId =
			gameplay::player::legacy::ServerLegacyOnlinePlayerRegistry().IdOf(lpPlayInfo);
		LegacyTimeShopGrantAdapter grant;
		gameplay::commerce::legacy::LegacyTimeCreditWalletAdapter& wallet =
			gameplay::commerce::legacy::ServerLegacyTimeCreditWalletAdapter();
		gameplay::commerce::PremiumPurchaseTransactionService transaction(wallet, wallet, grant);
		gameplay::commerce::PremiumPurchaseRequest request;
		request.player = playerId;
		request.price = price;
		request.itemCode = lpTransBuy->szItemCode;
		const gameplay::commerce::PremiumPurchaseResult result = transaction.Purchase(request);
		if(replay.Reserved())
		{
			if(result.Purchased())
				(void)SummitTimeShopReplayGuard().Commit(operation);
			else
				(void)SummitTimeShopReplayGuard().Abort(operation);
		}

		if(result.Purchased())
		{
			HANDLE hFile; char szFileName[255]; char szLog[500]; SYSTEMTIME SystemTime;
			GetLocalTime(&SystemTime);
			sprintf(szFileName, "TIMESHOP_LOG//%d-%d.log", SystemTime.wMonth, SystemTime.wDay);
			CreateDirectory("TIMESHOP_LOG", NULL);
			sprintf(szLog, "%d:%d:%d - (ID : %s) (Name : %s) buy (Time : %d) (CODE : %s)/r/n",
				SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond, lpPlayInfo->getAccount(),
				lpPlayInfo->getName(), static_cast<int>(result.after), lpTransBuy->szItemCode);
			hFile = CreateFile(szFileName, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if(hFile != INVALID_HANDLE_VALUE)
			{
				DWORD dwFileLength = GetFileSize(hFile, NULL); DWORD dwAccess;
				SetFilePointer(hFile, dwFileLength, NULL, FILE_BEGIN);
				WriteFile(hFile, szLog, strlen(szLog), &dwAccess, NULL); CloseHandle(hFile);
			}
		}
	}
	else if(replay.Reserved())
	{
		(void)SummitTimeShopReplayGuard().Abort(operation);
	}

	RecvGetItemCoin(lpPlayInfo);
}

int cCOINSHOP_T::GetPlayerCoin(rsPLAYINFO * lpPlayInfo)
{
	std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_CoinShopDB);
	char playerID[32];
	int coin = -1;

	lstrcpy(playerID, lpPlayInfo->getAccount());

	if (db->Open())
	{
		if (db->Prepare("INSERT INTO UserInfo_T (UserID, UserMinutos, LastChange) SELECT ?2, 0, datetime('now','localtime') WHERE NOT EXISTS(SELECT 1 FROM UserInfo_T WHERE UserID=?1)"))
		{
			db->BindInputParameter(playerID, 1, PARAMTYPE_String);
			db->BindInputParameter(playerID, 2, PARAMTYPE_String);
			db->Execute();
		}

		if (db->Prepare("SELECT * FROM UserInfo_T WHERE UserID=?"))
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

void cCOINSHOP_T::AddPlayerCoin(rsPLAYINFO * lpPlayInfo, int coin)
{
	std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_CoinShopDB);
	char playerID[32];

	lstrcpy(playerID, lpPlayInfo->getAccount());

	if (db->Open())
	{

		if (db->Prepare("INSERT INTO UserInfo_T (UserID, UserMinutos, LastChange) SELECT ?2, 0, datetime('now','localtime') WHERE NOT EXISTS(SELECT 1 FROM UserInfo_T WHERE UserID=?1)"))
		{
			db->BindInputParameter(playerID, 1, PARAMTYPE_String);
			db->BindInputParameter(playerID, 2, PARAMTYPE_String);
			db->Execute();
		}

		if (db->Prepare("UPDATE UserInfo_T SET UserMinutos = UserMinutos + ? WHERE UserID=?"))
		{
			db->BindInputParameter(&coin, 1, PARAMTYPE_Integer);
			db->BindInputParameter(playerID, 2, PARAMTYPE_String);
			db->Execute();
		}
	
		db->Close();
	}
}


