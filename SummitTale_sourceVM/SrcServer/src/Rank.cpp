#include "sinbaram\sinLinkHeader.h"
#include "server\OnServer.h"



void Rank::RecvRankclassToServer(rsPLAYINFO* Player, Rank_Job* infos)
{	
	ranktype = infos->type;
	rankjob_code = infos->job_code;

	Rank::GetInstance()->SendRankclassToGame(Player);
}

void Rank::SendRankclassToGame(rsPLAYINFO* Player)
{
	if (Player == NULL)
		return;
	std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_ServerDB);

	if (db->Open())
	{
		int i = 0;
		Rank_PACK packet;
		ZeroMemory(&packet, sizeof(Rank_PACK));
		packet.size = sizeof(Rank_PACK);
		packet.code = RANK_PACKET_RECV_RANKCLASS;
		if (ranktype == 1)
		{
			if (rankjob_code == 0)
			{
				db->Prepare("SELECT * FROM [PVP] where id not in (Select accountId COLLATE Latin1_General_CI_AI From accountdb..GameMasters where active =1) and class <> 0 Order By LVL Desc");
				if (db->Execute())
				{
					do
					{
						db->GetData(packet.infos[i].Name, sizeof(packet.infos[i].Name), 2, PARAMTYPE_String);
						db->GetData(&packet.infos[i].Job_code, sizeof(packet.infos[i].Job_code), 3, PARAMTYPE_Integer);
						db->GetData(&packet.infos[i].Level, sizeof(packet.infos[i].Level), 4, PARAMTYPE_Int64);
						i++;

						if (i > 50) break;
					} while (db->NextRow());

					if (Player->lpsmSock)
						Player->lpsmSock->Send((char*)&packet, packet.size, TRUE);


				}
				db->Close();
			}
			else
			{
				db->Prepare("SELECT * FROM [PVP] WHERE [CLASS]=? and ID not in (Select accountId COLLATE Latin1_General_CI_AI From accountdb.dbo.GameMasters where active =1) Order By LVL Desc");
				if (!db->BindInputParameter(&rankjob_code, 1, PARAMTYPE_Integer)) return;
				if (db->Execute())
				{
					do
					{

						db->GetData(packet.infos[i].Name, sizeof(packet.infos[i].Name), 2, PARAMTYPE_String);
						db->GetData(&packet.infos[i].Job_code, sizeof(packet.infos[i].Job_code), 3, PARAMTYPE_Integer);
						db->GetData(&packet.infos[i].Level, sizeof(packet.infos[i].Level), 4, PARAMTYPE_Int64);


						i++;

						if (i > 50) break;
					} while (db->NextRow());

					if (Player->lpsmSock)
						Player->lpsmSock->Send((char*)&packet, packet.size, TRUE);
				}
				db->Close();
			}
		}
		else if (ranktype == 2)
		{
			if (rankjob_code == 0)
			{


				db->Prepare("SELECT * FROM [PVP] where id not in (Select accountId COLLATE Latin1_General_CI_AI From accountdb.dbo.GameMasters where active =1) AND Kills > 0 Order By Kills Desc");
				if (db->Execute())
				{
					do
					{

						db->GetData(packet.infos[i].Name, 32, 2, PARAMTYPE_String);
						db->GetData(&packet.infos[i].Job_code, sizeof(int), 3, PARAMTYPE_Integer);
						db->GetData(&packet.infos[i].Kills, sizeof(int), 5, PARAMTYPE_Integer);


						i++;

						if (i > 50) break;
					} while (db->NextRow());

					if (Player->lpsmSock)
						Player->lpsmSock->Send((char*)&packet, packet.size, TRUE);
				}
				db->Close();
			}
			else
			{
				db->Prepare("SELECT * FROM [PVP] WHERE [Class]=? and id not in (Select accountId COLLATE Latin1_General_CI_AI From accountdb.dbo.GameMasters where active =1) AND Kills > 0  Order By Kills Desc");
				db->BindInputParameter(&rankjob_code, 1, PARAMTYPE_Integer);
				if (db->Execute())
				{
					do
					{
						db->GetData(packet.infos[i].Name, 32, 2, PARAMTYPE_String);
						db->GetData(&packet.infos[i].Job_code, sizeof(int), 3, PARAMTYPE_Integer);
						db->GetData(&packet.infos[i].Kills, sizeof(int), 5, PARAMTYPE_Integer);


						i++;

						if (i > 50) break;
					} while (db->NextRow());

					if (Player->lpsmSock)
						Player->lpsmSock->Send((char*)&packet, packet.size, TRUE);
				}
				db->Close();
			}
		}

	}

}
