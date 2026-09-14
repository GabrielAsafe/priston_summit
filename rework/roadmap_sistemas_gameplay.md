# Plano de Engenharia — Modernização e Expansão dos Sistemas de Gameplay

**Projeto:** Source atual / base legada Priston-like  
**Documento complementar:** `roadmap_modernizacao_ui.md`  
**Escopo principal:** Classes jogáveis, Guilda dos Aventureiros/Covenants, Profissões, Pesca, Mineração, Caça, Quest System V2 e Narrativa/História  
**Data da análise:** 2026-09-14  
**Status:** Proposta de arquitetura + roadmap de migração

---

# 1. Resumo executivo

A source atual suporta a implementação de todos os sistemas propostos sem exigir troca imediata de engine. O maior obstáculo não é capacidade técnica, mas a forma como a lógica existente está distribuída: regras de gameplay, persistência, rede, UI e conteúdo ainda aparecem frequentemente misturadas em arquivos legados grandes e em `switch`/arrays fixos.

A recomendação é **não implementar Fishing, Mining, Hunting, Covenants ou novas classes diretamente dentro dos singletons e handlers antigos**. Antes de adicionar volume de conteúdo, deve ser criada uma camada pequena de infraestrutura comum que forneça:

- eventos de gameplay tipados;
- serviços server-authoritative;
- repositories de persistência;
- definições data-driven separadas de estado runtime;
- registries para conteúdo extensível;
- sistema genérico de condições;
- sistema genérico de recompensas/modificadores;
- adapters para manter compatibilidade com quests, `JOBS`, clan skills e classes antigas.

O objetivo arquitetural deste documento é chegar ao seguinte modelo:

```text
                    CONTEÚDO / DEFINIÇÕES
        Classes | Quests | Factions | Profissões | História
                            |
                            v
                     Gameplay Services
                            |
          +-----------------+------------------+
          |                 |                  |
          v                 v                  v
   Persistence         Gameplay Events        Network
          |                 |                  |
          +-----------------+------------------+
                            |
                            v
                      Client Presentation
                            |
                            v
                          UI V2
```

E, sobretudo:

> **Adicionar conteúdo novo não deve exigir editar o dispatcher central, `OnSever`, o inventário ou uma lista fixa de classes.**

---

# 2. Resultado desejado

Ao final deste roadmap, a source deve permitir operações como:

### Adicionar uma nova classe

```text
1. criar NecromancerClass.cpp
2. implementar IPlayableClassModule
3. registrar com REGISTER_PLAYABLE_CLASS(NecromancerClass)
4. associar ClassAssetDefinition + SkillTreeDefinition
5. compilar
6. criação de personagem enumera a classe automaticamente
```

Sem editar:

```text
HoLogin.cpp
TempNewCharacterInit[4][6]
MorNewCharacterInit[4][6]
CharSkillCode[8]
ArmorJobNum[9]
MAX_JOBNUM
```

### Adicionar uma profissão

```text
ProfessionDefinition + atividades que produzem ProfessionXP
```

sem criar um novo singleton estilo `JOBS`.

### Adicionar uma quest

```text
QuestDefinition
  + Objectives[]
  + Rewards[]
  + Conditions[]
```

sem escrever um novo `switch` em C++.

### Adicionar uma facção/covenant

```text
FactionDefinition
  + ranks
  + reputation rules
  + modifiers
  + quest pool
```

sem utilizar o Clan como armazenamento improvisado.

### Reescrever um capítulo da história

```text
StoryArc -> Chapter -> Dialogue -> Quest -> WorldFlag
```

sem transformar narrativa em código procedural espalhado.

---

# 3. Diagnóstico técnico da source atual

## 3.1 Build: restrição importante de linguagem

O build atual não usa o mesmo padrão de C++ nos dois executáveis:

```text
Client: -std=gnu++17
Server: -std=gnu++14
```

Isto influencia diretamente a arquitetura compartilhada.

### Recomendação

Existem duas opções válidas:

**Opção A — recomendada a médio prazo:** elevar o servidor para C++17 e validar regressões de build.

**Opção B — menor risco inicial:** manter todo o novo núcleo em `Shared/` compatível com C++14 até o servidor ser atualizado.

Enquanto o servidor estiver em C++14, evitar no código compartilhado dependências obrigatórias de:

- `std::variant`;
- `std::optional` de C++17;
- structured bindings;
- filesystem padrão.

No cliente, a UI V2 pode continuar utilizando C++17.

---

## 3.2 O build já favorece módulos auto-descobertos

Os scripts de build localizam arquivos fonte com `find` dentro de:

```text
SrcGame/src
SrcServer/src
```

Portanto um novo `.cpp` entra naturalmente na compilação sem manter manualmente uma lista central de sources.

Isso é favorável ao objetivo de classes plugáveis. Um static registrar com registry baseado em function-local static permite que um arquivo de classe se registre durante inicialização sem depender da ordem de inicialização entre translation units.

---

## 3.3 Classes jogáveis estão parcialmente preparadas, mas fortemente hardcoded

Em `Shared/smPacket.h` existem atualmente:

```cpp
#define MAX_JOBNUM       8
#define MAX_JOB_CAPACITY 10

#define JOBCODE_FIGHTER       1
#define JOBCODE_MECHANICIAN   2
#define JOBCODE_ARCHER        3
#define JOBCODE_PIKEMAN       4
#define JOBCODE_ATALANTA      5
#define JOBCODE_KNIGHT        6
#define JOBCODE_MAGICIAN      7
#define JOBCODE_PRIESTESS     8
#define JOBCODE_ASSASSIN      9
#define JOBCODE_SHAMAN       10
```

Isso mostra que a source já reconhece conceitualmente Assassin e Shaman, mas o runtime habilitado permanece em 8 classes.

A criação de personagem ainda possui:

```cpp
#define CHARACTER_JOB_MAX 4
```

por raça, arrays fixos como:

```cpp
int TempNewCharacterInit[4][6];
int MorNewCharacterInit[4][6];
```

E montagem manual:

```cpp
JobCharacter[...].smCharInfo.JOB_CODE = JOBCODE_MECHANICIAN;
AddJobCharacter(...);
...
```

Há inclusive código comentado para Assassin dentro de `HoLogin_legacy_client.inl`.

### Outros hardcodes encontrados

A limitação não está apenas na tela de criação:

- `ArmorJobNum[9]`;
- `szArmorNumFile[MAX_JOBNUM][...]`;
- tabelas de faces distintas por classe;
- `CharSkillCode[8]` no sistema de skills;
- dezenas de `switch(JOB_CODE)`;
- restrições de item usando `JobCodeMask`/`JobBitMask`;
- lógicas de combate com listas explícitas de classes;
- autoridade de Tier 5 rejeitando `JOB_CODE > JOBCODE_PRIESTESS`.

Exemplo real da autoridade de Tier 5:

```cpp
if(lpPlayInfo->smCharInfo.JOB_CODE < JOBCODE_FIGHTER ||
   lpPlayInfo->smCharInfo.JOB_CODE > JOBCODE_PRIESTESS)
    return FALSE;
```

Portanto **a criação dinâmica de classes precisa ser um refactor transversal**.

---

## 3.4 `JOBS` existe, mas não deve ser a base direta das novas profissões

A source possui um sistema chamado `JOBS`, com profissões visíveis no cliente:

```text
1 ARCANISTA
2 FERREIRO
3 LENHADOR
4 MINERADOR
5 ARTESÃO
```

Persistência atual:

```text
JOBS__Jobs_Info
  ID
  NICK
  JOB
  Job_Level
  Job_xp
  M_A
  M_F
  M_L
  M_M
  M_R
```

E marketplace em:

```text
JOBS__Jobs_shop
```

Entretanto a implementação atual mistura:

- profissão;
- mineração/material;
- crafting;
- market;
- UI;
- packets;
- SQL;
- regras de level;
- receitas.

Além disso existe uma inconsistência concreta de protocolo:

```text
Servidor: JOBS_PACK.item[70]
Cliente:  JOBS_PACK.item[100]
```

E parte da progressão é explicitamente hardcoded no fluxo de morte de monstro. Atualmente matar um monstro chamado `"Hopi"` pode chamar:

```cpp
JOBS::GetInstance()->AddExpJOBS(...);
JOBS::GetInstance()->AddminJOBS(...);
```

Isso significa que o “Mining” legado não representa mineração de nós do mundo; é uma recompensa acoplada à morte de um monstro específico.

### Conclusão

`JOBS` é útil para:

- importar progressão existente;
- entender regras/receitas existentes;
- manter compatibilidade temporária;

mas não deve receber Fishing/Mining/Hunting novos diretamente.

---

## 3.5 Quest system moderno existe, mas ainda é especializado em matar monstros

O header legado de `sinQuest` contém explicitamente a orientação de que novas quests devem utilizar:

```text
cQuestWindow + cQuestHandler + QuestDB
```

Isso é uma boa base de migração.

O QuestDB atual possui:

```text
RepeatedQuest
DailyQuest
SingleQuest
QuestMonsters
MonsterKilled
Progress
Complete
Rewards
ItemRewards
```

A estrutura moderna já suporta:

```cpp
StartQuest(...)
CancelQuest(...)
GetReward(...)
HasCompletedQuest(...)
KillMonster(...)
CompleteQuestObjective(...)
```

Porém os objetivos ainda são modelados quase exclusivamente como `MonsterKilled`.

Há ainda limites inconsistentes:

- `smTRANS_QUEST_FULL` comporta até 10 monsters e 10 rewards;
- `cQuestHandler::GetQuestInfo()` carrega no máximo 4 monsters em partes do fluxo;
- `GetReward()` usa arrays locais `MonsterKilled[4]` e `MonsterCounts[4]`;
- o client `cQuestWindow` reduz `MonsterCount` e `RewardCount` para 4.

Logo, aumentar arrays não resolve o problema. O modelo deve passar de **monster list** para **objective list**.

---

## 3.6 Sistema antigo de quests ainda faz parte do save e de várias regras

O save legado ainda possui:

```cpp
sQUEST_INFO QuestInfo;
sLAST_QUEST_INFO LastQuestInfo;
DWORD dwLevelQuestLog;
DWORD dwElementaryQuestLog;
```

`sinQuest`, `HaQuest`, change-job quests e handlers especiais permanecem conectados a:

- NPCs;
- save/load;
- arena de quests;
- monster damage/kill;
- message boxes;
- help UI;
- evolução de classe.

Portanto o correto não é apagar o legado. Deve existir um adapter e uma migração progressiva.

---

## 3.7 Clan e Clan Skills podem inspirar Covenants, mas não devem ser reutilizados semanticamente

A source já possui um sistema social de clans e persistência extensa em `ClanDB`, além de lógica de Bless Castle.

Também existem três efeitos de Clan Skill:

```text
CLANSKILL_ABSORB
CLANSKILL_ATTACK
CLANSKILL_EVASION
```

Isso prova que o runtime já consegue manter buffs associados a uma afiliação/grupo.

Porém Clan significa:

- organização criada por jogador;
- membership social;
- líder;
- castle/siege;
- clan money/points;
- identidade pública.

Uma Guilda dos Aventureiros/Covenant é outro domínio:

- facção do mundo;
- reputação individual;
- ranks de NPC;
- perks/buffs;
- quests da facção;
- escolhas possivelmente exclusivas.

Misturar ambos geraria regras especiais em Clan. O sistema deve ser novo.

---

## 3.8 NPC dialogue atual é adequado para texto estático, não para narrativa moderna

`smCHAR_MONSTER_INFO` possui:

```cpp
char* lpNpcMessage[NPC_MESSAGE_MAX];
int NpcMsgCount;
int QuestCode;
int QuestParam;
```

O loader aceita mensagens como `*J_CHAT`, e o servidor pode escolher mensagens do array.

Isto atende NPCs com frases estáticas, mas não fornece nativamente:

- árvores de diálogo;
- condições por jogador;
- escolhas;
- consequências;
- flags narrativas;
- branches;
- integração genérica com reputação e quests.

Uma reescrita séria da história precisa de `DialogueService` e `NarrativeState` separados dessas mensagens estáticas.

---

# 4. Princípios arquiteturais para os novos sistemas

## 4.1 Server authoritative

O servidor decide:

- conclusão de objective;
- ganho de XP de profissão;
- recurso obtido;
- resultado da pesca;
- reputação;
- rank de covenant;
- buff válido;
- recompensa;
- prerequisitos de quest;
- disponibilidade de classe/skill quando relevante.

O cliente apresenta, anima e envia intenção.

Nunca:

```text
client -> "pesquei item X"
```

Preferir:

```text
client -> "tentei concluir a ação da sessão 781"
server -> valida sessão -> resolve resultado -> concede reward
```

---

## 4.2 Definition != Runtime State

Separar sempre:

```text
Definition
    configuração estática de conteúdo

State
    estado individual do jogador ou instância
```

Exemplo:

```cpp
ProfessionDefinition
PlayerProfessionState
```

Em vez de uma classe singleton conter configuração + estado + UI + DB.

---

## 4.3 Conteúdo data-driven, comportamento extensível por C++

Usar C++ para comportamento e extensão real.

Usar dados para:

- nomes;
- assets;
- valores de tuning;
- loot tables;
- prerequisites;
- ranks;
- textos;
- objetivos;
- posições de nodes;
- tempos de respawn;
- rewards.

Isso reduz recompilações por conteúdo.

---

## 4.4 Nenhum novo sistema deve depender diretamente da UI

Fluxo correto:

```text
UI -> ViewModel/Controller -> Client Service -> Packet
                                   |
                                   v
                              Server Service
```

O `ProfessionService` não deve conhecer `ProfessionWindow`.

---

## 4.5 Nenhum novo sistema deve adicionar lógica de feature diretamente a `OnSever`

O fluxo de morte atual já concentra quest, drop, XP, battle pass e `JOBS`.

Continuar acrescentando:

```cpp
Covenant::OnKill(...)
Fishing::...
Hunting::...
Story::...
```

diretamente nesse arquivo aumentaria o acoplamento.

A saída deve ser um ponto central de evento.

---

# 5. Fundação compartilhada recomendada

Antes dos sistemas individuais, implementar seis serviços pequenos.

---

## 5.1 `GameplayEventHub`

### Objetivo

Transformar hooks antigos em eventos consumíveis por múltiplos sistemas.

### Eventos iniciais

```cpp
struct MonsterKilledEvent;
struct ItemCollectedEvent;
struct ItemCraftedEvent;
struct NpcInteractedEvent;
struct WorldInteractableUsedEvent;
struct AreaEnteredEvent;
struct ResourceGatheredEvent;
struct FishingResolvedEvent;
struct ProfessionLeveledEvent;
struct QuestCompletedEvent;
struct FactionRankChangedEvent;
```

### Exemplo

Fluxo atual:

```text
monster death
 -> quest handler
 -> battle pass
 -> JOBS hardcode
 -> drop
```

Fluxo alvo:

```text
monster death
 -> GameplayEventHub.Publish(MonsterKilledEvent)
       -> QuestObjectiveRouter
       -> HuntingService
       -> BattlePassAdapter
       -> FactionReputationRules
       -> StoryTriggerService
```

### Classes

```cpp
class IGameplayEventListener;
class GameplayEventHub;
class LegacyGameplayEventBridge;
```

Não é necessário implementar um event bus extremamente genérico. Um dispatcher tipado simples é suficiente e mais fácil de depurar.

---

## 5.2 `ConditionService`

Quests, factions e história repetirão as mesmas perguntas:

```text
level >= X?
classe == Y?
quest completa?
possui item?
rank da facção >= N?
profissão >= N?
flag narrativa ativa?
mapa atual == X?
```

Centralizar essas regras.

### Tipos sugeridos

```cpp
enum class ConditionType
{
    LevelAtLeast,
    LevelAtMost,
    ClassIs,
    HasItem,
    QuestCompleted,
    QuestActive,
    ProfessionLevelAtLeast,
    FactionRankAtLeast,
    StoryFlagEquals,
    MapIs,
    Custom
};
```

Em C++14, usar tagged struct em vez de `std::variant`.

### Classes

```cpp
struct ConditionDefinition;
struct ConditionContext;
class IConditionEvaluator;
class ConditionService;
```

---

## 5.3 `RewardService`

Hoje cada feature concede coisas de forma diferente.

O novo serviço deve suportar:

```text
Item
Gold
CharacterXP
ProfessionXP
FactionReputation
Coin/TCoin
Title
StoryFlag
Unlock
GameplayModifier
```

### Classes

```cpp
enum class RewardType;
struct RewardDefinition;
struct RewardGrantRequest;
struct RewardGrantResult;
class RewardService;
```

### Requisito crítico

Toda recompensa deve aceitar um `grantId`/idempotency key para evitar duplicação em retry de packet ou reconexão.

Exemplo:

```text
quest:1007:reward:player:1234
```

