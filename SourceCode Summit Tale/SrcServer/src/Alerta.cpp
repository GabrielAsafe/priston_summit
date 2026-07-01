#include "sinbaram/sinLinkHeader.h"
#include "Alerta.h"
#include "Database/Database.h"

Alerta cAlertaHandler;

Alerta* Alerta::getInstance()
{
	return &cAlertaHandler;
}

extern int	rsSendServerChatMessageToUser2(int Color, char* msg, ...);

void Alerta::Init()
{
	vEnabled.clear();
	std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_ServerDB);

	if (db->Open())
	{
		if (db->Prepare("SELECT ItemCode FROM DropLogAlert"))
		{
			if (db->Execute(false))
			{
				while (db->NextRow())
				{
					char ItemCode[32];
					db->GetData(ItemCode, 32, 1, PARAMTYPE_String);

					for (int cnt = 0; cnt < MAX_ITEM; cnt++)
					{
						if (!lstrcmpi(sItem[cnt].LastCategory, ItemCode))
						{
							vEnabled[sItem[cnt].CODE] = true;
							break;
						}
					}
				}
			}
		}
		db->Close();
	}
}

void Alerta::AddItemLog(rsPLAYINFO* lpPlayInfo, smCHAR* lpChar, sITEMINFO* lpItemInfo)
{
	if (!vEnabled[lpItemInfo->CODE])
		return;

	AlertaInfo sLog;
	sLog.Head = lpItemInfo->ItemHeader.Head;
	sLog.ChkSum = lpItemInfo->ItemHeader.dwChkSum;
	lstrcpy(sLog.ItemName, lpItemInfo->ItemName);
	vAlertaInfo.push_back(sLog);
}

#include <iostream>
using namespace std;

extern int rsSendMessageAll(char* szText, DWORD dwColor);

void Alerta::ProcessItemLog(char* PlayerName, DWORD Code, DWORD Head, DWORD ChkSum)
{
	if (!vEnabled[Code])
		return;

	for (auto it = vAlertaInfo.begin(); it != vAlertaInfo.end(); it++)
	{
		auto sLog = (*it);
		if (sLog.Head == Head && sLog.ChkSum == ChkSum)
		{

			char mensagem[70] = { 0 };
			wsprintf(mensagem, " [ %s ] Dropou [ %s ]. ", PlayerName, sLog.ItemName);
			rsSendMessageAll(mensagem, 3);
		//	rsSendServerChatMessageToUser2(3, mensagem);

			vAlertaInfo.erase(it);
			break;
		}
	}
}
