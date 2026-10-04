#include "core/server/legacy_sin_runtime/sinLinkHeader.h"
#include "systems/progression/class_rank/Rank.h"
#include "core/server/legacy_runtime/onserver.h"
#include "systems/network/v2/legacy/LegacyPacketSender.h"
#include "Shared/gameplay/progression/v2/ClassRankReadModel.h"
#include "systems/progression/class_rank/v2/legacy/LegacyClassRankRepository.h"

#include <limits>
#include <vector>



namespace
{
bool SummitClassRankSend(rsPLAYINFO* recipient, const void* data, int size)
{
	if(!recipient || !data || size <= 0)
		return false;
	return gameplay::network::legacy::ServerLegacyPacketSender().Send(
		gameplay::PlayerId(static_cast<std::uint32_t>(recipient->dwObjectSerial)),
		gameplay::network::PacketView(data, static_cast<std::size_t>(size), 0u)).Succeeded();
}
}


void Rank::RecvRankclassToServer(rsPLAYINFO* Player, Rank_Job* infos)
{
	if (!infos)
		return;

	gameplay::progression::ClassRankQuery query;
	if (!gameplay::progression::ClassRankQueryService::TryFromLegacy(infos->type, infos->job_code, query))
		return;

	SendRankclassToGame(Player, query);
}

void Rank::SendRankclassToGame(rsPLAYINFO* Player)
{
	// UI-open callers do not carry a request-local metric/class selection.
	// Keep the open path compatible without reviving shared rank state.
	if (!Player)
		return;
}

void Rank::SendRankclassToGame(rsPLAYINFO* Player, const gameplay::progression::ClassRankQuery& query)
{
	if (Player == NULL)
		return;

	std::vector<gameplay::progression::ClassRankEntry> entries;
	if (!gameplay::progression::ClassRankReadService::LoadTop(
		gameplay::progression::legacy::ServerLegacyClassRankRepository(), query, entries))
		return;

	Rank_PACK packet;
	ZeroMemory(&packet, sizeof(Rank_PACK));
	packet.size = sizeof(Rank_PACK);
	packet.code = RANK_PACKET_RECV_RANKCLASS;

	for (std::size_t i = 0; i < entries.size() && i < 100u; ++i)
	{
		const gameplay::progression::ClassRankEntry& entry = entries[i];
		strcpy_s(packet.infos[i].Name, sizeof(packet.infos[i].Name), entry.name.c_str());
		packet.infos[i].Job_code = entry.classCode;

		if (query.metric == gameplay::progression::RankingMetric::Level)
		{
			if (entry.level > static_cast<std::int64_t>(std::numeric_limits<int>::max()))
				packet.infos[i].Level = std::numeric_limits<int>::max();
			else if (entry.level < static_cast<std::int64_t>(std::numeric_limits<int>::min()))
				packet.infos[i].Level = std::numeric_limits<int>::min();
			else
				packet.infos[i].Level = static_cast<int>(entry.level);
		}
		else
		{
			packet.infos[i].Kills = entry.kills;
		}
	}

	if (Player->lpsmSock)
		SummitClassRankSend(Player, &packet, packet.size);
}

