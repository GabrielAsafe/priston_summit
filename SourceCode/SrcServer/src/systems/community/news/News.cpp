#include <cstring>
#include <cstdio>
#include "systems/network/protocol/server/opcodes.h"
#include "systems/community/news/News.h"
#include "modules/File/File.h"
#include "smPacket.h"
#include <string>
#include "content/v1/localization/content/LocalizationRuntime.h"
#include "content/v1/localization/legacy/LocalizationLegacyEncoding.h"
#include "systems/persistence/database/core/Database.h"
#include "systems/commerce/coin_shop/CoinShop.h"
#include "systems/commerce/coin_shop/CoinShop_T.h"

extern int OpenBossTimeMin;

namespace
{
bool SummitNewsLegacyFormat(std::string &legacyFormat, const char *key)
{
    return summit::content::localization::GetLocalizationLegacyValue(key, legacyFormat);
}

bool SummitNewsFormatName(char *destination, std::size_t capacity, const char *key, const char *name)
{
    if(!destination || capacity == 0)
        return false;
    std::string format;
    if(!SummitNewsLegacyFormat(format, key))
        return false;
    const int written = std::snprintf(destination, capacity, format.c_str(), name ? name : "");
    if(written < 0 || static_cast<std::size_t>(written) >= capacity)
    {
        destination[0] = 0;
        return false;
    }
    return true;
}

bool SummitNewsFormatNumber(char *destination, std::size_t capacity, const char *key, int value)
{
    if(!destination || capacity == 0)
        return false;
    std::string format;
    if(!SummitNewsLegacyFormat(format, key))
        return false;
    const int written = std::snprintf(destination, capacity, format.c_str(), value);
    if(written < 0 || static_cast<std::size_t>(written) >= capacity)
    {
        destination[0] = 0;
        return false;
    }
    return true;
}
}


void News::welcome()
{
    if(!player || !player->lpsmSock)
        return;

    TRANS_CHATMESSAGE packet = {};
    if(!SummitNewsFormatName(
        packet.szMessage,
        sizeof(packet.szMessage),
        "news.welcome",
        player->getName()))
        return;

    packet.code = OpCode::OPCODE_WHISPERMESSAGE;
    packet.size = 32 + lstrlen(packet.szMessage);
    packet.dwIP = 3;
    packet.dwObjectSerial = 0;
    player->lpsmSock->Send((char*)&packet, packet.size, TRUE);
}

void News::title()
{
	try
	{
		File file("Configs//Sistemas.ini");

		std::string message = file.readString("Title", "message");
		
		if (player && player->lpsmSock)
		{
			TRANS_CHATMESSAGE packet;

			packet.code = OpCode::OPCODE_WHISPERMESSAGE;

			strcpy_s(packet.szMessage, sizeof(packet.szMessage), message.c_str());

			packet.size = 32 + lstrlen(packet.szMessage);
			packet.dwIP = 3;
			packet.dwObjectSerial = 0;

			player->lpsmSock->Send((char *)&packet, packet.size, TRUE);
		}
	}
	catch (FileException)
	{
	}
}

void News::msgCoinShop()
{
    if(!player || !player->lpsmSock)
        return;

    TRANS_CHATMESSAGE packet = {};
    if(SummitNewsFormatNumber(
        packet.szMessage,
        sizeof(packet.szMessage),
        "news.coin_shop_balance",
        cCoinShop.GetPlayerCoin(player)))
    {
        packet.code = OpCode::OPCODE_WHISPERMESSAGE;
        packet.size = 32 + lstrlen(packet.szMessage);
        packet.dwIP = 3;
        packet.dwObjectSerial = 0;
        player->lpsmSock->Send((char*)&packet, packet.size, TRUE);
    }

    ZeroMemory(&packet, sizeof(packet));
    if(SummitNewsFormatNumber(
        packet.szMessage,
        sizeof(packet.szMessage),
        "news.time_shop_balance",
        cCoinShop_T.GetPlayerCoin(player)))
    {
        packet.code = OpCode::OPCODE_WHISPERMESSAGE;
        packet.size = 32 + lstrlen(packet.szMessage);
        packet.dwIP = 3;
        packet.dwObjectSerial = 0;
        player->lpsmSock->Send((char*)&packet, packet.size, TRUE);
    }
}

void News::msgNotice()
{
	try
	{
	   char szMessageLogin[256] = { 0 };

	   std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_ServerDB);

	   if (db->Open())
	   {
		   if (db->Prepare("SELECT * FROM NoticeSystem WHERE Activated=1"))
		   {
			   if (db->Execute())
			   {
				  while (true)
				  {
					  int iMessageType = 0;

					  db->GetData(szMessageLogin, 256, 2, PARAMTYPE_String);
					  db->GetData(&iMessageType, 0, 3, PARAMTYPE_Integer);

					  TRANS_CHATMESSAGE	Login;

					  if (iMessageType == 1)
						  wsprintf(Login.szMessage, szMessageLogin, OpenBossTimeMin);
				      else
						  wsprintf(Login.szMessage, "%s", szMessageLogin);

					  Login.code = OpCode::OPCODE_WHISPERMESSAGE;
					  Login.size = 32 + lstrlen(Login.szMessage);
					  Login.dwIP = 13;
					  Login.dwObjectSerial = 0;

					  player->lpsmSock->Send((char*)& Login, Login.size, TRUE);

					  if (!db->NextRow())
						  break;
				  }
			   }
		   }
		db->Close();
	   }

	}
	catch (FileException)
	{

	}
}

void News::msgPlayerLogado(rsPLAYINFO *jogadores)
{
    if(!player || !player->lpsmSock || !jogadores || !jogadores->lpsmSock)
        return;

    TRANS_CHATMESSAGE packet = {};
    if(!SummitNewsFormatName(
        packet.szMessage,
        sizeof(packet.szMessage),
        "news.player_online",
        player->getName()))
        return;

    packet.code = OpCode::OPCODE_WHISPERMESSAGE;
    packet.size = 32 + lstrlen(packet.szMessage);
    packet.dwIP = 9;
    packet.dwObjectSerial = 0;
    jogadores->lpsmSock->Send((char*)&packet, packet.size, TRUE);
}


