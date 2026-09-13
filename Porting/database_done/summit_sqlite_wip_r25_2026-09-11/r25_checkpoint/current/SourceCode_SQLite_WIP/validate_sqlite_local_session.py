#!/usr/bin/env python3
"""Validate the implicit R19 SQLite local/free session lifecycle.

R23 adds an explicit local/legacy ownership layer, validated separately. This
validator intentionally keeps the original zero-billing-signature contract as
a regression oracle for accounts with no explicit policy.
"""

from __future__ import annotations

import sqlite3
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent
MIGRATIONS = ROOT / "Database" / "migrations"
DLLSERVER = ROOT / "SrcServer" / "src" / "core" / "server" / "legacy_runtime" / "DllServer.cpp"


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
        ServerName=None,
        SNo=None,
    )
    values.update(overrides)
    cols = list(values)
    conn.execute(
        f'INSERT INTO "{table}" ({",".join(f"[{c}]" for c in cols)}) '
        f'VALUES ({",".join("?" for _ in cols)})',
        [values[c] for c in cols],
    )


def claim(conn: sqlite3.Connection, userid: str, guid: str, server: str) -> bool:
    table = f"AccountDB__{userid[0].upper()}GameUser"
    conn.execute("BEGIN IMMEDIATE")
    row = conn.execute(
        f'SELECT GPCode,UsePeriod,inuse,BlockChk,DelChk,RNo,BNum,Channel '
        f'FROM "{table}" WHERE userid=? LIMIT 1',
        (userid,),
    ).fetchone()
    if row is None:
        conn.rollback()
        return False
    gp, period, inuse, block, delete, rno, bnum, channel = row
    zero = lambda value: value is None or str(value) in {"", "0"}
    if not (
        zero(gp)
        and int(period or 0) == 0
        and zero(inuse)
        and zero(block)
        and zero(delete)
        and int(rno or 0) == 0
        and int(bnum or 0) == 0
        and zero(channel)
    ):
        conn.rollback()
        return False
    conn.execute(
        f'UPDATE "{table}" SET StartDay=datetime("now","localtime"), '
        f'EditDay=datetime("now","localtime"), RNo=NULL, ServerName=?, inuse="1", SNo=? '
        f'WHERE userid=?',
        (server, guid, userid),
    )
    conn.commit()
    return True


def logout(conn: sqlite3.Connection, userid: str, guid: str, server: str) -> bool:
    table = f"AccountDB__{userid[0].upper()}GameUser"
    conn.execute("BEGIN IMMEDIATE")
    row = conn.execute(
        f'SELECT GPCode,UsePeriod,inuse,RNo,BNum,Channel,SNo,ServerName '
        f'FROM "{table}" WHERE userid=? LIMIT 1',
        (userid,),
    ).fetchone()
    if row is None:
        conn.rollback()
        return False
    gp, period, inuse, rno, bnum, channel, stored_guid, stored_server = row
    zero = lambda value: value is None or str(value) in {"", "0"}
    if not (
        zero(gp)
        and int(period or 0) == 0
        and str(inuse) == "1"
        and int(rno or 0) == 0
        and int(bnum or 0) == 0
        and zero(channel)
        and stored_guid == guid
        and stored_server == server
    ):
        conn.rollback()
        return False
    conn.execute(
        f'UPDATE "{table}" SET LastDay=datetime("now","localtime"), '
        f'inuse="0", RNo=NULL, SNo=NULL WHERE userid=? AND ServerName=?',
        (userid, server),
    )
    conn.commit()
    return True


def reset_server(conn: sqlite3.Connection, server: str) -> None:
    conn.execute("BEGIN IMMEDIATE")
    for initial in "ABCDEFGHIJKLMNOPQRSTUVWXYZ":
        conn.execute(
            f'UPDATE "AccountDB__{initial}GameUser" '
            f'SET LastDay=datetime("now","localtime"), inuse="0", RNo=NULL, SNo=NULL '
            f'WHERE ServerName=? AND inuse="1" AND SNo IS NOT NULL AND SNo<>"" '
            f'AND (GPCode IS NULL OR GPCode="" OR GPCode="0") '
            f'AND COALESCE(UsePeriod,0)=0 AND COALESCE(BNum,0)=0 '
            f'AND (Channel IS NULL OR Channel="" OR Channel="0") '
            f'AND (RNo IS NULL OR RNo=0)',
            (server,),
        )
    conn.commit()


def row(conn: sqlite3.Connection, userid: str):
    table = f"AccountDB__{userid[0].upper()}GameUser"
    return conn.execute(
        f'SELECT inuse,ServerName,SNo,RNo,StartDay,EditDay,LastDay,GPCode,UsePeriod,BNum,Channel '
        f'FROM "{table}" WHERE userid=? LIMIT 1',
        (userid,),
    ).fetchone()


def main() -> int:
    with tempfile.TemporaryDirectory() as tmp:
        conn = sqlite3.connect(Path(tmp) / "summit.sqlite3", isolation_level=None)
        migrate(conn)

        insert_account(conn, "SimpleUser", "Secret")
        insert_account(conn, "PaidUser", "Secret", GPCode="PAID", inuse="1", ServerName="GameServer", SNo="paid-guid")
        insert_account(conn, "OtherServer", "Secret", inuse="1", ServerName="Other", SNo="other-guid", RNo=None)
        conn.commit()

        assert claim(conn, "SimpleUser", "local-guid-1", "GameServer")
        claimed = row(conn, "SimpleUser")
        assert claimed[0] == "1" and claimed[1] == "GameServer" and claimed[2] == "local-guid-1"
        assert claimed[3] is None and claimed[4] and claimed[5]
        assert not claim(conn, "SimpleUser", "local-guid-2", "GameServer"), "duplicate local claim must be rejected"
        assert not logout(conn, "SimpleUser", "wrong-guid", "GameServer"), "wrong session GUID must not clear local session"
        assert logout(conn, "SimpleUser", "local-guid-1", "GameServer")
        released = row(conn, "SimpleUser")
        assert released[0] == "0" and released[2] is None and released[3] is None and released[6]

        assert claim(conn, "SimpleUser", "local-guid-3", "GameServer")
        reset_server(conn, "GameServer")
        reset_local = row(conn, "SimpleUser")
        paid = row(conn, "PaidUser")
        other = row(conn, "OtherServer")
        assert reset_local[0] == "0" and reset_local[2] is None, "stale local/free session must be reset"
        assert paid[0] == "1" and paid[2] == "paid-guid", "paid/billing row must not be reset"
        assert other[0] == "1" and other[2] == "other-guid", "other server session must not be reset"
        conn.close()

    source = DLLSERVER.read_text(encoding="latin1")
    required = (
        "ClaimSQLiteLocalSession",
        "TrySQLiteLocalLogout",
        "ResetSQLiteLocalSessionsForServer",
        "BeginTransaction()",
        "StartDay=datetime('now','localtime')",
        "inuse='1', SNo=?",
        "LastDay=datetime('now','localtime'), inuse='0', RNo=NULL, SNo=NULL",
        "strcmp(storedGuid, sessionGuid) == 0",
        "if(!TrySQLiteLocalLogout(logSqlData.szUserID, logSqlData.szGuid) && BillSql.SQLLogout)",
    )
    missing = [token for token in required if token not in source]
    if missing:
        raise AssertionError(f"missing R19 local-session source tokens: {missing}")

    print(
        "SQLite local session PASS: implicit R19 atomic free-session claim/release, duplicate claim rejection, "
        "GUID/server ownership check, stale local reset, paid/other-server rows preserved"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
