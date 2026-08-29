# Pikeman High-LOD TwoSide Witness Gate V1

Fixes the previous observational problem:

- C002 DefaultModel uses TbmC03
- C002 HighModel uses TbhC03
- the previous geometry opening existed only in TbmC03
- moving close enough to inspect the hole switched to untouched TbhC03

This gate keeps the already-approved widened TbmC03 artifact as its exact base
and also widens TbhC03 by 20% in X/Y.

It produces two variants:

A:
- TbmC03 widened
- TbhC03 widened
- material 0 TwoSide=0

B:
- exact same geometry bytes as A
- material 0 TwoSide=1

A vs B differs by exactly one byte.

This allows close-camera/high-LOD inspection without losing the opening.
