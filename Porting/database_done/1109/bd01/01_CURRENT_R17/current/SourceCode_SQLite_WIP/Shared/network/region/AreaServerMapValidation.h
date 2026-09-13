#pragma once

#include <cstddef>

namespace legacy_area_server
{
inline bool IsValidMapPacket(const TRANS_SEVER_MAP *packet)
{
	if(!packet)
		return false;

	const int headerSize = static_cast<int>(offsetof(TRANS_SEVER_MAP, dwIP));
	if(packet->size < headerSize || packet->size > static_cast<int>(sizeof(TRANS_SEVER_MAP)))
		return false;
	if(packet->IP_Count < 0 || packet->IP_Count > SERVER_MAP_MAX)
		return false;

	const int expectedSize = headerSize + static_cast<int>(sizeof(packet->dwIP[0])) * packet->IP_Count;
	return packet->size == expectedSize;
}
}
