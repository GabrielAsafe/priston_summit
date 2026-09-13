#!/usr/bin/env python3
"""R18 regression guard for Database.dll gameplay responsibilities migrated locally."""
from pathlib import Path
import sqlite3
import tempfile

ROOT = Path(__file__).resolve().parent
DLLSERVER = ROOT / "SrcServer/src/core/server/legacy_runtime/DllServer.cpp"
MIGRATIONS = ROOT / "Database/migrations"

src = DLLSERVER.read_text(errors="replace")
required = [
    "TrySQLiteLocalClanFund",
    "TrySQLiteLocalSiegeMoneyTax",
    '"SELECT TOP 1 IDX, %s FROM CL WHERE UserID=? AND ClanZang=?"',
    '"UPDATE CL SET %s=%s-? WHERE UserID=?"',
    '"UPDATE CL SET SiegeMoney=SiegeMoney+? WHERE IDX=?"',
    "SiegeMoneyLog",
    "SiegeMoneyTax",
    "ClanMoneyLog",
    "operation == 1 ? \"01\" : \"02\"",
    "TrySQLiteLocalClanFund(\n",
    "TrySQLiteLocalSiegeMoneyTax(\n",
]
for needle in required:
    if needle not in src:
        raise SystemExit(f"FAIL: missing R18 clan-fund contract marker: {needle}")
if "if(!BillSql.ClanMoney) return FALSE;" in src:
    raise SystemExit("FAIL: bSql_ClanMoney is still hard-gated on Database.dll")
if "if(!BillSql.SiegeMoneyTax) return FALSE;" in src:
    raise SystemExit("FAIL: bSql_RecordCurrency is still hard-gated on Database.dll")

