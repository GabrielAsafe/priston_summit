#include "opcodes.h"
#include "Login.h"
#include <string>
#include <stdio.h>
#include <smPacket.h>
#include <systems/network/client/netplay.h>
#include <systems/network/v2/legacy/LegacyBackendChannelSender.h>
#include <systems/security/client_integrity/Security.h>
#include <algorithm>

#pragma comment(lib, "iphlpapi")    //to get mac address
#include <iphlpapi.h>               //to get mac address

LoginResult	requestLogin(const std::string &username, const std::string &password)
{
	LoginPacket	packet;

	gameplay::network::legacy::LegacyBackendChannelSender& backend =
		gameplay::network::legacy::ClientLegacyBackendChannelSender();
	const bool dataAvailable = backend.Available(gameplay::network::BackendChannel::Data);

	FILE *fpHandshake = fopen("LOGS\\mingw_login_handshake_client.txt", "a");
	if(fpHandshake)
	{
		fprintf(fpHandshake,
			"[REQUEST_LOGIN_ENTER] accountLen=%u passwordLen=%u dataAvailable=%d\n",
			(unsigned)username.size(),
			(unsigned)password.size(),
			dataAvailable ? 1 : 0);
		fflush(fpHandshake);
		fclose(fpHandshake);
	}

	PIP_ADAPTER_INFO AdapterInfo;
	DWORD dwBufLen = sizeof(AdapterInfo);

	AdapterInfo = (IP_ADAPTER_INFO *)malloc(sizeof(IP_ADAPTER_INFO));

	if(GetAdaptersInfo(AdapterInfo, &dwBufLen) == ERROR_BUFFER_OVERFLOW)
	{
		AdapterInfo = (IP_ADAPTER_INFO *)malloc(dwBufLen);
	}

	if(GetAdaptersInfo(AdapterInfo, &dwBufLen) == NO_ERROR)
	{
		PIP_ADAPTER_INFO info = AdapterInfo; //Copy information
		sprintf_s(packet.mac, sizeof(packet.mac), "%02X-%02X-%02X-%02X-%02X-%02X", info->Address[0], info->Address[1], info->Address[2], info->Address[3], info->Address[4], info->Address[5]);
	}

	free(AdapterInfo);

	packet.size = sizeof(packet);
	packet.code = OpCode::OPCODE_ID_GETUSERINFO;

	//Capturar hash
	strcpy_s(packet.account, sizeof(packet.account), username.c_str());
	strcpy_s(packet.password, sizeof(packet.password), password.c_str());
	strcpy_s(packet.hash, sizeof(packet.hash), Security::getInstance()->getGameHash().c_str());

	//Caminho do Game.exe do PLayer.
	GetModuleFileName(NULL, packet.szPath, MAX_PATH);

	// Nome do Pc do Player.
	DWORD size = sizeof(packet.szPCName) / sizeof(packet.szPCName[0]);
	GetComputerName(packet.szPCName, &size);

	if(dataAvailable)
	{
		const gameplay::network::SendResult sent = backend.Send(
			gameplay::network::BackendChannel::Data,
			gameplay::network::PacketView(&packet, static_cast<std::size_t>(packet.size),
			                              static_cast<std::uint32_t>(packet.code)));
		const int sendResult = sent.Succeeded() ? TRUE : FALSE;
		FILE *fpSend = fopen("LOGS\\mingw_login_handshake_client.txt", "a");
		if(fpSend)
		{
			fprintf(fpSend,
				"[REQUEST_LOGIN_SEND] size=%d code=0x%08X hashLen=%u macLen=%u sendResult=%d\n",
				packet.size,
				(unsigned)packet.code,
				(unsigned)strnlen(packet.hash, sizeof(packet.hash)),
				(unsigned)strnlen(packet.mac, sizeof(packet.mac)),
				sendResult);
			fflush(fpSend);
			fclose(fpSend);
		}
		return sendResult ? LoginResult::OK : LoginResult::FAILED_CONNECTION;
	}

	FILE *fpFail = fopen("LOGS\\mingw_login_handshake_client.txt", "a");
	if(fpFail)
	{
		fprintf(fpFail, "[REQUEST_LOGIN_FAIL] dataChannel=unavailable\n");
		fflush(fpFail);
		fclose(fpFail);
	}

	return LoginResult::FAILED_CONNECTION;

	
}




