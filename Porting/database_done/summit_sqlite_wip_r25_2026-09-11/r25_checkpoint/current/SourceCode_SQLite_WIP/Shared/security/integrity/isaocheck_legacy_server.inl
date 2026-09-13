#include <cstring>
#define __ISAOCHECK_CPP__
#include <winsock2.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

#include "systems/persistence/database/core/Database.h"
#include <wininet.h>
#include <time.h>

#include "systems/social/clan/legacy/tjclanDEF.h"

#ifdef USE_PROSTONTALE 
#include "core/server/legacy_sin_runtime/sinLinkHeader.h"
#include "systems/presentation/opening/legacy_runtime/HoLinkHeader.h"
#include "systems/localization/config/language.h"
#include "systems/localization/legacy/ho_text/HoTextFile.h"
#include "systems/social/clan/legacy/cE_CViewClanInfo.h"
#endif


#include "systems/social/clan/legacy/BMPsmooth.h"
#include "systems/social/clan/legacy/tjclan.h"
#include "systems/social/clan/legacy/cE_CViewClanInfo.h"

#include <wininet.h>
#pragma comment(lib,"wininet.lib")



#include "systems/security/integrity/isaocheck.h"

extern void fd(char *fmt, ...);
extern void fd2(char *fmt, ...);

#define BUF_LEN 4096

extern char szConnServerName[16];
extern int nClanTicket;

extern BOOL g_bDebugOption;
void debug_fd2(char *fmt, ...)
{
	if (!g_bDebugOption)
		return;

	char buff[4096];

	va_list args;

	va_start(args, fmt);
	vsprintf_s(buff, sizeof(buff), fmt, args);
	va_end(args);

	strcat_s(buff, sizeof(buff), "/r/n");


	int errorMSGdr = 0;

	if (strlen(buff) > 2048)
	{
		buff[1024] = 0;
		errorMSGdr = 1;

		HANDLE hFile;
		DWORD dwAcess = 0;
		hFile = CreateFile("LOGS\\DDD2.txt", GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		if (hFile != INVALID_HANDLE_VALUE)
		{
			SetFilePointer(hFile, 0, NULL, FILE_END);


			int chk = WriteFile(hFile, buff, lstrlen(buff), &dwAcess, NULL);



			if (errorMSGdr == 1)
			{
				char msg[256] = "../n�޽����� �ʹ��� �߶���";
				WriteFile(hFile, msg, lstrlen(msg), &dwAcess, NULL);
			}



			CloseHandle(hFile);
		}
		return;
	}

	HANDLE hFile;
	DWORD dwAcess = 0;
	hFile = CreateFile("LOGS\\DDD2.txt", GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		SetFilePointer(hFile, 0, NULL, FILE_END);

		int chk = WriteFile(hFile, buff, lstrlen(buff), &dwAcess, NULL);


		if (errorMSGdr == 1)
		{
			char msg[256] = "/n�޽����� �ʹ��� �߶���";
			WriteFile(hFile, msg, lstrlen(msg), &dwAcess, NULL);
		}


		CloseHandle(hFile);
	}
}



BOOL GetHttpRequest(LPSTR httpname, LPSTR Request, DWORD MaxBuffer, int *len)
{
	// R17: SoD2/notice/mark consumers no longer use the generic legacy HTTP
	// fetch helper.  Preserve the symbol for compatibility but never perform
	// external I/O if an old path reaches it.
	(void)httpname;
	if(Request && MaxBuffer)
		Request[0] = 0;
	if(len)
		*len = 0;
	return FALSE;
}



ISAOCHECK::ISAOCHECK()
{

	HSsu = 0;
	for (int i = 0; i < 30; i++)
		fileDATA_HS[i] = NULL;
}

ISAOCHECK::~ISAOCHECK()
{
}

void ISAOCHECK::init()
{
	iCheckFlag = 0;
	hostPORT = 0;
}

void ISAOCHECK::init_HostName_Port(char *hostname, int port)
{
	if (hostname == NULL) return;

	hostPORT = port;
	ZeroMemory(hostName, sizeof(char) * 64);



	strcpy_s(hostName, sizeof(hostName), hostname);

}

void ISAOCHECK::init_HostName_Port1(char *hostname, int port)
{
	if (hostname == NULL) return;

	hostPORT1 = port;
	ZeroMemory(hostName1, sizeof(char) * 64);
	strcpy_s(hostName1, sizeof(hostName1), hostname);
}

void ISAOCHECK::init(int iCheckON, int iTimeOUT)
{
	iCheckFlag = iCheckON;
	iTimeOut = iTimeOUT;
}

void ISAOCHECK::close()
{
}


void ISAOCHECK::main()
{

}



int ISAOCHECK::connect_WEBdb(char *szWriteBuffer, char *szReadBuffer)
{
	char szSizeBuffer[32];
	DWORD dwLengthSizeBuffer = sizeof(szSizeBuffer);
	DWORD dwFileSize = 0;
	DWORD dwBytesRead = 0;

	int iRet = ISAO_ERR_WINSOCK_NOERR;

	ZeroMemory(szReadBuffer, BUF_LEN);


	HINTERNET m_Session = NULL;

	m_Session = InternetOpen("AppName",
		INTERNET_OPEN_TYPE_PRECONFIG,

		NULL,

		NULL,

		0);
	if (!m_Session)
	{
		iRet = ISAO_ERR_TCP_CONNECT;
		return iRet;
	}
	HINTERNET hHttpFile = NULL;
	hHttpFile = InternetOpenUrl(m_Session, (const char *)szWriteBuffer, NULL,
		0, 0, 0);
	if (hHttpFile)
	{
		BOOL bQuery = HttpQueryInfo(hHttpFile, HTTP_QUERY_CONTENT_LENGTH, szSizeBuffer,
			&dwLengthSizeBuffer, NULL);

		if (bQuery == TRUE)
		{
			dwFileSize = atol(szSizeBuffer);
			dwFileSize > BUF_LEN ? dwFileSize = BUF_LEN :
				dwFileSize = dwFileSize;

			BOOL bRead = InternetReadFile(hHttpFile, szReadBuffer, dwFileSize, &dwBytesRead);
			if (bRead)
			{

				if ((bRead == TRUE) && (dwFileSize != dwBytesRead))
				{
					iRet = ISAO_ERR_RECONNECT;
				}


			}
			else iRet = ISAO_ERR_RECONNECT;
			InternetCloseHandle(hHttpFile);
		}
	}
	else
	{
		iRet = ISAO_ERR_TCP_CONNECT;
	}
	if (hHttpFile) InternetCloseHandle(hHttpFile);
	if (m_Session) InternetCloseHandle(m_Session);
	return iRet;
}

int ISAOCHECK::connect_WEBdb_1(char *szWriteBuffer, char *szReadBuffer)
{
	// R17: this secondary web connector lost its last active caller when clan
	// creation moved to SQLite.  Keep the legacy ABI but fail closed locally.
	(void)szWriteBuffer;
	if(szReadBuffer)
		szReadBuffer[0] = 0;
	return ISAO_ERR_TCP_CONNECT;
}



int ISAOCHECK::connect_WEBdb_BINARY(char *szWriteBuffer, char *szReadBuffer, int *dataSize)
{
	// R17: binary clan-mark downloads now use Database\clan_marks.  The old
	// web connector is intentionally disabled.
	(void)szWriteBuffer;
	if(szReadBuffer)
		szReadBuffer[0] = 0;
	if(dataSize)
		*dataSize = 0;
	return ISAO_ERR_TCP_CONNECT;
}


int ISAOCHECK::chk_readbuffer(char *cmd, char *readbuf, int *iRejectC)
{
	int iRejectCode = 111111;
	char *pBuf = strstr(readbuf, cmd);

	if (pBuf == NULL)
		return ISAO_ERR_NONE_REEJCT;


	pBuf = pBuf + strlen(cmd);
	while (true)
	{
		if (*pBuf != ' ')
			break;

		pBuf++;
	}
	sscanf_s(pBuf, "%d", &iRejectCode);

	return iRejectCode;
}



int ISAOCHECK::chk_readStrBuf(char *cmd, char *readbuf, char *retBuf)
{
	int iRejectCode = 111111;
	char *pBuf = strstr(readbuf, cmd);

	if (!pBuf)
		return ISAO_ERR_NONE_REEJCT;


	pBuf = pBuf + strlen(cmd);

	while (true)
	{
		if (*pBuf != ' ')
			break;

		pBuf++;
	}
	sscanf_s(pBuf, "%s", retBuf, strlen(retBuf));

	return iRejectCode;
}

int ISAOCHECK::IsaoCheck(char *id, char *pass, char *ipaddr, long StartTime)
{
	// R17: this HTTP login bridge has no active production caller.  The actual
	// server login path is bSql_LogOn/SQLLogon, with the proven free-account
	// subset handled by SQLite.  Do not send credentials to the legacy web
	// service if this dormant API is reached accidentally.
	(void)id;
	(void)pass;
	(void)ipaddr;
	(void)StartTime;
	return ISAO_REJECT_DB_MAINTE;
}


int ISAOCHECK::IsaoCheck_GameExit(char *id, char *pass, char *ipaddr, long usertime)
{
	// R17: the historical Login_End.php hook has no active production caller.
	// Session persistence no longer relies on this web endpoint.  Keep the
	// dormant API side-effect free and report a local no-op success.
	(void)id;
	(void)pass;
	(void)ipaddr;
	(void)usertime;
	return ISAO_ERR_WINSOCK_NOERR;
}


int ISAOCHECK::isCheckClanJang(char *ClanZang_id, char* gserver)
{
	(void)gserver;
	std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_ClanDB);
	if(!db || !db->Open())
		return 100;

	int result = 100;
	if(db->Prepare("SELECT TOP 1 1 FROM CL WHERE UserID=?"))
	{
		if(db->BindInputParameter(ClanZang_id, 1, PARAMTYPE_String))
		{
			if(db->Execute())
				result = 1;
			else if(db->GetLastError()[0] == 0)
				result = 0;
		}
	}

	db->Close();
	return result;
}


