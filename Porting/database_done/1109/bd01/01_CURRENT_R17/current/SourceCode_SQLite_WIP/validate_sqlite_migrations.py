#!/usr/bin/env python3
"""Validate the Summit SQLite migration set without touching runtime data."""

from __future__ import annotations

import sqlite3
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent
MIGRATION_DIR = ROOT / "Database" / "migrations"
EXPECTED_LATEST = 29
EXPECTED_USER_TABLES = 332


def execute_script_in_current_transaction(db: sqlite3.Connection, sql: str) -> None:
    buffer = ""
    for line in sql.splitlines(True):
        buffer += line
        if sqlite3.complete_statement(buffer):
            statement = buffer.strip()
            buffer = ""
            if statement:
                db.execute(statement)
    if buffer.strip():
        db.execute(buffer)


def migration_files() -> list[tuple[int, Path]]:
    result: list[tuple[int, Path]] = []
    for path in sorted(MIGRATION_DIR.glob("[0-9][0-9][0-9]_*.sql")):
        result.append((int(path.name[:3]), path))
    versions = [version for version, _ in result]
    expected = list(range(1, EXPECTED_LATEST + 1))
    if versions != expected:
        raise AssertionError(f"migration versions {versions!r} != {expected!r}")
    return result


def apply_pending(db: sqlite3.Connection, migrations: list[tuple[int, Path]]) -> None:
    current = db.execute("PRAGMA user_version").fetchone()[0]
    if current > EXPECTED_LATEST:
        raise AssertionError(
            f"database version {current} is newer than supported {EXPECTED_LATEST}"
        )

    for version, path in migrations:
        if version <= current:
            continue
        db.execute("BEGIN IMMEDIATE")
        try:
            execute_script_in_current_transaction(db, path.read_text(encoding="utf-8"))
            db.execute(f"PRAGMA user_version={version}")
            db.execute("COMMIT")
        except Exception:
            db.execute("ROLLBACK")
            raise
        current = version


def columns(db: sqlite3.Connection, table: str) -> list[str]:
    return [row[1] for row in db.execute(f'PRAGMA table_info("{table}")')]


def validate_billingdb_catalog(db: sqlite3.Connection) -> None:
    channel_tables = ["0CHANNEL", "9CHANNEL"] + [f"{ch}CHANNEL" for ch in "ABCDEFGHIJKLMNOPQRSTUVWXYZ"]
    assert len(channel_tables) == 28
    expected_channel = ["PRISTONTALE_ID", "TABLE_NAME", "INSERT_DATE"]
    for name in channel_tables:
        assert columns(db, f"BillingDB__{name}") == expected_channel

    base_billing = [
        "NUM", "EXTERNAL_ID", "PRISTONTALE_ID", "GOODS_TYPE", "CHARGE_TYPE",
        "CHARGE_FORM", "AMOUNT", "START_DAY", "END_DAY", "LEFT_TIME",
        "CHARGE_DAY", "UPDATE_CHECK", "NOTE",
    ]
    history_billing = ["NUM", "IDX"] + base_billing[1:]
    assert columns(db, "BillingDB__BillingPointCheck20060329") == ["IDX", "UserID", "EndDay"]
    assert columns(db, "BillingDB__Excite") == base_billing
    assert columns(db, "BillingDB__NetMarble") == base_billing
    assert columns(db, "BillingDB__NTTBilling") == base_billing
    for provider in ("Excite", "NetMarble"):
        for suffix in ("Amount", "Bak", "Error", "Free", "NoUse"):
            assert columns(db, f"BillingDB__{provider}_{suffix}") == history_billing
    assert columns(db, "BillingDB__Table1") == ["day", "amount", "count"]

    billing_tables = db.execute(
        "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name LIKE 'BillingDB__%'"
    ).fetchone()[0]
    assert billing_tables == 43, billing_tables


