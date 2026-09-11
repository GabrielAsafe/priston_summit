#include "systems/persistence/database/sqlite/SQLiteConnection.h"

#include <windows.h>

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <limits>
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

bool StartsWithNoCase(const std::string &value, const char *prefix)
{
	if(!prefix)
		return false;

	const std::size_t prefixLength = std::strlen(prefix);
	if(value.size() < prefixLength)
		return false;

	for(std::size_t i = 0; i < prefixLength; ++i)
	{
		if(std::toupper(static_cast<unsigned char>(value[i])) !=
		   std::toupper(static_cast<unsigned char>(prefix[i])))
			return false;
	}

	return true;
}

std::size_t FindNoCase(const std::string &value, const char *needle, std::size_t start = 0)
{
	if(!needle || !needle[0])
		return std::string::npos;

	const std::size_t needleLength = std::strlen(needle);
	if(needleLength > value.size())
		return std::string::npos;

	for(std::size_t i = start; i + needleLength <= value.size(); ++i)
	{
		bool matches = true;
		for(std::size_t j = 0; j < needleLength; ++j)
		{
			if(std::toupper(static_cast<unsigned char>(value[i + j])) !=
			   std::toupper(static_cast<unsigned char>(needle[j])))
			{
				matches = false;
				break;
			}
		}

		if(matches)
			return i;
	}

	return std::string::npos;
}

void ReplaceAllNoCase(std::string &value, const char *needle, const char *replacement)
{
	const std::size_t needleLength = std::strlen(needle);
	const std::size_t replacementLength = std::strlen(replacement);
	std::size_t position = 0;

	while((position = FindNoCase(value, needle, position)) != std::string::npos)
	{
		value.replace(position, needleLength, replacement);
		position += replacementLength;
	}
}

bool NormalizeTopClause(std::string &query, std::string &error)
{
	const char *prefix = "SELECT TOP ";
	if(!StartsWithNoCase(query, prefix))
		return true;

	std::size_t position = std::strlen(prefix);
	std::size_t numberStart = position;
	while(position < query.size() && std::isdigit(static_cast<unsigned char>(query[position])))
		++position;

	if(position == numberStart || position >= query.size() || !std::isspace(static_cast<unsigned char>(query[position])))
	{
		error = "SQLite migration: SELECT TOP sem limite inteiro nao e suportado.";
		return false;
	}

	const std::string limit = query.substr(numberStart, position - numberStart);
	while(position < query.size() && std::isspace(static_cast<unsigned char>(query[position])))
		++position;

	query = "SELECT " + query.substr(position);

	while(!query.empty() && std::isspace(static_cast<unsigned char>(query.back())))
		query.pop_back();

	bool hadSemicolon = false;
	if(!query.empty() && query.back() == ';')
	{
		hadSemicolon = true;
		query.pop_back();
	}

	query += " LIMIT " + limit;
	if(hadSemicolon)
		query += ';';

	return true;
}

bool NormalizeQuery(const char *input, std::string &output, std::string &error)
{
	if(!input || !input[0])
	{
		error = "SQLite migration: query vazia.";
		return false;
	}

	output = input;
	ReplaceAllNoCase(output, "GETDATE()", "CURRENT_TIMESTAMP");

	if(!NormalizeTopClause(output, error))
		return false;

	const char *unsupported[] = {
		"IF EXISTS(",
		"IF NOT EXISTS(",
		" BEGIN ",
		" ELSE ",
		"COLLATE SQL_",
		".."
	};

	for(std::size_t i = 0; i < sizeof(unsupported) / sizeof(unsupported[0]); ++i)
	{
		if(FindNoCase(output, unsupported[i]) != std::string::npos)
		{
			error = "SQLite migration: query ainda contem sintaxe T-SQL nao portavel: ";
			error += unsupported[i];
			return false;
		}
	}

	return true;
}

} // namespace

namespace summit
{
namespace db
{

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
	BusyTimeoutFn busyTimeout = nullptr;
	ExecFn exec = nullptr;

