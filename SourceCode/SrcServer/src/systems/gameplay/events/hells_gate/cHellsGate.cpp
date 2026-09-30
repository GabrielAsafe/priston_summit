#include <cstring>
#include "systems/network/protocol/server/opcodes.h"
#include "core/server/legacy_sin_runtime/sinLinkHeader.h"
#include "systems/gameplay/events/hells_gate/cHellsGate.h"
#include "systems/gameplay/events/v2/GameplayEventRuntime.h"
#include "systems/gameplay/events/v2/legacy/LegacyWorldEventRewardGateway.h"
#include "systems/gameplay/events/v2/legacy/LegacyWorldEventAdmissionAdapter.h"
#include "systems/gameplay/player/v2/legacy/LegacyPlayerAdapter.h"
#include "gameplay/world/v2/ITeleportService.h"
#include "gameplay/world/v2/ISpawnService.h"
#include "gameplay/social/v2/ChatDelivery.h"
#include "network/v2/PacketSender.h"
#include "systems/community/chat/v2/legacy/LegacyChatDeliveryAdapter.h"
#include "systems/network/v2/legacy/LegacyPacketSender.h"
#include "systems/gameplay/world/v2/legacy/LegacyTeleportAdapter.h"
#include "systems/gameplay/world/v2/legacy/LegacySpawnAdapter.h"

extern int SendToAll(char* Buff, int Size);
extern int Event_HellsGate;

cHELLSGATE cHg;


cHELLSGATE::cHELLSGATE()
{
	//dwLastHgHour = 0;
	iState = 0;
	dwTickWaiting = 0;
	dwTick = 0;
	(void)gameplay::events::ServerGameplayEventHub().Subscribe<gameplay::events::MonsterDefeatedEvent>(
		[this](const gameplay::events::MonsterDefeatedEvent& event)
		{
			MonsterKilled(event.monster);
		});
}

void cHELLSGATE::Main()
{
	SYSTEMTIME st;
	GetLocalTime(&st);

	/*if (st.wHour % 6 == 0 && Event_HellsGate == 1 != st.wMinute == 0)
	{
		if (iState == 0)
			StartEvent();
	}*/

	if (iState == 1)
	{
		if (dwTickWaiting && dwTickWaiting + HG_WAIT_TIME < GetCurrentTime())
		{
			SpawnBoss();
			SendMessageToPlayers("> O Hell's Gate come\347ou, derrote o boss e obtenha a sua recompensa.");
			return;
		}

		UpdateWaiting();
	}
	else if (iState == 2)
	{
		if (dwTick && dwTick + HG_RUN_TIME < GetCurrentTime())
		{
			EndEvent();
			return;
		}

		UpdateRunning();
	}
}

void cHELLSGATE::SendOpenNpc(gameplay::PlayerId player)
{
	auto sendMessage = [player](const char* text)
	{
		gameplay::social::legacy::ServerLegacyChatDeliveryAdapter().SendToPlayer(
			player, gameplay::social::ChatMessage(
				text ? text : "", 0, gameplay::social::ChatDeliveryKind::Whisper));
	};

	if (vPlayers.size() >= MAX_HG_PLAYER)
	{
		sendMessage("> O evento est\341 cheio.");
		return;
	}

	if (iState != 1)
	{
		if (iState == 0)
			sendMessage("> O evento ainda n\343o abriu.");

		if (iState == 2)
			sendMessage("> O evento j\341 est\341 em andamento.");

		return;
	}

	smTRANS_COMMAND packet = {};
	packet.code = OpCode::OPCODE_OPEN_HG;
	packet.size = sizeof(smTRANS_COMMAND);
	gameplay::network::legacy::ServerLegacyPacketSender().Send(
		player, gameplay::network::PacketView(
			&packet, sizeof(packet), static_cast<std::uint32_t>(packet.code)));
}

void cHELLSGATE::RequestJoin(gameplay::PlayerId playerId)
{
	if (!gameplay::player::legacy::ServerLegacyPlayerAdapter().IsOnline(playerId))
		return;

	auto sendMessage = [playerId](const char* text)
	{
		gameplay::social::legacy::ServerLegacyChatDeliveryAdapter().SendToPlayer(
			playerId, gameplay::social::ChatMessage(
				text ? text : "", 0, gameplay::social::ChatDeliveryKind::Whisper));
	};

	const gameplay::events::WorldEventAdmissionStatus admission =
		gameplay::events::legacy::ServerLegacyWorldEventAdmissionAdapter().Check(
			gameplay::events::WorldEventAdmissionRequest(
				playerId, gameplay::MapId(23u), true));
	if (admission == gameplay::events::WorldEventAdmissionStatus::PlayerUnavailable)
		return;
	if (admission == gameplay::events::WorldEventAdmissionStatus::RemoteAddressAlreadyPresent)
	{
		sendMessage("HG> M\341ximo 1 personagem por pessoa!");
		return;
	}

	if (vPlayers.size() >= MAX_HG_PLAYER)
	{
		sendMessage("> O evento est\341 cheio.");
		return;
	}

	if (iState != 1)
	{
		if (iState == 0)
			sendMessage("> O evento ainda n\343o abriu.");

		if (iState == 2)
			sendMessage("> O evento j\341 est\341 em andamento.");

		return;
	}

	vPlayers.push_back(playerId);

	gameplay::world::legacy::ServerLegacyTeleportAdapter().Teleport(
		playerId, gameplay::WorldPosition(gameplay::MapId(23u), gameplay::WorldGridToFixed(-3624), 0, gameplay::WorldGridToFixed(-43172)));
}

