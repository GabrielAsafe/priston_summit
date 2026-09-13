# Summit Port Rebuild — Behavioral Preservation Contract

## Objetivo

Reconstruir o porte usando `pre_refactor.zip` como referência funcional e `final_07.zip`/histórico do porte como referência arquitetural.

O objetivo NÃO é melhorar o código. O objetivo é manter a mesma estrutura modular do porte (pastas, adapters, abstrações, classes, headers compartilhados e extrações de monólitos) preservando o comportamento executável do `pre_refactor`.

## Fontes de verdade

1. **Comportamento / implementação funcional:** `pre_refactor.zip`.
2. **Estrutura / organização / abstrações:** `final_07.zip` e `REFACTORING_HISTORY.bundle`.
3. Quando houver conflito entre uma implementação nova e o comportamento antigo, prevalece o `pre_refactor`.
4. Código estranho, redundante, permissivo ou aparentemente incorreto no `pre_refactor` deve ser preservado se a alteração não for estritamente necessária para mover/extrair o código.

## Regras obrigatórias

Durante a reconstrução não será introduzida, de forma intencional, nenhuma das alterações abaixo:

1. Nenhuma correção de bugs de gameplay/runtime do código original.
2. Nenhuma otimização.
3. Nenhuma validação nova.
4. Nenhuma alteração de algoritmo.
5. Nenhuma mudança de constantes, limites ou capacidades funcionais.
6. Nenhuma mudança de comportamento entre MinGW e MSVC.
7. Nenhuma "melhoria" de segurança que altere entradas aceites, saídas, fluxos ou estados.
8. Nenhuma alteração de ownership, lifetime ou estratégia de memória.
9. Nenhuma alteração de ordem de inicialização/execução relevante.
10. Nenhuma alteração de parsing ou serialização.
11. Nenhuma alteração funcional de collision, rendering, network ou gameplay.

## O que PODE mudar

São permitidas mudanças estritamente estruturais necessárias para reproduzir a arquitetura do porte:

- mover ficheiros entre diretórios;
- mover funções/implementações para `.inl`, `.h`, `.cpp` ou módulos `Shared/`;
- criar/manter adapters cliente/servidor;
- criar/manter abstrações e classes já previstas pela arquitetura do porte;
- substituir duplicação cliente/servidor por um corpo compartilhado somente quando o corpo extraído continuar semanticamente equivalente ao original de cada target;
- ajustar caminhos de `#include` em consequência de movimento de ficheiros;
- adicionar forward declarations apenas quando equivalentes às declarações existentes e sem mudar linkage;
- adicionar declarations/adapters necessários para manter a estrutura sem alterar ABI/comportamento;
- ajustar scripts/listas de build exclusivamente para apontarem para os novos caminhos.

## Regra para abstrações e classes

As abstrações/classes do porte devem ser mantidas. Elas não devem ser removidas simplesmente para voltar aos monólitos.

Porém, a implementação dentro dessas abstrações deve ser reconstruída a partir do código funcional de `pre_refactor`.

Exemplo conceitual:

```text
pre_refactor/LegacyMonolith.cpp
        |  extrair mecanicamente
        v
Shared/subsystem/LegacyCommon.inl
Shared/subsystem/Policy.h
SrcGame/.../LegacyMonolith.cpp   (adapter)
SrcServer/.../LegacyMonolith.cpp (adapter)
```

A extração pode mudar onde o código reside, mas não o que o código faz.

## Política para monólitos

Ao separar um monólito:

- preservar o corpo das funções;
- preservar condições e ordem dos `if`/`switch`/loops;
- preservar casts;
- preservar operações aritméticas e ordem das operações;
- preservar globals/statics e linkage;
- preservar macros e `#ifdef`;
- preservar valores de retorno;
- preservar ordem relativa de side effects;
- preservar tratamento de erro do baseline;
- preservar comportamento aparentemente estranho se fazia parte do baseline.

Não converter uma função legacy para uma implementação "equivalente" escrita de novo se for possível mover o corpo original.

## Linkage / ABI

Pontos tratados como críticos:

- `static` e anonymous namespaces;
- `extern` versus definição real;
- ordem de `#define` antes de includes;
- calling convention;
- packing/alinhamento de structs;
- tamanho e layout de estruturas serializadas;
- globals compartilhados entre cliente e servidor;
- ordem de inicialização estática;
- macros específicas de target/compiler.

Nenhuma mudança de linkage deve ser feita apenas para "limpar" o código.

## Parsing / serialização

Loaders, caches e formatos (`.inx`, `.smd`, configs, character/item/stage/server configs, warehouse etc.) devem aceitar/rejeitar exatamente segundo o comportamento funcional do `pre_refactor`, salvo mudança estritamente necessária para mover o código sem alterar a semântica.

Validações criadas pelo porte que não existiam no baseline devem ser removidas/revertidas quando alteram comportamento.

## Collision / rendering / gameplay / network

Refactors podem mover algoritmos para helpers/classes, mas o cálculo original deve permanecer intacto. Não corrigir fórmulas legacy, thresholds, retornos especiais, fallbacks ou ordem de processamento.

## MinGW / MSVC

Não unificar comportamentos que originalmente eram diferentes. Macros e caminhos específicos de compilador/plataforma devem permanecer funcionalmente equivalentes ao `pre_refactor`.

## Critério de revisão por mudança

Cada mudança estrutural deve poder ser descrita como uma destas categorias:

### A. Movimento estrutural puro

O mesmo código foi movido para outro ficheiro/módulo sem alteração funcional.

### B. Adaptação estrutural mínima

Alteração necessária apenas para permitir que o mesmo código compile no novo local (include path, declaração, namespace/policy/adaptor), sem alteração funcional.

Qualquer mudança que não se enquadre claramente em A ou B deve ser rejeitada ou explicitamente isolada para decisão manual.

## Registo obrigatório de mudanças

Para cada módulo migrado, este ficheiro deve ser expandido com:

- **Origem:** ficheiro/função do `pre_refactor`.
- **Destino:** ficheiro/abstração no porte.
- **Tipo:** movimento estrutural puro ou adaptação estrutural mínima.
- **O que mudou:** descrição objetiva.
- **O que NÃO mudou:** algoritmo, constantes, parsing, ownership, etc.
- **Risco conhecido:** linkage, macros, initialization order, ABI, etc.
- **Validação estática:** comparação/diff/teste usado.
- **Validação de build:** cliente/servidor quando disponível.
- **Validação runtime:** deve ser preenchida após teste real do jogo.

## Critério de aceitação

O porte só deve ser considerado concluído quando:

1. cliente compila;
2. cliente linka;
3. servidor compila;
4. servidor linka;
5. os principais fluxos runtime funcionam como no `pre_refactor`;
6. qualquer diferença funcional conhecida está documentada como bloqueio, não escondida como "melhoria";
7. não existem alterações intencionais das regras proibidas acima.

## Garantia possível

Não é tecnicamente possível garantir antes do teste no ambiente real que o primeiro executável nunca irá falhar. O compromisso verificável desta reconstrução é mais forte e objetivo: **nenhuma mudança funcional será introduzida deliberadamente; a arquitetura nova será construída em torno do comportamento do `pre_refactor`, e não o contrário.**


# Ledger inverso dos bugs do porte

Esta secção usa o `BUGS_FOUND.md` do porte como inventário de mudanças funcionais. Como a reconstrução tem `pre_refactor` como oracle, correções do porte que alteravam o comportamento original são tratadas como comportamento legacy a restaurar. BUG-060..063 são exceções porque recuperam comportamento/linkage já existente no oracle funcional.

