#!/usr/bin/env python3
"""Administration helper for Summit's local SQLite auth layer (R23+).

The server remains the migration owner. Run it once (or otherwise apply migrations)
before using this utility. Policy changes are transactional; the legacy-billing
runtime switch is read at server startup and therefore requires a restart.
"""
from __future__ import annotations

import argparse
import getpass
import sqlite3
import sys
from datetime import datetime
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_DB = ROOT / "Database" / "summit.sqlite3"
MIN_VERSION = 32
MAX_ACCOUNT = 31
MAX_PASSWORD = 31
MAX_GM_PASSWORD = 29  # GameMasters.cpp reads into char[30].


class AdminError(RuntimeError):
    pass


def account_initial(user: str) -> str:
    if not user or len(user) > MAX_ACCOUNT:
        raise AdminError(f"account must contain 1..{MAX_ACCOUNT} characters")
    initial = user[0].upper()
    if initial < "A" or initial > "Z":
        raise AdminError("account must start with A-Z")
    return initial


def account_table(user: str) -> str:
    return f"AccountDB__{account_initial(user)}GameUser"


def personal_table(user: str) -> str:
    return f"AccountDB__{account_initial(user)}PersonalMember"


def connect(path: Path) -> sqlite3.Connection:
    if not path.is_file():
        raise AdminError(f"database not found: {path}")
    db = sqlite3.connect(path, timeout=5.0)
    db.row_factory = sqlite3.Row
    db.execute("PRAGMA foreign_keys=ON")
    version = db.execute("PRAGMA user_version").fetchone()[0]
    if version < MIN_VERSION:
        db.close()
        raise AdminError(f"database user_version={version}; R23 requires >= {MIN_VERSION}")
    needed = {"LocalAuth__AccountPolicy", "LocalAuth__RuntimePolicy", "LocalAuth__GameplayEntitlement"}
    tables = {r[0] for r in db.execute("SELECT name FROM sqlite_master WHERE type='table'")}
    missing = sorted(needed - tables)
    if missing:
        db.close()
        raise AdminError(f"R23 local-auth tables missing: {', '.join(missing)}")
    return db


def password_value(value: str | None, *, gm: bool = False) -> str:
    limit = MAX_GM_PASSWORD if gm else MAX_PASSWORD
    if value is None:
        first = getpass.getpass("Password: ")
        second = getpass.getpass("Repeat password: ")
        if first != second:
            raise AdminError("passwords do not match")
        value = first
    if not value or len(value) > limit:
        raise AdminError(f"password must contain 1..{limit} characters")
    return value


def find_accounts(db: sqlite3.Connection, user: str) -> list[sqlite3.Row]:
    table = account_table(user)
    return list(db.execute(f'SELECT rowid,* FROM "{table}" WHERE lower(userid)=lower(?)', (user,)))


def require_one(db: sqlite3.Connection, user: str) -> tuple[str, sqlite3.Row]:
    table = account_table(user)
    rows = find_accounts(db, user)
    if not rows:
        raise AdminError(f"account not found: {user}")
    if len(rows) != 1:
        raise AdminError(f"account is ambiguous ({len(rows)} case-insensitive rows): {user}")
    return table, rows[0]


def required_insert_values(db: sqlite3.Connection, table: str) -> dict[str, object]:
    values: dict[str, object] = {}
    for col in db.execute(f'PRAGMA table_info("{table}")'):
        name, coltype, notnull, default, pk = col[1], col[2] or "", col[3], col[4], col[5]
        if notnull and default is None and not pk:
            values[name] = 0 if "INT" in coltype.upper() else "0"
    return values


def set_auth(db: sqlite3.Connection, user: str, mode: str) -> None:
    require_one(db, user)
    if mode == "implicit":
        db.execute("DELETE FROM LocalAuth__AccountPolicy WHERE UserID=?", (user,))
        return
    if mode not in {"local", "legacy"}:
        raise AdminError("auth mode must be local, legacy or implicit")
    db.execute(
        "INSERT INTO LocalAuth__AccountPolicy(UserID,AuthMode,Enabled,UpdatedAt) "
        "VALUES (?,?,1,datetime('now','localtime')) "
        "ON CONFLICT(UserID) DO UPDATE SET AuthMode=excluded.AuthMode,Enabled=1,UpdatedAt=excluded.UpdatedAt",
        (user, mode),
    )


