#pragma once

#include <memory>

#include "systems/persistence/database/core/Database.h"

namespace summit
{
namespace db
{

// SQLite migration target for the legacy SQLConnection API.
//
// This class deliberately mirrors the legacy call pattern so consumers can be
// moved one subsystem at a time. It loads sqlite3.dll dynamically; adding this
// source does not add a linker dependency to the existing server build.
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
	BOOL Close();

	const char *GetLastError() const;
	const char *GetDatabasePath() const;

private:
	struct Impl;
	std::unique_ptr<Impl> impl_;
};

} // namespace db
} // namespace summit
