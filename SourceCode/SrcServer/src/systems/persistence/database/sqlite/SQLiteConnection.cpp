#include "systems/persistence/database/sqlite/SQLiteConnection.h"
#include "systems/persistence/database/sqlite/SqliteQueryCompat.h"
#include "systems/persistence/database/sqlite/SqliteQueryCompat.inl"

#include <windows.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>

namespace
{

struct sqlite3;
struct sqlite3_stmt;

typedef long long sqlite3_int64;
typedef void (*sqlite3_destructor_type)(void *);

const int SQLITE_OK = 0;
const int SQLITE_INTEGER = 1;
const int SQLITE_FLOAT = 2;
const int SQLITE_TEXT = 3;
const int SQLITE_NULL = 5;
const int SQLITE_ROW = 100;
const int SQLITE_DONE = 101;

const int SQLITE_OPEN_READWRITE = 0x00000002;
const int SQLITE_OPEN_CREATE = 0x00000004;
const int SQLITE_OPEN_FULLMUTEX = 0x00010000;

sqlite3_destructor_type SqliteTransient()
{
	return reinterpret_cast<sqlite3_destructor_type>(-1);
}

} // namespace

namespace summit
{
namespace db
{

// R26_SQLITE_RUNTIME_ERROR_LOG
static void R26LogSQLiteRuntimeError(
	const char *stage,
	EDatabaseID databaseId,
	const char *query,
	const char *error)
{
	CreateDirectoryA("LOGS", NULL);

	FILE *f = std::fopen("LOGS\\sqlite_runtime_errors.log", "a");
	if(!f)
		return;

	SYSTEMTIME now = {};
	GetLocalTime(&now);

	std::fprintf(
		f,
		"%04u-%02u-%02u %02u:%02u:%02u stage=%s dbid=%d error=%s query=%s\n",
		static_cast<unsigned>(now.wYear),
		static_cast<unsigned>(now.wMonth),
		static_cast<unsigned>(now.wDay),
		static_cast<unsigned>(now.wHour),
		static_cast<unsigned>(now.wMinute),
		static_cast<unsigned>(now.wSecond),
		stage ? stage : "<null>",
		static_cast<int>(databaseId),
		error ? error : "<none>",
		query ? query : "<none>"
	);

	std::fclose(f);
}


struct SQLiteConnection::Impl
{
	typedef int (*OpenV2Fn)(const char *, sqlite3 **, int, const char *);
	typedef int (*CloseFn)(sqlite3 *);
	typedef const char *(*ErrMsgFn)(sqlite3 *);
	typedef int (*PrepareV2Fn)(sqlite3 *, const char *, int, sqlite3_stmt **, const char **);
	typedef int (*FinalizeFn)(sqlite3_stmt *);
	typedef int (*ResetFn)(sqlite3_stmt *);
	typedef int (*ClearBindingsFn)(sqlite3_stmt *);
	typedef int (*StepFn)(sqlite3_stmt *);
	typedef int (*StmtReadonlyFn)(sqlite3_stmt *);
	typedef int (*BindIntFn)(sqlite3_stmt *, int, int);
	typedef int (*BindInt64Fn)(sqlite3_stmt *, int, sqlite3_int64);
	typedef int (*BindDoubleFn)(sqlite3_stmt *, int, double);
	typedef int (*BindTextFn)(sqlite3_stmt *, int, const char *, int, sqlite3_destructor_type);
	typedef int (*ColumnIntFn)(sqlite3_stmt *, int);
	typedef sqlite3_int64 (*ColumnInt64Fn)(sqlite3_stmt *, int);
	typedef double (*ColumnDoubleFn)(sqlite3_stmt *, int);
	typedef const unsigned char *(*ColumnTextFn)(sqlite3_stmt *, int);
	typedef int (*ColumnTypeFn)(sqlite3_stmt *, int);
	typedef int (*ColumnBytesFn)(sqlite3_stmt *, int);
	typedef int (*BusyTimeoutFn)(sqlite3 *, int);
	typedef int (*ExecFn)(sqlite3 *, const char *, int (*)(void *, int, char **, char **), void *, char **);

