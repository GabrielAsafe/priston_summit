Sim. Com o estado atual e os teus testes completos, eu considero estes os **bugs abertos reais**. O build já está saudável; o trabalho agora é runtime, assets e comportamento.

| Área                                 | Bug aberto                                                                                                                   | Estado / prioridade                                                                                      |
| ------------------------------------ | ---------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------- |
| **UI**                               | Ao pressionar `C`, aparece texto contendo `%` que desliza e para no lado esquerdo                                            | **Aberto**. Já constava no handoff e continua reproduzível.                                              |
| **Quit**                             | Overlay/hover é menor que a imagem do botão                                                                                  | **Aberto**                                                                                               |
| **Quit**                             | `Quit` fecha o processo; deveria voltar à tela de login                                                                      | **Aberto**                                                                                               |
| **GM commands**                      | `/skiptier` exige relog para sincronizar Rank/FSP/skills                                                                     | **Aberto**. O comportamento de relog já era conhecido no handoff.                                        |
| **GM commands**                      | Idealmente todos os comandos GM/dev devem sincronizar imediatamente sem relog                                                | **Aberto / refactor necessário**                                                                         |
| **Respawn**                          | Personagem de **Pillai** respawna em **Ricarten**                                                                            | **Aberto**                                                                                               |
| **Mechanician**                      | Imagens/recursos visuais da página Tier 5 incompletos                                                                        | **Aberto**                                                                                               |
| **Mechanician — Hardened Ward**      | Timer nasce, mas falta imagem e o indicador circular                                                                         | **Aberto**                                                                                               |
| **Mechanician — Rupture**            | Só parte dos efeitos aparece                                                                                                 | **Aberto**; recuperamos também os WAVs faltantes                                                         |
| **Mechanician — Swarm Shot**         | Gameplay existe, mas sem efeitos                                                                                             | **Aberto**                                                                                               |
| **Mechanician — Magnetic Discharge** | Gameplay existe, mas sem efeitos                                                                                             | **Aberto**                                                                                               |
| **Pikeman — Ring of Spears**         | Funciona e tem efeito                                                                                                        | **OK / controle positivo**                                                                               |
| **Pikeman — Death Mantra**           | Funciona, mas sem animação/efeito e sem timer visível                                                                        | **Aberto**                                                                                               |
| **Pikeman — Ex-Death**               | Funciona, mas sem animação/efeito e sem timer visível                                                                        | **Aberto**                                                                                               |
| **Pikeman — Sinister Spin**          | Skill funciona, mas falta visual                                                                                             | **Aberto**                                                                                               |
| **Fighter T5**                       | Nenhuma das quatro skills pode ser usada; parece falha na validação/reconhecimento da arma                                   | **Crítico / aberto**                                                                                     |
| **Fighter T5**                       | Ícones aparentam corretos                                                                                                    | **OK**                                                                                                   |
| **Archer T5**                        | Ícones corretos, mas faltam efeitos nas quatro skills                                                                        | **Aberto**                                                                                               |
| **Archer — Bombardment**             | Particularmente suspeita de routing de motion/efeito                                                                         | **Aberto**                                                                                               |
| **Atalanta — Nature's Call**         | Som toca, mas não nasce o Tiger                                                                                              | **Em correção**: assets/template `Summon Tiger` foram preparados; falta validar depois do deploy/restart |
| **Atalanta — Nature's Call**         | Visual T5 ainda incompleto/incorreto                                                                                         | **Aberto**                                                                                               |
| **Atalanta — Plague Javelin**        | Som + outline do alvo funcionam, mas falta efeito gráfico principal                                                          | **Aberto**                                                                                               |
| **Atalanta — Triple Threat**         | Gameplay funciona, mas faltam efeitos                                                                                        | **Aberto**                                                                                               |
| **Atalanta — Plague Javelin**        | Suporte `Targets` existe, mas multi-target real ainda não foi implementado com algoritmo/raio comprovado                     | **Aberto técnico**                                                                                       |
| **Knight — Holy Conviction**         | Visual/gameplay funcionando                                                                                                  | **OK / controle positivo**                                                                               |
| **Knight — Divine Inquisition**      | Falta efeito visual; timer/outline aparece                                                                                   | **Aberto**                                                                                               |
| **Knight — Glorious Shield**         | Timer existe mas sem imagem; efeito do escudo desapareceu/regrediu                                                           | **Aberto / regressão**                                                                                   |
| **Knight — Divine Cross**            | Falta efeito visual                                                                                                          | **Aberto**                                                                                               |
| **Priestess T5**                     | Ícones corretos; todas as quatro sem efeitos adequados                                                                       | **Aberto**                                                                                               |
| **Priestess — Ice Elemental**        | Não cria o elemental                                                                                                         | **Em correção**: modelo/template server+client já preparado; precisa deploy completo + restart + teste   |
| **Priestess**                        | Skill temporizada cria timer sem imagem                                                                                      | **Aberto**                                                                                               |
| **Magician — Wizard Trance**         | Skill funciona com equipamento correto, mas falta efeito visual                                                              | **Aberto**                                                                                               |
| **Magician**                         | Demais skills funcionais/acionáveis, mas sem os efeitos visuais esperados                                                    | **Aberto**                                                                                               |
| **Magician**                         | Timer existente fica sem imagem                                                                                              | **Aberto**                                                                                               |
| **Stone Skin**                       | `R5StoneSkin.part` não foi encontrado no EU disponível                                                                       | **Asset ainda ausente**                                                                                  |
| **Wizard Trance**                    | `MG5OrbMastery.part` não foi encontrado no EU disponível                                                                     | **Asset ainda ausente**                                                                                  |
| **Timers T5**                        | Em vários buffs o timer existe logicamente, mas falta `MatIcon`/imagem e em alguns falta também o círculo/progresso visual   | **Bug estrutural ainda aberto**                                                                          |
| **Assets T5**                        | Ainda há **73 efeitos legados candidatos** dos clientes Official/ExMachina/velho sem mapeamento seguro para as skills atuais | **Auditoria em andamento**                                                                               |
| **Sounds/effects**                   | 12 assets EU comprovados estavam faltando e já foram recuperados pelo 173                                                    | **Corrigido em arquivos; precisa reteste ingame**                                                        |
| **Server summons**                   | Fedora recebeu Ice Elemental + LLTiger + `.inf`; VM ainda não recebeu corretamente todos devido ao bug do deploy             | **Em andamento**                                                                                         |
| **Deploy Fedora→VM**                 | `send_to_s.sh` ainda para após a primeira entrada real do manifesto; fix 178 preparado                                       | **Infra aberto até validar 178**                                                                         |
| **Tier 5 INIs**                      | INIs server-side terminam em Tier 4; T5 atualmente usa defaults compilados                                                   | **Não é bug funcional**, mas fica como pendência para permitir balanceamento sem recompilar              |
| **Assassin/Shaman**                  | Não existem no Summit Tale                                                                                                   | **Fora do escopo T5 atual**; seriam classes completas do zero                                            |

