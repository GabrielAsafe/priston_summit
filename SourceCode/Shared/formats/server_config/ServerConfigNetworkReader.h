#pragma once

#include <cstddef>

#include "formats/server_config/ServerConfigCommon.h"

namespace summit
{
namespace formats
{
namespace server_config
{

namespace network_token
{
static const char DisableIp[] = "\x2A\xC1\xA2\xBC\xD3\xC1\xA6\xC7\xD1\x49\x50";
static const char EnableIp[] = "\x2A\xC1\xA2\xBC\xD3\xC7\xE3\xB0\xA1\x49\x50";
static const char AreaServer[] = "\x2A\xC1\xF6\xBF\xAA\xBC\xAD\xB9\xF6";
static const char TtDataServer[] = "\x2A\xC5\xE4\xB3\xCA\xB8\xD5\xC6\xAE\xB0\xE8\xC1\xA4\xBC\xAD\xB9\xF6";
}

template <typename Array, typename Decoder>
inline bool AppendIpRange(legacy_config::LineCursor &cursor, Array &entries, int &count, Decoder decoder)
{
	const std::size_t capacity = sizeof(entries) / sizeof(entries[0]);
	if(count < 0 || static_cast<std::size_t>(count) > capacity)
		return false;
	if(static_cast<std::size_t>(count) == capacity)
		return true;

	char first[128];
	char second[128];
	if(!ReadTokenOrEmpty(cursor, first, sizeof(first)) || !ReadTokenOrEmpty(cursor, second, sizeof(second)))
		return false;
	entries[count][0] = decoder(first);
	entries[count][1] = decoder(second);
	++count;
	return true;
}

template <typename Area, typename AddressDecoder, typename AreaCode>
inline bool AppendAreaServers(legacy_config::LineCursor &cursor, Area &area, AddressDecoder decoder, AreaCode areaCode)
{
	const std::size_t capacity = sizeof(area.dwIP) / sizeof(area.dwIP[0]);
	if(area.IP_Count < 0 || static_cast<std::size_t>(area.IP_Count) > capacity)
		return false;

	for(int item = 0; item < 32 && static_cast<std::size_t>(area.IP_Count) < capacity; ++item)
	{
		char address[128];
		if(!ReadValueOrEmpty(cursor, address, sizeof(address)))
			return false;
		if(!address[0])
			break;

		area.dwIP[area.IP_Count++] = decoder(address);
		area.size = static_cast<decltype(area.size)>(sizeof(area) -
			sizeof(area.dwIP[0]) * (capacity - static_cast<std::size_t>(area.IP_Count)));
		area.code = static_cast<decltype(area.code)>(areaCode);
	}
	return true;
}

template <typename Entries>
inline bool AppendTtDataServer(legacy_config::LineCursor &cursor, Entries &entries, int &count)
{
	const std::size_t capacity = sizeof(entries) / sizeof(entries[0]);
	if(count < 0 || static_cast<std::size_t>(count) > capacity)
		return false;
	if(static_cast<std::size_t>(count) == capacity)
		return true;

	char serverId[256];
	char serverPath[256];
	if(!ReadValueOrEmpty(cursor, serverId, sizeof(serverId)) || !serverId[0])
		return true;
	if(!ReadValueOrEmpty(cursor, serverPath, sizeof(serverPath)) || !serverPath[0])
		return true;
	if(!CopyText(entries[count].szServerID, serverId) || !CopyText(entries[count].szServerPath, serverPath))
		return false;
	++count;
	return true;
}

template <typename Config, typename IpRangeDecoder, typename AddressDecoder, typename AreaCode>
inline ParseResult ApplyNetworkDirective(const char *command,
	const char *values,
	Config &config,
	IpRangeDecoder ipRangeDecoder,
	AddressDecoder addressDecoder,
	AreaCode areaCode)
{
	if(!command)
		return ParseResult::NotHandled;

	Config staged = config;
	legacy_config::LineCursor cursor(values);

	if(Matches(command, network_token::DisableIp, "*DISABLE_IP"))
	{
		if(!AppendIpRange(cursor, staged.DisableIP, staged.DisableIP_Count, ipRangeDecoder)) return ParseResult::Error;
	}
	else if(Matches(command, network_token::EnableIp, "*ENABLE_IP"))
	{
		if(!AppendIpRange(cursor, staged.EnableIP, staged.EnableIP_Count, ipRangeDecoder)) return ParseResult::Error;
	}
	else if(Matches(command, network_token::AreaServer, "*AREA_SERVER"))
	{
		if(!AppendAreaServers(cursor, staged.AreaServer, addressDecoder, areaCode)) return ParseResult::Error;
	}
	else if(Matches(command, network_token::TtDataServer, "*TT_DATA_SERVER"))
	{
		if(!AppendTtDataServer(cursor, staged.TT_DataServer, staged.TT_DataServer_Count)) return ParseResult::Error;
	}
	else return ParseResult::NotHandled;

	config = staged;
	return ParseResult::Handled;
}

} // namespace server_config
} // namespace formats
} // namespace summit
