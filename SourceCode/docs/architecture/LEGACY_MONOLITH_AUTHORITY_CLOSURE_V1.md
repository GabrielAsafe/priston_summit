# Legacy monolith authority closure V1

`AUTHORITY_RESIDUAL=ZERO_STATIC`

Scope: server-side `Shared/server/legacy_runtime/OnSever_legacy_server.inl`,
`SrcServer/src/systems/gameplay/character/core/character.cpp`, and the adjacent
legacy persistence/network shells they invoke.

The closure criterion is **not** that these large legacy files disappear. It is
that high-authority gameplay state no longer has an unclassified mutation/read
path outside the corresponding service/adapter boundary. Remaining state in the
monolith is shell state, construction/hydration, timers, presentation, or packet
translation and must stay under exact static budgets where applicable.

## Authority families closed by executable guards

- **Life / lethal / healing** — gameplay mutations are owned by combat/life
  adapters; exact construction/hydration writes are budgeted by
  `validate_server_life_mutation_ownership_v1.py`.
- **Mana / Stamina** — gameplay resource mutation is owned by the resource
  boundary; shell writes are exact-budgeted by
  `validate_server_resource_mutation_ownership_v1.py`.
- **Status effects** — stun/slow/poison/HolyMind gameplay mutation is owned by
  `LegacyStatusEffectAdapter`; timer/presentation state is exact-budgeted by
  `validate_server_status_effect_mutation_ownership_v1.py`.
- **Inventory** — direct add/delete kernels are confined to authorized legacy
  kernels/adapters by `validate_inventory_mutation_ownership_v3.py`.
- **Experience** — direct `rsPLAYINFO::AddExp` consumers are forbidden outside
  `LegacyPlayerExperienceAdapter` by `validate_player_experience_ownership_v1.py`.
- **Wallet / SCoins / TCoin** — money, SCoins and time-credit query/mutation
  authority is centralized by the wallet ownership guards.
- **Quest reads** — OnServer spoiler/completion checks and character-record quest
  reads cross the Quest repository/query adapters rather than calling the legacy
  handler directly.
- **Target selection** — `smCHAR::Main` player-target clears are delegated and
  guarded by `validate_character_main_targeting_authority_v2.py`.
- **Transport / packet routing** — direct socket ownership in production gameplay
  code is retired/guarded by the OnServer/character transport and RED socket
  ownership validators.

## Shell legítimo

The following categories are intentionally not migrated merely to make the
monolith smaller: constructor/reset values, spawn/hydration state, frame/timer
countdowns, presentation-only distortion/display values, DTO/packet translation,
and legacy orchestration that delegates the authoritative decision to a service.
Where these touch high-value state, the focused validators use exact source-line
budgets so a new write cannot be added silently.

## What this closure does not claim

This is a source/static closure. It does not claim native MinGW i686 build/link,
QuestDB production data parity, visual UI/effect parity, or in-game runtime/E2E.
Those are separate gates and remain independently reportable blockers until they
can be executed in the required environment.