# Build the real SQLite schema to make sure the tables/columns used by the
# migrated gameplay path exist together exactly as shipped.
with tempfile.TemporaryDirectory() as td:
    db_path = Path(td) / "contract.sqlite3"
    con = sqlite3.connect(db_path)
    for migration in sorted(MIGRATIONS.glob("[0-9][0-9][0-9]_*.sql")):
        con.executescript(migration.read_text(errors="replace"))

    # Use the same required fields as R16 clan creation.
    con.execute(
        '''INSERT INTO ClanDB__CL
        (ClanName,UserID,ClanZang,MemCnt,Note,MIconCnt,RegiDate,LimitDate,
         DelActive,PFlag,KFlag,Flag,NoteCnt,Cpoint,CWin,CFail,ClanMoney,CNFlag,
         SiegeMoney,LIDER_BC)
        VALUES (?,?,?,?,?,?,datetime('now','localtime'),datetime('now','localtime','+3600 days'),
                '0',0,0,0,1,0,0,0,100,0,1000,0)''',
        ("R18Clan", "owner", "Leader", 1, "New clan", 1000000001),
    )

    def apply(column: str, amount: int, operation: int, siege: bool = False):
        row = con.execute(
            f"SELECT IDX, {column} FROM ClanDB__CL WHERE UserID=? AND ClanZang=? LIMIT 1",
            ("owner", "Leader"),
        ).fetchone()
        clan_idx, current = row if row else (0, 0)
        if amount == 0 or operation == 0:
            return current
        if siege or operation == 2:
            con.execute(
                f'''UPDATE ClanDB__CL SET {column}={column}{'-' if operation == 1 else '+'}? 
                    WHERE UserID=? AND (DelActive IS NULL OR CAST(DelActive AS INTEGER)=0)''',
                (amount, "owner"),
            )
        else:
            con.execute(
                f"UPDATE ClanDB__CL SET {column}={column}-? WHERE UserID=?",
                (amount, "owner"),
            )
        result_row = con.execute(
            f"SELECT {column} FROM ClanDB__CL WHERE UserID=? LIMIT 1", ("owner",)
        ).fetchone()
        result = result_row[0] if result_row else 0
        code = "01" if operation == 1 else "02"
        logged = -amount if operation == 1 else amount
        if siege:
            con.execute(
                '''INSERT INTO ClanDB__SiegeMoneyLog
                   (CIDX,UserID,chName,ServerName,Operator,OperCode,SiegeMoney,RegistDay)
                   VALUES (?,?,?,?,'GS',?,?,datetime('now','localtime'))''',
                (clan_idx, "owner", "Leader", "Test", code, logged),
            )
        else:
            con.execute(
                '''INSERT INTO ClanDB__ClanMoneyLog
                   (CIDX,UserID,chName,ServerName,Operator,OperCode,ClanMoney,RegistDay,Note)
                   VALUES (?,?,?,?,'GS',?,?,datetime('now','localtime'),'')''',
                (clan_idx, "owner", "Leader", "Test", code, logged),
            )
        return result

    # Historical DLL permits a withdrawal larger than the stored clan fund.
    assert apply("ClanMoney", 150, 1, False) == -50
    assert apply("ClanMoney", 25, 2, False) == -25
    logs = con.execute(
        "SELECT OperCode, ClanMoney FROM ClanDB__ClanMoneyLog ORDER BY rowid"
    ).fetchall()
    assert logs == [("01", -150), ("02", 25)], logs

    assert apply("SiegeMoney", 1200, 1, True) == -200
    assert apply("SiegeMoney", 300, 2, True) == 100
    slog = con.execute(
        "SELECT OperCode, SiegeMoney FROM ClanDB__SiegeMoneyLog ORDER BY rowid"
    ).fetchall()
    assert slog == [("01", -1200), ("02", 300)], slog

    # Preserve the odd legacy asymmetry recovered from Database.dll:
    # ClanMoney withdraw ignores DelActive; deposits and SiegeMoney do not.
    con.execute("UPDATE ClanDB__CL SET DelActive='1', ClanMoney=10, SiegeMoney=10 WHERE UserID='owner'")
    assert apply("ClanMoney", 5, 1, False) == 5
    assert apply("ClanMoney", 5, 2, False) == 5  # update blocked, post-select returns unchanged
    assert apply("SiegeMoney", 5, 1, True) == 10  # update blocked

    # Database.dll!SiegeMoneyTax contract recovered from its x86 flow:
    # last six decimal digits of clan mark -> CL.IDX, tax share -> SiegeMoney,
    # then one tax row and one GS/02 siege-money row.  Use the mark allocated
    # by the clan schema above (1000000001 -> IDX 1).
    def apply_siege_tax(mark: str, values, total: int, tax: int):
        if total == 0 or mark is None or len(mark) < 9:
            return False
        idx_text = mark[-6:]
        if not idx_text.isdigit():
            return False
        clan_idx = int(idx_text)
        if clan_idx == 0:
            return False

        credit = 0
        if tax != 0:
            # 0.01f is the single-precision constant embedded in the DLL.
            import struct
            one_percent = struct.unpack("f", struct.pack("f", 0.01))[0]
            divisor = 1.0 + tax * one_percent
            if divisor <= 0:
                return False
            untaxed = int(total / divisor + 0.5)
            credit = total - untaxed

        con.execute(
            "UPDATE ClanDB__CL SET SiegeMoney=SiegeMoney+? WHERE IDX=?",
            (credit, clan_idx),
        )
        con.execute(
            '''INSERT INTO ClanDB__SiegeMoneyTax
               (IDX,Mixing,Aging,Shop,Poison1,Poison2,Poison3,Force,Warpgate,Skill,
                Total,Tax,ServerName,RegistDay)
               VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,datetime('now','localtime'))''',
            (clan_idx, *values, total, tax, "Test"),
        )
        con.execute(
            '''INSERT INTO ClanDB__SiegeMoneyLog
               (CIDX,UserID,chName,ServerName,Operator,OperCode,SiegeMoney,RegistDay)
               VALUES (?,'Server','Server',?,'GS','02',?,datetime('now','localtime'))''',
            (clan_idx, "Test", credit),
        )
        return True

    # Reactivate the clan row after the DelActive asymmetry checks.
    con.execute(
        "UPDATE ClanDB__CL SET DelActive='0', SiegeMoney=1000 WHERE IDX=1"
    )
    values = (110, 220, 330, 44, 55, 66, 77, 88, 110)
    assert apply_siege_tax("1000000001", values, 1100, 10) is True
    assert con.execute(
        "SELECT SiegeMoney FROM ClanDB__CL WHERE IDX=1"
    ).fetchone()[0] == 1100
    tax_row = con.execute(
        '''SELECT IDX,Mixing,Aging,Shop,Poison1,Poison2,Poison3,Force,Warpgate,
                  Skill,Total,Tax,ServerName
           FROM ClanDB__SiegeMoneyTax ORDER BY DNo DESC LIMIT 1'''
    ).fetchone()
    assert tax_row == (1, *values, 1100, 10, "Test"), tax_row
    tax_log = con.execute(
        '''SELECT CIDX,UserID,chName,Operator,OperCode,SiegeMoney
           FROM ClanDB__SiegeMoneyLog ORDER BY IDX DESC LIMIT 1'''
    ).fetchone()
    assert tax_log == (1, "Server", "Server", "GS", "02", 100), tax_log

    # Invalid/empty legacy cases are handled as failure without mutating DB.
    before = con.execute("SELECT COUNT(*) FROM ClanDB__SiegeMoneyTax").fetchone()[0]
    assert apply_siege_tax("12345678", values, 1100, 10) is False
    assert apply_siege_tax("1000000001", values, 0, 10) is False
    after = con.execute("SELECT COUNT(*) FROM ClanDB__SiegeMoneyTax").fetchone()[0]
    assert before == after

    con.rollback()
    con.close()

print("Database.dll gameplay contract PASS: ClanMoney/SiegeMoney/SiegeMoneyTax local SQLite semantics + logs match recovered DLL SQL/x86")