int ISAOCHECK::isCheckClanName(char *ClanName, char *gserver)
{
	(void)gserver;
	std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_ClanDB);
	if(!db || !db->Open())
		return 100;

	int result = 100;
	if(db->Prepare("SELECT TOP 1 1 FROM CL WHERE ClanName=?"))
	{
		if(db->BindInputParameter(ClanName, 1, PARAMTYPE_String))
		{
			if(db->Execute())
				result = 1;
			else if(db->GetLastError()[0] == 0)
				result = 0;
		}
	}

	db->Close();
	return result;
}


int ISAOCHECK::ReadClanInfo(char *clanNum, char* ret_Buf)
{
	if(ret_Buf)
		ret_Buf[0] = 0;

	std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_ClanDB);
	if(!db || !db->Open())
		return 100;

	int result = 100;
	int clanMark = clanNum ? atoi(clanNum) : 0;
	char clanName[128] = { 0 };
	char clanNote[1500] = { 0 };

	if(db->Prepare("SELECT TOP 1 ClanName, Note FROM CL WHERE MIconCnt=?"))
	{
		if(db->BindInputParameter(&clanMark, 1, PARAMTYPE_Integer))
		{
			if(db->Execute())
			{
				db->GetData(clanName, sizeof(clanName), 1, PARAMTYPE_String);
				db->GetData(clanNote, sizeof(clanNote), 2, PARAMTYPE_String);
				if(ret_Buf)
					sprintf_s(ret_Buf, 2048, "Code=1\r\nCName=%s\r\nCNote=%s\r\n", clanName, clanNote);
				result = 1;
			}
			else if(db->GetLastError()[0] == 0)
			{
				if(ret_Buf)
					strcpy_s(ret_Buf, 2048, "Code=0\r\n");
				result = 0;
			}
		}
	}

	db->Close();
	return result;
}

int ISAOCHECK::isCheckClanMember(char *pszServerName, char *pszUserName)
{
	(void)pszServerName;
	std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_ClanDB);
	if(!db || !db->Open())
		return 100;

	int result = 100;
	if(db->Prepare("SELECT TOP 1 1 FROM UL WHERE ChName=?"))
	{
		if(db->BindInputParameter(pszUserName, 1, PARAMTYPE_String))
		{
			if(db->Execute())
				result = 1;
			else if(db->GetLastError()[0] == 0)
				result = 0;
		}
	}

	db->Close();
	return result;
}


int ISAOCHECK::isClanMember(char *ClanZang_id, char* gserver, char* ClanZang_ChaName, char* ret_Buf)
{
	(void)ClanZang_id;
	(void)gserver;

	if(!ret_Buf || !ClanZang_ChaName || !ClanZang_ChaName[0])
		return 100;

	ret_Buf[0] = 0;

	std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_ClanDB);
	if(!db || !db->Open())
		return 100;

	char clanName[64] = { 0 };
	if(!db->Prepare("SELECT TOP 1 ClanName FROM UL WHERE ChName=?") ||
	   !db->BindInputParameter(ClanZang_ChaName, 1, PARAMTYPE_String))
	{
		db->Close();
		return 100;
	}

	if(!db->Execute())
	{
		if(db->GetLastError()[0] != 0)
		{
			db->Close();
			return 100;
		}

		sprintf_s(ret_Buf, 2048, "Code=0\r\nCMoney=500000\r\nCNFlag=0\r\n");
		db->Close();
		return 0;
	}

	if(!db->GetData(clanName, sizeof(clanName), 1, PARAMTYPE_String) || !clanName[0])
	{
		// The legacy ASP removed stale UL rows whose ClanName was empty.
		if(db->Prepare("DELETE FROM UL WHERE ChName=?") &&
		   db->BindInputParameter(ClanZang_ChaName, 1, PARAMTYPE_String))
			db->Execute(FALSE);

		sprintf_s(ret_Buf, 2048, "Code=0\r\nCMoney=500000\r\nCNFlag=0\r\n");
		db->Close();
		return 0;
	}

	char clanZang[64] = { 0 };
	char note[512] = { 0 };
	char regiDate[64] = { 0 };
	char limitDate[64] = { 0 };
	int memCnt = 0;
	int iconCnt = 0;
	int pFlag = 0;
	int kFlag = 0;
	int cnFlag = 0;
	long long clanMoney = 500000;

	if(!db->Prepare("SELECT TOP 1 ClanZang, MemCnt, Note, MIconCnt, RegiDate, LimitDate, PFlag, KFlag, ClanMoney, CNFlag FROM CL WHERE ClanName=?") ||
	   !db->BindInputParameter(clanName, 1, PARAMTYPE_String))
	{
		db->Close();
		return 100;
	}

	if(!db->Execute())
	{
		if(db->GetLastError()[0] != 0)
		{
			db->Close();
			return 100;
		}

		// Preserve the legacy cleanup for a member pointing at a missing clan.
		if(db->Prepare("DELETE FROM UL WHERE ChName=?") &&
		   db->BindInputParameter(ClanZang_ChaName, 1, PARAMTYPE_String))
			db->Execute(FALSE);

		sprintf_s(ret_Buf, 2048, "Code=0\r\nCMoney=500000\r\nCNFlag=0\r\n");
		db->Close();
		return 0;
	}

	if(!db->GetData(clanZang, sizeof(clanZang), 1, PARAMTYPE_String) ||
	   !db->GetData(&memCnt, sizeof(memCnt), 2, PARAMTYPE_Integer) ||
	   !db->GetData(note, sizeof(note), 3, PARAMTYPE_String) ||
	   !db->GetData(&iconCnt, sizeof(iconCnt), 4, PARAMTYPE_Integer) ||
	   !db->GetData(regiDate, sizeof(regiDate), 5, PARAMTYPE_String) ||
	   !db->GetData(limitDate, sizeof(limitDate), 6, PARAMTYPE_String) ||
	   !db->GetData(&pFlag, sizeof(pFlag), 7, PARAMTYPE_Integer) ||
	   !db->GetData(&kFlag, sizeof(kFlag), 8, PARAMTYPE_Integer) ||
	   !db->GetData(&clanMoney, sizeof(clanMoney), 9, PARAMTYPE_Int64) ||
	   !db->GetData(&cnFlag, sizeof(cnFlag), 10, PARAMTYPE_Integer))
	{
		db->Close();
		return 100;
	}

	char subChip[64] = { 0 };
	if(!db->Prepare("SELECT TOP 1 ChName FROM UL WHERE ClanName=? AND Permi='2'") ||
	   !db->BindInputParameter(clanName, 1, PARAMTYPE_String))
	{
		db->Close();
		return 100;
	}

	if(db->Execute())
	{
		if(!db->GetData(subChip, sizeof(subChip), 1, PARAMTYPE_String))
		{
			db->Close();
			return 100;
		}
	}
	else if(db->GetLastError()[0] != 0)
	{
		db->Close();
		return 100;
	}

	int roleCode = 1;
	if(lstrcmpi(clanZang, ClanZang_ChaName) == 0)
		roleCode = 2;
	else if(subChip[0] && lstrcmpi(subChip, ClanZang_ChaName) == 0)
		roleCode = 5;

	int responsePFlag = (roleCode == 1) ? 0 : pFlag;
	int responseKFlag = (roleCode == 1) ? 0 : kFlag;

	sprintf_s(ret_Buf, 2048,
		"Code=%d\r\n"
		"CName=%s\r\n"
		"CNote=%s\r\n"
		"CSubChip=%s\r\n"
		"CMCnt=%d\r\n"
		"CZang=%s\r\n"
		"CStats=1\r\n"
		"CSec=60\r\n"
		"CRegiD=%s\r\n"
		"CLimitD=%s\r\n"
		"CDelActive=0\r\n"
		"CIMG=%d\r\n"
		"CPFlag=%d\r\n"
		"CKFlag=%d\r\n"
		"CMoney=%lld\r\n"
		"CNFlag=%d\r\n",
		roleCode, clanName, note, subChip, memCnt, clanZang,
		regiDate, limitDate, iconCnt, responsePFlag, responseKFlag,
		clanMoney, cnFlag);

	// The legacy isClanMember.asp synchronized the member's cached clan-mark
	// id after composing the response. Preserve that side effect locally.
	if(db->Prepare("UPDATE UL SET MIconCnt=? WHERE ChName=?"))
	{
		if(db->BindInputParameter(&iconCnt, 1, PARAMTYPE_Integer) &&
		   db->BindInputParameter(ClanZang_ChaName, 2, PARAMTYPE_String))
			db->Execute(FALSE);
	}

	db->Close();
	return roleCode;
}


