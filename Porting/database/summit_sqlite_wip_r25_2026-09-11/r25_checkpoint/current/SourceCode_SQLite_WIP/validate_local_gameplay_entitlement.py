#!/usr/bin/env python3
"""R21 guard for the server-local gameplay entitlement source."""
from __future__ import annotations

import sqlite3
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent
MIGRATIONS = ROOT / "Database" / "migrations"
RUNNER = ROOT / "SrcServer/src/systems/persistence/database/sqlite/SQLiteConnection.cpp"
HEADER = ROOT / "SrcServer/src/core/server/legacy_runtime/onserver.h"
DLL_H = ROOT / "SrcServer/src/core/server/legacy_runtime/DllServer.h"
DLL_CPP = ROOT / "SrcServer/src/core/server/legacy_runtime/DllServer.cpp"
SERVER = ROOT / "Shared/server/legacy_runtime/OnSever_legacy_server.inl"


def require(text: str, needle: str, label: str) -> None:
    if needle not in text:
        raise AssertionError(f"missing {label}: {needle}")


def active(conn: sqlite3.Connection, user: str) -> bool:
    row = conn.execute(
        "SELECT Enabled FROM LocalAuth__GameplayEntitlement "
        "WHERE UserID=? AND Entitlement='pcbang_gameplay' AND Enabled=1 "
        "AND (ExpiresAt IS NULL OR ExpiresAt='' OR datetime(ExpiresAt) > datetime('now','localtime')) "
        "LIMIT 1",
        (user,),
    ).fetchone()
    return bool(row and row[0] == 1)