void cHELLSGATE::MonsterKilled(gameplay::EntityId monster)
{
	if (iState != 2)
		return;

	if (bossId.Valid() && monster.Valid() && bossId == monster)
	{
		EndEvent(true);
	}
}

void cHELLSGATE::StartEvent()
{
	iState = 1;
	dwTick = 0;
	dwTickWaiting = GetCurrentTime();
	gameplay::events::PublishWorldEventStarted(
		gameplay::events::WorldEventStarted("hells_gate"));

	//SendMessageToPlayers("> O Hell's Gate está aberto.", true);
}

void cHELLSGATE::SpawnBoss()
{
	const gameplay::world::SpawnResult spawned = gameplay::world::legacy::ServerLegacySpawnAdapter().Spawn(
		gameplay::world::SpawnRequest(
			"Esp\355rito do Mal",
			gameplay::WorldPosition(
				gameplay::MapId(23u),
				gameplay::WorldGridToFixed(-3624),
				gameplay::WorldGridToFixed(484),
				gameplay::WorldGridToFixed(-43172))));
	bossId = spawned.Succeeded() ? spawned.entity : gameplay::EntityId();

	dwTick = GetCurrentTime();
	iState = 2;
}

void cHELLSGATE::EndEvent(bool bKilled)
{
	bossId = gameplay::EntityId();
	iState = 0;
	dwTick = 0;
	dwTickWaiting = 0;
	gameplay::events::PublishWorldEventEnded(
		gameplay::events::WorldEventEnded("hells_gate", bKilled));

	for (const gameplay::PlayerId playerId : vPlayers)
	{
		if (bKilled)
			SendReward(playerId);

		SendLeaveArena(playerId);
	}

	SendMessageToPlayers("> O Hell's Gate foi finalizado.");

	vPlayers.clear();
}

void cHELLSGATE::UpdateRunning()
{
	smTRANS_COMMAND sPacket;
	sPacket.code = OpCode::OPCODE_UPDATE_HG;
	sPacket.size = sizeof(smTRANS_COMMAND);
	sPacket.WParam = ((dwTick + HG_RUN_TIME) - GetCurrentTime()) / 1000;
	sPacket.LParam = iState;
	SendToPlayers((char*)&sPacket, sPacket.size);
}

void cHELLSGATE::UpdateWaiting()
{
	smTRANS_COMMAND sPacket;
	sPacket.code = OpCode::OPCODE_UPDATE_HG;
	sPacket.size = sizeof(smTRANS_COMMAND);
	sPacket.WParam = ((dwTickWaiting + HG_WAIT_TIME) - GetCurrentTime()) / 1000;
	sPacket.LParam = iState;
	SendToPlayers((char*)&sPacket, sPacket.size);
}

void cHELLSGATE::SendToPlayers(char * psPacket, int size)
{
	if (!psPacket || size <= 0)
		return;

	const smTRANS_COMMAND* header = reinterpret_cast<const smTRANS_COMMAND*>(psPacket);
	const gameplay::network::PacketView packet(
		psPacket, static_cast<std::size_t>(size), static_cast<std::uint32_t>(header->code));
	for (const gameplay::PlayerId playerId : vPlayers)
	{
		gameplay::network::legacy::ServerLegacyPacketSender().Send(playerId, packet);
	}
}

void cHELLSGATE::SendMessageToPlayers(char * szMessage, bool bToAll)
{
	TRANS_CHATMESSAGE TransChatMessage;
	lstrcpy(TransChatMessage.szMessage, szMessage);
	TransChatMessage.code = OpCode::OPCODE_WHISPERMESSAGE;
	TransChatMessage.size = lstrlen(TransChatMessage.szMessage) + 32;
	TransChatMessage.dwIP = 3;
	TransChatMessage.dwObjectSerial = 0;
	if (bToAll)
		SendToAll((char*)&TransChatMessage, TransChatMessage.size);
	else
		SendToPlayers((char*)&TransChatMessage, TransChatMessage.size);
}

void cHELLSGATE::SendReward(gameplay::PlayerId playerId)
{
	gameplay::player::PlayerSnapshot player;
	if (!gameplay::player::legacy::ServerLegacyPlayerAdapter().TryGet(playerId, player))
		return;

	gameplay::rewards::RewardDefinition reward;
	reward.type = gameplay::rewards::RewardType::Item;
	reward.key = "CX101";
	reward.amount = 1;
	(void)gameplay::events::legacy::ServerWorldEventRewardService().Grant(
		player.playerKey, reward, "world_event:hells_gate");
}

void cHELLSGATE::SendLeaveArena(gameplay::PlayerId playerId)
{
	if (!playerId.Valid())
		return;

	smTRANS_COMMAND sPacket = {};
	sPacket.code = OpCode::OPCODE_LEAVE_HG;
	sPacket.size = sizeof(smTRANS_COMMAND);
	gameplay::network::legacy::ServerLegacyPacketSender().Send(
		playerId, gameplay::network::PacketView(
			&sPacket, sizeof(sPacket), static_cast<std::uint32_t>(sPacket.code)));
}