| ID | Classificação na reconstrução | Subsistema | Comportamento legacy/origem | Impacto conhecido | O que o porte tinha alterado |
|---|---|---|---|---|---|
| BUG-001 | LEGACY-PRESERVED — desfazer correção funcional do porte | Vários / 3D / stage | Arrays alocados com `new[]` eram destruídos com `delete` escalar. | Undefined behavior, heap corruption e crashes intermitentes. | Ownership centralizado e destruição corrigida para arrays. |
| BUG-002 | LEGACY-PRESERVED — desfazer correção funcional do porte | Win32 I/O | Ownership de streams/handles Win32 era inconsistente. | Handles podiam ficar abertos ou ser fechados por caminhos diferentes. | Fronteiras RAII `LegacyWin32Handle`/`LegacyWin32File` e cleanup previsível. |
| BUG-003 | LEGACY-PRESERVED — desfazer correção funcional do porte | Win32 file enumeration | `FindFirstFile` tinha caminho com handle leak. | Leak de handle a cada enumeração afetada. | Handle passou a ter lifecycle correto. |
| BUG-004 | LEGACY-PRESERVED — desfazer correção funcional do porte | ASE / talk-bone | Fluxo de talk-bone vazava recurso/memória. | Leak durante carga/merge de bones. | Cleanup corrigido no import ASE. |
| BUG-005 | LEGACY-PRESERVED — desfazer correção funcional do porte | `smPAT3D` | `smPAT3D::Close()` podia fazer double-delete. | Heap corruption/crash em close repetido. | `Close()` tornado repetível/idempotente. |
| BUG-006 | LEGACY-PRESERVED — desfazer correção funcional do porte | `smPAT3D` | Reload de pattern reutilizava o objeto sem libertar corretamente o conteúdo anterior. | Leak e/ou estado antigo sobrevivendo ao reload. | Storage antigo é libertado antes da reutilização. |
| BUG-007 | LEGACY-PRESERVED — desfazer correção funcional do porte | Materiais | Texturas temporárias não eram sempre libertadas em rollback. | Leak em falhas parciais de criação/material. | `AddMaterial` passou a ser transacional com cleanup. |
| BUG-008 | LEGACY-PRESERVED — desfazer correção funcional do porte | Materiais | Conversão de material com várias texturas normais para animação vazava recursos. | Leak persistente durante conversão. | Ownership da conversão foi corrigido. |
| BUG-009 | LEGACY-PRESERVED — desfazer correção funcional do porte | Materiais | `InitMaterial`/`CloseMaterial` não eram seguramente repetíveis e podiam deixar globals inconsistentes. | Double cleanup, stale pointers ou estado global incoerente. | Lifecycle centralizado e repetível. |
| BUG-010 | LEGACY-PRESERVED — desfazer correção funcional do porte | Stage | Arrays de `smSTAGE3D` tinham lifecycle/destruição incorretos. | UB/leaks/corrupção de heap. | `StageStorage` centraliza init/close com operador correto. |
| BUG-046 | LEGACY-PRESERVED — desfazer correção funcional do porte | `AssaEffect` servidor | `InitAssaEffect()` limpava o array `cAssaEffect[5000]` com `sizeof(cASSAEFFECT)` em vez do tamanho do array. | Apenas uma fração do array de ponteiros era inicializada (ou tamanho semanticamente incorreto), deixando slots stale e comportamento imprevisível. | `CONTINUED_27`: servidor usa `sizeof(cAssaEffect)`, igual ao storage real e ao cliente. |
| BUG-049 | LEGACY-PRESERVED — desfazer correção funcional do porte | `YameEffect` | `InitYameEffect()` limpava `cYameEffect[5000]` com `sizeof(cYAMEEFFECT)` em ambos os targets. | O array de ponteiros ficava apenas parcialmente zerado, permitindo slots stale durante init. | `CONTINUED_27`: ambos os targets usam `sizeof(cYameEffect)`. |
| BUG-050 | LEGACY-PRESERVED — desfazer correção funcional do porte | `ISAOCHECK::read_sod2` | `memset(szWriteBuffer, 0, sizeof(szWriteBuffer[BUF_LEN]))` limpava somente um `char`. | Request buffer permanecia quase totalmente não inicializado antes do `sprintf`. | `CONTINUED_27`: usa `sizeof(szWriteBuffer)`. |
| BUG-051 | LEGACY-PRESERVED — desfazer correção funcional do porte | `ISAOCHECK::read_sod2` | `memset(szSod2Buff, 0, sizeof(szSod2Buff))` usava `sizeof` do parâmetro ponteiro. | Só 4/8 bytes do output de 65536 bytes eram zerados antes do HTTP read. | `CONTINUED_27`: limpa explicitamente os 65536 bytes compatíveis com o contrato já usado por `GetHttpRequest`. |
| BUG-052 | LEGACY-PRESERVED — desfazer correção funcional do porte | `cYAMEEFFECT` | O construtor fazia `memset(this, 0, sizeof(cASSAEFFECT))`, usando o tamanho de outra classe. | Escrita podia ultrapassar ou deixar incompleto o subobjeto `cYAMEEFFECT`, corrompendo estado adjacente. | `CONTINUED_27`: tamanho corrigido para `sizeof(cYAMEEFFECT)` nos dois targets. |
| BUG-053 | LEGACY-PRESERVED — desfazer correção funcional do porte | Manufacture inventory | `g_Manufacture_ItemInfo[176]` era limpo com `sizeof(SManufacture_ResultItemInfo)`, equivalente a um único elemento. | Entradas 1..175 podiam manter dados stale após reset/load. | `CONTINUED_27`: cliente e servidor usam `sizeof(g_Manufacture_ItemInfo)`. |
| BUG-056 | LEGACY-PRESERVED — desfazer correção funcional do porte | `record.cpp` / Win32 file enumeration | Vários fluxos chamavam `FindClose(hFind)` imediatamente após `FindFirstFile`, inclusive quando o retorno era `INVALID_HANDLE_VALUE`. | API Win32 recebia um handle inválido em paths de lookup/restore/warehouse, produzindo lifecycle incorreto e comportamento dependente da implementação. | `CONTINUED_28`: todos os paths auditados só executam `FindClose` após confirmar handle válido; o loader admin falha cedo quando o ficheiro não existe. |
| BUG-060 | REFRACTOR-RECOVERY — manter; restaura baseline/linkage | Tier 5 Cry / stage item ownership | O refactor perdeu o campo server-only `STG_ITEMS::dwExclusiveOwnerCode` que já existia no baseline e a integração FIX091–171 reteve apenas parte das referências ao ownership. | O servidor MinGW não compilava o spawn T5 Cry; além disso, sem os gates completos, um drop exclusivo podia ser anunciado/coletado por jogador diferente do dono e o token de pickup não era registrado corretamente. | `CONTINUED_31`: campo restaurado somente no header do servidor e lifecycle final do oracle restaurado em `AddItem`, `DeleteItem`, ambos `SendStgItem`, `SendStgItemToNearUsers` e `OPCODE_PUTITEM`; differential dedicado prende o contrato ao parent `d88ab3b`. |
| BUG-011 | LEGACY-PRESERVED — desfazer correção funcional do porte | ASE/material/physique/stage | Vários loaders não validavam índices, contagens ou capacidades antes de escrever. | Out-of-bounds read/write e crashes em assets malformados. | Guards/capacidades adicionados progressivamente; loaders endurecidos. |
| BUG-012 | LEGACY-PRESERVED — desfazer correção funcional do porte | `smOBJ3D::WorldForm` | Escrita podia exceder `RENDER_VERTEX_MAX`. | Buffer overflow no buffer de vertices de render. | Capacidade passou a ser parâmetro explícito e overflow é rejeitado. |
| BUG-013 | LEGACY-PRESERVED — desfazer correção funcional do porte | Bone merge | Merge não validava adequadamente motions carregados e limites de frame. | Acesso fora de faixa/cópia inválida em animações. | Motions e frame bounds são validados antes do merge. |
| BUG-014 | LEGACY-PRESERVED — desfazer correção funcional do porte | Character config | `MotionInfo` possuía 512 slots, mas `NpcMotionRate` apenas 30, e o reader podia escrever além de `NpcMotionRate`. | Buffer overflow direto. | Reader compartilhado limita a escrita pela capacidade real. |
| BUG-015 | LEGACY-PRESERVED — desfazer correção funcional do porte | Character config | Character drops/plus-drops podiam ultrapassar os arrays de destino. | Buffer overflow em configuração textual. | Reader compartilhado valida as capacidades. |
| BUG-016 | LEGACY-PRESERVED — desfazer correção funcional do porte | Stage areas | Faces com índices inválidos/cobertura espacial acima do limite podiam chegar ao builder. | OOB, corrupção do índice espacial ou stage inconsistente. | `StageAreaBuilder` valida antes do commit. |
| BUG-017 | LEGACY-PRESERVED — desfazer correção funcional do porte | Server config cliente | `*SYSTEM_IP` usava `ADMIN_IP_MAX` (16) para limitar `SystemIP[32]`. | Apenas metade da capacidade de `SYSTEM_IP_MAX` era carregável; configuração válida era truncada. | `CONTINUED_14`: gate alterado para `SYSTEM_IP_MAX` e testado até 32 entradas. |
| BUG-018 | LEGACY-PRESERVED — desfazer correção funcional do porte | Server config cliente | `*LOGIN_SERVER_IP` testava `SystemIP_Count`, mas escrevia/incrementava `LoginServerIP_Count`. | Se `SystemIP_Count < 16`, mais de 16 logins podiam escrever além de `LoginServerIP[16]`. | `CONTINUED_14`: gate usa `LoginServerIP_Count`; teste com canary garante que a 17ª entrada não escreve. |
| BUG-044 | LEGACY-PRESERVED — desfazer correção funcional do porte | Area server / network | `RecvAreaServerMap` no servidor copiava `lpTransServerMap->size` bytes para `TransAreaServer` sem validar ponteiro, `size`, `IP_Count` ou tamanho esperado do payload. | Pacote malformado podia provocar `memcpy` além de `TRANS_SEVER_MAP`, causando overwrite de memória; `IP_Count` inválido também era aceito. | `CONTINUED_17`: validação comum `AreaServerMapValidation.h` agora é executada por cliente e servidor antes do `memcpy`; harness cobre `nullptr`, sizes inválidos, `IP_Count` -1/65 e todas as contagens válidas 0–64. |
| BUG-054 | LEGACY-PRESERVED — desfazer correção funcional do porte | Warehouse / character record cliente | `rsLoadWareHouseData` iterava `cnt < 300` sobre `sWAREHOUSE::WareHouseItem[100]`; o mesmo índice podia alimentar `rsPLAYINFO::WareHouseItemInfo[120]`. | Leitura fora dos 100 slots reais e potencial escrita fora do array de 120 entries no estado do jogador. | `CONTINUED_28`: iteração limitada à capacidade real de 100 slots e differential prende o fix ao snapshot anterior. |
| BUG-058 | LEGACY-PRESERVED — desfazer correção funcional do porte | `ClientFuncPos` / network-integrity | `TRANS_CLIENT_FUNPOS::FuncCount` e `size` recebidos da rede eram aceitos sem validação; `FuncCount` alimenta loops/índices sobre `dwFuncValue[64]` e `size` era usado para persistência. | `FuncCount > 64` podia causar OOB; `size` malformado podia persistir bytes fora do contrato da estrutura. | `CONTINUED_28`: entrada, add e reload do cache exigem `size == sizeof(TRANS_CLIENT_FUNPOS)` e `FuncCount` em 0..64; persistência grava tamanho fixo da estrutura. |
| BUG-019 | LEGACY-PRESERVED — desfazer correção funcional do porte | `SetupPolyAreas` | Ponteiros `DWORD*` nulos eram incrementados como se fossem contadores. | Undefined behavior e superalocação acidental. | Builder refeito em duas passagens: contagem inteira + alocação/preenchimento. |
| BUG-020 | LEGACY-PRESERVED — desfazer correção funcional do porte | Serialização SMD | Aritmética de ponteiros serializados era espalhada pelo domínio como se valores persistidos fossem ponteiros válidos. | UB, rebases inválidos e maior superfície para corrupção. | Ponteiros persistidos tratados como offsets inteiros validados no adapter legado. |
| BUG-021 | BASELINE-PENDING — preservar exatamente como no pre_refactor | `smCharDecode` legado | Snapshot/oráculo ainda contém cast de inteiro 32-bit para pointer, visível como warning em host 64-bit. | Em build 64-bit a largura pode truncar endereço/offset; no target histórico x86 pode ser parte do formato. | Não alterado sem validação do toolchain/runtime Windows real. |
| BUG-041 | LEGACY-PRESERVED — desfazer correção funcional do porte | `HoEffectModel` cliente | O estado “sem current event” era implementado escrevendo `NULL` nos primeiros bytes de um `std::vector<...>::iterator` via `*(int*)&m_CurrentEvent`. | Undefined behavior: viola representação/alignment do iterator e pode produzir iterator corrompido, especialmente fora do layout x86/STL histórico. | `CONTINUED_15`: removido o type-punning com API explícita de cursor. `CONTINUED_16`: cliente e servidor passaram a usar uma representação compartilhada por índice (`m_CurrentEventIndex`), mantendo `ResetCurrentEvent()`/`HasCurrentEvent()` sem depender do layout da STL. |
| BUG-045 | LEGACY-PRESERVED — desfazer correção funcional do porte | `HoNewParticle` cliente | O comparator anônimo usado por `SortEvents()` tinha retorno apenas dentro de `if (lhs && rhs)` e terminava uma função `bool` sem `return` nos demais caminhos. | Caminho sem retorno é undefined behavior e viola o contrato do comparator se um ponteiro inválido/nulo chegar ao range; também gera implementação diferente desnecessária entre targets. | `CONTINUED_17`: header unificado usa `HoNewParticleEvent_CompareFunc`, que sempre retorna a comparação de `GetActualTime()` para os eventos válidos mantidos pelo parser; differential confirma que o restante do header é equivalente. |
| BUG-059 | LEGACY-PRESERVED — desfazer correção funcional do porte | Item runtime / `CreateDefItemPerf` | `dwSpecialJob` era declarado sem inicialização e podia ser comparado contra `sSpecialRevision[]` quando `SpJobCode > 0` mas `getJobDataCode(SpJobCode, 0)` retornava `nullptr`. | Undefined behavior; a revisão de atributos podia depender de valor indeterminado em lookup de job inválido/ausente. | `CONTINUED_29`: `dwSpecialJob` passa a ser inicializado com o `JobCodeMask` atual antes dos lookups; os caminhos com lookup válido continuam sobrescrevendo-o com `JobBitCode`. Differential dedicado confirma que o warning desaparece da implementação compartilhada. |
| BUG-022 | LEGACY-PRESERVED — desfazer correção funcional do porte | ASE | Falha de `ReadASE_GEOMOBJECT` era ignorada. | Import podia continuar com geometria inválida/parcial e sinalizar sucesso. | Falha agora é propagada. |
| BUG-023 | LEGACY-PRESERVED — desfazer correção funcional do porte | Vários loaders | Diversas funções ignoravam retorno de erro de subrotinas. | Estado parcial podia ser publicado como sucesso. | Entry points/refactors passaram a propagar falha onde identificado. |
| BUG-024 | LEGACY-PRESERVED — desfazer correção funcional do porte | ASE material | Registro de material podia marcar `Regist = TRUE` mesmo após falha real. | Material inválido era tratado como registrado. | Registro agora respeita o resultado real. |
| BUG-025 | LEGACY-PRESERVED — desfazer correção funcional do porte | Materiais | Script nulo/animação malformada podia chegar a dereference/scan inválido. | Crash ou leitura além da string. | Parser valida null/malformed input antes de processar. |
| BUG-026 | LEGACY-PRESERVED — desfazer correção funcional do porte | Materiais | Parser do formato de animação modificava a string recebida. | Side effect inesperado/corrupção de input compartilhado ou literal. | Parsing passou a operar sem mutar o input. |
| BUG-027 | LEGACY-PRESERVED — desfazer correção funcional do porte | Materiais | Formatos incompatíveis podiam alimentar formatting/`printf` inseguro. | Leitura de argumentos incorretos/UB em formatting. | Formatos incompatíveis são rejeitados antes do formatting. |
| BUG-028 | LEGACY-PRESERVED — desfazer correção funcional do porte | Materiais | `FindNextMaterialFromTextureName` não avançava corretamente. | Loop/resultado repetido ou busca que não progride. | Iteração corrigida para avançar ao próximo material. |
| BUG-029 | LEGACY-PRESERVED — desfazer correção funcional do porte | `auth.h` | Include guard `__AUTH_H__` terminava no meio do ficheiro; `THREADstruct`, `WebDB_Thread`, `WebDB` e APIs finais ficavam fora do guard. | Incluir `auth.h` mais de uma vez podia redefinir tipos/classes/objetos. | `CONTINUED_14`: header centralizado e guard fecha apenas no EOF; teste pré-processa inclusão dupla e confirma uma única classe. |
| BUG-047 | LEGACY-PRESERVED — desfazer correção funcional do porte | `AssaUtil` / Win32 enumeration | `FindFileRecursive`/`ExistAseFile` consumiam `WIN32_FIND_DATA` mesmo quando `FindFirstFile` falhava; no servidor `LoadResFile` também não fazia gate real. | Leitura de estrutura não inicializada e chamadas subsequentes sobre enumeração inválida. | `CONTINUED_27`: os três fluxos rejeitam `INVALID_HANDLE_VALUE` antes de tocar em `wfd`; diagnostics client-only foram preservados. |
| BUG-048 | LEGACY-PRESERVED — desfazer correção funcional do porte | `AssaUtil` extension parsing | `ExistAseFile`/`LoadResFile` copiavam filename/extensão para buffers fixos e podiam usar `tempExtName` sem inicialização quando não havia `.`. | Uso de dados indeterminados e possibilidade de overflow para nomes/extensões longos. | `CONTINUED_27`: parsing usa `strrchr` diretamente sobre `wfd.cFileName`, sem cópias temporárias e com fallback `""`. |
| BUG-061 | REFRACTOR-RECOVERY — manter; restaura baseline/linkage | MinGW/link / `Svr_Damge` | Após o refactor, `SendWareHouseToClient` e `SendQuickMessage` eram declarados por `extern` dentro de métodos de `LegacyDamagePolicy`, que vive em `namespace {}`. | O GCC/MinGW resolveu as referências como símbolos do anonymous namespace (`(anonymous namespace)::...`), embora as definições reais estejam em `OnSever.cpp` com linkage global. | `CONTINUED_32`: declarações movidas para escopo global antes do anonymous namespace; os métodos da policy passam a resolver as definições reais de `OnSever.cpp`. |
| BUG-062 | REFRACTOR-RECOVERY — manter; restaura baseline/linkage | Tier 5 quest / item grant | A integração FIX091–171 trouxe os callers de `rsPutItemChecked`, mas omitiu a própria implementação de `OnSever.cpp` e a declaração de `onserver.h` presentes no oracle FIX171. | Build 187/187 compilava, mas o linker falhava nas três chamadas de `cQuestHandler`; o grant de itens T5 não tinha provider. | `CONTINUED_32`: implementação e declaração restauradas exatamente pelo contrato FIX171; função rejeita player/socket inválido e item ausente/unique antes de delegar a `rsPutItem`. |
| BUG-063 | REFRACTOR-RECOVERY — manter; restaura baseline/linkage | Clan help / `g_GuideHelp` | A extração para `GuideHelpLegacyCommon.inl` moveu `#define GUIDEHELP_CPP` para depois de `GuideHelp.h`. | Ambos os wrappers viam apenas `extern g_GuideHelp`; nenhum translation unit instanciava o global e o linker falhava em `Help.o`. | `CONTINUED_32`: `GUIDEHELP_CPP` volta a ser definido no início dos wrappers cliente/servidor, antes de qualquer header que possa incluir `GuideHelp.h`; o `.inl` deixa de controlar esse linkage. |
| BUG-030 | LEGACY-PRESERVED — desfazer correção funcional do porte | Animation tracks | Scale track cuja primeira posição/offset era zero era interpretada como “sem animação”. | Animação válida podia ser descartada. | Presença de track deixou de depender de offset zero como sentinela. |
| BUG-031 | LEGACY-PRESERVED — desfazer correção funcional do porte | Talk animation cache | Cache confundia dois `TalkPattern` diferentes. | Animação/conteúdo de um pattern podia ser reutilizado para outro. | Identidade/cache corrigidos no refactor. |
| BUG-032 | LEGACY-PRESERVED — desfazer correção funcional do porte | ASE frames | Frame entries não inicializadas eram copiadas. | Propagação de lixo/estado indefinido para animação. | Cópia de entries não inicializadas removida. |
| BUG-033 | LEGACY-PRESERVED — desfazer correção funcional do porte | Collision | Um vetor de colisão deixava componentes Y/Z sempre zero em um dos sentidos. | Direção/resposta de colisão incorreta. | Cálculo corrigido no hardening consolidado. |
| BUG-034 | LEGACY-PRESERVED — desfazer correção funcional do porte | Stage face lookup | Índice `0` era usado ao mesmo tempo como face válida e sentinela de “não encontrado”. | Face zero podia ser confundida com falha de lookup. | Fluxo foi desacoplado da sentinela ambígua durante o refactor. |
| BUG-043 | LEGACY-PRESERVED — desfazer correção funcional do porte | `HoEffectBoundingBox` | O construtor default escrevia `m_AxisX.Min/Max` três vezes e nunca inicializava `m_AxisY`/`m_AxisZ`. | `GetPos()` podia usar limites Y/Z indefinidos quando o bounding box era usado antes de `Init()`, produzindo posições imprevisíveis/UB de dados não inicializados. | `CONTINUED_16`: defaults corrigidos para inicializar X, Y e Z em `[-10, 10]` na declaração compartilhada; differential confirma o bug nos dois snapshots anteriores e o fix único atual. |
| BUG-035 | LEGACY-PRESERVED — desfazer correção funcional do porte | Object/pattern serializers | Loader destruía/alterava objeto válido antes de descobrir que o ficheiro novo estava corrompido. | Uma tentativa de load falhada podia perder o estado válido anterior. | Leitura tornou-se transacional; commit só ocorre após validação completa. |
| BUG-036 | LEGACY-PRESERVED — desfazer correção funcional do porte | Stage serializer | `StageArea` histórico era escrito em `WORD` mas lido como `DWORD`. | Incompatibilidade de formato, offsets/leitura incorreta em ficheiros históricos. | Loader aceita `WORD` histórico e `DWORD`; novos saves escrevem `DWORD`. |
| BUG-037 | LEGACY-PRESERVED — desfazer correção funcional do porte | Stage save | Save usava comportamento equivalente a `OPEN_ALWAYS`, podendo deixar trailing bytes de um ficheiro anterior maior. | Lixo residual permanecia no fim do asset salvo. | Save usa `CREATE_ALWAYS`; loader ainda tolera trailing histórico. |
| BUG-038 | LEGACY-PRESERVED — desfazer correção funcional do porte | Stage serializer | `NextTex`/`lpTexLink` persistidos podiam ser rebased sem validação suficiente. | Pointer/link corrompido podia causar acesso inválido e destruir o stage carregado. | Offsets e tamanhos são validados antes do commit transacional. |
| BUG-039 | LEGACY-PRESERVED — desfazer correção funcional do porte | Material serializer | Nomes dependiam de um bloco fixo de 4096 bytes. | Assets/names maiores podiam truncar/falhar de forma insegura. | Serializer deixou de depender do buffer fixo. |
| BUG-042 | LEGACY-PRESERVED — desfazer correção funcional do porte | `HoEffectEventController` servidor | Storage usava `std::array<...,100>` e, na 101ª inserção, fazia `cursor = 0` e sobrescrevia slots existentes sem liberar os eventos anteriores. | Perda silenciosa de eventos e memory leak; `Clear()` após o wrap percorria apenas o cursor reiniciado, deixando eventos antigos sem destruição. | `CONTINUED_15`: storage passou a `std::vector`, sem wrap fixo; `Clear()` destrói todos os eventos armazenados. |
| BUG-055 | LEGACY-PRESERVED — desfazer correção funcional do porte | Warehouse persistence | Loaders liam diretamente para `TRANS_WAREHOUSE`, ignoravam o resultado de `fread` e podiam continuar com objeto parcial/indeterminado quando o ficheiro existia mas a leitura falhava ou estava truncada. | `size`, `DataSize`, checksum/money e payload podiam ser consumidos a partir de estado inválido; um load falhado podia publicar dados sem validação estrutural. | `CONTINUED_28`: leitura passou a usar objeto temporário zero-inicializado e commit somente após validar bytes lidos, header, `size`, `DataSize` e o tamanho interno do stream comprimido. |
| BUG-057 | LEGACY-PRESERVED — desfazer correção funcional do porte | Warehouse persistence / save | `rsSaveWareHouseData` passava `lpTransWareHouse->size` diretamente a `fwrite` sem verificar se o tamanho declarado cabia no `TRANS_WAREHOUSE` e se o payload era coerente. | `size` malformado podia fazer `fwrite` ler além do storage da estrutura e persistir dados inválidos. | `CONTINUED_28`: `ValidateWarehouseTransferLayout` valida ponteiro, header, tamanho persistido, `DataSize`, capacidade e encoded size antes do save. |

