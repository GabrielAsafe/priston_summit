# SUMMIT — Documentação técnica das Quests V2
**Versão:** 26/10/2026 · **Base:** PATCH 351C · **Natureza:** estado verificado de código, não certificação em jogo

## Resumo executivo
Existem 88 definições de referência com IDs V2 únicos e dados consultáveis (351B). A 351C acrescenta início, cancelamento e contagem persistente de mortes para 65 quests (59 diárias/Aline + 6 semanais), **desativados por defeito**; não paga recompensas. As 23 quests de progressão continuam sob autoridade SinQuest/HaQuest e não têm execução canónica V2. A execução ponta a ponta das 88 quests não está concluída.

| Família | Catálogo | Progresso V2 | Recompensas V2 | Runtime completo |
|---|---:|---|---|---|
| Progressão | 23 | SinQuest/HaQuest (não migrado) | Não | Não certificado |
| Diárias | 35 | 351C experimental, opt-in | Não | Não certificado |
| Diárias da Aline | 24 | 351C experimental, opt-in | Não | Não certificado |
| Semanais | 6 | 351C experimental, opt-in | Não | Não certificado |

## Fontes e rastreabilidade
- `summit_quests_88_full_351a.json` — 88 entradas de **referência de gameplay** extraídas do JSON canónico; incluem objetivos e recompensas narrativos, não IDs técnicos autoritativos.
- `SUMMIT_PATCH_350Y...` e `SUMMIT_PATCH_350Z...` — adaptação QuestDB e identificação de 22 códigos SinQuest/HaQuest com namespace separado.
- `SUMMIT_PATCH_351B.../LEIA_PRIMEIRO_351B.md` — catálogo canónico e consulta V2 dos 88 IDs sem execução.
- `SUMMIT_PATCH_351C.../LEIA_PRIMEIRO_351C.md`, `OBJECTIVE_INDEX_351C.json` e mapa de implementação — 183 regras de objetivos para 65 quests e persistência opt-in.
- `Texto colado(20261026-122516).txt` — compilação/link e deploy MinGW concluídos após uma falha inicial de tipos; não prova execução em Wine.
- SHA-256 do JSON de referência: `320004fa8014c9e4e5ec72a4ebfe4be184cb9061b12cf5f3460aa6837708fa2a`.

## Inventário exato
- 88 IDs canónicos `0xE3510001` a `0xE3510058`, 106 etapas narrativas de progressão e 65 nós sintéticos de visualização; 337 objetivos descritos, 364 descrições de recompensa.
- 183 objetivos estruturados da 351C: 89 das 35 diárias, 88 das 24 Aline, 6 boss semanais. O total 337 conta também objetivos narrativos; não confundir as métricas.
- QuestDB SQLite: seis definições verificadas no snapshot examinado; **não** é o catálogo completo das 88. SinQuest/HaQuest representam outra fonte e não devem ser confundidos com os IDs V2.

## Arquitetura e fronteiras de autoridade
```text
JSON de referência (88) → Catálogo 351B / LegacyQuestCatalog → consulta V2 (sem concessão)
       ↳ 65 quests de caça → [flag 351C] Start / Cancel → evento MonsterKilledEvent
                                 → QuestDB__SummitV2QuestKill351C → contadores V2
                                 → ClaimReward BLOQUEADO
       ↳ 23 quests de progressão → SinQuest / HaQuest → records e efeitos legados
QuestDB SQL original (6 entradas no snapshot) → handler SQL independente
```
A flag `SUMMIT_ENABLE_QUEST_PROGRESS_351C=1` serve apenas para testes numa QuestDB e personagens descartáveis. Por omissão fica desligada. O reset é à meia-noite UTC (diárias) e segunda-feira UTC (semanais).