def insert_row(db: sqlite3.Connection, table: str, values: dict[str, object]) -> None:
    existing_columns = {row[1] for row in db.execute(f'PRAGMA table_info("{table}")')}
    if not existing_columns:
        raise AdminError(f"required table missing: {table}")
    filtered = {key: value for key, value in values.items() if key in existing_columns}
    cols = list(filtered)
    db.execute(
        f'INSERT INTO "{table}" ({",".join(f"[{c}]" for c in cols)}) '
        f'VALUES ({",".join("?" for _ in cols)})',
        [filtered[c] for c in cols],
    )


def create_account(db: sqlite3.Connection, user: str, password: str, mode: str) -> None:
    table = account_table(user)
    all_game = "AccountDB__AllGameUser"
    all_personal = "AccountDB__ALLPersonalMember"
    letter_personal = personal_table(user)

    if find_accounts(db, user):
        raise AdminError(f"account already exists (case-insensitive): {user}")
    if db.execute(
        f'SELECT 1 FROM "{all_game}" WHERE lower(userid)=lower(?) LIMIT 1', (user,)
    ).fetchone():
        raise AdminError(f"account already exists in AllGameUser: {user}")

    now = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    game_values = required_insert_values(db, table)
    game_values.update(
        userid=user,
        Passwd=password,
        GPCode="",
        UsePeriod=0,
        inuse="0",
        # Historical web registration used Grade='U'. Keep that account
        # contract while AuthMode=local remains authoritative for billing.
        Grade="U",
        EventChk="0",
        SelectChk="0",
        BlockChk="0",
        SpecialChk="0",
        Credit="0",
        RNo=0,
        DelChk="0",
        SNo=None,
        Channel="",
        BNum=0,
        RegistDay=now,
        EditDay=now,
        ServerName="",
    )

    all_game_values = required_insert_values(db, all_game)
    all_game_values.update(game_values)
    all_game_values.update(BlockMOTIVO=None, BlockDATA=None)

    personal_values = {
        "PMNo": 0,
        "Userid": user,
        "Passwd": password,
        "RegistDay": now,
    }
    all_personal_values = required_insert_values(db, all_personal)
    all_personal_values.update(personal_values)
    letter_personal_values = required_insert_values(db, letter_personal)
    letter_personal_values.update(personal_values)

    # sqlite3.Connection context in main makes these four legacy mirrors plus
    # LocalAuth__AccountPolicy one atomic registration operation.
    insert_row(db, table, game_values)
    insert_row(db, all_game, all_game_values)
    insert_row(db, all_personal, all_personal_values)
    insert_row(db, letter_personal, letter_personal_values)
    set_auth(db, user, mode)


def set_account_password(db: sqlite3.Connection, user: str, password: str) -> None:
    table, row = require_one(db, user)
    db.execute(f'UPDATE "{table}" SET Passwd=?, EditDay=datetime(\'now\',\'localtime\') WHERE rowid=?', (password, row["rowid"]))
    db.execute(
        'UPDATE AccountDB__AllGameUser SET Passwd=?, EditDay=datetime(\'now\',\'localtime\') '
        'WHERE lower(userid)=lower(?)',
        (password, user),
    )
    db.execute(
        'UPDATE AccountDB__ALLPersonalMember SET Passwd=? WHERE lower(Userid)=lower(?)',
        (password, user),
    )
    db.execute(
        f'UPDATE "{personal_table(user)}" SET Passwd=? WHERE lower(Userid)=lower(?)',
        (password, user),
    )