int ISAOCHECK::joinClanMember(char *ClanZang_ID, char *gserver, char *ClanZang_ChaName, char* ClanName, char* ClanWon, char* ClanWon_ID, int level, DWORD chtype, int joinlevel, char *chipflag)
{
	(void)level;
	(void)chipflag;

	if(!ClanZang_ID || !ClanZang_ID[0] ||
	   !gserver || !gserver[0] ||
	   !ClanZang_ChaName || !ClanZang_ChaName[0] ||
	   !ClanName || !ClanName[0] ||
	   !ClanWon || !ClanWon[0] ||
	   !ClanWon_ID || !ClanWon_ID[0])
		return 100;

	std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_ClanDB);
	if(!db || !db->Open())
		return 100;
	if(!db->BeginTransaction())
	{
		db->Close();
		return 100;
	}

	int clanIdx = 0;
	int memCnt = 0;
	char clanLeader[64] = { 0 };

	if(!db->Prepare("SELECT TOP 1 IDX, ClanZang, MemCnt FROM CL WHERE ClanName=?") ||
	   !db->BindInputParameter(ClanName, 1, PARAMTYPE_String))
	{
		db->RollbackTransaction();
		db->Close();
		return 100;
	}

	if(!db->Execute())
	{
		const bool queryFailed = db->GetLastError()[0] != 0;
		db->RollbackTransaction();
		db->Close();
		return queryFailed ? 100 : 0;
	}

	if(!db->GetData(&clanIdx, sizeof(clanIdx), 1, PARAMTYPE_Integer) ||
	   !db->GetData(clanLeader, sizeof(clanLeader), 2, PARAMTYPE_String) ||
	   !db->GetData(&memCnt, sizeof(memCnt), 3, PARAMTYPE_Integer))
	{
		db->RollbackTransaction();
		db->Close();
		return 100;
	}

	if((memCnt + 1) > 100)
	{
		db->RollbackTransaction();
		db->Close();
		return 2;
	}

	char clanSubChief[64] = { 0 };
	if(!db->Prepare("SELECT TOP 1 ChName FROM UL WHERE Permi='2' AND ClanName=?") ||
	   !db->BindInputParameter(ClanName, 1, PARAMTYPE_String))
	{
		db->RollbackTransaction();
		db->Close();
		return 100;
	}
	if(db->Execute())
	{
		if(!db->GetData(clanSubChief, sizeof(clanSubChief), 1, PARAMTYPE_String))
		{
			db->RollbackTransaction();
			db->Close();
			return 100;
		}
	}
	else if(db->GetLastError()[0] != 0)
	{
		db->RollbackTransaction();
		db->Close();
		return 100;
	}

	if(strcmp(clanLeader, ClanZang_ChaName) != 0 &&
	   (!clanSubChief[0] || strcmp(clanSubChief, ClanZang_ChaName) != 0))
	{
		db->RollbackTransaction();
		db->Close();
		return 0;
	}

	char existingClan[64] = { 0 };
	if(!db->Prepare("SELECT TOP 1 ClanName FROM UL WHERE ChName=?") ||
	   !db->BindInputParameter(ClanWon, 1, PARAMTYPE_String))
	{
		db->RollbackTransaction();
		db->Close();
		return 100;
	}

	const BOOL existingMember = db->Execute();
	if(existingMember)
	{
		if(!db->GetData(existingClan, sizeof(existingClan), 1, PARAMTYPE_String))
		{
			db->RollbackTransaction();
			db->Close();
			return 100;
		}

		if(existingClan[0])
		{
			db->RollbackTransaction();
			db->Close();
			return 0;
		}

		if(!db->Prepare("DELETE FROM UL WHERE ChName=?") ||
		   !db->BindInputParameter(ClanWon, 1, PARAMTYPE_String) ||
		   !db->Execute(FALSE))
		{
			db->RollbackTransaction();
			db->Close();
			return 100;
		}
	}
	else if(db->GetLastError()[0] != 0)
	{
		db->RollbackTransaction();
		db->Close();
		return 100;
	}

	int newMemCnt = memCnt + 1;
	if(!db->Prepare("UPDATE CL SET MemCnt=? WHERE ClanName=?") ||
	   !db->BindInputParameter(&newMemCnt, 1, PARAMTYPE_Integer) ||
	   !db->BindInputParameter(ClanName, 2, PARAMTYPE_String) ||
	   !db->Execute(FALSE))
	{
		db->RollbackTransaction();
		db->Close();
		return 100;
	}

	int chTypeValue = static_cast<int>(chtype);
	if(!db->Prepare(
		"INSERT INTO UL (IDX, userid, ChName, ClanName, ChType, ChLv, Permi, JoinDate, DelActive, PFlag, KFlag, MIconCnt) "
		"VALUES (?, ?, ?, ?, ?, ?, '0', GETDATE(), '0', 0, 0, 0)") ||
	   !db->BindInputParameter(&clanIdx, 1, PARAMTYPE_Integer) ||
	   !db->BindInputParameter(ClanWon_ID, 2, PARAMTYPE_String) ||
	   !db->BindInputParameter(ClanWon, 3, PARAMTYPE_String) ||
	   !db->BindInputParameter(ClanName, 4, PARAMTYPE_String) ||
	   !db->BindInputParameter(&chTypeValue, 5, PARAMTYPE_Integer) ||
	   !db->BindInputParameter(&joinlevel, 6, PARAMTYPE_Integer) ||
	   !db->Execute(FALSE))
	{
		db->RollbackTransaction();
		db->Close();
		return 100;
	}

	if(!db->CommitTransaction())
	{
		db->RollbackTransaction();
		db->Close();
		return 100;
	}

	db->Close();
	return 1;
}


int ISAOCHECK::makeClan(char *ClanZang_ID, char* gserver, char* ClanZang_ChaName, char* ClanName, char *explanation, DWORD chtype, int level, char* data)
{
	(void)explanation;

	// Sandurr/Gregoo clanInsert.aspx compatibility:
	// - required identity/server/character/clan inputs -> Code=100 when absent;
	// - a character already in a clan -> Code=2;
	// - an existing clan name -> Code=3;
	// - LI allocates the next clan mark starting from Icon_Default=1000000000;
	// - CL/UL are inserted and success returns Code=1 + CMoney=500000.
	// The historical script copied the default BMP only when it existed and did
	// not fail clan creation when it was absent. Preserve that observable rule.
	if(!ClanZang_ID || !ClanZang_ID[0] ||
	   !gserver || !gserver[0] ||
	   !ClanZang_ChaName || !ClanZang_ChaName[0] ||
	   !ClanName || !ClanName[0])
		return 100;

	std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_ClanDB);
	if(!db || !db->Open())
		return 100;
	if(!db->BeginTransaction())
	{
		db->Close();
		return 100;
	}

	char existingClan[64] = { 0 };
	if(!db->Prepare("SELECT TOP 1 ClanName FROM UL WHERE ChName=?") ||
	   !db->BindInputParameter(ClanZang_ChaName, 1, PARAMTYPE_String))
	{
		db->RollbackTransaction();
		db->Close();
		return 100;
	}

	if(db->Execute())
	{
		if(!db->GetData(existingClan, sizeof(existingClan), 1, PARAMTYPE_String))
		{
			db->RollbackTransaction();
			db->Close();
			return 100;
		}

		if(existingClan[0])
		{
			db->RollbackTransaction();
			db->Close();
			return 2;
		}

		// The ASP removed stale UL rows whose ClanName was empty before creating.
		if(!db->Prepare("DELETE FROM UL WHERE ChName=?") ||
		   !db->BindInputParameter(ClanZang_ChaName, 1, PARAMTYPE_String) ||
		   !db->Execute(FALSE))
		{
			db->RollbackTransaction();
			db->Close();
			return 100;
		}
	}
	else if(db->GetLastError()[0] != 0)
	{
		db->RollbackTransaction();
		db->Close();
		return 100;
	}

	if(!db->Prepare("SELECT TOP 1 ClanZang FROM CL WHERE ClanName=?") ||
	   !db->BindInputParameter(ClanName, 1, PARAMTYPE_String))
	{
		db->RollbackTransaction();
		db->Close();
		return 100;
	}
	if(db->Execute())
	{
		db->RollbackTransaction();
		db->Close();
		return 3;
	}
	if(db->GetLastError()[0] != 0)
	{
		db->RollbackTransaction();
		db->Close();
		return 100;
	}

	const int iconDefault = 1000000000;
	int currentImage = iconDefault;
	if(!db->Prepare("SELECT TOP 1 IMG FROM LI WHERE ID=1"))
	{
		db->RollbackTransaction();
		db->Close();
		return 100;
	}

	if(db->Execute())
	{
		if(!db->GetData(&currentImage, sizeof(currentImage), 1, PARAMTYPE_Integer))
		{
			db->RollbackTransaction();
			db->Close();
			return 100;
		}
	}
	else if(db->GetLastError()[0] != 0)
	{
		db->RollbackTransaction();
		db->Close();
		return 100;
	}
	else
	{
		// Historical first-use path inserted (Icon_Default + 1, ID=1).
		int initialImage = iconDefault + 1;
		if(!db->Prepare("INSERT INTO LI (IMG, ID) VALUES (?, 1)") ||
		   !db->BindInputParameter(&initialImage, 1, PARAMTYPE_Integer) ||
		   !db->Execute(FALSE))
		{
			db->RollbackTransaction();
			db->Close();
			return 100;
		}
		currentImage = iconDefault;
	}

	int imageId = currentImage + 1;
	if(!db->Prepare("UPDATE LI SET IMG=? WHERE ID=1") ||
	   !db->BindInputParameter(&imageId, 1, PARAMTYPE_Integer) ||
	   !db->Execute(FALSE))
	{
		db->RollbackTransaction();
		db->Close();
		return 100;
	}

	char noteDefault[] = "New clan";
	int memberCount = 1;
	int noteCount = 1;
	if(!db->Prepare(
		"INSERT INTO CL "
		"(ClanName,UserID,ClanZang,MemCnt,Note,MIconCnt,RegiDate,LimitDate,DelActive,PFlag,KFlag,Flag,NoteCnt,Cpoint,CWin,CFail,ClanMoney,CNFlag,SiegeMoney,LIDER_BC) "
		"VALUES (?,?,?,?,?,?,datetime('now','localtime'),datetime('now','localtime','+3600 days'),'0',0,0,0,?,0,0,0,0,0,0,0)") ||
	   !db->BindInputParameter(ClanName, 1, PARAMTYPE_String) ||
	   !db->BindInputParameter(ClanZang_ID, 2, PARAMTYPE_String) ||
	   !db->BindInputParameter(ClanZang_ChaName, 3, PARAMTYPE_String) ||
	   !db->BindInputParameter(&memberCount, 4, PARAMTYPE_Integer) ||
	   !db->BindInputParameter(noteDefault, 5, PARAMTYPE_String) ||
	   !db->BindInputParameter(&imageId, 6, PARAMTYPE_Integer) ||
	   !db->BindInputParameter(&noteCount, 7, PARAMTYPE_Integer) ||
	   !db->Execute(FALSE))
	{
		db->RollbackTransaction();
		db->Close();
		return 100;
	}

	int clanIdx = 0;
	if(!db->Prepare("SELECT TOP 1 IDX FROM CL WHERE ClanName=?") ||
	   !db->BindInputParameter(ClanName, 1, PARAMTYPE_String) ||
	   !db->Execute() ||
	   !db->GetData(&clanIdx, sizeof(clanIdx), 1, PARAMTYPE_Integer))
	{
		db->RollbackTransaction();
		db->Close();
		return 100;
	}

	int chType = static_cast<int>(chtype);
	if(!db->Prepare(
		"INSERT INTO UL "
		"(IDX,userid,ChName,ClanName,ChType,ChLv,Permi,JoinDate,DelActive,PFlag,KFlag,MIconCnt) "
		"VALUES (?,?,?,?,?,?,'0',datetime('now','localtime'),'0',0,0,?)") ||
	   !db->BindInputParameter(&clanIdx, 1, PARAMTYPE_Integer) ||
	   !db->BindInputParameter(ClanZang_ID, 2, PARAMTYPE_String) ||
	   !db->BindInputParameter(ClanZang_ChaName, 3, PARAMTYPE_String) ||
	   !db->BindInputParameter(ClanName, 4, PARAMTYPE_String) ||
	   !db->BindInputParameter(&chType, 5, PARAMTYPE_Integer) ||
	   !db->BindInputParameter(&level, 6, PARAMTYPE_Integer) ||
	   !db->BindInputParameter(&imageId, 7, PARAMTYPE_Integer) ||
	   !db->Execute(FALSE))
	{
		db->RollbackTransaction();
		db->Close();
		return 100;
	}

	if(!db->CommitTransaction())
	{
		db->RollbackTransaction();
		db->Close();
		return 100;
	}
	db->Close();

	// Preserve the original best-effort asset copy. Missing/default-copy errors
	// do not change the successful database result.
	char defaultMark[MAX_PATH] = { 0 };
	char clanMark[MAX_PATH] = { 0 };
	sprintf_s(defaultMark, sizeof(defaultMark), "Database\\clan_marks\\%d.bmp", iconDefault);
	sprintf_s(clanMark, sizeof(clanMark), "Database\\clan_marks\\%d.bmp", imageId);
	if(GetFileAttributesA(defaultMark) != INVALID_FILE_ATTRIBUTES)
		CopyFileA(defaultMark, clanMark, FALSE);

	if(data)
		strcpy_s(data, 2048, "Code=1\rCMoney=500000\r");
	return 1;
}


