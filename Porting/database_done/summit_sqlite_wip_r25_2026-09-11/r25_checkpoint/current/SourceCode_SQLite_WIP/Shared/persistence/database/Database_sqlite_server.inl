#include "systems/persistence/database/core/Database.h"
#include "systems/persistence/database/sqlite/SQLiteConnection.h"
#include "systems/persistence/database/sqlite/SqliteQueryCompat.h"
#include "modules/Helper/Utils/utils_logging.h"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <vector>

#define LOG_SQL    0
#define LOG_SERVER 1

std::vector<std::shared_ptr<SQLConnection>> vSQLConnection;

namespace
{
DatabaseStructureData sDatabaseData[] =
{
	{ DATABASEID_AccountDB,    "AccountDB" },
	{ DATABASEID_BillingDB,    "BillingDB" },
	{ DATABASEID_BattlePass,   "BattlePass" },
	{ DATABASEID_BillingLogDB, "BillingLogDB" },
	{ DATABASEID_ClanDB,       "ClanDB" },
	{ DATABASEID_GameLogDB,    "GameLogDB" },
	{ DATABASEID_ItemLogDB,    "ItemLogDB" },
	{ DATABASEID_LogDB,        "LogDB" },
	{ DATABASEID_PCRoom,       "PCRoom" },
	{ DATABASEID_PCRoomLog,    "PCRoomLog" },
	{ DATABASEID_rPTDB,        "rPTDB" },
	{ DATABASEID_SoD2DB,       "SoD2DB" },
	{ DATABASEID_SodDB,        "SodDB" },
	{ DATABASEID_ServerDB,     "ServerDB" },
	{ DATABASEID_SecurityDB,   "SecurityDB" },
	{ DATABASEID_CoinShopDB,   "CoinShopDB" },
	{ DATABASEID_QuestDB,      "QuestDB" },
	{ DATABASEID_StartUserDB,  "StartUserDB" },
};

const DatabaseStructureData *GetDatabaseData(EDatabaseID databaseId)
{
	for(std::size_t i = 0; i < sizeof(sDatabaseData) / sizeof(sDatabaseData[0]); ++i)
		if(sDatabaseData[i].eDatabaseID == databaseId)
			return &sDatabaseData[i];
	return nullptr;
}
}

SQLConnection::SQLConnection()
	: eID(DATABASEID_NULL), sqlite_(new summit::db::SQLiteConnection())
{
	szDatabaseName[0] = 0;
}

SQLConnection::~SQLConnection() = default;

bool SQLConnection::Init(EDatabaseID databaseId)
{
	const DatabaseStructureData *data = GetDatabaseData(databaseId);
	if(!data)
	{
		Utils_Log(LOG_SQL, "SQLite: Database ID invalido.");
		return false;
	}

	eID = databaseId;
	std::strncpy(szDatabaseName, data->pszDatabaseName, sizeof(szDatabaseName) - 1);
	szDatabaseName[sizeof(szDatabaseName) - 1] = 0;

	if(!sqlite_->Init(databaseId, "Database"))
	{
		std::cerr << "SQLite: falha ao abrir " << szDatabaseName
			<< " em " << sqlite_->GetDatabasePath() << ": "
			<< sqlite_->GetLastError() << std::endl;
		Utils_Log(LOG_SQL, sqlite_->GetLastError());
		return false;
	}
	return true;
}

BOOL SQLConnection::SQLTimeStampToSystemTime(SYSTEMTIME *systemTime, SQL_TIMESTAMP_STRUCT *sqlTimeStamp)
{
	if(!systemTime || !sqlTimeStamp)
		return FALSE;

	systemTime->wYear = static_cast<WORD>(sqlTimeStamp->year);
	systemTime->wMonth = static_cast<WORD>(sqlTimeStamp->month);
	systemTime->wDayOfWeek = 0;
	systemTime->wDay = static_cast<WORD>(sqlTimeStamp->day);
	systemTime->wHour = static_cast<WORD>(sqlTimeStamp->hour);
	systemTime->wMinute = static_cast<WORD>(sqlTimeStamp->minute);
	systemTime->wSecond = static_cast<WORD>(sqlTimeStamp->second);
	systemTime->wMilliseconds = static_cast<WORD>(sqlTimeStamp->fraction / 1000000U);
	return TRUE;
}

