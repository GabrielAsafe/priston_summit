# Priston Tale — Bíblia Consolidada de Mundo, Mapas e Classes

> Base de trabalho para esta versão do jogo. O objetivo é separar **o que o catálogo realmente conecta**, **a espinha narrativa consolidada** e **propostas de expansão** sem misturar esses três níveis.

## 1. Inventário real desta build

O catálogo do cliente contém **53 registros estáticos (códigos 0–52)**. Um deles é `map.office` (código 16, `room//office.ase`), sem `map_image_id`, start points, gates, warp gates, objetos ou ambiente. Por isso, para o mapa mundial, ele é tratado como **sala técnica/interna** e ficam **52 mapas representáveis**.

A topologia de `gates` e `warp_gates` é igual entre os catálogos do cliente e do servidor. Os níveis mínimos diferem em alguns mapas, portanto este documento usa **o cliente** apenas para mostrar o `limit_level` informativo; as ligações vêm da estrutura comum.

### Legenda

- `──` ou `—` = adjacência física registrada por `gate` (desenhada sem direção).
- `⇄` = warp presente nos dois sentidos.
- `→` / `←` = warp codificado em apenas um sentido.
- `↺` = teleporte interno no próprio mapa.
- “sem ligação estática” **não significa necessariamente inacessível**: pode ser instância, NPC, evento ou transição implementada fora desta tabela.
- Os nomes dos códigos 50 e 51 são identificações prováveis pela sequência/asset; o catálogo grava literalmente `map.mar` e `map.lost3`.

## 2. Mapa ASCII — continente principal

```text
[21 PILLAI]
                                         │
                         [20 Estrada dos Ventos]
                                         │
                           [19 Vale Tranquilo]
                                         │
              [25 Caverna das Abelhas] ⇄ [18 Floresta das Ilusões / Crepúsculo]
                    ⇅                    │
          [26 Santuário Sombrio]       [17 Floresta dos Espíritos]
                    ⇅                    │
      [24 Caverna dos Cogumelos]       [06 VILA RUINEN] ── [34 Lago da Ganância]
                    ⇅                  /              \
         [00 Mata das Acácias]        /                \
                    │          [05 Castelo Perdidos]   [07 Terra Maldita]
          [01 Floresta Bamboo]        │                 /              \
                    │          [04 Refúgio Anciões] [08 Terra Esquecida] ⇄ [13 Calabouço 1]
          [02 Jardim Liberdade]        │              /        \                 ⇅
                    │                  └──────────────┘       [10 Oásis]       [14 Calabouço 2]
              [03 RICARTEN]                                  │                  ⇅
                    ▲                                     [09 NAVISKO]       [15 Calabouço 3]
                    │                                        │
          [52 Dark Boss / dc1]                         [11 Campo de Batalha] ── [12 Terra Proibida]
                                                             │      \                 │
                                                             │       ⇄ [46 Lab. Secreto]
                                                             │                        │
                                                             ⇄                  [27 Estrada de Ferro]
                                                       [22 Templo Maldito 1]           │
                                                             ⇅                  [28 Coração de Perum]
                                                       [23 Templo Maldito 2]           │
                                                             ⇅                       [29 Eura]
                                                       [42 Templo Maldito 3]           │
                                                                                 [31 Vale Gallubia]
                                                                                   /          \
                                                                     ⇄ [44 Mina de Gelo]   [35 Santuário Congelado]
                                                                                                  ⇅
                                                                                           [36 Covil Kelvezu]

                  [09 Navisko] ── [30 Bellatra / SoD]

                  [39 Fall Game] ──warp→ [21 Pillai]
```

### Correção geográfica mais importante

O rascunho anterior fazia o caminho Tempskron passar por:

`Ricarten → Jardim → Bamboo → Acácias → Floresta dos Espíritos → Ruinen`.

Essa cadeia **não existe** no catálogo. A geografia correta para a convergência é:

```text
TEMPSKRON
Ricarten
   │
Jardim da Liberdade
   │
Refúgio dos Anciões
   │
Castelo dos Perdidos
   │
Vila Ruinen

Ramo inicial opcional:
Jardim da Liberdade ── Floresta Bamboo ── Mata das Acácias
```

Enquanto o eixo Morion já encaixa quase perfeitamente:

```text
MORION
Pillai
  │
Estrada dos Ventos
  │
Vale Tranquilo
  │
Floresta das Ilusões / Terra do Crepúsculo
  │
Floresta dos Espíritos
  │
Vila Ruinen
```