## Matriz de estado técnico
| Componente | Realizado até à 351C | Em falta / condição de fecho |
|---|---|---|
| Catálogo e detalhes | 88 definições/IDs sem colisões SQL/legado | Confirmar detalhe, filtros e NPC em Wine |
| Aceitação/cancelamento | 65 com persistência e validação de limites | Validação NPC, pré-requisitos, limites reais, reinício |
| Kills | 183 regras; bossOnly; tetos; testes SQLite | Nomes técnicos/IDs de monstro e autoria do kill em runtime |
| Progressão | 23 de consulta, autoridade do legado | Eventos NPC, items, etapas, arenas, tiers, timers; compatibilidade records |
| Recompensas | 364 descrições por variante | IDs de item/XP autoritativos, pagamento único atómico, versão do servidor |
| Persistência | Tabela física QuestDB__SummitV2QuestKill351C | Recovery/backup/corrupção/escritas simultâneas e multi-char |
| UI / NPC | Consulta dos 88 no V2 | Aceitar e entregar por NPC e reentrada sem desfasamento |
| Runtime | Compilação/link reportados PASS | E2E Wine, log de servidor, resets e duplicação |
| Companion | Journal e sincronização live observados historicamente | Bloqueio das mochilas regressou; 350X/350W não certificados |

## Regras de integridade para a migração
1. O servidor é a única autoridade para aceitar evento, progresso, nível, facção e recompensa. O cliente só apresenta e solicita.
2. Namespace `E351....` de referência nunca deve ser gravado como ID SQL legado nem substituir diretamente as flags SinQuest/HaQuest.
3. Não inferir códigos de item ou monstro a partir de nomes da wiki; mapear por ID real e ramo de conteúdo Classic/Idhas/Cronus.
4. Recompensa idempotente: ledger único por personagem/quest/ciclo/claim, transação e prova da concessão; impedir crédito duplo após timeout/relog/queda.
5. Validar XP em 64 bits; percentagens de barra XP, SP/EP, mudança de classe, objetos especiais e equipamento têm handlers distintos.
6. Não desbloquear inventário/entrega se save, ACK, ledger ou journal não tiverem estado confirmado; não desativar anticheat.

## Critérios de aceitação
- 65 diárias/semanais: seleção por nível e facção, NPC correto, progresso ao matar alvo autêntico, bosses válidos, conclusão ao regressar, entrega exata de item/XP, reset por ciclo, persistência após relog, sem duplicação em reconexão/crash.
- 23 de progressão: variantes facção/classe, NPCs, entregas, quest items, arenas e temporizadores, tiers/skill reset, recompensas, flag no record e leitura V2 equivalente antes/depois do relog.
- Verificar cliente/servidor MinGW + Wine e testes de recuperação; smoke tests isolados não constituem certificação final.

## Prioridade transversal após esta documentação
1. **P0 Companion:** resolver mochilas congeladas no percurso após commit/live e confirmar envio e receção consecutivos com fecho da interface, relog e falha de rede; depois equipamento e atributos. Histórico 350Q de sucesso não constitui certificação pós-350X.
2. **P1 Crafting real:** começar por Arcanist → Pedra Azul BI101, mas primeiro confirmar IDs de item/materiais/receita no catálogo autoritativo; efetuar reserva + débito + criação + save, com rollback e log único. A pesquisa no snapshot pós-349 só encontrou BI101 em testes de compra premium, não uma receita Arcanist autoritativa; por isso não há fundamento para afirmar craft implementado.
3. **P2 Profissões e nodes:** Miner, Lumberjack, Fisher, Hunter; proximidade/interação, XP persistente, level-up e drops exatos; avaliar os 4 nodes do mapa 58 com cliente presente, não apenas SPAWN no servidor.
4. **P3 Quests:** concluir as 65 com NPC/pagamento idempotente; migrar 23 percursos especiais com SinQuest/HaQuest preservado; validar 88/88 E2E.
5. **P4** Covenants, Story/World Phases, eventos, pets, Legends Arena e mapas restantes.

## Anexo — inventário 88 entradas

