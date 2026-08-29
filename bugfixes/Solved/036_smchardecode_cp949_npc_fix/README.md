# 036 — smCharDecode CP949 / NPC model fix

## Status

**RESOLVIDO E VALIDADO EM RUNTIME — 2026-08-24**

## Sintoma

Vários NPCs carregavam/reutilizavam o mesmo modelo visual.

O problema coexistia com outro sintoma ainda aberto: monstros não apareciam. Este bugfix fecha somente o problema dos NPCs.

## Arquivo alterado

`SourceCode/SrcServer/src/fileread.cpp`

Função principal:

`smCharDecode()`

## Regressão

Os arquivos legados de definição de personagens usam tokens coreanos CP949.

No CURRENT, grande parte dos literais correspondentes em `smCharDecode()` havia sido destruída por conversão de encoding e aparecia como U+FFFD (`�`), bytes UTF-8 `EF BF BD`.

Isso fazia `lstrcmp` / `lstrcmpi` comparar os tokens dos arquivos de dados contra bytes diferentes dos esperados.

Consequência direta: campos das definições deixavam de ser reconhecidos corretamente.

Entre os campos críticos já comprovados:

- NAME (`*이름`);
- MODEL (`*모양파일`);
- CHAT (`*대화`).

MODEL alimenta `szModelName`. Quando MODEL não é reconhecido, as definições de NPC deixam de fornecer corretamente o modelo individual.

## Fonte correta da restauração

A restauração foi extraída do working tree recuperado imediatamente anterior ao reset:

`/home/gabriel/Transferências/recovery_pre_reset_20260824_0958/working_tree_vs_HEAD.patch`

O armazém histórico foi usado em auditorias anteriores apenas para confirmar a origem semântica dos bytes CP949. Nenhum código foi portado dele neste fix.

## Escopo aplicado

O patch focal de `smCharDecode()` foi validado antes de alterar o source:

- 63 hunks;
- 96 linhas removidas;
- 96 linhas adicionadas;
- 100 literais CP949 em formato C `\xNN`;
- todas as alterações focalizadas eram comparações `lstrcmp` / `lstrcmpi`;
- nenhum item/drop;
- nenhum `DecodeItemInfo()`;
- nenhum `DecodeOpenMonster()`;
- nenhuma alteração de `smModelDecode()`;
- nenhuma alteração de loader de pattern.

Validação registrada:

`VALIDATED_FULL_SET=YES`

SHA-256 do patch focal:

`8ec4ebfb2b6b49cd8bcb7f38ec902a5a482101fbd44a8fea2f2e29b39d159fef`

## Estado após o fix

SHA-256 observado de `fileread.cpp`:

`a4c5d51661551bdd4bed240581b567fd11943e44dab7a0bb841038260c7cb726`

SHA-256 observado de `smCharDecode()`:

`4ffa039f6c009072a76eeeb8f13760f07a6df02b5d9d5d59598e8131b9909f20`

Após a restauração:

- `EF BF BD count = 0` dentro de `smCharDecode()`;
- `NAME_HEX_ESCAPE_PRESENT=True`;
- `MODEL_HEX_ESCAPE_PRESENT=True`;
- `CHAT_HEX_ESCAPE_PRESENT=True`.

## Validação runtime

O servidor foi compilado e executado com o conjunto completo restaurado.

Resultado:

**os NPCs deixaram de repetir o mesmo modelo.**

Isso fornece validação causal de que a corrupção dos tokens CP949 de `smCharDecode()` era uma regressão real responsável pelo bug dos NPCs.

## MODEL-only não representa o fix

Antes da restauração completa houve um experimento MODEL-only.

Esse estado era híbrido: MODEL estava funcional, mas NAME e dezenas de outros tokens permaneciam corrompidos. O servidor percorria mais etapas de carregamento e posteriormente crashava.

Esse experimento não invalida o conjunto CP949. O estado válido é a restauração completa recuperada do working tree pré-reset.

## Experimento 035

`SrcServer/src/character.cpp` contém/continha um experimento independente recuperado no passo 035: guard de `smMotionInfo == NULL` em `FindMotionCountFromCode()`.

Esse arquivo **não faz parte do Fix 036** e não é incluído neste commit.

Ele permanece como experimento separado até decidirmos mantê-lo ou revertê-lo.

## Cadeia técnica relevante

`char/*.ini / *.inf`
→ `smCharDecode()`
→ `szName`, `szModelName`, atributos
→ inicialização de NPC/monstro
→ carregamento correspondente.

Há memória operacional de que o servidor funcional levava mais tempo carregando os personagens. Isso é compatível com MODEL e os demais campos voltando a alimentar a cadeia real de carregamento.

## Resultado

### NPCs

**RESOLVIDO.**

### Monstros

**AINDA ABERTO.**

O Fix 036 é a nova base e não deve ser revertido durante a investigação dos monstros.

## Próximo alvo

O recovery pré-reset também contém correções CP949 dentro de `DecodeOpenMonster()`, parser dos arquivos `.spm`.

Os tokens recuperados incluem:

- ACTOR;
- BOSS_ACTOR;
- MAX_ACTOR_POS;
- DELAY;
- MAX_ACTOR.

Esse é o próximo candidato porque ACTOR participa diretamente da correspondência:

`.spm ACTOR name`
→ `lpCharInfoList[].szName`
→ `rsMonster[]`
→ `MonCnt`.

Se ACTOR estiver corrompido, o arquivo de spawn pode ser lido sem nunca registrar os monstros.