Existe ainda uma ligação subterrânea muito útil para quests e exploração:

```text
Mata das Acácias
      ⇄
Caverna dos Cogumelos
      ⇄
Santuário Sombrio
      ⇄
Caverna das Abelhas
      ⇄
Floresta das Ilusões / Crepúsculo
```

Isso cria uma ponte natural entre as regiões Tempskron e Morion **sem precisar inventar um novo mapa**.

## 3. Mapa ASCII — Ilha Perdida / Torre Sem Fim

```text
[37 Ilha Perdida] ── [38 Templo Perdido] ←warp── [40 Torre Sem Fim 1]
                                                    ⇅
                                             [41 Torre Sem Fim 2]
                                                    ⇅
                                             [43 Torre Sem Fim 3]
```

## 4. Mapas de instância, evento ou sem saída estática no catálogo

Os seguintes registros existem nesta build, mas a tabela de campos não descreve uma ligação externa normal para eles (ou só descreve retorno/teleporte interno):

- **32** Arena da Quest de Rank/Tier 4 (`map.quest_iv`)
- **33** Castelo Bless (`map.castle`) — apenas teleporte interno no próprio mapa
- **45** SoD-2 / arena interna (`map.sod_2`)
- **47** Arma Antiga (`map.ancientw`)
- **48** Abismo do Mar (`map.seaa`)
- **49** Santuário/Templo do Abismo (`map.stemple`)
- **50** `map.mar` — provável Abismo do Mar Congelado
- **51** `map.lost3` — provável Estrada Sombria
- **52** Dark Boss / `dc1` — possui retorno por warp a Ricarten
- **39** Fall Game — possui retorno por warp a Pillai

Esses mapas devem ser ligados na campanha somente depois de confirmar **qual sistema os instancia** (NPC, quest, core, evento ou código separado), em vez de inventar um `gate` que não existe no catálogo.

## 5. Ledger dos 52 mapas e suas ligações

