#!/usr/bin/env python3
"""Physical SQLite restart contract for Covenant V1.

This intentionally exercises the schema as the server sees it on disk: all
migrations are applied in order, state is committed, the connection is closed,
and the database is reopened before assertions are made.
"""
from __future__ import annotations

import os
import sqlite3
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MIGRATIONS = ROOT / "Database" / "migrations"


def fail(message: str) -> None:
    raise AssertionError(message)


def apply_all(db: sqlite3.Connection) -> None:
    files = sorted(MIGRATIONS.glob("[0-9][0-9][0-9]_*.sql"))
    if not files:
        fail("no migrations found")
    for path in files:
        db.executescript(path.read_text(encoding="utf-8", errors="replace"))
    db.execute(f"PRAGMA user_version={int(files[-1].name[:3])}")
    db.commit()


def reopen(path: str) -> sqlite3.Connection:
    db = sqlite3.connect(path)
    db.execute("PRAGMA foreign_keys=ON")
    return db


def main() -> None:
    handle, path = tempfile.mkstemp(prefix="summit_covenant_restart_", suffix=".sqlite3")
    os.close(handle)
    try:
        db = reopen(path)
        apply_all(db)

        player = "CovenantRestartSmoke"
        now = 1_800_000_000

        # Durable membership in every Covenant, one active at a time.
        for covenant_id in range(1, 6):
            db.execute(
                "INSERT INTO ServerDB__CovenantMembership"
                "(PlayerKey,CovenantId,Reputation,RankId,Active) VALUES(?,?,?,?,?)",
                (player, covenant_id, covenant_id * 100, 1, 1 if covenant_id == 1 else 0),
            )
        db.commit()

        # The database itself must reject a second active membership.
        try:
            db.execute(
                "UPDATE ServerDB__CovenantMembership SET Active=1 "
                "WHERE PlayerKey=? AND CovenantId=2",
                (player,),
            )
            db.commit()
            fail("unique-active Covenant constraint did not reject a second active membership")
        except sqlite3.IntegrityError:
            db.rollback()

        # Simulate the runtime's atomic switch and persisted switch cooldown.
        db.execute("BEGIN IMMEDIATE")
        db.execute(
            "UPDATE ServerDB__CovenantMembership SET Active=0 WHERE PlayerKey=? AND Active=1",
            (player,),
        )
        db.execute(
            "UPDATE ServerDB__CovenantMembership SET Active=1 WHERE PlayerKey=? AND CovenantId=2",
            (player,),
        )
        db.execute(
            "INSERT INTO ServerDB__CovenantPlayerState(PlayerKey,SwitchCooldownUntil) VALUES(?,?) "
            "ON CONFLICT(PlayerKey) DO UPDATE SET SwitchCooldownUntil=excluded.SwitchCooldownUntil, "
            "UpdatedAt=datetime('now','localtime')",
            (player, now + 300),
        )
        db.commit()

        # Challenge progress/reward state.
        db.execute(
            "INSERT INTO ServerDB__CovenantChallengeProgress"
            "(PlayerKey,CovenantId,ChallengeKey,Progress,Completed,RewardClaimed) "
            "VALUES(?,?,?,?,?,?)",
            (player, 2, "gatherer_trial_i", 10, 1, 1),
        )

        # Permanent mastery + collectible + inventory sidecar.
        db.execute(
            "INSERT INTO ServerDB__CovenantMastery(PlayerKey,CovenantId,MasteredAt) VALUES(?,?,?)",
            (player, 2, now),
        )
        db.execute(
            "INSERT INTO ServerDB__CovenantCollection(PlayerKey,CovenantId,CollectibleKey,AcquiredAt) "
            "VALUES(?,?,?,?)",
            (player, 2, "sigil:gatherers", now),
        )
        db.execute(
            "INSERT INTO ServerDB__CovenantInventory(PlayerKey,CovenantId,ItemKey,Quantity,UpdatedAt) "
            "VALUES(?,?,?,?,?)",
            (player, 2, "special_material:iron_ore", 4, now),
        )

        # Persisted Artisan mode is player-selected and must survive restart.
        db.execute(
            "INSERT INTO ServerDB__CovenantArtisanState(PlayerKey,Mode) VALUES(?,?)",
            (player, 2),
        )

        # Tier III power state has to survive restart, including cooldown.
        db.execute(
            "INSERT INTO ServerDB__CovenantPowerState"
            "(PlayerKey,CovenantId,PowerKey,ActiveUntil,CooldownUntil,TargetSerial) "
            "VALUES(?,?,?,?,?,?)",
            (player, 4, "tempt_fate", now + 60, now + 600, 0),
        )
        db.commit()
        db.close()

        # Real restart boundary.
        db = reopen(path)
        version = db.execute("PRAGMA user_version").fetchone()[0]
        if version < 43:
            fail(f"unexpected schema version after restart: {version}")

        active = db.execute(
            "SELECT CovenantId FROM ServerDB__CovenantMembership WHERE PlayerKey=? AND Active=1",
            (player,),
        ).fetchall()
        if active != [(2,)]:
            fail(f"active Covenant was not preserved across restart: {active}")

        preserved = db.execute(
            "SELECT CovenantId,Reputation,RankId FROM ServerDB__CovenantMembership "
            "WHERE PlayerKey=? ORDER BY CovenantId",
            (player,),
        ).fetchall()
        if len(preserved) != 5 or preserved[0][1:] != (100, 1) or preserved[-1][1:] != (500, 1):
            fail(f"inactive Covenant progress was not preserved: {preserved}")

        cooldown = db.execute(
            "SELECT SwitchCooldownUntil FROM ServerDB__CovenantPlayerState WHERE PlayerKey=?",
            (player,),
        ).fetchone()
        if cooldown != (now + 300,):
            fail(f"switch cooldown did not persist: {cooldown}")

        challenge = db.execute(
            "SELECT Progress,Completed,RewardClaimed FROM ServerDB__CovenantChallengeProgress "
            "WHERE PlayerKey=? AND CovenantId=2 AND ChallengeKey='gatherer_trial_i'",
            (player,),
        ).fetchone()
        if challenge != (10, 1, 1):
            fail(f"challenge state did not persist: {challenge}")

        mastery = db.execute(
            "SELECT CovenantId FROM ServerDB__CovenantMastery WHERE PlayerKey=?",
            (player,),
        ).fetchall()
        if mastery != [(2,)]:
            fail(f"mastery did not persist: {mastery}")

        collectible = db.execute(
            "SELECT CollectibleKey FROM ServerDB__CovenantCollection WHERE PlayerKey=?",
            (player,),
        ).fetchall()
        if collectible != [("sigil:gatherers",)]:
            fail(f"Sigil collection did not persist: {collectible}")

        inventory = db.execute(
            "SELECT ItemKey,Quantity FROM ServerDB__CovenantInventory WHERE PlayerKey=?",
            (player,),
        ).fetchall()
        if inventory != [("special_material:iron_ore", 4)]:
            fail(f"Covenant inventory did not persist: {inventory}")

        artisan_mode = db.execute(
            "SELECT Mode FROM ServerDB__CovenantArtisanState WHERE PlayerKey=?",
            (player,),
        ).fetchone()
        if artisan_mode != (2,):
            fail(f"Artisan mode did not persist: {artisan_mode}")

        power = db.execute(
            "SELECT ActiveUntil,CooldownUntil,TargetSerial FROM ServerDB__CovenantPowerState "
            "WHERE PlayerKey=? AND CovenantId=4 AND PowerKey='tempt_fate'",
            (player,),
        ).fetchone()
        if power != (now + 60, now + 600, 0):
            fail(f"Tier III power state did not persist: {power}")

        # Rerun the newest migrations to prove idempotence/backfill safety.
        for name in (
            "039_covenant_single_active.sql",
            "040_covenant_default_content.sql",
            "041_covenant_runtime_complete.sql",
            "042_covenant_inventory.sql",
            "043_covenant_artisan_mode.sql",
        ):
            db.executescript((MIGRATIONS / name).read_text(encoding="utf-8", errors="replace"))
        db.commit()
        duplicate_sigils = db.execute(
            "SELECT COUNT(*) FROM ServerDB__CovenantInventory "
            "WHERE PlayerKey=? AND ItemKey='sigil:gatherers'",
            (player,),
        ).fetchone()[0]
        if duplicate_sigils != 1:
            fail(f"migration rerun duplicated Sigil inventory rows: {duplicate_sigils}")

        db.close()
        print("Covenant SQLite restart E2E: PASS")
    finally:
        try:
            os.remove(path)
        except OSError:
            pass


if __name__ == "__main__":
    main()
