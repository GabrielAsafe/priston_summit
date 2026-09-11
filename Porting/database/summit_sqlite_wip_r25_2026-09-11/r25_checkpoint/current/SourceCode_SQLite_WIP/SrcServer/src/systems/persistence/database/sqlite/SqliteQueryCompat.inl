#include "systems/persistence/database/sqlite/SqliteQueryCompat.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <string>

namespace
{

bool IsIdentChar(char c)
{
	return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
}

std::string Upper(const std::string &value)
{
	std::string out = value;
	for(std::size_t i = 0; i < out.size(); ++i)
		out[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(out[i])));
	return out;
}

bool EqualsNoCase(const std::string &a, const char *b)
{
	if(!b)
		return false;
	const std::size_t n = std::strlen(b);
	if(a.size() != n)
		return false;
	for(std::size_t i = 0; i < n; ++i)
	{
		if(std::toupper(static_cast<unsigned char>(a[i])) !=
		   std::toupper(static_cast<unsigned char>(b[i])))
			return false;
	}
	return true;
}

std::size_t FindNoCase(const std::string &value, const std::string &needle, std::size_t start = 0)
{
	if(needle.empty() || needle.size() > value.size())
		return std::string::npos;

	for(std::size_t i = start; i + needle.size() <= value.size(); ++i)
	{
		bool matches = true;
		for(std::size_t j = 0; j < needle.size(); ++j)
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

void ReplaceAllNoCase(std::string &value, const std::string &needle, const std::string &replacement)
{
	std::size_t position = 0;
	while((position = FindNoCase(value, needle, position)) != std::string::npos)
	{
		value.replace(position, needle.size(), replacement);
		position += replacement.size();
	}
}

std::string StripBrackets(const std::string &value)
{
	if(value.size() >= 2 && value.front() == '[' && value.back() == ']')
		return value.substr(1, value.size() - 2);
	return value;
}

bool ReadIdentifier(const std::string &query, std::size_t &position, std::string &identifier)
{
	while(position < query.size() && std::isspace(static_cast<unsigned char>(query[position])))
		++position;
	if(position >= query.size())
		return false;

	if(query[position] == '[')
	{
		const std::size_t end = query.find(']', position + 1);
		if(end == std::string::npos)
			return false;
		identifier = query.substr(position + 1, end - position - 1);
		position = end + 1;
		return true;
	}

	if(!IsIdentChar(query[position]))
		return false;
	const std::size_t begin = position;
	while(position < query.size() && IsIdentChar(query[position]))
		++position;
	identifier = query.substr(begin, position - begin);
	return !identifier.empty();
}

std::string InferLegacyDatabase(const std::string &table)
{
	const std::string t = Upper(table);

	// Tables accessed through the old SQL singleton had no selected database.
	// The original SQL Server login's default database implicitly resolved them.
	// Keep that resolution explicit in the single SQLite file.
	if(t == "LEVEL" || t == "ITEMS" || t == "EXTRAITEMS")
		return "StartUserDB";
	if(t == "CL" || t == "UL")
		return "ClanDB";
	if(t == "GAMEMASTERS")
		return "AccountDB";
	if(t == "AGINGFAILED" || t == "AGINGRECORD" || t == "BANNEDIP" ||
	   t == "BANNEDMAC" || t == "CHEATLOG" || t == "FUNCTIONCHECKSUMLIST" ||
	   t == "WINDOWCHEATLIST" || t == "SECURITY" || t == "CLASSCHEATLIST")
		return "SecurityDB";
	if(t == "ITEMLIST" || t == "ITEMLIST_T" || t == "USERINFO" || t == "USERINFO_T")
		return "CoinShopDB";
	if(t == "COMPLETE" || t == "DAILYQUEST" || t == "ITEMREWARDS" ||
	   t == "MONSTERKILLED" || t == "PROGRESS" || t == "QUESTMONSTERS" ||
	   t == "REPEATEDQUEST" || t == "REWARDS" || t == "SINGLEQUEST")
		return "QuestDB";
	if(t == "BOXREWARDS" || t == "DROPLOGALERT" || t == "NOTICESYSTEM" ||
	   t == "PREMIUMDATA" || t == "PVP" || t == "VIP")
		return "ServerDB";

	// Unknown bare tables in the SQL singleton are unsafe: unlike SQLConnection,
	// the singleton carries no EDatabaseID. Never guess a physical namespace.
	return std::string();
}

std::string PhysicalTable(const std::string &database, const std::string &table)
{
	if(database.empty())
		return table;
	return database + "__" + table;
}

bool RewriteQualifiedNames(std::string &query, std::string &error)
{
	std::string output;
	output.reserve(query.size() + 64);

	for(std::size_t i = 0; i < query.size();)
	{
		// Bracketed SQL Server three-part identifier: [DB].[dbo].[Table]
		if(query[i] == '[')
		{
			std::size_t p = i;
			std::string db, schema, table;
			if(ReadIdentifier(query, p, db) && p < query.size() && query[p] == '.')
			{
				++p;
				if(ReadIdentifier(query, p, schema) && p < query.size() && query[p] == '.')
				{
					++p;
					if(ReadIdentifier(query, p, table) && EqualsNoCase(schema, "dbo"))
					{
						output += '[';
						output += PhysicalTable(db, table);
						output += ']';
						i = p;
						continue;
					}
				}
			}
		}

		// Unbracketed DB..Table / DB.dbo.Table.
		if(IsIdentChar(query[i]) && (i == 0 || !IsIdentChar(query[i - 1])))
		{
			std::size_t p = i;
			std::string db;
			if(ReadIdentifier(query, p, db) && p < query.size() && query[p] == '.')
			{
				const std::size_t afterFirstDot = p + 1;
				if(afterFirstDot < query.size() && query[afterFirstDot] == '.')
				{
					p = afterFirstDot + 1;
					std::string table;
					if(ReadIdentifier(query, p, table))
					{
						output += '[' + PhysicalTable(db, table) + ']';
						i = p;
						continue;
					}
				}
				else
				{
					p = afterFirstDot;
					std::string schema;
					if(ReadIdentifier(query, p, schema) && EqualsNoCase(schema, "dbo") &&
					   p < query.size() && query[p] == '.')
					{
						++p;
						std::string table;
						if(ReadIdentifier(query, p, table))
						{
							output += '[' + PhysicalTable(db, table) + ']';
							i = p;
							continue;
						}
					}
				}
			}
		}

		output.push_back(query[i]);
		++i;
	}

	query.swap(output);
	return true;
}

bool IsKeywordAt(const std::string &query, std::size_t position, const char *keyword)
{
	const std::size_t n = std::strlen(keyword);
	if(position + n > query.size())
		return false;
	if(position > 0 && IsIdentChar(query[position - 1]))
		return false;
	if(position + n < query.size() && IsIdentChar(query[position + n]))
		return false;
	for(std::size_t i = 0; i < n; ++i)
	{
		if(std::toupper(static_cast<unsigned char>(query[position + i])) !=
		   std::toupper(static_cast<unsigned char>(keyword[i])))
			return false;
	}
	return true;
}

bool RewriteBareTables(std::string &query, EDatabaseID currentDatabase, std::string &error)
{
	const char *keywords[] = { "FROM", "JOIN", "INTO", "UPDATE" };
	std::string output;
	output.reserve(query.size() + 64);

	for(std::size_t i = 0; i < query.size();)
	{
		const char *matched = nullptr;
		for(std::size_t k = 0; k < sizeof(keywords) / sizeof(keywords[0]); ++k)
		{
			if(IsKeywordAt(query, i, keywords[k]))
			{
				matched = keywords[k];
				break;
			}
		}

		if(!matched)
		{
			output.push_back(query[i++]);
			continue;
		}

		const std::size_t keywordLength = std::strlen(matched);
		output.append(query, i, keywordLength);
		i += keywordLength;

		const std::size_t whitespaceStart = i;
		while(i < query.size() && std::isspace(static_cast<unsigned char>(query[i])))
			++i;
		output.append(query, whitespaceStart, i - whitespaceStart);

		if(i >= query.size() || query[i] == '(')
			continue;

		std::size_t p = i;
		std::string table;
		if(!ReadIdentifier(query, p, table))
			continue;

		// Already rewritten qualified name.
		if(table.find("__") != std::string::npos)
		{
			output.append(query, i, p - i);
			i = p;
			continue;
		}

		std::string database;
		if(currentDatabase != DATABASEID_NULL)
		{
			const char *name = summit::db::LogicalDatabaseName(currentDatabase);
			if(name)
				database = name;
		}
		if(database.empty())
			database = InferLegacyDatabase(table);

		if(database.empty())
		{
			error = "SQLite: tabela bare sem mapeamento de database no singleton: ";
			error += table;
			return false;
		}

		output += '[' + PhysicalTable(database, table) + ']';
		i = p;
	}

	query.swap(output);
	return true;
}

bool NormalizeTopClause(std::string &query, std::string &error)
{
	std::size_t select = FindNoCase(query, "SELECT TOP ");
	if(select == std::string::npos)
		return true;

	// The legacy code only uses TOP on the outer SELECT. Reject ambiguous forms.
	if(select != 0)
	{
		error = "SQLite: SELECT TOP interno nao suportado automaticamente.";
		return false;
	}

	std::size_t position = std::strlen("SELECT TOP ");
	const std::size_t numberStart = position;
	while(position < query.size() && std::isdigit(static_cast<unsigned char>(query[position])))
		++position;
	if(position == numberStart)
	{
		error = "SQLite: SELECT TOP sem limite inteiro.";
		return false;
	}
	const std::string limit = query.substr(numberStart, position - numberStart);
	while(position < query.size() && std::isspace(static_cast<unsigned char>(query[position])))
		++position;
	query = "SELECT " + query.substr(position);

	while(!query.empty() && std::isspace(static_cast<unsigned char>(query.back())))
		query.pop_back();
	bool semicolon = false;
	if(!query.empty() && query.back() == ';')
	{
		semicolon = true;
		query.pop_back();
	}
	query += " LIMIT " + limit;
	if(semicolon)
		query.push_back(';');
	return true;
}

void NormalizeCollations(std::string &query)
{
	const char *known[] = {
		"COLLATE SQL_Latin1_General_CP1_CI_AS",
		"COLLATE Latin1_General_CI_AI"
	};
	for(std::size_t i = 0; i < sizeof(known) / sizeof(known[0]); ++i)
		ReplaceAllNoCase(query, known[i], "COLLATE NOCASE");
}

void NormalizeDateParts(std::string &query)
{
	ReplaceAllNoCase(query, "DAY(GETDATE())", "CAST(strftime('%d','now','localtime') AS INTEGER)");
	ReplaceAllNoCase(query, "MONTH(GETDATE())", "CAST(strftime('%m','now','localtime') AS INTEGER)");

	// DAY(column) / MONTH(column) are only used by the legacy quest queries.
	const char *functions[] = { "DAY(", "MONTH(" };
	const char *formats[] = { "%d", "%m" };
	for(int f = 0; f < 2; ++f)
	{
		std::size_t pos = 0;
		while((pos = FindNoCase(query, functions[f], pos)) != std::string::npos)
		{
			const std::size_t argStart = pos + std::strlen(functions[f]);
			const std::size_t close = query.find(')', argStart);
			if(close == std::string::npos)
				break;
			const std::string arg = query.substr(argStart, close - argStart);
			if(FindNoCase(arg, "GETDATE") != std::string::npos)
			{
				pos = close + 1;
				continue;
			}
			const std::string repl = "CAST(strftime('" + std::string(formats[f]) + "'," + arg + ") AS INTEGER)";
			query.replace(pos, close - pos + 1, repl);
			pos += repl.size();
		}
	}
}

bool NormalizeDateAdd(std::string &query, std::string &error)
{
	std::size_t pos = 0;
	while((pos = FindNoCase(query, "DATEADD(DAY,", pos)) != std::string::npos)
	{
		std::size_t p = pos + std::strlen("DATEADD(DAY,");
		while(p < query.size() && std::isspace(static_cast<unsigned char>(query[p])))
			++p;
		const std::size_t numberStart = p;
		if(p < query.size() && (query[p] == '+' || query[p] == '-'))
			++p;
		while(p < query.size() && std::isdigit(static_cast<unsigned char>(query[p])))
			++p;
		if(p == numberStart)
		{
			error = "SQLite: DATEADD(DAY,...) requer numero literal nesta migracao.";
			return false;
		}
		const std::string number = query.substr(numberStart, p - numberStart);
		while(p < query.size() && std::isspace(static_cast<unsigned char>(query[p])))
			++p;
		if(p >= query.size() || query[p] != ',')
		{
			error = "SQLite: DATEADD malformado.";
			return false;
		}
		++p;
		while(p < query.size() && std::isspace(static_cast<unsigned char>(query[p])))
			++p;
		const std::string tail = "GETDATE())";
		if(p + tail.size() > query.size() || FindNoCase(query.substr(p, tail.size()), tail) != 0)
		{
			error = "SQLite: DATEADD suportado apenas sobre GETDATE().";
			return false;
		}
		p += tail.size();
		std::string modifier = number;
		if(!modifier.empty() && modifier[0] != '+' && modifier[0] != '-')
			modifier = "+" + modifier;
		const std::string repl = "datetime('now','localtime','" + modifier + " day')";
		query.replace(pos, p - pos, repl);
		pos += repl.size();
	}
	return true;
}

} // namespace

namespace summit
{
namespace db
{

const char *LogicalDatabaseName(EDatabaseID databaseId)
{
	switch(databaseId)
	{
	case DATABASEID_AccountDB: return "AccountDB";
	case DATABASEID_BillingDB: return "BillingDB";
	case DATABASEID_BillingLogDB: return "BillingLogDB";
	case DATABASEID_BattlePass: return "BattlePass";
	case DATABASEID_ClanDB: return "ClanDB";
	case DATABASEID_GameLogDB: return "GameLogDB";
	case DATABASEID_ItemLogDB: return "ItemLogDB";
	case DATABASEID_LogDB: return "LogDB";
	case DATABASEID_PCRoom: return "PCRoom";
	case DATABASEID_PCRoomLog: return "PCRoomLog";
	case DATABASEID_rPTDB: return "rPTDB";
	case DATABASEID_SoD2DB: return "SoD2DB";
	case DATABASEID_SodDB: return "SodDB";
	case DATABASEID_SecurityDB: return "SecurityDB";
	case DATABASEID_ServerDB: return "ServerDB";
	case DATABASEID_CoinShopDB: return "CoinShopDB";
	case DATABASEID_QuestDB: return "QuestDB";
	case DATABASEID_StartUserDB: return "StartUserDB";
	default: return nullptr;
	}
}

bool RewriteLegacyQuery(
	const char *input,
	EDatabaseID currentDatabase,
	std::string &output,
	std::string &error)
{
	if(!input || !input[0])
	{
		error = "SQLite: query vazia.";
		return false;
	}

	output = input;

	// SQL Server date/time functions used by the current source.
	// DATEADD must be converted before the generic GETDATE replacement or
	// its pattern would disappear.
	NormalizeDateParts(output);
	if(!NormalizeDateAdd(output, error))
		return false;
	ReplaceAllNoCase(output, "GETDATE()", "datetime('now','localtime')");

	NormalizeCollations(output);

	if(!RewriteQualifiedNames(output, error))
		return false;
	if(!RewriteBareTables(output, currentDatabase, error))
		return false;

	if(!NormalizeTopClause(output, error))
		return false;

	// Control-flow statements are migrated explicitly at their call sites so
	// parameter numbering and row-count semantics remain obvious.
	if(FindNoCase(output, "IF NOT EXISTS(") != std::string::npos ||
	   FindNoCase(output, "IF EXISTS(") != std::string::npos ||
	   FindNoCase(output, " BEGIN ") != std::string::npos ||
	   FindNoCase(output, " ELSE ") != std::string::npos)
	{
		error = "SQLite: query T-SQL com IF/BEGIN/ELSE ainda precisa migracao explicita.";
		return false;
	}

	return true;
}

} // namespace db
} // namespace summit