## Regra para este ledger

- `LEGACY-PRESERVED`: o comportamento do `pre_refactor` deve ser mantido, mesmo se o porte o classificou como bug. A ocorrência e o risco ficam documentados, não corrigidos.
- `REFRACTOR-RECOVERY`: manter a correção porque ela apenas restaura algo perdido pelo refactor ou resolve linkage/estrutura sem mudar a lógica funcional do oracle.
- `BASELINE-PENDING`: manter exatamente o estado do `pre_refactor`; não promover para fix durante esta reconstrução.

---

# Marco R0 — reconstrução por oracle funcional

## Objetivo deste marco

Este marco não tenta provar que as novas abstrações do porte reproduzem o comportamento antigo por reimplementação. Em vez disso, estabelece primeiro uma base executável com **paridade funcional por construção**:

- a árvore, nomes de módulos, pastas e abstrações adicionadas pelo porte permanecem presentes;
- todo ficheiro efetivo `.cpp`, `.h` ou `.inc` que tinha sido alterado funcionalmente pelo porte é substituído por um wrapper estrutural para uma cópia exata do `pre_refactor`, ou por uma cópia direta quando mover o texto mudaria o preprocessamento;
- as abstrações novas que ainda não foram demonstradas equivalentes permanecem disponíveis na árvore, mas não substituem o oracle no runtime deste marco;
- a subdivisão posterior dos blocos oracle deve ser mecânica e validada contra os hashes deste marco.

