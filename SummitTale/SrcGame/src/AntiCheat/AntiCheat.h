#pragma once

#include <Windows.h>

#ifndef _DEBUG
#define ANTI_CHEAT_ENABLE
#endif
enum ESystemOS
{
	SYSTEMOS_Unknown = 0,

	SYSTEMOS_Windows2000 = 1,
	SYSTEMOS_WindowsXP = 2,
	SYSTEMOS_Windows2003 = 3,
	SYSTEMOS_Windows2003R2 = 4,
	SYSTEMOS_WindowsVista = 5,
	SYSTEMOS_Windows2008 = 6,
	SYSTEMOS_Windows2008R2 = 7,
	SYSTEMOS_Windows7 = 8,
	SYSTEMOS_Windows2012 = 9,
	SYSTEMOS_Windows8 = 10,
	SYSTEMOS_Windows2012R2 = 11,
	SYSTEMOS_Windows8_1 = 12,
	SYSTEMOS_Windows2016 = 13,
	SYSTEMOS_Windows10 = 14,
};

struct ChecksumFunction
{
	DWORD										dwSize;
	DWORD										dwAddress;
	DWORD										dwCheckSum;
};

#define CHECKSUM_FUNCTION_TOTAL					400

struct Packet
{
	int										iLength;
	int										iHeader;
};

struct PacketLogCheat : Packet
{
	int										LParam;
	int										iCheatID;
	int										SParam;
	int										EParam;

	int										LxParam;
	int										WxParam;
	int										SxParam;
	int										ExParam;

};

struct PacketLogCheatEx : Packet
{
	int										LParam;
	int										iCheatID;
	int										SParam;
	int										EParam;

	int										LxParam;
	int										WxParam;
	int										SxParam;
	int										ExParam;

	char									szParam[256];
};

struct PacketChecksumFunctionList : Packet
{
	BYTE										bKey;
	ChecksumFunction							sCheckSum[CHECKSUM_FUNCTION_TOTAL];
};

#define WINDOW_CHEATLIST_TOTAL					50

struct PacketWindowList : Packet
{
	int											iCount;
	char										szaWindowName[WINDOW_CHEATLIST_TOTAL][64];
};
struct PacketLoginUser : Packet
{

	DWORD									dwUnk[3];

	char									szUserID[32];
	char									szPassword[32];
	char									szMacAddr[20];
	char									szPCName[32];
	DWORD									dwSerialHD;

	char									szVideoName[256];

	UINT									uWidthScreen;
	UINT									uHeightScreen;

	int										iSystemOS;
};

class AntiCheat
{
protected:
	struct PacketChecksumFunctionList			* psPacketChecksum = NULL;
	OSVERSIONINFO		  os_version_info;
	OSVERSIONINFOEX		  os_version_info_ex;
	SYSTEM_INFO			  sys_info;
	LANGID				  lang_id;
	DWORD										dwChecksumRet = 0;
	HANDLE										hThread = NULL;
private:
	BOOL										ChecksumCheckMismatch(struct ChecksumFunction * pChecksum);
	BOOL										GetModuleName(DWORD dwThreadID, char * pszBuf, int iSizeBuf);
	static DWORD WINAPI							EnumWindowsProc(HWND hWnd, LPARAM lParam);
	void										WindowFunctionHandler();
public:

	struct PacketWindowList						* psPacketWindowCheat = NULL;

	DWORD										dwThreadsID[8];

	AntiCheat();
	virtual ~AntiCheat();

	BOOL										AddNewThreadException(DWORD dwThreadID);

	BOOL										IsGameModules(char * pszFile);
	void										ChecksumFunctionHandler();
	void										FindHackDllThread();

	static DWORD WINAPI							ThreadAntiCheatHandler();

	static void __cdecl							CheckThread();

	void										ReceivePacketHandler(struct PacketChecksumFunctionList * psPacket);

	void										HandlePacket(PacketWindowList * psPacket);
	void										SendLoginData();
	ESystemOS									GetSystemOS();
};
