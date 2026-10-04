#include "opcodes.h"
#include <windows.h>
#include <process.h>

#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/network/transport/legacy/smwsock.h"
#include "systems/network/v2/legacy/LegacyBackendChannelSender.h"
#include "smPacket.h"
#include "systems/security/legacy/process_integrity/cracker.h"
#include "systems/network/client/netplay.h"

static bool LegacyCrackerDataChannelAvailable()
{
	return gameplay::network::legacy::ClientLegacyBackendChannelSender().Available(
		gameplay::network::BackendChannel::Data);
}

static bool LegacyCrackerSendDataPacket(const void* data, int size, unsigned int opcode)
{
	return gameplay::network::legacy::ClientLegacyBackendChannelSender().Send(
		gameplay::network::BackendChannel::Data,
		gameplay::network::PacketView(data, static_cast<std::size_t>(size), opcode)).Succeeded();
}

#include "security/integrity/CrackerLegacyCommon.inl"