def set_entitlement(db: sqlite3.Connection, user: str, enabled: bool, expires: str | None = None) -> None:
    require_one(db, user)
    if enabled:
        if expires:
            parsed = db.execute("SELECT datetime(?)", (expires,)).fetchone()[0]
            if parsed is None:
                raise AdminError("--expires must be a SQLite/ISO datetime, e.g. 2026-12-31 23:59:59")
            expires = parsed
        db.execute(
            "INSERT INTO LocalAuth__GameplayEntitlement(UserID,Entitlement,Enabled,ExpiresAt,UpdatedAt) "
            "VALUES (?,'pcbang_gameplay',1,?,datetime('now','localtime')) "
            "ON CONFLICT(UserID,Entitlement) DO UPDATE SET Enabled=1,ExpiresAt=excluded.ExpiresAt,UpdatedAt=excluded.UpdatedAt",
            (user, expires),
        )
    else:
        db.execute(
            "INSERT INTO LocalAuth__GameplayEntitlement(UserID,Entitlement,Enabled,ExpiresAt,UpdatedAt) "
            "VALUES (?,'pcbang_gameplay',0,NULL,datetime('now','localtime')) "
            "ON CONFLICT(UserID,Entitlement) DO UPDATE SET Enabled=0,ExpiresAt=NULL,UpdatedAt=excluded.UpdatedAt",
            (user,),
        )


def set_gm(db: sqlite3.Connection, user: str, password: str, security: int, active: int) -> None:
    require_one(db, user)
    if security < 1 or security > 4:
        raise AdminError("GM security must be 1..4 (VIP, moderator, gamemaster, administrator)")
    db.execute("DELETE FROM AccountDB__GameMasters WHERE lower(accountId)=lower(?)", (user,))
    db.execute(
        "INSERT INTO AccountDB__GameMasters(accountId,password,security,active) VALUES (?,?,?,?)",
        (user, password, security, active),
    )


def show_account(db: sqlite3.Connection, user: str) -> None:
    _, r = require_one(db, user)
    policy = db.execute(
        "SELECT AuthMode,Enabled,UpdatedAt FROM LocalAuth__AccountPolicy WHERE UserID=?", (user,)
    ).fetchone()
    ent = db.execute(
        "SELECT Enabled,ExpiresAt,UpdatedAt FROM LocalAuth__GameplayEntitlement "
        "WHERE UserID=? AND Entitlement='pcbang_gameplay'", (user,)
    ).fetchone()
    gms = list(db.execute(
        "SELECT security,active FROM AccountDB__GameMasters WHERE lower(accountId)=lower(?)", (user,)
    ))
    print(f"account={r['userid']} table={account_table(user)} inuse={r['inuse']} server={r['ServerName'] or ''}")
    print(f"billing GPCode={r['GPCode'] or ''!r} RNo={r['RNo'] or 0} BNum={r['BNum'] or 0} Channel={r['Channel'] or ''!r}")
    print(f"auth_policy={dict(policy) if policy else 'implicit'}")
    print(f"pcbang_gameplay={dict(ent) if ent else 'not configured'}")
    print(f"gm={ [dict(x) for x in gms] if gms else 'not configured'}")


def status(db: sqlite3.Connection) -> None:
    version = db.execute("PRAGMA user_version").fetchone()[0]
    fallback = db.execute(
        "SELECT PolicyValue FROM LocalAuth__RuntimePolicy WHERE PolicyKey='legacy_billing_fallback'"
    ).fetchone()
    policies = db.execute("SELECT COUNT(*) FROM LocalAuth__AccountPolicy WHERE Enabled=1").fetchone()[0]
    locals_ = db.execute("SELECT COUNT(*) FROM LocalAuth__AccountPolicy WHERE Enabled=1 AND lower(AuthMode)='local'").fetchone()[0]
    legacy = db.execute("SELECT COUNT(*) FROM LocalAuth__AccountPolicy WHERE Enabled=1 AND lower(AuthMode)='legacy'").fetchone()[0]
    ent = db.execute("SELECT COUNT(*) FROM LocalAuth__GameplayEntitlement WHERE Enabled=1").fetchone()[0]
    gms = db.execute("SELECT COUNT(*) FROM AccountDB__GameMasters WHERE active=1").fetchone()[0]
    print(f"database={db.execute('PRAGMA database_list').fetchone()[2]}")
    print(f"user_version={version}")
    print(f"legacy_billing_fallback={(fallback[0] if fallback else '1')} (read on server startup)")
    print(f"account_policies={policies} local={locals_} legacy={legacy}")
    print(f"active_gameplay_entitlements={ent}")
    print(f"active_game_masters={gms}")



