#pragma once

#include <cstddef>

#include "formats/server_config/ServerConfigCommon.h"

namespace summit
{
namespace formats
{
namespace server_config
{

struct CorePolicy
{
	int defaultPkField;
	bool allowExtendedPkFields;
	bool allowAdminCredentials;
};

namespace token
{
static const char ServerCode[] = "\x2A\xBC\xAD\xB9\xF6\xC4\xDA\xB5\xE5\xB9\xF8\xC8\xA3";
static const char ServerAutoplay[] = "\x2A\xC0\xDA\xB5\xBF\xC4\xB3\xB8\xAF\xC5\xCD\xBB\xE7\xBF\xEB";
static const char ServerRecordMemory[] = "\x2A\xC0\xFA\xC0\xE5\xB9\xF6\xC6\xDB\xBB\xE7\xBF\xEB";
static const char DisableParty[] = "\x2A\xB5\xBF\xB7\xE1\xB1\xDD\xC1\xF6";
static const char MaxUsers[] = "\x2A\xC1\xA2\xBC\xD3\xC1\xA6\xC7\xD1\xC0\xCE\xBF\xF8";
static const char TestServer[] = "\x2A\xC5\xD7\xBD\xBA\xC6\xAE\xBC\xAD\xB9\xF6";
static const char ConnectClientCode[] = "\x2A\xC1\xA2\xBC\xD3\xB0\xA1\xB4\xC9\xC4\xDA\xB5\xE5";
static const char DisconnectClientBadCode[] = "\x2A\xC4\xDA\xB5\xE5\xC1\xA2\xBC\xD3\xC1\xBE\xB7\xE1";
static const char ClientSafeCode[] = "\x2A\xB3\xBB\xBA\xCE\xBA\xB8\xC8\xA3\xC4\xDA\xB5\xE5";
static const char DisconnectClientSafeCode[] = "\x2A\xBA\xB8\xC8\xA3\xC4\xDA\xB5\xE5\xBF\xC0\xB7\xF9\xC1\xBE\xB7\xE1";
static const char Version[] = "\x2A\xC1\xA6\xC7\xD1\xB9\xF6\xC0\xFC";
static const char ServerName[] = "\x2A\xBC\xAD\xB9\xF6\xC0\xCC\xB8\xA7";
static const char EnablePk[] = "\x2A\x50\x4B\xC7\xE3\xB0\xA1";
static const char PkField[] = "\x2A\x50\x4B\xC7\xCA\xB5\xE5";
static const char PkFieldAll[] = "\x2A\x50\x4B\xB8\xF0\xB5\xE7\xC7\xCA\xB5\xE5";
static const char PotionMonitor[] = "\x2A\xB9\xB0\xBE\xE0\xB0\xA8\xBD\xC3";
static const char ClientProtect[] = "\x2A\xBA\xB8\xBE\xC8\xB0\xCB\xBB\xE7";
static const char RemoveProtectTime[] = "\x2A\xBA\xB8\xBE\xC8\xC1\xA6\xB0\xC5";
static const char BlockLevel[] = "\x2A\xBA\xED\xB7\xB0\xB4\xDC\xB0\xE8";
static const char BlockLogout[] = "\x2A\xBA\xED\xB7\xB0\xBE\xC6\xBF\xF4";
static const char DisconnectItemError[] = "\x2A\xBE\xC6\xC0\xCC\xC5\xDB\xBF\xC0\xB7\xF9\xC1\xBE\xB7\xE1";
static const char ClientCheckFunction[] = "\x2A\xC6\xE3\xBC\xC7\xB0\xCB\xBB\xE7";
}

template <typename Array>
inline bool AppendIntegerList(legacy_config::LineCursor &cursor, Array &destination, int &count)
{
	const std::size_t capacity = sizeof(destination) / sizeof(destination[0]);
	if(count < 0 || static_cast<std::size_t>(count) > capacity)
		return false;

	for(int item = 0; item < 32 && !cursor.Empty(); ++item)
	{
		if(static_cast<std::size_t>(count) >= capacity)
			return false;
		int value = 0;
		if(!ReadInteger(cursor, &value))
			return false;
		destination[count++] = value;
	}
	return true;
}

template <typename Config>
inline ParseResult ApplyCoreDirective(const char *command, const char *values, Config &config, const CorePolicy &policy)
{
	if(!command)
		return ParseResult::NotHandled;

	Config staged = config;
	legacy_config::LineCursor cursor(values);
	int value = 0;

	if(Matches(command, token::ServerCode, "*SERVER_CODE"))
	{
		if(!ReadInteger(cursor, &value)) return ParseResult::Error;
		staged.ServerCode = static_cast<decltype(staged.ServerCode)>(value);
	}
	else if(Matches(command, token::ServerAutoplay, "*SERVER_AUTOPLAY")) staged.AutoPlayer = 1;
	else if(Matches(command, token::ServerRecordMemory, "*SERVER_RECORD_MEM")) staged.UseRecorMemory = 1;
	else if(Matches(command, token::DisableParty, 0)) staged.DisableParty = 1;
	else if(Matches(command, token::MaxUsers, "*MAX_USERS"))
	{
		if(!ReadInteger(cursor, &value)) return ParseResult::Error;
		staged.ConnectUserMax = static_cast<decltype(staged.ConnectUserMax)>(value);
	}
	else if(Matches(command, token::TestServer, "*TEST_SERVER"))
	{
		if(!ReadOptionalInteger(cursor, 1, &value)) return ParseResult::Error;
		staged.TestSeverMode = value;
	}
	else if(Matches(command, token::ConnectClientCode, "*CONNECT_CLIENT_CODE"))
	{
		if(!AppendIntegerList(cursor, staged.dwClientCheckSum, staged.ClientCheckSumCount)) return ParseResult::Error;
	}
	else if(Matches(command, token::DisconnectClientBadCode, "*DISCONNECT_CLIENT_BADCODE")) staged.ClientCheckOut = 1;
	else if(Matches(command, token::ClientSafeCode, "*CLSAFE_CODE"))
	{
		if(!AppendIntegerList(cursor, staged.dwSafeClientCode, staged.SafeClientCodeCount)) return ParseResult::Error;
	}
	else if(Matches(command, token::DisconnectClientSafeCode, "*DISCONNECT_CLSAFE_CODE")) staged.ClientCodeCheckOut = 1;
	else if(Matches(command, token::Version, "*VERSION"))
	{
		if(!ReadInteger(cursor, &value)) return ParseResult::Error;
		staged.LimitVersion = value;
	}
	else if(policy.allowAdminCredentials && legacy_config::EqualsIgnoreCaseAscii(command, "*ADMIN_IP"))
	{
		char text[128]; if(!cursor.NextValue(text, sizeof(text)) || !CopyText(staged.szAdminIP, text)) return ParseResult::Error;
	}
	else if(policy.allowAdminCredentials && legacy_config::EqualsIgnoreCaseAscii(command, "*ADMIN_LOGIN"))
	{
		char text[128]; if(!cursor.NextValue(text, sizeof(text)) || !CopyText(staged.szAdminLogPassword, text)) return ParseResult::Error;
	}
	else if(Matches(command, token::ServerName, "*SERVER_NAME"))
	{
		char text[128]; if(!cursor.NextValue(text, sizeof(text)) || !CopyText(staged.szServerName, text)) return ParseResult::Error;
	}
	else if(Matches(command, token::EnablePk, "*ENABLE_PK"))
	{
		if(!ReadInteger(cursor, &value)) return ParseResult::Error;
		staged.Enable_PK = value == 0 ? 1 : value;
	}
	else if(Matches(command, token::PkField, "*PK_FIELD"))
	{
		if(!ReadInteger(cursor, &value)) return ParseResult::Error;
		staged.Enable_PKField = value == 0 ? policy.defaultPkField : value;
	}
	else if(policy.allowExtendedPkFields && legacy_config::EqualsIgnoreCaseAscii(command, "*PK_FIELD1"))
	{
		if(!ReadInteger(cursor, &value)) return ParseResult::Error;
		staged.Enable_PKField1 = value == 0 ? 47 : value;
	}
	else if(policy.allowExtendedPkFields && legacy_config::EqualsIgnoreCaseAscii(command, "*PK_FIELD2"))
	{
		if(!ReadInteger(cursor, &value)) return ParseResult::Error;
		staged.Enable_PKField2 = value == 0 ? 50 : value;
	}
	else if(policy.allowExtendedPkFields && legacy_config::EqualsIgnoreCaseAscii(command, "*PK_FIELD3"))
	{
		if(!ReadInteger(cursor, &value)) return ParseResult::Error;
		staged.Enable_PKField3 = value == 0 ? 37 : value;
	}
	else if(Matches(command, token::PkFieldAll, "*PK_FIELD_ALL"))
	{
		if(!ReadInteger(cursor, &value)) return ParseResult::Error;
		staged.Enable_PKField_All = value;
	}
	else if(Matches(command, token::PotionMonitor, "*POTION_MONITOR"))
	{
		if(!ReadOptionalInteger(cursor, 1, &value)) return ParseResult::Error;
		staged.PotionMonitor = value;
	}
	else if(Matches(command, token::ClientProtect, "*CL_PROTECT")) staged.CheckProtect = 1;
#ifdef LOG_THE_PK
	else if(legacy_config::EqualsIgnoreCaseAscii(command, "*PK_LOG")) staged.bPK_LOG = true;
#endif
	else if(Matches(command, token::RemoveProtectTime, "*REMOVE_PROTECT_TIME"))
	{
		if(!ReadInteger(cursor, &value)) return ParseResult::Error;
		staged.RemoveNProtectTime = static_cast<decltype(staged.RemoveNProtectTime)>(value);
	}
	else if(Matches(command, token::BlockLevel, "*BLOCK_LEVEL"))
	{
		if(!ReadInteger(cursor, &value)) return ParseResult::Error;
		staged.BlockLevel = value;
	}
	else if(Matches(command, token::BlockLogout, "*BLOCK_LOGOUT")) staged.BlockLevel_LogOut = 1;
	else if(Matches(command, token::DisconnectItemError, "*DISCONNECT_ITEM_ERROR"))
	{
		if(!ReadOptionalInteger(cursor, 1, &value)) return ParseResult::Error;
		staged.ItemCodeOut = value;
	}
	else if(Matches(command, token::ClientCheckFunction, "*CLIENT_CHECK_FUNC"))
	{
		if(!ReadOptionalInteger(cursor, 0, &value)) return ParseResult::Error;
		staged.ClientFuncCheckMode = value;
	}
	else return ParseResult::NotHandled;

	config = staged;
	return ParseResult::Handled;
}

} // namespace server_config
} // namespace formats
} // namespace summit