def validate_contracts(db: sqlite3.Connection) -> None:
    validate_billingdb_catalog(db)
    assert columns(db, "QuestDB__Progress") == [
        "QuestID", "QuestKind", "QuestType", "PlayerName", "StartDate"
    ]
    assert columns(db, "QuestDB__Rewards") == [
        "QuestID", "QuestKind", "Gold", "Exp", "QuestType", "Coin", "TCoin"
    ]
    assert columns(db, "QuestDB__ItemRewards") == [
        "QuestID", "QuestKind", "ItemCode", "ItemCount", "QuestType"
    ]
    assert columns(db, "ServerDB__NoticeSystem") == [
        "ID", "Message", "Type", "Activated"
    ]
    assert columns(db, "SecurityDB__CheatLog") == [
        "ID", "IP", "AccountName", "Action", "LogID", "Description", "Date"
    ]
    assert columns(db, "SecurityDB__FunctionChecksumList") == [
        "ID", "Size", "Address", "Checksum"
    ]
    assert columns(db, "SecurityDB__WindowCheatList") == ["ID", "WindowName"]
    assert columns(db, "StartUserDB__Level") == ["Level"]
    assert columns(db, "StartUserDB__Items") == [
        "Classe", "Weapon", "Aging1", "Mix1", "Armor", "Aging2", "Mix2",
        "Boots", "Aging3", "Mix3", "Gloves", "Aging4", "Mix4",
        "Bracelet", "Aging5", "Mix5"
    ]
    assert columns(db, "StartUserDB__ExtraItems") == ["ItemCode"]
    assert columns(db, "SecurityDB__BannedIP") == [
        "ID", "IP", "AccountID", "Date"
    ]
    assert columns(db, "SecurityDB__BannedMac") == ["ID", "MacAddress"]
    assert columns(db, "SecurityDB__AgingRecord") == [
        "ID", "ItemName", "ItemAging", "Head", "ChkSum"
    ]
    assert columns(db, "ClanDB__UL") == [
        "IDX", "MIDX", "userid", "ChName", "ChType", "ChLv", "ClanName",
        "Permi", "JoinDate", "DelActive", "PFlag", "KFlag", "MIconCnt"
    ]
    assert columns(db, "ClanDB__CL") == [
        "IDX", "ClanName", "Note", "NoteCnt", "UserID", "ClanZang", "Flag",
        "MemCnt", "MIconCnt", "RegiDate", "LimitDate", "DelActive", "PFlag",
        "KFlag", "Cpoint", "CWin", "CFail", "ClanMoney", "CNFlag",
        "SiegeMoney", "LIDER_BC"
    ]
    assert columns(db, "ClanDB__ClanMainDeleted") == [
        "IDX", "MIDX", "ClanName", "Userid", "ClanZang", "Flag", "MemCnt",
        "MIConCnt", "RegiDate", "LimitDate", "DelDate", "DelActive", "PFlag",
        "KFlag", "DelState", "DelCase", "Server"
    ]
    assert columns(db, "ClanDB__ClanSubDeleted") == [
        "IDX", "MIDX", "userid", "ChName", "ChType", "ChLv", "ClanName",
        "Permi", "JoinDate", "DelActive", "PFlag", "KFlag", "DelDate",
        "DelState", "DelCase", "Server"
    ]
    assert columns(db, "JOBS__Jobs_Info") == [
        "ID", "NICK", "JOB", "Job_Level", "Job_xp", "M_A", "M_F", "M_L", "M_M", "M_R"
    ]
    assert columns(db, "JOBS__Jobs_shop") == [
        "ID", "NICK", "MINE_TYPE", "MINE_QTD", "MINE_PRICE", "CHECKSUM"
    ]
    assert len(columns(db, "BattlePass__BP_Info")) == 44
    assert columns(db, "PristonDB__Coroa") == ["ID"]
    assert columns(db, "PristonDB__Coroas") == ["SubTag", "Name", "CodeIT", "ID"]
    assert columns(db, "PristonDB__Tags") == ["SubTag", "Clan", "CodeIT", "Slot"]
    assert len(columns(db, "SoD2DB__SOD2RecBySandurr")) == 22
    assert columns(db, "LogadoPremiado__ItemList") == ["ItemCode"]
    assert columns(db, "ServerDB__PremiumData") == [
        "CharacterName", "TimeLeft", "ItemType"
    ]
    assert columns(db, "ServerDB__BoxRewards") == [
        "Kind", "ItemCode", "Count", "Class"
    ]
    assert columns(db, "SecurityDB__AgingFailed") == [
        "PlayerID", "PlayerName", "ItemName", "ItemAging", "Head", "ChkSum", "Date"
    ]

    # CoinShop rows are a single logical balance per account. This uniqueness
    # is shared by CoinShop and the migrated JOBS market balance path.
    db.execute(
        "INSERT INTO CoinShopDB__UserInfo(UserID,UserCoin,LastChange) "
        "VALUES ('CoinValidator',100,datetime('now','localtime'))"
    )
    try:
        db.execute(
            "INSERT INTO CoinShopDB__UserInfo(UserID,UserCoin,LastChange) "
            "VALUES ('CoinValidator',200,datetime('now','localtime'))"
        )
        raise AssertionError("CoinShopDB__UserInfo UserID uniqueness not enforced")
    except sqlite3.IntegrityError:
        pass
    assert db.execute(
        "SELECT UserCoin FROM CoinShopDB__UserInfo WHERE UserID='CoinValidator'"
    ).fetchone() == (100,)

    # JOBS market now shares the proven CoinShopDB__UserInfo balance table
    # instead of the missing CoinShopDB/PAINEL_NET Usuario tables.
    db.execute(
        "INSERT INTO CoinShopDB__UserInfo(UserID,UserCoin,LastChange) "
        "VALUES ('JobBuyer',100,datetime('now','localtime'))"
    )
    db.execute(
        "INSERT INTO CoinShopDB__UserInfo(UserID,UserCoin,LastChange) "
        "VALUES ('JobSeller',20,datetime('now','localtime'))"
    )
    db.execute(
        "INSERT INTO JOBS__Jobs_Info(ID,NICK,JOB,Job_Level,Job_xp,M_A,M_F,M_L,M_M,M_R) "
        "VALUES ('JobBuyer','Buyer',1,1,0,10,0,0,0,0)"
    )
    db.execute(
        "INSERT INTO JOBS__Jobs_shop(ID,NICK,MINE_TYPE,MINE_QTD,MINE_PRICE,CHECKSUM) "
        "VALUES ('JobSeller','Seller',1,5,30,12345)"
    )
    buyer = db.execute(
        "SELECT UserCoin FROM CoinShopDB__UserInfo WHERE UserID='JobBuyer'"
    ).fetchone()[0]
    seller = db.execute(
        "SELECT UserCoin FROM CoinShopDB__UserInfo WHERE UserID='JobSeller'"
    ).fetchone()[0]
    price, qty = db.execute(
        "SELECT MINE_PRICE,MINE_QTD FROM JOBS__Jobs_shop WHERE CHECKSUM=12345"
    ).fetchone()
    assert buyer >= price
    db.execute(
        "UPDATE CoinShopDB__UserInfo SET UserCoin=?,LastChange=datetime('now','localtime') "
        "WHERE UserID='JobBuyer'", (buyer - price,)
    )
    db.execute(
        "UPDATE CoinShopDB__UserInfo SET UserCoin=?,LastChange=datetime('now','localtime') "
        "WHERE UserID='JobSeller'", (seller + price,)
    )
    db.execute("DELETE FROM JOBS__Jobs_shop WHERE CHECKSUM=12345")
    db.execute("UPDATE JOBS__Jobs_Info SET M_A=M_A+? WHERE ID='JobBuyer'", (qty,))
    assert db.execute(
        "SELECT UserCoin FROM CoinShopDB__UserInfo WHERE UserID='JobBuyer'"
    ).fetchone() == (70,)
    assert db.execute(
        "SELECT UserCoin FROM CoinShopDB__UserInfo WHERE UserID='JobSeller'"
    ).fetchone() == (50,)
    assert db.execute(
        "SELECT M_A FROM JOBS__Jobs_Info WHERE ID='JobBuyer'"
    ).fetchone() == (15,)
    assert db.execute(
        "SELECT COUNT(*) FROM JOBS__Jobs_shop WHERE CHECKSUM=12345"
    ).fetchone() == (0,)

    db.execute(
        "INSERT INTO StartUserDB__ExtraItems(ItemCode) VALUES ('BI999')"
    )
    assert db.execute(
        "SELECT ItemCode FROM StartUserDB__ExtraItems"
    ).fetchone() == ('BI999',)

    db.execute(
        "INSERT INTO SecurityDB__BannedIP(IP,AccountID,Date) "
        "VALUES ('127.0.0.2','Validator',datetime('now','localtime'))"
    )
    assert db.execute(
        "SELECT 1 FROM SecurityDB__BannedIP WHERE IP='127.0.0.2'"
    ).fetchone() == (1,)
    db.execute(
        "INSERT INTO SecurityDB__BannedMac(MacAddress) VALUES ('00-00-00-00-00-01')"
    )
    assert db.execute(
        "SELECT 1 FROM SecurityDB__BannedMac WHERE MacAddress='00-00-00-00-00-01'"
    ).fetchone() == (1,)
    db.execute(
        "INSERT INTO SecurityDB__AgingRecord(ItemName,ItemAging,Head,ChkSum) "
        "VALUES ('Sword',22,123,456)"
    )
    assert db.execute(
        "SELECT ItemName,ItemAging,Head,ChkSum FROM SecurityDB__AgingRecord"
    ).fetchone() == ('Sword',22,123,456)

    db.execute(
        "INSERT OR REPLACE INTO QuestDB__Complete "
        "(QuestID,QuestKind,QuestType,PlayerName,EndDate) VALUES (1,7,1,'Validator','a')"
    )
    db.execute(
        "INSERT OR REPLACE INTO QuestDB__Complete "
        "(QuestID,QuestKind,QuestType,PlayerName,EndDate) VALUES (1,7,1,'Validator','b')"
    )
    assert db.execute(
        "SELECT COUNT(*), MAX(EndDate) FROM QuestDB__Complete WHERE PlayerName='Validator'"
    ).fetchone() == (1, "b")

    db.execute(
        "INSERT OR IGNORE INTO ServerDB__PVP(ID,Name,Class,Lvl,Kills,Deads) "
        "VALUES ('a','Validator',1,1,0,0)"
    )
    db.execute(
        "INSERT OR IGNORE INTO ServerDB__PVP(ID,Name,Class,Lvl,Kills,Deads) "
        "VALUES ('b','Validator',2,2,0,0)"
    )
    assert db.execute(
        "SELECT COUNT(*) FROM ServerDB__PVP WHERE Name='Validator'"
    ).fetchone()[0] == 1

    db.execute(
        "INSERT INTO ServerDB__BoxRewards(Kind,ItemCode,Count,Class) "
        "VALUES (2,'BI123',4,0)"
    )
    assert db.execute(
        "SELECT ItemCode, Count, Class FROM ServerDB__BoxRewards WHERE Kind=2"
    ).fetchone() == ("BI123", 4, 0)

    db.execute(
        "INSERT INTO ServerDB__PremiumData(CharacterName,TimeLeft,ItemType) "
        "VALUES ('Validator',3600,14)"
    )
    assert db.execute(
        "SELECT TimeLeft, ItemType FROM ServerDB__PremiumData WHERE CharacterName='Validator'"
    ).fetchone() == (3600, 14)

    db.execute(
        "INSERT INTO SecurityDB__AgingFailed"
        "(PlayerID,PlayerName,ItemName,ItemAging,Head,ChkSum,Date) "
        "VALUES ('acc','Validator','Sword',10,11,22,datetime('now','localtime'))"
    )
    assert db.execute(
        "SELECT Head,ChkSum FROM SecurityDB__AgingFailed "
        "WHERE PlayerID='acc' AND PlayerName='Validator' ORDER BY Date DESC"
    ).fetchone() == (11, 22)

    db.execute(
        "INSERT INTO QuestDB__Rewards(QuestID,QuestKind,Gold,Exp,QuestType,Coin,TCoin) "
        "VALUES (99,9,100,200,99,3,4)"
    )
    assert db.execute(
        "SELECT Gold,Exp,Coin,TCoin FROM QuestDB__Rewards "
        "WHERE QuestID=99 AND QuestKind=9 AND QuestType=99"
    ).fetchone() == (100, 200, 3, 4)

    db.execute(
        "INSERT INTO QuestDB__ItemRewards(QuestID,QuestKind,ItemCode,ItemCount,QuestType) "
        "VALUES (99,9,'BI001',2,99)"
    )
    assert db.execute(
        "SELECT ItemCode,ItemCount FROM QuestDB__ItemRewards "
        "WHERE QuestID=99 AND QuestKind=9 AND QuestType=99"
    ).fetchone() == ('BI001', 2)

    db.execute(
        "INSERT INTO SecurityDB__CheatLog(IP,AccountName,Description,Date) "
        "VALUES ('127.0.0.1','Validator','test',datetime('now','localtime'))"
    )
    assert db.execute(
        "SELECT IP,AccountName,Description FROM SecurityDB__CheatLog WHERE AccountName='Validator'"
    ).fetchone() == ('127.0.0.1', 'Validator', 'test')

    db.execute(
        "INSERT INTO StartUserDB__Items(Weapon,Aging1,Mix1,Armor,Aging2,Mix2,Boots,Aging3,Mix3,"
        "Gloves,Aging4,Mix4,Bracelet,Aging5,Mix5) "
        "VALUES ('W',1,2,'A',3,4,'B',5,6,'G',7,8,'R',9,10)"
    )
    assert db.execute(
        "SELECT Classe,Weapon,Aging1,Mix1,Armor,Aging2,Mix2,Boots,Aging3,Mix3,"
        "Gloves,Aging4,Mix4,Bracelet,Aging5,Mix5,NULL,0,0 FROM StartUserDB__Items"
    ).fetchone()[16:] == (None, 0, 0)


