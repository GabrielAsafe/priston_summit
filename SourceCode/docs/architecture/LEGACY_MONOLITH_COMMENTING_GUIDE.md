# Legacy Monolith Commenting Guide

This pass documents active RED monoliths without moving or rewriting code.

Each section uses four fields:

- **AREA**: navigation/obligation label.
- **TIPO**: subsystem category (network, audio/presentation, inventory, combat, etc.).
- **FUNCIONAMENTO**: observed behavior of the block/function, not guessed intent.
- **FRONTEIRA**: where modern development should enter instead of growing the monolith.

## Safety rule

The annotation commit must pass `tools/architecture/validate_comment_only_change.py` for every annotated legacy source. This proves the C/C++ token stream is unchanged. Oracle/reference copies are intentionally not annotated.

## Scope of this pass

The active server/client OnServer, netplay, character, inventory, item tooltip, HoEffect, trade/craft/aging, skill UI/state and Assa particle monoliths receive navigation maps and area headers. The goal is readability before native certification; functional extraction can happen later behind the existing GREEN/AMBER boundaries.
