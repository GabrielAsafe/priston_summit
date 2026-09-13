#pragma once

#include <memory>
#include <vector>
#include <windows.h>

namespace summit { namespace db { class SQLiteConnection; } }

enum EDatabaseID
{
	DATABASEID_NULL,
	DATABASEID_AccountDB,
	DATABASEID_BillingDB,
	DATABASEID_BillingLogDB,
	DATABASEID_BattlePass,
	DATABASEID_ClanDB,
	DATABASEID_GameLogDB,
	DATABASEID_ItemLogDB,
	DATABASEID_LogDB,
	DATABASEID_PCRoom,
	DATABASEID_PCRoomLog,
	DATABASEID_rPTDB,
	DATABASEID_SoD2DB,
	DATABASEID_SodDB,
	DATABASEID_SecurityDB,
	DATABASEID_ServerDB,
	DATABASEID_CoinShopDB,
	DATABASEID_QuestDB,
	DATABASEID_StartUserDB,
};

enum EDatabaseDataType
{
	PARAMTYPE_Integer,
	PARAMTYPE_String,
	PARAMTYPE_Float,
	PARAMTYPE_Int64,
	PARAMTYPE_Short,
	PARAMTYPE_Time,
};

// ODBC exposed this timestamp layout through the legacy public header.
// Keep the type locally so callers do not need sqltypes.h after the SQLite
// migration. The field widths match SQL_TIMESTAMP_STRUCT on Win32 ODBC.
struct SQL_TIMESTAMP_STRUCT
{
	short year;
	unsigned short month;
	unsigned short day;
	unsigned short hour;
	unsigned short minute;
	unsigned short second;
	unsigned int fraction;
};

struct DatabaseStructureData
{
	EDatabaseID eDatabaseID;
	const char *pszDatabaseName;
};

class SQLConnection
{
private:
	EDatabaseID eID;
	char szDatabaseName[64];
	std::unique_ptr<summit::db::SQLiteConnection> sqlite_;

public:
	bool Init(EDatabaseID eDatabaseID);
	BOOL SQLTimeStampToSystemTime(SYSTEMTIME *psSystemTime, SQL_TIMESTAMP_STRUCT *psSQLTimeStamp);
	BOOL Open();
	BOOL Prepare(const char *pszQuery);
	BOOL Execute(BOOL bFetch = TRUE);
	BOOL BindInputParameter(void *pParameter, int iPosition, EDatabaseDataType eDataType);
	BOOL GetData(void *pParameter, int iSize, int iPosition, EDatabaseDataType eDataType);
	BOOL NextRow();
	BOOL BeginTransaction();
	BOOL CommitTransaction();
	BOOL RollbackTransaction();
	BOOL Close();

	const char *GetLastError() const;
	static std::shared_ptr<SQLConnection> GetConnection(EDatabaseID eDatabaseID);

	SQLConnection();
	virtual ~SQLConnection();
};

extern void openDatabase();
