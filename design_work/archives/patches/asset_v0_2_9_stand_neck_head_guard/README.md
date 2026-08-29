# PT Asset Tool V0.2.9 — Monbagon STAND Neck/Head Guard

## Motivo

A V0.2.8 testou a hipótese de que o frame 42 era um anchor/reset de container para a STAND.
O teste visual falsificou essa hipótese: a deformação piorou e passou a atingir também o ombro esquerdo.
Embora SOURCE_DIRECT e BLENDER_ARMATURE tenham ficado quase idênticos numericamente, ambos passaram a
reproduzir a pose source errada. Portanto V0.2.9 remove integralmente o container-anchor rebase.

Antes da V0.2.8, a falha visual reportada na STAND estava localizada em cabeça/pescoço, enquanto WALK e
ATTACK já estavam visualmente corretos. Esta versão mantém o caminho GLOBAL_BIND já validado e aplica um
guard mínimo apenas ao Monbagon, motion STAND, bones `Bip01 Neck` e `Bip01 Head`.

## Correção

Para Neck e Head na STAND do Monbagon, a matriz global source produzida pelo evaluator não é usada como
transform local animado. Em vez disso, cada osso mantém sua relação local do bind com o pai atual:

    bind_local_row = bind_child_row @ inverse(bind_parent_row)
    current_child_row = bind_local_row @ current_parent_row

Isso preserva o acompanhamento do torso/parent, mas elimina os transforms locais da STAND que estavam
visualmente deformando a cadeia Neck/Head.

## Escopo

- Monbagon somente (nome do SMB contém `monbagon`).
- STAND somente.
- `Bip01 Neck` e `Bip01 Head` somente.
- WALK e ATTACK continuam intocados.
- Nenhum SMB/INX/SMD/cliente é modificado.
- O bridge Blender `matrix_basis` da V0.2.5 permanece.
- Sem enfraquecimento de validators/certificações.

O JSON de diagnóstico/validação informa:

    source_pose_reference = GLOBAL_BIND
    pose_guard = MONBAGON_STAND_BIND_LOCAL_V1
    pose_guard_bones = ["Bip01 Neck", "Bip01 Head"]

Esta é uma correção conservadora para validar visualmente a cadeia que efetivamente falhava antes da
regressão V0.2.8; ela não é uma alegação de semântica genérica para todos os assets.
