#pragma once

#include "formats/server_config/ServerConfigCommon.h"

namespace summit
{
namespace formats
{
namespace server_config
{

namespace client_local_token
{
static const char AdministratorIp[] = "*관리접속IP";
static const char SystemIp[] = "*시스템IP";
static const char LoginServerIp[] = "*로그인서버IP";
static const char ServerLinkIp[] = "*서버연결IP";
}

template <typename Config, typename IpDecoder>
inline ParseResult ApplyClientLocalDirective(
	const char *command,
	const char *values,
	Config &config,
	IpDecoder decodeIp)
{
	if(!command)
		return ParseResult::NotHandled;

	legacy_config::LineCursor cursor(values);
	char first[128] = { 0 };
	char second[128] = { 0 };

	if(Matches(command, client_local_token::AdministratorIp, "*ADMINISTRATOR_IP"))
	{
		if(config.AdminIP_Count < ADMIN_IP_MAX)
		{
			if(!ReadTokenOrEmpty(cursor, first, sizeof(first)) || !ReadTokenOrEmpty(cursor, second, sizeof(second)))
				return ParseResult::Error;
			config.AdminIP[config.AdminIP_Count][0] = decodeIp(first);
			config.AdminIP[config.AdminIP_Count][1] = decodeIp(second);
			++config.AdminIP_Count;
		}
		return ParseResult::Handled;
	}

	if(Matches(command, client_local_token::SystemIp, "*SYSTEM_IP"))
	{
		if(config.SystemIP_Count < SYSTEM_IP_MAX)
		{
			if(!ReadTokenOrEmpty(cursor, first, sizeof(first)) || !ReadTokenOrEmpty(cursor, second, sizeof(second)))
				return ParseResult::Error;
			config.SystemIP[config.SystemIP_Count][0] = decodeIp(first);
			config.SystemIP[config.SystemIP_Count][1] = decodeIp(second);
			++config.SystemIP_Count;
		}
		return ParseResult::Handled;
	}

	if(Matches(command, client_local_token::LoginServerIp, "*LOGIN_SERVER_IP"))
	{
		if(config.LoginServerIP_Count < LOGIN_IP_MAX)
		{
			if(!ReadTokenOrEmpty(cursor, first, sizeof(first)) || !ReadTokenOrEmpty(cursor, second, sizeof(second)))
				return ParseResult::Error;
			config.LoginServerIP[config.LoginServerIP_Count][0] = decodeIp(first);
			config.LoginServerIP[config.LoginServerIP_Count][1] = decodeIp(second);
			++config.LoginServerIP_Count;
		}
		return ParseResult::Handled;
	}

	if(Matches(command, client_local_token::ServerLinkIp, "*SERVER_LINK_IP"))
	{
		if(!ReadValueOrEmpty(cursor, first, sizeof(first)) || !CopyText(config.szLinkServerIP, first))
			return ParseResult::Error;
		return ParseResult::Handled;
	}

	return ParseResult::NotHandled;
}

} // namespace server_config
} // namespace formats
} // namespace summit
