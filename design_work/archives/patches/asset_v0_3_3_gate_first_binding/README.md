# PT Asset Tool V0.3.3 — Gate-First Root Skeleton Binding

## Motivo

O teste real da V0.3.2 em `char/tmABCD/CmfbB02.smd` mostrou:

- 958 arquivos SMB físicos;
- apenas 3 parseados pelo evaluator legado;
- 955 falhas de parse;
- 1 skeleton group construído;
- `NO_MATCH` após testar somente esse universo reduzido.

Isso tornava o `NO_MATCH` inválido como evidência: a etapa de assinatura estava
eliminando candidatos antes do structural gate.

## Correção conceitual

A ordem agora é:

1. Enumerar o universo físico completo de SMBs.
2. Rodar `pt_character_structural_gate_v2.py` contra o SMD config-only.
3. Ignorar SMBs que falham estruturalmente, independentemente de o evaluator
   conseguir ou não parseá-los.
4. Somente para SMBs que passam estruturalmente, tentar parsear o skeleton e
   calcular `topology_signature` + `skeleton_signature`.
5. Dois SMBs PASS com payloads diferentes mas a mesma assinatura de skeleton
   contam como um único candidato.
6. Um PASS estrutural cujo skeleton não pode ser parseado NÃO vira `NO_MATCH`:
   retorna `PASS_SIGNATURE_INCOMPLETE` e permanece fail-closed.
7. Um único skeleton assinado após scan completo => `UNIQUE_SKELETON`.
8. Dois skeletons assinados diferentes => `AMBIGUOUS_SKELETON_MATCH`.

Assim, cobertura incompleta do parser de animação não pode mais produzir uma
conclusão negativa falsa sobre root binding.

## Cache novo

A V0.3.3 não reutiliza a decisão da V0.3.2:

    logs/asset_family_refine_v2/root_skeleton_bindings_v3.json

O estado retomável por asset fica em:

    pt_build/pt_asset_tool/<ASSET_ID>/root_skeleton_discovery/probe_state_v3.json

Relatórios do structural gate ficam em `reports_v3/`.

## Estados

### UNIQUE_SKELETON

Todos os candidatos foram testados; existe exatamente uma assinatura de
skeleton entre todos os structural PASSes; nenhum PASS ficou sem assinatura.
Pode promover para `ROOT_SKELETON_BOUND`.

### AMBIGUOUS_SKELETON_MATCH

Dois skeletons assinados realmente diferentes passaram. Fail-closed.

### PASS_SIGNATURE_INCOMPLETE

Existe pelo menos um SMB que passou o structural gate, mas o evaluator atual
não conseguiu extrair sua identidade de skeleton. Fail-closed. O JSON inclui:

- `pass_signature_error_counts`
- `pass_signature_error_samples`
- `unsigned_structural_passes`

### NO_MATCH

Somente pode ocorrer depois de o structural gate testar o universo completo e
nenhum SMB passar. Falhas de parse de SMBs que já falharam estruturalmente não
participam desta decisão.

## Segurança

- Não modifica cliente, SMD, SMB ou INX.
- Não escolhe root por filename/diretório.
- Structural gate continua obrigatório.
- Parser de animação não é usado como pré-filtro.
- PASS sem identidade legível bloqueia em vez de ser descartado.
- Bridge Blender V0.2.5 e guard Monbagon STAND V0.2.9 permanecem inalterados.

## Uso

    cd "/home/gabriel/Área de Trabalho/summit/design_work"
    python3 pt_asset.py bindings --discover char/tmABCD/CmfbB02.smd

O comando é retomável. Para ignorar o estado V3:

    python3 pt_asset.py bindings --discover char/tmABCD/CmfbB02.smd --no-resume

`--full-scan` força o scan a continuar mesmo depois de duas assinaturas
compatíveis distintas já provarem ambiguidade.