Os dois bugs de UI (`C` e Quit) já estavam explicitamente deixados para depois do Tier 5 no handoff antigo. 

### Ordem que eu usaria depois de você mandar o servidor inteiro

**P0:** consolidar a nova source + servidor e parar de trabalhar sobre o snapshot antigo.

**P1:** Fighter não consegue usar nenhuma T5; summons Ice Elemental/Tiger; sincronização GM sem relog; respawn Pillai/Ricarten.

**P2:** corrigir de forma global o sistema **motion/effect/timer**, porque isso resolve várias classes de uma vez, em vez de patch por skill.

**P3:** completar assets faltantes/legados e sons.

**P4:** tecla `C`, Quit/retorno ao login e outros bugs gerais antigos.

Quando você mandar **o servidor inteiro + a nova source refatorada**, eu faria uma nova baseline e consideraria essa lista o backlog oficial para não continuarmos misturando patches antigos com o estado novo.


Perfeito. Agora temos algo que antes não tínhamos: **uma baseline comprovadamente funcional**. Isso permite tratar bugs sem confundir “bug antigo” com “bug introduzido pelo refactor”.

Eu dividiria o estado atual em quatro níveis.

### 1. Bugs reais graves — memória/corrupção

Estes eu considero bugs inequívocos, mesmo que o jogo atualmente funcione:

