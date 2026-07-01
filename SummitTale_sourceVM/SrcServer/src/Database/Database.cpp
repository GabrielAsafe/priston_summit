#include "Database.h"
#include <iostream>
#include <memory>
#include <string>
#include "File\File.h"
#include "Helper\common.h"
#include "Database.h"

using namespace std;

std::vector<std::shared_ptr<SQLConnection>> vSQLConnection;

BOOL show_errorSQL(unsigned int handletype, const SQLHANDLE& handle)
{
	return TRUE;
	SQLCHAR sqlstateSqlError[1024];
	SQLCHAR messageSqlError[1024];
	char szBufSQLError[2048];
	ZeroMemory(szBufSQLError, 2048);
	ZeroMemory(sqlstateSqlError, 1024);
	ZeroMemory(messageSqlError, 1024);

	if(SQL_SUCCESS == SQLGetDiagRecA(handletype, handle, 1, sqlstateSqlError, NULL, messageSqlError, 1024, NULL))
	{
		wsprintfA(szBufSQLError, "Message: %s\nSQLSTATE: %s\n", messageSqlError, sqlstateSqlError);

		Utils_Log(LOG_SQL, szBufSQLError);
		/*printf(szBufSQLError);

		using namespace std;
		freopen("SQL.txt", "w", stderr);

		cerr << szBufSQLError << endl;*/
		return TRUE;
	}
	return FALSE;
}

DatabaseStructureData sDatabaseData[]
{
	{ DATABASEID_AccountDB,     "AccountDB"       },
	{ DATABASEID_BillingDB,     "BillingDB"       },
	{ DATABASEID_BattlePass,     "BattlePass"	 },
	{ DATABASEID_BillingLogDB,  "BillingLogDB"    },
	{ DATABASEID_ClanDB,        "ClanDB"          },
	{ DATABASEID_GameLogDB,     "GameLogDB"       },
	{ DATABASEID_ItemLogDB,     "ItemLogDB"       },
	{ DATABASEID_LogDB,         "LogDB"           },
	{ DATABASEID_PCRoom,        "PCRoom"          },
	{ DATABASEID_PCRoomLog,     "PCRoomLog"       },
	{ DATABASEID_rPTDB,         "rPTDB"           },
	{ DATABASEID_SoD2DB,        "SoD2DB"          },
	{ DATABASEID_SodDB,         "SodDB"			  },
	{ DATABASEID_ServerDB,      "ServerDB"        },
	{ DATABASEID_SecurityDB,    "SecurityDB"      },
	{ DATABASEID_CoinShopDB,	"CoinShopDB"      },
	{ DATABASEID_QuestDB,		"QuestDB"		  },
	{ DATABASEID_StartUserDB,	"StartUserDB"	  },
};


SQLConnection::SQLConnection()
{}


SQLConnection::~SQLConnection()
{}

DatabaseStructureData * SQLConnection::GetDatabaseData(EDatabaseID eDatabaseID)
{
	for(int i = 0; i < _countof(sDatabaseData); i++)
	{
		if(sDatabaseData[i].eDatabaseID == eDatabaseID)
			return &sDatabaseData[i];
	}
	return NULL;
}

bool SQLConnection::Init(EDatabaseID eDatabaseID)
{
	// Copy database name to class
	strcpy_s(szDatabaseName, GetDatabaseData(eDatabaseID)->pszDatabaseName);
	eID = eDatabaseID;

	// Try alloc Handle SQL ODBC
	if(SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv))
	{
		//show_errorSQL(SQL_HANDLE_ENV, hStatement);
		Utils_Log(LOG_SQL, "SQLDATA: Unable to allocate handle for environment.");
		return false;
	}

	// Set Version ODBC
	if(SQL_SUCCESS != SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0))
	{
		Utils_Log(LOG_SQL, "SQLDATA: Unable to set ODBC version.");
		return false;
	}
	// Set statement of cursor type
	SQLSetStmtAttr(hEnv, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_DYNAMIC, 0);

	// Try alloc handle connection
	if(SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hConnection))
	{
		//show_errorSQL(SQL_HANDLE_DBC, hStatement);
		Utils_Log(LOG_SQL, "SQLDATA: Unable to allocate handle for connection.");
		return false;
	}

	// Set connection timeout
	SQLSetConnectAttrA(hConnection, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);

	char szStringConnectionSQL[512];
	SQLCHAR strconn[512];
	ZeroMemory(szStringConnectionSQL, 512);

	string Driver = "";
	string Host = "";
	string User = "";
	string Password = "";

	try
	{
		File file("Configs\\Sistemas.ini");

		Driver = file.readString("Database", "Driver");
		Host = file.readString("Database", "Host");
		User = file.readString("Database", "User");
		Password = file.readString("Database", "Password");
	}
	catch(FileException &e)
	{
		cerr << "bool SQLConnection::Init(EDatabaseID eDatabaseID): " << e.what() << endl;
	}
