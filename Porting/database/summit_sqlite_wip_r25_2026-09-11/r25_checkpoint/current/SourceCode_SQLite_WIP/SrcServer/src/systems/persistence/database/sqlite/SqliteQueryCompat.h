#pragma once

#include <string>

#include "systems/persistence/database/core/Database.h"

namespace summit
{
namespace db
{

const char *LogicalDatabaseName(EDatabaseID databaseId);

// Rewrites the SQL Server dialect and logical database/table names used by the
// legacy server into the single-file SQLite layout. The caller's DATABASEID is
// used only for unqualified table names. Explicit [Database].[dbo].[Table]
// references always win.
bool RewriteLegacyQuery(
	const char *input,
	EDatabaseID currentDatabase,
	std::string &output,
	std::string &error
);

} // namespace db
} // namespace summit
