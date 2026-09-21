#!/usr/bin/env python3
"""Validate the TAKEOFF Covenant/StoryState SQLite schema migrations."""

from pathlib import Path
import sqlite3
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

EXPECTED_TABLES = {
    "ServerDB__CovenantMembership",
    "ServerDB__CovenantReputationLedger",
    "ServerDB__StoryProgress",
    "ServerDB__StoryPlayerFlag",
    "ServerDB__StoryWorldFlag",
    "ServerDB__StoryEffectReceipt",
    "ServerDB__StoryNodeState",
    "ServerDB__ProgressionCovenantDefinition",
    "ServerDB__ProgressionCovenantRank",
    "ServerDB__ProgressionCovenantJoinCondition",
    "ServerDB__ProgressionCovenantRankBenefit",
    "ServerDB__ProgressionCovenantRelatedQuest",
    "ServerDB__ProgressionCovenantRelatedStory",
    "ServerDB__ProgressionStoryDefinition",
    "ServerDB__ProgressionStoryStartCondition",
    "ServerDB__ProgressionStoryArc",
    "ServerDB__ProgressionStoryChapter",
    "ServerDB__ProgressionStoryChapterCondition",
    "ServerDB__ProgressionStoryNode",
    "ServerDB__ProgressionStoryNodeEntryCondition",
    "ServerDB__ProgressionStoryNodeCompletionCondition",
    "ServerDB__ProgressionStoryNodeEnterAction",
    "ServerDB__ProgressionStoryNodeCompletionAction",
    "ServerDB__ProgressionStoryTransition",
    "ServerDB__ProgressionStoryNodeReward",
    "ServerDB__ProgressionStoryDialogueOption",
    "ServerDB__CovenantEffectReceipt",
    "ServerDB__ProgressionCovenantReward",
    "ServerDB__ProgressionStoryDialogueChoice",
    "ServerDB__ProgressionStoryDialogueChoiceCondition",
    "ServerDB__ProgressionReputationDefinition",
    "ServerDB__GenericReputation",
    "ServerDB__GenericReputationLedger",
}

EXPECTED_INDEXES = {
    "ServerDB__CovenantMembership_Active",
    "ServerDB__StoryEffectReceipt_Pending",
    "ServerDB__StoryNodeState_ByStory",
    "ServerDB__ProgressionCovenantRank_ByCovenant",
    "ServerDB__ProgressionStoryChapter_ByStory",
    "ServerDB__ProgressionStoryNode_ByStory",
    "ServerDB__ProgressionStoryTransition_ByNode",
    "ServerDB__CovenantEffectReceipt_Pending",
    "ServerDB__ProgressionCovenantReward_ByMilestone",
    "ServerDB__ProgressionStoryDialogueChoice_ByNode",
}


