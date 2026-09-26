# Companion Advanced AI V38 — Healer positioning

V38 closes the movement half of Priestess healing AI without changing Healing's authoritative executor.

## Existing behavior

`TryExecuteCompanionHeal()` already selects the lowest-health living owner/companion below 70% HP inside the legacy 320-unit heal range. If every injured ally was outside that range, the attempt returned `NO_TARGET` and normal offensive AI resumed immediately.

## V38 behavior

When all of the following are true:

- the companion is a Priestess;
- Healing is actually learned in the persisted T1 slot;
- the heal attempt failed specifically with `NO_TARGET` (not cooldown/resource/lock);
- a living owner or companion below 70% HP exists between 320 and 600 world units;

…the Priestess spends the AI tick moving toward the lowest-health eligible recipient (nearest breaks equal-HP ties). It disengages its offensive target, refreshes the ally movement anchor every AI tick and uses RUN with WALK fallback. Once the ally enters 320 units, the unchanged Healing executor selects and heals it normally.

Passive mode remains non-casting and therefore does not reposition for healing. Follow mode keeps its existing support permission.