Este desenho prioriza o requisito principal: **primeiro recuperar o comportamento do `pre_refactor`; depois ativar/subdividir as abstrações sem reescrever a lógica.**

## Cobertura objetiva

Comparação entre `pre_refactor` e a árvore estrutural `final_07`:

- ficheiros efetivos `.cpp/.h/.inc` existentes no `pre_refactor`: **1141**;
- ficheiros sem alteração de bytes no porte: **853**;
- ficheiros cujo conteúdo tinha mudado no porte: **288**;
- ficheiros do `pre_refactor` ausentes no porte: **0**;
- dos 288 alterados, **265** usam wrapper para `Shared/legacy_oracle/...`;
- **23** permanecem byte-a-byte no path target por dependência de contexto de preprocessamento;
- o manifesto completo e SHA-256 de cada item está em `ORACLE_PARITY_MANIFEST.tsv`;
- o resumo mecânico está em `ORACLE_PARITY_SUMMARY.json`.

### Movimento estrutural utilizado

Para um ficheiro comum, por exemplo:

```text
SrcGame/src/<modulo>/Arquivo.cpp
```

o corpo funcional passa a existir em:

```text
Shared/legacy_oracle/client/<modulo>/Arquivo.cpp.inl
```

e o path moderno contém somente:

```cpp
// Structural wrapper only; behavior is the pre_refactor oracle.
#include "legacy_oracle/client/<modulo>/Arquivo.cpp.inl"
```

O mesmo vale para headers, sem acrescentar include guard no wrapper. Isto é intencional: o próprio header do `pre_refactor` continua responsável pela sua semântica de dupla inclusão, inclusive onde ela é defeituosa (por exemplo BUG-029).

## Exceções mantidas diretamente no path target

Os itens abaixo **não são movidos para `Shared/legacy_oracle`** porque isso mudaria resolução de includes locais, `__FILE__` ou outro contexto observável do preprocessor. Permanecem byte-a-byte iguais ao `pre_refactor`:

| Ficheiro | Motivo |
|---|---|
| `SrcGame/src/compat/client_server/server_legacy/Svr_Damge.cpp` | include relativo condicionado por `_LANGUAGE_VEITNAM`; preservar diretório-base de preprocessamento |
| `SrcServer/src/systems/combat/server_legacy/Svr_Damge.cpp` | mesmo motivo do target cliente |
| `SrcGame/src/systems/rendering/legacy_3d/smJpeg.cpp` | usa `__FILE__`; mover alteraria valor observável |
| `SrcServer/src/systems/rendering/legacy_3d/smJpeg.cpp` | usa `__FILE__`; mover alteraria valor observável |
| `SrcGame/src/systems/audio/playback/legacy/Wave.cpp` | include local `Wave.h`; preservar prioridade de lookup |
| `SrcGame/src/systems/audio/voice/MicVolume.cpp` | include local `MicVolume.h`; preservar prioridade de lookup |
| `SrcGame/src/systems/inventory/legacy/sinItem.cpp` | include local `item_catalog_recovered.inc`; preservar diretório-base |
| `SrcGame/src/systems/social/clan/*.cpp` listados no manifesto como `direct-exact` | vários headers locais (`tjclan.h`, `cE_*.h`, etc.); mover mudaria lookup por source directory |
| `SrcServer/src/systems/inventory/legacy/item_code_map_mingw.inc` | fragmento incluído; restaurado diretamente porque faz parte do código efetivo e perdeu entradas Tier 5 no porte |

A lista exata, não abreviada, está em `ORACLE_PARITY_SUMMARY.json`.

## Build graph — SQLite pós-refactor

### O que foi encontrado

O porte acrescentou:

```text
SrcServer/src/systems/persistence/database/sqlite/SQLiteConnection.cpp
```

Esse módulo não existe no `pre_refactor`, mas faz parte da próxima etapa planeada do projeto.

### Decisão atual

- os ficheiros/classe SQLite permanecem na árvore e no source graph normal do servidor;
- nenhuma integração nova com base de dados é introduzida nesta reconstrução;
- o módulo é tratado como infraestrutura futura/dormente enquanto não alterar o fluxo funcional atual;
- a exclusão temporária que existiu no Marco R0 foi removida após confirmação do owner do projeto.

Isto é uma **exceção arquitetural autorizada**, não uma tentativa de fazer o `pre_refactor` usar SQLite.

## Auditoria de include shadowing

O cliente usa `-IShared` antes de `-ISrcGame/src`. Por isso foi auditado se algum include do `pre_refactor` que originalmente resolvia para `SrcGame/src/...` passou a resolver primeiro para um novo ficheiro de `Shared/...`.

Resultado desta auditoria no Marco R0:

```text
client new Shared shadow candidates: 0
server new Shared shadow candidates: 0
```

Os ficheiros com includes locais que dependem da pasta física do source foram colocados em `direct-exact`, conforme tabela acima.

## Validação mecânica

Foi adicionado `validate_oracle_parity.py`. O validador verifica:

1. cada hash SHA-256 do oracle contra `ORACLE_PARITY_MANIFEST.tsv`;
2. que cada wrapper contém somente o include estrutural esperado;
3. que nenhum oracle movido contém `__FILE__`, `__BASE_FILE__`, `__INCLUDE_LEVEL__` ou include relativo `./../` que torne a mudança de diretório observável;
4. hashes dos ficheiros `direct-exact`;
5. exclusão do SQLite pós-refactor no build graph de servidor;
6. consistência entre manifesto e resumo.

Resultado atual:

```text
Oracle entries : 288
Wrappers       : 265
Direct exact   : 23
RESULT         : PASS
```

Os scripts de build/link foram ainda validados com `bash -n`; cliente e servidor passaram a validação sintática.

## Whitespace legacy

`git diff --check` não é critério de sucesso deste marco porque vários ficheiros oracle possuem trailing whitespace/newline patterns já presentes no `pre_refactor`. Esses bytes **não são normalizados** de propósito. Corrigi-los seria uma alteração textual sem necessidade funcional e destruiria a identidade byte-a-byte usada neste marco.

## Limitação de validação deste ambiente

Não existe `i686-w64-mingw32-g++` neste ambiente de reconstrução. Portanto:

- a paridade source/oracle foi validada mecanicamente;
- o build graph foi auditado estaticamente;
- scripts passaram `bash -n`;
- **compilação, link e runtime Windows ainda não foram executados aqui**.

O primeiro build/runtime real deve ser registado nesta secção sem reinterpretar falhas como oportunidade de corrigir comportamento legacy. Se uma falha vier de adaptação estrutural, corrige-se apenas essa adaptação; se vier do próprio `pre_refactor`, é documentada e preservada.

## BUG-040 — pendência legacy preservada

**Classificação:** `BASELINE-PENDING — preservar exatamente como no pre_refactor`  
**Subsistema:** source/localization.  
**Comportamento:** existem caracteres `U+FFFD` (`�`) e casos de mojibake já gravados no baseline funcional.  
**Risco conhecido:** há perda real de informação textual em parte da source.  
**Decisão desta reconstrução:** não corrigir encoding como parte do porte estrutural. Qualquer saneamento futuro deve ser uma mudança separada e opt-in, nunca misturada com equivalência funcional.