```text
[00 Mata das Acácias]  — [01 Floresta Bamboo]; ⇄ [24 Caverna dos Cogumelos]
    id=map.fore_3; stage=forest//fore-3.ase; limit_level(client)=0
[01 Floresta Bamboo]  — [00 Mata das Acácias]; — [02 Jardim da Liberdade]; ↺ teleporte interno
    id=map.fore_2; stage=forest//fore-2.ase; limit_level(client)=0
[02 Jardim da Liberdade]  — [01 Floresta Bamboo]; — [03 Ricarten]; — [04 Refúgio dos Anciões]
    id=map.fore_1; stage=forest//fore-1.ase; limit_level(client)=0
[03 Ricarten]  — [02 Jardim da Liberdade]; ← [52 Dark Boss / arena técnica (dc1)]; ↺ teleporte interno
    id=map.village_2; stage=ricarten//village-2.ase; limit_level(client)=0
[04 Refúgio dos Anciões]  — [02 Jardim da Liberdade]; — [05 Castelo dos Perdidos]
    id=map.ruin_4; stage=Ruin//ruin-4.ase; limit_level(client)=0
[05 Castelo dos Perdidos]  — [04 Refúgio dos Anciões]; — [06 Vila Ruinen]
    id=map.ruin_3; stage=Ruin//ruin-3.ase; limit_level(client)=0
[06 Vila Ruinen]  — [05 Castelo dos Perdidos]; — [07 Terra Maldita]; — [17 Floresta dos Espíritos]; — [34 Lago da Ganância]; ↺ teleporte interno
    id=map.ruin_2; stage=Ruin//ruin-2.ase; limit_level(client)=0
[07 Terra Maldita]  — [06 Vila Ruinen]; — [08 Terra Esquecida]; ⇄ [13 Calabouço Antigo — 1º Andar]
    id=map.ruin_1; stage=Ruin//ruin-1.ase; limit_level(client)=0
[08 Terra Esquecida]  — [07 Terra Maldita]; — [09 Navisko]; — [10 Oásis]
    id=map.de_1; stage=desert//De-1.ase; limit_level(client)=0
[09 Navisko]  — [08 Terra Esquecida]; — [11 Campo de Batalha dos Anciões]; — [30 Bellatra / SoD]; ↺ teleporte interno
    id=map.village_1; stage=forest//village-1.ase; limit_level(client)=0
[10 Oásis]  — [08 Terra Esquecida]
    id=map.de_2; stage=desert//De-2.ase; limit_level(client)=0
[11 Campo de Batalha dos Anciões]  — [09 Navisko]; — [12 Terra Proibida]; ⇄ [22 Templo Maldito — 1º Andar]; ⇄ [46 Laboratório Secreto]
    id=map.de_3; stage=desert//De-3.ase; limit_level(client)=55
[12 Terra Proibida]  — [11 Campo de Batalha dos Anciões]; — [27 Estrada de Ferro do Caos]; ↺ teleporte interno
    id=map.de_4; stage=desert//De-4.ase; limit_level(client)=55
[13 Calabouço Antigo — 1º Andar]  ⇄ [07 Terra Maldita]; ⇄ [14 Calabouço Antigo — 2º Andar]
    id=map.dun_1; stage=dungeon//dun-1.ase; limit_level(client)=40
[14 Calabouço Antigo — 2º Andar]  ⇄ [13 Calabouço Antigo — 1º Andar]; ⇄ [15 Calabouço Antigo — 3º Andar]
    id=map.dun_2; stage=dungeon//dun-2.ase; limit_level(client)=50
[15 Calabouço Antigo — 3º Andar]  ⇄ [14 Calabouço Antigo — 2º Andar]
    id=map.dun_3; stage=dungeon//dun-3.ase; limit_level(client)=60
[17 Floresta dos Espíritos]  — [06 Vila Ruinen]; — [18 Floresta das Ilusões / Terra do Crepúsculo]
    id=map.forever_fall_04; stage=forever-fall//forever-fall-04.ASE; limit_level(client)=0
[18 Floresta das Ilusões / Terra do Crepúsculo]  — [17 Floresta dos Espíritos]; — [19 Vale Tranquilo]; ⇄ [25 Caverna das Abelhas]; ↺ teleporte interno
    id=map.forever_fall_03; stage=forever-fall//forever-fall-03.ASE; limit_level(client)=0
[19 Vale Tranquilo]  — [18 Floresta das Ilusões / Terra do Crepúsculo]; — [20 Estrada dos Ventos]
    id=map.forever_fall_02; stage=forever-fall//forever-fall-02.ASE; limit_level(client)=0
[20 Estrada dos Ventos]  — [19 Vale Tranquilo]; — [21 Pillai]; ⇄ [21 Pillai]
    id=map.forever_fall_01; stage=forever-fall//forever-fall-01.ASE; limit_level(client)=0
[21 Pillai]  — [20 Estrada dos Ventos]; ⇄ [20 Estrada dos Ventos]; ← [39 Fall Game (mapa de evento)]; ↺ teleporte interno
    id=map.pilai; stage=forever-fall//pilai.ASE; limit_level(client)=0
[22 Templo Maldito — 1º Andar]  ⇄ [11 Campo de Batalha dos Anciões]; ⇄ [23 Templo Maldito — 2º Andar]
    id=map.dun_4; stage=dungeon//dun-4.ase; limit_level(client)=70
[23 Templo Maldito — 2º Andar]  ⇄ [22 Templo Maldito — 1º Andar]; ⇄ [42 Templo Maldito — 3º Andar]
    id=map.dun_5; stage=dungeon//dun-5.ase; limit_level(client)=75
[24 Caverna dos Cogumelos]  ⇄ [00 Mata das Acácias]; ⇄ [26 Santuário Sombrio]
    id=map.tcave; stage=cave//Tcave.ase; limit_level(client)=55
[25 Caverna das Abelhas]  ⇄ [18 Floresta das Ilusões / Terra do Crepúsculo]; ⇄ [26 Santuário Sombrio]
    id=map.mcave; stage=cave//Mcave.ase; limit_level(client)=55
[26 Santuário Sombrio]  ⇄ [24 Caverna dos Cogumelos]; ⇄ [25 Caverna das Abelhas]
    id=map.dcave; stage=cave//Dcave.ase; limit_level(client)=65
[27 Estrada de Ferro do Caos]  — [12 Terra Proibida]; — [28 Coração de Perum]
    id=map.iron_1; stage=Iron//iron-1.ASE; limit_level(client)=80
[28 Coração de Perum]  — [27 Estrada de Ferro do Caos]; — [29 Eura]
    id=map.iron_2; stage=Iron//iron-2.ASE; limit_level(client)=85
[29 Eura]  — [28 Coração de Perum]; — [31 Vale Gallubia]; ↺ teleporte interno
    id=map.ice_ura; stage=Ice//ice_ura.ASE; limit_level(client)=90
[30 Bellatra / SoD]  — [09 Navisko]
    id=map.sod_1; stage=Sod//sod-1.ASE; limit_level(client)=0
[31 Vale Gallubia]  — [29 Eura]; — [35 Santuário Congelado]; ⇄ [44 Mina de Gelo — 1º Andar]
    id=map.ice1; stage=Ice//ice1.ASE; limit_level(client)=90
[32 Arena da Quest de Rank/Tier 4]  (sem ligação estática codificada neste catálogo)
    id=map.quest_iv; stage=quest//quest_IV.ASE; limit_level(client)=0
[33 Castelo Bless]  ↺ teleporte interno
    id=map.castle; stage=castle//castle.ase; limit_level(client)=50
[34 Lago da Ganância]  — [06 Vila Ruinen]
    id=map.greedy; stage=Greedy//Greedy.ase; limit_level(client)=70
[35 Santuário Congelado]  — [31 Vale Gallubia]; ⇄ [36 Covil do Kelvezu]
    id=map.ice2; stage=Ice//ice_2.ASE; limit_level(client)=90
[36 Covil do Kelvezu]  ⇄ [35 Santuário Congelado]
    id=map.boss; stage=Boss//Boss.ASE; limit_level(client)=90
[37 Ilha Perdida]  — [38 Templo Perdido]; ↺ teleporte interno
    id=map.lost; stage=lost//lostisland.ASE; limit_level(client)=95
[38 Templo Perdido]  — [37 Ilha Perdida]; ← [40 Torre Sem Fim — 1º Andar]
    id=map.losttemple; stage=Losttemple//lost_temple.ASE; limit_level(client)=100
[39 Fall Game (mapa de evento)]  → [21 Pillai]; ↺ teleporte interno
    id=map.fall_game; stage=Fall_Game//fall_game.ASE; limit_level(client)=0
[40 Torre Sem Fim — 1º Andar]  ⇄ [41 Torre Sem Fim — 2º Andar]; → [38 Templo Perdido]
    id=map.dun_7; stage=endless//dun-7.ase; limit_level(client)=102
[41 Torre Sem Fim — 2º Andar]  ⇄ [40 Torre Sem Fim — 1º Andar]; ⇄ [43 Torre Sem Fim — 3º Andar]
    id=map.dun_8; stage=endless//dun-8.ase; limit_level(client)=105
[42 Templo Maldito — 3º Andar]  ⇄ [23 Templo Maldito — 2º Andar]
    id=map.dun_6; stage=dungeon//Dun-6a.ASE; limit_level(client)=80
[43 Torre Sem Fim — 3º Andar]  ⇄ [41 Torre Sem Fim — 2º Andar]
    id=map.dun_9; stage=endless//dun-9.ASE; limit_level(client)=108
[44 Mina de Gelo — 1º Andar]  ⇄ [31 Vale Gallubia]
    id=map.mine_1; stage=Mine//mine-1.ASE; limit_level(client)=110
[45 SoD-2 / arena interna]  (sem ligação estática codificada neste catálogo)
    id=map.sod_2; stage=Sod//sod-2.ASE; limit_level(client)=0
[46 Laboratório Secreto]  ⇄ [11 Campo de Batalha dos Anciões]
    id=map.slab; stage=Slab//Slab.ASE; limit_level(client)=118
[47 Arma Antiga]  (sem ligação estática codificada neste catálogo)
    id=map.ancientw; stage=AncientW//ancientW.ASE; limit_level(client)=118
[48 Abismo do Mar]  (sem ligação estática codificada neste catálogo)
    id=map.seaa; stage=SeaA//SeaA.ASE; limit_level(client)=125
[49 Santuário/Templo do Abismo]  (sem ligação estática codificada neste catálogo)
    id=map.stemple; stage=SeaA//Stemple.ASE; limit_level(client)=0
[50 Abismo do Mar Congelado (identificação provável)]  (sem ligação estática codificada neste catálogo)
    id=map.mar; stage=mar//ruin-arenaSF.ASE; limit_level(client)=140
[51 Estrada Sombria (identificação provável)]  (sem ligação estática codificada neste catálogo)
    id=map.lost3; stage=lost//lost3.ASE; limit_level(client)=130
[52 Dark Boss / arena técnica (dc1)]  → [03 Ricarten]
    id=map.dc1; stage=boss//dark_boss.ASE; limit_level(client)=1
```

