# Covenant V1 — chat-first runtime contract

The authoritative Covenant content is seeded by `040_covenant_default_content.sql`.
Runtime state/content is extended by `041_covenant_runtime_complete.sql` and the
Covenant-only inventory by `042_covenant_inventory.sql`, and persistent Artisan
crafting mode by `043_covenant_artisan_mode.sql`.

This release intentionally has **no NPC/altar/model dependency**. Chat is the
first player surface; a later UI and world entity must call the same server APIs.

## Membership and activation

- A player may keep durable membership/progress in all five Covenants.
- At most one Covenant is active at a time.
- The first joined Covenant activates automatically.
- Joining another Covenant does not switch the current one.
- `CovenantService::Activate()` switches while preserving rank/reputation.
- Switch cooldown is persisted (`SUMMIT_COVENANT_SWITCH_COOLDOWN_SECONDS`,
  default 300 seconds).
- The legacy chat boundary additionally blocks switching while dead, during
  recent combat or during the recent-trade window.
- Switching cancels the previous Covenant's currently-active Tier III power but
  preserves that power's cooldown.
- Membership/rank/reputation conditions can inspect inactive preserved progress.
- Gameplay modifiers are emitted only by the active Covenant.

## Sigils and Covenant inventory

Sigils **do not activate Covenant effects**. A first join grants a collectible
and a Covenant-inventory stack (`sigil:<covenant>`). Migration 042 backfills
Sigils for pre-existing memberships and is idempotent.

The Covenant inventory is persistent and currently also stores:

- Hunter trophies (`trophy:<monster-code>`),
- refined Hunter trophies (`refined_trophy:<monster-code>`),
- Gatherer special materials (`special_material:<resource-key>`),
- Gatherer refined materials (`refined_material:<resource-key>`).

It is intentionally independent of legacy item IDs so content can later map
these stacks to real item assets/recipes without changing Covenant progression.

## Rank content — Option A

Ranks are cumulative **in content**. Tier II/III repeat every semantic modifier
from the prior tier with the value intended at that tier. `ValidateDefinitions()`
rejects a rank that silently drops a prior stat/scope/operation key.

Default thresholds:

- Tier I: 0 reputation
- Tier II: 1000 reputation
- Tier III: 3000 reputation

## Repeatable activity, Challenges and Mastery

Activity reputation is server-authoritative and intentionally does not create a
ledger receipt per kill/harvest/craft. Challenge rewards remain idempotent.

Default activities:

- Hunters: `hunt.kill`
- Gatherers: `gathering.harvest`
- Artisans: `crafting.success`
- Fortune: `monster.kill`
- Ascendants: `monster.kill`

Each Covenant has three persistent challenges. Covenant Mastery is granted only
when the player has reached Tier III and completed all configured challenges.
Mastery is permanent and independent of the currently-active Covenant.

## Tier III powers

The power definitions, durations, cooldowns and effect modifiers are data-driven
in migration 041:

- Hunters — `hunters_mark` (targeted Hunt/boss target)
- Fortune — `tempt_fate`
- Ascendants — `focused_training`

Power active-state and cooldown survive restart. Hunter's Mark modifiers are
applied only when the gameplay callsite supplies the marked target serial.

## Player chat surface

Supported commands:

- `/covenant list`
- `/covenant info <hunters|gatherers|artisans|fortune|ascendants>`
- `/covenant join <name>`
- `/covenant activate <name>`
- `/covenant status`
- `/covenant challenges [name]`
- `/covenant mastery`
- `/covenant sigils`
- `/covenant inventory`
- `/covenant refine <trophy:key>`
- `/covenant power`
- `/covenant survey`
- `/covenant artisan [off|reforge|forgemaster|master]`

Admin/testing:

- `/covenant rep <name> <amount> <unique-source-key>`

`CovenantChatCommand` is transport-independent. The legacy `RecvCommand()` only
builds the player context, calls the handler and writes response lines back to
the issuing player.

## Live gameplay hooks

Covenant balance uses semantic modifier keys rather than Covenant-ID branches in
hot paths.

Implemented authoritative hooks:

- solo monster XP — `xp.monster_gain`
- party monster XP — `xp.monster_gain`
- normal monster drops — `monster.drop_chance`
- Hunt/boss drops — `hunt.drop_chance`
- Fortune extra roll — `loot.extra_roll_chance`
- incoming monster damage — `combat.damage_taken`
- boss/Hunt kill activity and trophies
- Mining/Gathering yield and extra yield
- Gatherer special/refined Covenant materials
- manufacture success activity
- Artisan blacksmith sale value
- Artisan Reforge: one class-specialization property using one `special_material:*`
- Artisan Forgemaster: class-specialized full reroll using one `refined_trophy:*`
- Artisan Master: perfect class-specialized output using one `refined_material:*`

Mining consumes Covenant effects through `ICovenantGameplayBridge`, keeping the
domain service independently testable and preventing `ProgressionRuntime`/SQLite
from leaking into the Mining domain.

## Presentation/content extension points

The seeded Covenant gameplay modifiers are all delivered by an authoritative runtime hook.
Remaining work is presentation/content, not missing Covenant mechanics:

- Gatherer Surveyor is live through `/covenant survey` over registered Mining nodes; world-content population and future map visualization remain external presentation/content work.
Artisan enhancement deliberately reuses the legacy-authoritative `CreateDefItem` /
`CreateDefItemPerf` builders and final `ReformItem` checksum path instead of inventing
a second item mutation format. The player selects a persistent mode explicitly; the
server consumes the matching Covenant material only when the selected transformation
can be applied.

Ascendant Tier II intentionally strengthens the existing server-authoritative XP/drop tradeoff instead of mutating legacy skill mastery, whose T1–T4 persistence remains client/record-driven. Tier III `Focused Training` supplies the active progression specialization.

## UI V2 Covenant surface

Covenant V1.1 adds a structured client/server UI surface. It does **not** send
slash commands through chat. The client sends `CovenantUIActionPacket` and the
server responds with an authoritative `CovenantUIStatePacket` containing the
five Covenants, membership/active state, reputation/rank, current cumulative
benefits, challenges, Mastery, Covenant inventory, Surveyor data, Artisan mode
and Tier III power/cooldown state.

Player access:

- open Character UI and select the **Covenants** tab; or
- press **N** to open/toggle the Character UI directly on Covenants.

Available UI actions:

- join a Covenant;
- activate/switch an already joined Covenant;
- use Hunter's Mark / Tempt Fate / Focused Training;
- cycle the persistent Artisan mode allowed by the current rank;
- refine an available Hunter trophy;
- inspect rank thresholds/effects, challenges, Mastery, Sigils/inventory and
  Gatherer Surveyor entries.

The legacy `/covenant ...` commands remain available as a fallback/admin/debug
surface. NPCs/altars can later open the same V2 Covenant tab without changing
Covenant business rules or persistence.