BOOL SQLConnection::Open() { return sqlite_ ? sqlite_->Open() : FALSE; }
BOOL SQLConnection::Prepare(const char *query) { return sqlite_ ? sqlite_->Prepare(query) : FALSE; }
BOOL SQLConnection::Execute(BOOL fetch) { return sqlite_ ? sqlite_->Execute(fetch) : FALSE; }
BOOL SQLConnection::BindInputParameter(void *p, int pos, EDatabaseDataType type)
{
	return sqlite_ ? sqlite_->BindInputParameter(p, pos, type) : FALSE;
}
BOOL SQLConnection::GetData(void *p, int size, int pos, EDatabaseDataType type)
{
	return sqlite_ ? sqlite_->GetData(p, size, pos, type) : FALSE;
}
BOOL SQLConnection::NextRow() { return sqlite_ ? sqlite_->NextRow() : FALSE; }
BOOL SQLConnection::BeginTransaction() { return sqlite_ ? sqlite_->BeginTransaction() : FALSE; }
BOOL SQLConnection::CommitTransaction() { return sqlite_ ? sqlite_->CommitTransaction() : FALSE; }
BOOL SQLConnection::RollbackTransaction() { return sqlite_ ? sqlite_->RollbackTransaction() : FALSE; }
BOOL SQLConnection::Close() { return sqlite_ ? sqlite_->Close() : FALSE; }
const char *SQLConnection::GetLastError() const { return sqlite_ ? sqlite_->GetLastError() : "SQLite: conexao inexistente."; }

void CreateSQLConnection(EDatabaseID databaseId)
{
	std::shared_ptr<SQLConnection> connection = std::make_shared<SQLConnection>();
	if(connection->Init(databaseId))
	{
		vSQLConnection.push_back(connection);
		return;
	}

	std::cerr << "Falha ao abrir summit.sqlite3 para "
		<< (summit::db::LogicalDatabaseName(databaseId) ? summit::db::LogicalDatabaseName(databaseId) : "<unknown>")
		<< std::endl;
	Utils_Log(LOG_SERVER, "Falha ao abrir summit.sqlite3.");
	Sleep(1500);
	exit(0);
}

void openDatabase()
{
	const EDatabaseID databases[] =
	{
		DATABASEID_AccountDB,
		DATABASEID_BillingDB,
		DATABASEID_BillingLogDB,
		DATABASEID_ClanDB,
		DATABASEID_GameLogDB,
		DATABASEID_ItemLogDB,
		DATABASEID_LogDB,
		DATABASEID_PCRoom,
		DATABASEID_PCRoomLog,
		DATABASEID_rPTDB,
		DATABASEID_SoD2DB,
		DATABASEID_SodDB,
		DATABASEID_ServerDB,
		DATABASEID_SecurityDB,
		DATABASEID_CoinShopDB,
		DATABASEID_QuestDB,
		DATABASEID_StartUserDB,
	};

	std::cout << "Abrindo SQLite: Database/summit.sqlite3" << std::endl;
	for(std::size_t i = 0; i < sizeof(databases) / sizeof(databases[0]); ++i)
		CreateSQLConnection(databases[i]);
	std::cout << "SQLite pronto (" << vSQLConnection.size() << " contextos logicos)." << std::endl;
}

std::shared_ptr<SQLConnection> SQLConnection::GetConnection(EDatabaseID databaseId)
{
	for(std::size_t i = 0; i < vSQLConnection.size(); ++i)
		if(vSQLConnection[i] && vSQLConnection[i]->eID == databaseId)
			return vSQLConnection[i];
	return nullptr;
}