## ARCH-002 — SQLite futuro preservado no build

**Local:** `SrcServer/src/systems/persistence/database/sqlite/SQLiteConnection.cpp` e `build_server_mingw_i686_j8.sh`

**Classificação:** infraestrutura futura permitida; não é comportamento legacy a restaurar.

**Decisão:** a exclusão temporária de `SQLiteConnection.cpp` do build de paridade foi removida. O módulo faz parte da próxima etapa do projeto e permanece na estrutura/build atual. Nesta fase ele é considerado dormente: não se altera a API, não se liga a uma base de dados e não se adicionam efeitos de runtime para tentar aproximá-lo do `pre_refactor`.

**Regra:** a presença de infraestrutura nova que não modifica o fluxo funcional atual é permitida. Se futuramente o módulo passar a alterar inicialização, persistência ou comportamento do servidor, essa integração será tratada como etapa separada e não como parte da reconstrução de paridade.

# Marco R1 — Estrutura ativa com paridade funcional

## Estado atual da reconstrução

O Marco R0 usava `Shared/legacy_oracle` como caminho de execução temporário. No Marco R1 isso deixou de acontecer.

Classificação atual dos 288 ficheiros de source que diferiam no porte:

- **178 `structured-safe`** — usam diretamente a estrutura/abstrações do porte; o histórico completo do path contém somente extrações/deduplicações classificadas como byte-equivalent/legacy/safe, sem commit posterior de `fix`, `harden`, `validate`, `recover` ou integração Tier5 que altere o mesmo path/dependência ativa.
- **87 `structured-exact`** — o target permanece no path moderno, mas o corpo/declaration efetivo foi movido para o domínio moderno correspondente em `Shared/...` e é byte-a-byte igual ao ficheiro do `pre_refactor` (SHA-256 preso em `STRUCTURAL_EXACT_MOVES.tsv`).
- **23 `direct-exact`** — permanecem no próprio path target porque movê-los alteraria lookup local, `__FILE__` ou outro contexto de preprocessamento.
- **0 `oracle-wrapper` ativos** — `Shared/legacy_oracle` é somente referência de validação; nenhum source ativo o inclui.

Os 1204 ficheiros `.cpp/.c/.h/.hpp/.inc` do `pre_refactor` estão presos por `PRE_REFACTOR_SOURCE_MANIFEST.tsv`. `validate_full_source_parity.py` valida hashes, movimentos estruturais e o source graph.

## Source graph

- nenhuma translation unit existente no `pre_refactor` foi removida;
- uma translation unit nova é permitida: `SrcServer/src/systems/persistence/database/sqlite/SQLiteConnection.cpp`;
- SQLite é infraestrutura futura autorizada e atualmente não possui caller no source ativo;
- os quatro scripts `build_*` / `link_*` são byte-a-byte iguais aos scripts fornecidos em `pre_refactor.zip`.

## Auditoria de resolução de includes após movimentação

Para cada `structured-exact`, foi comparada a resolução de todo `#include "..."` no diretório original e no novo ficheiro de `Shared`, usando a ordem de `-I` dos scripts MinGW fornecidos.

Resultado:

- **0 includes mudaram para outro ficheiro**;
- os includes não resolvidos dentro deste workspace também não resolvem no ZIP original fornecido e pertencem a dependências externas não incluídas nos artefactos (`modules`, `Dependency`, etc.); portanto não são consequência da movimentação.

## Auditoria de dependências Shared alteradas por hardening

Foi gerado um grafo recursivo de quoted-includes a partir das translation units cliente/servidor e comparado com todos os ficheiros `Shared` tocados por commits com `fix`, `harden`, `validate`, `integrate`, `recover` ou equivalentes.

Dependências indiretas inicialmente encontradas e retiradas do caminho ativo:

- `Shared/network/transport/legacy/SmWSockLegacyShared.h` → headers `smwsock.h` target restaurados como `structured-exact`;
- `Shared/gameplay/skills/TempskronLegacyCommon.h` → headers `Tempskron.h` target restaurados como `structured-exact`;
- `Shared/gameplay/skills/MorayionLegacyCommon.h` e `MorayionLegacySkillFunctions.inl` → `Morayion` servidor restaurado como `structured-exact`;
- `Shared/security/integrity/IsaoCheckLegacyCommon.inl` → `isaocheck.cpp` cliente/servidor restaurados como `structured-exact`;
- `Shared/rendering/effects/AssaUtilLegacyCommon.inl` → `AssaUtil.cpp` cliente/servidor restaurados como `structured-exact`.

Depois dessa correção estrutural, o único ficheiro dessa categoria ainda alcançável é `Shared/smPacket.h`. Ele foi verificado contra o ZIP funcional e possui exatamente o mesmo SHA-256 do `pre_refactor`: `3f131b89558f3981f8ad99e0a99b02c91c558e651952342d5036bb7ec01be912`. Portanto não existe mudança funcional nesse header.

# Legacy quirks confirmados e deliberadamente preservados

Os itens abaixo são comportamentos adicionais observados durante a comparação direta. Eles **não devem ser corrigidos durante esta reconstrução**, mesmo quando parecem incorretos. Alterá-los exige uma etapa funcional futura separada, com teste dentro do jogo.

## LQ-001 — `.inx` aceita motion counts maiores que a tabela NPC

**Local:** `fileread.cpp`, model/motion config, `smMODELINFO`.

O formato/runtime legacy possui `MotionInfo` com capacidade muito superior à tabela `NpcMotionRate`. Caches reais observados possuem 63, 64, 78 e 117 motions. O código legacy não trata `NPC_MOTION_INFO_MAX` como limite global de `MotionCount` e pode acessar a tabela de rates de forma permissiva/insegura. O porte transformou o limite de 30 em validação global e fez personagens desaparecerem. Nesta reconstrução vale exatamente a semântica do `pre_refactor`; não é introduzido limite novo.

## LQ-002 — `.smd` full-load não usa metadata auxiliar como invariantes rígidos

**Local:** `smPAT3D` / `smRead3d` / pattern serialization.

O loader funcional lê os objetos de um pattern sequencialmente. Campos históricos como `ObjFilePoint`, `Length`, `MatFilePoint` e `First_ObjInfoPoint` não são todos tratados como condições obrigatórias para aceitar o asset. O porte passou a rejeitar metadata stale mesmo quando o conteúdo sequencial era carregável. Essa rejeição não existe no caminho ativo R1.

## LQ-003 — Material serializer é permissivo com `MaxMaterial` e padding

**Local:** `smMATERIAL_GROUP`, `smTexture.cpp`.

O baseline usa `MaterialCount` para carregar/alocar materiais e não exige a relação rígida `MaterialCount <= MaxMaterial` introduzida no porte. Também não exige consumo exato do bloco histórico de nomes/texturas. Padding/metadata legacy continuam tolerados.

## LQ-004 — `smOBJ3D` não trata `MaxVertex`/`MaxFace` serializados como invariantes

**Local:** `smOBJ3D::LoadFile` / object serialization.

O comportamento funcional aloca/lê principalmente a partir de `nVertex`, `nFace` e `nTexLink`. `MaxVertex`/`MaxFace` serializados podem estar stale. O porte passou a rejeitar objetos quando `nVertex > MaxVertex` ou `nFace > MaxFace`; essa validação não existe no caminho ativo R1.

## LQ-005 — `smPAT3D::AddObject` possui cálculo legacy peculiar de `MaxFrame`

**Local:** `smPAT3D::AddObject`.

O baseline não calcula simplesmente `max(rotation, position, scale)`. O track de position pode sobrescrever o frame vindo de rotation e scale não participa da mesma forma. O porte normalizou isso para um máximo matematicamente mais intuitivo, alterando duração/loop de animação. R1 preserva o cálculo antigo.

## LQ-006 — Scale track de índice zero funciona como sentinela em decisões legacy

**Local:** object animation / track sampling.

Alguns caminhos legacy testam `NumTmScale > 0`; portanto índice/offset zero pode significar “sem animação” naquela decisão. O porte mudou a condição para aceitar zero (`>= 0`). R1 mantém a condição histórica.

## LQ-007 — `GetEmptyHeight` sem faces devolve a sentinela legacy

**Local:** `smSTAGE3D::GetEmptyHeight`.

Quando a consulta não encontra faces, o baseline mantém o retorno/sentinela usado pelo algoritmo antigo (`CLIP_OUT` no caminho observado), em vez de normalizar o resultado para `0`. O valor é preservado porque callers podem distinguir “sem superfície” de altura zero.

## LQ-008 — `CheckVecImpact` sem faces não avança `Posi`

**Local:** `smSTAGE3D::CheckVecImpact`.

No baseline, se não houver faces relevantes, a função retorna sucesso conforme o fluxo antigo sem atualizar `Posi` para o endpoint calculado. O porte passou a avançar a posição. R1 mantém o comportamento original.

## LQ-009 — Fórmula de collision possui componentes Y/Z deliberadamente estranhas

**Local:** `GetTriangleImact` / collision stage.

Num dos sentidos do teste, a fórmula legacy produz deltas Y/Z equivalentes a zero enquanto X é invertido/ajustado. Isso parece erro matemático e foi “corrigido” no porte, mas altera hitbox/collision. É preservado e também corresponde ao espírito do BUG-033 do ledger.

## LQ-010 — `CheckSolid` mantém o algoritmo geométrico histórico

**Local:** `smSTAGE3D::CheckSolid`.

O refactor trocou partes do teste por helpers de interseção mais gerais. R1 executa o algoritmo original completo, incluindo ordem de testes, sentinelas e early returns.

## LQ-011 — Falha de `ReformTexture` não implica ocultar o objeto

**Local:** pattern/object render.

O fluxo funcional chamava `ReformTexture` e continuava para o render conforme a lógica original. O porte passou a usar o retorno do novo helper como gate para `continue`, podendo fazer um objeto inteiro desaparecer. R1 mantém o fluxo original.

## LQ-012 — Parsing numérico legacy usa conversões permissivas

**Local:** `fileread.cpp` e configs legacy.

Vários tokens passam por `atoi`/conversões C permissivas; textos como prefixo numérico seguido de caracteres podem ser aceitos de forma diferente de um parser estrito. Os readers novos do porte usam validações mais rígidas (`strtol`, cursores com erro, bounds). Esses readers permanecem como scaffolding, mas não substituem o parser ativo R1.

## LQ-013 — Checksum/path de model cache possui semântica histórica dependente do fluxo

**Local:** `.ini/.inx` model cache.

