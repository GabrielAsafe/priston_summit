#include <objbase.h>\n#include <cstring>
#include <windows.h>
#include <io.h>
#include <stdio.h>         
#include <string.h>        
#include <memory.h>
#include <process.h>       

#include "platform/windows/resources/legacy_runtime/resource.h"
#include "systems/network/transport/legacy/smwsock.h"

#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "core/data/legacy_loader/fileread.h"
#include "core/server/gameplay_runtime/playmain.h"
#include "systems/world/field/field.h"
#include "core/server/legacy_sin_runtime/sinLinkHeader.h"
#include "systems/audio/effects/legacy/effectsnd.h"

#include "systems/persistence/character_record/record.h"
#include "core/gameplay/legacy_support/playsub.h"

#include "core/server/legacy_runtime/DllServer.h"
#include "systems/network/protocol/server/opcodes.h"
#include "core/server/legacy_runtime/onserver.h"
#include "systems/authentication/protocol/Login.h"
#include "systems/persistence/database/core/Database.h"
#include "gameplay/classes/v2/services/ClassService.h"
#include "systems/network/v2/legacy/LegacyPacketSender.h"
#include "systems/commerce/v2/legacy/LegacyWalletAdapter.h"


namespace
{
bool SummitDllServerSend(rsPLAYINFO* recipient, const void* data, int size)
{
	if(!recipient || !data || size <= 0)
		return false;
	const gameplay::PlayerId playerId =
		gameplay::player::legacy::ServerLegacyOnlinePlayerRegistry().IdOf(recipient);
	return gameplay::network::legacy::ServerLegacyPacketSender().Send(
		playerId, gameplay::network::PacketView(data, static_cast<std::size_t>(size), 0u)).Succeeded();
}
}

#define		BILLING_METER_RATE

#define	SWM_DISPDEBUG	(WM_USER+77)


char szServerName[64] = "Unnamed Server";


CRITICAL_SECTION cSqlSection;

#ifdef _W_SERVER


struct sLOG_SQLDATA
{
	DWORD	dwRecCode;
	char	szUserID[ACCOUNT_SIZE];
	char	szCharName[NAME_SIZE];
	char	szRace[16];
	char	szOccupation[16];
	int 	Occupation;
	DWORD	dwExpPoint;
	INT64	Exp64;
	DWORD	Level;
	DWORD	dwMoney;
	char	szIP[16];
	char	szGuid[128];

	char	szGPCode[16];
	time_t	DisuseDay;
	int		UsePeriod;
	int		PC_RNo;

	int		Time;


	rsPLAYINFO	*lpPlayInfo;
	DWORD	dwConnectCode;

	sLOG_SQLDATA()
	{
		dwRecCode = 0;
		ZeroMemory(szUserID, sizeof(szUserID));
		ZeroMemory(szCharName, sizeof(szCharName));
		ZeroMemory(szRace, sizeof(szRace));
		ZeroMemory(szOccupation, sizeof(szOccupation));
		Occupation = 0;
		dwExpPoint = 0;
		Exp64 = 0;
		Level = 0;
		dwMoney = 0;
		ZeroMemory(szIP, sizeof(szIP));
		ZeroMemory(szGuid, sizeof(szGuid));
		ZeroMemory(szGPCode, sizeof(szGPCode));
		DisuseDay = 0;
		UsePeriod = 0;
		PC_RNo = 0;
		Time = 0;
		lpPlayInfo = nullptr;
		dwConnectCode = 0;
	}


};

sLOG_SQLDATA *LogSqlData = 0;
int			LogSqlDataCount;
int			LogSqlSendCount;
CRITICAL_SECTION	cLogSqlSection;
HANDLE			hLogSqlThread = 0;
DWORD			dwLogSqlThreadId;
int			LogSqlLossCount = 0;

DWORD		LogSqlProcessTime = 0;
DWORD		LogSqlProcessTimeMax = 0;
int			LogSqlProcessCount = 0;
int			LogSqlProcessQueMax = 0;

int			UseDLL_Code = 0;




#define		LOG_SQL_BUFF_MAX		2048
#define		LOG_SQL_BUFF_MASK		2047
#define		LOG_SQL_BUFF_LIMIT		1023




LogITEM_QUE *LogItemQue = 0;
int			LogItemQuePush;
int			LogItemQuePop;
int			LogItemLossCount = 0;
DWORD		LogItemQueProcessTime = 0;


struct	sLOG_EVENT
{
	char	szID[ACCOUNT_SIZE];
	char	szName[NAME_SIZE];
	char	szGPCode[16];
	char	szIP[16];
	int		Param[32];
};

struct	sCLAN_EVENT
{
	char	szID[ACCOUNT_SIZE];
	char	szName[NAME_SIZE];
	char	szIP[16];
	int		TicketNum;
	int		Param[8];
};

struct POLL
{
	int		size;
	char	UserID[ACCOUNT_SIZE];
	char	CharID[NAME_SIZE];
	int		PollCode;
	int		PollCount;
	int		PollItem[10];
};




LogEVENT_QUE *LogEventQue = 0;
int			LogEventQuePush;
int			LogEventQuePop;
int			LogEventLossCount = 0;
DWORD		LogEventQueProcessTime = 0;















#ifdef	BILLING_METER_RATE		
typedef int(*LPFN_SQLLogon)(char *lpszPID, char *lpszPWD, char *lpszIP, char *lpszGPCode, char *lpszDisuseDay, char *lpszUsePeriod, char *s_guid, char *lpszPCRNo, char *lpszGubunCode);
#else
typedef int(*LPFN_SQLLogon)(char *lpszPID, char *lpszPWD, char *lpszIP, char *lpszGPCode, char *lpszDisuseDay, char *lpszUsePeriod, char *s_guid, char *lpszPCRNo);
#endif

typedef int(*LPFN_EventSQLLogon)(char *lpszPID, char *lpszPWD, char *lpszIP, char *lpszGPCode, char *lpszDisuseDay, char *lpszUsePeriod, char *lpszS_guid, char *lpszPCRNo, char *lpszGubunCode);
typedef int(*LPFN_SQLLogout)(char *lpszPID, char *lpszCID, char *lpszIP, char *lpszGPCode, time_t lpszDisuseDay, int lpszUsePeriod, char *s_guid, int PCRNo);
typedef int(*LPFN_EventSQLLogout)(char *lpszPID, char *lpszCID, char *lpszIP, char *lpszGPCode, time_t lpszDisuseDay, long lpszUsePeriod, char *s_guid, long PCRNo, char *lpszNewGPCode, char *News_guid);

typedef int(*LPFN_UserUseTime)(char *lpszPID, char *lpszCID, char *lpszIP, char *ServerName, char *lpszGPCode, int lpszUseTime, char* pFlag);
typedef int(*LPFN_PCUseTime)(char *lpszPID, char *lpszCID, char *ServerName, char *lpszGPCode, int lpszUseTime, char *lpszIP, char* pFlag);
typedef int(*LPFN_SetGameUser)(char *lpszPID, char *lpszCID, char *ServerName, char* pFlag);
typedef int(*LPFN_GetGPCode)(char *GPCode, char *PG);
typedef int(*LPFN_PCUse)(char *lpszPID, char *lpszGPCode, char *ServerName, char *lpszIP, int lpszRNo, char *s_guid);
typedef int(*LPFN_PCSearchReserv)(char *lpszuserid, int lpszRNo, char *lpszGPCode, char *IP, char *NewRNo, char *NewGPCode, char *NewDisuseDay, char *NewUsePeriod, char *s_guid);
typedef int(*LPFN_ChangeUseridGPCodeGame)(int PCRNo, char *lpszUserID, char *lpszNewGPCOde, char *lpszNewDisUseDay, char *lpszNewUsePeriod, char *lpszNewGUID);


typedef long(*LPFN_PCRNoReservProduct)(int lpszRNo, char *lpszGPCode);


typedef int(*LPFN_CharLog)(char *lpszPID, int lpszCharType, char *lpszLID, char *lpszGPCode, char *lpszIP, char *lpszServerName, int lpszGameLevel, int lpszExp, int lpszGameMoney, int nFlag);
typedef int(*LPFN_GameLog)(char *lpszPID, int lpszCharType, char *lpszCID, char *lpszGPCode, char *lpszIP, char *m_guid, int m_nServerPacket, int m_nClientPacket, int nFlag);
typedef int(*LPFN_CharRecordLog)(char *lpszPID, int lpszCharType, char *lpszLID, char *lpszGPCode, char *lpszIP, char *lpszServerName, int lpszGameLevel, __int64 lpszExp, int lpszGameMoney, int nFlag);

typedef int(*LPFN_SetAllGameUser)();
typedef int(*LPFN_ServerDown)();
typedef int(*LPFN_ServerExit)();

typedef int(*LPFN_LogITEMRecord)(char *lpszPID, LogITEM *LogItem);
typedef int(*LPFN_LogITEMTradeRecord)(char *lpszPID, LogITEM_TRADE *LogItem);
typedef int(*LPFN_LogITEMPostRecord)(char *lpszPID, LogGiftITEM *GiftItem);

typedef int(*LPFN_IPRecord)(char *lpszServerName, JUNGIP *JungIP);
typedef int(*LPFN_IPRecordTime)(char *lpszServerName, JUNGIP *JungIP);

typedef int(*LPFN_WithSODRecordRanks)(char *lpszUserID, char *lpszCharName, int CharType, int point, int KillCount, int GLevel, int TotalPoint, int TotalUser, int SuccessUser, int PCRNo, char *lpszGPCode, char *lpszIP, long servertime);
typedef int(*LPFN_UserStar)(char *lpszPID, int lpszStar);
typedef int(*LPFN_UserStarRecord)(char *lpszPID, int lpszStar, char *GameLogonID, int CharType, int GLevel, int Gubun);
typedef int(*LPFN_Contribution)(char *lpszUserID, char *lpszCharName, int lpszMoney);
typedef int(*LPFN_ClanMoney)(char *lpszPID, char *GameLogonID, int lpszClanMoney, int Gubun);
typedef int(*LPFN_EventJoin)(char *lpszUserID, char *lpszCharName);

typedef __int64(*LPFN_SiegeMoney)(char *lpszPID, char *GameLogonID, __int64 lpszSiegeMoney, int Gubun);
typedef int(*LPFN_SiegeMoneyTax)(char *lpszClanMarkNum, __int64 lpszMx, __int64 lpszAg, __int64 lpszSh, __int64 lpszPo1, __int64 lpszPo2, __int64 lpszPo3, __int64 lpszFo, __int64 lpszWp, __int64 lpszSk, __int64 lpszTt, __int64 lpszTax);
typedef	int(*LPFN_PollRecord)(char *lpszPID, time_t PollTime, struct POLL *PollItem, int Gubun);


typedef int(*LPFN_GetTicket)();
typedef int(*LPFN_ClanUserTicket)(char *lpszuserid, char *lpszChName, char *lpszIP, int TicketNum, int gubun);
typedef int(*LPFN_ClanUserTicKets)(char *lpszuserid, char *lpszChName, char *lpszIP, int TicketNum, int gubun, int CharType, int CharLevel);
typedef int(*LPFN_ClanPointInsert)(char *lpszuserid, char *lpszChName, char * lpszIP, int TicketNum, int Cpoint);
typedef int(*LPFN_ClanUserMarkNum)(char *lpszuserid, char *lpszChName);


class	BILL_SQL
{

public:
	HINSTANCE hLogSqlDLL;
	HINSTANCE hClanDLL;

	LPFN_SQLLogon			SQLLogon;
	LPFN_SQLLogout			SQLLogout;
	LPFN_EventSQLLogout		EventSQLLogout;
	LPFN_EventSQLLogon		EventSQLLogon;
	LPFN_UserUseTime		UserUseTime;
	LPFN_PCUseTime			PCUseTime;
	LPFN_SetGameUser		SetGameUser;
	LPFN_GetGPCode			GetGPCode;
	LPFN_PCUse				PCUse;
	LPFN_PCSearchReserv		PCSearchReserv;
	LPFN_ChangeUseridGPCodeGame	ChangeUseridGPCodeGame;
	LPFN_PCRNoReservProduct	PCRNoReservProduct;


	LPFN_CharLog			CharLog;
	LPFN_GameLog			GameLog;
	LPFN_CharRecordLog		CharRecordLog;

	LPFN_LogITEMRecord		LogITEMRecord;
	LPFN_LogITEMTradeRecord	LogITEMTradeRecord;
	LPFN_LogITEMPostRecord	LogITEMPostRecord;

	LPFN_IPRecord			IPRecord;
	LPFN_IPRecordTime		IPRecordTime;

	LPFN_SetAllGameUser		SetAllGameUser;
	LPFN_WithSODRecordRanks	WithSODRecordRanks;
	LPFN_UserStar			UserStar;
	LPFN_UserStarRecord		UserStarRecord;
	LPFN_Contribution		Contribution;
	LPFN_ClanMoney			ClanMoney;
	LPFN_EventJoin			EventJoin;

	LPFN_SiegeMoney			SiegeMoney;
	LPFN_SiegeMoneyTax		SiegeMoneyTax;
	LPFN_PollRecord			PollRecord;

	LPFN_ServerDown			ServerDown;
	LPFN_ServerExit			ServerExit;


	LPFN_GetTicket			GetTicket;
	LPFN_ClanUserTicket		ClanUserTicket;
	LPFN_ClanUserTicKets	ClanUserTicKets;
	LPFN_ClanPointInsert	ClanPointInsert;
	LPFN_ClanUserMarkNum	ClanUserMarkNum;



	BILL_SQL();
	~BILL_SQL();
	int Init(BOOL loadLegacyDatabase);

};

BILL_SQL::BILL_SQL()
{
	ZeroMemory(this, sizeof(BILL_SQL));
}
BILL_SQL::~BILL_SQL()
{

}

int BILL_SQL::Init(BOOL loadLegacyDatabase)
{

	hClanDLL = LoadLibrary("Clan");
	if(hClanDLL)
	{
		GetTicket = (LPFN_GetTicket)GetProcAddress(hClanDLL, "GetTicket");
		ClanUserTicket = (LPFN_ClanUserTicket)GetProcAddress(hClanDLL, "ClanUserTicKet");
		ClanUserTicKets = (LPFN_ClanUserTicKets)GetProcAddress(hClanDLL, "ClanUserTicKets");
		ClanPointInsert = (LPFN_ClanPointInsert)GetProcAddress(hClanDLL, "ClanPointInsert");
		ClanUserMarkNum = (LPFN_ClanUserMarkNum)GetProcAddress(hClanDLL, "ClanUserMarkNum");
	}

	/* R23: local-only deployments can explicitly disable the historical
	 * billing provider. BILL_SQL is zero-initialized, so leaving this module
	 * unloaded also leaves every provider function pointer safely null. */
	if(!loadLegacyDatabase)
		return UseDLL_Code;

	hLogSqlDLL = LoadLibrary("Database");

	if(hLogSqlDLL != NULL)
	{
		CharLog = (LPFN_CharLog)GetProcAddress(hLogSqlDLL, "CharLog");
		GameLog = (LPFN_GameLog)GetProcAddress(hLogSqlDLL, "GameLog");
		CharRecordLog = (LPFN_CharRecordLog)GetProcAddress(hLogSqlDLL, "CharRecordLog");

		SQLLogon = (LPFN_SQLLogon)GetProcAddress(hLogSqlDLL, "SQLLogon");
		SQLLogout = (LPFN_SQLLogout)GetProcAddress(hLogSqlDLL, "SQLLogout");
		EventSQLLogon = (LPFN_EventSQLLogon)GetProcAddress(hLogSqlDLL, "EventSQLLogon");
		EventSQLLogout = (LPFN_EventSQLLogout)GetProcAddress(hLogSqlDLL, "EventSQLLogout");

		UserUseTime = (LPFN_UserUseTime)GetProcAddress(hLogSqlDLL, "UserUseTime");
		PCUseTime = (LPFN_PCUseTime)GetProcAddress(hLogSqlDLL, "PCUseTime");
		SetGameUser = (LPFN_SetGameUser)GetProcAddress(hLogSqlDLL, "SetGameUser");
		GetGPCode = (LPFN_GetGPCode)GetProcAddress(hLogSqlDLL, "GetGPCode");
		PCUse = (LPFN_PCUse)GetProcAddress(hLogSqlDLL, "PCUse");
		PCSearchReserv = (LPFN_PCSearchReserv)GetProcAddress(hLogSqlDLL, "PCSearchReserv");
		ChangeUseridGPCodeGame = (LPFN_ChangeUseridGPCodeGame)GetProcAddress(hLogSqlDLL, "ChangeUseridGPCodeGame");

		PCRNoReservProduct = (LPFN_PCRNoReservProduct)GetProcAddress(hLogSqlDLL, "PCRNoReservProduct");


		SetAllGameUser = (LPFN_SetAllGameUser)GetProcAddress(hLogSqlDLL, "SetAllGameUser");
		ServerDown = (LPFN_ServerDown)GetProcAddress(hLogSqlDLL, "ServerDown");
		ServerExit = (LPFN_ServerExit)GetProcAddress(hLogSqlDLL, "ServerExit");

		LogITEMRecord = (LPFN_LogITEMRecord)GetProcAddress(hLogSqlDLL, "LogITEMRecord");
		LogITEMTradeRecord = (LPFN_LogITEMTradeRecord)GetProcAddress(hLogSqlDLL, "LogITEMTradeRecord");
		LogITEMPostRecord = (LPFN_LogITEMPostRecord)GetProcAddress(hLogSqlDLL, "LogITEMPostRecord");

		IPRecord = (LPFN_IPRecord)GetProcAddress(hLogSqlDLL, "IPRecord");
		IPRecordTime = (LPFN_IPRecord)GetProcAddress(hLogSqlDLL, "IPRecordTime");

		WithSODRecordRanks = (LPFN_WithSODRecordRanks)GetProcAddress(hLogSqlDLL, "WithSOD2RecordRanks");
		UserStar = (LPFN_UserStar)GetProcAddress(hLogSqlDLL, "UserStar");
		UserStarRecord = (LPFN_UserStarRecord)GetProcAddress(hLogSqlDLL, "UserStarRecord");
		Contribution = (LPFN_Contribution)GetProcAddress(hLogSqlDLL, "Contribution");
		ClanMoney = (LPFN_ClanMoney)GetProcAddress(hLogSqlDLL, "ClanMoney");
		EventJoin = (LPFN_EventJoin)GetProcAddress(hLogSqlDLL, "EventJoin");

		SiegeMoney = (LPFN_SiegeMoney)GetProcAddress(hLogSqlDLL, "SiegeMoney");
		SiegeMoneyTax = (LPFN_SiegeMoneyTax)GetProcAddress(hLogSqlDLL, "SiegeMoneyTax");
		PollRecord = (LPFN_PollRecord)GetProcAddress(hLogSqlDLL, "PollRecord");


		UseDLL_Code = LOAD_DLL_SQL;

	}

	return UseDLL_Code;
}



BILL_SQL BillSql;

#include <iostream>

using namespace std;


/* R18: Database.dll historically implemented ClanMoney/SiegeMoney even
 * though these are gameplay persistence, not commercial billing.  The SQL
 * used by the supplied DLL is visible in its embedded query strings and maps
 * directly to ClanDB.CL + the two audit tables recovered from ClanDB.bak.
 * Keep the public/event interfaces unchanged, but prefer SQLite locally.
 * A FALSE return here means infrastructure/SQL failure and allows the
 * historical DLL to remain a temporary fallback.  A handled gameplay result
 * (including -1111 for an invalid clan operation) returns TRUE. */
