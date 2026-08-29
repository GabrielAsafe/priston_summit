# PT Asset Tool Backend V0.2.2 — Animation Bridge Diagnostic

Objetivo: isolar a causa da explosão visual do rig sem fazer uma terceira correção por tentativa.

O comando novo:

```bash
python3 pt_asset.py diagnose animation Monbagon-C.smd --motion ATTACK
```

faz:

1. valida o asset rigid;
2. prepara overlay de texturas sem alterar o cliente;
3. importa bind pose;
4. gera a Action com o baker V0.2.1 atual;
5. compara, em todos os frames inteiros da Action:
   - `SOURCE_DIRECT`: cada vértice rigidamente deformado pela matriz source `current * inverse(bind)`;
   - `BLENDER_ARMATURE`: posição produzida pelo Armature Modifier;
6. mede `max_error`, `rms_error`, `p95_error` e maiores erros por bone;
7. escolhe automaticamente o pior frame;
8. cria `source_vs_armature_diagnostic.blend` com duas poses congeladas lado a lado:
   - esquerda: `SOURCE_DIRECT`;
   - direita: `BLENDER_ARMATURE`;
9. abre Blender para inspeção humana;
10. registra `DIAG_SOURCE_DIRECT` e `DIAG_ARMATURE_MATCH`.

Interpretação:

- SOURCE_DIRECT correto + ARMATURE errado: evaluator/source skinning está coerente; bug está no bridge para PoseBone/Armature.
- SOURCE_DIRECT errado + ARMATURE errado: voltar ao evaluator/source animation semantics.
- ambos corretos no frame congelado, mas animação anterior explodia: investigar interpolação/subframes do Blender.

A geometria do cliente não é modificada. O diagnóstico trabalha sobre cópias em `pt_build`.
