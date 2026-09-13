#pragma once

#include <memory>

#include "systems/persistence/database/core/Database.h"

namespace summit
{
namespace db
{

// Thin SQLite implementation used by both legacy database APIs.
// Every logical database ID is stored in one physical Database/summit.sqlite3
// file; SqliteQueryCompat rewrites logical table names to DB__Table.
class SQLiteConnection
{
public:
	SQLiteConnection();
	~SQLiteConnection();

	SQLiteConnection(const SQLiteConnection &) = delete;
	SQLiteConnection &operator=(const SQLiteConnection &) = delete;

	bool Init(EDatabaseID databaseId, const char *databaseDirectory = "Database");

	BOOL Open();
	BOOL Prepare(const char *query);
	BOOL Execute(BOOL fetchFirstRow = TRUE);
	BOOL BindInputParameter(void *parameter, int position, EDatabaseDataType dataType);
	BOOL GetData(void *parameter, int size, int position, EDatabaseDataType dataType);
	BOOL NextRow();
	BOOL BeginTransaction();
	BOOL CommitTransaction();
	BOOL RollbackTransaction();
	BOOL Close();

	int GetColumnLength(int position) const;

	const char *GetLastError() const;
	const char *GetDatabasePath() const;
	EDatabaseID GetDatabaseId() const;

private:
	struct Impl;
	std::unique_ptr<Impl> impl_;
};

} // namespace db
} // namespace summit