	~Impl()
	{
		FinalizeStatement();

		if(database && close)
			close(database);
		database = nullptr;

		if(criticalSectionInitialized)
		{
			DeleteCriticalSection(&criticalSection);
			criticalSectionInitialized = false;
		}

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

	bool LoadSymbol(FARPROC &target, const char *name)
	{
		target = GetProcAddress(library, name);
		if(target)
			return true;

		lastError = "SQLite: simbolo ausente em sqlite3.dll: ";
		lastError += name;
		return false;
	}

	template <typename T>
	bool Load(T &target, const char *name)
	{
		FARPROC proc = NULL;
		if(!LoadSymbol(proc, name))
			return false;
		target = reinterpret_cast<T>(proc);
		return true;
	}

	bool LoadApi()
	{
		if(library)
			return true;

		library = LoadLibraryA("sqlite3.dll");
		if(!library)
		{
			lastError = "SQLite: sqlite3.dll nao encontrada no PATH/diretorio do servidor.";
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
	const char *databaseName = GetDatabaseName(databaseId);
	if(!databaseName || !databaseDirectory || !databaseDirectory[0])
	{
		impl_->lastError = "SQLite: database ID ou diretorio invalido.";
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
			message << "SQLite: nao foi possivel criar diretorio '"
				<< databaseDirectory << "' (erro " << error << ").";
			impl_->lastError = message.str();
			return false;
		}
	}

	impl_->databaseId = databaseId;
	impl_->databasePath = databaseDirectory;
	if(!impl_->databasePath.empty() &&
	   impl_->databasePath.back() != '\\' &&
	   impl_->databasePath.back() != '/')
		impl_->databasePath += '\\';
	impl_->databasePath += databaseName;
	impl_->databasePath += ".sqlite3";

	const int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX;
	const int result = impl_->openV2(impl_->databasePath.c_str(), &impl_->database, flags, nullptr);
	if(result != SQLITE_OK || !impl_->database)
	{
		impl_->SetDatabaseError("SQLite: falha ao abrir database.");
		return false;
	}

	impl_->busyTimeout(impl_->database, 5000);

	// These pragmas are conservative defaults for a game server: foreign keys
	// become effective, while WAL permits readers during writes.
	if(impl_->exec(impl_->database, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr) != SQLITE_OK ||
	   impl_->exec(impl_->database, "PRAGMA journal_mode = WAL;", nullptr, nullptr, nullptr) != SQLITE_OK)
	{
		impl_->SetDatabaseError("SQLite: falha ao configurar pragmas iniciais.");
		return false;
	}

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
	if(!NormalizeQuery(query, normalized, impl_->lastError))
		return FALSE;

	impl_->FinalizeStatement();

	const int result = impl_->prepareV2(
		impl_->database,
		normalized.c_str(),
		static_cast<int>(normalized.size()),
		&impl_->statement,
		nullptr
	);

	if(result != SQLITE_OK || !impl_->statement)
	{
		impl_->SetDatabaseError("SQLite: falha ao preparar query.");
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
		impl_->lastError = "SQLite: Execute chamado sem statement preparado.";
		return FALSE;
	}

	// Legacy SQLConnection::Execute(FALSE) does not fetch SELECT results; callers
	// subsequently use NextRow(). For write statements SQLite must step now.
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
		result = impl_->bindText(
			impl_->statement,
			position,
			static_cast<const char *>(parameter),
			-1,
			SqliteTransient()
		);
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
		if(time->wYear < 1 || time->wMonth < 1 || time->wMonth > 12 ||
		   time->wDay < 1 || time->wDay > 31 || time->wHour > 23 ||
		   time->wMinute > 59 || time->wSecond > 59)
		{
			impl_->lastError = "SQLite: SYSTEMTIME invalido para bind.";
			return FALSE;
		}

		char buffer[64] = { 0 };
		const int written = std::snprintf(
			buffer,
			sizeof(buffer),
			"%04u-%02u-%02u %02u:%02u:%02u",
			static_cast<unsigned int>(time->wYear),
			static_cast<unsigned int>(time->wMonth),
			static_cast<unsigned int>(time->wDay),
			static_cast<unsigned int>(time->wHour),
			static_cast<unsigned int>(time->wMinute),
			static_cast<unsigned int>(time->wSecond)
		);

		if(written < 0 || written >= static_cast<int>(sizeof(buffer)))
		{
			impl_->lastError = "SQLite: timestamp excedeu buffer interno.";
			return FALSE;
		}

		result = impl_->bindText(impl_->statement, position, buffer, -1, SqliteTransient());
		break;
	}
	default:
		impl_->lastError = "SQLite: tipo de bind nao suportado.";
		return FALSE;
	}

