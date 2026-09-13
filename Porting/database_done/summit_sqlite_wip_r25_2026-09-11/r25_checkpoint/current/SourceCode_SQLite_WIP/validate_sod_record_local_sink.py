#!/usr/bin/env python3
from __future__ import annotations

import sqlite3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SOURCE = ROOT / "SrcServer/src/core/server/legacy_runtime/DllServer.cpp"


def apply_migrations(db: sqlite3.Connection) -> None:
    for path in sorted((ROOT / "Database/migrations").glob("[0-9][0-9][0-9]_*.sql")):
        db.executescript(path.read_text(encoding="utf-8"))


def assert_source_contract() -> None:
    text = SOURCE.read_text(encoding="latin1")
    helper_start = text.index("static BOOL TrySQLiteLocalSODRecord(")
    helper_end = text.index("\nstatic BOOL IsSQLiteLocalZeroValue", helper_start)
    helper = text[helper_start:helper_end]
    required = [
        "SQLConnection::GetConnection(DATABASEID_SoD2DB)",
        "INSERT INTO SOD2RecBySandurr",
        "BusinessName,BAddress1,PMNo,Contribute,RegistDay,ServerTime,Flag,ClanIDX,ClanName",
        "datetime('now','localtime')",
        "db->Execute(FALSE)",
    ]
    missing = [fragment for fragment in required if fragment not in helper]
    assert not missing, f"missing local SoD sink fragments: {missing}"

    case_start = text.index("case GAMELOG_SQL_SOD:")
    case_end = text.index("case GAMELOG_SQL_STARPOINT:", case_start)
    case = text[case_start:case_end]
    local_pos = case.index("TrySQLiteLocalSODRecord(")
    dll_pos = case.index("BillSql.WithSODRecordRanks")
    assert local_pos < dll_pos, "Database.dll is attempted before the SQLite SoD sink"
    assert "&&\n\t\t\t\t\tBillSql.WithSODRecordRanks" in case, "DLL fallback is not guarded by local failure"


def assert_sqlite_contract() -> None:
    db = sqlite3.connect(":memory:")
    apply_migrations(db)

    db.execute(
        '''INSERT INTO "SoD2DB__SOD2RecBySandurr"
        (UserID,CharName,CharType,Point,KillCount,GLevel,TotalPoint,TotalUser,SuccessUser,
         ServerName,PCRNo,GPCode,BusinessName,BAddress1,PMNo,Contribute,RegistDay,ServerTime,Flag,ClanIDX,ClanName)
        VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?, ?,datetime('now','localtime'),?,?,?,?)''',
        (
            "account", "character", 2, 12345, 77, 110, 99999, 8, 3,
            "TestServer", 0, "", "", "127.0.0.1", 0, 0,
            7, 0, 0, "",
        ),
    )
    row = db.execute(
        '''SELECT UserID,CharName,CharType,Point,KillCount,GLevel,TotalPoint,TotalUser,
                  SuccessUser,ServerName,PCRNo,GPCode,BusinessName,BAddress1,PMNo,
                  Contribute,ServerTime,Flag,ClanIDX,ClanName
           FROM "SoD2DB__SOD2RecBySandurr" WHERE UserID='account' '''
    ).fetchone()
    assert row == (
        "account", "character", 2, 12345, 77, 110, 99999, 8, 3,
        "TestServer", 0, "", "", "127.0.0.1", 0, 0, 7, 0, 0, "",
    ), row
    assert db.execute(
        'SELECT length(RegistDay) > 0 FROM "SoD2DB__SOD2RecBySandurr" WHERE UserID=?',
        ("account",),
    ).fetchone() == (1,)


def main() -> int:
    assert_source_contract()
    assert_sqlite_contract()
    print("SoD record local sink PASS: async SQLite-first insert + Database.dll fallback preserved")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
