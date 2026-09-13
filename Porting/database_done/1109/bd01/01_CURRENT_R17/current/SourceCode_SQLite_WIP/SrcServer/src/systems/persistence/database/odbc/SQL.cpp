#include "systems/persistence/database/odbc/SQL.h"
#include "systems/persistence/database/sqlite/SQLiteConnection.h"

SQL::SQL()
	: m_bConnect(FALSE), m_sectionInitialized(false), sqlite_(new summit::db::SQLiteConnection())
{
}

SQL::~SQL()
{
	Disconnect();
}

BOOL SQL::Connect(LPCSTR, LPCSTR, LPCSTR)
{
	if(m_bConnect)
		return TRUE;

	if(!m_sectionInitialized)
	{
		InitializeCriticalSection(&m_SqlSection);
		m_sectionInitialized = true;
	}

	// DATABASEID_NULL means queries may select logical DBs explicitly. Bare
	// legacy table names are resolved by SqliteQueryCompat's historic mapping.
	if(!sqlite_->Init(DATABASEID_NULL, "Database"))
		return FALSE;

	m_bConnect = TRUE;
	return TRUE;
}

void SQL::Disconnect()
{
	if(!m_bConnect && !m_sectionInitialized)
		return;

	m_bConnect = FALSE;
	sqlite_.reset(new summit::db::SQLiteConnection());
	if(m_sectionInitialized)
	{
		DeleteCriticalSection(&m_SqlSection);
		m_sectionInitialized = false;
	}
}

void SQL::Init()
{
	if(!m_sectionInitialized)
	{
		InitializeCriticalSection(&m_SqlSection);
		m_sectionInitialized = true;
	}
}

void SQL::EnterSqlSection()
{
	if(m_sectionInitialized)
		EnterCriticalSection(&m_SqlSection);
}

void SQL::LeaveSqlSection()
{
	if(m_sectionInitialized)
		LeaveCriticalSection(&m_SqlSection);
}

BOOL SQL::Execute(LPCSTR query)
{
	if(!m_bConnect || !sqlite_ || !sqlite_->Prepare(query))
		return FALSE;
	return sqlite_->Execute(FALSE);
}

BOOL SQL::Fetch()
{
	return (m_bConnect && sqlite_) ? sqlite_->NextRow() : FALSE;
}

long SQL::GetColLength(int nCol)
{
	return (m_bConnect && sqlite_) ? sqlite_->GetColumnLength(nCol) : 0;
}

BOOL SQL::GetDataValue(int nCol, char *szData)
{
	return (m_bConnect && sqlite_) ? sqlite_->GetData(szData, 0, nCol, PARAMTYPE_String) : FALSE;
}

BOOL SQL::GetDataValue(int nCol, int *nData)
{
	return (m_bConnect && sqlite_) ? sqlite_->GetData(nData, 0, nCol, PARAMTYPE_Integer) : FALSE;
}