	HMODULE library = NULL;
	sqlite3 *database = nullptr;
	sqlite3_stmt *statement = nullptr;
	EDatabaseID databaseId = DATABASEID_NULL;
	std::string databasePath;
	std::string query;
	std::string lastError;
	CRITICAL_SECTION criticalSection;
	bool criticalSectionInitialized = false;
	int lockDepth = 0;
	bool transactionActive = false;

	OpenV2Fn openV2 = nullptr;
	CloseFn close = nullptr;
	ErrMsgFn errMsg = nullptr;
	PrepareV2Fn prepareV2 = nullptr;
	FinalizeFn finalize = nullptr;
	ResetFn reset = nullptr;
	ClearBindingsFn clearBindings = nullptr;
	StepFn step = nullptr;
	StmtReadonlyFn stmtReadonly = nullptr;
	BindIntFn bindInt = nullptr;
	BindInt64Fn bindInt64 = nullptr;
	BindDoubleFn bindDouble = nullptr;
	BindTextFn bindText = nullptr;
	ColumnIntFn columnInt = nullptr;
	ColumnInt64Fn columnInt64 = nullptr;
	ColumnDoubleFn columnDouble = nullptr;
	ColumnTextFn columnText = nullptr;
	ColumnTypeFn columnType = nullptr;
	ColumnBytesFn columnBytes = nullptr;
	BusyTimeoutFn busyTimeout = nullptr;
	ExecFn exec = nullptr;

	~Impl()
	{
		FinalizeStatement();
		if(database && transactionActive && exec)
			exec(database, "ROLLBACK;", nullptr, nullptr, nullptr);
		transactionActive = false;
		if(database && close)
			close(database);
		database = nullptr;
		if(criticalSectionInitialized)
			DeleteCriticalSection(&criticalSection);
		criticalSectionInitialized = false;
		if(library)
			FreeLibrary(library);
		library = NULL;
	}

	void SetDatabaseError(const char *fallback)
	{
		if(database && errMsg)
		{
			const char *message = errMsg(database);
			if(message && message[0])
			{
				lastError = message;
				return;
			}
		}
		lastError = fallback ? fallback : "SQLite: erro desconhecido.";
	}

	void FinalizeStatement()
	{
		if(statement && finalize)
			finalize(statement);
		statement = nullptr;
		query.clear();
	}

	template <typename T>
	bool Load(T &target, const char *name)
	{
		FARPROC proc = GetProcAddress(library, name);
		if(!proc)
		{
			lastError = "SQLite: simbolo ausente em sqlite3.dll: ";
			lastError += name;
			return false;
		}
		target = reinterpret_cast<T>(proc);
		return true;
	}

	static int UserVersionCallback(void *context, int argc, char **argv, char **)
	{
		if(!context || argc < 1 || !argv || !argv[0])
			return 0;
		*static_cast<int *>(context) = std::atoi(argv[0]);
		return 0;
	}

	bool ReadUserVersion(int &version)
	{
		version = 0;
		if(exec(database, "PRAGMA user_version;", &UserVersionCallback, &version, nullptr) != SQLITE_OK)
		{
			SetDatabaseError("SQLite: falha ao ler PRAGMA user_version.");
			return false;
		}
		return true;
	}

	bool ReadMigrationFile(const std::string &path, std::string &contents)
	{
		std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
		if(!file)
		{
			lastError = "SQLite: migration ausente: ";
			lastError += path;
			return false;
		}

		std::ostringstream buffer;
		buffer << file.rdbuf();
		if(!file.good() && !file.eof())
		{
			lastError = "SQLite: falha ao ler migration: ";
			lastError += path;
			return false;
		}

		contents = buffer.str();
		if(contents.empty())
		{
			lastError = "SQLite: migration vazia: ";
			lastError += path;
			return false;
		}
		return true;
	}