static BOOL TrySQLiteLocalClanFund(
    const char *account,
    const char *character,
    INT64 amount,
    int operation,
    BOOL siegeMoney,
    INT64 *outResult)
{
    if(!outResult)
        return FALSE;

    *outResult = 0;

    if(!account || !account[0] || !character || !character[0])
        return TRUE;

    std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_ClanDB);
    if(!db || !db->Open())
        return FALSE;

    if(!db->BeginTransaction())
    {
        db->Close();
        return FALSE;
    }

    const char *moneyColumn = siegeMoney ? "SiegeMoney" : "ClanMoney";
    char query[512] = { 0 };
    sprintf_s(
        query,
        sizeof(query),
        "SELECT TOP 1 IDX, %s FROM CL WHERE UserID=? AND ClanZang=?",
        moneyColumn);

    char mutableAccount[ACCOUNT_SIZE] = { 0 };
    char mutableCharacter[NAME_SIZE] = { 0 };
    strcpy_s(mutableAccount, account);
    strcpy_s(mutableCharacter, character);

    if(!db->Prepare(query) ||
       !db->BindInputParameter(mutableAccount, 1, PARAMTYPE_String) ||
       !db->BindInputParameter(mutableCharacter, 2, PARAMTYPE_String))
    {
        db->RollbackTransaction();
        db->Close();
        return FALSE;
    }

    int clanIdx = 0;
    INT64 currentMoney = 0;
    if(db->Execute())
    {
        if(!db->GetData(&clanIdx, sizeof(clanIdx), 1, PARAMTYPE_Integer) ||
           !db->GetData(&currentMoney, sizeof(currentMoney), 2, PARAMTYPE_Int64))
        {
            db->RollbackTransaction();
            db->Close();
            return FALSE;
        }
    }
    else if(db->GetLastError()[0] != 0)
    {
        db->RollbackTransaction();
        db->Close();
        return FALSE;
    }

    /* Exact Database.dll contract recovered from its embedded SQL + x86 flow:
     *   amount == 0: return the balance from the leader-qualified SELECT.
     *   Gubun 1: subtract by UserID and log OperCode 01 / negative amount.
     *   Gubun 2: add by UserID and log OperCode 02 / positive amount.
     * The DLL does not check for insufficient balance here.  Preserve that
     * legacy behavior during the persistence migration; do not fold gameplay
     * fixes into this step.
     */
    if(amount == 0 || operation == 0)
    {
        *outResult = currentMoney;
        db->RollbackTransaction();
        db->Close();
        return TRUE;
    }

    if(amount < 0 || (operation != 1 && operation != 2))
    {
        db->RollbackTransaction();
        db->Close();
        return TRUE;
    }

    const char op = operation == 1 ? '-' : '+';
    if(siegeMoney || operation == 2)
    {
        sprintf_s(
            query,
            sizeof(query),
            "UPDATE CL SET %s=%s%c? WHERE UserID=? "
            "AND (DelActive IS NULL OR CAST(DelActive AS INTEGER)=0)",
            moneyColumn,
            moneyColumn,
            op);
    }
    else
    {
        /* Historical ClanMoney withdrawal query had no DelActive predicate. */
        sprintf_s(
            query,
            sizeof(query),
            "UPDATE CL SET %s=%s-? WHERE UserID=?",
            moneyColumn,
            moneyColumn);
    }

    if(!db->Prepare(query) ||
       !db->BindInputParameter(&amount, 1, PARAMTYPE_Int64) ||
       !db->BindInputParameter(mutableAccount, 2, PARAMTYPE_String) ||
       !db->Execute(FALSE))
    {
        db->RollbackTransaction();
        db->Close();
        return FALSE;
    }

    /* Database.dll performs a second SELECT by UserID after the mutation and
     * returns that value to the game server. */
    sprintf_s(
        query,
        sizeof(query),
        "SELECT TOP 1 %s FROM CL WHERE UserID=?",
        moneyColumn);
    if(!db->Prepare(query) ||
       !db->BindInputParameter(mutableAccount, 1, PARAMTYPE_String))
    {
        db->RollbackTransaction();
        db->Close();
        return FALSE;
    }

    INT64 resultMoney = 0;
    if(db->Execute())
    {
        if(!db->GetData(&resultMoney, sizeof(resultMoney), 1, PARAMTYPE_Int64))
        {
            db->RollbackTransaction();
            db->Close();
            return FALSE;
        }
    }
    else if(db->GetLastError()[0] != 0)
    {
        db->RollbackTransaction();
        db->Close();
        return FALSE;
    }

    char operCode[3] = { 0 };
    strcpy_s(operCode, operation == 1 ? "01" : "02");
    INT64 loggedAmount = operation == 1 ? -amount : amount;
    char serverName[64] = { 0 };
    strcpy_s(serverName, rsServerConfig.szServerName);

    if(siegeMoney)
    {
        if(!db->Prepare(
            "INSERT INTO SiegeMoneyLog "
            "(CIDX,UserID,chName,ServerName,Operator,OperCode,SiegeMoney,RegistDay) "
            "VALUES (?,?,?,?,'GS',?,?,datetime('now','localtime'))") ||
           !db->BindInputParameter(&clanIdx, 1, PARAMTYPE_Integer) ||
           !db->BindInputParameter(mutableAccount, 2, PARAMTYPE_String) ||
           !db->BindInputParameter(mutableCharacter, 3, PARAMTYPE_String) ||
           !db->BindInputParameter(serverName, 4, PARAMTYPE_String) ||
           !db->BindInputParameter(operCode, 5, PARAMTYPE_String) ||
           !db->BindInputParameter(&loggedAmount, 6, PARAMTYPE_Int64) ||
           !db->Execute(FALSE))
        {
            db->RollbackTransaction();
            db->Close();
            return FALSE;
        }
    }
    else
    {
        /* The modern backup marks Note NOT NULL although the historical DLL
         * insert omitted it.  Supply the neutral empty value explicitly. */
        char emptyNote[1] = { 0 };
        if(!db->Prepare(
            "INSERT INTO ClanMoneyLog "
            "(CIDX,UserID,chName,ServerName,Operator,OperCode,ClanMoney,RegistDay,Note) "
            "VALUES (?,?,?,?,'GS',?,?,datetime('now','localtime'),?)") ||
           !db->BindInputParameter(&clanIdx, 1, PARAMTYPE_Integer) ||
           !db->BindInputParameter(mutableAccount, 2, PARAMTYPE_String) ||
           !db->BindInputParameter(mutableCharacter, 3, PARAMTYPE_String) ||
           !db->BindInputParameter(serverName, 4, PARAMTYPE_String) ||
           !db->BindInputParameter(operCode, 5, PARAMTYPE_String) ||
           !db->BindInputParameter(&loggedAmount, 6, PARAMTYPE_Int64) ||
           !db->BindInputParameter(emptyNote, 7, PARAMTYPE_String) ||
           !db->Execute(FALSE))
        {
            db->RollbackTransaction();
            db->Close();
            return FALSE;
        }
    }

    if(!db->CommitTransaction())
    {
        db->RollbackTransaction();
        db->Close();
        return FALSE;
    }

    *outResult = resultMoney;
    db->Close();
    return TRUE;
}

/* R18: Database.dll!SiegeMoneyTax is also gameplay persistence rather than
 * commercial billing.  Reverse engineering the supplied DLL proves that it:
 *   - rejects total==0 and malformed clan-mark strings (<9 chars);
 *   - derives ClanDB.CL.IDX from the last six decimal digits of the mark;
 *   - computes the tax share as Total - round(Total/(1 + Tax*0.01));
 *   - increments CL.SiegeMoney by that share;
 *   - records both SiegeMoneyTax and SiegeMoneyLog (OperCode 02, Server).
 *
 * The old SQL Server schema supplied RegistDay through DB defaults that are
 * not present in the catalog-only SQLite reconstruction, so this adapter
 * supplies local time explicitly.  FALSE means infrastructure/SQL failure
 * and lets the legacy DLL remain a temporary fallback. */
static BOOL TrySQLiteLocalSiegeMoneyTax(
    const char *clanMarkNum,
    INT64 mixing,
    INT64 aging,
    INT64 shop,
    INT64 poison1,
    INT64 poison2,
    INT64 poison3,
    INT64 force,
    INT64 warpgate,
    INT64 skill,
    INT64 total,
    INT64 tax,
    BOOL *outResult)
{
    if(!outResult)
        return FALSE;

    *outResult = FALSE;

    if(total == 0)
        return TRUE;

    if(!clanMarkNum)
        return TRUE;

    const size_t markLength = strlen(clanMarkNum);
    if(markLength < 9)
        return TRUE;

    const char *idxText = clanMarkNum + (markLength - 6);
    for(const char *p = idxText; *p; ++p)
    {
        if(*p < '0' || *p > '9')
            return TRUE;
    }

    const int clanIdx = atoi(idxText);
    if(clanIdx == 0)
        return TRUE;

    INT64 siegeCredit = 0;
    if(tax != 0)
    {
        /* The DLL performs this in x87 using the single-precision constant
         * 0.01f and then converts with FISTP (round-to-nearest in the normal
         * control mode).  Inputs here are non-negative currency/tax values;
         * +0.5 before integer conversion reproduces that positive-domain
         * rounding without introducing a new CRT/math dependency. */
        const float onePercent = 0.01f;
        const long double divisor = 1.0L + ((long double)tax * (long double)onePercent);
        if(divisor <= 0.0L)
            return TRUE;

        const long double untaxedValue = (long double)total / divisor;
        const INT64 untaxedRounded = (INT64)(untaxedValue + 0.5L);
        siegeCredit = total - untaxedRounded;
    }

    std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_ClanDB);
    if(!db || !db->Open())
        return FALSE;

    if(!db->BeginTransaction())
    {
        db->Close();
        return FALSE;
    }

    INT64 mutableCredit = siegeCredit;
    int mutableClanIdx = clanIdx;
    if(!db->Prepare("UPDATE CL SET SiegeMoney=SiegeMoney+? WHERE IDX=?") ||
       !db->BindInputParameter(&mutableCredit, 1, PARAMTYPE_Int64) ||
       !db->BindInputParameter(&mutableClanIdx, 2, PARAMTYPE_Integer) ||
       !db->Execute(FALSE))
    {
        db->RollbackTransaction();
        db->Close();
        return FALSE;
    }

    char serverName[64] = { 0 };
    strcpy_s(serverName, rsServerConfig.szServerName);

    INT64 mutableMixing = mixing;
    INT64 mutableAging = aging;
    INT64 mutableShop = shop;
    INT64 mutablePoison1 = poison1;
    INT64 mutablePoison2 = poison2;
    INT64 mutablePoison3 = poison3;
    INT64 mutableForce = force;
    INT64 mutableWarpgate = warpgate;
    INT64 mutableSkill = skill;
    INT64 mutableTotal = total;
    INT64 mutableTax = tax;

    if(!db->Prepare(
        "INSERT INTO SiegeMoneyTax "
        "(IDX,Mixing,Aging,Shop,Poison1,Poison2,Poison3,Force,Warpgate,Skill,Total,Tax,ServerName,RegistDay) "
        "VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,datetime('now','localtime'))") ||
       !db->BindInputParameter(&mutableClanIdx, 1, PARAMTYPE_Integer) ||
       !db->BindInputParameter(&mutableMixing, 2, PARAMTYPE_Int64) ||
       !db->BindInputParameter(&mutableAging, 3, PARAMTYPE_Int64) ||
       !db->BindInputParameter(&mutableShop, 4, PARAMTYPE_Int64) ||
       !db->BindInputParameter(&mutablePoison1, 5, PARAMTYPE_Int64) ||
       !db->BindInputParameter(&mutablePoison2, 6, PARAMTYPE_Int64) ||
       !db->BindInputParameter(&mutablePoison3, 7, PARAMTYPE_Int64) ||
       !db->BindInputParameter(&mutableForce, 8, PARAMTYPE_Int64) ||
       !db->BindInputParameter(&mutableWarpgate, 9, PARAMTYPE_Int64) ||
       !db->BindInputParameter(&mutableSkill, 10, PARAMTYPE_Int64) ||
       !db->BindInputParameter(&mutableTotal, 11, PARAMTYPE_Int64) ||
       !db->BindInputParameter(&mutableTax, 12, PARAMTYPE_Int64) ||
       !db->BindInputParameter(serverName, 13, PARAMTYPE_String) ||
       !db->Execute(FALSE))
    {
        db->RollbackTransaction();
        db->Close();
        return FALSE;
    }

    char serverUser[7] = "Server";
    char operCode[3] = "02";
    if(!db->Prepare(
        "INSERT INTO SiegeMoneyLog "
        "(CIDX,UserID,chName,ServerName,Operator,OperCode,SiegeMoney,RegistDay) "
        "VALUES (?,?,?,?, 'GS', ?, ?, datetime('now','localtime'))") ||
       !db->BindInputParameter(&mutableClanIdx, 1, PARAMTYPE_Integer) ||
       !db->BindInputParameter(serverUser, 2, PARAMTYPE_String) ||
       !db->BindInputParameter(serverUser, 3, PARAMTYPE_String) ||
       !db->BindInputParameter(serverName, 4, PARAMTYPE_String) ||
       !db->BindInputParameter(operCode, 5, PARAMTYPE_String) ||
       !db->BindInputParameter(&mutableCredit, 6, PARAMTYPE_Int64) ||
       !db->Execute(FALSE))
    {
        db->RollbackTransaction();
        db->Close();
        return FALSE;
    }

    if(!db->CommitTransaction())
    {
        db->RollbackTransaction();
        db->Close();
        return FALSE;
    }

    *outResult = TRUE;
    db->Close();
    return TRUE;
}


/* R22: WithSOD2RecordRanks is a non-authoritative statistics sink.  The
 * active SoD UI/ranking read path is already local and the recovered backup
 * contains the exact SoD2DB.SOD2RecBySandurr catalog.  Persist the event to
 * SQLite first and use Database.dll only as an infrastructure fallback.
 *
 * Database.dll enriches some PC-room/business columns that are not gameplay
 * inputs.  The simplified local server records the values it actually owns:
 * server, account/character, score totals, legacy PCRNo/GPCode and IP.  The
 * provider-only metadata is stored as neutral values rather than fabricating
 * commercial identity. */
static BOOL TrySQLiteLocalSODRecord(
    const char *userID,
    const char *charName,
    int charType,
    int point,
    int killCount,
    int gameLevel,
    int totalPoint,
    int totalUser,
    int successUser,
    int pcRNo,
    const char *gpCode,
    const char *ip,
    int serverTime)
{
    if(!userID || !userID[0] || !charName || !charName[0])
        return FALSE;

    std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_SoD2DB);
    if(!db || !db->Open())
        return FALSE;

    char mutableUserID[ACCOUNT_SIZE] = { 0 };
    char mutableCharName[NAME_SIZE] = { 0 };
    char mutableGPCode[16] = { 0 };
    char mutableIP[16] = { 0 };
    char serverName[64] = { 0 };
    char emptyText[1] = { 0 };

    strcpy_s(mutableUserID, userID);
    strcpy_s(mutableCharName, charName);
    if(gpCode)
        strcpy_s(mutableGPCode, gpCode);
    if(ip)
        strcpy_s(mutableIP, ip);
    strcpy_s(serverName, rsServerConfig.szServerName);

    int pmNo = 0;
    int contribute = 0;
    int flag = 0;
    int clanIdx = 0;

    BOOL ok = db->Prepare(
        "INSERT INTO SOD2RecBySandurr "
        "(UserID,CharName,CharType,Point,KillCount,GLevel,TotalPoint,TotalUser,SuccessUser,"
        "ServerName,PCRNo,GPCode,BusinessName,BAddress1,PMNo,Contribute,RegistDay,ServerTime,Flag,ClanIDX,ClanName) "
        "VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?, ?,datetime('now','localtime'),?,?,?,?)") &&
        db->BindInputParameter(mutableUserID, 1, PARAMTYPE_String) &&
        db->BindInputParameter(mutableCharName, 2, PARAMTYPE_String) &&
        db->BindInputParameter(&charType, 3, PARAMTYPE_Integer) &&
        db->BindInputParameter(&point, 4, PARAMTYPE_Integer) &&
        db->BindInputParameter(&killCount, 5, PARAMTYPE_Integer) &&
        db->BindInputParameter(&gameLevel, 6, PARAMTYPE_Integer) &&
        db->BindInputParameter(&totalPoint, 7, PARAMTYPE_Integer) &&
        db->BindInputParameter(&totalUser, 8, PARAMTYPE_Integer) &&
        db->BindInputParameter(&successUser, 9, PARAMTYPE_Integer) &&
        db->BindInputParameter(serverName, 10, PARAMTYPE_String) &&
        db->BindInputParameter(&pcRNo, 11, PARAMTYPE_Integer) &&
        db->BindInputParameter(mutableGPCode, 12, PARAMTYPE_String) &&
        db->BindInputParameter(emptyText, 13, PARAMTYPE_String) &&
        db->BindInputParameter(mutableIP, 14, PARAMTYPE_String) &&
        db->BindInputParameter(&pmNo, 15, PARAMTYPE_Integer) &&
        db->BindInputParameter(&contribute, 16, PARAMTYPE_Integer) &&
        db->BindInputParameter(&serverTime, 17, PARAMTYPE_Integer) &&
        db->BindInputParameter(&flag, 18, PARAMTYPE_Integer) &&
        db->BindInputParameter(&clanIdx, 19, PARAMTYPE_Integer) &&
        db->BindInputParameter(emptyText, 20, PARAMTYPE_String) &&
        db->Execute(FALSE);

    db->Close();
    return ok;
}


/* R22: CharLog/CharRecordLog are audit/history sinks.  Their return values do
 * not drive gameplay.  Record the exact server-owned event payload in a local
 * append-only table; only fall back to Database.dll if the local write fails.
 * EventType preserves the legacy nFlag values: 1=new, 2=delete, 3=logout. */
static BOOL TrySQLiteLocalCharacterAudit(const sLOG_SQLDATA &data, int eventType)
{
    std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_GameLogDB);
    if(!db || !db->Open())
        return FALSE;

    char userID[ACCOUNT_SIZE] = { 0 };
    char charName[NAME_SIZE] = { 0 };
    char gpCode[16] = { 0 };
    char ip[16] = { 0 };
    char serverName[64] = { 0 };
    strcpy_s(userID, data.szUserID);
    strcpy_s(charName, data.szCharName);
    strcpy_s(gpCode, data.szGPCode);
    strcpy_s(ip, data.szIP);
    strcpy_s(serverName, szServerName);

    int charType = data.Occupation;
    int level = (int)data.Level;
    INT64 exp = data.Exp64;
    INT64 money = (INT64)data.dwMoney;

    BOOL ok = db->Prepare(
        "INSERT INTO LocalCharacterEvent "
        "(EventType,UserID,CharName,CharType,GPCode,IP,ServerName,GameLevel,Exp,GameMoney,CreatedAt) "
        "VALUES (?,?,?,?,?,?,?,?,?,?,datetime('now','localtime'))") &&
        db->BindInputParameter(&eventType, 1, PARAMTYPE_Integer) &&
        db->BindInputParameter(userID, 2, PARAMTYPE_String) &&
        db->BindInputParameter(charName, 3, PARAMTYPE_String) &&
        db->BindInputParameter(&charType, 4, PARAMTYPE_Integer) &&
        db->BindInputParameter(gpCode, 5, PARAMTYPE_String) &&
        db->BindInputParameter(ip, 6, PARAMTYPE_String) &&
        db->BindInputParameter(serverName, 7, PARAMTYPE_String) &&
        db->BindInputParameter(&level, 8, PARAMTYPE_Integer) &&
        db->BindInputParameter(&exp, 9, PARAMTYPE_Int64) &&
        db->BindInputParameter(&money, 10, PARAMTYPE_Int64) &&
        db->Execute(FALSE);

    db->Close();
    return ok;
}

