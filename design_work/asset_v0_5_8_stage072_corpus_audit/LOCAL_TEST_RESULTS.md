# V0.5.8 local test results

Environment available to this build does not contain the 63 physical Stage0.72 client files, so the real corpus certificate must run on the user's client.

Local deterministic tests performed:

1. Python compileall of the complete package: PASS.
2. CLI version and `audit stage072-corpus --help`: PASS.
3. Synthetic source-layout Stage0.72, single texture stage:
   - raw smSTAGE3D header parse: PASS
   - exact hTexture material-slot mapping: PASS
   - TexLink pointer rebasing: PASS
   - StageArea WORD16 payload closure: PASS
4. Synthetic no-TVertex / zero-texture placeholder:
   - hTexture=0 single TexLink classified as textureless placeholder: PASS
   - remains fail-closed for non-placeholder stage overruns.
5. Synthetic two-stage + custom vertex colors:
   - depth=2: PASS
   - custom vertex colors detected: PASS
   - exact hTexture stage 0/stage 1 mapping: PASS
6. Three-file synthetic corpus audit + representative ZIP generation: PASS.

No original client file is modified by this audit.