---

## 5.4 `GameplayModifierService`

Deve servir como base para:

- buffs de covenant;
- perks de profession;
- buffs de eventos;
- futuros equipamentos/passivas;
- eventualmente adapter dos clan skills antigos.

### Modelo

```cpp
enum class ModifierStat
{
    AttackDamagePercent,
    AbsorbFlat,
    EvasionPercent,
    DropRatePercent,
    ProfessionYieldPercent,
    HuntingDamagePercent,
    FishingLuck,
    MiningYield,
    ExperiencePercent
};

struct GameplayModifier
{
    ModifierId id;
    ModifierStat stat;
    ModifierOperation operation;
    float value;
    ModifierSource source;
};
```

### Classes

```cpp
class GameplayModifierService;
class IModifierProvider;
class LegacyClanSkillModifierAdapter;
```

Não usar um skill code como representação genérica de um buff de covenant.

---

## 5.5 `ContentRegistry`

Registry para definitions carregadas no boot:

```cpp
class ContentRegistry
{
public:
    const QuestDefinition* FindQuest(QuestId) const;
    const ProfessionDefinition* FindProfession(ProfessionId) const;
    const FactionDefinition* FindFaction(FactionId) const;
    const GatheringNodeDefinition* FindGatheringNode(NodeDefinitionId) const;
};
```

Validar conteúdo no startup e falhar cedo quando houver:

- ID duplicado;
- asset obrigatório ausente;
- quest apontando para objective inválido;
- reward inválido;
- faction rank inconsistente;
- referência circular proibida.

---

## 5.6 Repository boundary

Não espalhar `SELECT/UPDATE/INSERT` por serviços.

Criar interfaces como:

```cpp
class IQuestProgressRepository;
class IProfessionRepository;
class IFactionRepository;
class IStoryStateRepository;
```

E implementações:

```cpp
class SqliteQuestProgressRepository;
class SqliteProfessionRepository;
class SqliteFactionRepository;
class SqliteStoryStateRepository;
```

A lógica de domínio não deve conhecer strings SQL.

---

# 6. Arquitetura de classes jogáveis plugáveis

## 6.1 Meta

Chegar o mais próximo possível de:

> “criei a classe C++, defini assets e ela automaticamente passa a fazer parte do jogo”.

Em C++, a mera existência de uma classe não pode ser descoberta sem algum mecanismo de registro. A solução prática é um macro de auto-registro.

---

## 6.2 Tipos recomendados

### `ClassId`

Não utilizar índices de array como identidade.

```cpp
typedef uint16_t ClassId;
```

Manter adapter para `DWORD JOB_CODE` enquanto o protocolo legado existir.

---

### `PlayableClassDefinition`

```cpp
struct BaseStats
{
    int strength;
    int spirit;
    int talent;
    int dexterity;
    int health;
};

struct PlayableClassDefinition
{
    ClassId id;
    const char* internalName;
    const char* displayNameKey;
    TribeId tribe;
    BaseStats baseStats;
    SkillTreeId skillTree;
    ClassAssetSetId assetSet;
    EquipmentProfileId equipmentProfile;
    bool enabled;
};
```

---

### `IPlayableClassModule`

```cpp
class IPlayableClassModule
{
public:
    virtual ~IPlayableClassModule() {}
    virtual const PlayableClassDefinition& Definition() const = 0;

    // Apenas hooks realmente específicos da classe.
    virtual void RegisterClassBehaviors(ClassBehaviorRegistry&) {}
};
```

A maioria das diferenças entre classes deve ficar em data/configuração. Não criar virtual method para cada fórmula só porque cada classe existe.

---

### `PlayableClassRegistry`

```cpp
class PlayableClassRegistry
{
public:
    static PlayableClassRegistry& Instance();

    void Register(IPlayableClassModule* module);
    const IPlayableClassModule* Find(ClassId id) const;
    std::vector<const IPlayableClassModule*> GetPlayable() const;
    std::vector<const IPlayableClassModule*> GetByTribe(TribeId tribe) const;
    size_t CountPlayable() const;
};
```

---

### Auto-registro

```cpp
class NecromancerClass : public IPlayableClassModule
{
public:
    const PlayableClassDefinition& Definition() const override;
};

REGISTER_PLAYABLE_CLASS(NecromancerClass);
```

Implementar `REGISTER_PLAYABLE_CLASS` usando um registrar estático que chama `PlayableClassRegistry::Instance()`, onde `Instance()` usa function-local static.

---

## 6.3 Assets

Criar:

```cpp
struct ClassAssetDefinition
{
    std::vector<std::string> bodyModels;
    std::vector<std::string> faceModels;
    std::vector<std::string> armorModelPrefixes;
    std::string selectionIcon;
    std::string portrait;
};
```

E:

```cpp
class ClassAssetRegistry;
class CharacterModelResolver;
```

### O que deve desaparecer gradualmente

```text
szTempFighterBodyName[]
szTempMechBodyName[]
szMorKnightBodyName[]
szModel_FighterFaceName[][]
szModel_MechFaceName[][]
...
```

Esses arrays podem continuar por trás de `LegacyClassAssetAdapter` enquanto as oito classes antigas são migradas.

---

## 6.4 Criação de personagem

Criar:

```cpp
class CharacterCreationService;
class CharacterCreationCatalog;
class CharacterCreationValidator;
class LegacyCharacterCreationAdapter;
```

A UI deve fazer:

```cpp
auto classes = CharacterCreationCatalog::GetClassesForTribe(selectedTribe);
```

Nunca:

```cpp
AddJobCharacter(Mechanician);
AddJobCharacter(Fighter);
...
```

---

## 6.5 Stats iniciais

Substituir:

```cpp
TempNewCharacterInit[4][6]
MorNewCharacterInit[4][6]
```

por:

```cpp
classDefinition.baseStats
```

O servidor deve validar novamente os stats recebidos na criação; não confiar nos valores enviados pelo cliente.

---

## 6.6 Skill tree por classe

O legado possui grupos e listas fixas, inclusive `CharSkillCode[8]`.

Criar:

```cpp
struct SkillTreeDefinition;
struct SkillDefinition;
class SkillRegistry;
class ClassSkillResolver;
class LegacySkillBridge;
```

A relação deve ser:

```text
ClassDefinition -> SkillTreeId -> SkillTreeDefinition -> SkillIds[]
```

Em vez de deduzir classe por posição fixa de array.

### Migração

Não reescrever o cálculo de todas as skills no primeiro passo.

Primeiro:

```text
SkillRegistry
  -> skill antiga aponta para LegacySkillHandler
```

Depois skills novas podem nascer como handlers separados.

---

## 6.7 Equipamento e `JobCodeMask`

O modelo atual usa máscaras de bits para restrição de item.

Isso pode ser mantido temporariamente, mas não deve ser o contrato final do domínio.

Criar:

```cpp
struct ClassRestrictionSet;
class EquipmentEligibilityService;
class LegacyJobMaskAdapter;
```

### Curto prazo

Para classes dentro da capacidade atual de bits, gerar mask automaticamente a partir do registry/adapters.

### Longo prazo

Usar IDs/tags de classe em vez de depender de `DWORD` como catálogo completo.

Exemplo:

```text
AllowedClasses: [fighter, knight, necromancer]
RequiredTags: [caster]
DeniedTags: [ranged]
```

---

## 6.8 Checklist de hardcodes a eliminar

Antes de considerar o sistema plugável concluído, auditar e substituir:

- `MAX_JOBNUM` como quantidade lógica;
- `CHARACTER_JOB_MAX`;
- `TempNewCharacterInit`;
- `MorNewCharacterInit`;
- `ArmorJobNum`;
- `CharSkillCode[8]`;
- checks por range numérico `JOB_CODE <= PRIESTESS`;
- `switch(JOB_CODE)` de regras que podem ser metadata/tag;
- arrays de face/body por classe;
- código que assume quatro classes por tribe;
- tabelas de localização duplicadas por classe quando puderem usar localization key.

---

# 7. Guilda dos Aventureiros / Covenant / Faction System

## 7.1 Decisão de domínio

Não reutilizar Clan.

Criar o conceito mais geral de **Faction**.

Um faction pode representar:

```text
Adventurer Guild
Covenant
Hunter Lodge
Mining Consortium
Religious Order
Story Faction
```

---

## 7.2 Modelo recomendado

