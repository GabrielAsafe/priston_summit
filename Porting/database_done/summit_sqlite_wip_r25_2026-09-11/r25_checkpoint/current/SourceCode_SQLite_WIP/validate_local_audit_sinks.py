#!/usr/bin/env python3
from __future__ import annotations

import sqlite3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
MIGRATIONS = ROOT / "Database/migrations"
DLLSERVER = ROOT / "SrcServer/src/core/server/legacy_runtime/DllServer.cpp"
RUNNER = ROOT / "SrcServer/src/systems/persistence/database/sqlite/SQLiteConnection.cpp"


def execute_script(db: sqlite3.Connection, sql: str) -> None:
    db.executescript(sql)


def apply_through(db: sqlite3.Connection, last: int) -> None:
    for path in sorted(MIGRATIONS.glob("[0-9][0-9][0-9]_*.sql")):
        version = int(path.name[:3])
        if version > last:
            break
        execute_script(db, path.read_text(encoding="utf-8"))
        db.execute(f"PRAGMA user_version={version}")


def columns(db: sqlite3.Connection, table: str) -> list[str]:
    return [row[1] for row in db.execute(f'PRAGMA table_info("{table}")')]


def assert_source_contract() -> None:
    text = DLLSERVER.read_text(encoding="latin1")
    runner = RUNNER.read_text(encoding="latin1")
    assert '{ 31, "031_local_audit_events.sql" }' in runner

    char_start = text.index("static BOOL TrySQLiteLocalCharacterAudit(")
    char_end = text.index("static BOOL TrySQLiteLocalItemAudit(", char_start)
    char_helper = text[char_start:char_end]
    assert "DATABASEID_GameLogDB" in char_helper
    assert "INSERT INTO LocalCharacterEvent" in char_helper
    assert "PARAMTYPE_Int64" in char_helper

    for label, event_type in (("LOG_SQL_NEWCHAR", 1), ("LOG_SQL_DELCHAR", 2), ("LOG_SQL_LOGOUT", 3)):
        start = text.index(f"case {label}:")
        end = text.index("break;", start)
        case = text[start:end]
        local = case.index(f"TrySQLiteLocalCharacterAudit(logSqlData, {event_type})")
        dll_positions = [pos for token in ("BillSql.CharRecordLog", "BillSql.CharLog") if (pos := case.find(token)) >= 0]
        assert dll_positions and local < min(dll_positions), f"{label} is not local-first"

    item_start = text.index("static BOOL TrySQLiteLocalItemAudit(")
    item_end = text.index("static BOOL IsSQLiteLocalZeroValue", item_start)
    item_helper = text[item_start:item_end]
    for fragment in (
        "DATABASEID_ItemLogDB",
        "BeginTransaction()",
        "INSERT INTO LocalItemEvent",
        "SELECT last_insert_rowid()",
        "INSERT INTO LocalItemEventEntry",
        "CommitTransaction()",
    ):
        assert fragment in item_helper, fragment

    queue_start = text.index("int\tb_PushLogItemQue(int type")
    queue_end = text.index("int b_PopLogItemQue", queue_start)
    queue = text[queue_start:queue_end]
    assert "!BillSql.LogITEMRecord" not in queue
    assert "!BillSql.LogITEMTradeRecord" not in queue

    worker_start = text.index("DWORD WINAPI LogItemDataThreadProc")
    worker_end = text.index("int\tb_PushLogItemQue", worker_start)
    worker = text[worker_start:worker_end]
    assert worker.index("TrySQLiteLocalItemAudit(") < worker.index("BillSql.LogITEMRecord")
    assert worker.index("TrySQLiteLocalItemAudit(", worker.index("case 2:")) < worker.index("BillSql.LogITEMTradeRecord")
    assert worker.index("TrySQLiteLocalItemAudit(", worker.index("case 3:")) < worker.index("BillSql.LogITEMPostRecord")