---

# 6. História consolidada

## Regra de canon

O documento original tinha uma boa regra e ela deve continuar: separar **CANON/BASE**, **INFERÊNCIA** e **NOVA PROPOSTA**. A lore pública de Priston Tale é fragmentada; portanto a campanha não deve transformar uma inferência em fato histórico sem marcar a mudança.

## Prólogo — A Guerra dos Deuses

A memória mais antiga do continente fala de uma guerra entre deuses, incluindo Migal, Mardanos, Tempritus/Tempritos, Skroniff/Skronipe e Midranda. Midranda entra em conflito com os demais e a guerra deixa marcas no mundo, nos monstros e nas civilizações posteriores.

O jogador não recebe a cosmologia inteira no tutorial. No começo, basta saber:

> Houve uma guerra. O mundo sobreviveu. As civilizações se reconstruíram.

A revelação posterior é mais forte:

> A guerra acabou, mas suas causas e consequências não.

## Era das duas culturas

Dos sobreviventes surgem duas grandes identidades.

**Tempskron** desenvolve uma cultura de guerra, engenharia, máquinas, comércio e pragmatismo, com Ricarten como grande centro.

**Morion** desenvolve uma cultura de religião, magia, tradição e estudo do sobrenatural, com Pillai como referência.

A campanha inicial deve mostrar essas culturas em rotas diferentes, mas sem afirmar que são mundos separados. O objetivo é fazê-las convergir organicamente em **Vila Ruinen**.