```cpp
enum class FactionType
{
    AdventurerGuild,
    Covenant,
    Organization,
    StoryFaction
};

struct FactionRankDefinition
{
    int rank;
    int reputationRequired;
    std::vector<RewardDefinition> unlockRewards;
    std::vector<ModifierDefinition> modifiers;
};

struct FactionDefinition
{
    FactionId id;
    FactionType type;
    std::string internalName;
    std::string displayNameKey;
    std::string exclusiveGroup;
    std::vector<FactionRankDefinition> ranks;
};
```

`exclusiveGroup` permite Dark Souls style:

```text
covenant_primary
```

onde o jogador pode ter apenas um covenant ativo nesse grupo, sem impedir outras reputações não exclusivas.

---

## 7.3 Estado do jogador

```cpp
struct PlayerFactionState
{
    FactionId factionId;
    int reputation;
    int rank;
    bool active;
};
```

---

## 7.4 Classes recomendadas

```text
FactionService
FactionRegistry
FactionReputationService
FactionRankService
FactionMembershipService
FactionModifierProvider
FactionQuestProvider
FactionRewardService
IFactionRepository
SqliteFactionRepository
```

### Responsabilidades

`FactionService`
: API principal e invariantes.

`FactionReputationService`
: recebe regras/eventos e calcula reputação.

`FactionRankService`
: promove/rebaixa rank e dispara rewards.

`FactionModifierProvider`
: traduz rank/membership em buffs para `GameplayModifierService`.

`FactionQuestProvider`
: disponibiliza quests condicionadas à facção/rank.

---

## 7.5 Buffs

Não transformar cada buff em uma skill contínua legada.

Exemplo:

```text
Guilda dos Caçadores — Rank 2
  HuntingYield +5%
  DamageVsBeast +3%

Covenant of Ash — Rank 3
  FireResistance +5%
  OnBossKill Reputation +10%
```

O `GameplayModifierService` calcula os efeitos ativos.

Clan Skills antigos podem ser adaptados ao mesmo sistema posteriormente.

---

## 7.6 Persistência sugerida

```sql
FactionDefinition             -- opcional se definitions estiverem em arquivo
FactionRankDefinition
PlayerFaction
FactionReputationLog
FactionUnlock
```

`PlayerFaction`:

```text
CharacterId
FactionId
Reputation
Rank
Active
JoinedAt
UpdatedAt
```

A identidade recomendada é `CharacterId`, não nome do personagem, para evitar problemas com rename.

---

## 7.7 Regras Dark Souls-like

Suportar desde o início:

- join condition;
- leave policy;
- betrayal penalty;
- exclusive group;
- reputation gain/loss;
- rank thresholds;
- faction-specific quest availability;
- modifiers por rank;
- unlocks cosméticos/items/títulos.

Não é necessário implementar invasões/PvP covenant no primeiro release; o domínio deve apenas não impedir isso no futuro.

---

# 8. Profession System V2

## 8.1 Objetivo

Substituir o conceito monolítico `JOBS` por um sistema genérico de progressão profissional.

---

## 8.2 Modelo

```cpp
struct ProfessionDefinition
{
    ProfessionId id;
    std::string internalName;
    std::string displayNameKey;
    int maxLevel;
    ExperienceCurveId xpCurve;
};

struct PlayerProfessionState
{
    ProfessionId professionId;
    int level;
    int64_t experience;
};
```

---

## 8.3 Classes

```text
ProfessionRegistry
ProfessionService
ProfessionProgressionService
ProfessionExperienceCurve
ProfessionRewardProvider
IProfessionRepository
SqliteProfessionRepository
LegacyJobsAdapter
LegacyJobsMigrationService
```

---

## 8.4 Scope de progressão

O legado usa `Jobs_Info.ID` ligado ao account.

É necessário decidir explicitamente se profissões novas são:

```text
Account scoped
ou
Character scoped
```

Para um RPG com classes, história e covenant por personagem, **Character scope é o desenho mais coerente**.

Porém a migração do estado antigo deve preservar dados. Sugestão:

1. importar o `JOBS` antigo para uma tabela `LegacyProfessionImport`;
2. na primeira migração controlada, associar o progresso a um personagem escolhido/regra definida;
3. não destruir a tabela antiga até validação.

Se a decisão de design for manter profissão account-wide, o repository suporta `AccountId` como owner. Não deixar essa escolha implícita.

---

# 9. Gathering Core

Pesca e mineração compartilham grande parte da infraestrutura. Criar o core antes dos dois sistemas.

---

## 9.1 `WorldInteractionService`

Sistema para o jogador interagir com algo no mundo que não seja necessariamente:

- monstro;
- item dropado;
- NPC de loja.

### Tipos

```cpp
struct WorldInteractableId;
struct InteractionRequest;
struct InteractionContext;
struct InteractionResult;

class IWorldInteractableHandler;
class WorldInteractionService;
```

O servidor valida:

- mapa;
- distância;
- entidade ativa;
- cooldown;
- line/range quando necessário;
- ferramenta;
- estado do jogador;
- concorrência.

---

## 9.2 `GatheringNodeDefinition`

```cpp
enum class GatheringType
{
    Mining,
    Logging,
    Herbalism,
    FishingSpot
};

struct GatheringNodeDefinition
{
    GatheringNodeDefinitionId id;
    GatheringType type;
    int requiredProfessionLevel;
    ToolTag requiredTool;
    LootTableId lootTable;
    int interactionDurationMs;
    int respawnMinMs;
    int respawnMaxMs;
};
```

---

## 9.3 `GatheringNodeInstance`

```cpp
struct GatheringNodeInstance
{
    GatheringNodeInstanceId instanceId;
    GatheringNodeDefinitionId definitionId;
    MapId mapId;
    Vector3 position;
    GatheringNodeState state;
    Timestamp respawnAt;
};
```

---

## 9.4 Serviços

```text
GatheringService
GatheringNodeRegistry
GatheringSpawnService
GatheringLootResolver
GatheringValidationService
GatheringSessionService
```

---

## 9.5 Representação visual inicial

A engine atual possui `smCHAR`, `STG_AREA`, NPCs e monsters como entidades visíveis amplamente integradas.

Existem duas estratégias:

### MVP de baixo risco

Representar nodes visualmente através de um adapter para uma entidade/modelo já suportado pelo mundo, mas impedir combate e tratar interação no novo service.

### Arquitetura final

Criar uma categoria própria de `WorldInteractable`/resource entity.

Não codificar permanentemente minério como “monster invisível”. O adapter deve ser temporário e explícito.

---

# 10. Mining V2

## 10.1 O que remover do comportamento legado

Mineração não deve ocorrer por:

```cpp
if (monsterName == "Hopi")
    AddminJOBS(...);
```

Esse hook deve desaparecer após a migração.

---

## 10.2 Loop alvo

```text
jogador encontra vein
   -> interact
   -> servidor valida pickaxe + level + range
   -> inicia GatheringSession
   -> duração/animation
   -> servidor resolve loot table
   -> concede minério
   -> concede Mining XP
   -> marca node depleted
   -> programa respawn
   -> publica ResourceGatheredEvent
```

---

## 10.3 Classes específicas

O core genérico cobre quase tudo. Mining precisa apenas de extensões quando necessário:

```text
MiningInteractionHandler
MiningYieldPolicy
MiningNodePresentationAdapter
```

Não criar `MiningManager` gigante com SQL/UI/reward dentro.

---

## 10.4 Conteúdo

Exemplo de definition:

```text
iron_vein_low
  profession: mining
  requiredLevel: 5
  tool: pickaxe
  respawn: 120-180s
  lootTable: mining_iron_t1
```

Loot table:

```text
IronOre       weight 80
DenseIronOre  weight 15
GemShard      weight 5
```

---

# 11. Fishing V2

## 11.1 Sistema novo

Não foi identificado um sistema completo de pesca na source. Deve nascer na arquitetura nova.

---

## 11.2 Fishing Zones

Evitar criar centenas de objetos individuais se a pescaria ocorrer em água.

```cpp
struct FishingZoneDefinition
{
    FishingZoneId id;
    MapId mapId;
    AreaShape area;
    int requiredLevel;
    LootTableId lootTable;
    FishingRuleSetId ruleSet;
};
```

O server pode validar a posição do jogador/cast contra a zona.

---

## 11.3 State machine

```cpp
enum class FishingSessionState
{
    Casting,
    WaitingForBite,
    BiteWindow,
    Resolving,
    Completed,
    Cancelled
};
```

### Fluxo

```text
StartFishingRequest
 -> validate zone / rod / state
 -> create FishingSession
 -> server chooses hidden bite window
 -> client receives session state, not loot
 -> bite event
 -> player reacts
 -> server validates timing
 -> FishingLootResolver
 -> RewardService
 -> profession XP
 -> FishingResolvedEvent
```

