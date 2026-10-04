#include <cstring>
#include "systems/network/protocol/server/opcodes.h"
#include "core/server/legacy_sin_runtime/sinLinkHeader.h"
#include "modules/File/File.h"
#include "core/server/legacy_runtime/onserver.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"
#include "systems/network/v2/legacy/LegacyPacketSender.h"



struct pssITEM
{
	int		state;
	int		Serial;
	sITEMINFO	ItemInfo;
};

string cnome;
int	rsRegist_ItemSecCode(rsPLAYINFO *lpPlayInfo, TRANS_ITEMINFO *lpTransItemInfo, int NewItem);
int CreateItem(pssITEM *lpsItem, char  *dwCode, int JobCode);
int	rsAddInvenItem(rsPLAYINFO *lpPlayInfo, DWORD dwCode, DWORD dwHead, DWORD dwChkSum);
DWORD GetServerTime();

static bool SummitClanCustomSend(rsPLAYINFO *recipient, const void *data, int size)
{
	if(!recipient || !data || size <= 0)
		return false;
	const gameplay::PlayerId playerId =
		gameplay::player::legacy::ServerLegacyOnlinePlayerRegistry().IdOf(recipient);
	return gameplay::network::legacy::ServerLegacyPacketSender().Send(
		playerId, gameplay::network::PacketView(data, static_cast<std::size_t>(size), 0u)).Succeeded();
}

static bool IsSafeClanAssetCode(const char *value)
{
	if(!value)
		return false;

	for(int i = 0; i < 32; ++i)
	{
		const unsigned char c = (unsigned char)value[i];
		if(c == 0)
			return i > 0;
		if(!((c >= '0' && c <= '9') ||
			 (c >= 'A' && c <= 'Z') ||
			 (c >= 'a' && c <= 'z') ||
			 c == '_' || c == '-'))
			return false;
	}

	return false;
}


void ClanCustom::SendCrownToClient(rsPLAYINFO * Player)
{
	if (SQL::GetInstance()->IsConnected())
	{
		char szQuery[512];
		CROWN packet;
		SQL::GetInstance()->EnterSqlSection();
		wsprintf(szQuery, "SELECT * FROM [PristonDB].[dbo].[Coroa]");
		if (SQL::GetInstance()->Execute(szQuery))
		{
			if (SQL::GetInstance()->Fetch()) {
				SQL::GetInstance()->GetDataValue(1, &packet.idnumber);
				packet.size = sizeof(CROWN);
				packet.code = CCPACKET_SEND_CROWN_GAME;
				(void)SummitClanCustomSend(Player, &packet, packet.size);
			}
			else
			{
				packet.size = sizeof(CROWN);
				packet.code = CCPACKET_SEND_CROWN_GAME;
				packet.idnumber = 1;
				(void)SummitClanCustomSend(Player, &packet, packet.size);
			}
		}
		SQL::GetInstance()->LeaveSqlSection();
	}
}


void ClanCustom::SendItemToClient(rsPLAYINFO * Player)
{
	if(!Player || !Player->lpsmSock)
		return;

	if(!Player || !Player->lpsmSock)
		return;

	if (SQL::GetInstance()->IsConnected())
	{
		int i = 0;
		char szQuery[512];
		CCSHOP packet;
		ZeroMemory(&packet, sizeof(CCSHOP));
		packet.size = sizeof(CCSHOP);
		packet.code = CCPACKET_SEND_ITEM_GAME;
		SQL::GetInstance()->EnterSqlSection();
		wsprintf(szQuery, "SELECT * FROM [PristonDB].[dbo].[Coroas]");
		if (SQL::GetInstance()->Execute(szQuery))
		{
			while (i < (int)(sizeof(packet.item) / sizeof(packet.item[0])) && SQL::GetInstance()->Fetch())
			{
				SQL::GetInstance()->GetDataValue(1, &packet.item[i].SubTag);
				SQL::GetInstance()->GetDataValue(2, packet.item[i].Name);
				SQL::GetInstance()->GetDataValue(3, packet.item[i].itCode);

				i++;
			}

			(void)SummitClanCustomSend(Player, &packet, packet.size);
		}

		int i2 = 0;
		wsprintf(szQuery, "SELECT * FROM [PristonDB].[dbo].[Tags]");
		if (SQL::GetInstance()->Execute(szQuery))
		{
			while (i2 < (int)(sizeof(packet.item2) / sizeof(packet.item2[0])) && SQL::GetInstance()->Fetch())
			{
				SQL::GetInstance()->GetDataValue(1, &packet.item2[i2].SubTag);
				SQL::GetInstance()->GetDataValue(2, packet.item2[i2].Name);
				SQL::GetInstance()->GetDataValue(3, packet.item2[i2].itCode);
				SQL::GetInstance()->GetDataValue(4, &packet.item2[i2].spec);

				i2++;
			}

			(void)SummitClanCustomSend(Player, &packet, packet.size);
		}

		SQL::GetInstance()->LeaveSqlSection();
	}

}