## Arco I — Quem somos nós? (Lv. 1–20)

O personagem começa como aventureiro desconhecido. Os primeiros mapas ensinam cidade, instrutores, equipamentos, comércio e combate.

Cada família de monstros deve responder, mesmo que o jogador ainda não saiba a resposta:

1. De onde veio?
2. Por que está neste habitat?
3. O que a alterou, se algo a alterou?

O mundo deixa de parecer uma sequência de “mobs por nível” e passa a parecer um ecossistema.

## Ruinen — o primeiro grande ponto de convergência

**Ruinen é o marco da campanha.**

Antes de Ruinen, a pergunta é:

> “Quem somos nós?”

Depois de Ruinen:

> “O que está acontecendo com Priston?”

A posição real do mapa ajuda muito: Ruinen toca o eixo Tempskron por Castelo dos Perdidos, o eixo Morion por Floresta dos Espíritos, segue para Terra Maldita e possui o ramo de Lago da Ganância.

A cidade pode funcionar como o primeiro lugar onde Morions e Tempskrons percebem que as ameaças são continentais, não tribais.

## Arco II — Coisas que não deveriam estar aqui (Lv. 20–40)

Sinais pequenos começam a se repetir em regiões diferentes:

- animais abandonam habitats;
- mineradores encontram materiais estranhos;
- pescadores recuperam objetos antigos;
- símbolos semelhantes aparecem em ruínas;
- monstros surgem longe de seu ecossistema.

Mining, Fishing, Hunting e Collecting deixam de ser atividades paralelas e passam a ser **formas de investigação**.

Três linhas funcionam particularmente bem:

- **A Pedra Negra:** mineração → símbolo → ruína → passagem selada.
- **O que o mar devolveu:** pesca → objetos → mapa antigo → expedição desaparecida.
- **Os Animais que Fugiram:** caça → migração → energia/cristais → criatura marcada.

## Arco III — A Cidade que se lembra (Lv. 40–55)

Navisko vira a primeira prova de que “monstro” não é uma categoria moral simples.

A maldição, os habitantes transformados e a ligação com Bellatra devem produzir a pergunta:

> Quem causou isso?

A resposta correta neste ponto é:

> Ainda não sabemos.

A questline **Os Nomes de Navisko** funciona como ponte emocional: registros de famílias, casas abandonadas e monstros que podem ter sido pessoas.

## Arco IV — A guerra que não acabou (Lv. 55–70)

Cavernas, ruínas e monstros antigos começam a conectar o presente à Guerra dos Deuses.

O jogador compara registros Morion e Tempskron e percebe que ambos são incompletos. Uma terceira versão da história deve aparecer sem ser imediatamente declarada “a verdadeira”.

Midranda progride em quatro estados:

1. história;
2. influência;
3. ameaça;
4. presença.