O código histórico contém tratamento de paths/separadores associado ao fluxo MinGW/cache e não é perfeitamente simétrico em todos os pontos de read/write. Durante a depuração anterior, “normalizar” esse comportamento em helpers novos produziu incompatibilidade com caches existentes. R1 usa o fluxo completo do `pre_refactor`, sem tentar uniformizá-lo.

## Regra para estes LQ

`LQ-*` significa **Legacy Quirk preservado**. É documentação, não backlog automático. Nenhum LQ deve ser alterado durante o porte estrutural. Qualquer correção futura precisa de branch/commit separado, teste A/B no jogo e decisão explícita do owner.

# Marco R2 — Paridade estrutural provada no grafo ativo

Este marco substitui as contagens provisórias do R1. O princípio continua inalterado: `pre_refactor` é a autoridade funcional; a árvore do porte é a autoridade estrutural.

## Classificação final dos 288 paths alterados pelo porte

- **66 `structured-safe`** — o ficheiro target usa abstrações/extracções `Shared` e, ao expandir recursivamente essas extracções no ponto dos `#include`, o fluxo de tokens reconstruído é idêntico ao oracle `pre_refactor` correspondente.
- **89 `structured-exact`** — o path target é wrapper estrutural e a implementação movida em `Shared/...` possui SHA-256 idêntico ao source/header funcional original.
- **133 `direct-exact*`** — o conteúdo continua byte-a-byte no path target. Este grupo inclui casos conservadores onde a abstração original do porte não foi provada equivalente; manter o source funcional tem prioridade sobre activar uma abstração não provada.
- **0 `oracle-wrapper` activos** — `Shared/legacy_oracle` é somente evidência/fixture para validação e não participa no runtime.

## PROOF-001 — Todas as extracções Shared activas foram provadas contra o pre_refactor

**Escopo:** grafo recursivo de includes a partir das translation units cliente/servidor descobertas pelos scripts MinGW.

O grafo activo alcança **179 ficheiros em `Shared`**:

- **17** já existiam no `pre_refactor` e são byte-a-byte idênticos;
- **85** são implementações/declarations movidas exactas (`*_legacy_client/server.*`) presas por hash;
- **77** são helpers estruturais novos do porte.

Os 77 helpers novos foram canonicalizados por lexer preservando strings/chars/preprocessor relevante e todos aparecem como **blocos de tokens contíguos** em ficheiros do `pre_refactor`. Resultado: **77/77 PASS**.

Isto é mais forte do que confiar no nome do commit `share legacy`: qualquer lógica nova inserida num helper faria a prova falhar.

## PROOF-002 — Os 66 structured-safe reconstroem o source funcional completo

Para cada path `structured-safe`, os includes que apontam para helpers `Shared` novos são expandidos recursivamente no mesmo ponto do ficheiro. O fluxo de tokens resultante é comparado com a cópia oracle byte-exacta do `pre_refactor`.

Resultado: **66/66 PASS**.

Assim, estes 66 paths mantêm as abstrações do porte sem reescrever a lógica. A fronteira de ficheiro muda; a sequência funcional de declarations/statements/macros permanece equivalente.

## PROOF-003 — Relocação structured-exact não muda resolução de include local

Foram auditados todos os `#include "..."` contidos nas 89 implementações movidas para `Shared`.

- includes verificados: **474**;
- resolução alterada entre a localização original e a localização movida: **0**.

Isto protege contra um problema silencioso comum ao mover monólitos: um include local passar a resolver outro header com o mesmo nome devido ao novo directório.

## PROOF-004 — Build/link scripts preservados

Os quatro scripts usados como referência continuam byte-a-byte iguais aos fornecidos no `pre_refactor`:

- `build_client_mingw_i686_multicore.sh`;
- `build_server_mingw_i686_j8.sh`;
- `link_client_mingw_i686.sh`;
- `link_server_mingw_i686.sh`.

Não foram introduzidas flags, macros, ordem de `-I` ou bibliotecas como parte desta reconstrução.

## ARCH-003 — Abstrações não provadas permanecem dormentes, não são apagadas

As classes/helpers criados pelo porte continuam fisicamente na árvore. Quando não há prova de equivalência, o runtime usa a implementação funcional exacta e a abstração permanece dormente.

Isto é deliberado: o objectivo final continua a ser a arquitectura do porte. A promoção futura deve seguir `direct-exact -> structured-exact -> structured-safe` e só ocorre quando a extracção pode ser demonstrada sem mudança funcional.

## Regra R2 para novos trabalhos

Nenhum helper novo pode entrar no grafo activo apenas porque compila ou porque parece uma melhoria. Para promoção, deve satisfazer pelo menos uma das provas:

1. implementação movida com hash exacto do oracle + include-resolution equivalente; ou
2. extracção estrutural cujo caller expandido reconstrói token-a-token o oracle funcional.

Bugs/quirks observados durante estas provas continuam a ser anotados como `BUG-*`/`LQ-*`, nunca corrigidos dentro desta reconstrução.

# Marco R3 — `fileread.cpp` dividido sem reescrita funcional

R3 avança a arquitectura a partir da baseline R2 já provada. O primeiro monólito crítico escolhido foi `fileread.cpp`, porque concentra model cache, configs, items, character decode, spawn, Belatra e cracker parsing — precisamente áreas onde o porte anterior tinha introduzido validações e parsers novos.

## STRUCT-001 — Fileread client/server repartido por domínio com expansão byte-exacta

**Targets:**

- `SrcGame/src/core/data/legacy_loader/fileread.cpp`;
- `SrcServer/src/core/data/legacy_loader/fileread.cpp`.

**Método:** o preâmbulo original (includes, globals iniciais e `SetNPCMsg`) permanece no target. Todo o restante source foi cortado em **blocos contíguos**, sem editar statements, assinaturas, constantes ou algoritmos. Esses blocos foram colocados nos directórios de formato já existentes no porte:

- `Shared/formats/model_config/LegacyModelConfigExact_{client,server}.inl`;
- `Shared/formats/server_config/LegacyServerConfigExact_{client,server}.inl`;
- `Shared/formats/legacy_config/LegacyMainConfigExact_{client,server}.inl`;
- `Shared/formats/item_config/LegacyItemDefinitionExact_{client,server}.inl`;
- `Shared/formats/character_config/LegacyCharacterConfigExact_{client,server}.inl`;
- `Shared/formats/stage_spawn/LegacyStageSpawnExact_{client,server}.inl`;
- `Shared/formats/item_config/LegacyItemRuntimeExact_{client,server}.inl`;
- `Shared/formats/model_config/LegacyModelKeywordExact_{client,server}.inl`;
- `Shared/formats/belatra/LegacyBelatraExact_{client,server}.inl`;
- `Shared/formats/security/LegacyCrackerConfigExact_{client,server}.inl`.

São **20 chunks** no total (10 cliente + 10 servidor).

**Prova principal:** ao substituir cada `#include` do wrapper pelo conteúdo do chunk correspondente, o resultado é **byte-a-byte idêntico** ao `fileread.cpp` do `pre_refactor`, para cliente e servidor. Não se trata apenas de equivalência tokenizada.

**Includes internos:** existem dois includes dentro dos chunks de item runtime (`b_JobCode.h`). Foi comparada a resolução a partir do local original e do novo local em `Shared`; ambos continuam a resolver exactamente o mesmo path target.

## Decisão sobre os readers/classes novos do porte

As classes/readers existentes (`ModelCacheReader`, `CharacterConfigReader`, `ServerConfig*Reader`, `ItemRuntimeBuilder`, `StageMonsterConfigReader`, etc.) **não são apagados**. Continuam como estrutura/scaffolding do porte, mas nesta baseline não substituem o código funcional enquanto a implementação deles não puder ser demonstrada equivalente.

O objectivo arquitectural continua a existir; a prioridade de R3 é mover o monólito para os domínios certos sem repetir a reescrita semântica que quebrou caches/personagens.

## TOOL-001 — Validador structured-safe tinha path absoluto de workspace

Durante a promoção R3 foi detectado que `validate_structured_safe_parity.py` ainda apontava para `/mnt/data/rebuild_v0/SourceCode`. Isso fazia uma execução R3 validar acidentalmente o checkpoint R2 e reportar 66 paths em vez dos 68 actuais.

**Correcção de tooling:** o validador agora usa `Path(__file__).resolve().parent`. Isto não altera source/runtime; apenas garante que a prova é executada contra a árvore onde o script está localizado.

## Estado R3

Dos 288 paths que diferiam no porte:

- **68 structured-safe** (inclui agora os dois `fileread.cpp` modularizados);
- **87 structured-exact**;
- **133 direct-exact***;
- **0 oracle-wrapper activos**.

Grafo `Shared` activo:

- **197** ficheiros;
- **17** baseline Shared byte-exact;
- **83** implementations/declarations movidas exactas;
- **97** helpers/chunks novos com prova de extracção token-contígua (**97/97 PASS**).

Prova de caller expandido: **68/68 PASS**.

Includes quoted nas 87 implementações `structured-exact`: **444 verificados, 0 mudanças de resolução**.

# Marco R4 — equivalência por macro/policy sem abandonar as abstrações

R4 estende a regra de paridade para casos em que a arquitectura do porte usa macros ou pequenas policies target-specific. Nesses casos a comparação textual simples não é suficiente; o validador expande a abstração e exige que o source resultante seja equivalente ao `pre_refactor`.

## TOOL-002 — Validadores de macro/policy tornados portáveis

`validate_structural_macro_equivalence.py` e `validate_structural_policy_equivalence.py` deixaram de consultar `/mnt/data/rebuild_pre`. A distinção entre `Shared` baseline e `Shared` criado pelo porte é agora derivada de `PRE_REFACTOR_SOURCE_MANIFEST.tsv`, que viaja com o checkpoint.

Isto é tooling apenas; não altera source/runtime.

## PROOF-005 — Macro-equivalent targets

O porte mantém macros target-specific quando elas apenas representam tokens que já eram diferentes no cliente/servidor. O validador remove a definição estrutural, substitui o alias pelo valor target e exige equivalência com o oracle.

Actualmente: **8/8 PASS**, usando **4 helpers congelados por SHA-256**.

Abrange:

- `sinSkillEffect.cpp` — `LEGACY_SHIELD_STRIKE_MATERIAL_COUNT`;
- `HoParticle.h` — vector Direct3D target-specific;
- `isaocheck.h` — include/header target-specific;
- `HoLogin.h` — tipo de surface target-specific e macro `MAX_LOADING_IMAGE` apenas relocada.

