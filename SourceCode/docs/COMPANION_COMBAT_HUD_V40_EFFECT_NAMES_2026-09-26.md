# Companion Combat HUD V40 — Canonical effect names

V40 improves buff visibility without changing the runtime protocol.

The server already publishes positive runtime effects as `effectCode + remainingMs`. The client now resolves those codes through a stable ASCII companion-skill name table generated from the same 128-entry T1–T4 catalog used by the server. This deliberately avoids `sSkillName` as display text because legacy DAT/ANSI names have produced mojibake in prior Companion UI work.

The party row shows the first active positive effect with remaining seconds and `+N` when more effects are active. Existing status ailments, AGGRO, cooldown and cast progress remain on the same compact row.

No fields are added to `CompanionRuntimeStatePacket`; the `<1024` packet contract remains unchanged.
