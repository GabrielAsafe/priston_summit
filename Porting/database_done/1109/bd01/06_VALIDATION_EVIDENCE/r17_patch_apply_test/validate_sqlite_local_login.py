#!/usr/bin/env python3
"""Validate the conservative SQLite local/free login contract added in R12.

This does not pretend to emulate the historical Database.dll billing engine.
It proves only the subset intentionally handled locally:
- account rows live in <initial>GameUser;
- exact password match;
- no GPCode/period/inuse/block/delete/PC-room/channel billing state;
- DisuseDay may be empty, an epoch, or a SQL datetime;
- everything ambiguous remains eligible for Database.dll fallback.
"""

from __future__ import annotations

import sqlite3
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent
MIGRATIONS = ROOT / "Database" / "migrations"
DLLSERVER = ROOT / "SrcServer" / "src" / "core" / "server" / "legacy_runtime" / "DllServer.cpp"
ONSERVER = ROOT / "Shared" / "server" / "legacy_runtime" / "OnSever_legacy_server.inl"


def migrate(conn: sqlite3.Connection) -> None:
    for path in sorted(MIGRATIONS.glob("[0-9][0-9][0-9]_*.sql")):
        conn.executescript(path.read_text(encoding="utf-8"))


def required_defaults(conn: sqlite3.Connection, table: str) -> dict[str, object]:
    values: dict[str, object] = {}
    for _, name, coltype, notnull, default, pk in conn.execute(f'PRAGMA table_info("{table}")'):
        if notnull and default is None and not pk:
            values[name] = 0 if "INT" in (coltype or "").upper() else "0"
    return values


def insert_account(conn: sqlite3.Connection, userid: str, password: str, **overrides: object) -> None:
    table = f"AccountDB__{userid[0].upper()}GameUser"
    values = required_defaults(conn, table)
    values.update(
        userid=userid,
        Passwd=password,
        GPCode="",
        DisuseDay="2030-01-02 03:04:05",
        UsePeriod=0,
        inuse="0",
        BlockChk="0",
        DelChk="0",
        RNo=0,
        BNum=0,
        Channel="",
    )
    values.update(overrides)
    cols = list(values)
    sql = f'INSERT INTO "{table}" ({",".join(f"[{c}]" for c in cols)}) VALUES ({",".join("?" for _ in cols)})'
    conn.execute(sql, [values[c] for c in cols])


def fetch_login_row(conn: sqlite3.Connection, userid: str):
    table = f"AccountDB__{userid[0].upper()}GameUser"
    return conn.execute(
        f'SELECT Passwd,GPCode,DisuseDay,UsePeriod,inuse,BlockChk,DelChk,RNo,BNum,Channel '
        f'FROM "{table}" WHERE userid=? LIMIT 1',
        (userid,),
    ).fetchone()


def zero_value(value: object) -> bool:
    return value is None or str(value) in {"", "0"}


def eligible(row, supplied_password: str) -> bool:
    if row is None:
        return False
    passwd, gp, _day, period, inuse, block, delete, rno, bnum, channel = row
    return (
        passwd == supplied_password
        and zero_value(gp)
        and int(period or 0) == 0
        and zero_value(inuse)
        and zero_value(block)
        and zero_value(delete)
        and int(rno or 0) == 0
        and int(bnum or 0) == 0
        and zero_value(channel)
    )


def main() -> int:
    with tempfile.TemporaryDirectory() as tmp:
        dbpath = Path(tmp) / "summit.sqlite3"
        conn = sqlite3.connect(dbpath)
        migrate(conn)

        tables = {row[0] for row in conn.execute("SELECT name FROM sqlite_master WHERE type='table'")}
        for initial in "ABCDEFGHIJKLMNOPQRSTUVWXYZ":
            table = f"AccountDB__{initial}GameUser"
            assert table in tables, f"missing per-letter account table: {table}"
            cols = {row[1] for row in conn.execute(f'PRAGMA table_info("{table}")')}
            required_cols = {"userid", "Passwd", "GPCode", "DisuseDay", "UsePeriod", "inuse", "BlockChk", "DelChk", "RNo", "BNum", "Channel"}
            assert required_cols <= cols, f"{table} missing local-login columns: {sorted(required_cols - cols)}"

        insert_account(conn, "SimpleUser", "Secret")
        insert_account(conn, "PaidUser", "Secret", GPCode="PTP-RUD001")
        insert_account(conn, "PeriodUser", "Secret", UsePeriod=3600)
        insert_account(conn, "BlockedUser", "Secret", BlockChk="1")
        insert_account(conn, "PcRoomUser", "Secret", RNo=7)
        insert_account(conn, "ExpiredUser", "Secret", DisuseDay="2020-01-01 00:00:00")
        conn.commit()

        assert eligible(fetch_login_row(conn, "SimpleUser"), "Secret")
        assert not eligible(fetch_login_row(conn, "SimpleUser"), "secret"), "password mismatch must fall back"
        assert not eligible(fetch_login_row(conn, "MissingUser"), "Secret")
        assert not eligible(fetch_login_row(conn, "PaidUser"), "Secret")
        assert not eligible(fetch_login_row(conn, "PeriodUser"), "Secret")
        assert not eligible(fetch_login_row(conn, "BlockedUser"), "Secret")
        assert not eligible(fetch_login_row(conn, "PcRoomUser"), "Secret")
        # The C++ path additionally compares parsed DisuseDay with time(NULL).
        assert fetch_login_row(conn, "ExpiredUser")[2] == "2020-01-01 00:00:00"
        conn.close()

    source = DLLSERVER.read_text(encoding="latin1")
    required = (
        "TrySQLiteLocalLogOn",
        "FROM %cGameUser WHERE userid=? LIMIT 1",
        "if(BillSql.SQLLogon)",
        "UseDLL_Code = LOAD_DLL_SQL;",
        "if(IsSQLiteLocalZeroValue(GPCode))",
        "CoCreateGuid",
        "disuseDay < time(NULL)",
    )
    missing = [token for token in required if token not in source]
    if missing:
        raise AssertionError(f"missing source contract tokens: {missing}")

    onserver = ONSERVER.read_text(encoding="latin1")
    assert "case LOAD_DLL_SQL:" in onserver
    assert "val = bSql_LogOn(" in onserver

    print(
        "SQLite local login PASS: simple/free account handled locally; "
        "password mismatch, missing account and billing/blocked/PC-room/expired rows remain fallback-only"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
