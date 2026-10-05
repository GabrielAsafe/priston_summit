# Quest closure data contract V1

`QUEST_SOURCE_READINESS=PASS`
`QUEST_RUNTIME_CLOSURE=BLOCKED_MISSING_QUESTDB_SNAPSHOT`

The canonical pack contains 88 Quest Codex entries. The current verified source
bindings cover 13 entries; 75 entries still require runtime evidence. The tracked
`runtime_binding_tasks.tsv` is the executable queue for those 75 mappings.

## Required external evidence

Export the real rows from the **same QuestDB used by the target GameServer** for:

- `RepeatedQuest`, `DailyQuest`, `SingleQuest` — exact definition/conditions;
- `QuestMonsters` — DB-backed kill objectives;
- `Rewards`, `ItemRewards` — DB-backed reward authority.

`Database/exports/questdb_runtime_snapshot_v1.sql` is the source-controlled query
contract. Preserve raw IDs, `QuestType`, levels, party flags, names/descriptions,
objectives and rewards. The normalized JSON must use schema
`summit.questdb-runtime-snapshot/v1` and pass
`tools/quest_port/validate_questdb_runtime_snapshot_v1.py`.

## Mapping rule

A Codex guide can become `supported=true` only after one of these proofs exists:

1. exact QuestDB identity and semantics are reconciled (definition + relevant
   objective/reward rows); or
2. an existing non-QuestDB runtime handler/tag is shown to implement the same
   quest semantics.

Names, level bands, NPC text, or apparent numeric patterns are candidate hints,
not proof. Never invent `QuestID`, kind/type, objectives or rewards to increase
coverage.

## Closure after snapshot arrival

For each row in `runtime_binding_tasks.tsv`, record the proven runtime IDs/tags,
validate objectives/conditions/rewards and persistence behavior, then update
`runtime_bindings.json`. Re-run Codex/UI, repository/lifecycle/progress/objective,
admin and progression portable gates. Tier/reconnect/restart remain runtime E2E
checks even after the data mapping is complete.
