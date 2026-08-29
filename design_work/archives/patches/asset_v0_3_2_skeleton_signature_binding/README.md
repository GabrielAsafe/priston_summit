# PT Asset Tool V0.3.2 — Skeleton Signature Binding

## Motivo

V0.3.1 tratava dois SMBs de SHA diferente como dois skeletons distintos.
O cliente real mostrou que isso é insuficiente: `worldtogo.smb` e
`worldtogo_attack.smb` passaram o structural gate para a mesma peça. SMB é
container de skeleton + animação, portanto dois arquivos podem ter payload de
motion diferente e carregar exatamente o mesmo hierarchy/bind skeleton.

## Correção

`bindings --discover` agora:

1. enumera os SMBs físicos do inventário;
2. parseia o skeleton interno de cada SMB;
3. calcula duas assinaturas independentes:
   - topology signature: bone names + parent hierarchy + tm_sort;
   - skeleton signature: topology + bind matrices normalizadas;
4. agrupa todos os SMBs que carregam o mesmo skeleton;
5. executa `pt_character_structural_gate_v2.py` apenas uma vez por skeleton
   distinto, usando um SMB representante;
6. vários SMBs diferentes do mesmo skeleton contam como UM candidato;
7. somente um skeleton signature compatível depois do universo completo =>
   `UNIQUE_SKELETON` / `ROOT_SKELETON_BOUND`;
8. dois skeleton signatures realmente diferentes compatíveis =>
   `AMBIGUOUS_SKELETON_MATCH` e fail-closed.

Animation payload, header frames e SHA do arquivo SMB não participam da
identidade do skeleton.

## Caches novos

V0.3.2 não reutiliza a decisão V0.3.1:

    logs/asset_family_refine_v2/root_skeleton_bindings_v2.json

Assinaturas SMB são cacheadas separadamente:

    logs/asset_family_refine_v2/smb_skeleton_signatures_v1.json

O probe por asset usa:

    pt_build/pt_asset_tool/<ASSET_ID>/root_skeleton_discovery/probe_state_v2.json

Assim `AMBIGUOUS_STRUCTURAL_MATCH` gravado pela V0.3.1 não contamina o scan V2.

## Skeleton binding != motion binding

Mesmo depois de `UNIQUE_SKELETON`, `asset motions` e `validate animation` só
usam MotionInfo se o pairing permitir resolver um root INX único. Um skeleton
único com vários INXs continua válido para rig/geometry, mas animação permanece
bloqueada até o INX correto ser provado.

## Segurança

- Não modifica cliente, SMD, SMB ou INX.
- Não escolhe root por basename/diretório.
- Não considera SHA do SMB como identidade de skeleton.
- Não aceita duas assinaturas de skeleton diferentes.
- Structural gate continua obrigatório.
- O bridge Blender e o guard Monbagon STAND da V0.2.9 permanecem inalterados.

## Comando principal

    python3 pt_asset.py bindings --discover char/tmABCD/CmfbB02.smd

O primeiro run constrói o índice de assinaturas dos SMBs. Runs seguintes
reutilizam esse índice enquanto size/mtime do arquivo físico não mudarem.
