#pragma once

#include "systems/network/protocol/server/opcodes.h"

#ifndef SERVER_TCP_PORT
/* MINGW_SERVER_LOGIN_PORT_1997_V1
   O cliente ConnectServer_InfoMain usa TCP_SERVPORT=1997.
   TCP_GAMEPORT continua separado em 9000. */
#define SERVER_TCP_PORT 1997
#endif