	bool ApplyMigrations(const char *databaseDirectory)
	{
		struct MigrationSpec
		{
			int version;
			const char *filename;
		};

		static const MigrationSpec migrations[] =
		{
			{ 1, "001_accountdb.sql" },
			{ 2, "002_coinshopdb.sql" },
			{ 3, "003_questdb_core.sql" },
			{ 4, "004_serverdb_pvp.sql" },
			{ 5, "005_questdb_progress.sql" },
			{ 6, "006_serverdb_active_contracts.sql" },
			{ 7, "007_securitydb_active_contracts.sql" },
			{ 8, "008_serverdb_premiumdata.sql" },
			{ 9, "009_questdb_rewards.sql" },
			{ 10, "010_serverdb_notice.sql" },
			{ 11, "011_securitydb_catalog.sql" },
			{ 12, "012_startuserdb.sql" },
			{ 13, "013_active_legacy_catalogs.sql" },
			{ 14, "014_coinshop_identity.sql" },
			{ 15, "015_source_only_legacy_contracts.sql" },
			{ 16, "016_clandb_members.sql" },
			{ 17, "017_clandb_deleted_archives.sql" },
			{ 18, "018_billingdb_catalog.sql" },
			{ 19, "019_billinglogdb_catalog.sql" },
			{ 20, "020_gamelogdb_catalog.sql" },
			{ 21, "021_itemlogdb_catalog.sql" },
			{ 22, "022_logdb_catalog.sql" },
			{ 23, "023_pcroom_catalog.sql" },
			{ 24, "024_pcroomlog_catalog.sql" },
			{ 25, "025_rptdb_catalog.sql" },
			{ 26, "026_sod2db_catalog.sql" },
			{ 27, "027_soddb_catalog.sql" },
			{ 28, "028_clandb_catalog.sql" },
			{ 29, "029_remaining_backup_catalogs.sql" },
			{ 30, "030_local_gameplay_entitlement.sql" },
			{ 31, "031_local_audit_events.sql" },
			{ 32, "032_local_auth_policy.sql" },
			{ 33, "033_progression_v2.sql" },
			{ 34, "034_story_node_state.sql" },
			{ 35, "035_progression_content.sql" },
			{ 36, "036_covenant_rewards.sql" },
			{ 37, "037_story_dialogue_choices.sql" },
			{ 38, "038_generic_reputation.sql" },
			{ 39, "039_covenant_single_active.sql" },
			{ 40, "040_covenant_default_content.sql" },
			{ 41, "041_covenant_runtime_complete.sql" },
			{ 42, "042_covenant_inventory.sql" },
			{ 43, "043_covenant_artisan_mode.sql" },
			{ 44, "044_gathering_mining_runtime.sql" },
			{ 45, "045_gathering_serverdb_namespace_fix.sql" },
			{ 46, "046_fishing_vertical.sql" },
			{ 47, "047_premium_purchase_receipts.sql" },
			{ 48, "048_professions_crafting_clan_spawn_361.sql" },
			{ 49, "049_professions_bundle_372_378.sql" }
		};

		int currentVersion = 0;
		if(!ReadUserVersion(currentVersion))
			return false;

		const int latestVersion = migrations[(sizeof(migrations) / sizeof(migrations[0])) - 1].version;
		if(currentVersion > latestVersion)
		{
			std::ostringstream message;
			message << "SQLite: summit.sqlite3 usa schema version " << currentVersion
				<< ", mas este servidor suporta ate " << latestVersion << ".";
			lastError = message.str();
			return false;
		}

		std::string migrationDirectory = databaseDirectory ? databaseDirectory : "";
		if(!migrationDirectory.empty() && migrationDirectory.back() != '\\' && migrationDirectory.back() != '/')
			migrationDirectory += '\\';
		migrationDirectory += "migrations\\";

		for(size_t i = 0; i < sizeof(migrations) / sizeof(migrations[0]); ++i)
		{
			const MigrationSpec &migration = migrations[i];
			if(migration.version <= currentVersion)
				continue;

			const std::string path = migrationDirectory + migration.filename;
			std::string sql;
			if(!ReadMigrationFile(path, sql))
				return false;

			if(exec(database, "BEGIN IMMEDIATE;", nullptr, nullptr, nullptr) != SQLITE_OK)
			{
				SetDatabaseError("SQLite: falha ao iniciar transaction de migration.");
				return false;
			}

			if(exec(database, sql.c_str(), nullptr, nullptr, nullptr) != SQLITE_OK)
			{
				const char *rawError = errMsg ? errMsg(database) : nullptr;
				std::string detail = (rawError && rawError[0]) ? rawError : "erro SQLite desconhecido";
				exec(database, "ROLLBACK;", nullptr, nullptr, nullptr);
				lastError = "SQLite: falha na migration ";
				lastError += migration.filename;
				lastError += ": ";
				lastError += detail;
				return false;
			}

			std::ostringstream setVersion;
			setVersion << "PRAGMA user_version=" << migration.version << ";";
			if(exec(database, setVersion.str().c_str(), nullptr, nullptr, nullptr) != SQLITE_OK ||
			   exec(database, "COMMIT;", nullptr, nullptr, nullptr) != SQLITE_OK)
			{
				const char *rawError = errMsg ? errMsg(database) : nullptr;
				std::string detail = (rawError && rawError[0]) ? rawError : "erro SQLite desconhecido";
				exec(database, "ROLLBACK;", nullptr, nullptr, nullptr);
				lastError = "SQLite: falha ao concluir migration ";
				lastError += migration.filename;
				lastError += ": ";
				lastError += detail;
				return false;
			}

			currentVersion = migration.version;
		}

		return true;
	}

