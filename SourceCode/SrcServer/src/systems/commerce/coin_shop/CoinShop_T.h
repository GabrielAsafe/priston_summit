#include "skillsPacket.h"
#pragma once

#include "core/server/legacy_sin_runtime/sinLinkHeader.h"
#include "systems/persistence/database/core/Database.h"

class cCOINSHOP_T
{
public:
	cCOINSHOP_T();
	~cCOINSHOP_T();

	void LoadItems();
	void RecvGetItemShop(rsPLAYINFO* lpPlayInfo, smTRANS_COMMAND* lpTransCommand);
	void RecvGetItemCoin(rsPLAYINFO* lpPlayInfo);
	void RecvBuyShopItem(rsPLAYINFO* lpPlayInfo, smTRANS_BUYSHOP* lpTransBuy);
	int GetPlayerCoin(rsPLAYINFO * lpPlayInfo);
	void AddPlayerCoin(rsPLAYINFO * lpPlayInfo, int coin);

private:
	struct sITEMSHOP
	{
		char szItemCode[32];
		int Price;
		char ItemName[32];
	};

	vector<sITEMSHOP*> vItemShop[7];
};

extern cCOINSHOP_T cCoinShop_T;


