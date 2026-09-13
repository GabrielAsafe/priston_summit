#pragma once

#include "formats/server_config/ServerConfigCoreReader.h"
#include "formats/server_config/ServerConfigEventReader.h"
#include "formats/server_config/ServerConfigRuntimeReader.h"
#include "formats/server_config/ServerConfigNetworkReader.h"

namespace summit
{
namespace formats
{
namespace server_config
{

template <typename Config, typename IpRangeDecoder, typename AddressDecoder, typename AreaCode>
inline ParseResult ApplyCommonDirective(
	const char *command,
	const char *values,
	Config &config,
	const CorePolicy &corePolicy,
	IpRangeDecoder ipRangeDecoder,
	AddressDecoder addressDecoder,
	AreaCode areaCode)
{
	ParseResult result = ApplyCoreDirective(command, values, config, corePolicy);
	if(result != ParseResult::NotHandled)
		return result;

	result = ApplyEventDirective(command, values, config);
	if(result != ParseResult::NotHandled)
		return result;

	result = ApplyRuntimeDirective(command, values, config);
	if(result != ParseResult::NotHandled)
		return result;

	return ApplyNetworkDirective(command, values, config, ipRangeDecoder, addressDecoder, areaCode);
}

} // namespace server_config
} // namespace formats
} // namespace summit