---

## 11.4 Classes

```text
FishingService
FishingZoneRegistry
FishingSessionService
FishingValidationService
FishingLootResolver
FishingSkillCheckPolicy
FishingInteractionController (client)
FishingSessionViewModel (client)
```

---

## 11.5 Anticheat

O servidor deve guardar:

- session id;
- start time;
- valid bite start/end;
- player position/map;
- rod/tool snapshot;
- loot seed/decision quando aplicável.

O packet de input só informa a ação do jogador.

Nunca aceitar do cliente:

```text
fishId
rarity
reward count
```

como autoridade.

---

# 12. Hunting V2

## 12.1 Aproveitar o hook de kill existente

O servidor já possui um ponto claro de morte de monstro usado por:

- Quest Handler;
- XP;
- Battle Pass;
- drops;
- `JOBS`.

Esse ponto deve publicar `MonsterKilledEvent`.

Hunting então apenas assina o evento.

---

## 12.2 Não identificar presas pelo nome textual

Evitar:

```cpp
strcmp(monsterName, "Wolf") == 0
```

A estrutura de monster já possui campos como:

```text
ClassCode
Nature
Undead
```

que podem servir como seed para classificação, mas o alvo deve ser tags explícitas.

---

## 12.3 `CreatureTagService`

```cpp
enum class CreatureTag
{
    Beast,
    Undead,
    Humanoid,
    Demon,
    Boss,
    Huntable,
    RareHunt
};

class CreatureTagService;
```

O conteúdo pode mapear monster definition -> tags.

---

## 12.4 Classes

```text
HuntingService
HuntingProgressionPolicy
CreatureTagService
HuntingLootResolver
HuntingBountyService
TrophyService          // opcional fase posterior
```

---

## 12.5 Loop inicial

```text
MonsterKilledEvent
 -> creature has Huntable tag?
 -> validate participation/credit
 -> grant Hunting XP
 -> optional skin/trophy roll
 -> publish HuntingProgressEvent
 -> QuestObjectiveRouter may consume it
```

Isso permite quests como:

```text
Hunt 5 beasts
Hunt a rare beast
Obtain 3 pristine hides through Hunting
```

sem duplicar kill logic.

---

# 13. Quest System V2

## 13.1 Objetivo

Transformar o sistema atual de:

```text
Quest -> MonsterKilled rows -> reward
```

em:

```text
Quest
  -> Conditions
  -> Objectives[]
  -> Rewards[]
  -> Followups[]
```

---

## 13.2 `QuestDefinition`

```cpp
struct QuestDefinition
{
    QuestId id;
    std::string internalName;
    std::string titleKey;
    std::string descriptionKey;
    QuestRepeatPolicy repeatPolicy;
    PartyPolicy partyPolicy;
    std::vector<ConditionDefinition> startConditions;
    std::vector<QuestObjectiveDefinition> objectives;
    std::vector<RewardDefinition> rewards;
    std::vector<QuestId> nextQuests;
};
```

---

## 13.3 Objective model

```cpp
enum class QuestObjectiveType
{
    Kill,
    Collect,
    TalkToNpc,
    Interact,
    VisitArea,
    Gather,
    Mine,
    Fish,
    Hunt,
    Craft,
    Defend,
    Escort,
    Custom
};
```

### Estrutura C++14 friendly

```cpp
struct QuestObjectiveDefinition
{
    QuestObjectiveId id;
    QuestObjectiveType type;
    int requiredCount;

    // references genericamente serializáveis
    std::string targetId;
    int intParam1;
    int intParam2;
    std::string stringParam1;
};
```

Depois de upgrade para C++17, pode-se evoluir para tipos mais fortes se desejado.

---

## 13.4 Runtime

```cpp
struct QuestProgress
{
    QuestId questId;
    QuestProgressState state;
};

struct QuestObjectiveProgress
{
    QuestId questId;
    QuestObjectiveId objectiveId;
    int currentCount;
    bool completed;
};
```

---

## 13.5 Classes recomendadas

```text
QuestService
QuestRegistry
QuestAvailabilityService
QuestProgressService
QuestObjectiveRouter
QuestRewardService
QuestPartyCreditPolicy
QuestCompletionService
IQuestProgressRepository
SqliteQuestProgressRepository
LegacyQuestAdapter
LegacyQuestDbAdapter
```

---

## 13.6 `QuestObjectiveRouter`

Conecta eventos do mundo a objectives.

Exemplo:

```text
MonsterKilledEvent
 -> KillObjectiveHandler
 -> HuntObjectiveHandler

ResourceGatheredEvent
 -> GatherObjectiveHandler
 -> MineObjectiveHandler

FishingResolvedEvent
 -> FishObjectiveHandler

NpcInteractedEvent
 -> TalkObjectiveHandler

AreaEnteredEvent
 -> VisitObjectiveHandler
```

### Handlers

```cpp
class IQuestObjectiveHandler;
class KillObjectiveHandler;
class CollectObjectiveHandler;
class TalkObjectiveHandler;
class InteractObjectiveHandler;
class VisitObjectiveHandler;
class GatherObjectiveHandler;
class FishObjectiveHandler;
class HuntObjectiveHandler;
class CraftObjectiveHandler;
```

---

## 13.7 Unificação do sistema antigo e atual

A UI e o resto do jogo devem conhecer apenas:

```cpp
class IQuestFacade
{
public:
    GetAvailableQuests(...);
    GetActiveQuests(...);
    GetCompletedQuests(...);
    StartQuest(...);
    CancelQuest(...);
    ClaimReward(...);
};
```

Por baixo:

```text
QuestFacade
  |
  +-- QuestV2Service
  +-- LegacyQuestDbAdapter       (cQuestHandler atual)
  +-- SinQuestLegacyAdapter      (sinQuest/HaQuest)
```

O journal novo mostra tudo de forma unificada.

---

## 13.8 Estratégia de migração de quests

Não migrar todas simultaneamente.

### Ordem recomendada

1. nova quest V2 simples Kill;
2. provar compatibilidade com UI/journal;
3. adicionar Talk/Visit;
4. adicionar Mine/Fish/Hunt;
5. migrar uma quest antiga representativa;
6. migrar chains de história;
7. deixar change-job/quests extremamente especiais por último.

Cada quest migrada recebe um mapping:

```text
LegacyQuestCode -> QuestId V2
```

para impedir dupla ativação/conclusão.

---

## 13.9 Persistência V2

Sugestão normalizada:

```text
QuestDefinition           // caso DB seja fonte de definitions
QuestObjectiveDefinition
QuestRewardDefinition
QuestPrerequisite

PlayerQuest
PlayerQuestObjective
PlayerQuestRewardClaim
```

### `PlayerQuest`

```text
CharacterId
QuestId
State
StartedAt
CompletedAt
DefinitionVersion
```

### `PlayerQuestObjective`

```text
CharacterId
QuestId
ObjectiveId
Progress
Completed
```

Isso elimina a tabela semanticamente específica `MonsterKilled` como storage universal.

---

# 14. Narrative / Story System

## 14.1 Objetivo

A história não deve ser “um conjunto de quest descriptions”.

Criar uma camada de narrativa capaz de coordenar:

- capítulos;
- quests;
- diálogos;
- condições;
- decisões;
- flags;
- facções;
- unlocks.

---

## 14.2 Modelo

```cpp
struct StoryArcDefinition
{
    StoryArcId id;
    std::vector<StoryChapterId> chapters;
};

struct StoryChapterDefinition
{
    StoryChapterId id;
    std::vector<ConditionDefinition> unlockConditions;
    std::vector<QuestId> quests;
};
```

---

## 14.3 Story flags

```cpp
struct StoryFlagValue
{
    StoryFlagId id;
    int value;
};

class StoryStateService;
class IStoryStateRepository;
```

Exemplos:

```text
met_old_king = 1
saved_village = 1
joined_covenant = ash
chapter_02_complete = 1
```

Preferir flags semânticas e documentadas a bits anônimos em `DWORD`.

---

## 14.4 Dialogue System

```cpp
struct DialogueDefinition
{
    DialogueId id;
    DialogueNodeId startNode;
};

struct DialogueNode
{
    DialogueNodeId id;
    LocalizationKey text;
    std::vector<ConditionDefinition> conditions;
    std::vector<DialogueChoice> choices;
    std::vector<NarrativeActionDefinition> onEnterActions;
};

struct DialogueChoice
{
    LocalizationKey text;
    std::vector<ConditionDefinition> conditions;
    DialogueNodeId nextNode;
    std::vector<NarrativeActionDefinition> actions;
};
```

