# Local test results — V0.3.3

## Compile

`python3 -m compileall -q pt_asset_tool`: PASS.

## Gate-first unique skeleton fixture

Fixture com quatro SMBs:

- `root.smb`: structural PASS, skeleton Root;
- `root_attack.smb`: structural PASS, bytes/payload diferente, mesmo skeleton Root;
- `wrong.smb`: structural FAIL;
- `zzz_malformed.smb`: arquivo que o evaluator não consegue parsear, structural FAIL.

Resultado:

- candidate_universe=4
- tested_candidates=4
- structural_passes=2
- distinct_signed_skeletons=1
- unsigned_structural_passes=0
- status=UNIQUE_SKELETON
- os dois SMBs PASS foram agrupados na mesma skeleton_signature

Isto prova que SMB irrelevante não parseável não reduz mais o universo antes do
gate.

## Fail-closed para PASS sem assinatura

Fixture adversarial em que o structural gate foi configurado para aceitar o SMB
malformado também.

Resultado:

- structural_passes=4
- unsigned_structural_passes=1
- status=PASS_SIGNATURE_INCOMPLETE
- exit code=5
- erro de parser incluído em `pass_signature_error_counts/samples`

Isto prova que um PASS real não parseável nunca é descartado como se não fosse
candidato.

## Registry absorption

Depois de UNIQUE_SKELETON, `asset inspect` no fixture mostrou:

- binding_mode=ROOT_SKELETON_BOUND
- binding_source=GATE_FIRST_SKELETON_UNIQUE
- rigid_binding_ready=true
- visual_mode=RIGID

PASS.