int ISAOCHECK::read_ClanName(int readClumns)
{
	// R17: clan_readClanName.php is dead legacy in the server call graph.
	// Active clan reads use ClanDB through the SQLite-backed SQLConnection.
	(void)readClumns;
	return ISAO_ERR_NONE_REEJCT;
}


int ISAOCHECK::read_ClanData(char *ClanZang_ID, char* gserver, char* ClanZang_ChaName, char* ret_Buf)
{
	(void)ClanZang_ID;
	(void)gserver;

	if(!ret_Buf || !ClanZang_ChaName || !ClanZang_ChaName[0])
		return 100;

	ret_Buf[0] = 0;

	std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_ClanDB);
	if(!db || !db->Open())
		return 100;

	char clanName[64] = { 0 };
	if(!db->Prepare("SELECT TOP 1 ClanName FROM UL WHERE ChName=?") ||
	   !db->BindInputParameter(ClanZang_ChaName, 1, PARAMTYPE_String))
	{
		db->Close();
		return 100;
	}

	if(!db->Execute())
	{
		const bool queryFailed = db->GetLastError()[0] != 0;
		if(!queryFailed)
			strcpy_s(ret_Buf, 2048, "Code=0\r");
		db->Close();
		return queryFailed ? 100 : 0;
	}

	if(!db->GetData(clanName, sizeof(clanName), 1, PARAMTYPE_String))
	{
		db->Close();
		return 100;
	}

	if(!clanName[0])
	{
		if(db->Prepare("DELETE FROM UL WHERE ChName=?") &&
		   db->BindInputParameter(ClanZang_ChaName, 1, PARAMTYPE_String))
			db->Execute(FALSE);
		strcpy_s(ret_Buf, 2048, "Code=0\r");
		db->Close();
		return 0;
	}

	if(!db->Prepare("SELECT TOP 1 1 FROM CL WHERE ClanName=?") ||
	   !db->BindInputParameter(clanName, 1, PARAMTYPE_String))
	{
		db->Close();
		return 100;
	}

	if(!db->Execute())
	{
		const bool queryFailed = db->GetLastError()[0] != 0;
		if(!queryFailed)
		{
			if(db->Prepare("DELETE FROM UL WHERE ChName=?") &&
			   db->BindInputParameter(ClanZang_ChaName, 1, PARAMTYPE_String))
				db->Execute(FALSE);
			strcpy_s(ret_Buf, 2048, "Code=0\r");
		}
		db->Close();
		return queryFailed ? 100 : 0;
	}

	if(!db->Prepare("SELECT ChName FROM UL WHERE ClanName=? ORDER BY MIDX") ||
	   !db->BindInputParameter(clanName, 1, PARAMTYPE_String))
	{
		db->Close();
		return 100;
	}

	if(!db->Execute())
	{
		const bool queryFailed = db->GetLastError()[0] != 0;
		if(!queryFailed)
			strcpy_s(ret_Buf, 2048, "Code=0\r");
		db->Close();
		return queryFailed ? 100 : 0;
	}

	// GetAllMyClanMember.asp returned records separated only by CR.  The
	// legacy parser searches the first CMem= and then skips exactly the six
	// bytes "\rCMem=" between subsequent names.
	strcpy_s(ret_Buf, 2048, "Code=1\r");
	do
	{
		char memberName[64] = { 0 };
		if(!db->GetData(memberName, sizeof(memberName), 1, PARAMTYPE_String))
		{
			db->Close();
			return 100;
		}

		char memberLine[80] = { 0 };
		sprintf_s(memberLine, sizeof(memberLine), "CMem=%s\r", memberName);
		if(strlen(ret_Buf) + strlen(memberLine) >= 2048)
		{
			db->Close();
			return 100;
		}
		strcat_s(ret_Buf, 2048, memberLine);
	} while(db->NextRow());

	if(db->GetLastError()[0] != 0)
	{
		db->Close();
		return 100;
	}

	db->Close();
	return 1;
}

