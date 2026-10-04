#include "systems/network/protocol/server/opcodes.h"
#include "core/server/legacy_sin_runtime/sinLinkHeader.h"
#include "systems/commerce/restoration/cRestaureHandler.h"
#include "systems/persistence/database/core/Database.h"
#include "core/server/legacy_runtime/onserver.h"
#include "systems/commerce/coin_shop/CoinShop.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"
#include "systems/network/v2/legacy/LegacyPacketSender.h"
#include "Shared/gameplay/commerce/v2/RestorationPricingPolicy.h"
#include "Shared/gameplay/commerce/v2/RestorationTransactionService.h"
#include "systems/commerce/v2/legacy/LegacyCoinWalletAdapter.h"
#include "systems/commerce/restoration/v2/legacy/LegacyRestorationTransactionAdapters.h"

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

namespace
{
bool SummitRestorationSend(rsPLAYINFO *recipient, const void *data, int size)
{
	if(!recipient || !data || size <= 0)
		return false;

	const gameplay::PlayerId playerId =
		gameplay::player::legacy::ServerLegacyOnlinePlayerRegistry().IdOf(recipient);
	return gameplay::network::legacy::ServerLegacyPacketSender().Send(
		playerId, gameplay::network::PacketView(data, static_cast<std::size_t>(size), 0u)).Succeeded();
}
}

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
	(void)coinType;
	(void)StoneCode;
	(void)StoneHead;
	(void)StoneSum;

	char playerID[32] = { 0 };
	char playerName[32] = { 0 };
	char itemName[128] = { 0 };
	int itemAging = 0;
	int Price = gameplay::commerce::RestorationPricingPolicy::UnsupportedPrice;
	bool recordLoaded = false;

	std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_SecurityDB);
	if (lpPlayInfo && db && db->Open())
	{
		if (db->Prepare("SELECT * FROM AgingFailed WHERE PlayerID=? AND PlayerName=? AND Head=? AND ChkSum=?"))
		{
			lstrcpy(playerID, lpPlayInfo->getAccount());
			lstrcpy(playerName, lpPlayInfo->getName());
			db->BindInputParameter(playerID, 1, PARAMTYPE_String);
			db->BindInputParameter(playerName, 2, PARAMTYPE_String);
			db->BindInputParameter(&Head, 3, PARAMTYPE_Integer);
			db->BindInputParameter(&dwChkSum, 4, PARAMTYPE_Integer);
			if (db->Execute())
			{
				db->GetData(itemName, sizeof(itemName), 3, PARAMTYPE_String);
				db->GetData(&itemAging, 0, 4, PARAMTYPE_Integer);

				char buff[255];
				wsprintf(buff, "%d@%d", Head, dwChkSum);
				psITEM psItem;
				if (srSetItemFromLogData(&psItem, buff))
				{
					Price = gameplay::commerce::RestorationPricingPolicy::ChargePrice(
						psItem.ItemInfo.ItemAgingNum[0]);
					recordLoaded = true;
				}
			}
		}
		db->Close();
	}

	if (recordLoaded && Price != gameplay::commerce::RestorationPricingPolicy::UnsupportedPrice)
	{
		gameplay::commerce::RestorationTransactionRequest request;
		request.player = gameplay::player::legacy::ServerLegacyOnlinePlayerRegistry().IdOf(lpPlayInfo);
		request.price = Price;
		request.record.account = playerID;
		request.record.character = playerName;
		request.record.itemName = itemName;
		request.record.itemAging = itemAging;
		request.record.head = static_cast<std::uint32_t>(Head);
		request.record.checksum = static_cast<std::uint32_t>(dwChkSum);

		gameplay::commerce::legacy::LegacyRestorationClaimStore claimStore;
		gameplay::commerce::legacy::LegacyRestorationGrant grant(lpPlayInfo);
		gameplay::commerce::RestorationTransactionService transaction(
			gameplay::commerce::legacy::ServerLegacyCoinWalletAdapter(),
			gameplay::commerce::legacy::ServerLegacyCoinWalletAdapter(),
			claimStore, grant);
		(void)transaction.Restore(request);
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

						restaureList.ItemList[restaureList.Count].CoinPrice =
							gameplay::commerce::RestorationPricingPolicy::QuotedPrice(
								psItem.ItemInfo.ItemAgingNum[0]);

						restaureList.Count++;

						if (restaureList.Count >= 15)
						{
							(void)SummitRestorationSend(lpPlayInfo, &restaureList, restaureList.size);

							restaureList.Count = 0;
							ZeroMemory(&restaureList.ItemList, sizeof(sITEMINFO) * 15);
						}
					}
				}
			}
		}
		db->Close();
	}

	(void)SummitRestorationSend(lpPlayInfo, &restaureList, restaureList.size);
}


