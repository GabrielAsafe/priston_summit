#!/usr/bin/env python3
"""Exercise physical SQLite progression contracts, rollback and restart semantics."""

from pathlib import Path
import os
import sqlite3
import tempfile
import sys

ROOT = Path(__file__).resolve().parents[1]
MIGRATIONS = [
    ROOT / "Database" / "migrations" / "033_progression_v2.sql",
    ROOT / "Database" / "migrations" / "034_story_node_state.sql",
    ROOT / "Database" / "migrations" / "035_progression_content.sql",
    ROOT / "Database" / "migrations" / "036_covenant_rewards.sql",
    ROOT / "Database" / "migrations" / "037_story_dialogue_choices.sql",
    ROOT / "Database" / "migrations" / "038_generic_reputation.sql",
]


def fail(message: str) -> None:
    print(f"progression repository contracts: FAIL: {message}", file=sys.stderr)
    raise SystemExit(1)


def apply_migrations(db: sqlite3.Connection) -> None:
    for path in MIGRATIONS:
        if not path.is_file():
            fail(f"missing migration {path}")
        db.executescript(path.read_text(encoding="utf-8"))


def row(db: sqlite3.Connection, sql: str, args=()):
    return db.execute(sql, args).fetchone()


def main() -> None:
    fd, db_path = tempfile.mkstemp(prefix="takeoff_progression_", suffix=".sqlite3")
    os.close(fd)
    try:
        db = sqlite3.connect(db_path)
        apply_migrations(db)

        # Covenant membership + source-key ledger.
        with db:
            db.execute(
                "INSERT INTO ServerDB__CovenantMembership"
                "(PlayerKey,CovenantId,Reputation,RankId,Active) VALUES(?,?,?,?,?)",
                ("Alice", 1, 0, 10, 1),
            )
            db.execute(
                "UPDATE ServerDB__CovenantMembership SET Reputation=?,RankId=? "
                "WHERE PlayerKey=? AND CovenantId=?",
                (25, 10, "Alice", 1),
            )
            db.execute(
                "INSERT INTO ServerDB__CovenantReputationLedger"
                "(PlayerKey,CovenantId,SourceKey,Delta,ReputationAfter,RankIdAfter) "
                "VALUES(?,?,?,?,?,?)",
                ("Alice", 1, "story.1.reward.0", 25, 25, 10),
            )

        ledger = row(
            db,
            "SELECT Delta,ReputationAfter,RankIdAfter FROM ServerDB__CovenantReputationLedger "
            "WHERE PlayerKey=? AND CovenantId=? AND SourceKey=?",
            ("Alice", 1, "story.1.reward.0"),
        )
        if ledger != (25, 25, 10):
            fail(f"unexpected reputation ledger row {ledger}")
        try:
            with db:
                db.execute(
                    "INSERT INTO ServerDB__CovenantReputationLedger"
                    "(PlayerKey,CovenantId,SourceKey,Delta,ReputationAfter,RankIdAfter) "
                    "VALUES(?,?,?,?,?,?)",
                    ("Alice", 1, "story.1.reward.0", 99, 124, 10),
                )
        except sqlite3.IntegrityError:
            pass
        else:
            fail("duplicate Covenant reputation source key was accepted")

        # Generic Reputation has an independent state namespace and source-key ledger.
        with db:
            db.execute(
                "INSERT INTO ServerDB__ProgressionReputationDefinition"
                "(ReputationKey,DisplayNameKey,HasBounds,MinValue,MaxValue,Enabled) VALUES(?,?,?,?,?,?)",
                ("faction.alpha", "faction.alpha.name", 1, -100, 1000, 1),
            )
            db.execute(
                "INSERT INTO ServerDB__GenericReputation(PlayerKey,ReputationKey,Value) VALUES(?,?,?)",
                ("Alice", "faction.alpha", 25),
            )
            db.execute(
                "INSERT INTO ServerDB__GenericReputationLedger"
                "(PlayerKey,ReputationKey,SourceKey,Delta,ValueAfter) VALUES(?,?,?,?,?)",
                ("Alice", "faction.alpha", "story.1.generic_rep", 25, 25),
            )
        db.close()
        db = sqlite3.connect(db_path)
        if row(
            db,
            "SELECT Value FROM ServerDB__GenericReputation WHERE PlayerKey=? AND ReputationKey=?",
            ("Alice", "faction.alpha"),
        ) != (25,):
            fail("generic Reputation state did not survive restart")
        if row(
            db,
            "SELECT Delta,ValueAfter FROM ServerDB__GenericReputationLedger "
            "WHERE PlayerKey=? AND ReputationKey=? AND SourceKey=?",
            ("Alice", "faction.alpha", "story.1.generic_rep"),
        ) != (25, 25):
            fail("generic Reputation source ledger did not survive restart")
        try:
            with db:
                db.execute(
                    "INSERT INTO ServerDB__GenericReputationLedger"
                    "(PlayerKey,ReputationKey,SourceKey,Delta,ValueAfter) VALUES(?,?,?,?,?)",
                    ("Alice", "faction.alpha", "story.1.generic_rep", 99, 124),
                )
        except sqlite3.IntegrityError:
            pass
        else:
            fail("duplicate generic Reputation source key was accepted")

        # Covenant reward receipts are durable one-shot milestones.
        with db:
            db.execute(
                "INSERT INTO ServerDB__CovenantEffectReceipt"
                "(PlayerKey,CovenantId,EffectKey,Applied) VALUES(?,?,?,0)",
                ("Alice", 1, "join.gold"),
            )
        db.close()
        db = sqlite3.connect(db_path)
        if row(
            db,
            "SELECT Applied FROM ServerDB__CovenantEffectReceipt "
            "WHERE PlayerKey=? AND CovenantId=? AND EffectKey=?",
            ("Alice", 1, "join.gold"),
        ) != (0,):
            fail("pending Covenant reward receipt did not survive restart")
        with db:
            db.execute(
                "UPDATE ServerDB__CovenantEffectReceipt SET Applied=1 "
                "WHERE PlayerKey=? AND CovenantId=? AND EffectKey=? AND Applied=0",
                ("Alice", 1, "join.gold"),
            )
        if row(
            db,
            "SELECT Applied FROM ServerDB__CovenantEffectReceipt "
            "WHERE PlayerKey='Alice' AND CovenantId=1 AND EffectKey='join.gold'",
        ) != (1,):
            fail("Covenant reward acknowledgement was not persisted")

        # Story initial state is durable and node/progress agree.
        with db:
            db.execute(
                "INSERT INTO ServerDB__StoryNodeState(PlayerKey,StoryId,NodeId,State) VALUES(?,?,?,?)",
                ("Alice", 100, 10, 1),
            )
            db.execute(
                "INSERT INTO ServerDB__StoryProgress"
                "(PlayerKey,StoryId,ChapterId,NodeId,State,EnterApplied,CompletionApplied) "
                "VALUES(?,?,?,?,?,?,?)",
                ("Alice", 100, 1, 10, 1, 1, 0),
            )

        # A failed transition must roll back all three writes.
        try:
            db.execute("BEGIN IMMEDIATE")
            db.execute(
                "UPDATE ServerDB__StoryNodeState SET State=2 WHERE PlayerKey=? AND StoryId=? AND NodeId=?",
                ("Alice", 100, 10),
            )
            db.execute(
                "INSERT INTO ServerDB__StoryNodeState(PlayerKey,StoryId,NodeId,State) VALUES(?,?,?,?)",
                ("Alice", 100, 20, 1),
            )
            db.execute(
                "UPDATE ServerDB__StoryProgress SET NodeId=20,State=1,EnterApplied=0,CompletionApplied=0 "
                "WHERE PlayerKey=? AND StoryId=?",
                ("Alice", 100),
            )
            raise RuntimeError("simulate crash before commit")
        except RuntimeError:
            db.rollback()

        if row(db, "SELECT State FROM ServerDB__StoryNodeState WHERE PlayerKey='Alice' AND StoryId=100 AND NodeId=10") != (1,):
            fail("rollback did not restore old node state")
        if row(db, "SELECT 1 FROM ServerDB__StoryNodeState WHERE PlayerKey='Alice' AND StoryId=100 AND NodeId=20") is not None:
            fail("rollback left target node state behind")
        if row(db, "SELECT NodeId,State FROM ServerDB__StoryProgress WHERE PlayerKey='Alice' AND StoryId=100") != (10, 1):
            fail("rollback left StoryProgress transitioned")

        # Successful transition is atomic from the physical schema perspective.
        with db:
            db.execute(
                "UPDATE ServerDB__StoryNodeState SET State=2 WHERE PlayerKey=? AND StoryId=? AND NodeId=?",
                ("Alice", 100, 10),
            )
            db.execute(
                "INSERT INTO ServerDB__StoryNodeState(PlayerKey,StoryId,NodeId,State) VALUES(?,?,?,?)",
                ("Alice", 100, 20, 1),
            )
            db.execute(
                "UPDATE ServerDB__StoryProgress SET NodeId=20,State=1,EnterApplied=0,CompletionApplied=0 "
                "WHERE PlayerKey=? AND StoryId=?",
                ("Alice", 100),
            )

        # Pending receipt survives restart and explicit acknowledgement does not replay anything.
        with db:
            db.execute(
                "INSERT INTO ServerDB__StoryEffectReceipt"
                "(PlayerKey,StoryId,NodeId,EffectKey,Applied) VALUES(?,?,?,?,0)",
                ("Alice", 100, 20, "reward.0"),
            )
        db.close()

        db = sqlite3.connect(db_path)
        receipt = row(
            db,
            "SELECT Applied FROM ServerDB__StoryEffectReceipt "
            "WHERE PlayerKey=? AND StoryId=? AND NodeId=? AND EffectKey=?",
            ("Alice", 100, 20, "reward.0"),
        )
        if receipt != (0,):
            fail(f"pending receipt did not survive restart: {receipt}")
        with db:
            db.execute(
                "UPDATE ServerDB__StoryEffectReceipt SET Applied=1 "
                "WHERE PlayerKey=? AND StoryId=? AND NodeId=? AND EffectKey=? AND Applied=0",
                ("Alice", 100, 20, "reward.0"),
            )
        if row(db, "SELECT Applied FROM ServerDB__StoryEffectReceipt WHERE PlayerKey='Alice' AND StoryId=100 AND NodeId=20 AND EffectKey='reward.0'") != (1,):
            fail("receipt acknowledgement was not persisted")

        # Player/world flags also survive the same physical database restart.
        with db:
            db.execute(
                "INSERT INTO ServerDB__StoryPlayerFlag(PlayerKey,FlagKey,IntValue) VALUES(?,?,?)",
                ("Alice", "story.flag", 7),
            )
            db.execute(
                "INSERT INTO ServerDB__StoryWorldFlag(FlagKey,IntValue) VALUES(?,?)",
                ("world.flag", 9),
            )
        db.close()
        db = sqlite3.connect(db_path)
        if row(db, "SELECT IntValue FROM ServerDB__StoryPlayerFlag WHERE PlayerKey='Alice' AND FlagKey='story.flag'") != (7,):
            fail("player flag did not survive restart")
        if row(db, "SELECT IntValue FROM ServerDB__StoryWorldFlag WHERE FlagKey='world.flag'") != (9,):
            fail("world flag did not survive restart")
        db.close()

        print("progression repository contracts: PASS")
    finally:
        try:
            os.unlink(db_path)
        except FileNotFoundError:
            pass


if __name__ == "__main__":
    main()
