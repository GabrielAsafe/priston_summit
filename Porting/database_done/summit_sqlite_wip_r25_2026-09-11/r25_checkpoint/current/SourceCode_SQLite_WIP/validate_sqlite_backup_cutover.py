#!/usr/bin/env python3
"""Validate R25 consistent backup and guarded local-only preparation workflow."""
from pathlib import Path
import sqlite3
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parent
MIG = ROOT / "Database" / "migrations"
TOOL = ROOT / "tools" / "sqlite_local_admin.py"


def apply(db: sqlite3.Connection) -> None:
    for p in sorted(MIG.glob("[0-9][0-9][0-9]_*.sql")):
        v = int(p.name[:3])
        db.executescript(p.read_text(encoding="utf-8"))
        db.execute(f"PRAGMA user_version={v}")


def defaults(db: sqlite3.Connection, table: str) -> dict[str, object]:
    out: dict[str, object] = {}
    for _, name, coltype, notnull, default, pk in db.execute(f'PRAGMA table_info("{table}")'):
        if notnull and default is None and not pk:
            out[name] = 0 if "INT" in (coltype or "").upper() else "0"
    return out


def add_account(db: sqlite3.Connection, user: str, *, mode: str = "local", inuse: str = "0") -> None:
    table = f"AccountDB__{user[0].upper()}GameUser"
    values = defaults(db, table)
    values.update(
        userid=user, Passwd="pw", GPCode="", UsePeriod=0, inuse=inuse,
        BlockChk="0", DelChk="0", RNo=0, BNum=0, Channel="", ServerName="", SNo=None,
    )
    cols = list(values)
    db.execute(
        f'INSERT INTO "{table}" ({",".join("[" + c + "]" for c in cols)}) '
        f'VALUES ({",".join("?" for _ in cols)})',
        [values[c] for c in cols],
    )
    db.execute(
        "INSERT INTO LocalAuth__AccountPolicy(UserID,AuthMode,Enabled) VALUES (?,?,1)",
        (user, mode),
    )


def run(path: Path, *args: str) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        [sys.executable, str(TOOL), "--db", str(path), *args],
        text=True, capture_output=True,
    )


def main() -> int:
    with tempfile.TemporaryDirectory(prefix="r25_backup_") as td:
        td = Path(td)
        source = td / "summit.sqlite3"
        backup = td / "rollback.sqlite3"
        prepared = td / "prepared.sqlite3"
        blocked = td / "blocked.sqlite3"
        db = sqlite3.connect(source)
        db.row_factory = sqlite3.Row
        apply(db)
        add_account(db, "LocalUser")
        db.execute(
            "INSERT INTO LocalAuth__GameplayEntitlement(UserID,Entitlement,Enabled) "
            "VALUES ('LocalUser','pcbang_gameplay',1)"
        )
        db.commit()
        db.execute("PRAGMA journal_mode=WAL")
        db.execute("UPDATE AccountDB__LGameUser SET Credit='777' WHERE userid='LocalUser'")
        db.commit()
        db.close()

        p = run(source, "backup", str(backup))
        assert p.returncode == 0, (p.stdout, p.stderr)
        assert "backup_verified=yes" in p.stdout and backup.is_file(), p.stdout
        b = sqlite3.connect(backup)
        assert b.execute("PRAGMA quick_check").fetchone()[0] == "ok"
        assert not list(b.execute("PRAGMA foreign_key_check"))
        assert b.execute("PRAGMA user_version").fetchone()[0] == 32
        assert b.execute("SELECT Credit FROM AccountDB__LGameUser WHERE userid='LocalUser'").fetchone()[0] == "777"
        assert b.execute(
            "SELECT Enabled FROM LocalAuth__GameplayEntitlement WHERE UserID='LocalUser' AND Entitlement='pcbang_gameplay'"
        ).fetchone()[0] == 1
        b.close()

        p = run(source, "backup", str(backup))
        assert p.returncode == 2 and "backup already exists" in p.stderr, (p.stdout, p.stderr)
        p = run(source, "backup", str(backup), "--overwrite")
        assert p.returncode == 0, (p.stdout, p.stderr)

        p = run(source, "prepare-local-only", str(prepared))
        assert p.returncode == 0, (p.stdout, p.stderr)
        assert "prepare_local_only_ready=yes" in p.stdout and prepared.is_file(), p.stdout

        db = sqlite3.connect(source)
        db.execute("UPDATE AccountDB__LGameUser SET inuse='1' WHERE userid='LocalUser'")
        db.commit(); db.close()
        p = run(source, "prepare-local-only", str(blocked))
        assert p.returncode == 3, (p.stdout, p.stderr)
        assert "reason=active_sessions" in p.stdout and not blocked.exists(), p.stdout

        db = sqlite3.connect(source)
        db.execute("UPDATE AccountDB__LGameUser SET inuse='0' WHERE userid='LocalUser'")
        db.execute("UPDATE LocalAuth__AccountPolicy SET AuthMode='legacy' WHERE UserID='LocalUser'")
        db.commit(); db.close()
        p = run(source, "prepare-local-only", str(blocked))
        assert p.returncode == 3, (p.stdout, p.stderr)
        assert "reason=catalog_blockers" in p.stdout and not blocked.exists(), p.stdout

        p = run(source, "backup", str(source))
        assert p.returncode == 2 and "must differ" in p.stderr, (p.stdout, p.stderr)

    src = TOOL.read_text(encoding="utf-8")
    for token in ["db.backup(out)", "backup_verified=yes", "prepare_local_only_ready=", "count_active_sessions"]:
        assert token in src, token
    print("SQLite backup/cutover safety PASS: consistent backup, verification, overwrite guard, quiescent/local-only preflight and rollback snapshot verified")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