void ClanCustom::RecvItemToClient(rsPLAYINFO * Player, CCSHOP_BUY * Shop)
{
	if(!Player || !Shop)
		return;

	if(!Player || !Shop)
		return;

	if (Shop->item.SubTag == 1)
	{
		if (SQL::GetInstance()->IsConnected())
		{
			int Price = 0;
			char Cname[32] = { 0 };
			char Cid[32] = { 0 };
			char szQuery[512];
			SQL::GetInstance()->EnterSqlSection();
			wsprintf(szQuery, "SELECT * FROM [ClanDB].[dbo].[CL] WHERE [MiconCnt]='%d'", (rsBlessCastle.dwMasterClan + 1000000));
			if (SQL::GetInstance()->Execute(szQuery))
			{
				if (SQL::GetInstance()->Fetch())
					SQL::GetInstance()->GetDataValue(6, Cname);
				if(!IsSafeClanAssetCode(Shop->item.itCode))
				{
					SQL::GetInstance()->LeaveSqlSection();
					return;
				}
				wsprintf(szQuery, "SELECT * FROM [PristonDB].[dbo].[Coroas] WHERE [CodeIT]='%s'", Shop->item.itCode);
				if (SQL::GetInstance()->Execute(szQuery))
				{
					if (SQL::GetInstance()->Fetch())
						SQL::GetInstance()->GetDataValue(4, &Price);
				}
				wsprintf(szQuery, "SELECT * FROM [ClanDB].[dbo].[CL] WHERE [MiconCnt]='%d'", (rsBlessCastle.dwMasterClan + 1000000));
				if (SQL::GetInstance()->Execute(szQuery))
				{
					if (SQL::GetInstance()->Fetch())
						SQL::GetInstance()->GetDataValue(5, Cid);
				}


				if ((rsBlessCastle.dwMasterClan == (Player->smCharInfo.ClassClan -1000000) || rsBlessCastle.dwMasterClan == (Player->smCharInfo.ClassClan)) && lstrcmpi(Player->smCharInfo.szName, Cname) == 0 && lstrcmpi(Player->getAccount(), Cid) == 0)
				{
					if (Price == 101)
					{
						wsprintf(szQuery, "UPDATE  [PristonDB].[dbo].[Coroa] SET ID=1");
						if (SQL::GetInstance()->Execute(szQuery))
						{
							TRANS_CHATMESSAGE	TransChatMessage;
							wsprintf(TransChatMessage.szMessage, "Voc\352 alterou a Coroa do Clan: Coroa Padr\343o.");
							TransChatMessage.code = OpCode::OPCODE_WHISPERMESSAGE;
							TransChatMessage.size = 32 + lstrlen(TransChatMessage.szMessage);
							TransChatMessage.dwIP = 3;
							TransChatMessage.dwObjectSerial = 0;
							(void)SummitClanCustomSend(Player, &TransChatMessage, TransChatMessage.size);
						}
					}
					else if (Price == 102)
					{
						wsprintf(szQuery, "UPDATE  [PristonDB].[dbo].[Coroa] SET ID=2");
						if (SQL::GetInstance()->Execute(szQuery))
						{
							TRANS_CHATMESSAGE	TransChatMessage;
							wsprintf(TransChatMessage.szMessage, "Voc\352 alterou a Coroa do Clan: Asas Angelicais.");
							TransChatMessage.code = OpCode::OPCODE_WHISPERMESSAGE;
							TransChatMessage.size = 32 + lstrlen(TransChatMessage.szMessage);
							TransChatMessage.dwIP = 3;
							TransChatMessage.dwObjectSerial = 0;
							(void)SummitClanCustomSend(Player, &TransChatMessage, TransChatMessage.size);
						}

					}
					else if (Price == 103)
					{
						wsprintf(szQuery, "UPDATE  [PristonDB].[dbo].[Coroa] SET ID=3");
						if (SQL::GetInstance()->Execute(szQuery))
						{
							TRANS_CHATMESSAGE	TransChatMessage;
							wsprintf(TransChatMessage.szMessage, "Voc\352 alterou a Coroa do Clan: Asas Demoniacas.");
							TransChatMessage.code = OpCode::OPCODE_WHISPERMESSAGE;
							TransChatMessage.size = 32 + lstrlen(TransChatMessage.szMessage);
							TransChatMessage.dwIP = 3;
							TransChatMessage.dwObjectSerial = 0;
							(void)SummitClanCustomSend(Player, &TransChatMessage, TransChatMessage.size);
						}

					}
					else if (Price == 104)
					{
						wsprintf(szQuery, "UPDATE  [PristonDB].[dbo].[Coroa] SET ID=4");
						if (SQL::GetInstance()->Execute(szQuery))
						{
							TRANS_CHATMESSAGE	TransChatMessage;
							wsprintf(TransChatMessage.szMessage, "Voc\352 alterou a Coroa do Clan: Asas Ice.");
							TransChatMessage.code = OpCode::OPCODE_WHISPERMESSAGE;
							TransChatMessage.size = 32 + lstrlen(TransChatMessage.szMessage);
							TransChatMessage.dwIP = 3;
							TransChatMessage.dwObjectSerial = 0;
							(void)SummitClanCustomSend(Player, &TransChatMessage, TransChatMessage.size);
						}

					}

					ClanCustom::GetInstance()->SendCrownToClient(Player);
				}
				else
				{
					TRANS_CHATMESSAGE	TransChatMessage;
					wsprintf(TransChatMessage.szMessage, "Somente o lider do Clan pode alterar a Coroa!");
					TransChatMessage.code = OpCode::OPCODE_WHISPERMESSAGE;
					TransChatMessage.size = 32 + lstrlen(TransChatMessage.szMessage);
					TransChatMessage.dwIP = 3;
					TransChatMessage.dwObjectSerial = 0;
							(void)SummitClanCustomSend(Player, &TransChatMessage, TransChatMessage.size);
				}

			}
			else
			{
				TRANS_CHATMESSAGE	TransChatMessage;
				wsprintf(TransChatMessage.szMessage, "Somente o lider do Clan Vencedor de Bless pode alterar a Coroa!");
				TransChatMessage.code = OpCode::OPCODE_WHISPERMESSAGE;
				TransChatMessage.size = 32 + lstrlen(TransChatMessage.szMessage);
				TransChatMessage.dwIP = 3;
				TransChatMessage.dwObjectSerial = 0;
							(void)SummitClanCustomSend(Player, &TransChatMessage, TransChatMessage.size);
			}
			SQL::GetInstance()->LeaveSqlSection();
		}
	}
	else if (Shop->item.SubTag == 2)
	{
		if (SQL::GetInstance()->IsConnected())
		{
			char Cname[32] = { 0 };
			char Clanname[32] = { 0 };
			char Cid[32] = { 0 };
			char Clanaux[32] = { 0 };
			char szQuery[512];
			char codeant[512] = { 0 };
			char codeatt[512] = { 0 };
			char sourcePath[512] = { 0 };
			char destinationPath[512] = { 0 };
			SQL::GetInstance()->EnterSqlSection();
			wsprintf(szQuery, "SELECT * FROM [ClanDB].[dbo].[CL] WHERE [MiconCnt]='%d'", Player->smCharInfo.ClassClan);
			if (SQL::GetInstance()->Execute(szQuery))
			{
				if (SQL::GetInstance()->Fetch())
					SQL::GetInstance()->GetDataValue(2, Clanname);
				SQL::GetInstance()->GetDataValue(5, Cid);
				SQL::GetInstance()->GetDataValue(6, Cname);

				wsprintfA(codeant, "%d", Player->smCharInfo.ClassClan);

				if (lstrcmpi(Player->smCharInfo.szName, Cname) == 0 && lstrcmpi(Player->getAccount(), Cid) == 0)
				{

				//	if (rsBlessCastle.dwMasterClan == (Player->smCharInfo.ClassClan - 1000000) || rsBlessCastle.dwMasterClan == (Player->smCharInfo.ClassClan))
					//{
					//	wsprintfA(Clanaux, "%s", (Shop->item.itCode -1000000));
					//	rsBlessCastle.dwMasterClan = atoi(Clanaux);
					//}

					wsprintf(szQuery, "SELECT * FROM [PristonDB].[dbo].[Tags] WHERE [Clan]='%s'", Clanname);
					if (SQL::GetInstance()->Execute(szQuery))
					{
						if (SQL::GetInstance()->Fetch())
							SQL::GetInstance()->GetDataValue(3, codeatt);
					}



					if(!IsSafeClanAssetCode(Shop->item.itCode))
					{
						SQL::GetInstance()->LeaveSqlSection();
						return;
					}

					wsprintfA(Clanaux, "%s", Shop->item.itCode);

					wsprintf(szQuery, "UPDATE [PristonDB].[dbo].[Tags] SET Slot='%d' WHERE [CodeIT]='%s'", 1, Clanaux);
					if (SQL::GetInstance()->Execute(szQuery))
					{
						wsprintf(szQuery, "UPDATE [PristonDB].[dbo].[Tags] SET Clan='%s' WHERE [CodeIT]='%s'", Clanname, Clanaux);
						if (SQL::GetInstance()->Execute(szQuery))	
						{
							//wsprintf(szQuery, "UPDATE [ClanDB].[dbo].[CL] SET MiconCnt='%s' WHERE [ClanName]='%s'", Clanaux, Clanname);
							//if (SQL::GetInstance()->Execute(szQuery))
							//{						
							//}
						}
					}
					wsprintfA(sourcePath, "C:\\inetpub\\wwwroot\\brnxcontenttags\\%s.bmp", Clanaux);
					wsprintfA(destinationPath, "C:\\inetpub\\wwwroot\\ClanContent\\%s.bmp", codeant);
					CopyFileA(sourcePath, destinationPath, FALSE);

					wsprintf(szQuery, "UPDATE [PristonDB].[dbo].[Tags] SET Slot='%d' WHERE [CodeIT]='%s'", 0, codeatt);
					if (SQL::GetInstance()->Execute(szQuery))
					{
						wsprintf(szQuery, "UPDATE [PristonDB].[dbo].[Tags] SET Clan='%s' WHERE [CodeIT]='%s'","NA", codeatt);
						if (SQL::GetInstance()->Execute(szQuery))
						{
						}
					}


					TRANS_CHATMESSAGE	TransChatMessage;
					wsprintf(TransChatMessage.szMessage, "Tag alterada com sucesso! Relogue por favor.");
					TransChatMessage.code = OpCode::OPCODE_WHISPERMESSAGE;
					TransChatMessage.size = 32 + lstrlen(TransChatMessage.szMessage);
					TransChatMessage.dwIP = 3;
					TransChatMessage.dwObjectSerial = 0;
						(void)SummitClanCustomSend(Player, &TransChatMessage, TransChatMessage.size);

				
					ClanCustom::GetInstance()->SendItemToClient(Player);
				}
				else
				{

					TRANS_CHATMESSAGE	TransChatMessage;
					wsprintf(TransChatMessage.szMessage, "Somente o lider do Clan pode alterar a Tag!");
					TransChatMessage.code = OpCode::OPCODE_WHISPERMESSAGE;
					TransChatMessage.size = 32 + lstrlen(TransChatMessage.szMessage);
					TransChatMessage.dwIP = 3;
					TransChatMessage.dwObjectSerial = 0;
						(void)SummitClanCustomSend(Player, &TransChatMessage, TransChatMessage.size);
				}
			}			
			SQL::GetInstance()->LeaveSqlSection();
		}
	}
}