def is_zero_billing_value(value: object) -> bool:
    return value is None or str(value) in {"", "0"}


def audit_local_only(db: sqlite3.Connection, details: bool = False) -> bool:
    """Report whether every usable account can be served without Database.dll.

    This is intentionally read-only. Explicit legacy ownership and implicit
    rows carrying historical billing/PC-room identity are migration blockers.
    Explicit local rows are safe even when those historical columns remain.
    """
    accounts: dict[str, list[sqlite3.Row]] = {}
    duplicates: list[str] = []
    active_sessions = 0
    for initial in "ABCDEFGHIJKLMNOPQRSTUVWXYZ":
        table = f"AccountDB__{initial}GameUser"
        rows = list(db.execute(
            f'SELECT userid,GPCode,UsePeriod,inuse,RNo,BNum,Channel,BlockChk,DelChk '
            f'FROM "{table}"'
        ))
        grouped: dict[str, list[sqlite3.Row]] = {}
        for row in rows:
            key = str(row["userid"] or "").lower()
            grouped.setdefault(key, []).append(row)
            accounts.setdefault(key, []).append(row)
            if str(row["inuse"] or "0") not in {"", "0"}:
                active_sessions += 1
        duplicates.extend(str(group[0]["userid"]) for group in grouped.values() if len(group) > 1)

    policies = {
        str(r["UserID"]).lower(): str(r["AuthMode"]).lower()
        for r in db.execute("SELECT UserID,AuthMode FROM LocalAuth__AccountPolicy WHERE Enabled=1")
    }

    explicit_local = 0
    explicit_legacy: list[str] = []
    implicit_safe = 0
    implicit_provider: list[str] = []
    for key, rows in accounts.items():
        # Ambiguous duplicate identities are reported separately and never
        # considered ready even if one of the rows has a policy.
        row = rows[0]
        mode = policies.get(key)
        if mode == "local":
            explicit_local += 1
            continue
        if mode == "legacy":
            explicit_legacy.append(str(row["userid"]))
            continue
        safe_signature = (
            is_zero_billing_value(row["GPCode"])
            and int(row["UsePeriod"] or 0) == 0
            and int(row["RNo"] or 0) == 0
            and int(row["BNum"] or 0) == 0
            and is_zero_billing_value(row["Channel"])
        )
        if safe_signature:
            implicit_safe += 1
        else:
            implicit_provider.append(str(row["userid"]))

    orphan_policies = sorted(
        str(r["UserID"])
        for r in db.execute("SELECT UserID FROM LocalAuth__AccountPolicy WHERE Enabled=1")
        if str(r["UserID"]).lower() not in accounts
    )
    orphan_entitlements = sorted(
        str(r["UserID"])
        for r in db.execute("SELECT DISTINCT UserID FROM LocalAuth__GameplayEntitlement WHERE Enabled=1")
        if str(r["UserID"]).lower() not in accounts
    )
    orphan_gms = sorted(
        str(r["accountId"])
        for r in db.execute("SELECT accountId FROM AccountDB__GameMasters WHERE active=1")
        if str(r["accountId"]).lower() not in accounts
    )
    fk_violations = list(db.execute("PRAGMA foreign_key_check"))
    quick_check = db.execute("PRAGMA quick_check").fetchone()[0]

    blockers = len(explicit_legacy) + len(implicit_provider) + len(duplicates)
    integrity_issues = len(orphan_policies) + len(orphan_entitlements) + len(orphan_gms) + len(fk_violations)
    ready = blockers == 0 and integrity_issues == 0 and str(quick_check).lower() == "ok"

    print(f"accounts_total={sum(len(v) for v in accounts.values())}")
    print(f"explicit_local={explicit_local}")
    print(f"explicit_legacy={len(explicit_legacy)}")
    print(f"implicit_safe_local={implicit_safe}")
    print(f"implicit_provider_dependent={len(implicit_provider)}")
    print(f"active_sessions={active_sessions}")
    print(f"casefold_duplicates={len(duplicates)}")
    print(f"orphan_policies={len(orphan_policies)} orphan_entitlements={len(orphan_entitlements)} orphan_gms={len(orphan_gms)}")
    print(f"foreign_key_violations={len(fk_violations)} quick_check={quick_check}")
    print(f"local_only_ready={'yes' if ready else 'no'}")

    if details:
        for label, values in (
            ("legacy", explicit_legacy),
            ("implicit-provider", implicit_provider),
            ("duplicate", sorted(duplicates)),
            ("orphan-policy", orphan_policies),
            ("orphan-entitlement", orphan_entitlements),
            ("orphan-gm", orphan_gms),
        ):
            for value in values[:100]:
                print(f"{label}: {value}")
    return ready


