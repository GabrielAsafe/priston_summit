# Summit Tale Asset Tool V0.3.9 — Player Head Assembly

## Por que esta versão existe

A V0.3.8 resolveu corretamente o binding dos modelos modulares `CONFIG_ONLY`
pelo `INX LinkFile/LinkCode/FileCode`. O teste de `CmfbB02.smd`, porém, validava
apenas a peça corporal.

No cliente real, a cabeça/face é outra peça rigid Physique. Ela possui INF/INX
próprios e é carregada/animada separadamente. Exemplo real da Atalanta:

    corpo: CmfbB02.smd -> Cmb002.inx -> M2Bip.inx -> m2.smb
    cabeça: MfhB01.smd -> Mfh-B01.inx -> M2Bip.inx -> m2.smb

`MfhB01.smd` possui três objetos de LOD (`H/M/L`) e usa bones de cabeça/cabelo.
O structural gate contra `m2.smb` passa sem bones ausentes.

## Regra de descoberta de cabeça

`asset heads <body>` não usa prefixo `Mfh/Tfh/...` como regra.

Um candidato precisa:

1. ser `CHARACTER / SMD_MODEL_062_RIGID_PHYSIQUE`;
2. possuir binding runtime verificado `INX_LINK_FILE`;
3. convergir para o mesmo `motion_inx` root do corpo;
4. convergir para o mesmo `motion_smb` do corpo;
5. possuir um `.inf` físico irmão do INX da peça.

O `.inf` é a evidência de que o modelo pertence à camada runtime de face/head.
O modo `auto` escolhe somente um representante técnico. Quando existe o layout
normal de três objetos H/M/L, ele é priorizado. Isso não significa que aquela
é a aparência escolhida pelo jogador.

## Animação do assembly

`validate animation` agora aceita:

    --head auto   # padrão
    --head none
    --head <asset-id-ou-path>

Para um playable character modular com `--head auto`:

1. corpo é importado com seu INX e root skeleton;
2. cabeça é importada separadamente com seu próprio INX;
3. a mesma motion source é baked no armature do corpo;
4. a mesma motion source é baked, independentemente, no armature da cabeça;
5. os dois blends animados são combinados apenas para inspeção humana;
6. os dois armatures continuam distintos no blend final.

Isso preserva a semântica de peças independentes do cliente em vez de fundir a
cabeça no rig do corpo.

O blend final registra:

    pt_player_assembly = BODY_PLUS_INDEPENDENT_HEAD

## Certificação

Quando há cabeça montada, o gate pergunta também:

- corpo e cabeça permanecem alinhados durante a animação;
- rig/deformação do conjunto está correto;
- motion do conjunto está correta;
- texturas/materiais de corpo+cabeça estão corretos.

São registrados ainda:

- `VISUAL_PLAYER_ASSEMBLY` no body asset;
- `VISUAL_ANIMATION_ASSEMBLED_PART` no head asset;
- os gates já existentes de rig/animação/material.

Certificações antigas não são removidas. O PASS visual V0.3.8 do `CmfbB02`
continua sendo evidência válida da peça corporal, não do assembly completo.

## Comandos

Listar cabeças tecnicamente compatíveis:

```bash
cd "/home/gabriel/Área de Trabalho/summit/design_work"
python3 pt_asset.py asset heads char/tmABCD/CmfbB02.smd --limit 20
```

Validar um STAND do assembly completo com representante automático:

```bash
cd "/home/gabriel/Área de Trabalho/summit/design_work"
python3 pt_asset.py validate animation char/tmABCD/CmfbB02.smd \
  --motion 10 \
  --load-actions selected
```

Validar ATTACK representativo:

```bash
cd "/home/gabriel/Área de Trabalho/summit/design_work"
python3 pt_asset.py validate animation char/tmABCD/CmfbB02.smd \
  --motion 40 \
  --load-actions selected
```

Escolher uma cabeça específica:

```bash
cd "/home/gabriel/Área de Trabalho/summit/design_work"
python3 pt_asset.py validate animation char/tmABCD/CmfbB02.smd \
  --motion 10 \
  --load-actions selected \
  --head char/tmABCD/MfhB01.smd
```

Desativar assembly de cabeça para diagnóstico de peça isolada:

```bash
cd "/home/gabriel/Área de Trabalho/summit/design_work"
python3 pt_asset.py validate animation char/tmABCD/CmfbB02.smd \
  --motion 10 \
  --load-actions selected \
  --head none
```

A V0.3.9 não altera SMD/INX/SMB nem arquivos do cliente.