def assert_schema_and_payloads() -> None:
    db = sqlite3.connect(":memory:")
    db.execute("PRAGMA foreign_keys=ON")
    apply_through(db, 31)
    assert db.execute("PRAGMA user_version").fetchone() == (31,)
    assert columns(db, "GameLogDB__LocalCharacterEvent") == [
        "ID", "EventType", "UserID", "CharName", "CharType", "GPCode", "IP",
        "ServerName", "GameLevel", "Exp", "GameMoney", "CreatedAt",
    ]
    assert columns(db, "ItemLogDB__LocalItemEvent") == [
        "ID", "EventType", "UserID", "CharName", "IP", "Flag", "TargetUserID",
        "TargetCharName", "TargetIP", "Money", "GiftNo", "Telephone", "ItemCount",
        "CreatedAt",
    ]
    assert columns(db, "ItemLogDB__LocalItemEventEntry") == [
        "EventID", "ItemIndex", "ItemCode", "ItemINo", "ItemINo1",
    ]

    huge_exp = 9_223_372_036_000_000_000
    db.execute(
        '''INSERT INTO GameLogDB__LocalCharacterEvent
        (EventType,UserID,CharName,CharType,GPCode,IP,ServerName,GameLevel,Exp,GameMoney,CreatedAt)
        VALUES (?,?,?,?,?,?,?,?,?,?,datetime('now','localtime'))''',
        (3, "audit-user", "AuditChar", 8, "", "127.0.0.1", "TestServer", 120, huge_exp, 777),
    )
    assert db.execute(
        "SELECT EventType,UserID,CharName,Exp,GameMoney FROM GameLogDB__LocalCharacterEvent"
    ).fetchone() == (3, "audit-user", "AuditChar", huge_exp, 777)

    cur = db.execute(
        '''INSERT INTO ItemLogDB__LocalItemEvent
        (EventType,UserID,CharName,IP,Flag,TargetUserID,TargetCharName,TargetIP,Money,GiftNo,Telephone,ItemCount,CreatedAt)
        VALUES (?,?,?,?,?,?,?,?,?,?,?,?,datetime('now','localtime'))''',
        (2, "from", "FromChar", 123, 9, "to", "ToChar", 456, 5000, 0, "", 2),
    )
    event_id = cur.lastrowid
    db.executemany(
        "INSERT INTO ItemLogDB__LocalItemEventEntry(EventID,ItemIndex,ItemCode,ItemINo,ItemINo1) VALUES (?,?,?,?,?)",
        [(event_id, 0, 0x01020304, 11, 12), (event_id, 1, 0xF1020304, 21, 22)],
    )
    assert db.execute(
        "SELECT EventType,TargetUserID,Money,ItemCount FROM ItemLogDB__LocalItemEvent WHERE ID=?",
        (event_id,),
    ).fetchone() == (2, "to", 5000, 2)
    assert db.execute(
        "SELECT ItemIndex,ItemCode,ItemINo,ItemINo1 FROM ItemLogDB__LocalItemEventEntry WHERE EventID=? ORDER BY ItemIndex",
        (event_id,),
    ).fetchall() == [(0, 0x01020304, 11, 12), (1, 0xF1020304, 21, 22)]

    try:
        db.execute(
            "INSERT INTO ItemLogDB__LocalItemEventEntry(EventID,ItemIndex,ItemCode,ItemINo,ItemINo1) VALUES (999999,0,1,2,3)"
        )
        raise AssertionError("item audit FK not enforced")
    except sqlite3.IntegrityError:
        pass


def assert_v30_to_v31_upgrade() -> None:
    db = sqlite3.connect(":memory:")
    apply_through(db, 30)
    db.execute(
        "INSERT INTO LocalAuth__GameplayEntitlement(UserID,Entitlement,Enabled) VALUES ('KeepMe','pcbang_gameplay',1)"
    )
    db.execute("INSERT INTO CoinShopDB__UserInfo(UserID,UserCoin,LastChange) VALUES ('KeepCoin',321,'now')")
    migration31 = MIGRATIONS / "031_local_audit_events.sql"
    db.executescript(migration31.read_text(encoding="utf-8"))
    db.execute("PRAGMA user_version=31")
    assert db.execute(
        "SELECT Enabled FROM LocalAuth__GameplayEntitlement WHERE UserID='KeepMe'"
    ).fetchone() == (1,)
    assert db.execute(
        "SELECT UserCoin FROM CoinShopDB__UserInfo WHERE UserID='KeepCoin'"
    ).fetchone() == (321,)
    assert columns(db, "GameLogDB__LocalCharacterEvent")
    assert columns(db, "ItemLogDB__LocalItemEventEntry")


def main() -> int:
    assert_source_contract()
    assert_schema_and_payloads()
    assert_v30_to_v31_upgrade()
    print("Local audit sinks PASS: v31; char/item/trade/gift payloads local-first; v30->v31 preservation PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