/* R22: preserve the complete in-process item audit payload locally rather
 * than guessing the historical Database.dll storage procedure.  The event
 * header and item list are written transactionally. */
static BOOL TrySQLiteLocalItemAudit(
    int eventType,
    const char *userID,
    const char *charName,
    long ip,
    int flag,
    const char *targetUserID,
    const char *targetCharName,
    long targetIP,
    int money,
    long giftNo,
    const char *telephone,
    int itemCount,
    const _LogITEM *items)
{
    if(!userID || !charName || itemCount < 0 || itemCount > 32 || (itemCount && !items))
        return FALSE;

    std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_ItemLogDB);
    if(!db || !db->Open())
        return FALSE;
    if(!db->BeginTransaction())
    {
        db->Close();
        return FALSE;
    }

    char mutableUserID[ACCOUNT_SIZE] = { 0 };
    char mutableCharName[NAME_SIZE] = { 0 };
    char mutableTargetUserID[ACCOUNT_SIZE] = { 0 };
    char mutableTargetCharName[NAME_SIZE] = { 0 };
    char mutableTelephone[16] = { 0 };
    strcpy_s(mutableUserID, userID);
    strcpy_s(mutableCharName, charName);
    if(targetUserID)
        strcpy_s(mutableTargetUserID, targetUserID);
    if(targetCharName)
        strcpy_s(mutableTargetCharName, targetCharName);
    if(telephone)
        strcpy_s(mutableTelephone, telephone);

    INT64 ip64 = (INT64)ip;
    INT64 targetIP64 = (INT64)targetIP;
    INT64 money64 = (INT64)money;
    INT64 giftNo64 = (INT64)giftNo;

    BOOL ok = db->Prepare(
        "INSERT INTO LocalItemEvent "
        "(EventType,UserID,CharName,IP,Flag,TargetUserID,TargetCharName,TargetIP,Money,GiftNo,Telephone,ItemCount,CreatedAt) "
        "VALUES (?,?,?,?,?,?,?,?,?,?,?,?,datetime('now','localtime'))") &&
        db->BindInputParameter(&eventType, 1, PARAMTYPE_Integer) &&
        db->BindInputParameter(mutableUserID, 2, PARAMTYPE_String) &&
        db->BindInputParameter(mutableCharName, 3, PARAMTYPE_String) &&
        db->BindInputParameter(&ip64, 4, PARAMTYPE_Int64) &&
        db->BindInputParameter(&flag, 5, PARAMTYPE_Integer) &&
        db->BindInputParameter(mutableTargetUserID, 6, PARAMTYPE_String) &&
        db->BindInputParameter(mutableTargetCharName, 7, PARAMTYPE_String) &&
        db->BindInputParameter(&targetIP64, 8, PARAMTYPE_Int64) &&
        db->BindInputParameter(&money64, 9, PARAMTYPE_Int64) &&
        db->BindInputParameter(&giftNo64, 10, PARAMTYPE_Int64) &&
        db->BindInputParameter(mutableTelephone, 11, PARAMTYPE_String) &&
        db->BindInputParameter(&itemCount, 12, PARAMTYPE_Integer) &&
        db->Execute(FALSE);

    if(!ok || !db->Prepare("SELECT last_insert_rowid()") || !db->Execute())
    {
        db->RollbackTransaction();
        db->Close();
        return FALSE;
    }

    INT64 eventID = 0;
    if(!db->GetData(&eventID, sizeof(eventID), 1, PARAMTYPE_Int64) || eventID <= 0)
    {
        db->RollbackTransaction();
        db->Close();
        return FALSE;
    }

    for(int index = 0; index < itemCount; ++index)
    {
        int itemIndex = index;
        INT64 itemCode = (INT64)(DWORD)items[index].dwCode;
        INT64 itemINo = (INT64)(DWORD)items[index].dwINo[0];
        INT64 itemINo1 = (INT64)(DWORD)items[index].dwINo[1];
        if(!db->Prepare(
            "INSERT INTO LocalItemEventEntry (EventID,ItemIndex,ItemCode,ItemINo,ItemINo1) "
            "VALUES (?,?,?,?,?)") ||
           !db->BindInputParameter(&eventID, 1, PARAMTYPE_Int64) ||
           !db->BindInputParameter(&itemIndex, 2, PARAMTYPE_Integer) ||
           !db->BindInputParameter(&itemCode, 3, PARAMTYPE_Int64) ||
           !db->BindInputParameter(&itemINo, 4, PARAMTYPE_Int64) ||
           !db->BindInputParameter(&itemINo1, 5, PARAMTYPE_Int64) ||
           !db->Execute(FALSE))
        {
            db->RollbackTransaction();
            db->Close();
            return FALSE;
        }
    }

    if(!db->CommitTransaction())
    {
        db->RollbackTransaction();
        db->Close();
        return FALSE;
    }

    db->Close();
    return TRUE;
}

static BOOL IsSQLiteLocalZeroValue(const char *value)
{
	return !value || !value[0] || (value[0] == '0' && value[1] == 0);
}

static void LogSQLiteLocalAuthStage(const char *userID, const char *stage, const char *detail)
{
	FILE *fp = fopen("LOGS\\mingw_sqlite_local_auth.txt", "a");
	if(!fp)
		return;
	fprintf(
		fp,
		"[SQLITE_LOCAL_AUTH] user=%s stage=%s%s%s\n",
		userID ? userID : "",
		stage ? stage : "",
		(detail && detail[0]) ? " detail=" : "",
		(detail && detail[0]) ? detail : "");
	fflush(fp);
	fclose(fp);
}

static BOOL ParseSQLiteLocalAccountTime(const char *value, time_t *outValue)
{
	if(!outValue)
		return FALSE;

	*outValue = 0;
	if(IsSQLiteLocalZeroValue(value))
		return TRUE;

	BOOL digitsOnly = TRUE;
	for(const char *p = value; *p; ++p)
	{
		if(*p < '0' || *p > '9')
		{
			digitsOnly = FALSE;
			break;
		}
	}

	if(digitsOnly)
	{
		*outValue = (time_t)_atoi64(value);
		return TRUE;
	}

	int year = 0;
	int month = 0;
	int day = 0;
	int hour = 0;
	int minute = 0;
	int second = 0;
	if(sscanf(value, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second) != 6)
		return FALSE;

	struct tm localTime;
	ZeroMemory(&localTime, sizeof(localTime));
	localTime.tm_year = year - 1900;
	localTime.tm_mon = month - 1;
	localTime.tm_mday = day;
	localTime.tm_hour = hour;
	localTime.tm_min = minute;
	localTime.tm_sec = second;
	localTime.tm_isdst = -1;

	time_t parsed = mktime(&localTime);
	if(parsed == (time_t)-1)
		return FALSE;

	*outValue = parsed;
	return TRUE;
}

static BOOL MakeSQLiteLocalSessionGuid(char *outGuid)
{
	if(!outGuid)
		return FALSE;

	GUID guid;
	if(FAILED(CoCreateGuid(&guid)))
	{
		outGuid[0] = 0;
		return FALSE;
	}

	wsprintfA(
		outGuid,
		"%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
		(unsigned long)guid.Data1,
		(unsigned int)guid.Data2,
		(unsigned int)guid.Data3,
		(unsigned int)guid.Data4[0],
		(unsigned int)guid.Data4[1],
		(unsigned int)guid.Data4[2],
		(unsigned int)guid.Data4[3],
		(unsigned int)guid.Data4[4],
		(unsigned int)guid.Data4[5],
		(unsigned int)guid.Data4[6],
		(unsigned int)guid.Data4[7]);
	return TRUE;
}

static BOOL GetSQLiteLocalAccountInitial(const char *userID, char *outInitial)
{
	if(!userID || !userID[0] || !outInitial)
		return FALSE;

	char initial = userID[0];
	if(initial >= 'a' && initial <= 'z')
		initial = (char)(initial - 'a' + 'A');
	if(initial < 'A' || initial > 'Z')
		return FALSE;

	*outInitial = initial;
	return TRUE;
}

static BOOL RollbackAndCloseSQLiteLocalSession(const std::shared_ptr<SQLConnection> &db)
{
	if(!db)
		return FALSE;

	db->RollbackTransaction();
	db->Close();
	return FALSE;
}


enum SQLiteLocalAuthMode
{
	SQLITE_LOCAL_AUTH_IMPLICIT = 0,
	SQLITE_LOCAL_AUTH_LOCAL = 1,
	SQLITE_LOCAL_AUTH_LEGACY = 2
};

/* R23: resolve explicit authentication ownership without depending on the
 * historical billing columns. The scalar subquery always returns one row so
 * an absent policy is distinguishable from a query failure. */
static BOOL ReadSQLiteLocalAuthMode(
	const std::shared_ptr<SQLConnection> &db,
	char *userID,
	int *outMode)
{
	if(!db || !userID || !userID[0] || !outMode)
		return FALSE;

	/* Keep this query free of CASE/ELSE. SqliteQueryCompat deliberately
	 * rejects T-SQL control-flow tokens and its legacy ELSE detector also
	 * matches CASE ... ELSE ... END. Resolve the textual policy in C++
	 * instead of encoding it as a SQL CASE expression. */
	const char *query =
		"SELECT COALESCE((SELECT lower(AuthMode) "
		"FROM LocalAuth__AccountPolicy WHERE UserID=? AND Enabled=1 LIMIT 1),'')";

	BOOL prepared = db->Prepare(query);
	if(prepared)
		prepared = db->BindInputParameter(userID, 1, PARAMTYPE_String);

	char modeText[32] = { 0 };
	if(!prepared || !db->Execute() ||
		!db->GetData(modeText, sizeof(modeText), 1, PARAMTYPE_String))
	{
		return FALSE;
	}

	int mode = SQLITE_LOCAL_AUTH_IMPLICIT;
	if(strcmp(modeText, "local") == 0)
		mode = SQLITE_LOCAL_AUTH_LOCAL;
	else if(strcmp(modeText, "legacy") == 0)
		mode = SQLITE_LOCAL_AUTH_LEGACY;

	*outMode = mode;
	return TRUE;
}

static BOOL IsSQLiteLegacyBillingFallbackEnabled()
{
	std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_AccountDB);
	if(!db || !db->Open())
		return TRUE;

	const char *query =
		"SELECT COALESCE((SELECT PolicyValue FROM LocalAuth__RuntimePolicy "
		"WHERE PolicyKey='legacy_billing_fallback' LIMIT 1),'1')";

	char value[32] = { 0 };
	BOOL ok = db->Prepare(query) && db->Execute() &&
		db->GetData(value, sizeof(value), 1, PARAMTYPE_String);
	db->Close();

	/* Failure keeps the historical provider enabled. This makes the switch
	 * opt-in and preserves R22 behavior on any unexpected policy read error. */
	if(!ok)
		return TRUE;
	return strcmp(value, "0") == 0 ? FALSE : TRUE;
}

/*
 * Claim the AccountDB row only after the local/free eligibility checks have
 * succeeded.  The supplied Database.dll proves the historical session write:
 * StartDay/EditDay, ServerName, inuse='1', SNo=<session guid>, RNo=NULL.
 * BEGIN IMMEDIATE keeps the eligibility read and claim atomic across server
 * threads/processes sharing summit.sqlite3.
 */
static BOOL ClaimSQLiteLocalSession(
	const std::shared_ptr<SQLConnection> &db,
	char accountInitial,
	char *userID,
	char *sessionGuid)
{
	if(!db || !userID || !sessionGuid || !sessionGuid[0])
		return FALSE;

	char query[512];
	wsprintfA(
		query,
		"UPDATE %cGameUser "
		"SET StartDay=datetime('now','localtime'), EditDay=datetime('now','localtime'), "
		"RNo=NULL, ServerName=?, inuse='1', SNo=? "
		"WHERE userid=?",
		accountInitial);

	char *serverName = rsServerConfig.szServerName;
	BOOL prepared = db->Prepare(query);
	if(prepared)
		prepared = db->BindInputParameter(serverName, 1, PARAMTYPE_String);
	if(prepared)
		prepared = db->BindInputParameter(sessionGuid, 2, PARAMTYPE_String);
	if(prepared)
		prepared = db->BindInputParameter(userID, 3, PARAMTYPE_String);
	return prepared && db->Execute(FALSE);
}

/*
 * Local/free logout counterpart to ClaimSQLiteLocalSession.
 *
 * R23 adds explicit ownership: an AuthMode=local account is handled by SQLite
 * even when its historical billing columns are non-zero. Once ownership is
 * local, GUID/server mismatches or local write failures must not fall through
 * to Database.dll, because that provider no longer owns the session.
 */
static BOOL TrySQLiteLocalLogout(char *userID, char *sessionGuid)
{
	char initial = 0;
	if(!GetSQLiteLocalAccountInitial(userID, &initial))
		return FALSE;

	std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_AccountDB);
	if(!db || !db->Open())
		return FALSE;
	if(!db->BeginTransaction())
	{
		db->Close();
		return FALSE;
	}

	int authMode = SQLITE_LOCAL_AUTH_IMPLICIT;
	if(!ReadSQLiteLocalAuthMode(db, userID, &authMode))
		return RollbackAndCloseSQLiteLocalSession(db);
	if(authMode == SQLITE_LOCAL_AUTH_LEGACY)
		return RollbackAndCloseSQLiteLocalSession(db);

	const BOOL explicitlyLocal = authMode == SQLITE_LOCAL_AUTH_LOCAL;
	if(!sessionGuid || !sessionGuid[0])
	{
		db->RollbackTransaction();
		db->Close();
		return explicitlyLocal ? TRUE : FALSE;
	}

	char query[512];
	wsprintfA(
		query,
		"SELECT GPCode,UsePeriod,inuse,RNo,BNum,Channel,SNo,ServerName "
		"FROM %cGameUser WHERE userid=? LIMIT 1",
		initial);

	BOOL prepared = db->Prepare(query);
	if(prepared)
		prepared = db->BindInputParameter(userID, 1, PARAMTYPE_String);
	if(!prepared || !db->Execute())
	{
		db->RollbackTransaction();
		db->Close();
		return explicitlyLocal ? TRUE : FALSE;
	}

	char gpCode[64] = { 0 };
	char inUse[16] = { 0 };
	char channel[64] = { 0 };
	char storedGuid[128] = { 0 };
	char storedServer[128] = { 0 };
	int usePeriod = 0;
	int pcRNo = 0;
	int billingNumber = 0;

	BOOL rowOK =
		db->GetData(gpCode, sizeof(gpCode), 1, PARAMTYPE_String) &&
		db->GetData(&usePeriod, 0, 2, PARAMTYPE_Integer) &&
		db->GetData(inUse, sizeof(inUse), 3, PARAMTYPE_String) &&
		db->GetData(&pcRNo, 0, 4, PARAMTYPE_Integer) &&
		db->GetData(&billingNumber, 0, 5, PARAMTYPE_Integer) &&
		db->GetData(channel, sizeof(channel), 6, PARAMTYPE_String) &&
		db->GetData(storedGuid, sizeof(storedGuid), 7, PARAMTYPE_String) &&
		db->GetData(storedServer, sizeof(storedServer), 8, PARAMTYPE_String);

	BOOL sessionMatches = rowOK && strcmp(inUse, "1") == 0 &&
		strcmp(storedGuid, sessionGuid) == 0 &&
		strcmp(storedServer, rsServerConfig.szServerName) == 0;
	BOOL implicitSignature = rowOK && IsSQLiteLocalZeroValue(gpCode) &&
		usePeriod == 0 && pcRNo == 0 && billingNumber == 0 &&
		IsSQLiteLocalZeroValue(channel);

	if(!sessionMatches || (!explicitlyLocal && !implicitSignature))
	{
		db->RollbackTransaction();
		db->Close();
		return explicitlyLocal ? TRUE : FALSE;
	}

	wsprintfA(
		query,
		"UPDATE %cGameUser "
		"SET LastDay=datetime('now','localtime'), inuse='0', RNo=NULL, SNo=NULL "
		"WHERE userid=? AND ServerName=?",
		initial);

	prepared = db->Prepare(query);
	if(prepared)
		prepared = db->BindInputParameter(userID, 1, PARAMTYPE_String);
	if(prepared)
		prepared = db->BindInputParameter(rsServerConfig.szServerName, 2, PARAMTYPE_String);
	if(!prepared || !db->Execute(FALSE))
	{
		db->RollbackTransaction();
		db->Close();
		return explicitlyLocal ? TRUE : FALSE;
	}

	if(!db->CommitTransaction())
	{
		db->RollbackTransaction();
		db->Close();
		return explicitlyLocal ? TRUE : FALSE;
	}
	db->Close();
	return TRUE;
}

/*
 * Clear only stale sessions that match the local/free signature introduced
 * above.  This replaces the intent of the optional SetAllGameUser startup
 * cleanup without touching paid/PC-room rows.
 */
static BOOL ResetSQLiteLocalSessionsForServer()
{
	std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_AccountDB);
	if(!db || !db->Open())
		return FALSE;
	if(!db->BeginTransaction())
	{
		db->Close();
		return FALSE;
	}

	for(char initial = 'A'; initial <= 'Z'; ++initial)
	{
		char query[1024];
		wsprintfA(
			query,
			"UPDATE %cGameUser "
			"SET LastDay=datetime('now','localtime'), inuse='0', RNo=NULL, SNo=NULL "
			"WHERE ServerName=? AND inuse='1' AND SNo IS NOT NULL AND SNo<>'' "
			"AND (((GPCode IS NULL OR GPCode='' OR GPCode='0') "
			"AND COALESCE(UsePeriod,0)=0 AND COALESCE(BNum,0)=0 "
			"AND (Channel IS NULL OR Channel='' OR Channel='0') "
			"AND (RNo IS NULL OR RNo=0)) "
			"OR userid COLLATE NOCASE IN "
			"(SELECT UserID FROM LocalAuth__AccountPolicy "
			"WHERE Enabled=1 AND lower(AuthMode)='local'))",
			initial);

		BOOL prepared = db->Prepare(query);
		if(prepared)
			prepared = db->BindInputParameter(rsServerConfig.szServerName, 1, PARAMTYPE_String);
		if(!prepared || !db->Execute(FALSE))
			return RollbackAndCloseSQLiteLocalSession(db);
	}

	if(!db->CommitTransaction())
		return RollbackAndCloseSQLiteLocalSession(db);
	db->Close();
	return TRUE;
}

/*
 * SQLite local-login compatibility path.
 *
 * It intentionally handles only the subset whose AccountDB row proves that
 * no billing/PC-room state is involved.  Any ambiguous/paid account is left
 * to the legacy Database.dll while that compatibility provider is present.
 * This lets a local/free server authenticate directly from summit.sqlite3
 * without guessing the wider SQLLogon billing contract.
 */
