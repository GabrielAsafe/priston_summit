# Summit Tale Asset Tool V0.3.8 — Runtime INX Link Binding

## O problema que esta versão fecha

As versões V0.3.0–V0.3.7 tentaram descobrir o root skeleton de modelos rigid
`CONFIG_ONLY` por relações secundárias: mesmo INX, compatibilidade estrutural,
assinatura de skeleton, subset Physique, referências literais e assembly graph.
Essas abordagens foram úteis como diagnóstico, mas `Cmb002.inx` continuou
isolado no pairing e não expunha uma referência `.inx/.smb/.smd` literal.

O pacote de diagnóstico do cliente revelou o vínculo que o runtime realmente usa:
o header INX possui `LinkFile`, `LinkCode` e `FileCode`.

Exemplo real:

    char/tmABCD/Cmb002.inx
        LinkFile = char\\tmABCD\\M2Bip.in
        LinkCode = 0xF915414B

O `x` final não está no C-string serializado, porém o `LinkCode` é calculado
sobre o path completo `char\\tmABCD\\M2Bip.inx`.

O target real confirma a relação pelo outro lado:

    char/tmABCD/M2Bip.inx
        FileCode = 0xF915414B

E o pairing do target resolve:

    M2Bip.inx -> tfbD01.smd + m2.smb

Portanto `CmfbB02.smd`, cuja config é `Cmb002.inx`, tem a relação runtime:

    CmfbB02.smd
      -> Cmb002.inx
      -> LinkFile/LinkCode verificados
      -> M2Bip.inx
      -> m2.smb
      -> tfbD01.smd

Nenhuma heurística de basename, diretório, classe ou similaridade de skeleton é
necessária para essa decisão.

## Regra V0.3.8

Para um `SMD_MODEL_062_RIGID_PHYSIQUE` sem motion direta:

1. resolve fisicamente o INX da peça;
2. lê somente o tail de identidade do layout INX conhecido;
3. reconstrói `FileCode` e `LinkCode` com a mesma semântica já comprovada no
   `pt_inx_motion_core_v2.py`;
4. reconstrói o target `.inx` a partir do `LinkFile` serializado;
5. exige `SpeedSum(target) == source.LinkCode`;
6. resolve fisicamente o target INX;
7. exige `target.FileCode == source.LinkCode`;
8. exige que o pairing exato do target converja para um único `motion_smb`;
9. exige que o SMB e os root models declarados existam fisicamente;
10. somente então promove para `ROOT_SKELETON_BOUND / INX_LINK_FILE`.

A ligação é forte o suficiente para resolver também o contexto de animação:
`motion_inx` passa a ser o INX root linkado e `motion_smb` o SMB daquele target.
Assim `animation_binding_ready=true` quando todas as configs do asset convergem
para o mesmo target/motion.

## Fail-closed

- várias configs do mesmo SMD apontando para roots/motions diferentes:
  `INX_LINK_FILE_AMBIGUOUS`;
- LinkFile presente mas código inválido/target ausente/target não parseável:
  `INX_LINK_FILE_UNRESOLVED`;
- INX físico ausente ou layout desconhecido: bloqueado; não cai para inferência
  mais fraca;
- fallbacks antigos só podem rodar quando todos os INX físicos foram parseados e
  declararam explicitamente `NO_LINK`;
- a regra nova é aplicada apenas à família rigid Physique; outras famílias
  preservam a semântica anterior.

## Cobertura real de tmABCD

Baseline V0.3.7, rigid Model 0.62 em `char/tmABCD`:

- `CONFIG_ONLY`: 1375
- `SELF_CONTAINED`: 7
- `NONE`: 11

V0.3.8:

- `ROOT_SKELETON_BOUND / INX_LINK_FILE`: 1374
- `CONFIG_ONLY / INX_LINK_FILE_AMBIGUOUS`: 1
- `SELF_CONTAINED / DIRECT_CONFIG_MOTION`: 7
- `NONE`: 11

As 1476 linhas de pairing pertencentes aos 1375 assets rigid config-only foram
testadas. Resultado: `1476 / 1476 VERIFIED` no vínculo LinkFile/LinkCode/FileCode.

O único asset não promovido é `char/tmABCD/ar0nDrac.smd`: suas cinco configs
são todas verificáveis, mas convergem para três roots/motions diferentes. Isso é
ambiguidade real de assembly e permanece bloqueado.

## Comandos úteis

Após instalar:

```bash
cd "/home/gabriel/Área de Trabalho/summit/design_work"
python3 pt_asset.py bindings --discover char/tmABCD/CmfbB02.smd
```

Agora esse comando retorna imediatamente `INX_LINK_FILE_BOUND`; não repete o
scan antigo de 958 SMBs.

```bash
cd "/home/gabriel/Área de Trabalho/summit/design_work"
python3 pt_asset.py asset inspect char/tmABCD/CmfbB02.smd --physical
```

Esperado:

    binding_mode = ROOT_SKELETON_BOUND
    binding_source = INX_LINK_FILE
    binding_skeleton_smb = char/tmabcd/m2.smb
    rigid_binding_ready = true
    animation_binding_ready = true

Structural:

```bash
cd "/home/gabriel/Área de Trabalho/summit/design_work"
python3 pt_asset.py validate structural char/tmABCD/CmfbB02.smd
```

Blender visual, para certificação humana de assembly:

```bash
cd "/home/gabriel/Área de Trabalho/summit/design_work"
python3 pt_asset.py validate visual char/tmABCD/CmfbB02.smd
```

A V0.3.8 não altera SMD/INX/SMB nem arquivos do cliente.
