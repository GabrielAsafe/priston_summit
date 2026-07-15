#pragma once

#ifndef SUMMIT_SERVER_COMPAT_PACKETS_H
#define SUMMIT_SERVER_COMPAT_PACKETS_H

#include <windows.h>
#ifndef QWORD
typedef unsigned long long QWORD;
#endif

struct smTRANS_COMMAND_QWORD
{
    int size, code;
    DWORD WParam;
    DWORD LParam;
    QWORD SParam;
    QWORD EParam;
};

#endif

#ifndef SUMMIT_SMTRANS_VIP_DEFINED
#define SUMMIT_SMTRANS_VIP_DEFINED
struct smTRANS_VIP
{
    int size, code;
    int Ativo;
};
#endif

#ifndef SUMMIT_PACKET_CROWN_DEFINED
#define SUMMIT_PACKET_CROWN_DEFINED
struct PACKET_CROWN
{
    int size, code;
    DWORD Silver;
    DWORD Bronze;
};
#endif