static BOOL TrySQLiteLocalLogOn(
	char *lpszPID,
	char *lpszPWD,
	char *lpszGPCode,
	char *lpszDisuseDay,
	char *lpszUsePeriod,
	char *szGuid,
	char *lpszPCRNo,
	char *lpszGubunCode,
	int *outResult)
{
	LogSQLiteLocalAuthStage(lpszPID, "enter", "");
	if(!lpszPID || !lpszPID[0] || !lpszPWD || !outResult)
	{
		LogSQLiteLocalAuthStage(lpszPID, "reject_bad_input", "");
		return FALSE;
	}

	char initial = 0;
	if(!GetSQLiteLocalAccountInitial(lpszPID, &initial))
	{
		LogSQLiteLocalAuthStage(lpszPID, "reject_bad_initial", "");
		return FALSE;
	}

	std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_AccountDB);
	if(!db || !db->Open())
	{
		LogSQLiteLocalAuthStage(lpszPID, "sqlite_open_failed", db ? db->GetLastError() : "no connection");
		return FALSE;
	}
	if(!db->BeginTransaction())
	{
		LogSQLiteLocalAuthStage(lpszPID, "begin_failed", db->GetLastError());
		db->Close();
		return FALSE;
	}

	int authMode = SQLITE_LOCAL_AUTH_IMPLICIT;
	if(!ReadSQLiteLocalAuthMode(db, lpszPID, &authMode))
	{
		LogSQLiteLocalAuthStage(lpszPID, "auth_mode_read_failed", db->GetLastError());
		return RollbackAndCloseSQLiteLocalSession(db);
	}
	{
		char detail[64] = { 0 };
		wsprintfA(detail, "mode=%d", authMode);
		LogSQLiteLocalAuthStage(lpszPID, "auth_mode", detail);
	}
	if(authMode == SQLITE_LOCAL_AUTH_LEGACY)
	{
		LogSQLiteLocalAuthStage(lpszPID, "fallback_legacy_policy", "");
		return RollbackAndCloseSQLiteLocalSession(db);
	}
	const BOOL explicitlyLocal = authMode == SQLITE_LOCAL_AUTH_LOCAL;

	char query[384];
	wsprintfA(
		query,
		"SELECT Passwd,GPCode,DisuseDay,UsePeriod,inuse,BlockChk,DelChk,RNo,BNum,Channel "
		"FROM %cGameUser WHERE userid=? LIMIT 1",
		initial);

	BOOL prepared = db->Prepare(query);
	if(prepared)
		prepared = db->BindInputParameter(lpszPID, 1, PARAMTYPE_String);
	if(!prepared || !db->Execute())
	{
		LogSQLiteLocalAuthStage(lpszPID, "account_query_failed_or_missing", db->GetLastError());
		db->RollbackTransaction();
		db->Close();
		if(explicitlyLocal)
		{
			*outResult = FALSE;
			return TRUE;
		}
		return FALSE;
	}

	char storedPassword[128] = { 0 };
	char gpCode[64] = { 0 };
	char disuseDayText[64] = { 0 };
	char inUse[16] = { 0 };
	char blockChk[16] = { 0 };
	char delChk[16] = { 0 };
	char channel[64] = { 0 };
	int usePeriod = 0;
	int pcRNo = 0;
	int billingNumber = 0;

	BOOL rowOK =
		db->GetData(storedPassword, sizeof(storedPassword), 1, PARAMTYPE_String) &&
		db->GetData(gpCode, sizeof(gpCode), 2, PARAMTYPE_String) &&
		db->GetData(disuseDayText, sizeof(disuseDayText), 3, PARAMTYPE_String) &&
		db->GetData(&usePeriod, 0, 4, PARAMTYPE_Integer) &&
		db->GetData(inUse, sizeof(inUse), 5, PARAMTYPE_String) &&
		db->GetData(blockChk, sizeof(blockChk), 6, PARAMTYPE_String) &&
		db->GetData(delChk, sizeof(delChk), 7, PARAMTYPE_String) &&
		db->GetData(&pcRNo, 0, 8, PARAMTYPE_Integer) &&
		db->GetData(&billingNumber, 0, 9, PARAMTYPE_Integer) &&
		db->GetData(channel, sizeof(channel), 10, PARAMTYPE_String);

	{
		char detail[512] = { 0 };
		wsprintfA(
			detail,
			"rowOK=%d storedPwdLen=%d suppliedPwdLen=%d inuse=[%s] block=[%s] del=[%s] gp=[%s] usePeriod=%d RNo=%d BNum=%d channel=[%s]",
			rowOK,
			(int)strlen(storedPassword),
			(int)strlen(lpszPWD),
			inUse,
			blockChk,
			delChk,
			gpCode,
			usePeriod,
			pcRNo,
			billingNumber,
			channel);
		LogSQLiteLocalAuthStage(lpszPID, "account_row", detail);
	}

	if(!rowOK || strcmp(storedPassword, lpszPWD) != 0)
	{
		LogSQLiteLocalAuthStage(lpszPID, rowOK ? "reject_password_mismatch" : "reject_row_decode", db->GetLastError());
		db->RollbackTransaction();
		db->Close();
		if(explicitlyLocal)
		{
			*outResult = FALSE;
			return TRUE;
		}
		return FALSE;
	}

	/* Block/deletion/session state remains authoritative for all local paths. */
	if(!IsSQLiteLocalZeroValue(inUse) || !IsSQLiteLocalZeroValue(blockChk) ||
		!IsSQLiteLocalZeroValue(delChk))
	{
		LogSQLiteLocalAuthStage(lpszPID, "reject_account_state", "inuse/block/del is non-zero");
		db->RollbackTransaction();
		db->Close();
		if(explicitlyLocal)
		{
			*outResult = FALSE;
			return TRUE;
		}
		return FALSE;
	}

	time_t disuseDay = 0;
	if(!explicitlyLocal)
	{
		/* Preserve the conservative R19 signature for accounts without an
		 * explicit ownership policy. */
		if(!IsSQLiteLocalZeroValue(gpCode) || usePeriod != 0 || pcRNo != 0 ||
			billingNumber != 0 || !IsSQLiteLocalZeroValue(channel))
		{
			LogSQLiteLocalAuthStage(lpszPID, "fallback_provider_signature", "");
			return RollbackAndCloseSQLiteLocalSession(db);
		}

		if(!ParseSQLiteLocalAccountTime(disuseDayText, &disuseDay))
		{
			LogSQLiteLocalAuthStage(lpszPID, "fallback_bad_disuse_day", disuseDayText);
			return RollbackAndCloseSQLiteLocalSession(db);
		}
		if(disuseDay != 0 && disuseDay < time(NULL))
		{
			LogSQLiteLocalAuthStage(lpszPID, "fallback_expired_disuse_day", disuseDayText);
			return RollbackAndCloseSQLiteLocalSession(db);
		}
	}
	else
	{
		/* Explicit local ownership deliberately neutralizes historical billing
		 * identity/period fields instead of manufacturing a fake provider row. */
		disuseDay = 0;
	}

	if(!MakeSQLiteLocalSessionGuid(szGuid))
	{
		LogSQLiteLocalAuthStage(lpszPID, "session_guid_failed", "CoCreateGuid");
		db->RollbackTransaction();
		db->Close();
		if(explicitlyLocal)
		{
			*outResult = FALSE;
			return TRUE;
		}
		return FALSE;
	}
	LogSQLiteLocalAuthStage(lpszPID, "session_guid_ok", "");
	if(!ClaimSQLiteLocalSession(db, initial, lpszPID, szGuid))
	{
		LogSQLiteLocalAuthStage(lpszPID, "session_claim_failed", db->GetLastError());
		db->RollbackTransaction();
		db->Close();
		if(explicitlyLocal)
		{
			*outResult = FALSE;
			return TRUE;
		}
		return FALSE;
	}
	LogSQLiteLocalAuthStage(lpszPID, "session_claim_ok", "");
	if(!db->CommitTransaction())
	{
		LogSQLiteLocalAuthStage(lpszPID, "commit_failed", db->GetLastError());
		db->RollbackTransaction();
		db->Close();
		if(explicitlyLocal)
		{
			*outResult = FALSE;
			return TRUE;
		}
		return FALSE;
	}
	db->Close();

	if(lpszGPCode)
		lpszGPCode[0] = 0;
	if(lpszDisuseDay)
		*((time_t *)lpszDisuseDay) = disuseDay;
	if(lpszUsePeriod)
		*((int *)lpszUsePeriod) = 0;
	if(lpszPCRNo)
		*((int *)lpszPCRNo) = 0;
	if(lpszGubunCode)
		*((int *)lpszGubunCode) = 0;

	*outResult = TRUE;
	LogSQLiteLocalAuthStage(lpszPID, "success", "");
	return TRUE;
}

/*
 * Local gameplay entitlement lookup.
 *
 * The row is intentionally independent from AccountDB billing columns.  An
 * active legacy PC-room session still reaches gameplay through Bl_RNo; this
 * lookup only supplies the additional local source used by the R21 facade.
 */
int bSql_GetLocalPCBangGameplayEntitlement(const char *userID)
{
	if(!userID || !userID[0])
		return FALSE;

	std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_AccountDB);
	if(!db || !db->Open())
		return FALSE;

	const char *query =
		"SELECT Enabled FROM LocalAuth__GameplayEntitlement "
		"WHERE UserID=? AND Entitlement='pcbang_gameplay' AND Enabled=1 "
		"AND (ExpiresAt IS NULL OR ExpiresAt='' OR datetime(ExpiresAt) > datetime('now','localtime')) "
		"LIMIT 1";

	char *bindUserID = const_cast<char *>(userID);
	BOOL prepared = db->Prepare(query);
	if(prepared)
		prepared = db->BindInputParameter(bindUserID, 1, PARAMTYPE_String);

	int enabled = 0;
	BOOL found = prepared && db->Execute() &&
		db->GetData(&enabled, 0, 1, PARAMTYPE_Integer);

	db->Close();
	return found && enabled == 1 ? TRUE : FALSE;
}

int	bSql_LogOn(char *lpszPID, char *lpszPWD, char *lpszIP, char *lpszGPCode, char *lpszDisuseDay, char *lpszUsePeriod, char *szGuid, char *lpszPCRNo, char *lpszGubunCode)
{
	int result = 0;

	EnterCriticalSection(&cSqlSection);

	if(!TrySQLiteLocalLogOn(
		lpszPID,
		lpszPWD,
		lpszGPCode,
		lpszDisuseDay,
		lpszUsePeriod,
		szGuid,
		lpszPCRNo,
		lpszGubunCode,
		&result))
	{
		if(BillSql.SQLLogon)
		{
			result = BillSql.SQLLogon(
				lpszPID,
				lpszPWD,
				lpszIP,
				lpszGPCode,
				lpszDisuseDay,
				lpszUsePeriod,
				szGuid,
				lpszPCRNo,
				lpszGubunCode);
		}
		else
		{
			/* No compatible local/free row and no legacy billing provider. */
			result = 0;
		}
	}

	LeaveCriticalSection(&cSqlSection);

	return result;
}


LoginResult login(std::string account, std::string password, std::string ip)
{

	LoginResult result = LoginResult::FAILED_CONNECTION;

	EnterCriticalSection(&cSqlSection);



	

	LeaveCriticalSection(&cSqlSection);

	return result;
}


int	bSql_ServerDown()
{
	if(BillSql.ServerDown)
	{
		BillSql.ServerDown();
		return TRUE;
	}

	return FALSE;
}

int	bSql_ServerExit()
{
	BOOL localResult = ResetSQLiteLocalSessionsForServer();
	BOOL dllResult = FALSE;

	if(BillSql.ServerExit)
	{
		BillSql.ServerExit();
		dllResult = TRUE;
	}

	return localResult || dllResult;
}



int LoadLogSqlDLL()
{

	InitializeCriticalSection(&cSqlSection);

	BOOL legacyBillingFallback = IsSQLiteLegacyBillingFallbackEnabled();
	if(BillSql.Init(legacyBillingFallback))
	{

		if(BillSql.SetAllGameUser)
			BillSql.SetAllGameUser();
	}

	/* Recover stale local/free sessions left by an unclean previous exit. */
	ResetSQLiteLocalSessionsForServer();

	/* SQLite now provides the local/free SQLLogon path.  Keep the historical
	 * LOAD_DLL_SQL dispatch active even when Database.dll is absent; every
	 * remaining DLL operation is already pointer-guarded and bSql_LogOn itself
	 * falls back to the DLL only for billing cases it cannot prove locally. */
	UseDLL_Code = LOAD_DLL_SQL;
	return TRUE;

}