| ID      | Local                | Problema                                                                              |
| ------- | -------------------- | ------------------------------------------------------------------------------------- |
| BUG-001 | 3D/stage             | arrays criados com `new[]` destruídos com `delete`                                    |
| BUG-005 | `smPAT3D::Close()`   | possibilidade de double-delete                                                        |
| BUG-011 | vários loaders       | índices/contagens sem validação → OOB                                                 |
| BUG-012 | `smOBJ3D::WorldForm` | possibilidade de ultrapassar `RENDER_VERTEX_MAX`                                      |
| BUG-014 | character config     | `MotionInfo[512]` versus `NpcMotionRate[30]`; possibilidade de escrita fora do array  |
| BUG-015 | character config     | drops podem ultrapassar arrays de destino                                             |
| BUG-016 | stage                | faces/índices inválidos podem corromper índice espacial                               |
| BUG-018 | login server config  | contador errado permite ultrapassar `LoginServerIP[16]`                               |
| BUG-019 | `SetupPolyAreas`     | aritmética/incremento sobre ponteiros nulos                                           |
| BUG-041 | `HoEffectModel`      | escreve `NULL` dentro da representação de um `std::vector::iterator` por type-punning |
| BUG-043 | effect bounding box  | inicializa X três vezes e deixa Y/Z indefinidos                                       |
| BUG-045 | particle comparator  | função `bool` pode chegar ao fim sem `return`                                         |
| BUG-046 | `AssaEffect`         | `memset` usa tamanho de um objeto em vez do array                                     |
| BUG-049 | `YameEffect`         | mesmo tipo de `memset` parcial                                                        |
| BUG-050 | `ISAOCHECK`          | `memset` limpa apenas **1 byte** do request buffer                                    |
| BUG-051 | `ISAOCHECK`          | `sizeof(pointer)` usado para limpar buffer de 65536 bytes                             |
| BUG-052 | `cYAMEEFFECT`        | `memset(this, sizeof(outra classe))`                                                  |
| BUG-053 | manufacture          | limpa apenas 1 elemento de um array de 176                                            |
| BUG-054 | warehouse            | loop até 300 sobre array com 100 elementos                                            |
| BUG-058 | network integrity    | `FuncCount` recebido da rede pode indexar além de `[64]`                              |
| BUG-059 | item runtime         | variável `dwSpecialJob` usada possivelmente sem inicialização                         |

Esses são os que eu colocaria no topo de uma futura fase de estabilização.

---

### 2. Bugs reais de persistência/network

Estes são especialmente importantes **antes ou durante a migração para SQLite**, porque não quero transportar comportamento inseguro para a nova base.

**Warehouse:** os loaders podem continuar depois de `fread` incompleto/truncado e publicar `TRANS_WAREHOUSE` parcialmente inicializado. O save também confia em `lpTransWareHouse->size` para `fwrite`, podendo ler além da estrutura se esse campo estiver corrompido.

**Area Server:** `RecvAreaServerMap` aceita `size` e `IP_Count` vindos da rede sem validação antes de fazer `memcpy`.

**ClientFuncPos:** `FuncCount` e `size` provenientes da rede são confiados diretamente.

**Serialização SMD/stage:** existem aritméticas sobre “pointers serializados”, campos históricos com tamanhos inconsistentes (`WORD`/`DWORD`) e rebasing de offsets sem validação suficiente.

Esses não são apenas problemas estéticos. São fronteiras onde **dados externos controlam memória/tamanho**.

Para SQLite, a regra deverá ser diferente da reconstrução do porte: não vamos simplesmente traduzir `SQLExecute(...)` para `sqlite3_exec(...)`. Primeiro precisamos definir o contrato de dados e preservar exatamente o resultado funcional esperado pelo servidor.

---

### 3. Bugs reais, mas de impacto mais limitado

Há vários que podem causar leak, estado estranho ou comportamento incorreto sem necessariamente derrubar o servidor:

* leaks de `FindFirstFile`/handles Win32;
* `FindClose(INVALID_HANDLE_VALUE)`;
* leaks de talk-bones;
* material rollback incompleto;
* reload de patterns mantendo estado anterior;
* lifecycle não idempotente de materiais/stage;
* material marcado `Regist = TRUE` mesmo quando falhou;
* parser de material modificando a própria string de entrada;
* `FindNextMaterialFromTextureName` que pode não avançar;
* cache de talk animation confundindo dois `TalkPattern`;
* frames não inicializados copiados;
* storage de `HoEffectEventController` que sobrescreve eventos após 100 e pode perdê-los/leaká-los;
* `dwSpecialJob` indeterminado;
* `ReadASE_GEOMOBJECT` e outras rotinas com retorno de erro ignorado.