#ifdef YWEENZY_PC
	wsprintfA(szStringConnectionSQL, "Persist Security Info=False; Integrated Security=False; Driver=%s; Server=%s; Database=%s; Uid=%s; Pwd=%s;",
		"{SQL Server}", "PTL\\SQLEXPRES", szDatabaseName, "sa", "#$pdl32xq");
#else
	wsprintfA(szStringConnectionSQL, "Persist Security Info=False; Integrated Security=False; Driver=%s; Server=%s; Database=%s; Uid=%s; Pwd=%s;",
			  Driver.c_str(), Host.c_str(), szDatabaseName, User.c_str(), Password.c_str());
#endif
	memcpy(strconn, szStringConnectionSQL, 512);


	SQLCHAR retconstring[1024] = { 0 };
	// Connect to driver
	SQLDriverConnectA(hConnection, NULL, strconn, SQL_NTS, retconstring, 1024, NULL, SQL_DRIVER_NOPROMPT);

	// Alloc statement of connection
	if(SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_STMT, hConnection, &hStatement))
	{
		//show_errorSQL(SQL_HANDLE_DBC, hStatement);
		Utils_Log(LOG_SQL, "SQLAllocHandle failed. Error 4");
		return false;
	}

	// Create critical section to use on mutex
	InitializeCriticalSection(&sCriticalSection);

	return true;
}

BOOL SQLConnection::SQLTimeStampToSystemTime(SYSTEMTIME * psSystemTime, SQL_TIMESTAMP_STRUCT * psSQLTimeStamp)
{
	if(!psSystemTime || !psSQLTimeStamp)
		return FALSE;

	psSystemTime->wYear = psSQLTimeStamp->year;
	psSystemTime->wMonth = psSQLTimeStamp->month;
	psSystemTime->wDayOfWeek = 0;
	psSystemTime->wDay = psSQLTimeStamp->day;
	psSystemTime->wHour = psSQLTimeStamp->hour;
	psSystemTime->wMinute = psSQLTimeStamp->minute;
	psSystemTime->wSecond = psSQLTimeStamp->second;
	psSystemTime->wMilliseconds = (WORD)psSQLTimeStamp->fraction;

	return TRUE;
}

BOOL SQLConnection::Open()
{
	// Lock Mutex
	EnterCriticalSection(&sCriticalSection);
	return TRUE;
}

BOOL SQLConnection::Prepare(const char * pszQuery)
{
	// Copy Query
	strcpy_s(szQuery, pszQuery);

	// Check SQLI
	for(int i = 0; i < 2048; i++)
	{
		// End string? break it
		if(szQuery[i] == 0)
			break;

		// Have quote mark? SQLI, return error
		if(szQuery[i] == 0x27)
		{
			// Query Not secure

			return FALSE;
		}
	}

	// Free Statement
	SQLFreeStmt(hStatement, SQL_CLOSE);

	// Prepare Query error? return
	if(SQLPrepareA(hStatement, (SQLCHAR*)szQuery, SQL_NTS) == SQL_ERROR)
	{
		//Utils_Log(LOG_SQL, "Error Prepare() - Query: %s", pszQuery);
		//show_errorSQL(SQL_HANDLE_STMT, hStatement);
		return FALSE;
	}
	// Success
	return TRUE;
}

