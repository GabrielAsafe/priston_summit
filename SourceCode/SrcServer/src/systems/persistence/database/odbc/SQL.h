#pragma once

#include <memory>
#include <windows.h>

namespace summit { namespace db { class SQLiteConnection; } }

// Legacy singleton API retained for gameplay code. The implementation no
// longer uses ODBC or SQL Server; it opens the same Database/summit.sqlite3
// used by SQLConnection.
class SQL
{
public:
	virtual ~SQL();

	static SQL *GetInstance()
	{
		static SQL instance;
		return &instance;
	}

	BOOL Connect(LPCSTR szServer, LPCSTR szUser, LPCSTR szPw);
	BOOL IsConnected() { return m_bConnect; }
	void Disconnect();
	void Init();
	void EnterSqlSection();
	void LeaveSqlSection();
	BOOL Execute(LPCSTR szQuery);
	BOOL Fetch();

	long GetColLength(int nCol);
	BOOL GetDataValue(int nCol, char *szData);
	BOOL GetDataValue(int nCol, int *nData);

private:
	SQL();
	CRITICAL_SECTION m_SqlSection;
	BOOL m_bConnect;
	bool m_sectionInitialized;
	std::unique_ptr<summit::db::SQLiteConnection> sqlite_;
};