int ISAOCHECK::BreakUPClan(char *ClanZang_ID, char *gserver, char *ClanZang_ChaName, char *ClanName)
{
	(void)gserver;

	if(!ClanZang_ID || !ClanZang_ID[0] || !ClanZang_ChaName || !ClanZang_ChaName[0] ||
	   !ClanName || !ClanName[0])
		return 100;

	std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_ClanDB);
	if(!db || !db->Open())
		return 100;

	if(!db->BeginTransaction())
	{
		db->Close();
		return 100;
	}

	char leaderName[64] = { 0 };
	if(!db->Prepare("SELECT TOP 1 ClanZang FROM CL WHERE ClanName=?") ||
	   !db->BindInputParameter(ClanName, 1, PARAMTYPE_String))
	{
		db->RollbackTransaction();
		db->Close();
		return 100;
	}

	if(!db->Execute())
	{
		const bool queryFailed = db->GetLastError()[0] != 0;
		db->RollbackTransaction();
		db->Close();
		return queryFailed ? 100 : 0;
	}

	if(!db->GetData(leaderName, sizeof(leaderName), 1, PARAMTYPE_String))
	{
		db->RollbackTransaction();
		db->Close();
		return 100;
	}

	// clanRemove.asp rejected dissolution unless chname matched ClanZang.
	if(strcmp(leaderName, ClanZang_ChaName) != 0)
	{
		db->RollbackTransaction();
		db->Close();
		return 0;
	}

	// Preserve the historical deletion audit trail. The old ASP selected the
	// member-side MIDX by account id while copying the CL row.
	if(!db->Prepare(
		"INSERT INTO ClanMainDeleted "
		"(IDX,MIDX,ClanName,Userid,ClanZang,Flag,MemCnt,MIconCnt,RegiDate,LimitDate,DelDate,DelActive,PFlag,KFlag,DelState,DelCase,Server) "
		"SELECT A.IDX,B.MIDX,A.ClanName,A.UserID,A.ClanZang,A.Flag,A.MemCnt,A.MIconCnt,A.RegiDate,A.LimitDate,GETDATE(),A.DelActive,A.PFlag,A.KFlag,'1','1',1 "
		"FROM CL A JOIN UL B ON B.userid=? WHERE A.ClanName=?") ||
	   !db->BindInputParameter(ClanZang_ID, 1, PARAMTYPE_String) ||
	   !db->BindInputParameter(ClanName, 2, PARAMTYPE_String) ||
	   !db->Execute(FALSE))
	{
		db->RollbackTransaction();
		db->Close();
		return 100;
	}

	if(!db->Prepare("DELETE FROM CL WHERE ClanName=?") ||
	   !db->BindInputParameter(ClanName, 1, PARAMTYPE_String) ||
	   !db->Execute(FALSE))
	{
		db->RollbackTransaction();
		db->Close();
		return 100;
	}

	if(!db->Prepare(
		"INSERT INTO ClanSubDeleted "
		"(IDX,MIDX,userid,ChName,ChType,ChLv,ClanName,Permi,JoinDate,DelActive,PFlag,KFlag,DelDate,DelState,DelCase,Server) "
		"SELECT IDX,MIDX,userid,ChName,ChType,ChLv,ClanName,Permi,JoinDate,DelActive,PFlag,KFlag,GETDATE(),'1','1',1 "
		"FROM UL WHERE ClanName=?") ||
	   !db->BindInputParameter(ClanName, 1, PARAMTYPE_String) ||
	   !db->Execute(FALSE))
	{
		db->RollbackTransaction();
		db->Close();
		return 100;
	}

	if(!db->Prepare("DELETE FROM UL WHERE ClanName=?") ||
	   !db->BindInputParameter(ClanName, 1, PARAMTYPE_String) ||
	   !db->Execute(FALSE))
	{
		db->RollbackTransaction();
		db->Close();
		return 100;
	}

	if(!db->CommitTransaction())
	{
		db->RollbackTransaction();
		db->Close();
		return 100;
	}

	db->Close();
	return 1;
}

int ISAOCHECK::connect_WEBdb_NOTICE(char *szWriteBuffer, char *szReadBuffer)
{
	// R17: active notices use ServerDB__NoticeSystem and the legacy ReadNotice
	// queue has no production producer.  Keep the ABI but disable HTTP.
	(void)szWriteBuffer;
	if(szReadBuffer)
		szReadBuffer[0] = 0;
	return ISAO_ERR_TCP_CONNECT;
}

int ConvertURL(char *szURL, char *szServer, size_t szServerSize, char *szPath, size_t szPathSize)
{
	char *lpServer = nullptr;
	char *lpPath = nullptr;
	char szbuff[256] = { 0 };

	strcpy_s(szbuff, szURL);

	int len = lstrlen(szbuff);

	for (int cnt = 0; cnt < len; cnt++)
	{
		if (!lpServer && szbuff[cnt] == '/' && szbuff[cnt + 1] == '/')
		{
			lpServer = &szbuff[cnt + 2];
			cnt += 2;
		}

		if (!lpPath && szbuff[cnt] == '/' && szbuff[cnt + 1] != '/')
			lpPath = &szbuff[cnt];
	}

	if (lpPath)
	{
		if (szPath)
		{
			strcpy_s(szPath, szPathSize, lpPath);
		}

		lpPath[0] = 0;
	}
	else
		if (szPath)
			szPath[0] = 0;

	if (szServer)
	{
		if (lpServer)
		{
			strcpy_s(szServer, szServerSize, lpServer);
		}
		else
		{
			strcpy_s(szServer, szServerSize, szbuff);
		}
	}

	return TRUE;
}


int ISAOCHECK::ReadNotice(char *userid, char *chaname, char *gserver, char *buf)
{
	// R17: the active notice subsystem is News::msgNotice over SQLite.  This
	// legacy queue path has no production caller and must not reintroduce a
	// dynamic notice URL.
	(void)userid;
	(void)chaname;
	(void)gserver;
	if(buf)
		buf[0] = 0;
	return ISAO_ERR_NONE_REEJCT;
}


int ISAOCHECK::Banishiment(char *ClanZang_ID, char *gserver, char *ClanZang_ChaName, char *ClanName, char *ClanWon)
{
	if(!ClanZang_ID || !ClanZang_ID[0] ||
	   !gserver || !gserver[0] ||
	   !ClanZang_ChaName || !ClanZang_ChaName[0] ||
	   !ClanName || !ClanName[0] ||
	   !ClanWon || !ClanWon[0])
		return 100;

	const char *targetStart = ClanWon;
	if(strncmp(targetStart, "clwon1=", 7) == 0)
		targetStart += 7;

	const char *targetEnd = strchr(targetStart, '&');
	size_t targetLength = targetEnd ? static_cast<size_t>(targetEnd - targetStart) : strlen(targetStart);
	if(targetLength == 0)
		return 100;
	if(targetLength >= 64)
		targetLength = 63;

	char targetName[64] = { 0 };
	memcpy(targetName, targetStart, targetLength);
	targetName[targetLength] = 0;

	std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_ClanDB);
	if(!db || !db->Open())
		return 100;
	if(!db->BeginTransaction())
	{
		db->Close();
		return 100;
	}

	char memberClan[64] = { 0 };
	if(!db->Prepare("SELECT TOP 1 ClanName FROM UL WHERE ChName=?") ||
	   !db->BindInputParameter(targetName, 1, PARAMTYPE_String))
	{
		db->RollbackTransaction();
		db->Close();
		return 100;
	}

	if(!db->Execute())
	{
		const bool queryFailed = db->GetLastError()[0] != 0;
		db->RollbackTransaction();
		db->Close();
		return queryFailed ? 100 : 0;
	}

	if(!db->GetData(memberClan, sizeof(memberClan), 1, PARAMTYPE_String))
	{
		db->RollbackTransaction();
		db->Close();
		return 100;
	}

	if(!memberClan[0])
	{
		if(!db->Prepare("DELETE FROM UL WHERE ChName=?") ||
		   !db->BindInputParameter(targetName, 1, PARAMTYPE_String) ||
		   !db->Execute(FALSE) ||
		   !db->CommitTransaction())
		{
			db->RollbackTransaction();
			db->Close();
			return 100;
		}
		db->Close();
		return 0;
	}

	if(strcmp(memberClan, ClanName) != 0)
	{
		db->RollbackTransaction();
		db->Close();
		return 0;
	}

	char clanLeader[64] = { 0 };
	int memCnt = 0;
	if(!db->Prepare("SELECT TOP 1 ClanZang, MemCnt FROM CL WHERE ClanName=?") ||
	   !db->BindInputParameter(ClanName, 1, PARAMTYPE_String) ||
	   !db->Execute())
	{
		db->RollbackTransaction();
		db->Close();
		return 100;
	}

	if(!db->GetData(clanLeader, sizeof(clanLeader), 1, PARAMTYPE_String) ||
	   !db->GetData(&memCnt, sizeof(memCnt), 2, PARAMTYPE_Integer))
	{
		db->RollbackTransaction();
		db->Close();
		return 100;
	}

	if(strcmp(targetName, clanLeader) == 0)
	{
		db->RollbackTransaction();
		db->Close();
		return 4;
	}

	int newMemCnt = memCnt - 1;
	if(!db->Prepare("UPDATE CL SET MemCnt=? WHERE ClanName=?") ||
	   !db->BindInputParameter(&newMemCnt, 1, PARAMTYPE_Integer) ||
	   !db->BindInputParameter(ClanName, 2, PARAMTYPE_String) ||
	   !db->Execute(FALSE) ||
	   !db->Prepare("DELETE FROM UL WHERE ChName=?") ||
	   !db->BindInputParameter(targetName, 1, PARAMTYPE_String) ||
	   !db->Execute(FALSE))
	{
		db->RollbackTransaction();
		db->Close();
		return 100;
	}

	if(!db->CommitTransaction())
	{
		db->RollbackTransaction();
		db->Close();
		return 100;
	}

	db->Close();
	return 1;
}


int ISAOCHECK::AppointSubChip(char *gserver, char* clanwon)
{
	(void)gserver;
	if(!clanwon || !clanwon[0])
		return 100;

	std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_ClanDB);
	if(!db || !db->Open())
		return 100;
	if(!db->BeginTransaction())
	{
		db->Close();
		return 100;
	}

	char clanName[64] = { 0 };
	if(!db->Prepare("SELECT TOP 1 ClanName FROM UL WHERE ChName=?") ||
	   !db->BindInputParameter(clanwon, 1, PARAMTYPE_String))
	{
		db->RollbackTransaction();
		db->Close();
		return 100;
	}

	if(!db->Execute())
	{
		const bool queryFailed = db->GetLastError()[0] != 0;
		db->RollbackTransaction();
		db->Close();
		return queryFailed ? 100 : 0;
	}

	if(!db->GetData(clanName, sizeof(clanName), 1, PARAMTYPE_String))
	{
		db->RollbackTransaction();
		db->Close();
		return 100;
	}
	if(!clanName[0])
	{
		db->RollbackTransaction();
		db->Close();
		return 0;
	}

	// Sandurr ClanSubChipUpdate.asp first cleared Permi for every member of
	// the target's clan, then promoted the selected character to Permi=2.
	if(!db->Prepare("UPDATE UL SET Permi='0' WHERE ClanName=?") ||
	   !db->BindInputParameter(clanName, 1, PARAMTYPE_String) ||
	   !db->Execute(FALSE) ||
	   !db->Prepare("UPDATE UL SET Permi='2' WHERE ChName=? AND ClanName=?") ||
	   !db->BindInputParameter(clanwon, 1, PARAMTYPE_String) ||
	   !db->BindInputParameter(clanName, 2, PARAMTYPE_String) ||
	   !db->Execute(FALSE))
	{
		db->RollbackTransaction();
		db->Close();
		return 100;
	}

	if(!db->CommitTransaction())
	{
		db->RollbackTransaction();
		db->Close();
		return 100;
	}

	db->Close();
	return 1;
}

