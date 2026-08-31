# 056 — PremiumItems ausentes no primeiro login

**Estado:** APROVADO

## Problema

Personagens novos recebiam os PremiumItems do starter kit, porém os ícones/buffs não apareciam no primeiro login.

Os PremiumItems só passavam a aparecer depois de sair e entrar novamente no personagem.

## Investigação

Foi instrumentado o handler cliente de:

`OPCODE_UPDATE_PREMIUMITEM_TIME`

O runtime mostrou inicialmente todos os timers em zero.

Depois da concessão do starter kit, os timers passaram corretamente para `43200` segundos (12 horas), incluindo:

- Third Eye
- EXP
- Vampiric Cuspid
- Mana Recharge
- Mana Reduce
- Might of Awell
- Phoenix Pet
- Stamina Reduce

Apesar disso, o probe continuava mostrando:

`active_before=0`

e:

`active_after=0`

Portanto o cliente recebia e armazenava corretamente os tempos, mas nenhum `UpKeepItem` era criado no primeiro login.

## Causa provada

O fluxo de sincronização de `OPCODE_UPDATE_PREMIUMITEM_TIME` atualizava os tempos, mas pressupunha que os elementos de `UpKeepItem[]` já existissem.

No fluxo inicial do starter kit, o CLIENT recebia `OPCODE_EXPLVL1`.

Esse handler chamava:

`chaPremiumitem.UsePremiumItem(155);`

porém o tratamento correspondente não inicializava os PremiumItems.

O próprio fluxo antigo dependia de relog para que `record.cpp` restaurasse os timers e executasse `SetUpKeepItem()`, momento em que os ícones finalmente apareciam.

## Alteração

Foi alterado somente:

`SourceCode/SrcGame/src/netplay.cpp`

O tratamento de `OPCODE_EXPLVL1` passou a inicializar imediatamente os PremiumItems concedidos pelo starter kit usando os mesmos mecanismos de `SetUpKeepItem()` utilizados pelo carregamento normal do personagem.

O handler genérico `OPCODE_UPDATE_PREMIUMITEM_TIME` não foi alterado.

## PremiumItems inicializados

- THIRD_EYES
- EXPUP_POTION
- VAMPIRIC_CUS
- MANA_RECHAR_P
- MANA_REDUCE_P
- MIGHT_OF_AWELL
- PHENIX_PET
- STAMINA_REDUCE_P

## Build

CLIENT compilado com sucesso.

## Teste no jogo

Teste realizado com personagem novo.

Resultado:

- starter items continuaram sendo recebidos;
- PremiumItems apareceram no primeiro login;
- não foi necessário relogar;
- comportamento esperado confirmado no runtime.

## Resultado

**APROVADO**

## Ficheiros

- `before/netplay.cpp`
- `after/netplay.cpp`
- `hashes_before.sha256`
- `hashes_after.sha256`
- `change.patch`
- `diff_check.txt`
- `commit-base.txt`

## Commit final

Pendente no momento da criação deste README.