BOOL SQLConnection::Execute(BOOL bFetch)
{
	// Try Execute query
	if(SQLExecute(hStatement) == SQL_ERROR)
	{
		//Utils_Log(LOG_SQL, "Error Execute() - Query: %s", szQuery);
		//show_errorSQL(SQL_HANDLE_STMT, hStatement);
		return FALSE;
	}
	if(bFetch)
	{
		if(!NextRow())
			return FALSE;
	}
	return TRUE;
}

BOOL SQLConnection::BindInputParameter(void * pParameter, int iPosition, EDatabaseDataType eDataType)
{
	int iType = 0;
	int iSQLType = 0;
	int iLen = 0;
	// Set Data Type
	switch(eDataType)
	{
		case PARAMTYPE_Integer:
		iType = SQL_C_LONG;
		iSQLType = SQL_INTEGER;
		break;
		case PARAMTYPE_String:
		iType = SQL_C_CHAR;
		iSQLType = SQL_VARCHAR;
		iLen = lstrlenA((char*)pParameter);
		break;
		case PARAMTYPE_Float:
		iType = SQL_C_FLOAT;
		iSQLType = SQL_REAL;
		break;
		case PARAMTYPE_Int64:
		iType = SQL_C_SBIGINT;
		iSQLType = SQL_BIGINT;
		break;
		case PARAMTYPE_Time:
		iType = SQL_C_TIMESTAMP;
		iSQLType = SQL_TYPE_TIMESTAMP;
		break;
		default:
		break;
	}
	// Bind parameters
	if(SQLBindParameter(hStatement, iPosition, SQL_PARAM_INPUT, iType, iSQLType, iLen, 0, pParameter, 0, NULL) == SQL_ERROR)
	{

		show_errorSQL(SQL_HANDLE_STMT, hStatement);
		return FALSE;
	}
	return TRUE;
}

BOOL SQLConnection::GetData(void * pParameter, int iSize, int iPosition, EDatabaseDataType eDataType)
{
	int iType = 0;
	//SQLINTEGER iDataSize = 0;

	SQLLEN iDataSize = 0;

	SQL_TIMESTAMP_STRUCT sTimeStamp;
	void* pParam = pParameter;
	int iSizeParam = iSize;

	// Set Data Type
	switch(eDataType)
	{
		case PARAMTYPE_Integer:
		iType = SQL_C_LONG;
		break;
		case PARAMTYPE_Short:
		iType = SQL_C_SHORT;
		break;

		case PARAMTYPE_String:
		iType = SQL_C_CHAR;
		break;
		case PARAMTYPE_Float:
		iType = SQL_C_FLOAT;
		break;
		case PARAMTYPE_Int64:
		iType = SQL_C_SBIGINT;
		break;
		case PARAMTYPE_Time:
		iType = SQL_C_TYPE_TIMESTAMP;
		pParam = &sTimeStamp;
		iSizeParam = 23;
		break;
		default:
		break;
	}
	// Get Data
	if(SQLGetData(hStatement, iPosition, iType, pParam, iSizeParam, &iDataSize) == SQL_ERROR)
	{

		show_errorSQL(SQL_HANDLE_STMT, hStatement);
		return FALSE;
	}

	if(eDataType == PARAMTYPE_Time)
		SQLTimeStampToSystemTime((SYSTEMTIME*)pParameter, &sTimeStamp);

	return TRUE;
}

BOOL SQLConnection::NextRow()
{
	// Fetch ( Next Row )
	if(!SQL_SUCCEEDED(SQLFetch(hStatement)))
		return FALSE;
	return TRUE;
}

BOOL SQLConnection::Close()
{
	// Free Statement
	SQLFreeStmt(hStatement, SQL_CLOSE);

	// SQL Cancel
	SQLCancel(hStatement);

	// Clear Query
	ZeroMemory(szQuery, 2048);

	// Unlock Mutex
	LeaveCriticalSection(&sCriticalSection);
	return TRUE;
}