DWORD WINAPI LogSqlDataThreadProc(void *pInfo)
{
	HANDLE	hThread;

	sLOG_SQLDATA	logSqlData;
	sLOG_SQLDATA	logSqlData2;
	int cnt, mcnt;
	int	result;
	int	NextFlag;
	DWORD	dwTime;
	DWORD	dwProcessTime;
	int	QueSz;
	char	szPG[16];
	int		GpCode;
	DWORD	dwIdCode;
	int		MeterCode;
	DWORD	dwPeriodTime2;


	hThread = GetCurrentThread();

	/* MINGW_LOGSQL_COM_INIT_V1
	 * Database.dll usa OLE DB/COM. COM deve ser inicializado
	 * na propria LogSqlDataThreadProc antes de LoadLogSqlDLL(),
	 * pois BillSql.Init() pode chamar SetAllGameUser imediatamente.
	 */
	HRESULT mingwLogSqlComHr =
		CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	BOOL mingwLogSqlComInitialized =
		SUCCEEDED(mingwLogSqlComHr);

	{
		FILE *fp = fopen("LOGS\\mingw_logsql_worker.txt", "a");

		if(fp)
		{
			fprintf(
				fp,
				"[THREAD-INIT] tid=%lu comHr=0x%08lX initialized=%d\n",
				(unsigned long)GetCurrentThreadId(),
				(unsigned long)mingwLogSqlComHr,
				mingwLogSqlComInitialized ? 1 : 0
			);
			fflush(fp);
			fclose(fp);
		}
	}

	LoadLogSqlDLL();

	{
		FILE *fp = fopen("LOGS\\mingw_logsql_worker.txt", "a");

		if(fp)
		{
			char modulePath[MAX_PATH] = { 0 };

			if(BillSql.hLogSqlDLL)
			{
				GetModuleFileNameA(
					BillSql.hLogSqlDLL,
					modulePath,
					MAX_PATH - 1
				);
			}

			fprintf(
				fp,
				"[DLL] tid=%lu handle=%p path='%s' "
				"CharRecordLog=%p CharLog=%p\n",
				(unsigned long)GetCurrentThreadId(),
				(void *)BillSql.hLogSqlDLL,
				modulePath,
				(void *)BillSql.CharRecordLog,
				(void *)BillSql.CharLog
			);

			fflush(fp);
			fclose(fp);
		}
	}

	Server_SqlReady = TRUE;

	while(1)
	{
		SuspendThread(hThread);


		for(cnt = LogSqlSendCount; cnt < LogSqlDataCount; cnt++)
		{
			EnterCriticalSection(&cLogSqlSection);
			memcpy(&logSqlData, &LogSqlData[cnt&LOG_SQL_BUFF_MASK], sizeof(sLOG_SQLDATA));
			LogSqlData[cnt&LOG_SQL_BUFF_MASK].dwRecCode = -1;
			LogSqlSendCount++;
			LeaveCriticalSection(&cLogSqlSection);

			if(rsServerConfig.Disable_LogQue) continue;

			dwProcessTime = GetCurrentTime();


			QueSz = LogSqlDataCount - LogSqlSendCount;
			if(QueSz > LogSqlProcessQueMax) LogSqlProcessQueMax = QueSz;

			switch(logSqlData.dwRecCode)
			{
			case LOG_SQL_LOGIN:

				EnterCriticalSection(&cSqlSection);
				if(hDebugWnd) PostMessage(hDebugWnd, SWM_DISPDEBUG, 1000 + logSqlData.dwRecCode, dwPlayServTime);

				//SQLLoginProcess(logSqlData.szUserID);

				if(hDebugWnd) PostMessage(hDebugWnd, SWM_DISPDEBUG, 0, dwPlayServTime);
				LeaveCriticalSection(&cSqlSection);
				break;

			case LOG_SQL_NEWCHAR:

				EnterCriticalSection(&cSqlSection);
				if(hDebugWnd) PostMessage(hDebugWnd, SWM_DISPDEBUG, 1000 + logSqlData.dwRecCode, dwPlayServTime);

				//SQLAddCharProcess(logSqlData.szUserID, logSqlData.szCharName);

				if(!TrySQLiteLocalCharacterAudit(logSqlData, 1))
				{
				if(BillSql.CharRecordLog)
				{

					{
						FILE *fp = fopen("LOGS\\mingw_logsql_worker.txt", "a");

						if(fp)
						{
							fprintf(
								fp,
								"[NEWCHAR-BEFORE] tid=%lu fn=%p "
								"id='%s' char='%s' occupation=%d "
								"level=%lu exp=%lld money=%lu\n",
								(unsigned long)GetCurrentThreadId(),
								(void *)BillSql.CharRecordLog,
								logSqlData.szUserID,
								logSqlData.szCharName,
								logSqlData.Occupation,
								(unsigned long)logSqlData.Level,
								(long long)logSqlData.Exp64,
								(unsigned long)logSqlData.dwMoney
							);
							fflush(fp);
							fclose(fp);
						}

						int mingwCharRecordResult =
							BillSql.CharRecordLog(
								logSqlData.szUserID,
								logSqlData.Occupation,
								logSqlData.szCharName,
								logSqlData.szGPCode,
								logSqlData.szIP,
								szServerName,
								logSqlData.Level,
								logSqlData.Exp64,
								logSqlData.dwMoney,
								1
							);

						fp = fopen("LOGS\\mingw_logsql_worker.txt", "a");

						if(fp)
						{
							fprintf(
								fp,
								"[NEWCHAR-AFTER] tid=%lu result=%d\n",
								(unsigned long)GetCurrentThreadId(),
								mingwCharRecordResult
							);
							fflush(fp);
							fclose(fp);
						}
					}
				}
				else if(BillSql.CharLog)
				{

					BillSql.CharLog(logSqlData.szUserID, logSqlData.Occupation, logSqlData.szCharName, logSqlData.szGPCode, logSqlData.szIP, szServerName, logSqlData.Level, logSqlData.dwExpPoint, logSqlData.dwMoney, 1);
				}

				}

				if(hDebugWnd) PostMessage(hDebugWnd, SWM_DISPDEBUG, 0, dwPlayServTime);
				LeaveCriticalSection(&cSqlSection);
				break;

			case LOG_SQL_DELCHAR:
				EnterCriticalSection(&cSqlSection);
				if(hDebugWnd) PostMessage(hDebugWnd, SWM_DISPDEBUG, 1000 + logSqlData.dwRecCode, dwPlayServTime);

				//SQLDelCharProcess(logSqlData.szUserID, logSqlData.szCharName);

				if(!TrySQLiteLocalCharacterAudit(logSqlData, 2))
				{
				if(BillSql.CharRecordLog)
				{

					BillSql.CharRecordLog(logSqlData.szUserID, logSqlData.Occupation, logSqlData.szCharName, logSqlData.szGPCode, logSqlData.szIP, szServerName, logSqlData.Level, logSqlData.Exp64, logSqlData.dwMoney, 2);
				}
				else if(BillSql.CharLog)
				{

					BillSql.CharLog(logSqlData.szUserID, logSqlData.Occupation, logSqlData.szCharName, logSqlData.szGPCode, logSqlData.szIP, szServerName, logSqlData.Level, logSqlData.dwExpPoint, logSqlData.dwMoney, 2);
				}

				}

				if(hDebugWnd) PostMessage(hDebugWnd, SWM_DISPDEBUG, 0, dwPlayServTime);
				LeaveCriticalSection(&cSqlSection);
				break;

			case LOG_SQL_LOGOUT:
				EnterCriticalSection(&cSqlSection);

				if(hDebugWnd)
					PostMessage(hDebugWnd, SWM_DISPDEBUG, 1000 + logSqlData.dwRecCode, dwPlayServTime);

				//SQLLogoutProcess(logSqlData.szUserID, logSqlData.szCharName, logSqlData.szRace, logSqlData.szOccupation, logSqlData.dwExpPoint, logSqlData.Level);


				if(!TrySQLiteLocalCharacterAudit(logSqlData, 3))
				{
				if(BillSql.CharRecordLog)
				{
					BillSql.CharRecordLog(logSqlData.szUserID, logSqlData.Occupation, logSqlData.szCharName, logSqlData.szGPCode, logSqlData.szIP, szServerName, logSqlData.Level, logSqlData.Exp64, logSqlData.dwMoney, 3);
				}
				else if(BillSql.CharLog)
				{

					BillSql.CharLog(logSqlData.szUserID, logSqlData.Occupation, logSqlData.szCharName, logSqlData.szGPCode, logSqlData.szIP, szServerName, logSqlData.Level, logSqlData.dwExpPoint, logSqlData.dwMoney, 3);
				}

				}

				if(hDebugWnd)
					PostMessage(hDebugWnd, SWM_DISPDEBUG, 0, dwPlayServTime);

				LeaveCriticalSection(&cSqlSection);
				break;

			case GAMELOG_SQL_LOGIN:
				break;

			case GAMELOG_SQL_LOGOUT:
				break;

			case BILLING_SQL_LOGOUT:
				EnterCriticalSection(&cSqlSection);
				if(hDebugWnd) PostMessage(hDebugWnd, SWM_DISPDEBUG, 1000 + logSqlData.dwRecCode, dwPlayServTime);

				if(!TrySQLiteLocalLogout(logSqlData.szUserID, logSqlData.szGuid) && BillSql.SQLLogout)
				{
					BillSql.SQLLogout(logSqlData.szUserID, logSqlData.szCharName, logSqlData.szIP, logSqlData.szGPCode,
						logSqlData.DisuseDay, logSqlData.UsePeriod, logSqlData.szGuid, logSqlData.PC_RNo);
				}

				if(hDebugWnd) PostMessage(hDebugWnd, SWM_DISPDEBUG, 0, dwPlayServTime);
				LeaveCriticalSection(&cSqlSection);
				break;

			case BILLING_SQL_EVENT_LOGOUT:

				if(BillSql.EventSQLLogout)
				{
					EnterCriticalSection(&cSqlSection);

					result = BillSql.EventSQLLogout(
						logSqlData.szUserID, logSqlData.szCharName, logSqlData.szIP, logSqlData.szGPCode,
						logSqlData.DisuseDay, logSqlData.UsePeriod, logSqlData.szGuid, logSqlData.PC_RNo,
						logSqlData2.szGPCode, logSqlData2.szGuid);

					if(result == 1 && logSqlData.lpPlayInfo && logSqlData.lpPlayInfo->lpsmSock && logSqlData.lpPlayInfo->dwConnectCount == logSqlData.dwConnectCode)
					{

						GpCode = bSql_GetGPCode(logSqlData2.szGPCode, szPG);
						logSqlData.lpPlayInfo->GpType = atoi(szPG);
						logSqlData.lpPlayInfo->Bl_RNo = 0;

						strcpy_s(logSqlData.lpPlayInfo->szGPCode, logSqlData2.szGPCode);
						strcpy_s(logSqlData.lpPlayInfo->szGuid, logSqlData2.szGuid);
					}
					LeaveCriticalSection(&cSqlSection);


					if(result == 1)
					{
						EnterCriticalSection(&cLogSqlSection);
						for(cnt = LogSqlSendCount; cnt < LogSqlDataCount; cnt++)
						{
							mcnt = cnt&LOG_SQL_BUFF_MASK;
							if(LogSqlData[mcnt].dwRecCode == BILLING_SQL_LOGOUT)
							{
								if(lstrcmpi(LogSqlData[mcnt].szUserID, logSqlData.szUserID) == 0)
								{
									strcpy_s(LogSqlData[mcnt].szGuid, logSqlData2.szGuid);
									strcpy_s(LogSqlData[mcnt].szGPCode, logSqlData2.szGPCode);
									break;
								}
							}
						}
						LeaveCriticalSection(&cLogSqlSection);
					}
				}
				break;


			case BILLING_SQL_PCUSE:
				if(BillSql.PCUse)
				{
					EnterCriticalSection(&cSqlSection);
					if(hDebugWnd) PostMessage(hDebugWnd, SWM_DISPDEBUG, 1000 + logSqlData.dwRecCode, dwPlayServTime);
					result = BillSql.PCUse(logSqlData.szUserID, logSqlData.szGPCode, rsServerConfig.szServerName, logSqlData.szIP, logSqlData.PC_RNo, logSqlData.szGuid);
					if(hDebugWnd) PostMessage(hDebugWnd, SWM_DISPDEBUG, 0, dwPlayServTime);
					LeaveCriticalSection(&cSqlSection);

					EnterCriticalSection(&cSerSection);

					if(logSqlData.lpPlayInfo && logSqlData.lpPlayInfo->lpsmSock && logSqlData.lpPlayInfo->dwConnectCount == logSqlData.dwConnectCode)
					{

						if((result & 0xFF000000) == 0x40000000)
						{
							dwPeriodTime2 = result & 0x00FFFFFF;
							logSqlData.lpPlayInfo->dwPeriodTime2 = (DWORD)tServerTime + dwPeriodTime2;
						}
						else
						{
							if(logSqlData.lpPlayInfo->DisuseDay)
							{
								logSqlData.lpPlayInfo->DisuseDay = result;
							}
							else
							{
								dwTime = (DWORD)tServerTime + result;
								logSqlData.lpPlayInfo->dwPeriodTime = dwTime;
							}
						}






					}
					LeaveCriticalSection(&cSerSection);
				}
				break;

			case BILLING_URS_EXPINED:

				break;

			case BILLING_SQL_PC_SRESERV:
				if(BillSql.PCSearchReserv)
				{

					EnterCriticalSection(&cSqlSection);
					if(hDebugWnd) PostMessage(hDebugWnd, SWM_DISPDEBUG, 1000 + logSqlData.dwRecCode, dwPlayServTime);

					result = BillSql.PCSearchReserv(logSqlData.szUserID, logSqlData.PC_RNo, logSqlData.szGPCode, logSqlData.szIP,
						(char *)&logSqlData2.PC_RNo, logSqlData2.szGPCode, (char *)&logSqlData2.DisuseDay, (char *)&logSqlData2.UsePeriod, logSqlData.szGuid);

					if((result & 0xFF000000) == 0x40000000)
					{
						dwPeriodTime2 = result & 0x00FFFFFF;
						result = 1;
					}
					else
						dwPeriodTime2 = 0;


					if(result == 1) GpCode = bSql_GetGPCode(logSqlData2.szGPCode, szPG);

					if(hDebugWnd) PostMessage(hDebugWnd, SWM_DISPDEBUG, 0, dwPlayServTime);
					LeaveCriticalSection(&cSqlSection);

					EnterCriticalSection(&cSerSection);


					if(result == 1 && logSqlData.lpPlayInfo && logSqlData.lpPlayInfo->lpsmSock && logSqlData.lpPlayInfo->dwConnectCount == logSqlData.dwConnectCode)
					{

						logSqlData.lpPlayInfo->GpCode = GpCode;

						if(logSqlData.lpPlayInfo->GpCode >= 0)
						{
							strcpy_s(logSqlData.lpPlayInfo->szGPCode, logSqlData2.szGPCode);
							logSqlData.lpPlayInfo->DisuseDay = logSqlData2.DisuseDay;
							logSqlData.lpPlayInfo->UsePeriod = logSqlData2.UsePeriod;
							logSqlData.lpPlayInfo->GpType = atoi(szPG);
							logSqlData.lpPlayInfo->Bl_RNo = logSqlData2.PC_RNo;

							if(logSqlData.lpPlayInfo->UsePeriod)
							{
								logSqlData.lpPlayInfo->dwPeriodTime = (DWORD)tServerTime + logSqlData.lpPlayInfo->UsePeriod;
							}
							else
							{
								logSqlData.lpPlayInfo->dwPeriodTime = 0;
							}

							if(dwPeriodTime2)
								logSqlData.lpPlayInfo->dwPeriodTime2 = (DWORD)tServerTime + dwPeriodTime2;
							else
								logSqlData.lpPlayInfo->dwPeriodTime2 = 0;

						}

						if(logSqlData.lpPlayInfo->Bl_RNo > 0)
							bSql_PCRNoReservProduct(logSqlData.lpPlayInfo);
						else
							srDisplayBillingTime(logSqlData.lpPlayInfo, 0);

					}
					LeaveCriticalSection(&cSerSection);
				}
				break;

			case BILLING_SQL_BILL_CHANGE:

				if(BillSql.ChangeUseridGPCodeGame)
				{

					EnterCriticalSection(&cSqlSection);
					if(hDebugWnd) PostMessage(hDebugWnd, SWM_DISPDEBUG, 1000 + logSqlData.dwRecCode, dwPlayServTime);

					result = BillSql.ChangeUseridGPCodeGame(logSqlData.PC_RNo, logSqlData2.szUserID, logSqlData2.szGPCode,
						(char *)&logSqlData2.DisuseDay, (char *)&logSqlData2.UsePeriod, logSqlData.szGuid);

					if(result == 1) GpCode = bSql_GetGPCode(logSqlData2.szGPCode, szPG);

					if(hDebugWnd) PostMessage(hDebugWnd, SWM_DISPDEBUG, 0, dwPlayServTime);
					LeaveCriticalSection(&cSqlSection);

					EnterCriticalSection(&cSerSection);


					if(result == 1)
					{
						dwIdCode = GetSpeedSum(logSqlData2.szUserID);
						for(cnt = 0; cnt < CONNECTMAX; cnt++)
						{
							//if(rsPlayInfo[cnt].lpsmSock && rsPlayInfo[cnt].username[0] && rsPlayInfo[cnt].dwCode_ID == dwIdCode)
							if(rsPlayInfo[cnt].lpsmSock && strlen(rsPlayInfo[cnt].getAccount()) && rsPlayInfo[cnt].dwCode_ID == dwIdCode)
							{
								if(lstrcmpi(logSqlData2.szUserID, rsPlayInfo[cnt].getAccount()) == 0)
								{

									rsPlayInfo[cnt].GpCode = GpCode;
									if(rsPlayInfo[cnt].GpCode >= 0)
									{

										strcpy_s(rsPlayInfo[cnt].szGPCode, logSqlData2.szGPCode);
										strcpy_s(rsPlayInfo[cnt].szGuid, logSqlData.szGuid);
										rsPlayInfo[cnt].DisuseDay = logSqlData2.DisuseDay;
										rsPlayInfo[cnt].UsePeriod = logSqlData2.UsePeriod;
										rsPlayInfo[cnt].GpType = atoi(szPG);

										if(rsPlayInfo[cnt].UsePeriod)
										{
											rsPlayInfo[cnt].dwPeriodTime = (DWORD)tServerTime + logSqlData.lpPlayInfo->UsePeriod;
										}
										else
										{
											rsPlayInfo[cnt].dwPeriodTime = 0;
										}
									}

									srDisplayBillingTime(&rsPlayInfo[cnt], 0);
									break;
								}
							}
						}
					}
					LeaveCriticalSection(&cSerSection);
				}
				break;

			case BILLING_SQL_EVENT_LOGIN:

				NextFlag = 0;
				if(BillSql.EventSQLLogon)
				{

					EnterCriticalSection(&cSqlSection);
					if(hDebugWnd) PostMessage(hDebugWnd, SWM_DISPDEBUG, 1000 + logSqlData.dwRecCode, dwPlayServTime);

					result = BillSql.EventSQLLogon(logSqlData.szUserID, "*FreeLevel", logSqlData.szIP,
						logSqlData2.szGPCode, (char *)&logSqlData2.DisuseDay, (char *)&logSqlData2.UsePeriod,
						logSqlData2.szGuid, (char *)&logSqlData2.PC_RNo, (char *)&MeterCode);

					if(result == 1) GpCode = bSql_GetGPCode(logSqlData2.szGPCode, szPG);

					if(hDebugWnd) PostMessage(hDebugWnd, SWM_DISPDEBUG, 0, dwPlayServTime);
					LeaveCriticalSection(&cSqlSection);

					EnterCriticalSection(&cSerSection);



					if(logSqlData.lpPlayInfo && logSqlData.lpPlayInfo->lpsmSock && logSqlData.lpPlayInfo->dwConnectCount == logSqlData.dwConnectCode)
					{
						if(result >= 1)
						{

							logSqlData.lpPlayInfo->GpCode = GpCode;

							if(logSqlData.lpPlayInfo->GpCode >= 0)
							{
								strcpy_s(logSqlData.lpPlayInfo->szGPCode, logSqlData2.szGPCode);
								strcpy_s(logSqlData.lpPlayInfo->szGuid, logSqlData2.szGuid);

								logSqlData.lpPlayInfo->DisuseDay = logSqlData2.DisuseDay;
								logSqlData.lpPlayInfo->UsePeriod = logSqlData2.UsePeriod;
								logSqlData.lpPlayInfo->GpType = atoi(szPG);
								logSqlData.lpPlayInfo->Bl_RNo = logSqlData2.PC_RNo;

								if(logSqlData.lpPlayInfo->UsePeriod)
								{
									logSqlData.lpPlayInfo->dwPeriodTime = (DWORD)tServerTime + logSqlData.lpPlayInfo->UsePeriod;
								}
								else
								{
									logSqlData.lpPlayInfo->dwPeriodTime = 0;
								}
								logSqlData.lpPlayInfo->Bl_Meter = MeterCode;


								rsEventLogOn_Sucess(logSqlData.lpPlayInfo);
							}

							if(logSqlData.lpPlayInfo->Bl_RNo > 0)
							{

								logSqlData.PC_RNo = logSqlData2.PC_RNo;
								strcpy_s(logSqlData.szGPCode, logSqlData2.szGPCode);
								NextFlag = TRUE;
							}
							else
							{
								srDisplayBillingTime(logSqlData.lpPlayInfo, 0);
							}
						}
						else
						{
							logSqlData.lpPlayInfo->DisuseDay = tServerTime + 60 * 5;
							logSqlData.lpPlayInfo->UsePeriod = 0;
							logSqlData.lpPlayInfo->dwPeriodTime = 0;
							logSqlData.lpPlayInfo->Bl_Meter = 0;
							srDisplayBillingTime(logSqlData.lpPlayInfo, 0);
						}
					}

					LeaveCriticalSection(&cSerSection);

				}
				if(NextFlag == 0) break;

			case BILLING_SQL_PC_TIME:
				if(BillSql.PCRNoReservProduct)
				{

					EnterCriticalSection(&cSqlSection);
					if(hDebugWnd) PostMessage(hDebugWnd, SWM_DISPDEBUG, 1000 + logSqlData.dwRecCode, dwPlayServTime);

					result = BillSql.PCRNoReservProduct(logSqlData.PC_RNo, logSqlData.szGPCode);

					if(hDebugWnd) PostMessage(hDebugWnd, SWM_DISPDEBUG, 0, dwPlayServTime);
					LeaveCriticalSection(&cSqlSection);

					EnterCriticalSection(&cSerSection);
					if(logSqlData.lpPlayInfo && logSqlData.lpPlayInfo->lpsmSock && logSqlData.lpPlayInfo->dwConnectCount == logSqlData.dwConnectCode)
					{
						srDisplayBillingTime(logSqlData.lpPlayInfo, result);
					}
					LeaveCriticalSection(&cSerSection);
				}
				break;


			case EGT_GET_EXPGAMETIME:
				break;
			}

			dwProcessTime = GetCurrentTime() - dwProcessTime;
			if(dwProcessTime)
			{
				if(LogSqlProcessTimeMax < dwProcessTime) LogSqlProcessTimeMax = dwProcessTime;
				LogSqlProcessTime += dwProcessTime;
				LogSqlProcessCount++;
			}

		}
	}

	if(mingwLogSqlComInitialized)
		CoUninitialize();

	ExitThread(TRUE);
	return TRUE;
}


int InitLogSql()
{
	LogSqlData = new sLOG_SQLDATA[LOG_SQL_BUFF_MAX];
	LogSqlDataCount = 0;
	LogSqlSendCount = 0;

	LogItemQue = new LogITEM_QUE[LOG_ITEM_QUE_MAX];
	LogItemQuePush = 0;
	LogItemQuePop = 0;

	LogEventQue = new LogEVENT_QUE[LOG_EVENT_QUE_MAX];
	LogEventQuePush = 0;
	LogEventQuePop = 0;




	InitializeCriticalSection(&cLogSqlSection);

	hLogSqlThread = CreateThread(NULL, 0, LogSqlDataThreadProc, 0, 0, &dwLogSqlThreadId);

	if(rsServerConfig.szServerName[0])
	{
		strcpy_s(szServerName, rsServerConfig.szServerName);
	}

	return TRUE;
}

int CloseLogSql()
{
	if(LogEventQue)
	{
		delete[] LogEventQue;
		LogEventQue = 0;
	}

	if(LogItemQue)
	{
		delete[] LogItemQue;
		LogItemQue = 0;
	}

	if(LogSqlData)
	{
		delete[] LogSqlData;
		LogSqlData = 0;
	}


	return TRUE;
}

