# PT Asset Tool V0.3.1 — Structural Root Skeleton Discovery

## Por que a V0.3.0 não resolveu o cliente real

A auditoria real mostrou 1375 assets rigid `CONFIG_ONLY` e zero `ROOT_SKELETON_BOUND`.
Logo, a hipótese "o root self-contained aparece como outra model_smd no mesmo INX" não descreve esses dados.

## Nova estratégia

V0.3.1 mantém a regra V0.3.0 como primeira tentativa, mas adiciona descoberta explícita por compatibilidade estrutural:

    python3 pt_asset.py bindings --discover <asset.smd>

O comando:

1. resolve fisicamente o SMD config-only;
2. enumera todos os `.smb` canônicos do inventário, não apenas os já pareados a modelos;
3. executa `pt_character_structural_gate_v2.py --smd ... --smb ...` para cada candidato;
4. agrupa aliases byte-identical pelo SHA-256 do SMB;
5. só escreve binding quando exatamente um conteúdo SMB distinto passa e todo o universo foi verificado;
6. grava o resultado em `logs/asset_family_refine_v2/root_skeleton_bindings_v1.json`;
7. o AssetRegistry lê esse cache e promove o asset para `ROOT_SKELETON_BOUND` com `binding_source=STRUCTURAL_GATE_UNIQUE`.

Se dois conteúdos SMB distintos passam, o scan para cedo por padrão e registra:

    AMBIGUOUS_STRUCTURAL_MATCH

sem promover o asset.

## Separação skeleton x motion INX

Compatibilidade SMD↔SMB prova o skeleton, mas não prova por si só qual INX deve fornecer MotionInfo.
Por isso V0.3.1 separa os dois conceitos:

- geometria/rig estrutural pode usar o SMB descoberto;
- animação usa `motion_inx` somente quando o pairing fornece um único root INX associado ao SMB;
- sem root INX único, `asset motions` / `validate animation` recusam com erro explícito em vez de usar o INX da peça por suposição.

## Resume

O scan grava estado incremental em:

    pt_build/pt_asset_tool/<ASSET_ID>/root_skeleton_discovery/probe_state.json

Reexecutar o mesmo comando continua de onde parou. Use `--no-resume` para descartar o estado.

## Segurança

- Não modifica SMD, SMB, INX ou cliente.
- Não usa basename/diretório para decidir compatibilidade.
- Dois skeletons fisicamente diferentes compatíveis => ambiguidade, fail-closed.
- Aliases byte-identical contam como um único conteúdo SMB.
- Structural gate normal continua obrigatório depois do binding.
