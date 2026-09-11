// Shared packet encoding/safety helpers from legacy Svr_Damge.cpp.
// LegacyDamagePolicy supplies the target-specific encoded packet opcode.

#ifdef _W_SERVER
int rsSetDamagePacketKey(rsPLAYINFO *lpPlayInfo)
{
	DWORD	dwChkSum;
	DWORD	dwObjSum;

	if(!lpPlayInfo->getName()[0] == 0)
		dwChkSum = GetSpeedSum(lpPlayInfo->getName());
	else
		dwChkSum = GetSpeedSum(lpPlayInfo->smCharInfo.szName);

	dwObjSum = lpPlayInfo->dwObjectSerial;

	lpPlayInfo->dwDamagePacketKey[0] = (dwChkSum*dwObjSum / 7) + 0x13;
	lpPlayInfo->dwDamagePacketKey[1] = lpPlayInfo->dwDamagePacketKey[0] ^ ((dwObjSum >> 8) + (dwObjSum << 15)) + 0x1F;

	dwObjSum *= (lpPlayInfo->smCharInfo.JOB_CODE + 2);
	lpPlayInfo->dwDamagePacketKey2[0] = (dwChkSum*dwObjSum / 11) + 0x34;
	lpPlayInfo->dwDamagePacketKey2[1] = lpPlayInfo->dwDamagePacketKey2[0] ^ ((dwObjSum >> 7) + (dwObjSum << 16)) + 0x37;

	return TRUE;
}

int rsSendDamageFuncToClient(rsPLAYINFO *lpPlayInfo)
{
	TRANS_FUNC_MEMORY	TransFuncMemory;

	ZeroMemory(&TransFuncMemory, sizeof(TRANS_FUNC_MEMORY));

	TransFuncMemory.size = DM_ENCODE_PACKET_SIZE + 64 + (rand() % 3) * 4;
	TransFuncMemory.code = OpCode::OPCODE_DAMAGE_ENCODE_MEM;

	TransFuncMemory.Param[0] = DM_ENCODE_PACKET_SIZE;

	memcpy(TransFuncMemory.szData, dm_EncodePacket_AsmCode, DM_ENCODE_PACKET_SIZE);

	rsSetDamagePacketKey(lpPlayInfo);

	((DWORD *)(TransFuncMemory.szData + DM_ENCODE_PACKET_KEY1))[0] = lpPlayInfo->dwDamagePacketKey[1];
	((DWORD *)(TransFuncMemory.szData + DM_ENCODE_PACKET_KEY2))[0] = lpPlayInfo->dwDamagePacketKey[0];

	if(lpPlayInfo && lpPlayInfo->lpsmSock)
	{
		return lpPlayInfo->lpsmSock->Send((char *)&TransFuncMemory, TransFuncMemory.size, TRUE);
	}

	return FALSE;
}

int rsSendDamageFuncToClient2(rsPLAYINFO *lpPlayInfo)
{
	TRANS_FUNC_MEMORY	TransFuncMemory;

	ZeroMemory(&TransFuncMemory, sizeof(TRANS_FUNC_MEMORY));

	TransFuncMemory.size = DM_DECODE_PACKET_SIZE + 64 + (rand() % 3) * 4;
	TransFuncMemory.code = OpCode::OPCODE_DAMAGE_ENCODE_MEM2;

	TransFuncMemory.Param[0] = DM_DECODE_PACKET_SIZE;

	memcpy(TransFuncMemory.szData, dm_DecodePacket_AsmCode, DM_DECODE_PACKET_SIZE);

	rsSetDamagePacketKey(lpPlayInfo);

	((DWORD *)(TransFuncMemory.szData + DM_DECODE_PACKET_KEY1))[0] = lpPlayInfo->dwDamagePacketKey2[1];
	((DWORD *)(TransFuncMemory.szData + DM_DECODE_PACKET_KEY2))[0] = lpPlayInfo->dwDamagePacketKey2[0];

	if(lpPlayInfo && lpPlayInfo->lpsmSock)
	{
		return lpPlayInfo->lpsmSock->Send((char *)&TransFuncMemory, TransFuncMemory.size, TRUE);
	}

	return FALSE;
}