int RecordLogSql(DWORD	dwRecCode, char *szIP, const char *lpUserID, const char *lpCharName, char *szGPCode, int Race, int JobCode, INT64 Exp64, DWORD dwLevel, DWORD dwMoney)
{
	int cnt;

	if((LogSqlDataCount - LogSqlSendCount) >= LOG_SQL_BUFF_LIMIT)
	{
		LogSqlLossCount++;
		return FALSE;
	}

	EnterCriticalSection(&cLogSqlSection);

	cnt = LogSqlDataCount & LOG_SQL_BUFF_MASK;

	LogSqlData[cnt].dwRecCode = dwRecCode;

	if(lpUserID)
		strcpy_s(LogSqlData[cnt].szUserID, lpUserID);
	else
		LogSqlData[cnt].szUserID[0] = 0;

	if(lpCharName)
		strcpy_s(LogSqlData[cnt].szCharName, lpCharName);
	else
		LogSqlData[cnt].szCharName[0] = 0;

	LogSqlData[cnt].szOccupation[0] = 0;
	LogSqlData[cnt].Occupation = 0;

	switch(JobCode)
	{
	case 1:
		strcpy_s(LogSqlData[cnt].szOccupation, "Lutador");
		LogSqlData[cnt].Occupation = 2;
		break;
	case 2:
		strcpy_s(LogSqlData[cnt].szOccupation, "Mec\342nico");
		LogSqlData[cnt].Occupation = 1;
		break;
	case 3:
		strcpy_s(LogSqlData[cnt].szOccupation, "Arqueiro");
		LogSqlData[cnt].Occupation = 4;
		break;
	case 4:
		strcpy_s(LogSqlData[cnt].szOccupation, "Pike");
		LogSqlData[cnt].Occupation = 3;
		break;
	case 5:
		strcpy_s(LogSqlData[cnt].szOccupation, "Atalanta");
		LogSqlData[cnt].Occupation = 7;
		break;
	case 6:
		strcpy_s(LogSqlData[cnt].szOccupation, "Cavaleiro");
		LogSqlData[cnt].Occupation = 6;
		break;
	case 7:
		strcpy_s(LogSqlData[cnt].szOccupation, "Mago");
		LogSqlData[cnt].Occupation = 5;
		break;
	case 8:
		strcpy_s(LogSqlData[cnt].szOccupation, "Sacerdotisa");
		LogSqlData[cnt].Occupation = 8;
		break;
	}


	LogSqlData[cnt].szRace[0] = 0;

	const gameplay::classes::ClassService &classService = gameplay::classes::DefaultClassService();
	if(classService.IsInBrood(JobCode, gameplay::classes::brood::Tempskron))
		strcpy_s(LogSqlData[cnt].szRace, "Tempskron");
	else if(classService.IsInBrood(JobCode, gameplay::classes::brood::Morayion))
		strcpy_s(LogSqlData[cnt].szRace, "Moryon");
	else if(classService.IsInBrood(JobCode, gameplay::classes::brood::Sopphetio))
		strcpy_s(LogSqlData[cnt].szRace, "Sopphetio");
	else
		strcpy_s(LogSqlData[cnt].szRace, "Unknown");

	strcpy_s(LogSqlData[cnt].szIP, szIP);

	if(szGPCode)
		strcpy_s(LogSqlData[cnt].szGPCode, szGPCode);
	else
		LogSqlData[cnt].szGPCode[0] = 0;



	LogSqlData[cnt].dwMoney = dwMoney;
	LogSqlData[cnt].dwExpPoint = (DWORD)Exp64;
	LogSqlData[cnt].Exp64 = Exp64;
	LogSqlData[cnt].Level = dwLevel;

	LogSqlDataCount++;
	LeaveCriticalSection(&cLogSqlSection);

	ResumeThread(hLogSqlThread);

	return TRUE;
}


int RecordGameLogSql(DWORD dwRecCode, char *szIP, const char *lpUserID, const char *lpCharName, char *szGPCode, int JobCode, char *szGuid, int BlockTime)
{
	if((LogSqlDataCount - LogSqlSendCount) >= LOG_SQL_BUFF_LIMIT)
	{
		LogSqlLossCount++;
		return FALSE;
	}

	EnterCriticalSection(&cLogSqlSection);

	int cnt = LogSqlDataCount & LOG_SQL_BUFF_MASK;

	LogSqlData[cnt].dwRecCode = dwRecCode;
	strcpy_s(LogSqlData[cnt].szIP, szIP);

	if(lpUserID)	strcpy_s(LogSqlData[cnt].szUserID, lpUserID);
	else LogSqlData[cnt].szUserID[0] = 0;

	if(lpCharName)	strcpy_s(LogSqlData[cnt].szCharName, lpCharName);
	else LogSqlData[cnt].szCharName[0] = 0;

	LogSqlData[cnt].szOccupation[0] = 0;
	LogSqlData[cnt].Occupation = 0;

	LogSqlData[cnt].Time = BlockTime;

	switch(JobCode)
	{
	case 1:
		strcpy_s(LogSqlData[cnt].szOccupation, "Lutador");
		LogSqlData[cnt].Occupation = 2;
		break;
	case 2:
		strcpy_s(LogSqlData[cnt].szOccupation, "Mec\342nico");
		LogSqlData[cnt].Occupation = 1;
		break;
	case 3:
		strcpy_s(LogSqlData[cnt].szOccupation, "Arqueira");
		LogSqlData[cnt].Occupation = 4;
		break;
	case 4:
		strcpy_s(LogSqlData[cnt].szOccupation, "Pike");
		LogSqlData[cnt].Occupation = 3;
		break;
	case 5:
		strcpy_s(LogSqlData[cnt].szOccupation, "Atalanta");
		LogSqlData[cnt].Occupation = 7;
		break;
	case 6:
		strcpy_s(LogSqlData[cnt].szOccupation, "Cavaleiro");
		LogSqlData[cnt].Occupation = 6;
		break;
	case 7:
		strcpy_s(LogSqlData[cnt].szOccupation, "Mago");
		LogSqlData[cnt].Occupation = 5;
		break;
	case 8:
		strcpy_s(LogSqlData[cnt].szOccupation, "Sacerdotisa");
		LogSqlData[cnt].Occupation = 8;
		break;
	}


	LogSqlData[cnt].szRace[0] = 0;

	const gameplay::classes::ClassService &classService = gameplay::classes::DefaultClassService();
	if(classService.IsInBrood(JobCode, gameplay::classes::brood::Tempskron))
		strcpy_s(LogSqlData[cnt].szRace, "Tempskron");
	else if(classService.IsInBrood(JobCode, gameplay::classes::brood::Morayion))
		strcpy_s(LogSqlData[cnt].szRace, "Moryon");
	else if(classService.IsInBrood(JobCode, gameplay::classes::brood::Sopphetio))
		strcpy_s(LogSqlData[cnt].szRace, "Sopphetio");
	else
		strcpy_s(LogSqlData[cnt].szRace, "Unknown");


	if(szGuid)  strcpy_s(LogSqlData[cnt].szGuid, szGuid);
	else  LogSqlData[cnt].szGuid[0] = 0;

	if(szGPCode) strcpy_s(LogSqlData[cnt].szGPCode, szGPCode);
	else LogSqlData[cnt].szGPCode[0] = 0;


	LogSqlDataCount++;
	LeaveCriticalSection(&cLogSqlSection);

	ResumeThread(hLogSqlThread);

	return TRUE;
}







int	bSql_Logout(const char *lpszPID, const char *lpszCID, char *lpszIP, char *lpszGPCode, time_t DisuseDay, int UsePeriod, char *szGuid, int PCRNo)
{


	int cnt;

	if((LogSqlDataCount - LogSqlSendCount) >= LOG_SQL_BUFF_MASK)
	{
		LogSqlLossCount++;
		return FALSE;
	}

	EnterCriticalSection(&cLogSqlSection);

	cnt = LogSqlDataCount & LOG_SQL_BUFF_MASK;

	LogSqlData[cnt].dwRecCode = BILLING_SQL_LOGOUT;
	strcpy_s(LogSqlData[cnt].szIP, lpszIP);

	if(lpszPID)	strcpy_s(LogSqlData[cnt].szUserID, lpszPID);
	else LogSqlData[cnt].szUserID[0] = 0;

	if(lpszCID)	strcpy_s(LogSqlData[cnt].szCharName, lpszCID);
	else LogSqlData[cnt].szCharName[0] = 0;

	LogSqlData[cnt].DisuseDay = DisuseDay;
	LogSqlData[cnt].UsePeriod = UsePeriod;

	if(szGuid) strcpy_s(LogSqlData[cnt].szGuid, szGuid);
	else LogSqlData[cnt].szGuid[0] = 0;

	if(lpszGPCode) strcpy_s(LogSqlData[cnt].szGPCode, lpszGPCode);
	else LogSqlData[cnt].szGPCode[0] = 0;

	LogSqlData[cnt].PC_RNo = PCRNo;

	LogSqlDataCount++;
	LeaveCriticalSection(&cLogSqlSection);

	ResumeThread(hLogSqlThread);

	return TRUE;

}


int	bSql_GetQueLogOut(char *szID)
{
	int cnt, mcnt;
	int Flag = 0;

	EnterCriticalSection(&cLogSqlSection);
	for(cnt = LogSqlSendCount; cnt < LogSqlDataCount; cnt++)
	{
		mcnt = cnt&LOG_SQL_BUFF_MASK;
		if(LogSqlData[mcnt].dwRecCode == BILLING_SQL_LOGOUT)
		{
			if(lstrcmpi(LogSqlData[mcnt].szUserID, szID) == 0)
			{
				Flag = TRUE;
				break;
			}
		}
	}
	LeaveCriticalSection(&cLogSqlSection);

	return Flag;
}

int	bSql_GetGPCode(char *GPCode, char *PG)
{
	/* Local/free SQLite accounts deliberately carry no billing GPCode. */
	if(IsSQLiteLocalZeroValue(GPCode))
	{
		if(PG)
		{
			PG[0] = '0';
			PG[1] = 0;
		}
		return 0;
	}

	if(BillSql.GetGPCode)
	{
		return BillSql.GetGPCode(GPCode, PG);
	}
	return -1;
}



int	bSql_PCUse(rsPLAYINFO *lpPlayInfo)
{
	int cnt;

	if((LogSqlDataCount - LogSqlSendCount) >= LOG_SQL_BUFF_LIMIT)
	{
		LogSqlLossCount++;
		return FALSE;
	}

	EnterCriticalSection(&cLogSqlSection);

	cnt = LogSqlDataCount & LOG_SQL_BUFF_MASK;

	LogSqlData[cnt].dwRecCode = BILLING_SQL_PCUSE;

	//strcpy_s(LogSqlData[cnt].szUserID, lpPlayInfo->getUsername().c_str());
	strcpy_s(LogSqlData[cnt].szUserID, lpPlayInfo->getAccount());
	strcpy_s(LogSqlData[cnt].szGPCode, lpPlayInfo->szGPCode);
	strcpy_s(LogSqlData[cnt].szIP, lpPlayInfo->lpsmSock->szIPAddr);
	strcpy_s(LogSqlData[cnt].szGuid, lpPlayInfo->szGuid);

	LogSqlData[cnt].PC_RNo = lpPlayInfo->Bl_RNo;


	LogSqlData[cnt].lpPlayInfo = lpPlayInfo;
	LogSqlData[cnt].dwConnectCode = lpPlayInfo->dwConnectCount;


	LogSqlDataCount++;
	LeaveCriticalSection(&cLogSqlSection);

	ResumeThread(hLogSqlThread);


	return TRUE;
}

int	bUrs_Expined(rsPLAYINFO *lpPlayInfo)
{
	int cnt;

	if((LogSqlDataCount - LogSqlSendCount) >= LOG_SQL_BUFF_LIMIT)
	{
		LogSqlLossCount++;
		return FALSE;
	}

	EnterCriticalSection(&cLogSqlSection);

	cnt = LogSqlDataCount & LOG_SQL_BUFF_MASK;

	LogSqlData[cnt].dwRecCode = BILLING_URS_EXPINED;

	//strcpy_s(LogSqlData[cnt].szUserID, lpPlayInfo->getUsername().c_str());
	strcpy_s(LogSqlData[cnt].szUserID, lpPlayInfo->getAccount());
	strcpy_s(LogSqlData[cnt].szIP, lpPlayInfo->lpsmSock->szIPAddr);


	LogSqlData[cnt].lpPlayInfo = lpPlayInfo;
	LogSqlData[cnt].dwConnectCode = lpPlayInfo->dwConnectCount;


	LogSqlDataCount++;
	LeaveCriticalSection(&cLogSqlSection);

	ResumeThread(hLogSqlThread);


	return TRUE;
}




int	bSql_PCSearchReserv(rsPLAYINFO *lpPlayInfo)
{
	int cnt;

	if((LogSqlDataCount - LogSqlSendCount) >= LOG_SQL_BUFF_LIMIT)
	{
		LogSqlLossCount++;
		return FALSE;
	}

	EnterCriticalSection(&cLogSqlSection);

	cnt = LogSqlDataCount & LOG_SQL_BUFF_MASK;

	LogSqlData[cnt].dwRecCode = BILLING_SQL_PC_SRESERV;

	//strcpy_s(LogSqlData[cnt].szUserID, lpPlayInfo->getUsername().c_str());
	strcpy_s(LogSqlData[cnt].szUserID, lpPlayInfo->getAccount());
	strcpy_s(LogSqlData[cnt].szGPCode, lpPlayInfo->szGPCode);
	strcpy_s(LogSqlData[cnt].szIP, lpPlayInfo->lpsmSock->szIPAddr);
	strcpy_s(LogSqlData[cnt].szGuid, lpPlayInfo->szGuid);

	LogSqlData[cnt].PC_RNo = lpPlayInfo->Bl_RNo;


	LogSqlData[cnt].lpPlayInfo = lpPlayInfo;
	LogSqlData[cnt].dwConnectCode = lpPlayInfo->dwConnectCount;


	LogSqlDataCount++;
	LeaveCriticalSection(&cLogSqlSection);

	ResumeThread(hLogSqlThread);

	return TRUE;
}


int	bSql_ChangeUserIdGpCodeGame(int PCRNo)
{
	int cnt;

	if((LogSqlDataCount - LogSqlSendCount) >= LOG_SQL_BUFF_LIMIT)
	{
		LogSqlLossCount++;
		return FALSE;
	}

	EnterCriticalSection(&cLogSqlSection);

	cnt = LogSqlDataCount & LOG_SQL_BUFF_MASK;

	LogSqlData[cnt].dwRecCode = BILLING_SQL_BILL_CHANGE;

	LogSqlData[cnt].PC_RNo = PCRNo;

	LogSqlDataCount++;
	LeaveCriticalSection(&cLogSqlSection);

	ResumeThread(hLogSqlThread);

	return TRUE;
}


int	bSql_EventLogon(rsPLAYINFO *lpPlayInfo)
{
	int cnt;

	if((LogSqlDataCount - LogSqlSendCount) >= LOG_SQL_BUFF_LIMIT)
	{
		LogSqlLossCount++;
		return FALSE;
	}

	EnterCriticalSection(&cLogSqlSection);

	cnt = LogSqlDataCount & LOG_SQL_BUFF_MASK;

	LogSqlData[cnt].dwRecCode = BILLING_SQL_EVENT_LOGIN;

	//strcpy_s(LogSqlData[cnt].szUserID, lpPlayInfo->getUsername().c_str());
	strcpy_s(LogSqlData[cnt].szUserID, lpPlayInfo->getAccount());
	strcpy_s(LogSqlData[cnt].szIP, lpPlayInfo->lpsmSock->szIPAddr);


	LogSqlData[cnt].lpPlayInfo = lpPlayInfo;
	LogSqlData[cnt].dwConnectCode = lpPlayInfo->dwConnectCount;

	LogSqlDataCount++;
	LeaveCriticalSection(&cLogSqlSection);

	ResumeThread(hLogSqlThread);

	return TRUE;
}


int	bSql_EventLogOut(rsPLAYINFO *lpPlayInfo)
{
	int cnt;

	if((LogSqlDataCount - LogSqlSendCount) >= LOG_SQL_BUFF_LIMIT)
	{
		LogSqlLossCount++;
		return FALSE;
	}

	EnterCriticalSection(&cLogSqlSection);

	cnt = LogSqlDataCount & LOG_SQL_BUFF_MASK;

	LogSqlData[cnt].dwRecCode = BILLING_SQL_EVENT_LOGOUT;
	strcpy_s(LogSqlData[cnt].szIP, lpPlayInfo->lpsmSock->szIPAddr);

	//strcpy_s(LogSqlData[cnt].szUserID, lpPlayInfo->getUsername().c_str());
	//strcpy_s(LogSqlData[cnt].szCharName, lpPlayInfo->getPassword().c_str());
	strcpy_s(LogSqlData[cnt].szUserID, lpPlayInfo->getAccount());
	strcpy_s(LogSqlData[cnt].szCharName, lpPlayInfo->getName());
	strcpy_s(LogSqlData[cnt].szGuid, lpPlayInfo->szGuid);
	strcpy_s(LogSqlData[cnt].szGPCode, lpPlayInfo->szGPCode);

	LogSqlData[cnt].DisuseDay = lpPlayInfo->DisuseDay;
	LogSqlData[cnt].UsePeriod = lpPlayInfo->UsePeriod;
	LogSqlData[cnt].PC_RNo = lpPlayInfo->Bl_RNo;


	LogSqlData[cnt].lpPlayInfo = lpPlayInfo;
	LogSqlData[cnt].dwConnectCode = lpPlayInfo->dwConnectCount;

	LogSqlDataCount++;
	LeaveCriticalSection(&cLogSqlSection);

	ResumeThread(hLogSqlThread);

	return TRUE;
}



int	bSql_GetExpGameTime(rsPLAYINFO *lpPlayInfo, int GamePlayTime)
{
	int cnt;

	if((LogSqlDataCount - LogSqlSendCount) >= LOG_SQL_BUFF_LIMIT)
	{
		LogSqlLossCount++;
		return FALSE;
	}

	EnterCriticalSection(&cLogSqlSection);

	cnt = LogSqlDataCount & LOG_SQL_BUFF_MASK;

	LogSqlData[cnt].dwRecCode = EGT_GET_EXPGAMETIME;

	strcpy_s(LogSqlData[cnt].szUserID, lpPlayInfo->szUserExpTimeID);
	strcpy_s(LogSqlData[cnt].szCharName, lpPlayInfo->szUserExpTimeName);

	LogSqlData[cnt].Time = GamePlayTime;


	LogSqlData[cnt].lpPlayInfo = lpPlayInfo;
	LogSqlData[cnt].dwConnectCode = lpPlayInfo->dwConnectCount;


	LogSqlDataCount++;
	LeaveCriticalSection(&cLogSqlSection);

	ResumeThread(hLogSqlThread);

	return TRUE;
}




int	bSql_PCRNoReservProduct(rsPLAYINFO *lpPlayInfo)
{
	int cnt;
	int mcnt;

	if((LogSqlDataCount - LogSqlSendCount) >= LOG_SQL_BUFF_LIMIT)
	{
		LogSqlLossCount++;
		return FALSE;
	}

	EnterCriticalSection(&cLogSqlSection);

	for(cnt = LogSqlSendCount; cnt < LogSqlDataCount; cnt++)
	{
		mcnt = cnt&LOG_SQL_BUFF_MASK;
		if(LogSqlData[mcnt].dwRecCode == BILLING_SQL_PC_TIME && LogSqlData[mcnt].lpPlayInfo == lpPlayInfo)
		{
			LeaveCriticalSection(&cLogSqlSection);
			return FALSE;
		}
	}

	cnt = LogSqlDataCount & LOG_SQL_BUFF_MASK;

	LogSqlData[cnt].dwRecCode = BILLING_SQL_PC_TIME;

	//strcpy_s(LogSqlData[cnt].szUserID, lpPlayInfo->getUsername().c_str());
	strcpy_s(LogSqlData[cnt].szUserID, lpPlayInfo->getAccount());
	strcpy_s(LogSqlData[cnt].szGPCode, lpPlayInfo->szGPCode);
	strcpy_s(LogSqlData[cnt].szIP, lpPlayInfo->lpsmSock->szIPAddr);

	LogSqlData[cnt].PC_RNo = lpPlayInfo->Bl_RNo;


	LogSqlData[cnt].lpPlayInfo = lpPlayInfo;
	LogSqlData[cnt].dwConnectCode = lpPlayInfo->dwConnectCount;


	LogSqlDataCount++;
	LeaveCriticalSection(&cLogSqlSection);

	ResumeThread(hLogSqlThread);

	return TRUE;
}


