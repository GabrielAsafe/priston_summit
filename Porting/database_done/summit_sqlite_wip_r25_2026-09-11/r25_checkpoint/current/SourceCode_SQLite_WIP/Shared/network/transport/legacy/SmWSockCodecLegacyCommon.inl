int	EncodePacket(DWORD dwKey, DWORD PacketCnt, BYTE *lpPacket, BYTE *lpEncBuff)
{
	int cnt;
	BYTE ch;
	BYTE bNear;
	DWORD rKey, sKey;
	int	len;
	WORD wTotal;

	sKey = (dwKey << 16) | (PacketCnt & 0xFFFF);

	len = ((int *)lpPacket)[0];
	((int *)lpEncBuff)[0] = len + 8;
	((int *)lpEncBuff)[2] = sKey;

	rKey = sKey >> 4;
	rKey += len;

	bNear = (BYTE)(PacketCnt + dwKey);
	wTotal = 0;

	for(cnt = 4; cnt < len; cnt++)
	{
		wTotal += lpPacket[cnt];
		ch = lpPacket[cnt] ^ ((BYTE)rKey) ^ bNear;
		lpEncBuff[cnt + 8] = ch;
		rKey += cnt*(cnt >> 1);
#ifdef	_PACKET_PASS_XOR
		bNear = ch^_PACKET_PASS_XOR;
#else
		bNear = ch;
#endif
	}

	((int *)lpEncBuff)[1] = (dwKey & 0xFFFF0000) | wTotal;



	return TRUE;
}


DWORD DecodePacket(BYTE *lpPacket, BYTE *lpDecBuff)
{
	int cnt;
	BYTE ch;
	BYTE ch2;
	BYTE bNear;
	DWORD rKey, sKey;
	int	len;
	DWORD dwKey;
	DWORD PacketCnt;
	WORD wTotal;


	len = ((int *)lpPacket)[0] - 8;
	sKey = ((int *)lpPacket)[2];

	dwKey = sKey >> 16;
	PacketCnt = sKey & 0xFFFF;

	rKey = sKey >> 4;
	rKey += len;

	bNear = (BYTE)(PacketCnt + dwKey);
	wTotal = 0;

	((int *)lpDecBuff)[0] = len;

	for(cnt = 4; cnt < len; cnt++)
	{
		ch = lpPacket[cnt + 8];
		ch2 = ch;
		ch ^= ((BYTE)rKey) ^ bNear;
		lpDecBuff[cnt] = ch;
		rKey += cnt*(cnt >> 1);
#ifdef	_PACKET_PASS_XOR
		bNear = ch2^_PACKET_PASS_XOR;
#else
		bNear = ch2;
#endif
		wTotal += ch;
	}

	if((((int *)lpPacket)[1] & 0xFFFF) != wTotal)
		return NULL;

	return sKey;
}





int	EncodePacket2(DWORD dwKey, DWORD PacketCnt, BYTE *lpPacket, BYTE *lpEncBuff, BYTE bEncXor)
{
	int cnt;
	BYTE ch;
	BYTE bNear;
	DWORD rKey, sKey;
	int	len;
	WORD wTotal;

	dwKey |= OPCODE_ENCODE_PACKET;
	sKey = (dwKey << 16) | (PacketCnt & 0xFFFF);

	len = ((int *)lpPacket)[0];
	((int *)lpEncBuff)[0] = len + 8;
	((int *)lpEncBuff)[2] = sKey;

	rKey = sKey >> 4;
	rKey += len;

	bNear = (BYTE)(PacketCnt + dwKey);
	wTotal = 0;

	for(cnt = 4; cnt < len; cnt++)
	{
		wTotal += lpPacket[cnt];
		ch = lpPacket[cnt] ^ ((BYTE)rKey) ^ bNear;
		lpEncBuff[cnt + 8] = ch;
		rKey += cnt*(cnt >> 1);
		bNear = ch^bEncXor;
	}

	((int *)lpEncBuff)[1] = (dwKey & 0xFFFF0000) | wTotal;



	return TRUE;
}


DWORD DecodePacket2(BYTE *lpPacket, BYTE *lpDecBuff, BYTE bDecXor)
{
	int cnt;
	BYTE ch;
	BYTE ch2;
	BYTE bNear;
	DWORD rKey, sKey;
	int	len;
	DWORD dwKey;
	DWORD PacketCnt;
	WORD wTotal;


	len = ((int *)lpPacket)[0] - 8;
	sKey = ((int *)lpPacket)[2];

	dwKey = sKey >> 16;
	PacketCnt = sKey & 0xFFFF;

	rKey = sKey >> 4;
	rKey += len;

	bNear = (BYTE)(PacketCnt + dwKey);
	wTotal = 0;

	((int *)lpDecBuff)[0] = len;

	for(cnt = 4; cnt < len; cnt++)
	{
		ch = lpPacket[cnt + 8];
		ch2 = ch;
		ch ^= ((BYTE)rKey) ^ bNear;
		lpDecBuff[cnt] = ch;
		rKey += cnt*(cnt >> 1);
		bNear = ch2^bDecXor;
		wTotal += ch;
	}

	if((((int *)lpPacket)[1] & 0xFFFF) != wTotal)
		return NULL;

	return sKey;
}