### Progressão (23)
- **E3510001 — Quests da Instrutora Amanda** (`instrutora-amanda`); nível: Até 39; estado: Leitura; runtime legado. Alvos / passos: Falar com a Instrutora Amanda em Ricarten ou Pillai.; Entregar o bolo ao vigia da facção e retornar à Amanda.; Falar com a Instrutora Amanda.; Caçar 5 Hopis (Tempskrons) no Jardim da Liberdade ou 5 Coelhos (Morions) na…
- **E3510002 — Quest do Level 20 - 1° Desafio de Classe** (`level-20-1-desafio-classe`); nível: 20; estado: Leitura; runtime legado. Alvos / passos: Falar com Mestre Verkan em Ricarten.; Obter Garra do Wolverine de Bronze na Floresta Bamboo, Chifre do Wolverine Prateado e Cauda do Wolverine Dourado na Mata das Acácias.; Entregar os 3 itens ao Mestre Verkan.; Falar c…
- **E3510003 — Quest do Level 30 - Por Ela** (`level-30-por-ela`); nível: 30; estado: Leitura; runtime legado. Alvos / passos: Falar com Allya em Ricarten.; Caçar 20 Bargons em Terra Esquecida e retornar.; Falar com Phillya em Pillai.; Caçar 20 Bargons em Terra Esquecida e retornar.
- **E3510004 — Quest do Level 40 - 2° Desafio de Classe** (`level-40-2-desafio-classe-melee`); nível: 40; estado: Leitura; runtime legado. Alvos / passos: Falar com o Mestre das Habilidades correspondente.; Obter 3 Sheltoms Vamp de Minigues Prateados durante a noite em Navisko e entregar ao Mestre.; Equipar a arma especial e caçar 30 Bargons para a primeira maturação.; Co…
- **E3510005 — Quest do Level 40 - 2° Desafio de Classe** (`level-40-2-desafio-classe-ranged`); nível: 40; estado: Leitura; runtime legado. Alvos / passos: Falar com o Mestre das Habilidades correspondente.; Obter 3 Sheltoms Vamp de Minigues Prateados durante a noite em Navisko e entregar ao Mestre.; Equipar a arma especial e caçar 25 Besouros Blindados em Terra Esquecida…
- **E3510006 — Quest do Level 40 - 2° Desafio de Classe** (`level-40-2-desafio-classe-casters`); nível: 40; estado: Leitura; runtime legado. Alvos / passos: Falar com o Mestre das Habilidades correspondente.; Obter 3 Sheltoms Vamp de Minigues Prateados durante a noite em Navisko e entregar ao Mestre.; Equipar a arma especial e caçar 50 monstros quaisquer em Terra Esquecida…
- **E3510007 — Quest do Level 55 - A Caverna** (`level-55-a-caverna`); nível: 55; estado: Leitura; runtime legado. Alvos / passos: Falar com Mestre Verkan em Ricarten.; Caçar 20 Muffins na Caverna dos Cogumelos e retornar.; Falar com Mestre Raymon em Pillai.; Caçar 20 Abelhas Selvagens na Caverna das Abelhas e retornar.
- **E3510008 — Quest do Level 60 - 3° Desafio de Classe** (`level-60-3-desafio-classe`); nível: 60; estado: Leitura; runtime legado. Alvos / passos: Falar com o Mestre das Habilidades.; Caçar 50 monstros solicitados em até 70 minutos e retornar.; Caçar outros 50 monstros solicitados em até 70 minutos e retornar.; Obter o Amuleto Real, normalmente caçando Caramujo Ri…
- **E3510009 — Quest do Level 70 - A Amizade de Michelle** (`level-70-amizade-de-michelle`); nível: 70; estado: Leitura; runtime legado. Alvos / passos: Falar com a Clériga do Templo no Campo de Batalha dos Anciões.; Caçar 10 Tiphoons no Templo Maldito - 1º Andar e retornar.
- **E351000A — Quest do Level 80 - 4° Desafio de Classe** (`level-80-4-desafio-classe`); nível: 80; estado: Leitura; runtime legado. Alvos / passos: Caçar 40 monstros solicitados em até 90 minutos; podem ser Aranha Rainha, Punhos de Aço ou Golem de Aço.; Caçar 40 monstros solicitados em até 90 minutos; podem ser Duende Xamã, Hiper-Máquina ou Morte Súbita.; Caçar 40…
- **E351000B — Quests do Level 80** (`level-80-quests`); nível: 80; estado: Leitura; runtime legado. Alvos / passos: Falar com a Clériga do Templo.; Caçar 5 Ratoos no Templo Maldito - 2º Andar e retornar.; Falar com Comandante Derik em Ricarten e Civil Minuren em Navisko.; Obter Vamp de Ouro, Vamp de Prata e Vamp de Bronze de bosses d…
- **E351000C — Quest do Level 85 - As Lágrimas de Calliar** (`level-85-lagrimas-de-calliar`); nível: 85; estado: Leitura; runtime legado. Alvos / passos: Falar com a Clériga do Templo e receber Lágrimas de Caliar.; Caçar 10 Grotescos na Estrada de Ferro do Caos e retornar.
- **E351000D — Quests do Level 90** (`level-90-quests`); nível: 90; estado: Leitura; runtime legado. Alvos / passos: Falar com Mestre da Força em Vila Eura.; Caçar 10 Punhos de Aço no Coração de Perum e retornar.; Falar com Comandante Derik e Mago Real Ray.; Com poder de ataque reduzido em 30%, caçar 50 monstros no Coração de Perum; a…
- **E351000E — Quest do Level 100 - Fantasma do Fúria** (`level-100-fantasma-do-furia`); nível: 100; estado: Leitura; runtime legado. Alvos / passos: Falar com Mago Real Ray.; Caçar 50 monstros quaisquer no Coração de Perum e retornar.; Na arena de treinamento, derrotar o espectro defensivo do Fúria e retornar ao Mago Real Ray.; Caçar 100 monstros quaisquer no Vale G…
- **E351000F — Quest do Level 103** (`level-103`); nível: 103; estado: Leitura; runtime legado. Alvos / passos: Falar com Mensageiro Ian (Tempskrons) ou Mensageira Rucia (Morions).; Obter 5 pedaços derrotando Morgonia, Montanha Azul, Olho de Fogo, Tartaruga Dragão e Cão Esqueleto.; Entregar os 5 pedaços ao mensageiro.; Receber Pe…
- **E3510010 — Quest do Level 110** (`level-110`); nível: 110; estado: Leitura; runtime legado. Alvos / passos: Falar com Mestre da Força em Vila Eura.; Caçar 20 Olhar de Gelo no Vale Gallubia e retornar.; Caçar Caoscara no Vale Gallubia até obter Anel Amaldiçoado e retornar.; Levar o Anel Amaldiçoado ao Mago Real Ray em Pillai.;…
- **E3510011 — Quest do Level 118** (`level-118`); nível: 118; estado: Leitura; runtime legado. Alvos / passos: Obter Documento Suspeito de Engenheiro do Laboratório no Laboratório Secreto e entregar ao Comandante Derik.; Levar Documento Suspeito à Salon, a Artesã.; Levar Relatório de Salon ao Comandante Derik.; Caçar 60 invasore…
- **E3510012 — Quest do Level 123** (`level-123`); nível: 123; estado: Leitura; runtime legado. Alvos / passos: Falar com Alma Flutuante na Arma Antiga.; Caçar 50 Nihil e retornar.; Levar Concentrado de Almas à Sábia Celina em Pillai.; Obter 4 Cristais de Alma Maligna: amarelo de Soldado Zumbi, laranja de Elite Zumbi, vermelho de…
- **E3510013 — Quest do Level 130** (`level-130`); nível: 130; estado: Leitura; runtime legado. Alvos / passos: Falar com Mago Real Ray em Pillai e depois com o Xamã Real na Estrada dos Ventos.; Instalar uma CStone na Estrada Sombria, região do Jardim Rosemary, coordenada H8, e retornar ao Xamã Real.; Com ataque reduzido em 30%,…
- **E3510014 — Quest do Level 140** (`level-140`); nível: 140; estado: Leitura; runtime legado. Alvos / passos: Receber Respirador Mágico com Sábia Celina e falar com Marina no Abismo do Mar.; Obter Escama Áspera de Dey ou Koon e entregar à Marina.; Obter Alma Azul de Sathla e Alma Vermelha de Arhdyra; entregar à Sábia Celina.; O…
- **E3510015 — Quest do Level 155** (`level-155`); nível: 155; estado: Leitura; runtime legado. Alvos / passos: Falar com Sábia Celina e depois com Rubi, a Sacerdotisa do Fogo, em Pillai.; Caçar 150 monstros no Vale Gallubia e ativar a Esfera do Fogo próximo ao Vale Snowstorm, coordenada E3.; Caçar 100 monstros no Abismo do Mar C…
- **E3510016 — Quest do Level 165** (`level-165`); nível: 165; estado: Leitura; runtime legado. Alvos / passos: Explorar o Santuário do Abismo e ativar uma de 3 áreas mágicas sorteadas; retornar ao Comandante Derik.; Obter Tentáculo de Faugn de Faugn Insano e entregar à Sábia Celina.; Obter 3 Lâminas Afiadas de Crios Insano e ent…
- **E3510017 — Quest do Level 172** (`level-172`); nível: 172; estado: Leitura; runtime legado. Alvos / passos: Explorar Coração de Fogo e ativar uma de 3 áreas mágicas sorteadas; retornar à Sábia Celina.; Obter 5 Fragmentos de Fogo de Igniculus, Guardião de Lava ou Arqueiro de Lava e entregar à Sábia Celina.; Com Cristal Misteri…