---

## 14.5 Narrative actions

```text
StartQuest
CompleteQuest
SetStoryFlag
AddFactionReputation
JoinFaction
GiveReward
OpenShop
Teleport
PlayCutscene/Media
EndDialogue
Custom
```

### Classes

```text
NarrativeService
DialogueService
DialogueRegistry
NarrativeActionExecutor
StoryStateService
StoryProgressionService
LegacyNpcDialogueAdapter
```

---

## 14.6 Integração com NPC legado

`lpNpcMessage[]` continua funcionando para NPCs não migrados.

NPCs novos/migrados devem receber um `DialogueId` resolvido pelo novo sistema.

Fluxo:

```text
interact NPC
 -> NPC has DialogueId?
      yes -> DialogueService
      no  -> LegacyNpcDialogueAdapter
```

---

## 14.7 Limitação importante: world phasing

Na análise da source não há uma camada geral de **per-player world phasing** equivalente a engines modernas.

Portanto, no primeiro estágio, a narrativa deve preferir consequências que possam ser individualizadas por:

- diálogo;
- quest availability;
- rewards;
- faction state;
- NPC interaction behavior;
- UI;
- teleports/instâncias já suportadas.

Exemplo que funciona bem:

```text
NPC fala texto diferente e oferece quest diferente conforme story flag.
```

Exemplo que exige infraestrutura adicional:

```text
para jogador A a cidade está destruída;
para jogador B a mesma cidade está intacta;
ambos ocupam o mesmo mapa simultaneamente.
```

Se esse tipo de consequência for desejado, criar posteriormente:

```text
EntityVisibilityService
WorldPhaseId
PlayerPhaseState
PhasedSpawnService
```

Não misturar phasing com o primeiro rework de história.

---

# 15. Data-driven content

## 15.1 Onde armazenar definitions

Três opções são compatíveis:

### SQLite

Prós:

- infraestrutura já existente;
- migrations;
- queries;
- fácil integração server-side.

Contras:

- authoring menos confortável.

### JSON/YAML compilado/validado

Prós:

- fácil de versionar em Git;
- diff legível;
- bom para quests/factions/loot.

Contras:

- exige parser/tooling.

### Lua

A source possui Lua, portanto é tecnicamente possível.

Contudo a recomendação é **não transformar toda a narrativa em scripts arbitrários no início**. Scripts livres são mais difíceis de validar, indexar e migrar.

### Recomendação

Definitions estruturadas em dados + registry C++ de handlers.

Lua apenas como escape hatch para comportamento realmente especial.

---

## 15.2 Manifest de classe

Mesmo com uma classe C++ por playable class, assets/tuning devem poder ficar fora do código.

Exemplo conceitual:

```yaml
id: 11
name: necromancer
tribe: moryon
base_stats:
  strength: 14
  spirit: 32
  talent: 22
  dexterity: 13
  health: 19
skill_tree: necromancer_default
assets: necromancer_default
```

A C++ class referencia `necromancer` e registra comportamento especial, caso exista.

---

# 16. Networking V2

## 16.1 Não reutilizar packets inconsistentes

O mismatch encontrado em `JOBS_PACK` demonstra o risco de copiar structs cliente/servidor independentes.

Novos packet contracts devem existir em `Shared/` como fonte única.

---

## 16.2 Pacotes recomendados

```text
ProfessionStateRequest/Response
GatheringStartRequest
GatheringStateUpdate
GatheringCompleteRequest/Result
FishingStartRequest
FishingStateUpdate
FishingActionRequest
FishingResult
FactionStateRequest/Response
FactionJoinRequest/Result
QuestCatalogPage
QuestStateUpdate
QuestActionRequest/Result
DialogueStartRequest/Response
DialogueChoiceRequest/Response
ClassCatalogResponse          // opcional; principalmente server validation
```

---

## 16.3 Regras

Todo packet novo deve:

- possuir `size` e opcode compatíveis com a stack existente;
- definir limite máximo explícito de arrays;
- validar `count` antes de iterar;
- evitar raw pointers;
- compartilhar a mesma definição estrutural client/server;
- suportar paginação/chunking quando catálogo puder crescer;
- não permitir que cliente determine reward.

---

# 17. Persistência e migrations

## 17.1 IDs estáveis

Novas tabelas devem preferir:

```text
AccountId
CharacterId
```

a nomes textuais.

Nome é atributo mutável, não primary key ideal.

---

## 17.2 Tabelas recomendadas

### Profissões

```text
Gameplay__ProfessionState
Gameplay__ProfessionUnlock
Gameplay__LegacyJobsImport
```

### Factions

```text
Gameplay__PlayerFaction
Gameplay__FactionUnlock
Gameplay__FactionReputationLog
```

### Quests

```text
Gameplay__PlayerQuest
Gameplay__PlayerQuestObjective
Gameplay__QuestRewardClaim
```

### História

```text
Gameplay__StoryFlag
Gameplay__StoryChapter
```

### Gathering

Nodes estáticos podem ser definitions e não precisam ser persistidos individualmente. Persistir apenas quando houver cooldown que precise sobreviver a restart:

```text
Gameplay__GatheringNodeState
```

---

## 17.3 Migrations destrutivas

Nunca substituir imediatamente:

```text
JOBS__Jobs_Info
QuestDB__Progress
QuestDB__MonsterKilled
legacy save quest fields
```

Fases:

```text
read old -> write old + new (quando seguro)
validate
new becomes source of truth
old read-only fallback
remove dependency
cleanup em versão posterior
```

---

# 18. Estrutura de diretórios recomendada

Uma proposta compatível com a organização já iniciada:

```text
Shared/
  gameplay/
    core/
      events/
      conditions/
      rewards/
      modifiers/
    classes/
      PlayableClassDefinition.h
      PlayableClassRegistry.h
    professions/
      ProfessionDefinition.h
    gathering/
      GatheringDefinitions.h
    factions/
      FactionDefinition.h
    quests/
      QuestDefinition.h
      QuestObjectiveDefinition.h
    narrative/
      StoryDefinition.h
      DialogueDefinition.h
    protocol/
      GameplayPackets.h

SrcServer/src/systems/gameplay/
  core/
    GameplayEventHub.cpp
    ConditionService.cpp
    RewardService.cpp
    GameplayModifierService.cpp
  classes/
    PlayableClassRegistry.cpp
    LegacyClassAdapter.cpp
    modules/
      FighterClass.cpp
      ...
      NecromancerClass.cpp
  professions/
    ProfessionService.cpp
    LegacyJobsAdapter.cpp
  gathering/
    GatheringService.cpp
    GatheringSpawnService.cpp
    mining/
    fishing/
    hunting/
  factions/
    FactionService.cpp
  quests/v2/
    QuestService.cpp
    QuestObjectiveRouter.cpp
    objective_handlers/
  narrative/
    NarrativeService.cpp
    DialogueService.cpp
  persistence/
    repositories/

SrcGame/src/systems/gameplay/
  classes/
    CharacterCreationCatalog.cpp
  professions/
    ProfessionClientService.cpp
  gathering/
    GatheringClientService.cpp
  factions/
    FactionClientService.cpp
  quests/v2/
    QuestClientService.cpp
  narrative/
    DialogueClientService.cpp
```

UI visual permanece em `systems/ui/v2` conforme o roadmap anterior.

---

# 19. Integração com a UI V2

O roadmap de UI deve consumir ViewModels desses sistemas.

## 19.1 Novas tabs/janelas

```text
CharacterWindow
  Inventory
  Attributes
  Skills
  Quests
  Professions
  Factions/Covenants
```

Possíveis telas separadas:

```text
DialogueWindow
GatheringProgressWidget
FishingMinigameWidget
QuestTrackerHUD
FactionRankUpToast
```

---

## 19.2 ViewModels

```text
QuestJournalViewModel
ProfessionViewModel
FactionViewModel
DialogueViewModel
FishingSessionViewModel
GatheringProgressViewModel
CharacterCreationViewModel
```

Nenhum desses deve executar SQL ou calcular recompensa.

---

# 20. Roadmap de implementação

## Fase 0 — Guardrails e baseline

**Complexidade:** M  
**Objetivo:** poder refatorar sem perder comportamento silenciosamente.

### Entregas

- documentar build client/server;
- smoke test de login/criação/combate/inventário/quest;
- logging estruturado mínimo para novos services;
- IDs de jogador/character acessíveis por adapters;
- feature flags para sistemas V2.