int ISAOCHECK::ReleaseSubChip(char *gserver, char* clanwon)
{
	(void)gserver;
	if(!clanwon || !clanwon[0])
		return 100;

	std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_ClanDB);
	if(!db || !db->Open())
		return 100;
	if(!db->BeginTransaction())
	{
		db->Close();
		return 100;
	}

	if(!db->Prepare("SELECT TOP 1 ClanName FROM UL WHERE ChName=?") ||
	   !db->BindInputParameter(clanwon, 1, PARAMTYPE_String))
	{
		db->RollbackTransaction();
		db->Close();
		return 100;
	}

	if(!db->Execute())
	{
		const bool queryFailed = db->GetLastError()[0] != 0;
		db->RollbackTransaction();
		db->Close();
		return queryFailed ? 100 : 0;
	}

	if(!db->Prepare("UPDATE UL SET Permi='0' WHERE ChName=?") ||
	   !db->BindInputParameter(clanwon, 1, PARAMTYPE_String) ||
	   !db->Execute(FALSE))
	{
		db->RollbackTransaction();
		db->Close();
		return 100;
	}

	if(!db->CommitTransaction())
	{
		db->RollbackTransaction();
		db->Close();
		return 100;
	}

	db->Close();
	return 1;
}

int ISAOCHECK::LeaveClan(char *clanname, char *chaname, char *gserver)
{
	int ret_chk;
	int iRejectCode = 111111;
	char szWriteBuffer[BUF_LEN];
	char szReadBuffer[BUF_LEN];

	sprintf(szWriteBuffer, "http://%s//ServerMain//clanChipChange.asp?chname=%s&gserver=%s&clName=%s&ticket=%d", hostName, chaname, gserver, clanname, nClanTicket);

	int cnt = 0;
	while (true)
	{
		ret_chk = connect_WEBdb(szWriteBuffer, szReadBuffer);

		if (ret_chk != ISAO_ERR_RECONNECT)
			break;

		if (cnt > 5)
			break;
		cnt++;
	}

	debug_fd2("LeaveClan() ������ ���� : %d ���� : %s ", strlen(szWriteBuffer), szWriteBuffer);
	debug_fd2("LeaveClan() ������ ���� : %d ���� : %s ", strlen(szReadBuffer), szReadBuffer);

	switch (ret_chk)
	{
	case ISAO_ERR_WINSOCK_NOERR:
		fd2("From server: ---from--/n%s--end--/n ", szReadBuffer);
		break;
	case ISAO_ERR_WINSOCK_START:
		fd2("WSAStartup()����");
		goto IsaoCheck_winsockERR;
	case ISAO_ERR_WINSOCK_VERSION:
		fd2("������ ���� ����");
		goto IsaoCheck_winsockERR;
	case ISAO_ERR_TCP_CONNECT:
		fd2("connect() ���� ����");
		goto IsaoCheck_winsockERR;
	case ISAO_ERR_SSL_READ:
		fd2(" recv() �б� ����");
		goto IsaoCheck_winsockERR;
	case ISAO_ERR_SSL_TIMEOUT:
		fd2("---------------- select() Ÿ�Ӿƿ� ����");
		goto IsaoCheck_winsockERR;
	}

	ret_chk = chk_readbuffer("Code=", szReadBuffer, &iRejectCode);
	switch (ret_chk)
	{

	case 1:
		break;
	case 2:
		break;
	case 3:
		break;
	case 4:
		break;
	case 5:
		break;
	case 6:
		break;
	case 100:
	case 101:
	case 102:
	case 103:
		goto IsaoCheck_Free;

	case ISAO_ERR_NONE_REEJCT:
		fd2("ISAO_ERR_NONE_REEJCT : �����ȣ�� ����");
		goto IsaoCheck_Free;
	}


IsaoCheck_Free:
	return ret_chk;


IsaoCheck_winsockERR:
	return ret_chk;
}

int ISAOCHECK::Secession(char* clanzang, char* gserver, char* SecessionID, char* clanname)
{
	if(!clanzang || !clanzang[0] ||
	   !gserver || !gserver[0] ||
	   !SecessionID || !SecessionID[0] ||
	   !clanname || !clanname[0])
		return 100;

	std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_ClanDB);
	if(!db || !db->Open())
		return 100;
	if(!db->BeginTransaction())
	{
		db->Close();
		return 100;
	}

	char memberClan[64] = { 0 };
	char permission[8] = { 0 };
	if(!db->Prepare("SELECT TOP 1 ClanName, Permi FROM UL WHERE ChName=?") ||
	   !db->BindInputParameter(SecessionID, 1, PARAMTYPE_String))
	{
		db->RollbackTransaction();
		db->Close();
		return 100;
	}

	if(!db->Execute())
	{
		const bool queryFailed = db->GetLastError()[0] != 0;
		db->RollbackTransaction();
		db->Close();
		return queryFailed ? 100 : 0;
	}

	if(!db->GetData(memberClan, sizeof(memberClan), 1, PARAMTYPE_String) ||
	   !db->GetData(permission, sizeof(permission), 2, PARAMTYPE_String))
	{
		db->RollbackTransaction();
		db->Close();
		return 100;
	}

	if(!memberClan[0])
	{
		if(!db->Prepare("DELETE FROM UL WHERE ChName=?") ||
		   !db->BindInputParameter(SecessionID, 1, PARAMTYPE_String) ||
		   !db->Execute(FALSE) ||
		   !db->CommitTransaction())
		{
			db->RollbackTransaction();
			db->Close();
			return 100;
		}
		db->Close();
		return 0;
	}

	if(strcmp(memberClan, clanname) != 0)
	{
		db->RollbackTransaction();
		db->Close();
		return 0;
	}

	char clanLeader[64] = { 0 };
	int memCnt = 0;
	if(!db->Prepare("SELECT TOP 1 ClanZang, MemCnt FROM CL WHERE ClanName=?") ||
	   !db->BindInputParameter(clanname, 1, PARAMTYPE_String) ||
	   !db->Execute())
	{
		db->RollbackTransaction();
		db->Close();
		return 100;
	}

	if(!db->GetData(clanLeader, sizeof(clanLeader), 1, PARAMTYPE_String) ||
	   !db->GetData(&memCnt, sizeof(memCnt), 2, PARAMTYPE_Integer))
	{
		db->RollbackTransaction();
		db->Close();
		return 100;
	}

	if(strcmp(SecessionID, clanLeader) == 0)
	{
		db->RollbackTransaction();
		db->Close();
		return 4;
	}

	int newMemCnt = memCnt - 1;
	if(!db->Prepare("UPDATE CL SET MemCnt=? WHERE ClanName=?") ||
	   !db->BindInputParameter(&newMemCnt, 1, PARAMTYPE_Integer) ||
	   !db->BindInputParameter(clanname, 2, PARAMTYPE_String) ||
	   !db->Execute(FALSE) ||
	   !db->Prepare("DELETE FROM UL WHERE ChName=?") ||
	   !db->BindInputParameter(SecessionID, 1, PARAMTYPE_String) ||
	   !db->Execute(FALSE))
	{
		db->RollbackTransaction();
		db->Close();
		return 100;
	}

	if(!db->CommitTransaction())
	{
		db->RollbackTransaction();
		db->Close();
		return 100;
	}

	db->Close();
	return 1;
}


int ISAOCHECK::isKPFlag(char *userid, char *clanname, char *chaname, char *gserver, char *pflag, char *gunbun)
{
	// The historical clan scripts used by this client treated isKflag as an
	// acknowledgement gate: successful service returned Code=1 and no
	// persistent database mutation was required. Preserve that observable
	// contract locally instead of keeping the legacy HTTP dependency.
	(void)userid;
	(void)clanname;
	(void)chaname;
	(void)gserver;
	(void)pflag;
	(void)gunbun;
	return 1;
}

int ISAOCHECK::isPFlag(char *userid, char *clanname, char *chaname, char *gserver, char *pflag, char *gunbun)
{
	// See isKPFlag(): the legacy endpoint's observable contract here is only
	// the success acknowledgement consumed by the auth/client state machine.
	(void)userid;
	(void)clanname;
	(void)chaname;
	(void)gserver;
	(void)pflag;
	(void)gunbun;
	return 1;
}

int ISAOCHECK::WarhouseRight(char *clname, char *right_str)
{
	// R17: no production caller queues this legacy warehouse-right endpoint in
	// the current server tree, and the historical persistence contract has not
	// been recovered.  Fail closed rather than inventing a ClanDB field or
	// contacting clan_WarehouseRight.php.
	(void)clname;
	(void)right_str;
	return ISAO_ERR_NONE_REEJCT;
}

int ISAOCHECK::updateFlag(char *clname)
{
	// R17: the server call graph has no producer for this dormant queue command,
	// and the updateFlag.php persistence contract remains unproven.  Fail closed
	// locally instead of performing a speculative write or HTTP request.
	(void)clname;
	return ISAO_ERR_NONE_REEJCT;
}