### Diárias (35)
- **E3510018 — Level 8 a 15 - Morions** (`daily-8-15-morion`); nível: 8–15; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 15 × Coelho
- **E3510019 — Level 16 a 30 - Morions** (`daily-16-30-morion`); nível: 16–30; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 10 × Zeldi; 5 × Duende Selvagem
- **E351001A — Level 8 a 15 - Tempskrons** (`daily-8-15-tempskron`); nível: 8–15; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 15 × Hopi
- **E351001B — Level 16 a 30 - Tempskrons** (`daily-16-30-tempskron`); nível: 16–30; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 10 × Tatu; 5 × Duende Macabro
- **E351001C — Level 31 a 49** (`daily-31-49`); nível: 31–49; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 15 × Arqueiro do Mal; 15 × Cripto
- **E351001D — Level 50 a 54** (`daily-50-54`); nível: 50–54; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 15 × Duende Líder; 10 × Decapitador
- **E351001E — Level 55 a 60** (`daily-55-60`); nível: 55–60; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 15 × Golem de Areia; 10 × Carniceiro
- **E351001F — Level 61 a 69** (`daily-61-69`); nível: 61–69; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 15 × Figon; 10 × Gigante de Pedra
- **E3510020 — Level 70 a 79** (`daily-70-79`); nível: 70–79; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 20 × Aranha Rainha
- **E3510021 — Level 80 a 85** (`daily-80-85`); nível: 80–85; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 25 × Máquina Vampira
- **E3510022 — Level 86 a 90** (`daily-86-90`); nível: 86–90; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 15 × Máquina-D; 12 × Guardião Antigo
- **E3510023 — Level 91 a 95** (`daily-91-95`); nível: 91–95; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 15 × Conjuradora; 10 × Tartaruga Blindada
- **E3510024 — Level 96 a 99** (`daily-96-99`); nível: 96–99; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 20 × Cão Infernal; 20 × Guardião Sombrio
- **E3510025 — Level 100 a 102** (`daily-100-102`); nível: 100–102; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 20 × Cavaleiro Sanguinário; 20 × Seto
- **E3510026 — Level 103 a 104** (`daily-103-104`); nível: 103–104; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 10 × Pike das Cinzas; 10 × Mecânico das Cinzas; 15 × Esqueleto de rei Hopi; 5 × Golem Robô
- **E3510027 — Level 105 a 107** (`daily-105-107`); nível: 105–107; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 10 × Cavaleiro das Cinzas; 10 × Sacerdotisa das Cinzas; 15 × Esqueleto de rei Hopi; 5 × Gorgon
- **E3510028 — Level 108 a 109** (`daily-108-109`); nível: 108–109; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 20 × Kakoa; 10 × Igolation; 5 × Xetan
- **E3510029 — Level 110 a 117** (`daily-110-117`); nível: 110–117; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 20 × Mineiro das Cinzas; 10 × Tarantulika; 5 × Serpente de Gelo
- **E351002A — Level 118 a 120** (`daily-118-120`); nível: 118–120; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 20 × Técnico do Laboratório; 10 × Soldado Invasor; 5 × Elite Invasor
- **E351002B — Level 121 a 122** (`daily-121-122`); nível: 121–122; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 25 × Bombardeiro Invasor; 10 × Guardião do Laboratório; 5 × Elite Invasor
- **E351002C — Level 123 a 126** (`daily-123-126`); nível: 123–126; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 25 × Técnico Zumbi; 10 × Soldado Zumbi; 5 × Nihil
- **E351002D — Level 127 a 129** (`daily-127-129`); nível: 127–129; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 25 × Bombardeiro Zumbi; 10 × Elite Zumbi; 5 × Chalybs
- **E351002E — Level 130 a 134** (`daily-130-134`); nível: 130–134; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 25 × Inferno; 10 × Guardião Enferrujado; 5 × Najan
- **E351002F — Level 135 a 139** (`daily-135-139`); nível: 135–139; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 25 × Ignis; 10 × Crios; 5 × Bruxa Mestre
- **E3510030 — Level 140 a 144** (`daily-140-144`); nível: 140–144; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 25 × Gurkob; 10 × Koon; 5 × Arhdyra
- **E3510031 — Level 145 a 149** (`daily-145-149`); nível: 145–149; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 25 × Dey; 10 × Faugn; 5 × Sathla
- **E3510032 — Level 150 a 154** (`daily-150-154`); nível: 150–154; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 25 × Koon; 10 × Shogoth; 5 × Arhdyra
- **E3510033 — Level 155 a 159** (`daily-155-159`); nível: 155–159; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 20 × Gurcold; 15 × Snowkoon; 5 × Polusgoth
- **E3510034 — Level 160 a 164** (`daily-160-164`); nível: 160–164; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 20 × Kryosdey; 15 × Faugelus; 5 × Coldyhra
- **E3510035 — Level 165 a 169** (`daily-165-169`); nível: 165–169; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 20 × Koon Ferido; 15 × Crios Insano; 5 × Arhdyra Quebrado
- **E3510036 — Level 170 a 171** (`daily-170-171`); nível: 170–171; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 20 × Dey Ferido; 15 × Sathla Ferido; 5 × Bruxa Insana
- **E3510037 — Level 172 a 174** (`daily-172-174`); nível: 172–174; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 15 × Igniculus; 15 × Guardião de Lava; 5 × Golem de Lava
- **E3510038 — Level 175 a 177** (`daily-175-177`); nível: 175–177; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 15 × Igniculus; 15 × Arqueiro de Lava; 5 × Golem de Lava
- **E3510039 — Level 178 a 179** (`daily-178-179`); nível: 178–179; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 15 × Insec; 15 × Arqueiro de Lava; 5 × Gigante de Lava
- **E351003A — Level 180+** (`daily-180-plus`); nível: 180+; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 15 × Insec; 10 × Gigante de Lava; 5 × Flamo

