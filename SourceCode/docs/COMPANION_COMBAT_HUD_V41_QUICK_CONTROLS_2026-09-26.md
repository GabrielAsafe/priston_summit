# Companion Combat HUD V41 — Quick controls

V41 closes two compact party-HUD controls without adding another window.

Per active companion row:

- Left click: existing mode cycle.
- Middle click: existing `OwnerFocus` tactic (focus the owner's selected target).
- Right click: existing dismiss.
- **Shift + Left**: cycle active skill slot 1 forward.
- **Shift + Middle**: cycle support/recast slot 1 forward.

The actions use the same server-authoritative UI protocol and persisted loadout path as the full Companions page; the HUD only sends existing `CycleActiveSkill` / `CycleBuffSkill` actions.