int	bSql_SODRecord(rsPLAYINFO *lpPlayInfo, int Level, int SodScore, int SodKillCount, int SodTotalScore, int SodTotalSucessUser, int SodTotalUser, int SodCode)
{
	sLOG_EVENT sEventData;

	lpPlayInfo->szGPCode[15] = 0;

	strcpy_s(sEventData.szID, lpPlayInfo->getAccount());
	strcpy_s(sEventData.szName, lpPlayInfo->getName());
	strcpy_s(sEventData.szGPCode, lpPlayInfo->szGPCode);
	strcpy_s(sEventData.szIP, lpPlayInfo->lpsmSock->szIPAddr);

	sEventData.Param[0] = lpPlayInfo->smCharInfo.JOB_CODE;
	sEventData.Param[1] = SodScore;
	sEventData.Param[2] = SodKillCount;
	sEventData.Param[3] = Level;
	sEventData.Param[4] = SodTotalScore;
	sEventData.Param[5] = SodTotalUser;
	sEventData.Param[6] = SodTotalSucessUser;
	sEventData.Param[7] = lpPlayInfo->Bl_RNo;
	sEventData.Param[8] = SodCode;


	return b_PushLogEventQue(GAMELOG_SQL_SOD, sizeof(sLOG_EVENT), &sEventData);
}


int	bSql_StarPoint(rsPLAYINFO *lpPlayInfo, int StarPoint, int CashFlag)
{
	sLOG_EVENT sEventData;

	if(!BillSql.UserStar && !BillSql.UserStarRecord) return FALSE;

	ZeroMemory(&sEventData, sizeof(sLOG_EVENT));

	lpPlayInfo->szGPCode[15] = 0;

	strcpy_s(sEventData.szID, lpPlayInfo->getAccount());
	strcpy_s(sEventData.szName, lpPlayInfo->getName());
	strcpy_s(sEventData.szGPCode, lpPlayInfo->szGPCode);
	strcpy_s(sEventData.szIP, lpPlayInfo->lpsmSock->szIPAddr);

	sEventData.Param[0] = StarPoint;
	sEventData.Param[1] = (DWORD)lpPlayInfo;
	sEventData.Param[2] = lpPlayInfo->dwConnectCount;

	sEventData.Param[3] = lpPlayInfo->smCharInfo.JOB_CODE;
	sEventData.Param[4] = lpPlayInfo->smCharInfo.Level;
	sEventData.Param[5] = CashFlag;



	return b_PushLogEventQue(GAMELOG_SQL_STARPOINT, sizeof(sLOG_EVENT), &sEventData);
}


int	bSql_PollRecord(rsPLAYINFO *lpPlayInfo, int PollKind, int PollCount, BYTE *PollData)
{
	sLOG_EVENT sEventData;
	int cnt;

	if(!BillSql.PollRecord) return FALSE;

	ZeroMemory(&sEventData, sizeof(sLOG_EVENT));

	lpPlayInfo->szGPCode[15] = 0;

	strcpy_s(sEventData.szID, lpPlayInfo->getAccount());
	strcpy_s(sEventData.szName, lpPlayInfo->getName());
	strcpy_s(sEventData.szGPCode, lpPlayInfo->szGPCode);
	strcpy_s(sEventData.szIP, lpPlayInfo->lpsmSock->szIPAddr);

	sEventData.Param[10] = PollCount;
	sEventData.Param[11] = PollKind;
	sEventData.Param[12] = (DWORD)lpPlayInfo;
	sEventData.Param[13] = lpPlayInfo->dwConnectCount;

	if(PollCount && PollData)
	{
		for(cnt = 0; cnt < PollCount; cnt++)
		{
			sEventData.Param[cnt] = PollData[cnt];
		}
	}


	return b_PushLogEventQue(GAMELOG_SQL_POLLRECORD, sizeof(sLOG_EVENT), &sEventData);
}



int	bSql_EventJoin(rsPLAYINFO *lpPlayInfo)
{
	sLOG_EVENT sEventData;

	if(!BillSql.EventJoin) return FALSE;

	ZeroMemory(&sEventData, sizeof(sLOG_EVENT));

	lpPlayInfo->szGPCode[15] = 0;

	strcpy_s(sEventData.szID, lpPlayInfo->getAccount());
	strcpy_s(sEventData.szName, lpPlayInfo->getName());
	strcpy_s(sEventData.szGPCode, lpPlayInfo->szGPCode);
	strcpy_s(sEventData.szIP, lpPlayInfo->lpsmSock->szIPAddr);


	return b_PushLogEventQue(GAMELOG_SQL_EVENTJOIN, sizeof(sLOG_EVENT), &sEventData);
}



int	bSql_ClanMoney(rsPLAYINFO *lpPlayInfo, int Cash, int CashFlag, int Kind)
{
	sLOG_EVENT sEventData;

	/* R18: Clan/Siege money is gameplay persistence and has a SQLite path. */

	ZeroMemory(&sEventData, sizeof(sLOG_EVENT));

	lpPlayInfo->szGPCode[15] = 0;

	strcpy_s(sEventData.szID, lpPlayInfo->getAccount());
	strcpy_s(sEventData.szName, lpPlayInfo->getName());
	strcpy_s(sEventData.szGPCode, lpPlayInfo->szGPCode);
	strcpy_s(sEventData.szIP, lpPlayInfo->lpsmSock->szIPAddr);

	sEventData.Param[0] = Cash;
	sEventData.Param[1] = (DWORD)lpPlayInfo;
	sEventData.Param[2] = lpPlayInfo->dwConnectCount;

	sEventData.Param[3] = lpPlayInfo->smCharInfo.JOB_CODE;
	sEventData.Param[4] = lpPlayInfo->smCharInfo.Level;
	sEventData.Param[5] = CashFlag;
	sEventData.Param[6] = Kind;





	return b_PushLogEventQue(GAMELOG_SQL_CLANMONEY, sizeof(sLOG_EVENT), &sEventData);
}


int	bSql_RecordCurrency()
{

	sLOG_EVENT sEventData;

	ZeroMemory(&sEventData, sizeof(sLOG_EVENT));


	return b_PushLogEventQue(GAMELOG_SQL_SIEGETAX, sizeof(sLOG_EVENT), &sEventData);

}




int	bSql_GiveMoney(rsPLAYINFO *lpPlayInfo, int Money)
{
	sLOG_EVENT sEventData;

	if(!BillSql.Contribution) return FALSE;

	ZeroMemory(&sEventData, sizeof(sLOG_EVENT));

	lpPlayInfo->szGPCode[15] = 0;

	strcpy_s(sEventData.szID, lpPlayInfo->getAccount());
	strcpy_s(sEventData.szName, lpPlayInfo->getName());
	strcpy_s(sEventData.szGPCode, lpPlayInfo->szGPCode);
	strcpy_s(sEventData.szIP, lpPlayInfo->lpsmSock->szIPAddr);

	sEventData.Param[0] = Money;
	sEventData.Param[1] = (DWORD)lpPlayInfo;
	sEventData.Param[2] = lpPlayInfo->dwConnectCount;


	return b_PushLogEventQue(GAMELOG_SQL_GIVEMONEY, sizeof(sLOG_EVENT), &sEventData);
}





int bClan_GetTicket()
{
	if(BillSql.GetTicket)
	{
		return BillSql.GetTicket();
	}
	return NULL;
}


int	bClan_UserTicket(rsPLAYINFO *lpPlayInfo, int gubun)
{
	sCLAN_EVENT	sClanEvent;

	if(!BillSql.hClanDLL || !BillSql.GetTicket)
	{
		lpPlayInfo->ClanTicketCode = 0;
		return NULL;
	}

	strcpy_s(sClanEvent.szID, lpPlayInfo->getAccount());
	strcpy_s(sClanEvent.szName, lpPlayInfo->getName());
	strcpy_s(sClanEvent.szIP, lpPlayInfo->lpsmSock->szIPAddr);

	if(gubun == 0)
	{
		sClanEvent.TicketNum = lpPlayInfo->ClanTicketCode;
	}
	else
	{



		sClanEvent.TicketNum = lpPlayInfo->ClanTicketCode;
	}

	sClanEvent.Param[0] = gubun;
	sClanEvent.Param[1] = lpPlayInfo->smCharInfo.JOB_CODE;
	sClanEvent.Param[2] = lpPlayInfo->smCharInfo.Level;

	return b_PushLogEventQue(CLAN_USER_TICKET, sizeof(sCLAN_EVENT), &sClanEvent);
}



int	bClan_InsertPoint(rsPLAYINFO *lpPlayInfo, int point)
{
	sCLAN_EVENT	sClanEvent;

	if(!BillSql.hClanDLL || !BillSql.GetTicket) return NULL;

	strcpy_s(sClanEvent.szID, lpPlayInfo->getAccount());
	strcpy_s(sClanEvent.szName, lpPlayInfo->getName());
	strcpy_s(sClanEvent.szIP, lpPlayInfo->lpsmSock->szIPAddr);

	sClanEvent.TicketNum = lpPlayInfo->ClanTicketCode;
	sClanEvent.Param[0] = point;

	return b_PushLogEventQue(CLAN_INSERT_POINT, sizeof(sCLAN_EVENT), &sClanEvent);
}


int	bClan_GetClanMarkCode(rsPLAYINFO *lpPlayInfo)
{
	sCLAN_EVENT	sClanEvent;

	if(!BillSql.hClanDLL || !BillSql.ClanUserMarkNum) return NULL;

	strcpy_s(sClanEvent.szID, lpPlayInfo->getAccount());
	strcpy_s(sClanEvent.szName, lpPlayInfo->getName());
	strcpy_s(sClanEvent.szIP, lpPlayInfo->lpsmSock->szIPAddr);

	sClanEvent.Param[0] = 0;
	sClanEvent.Param[1] = (DWORD)lpPlayInfo;
	sClanEvent.Param[2] = lpPlayInfo->dwConnectCount;

	lpPlayInfo->dwCheck_MasterClanTime = dwPlayServTime;

	return b_PushLogEventQue(CLAN_GET_MARKCODE, sizeof(sCLAN_EVENT), &sClanEvent);
}






int	b_IPRecord(char *lpszServerName, struct JUNGIP *JungIP)
{
	if(BillSql.IPRecord)
	{
		return BillSql.IPRecord(lpszServerName, JungIP);
	}
	return TRUE;
}

int	b_IPRecordTime(char *lpszServerName, struct JUNGIP *JungIP)
{
	if(BillSql.IPRecordTime)
	{
		return BillSql.IPRecordTime(lpszServerName, JungIP);
	}
	return TRUE;
}

int b_SqlGetQueSize()
{
	int cnt;
	cnt = LogSqlDataCount - LogSqlSendCount;

	if(cnt < LogSqlProcessQueMax) cnt = LogSqlProcessQueMax;

	LogSqlProcessQueMax = 0;

	return cnt;
}

int b_SqlGetLossCount()
{
	return LogSqlLossCount;
}

int b_SqlGetQueProcessTime(DWORD *pTimeMax, DWORD *pAvgTime)
{
	int cnt;

	cnt = LogSqlProcessCount;

	*pTimeMax = LogSqlProcessTimeMax;
	if(LogSqlProcessCount > 0)
		*pAvgTime = LogSqlProcessTime / LogSqlProcessCount;
	else
		*pAvgTime = 0;


	LogSqlProcessTime = 0;
	LogSqlProcessTimeMax = 0;
	LogSqlProcessCount = 0;

	return cnt;
}






DWORD WINAPI LogItemDataThreadProc(void *pInfo)
{
	HANDLE	hThread;

	LogITEM_QUE		LogItemData;
	char			szID[ACCOUNT_SIZE];
	int	LogType;
	LogITEM			*lpLogItem;
	LogITEM_TRADE	*lpLogItemTrade;
	LogGiftITEM		*lpLogGiftItem;


	hThread = GetCurrentThread();

	while(1)
	{
		while(1)
		{
			EnterCriticalSection(&cSerSection);
			LogType = b_PopLogItemQue(&LogItemData);
			LeaveCriticalSection(&cSerSection);
			if(!LogType) break;


			if(hDebugWnd) PostMessage(hDebugWnd, SWM_DISPDEBUG, 100 + LogType, dwPlayServTime);
			switch(LogType)
			{
			case 1:
				lpLogItem = (LogITEM *)LogItemData.szBuff;
				lpLogItem->UserID[31] = 0;
				strcpy_s(szID, lpLogItem->UserID);
				if(lpLogItem->ItemCount < 32 && lpLogItem->size < sizeof(LogITEM_TRADE))
				{
					if(!TrySQLiteLocalItemAudit(
						1, lpLogItem->UserID, lpLogItem->CharName, lpLogItem->IP, lpLogItem->Flag,
						NULL, NULL, 0, 0, 0, NULL, lpLogItem->ItemCount, lpLogItem->Item) &&
						BillSql.LogITEMRecord)
					{
						BillSql.LogITEMRecord(szID, lpLogItem);
					}
				}
				break;

			case 2:
				lpLogItemTrade = (LogITEM_TRADE *)LogItemData.szBuff;
				lpLogItemTrade->UserID[31] = 0;
				strcpy_s(szID, lpLogItemTrade->UserID);
				if(lpLogItemTrade->ItemCount < 32 && lpLogItemTrade->size < sizeof(LogITEM_TRADE))
				{
					if(!TrySQLiteLocalItemAudit(
						2, lpLogItemTrade->UserID, lpLogItemTrade->CharName, lpLogItemTrade->IP, lpLogItemTrade->Flag,
						lpLogItemTrade->TUserID, lpLogItemTrade->TCharName, lpLogItemTrade->TIP, lpLogItemTrade->Money,
						0, NULL, lpLogItemTrade->ItemCount, lpLogItemTrade->Item) &&
						BillSql.LogITEMTradeRecord)
					{
						BillSql.LogITEMTradeRecord(szID, lpLogItemTrade);
					}
				}
				break;

			case 3:
				lpLogGiftItem = (LogGiftITEM *)LogItemData.szBuff;
				lpLogGiftItem->UserID[31] = 0;
				strcpy_s(szID, lpLogGiftItem->UserID);
				if(lpLogGiftItem->size < sizeof(LogITEM_TRADE))
				{
					if(!TrySQLiteLocalItemAudit(
						3, lpLogGiftItem->UserID, lpLogGiftItem->CharName, lpLogGiftItem->IP, lpLogGiftItem->Flag,
						NULL, NULL, 0, 0, lpLogGiftItem->GNo, lpLogGiftItem->TEL, 1, &lpLogGiftItem->Gitem) &&
						BillSql.LogITEMPostRecord)
					{
						BillSql.LogITEMPostRecord(szID, lpLogGiftItem);
					}
				}
				break;

			}
			if(hDebugWnd) PostMessage(hDebugWnd, SWM_DISPDEBUG, 0, dwPlayServTime);

		}

		SuspendThread(hThread);
		if(quit) break;
	}

	ExitThread(TRUE);
	return TRUE;
}



HANDLE	hLogItemThread = 0;
DWORD	dwLogItemThreadID;


int	b_PushLogItemQue(int type, int size, void *lpLogItemData)
{
	int cnt;

	/* R22 local audit no longer depends on Database.dll log exports. */
	if(!hLogItemThread)
	{
		hLogItemThread = CreateThread(NULL, 0, LogItemDataThreadProc, 0, 0, &dwLogItemThreadID);
		if(!hLogItemThread) return FALSE;
	}

	if((LogItemQuePush - LogItemQuePop) > LOG_ITEM_QUE_MASK || size > sizeof(LogITEM_TRADE))
	{
		LogItemLossCount++;
		return FALSE;
	}



	cnt = LogItemQuePush&LOG_ITEM_QUE_MASK;

	LogItemQue[cnt].LogType = type;
	memcpy(LogItemQue[cnt].szBuff, lpLogItemData, size);
	LogItemQuePush++;

	ResumeThread(hLogItemThread);



	return TRUE;
}


int b_PopLogItemQue(LogITEM_QUE *lpLogItemQue)
{
	int cnt;
	int len;

	len = LogItemQuePush - LogItemQuePop;
	if(len > LOG_ITEM_QUE_MASK)
	{

		LogItemQuePop = LogItemQuePush - LOG_ITEM_QUE_MASK;
	}
	if(len <= 0)
	{

		return NULL;
	}



	cnt = LogItemQuePop&LOG_ITEM_QUE_MASK;

	len = LogItemQue[cnt].LogType;
	memcpy(lpLogItemQue, &LogItemQue[cnt], sizeof(LogITEM_QUE));
	LogItemQuePop++;



	return len;
}


int b_SqlGetItemQueSize()
{
	return LogItemQuePush - LogItemQuePop;
}
int b_SqlGetItemLossCount()
{
	return LogItemLossCount;
}