	if(result != SQLITE_OK)
	{
		impl_->SetDatabaseError("SQLite: falha ao fazer bind de parametro.");
		return FALSE;
	}

	return TRUE;
}

BOOL SQLiteConnection::GetData(void *parameter, int size, int position, EDatabaseDataType dataType)
{
	if(!impl_->statement || !parameter || size <= 0 || position <= 0)
	{
		impl_->lastError = "SQLite: destino de coluna invalido.";
		return FALSE;
	}

	const int column = position - 1;
	if(impl_->columnType(impl_->statement, column) == SQLITE_NULL)
	{
		std::memset(parameter, 0, static_cast<std::size_t>(size));
		return TRUE;
	}

	switch(dataType)
	{
	case PARAMTYPE_Integer:
		if(size < static_cast<int>(sizeof(int)))
			return FALSE;
		*static_cast<int *>(parameter) = impl_->columnInt(impl_->statement, column);
		break;
	case PARAMTYPE_Short:
		if(size < static_cast<int>(sizeof(short)))
			return FALSE;
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

		const std::size_t capacity = static_cast<std::size_t>(size);
		std::strncpy(destination, reinterpret_cast<const char *>(text), capacity - 1);
		destination[capacity - 1] = 0;
		break;
	}
	case PARAMTYPE_Float:
		if(size < static_cast<int>(sizeof(float)))
			return FALSE;
		*static_cast<float *>(parameter) = static_cast<float>(impl_->columnDouble(impl_->statement, column));
		break;
	case PARAMTYPE_Int64:
		if(size < static_cast<int>(sizeof(sqlite3_int64)))
			return FALSE;
		*static_cast<sqlite3_int64 *>(parameter) = impl_->columnInt64(impl_->statement, column);
		break;
	case PARAMTYPE_Time:
	{
		if(size < static_cast<int>(sizeof(SYSTEMTIME)))
			return FALSE;

		const unsigned char *text = impl_->columnText(impl_->statement, column);
		if(!text)
			return FALSE;

		unsigned int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;
		if(std::sscanf(
			reinterpret_cast<const char *>(text),
			"%u-%u-%u %u:%u:%u",
			&year, &month, &day, &hour, &minute, &second
		) != 6)
		{
			impl_->lastError = "SQLite: timestamp fora do formato YYYY-MM-DD HH:MM:SS.";
			return FALSE;
		}

		SYSTEMTIME *time = static_cast<SYSTEMTIME *>(parameter);
		std::memset(time, 0, sizeof(*time));
		time->wYear = static_cast<WORD>(year);
		time->wMonth = static_cast<WORD>(month);
		time->wDay = static_cast<WORD>(day);
		time->wHour = static_cast<WORD>(hour);
		time->wMinute = static_cast<WORD>(minute);
		time->wSecond = static_cast<WORD>(second);
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

BOOL SQLiteConnection::Close()
{
	if(impl_->statement)
	{
		impl_->reset(impl_->statement);
		impl_->clearBindings(impl_->statement);
	}

	if(impl_->criticalSectionInitialized && impl_->lockDepth > 0)
	{
		--impl_->lockDepth;
		LeaveCriticalSection(&impl_->criticalSection);
	}

	return TRUE;
}

const char *SQLiteConnection::GetLastError() const
{
	return impl_->lastError.c_str();
}

const char *SQLiteConnection::GetDatabasePath() const
{
	return impl_->databasePath.c_str();
}

} // namespace db
} // namespace summit
