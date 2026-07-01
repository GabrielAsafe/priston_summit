#pragma once


class CCaravanaHandler
{
private:

public:
	static CCaravanaHandler* GetInstance() { static CCaravanaHandler instance; return &instance; };

	void AddCaravan(rsPLAYINFO* lpPlayInfo);
	void DelCaravan(smCHAR* lpChar);
};

#define HOSKILL_CARAVAN -1

#define CARAVANHANDLER CCaravanaHandler::GetInstance()