def main() -> int:
    runner = RUNNER.read_text(errors="ignore")
    header = HEADER.read_text(errors="ignore")
    dll_h = DLL_H.read_text(errors="ignore")
    dll = DLL_CPP.read_text(errors="ignore")
    server = SERVER.read_text(errors="ignore")

    require(runner, '{ 30, "030_local_gameplay_entitlement.sql" }', "migration 030 runner entry")
    require(dll_h, 'bSql_GetLocalPCBangGameplayEntitlement(const char *userID)', "entitlement DB API")
    require(dll, 'FROM LocalAuth__GameplayEntitlement', "local entitlement SQLite lookup")
    require(dll, "Entitlement='pcbang_gameplay' AND Enabled=1", "entitlement key/enabled filter")
    require(dll, "datetime(ExpiresAt) > datetime('now','localtime')", "entitlement expiry filter")
    require(header, 'lpPlayInfo->Bl_RNo > 0 || rsHasLocalPCBangGameplayEntitlement(lpPlayInfo)', "legacy OR local facade")
    require(server, 'static volatile LONG g_LocalPCBangGameplayEntitlement[CONNECTMAX]', "session-local cache")
    require(server, 'InterlockedCompareExchange(', "atomic entitlement cache read")
    require(server, 'InterlockedExchange(&g_LocalPCBangGameplayEntitlement[index], enabled ? 1 : 0);', "atomic entitlement cache write")
    require(server, 'rsSetLocalPCBangGameplayEntitlement(&rsPlayInfo[cnt], false);', "slot allocation reset")
    require(server, 'bSql_GetLocalPCBangGameplayEntitlement(LogAccount->szID) == TRUE', "login entitlement load")
    if server.count('rsSetLocalPCBangGameplayEntitlement(tpInfo, false);') < 2:
        raise AssertionError("both disconnect paths must clear local entitlement cache")

    with tempfile.TemporaryDirectory(prefix="summit_r21_entitlement_") as td:
        conn = sqlite3.connect(Path(td) / "summit.sqlite3")
        for path in sorted(MIGRATIONS.glob("[0-9][0-9][0-9]_*.sql")):
            conn.executescript(path.read_text(encoding="utf-8"))

        cols = [r[1] for r in conn.execute('PRAGMA table_info("LocalAuth__GameplayEntitlement")')]
        assert cols == ["UserID", "Entitlement", "Enabled", "ExpiresAt", "UpdatedAt"], cols

        assert not active(conn, "LocalPerk")
        conn.execute(
            "INSERT INTO LocalAuth__GameplayEntitlement(UserID,Entitlement,Enabled,ExpiresAt) "
            "VALUES ('LocalPerk','pcbang_gameplay',1,NULL)"
        )
        assert active(conn, "localperk")  # NOCASE ownership

        conn.execute(
            "UPDATE LocalAuth__GameplayEntitlement SET Enabled=0 WHERE UserID='LocalPerk'"
        )
        assert not active(conn, "LocalPerk")

        conn.execute(
            "UPDATE LocalAuth__GameplayEntitlement "
            "SET Enabled=1,ExpiresAt=datetime('now','localtime','-1 minute') "
            "WHERE UserID='LocalPerk'"
        )
        assert not active(conn, "LocalPerk")

        conn.execute(
            "UPDATE LocalAuth__GameplayEntitlement "
            "SET ExpiresAt=datetime('now','localtime','+1 day') WHERE UserID='LocalPerk'"
        )
        assert active(conn, "LocalPerk")

        try:
            conn.execute(
                "INSERT INTO LocalAuth__GameplayEntitlement(UserID,Entitlement,Enabled) "
                "VALUES ('LOCALPERK','PCBANG_GAMEPLAY',1)"
            )
            raise AssertionError("case-insensitive entitlement PK not enforced")
        except sqlite3.IntegrityError:
            pass

        try:
            conn.execute(
                "INSERT INTO LocalAuth__GameplayEntitlement(UserID,Entitlement,Enabled) "
                "VALUES ('BadFlag','pcbang_gameplay',2)"
            )
            raise AssertionError("Enabled CHECK constraint not enforced")
        except sqlite3.IntegrityError:
            pass
        conn.close()

    # Upgrade proof: a v29 database with existing account data gains only the
    # R21 table and keeps the existing row untouched.
    with tempfile.TemporaryDirectory(prefix="summit_r21_upgrade_") as td:
        conn = sqlite3.connect(Path(td) / "summit.sqlite3")
        paths = sorted(MIGRATIONS.glob("[0-9][0-9][0-9]_*.sql"))
        for path in paths:
            if int(path.name[:3]) > 29:
                break
            conn.executescript(path.read_text(encoding="utf-8"))
        conn.execute("PRAGMA user_version=29")
        conn.execute(
            "INSERT INTO AccountDB__AGameUser("
            "userid,Passwd,inuse,Grade,EventChk,SelectChk,BlockChk,SpecialChk,Credit,DelChk"
            ") VALUES ('UpgradeUser','pw','0','0','0','0','0','0','0','0')"
        )
        before = conn.execute(
            "SELECT userid,Passwd,inuse FROM AccountDB__AGameUser WHERE userid='UpgradeUser'"
        ).fetchone()
        migration30 = MIGRATIONS / "030_local_gameplay_entitlement.sql"
        conn.executescript(migration30.read_text(encoding="utf-8"))
        conn.execute("PRAGMA user_version=30")
        after = conn.execute(
            "SELECT userid,Passwd,inuse FROM AccountDB__AGameUser WHERE userid='UpgradeUser'"
        ).fetchone()
        assert before == after == ("UpgradeUser", "pw", "0")
        assert conn.execute("PRAGMA user_version").fetchone()[0] == 30
        assert conn.execute(
            "SELECT COUNT(*) FROM sqlite_master WHERE type='table' "
            "AND name='LocalAuth__GameplayEntitlement'"
        ).fetchone()[0] == 1
        conn.close()

    print(
        "Local gameplay entitlement PASS: migration v30; independent SQLite grant; "
        "NOCASE identity; enabled/expiry semantics; v29->v30 data-preserving upgrade; "
        "session cache load/reset; legacy Bl_RNo OR preserved"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
