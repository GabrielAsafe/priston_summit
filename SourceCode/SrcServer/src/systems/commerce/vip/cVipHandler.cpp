#include "compat/server_build/server_compat_packets.h"
#include "systems/network/protocol/server/opcodes.h"
#include "core/server/legacy_runtime/onserver.h"
#include "systems/commerce/vip/cVipHandler.h"
#include "systems/persistence/database/core/Database.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"
#include "systems/network/v2/legacy/LegacyPacketSender.h"
#include "systems/commerce/v2/legacy/LegacyVipEntitlementRepository.h"

extern int SendQuickMessage(rsPLAYINFO* lpPlayInfo, char* szMessage);

namespace
{
bool SummitVipSend(rsPLAYINFO *recipient, const void *data, int size)
{
	if(!recipient || !data || size <= 0)
		return false;

	const gameplay::PlayerId playerId =
		gameplay::player::legacy::ServerLegacyOnlinePlayerRegistry().IdOf(recipient);
	return gameplay::network::legacy::ServerLegacyPacketSender().Send(
		playerId, gameplay::network::PacketView(data, static_cast<std::size_t>(size), 0u)).Succeeded();
}
}

void cVipHandler::InsertDB(rsPLAYINFO* lpPlayInfo, int IdVip, int time)
{
	const gameplay::PlayerId playerId =
		gameplay::player::legacy::ServerLegacyOnlinePlayerRegistry().IdOf(lpPlayInfo);
	const gameplay::commerce::VipGrantCode grant =
		gameplay::commerce::legacy::ServerPremiumEntitlementService().GrantVip(
			gameplay::commerce::VipGrantRequest(playerId, IdVip, time));

	if(grant != gameplay::commerce::VipGrantCode::Granted)
		return;

	smTRANS_VIP sPacket = {};
	sPacket.code = OpCode::OPCODE_VIP;
	sPacket.size = sizeof(sPacket);
	sPacket.Ativo = TRUE;
	(void)SummitVipSend(lpPlayInfo, &sPacket, sPacket.size);
	SendQuickMessage(lpPlayInfo, "/comandos para ver suas recompensas do VIP.");
}

void cVipHandler::IsVIp(rsPLAYINFO* lpPlayInfo)
{
	const gameplay::PlayerId playerId =
		gameplay::player::legacy::ServerLegacyOnlinePlayerRegistry().IdOf(lpPlayInfo);
	gameplay::commerce::VipEntitlementSnapshot entitlement;
	if(!gameplay::commerce::legacy::ServerPremiumEntitlementService().TryGetVip(playerId, entitlement) ||
	   !entitlement.active)
		return;

	smTRANS_VIP sPacket = {};
	sPacket.code = OpCode::OPCODE_VIP;
	sPacket.size = sizeof(sPacket);
	sPacket.Ativo = TRUE;
	(void)SummitVipSend(lpPlayInfo, &sPacket, sPacket.size);
	SendQuickMessage(lpPlayInfo, "/comandos para ver suas recompensas do VIP.");
}

bool cVipHandler::returnVip(rsPLAYINFO* lpPlayInfo)
{
	const gameplay::PlayerId playerId =
		gameplay::player::legacy::ServerLegacyOnlinePlayerRegistry().IdOf(lpPlayInfo);
	gameplay::commerce::VipEntitlementSnapshot entitlement;
	return gameplay::commerce::legacy::ServerPremiumEntitlementService().TryGetVip(playerId, entitlement) &&
		entitlement.active;
}