def default_backup_path(source: Path) -> Path:
    stamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    return source.parent / "backups" / f"{source.stem}_{stamp}.sqlite3"


def database_health(db: sqlite3.Connection) -> tuple[str, int, int, int]:
    quick = str(db.execute("PRAGMA quick_check").fetchone()[0])
    fk_count = sum(1 for _ in db.execute("PRAGMA foreign_key_check"))
    version = int(db.execute("PRAGMA user_version").fetchone()[0])
    table_count = int(db.execute(
        "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name NOT LIKE 'sqlite_%'"
    ).fetchone()[0])
    return quick, fk_count, version, table_count


def backup_database(db: sqlite3.Connection, source: Path, destination: Path | None, overwrite: bool = False) -> Path:
    source = source.resolve()
    destination = (destination or default_backup_path(source)).expanduser().resolve()
    if destination == source:
        raise AdminError("backup destination must differ from source database")
    destination.parent.mkdir(parents=True, exist_ok=True)
    if destination.exists():
        if not overwrite:
            raise AdminError(f"backup already exists: {destination} (use --overwrite to replace it)")
        destination.unlink()

    source_quick, source_fk, source_version, source_tables = database_health(db)
    if source_quick.lower() != "ok" or source_fk:
        raise AdminError(
            f"source integrity check failed: quick_check={source_quick!r}, foreign_key_violations={source_fk}"
        )

    out = sqlite3.connect(destination, timeout=5.0)
    try:
        # sqlite3_backup captures a consistent snapshot, including committed WAL
        # content, without copying the database/WAL files by hand.
        db.backup(out)
        out.execute("PRAGMA foreign_keys=ON")
        backup_quick, backup_fk, backup_version, backup_tables = database_health(out)
    except Exception:
        out.close()
        destination.unlink(missing_ok=True)
        raise
    finally:
        try:
            out.close()
        except Exception:
            pass

    if (backup_quick.lower() != "ok" or backup_fk or backup_version != source_version
            or backup_tables != source_tables):
        destination.unlink(missing_ok=True)
        raise AdminError(
            "backup verification failed: "
            f"quick_check={backup_quick!r} foreign_key_violations={backup_fk} "
            f"user_version={backup_version}/{source_version} tables={backup_tables}/{source_tables}"
        )

    print(f"backup={destination}")
    print(f"backup_verified=yes user_version={backup_version} tables={backup_tables} quick_check={backup_quick}")
    return destination


def count_active_sessions(db: sqlite3.Connection) -> int:
    total = 0
    for initial in "ABCDEFGHIJKLMNOPQRSTUVWXYZ":
        table = f"AccountDB__{initial}GameUser"
        total += int(db.execute(
            f"SELECT COUNT(*) FROM \"{table}\" WHERE COALESCE(CAST(inuse AS TEXT),'0') NOT IN ('','0')"
        ).fetchone()[0])
    return total


def prepare_local_only(db: sqlite3.Connection, source: Path, destination: Path | None, overwrite: bool = False) -> bool:
    ready = audit_local_only(db, details=True)
    active = count_active_sessions(db)
    if not ready:
        print("prepare_local_only_ready=no reason=catalog_blockers")
        return False
    if active:
        print(f"prepare_local_only_ready=no reason=active_sessions count={active}")
        return False
    path = backup_database(db, source, destination, overwrite)
    print(f"prepare_local_only_ready=yes rollback_backup={path}")
    print("next_step=set-billing-fallback off, then restart and run the Windows runtime smoke suite")
    return True

def parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--db", type=Path, default=DEFAULT_DB, help=f"SQLite DB (default: {DEFAULT_DB})")
    sub = p.add_subparsers(dest="command", required=True)
    sub.add_parser("status")
    s = sub.add_parser("audit-local-only"); s.add_argument("--details", action="store_true")
    s = sub.add_parser("backup"); s.add_argument("destination", nargs="?", type=Path); s.add_argument("--overwrite", action="store_true")
    s = sub.add_parser("prepare-local-only"); s.add_argument("destination", nargs="?", type=Path); s.add_argument("--overwrite", action="store_true")
    s = sub.add_parser("show"); s.add_argument("user")
    s = sub.add_parser("create-account"); s.add_argument("user"); s.add_argument("--password"); s.add_argument("--auth", choices=("local","legacy"), default="local")
    s = sub.add_parser("set-password"); s.add_argument("user"); s.add_argument("--password")
    s = sub.add_parser("set-auth"); s.add_argument("user"); s.add_argument("mode", choices=("local","legacy","implicit"))
    s = sub.add_parser("set-billing-fallback"); s.add_argument("state", choices=("on","off"))
    s = sub.add_parser("grant-pcbang"); s.add_argument("user"); s.add_argument("--expires")
    s = sub.add_parser("revoke-pcbang"); s.add_argument("user")
    s = sub.add_parser("set-gm"); s.add_argument("user"); s.add_argument("--password"); s.add_argument("--security", type=int, default=4); s.add_argument("--inactive", action="store_true")
    s = sub.add_parser("remove-gm"); s.add_argument("user")
    return p


def main(argv: list[str] | None = None) -> int:
    args = parser().parse_args(argv)
    try:
        db = connect(args.db.resolve())
        try:
            exit_code = 0
            with db:
                c = args.command
                if c == "status": status(db)
                elif c == "audit-local-only": exit_code = 0 if audit_local_only(db, args.details) else 3
                elif c == "backup": backup_database(db, args.db.resolve(), args.destination, args.overwrite)
                elif c == "prepare-local-only": exit_code = 0 if prepare_local_only(db, args.db.resolve(), args.destination, args.overwrite) else 3
                elif c == "show": show_account(db, args.user)
                elif c == "create-account": create_account(db, args.user, password_value(args.password), args.auth); print(f"created {args.user} ({args.auth})")
                elif c == "set-password": set_account_password(db, args.user, password_value(args.password)); print(f"password updated: {args.user}")
                elif c == "set-auth": set_auth(db, args.user, args.mode); print(f"auth={args.mode}: {args.user}")
                elif c == "set-billing-fallback":
                    value = "1" if args.state == "on" else "0"
                    db.execute(
                        "INSERT INTO LocalAuth__RuntimePolicy(PolicyKey,PolicyValue,UpdatedAt) VALUES ('legacy_billing_fallback',?,datetime('now','localtime')) "
                        "ON CONFLICT(PolicyKey) DO UPDATE SET PolicyValue=excluded.PolicyValue,UpdatedAt=excluded.UpdatedAt", (value,)
                    )
                    print(f"legacy_billing_fallback={value}; restart server for this switch to take effect")
                elif c == "grant-pcbang": set_entitlement(db,args.user,True,args.expires); print(f"pcbang_gameplay enabled: {args.user}")
                elif c == "revoke-pcbang": set_entitlement(db,args.user,False); print(f"pcbang_gameplay disabled: {args.user}")
                elif c == "set-gm": set_gm(db,args.user,password_value(args.password,gm=True),args.security,0 if args.inactive else 1); print(f"GM updated: {args.user}")
                elif c == "remove-gm": require_one(db,args.user); db.execute("DELETE FROM AccountDB__GameMasters WHERE lower(accountId)=lower(?)",(args.user,)); print(f"GM removed: {args.user}")
                else: raise AdminError(f"unsupported command: {c}")
        finally:
            db.close()
        return exit_code
    except (AdminError, sqlite3.Error) as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