Isso evita que toda anomalia do mundo seja reduzida a “Midranda fez isso”.

## Arco V — A ciência da corrupção (Lv. 70–85)

Tulla, cristais e o Laboratório Secreto transformam a campanha.

O ponto temático é importante: nem todo desastre precisa vir de um deus. Pessoas também podem descobrir poder antigo e usá-lo mal.

Tulla funciona melhor como alguém que **descobriu uma parte de um sistema antigo que não compreendia por inteiro**, e não como explicação universal para a corrupção.

## Arco VI — A Cidade Ferida (Lv. 85–100)

Greedy e a Arma Antiga levam a guerra até Pillai.

A cidade deve carregar consequências persistentes:

- estruturas danificadas;
- NPCs deslocados;
- áreas interditadas;
- novos diálogos;
- memoriais;
- sobreviventes que reaparecem depois.

O mundo precisa lembrar do que aconteceu.

## Arco VII — A Segunda Guerra (Lv. 100–135)

Midranda deixa de ser lenda e torna-se força ativa. A escalada pode seguir:

ataques → preparação → invasão → defesa → confronto → queda → fuga.

**Castelo Bless** funciona como palco terrestre onde Morions, Tempskrons, clãs e personagens recorrentes convergem.

A guerra fica mais forte se NPCs apresentados no início reaparecerem aqui.

## Arco VIII — O Abismo

Quando a campanha parece poder terminar com Midranda, o oceano amplia a cosmologia.

O jogador descobre que há forças com objetivos próprios e que nem toda entidade poderosa pertence ao eixo “deuses conhecidos versus Midranda”.

O Abismo deve responder algumas perguntas e criar outras:

- quem controla essas regiões?
- por que o poder de Midranda interessa a eles?
- o que existia antes da Guerra dos Deuses?
- os deuses conhecidos foram realmente os primeiros?

Midranda não precisa virar herói. Uma aliança temporária contra algo pior é mais interessante do que uma redenção simples.

## Arco IX — Os Elementos

Coração de Fogo, Mina de Gelo, cristais e regiões abissais revelam que a guerra antiga afetou o próprio funcionamento do continente: fogo, água, gelo, terra, energia e mana.

A cadeia temática fica:

`cristais → Tulla → laboratório → tecnologia antiga → armas → guerra → ruptura elemental`

## Endgame — O Primeiro Passado

A pergunta final da campanha é:

> Por que Priston sobreviveu à Guerra dos Deuses?

A proposta mais forte do documento original é manter aberta a hipótese de que:

> Priston foi reconstruído sobre ruínas de algo anterior às civilizações Morion e Tempskron — talvez anterior até à ordem divina conhecida.

Isso permite continuar o jogo sem apenas introduzir “um Midranda maior”.

## Regras para futuras quests

Uma quest narrativa deve fazer pelo menos uma destas coisas:

- revelar algo do mundo;
- desenvolver personagem ou facção;
- alterar o estado do mundo;
- responder uma pergunta anterior;
- criar uma pergunta nova;
- conectar dois elementos de lore;
- preparar uma revelação futura.

“Mate 30 monstros” é tarefa.  
“Descubra por que 30 criaturas abandonaram o habitat” é história.

---

# 7. Novas classes que combinam com Priston Tale

A lista de D&D oferece dezenas de nomes, mas Priston já cobre muito bem os papéis clássicos de **Tank, Healer, Nuker e DPS**. Para uma nova classe valer a pena, ela deveria preencher sobretudo os espaços de **Buffer/Leader, Debuffer/Controller, Petmaster/Trapper ou híbrido de utilidade**, em vez de repetir Fighter, Knight, Archer, Assassin, Magician, Priestess ou Shaman.

## Candidatas mais naturais

### 1. Marshal / Marechal → **Comandante Tempskron**
**Papel:** Buffer + controle de aggro + melee de suporte.  
**Fantasia:** veterano de campanhas, ordens de batalha, estandartes, formações e gritos de guerra.  
**Por que funciona:** Priston tem clãs, Bellatra, Bless Castle, guerras antigas e uma cultura Tempskron militar. Falta uma classe cujo valor principal seja **fazer o grupo funcionar melhor**.  
**Evitar:** transformá-lo em outro Knight.