São bugs verdadeiros, mas eu não os misturaria com a migração SQLite.

---

### 4. Bugs/comportamentos estranhos que podem ser parte do gameplay legacy

Aqui precisamos ter muito mais cuidado.

Exemplo clássico:

```cpp
GetTriangleImact(...)
```

tem uma fórmula onde, num dos sentidos, os componentes Y/Z acabam equivalentes a zero. Parece matematicamente errado, mas **mudar isso altera collision/hitbox**.

Também existem:

* índice `0` usado simultaneamente como face válida e “não encontrado”;
* scale track com offset zero tratado como ausência de animação;
* `MaxFrame` calculado de forma legacy;
* arrays/tabelas com limites inconsistentes que assets existentes aparentemente exploram;
* diferenças reais cliente/servidor de tipos e capacidades;
* comportamento estranho de cache de animação.

Esses eu chamaria de:

> **bug provável, mas comportamento funcional contratado.**

Não devem ser corrigidos junto com infraestrutura.

---

# Más práticas de implementação

Separadamente dos bugs, há padrões arquiteturais que tornam o projeto extremamente frágil.

**1. Memória manual em todo lado.** `new/delete`, arrays crus, `memset(this, ...)`, ownership implícito. É a origem de uma boa parte dos bugs acima.

**2. Uso excessivo de buffers fixos.** Há muitos contratos do tipo `[30]`, `[64]`, `[100]`, `[120]`, `[176]`, `[5000]`, 4096 bytes etc., frequentemente com outro contador/tamanho vindo de fora.

**3. `sizeof` usado como lógica de runtime.** Exemplos como:

```cpp
memset(array, 0, sizeof(Element));
memset(buffer, 0, sizeof(pointer));
```

mostram que não existe uma abstração clara para capacidade.

**4. Persistência baseada na representação de memória.** Há estruturas, pointers/offsets e `fread/fwrite` muito próximos do layout C++ em memória. Isso dificulta versionamento e validação.

**5. Confiar em campos `size` enviados pelo exterior.** Pacote diz o tamanho → código copia/escreve esse tamanho. Essa fronteira precisa ser tratada explicitamente.

**6. Globals por toda a aplicação.** Estado de render, server, inventory, effect, warehouse, sockets, configs etc. é partilhado implicitamente. Ordem de inicialização passa a importar demais.

**7. Linkage controlado por macros.** O caso:

```cpp
#define GUIDEHELP_CPP
```

decidindo se um header cria ou declara um global é particularmente frágil.

**8. `extern`, `static` e anonymous namespaces usados como mecanismo arquitetural.** Foi justamente o que quebrou `SendWareHouseToClient`/`SendQuickMessage` no primeiro porte.

**9. Includes fazem parte da semântica.** Ordem dos headers, diretório físico do `.cpp`, capitalização de path e até presença de `#pragma once` alteram o programa.

**10. Erros frequentemente ignorados.** Muitos loaders fazem algo equivalente a:

```cpp
LoadSomething(...);
continue_execution();
```

sem testar o resultado.

**11. API Win32 diretamente espalhada pelo domínio.** File handles, registry, networking, HTTP, GDI etc. são chamados diretamente por gameplay/persistence/rendering, dificultando ownership e testes.

**12. Cliente e servidor duplicados mas quase iguais.** Isso gera divergências minúsculas e perigosas. A estrutura que reconstruímos já melhora bastante este problema sem mudar comportamento.

**13. Formatos não versionados adequadamente.** Vários assets/persistências dependem da estrutura histórica em vez de um schema explícito.

**14. Mistura de parsing + validação + estado global + I/O.** `fileread.cpp` era o melhor exemplo: abrir ficheiro, interpretar formato e preencher globals aconteciam no mesmo nível.

**15. Código dependente do ABI x86.** Há casts integer↔pointer, assumptions de tamanho, layouts de STL e estruturas que claramente nasceram num ambiente 32-bit específico.