int rsSendPacketFuncToClient(rsPLAYINFO *lpPlayInfo, DWORD dwEncPacketCode, BYTE bEncXor, BYTE bDecXor)
{

	TRANS_FUNC_MEMORY	TransFuncMemory;

	ZeroMemory(&TransFuncMemory, sizeof(TRANS_FUNC_MEMORY));

	TransFuncMemory.size = ENC_PACKET_ASM_SIZE + 64 + (rand() % 3) * 4;
	TransFuncMemory.code = OpCode::OPCODE_PACKET_DYNFUNC;

	TransFuncMemory.Param[0] = ENC_PACKET_ASM_SIZE;
	TransFuncMemory.Param[1] = POS_ENC_START;
	TransFuncMemory.Param[2] = POS_DEC_START;

	memcpy(TransFuncMemory.szData, smwsock_EncodePacket_AsmCode, ENC_PACKET_ASM_SIZE);

	((DWORD *)(TransFuncMemory.szData + POS_ENC_PACCODE))[0] = dwEncPacketCode;
	((BYTE *)(TransFuncMemory.szData + POS_ENC_XOR))[0] = bEncXor;
	((BYTE *)(TransFuncMemory.szData + POS_DEC_XOR))[0] = bDecXor;

	if(lpPlayInfo && lpPlayInfo->lpsmSock)
	{
		return lpPlayInfo->lpsmSock->Send((char *)&TransFuncMemory, TransFuncMemory.size, TRUE);
	}

	return FALSE;
}

int	rsProcessSafePacket(rsPLAYINFO *lpPlayInfo)
{

	DWORD	dwChkSum;
	DWORD	dwObjSum;
	BYTE	bEncXor, bDecXor;
	DWORD	dwEncPacketCode;

	if(!rsServerConfig.CrazyPacketMode) return FALSE;

	//if(!lpPlayInfo->getPassword().empty())
	if(strlen(lpPlayInfo->getName()))
		dwChkSum = GetSpeedSum(lpPlayInfo->getName());
	else
		dwChkSum = GetSpeedSum(lpPlayInfo->smCharInfo.szName);

	dwObjSum = (lpPlayInfo->dwObjectSerial + PACKET_SAFE_CODE)*dwChkSum;

	dwEncPacketCode = (dwObjSum^dwChkSum) & 0x0FFF0000;
	dwEncPacketCode |= LegacyDamagePolicy::SafePacketEncodeOpcode();
	bEncXor = (BYTE)(dwObjSum ^ (dwChkSum >> PACKET_SAFE_SHIFT1)) & 0x7F;
	bDecXor = (BYTE)((dwObjSum >> PACKET_SAFE_SHIFT2) ^ dwChkSum) & 0x7F;

	rsSendPacketFuncToClient(lpPlayInfo, dwEncPacketCode, bDecXor, bEncXor);

	lpPlayInfo->dwDecPacketCode = dwEncPacketCode;
	lpPlayInfo->bPacketXor[0] = bEncXor;
	lpPlayInfo->bPacketXor[1] = bDecXor;

	lpPlayInfo->lpsmSock->dwEncPacketCode = LegacyDamagePolicy::SafePacketEncodeOpcode();
	lpPlayInfo->lpsmSock->bEncXor = bEncXor;
	lpPlayInfo->lpsmSock->bDecXor = bDecXor;

	lpPlayInfo->dwDecPacketTime = dwPlayServTime;
	lpPlayInfo->dwDecPacketTime2 = dwPlayServTime + 60000;

	return TRUE;
}