### Critério de conclusão

É possível habilitar/desabilitar cada novo subsystem sem recompilar o legado inteiro ou quebrar save existente.

---

## Fase 1 — Gameplay Core V2

**Complexidade:** L  
**Dependência:** nenhuma.

### Implementar

```text
GameplayEventHub
ConditionService
RewardService
GameplayModifierService
ContentRegistry
repository interfaces
```

### Primeiro bridge

Publicar `MonsterKilledEvent` no ponto onde hoje são chamados Quest/BattlePass/JOBS.

Inicialmente manter chamadas antigas em paralelo.

### Critério de conclusão

Um listener de teste recebe monster kills sem alterar o resultado atual do combate.

---

## Fase 2 — PlayableClassRegistry

**Complexidade:** XL  
**Prioridade:** muito alta.

### Implementar

```text
PlayableClassDefinition
IPlayableClassModule
PlayableClassRegistry
ClassAssetRegistry
CharacterCreationCatalog
LegacyClassAdapter
LegacyJobMaskAdapter
ClassSkillResolver
CharacterModelResolver
```

### Migração inicial

Registrar as oito classes atuais através do registry sem mudar gameplay.

### Depois

Trocar criação de personagem para enumeração do registry.

### Critério de conclusão

Adicionar uma classe dummy/desabilitada não exige editar arrays centrais, e habilitá-la faz a criação enumerá-la automaticamente.

---

## Fase 3 — Quest Facade e Objective Core

**Complexidade:** XL

### Implementar

```text
QuestFacade
QuestService
QuestObjectiveRouter
QuestProgressRepository
LegacyQuestDbAdapter
SinQuestLegacyAdapter
KillObjectiveHandler
TalkObjectiveHandler
VisitObjectiveHandler
```

### Critério

Uma quest V2 e uma quest legacy aparecem no mesmo journal/API sem duplicar estado.

---

## Fase 4 — Profession V2 + import do JOBS

**Complexidade:** L

### Implementar

```text
ProfessionRegistry
ProfessionService
ProfessionRepository
LegacyJobsAdapter
LegacyJobsMigrationService
```

### Critério

O novo service consegue apresentar/importar level/XP do sistema antigo sem utilizar a UI de `JOBS` como fonte de estado.

---

## Fase 5 — Gathering Core + Mining

**Complexidade:** L

### Implementar

```text
WorldInteractionService
GatheringService
GatheringNodeRegistry
GatheringSpawnService
GatheringLootResolver
MiningInteractionHandler
```

### Remover depois da validação

Hardcode de `Hopi -> AddminJOBS`.

### Critério

Jogador minera um node real, recebe recurso/XP server-side e o node respawna.

---

## Fase 6 — Hunting

**Complexidade:** M

### Implementar

```text
CreatureTagService
HuntingService
HuntingProgressionPolicy
HuntingLootResolver
```

### Critério

Matar uma criatura com tag `Huntable` gera hunting progress sem qualquer `strcmp` pelo nome.

---

## Fase 7 — Fishing

**Complexidade:** L

### Implementar

```text
FishingZoneRegistry
FishingService
FishingSessionService
FishingLootResolver
FishingValidationService
client FishingInteractionController
```

### Critério

Sessão completa é validada pelo servidor e resiste a replay básico de packet.

---

## Fase 8 — Factions/Covenants

**Complexidade:** L

### Implementar

```text
FactionRegistry
FactionService
FactionMembershipService
FactionReputationService
FactionRankService
FactionModifierProvider
FactionQuestProvider
FactionRepository
```

### Critério

Jogador entra em uma faction, ganha reputação por evento, sobe rank, recebe modifier e desbloqueia quest.

---

## Fase 9 — Narrative/Story V2

**Complexidade:** XL

### Implementar

```text
StoryStateService
NarrativeService
DialogueService
DialogueRegistry
NarrativeActionExecutor
LegacyNpcDialogueAdapter
```

### Critério

Um NPC inicia diálogo condicionado por story flag, escolha altera flag e inicia uma quest V2.

---

## Fase 10 — Reescrita real da história

**Complexidade:** conteúdo XL

Aqui começa a migração editorial, não apenas técnica.

### Processo por arco

```text
1. inventário das quests existentes
2. definir canon/objetivo do capítulo
3. mapear NPCs
4. criar diálogos
5. migrar quests
6. definir escolhas/factions quando houver
7. validar leveling/rewards
8. remover entrada legacy equivalente
```

Não reescrever toda a história antes do framework narrativo estar provado com um capítulo piloto.

---

## Fase 11 — Desativação progressiva de legacy

**Complexidade:** XL cumulativa

Candidatos:

```text
JOBS Draw/Button/UI antiga
Hopi profession hook
quest journal antigo
quest DB monster-only assumptions
class creation arrays
class face/body switch tables
legacy clan skill calculation, se migrado para modifiers
```

Change-job quests e skills antigas complexas devem ser removidas por último.

---

# 21. Ordem sugerida de commits técnicos

Uma sequência prática para reduzir regressões:

```text
01 add gameplay/core ids and event types
02 add GameplayEventHub
03 publish MonsterKilledEvent alongside legacy flow
04 add ConditionService
05 add RewardService
06 add GameplayModifierService
07 add PlayableClassDefinition + Registry
08 register current 8 classes without behavior change
09 add ClassAssetRegistry and legacy asset adapter
10 change character creation catalog to registry enumeration
11 remove CHARACTER_JOB_MAX dependency from new creation path
12 add class validity service and replace range guard in new code
13 add QuestFacade
14 add Quest V2 persistence tables
15 add QuestObjectiveRouter + Kill objective
16 add Talk/Visit objectives
17 connect new Quest Journal ViewModel
18 add ProfessionService + repository
19 import/read legacy JOBS state
20 add WorldInteractionService
21 add GatheringService
22 add Mining node MVP
23 migrate Mining profession progression
24 remove Hopi mining hook
25 add CreatureTagService
26 add HuntingService
27 add Hunt objective
28 add FishingZone + FishingSession
29 add Fishing client interaction
30 add Fish objective
31 add FactionService
32 add Faction modifiers
33 add Faction quest conditions
34 add StoryStateService
35 add DialogueService
36 add Narrative actions
37 migrate one story chapter end-to-end
38 create first truly new playable class through registry only
```

A ordem é deliberada: a primeira classe totalmente nova entra **depois** de o registry e as integrações críticas estarem provados. Isso transforma a classe nova em teste de arquitetura.

---

# 22. Testes recomendados

## 22.1 Unit tests

Prioridade para código que não depende do renderer:

```text
ConditionService
RewardService
FactionRankService
ProfessionExperienceCurve
FishingSession state machine
QuestObjectiveRouter
QuestCompletionService
PlayableClassRegistry duplicate detection
Content validation
```

---

## 22.2 Integration tests server-side

Cenários:

```text
monster kill -> hunt XP + quest progress
mine -> item + mining XP + quest progress
fish -> item + fishing XP + quest progress
quest complete -> rewards once
faction rank -> modifier active
leave covenant -> modifier removed
reconnect -> profession/faction/quest state preserved
```

---

## 22.3 Regression tests

Manter uma matriz das oito classes antigas:

```text
login
create
load model
wear armor
learn skill
use skill
change job
save/reload
quest kill credit
party credit
```

Especialmente durante a remoção de range checks e arrays fixos.

---

## 22.4 Content validation tool

Criar uma ferramenta CLI que valide definitions antes do servidor subir.

Exemplo:

```text
validate_content.exe
```

Verificar:

- IDs duplicados;
- referência a NPC inexistente;
- quest sem objective;
- objective com target inexistente;
- reward item inexistente;
- faction rank fora de ordem;
- loot table vazia;
- class asset ausente;
- skill tree inexistente;
- story graph com node inalcançável.

Isso economiza muitos crashes/runtime bugs.

---

# 23. Observabilidade e debugging

Cada novo sistema deve emitir logs com identificadores estáveis.

Exemplo:

```text
[Quest] character=42 quest=1007 objective=3 progress=4/10
[Profession] character=42 profession=mining xp=1250 level=7
[Fishing] character=42 session=781 result=success loot=fish_salmon
[Faction] character=42 faction=hunters reputation=240 rank=2
```

Evitar logs dependentes apenas do nome do jogador.

Adicionar comandos administrativos posteriormente:

```text
/gm quest status <char> <id>
/gm quest complete <char> <id>
/gm profession set <char> mining 10
/gm faction rep <char> hunters 500
/gm story flag <char> chapter_1_done 1
/gm content reload <domain>     // somente se hot reload for implementado com segurança
```

---

# 24. Principais riscos

## 24.1 Tentar “modernizar tudo” de uma vez

Maior risco do projeto.

Mitigação:

- adapters;
- feature flags;
- um domínio por vez;
- manter legacy como fallback durante validação.

---

## 24.2 Criar uma segunda camada de hardcode

Exemplo ruim:

```cpp
if (profession == MINING)
...
else if (profession == FISHING)
...
```

espalhado pelo servidor.

Mitigação:

- definitions;
- handler registries;
- typed events.

---

## 24.3 Misturar Clan e Faction

Isso parece mais rápido, mas cria dívida estrutural.

Mitigação:

- Clan continua social/player-created;
- Faction é world/system-controlled.

---

## 24.4 Fazer novas quests em `sinQuest`

O próprio código já marca o subsystem como deprecated para desenvolvimento novo.

Mitigação:

- toda quest nova nasce em V2;
- legado apenas via adapter.

---

## 24.5 Confiar em nomes como IDs

A source atual possui muitos checks por string de monster/NPC.

Mitigação:

- IDs estáveis + tags;
- string apenas como display/internal name.

---

## 24.6 Static registration invisível

Auto-registro é conveniente, mas bugs de registro podem ser silenciosos.

Mitigação:

No boot, imprimir catálogo:

```text
Playable classes registered: 9
[1] fighter
[2] mechanician
...
[11] necromancer
```

E falhar se IDs forem duplicados.

---

## 24.7 Reward duplication

Reconnect/retry pode duplicar recompensa se o fluxo não for idempotente.

Mitigação:

- `grantId`;
- tabela de claim;
- conclusão persistida antes/de forma coordenada com grant;
- retry seguro.

---

## 24.8 Persistência por nome de personagem

QuestDB atual utiliza `PlayerName` em vários pontos.

Mitigação:

Novos sistemas devem usar `CharacterId`. Adapters convertem enquanto necessário.

---

# 25. O que reaproveitar e o que substituir

| Área | Reaproveitar | Encapsular/Migrar | Evitar expandir diretamente |
|---|---|---|---|
| Classes | `JOB_CODE`, assets/model loader, skill handlers existentes | creation, masks, model lookup, class validity | `MAX_JOBNUM`, arrays 4+4 |
| Profissões | dados existentes de `JOBS`, receitas úteis | progresso/market legado | singleton `JOBS` como core novo |
| Mining | item system, rewards, world models | legacy material state | Hopi kill hardcode |
| Hunting | monster kill hook, monster metadata | classificação/tagging | checks por monster name |
| Fishing | inventory/reward/network primitives | — | client-authoritative result |
| Quests | `cQuestHandler`, QuestDB como fonte de migração | legacy + monster-only progress | novas quests em `sinQuest` |
| Faction | ideias de clan skill + buff behavior | eventualmente adapter de buffs | usar Clan como covenant |
| História | NPC assets/talk animation/media hooks | static messages | branches em `QuestCode` switches |
| UI | UI V2 do documento anterior | views legacy | gameplay dentro de widgets |

---

# 26. Critérios de sucesso do projeto

A modernização dos sistemas pode ser considerada bem-sucedida quando:

### Classes

- quantidade jogável vem de `PlayableClassRegistry`;
- criação enumera registry;
- nova classe não exige editar arrays globais de quantidade;
- assets são resolvidos por `ClassAssetRegistry`;
- skill tree é associada por definition;
- server valida classe pelo registry, não por range numérico.

### Profissões

- Fishing/Mining/Hunting compartilham `ProfessionService`;
- estado não vive na UI;
- SQL está isolado em repository;
- `JOBS` antigo é adapter/import, não core.

### Gathering

- resource nodes têm identidade própria;
- server valida interação;
- loot e XP são server-side;
- quests podem consumir `ResourceGatheredEvent`.

### Quests

- objetivos não são limitados conceitualmente a monster kill;
- quest antiga e nova aparecem no mesmo facade/journal;
- cada objective tem progresso independente;
- rewards são idempotentes;
- novas quests não exigem código C++ quando usam objective types existentes.

### Factions

- covenant não depende de Clan;
- rank/reputation persistem;
- buffs vêm do modifier service;
- quests podem exigir rank/reputation.

### História

- diálogo suporta condição/escolha/ação;
- story flags persistem;
- chapters podem desbloquear por condições;
- um arco inteiro funciona sem novo `switch(QuestCode)`.

---

# 27. Prioridade arquitetural final

Se for necessário resumir todo este documento em uma sequência de decisões:

```text
1. GameplayEventHub
2. Condition + Reward + Modifier services
3. PlayableClassRegistry
4. QuestFacade + Objective V2
5. ProfessionService
6. WorldInteraction/Gathering
7. Mining
8. Hunting
9. Fishing
10. Faction/Covenant
11. Narrative/Dialogue
12. migração de conteúdo
13. remoção gradual do legado
```

Os sistemas não precisam esperar todos os passos para começar, mas essa ordem reduz retrabalho.

---

# 28. Arquitetura alvo resumida

```text
                         +----------------------+
                         |   ContentRegistry    |
                         +----------+-----------+
                                    |
       +----------------------------+-----------------------------+
       |                |              |             |             |
       v                v              v             v             v
 ClassRegistry      QuestRegistry  ProfessionReg  FactionReg   StoryRegistry
       |                |              |             |             |
       +----------------+--------------+-------------+-------------+
                                    |
                                    v
                         +----------------------+
                         | GameplayEventHub     |
                         +----------+-----------+
                                    |
             +----------------------+----------------------+
             |                      |                      |
             v                      v                      v
      QuestObjectiveRouter    ProfessionService       FactionService
             |                      |                      |
             |               GatheringService              |
             |              /      |       \               |
             |          Mining   Fishing   Hunting          |
             |                      |                      |
             +----------------------+----------------------+
                                    |
                          +---------v----------+
                          | ConditionService   |
                          | RewardService      |
                          | ModifierService    |
                          +---------+----------+
                                    |
                           +--------v---------+
                           | Repositories     |
                           +--------+---------+
                                    |
                               SQLite/DB
```

---

# 29. Recomendação de primeiro milestone real

O primeiro milestone de gameplay não deve ser “Fishing pronto”.

Deve ser:

> **Uma kill de monstro gera um `MonsterKilledEvent`; uma quest V2 e o HuntingService consomem o mesmo evento; os dois persistem progresso sem adicionar lógica nova ao bloco de morte legado.**

Esse milestone prova a arquitetura que será reutilizada por quase todos os sistemas pedidos.

O segundo milestone deve ser:

> **As oito classes atuais aparecem através do `PlayableClassRegistry`, sem mudança funcional visível.**

A partir daí, adicionar a nona/décima/nova classe passa a ser um teste objetivo do desacoplamento.

---

# 30. Conclusão

A source possui infraestrutura suficiente para suportar a expansão proposta. O trabalho fundamental não é criar Fishing, Mining, Hunting ou Covenants isoladamente; é introduzir **seams arquiteturais** para que esses sistemas deixem de depender diretamente de arquivos legados centrais.

Os pontos mais importantes revelados pela análise são:

1. classes novas exigem remover hardcodes em criação, models, armor, skills e autoridade server-side;
2. o sistema `JOBS` existente contém dados úteis, mas sua arquitetura não é adequada como base de profissões novas;
3. Mining atual está ligado a um hack de kill de `Hopi`, portanto precisa ser redesenhado como interação com resource nodes;
4. Hunting encaixa naturalmente no hook de monster death, desde que esse hook vire evento;
5. Fishing deve ser uma state machine server-authoritative nova;
6. o QuestDB atual é uma boa ponte, mas continua monster-centric e deve evoluir para objectives genéricos;
7. `sinQuest/HaQuest` não podem ser apagados no início por fazerem parte de save/change-job e outros fluxos;
8. Covenants devem ser Factions separadas de Clan;
9. a história precisa de StoryState + Dialogue, porque mensagens estáticas de NPC não suportam narrativa ramificada;
10. o registry de classes, o event hub e os services comuns reduzem o custo de todas as features seguintes.

Com essa arquitetura, o projeto deixa de evoluir por “editar os arquivos gigantes corretos” e passa a evoluir por **registrar definitions, implementar handlers e conectar eventos**, que é a mudança estrutural necessária para sustentar novas classes, sistemas e conteúdo por muitos ciclos de desenvolvimento.
