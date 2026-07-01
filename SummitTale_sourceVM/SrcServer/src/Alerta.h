#pragma once

struct AlertaInfo
{
	DWORD Head, ChkSum;
	char ItemName[32];
	char MonsterName[32];
	int iMap;
};

class Alerta
{
public:
	static Alerta* getInstance();

	void Init();

	void AddItemLog(rsPLAYINFO* lpPlayInfo, smCHAR* lpChar, sITEMINFO* lpItemInfo);
	void ProcessItemLog(char* PlayerName, DWORD Code, DWORD Head, DWORD ChkSum);

private:
	vector<AlertaInfo> vAlertaInfo;
	map<DWORD, bool> vEnabled;
};

#define ALERTAHANDLER Alerta::getInstance()