void CreateSQLConnection(EDatabaseID eDatabaseID)
{
	std::shared_ptr<SQLConnection> pcSQL = std::make_shared<SQLConnection>();

	if(pcSQL->Init(eDatabaseID))
	{
		vSQLConnection.push_back(pcSQL);
	}
	else
	{
		cout << "Falha ao conectar-se ao banco de dados! Log: SQL.txt" << endl;

		Utils_Log(LOG_SERVER, "Falha ao conectar-se ao banco de dados! Log: SQL.txt");

		Sleep(1500);
		exit(0);
	}
}

void openDatabase()
{
	cout << "Conectando-se ao Banco de Dados: AccountDB" << endl;
	CreateSQLConnection(DATABASEID_AccountDB);
	cout << "Conectado com sucesso" << endl;

	cout << "Conectando-se ao Banco de Dados: BillingDB" << endl;
	CreateSQLConnection(DATABASEID_BillingDB);
	cout << "Conectado com sucesso" << endl;

	cout << "Conectando-se ao Banco de Dados: BillingLogDB" << endl;
	CreateSQLConnection(DATABASEID_BillingLogDB);
	cout << "Conectado com sucesso" << endl;

	cout << "Conectando-se ao Banco de Dados: ClanDB" << endl;
	CreateSQLConnection(DATABASEID_ClanDB);
	cout << "Conectado com sucesso" << endl;

	cout << "Conectando-se ao Banco de Dados: GameLogDB" << endl;
	CreateSQLConnection(DATABASEID_GameLogDB);
	cout << "Conectado com sucesso" << endl;

	cout << "Conectando-se ao Banco de Dados: ItemLogDB" << endl;
	CreateSQLConnection(DATABASEID_ItemLogDB);
	cout << "Conectado com sucesso" << endl;

	cout << "Conectando-se ao Banco de Dados: LogDB" << endl;
	CreateSQLConnection(DATABASEID_LogDB);
	cout << "Conectado com sucesso" << endl;

	cout << "Conectando-se ao Banco de Dados: PCRoom" << endl;
	CreateSQLConnection(DATABASEID_PCRoom);
	cout << "Conectado com sucesso" << endl;

	cout << "Conectando-se ao Banco de Dados: PCRoomLog" << endl;
	CreateSQLConnection(DATABASEID_PCRoomLog);
	cout << "Conectado com sucesso" << endl;

	cout << "Conectando-se ao Banco de Dados: rPTDB" << endl;
	CreateSQLConnection(DATABASEID_rPTDB);
	cout << "Conectado com sucesso" << endl;

	cout << "Conectando-se ao Banco de Dados: SoD2DB" << endl;
	CreateSQLConnection(DATABASEID_SoD2DB);
	cout << "Conectado com sucesso" << endl;

	cout << "Conectando-se ao Banco de Dados: SodDB" << endl;
	CreateSQLConnection(DATABASEID_SodDB);
	cout << "Conectado com sucesso" << endl;

	cout << "Conectando-se ao Banco de Dados: ServerDB" << endl;
	CreateSQLConnection(DATABASEID_ServerDB);
	cout << "Conectado com sucesso" << endl;

	cout << "Conectando-se ao Banco de Dados: SecurityDB" << endl;
	CreateSQLConnection(DATABASEID_SecurityDB);
	cout << "Conectado com sucesso" << endl;

	cout << "Conectando-se ao Banco de Dados: CoinShopDB" << endl;
	CreateSQLConnection(DATABASEID_CoinShopDB);
	cout << "Conectado com sucesso" << endl;

	cout << "Conectando-se ao Banco de Dados: QuestDB" << endl;
	CreateSQLConnection(DATABASEID_QuestDB);
	cout << "Conectado com sucesso" << endl;

	cout << "Conectando-se ao Banco de Dados: StartUserDB" << endl;
	CreateSQLConnection(DATABASEID_StartUserDB);
	cout << "Conectado com sucesso" << endl;
}

std::shared_ptr<SQLConnection> SQLConnection::GetConnection(EDatabaseID eDatabaseID)
{
	// Find connection pointer
	for(UINT u = 0; u < vSQLConnection.size(); u++)
	{
		// Mismatch Database? return pointer of class
		if(vSQLConnection[u]->eID == eDatabaseID)
		{
			return vSQLConnection[u];
		}
	}

	return nullptr;
}