def fail(message: str) -> None:
    print(f"progression migration validation: FAIL: {message}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    sql_parts = []
    for migration in MIGRATIONS:
        if not migration.is_file():
            fail(f"missing {migration}")
        sql_parts.append(migration.read_text(encoding="utf-8"))

    db = sqlite3.connect(":memory:")
    try:
        # Bootstrap/repair must be idempotent.
        for sql in sql_parts:
            db.executescript(sql)
        for sql in sql_parts:
            db.executescript(sql)

        tables = {
            row[0]
            for row in db.execute(
                "SELECT name FROM sqlite_master WHERE type='table' AND name LIKE 'ServerDB__%'"
            )
        }
        if not EXPECTED_TABLES.issubset(tables):
            fail(f"missing tables: {sorted(EXPECTED_TABLES - tables)}")

        indexes = {
            row[0]
            for row in db.execute(
                "SELECT name FROM sqlite_master WHERE type='index' AND name LIKE 'ServerDB__%'"
            )
        }
        if not EXPECTED_INDEXES.issubset(indexes):
            fail(f"missing indexes: {sorted(EXPECTED_INDEXES - indexes)}")

        with db:
            db.execute(
                "INSERT INTO ServerDB__CovenantMembership"
                "(PlayerKey,CovenantId,Reputation,RankId,Active) VALUES(?,?,?,?,?)",
                ("validator", 1, 0, 0, 1),
            )
            db.execute(
                "INSERT INTO ServerDB__CovenantReputationLedger"
                "(PlayerKey,CovenantId,SourceKey,Delta,ReputationAfter,RankIdAfter) "
                "VALUES(?,?,?,?,?,?)",
                ("validator", 1, "validator.source", 10, 10, 1),
            )
            db.execute(
                "INSERT INTO ServerDB__StoryProgress"
                "(PlayerKey,StoryId,ChapterId,NodeId,State,EnterApplied,CompletionApplied) "
                "VALUES(?,?,?,?,?,?,?)",
                ("validator", 1, 1, 1, 1, 0, 0),
            )
            db.execute(
                "INSERT INTO ServerDB__StoryPlayerFlag(PlayerKey,FlagKey,IntValue) VALUES(?,?,?)",
                ("validator", "validator.flag", 1),
            )
            db.execute(
                "INSERT INTO ServerDB__StoryWorldFlag(FlagKey,IntValue) VALUES(?,?)",
                ("validator.world", 1),
            )
            db.execute(
                "INSERT INTO ServerDB__StoryEffectReceipt"
                "(PlayerKey,StoryId,NodeId,EffectKey,Applied) VALUES(?,?,?,?,?)",
                ("validator", 1, 1, "validator.effect", 1),
            )
            db.execute(
                "INSERT INTO ServerDB__StoryNodeState(PlayerKey,StoryId,NodeId,State) VALUES(?,?,?,?)",
                ("validator", 1, 1, 2),
            )
            # Generic Reputation definitions/state use their own namespace and source ledger.
            db.execute(
                "INSERT INTO ServerDB__ProgressionReputationDefinition"
                "(ReputationKey,DisplayNameKey,HasBounds,MinValue,MaxValue,Enabled) VALUES(?,?,?,?,?,?)",
                ("validator.faction", "validator.faction.name", 1, -100, 1000, 1),
            )
            db.execute(
                "INSERT INTO ServerDB__GenericReputation(PlayerKey,ReputationKey,Value) VALUES(?,?,?)",
                ("validator", "validator.faction", 10),
            )
            db.execute(
                "INSERT INTO ServerDB__GenericReputationLedger"
                "(PlayerKey,ReputationKey,SourceKey,Delta,ValueAfter) VALUES(?,?,?,?,?)",
                ("validator", "validator.faction", "validator.source", 10, 10),
            )
            # Production content catalog: intentionally explicit and data-driven.
            db.execute(
                "INSERT INTO ServerDB__ProgressionCovenantDefinition"
                "(CovenantId,Key,DisplayNameKey,ExclusiveMembership,HasReputationBounds,MinReputation,MaxReputation,Enabled) "
                "VALUES(?,?,?,?,?,?,?,?)",
                (101, "validator.covenant", "validator.covenant.name", 1, 1, -100, 1000, 1),
            )
            db.execute(
                "INSERT INTO ServerDB__ProgressionCovenantRank"
                "(CovenantId,RankId,Key,DisplayNameKey,MinReputation) VALUES(?,?,?,?,?)",
                (101, 1, "rank.initiate", "rank.initiate.name", 0),
            )
            db.execute(
                "INSERT INTO ServerDB__ProgressionCovenantJoinCondition"
                "(CovenantId,SortOrder,ConditionType,Key,Id,Value,AuxiliaryId) VALUES(?,?,?,?,?,?,?)",
                (101, 0, 1, "", 0, 10, 0),
            )
            db.execute(
                "INSERT INTO ServerDB__ProgressionCovenantRankBenefit"
                "(CovenantId,RankId,SortOrder,StatKey,Operation,Value,ScopeKey) VALUES(?,?,?,?,?,?,?)",
                (101, 1, 0, "validator.stat", 0, 5.0, "validator.scope"),
            )
            db.execute(
                "INSERT INTO ServerDB__ProgressionCovenantRelatedQuest"
                "(CovenantId,SortOrder,QuestId) VALUES(?,?,?)", (101, 0, 9001),
            )
            db.execute(
                "INSERT INTO ServerDB__ProgressionCovenantRelatedStory"
                "(CovenantId,SortOrder,StoryId) VALUES(?,?,?)", (101, 0, 201),
            )
            db.execute(
                "INSERT INTO ServerDB__CovenantEffectReceipt"
                "(PlayerKey,CovenantId,EffectKey,Applied) VALUES(?,?,?,?)",
                ("validator", 101, "join.gold", 0),
            )
            db.execute(
                "INSERT INTO ServerDB__ProgressionCovenantReward"
                "(CovenantId,RewardKey,RankId,SortOrder,RewardType,Key,Id,Amount) "
                "VALUES(?,?,?,?,?,?,?,?)",
                (101, "join.gold", 0, 0, 1, "", 0, 10),
            )
            db.execute(
                "INSERT INTO ServerDB__ProgressionCovenantReward"
                "(CovenantId,RewardKey,RankId,SortOrder,RewardType,Key,Id,Amount) "
                "VALUES(?,?,?,?,?,?,?,?)",
                (101, "rank.flag", 1, 0, 4, "validator.rank.flag", 0, 1),
            )
            db.execute(
                "INSERT INTO ServerDB__ProgressionStoryDefinition"
                "(StoryId,Key,DisplayNameKey,InitialChapterId,EntryNodeId,Enabled) VALUES(?,?,?,?,?,?)",
                (201, "validator.story", "validator.story.name", 1, 10, 1),
            )
            db.execute(
                "INSERT INTO ServerDB__ProgressionStoryStartCondition"
                "(StoryId,SortOrder,ConditionType,Key,Id,Value,AuxiliaryId) VALUES(?,?,?,?,?,?,?)",
                (201, 0, 1, "", 0, 1, 0),
            )
            db.execute(
                "INSERT INTO ServerDB__ProgressionStoryStartCondition"
                "(StoryId,SortOrder,ConditionType,Key,Id,Value,AuxiliaryId) VALUES(?,?,?,?,?,?,?)",
                (201, 1, 11, "world.ready", 0, 0, 0),
            )
            db.execute(
                "INSERT INTO ServerDB__ProgressionStoryStartCondition"
                "(StoryId,SortOrder,ConditionType,Key,Id,Value,AuxiliaryId) VALUES(?,?,?,?,?,?,?)",
                (201, 2, 12, "validator.faction", 0, 5, 0),
            )
            db.execute(
                "INSERT INTO ServerDB__ProgressionStoryArc"
                "(StoryId,ArcId,Key,DisplayNameKey) VALUES(?,?,?,?)",
                (201, 1, "arc.validator", "arc.validator.name"),
            )
            db.execute(
                "INSERT INTO ServerDB__ProgressionStoryChapter"
                "(StoryId,ChapterId,ArcId,Key,DisplayNameKey) VALUES(?,?,?,?,?)",
                (201, 1, 1, "chapter.validator", "chapter.validator.name"),
            )
            db.execute(
                "INSERT INTO ServerDB__ProgressionStoryChapterCondition"
                "(StoryId,ChapterId,SortOrder,ConditionType,Key,Id,Value,AuxiliaryId) VALUES(?,?,?,?,?,?,?,?)",
                (201, 1, 0, 0, "", 0, 0, 0),
            )
            db.execute(
                "INSERT INTO ServerDB__ProgressionStoryNode"
                "(StoryId,NodeId,ChapterId,Key,DialogueKey,SpeakerKey,QuestId,CovenantId) VALUES(?,?,?,?,?,?,?,?)",
                (201, 10, 1, "node.validator", "dialogue.validator", "speaker.validator", 9001, 101),
            )
            db.execute(
                "INSERT INTO ServerDB__ProgressionStoryNodeEntryCondition"
                "(StoryId,NodeId,SortOrder,ConditionType,Key,Id,Value,AuxiliaryId) VALUES(?,?,?,?,?,?,?,?)",
                (201, 10, 0, 6, "", 101, 0, 0),
            )
            db.execute(
                "INSERT INTO ServerDB__ProgressionStoryNodeCompletionCondition"
                "(StoryId,NodeId,SortOrder,ConditionType,Key,Id,Value,AuxiliaryId) VALUES(?,?,?,?,?,?,?,?)",
                (201, 10, 0, 5, "", 9001, 2, 0),
            )
            db.execute(
                "INSERT INTO ServerDB__ProgressionStoryNodeEnterAction"
                "(StoryId,NodeId,SortOrder,ActionType,Key,Id,Value) VALUES(?,?,?,?,?,?,?)",
                (201, 10, 0, 0, "validator.entered", 0, 1),
            )
            db.execute(
                "INSERT INTO ServerDB__ProgressionStoryNodeCompletionAction"
                "(StoryId,NodeId,SortOrder,ActionType,Key,Id,Value) VALUES(?,?,?,?,?,?,?)",
                (201, 10, 0, 3, "", 101, 5),
            )
            db.execute(
                "INSERT INTO ServerDB__ProgressionStoryTransition"
                "(StoryId,FromNodeId,SortOrder,ToNodeId) VALUES(?,?,?,?)",
                (201, 10, 0, 20),
            )
            db.execute(
                "INSERT INTO ServerDB__ProgressionStoryNodeReward"
                "(StoryId,NodeId,SortOrder,RewardType,Key,Id,Amount) VALUES(?,?,?,?,?,?,?)",
                (201, 10, 0, 5, "", 101, 5),
            )
            db.execute(
                "INSERT INTO ServerDB__ProgressionStoryNodeReward"
                "(StoryId,NodeId,SortOrder,RewardType,Key,Id,Amount) VALUES(?,?,?,?,?,?,?)",
                (201, 10, 1, 3, "validator.faction", 0, 5),
            )
            db.execute(
                "INSERT INTO ServerDB__ProgressionStoryDialogueOption"
                "(StoryId,NodeId,SortOrder,OptionKey) VALUES(?,?,?,?)",
                (201, 10, 0, "dialogue.validator.continue"),
            )
            db.execute(
                "INSERT INTO ServerDB__ProgressionStoryDialogueChoice"
                "(StoryId,NodeId,SortOrder,ChoiceKey,DisplayKey,ToNodeId) VALUES(?,?,?,?,?,?)",
                (201, 10, 0, "dialogue.validator.left", "dialogue.validator.left.text", 20),
            )
            db.execute(
                "INSERT INTO ServerDB__ProgressionStoryDialogueChoiceCondition"
                "(StoryId,NodeId,ChoiceKey,SortOrder,ConditionType,Key,Id,Value,AuxiliaryId) "
                "VALUES(?,?,?,?,?,?,?,?,?)",
                (201, 10, "dialogue.validator.left", 0, 1, "", 0, 1, 0),
            )

        # Re-running the v38 repair with populated old/new content must preserve data,
        # including the newly enabled generic Reputation condition/reward forms.
        db.executescript(MIGRATIONS[-1].read_text(encoding="utf-8"))
        if db.execute(
            "SELECT COUNT(*) FROM ServerDB__ProgressionStoryStartCondition "
            "WHERE StoryId=201 AND ConditionType=12 AND Key='validator.faction'"
        ).fetchone() != (1,):
            fail("v38 idempotent rebuild lost ReputationAtLeast content")
        if db.execute(
            "SELECT COUNT(*) FROM ServerDB__ProgressionStoryNodeReward "
            "WHERE StoryId=201 AND NodeId=10 AND RewardType=3 AND Key='validator.faction'"
        ).fetchone() != (1,):
            fail("v38 idempotent rebuild lost generic Reputation reward content")
        if db.execute(
            "SELECT Value FROM ServerDB__GenericReputation "
            "WHERE PlayerKey='validator' AND ReputationKey='validator.faction'"
        ).fetchone() != (10,):
            fail("v38 idempotent rebuild disturbed generic Reputation state")

        for sql, args, label in [
            (
                "INSERT INTO ServerDB__CovenantMembership"
                "(PlayerKey,CovenantId,Reputation,RankId,Active) VALUES(?,?,?,?,?)",
                ("invalid", 2, 0, 0, 2),
                "CovenantMembership.Active",
            ),
            (
                "INSERT INTO ServerDB__StoryNodeState(PlayerKey,StoryId,NodeId,State) VALUES(?,?,?,?)",
                ("invalid", 1, 2, 9),
                "StoryNodeState.State",
            ),
            (
                "INSERT INTO ServerDB__ProgressionCovenantDefinition"
                "(CovenantId,Key,ExclusiveMembership,HasReputationBounds,Enabled) VALUES(?,?,?,?,?)",
                (999, "invalid.bool", 2, 0, 1),
                "ProgressionCovenantDefinition.ExclusiveMembership",
            ),
            (
                "UPDATE ServerDB__ProgressionStoryDefinition SET StartMode=? WHERE StoryId=?",
                (9, 201),
                "ProgressionStoryDefinition.StartMode",
            ),
            (
                "INSERT INTO ServerDB__ProgressionStoryStartCondition"
                "(StoryId,SortOrder,ConditionType,Key,Id,Value,AuxiliaryId) VALUES(?,?,?,?,?,?,?)",
                (201, 99, 2, "", 0, 9, 0),
                "ProgressionStoryStartCondition.ClassIs frozen",
            ),
            (
                "INSERT INTO ServerDB__ProgressionStoryStartCondition"
                "(StoryId,SortOrder,ConditionType,Key,Id,Value,AuxiliaryId) VALUES(?,?,?,?,?,?,?)",
                (201, 100, 11, "", 0, 0, 0),
                "ProgressionStoryStartCondition.HasWorldStoryFlag key",
            ),
            (
                "INSERT INTO ServerDB__ProgressionStoryStartCondition"
                "(StoryId,SortOrder,ConditionType,Key,Id,Value,AuxiliaryId) VALUES(?,?,?,?,?,?,?)",
                (201, 101, 12, "", 0, 0, 0),
                "ProgressionStoryStartCondition.ReputationAtLeast key",
            ),
            (
                "INSERT INTO ServerDB__ProgressionStoryNodeReward"
                "(StoryId,NodeId,SortOrder,RewardType,Key,Id,Amount) VALUES(?,?,?,?,?,?,?)",
                (201, 10, 91, 3, "", 1, 10),
                "ProgressionStoryNodeReward.GenericReputation contract",
            ),
            (
                "INSERT INTO ServerDB__ProgressionStoryNodeReward"
                "(StoryId,NodeId,SortOrder,RewardType,Key,Id,Amount) VALUES(?,?,?,?,?,?,?)",
                (201, 10, 92, 0, "ITTEST", 0, 2),
                "ProgressionStoryNodeReward.Item amount must be one",
            ),
            (
                "INSERT INTO ServerDB__ProgressionStoryNodeCompletionCondition"
                "(StoryId,NodeId,SortOrder,ConditionType,Key,Id,Value,AuxiliaryId) VALUES(?,?,?,?,?,?,?,?)",
                (201, 10, 94, 5, "", 9001, 3, 0),
                "ProgressionStoryNodeCompletionCondition.QuestState range",
            ),
            (
                "INSERT INTO ServerDB__ProgressionStoryNodeCompletionAction"
                "(StoryId,NodeId,SortOrder,ActionType,Key,Id,Value) VALUES(?,?,?,?,?,?,?)",
                (201, 10, 95, 3, "", 101, 0),
                "ProgressionStoryNodeCompletionAction.Covenant reputation no-op",
            ),
            (
                "INSERT INTO ServerDB__ProgressionStoryNodeReward"
                "(StoryId,NodeId,SortOrder,RewardType,Key,Id,Amount) VALUES(?,?,?,?,?,?,?)",
                (201, 10, 96, 5, "", 101, 0),
                "ProgressionStoryNodeReward.Covenant reputation no-op",
            ),
            (
                "INSERT INTO ServerDB__ProgressionCovenantReward"
                "(CovenantId,RewardKey,RankId,SortOrder,RewardType,Key,Id,Amount) VALUES(?,?,?,?,?,?,?,?)",
                (101, "invalid.generic.rep", 0, 91, 3, "", 1, 10),
                "ProgressionCovenantReward.GenericReputation contract",
            ),
            (
                "INSERT INTO ServerDB__ProgressionCovenantReward"
                "(CovenantId,RewardKey,RankId,SortOrder,RewardType,Key,Id,Amount) VALUES(?,?,?,?,?,?,?,?)",
                (101, "invalid.item.amount", 0, 92, 0, "ITTEST", 0, 2),
                "ProgressionCovenantReward.Item amount must be one",
            ),
            (
                "INSERT INTO ServerDB__ProgressionCovenantReward"
                "(CovenantId,RewardKey,RankId,SortOrder,RewardType,Key,Id,Amount) VALUES(?,?,?,?,?,?,?,?)",
                (101, "x" * 201, 0, 93, 1, "", 0, 1),
                "ProgressionCovenantReward.RewardKey source-key boundary",
            ),
            (
                "INSERT INTO ServerDB__CovenantEffectReceipt"
                "(PlayerKey,CovenantId,EffectKey,Applied) VALUES(?,?,?,?)",
                ("invalid", 101, "bad.applied", 2),
                "CovenantEffectReceipt.Applied",
            ),
            (
                "INSERT INTO ServerDB__ProgressionStoryDialogueChoice"
                "(StoryId,NodeId,SortOrder,ChoiceKey,DisplayKey,ToNodeId) VALUES(?,?,?,?,?,?)",
                (201, 10, 98, "", "dialogue.invalid", 20),
                "ProgressionStoryDialogueChoice requires key",
            ),
            (
                "INSERT INTO ServerDB__ProgressionStoryDialogueChoice"
                "(StoryId,NodeId,SortOrder,ChoiceKey,DisplayKey,ToNodeId) VALUES(?,?,?,?,?,?)",
                (201, 10, 97, "dialogue.invalid.display", "", 20),
                "ProgressionStoryDialogueChoice requires display key",
            ),
            (
                "INSERT INTO ServerDB__ProgressionStoryDialogueChoiceCondition"
                "(StoryId,NodeId,ChoiceKey,SortOrder,ConditionType,Key,Id,Value,AuxiliaryId) VALUES(?,?,?,?,?,?,?,?,?)",
                (201, 10, "dialogue.validator.left", 99, 2, "", 0, 1, 0),
                "ProgressionStoryDialogueChoiceCondition.ClassIs frozen",
            ),
            (
                "INSERT INTO ServerDB__ProgressionStoryNodeCompletionAction"
                "(StoryId,NodeId,SortOrder,ActionType,Key,Id,Value) VALUES(?,?,?,?,?,?,?)",
                (201, 10, 93, 5, "", 0, 0),
                "ProgressionStoryNodeCompletionAction.Custom requires key",
            ),
        ]:
            try:
                db.execute(sql, args)
            except sqlite3.IntegrityError:
                pass
            else:
                fail(f"{label} CHECK constraint did not fire")
    finally:
        db.close()

    print("progression migration validation: PASS")


if __name__ == "__main__":
    main()