### 2. Druid → **Guardião Natural Morion**
**Papel:** Controller + Petmaster leve + suporte.  
**Fantasia:** flora, espíritos, raízes, animais e restauração de áreas corrompidas.  
**Por que funciona:** conversa diretamente com a campanha ecológica, migração de monstros, Floresta dos Espíritos e corrupção do ambiente.  
**Evitar:** copiar Priestess na cura ou Shaman nos mesmos totens/espíritos.

### 3. Warlock / Hexblade → **Ocultista**
**Papel:** Debuffer + DoT + controle de alvo.  
**Fantasia:** usa conhecimento proibido das ruínas, marcas e pactos/selos — sem necessariamente servir Midranda.  
**Por que funciona:** dá forma jogável à ideia de que estudar a corrupção é perigoso e que conhecimento antigo tem preço.  
**Melhor encaixe:** classe neutra desbloqueada em Ruinen/Navisko ou Morion dissidente.  
**Evitar:** “mago roxo”; precisa de mecânica própria de maldições, marcas ou custo/risco.

### 4. Barbarian → **Berserker Tempskron**
**Papel:** Scrapper DPS + sobrevivência por risco.  
**Fantasia:** dano crescente, fúria, HP como recurso, quebrar defesa.  
**Por que funciona:** visual e mecanicamente é muito Priston.  
**Problema:** o Fighter já possui identidade agressiva e até mecânicas de berserk; portanto só vale como nova classe se usar arma, ritmo e recurso claramente diferentes.

## Boas ideias, mas melhores como especialização

### Artificer
Combina com tecnologia Tempskron, mas entra diretamente no território do **Mechanician**. Melhor como árvore/avanço do Mecânico, focada em torres, minas, drones ou alquimia de campo.

### Duskblade / Spellblade / Warblade
Uma lâmina com magia rúnica combina com o universo e com a Arma Antiga, mas pode colidir com Knight, Fighter e Magician. Funciona melhor como classe desbloqueável de endgame ou especialização rúnica.

### Bard
O papel de Buffer encaixa; a estética musical clássica, menos. Uma reinterpretação como **Arauto**, **Cantor de Guerra** ou **Orador de Batalha** encaixaria melhor do que um bardo de taverna.

### Necromancer / Dread Necromancer
A lore possui mortos-vivos suficientes para justificar a fantasia, mas como classe inicial tende a conflitar com o tom heroico das duas tribos. Funciona melhor como classe proibida/desbloqueável, NPC ou especialização do Ocultista.

## Classes que eu não adicionaria como novas bases

- **Paladin/Crusader:** Knight já ocupa esse território.
- **Cleric/Healer/Favored Soul:** Priestess já ocupa.
- **Ranger/Scout:** Archer e Atalanta já cobrem boa parte.
- **Rogue:** Assassin já ocupa.
- **Wizard/Sorcerer/Warmage/Evoker:** Magician já ocupa.
- **Shaman/Spirit Shaman:** redundante se a build já usa Shaman.
- **Fighter/Warrior/Samurai/Swashbuckler:** pouca diferenciação frente ao bloco físico existente.
- **Psion/Soulknife/Truenamer/Binder:** podem funcionar, mas exigem acrescentar uma linguagem cosmológica nova antes de parecerem “Priston”.

## Pacote recomendado de expansão

Se a meta for lançar **quatro classes realmente diferentes**, a combinação mais coerente é:

```text
TEMPSKRON
  Comandante (Marshal)      — Buffer / Tank de suporte
  Berserker                 — DPS de risco / bruiser

MORION
  Guardião Natural (Druid)  — Controller / Petmaster / suporte
  Ocultista (Warlock)       — Debuffer / DoT / controle
```

Se quiser reduzir sobreposição com Fighter, substitua Berserker por uma classe **Trapper/Artificer**, mas faça dela um ramo tecnológico que não copie o Mecânico.

---

# 8. Próximos passos de design

Antes de adicionar novas ligações aos mapas 47–52, vale localizar no código a rotina que instancia/teleporta para esses campos. A tabela estática não conta a história inteira desses acessos.

Para as classes, o próximo documento deveria definir para cada candidata:

- tribo;
- arma exclusiva;
- atributo primário/secundário;
- função em party;
- identidade 1x1 e AoE;
- quatro tiers de skills + Tier 5;
- três skills assinatura;
- fraqueza explícita;
- mapa/NPC de origem;
- quest de avanço que introduz sua lore.

Assim a classe nasce como parte de Priston, e não como uma classe de D&D apenas renomeada.