### Diárias da Aline (24)
- **E351003B — Treinamento Básico** (`aline-treinamento-basico`); nível: 40+; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 50 × Monstros quaisquer
- **E351003C — A Prisão Antiga** (`aline-a-prisao-antiga`); nível: 60+; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 15 × Cavaleiro Fantasma; 10 × Figon; 5 × Rei Hopi
- **E351003D — Perigos do Lago** (`aline-perigos-do-lago`); nível: 70+; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 15 × Grande Griven; 15 × Lagarto do Lago; 5 × Aranha Rainha
- **E351003E — Máquinas de Combate** (`aline-maquinas-de-combate`); nível: 80+; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 10 × Grotesco; 15 × Hiper-Máquina; 15 × Destruidor
- **E351003F — Engrenagens do Destino** (`aline-engrenagens-do-destino`); nível: 85+; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 10 × Punhos de Aço; 15 × Montanha; 20 × Máquina-D
- **E3510040 — Entrando Numa Fria** (`aline-entrando-numa-fria`); nível: 90+; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 20 × Olhar de Gelo; 15 × Gelo; 15 × Conjuradora
- **E3510041 — Combate na Ilha** (`aline-combate-na-ilha`); nível: 95+; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 15 × Cavaleiro Sanguinário; 20 × Mago Negro; 20 × Verme de Fogo
- **E3510042 — Aberrações Mortais** (`aline-aberracões-mortais`); nível: 100+; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 20 × Quimera; 15 × Abominação; 15 × Pike das Cinzas; 10 × Golem Robô
- **E3510043 — Guerreiros do Passado** (`aline-guerreiros-do-passado`); nível: 105+; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 10 × Gorgon; 20 × Atalanta das Cinzas; 15 × Mago das Cinzas; 15 × Sacerdotisa das Cinzas
- **E3510044 — Guardiões Igolanos** (`aline-guardiões-igolanos`); nível: 108+; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 10 × Xetan; 20 × Kakoa; 20 × Ruca; 20 × Igolation
- **E3510045 — Combate Congelante** (`aline-combate-congelante`); nível: 110+; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 15 × Serpente de Gelo; 20 × Selamento Explosivo; 20 × Mineiro das Cinzas; 20 × Verme do Gelo
- **E3510046 — Máquinas das Trevas** (`aline-maquinas-das-trevas`); nível: 118+; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 20 × Técnico Invasor; 20 × Técnico do Laboratório; 20 × Engenheiro do Laboratório; 20 × Soldado Invasor
- **E3510047 — Protetores da Arma** (`aline-protetores-da-arma`); nível: 123+; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 25 × Técnico Zumbi; 20 × Soldado Zumbi; 20 × Acero; 20 × Nihil
- **E3510048 — O Mundo Sombrio** (`aline-o-mundo-sombrio`); nível: 130+; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 25 × Inferno; 25 × Guardião Enferrujado; 20 × Sombra Misteriosa; 20 × Crios
- **E3510049 — Descobrindo as Profundezas** (`aline-descobrindo-as-profundezas`); nível: 140+; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 25 × Gurkob; 25 × Koon; 25 × Faugn; 20 × Shogoth
- **E351004A — Confronto sob as Águas** (`aline-confronto-sob-as-aguas`); nível: 150+; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 25 × Gurkob; 25 × Koon; 25 × Faugn; 15 × Shogoth; 15 × Arhdyra
- **E351004B — Ruínas da Cidade Antiga** (`aline-ruinas-da-cidade-antiga`); nível: 155+; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 25 × Gurcold; 25 × Snowkoon; 25 × Faugelus; 15 × Coldyhra
- **E351004C — Covil do Dragão de Gelo** (`aline-covil-do-dragao-de-gelo`); nível: 160+; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 25 × Kryosdey; 25 × Snowkoon; 25 × Faugelus; 25 × Gelathla
- **E351004D — O Recomeço da Guerra** (`aline-o-recomeco-da-guerra`); nível: 165+; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 25 × Koon Ferido; 25 × Dey Ferido; 20 × Arhdyra Quebrado; 15 × Bruxa Insana
- **E351004E — Salve o Reino do Mar** (`aline-salve-o-reino-do-mar`); nível: 170+; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 25 × Koon Ferido; 25 × Crios Insano; 20 × Faugn Insano; 15 × Sathla Ferido
- **E351004F — Ameaça Flamejante** (`aline-ameaca-flamejante`); nível: 172+; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 25 × Igniculus; 20 × Guardião de Lava; 20 × Arqueiro de Lava; 15 × Flamo
- **E3510050 — A Essência do Fogo** (`aline-a-essência-do-fogo`); nível: 175+; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 25 × Igniculus; 20 × Arqueiro de Lava; 20 × Insec; 15 × Guardião de Lava
- **E3510051 — A Alma de Fogo** (`aline-a-alma-de-fogo`); nível: 178+; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 25 × Guardião de Lava; 20 × Arqueiro de Lava; 20 × Insec; 15 × Flamo
- **E3510052 — Em Busca de Respostas** (`aline-em-busca-de-respostas`); nível: 180+; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: 25 × Insec; 20 × Flamo; 15 × Gigante de Lava; 5 × Golem de Lava

