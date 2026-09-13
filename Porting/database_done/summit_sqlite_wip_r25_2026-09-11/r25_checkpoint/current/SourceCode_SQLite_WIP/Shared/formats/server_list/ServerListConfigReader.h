#pragma once

#include <cstddef>
#include <cstdlib>
#include <cstring>

#include "formats/legacy_config/LegacyLineCursor.h"

namespace summit
{
namespace formats
{
namespace server_list
{

namespace token
{
static const char GameServer[]="\x2A\xB0\xD4\xC0\xD3\xBC\xAD\xB9\xF6\xC1\xA4\xBA\xB8";
static const char ClanServer[]="\x2A\xC5\xAC\xB7\xA3\xBC\xAD\xB9\xF6\xC1\xA4\xBA\xB8";
}

inline bool IsDirective(const char *command,const char *cp949,const char *ascii)
{
	return command && (std::strcmp(command,cp949)==0 || lstrcmpi(command,ascii)==0);
}

template <std::size_t N>
inline bool CopyServerText(char (&destination)[N],const char *source)
{
	if(!source) return false;
	const std::size_t length=std::strlen(source);
	if(length >= N) return false;
	std::memcpy(destination,source,length+1);
	return true;
}

inline bool ReadOptionalValue(legacy_config::LineCursor &cursor,char *output,std::size_t capacity)
{
	if(cursor.Empty()) { if(output && capacity) output[0]=0; return true; }
	return cursor.NextValue(output,capacity);
}

inline bool AppendGameServer(legacy_config::LineCursor &cursor,TRANS_SERVER_LIST &list,DWORD defaultPort)
{
	if(list.ServerCount < 0 || list.ClanServerCount < 0 || list.ServerCount >= TRANS_SERVER_MAX ||
		list.ServerCount + list.ClanServerCount >= TRANS_SERVER_MAX)
		return false;
	_TRANS_SERVER_INFO &server=list.ServerInfo[list.ServerCount];
	char value[128];
	if(!ReadOptionalValue(cursor,value,sizeof(value))) return false;
	if(value[0] && !CopyServerText(server.szServerName,value)) return false;
	if(!ReadOptionalValue(cursor,value,sizeof(value))) return false;
	if(value[0]) { if(!CopyServerText(server.szServerIp1,value)) return false; server.dwServerPort1=defaultPort; }
	if(!ReadOptionalValue(cursor,value,sizeof(value))) return false;
	if(value[0]) { if(!CopyServerText(server.szServerIp2,value)) return false; server.dwServerPort2=defaultPort; }
	if(!ReadOptionalValue(cursor,value,sizeof(value))) return false;
	if(value[0]) { if(!CopyServerText(server.szServerIp3,value)) return false; server.dwServerPort3=defaultPort; }
	++list.ServerCount;
	return true;
}

inline bool AppendClanServer(legacy_config::LineCursor &cursor,TRANS_SERVER_LIST &list)
{
	if(list.ServerCount < 0 || list.ClanServerCount < 0 || list.ServerCount + list.ClanServerCount >= TRANS_SERVER_MAX)
		return false;
	const int index=list.ServerCount+list.ClanServerCount;
	_TRANS_SERVER_INFO &server=list.ServerInfo[index];
	if(!CopyServerText(server.szServerName,"Clan")) return false;
	char value[128];
	if(!ReadOptionalValue(cursor,value,sizeof(value))) return false;
	if(value[0] && !CopyServerText(server.szServerIp1,value)) return false;
	if(!ReadOptionalValue(cursor,value,sizeof(value))) return false;
	if(value[0]) server.dwServerPort1=static_cast<DWORD>(std::atoi(value));
	if(!ReadOptionalValue(cursor,value,sizeof(value))) return false;
	if(value[0] && !CopyServerText(server.szServerIp2,value)) return false;
	if(!ReadOptionalValue(cursor,value,sizeof(value))) return false;
	if(value[0]) server.dwServerPort2=static_cast<DWORD>(std::atoi(value));
	if(!ReadOptionalValue(cursor,value,sizeof(value))) return false;
	if(value[0] && !CopyServerText(server.szServerIp3,value)) return false;
	if(!ReadOptionalValue(cursor,value,sizeof(value))) return false;
	if(value[0]) server.dwServerPort3=static_cast<DWORD>(std::atoi(value));
	++list.ClanServerCount;
	return true;
}

inline int ApplyServerListDirective(const char *command,const char *values,TRANS_SERVER_LIST &list,DWORD defaultGamePort)
{
	if(!command) return 0;
	legacy_config::LineCursor cursor(values);
	if(IsDirective(command,token::GameServer,"*GAME_SERVER"))
		return AppendGameServer(cursor,list,defaultGamePort) ? 1 : -1;
	if(IsDirective(command,token::ClanServer,"*CLAN_SERVER"))
		return AppendClanServer(cursor,list) ? 1 : -1;
	return 0;
}

} // namespace server_list
} // namespace formats
} // namespace summit