int	rsCompareSafePacket(rsPLAYINFO *lpPlayInfo, DWORD dwRcvPacketCode)
{
	DWORD	dwChkSum;
	DWORD	dwObjSum;
	BYTE	bEncXor, bDecXor;
	DWORD	dwEncPacketCode;

	if((dwRcvPacketCode >> 28) != 9) return FALSE;

	if(!lpPlayInfo->getName()[0] == 0)
		dwChkSum = GetSpeedSum(lpPlayInfo->getName());
	else
		dwChkSum = GetSpeedSum(lpPlayInfo->smCharInfo.szName);

	dwObjSum = (lpPlayInfo->dwObjectSerial + PACKET_SAFE_CODE)*dwChkSum;

	dwEncPacketCode = (dwObjSum^dwChkSum) & 0x0FFF0000;
	dwEncPacketCode |= LegacyDamagePolicy::SafePacketEncodeOpcode();
	bEncXor = (BYTE)(dwObjSum ^ (dwChkSum >> PACKET_SAFE_SHIFT1)) & 0x7F;
	bDecXor = (BYTE)((dwObjSum >> PACKET_SAFE_SHIFT2) ^ dwChkSum) & 0x7F;

	if(dwRcvPacketCode == dwEncPacketCode)
	{
		lpPlayInfo->dwDecPacketCode = dwEncPacketCode;
		lpPlayInfo->bPacketXor[0] = bEncXor;
		lpPlayInfo->bPacketXor[1] = bDecXor;

		lpPlayInfo->lpsmSock->dwEncPacketCode = LegacyDamagePolicy::SafePacketEncodeOpcode();
		lpPlayInfo->lpsmSock->bEncXor = bEncXor;
		lpPlayInfo->lpsmSock->bDecXor = bDecXor;

		return TRUE;
	}

	return FALSE;
}

int	rsDecodeDamagePacket(rsPLAYINFO *lpPlayInfo, void *lpPacket)
{
	DWORD size, code;
	DWORD cnt, cnt2, cnt3;
	DWORD dwCode;
	DWORD dwCode2;
	DWORD *lpDword = (DWORD *)lpPacket;

	size = lpDword[0];
	code = lpDword[1];
	size /= 4;

	dwCode = (code ^ (code << 7)) + size*lpPlayInfo->dwDamagePacketKey[1];
	cnt2 = (code + lpPlayInfo->dwDamagePacketKey[0]) & 0xF5ABCF;
	cnt3 = cnt2 & 0x3C;

	for(cnt = 2; cnt < size; cnt++)
	{
		dwCode2 = lpDword[cnt];
		lpDword[cnt] = dwCode2 ^ (cnt2 << 6) ^ dwCode;
		dwCode = dwCode2;
		cnt2 += cnt2 + cnt3;
	}

	return TRUE;
}

int	rsEncodeDamagePacket(rsPLAYINFO *lpPlayInfo, void *lpPacket)
{
	DWORD size, code;
	DWORD cnt, cnt2, cnt3;
	DWORD dwCode;
	DWORD *lpDword = (DWORD *)lpPacket;

	size = lpDword[0];
	code = lpDword[1];
	size /= 4;


	dwCode = (code ^ (code << 5)) + size*lpPlayInfo->dwDamagePacketKey2[1];
	cnt2 = (code + lpPlayInfo->dwDamagePacketKey2[0]) & 0xBCAFF5;
	cnt3 = cnt2 & 0x5E;

	for(cnt = 2; cnt < size; cnt++)
	{
		dwCode = dwCode ^ (cnt2 << 7) ^ lpDword[cnt];
		lpDword[cnt] = dwCode;
		cnt2 += cnt2 + cnt3;
	}

	return TRUE;
}

#else

int rsSetDamagePacketKey(rsPLAYINFO *lpPlayInfo)
{
	return TRUE;
}

int rsSendDamageFuncToClient(rsPLAYINFO *lpPlayInfo)
{
	return TRUE;
}

int	rsDecodeDamagePacket(rsPLAYINFO *lpPlayInfo, void *lpPacket)
{
	return TRUE;
}

int	rsEncodeDamagePacket(rsPLAYINFO *lpPlayInfo, void *lpPacket)
{
	return TRUE;
}

#endif

int rsRecordDamageError(rsPLAYINFO *lpPlayInfo, smTRANS_COMMAND *lpTransCommand)
{

	if(lpPlayInfo->WarningCount < 20)
	{
		rsSendDataServer(lpPlayInfo->lpsmSock, lpTransCommand);
		lpPlayInfo->WarningCount++;
	}

	return TRUE;
}
