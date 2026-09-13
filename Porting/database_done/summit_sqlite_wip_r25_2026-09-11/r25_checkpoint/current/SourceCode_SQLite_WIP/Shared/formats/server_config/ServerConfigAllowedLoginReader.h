#pragma once

#include <cstdlib>

#include "formats/server_config/ServerConfigCommon.h"

namespace summit
{
namespace formats
{
namespace server_config
{

template <typename Setter>
inline ParseResult ApplyAllowedLoginDirective(const char *command, const char *values, Setter setter)
{
	if(!command || !legacy_config::EqualsIgnoreCaseAscii(command, "*ALLOWED_LOGIN"))
		return ParseResult::NotHandled;

	legacy_config::LineCursor cursor(values);
	char value[128] = { 0 };
	if(!ReadValueOrEmpty(cursor, value, sizeof(value)))
		return ParseResult::Error;

	setter(value[0] != 0, value[0] ? std::atoi(value) : 0);
	return ParseResult::Handled;
}

} // namespace server_config
} // namespace formats
} // namespace summit