	bool LoadApi()
	{
		if(library)
			return true;

		library = LoadLibraryA("sqlite3.dll");
		if(!library)
		{
			lastError = "SQLite: sqlite3.dll x86 nao encontrada ao lado do servidor ou no PATH.";
			return false;
		}

		const bool loaded =
			Load(openV2, "sqlite3_open_v2") &&
			Load(close, "sqlite3_close") &&
			Load(errMsg, "sqlite3_errmsg") &&
			Load(prepareV2, "sqlite3_prepare_v2") &&
			Load(finalize, "sqlite3_finalize") &&
			Load(reset, "sqlite3_reset") &&
			Load(clearBindings, "sqlite3_clear_bindings") &&
			Load(step, "sqlite3_step") &&
			Load(stmtReadonly, "sqlite3_stmt_readonly") &&
			Load(bindInt, "sqlite3_bind_int") &&
			Load(bindInt64, "sqlite3_bind_int64") &&
			Load(bindDouble, "sqlite3_bind_double") &&
			Load(bindText, "sqlite3_bind_text") &&
			Load(columnInt, "sqlite3_column_int") &&
			Load(columnInt64, "sqlite3_column_int64") &&
			Load(columnDouble, "sqlite3_column_double") &&
			Load(columnText, "sqlite3_column_text") &&
			Load(columnType, "sqlite3_column_type") &&
			Load(columnBytes, "sqlite3_column_bytes") &&
			Load(busyTimeout, "sqlite3_busy_timeout") &&
			Load(exec, "sqlite3_exec");

		if(!loaded)
		{
			FreeLibrary(library);
			library = NULL;
		}
		return loaded;
	}
};

SQLiteConnection::SQLiteConnection()
	: impl_(new Impl())
{
}

SQLiteConnection::~SQLiteConnection()
{
}

bool SQLiteConnection::Init(EDatabaseID databaseId, const char *databaseDirectory)
{
	if(!databaseDirectory || !databaseDirectory[0])
	{
		impl_->lastError = "SQLite: diretorio invalido.";
		return false;
	}
	if(!impl_->LoadApi())
		return false;

	if(!CreateDirectoryA(databaseDirectory, NULL))
	{
		const DWORD error = ::GetLastError();
		if(error != ERROR_ALREADY_EXISTS)
		{
			std::ostringstream message;
			message << "SQLite: nao foi possivel criar diretorio '" << databaseDirectory
				<< "' (erro " << error << ").";
			impl_->lastError = message.str();
			return false;
		}
	}

	impl_->databaseId = databaseId;
	impl_->databasePath = databaseDirectory;
	if(!impl_->databasePath.empty() && impl_->databasePath.back() != '\\' && impl_->databasePath.back() != '/')
		impl_->databasePath += '\\';
	impl_->databasePath += "summit.sqlite3";

	const int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX;
	const int result = impl_->openV2(impl_->databasePath.c_str(), &impl_->database, flags, nullptr);
	if(result != SQLITE_OK || !impl_->database)
	{
		impl_->SetDatabaseError("SQLite: falha ao abrir summit.sqlite3.");
		return false;
	}

	impl_->busyTimeout(impl_->database, 5000);
	if(impl_->exec(impl_->database, "PRAGMA foreign_keys=ON;", nullptr, nullptr, nullptr) != SQLITE_OK ||
	   impl_->exec(impl_->database, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr) != SQLITE_OK ||
	   impl_->exec(impl_->database, "PRAGMA synchronous=NORMAL;", nullptr, nullptr, nullptr) != SQLITE_OK)
	{
		impl_->SetDatabaseError("SQLite: falha ao configurar pragmas.");
		return false;
	}

	if(!impl_->ApplyMigrations(databaseDirectory))
		return false;

	InitializeCriticalSection(&impl_->criticalSection);
	impl_->criticalSectionInitialized = true;
	impl_->lastError.clear();
	return true;
}

BOOL SQLiteConnection::Open()
{
	if(!impl_->database || !impl_->criticalSectionInitialized)
	{
		impl_->lastError = "SQLite: conexao nao inicializada.";
		return FALSE;
	}
	EnterCriticalSection(&impl_->criticalSection);
	++impl_->lockDepth;
	return TRUE;
}

BOOL SQLiteConnection::Prepare(const char *query)
{
	if(!impl_->database)
	{
		impl_->lastError = "SQLite: conexao nao inicializada.";
		return FALSE;
	}

	std::string normalized;
	if(!RewriteLegacyQuery(query, impl_->databaseId, normalized, impl_->lastError))
	{
		R26LogSQLiteRuntimeError(
			"rewrite",
			impl_->databaseId,
			query,
			impl_->lastError.c_str()
		);
		return FALSE;
	}

	impl_->FinalizeStatement();
	const int result = impl_->prepareV2(impl_->database, normalized.c_str(), -1, &impl_->statement, nullptr);
	if(result != SQLITE_OK || !impl_->statement)
	{
		impl_->query = normalized;
		impl_->SetDatabaseError("SQLite: falha ao preparar query.");
		R26LogSQLiteRuntimeError(
			"prepare",
			impl_->databaseId,
			impl_->query.c_str(),
			impl_->lastError.c_str()
		);
		return FALSE;
	}
	impl_->query = normalized;
	impl_->lastError.clear();
	return TRUE;
}

BOOL SQLiteConnection::Execute(BOOL fetchFirstRow)
{
	if(!impl_->statement)
	{
		impl_->lastError = "SQLite: Execute sem statement preparado.";
		return FALSE;
	}

	if(!fetchFirstRow && impl_->stmtReadonly(impl_->statement))
		return TRUE;

	const int result = impl_->step(impl_->statement);
	if(fetchFirstRow)
	{
		if(result == SQLITE_ROW)
			return TRUE;
		if(result == SQLITE_DONE)
		{
			impl_->lastError.clear();
			return FALSE;
		}
	}
	else if(result == SQLITE_DONE || result == SQLITE_ROW)
	{
		return TRUE;
	}

	impl_->SetDatabaseError("SQLite: falha ao executar query.");
	R26LogSQLiteRuntimeError(
		"execute",
		impl_->databaseId,
		impl_->query.c_str(),
		impl_->lastError.c_str()
	);
	return FALSE;
}

BOOL SQLiteConnection::BindInputParameter(void *parameter, int position, EDatabaseDataType dataType)
{
	if(!impl_->statement || !parameter || position <= 0)
	{
		impl_->lastError = "SQLite: parametro de bind invalido.";
		return FALSE;
	}

	int result = SQLITE_OK;
	switch(dataType)
	{
	case PARAMTYPE_Integer:
		result = impl_->bindInt(impl_->statement, position, *static_cast<int *>(parameter));
		break;
	case PARAMTYPE_Short:
		result = impl_->bindInt(impl_->statement, position, *static_cast<short *>(parameter));
		break;
	case PARAMTYPE_String:
		result = impl_->bindText(impl_->statement, position, static_cast<const char *>(parameter), -1, SqliteTransient());
		break;
	case PARAMTYPE_Float:
		result = impl_->bindDouble(impl_->statement, position, *static_cast<float *>(parameter));
		break;
	case PARAMTYPE_Int64:
		result = impl_->bindInt64(impl_->statement, position, *static_cast<sqlite3_int64 *>(parameter));
		break;
	case PARAMTYPE_Time:
	{
		const SYSTEMTIME *time = static_cast<const SYSTEMTIME *>(parameter);
		char buffer[64] = { 0 };
		std::snprintf(buffer, sizeof(buffer), "%04u-%02u-%02u %02u:%02u:%02u",
			static_cast<unsigned>(time->wYear), static_cast<unsigned>(time->wMonth),
			static_cast<unsigned>(time->wDay), static_cast<unsigned>(time->wHour),
			static_cast<unsigned>(time->wMinute), static_cast<unsigned>(time->wSecond));
		result = impl_->bindText(impl_->statement, position, buffer, -1, SqliteTransient());
		break;
	}
	default:
		impl_->lastError = "SQLite: tipo de bind nao suportado.";
		return FALSE;
	}

	if(result != SQLITE_OK)
	{
		impl_->SetDatabaseError("SQLite: falha ao fazer bind.");
		R26LogSQLiteRuntimeError(
			"bind",
			impl_->databaseId,
			impl_->query.c_str(),
			impl_->lastError.c_str()
		);
		return FALSE;
	}
	return TRUE;
}

BOOL SQLiteConnection::GetData(void *parameter, int size, int position, EDatabaseDataType dataType)
{
	if(!impl_->statement || !parameter || position <= 0)
	{
		impl_->lastError = "SQLite: destino de coluna invalido.";
		return FALSE;
	}

	const int column = position - 1;
	if(impl_->columnType(impl_->statement, column) == SQLITE_NULL)
	{
		if(size > 0)
			std::memset(parameter, 0, static_cast<std::size_t>(size));
		else if(dataType == PARAMTYPE_Integer)
			*static_cast<int *>(parameter) = 0;
		else if(dataType == PARAMTYPE_Short)
			*static_cast<short *>(parameter) = 0;
		else if(dataType == PARAMTYPE_Float)
			*static_cast<float *>(parameter) = 0.0f;
		else if(dataType == PARAMTYPE_Int64)
			*static_cast<sqlite3_int64 *>(parameter) = 0;
		else if(dataType == PARAMTYPE_String)
			static_cast<char *>(parameter)[0] = 0;
		return TRUE;
	}

	switch(dataType)
	{
	case PARAMTYPE_Integer:
		*static_cast<int *>(parameter) = impl_->columnInt(impl_->statement, column);
		break;
	case PARAMTYPE_Short:
		*static_cast<short *>(parameter) = static_cast<short>(impl_->columnInt(impl_->statement, column));
		break;
	case PARAMTYPE_String:
	{
		const unsigned char *text = impl_->columnText(impl_->statement, column);
		char *destination = static_cast<char *>(parameter);
		if(!text)
		{
			destination[0] = 0;
			break;
		}
		if(size <= 0)
		{
			// Legacy SQL::GetDataValue did not receive destination capacity.
			std::strcpy(destination, reinterpret_cast<const char *>(text));
		}
		else
		{
			const std::size_t capacity = static_cast<std::size_t>(size);
			std::strncpy(destination, reinterpret_cast<const char *>(text), capacity - 1);
			destination[capacity - 1] = 0;
		}
		break;
	}
	case PARAMTYPE_Float:
		*static_cast<float *>(parameter) = static_cast<float>(impl_->columnDouble(impl_->statement, column));
		break;
	case PARAMTYPE_Int64:
		*static_cast<sqlite3_int64 *>(parameter) = impl_->columnInt64(impl_->statement, column);
		break;
	case PARAMTYPE_Time:
	{
		if(size > 0 && size < static_cast<int>(sizeof(SYSTEMTIME)))
			return FALSE;
		const unsigned char *text = impl_->columnText(impl_->statement, column);
		if(!text)
			return FALSE;
		unsigned int year=0,month=0,day=0,hour=0,minute=0,second=0;
		if(std::sscanf(reinterpret_cast<const char *>(text), "%u-%u-%u %u:%u:%u",
			&year,&month,&day,&hour,&minute,&second) != 6)
			return FALSE;
		SYSTEMTIME *time = static_cast<SYSTEMTIME *>(parameter);
		std::memset(time, 0, sizeof(*time));
		time->wYear=static_cast<WORD>(year); time->wMonth=static_cast<WORD>(month);
		time->wDay=static_cast<WORD>(day); time->wHour=static_cast<WORD>(hour);
		time->wMinute=static_cast<WORD>(minute); time->wSecond=static_cast<WORD>(second);
		break;
	}
	default:
		impl_->lastError = "SQLite: tipo de coluna nao suportado.";
		return FALSE;
	}
	return TRUE;
}

BOOL SQLiteConnection::NextRow()
{
	if(!impl_->statement)
		return FALSE;
	const int result = impl_->step(impl_->statement);
	if(result == SQLITE_ROW)
		return TRUE;
	if(result == SQLITE_DONE)
	{
		impl_->lastError.clear();
		return FALSE;
	}
	impl_->SetDatabaseError("SQLite: falha ao avancar cursor.");
	return FALSE;
}

BOOL SQLiteConnection::BeginTransaction()
{
	if(!impl_->database || impl_->lockDepth <= 0)
	{
		impl_->lastError = "SQLite: BeginTransaction requer Open().";
		return FALSE;
	}
	if(impl_->transactionActive)
	{
		impl_->lastError = "SQLite: transacao ja ativa.";
		return FALSE;
	}

	impl_->FinalizeStatement();
	if(impl_->exec(impl_->database, "BEGIN IMMEDIATE;", nullptr, nullptr, nullptr) != SQLITE_OK)
	{
		impl_->SetDatabaseError("SQLite: falha ao iniciar transacao.");
		return FALSE;
	}

	impl_->transactionActive = true;
	impl_->lastError.clear();
	return TRUE;
}

BOOL SQLiteConnection::CommitTransaction()
{
	if(!impl_->database || !impl_->transactionActive)
	{
		impl_->lastError = "SQLite: nenhuma transacao ativa para commit.";
		return FALSE;
	}

	impl_->FinalizeStatement();
	if(impl_->exec(impl_->database, "COMMIT;", nullptr, nullptr, nullptr) != SQLITE_OK)
	{
		impl_->SetDatabaseError("SQLite: falha ao confirmar transacao.");
		return FALSE;
	}

	impl_->transactionActive = false;
	impl_->lastError.clear();
	return TRUE;
}

BOOL SQLiteConnection::RollbackTransaction()
{
	if(!impl_->database || !impl_->transactionActive)
	{
		impl_->lastError = "SQLite: nenhuma transacao ativa para rollback.";
		return FALSE;
	}

	impl_->FinalizeStatement();
	if(impl_->exec(impl_->database, "ROLLBACK;", nullptr, nullptr, nullptr) != SQLITE_OK)
	{
		impl_->SetDatabaseError("SQLite: falha ao reverter transacao.");
		return FALSE;
	}

	impl_->transactionActive = false;
	impl_->lastError.clear();
	return TRUE;
}

BOOL SQLiteConnection::Close()
{
	if(impl_->statement)
	{
		impl_->reset(impl_->statement);
		impl_->clearBindings(impl_->statement);
	}
	if(impl_->transactionActive && impl_->lockDepth == 1)
	{
		impl_->FinalizeStatement();
		if(impl_->exec(impl_->database, "ROLLBACK;", nullptr, nullptr, nullptr) != SQLITE_OK)
		{
			impl_->SetDatabaseError("SQLite: falha ao reverter transacao aberta no Close().");
			return FALSE;
		}
		impl_->transactionActive = false;
	}
	if(impl_->criticalSectionInitialized && impl_->lockDepth > 0)
	{
		--impl_->lockDepth;
		LeaveCriticalSection(&impl_->criticalSection);
	}
	return TRUE;
}

int SQLiteConnection::GetColumnLength(int position) const
{
	if(!impl_->statement || position <= 0)
		return 0;
	return impl_->columnBytes(impl_->statement, position - 1);
}

const char *SQLiteConnection::GetLastError() const { return impl_->lastError.c_str(); }
const char *SQLiteConnection::GetDatabasePath() const { return impl_->databasePath.c_str(); }
EDatabaseID SQLiteConnection::GetDatabaseId() const { return impl_->databaseId; }

} // namespace db
} // namespace summit