def main() -> int:
    migrations = migration_files()
    with tempfile.TemporaryDirectory(prefix="summit_sqlite_validate_") as tmp:
        db_path = Path(tmp) / "summit.sqlite3"
        db = sqlite3.connect(db_path, isolation_level=None)
        try:
            apply_pending(db, migrations)
            version = db.execute("PRAGMA user_version").fetchone()[0]
            assert version == EXPECTED_LATEST
            user_tables = db.execute(
                "SELECT COUNT(*) FROM sqlite_master "
                "WHERE type='table' AND name NOT LIKE 'sqlite_%'"
            ).fetchone()[0]
            assert user_tables == EXPECTED_USER_TABLES, user_tables

            # Applying the same idempotent DDL/seed again must not duplicate Hopi.
            for _, path in migrations:
                db.execute("BEGIN IMMEDIATE")
                try:
                    execute_script_in_current_transaction(db, path.read_text(encoding="utf-8"))
                    db.execute("COMMIT")
                except Exception:
                    db.execute("ROLLBACK")
                    raise
            assert db.execute(
                "SELECT COUNT(*) FROM QuestDB__QuestMonsters WHERE MonsterName='Hopi'"
            ).fetchone()[0] == 1

            validate_contracts(db)
        finally:
            db.close()

    print(
        f"SQLite migrations PASS: version={EXPECTED_LATEST}, "
        f"user_tables={EXPECTED_USER_TABLES}, contracts/idempotence=PASS"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
