# Pike Geometry + TwoSide A/B Gate V1

A (already observed in runtime):
- approved TbmC03 widened geometry
- Material 0 TwoSide=0
- Material 1 TwoSide=0

B:
- exact same approved geometry bytes
- Material 0 TwoSide=1
- Material 1 TwoSide=0

The builder requires the exact approved geometry artifact SHA:

40b02371a872745d98abc558042b62f75cfb001a297e1bc36582e6c5968495a5

and changes exactly one byte in material 0 TwoSide.

Purpose:
compare the same hole/joint opening from the same camera angle. The only
runtime variable is TwoSide on material 0 (TmbC03).