### Semanais (6)
- **E3510053 — Ameaça na Floresta** (`weekly-20-40-ameaca-na-floresta`); nível: 20–40; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: Derrotar o boss Planta Infernal na Mata das Acácias e retornar a Proud.
- **E3510054 — Zumbi Enfadonho** (`weekly-20-40-zumbi-enfadonho`); nível: 20–40; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: Derrotar o boss Esqueleto Assassino no Refúgio dos Anciões e retornar a Proud.
- **E3510055 — Protetor Antigo** (`weekly-41-49-protetor-antigo`); nível: 41–49; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: Derrotar o boss Golem Ancião no Oásis e retornar a Proud.
- **E3510056 — Fantasma Revivido** (`weekly-50-64-fantasma-revivido`); nível: 50–64; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: Derrotar o boss Múmia Real no Calabouço Antigo - 2º Andar e retornar a Proud.
- **E3510057 — Assassino Nas Trevas** (`weekly-65-80-assassino-nas-trevas`); nível: 65–80; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: Derrotar o boss Perseguidor Sombrio no Santuário Sombrio e retornar a Proud.
- **E3510058 — Força Vs Força** (`weekly-81-90-forca-vs-forca`); nível: 81–90; estado: Progresso 351C, opt-in; sem prémios. Alvos / passos: Derrotar o boss Omicronos no Templo Maldito - 2º Andar e retornar a Proud.

**Versão:** 26/10/2026. Não autoriza ativar a 351C em produção nem assumir que Companion ou Quests estão concluídos.
