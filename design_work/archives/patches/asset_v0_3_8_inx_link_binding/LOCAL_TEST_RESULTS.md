# V0.3.8 local test results

## Build

- `python3 -m compileall pt_asset_tool`: PASS.
- versão: `0.3.8`.

## Evidência runtime INX — Cmb002

- `Cmb002.inx`:
  - size = 95268 / `MODERN_172`;
  - LinkFile = `char\\tmABCD\\M2Bip.in`;
  - LinkCode = `0xF915414B`.
- `SpeedSum("char\\tmABCD\\M2Bip.inx") = 0xF915414B`.
- `M2Bip.inx FileCode = 0xF915414B`.
- relação source -> target: VERIFIED.
- pairing target: `M2Bip.inx -> tfbD01.smd + m2.smb`.

## Registry target

`CmfbB02.smd`:

- `binding_mode=ROOT_SKELETON_BOUND`;
- `binding_source=INX_LINK_FILE`;
- `binding_skeleton_smb=char/tmabcd/m2.smb`;
- `binding_root_models=[char/tmabcd/tfbd01.smd]`;
- source config `Cmb002.inx`;
- motion/root config `M2Bip.inx`;
- `rigid_binding_ready=true`;
- `animation_binding_ready=true`.

`bindings --discover CmfbB02.smd` retorna imediatamente
`INX_LINK_FILE_BOUND`, sem executar o discovery 958-SMB antigo.

## INX + SMB gate

`M2Bip.inx + m2.smb`:

- layout `MODERN_172`;
- MotionCount = 78;
- keyword failures = 0;
- logical path FileCode match = true;
- SMB validation = PASS;
- `inx_motion_core_check=PASS`;
- `client_modified=false`.

## Structural gate — target

`CmfbB02.smd + m2.smb`:

- objects = 1;
- bind vertices = 880;
- bind bones used = 29;
- skeleton bones = 67;
- missing bind bones = [];
- `character_structural_gate_check=PASS`;
- `client_modified=false`.

O CLI completo `validate structural char/tmABCD/CmfbB02.smd` também retornou
`check=PASS`, `binding_source=INX_LINK_FILE` e `inx_checks=1`.

## Cobertura tmABCD

Baseline V0.3.7 — 1393 rigid Physique SMDs em tmABCD:

- CONFIG_ONLY = 1375;
- SELF_CONTAINED = 7;
- NONE = 11.

V0.3.8:

- ROOT_SKELETON_BOUND / INX_LINK_FILE = 1374;
- CONFIG_ONLY / INX_LINK_FILE_AMBIGUOUS = 1;
- SELF_CONTAINED / DIRECT_CONFIG_MOTION = 7;
- NONE = 11.

Para as 1476 linhas de pairing dos 1375 rigid config-only originais:

- VERIFIED = 1476 / 1476.

Targets:

- M1Bip = 480 rows;
- M2Bip = 314;
- M3Bip = 158;
- M4Bip = 160;
- M5Bip = 157;
- M6Bip = 104;
- M7Bip = 103.

## Sete roots — structural representatives

Todos PASS sem missing bind bones:

- M1: `ar0nHLL.smd + m1.smb`;
- M2: `ar0nASH.smd + m2.smb`;
- M3: `CmmbD02.smd + m3.smb`;
- M4: `CtmbC006.smd + m4.smb`;
- M5: `ar0nDressP.smd + m5.smb`;
- M6: `CtfbE01.smd + m6.smb`;
- M7: `CmmbE01.smd + m7.smb`.

## Fail-closed real

`char/tmABCD/ar0nDrac.smd`:

- five config rows have individually VERIFIED INX links;
- they converge to three distinct root INXs/motions;
- result: `CONFIG_ONLY / INX_LINK_FILE_AMBIGUOUS`;
- `animation_binding_ready=false`.

No arbitrary winner is selected.

## Regression

Registry state remains unchanged for previously proven self-contained assets:

- Monbagon: `SELF_CONTAINED / DIRECT_CONFIG_MOTION`;
- Ahin: `SELF_CONTAINED / DIRECT_CONFIG_MOTION`;
- Acero: `SELF_CONTAINED / DIRECT_CONFIG_MOTION`.

Non-rigid sample regression:

- `CartolaMG.smd`: remains `SMD_OTHER / CONFIG_ONLY / UNRESOLVED_CONFIG_ONLY`;
- `CartolaPS.smd`: remains `SMD_OTHER / CONFIG_ONLY / UNRESOLVED_CONFIG_ONLY`.

## Source integrity

SHA-256 of the test-client copies remained byte-identical to the uploaded source
package after all tests for:

- Cmb002.inx;
- M2Bip.inx;
- CmfbB02.smd;
- m2.smb.

## Blender

The execution container does not have Blender/Flatpak installed, so no Blender
visual gate was fabricated. The runtime-link, INX/SMB and structural gates above
were executed locally. Human Blender art-assembly certification remains a
separate visual gate after installation.

## Negative parser checks

- source LinkFile modified from `M2Bip` to `M3Bip` without updating LinkCode:
  `LINK_CODE_MISMATCH` (blocked).
- source INX truncated by one byte: unsupported layout/size error (blocked; no weak
  fallback is permitted for rigid config-only when physical INX evidence cannot
  be parsed).
- clean installer test: PASS; backup created and installed version reported
  `0.3.8`.
