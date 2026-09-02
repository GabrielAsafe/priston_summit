#include <cstring>
#include "systems/network/protocol/server/opcodes.h"
#include "systems/community/news/News.h"
#include "modules/File/File.h"
#include "smPacket.h"
#include <string>
#include "systems/persistence/database/core/Database.h"
#include "systems/commerce/coin_shop/CoinShop.h"
#include "systems/commerce/coin_shop/CoinShop_T.h"

extern int OpenBossTimeMin;

void News::welcome()
{
	TRANS_CHATMESSAGE TransChatMessage;
	//nick na msg de boas vindas
	/*wsprintf(TransChatMessage.szMessage, "> Ol� %s", player->getName());
	TransChatMessage.code = OpCode::OPCODE_WHISPERMESSAGE;
	TransChatMessage.size = 32 + lstrlen(TransChatMessage.szMessage);
	TransChatMessage.dwIP = 16;
	TransChatMessage.dwObjectSerial = 0;
	player->lpsmSock->Send((char*)& TransChatMessage, TransChatMessage.size, TRUE);*/


	//nick na msg de boas vindas
	wsprintf(TransChatMessage.szMessage, "Ola %s, Bem Vindo ao Priston Tale.", player->getName());
	TransChatMessage.code = OpCode::OPCODE_WHISPERMESSAGE;
	TransChatMessage.size = 32 + lstrlen(TransChatMessage.szMessage);
	TransChatMessage.dwIP = 3;
	TransChatMessage.dwObjectSerial = 0;
	player->lpsmSock->Send((char*)& TransChatMessage, TransChatMessage.size, TRUE);

	/*try
	{
		File file("Configs//Sistemas.ini");

		std::string message = file.readString("Welcome", "message");

		if (player && player->lpsmSock)
		{
			TRANS_CHATMESSAGE packet;

			packet.code = OpCode::OPCODE_WHISPERMESSAGE;

			strcpy_s(packet.szMessage, sizeof(packet.szMessage), message.c_str());

			packet.size = 32 + lstrlen(packet.szMessage);
			packet.dwIP = 16;
			packet.dwObjectSerial = 0;

			player->lpsmSock->Send((char *)&packet, packet.size, TRUE);
		}
	}
	catch (FileException)
	{
	}*/
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
	try
	{
		TRANS_CHATMESSAGE TransChatMessage;

		wsprintf(TransChatMessage.szMessage, "> Voc� Possui %d CoinShop", cCoinShop.GetPlayerCoin(player));
		TransChatMessage.code = OpCode::OPCODE_WHISPERMESSAGE;
		TransChatMessage.size = 32 + lstrlen(TransChatMessage.szMessage);
		TransChatMessage.dwIP = 3;
		TransChatMessage.dwObjectSerial = 0;
		player->lpsmSock->Send((char*)& TransChatMessage, TransChatMessage.size, TRUE);
		
	
		wsprintf(TransChatMessage.szMessage, "> Voc� Possui %d TimeShop", cCoinShop_T.GetPlayerCoin(player));
		TransChatMessage.code = OpCode::OPCODE_WHISPERMESSAGE;
		TransChatMessage.size = 32 + lstrlen(TransChatMessage.szMessage);
		TransChatMessage.dwIP = 3;
		TransChatMessage.dwObjectSerial = 0;
		player->lpsmSock->Send((char*)& TransChatMessage, TransChatMessage.size, TRUE);
		
	}
	catch (FileException)
	{

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
	TRANS_CHATMESSAGE TransChatMessage;
	int cnt;

	if (player && player->lpsmSock)
	{

		wsprintf(TransChatMessage.szMessage, "> Player [%s] Est� Online", player->getName());
		TransChatMessage.code = OpCode::OPCODE_WHISPERMESSAGE;
		TransChatMessage.size = 32 + lstrlen(TransChatMessage.szMessage);
		TransChatMessage.dwIP = 9;
		TransChatMessage.dwObjectSerial = 0;


		for (cnt = 0; cnt < CONNECTMAX; cnt++)
		{
			if (jogadores[cnt].lpsmSock)
			{
				jogadores[cnt].lpsmSock->Send((char *)&TransChatMessage, TransChatMessage.size, TRUE);
			}
		}
	}
}