void clanDATA_save(char *fname, char *buff, int dataLen)
{
	HANDLE hFile;
	char allFname[64];
	sprintf_s(allFname, sizeof(allFname), "%s//%s//%s", SAVE_DIR, szConnServerName, fname);

	DWORD dwAcess = 0;
	hFile = CreateFile(allFname, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile != INVALID_HANDLE_VALUE)
	{

		int chk = WriteFile(hFile, buff, dataLen, &dwAcess, NULL);

		CloseHandle(hFile);
	}
}

void clanDATA_save2(char *fname, char *buff, int dataLen)
{
	HANDLE hFile;
	char allFname[64];
	sprintf_s(allFname, sizeof(allFname), "%s//%s", CLANINFO_DIR, fname);

	DWORD dwAcess = 0;
	hFile = CreateFile(allFname, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile != INVALID_HANDLE_VALUE)
	{

		int chk = WriteFile(hFile, buff, dataLen, &dwAcess, NULL);

		CloseHandle(hFile);
	}
}

extern char szWavUrl[255];
int ISAOCHECK::GetWavFile(char* FileName)
{
	// R17: the NPCSound downloader is compiled legacy but has no production
	// caller in the current server graph.  External NPCSound HTTP is disabled;
	// a future reactivation must use controlled local storage/protocol instead.
	(void)FileName;
	return ISAO_ERR_NONE_REEJCT;
}

static BOOL ReadLocalClanMarkBmp(const char *filename, char *buffer, DWORD maxBuffer, int *dataSize)
{
	if(!filename || !filename[0] || !buffer || !dataSize || maxBuffer < 2)
		return FALSE;

	// Clan mark identifiers are used as the basename by the legacy protocol.
	// Reject path syntax now that the asset is read from local server storage.
	for(const char *p = filename; *p; ++p)
	{
		if(*p == '\\' || *p == '/' || *p == ':' || *p == '.')
			return FALSE;
	}

	char path[MAX_PATH];
	if(sprintf_s(path, sizeof(path), "Database\\clan_marks\\%s.bmp", filename) < 0)
		return FALSE;

	HANDLE hFile = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
		return FALSE;

	DWORD highSize = 0;
	DWORD fileSize = GetFileSize(hFile, &highSize);
	if(fileSize == INVALID_FILE_SIZE || highSize != 0 || fileSize < 2 || fileSize > maxBuffer)
	{
		CloseHandle(hFile);
		return FALSE;
	}

	DWORD bytesRead = 0;
	BOOL ok = ReadFile(hFile, buffer, fileSize, &bytesRead, NULL);
	CloseHandle(hFile);
	if(!ok || bytesRead != fileSize)
		return FALSE;

	*dataSize = static_cast<int>(bytesRead);
	return buffer[0] == 'B' && buffer[1] == 'M';
}

int ISAOCHECK::read_ClanImage(char *clanname, char *filename)
{
	(void)clanname;
	int dataSize = 0;
	char szReadBuffer[65536] = { 0 };
	char szBMPName[255];
	ZeroMemory(szBMPName, sizeof(szBMPName));

	if(!ReadLocalClanMarkBmp(filename, szReadBuffer, sizeof(szReadBuffer), &dataSize))
		return 2;

	char server[4];
	char uniqueclannum[64];
	ZeroMemory(server, sizeof(server));
	ZeroMemory(uniqueclannum, sizeof(uniqueclannum));

	int nClanMarklen = lstrlen(filename);
	if(nClanMarklen == 9)
	{
		server[0] = filename[0];
		strcpy_s(uniqueclannum, sizeof(uniqueclannum), &filename[3]);
	}
	else
	{
		server[0] = filename[0];
		server[1] = filename[1];
		strcpy_s(uniqueclannum, sizeof(uniqueclannum), &filename[4]);
	}

	wsprintf(szBMPName, "%s_%s.%s", server, uniqueclannum, SAVE_EXE);
	clanDATA_save(szBMPName, szReadBuffer, dataSize);

	char path[128];
	char path2[128];
	int bmpSAVE(char *loadfname, char *savefname);
	wsprintf(path, "%s//%s//%s", SAVE_DIR, szConnServerName, szBMPName);
	wsprintf(path2, "%s//%s//%s_%s_16.%s", SAVE_DIR, szConnServerName, server, uniqueclannum, SAVE_EXE);
	bmpSAVE(path, path2);

	return 1;
}

int ISAOCHECK::read_ClanImage2(char *clanname, char *filename, CLANINFO *ci)
{
	(void)clanname;
	if(!ci)
		return 2;

	int dataSize = 0;
	char szReadBuffer[65536] = { 0 };
	char szBMPName[255];
	ZeroMemory(szBMPName, sizeof(szBMPName));

	if(!ReadLocalClanMarkBmp(filename, szReadBuffer, sizeof(szReadBuffer), &dataSize))
		return 2;

	wsprintf(szBMPName, "%s_%s.%s", ci->ClanInfoHeader.Gserver, ci->ClanInfoHeader.UniqueClaClanInfo_nNum, SAVE_EXE);
	clanDATA_save(szBMPName, szReadBuffer, dataSize);

	char path[128];
	char path1[128];
	int bmpSAVE(char *loadfname, char *savefname);
	wsprintf(path, "%s//%s//%s", SAVE_DIR, szConnServerName, szBMPName);
	wsprintf(path1, "%s//%s//%s_%s_16.%s", SAVE_DIR, szConnServerName, ci->ClanInfoHeader.Gserver, ci->ClanInfoHeader.UniqueClaClanInfo_nNum, SAVE_EXE);
	bmpSAVE(path, path1);

	return 1;
}

void ISAOCHECK::Convert_bin2HS(char *sorce, int len)
{
	int i, j, Len;
	for (i = 0; i < 30; i++)
	{
		if (fileDATA_HS[i]) delete fileDATA_HS[i];
	}

	HSsu = len / 2048;
	if (len % 2048) HSsu++;
	char byte0, byte1;


	for (j = 0; j < HSsu; j++)
	{
		if (j == (HSsu - 1))
		{
			fileDATA_HS[j] = new char[(len % 2048) * 2 + 1];
			Len = len % 2048;
		}
		else
		{
			fileDATA_HS[j] = new char[(2048 * 2) + 1];
			Len = 2048;
		}

		for (i = 0; i < Len; i++)
		{
			byte0 = *sorce & 0x0f;
			byte1 = (*sorce & 0xf0) >> 4;
			sorce++;

			if (byte0 < 10)
			{
				fileDATA_HS[j][i * 2 + 0] = byte0 + '0';
			}
			else
			{
				fileDATA_HS[j][i * 2 + 0] = (byte0 - 10) + 'A';
			}

			if (byte1 < 10)
			{
				fileDATA_HS[j][i * 2 + 1] = byte1 + '0';
			}
			else
			{
				fileDATA_HS[j][i * 2 + 1] = (byte1 - 10) + 'A';
			}
		}
		fileDATA_HS[j][i * 2] = 0;
	}
}



char *Convert_HS2bin(char *hs_buf, int len)
{
	char byte0, byte1;
	char* dest = new char[len];

	for (int i = 0; i < len; i++)
	{
		byte0 = hs_buf[i * 2 + 0];
		byte1 = hs_buf[i * 2 + 1];

		if (byte0 <= '9')
		{
			byte0 -= '0';
		}
		else
		{
			byte0 -= 'A';
			byte0 += 10;
		}

		if (byte1 <= '9')
		{
			byte1 -= '0';
		}
		else
		{
			byte1 -= 'A';
			byte1 += 10;
		}
		dest[i] = (byte1 << 4) | byte0;
	}
	return dest;
}



char *fileREAD(char *fileName, char *buf, int *len)
{
	DWORD dwAcess = 0;

	HANDLE hFile = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
		return 0;

	*len = GetFileSize(hFile, NULL);

	if (*len == 0)
	{
		CloseHandle(hFile);
		return 0;
	}

	char *newBuf = nullptr;

	if (!buf)
		newBuf = new char[*len];
	else
		newBuf = buf;

	ReadFile(hFile, newBuf, *len, &dwAcess, NULL);

	CloseHandle(hFile);

	return newBuf;
}

int ISAOCHECK::write_ClanImageFile(char *fname)
{
	// R17: clan marks now use Database\\clan_marks locally.  The historical web
	// upload routine has no active server caller, so never post mark bytes to
	// clan_insertClanImage.php if this dormant method is reached.
	(void)fname;
	return ISAO_ERR_NONE_REEJCT;
}


int ISAOCHECK::WEBdb_Exam()
{
	// R17: diagnostic ktj_login.php probe is dead legacy.  Keep it disabled and
	// side-effect free so a diagnostic call cannot reintroduce web dependency.
	return ISAO_ERR_NONE_REEJCT;
}


void WEBdb_Exam()
{
	iCheck.init_HostName_Port("74.122.192.107", 80);
	iCheck.init(1, 20);
	iCheck.WEBdb_Exam();
}