### LQ-R4-001 — `MatShieldStrike` mantém capacidade diferente cliente/servidor

No `pre_refactor`, o cliente declara `MatShieldStrike[2]` enquanto código legacy também alcança o índice 2; o servidor declara `[3]`. Apesar de o caso cliente parecer overflow, **não foi corrigido**. A macro estrutural conserva exactamente 2 no cliente e 3 no servidor.

## PROOF-006 — Policy-equivalent targets

Foi criado um proof que expande helpers `Shared`, remove a struct policy, inlineia funções `static` simples e aliases `using`, e compara o fluxo de tokens resultante com o `pre_refactor`.

Actualmente: **10/10 PASS**, usando **5 helpers congelados por SHA-256**.

Abrange:

- `MicVolume.cpp` cliente/servidor;
- `smMap3d.cpp` cliente/servidor;
- `sinWarpGate.cpp` cliente/servidor;
- `LowLevelPetSystem.cpp` cliente/servidor;
- `PCBangPetSystem.cpp` cliente/servidor.

### LQ-R4-002 — MicVolume mantém casts diferentes por target

A policy conserva as expressões originais: cliente usa o caminho `DWORD/std::min<DWORD>`, servidor conserva casts para `long` e as declarações `using std::min/std::max` originais. Não houve unificação.

### LQ-R4-003 — smMap3d mantém macro de cor diferente por target

Cliente continua a usar `D3DCOLOR_RGBA`; servidor continua a usar `RGBA_MAKE`. A policy apenas encapsula a diferença e é provada por inline.

### LQ-R4-004 — sinWarpGate mantém capitalização legacy diferente por target

O `pre_refactor` não usa as mesmas strings de asset:

- cliente: `image//Sinimage//help//WG_*.tga`;
- servidor: `Image//SinImage//Help//WG_*.tga`.

Mesmo que Windows costume ser case-insensitive, **não foi normalizado**. Também foi restaurada a ordem legacy de includes com `field.h` antes de `onserver.h`.

### LQ-R4-005 — pets mantêm tipo vector target-specific e ordem de declaração/include

`LowLevelPetSystem` e `PCBangPetSystem` conservam:

- cliente: `D3DXVECTOR3`;
- servidor: `D3DVECTOR`.

As classes `LegacyLowLevelPetPolicy` e `LegacyPCBangPetPolicy` continuam na arquitectura. O proof substitui `Policy::Vector3` pelo tipo original e obtém o oracle token-a-token.

Além disso, `HoPetRenderPoint`/`HoPcBangPetRenderPoint` voltaram a ser declarados **antes** de `damage.h`, exactamente como no source funcional. A declaração não foi deixada dentro do helper só por conveniência estrutural.

## LQ-R4-006 — HoAnimData não propaga includes exclusivos do servidor ao cliente

O helper comum deixou de possuir `<cstring>`/`<stdio.h>` só porque o servidor os tinha. Esses includes permanecem no wrapper servidor. O cliente conserva o conjunto de includes do `pre_refactor`.

## Estado R4 após estas promoções

Dos 288 paths que tinham sido alterados pelo porte:

- **130 structured-safe**;
- **86 structured-exact**;
- **8 structured-macro-equivalent**;
- **10 structured-policy-equivalent**;
- **54 direct-exact***;
- **0 oracle-wrapper activos**.

Grafo `Shared` activo: **230 ficheiros**. Os helpers macro/policy que deixam de ser extracções contíguas simples têm prova dedicada e hash congelado.

---

# Marco R5 — paridade estrutural fechada

## Estado atual

O conjunto de **288 ficheiros** cujo conteúdo tinha mudado entre o `pre_refactor` e a árvore do porte está agora classificado assim:

| Modo | Quantidade | Significado |
|---|---:|---|
| `structured-safe` | 135 | abstração/extração ativa cuja expansão reconstrói o oracle funcional |
| `structured-exact` | 101 | implementação target-specific byte-exata movida para o domínio moderno em `Shared/` |
| `structured-macro-equivalent` | 10 | macros target-specific provadas equivalentes ao código original |
| `structured-policy-equivalent` | 41 | policy/class adapter provado equivalente por inline estrutural |
| `direct-exact-port-native` | 1 | o próprio porte mantém este ficheiro monolítico no mesmo path; apenas o corpo funcional foi restaurado |
| `oracle-wrapper` | 0 | nenhuma camada oracle é executada no runtime |

O único `direct-exact-port-native` é:

```text
SrcGame/src/systems/inventory/legacy/sinItem.cpp
```

Isto **não é dívida arquitetural**: `final_07` também mantém `sinItem.cpp` nesse mesmo path, sem uma abstração correspondente que precise ser reativada. A reconstrução preserva o corpo do `pre_refactor`, incluindo o `memset`/`sizeof` legacy documentado no ledger.

## Fecho dos últimos casos sensíveis

### `smJpeg.cpp` — cliente e servidor

**Problema estrutural:** o source usa:

```cpp
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
```

Mover o ficheiro inteiro para `Shared` alteraria o valor observável de `__FILE__`.

**Solução R5:**

- `windows.h`, `smJpeg.h`, `math.h` e o bloco `THIS_FILE` permanecem no `.cpp` target original;
- somente o corpo posterior (`CJpeg::CJpeg()` em diante) foi movido para:
  - `Shared/rendering/legacy_3d/SmJpegLegacyBody_client.inl`;
  - `Shared/rendering/legacy_3d/SmJpegLegacyBody_server.inl`;
- cada body é um sufixo contíguo byte-exato do respetivo `pre_refactor`;
- a expansão do wrapper é token-equivalente ao oracle;
- nenhuma correção de JPEG, bounds checking, ownership ou parsing foi introduzida.

### `GuideHelp.cpp` cliente

**Problema estrutural:** o ficheiro depende da posição exata de `GUIDEHELP_CPP`, de `#ifdef USE_PROSTONTALE` e de vários headers locais de clan.

**Solução R5:**

- todo o preâmbulo/defines/includes permanece no `.cpp` original;
- o corpo a partir de `extern int GuideHelpButton[3];` foi movido byte-exato para:
  - `Shared/social/clan/GuideHelp_legacy_client_body.inl`;
- a abstração `GuideHelpLegacyCommon.inl` do porte permanece na árvore, mas não é usada quando ela deslocaria diretivas/preprocessing em relação ao oracle;
- paths, desenho, chamadas de texto e ordem de side effects permanecem os do `pre_refactor`.

### `clan_Enti.cpp` cliente

**Problema estrutural:** a policy `LegacyClanEntityPolicy` do porte introduzia um `TextContext` dummy e substituições de path/texto que não são textualmente nem estruturalmente idênticas ao oracle.

**Solução R5:**

- defines e todos os includes locais permanecem no `.cpp` original;
- o corpo a partir de `extern char szConnServerName[16];` foi movido byte-exato para:
  - `Shared/social/clan/ClanEntity_legacy_client_body.inl`;
- a policy genérica continua disponível como scaffolding arquitetural, mas não entra no runtime deste target enquanto não puder ser equivalente sem adicionar estado/fluxo.

## Grafo de fontes do build

A descoberta de sources dos scripts foi reproduzida sem depender do toolchain:

```text
Cliente — SrcGame/src, excluindo modules externos:
pre_refactor: 201
R5         : 201

Servidor — SrcServer/src, excluindo modules externos:
pre_refactor: 179
R5         : 180
```

A única adição no servidor é explicitamente autorizada:

```text
SrcServer/src/systems/persistence/database/sqlite/SQLiteConnection.cpp
```

No repositório real do owner, os `modules/MD5` acrescentam as mesmas fontes externas ao cliente que já eram usadas antes; esses diretórios não fazem parte dos ZIPs de referência recebidos.

## Grafo ativo de ficheiros novos

Auditoria recursiva de includes a partir das translation units atuais:

- ficheiros ativos que não existiam no baseline: 250;
- dentro de `Shared`: 248, todos cobertos por hash + prova de extração/macro/policy/implementação exata;
- fora de `Shared`: **2**, ambos SQLite autorizados:
  - `SQLiteConnection.cpp`;
  - `SQLiteConnection.h`.

Nenhum outro header novo em `SrcGame/src`/`SrcServer/src` está a entrar silenciosamente no runtime.

## SQLite — infraestrutura futura autorizada

`SQLiteConnection.cpp/.h` permanecem na árvore e no build do servidor conforme decisão do owner.

No estado R5:

- não existem callers de `SQLiteConnection` fora dos próprios ficheiros SQLite;
- não é adicionada integração de base de dados nesta reconstrução;
- o source carrega `sqlite3.dll` dinamicamente somente quando uma instância for usada;
- esta exceção não autoriza qualquer alteração aos fluxos ODBC/DB funcionais atuais.

## Novos validadores R5

Além das provas anteriores:

```bash
python3 validate_build_source_graph.py
python3 validate_active_nonbaseline_graph.py
```

`validate_build_source_graph.py` bloqueia remoção/adição de TUs descobertas pelos scripts, exceto SQLite.

`validate_active_nonbaseline_graph.py` percorre o grafo de includes e bloqueia novos ficheiros ativos fora de `Shared`, exceto `SQLiteConnection.cpp/.h`.

## Resultado estático R5

```text
288/288 paths alterados classificados
oracle wrappers ativos: 0
build source graph: PASS
active non-baseline graph: PASS
Shared proof: PASS
structured-safe flatten: PASS
macro equivalence: PASS
policy equivalence: PASS
structured-exact include resolution: PASS
full source parity: PASS
build/link shell syntax: PASS
```

## O que R5 NÃO declara

R5 não declara compile/link/runtime PASS porque o ambiente desta reconstrução não contém o toolchain/dependências MinGW i686 completos do repositório real.

O primeiro build no ambiente real deve ser tratado como **teste de integração estrutural**. Um erro de compilação/linkagem não autoriza corrigir o comportamento legacy: primeiro deve ser investigado como include, declaration, linkage, source graph ou contexto de preprocessamento perdido durante o movimento.

## Prova de preservação da estrutura do porte

Foi congelado `PORT_STRUCTURE_MANIFEST.tsv` a partir do inventário completo de `final_07`.

Resultado R5:

```text
ficheiros estruturais esperados: 1517
ficheiros estruturais ausentes : 0
RESULT                        : PASS
```

Isto garante que as reversões funcionais não apagaram abstrações/classes/scaffolding do porte. Helpers que não podem substituir o oracle sem mudar comportamento continuam presentes, apenas dormentes.