DWORD WINAPI LogEventDataThreadProc(void *pInfo)
{
	HANDLE	hThread;

	LogEVENT_QUE		LogEventData;
	int	LogType;
	sLOG_EVENT			*sLogEvent;
	sCLAN_EVENT			*sClanEvent;
	int	Result;
	rsPLAYINFO			*lpPlayInfo;
	smTRANS_COMMAND		smTransCommand;
	INT64				int64_Money;
	rsCURRENCY_LOG		currency;
	POLL				PollData;


	hThread = GetCurrentThread();

	while(1)
	{
		while(1)
		{
			EnterCriticalSection(&cSerSection);
			LogType = b_PopLogEventQue(&LogEventData);
			LeaveCriticalSection(&cSerSection);
			if(!LogType) break;

			switch(LogType)
			{

			case GAMELOG_SQL_SOD:
				sLogEvent = (sLOG_EVENT *)LogEventData.szBuff;

				sLogEvent->szID[31] = 0;
				sLogEvent->szName[31] = 0;
				sLogEvent->szGPCode[15] = 0;
				sLogEvent->szIP[15] = 0;

				if(hDebugWnd)
					PostMessage(hDebugWnd, SWM_DISPDEBUG, 500 + LogType, dwPlayServTime);

				if(!TrySQLiteLocalSODRecord(
					sLogEvent->szID,
					sLogEvent->szName,
					sLogEvent->Param[0],
					sLogEvent->Param[1],
					sLogEvent->Param[2],
					sLogEvent->Param[3],
					sLogEvent->Param[4],
					sLogEvent->Param[5],
					sLogEvent->Param[6],
					sLogEvent->Param[7],
					sLogEvent->szGPCode,
					sLogEvent->szIP,
					sLogEvent->Param[8]) &&
					BillSql.WithSODRecordRanks)
				{
					BillSql.WithSODRecordRanks(
						sLogEvent->szID, sLogEvent->szName,
						sLogEvent->Param[0], sLogEvent->Param[1], sLogEvent->Param[2], sLogEvent->Param[3],
						sLogEvent->Param[4], sLogEvent->Param[5], sLogEvent->Param[6], sLogEvent->Param[7],
						sLogEvent->szGPCode, sLogEvent->szIP, sLogEvent->Param[8]);
				}

				if(hDebugWnd)
					PostMessage(hDebugWnd, SWM_DISPDEBUG, 0, dwPlayServTime);
				break;
			case GAMELOG_SQL_STARPOINT:
				if(BillSql.UserStar || BillSql.UserStarRecord)
				{
					sLogEvent = (sLOG_EVENT *)LogEventData.szBuff;

					sLogEvent->szID[31] = 0;
					sLogEvent->szName[31] = 0;
					sLogEvent->szGPCode[15] = 0;

					if(BillSql.UserStarRecord)
					{
						Result = BillSql.UserStarRecord(sLogEvent->szID, sLogEvent->Param[0],
							sLogEvent->szName, sLogEvent->Param[3], sLogEvent->Param[4], sLogEvent->Param[5]);
					}
					else
					{
						Result = BillSql.UserStar(sLogEvent->szID, sLogEvent->Param[0]);
					}

					EnterCriticalSection(&cSerSection);

					lpPlayInfo = (rsPLAYINFO *)sLogEvent->Param[1];


					if(lpPlayInfo && lpPlayInfo->lpsmSock && lpPlayInfo->dwConnectCount == sLogEvent->Param[2])
					{
						smTransCommand.size = sizeof(smTRANS_COMMAND);
						smTransCommand.code = OpCode::OPCODE_STARPOINT;
						smTransCommand.WParam = Result;
						smTransCommand.LParam = 0;
						smTransCommand.SParam = 0;
						smTransCommand.EParam = 0;
						(void)SummitDllServerSend(lpPlayInfo, &smTransCommand, smTransCommand.size);
					}
					LeaveCriticalSection(&cSerSection);

				}
				break;

			case GAMELOG_SQL_GIVEMONEY:
				if(BillSql.Contribution)
				{
					sLogEvent = (sLOG_EVENT *)LogEventData.szBuff;

					sLogEvent->szID[31] = 0;
					sLogEvent->szName[31] = 0;
					sLogEvent->szGPCode[15] = 0;

					Result = BillSql.Contribution(sLogEvent->szID, sLogEvent->szName, sLogEvent->Param[0]);

					EnterCriticalSection(&cSerSection);

					lpPlayInfo = (rsPLAYINFO *)sLogEvent->Param[1];


					LeaveCriticalSection(&cSerSection);

				}
				break;

			case CLAN_USER_TICKET:
				sClanEvent = (sCLAN_EVENT *)LogEventData.szBuff;

				sClanEvent->szID[31] = 0;
				sClanEvent->szName[31] = 0;
				sClanEvent->szIP[15] = 0;

				if(BillSql.ClanUserTicKets)
				{
					BillSql.ClanUserTicKets(sClanEvent->szID, sClanEvent->szName, sClanEvent->szIP, sClanEvent->TicketNum, sClanEvent->Param[0], sClanEvent->Param[1], sClanEvent->Param[2]);
					break;
				}
				if(BillSql.ClanUserTicket)
				{
					BillSql.ClanUserTicket(sClanEvent->szID, sClanEvent->szName, sClanEvent->szIP, sClanEvent->TicketNum, sClanEvent->Param[0]);
					break;
				}

				break;

			case CLAN_INSERT_POINT:
				if(BillSql.ClanPointInsert)
				{
					sClanEvent = (sCLAN_EVENT *)LogEventData.szBuff;

					sClanEvent->szID[31] = 0;
					sClanEvent->szName[31] = 0;
					sClanEvent->szIP[15] = 0;

					BillSql.ClanPointInsert(sClanEvent->szID, sClanEvent->szName, sClanEvent->szIP, sClanEvent->TicketNum, sClanEvent->Param[0]);
				}
				break;

			case CLAN_GET_MARKCODE:
				if(BillSql.ClanUserMarkNum)
				{
					sClanEvent = (sCLAN_EVENT *)LogEventData.szBuff;

					sClanEvent->szID[31] = 0;
					sClanEvent->szName[31] = 0;
					sClanEvent->szIP[15] = 0;

					Result = BillSql.ClanUserMarkNum(sClanEvent->szID, sClanEvent->szName);

					EnterCriticalSection(&cSerSection);
					lpPlayInfo = (rsPLAYINFO *)sClanEvent->Param[1];

					if(lpPlayInfo && lpPlayInfo->lpsmSock && lpPlayInfo->dwConnectCount == sClanEvent->Param[2])
					{
						lpPlayInfo->CallBack_ClanMarkNum(Result);
					}
					LeaveCriticalSection(&cSerSection);
				}
				break;



			case GAMELOG_SQL_EVENTJOIN:
				if(BillSql.EventJoin)
				{
					sLogEvent = (sLOG_EVENT *)LogEventData.szBuff;

					sLogEvent->szID[31] = 0;
					sLogEvent->szName[31] = 0;
					sLogEvent->szGPCode[15] = 0;

					Result = BillSql.EventJoin(sLogEvent->szID, sLogEvent->szName);
				}
				break;


			case GAMELOG_SQL_CLANMONEY:
				{
					sLogEvent = (sLOG_EVENT *)LogEventData.szBuff;

					sLogEvent->szID[31] = 0;
					sLogEvent->szName[31] = 0;
					sLogEvent->szGPCode[15] = 0;

					INT64 localResult = -1111;
					BOOL handledLocally = TrySQLiteLocalClanFund(
						sLogEvent->szID,
						sLogEvent->szName,
						(INT64)sLogEvent->Param[0],
						sLogEvent->Param[5],
						sLogEvent->Param[6] ? TRUE : FALSE,
						&localResult);

					if(handledLocally)
					{
						Result = (int)localResult;
					}
					else if(sLogEvent->Param[6] && BillSql.SiegeMoney)
					{
						Result = (int)BillSql.SiegeMoney(sLogEvent->szID, sLogEvent->szName, (INT64)sLogEvent->Param[0], sLogEvent->Param[5]);
					}
					else if(!sLogEvent->Param[6] && BillSql.ClanMoney)
					{
						Result = BillSql.ClanMoney(sLogEvent->szID, sLogEvent->szName, sLogEvent->Param[0], sLogEvent->Param[5]);
					}
					else
					{
						Result = -1111;
					}

					EnterCriticalSection(&cSerSection);

					lpPlayInfo = (rsPLAYINFO *)sLogEvent->Param[1];


					if(lpPlayInfo && lpPlayInfo->lpsmSock && lpPlayInfo->dwConnectCount == sLogEvent->Param[2])
					{
						smTransCommand.size = sizeof(smTRANS_COMMAND);
						smTransCommand.code = OpCode::OPCODE_CLANMONEY;
						smTransCommand.WParam = Result;

						switch(sLogEvent->Param[5])
						{
						case 0:
							smTransCommand.LParam = 0;
							break;

						case 1:
							smTransCommand.LParam = sLogEvent->Param[0];
							(void)gameplay::commerce::legacy::ServerLegacyWalletAdapter().Credit(
								gameplay::player::legacy::ServerLegacyOnlinePlayerRegistry().IdOf(lpPlayInfo),
								sLogEvent->Param[0], gameplay::commerce::WalletReason::ClanMoney);
							rsClanMoneyLog(lpPlayInfo, smTransCommand.LParam, Result);
							break;

						case 2:
							smTransCommand.LParam = -sLogEvent->Param[0];
							(void)gameplay::commerce::legacy::ServerLegacyWalletAdapter().Debit(
								gameplay::player::legacy::ServerLegacyOnlinePlayerRegistry().IdOf(lpPlayInfo),
								sLogEvent->Param[0], gameplay::commerce::WalletReason::ClanMoney);
							rsClanMoneyLog(lpPlayInfo, smTransCommand.LParam, Result);
							break;
						}

						smTransCommand.SParam = sLogEvent->Param[6];
						smTransCommand.EParam = 0;
						(void)SummitDllServerSend(lpPlayInfo, &smTransCommand, smTransCommand.size);

					}
					LeaveCriticalSection(&cSerSection);

				}
				break;

			case GAMELOG_SQL_SIEGETAX:
				sLogEvent = (sLOG_EVENT *)LogEventData.szBuff;
				wsprintf(sLogEvent->szID, "%d", rsBlessCastle.dwMasterClan);

				EnterCriticalSection(&cSerSection);
				memcpy(&currency, &CurrencyLog, sizeof(rsCURRENCY_LOG));
				LeaveCriticalSection(&cSerSection);

				currency.int64_CraftMoney += (currency.int64_CraftMoney*rsBlessCastle.Tax) / 100;
				currency.int64_AgingMoney += (currency.int64_AgingMoney*rsBlessCastle.Tax) / 100;
				currency.int64_ShopMoney += (currency.int64_ShopMoney*rsBlessCastle.Tax) / 100;
				currency.int64_ShopPotionMoney[0] += (currency.int64_ShopPotionMoney[0] * rsBlessCastle.Tax) / 100;
				currency.int64_ShopPotionMoney[1] += (currency.int64_ShopPotionMoney[1] * rsBlessCastle.Tax) / 100;
				currency.int64_ShopPotionMoney[2] += (currency.int64_ShopPotionMoney[2] * rsBlessCastle.Tax) / 100;

				int64_Money = currency.int64_CraftMoney + currency.int64_AgingMoney + currency.int64_ShopMoney;
				int64_Money += currency.int64_ShopPotionMoney[0] + currency.int64_ShopPotionMoney[1] + currency.int64_ShopPotionMoney[2];
				int64_Money += currency.int64_ForceOrbMoney + currency.int64_WarpgateMoney + currency.int64_LearnSkillMoney;

				{
					BOOL localResult = FALSE;
					BOOL handledLocally = TrySQLiteLocalSiegeMoneyTax(
						sLogEvent->szID,
						currency.int64_CraftMoney,
						currency.int64_AgingMoney,
						currency.int64_ShopMoney,
						currency.int64_ShopPotionMoney[0],
						currency.int64_ShopPotionMoney[1],
						currency.int64_ShopPotionMoney[2],
						currency.int64_ForceOrbMoney,
						currency.int64_WarpgateMoney,
						currency.int64_LearnSkillMoney,
						int64_Money,
						rsBlessCastle.Tax,
						&localResult);

					if(handledLocally)
						Result = localResult;
					else if(BillSql.SiegeMoneyTax)
						Result = BillSql.SiegeMoneyTax(
							sLogEvent->szID,
							currency.int64_CraftMoney,
							currency.int64_AgingMoney,
							currency.int64_ShopMoney,
							currency.int64_ShopPotionMoney[0],
							currency.int64_ShopPotionMoney[1],
							currency.int64_ShopPotionMoney[2],
							currency.int64_ForceOrbMoney,
							currency.int64_WarpgateMoney,
							currency.int64_LearnSkillMoney,
							int64_Money,
							rsBlessCastle.Tax);
					else
						Result = FALSE;
				}

				if(Result)
				{
					EnterCriticalSection(&cSerSection);
					ZeroMemory(&CurrencyLog, sizeof(rsCURRENCY_LOG));
					LeaveCriticalSection(&cSerSection);
				}
				break;


			case GAMELOG_SQL_POLLRECORD:
				if(BillSql.PollRecord)
				{
					sLogEvent = (sLOG_EVENT *)LogEventData.szBuff;

					sLogEvent->szID[31] = 0;
					sLogEvent->szName[31] = 0;
					sLogEvent->szIP[15] = 0;

					ZeroMemory(&PollData, sizeof(POLL));

					strcpy_s(PollData.UserID, sLogEvent->szID);
					strcpy_s(PollData.CharID, sLogEvent->szName);
					PollData.PollCode = sLogEvent->Param[11];
					PollData.PollCount = sLogEvent->Param[10];

					if(PollData.PollCount)
					{
						memcpy(PollData.PollItem, sLogEvent->Param, sizeof(int)*PollData.PollCount);
						Result = BillSql.PollRecord(sLogEvent->szID, tServerTime, &PollData, 1);
					}
					else
						Result = BillSql.PollRecord(sLogEvent->szID, tServerTime, &PollData, 0);

					EnterCriticalSection(&cSerSection);
					lpPlayInfo = (rsPLAYINFO *)sLogEvent->Param[12];

					if(lpPlayInfo && lpPlayInfo->lpsmSock && lpPlayInfo->dwConnectCount == sLogEvent->Param[13])
					{
						if(!PollData.PollCount)
						{
							smTransCommand.code = OpCode::OPCODE_PUBLIC_POLLING;
							smTransCommand.size = sizeof(smTRANS_COMMAND);
							smTransCommand.WParam = sLogEvent->Param[11];
							if(Result)
								smTransCommand.LParam = 0;
							else
								smTransCommand.LParam = 1;
							smTransCommand.SParam = 0;
							smTransCommand.EParam = 0;
							(void)SummitDllServerSend(lpPlayInfo, &smTransCommand, smTransCommand.size);
						}
						else
						{
							if(Result)
							{

							}
						}
					}
					LeaveCriticalSection(&cSerSection);
				}
				break;
			}
		}

		SuspendThread(hThread);
		if(quit) break;
	}

	ExitThread(TRUE);
	return TRUE;
}



HANDLE	hLogEventThread = 0;
DWORD	dwLogEventThreadID;


int	b_PushLogEventQue(int type, int size, void *lpLogEventData)
{
	int cnt;

	if(!hLogEventThread)
	{
		hLogEventThread = CreateThread(NULL, 0, LogEventDataThreadProc, 0, 0, &dwLogEventThreadID);
		if(!hLogEventThread) return FALSE;
	}

	if((LogEventQuePush - LogEventQuePop) > LOG_EVENT_QUE_MASK || size > 512)
	{
		LogEventLossCount++;
		return FALSE;
	}

	cnt = LogEventQuePush&LOG_EVENT_QUE_MASK;

	LogEventQue[cnt].LogType = type;
	memcpy(LogEventQue[cnt].szBuff, lpLogEventData, size);
	LogEventQuePush++;

	ResumeThread(hLogEventThread);

	return TRUE;
}



int b_PopLogEventQue(LogEVENT_QUE *lpLogEventQue)
{
	int cnt;
	int len;


	len = LogEventQuePush - LogEventQuePop;
	if(len > LOG_EVENT_QUE_MASK)
	{

		LogEventQuePop = LogEventQuePush - LOG_EVENT_QUE_MASK;
	}
	if(len <= 0)
	{

		return NULL;
	}

	if(!LogEventQue) return NULL;

	cnt = LogEventQuePop&LOG_EVENT_QUE_MASK;

	len = LogEventQue[cnt].LogType;
	memcpy(lpLogEventQue, &LogEventQue[cnt], sizeof(LogEVENT_QUE));
	LogEventQuePop++;

	return len;
}


int b_SqlGetEventQueSize()
{
	return LogEventQuePush - LogEventQuePop;
}
int b_SqlGetEventLossCount()
{
	return LogEventLossCount;
}







#else

int InitLogSql()
{
	return TRUE;
}
int CloseLogSql()
{
	return TRUE;
}
int RecordLogSql(DWORD	dwRecCode, char *szIP, const char *lpUserID, const char *lpCharName, char *szGPCode, int Race, int JobCode, INT64 Exp64, DWORD dwLevel, DWORD dwMoney)
{
	return TRUE;
}
int RecordGameLogSql(DWORD	dwRecCode, char *szIP, const char *lpUserID, const char *lpCharName, char *szGPCode, int JobCode, char *szGuid, int BlockTime)
{
	return TRUE;
}

int	bSql_LogOn(char *lpszPID, char *lpszPWD, char *lpszIP, char *lpszGPCode, char *lpszDisuseDay, char *lpszUsePeriod, char *szGuid, char *lpszPCRNo)
{
	return TRUE;
}

int	bSql_GetLocalPCBangGameplayEntitlement(const char *userID)
{
	return FALSE;
}

int	bSql_Logout(const char *lpszPID, const char *lpszCID, char *lpszIP, char *lpszGPCode, time_t DisuseDay, int UsePeriod, char *szGuid, int PCRNo)
{
	return TRUE;
}


int	bSql_ServerDown()
{
	return TRUE;
}
int	bSql_ServerExit()
{
	return TRUE;
}
int	bSql_GetGPCode(char *GPCode, char *PG)
{
	return TRUE;
}


int	bSql_PCUse(rsPLAYINFO *lpPlayInfo)
{
	return TRUE;
}

int	bUrs_Expined(rsPLAYINFO *lpPlayInfo)
{
	return TRUE;
}


int	bSql_PCSearchReserv(rsPLAYINFO *lpPlayInfo)
{
	return TRUE;
}

int	bSql_ChangeUserIdGpCodeGame(int PCRNo)
{
	return TRUE;
}

int	bSql_EventLogon(rsPLAYINFO *lpPlayInfo)
{
	return TRUE;
}

int	bSql_EventLogOut(rsPLAYINFO *lpPlayInfo)
{
	return TRUE;
}



int	bSql_PCRNoReservProduct(rsPLAYINFO *lpPlayInfo)
{
	return TRUE;
}


int	b_IPRecord(char *lpszServerName, struct JUNGIP *JungIP)
{
	return TRUE;
}

int	b_IPRecordTime(char *lpszServerName, struct JUNGIP *JungIP)
{
	return TRUE;
}


int	bSql_SODRecord(rsPLAYINFO *lpPlayInfo, int Level, int SodScore, int SodKillCount, int SodTotalScore, int SodTotalSucessUser, int SodTotalUser, int SodCode)
{
	return TRUE;
}

int	bSql_StarPoint(rsPLAYINFO *lpPlayInfo, int StarPoint, int CashFlag)
{
	return TRUE;
}


int	bSql_PollData(rsPLAYINFO *lpPlayInfo, int PollKind, int PollCount, BYTE *PollData)
{
	return TRUE;
}


int	bSql_GetExpGameTime(rsPLAYINFO *lpPlayInfo, int GamePlayTime)
{
	return TRUE;
}


int	bSql_EventJoin(rsPLAYINFO *lpPlayInfo)
{
	return TRUE;
}

int	bSql_ClanMoney(rsPLAYINFO *lpPlayInfo, int Cash, int CashFlag, int Kind)
{
	return	TRUE;
}


int	bClan_GetClanMarkCode(rsPLAYINFO *lpPlayInfo)
{
	return TRUE;
}

int	bSql_RecordCurrency()
{
	return	TRUE;
}


int	bSql_GiveMoney(rsPLAYINFO *lpPlayInfo, int Money)
{
	return TRUE;
}

int b_SqlGetQueSize()
{
	return TRUE;
}
int b_SqlGetLossCount()
{
	return TRUE;
}
int b_SqlGetQueProcessTime(DWORD *pTimeMax, DWORD *pAvgTime)
{
	return TRUE;
}

int b_PopLogItemQue(LogITEM_QUE *lpLogItemQue)
{
	return TRUE;
}

int	b_PushLogItemQue(int type, int size, void *lpLogItemData)
{
	return TRUE;
}


int b_SqlGetItemQueSize()
{
	return TRUE;
}
int b_SqlGetItemLossCount()
{
	return TRUE;
}


int b_PopLogEventQue(LogEVENT_QUE *lpLogEventQue)
{
	return TRUE;
}

int	b_PushLogEventQue(int type, int size, void *lpLogEventData)
{
	return TRUE;
}


int b_SqlGetEventQueSize()
{
	return TRUE;
}
int b_SqlGetEventLossCount()
{
	return TRUE;
}



int	bClan_UserTicket(rsPLAYINFO *lpPlayInfo, int gubun)
{
	return TRUE;
}

int	bClan_InsertPoint(rsPLAYINFO *lpPlayInfo, int point)
{
	return TRUE;
}


DWORD nSprite_GetResult(DWORD nRecvNum)
{
	return TRUE;
}

#endif