int ISAOCHECK::read_sod2(char *usid, char *charname, char *gserver, int index, char *szSod2Buff)
{
	(void)gserver;

	/*
	 * Sandurr Clan 2.0 / SoD 1.0 `sod2info.asp` compatibility.
	 *
	 * The historical page used ClanDB.CL/UL (not the SoD2 record table) for
	 * both the Karina main page (index 1) and the high-score clan list
	 * (index 3).  Keep the exact text protocol consumed by cSINSOD2:
	 * index 1 uses pipe separators; index 3 uses CR separators.
	 */
	if(!szSod2Buff)
		return 100;

	szSod2Buff[0] = 0;
	if(!usid || !usid[0] || !charname || !charname[0])
	{
		strcpy_s(szSod2Buff, 65536, "Code=100");
		return 0;
	}

	if(index != 1 && index != 3)
	{
		strcpy_s(szSod2Buff, 65536, "Code=104");
		return 0;
	}

	std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_ClanDB);
	if(!db || !db->Open())
		return 100;

	if(index == 1)
	{
		int clanPoint = 0;
		int clanImage = 0;
		long long clanMoney = 0;
		char clanNote[1024] = { 0 };
		char clanName[128] = { 0 };
		char clanLeader[128] = { 0 };

		if(!db->Prepare(
			"SELECT TOP 1 Cpoint,Note,ClanName,MIconCnt,ClanZang,ClanMoney "
			"FROM CL ORDER BY Cpoint DESC"))
		{
			db->Close();
			return 100;
		}

		if(!db->Execute())
		{
			const bool queryFailed = db->GetLastError()[0] != 0;
			if(!queryFailed)
				strcpy_s(szSod2Buff, 65536, "Code=0");
			db->Close();
			return queryFailed ? 100 : 0;
		}

		if(!db->GetData(&clanPoint, sizeof(clanPoint), 1, PARAMTYPE_Integer) ||
		   !db->GetData(clanNote, sizeof(clanNote), 2, PARAMTYPE_String) ||
		   !db->GetData(clanName, sizeof(clanName), 3, PARAMTYPE_String) ||
		   !db->GetData(&clanImage, sizeof(clanImage), 4, PARAMTYPE_Integer) ||
		   !db->GetData(clanLeader, sizeof(clanLeader), 5, PARAMTYPE_String) ||
		   !db->GetData(&clanMoney, sizeof(clanMoney), 6, PARAMTYPE_Int64))
		{
			db->Close();
			return 100;
		}
		(void)clanPoint;

		char memberClan[128] = { 0 };
		char memberPermi[32] = { 0 };
		if(!db->Prepare("SELECT TOP 1 ClanName,Permi FROM UL WHERE ChName=?") ||
		   !db->BindInputParameter(charname, 1, PARAMTYPE_String))
		{
			db->Close();
			return 100;
		}

		if(!db->Execute())
		{
			const bool queryFailed = db->GetLastError()[0] != 0;
			if(!queryFailed)
			{
				sprintf_s(szSod2Buff, 65536,
					"Code=0|CClanMoney=0|CTax=0|CName=%s|CNote=%s|CZang=%s|CIMG=%d|",
					clanName, clanNote, clanLeader, clanImage);
			}
			db->Close();
			return queryFailed ? 100 : 0;
		}

		if(!db->GetData(memberClan, sizeof(memberClan), 1, PARAMTYPE_String) ||
		   !db->GetData(memberPermi, sizeof(memberPermi), 2, PARAMTYPE_String))
		{
			db->Close();
			return 100;
		}

		int code = 6;
		bool includeTotals = false;
		long long responseClanMoney = 0;

		if(lstrcmpi(clanName, memberClan) == 0)
		{
			includeTotals = true;
			if(lstrcmpi(clanLeader, charname) == 0)
			{
				code = 1;
				responseClanMoney = clanMoney;
			}
			else if(strcmp(memberPermi, "2") == 0)
			{
				code = 2;
			}
			else
			{
				code = 3;
			}
		}
		else
		{
			char memberClanLeader[128] = { 0 };
			if(!db->Prepare("SELECT TOP 1 ClanZang FROM CL WHERE ClanName=?") ||
			   !db->BindInputParameter(memberClan, 1, PARAMTYPE_String))
			{
				db->Close();
				return 100;
			}

			if(db->Execute())
			{
				if(!db->GetData(memberClanLeader, sizeof(memberClanLeader), 1, PARAMTYPE_String))
				{
					db->Close();
					return 100;
				}
			}
			else if(db->GetLastError()[0] != 0)
			{
				db->Close();
				return 100;
			}

			if(memberClanLeader[0] && lstrcmpi(memberClanLeader, charname) == 0)
				code = 4;
			else if(strcmp(memberPermi, "2") == 0)
				code = 5;
			else
				code = 6;
		}

		if(includeTotals)
		{
			sprintf_s(szSod2Buff, 65536,
				"Code=%d|CClanMoney=%lld|CTax=0|CName=%s|CNote=%s|CZang=%s|CIMG=%d|"
				"TotalEMoney=%lld|TotalMoney=%lld|",
				code, responseClanMoney, clanName, clanNote, clanLeader, clanImage,
				clanMoney, clanMoney);
		}
		else
		{
			sprintf_s(szSod2Buff, 65536,
				"Code=%d|CClanMoney=0|CTax=0|CName=%s|CNote=%s|CZang=%s|CIMG=%d|",
				code, clanName, clanNote, clanLeader, clanImage);
		}

		db->Close();
		return 0;
	}

	/* Index 3: the historical ASP examined at most the first nine CL rows. */
	if(!db->Prepare(
		"SELECT TOP 9 MIconCnt,ClanName,Cpoint,RegiDate FROM CL ORDER BY Cpoint DESC"))
	{
		db->Close();
		return 100;
	}

	if(!db->Execute())
	{
		const bool queryFailed = db->GetLastError()[0] != 0;
		if(!queryFailed)
			strcpy_s(szSod2Buff, 65536, "Code=0");
		db->Close();
		return queryFailed ? 100 : 0;
	}

	strcpy_s(szSod2Buff, 65536, "Code=1\r");
	do
	{
		int clanImage = 0;
		int clanPoint = 0;
		char clanName[128] = { 0 };
		char regiDate[128] = { 0 };
		if(!db->GetData(&clanImage, sizeof(clanImage), 1, PARAMTYPE_Integer) ||
		   !db->GetData(clanName, sizeof(clanName), 2, PARAMTYPE_String) ||
		   !db->GetData(&clanPoint, sizeof(clanPoint), 3, PARAMTYPE_Integer) ||
		   !db->GetData(regiDate, sizeof(regiDate), 4, PARAMTYPE_String))
		{
			db->Close();
			return 100;
		}

		/* The ASP's InStr(strReturn, ClanName) duplicate check was substring-
		 * based. Preserve it literally rather than silently tightening it. */
		if(clanPoint > 0 && clanName[0] && strstr(szSod2Buff, clanName) == NULL)
		{
			char *dateSeparator = strchr(regiDate, ' ');
			if(dateSeparator)
				*dateSeparator = 0;

			char row[512] = { 0 };
			sprintf_s(row, sizeof(row),
				"CIMG=%d\rCName=%s\rCPoint=%d\rCRegistDay=%s\r",
				clanImage, clanName, clanPoint, regiDate);
			if(strlen(szSod2Buff) + strlen(row) >= 65536)
			{
				db->Close();
				return 100;
			}
			strcat_s(szSod2Buff, 65536, row);
		}
	} while(db->NextRow());

	if(db->GetLastError()[0] != 0)
	{
		db->Close();
		return 100;
	}

	db->Close();
	return 0;
}


bool UTF8ToWeb(char *szUTF8, char *szWeb)
{
	int iCount = 0;
	char chHigh;
	char chLow;

	for (int i = 0; szUTF8[i] != 0; i++)
	{
		chHigh = chLow = 0;



		szWeb[iCount] = '%';
		iCount++;


		chHigh = (szUTF8[i] & 0xf0) >> 4;

		if ((0 <= chHigh) && (chHigh <= 9))
		{
			szWeb[iCount] = '0' + chHigh;
			iCount++;
		}
		else
		{
			szWeb[iCount] = 'a' + chHigh - 0x0a;
			iCount++;
		}


		chLow = szUTF8[i] & 0x0f;

		if ((0 <= chLow) && (chLow <= 9))
		{
			szWeb[iCount] = '0' + chLow;
			iCount++;
		}
		else
		{
			szWeb[iCount] = 'a' + chLow - 0x0a;
			iCount++;
		}

	}
	szWeb[iCount] = 0;

	return true;
}

bool MBCSToUTF8(char *szMBCS, char *szUTF8, int iMBCSCodePage)
{
	wchar_t wUniCode[1024]; memset(wUniCode, 0, sizeof(wUniCode));


	MultiByteToWideCharByCodePage(szMBCS, wUniCode, iMBCSCodePage);


	WideCharToMultiByteByCodePage(wUniCode, szUTF8, CP_UTF8);

	return true;
}

bool MultiByteToWideCharByCodePage(char *szMBCS, wchar_t *szUnicode, int iCodePage)
{
	int iLen = MultiByteToWideChar(iCodePage, 0, szMBCS, lstrlen(szMBCS), NULL, NULL);
	MultiByteToWideChar(iCodePage, 0, szMBCS, lstrlen(szMBCS), szUnicode, iLen);

	return true;
}



bool WideCharToMultiByteByCodePage(wchar_t *szUnicode, char *szMBCS, int iCodePage)
{
	int iLen = WideCharToMultiByte(iCodePage, 0, szUnicode, wcslen(szUnicode), NULL, NULL, NULL, NULL);
	WideCharToMultiByte(iCodePage, 0, szUnicode, wcslen(szUnicode), szMBCS, iLen, NULL, NULL);

	return true;
}

bool MBCSToWeb(char *szMBCS, char *szWeb, int iMBCSCodePage)
{
	char szUTF8[1024]; memset(szUTF8, 0, sizeof(szUTF8));
	MBCSToUTF8(szMBCS, szUTF8, iMBCSCodePage);
	UTF8ToWeb(szUTF8, szWeb);
	return true;
}