## Scripts de build/link

Os quatro scripts fornecidos no baseline foram preservados byte-a-byte e estão congelados em `BASELINE_BUILD_SCRIPT_HASHES.tsv`:

- `build_client_mingw_i686_multicore.sh`;
- `build_server_mingw_i686_j8.sh`;
- `link_client_mingw_i686.sh`;
- `link_server_mingw_i686.sh`.

O validador `validate_build_scripts.py` exige os hashes baseline e falha se flags, defines, ordem de include ou lógica de source discovery forem modificados.


# R6 — paridade estrita de preprocessing / `#pragma once`

## BUG-R6-001 — `#pragma once` adicionado/duplicado pelo porte

**Classe:** regressão estrutural de preprocessing.  **Regra aplicada:** preservar exatamente a semântica de reinclusão do `pre_refactor`; não tratar `#pragma once` como whitespace/cosmética.

Durante a validação R5, o lexer de equivalência ignorava deliberadamente `#pragma once`. Uma segunda auditoria estrita mostrou **50 targets `structured-safe`** em que o fluxo funcional era equivalente, mas o porte tinha acrescentado um pragma no wrapper. Isto podia alterar comportamento em headers legacy sem guard, com guard iniciado depois de includes, ou deliberadamente reincluíveis.

**Correção estrutural R6:** foi removido **somente o `#pragma once` adicional do wrapper target**. Nenhum pragma foi adicionado ao oracle e nenhum algoritmo/corpo/tipo/constante foi alterado. Quando o `pre_refactor` já possuía um pragma (`RingBuff.h`, `ZipLib.h`, `MicVolume.h`, `smJpeg.h`), o pragma original continua no helper compartilhado que contém o corpo legacy; apenas a duplicação do wrapper foi removida. Quando o baseline não possuía pragma, o wrapper voltou a não possuir pragma.

Targets afetados (50):

- `SrcGame/src/systems/audio/download/legacy_http/RingBuff.h`
- `SrcGame/src/systems/audio/download/legacy_http/ZipLib.h`
- `SrcGame/src/systems/audio/effects/legacy/effectsnd.h`
- `SrcGame/src/systems/audio/playback/legacy/SHELL.H`
- `SrcGame/src/systems/audio/playback/legacy/Wave.h`
- `SrcGame/src/systems/audio/voice/MicVolume.h`
- `SrcGame/src/systems/gameplay/pets/LowLevelPetSystem.h`
- `SrcGame/src/systems/gameplay/pets/PCBangPetSystem.h`
- `SrcGame/src/systems/gameplay/pets/legacy_sin/sinParkPcBangPet.h`
- `SrcGame/src/systems/gameplay/quests/legacy/sinQuest.h`
- `SrcGame/src/systems/rendering/effects/legacy_new/core/HoEffectResMgr.h`
- `SrcGame/src/systems/rendering/effects/legacy_new/core/HoEffectUtil.h`
- `SrcGame/src/systems/rendering/effects/legacy_new/core/HoEffectView.h`
- `SrcGame/src/systems/rendering/effects/legacy_new/math/COLOR3.H`
- `SrcGame/src/systems/rendering/effects/legacy_new/math/MATRIX4.H`
- `SrcGame/src/systems/rendering/effects/legacy_new/math/POINT4.H`
- `SrcGame/src/systems/rendering/effects/legacy_new/math/bSphere3.h`
- `SrcGame/src/systems/rendering/effects/legacy_new/math/color4.h`
- `SrcGame/src/systems/rendering/effects/legacy_new/math/mathGlobal.h`
- `SrcGame/src/systems/rendering/effects/legacy_new/math/point3.h`
- `SrcGame/src/systems/rendering/effects/legacy_new/script/PiScriptLuaCommand.h`
- `SrcGame/src/systems/rendering/legacy_3d/smJpeg.h`
- `SrcGame/src/systems/rendering/legacy_3d/smRead3d.h`
- `SrcGame/src/systems/rendering/legacy_3d/smmatrix.h`
- `SrcGame/src/systems/ui/help/legacy_sin/sinHelpPet.h`
- `SrcServer/src/systems/audio/download/legacy_http/RingBuff.h`
- `SrcServer/src/systems/audio/download/legacy_http/ZipLib.h`
- `SrcServer/src/systems/audio/playback/legacy/SHELL.H`
- `SrcServer/src/systems/audio/playback/legacy/Wave.h`
- `SrcServer/src/systems/audio/voice/MicVolume.h`
- `SrcServer/src/systems/gameplay/pets/LowLevelPetSystem.h`
- `SrcServer/src/systems/gameplay/pets/PCBangPetSystem.h`
- `SrcServer/src/systems/gameplay/pets/legacy_sin/sinParkPcBangPet.h`
- `SrcServer/src/systems/gameplay/quests/legacy/sinQuest.h`
- `SrcServer/src/systems/rendering/effects/assa/hoAssaParticleEffect.h`
- `SrcServer/src/systems/rendering/effects/legacy_new/core/HoEffectResMgr.h`
- `SrcServer/src/systems/rendering/effects/legacy_new/core/HoEffectUtil.h`
- `SrcServer/src/systems/rendering/effects/legacy_new/core/HoEffectView.h`
- `SrcServer/src/systems/rendering/effects/legacy_new/math/COLOR3.H`
- `SrcServer/src/systems/rendering/effects/legacy_new/math/MATRIX4.H`
- `SrcServer/src/systems/rendering/effects/legacy_new/math/POINT4.H`
- `SrcServer/src/systems/rendering/effects/legacy_new/math/bSphere3.h`
- `SrcServer/src/systems/rendering/effects/legacy_new/math/color4.h`
- `SrcServer/src/systems/rendering/effects/legacy_new/math/mathGlobal.h`
- `SrcServer/src/systems/rendering/effects/legacy_new/math/point3.h`
- `SrcServer/src/systems/rendering/effects/legacy_new/script/PiScriptLuaCommand.h`
- `SrcServer/src/systems/rendering/legacy_3d/smJpeg.h`
- `SrcServer/src/systems/rendering/legacy_3d/smRead3d.h`
- `SrcServer/src/systems/rendering/legacy_3d/smmatrix.h`
- `SrcServer/src/systems/ui/help/legacy_sin/sinHelpPet.h`

### Prova R6

Foram acrescentadas provas que **não ignoram `#pragma once`**:

```text
structured-safe strict preprocessor : 135/135 PASS
macro-equivalent strict             : 10/10 PASS
policy-equivalent strict            : 41/41 PASS
```

As provas existentes continuam a verificar o restante fluxo de tokens. Como os ficheiros baseline fora dos 288 paths geridos permanecem byte-a-byte iguais, e `structured-exact` mantém a implementação byte-exact, esta auditoria também fecha a classe irmã de **include guards deslocados/renomeados** no caminho funcional: diretivas `#ifndef/#define/#endif` fazem parte do fluxo estrito e não são descartadas.

### Comportamento legacy deliberadamente preservado

Alguns headers do `pre_refactor` não possuem `#pragma once` mesmo quando adicioná-lo pareceria mais seguro. Outros possuem pragma e include guard simultaneamente. R6 **não normaliza** nenhum desses casos. A estrutura de reinclusão original é tratada como comportamento observável do programa e fica preservada.

## Estado fechado R6

```text
135 structured-safe
101 structured-exact
 10 macro-equivalent
 41 policy-equivalent
  1 direct-exact-port-native
---
288/288 classificados
0 oracle wrappers ativos
```

R6 substitui R5 como checkpoint candidato ao primeiro compile/link/runtime real. R5 deve ser considerado obsoleto porque foi empacotado antes da auditoria estrita de `#pragma once`.

# R6.1 — adaptação mínima de compilador MinGW

## BUG-R6.1-001 — `cE_chip.cpp` chama overload inexistente de `SetFontTextColor`

**Origem funcional:** `pre_refactor/SrcGame/src/systems/social/clan/cE_chip.cpp`.

**Erro observado no primeiro build real do cliente R6 (MinGW i686):**

```text
TOTAL : 203
OK    : 202
FAIL  : 1
SrcGame/src/systems/social/clan/cE_chip.cpp:41:33: error: too many arguments to function 'void SetFontTextColor(COLORREF)'
```

**Comportamento legacy preservado:** o `pre_refactor` contém duas chamadas no bloco de desenho da lista de clan:

```cpp
SetFontTextColor(destdc, RGB(0, 255, 0));
SetFontTextColor(destdc, RGB(0, 0, 0));
```

Porém, a API legacy disponível no próprio projeto declara apenas:

```cpp
void SetFontTextColor(COLORREF crColor);
```

No fluxo equivalente de `cE_user.cpp`, o `pre_refactor` usa diretamente a API Win32/GDI correta para um `HDC` explícito:

```cpp
SetTextColor(destdc, color);
```

**Classificação:** adaptação mínima de compilador/porte. Isto NÃO é tratado como correção geral do algoritmo/UI legacy. O source/call site original de `cE_chip` é mantido. Foi adicionado somente, e apenas sob MinGW, um overload local compatível:

```cpp
#if defined(__MINGW32__) || defined(__MINGW64__)
static inline void SetFontTextColor(HDC dc, COLORREF color)
{
    SetTextColor(dc, color);
}
#endif
```

Assim:

- MinGW consegue resolver a chamada legacy de dois argumentos;
- MSVC/outros compiladores continuam sem este overload e mantêm o caminho original;
- `ClanChipLegacyCommon.inl` não foi alterado;
- a policy `LegacyClanUiPolicy` continua a chamar literalmente `SetFontTextColor(dc, color)`;
- nenhuma constante, condição, loop, layout, ownership ou ordem de execução foi alterada.

**Verificação adicional:** `HoSkyOld.cpp` também contém chamadas legacy de dois argumentos, mas está explicitamente excluído da lista de fontes do build cliente MinGW. No source graph real de 203 TUs, `cE_chip.cpp` era o único bloqueio desta classe observado pelo compilador.

**Auditoria:** foi criado `COMPILER_ADAPTATIONS.tsv` e `validate_compiler_adaptations.py`. O validador congela o hash do source R6 antes da adaptação e exige que remover exatamente o bloco MinGW recupere esse hash. Isso impede que a exceção seja usada para esconder outras alterações funcionais.

R6.1 substitui R6 para builds MinGW. A diferença funcional autorizada entre os checkpoints é exclusivamente esta adaptação de compilador.
