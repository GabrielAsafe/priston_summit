Eu começaria **pelas bordas do sistema, não pelo coração dele**. Ou seja: não começaria tentando transformar `OnSever.cpp` ou `Winmain.cpp` em “código bonito”. Esses arquivos são justamente onde mais coisas diferentes se cruzam; mexer neles cedo demais vai te ensinar mais sobre crashes do que sobre arquitetura.

O próprio `OnSever.cpp` mostra o problema: no mesmo arquivo temos carregamento de mapas/NPCs/monstros na região das ~8.500 linhas, validação de personagem e records perto das ~26.700, dispatch de packets perto das ~27.000 e até tratamento de mensagens/socket Windows acima das ~31.600.    Isso é um ótimo **destino** para modularização, mas um péssimo primeiro exercício.

## Onde eu começaria

Seu primeiro projeto de housekeeping seria:

**“Tornar carregamento de assets/configuração previsível sem alterar o comportamento do jogo.”**

Isso conversa diretamente com seu objetivo de pipeline de modelos e já temos provas de que essa região da codebase é problemática:

* path errado `.png`/`.tga` que quebrou o chat;
* diferenças de maiúsculas/minúsculas;
* paths duplicados;
* `Effect/Sky` e `StartImage/sky` coexistindo;
* arquivos CP949;
* `PetMessage.txt` derrubando o cliente;
* parsers antigos compartilhados por vários sistemas.

`GetWord`, por exemplo, não é uma função isolada: ela é usada pelo reader 3D, effects, help, pet e vários outros loaders.  Isso é exatamente o tipo de dependência que você quer descobrir **antes** de começar a mover funções.

### Fase 1 — arqueologia, sem refatorar

Durante alguns dias, eu faria uma regra para você mesmo:

> Nenhuma melhoria arquitetural pode mudar comportamento.

Comece criando um `docs/architecture/` no repositório e faça você mesmo documentos bem simples:

```text
docs/
  architecture/
    client-overview.md
    server-overview.md
    asset-loading.md
    networking.md
    game-loop.md
    file-formats.md
```

Não precisa escrever um livro. Para cada sistema:

```text
Responsabilidade:
Entrada:
Saída:
Estado global usado:
Arquivos lidos:
Quem chama:
Quem ele chama:
Cliente/Servidor:
Coisas estranhas:
```

Por exemplo:

```text
HoSky

Responsabilidade:
- céu dinâmico

Entrada:
- Effect/Sky/test32.txt
- BackImageCode

Chamado por:
- CreateSky
- MainSky
- DrawSky
- ChangeSky

Estado:
- SkyBackGround
- DrawIndex

Problemas conhecidos:
- ausência de test32 => céu preto
- pacote completo donor => crash
```

Isso parece trabalho simples, mas vai te ensinar a codebase muito mais rápido do que sair movendo funções.

---

# Seu primeiro refactor de verdade: Asset I/O

Eu criaria uma pequena camada nova, sem substituir tudo imediatamente:

```text
SrcGame/src/Core/
    AssetPath.h
    AssetPath.cpp
    FileSystem.h
    FileSystem.cpp
```

Não tente fazer um “AssetManager moderno” ainda.

Comece ridiculamente pequeno.

Por exemplo, conceitualmente:

```cpp
namespace Assets
{
    bool Exists(const char* path);
    std::string NormalizePath(const char* path);
    FILE* Open(const char* path, const char* mode);
}
```

Depois aprenda fazendo.

Hoje temos coisas como:

```cpp
CreateTextureMaterial(
    "image//Sinimage//Inter//Chat//ChatDefault-1.tga",
    ...
);
```

Seu primeiro passo **não** seria alterar `CreateTextureMaterial`.

Seria fazer algo independente:

```cpp
auto path = Assets::NormalizePath(
    "image//Sinimage//Inter//Chat//ChatDefault-1.tga"
);
```

Testar.

Entender.

Só depois começar a fazer chamadas reais passarem por isso.

### Por que eu escolheria isso primeiro?

Porque é uma área onde um erro geralmente produz:

* asset ausente;
* log;
* textura branca;
* modelo que não aparece;

e não:

* banco corrompido;
* personagem deletado;
* pacote inválido;
* server crashando durante login.

E é diretamente útil para sua futura pipeline de modelos.

---

# Segundo projeto: parsing e encoding

Esse eu considero fundamental.

A codebase ainda tem parsers antigos compartilhados entre sistemas. `GetWord()` aparece espalhado pelo reader 3D e por subsistemas de UI/effects/pet. 

Você já viu empiricamente o perigo disso com `PetMessage.txt`.

E o lado servidor também mistura parsing de arquivos com estruturas de domínio; por exemplo, `smCharDecode` preenche diretamente `smCHAR_INFO`, `smCHAR_MONSTER_INFO` e até os `FallItems`. 

Eu criaria algo como:

```text
Core/Text/
    TextReader.h
    TextReader.cpp
    Encoding.h
    Encoding.cpp
```

Mas novamente: **não substitua `GetWord()` de uma vez**.

Faça primeiro uma implementação nova:

```cpp
class TextReader
{
public:
    bool ReadLine(std::string& line);
    bool ReadToken(std::string& token);
    bool ReadQuotedString(std::string& value);
};
```

E escreva um programa pequeno fora do jogo:

```text
tools/parser_test/
```

que leia:

```text
PetMessage.txt
fore-1.ase.spm
1_Hopi.inf
algum INI CP949
```

e imprima os tokens.

Aí você está aprendendo ao mesmo tempo:

* parsing;
* encoding;
* buffers;
* ownership;
* file I/O;
* strings;
* testes.

E sem precisar iniciar o jogo.

Isso é um ótimo exercício de C++.

---

# Depois eu faria uma coisa muito importante: characterization tests

Você provavelmente não vai conseguir começar com unit tests bonitinhos porque o código não foi escrito para isso.

Então faça **testes de comportamento existente**.

Exemplo:

```text
tests/data/
  monster/
    hopi.inf
  field/
    fore-1.ase.spm
  pet/
    PetMessage.txt
```

Teste:

```cpp
TEST(MonsterParser, Hopi)
{
    MonsterData m = LoadMonster("hopi.inf");

    EXPECT_EQ(m.level, 1);
    EXPECT_EQ(m.name, "Hopi");
}
```

Mesmo se a arquitetura antiga for horrível.

O objetivo inicial não é provar que o código está “correto”.

É provar que:

> antes do refactor produzia X; depois continua produzindo X.

Essa proteção vai permitir você atacar os monstros de 20–30 mil linhas.

---

# Um ótimo arquivo para estudar antes de criar sua própria arquitetura

Olhe para `cSettingsWindow`.

Não porque ele esteja perfeito — claramente não está — mas porque já apresenta alguma separação:

```text
BuildWindow1()
BuildWindowVideo()
BuildWindowAudio()
BuildWindowGeneral()

LoadSettings()
ResetSettings()
SaveSettings()
```

e encapsula `pWindow1` como estado da classe. 

Compare isso mentalmente com `Winmain.cpp`, que ainda possui grandes quantidades de estado global como câmera, flags, texturas e interface no próprio arquivo. 

Essa comparação é muito educativa:

**ruim:**

```cpp
global
global
global

if (...)
{
   ...
}
```

**melhor:**

```cpp
class SettingsWindow
{
    BuildVideo();
    LoadSettings();
    SaveSettings();
};
```

Não significa que você precise transformar tudo em classes.

A lição é **ownership e responsabilidade**.

---

# Outro bom sistema para você estudar: o pet

O pet também já tem um lifecycle relativamente compreensível:

```text
Init
Load
Release
Draw
Main
Close
ShowPet
ClosePet
CheckPetData
```



E `Main()` contém lógica claramente pertencente ao pet: tempo de loading, mensagens, estado `PetKind/PetShow`. 

Esse é exatamente o tipo de desenho que seus sistemas novos deveriam perseguir.

Imagine:

```cpp
class FishingSystem
{
public:
    void Init();
    void Update();
    void HandlePacket(...);

    bool CanFish(...);
    void StartFishing(...);
    void CancelFishing(...);

private:
    FishingState state;
};
```

em vez de adicionar:

```cpp
if(FishingFlag)
```

em quinze lugares de `playmain.cpp`.

---

# Só depois eu atacaria OnSever.cpp

Quando você estiver mais confortável, aí sim.

Mas **não divida por número de linhas**:

```text
OnSever1.cpp
OnSever2.cpp
OnSever3.cpp
```

Isso não melhora arquitetura.

Divida por responsabilidade.

O primeiro candidato óbvio é o packet dispatch.

Hoje já vemos coisas como:

```cpp
case OpCode::OPCODE_INSRECORDDATA:
```

dentro do arquivo gigante, enquanto existe separadamente um `PacketFilter.cpp`. 

O destino poderia ser:

```text
Server/
  Network/
    PacketDispatcher.cpp
    PacketFilter.cpp

    handlers/
      CharacterPacketHandler.cpp
      InventoryPacketHandler.cpp
      CombatPacketHandler.cpp
      ChatPacketHandler.cpp
      ClanPacketHandler.cpp
```

Comece extraindo **um único case**.

Antes:

```cpp
switch(code)
{
    case OPCODE_FOO:
        // 70 linhas
        break;
}
```

Depois:

```cpp
case OPCODE_FOO:
    HandleFooPacket(context);
    break;
```

Compile.

Teste.

Commit.

Então mova `HandleFooPacket()` para outro `.cpp`.

Compile.

Teste.

Commit.

Esse ritmo importa.

---

# Não faça “comment everything”

Isso vai gerar milhares de comentários inúteis.

Evite:

```cpp
// Incrementa counter
counter++;
```

Comente principalmente quatro coisas.

**1. Por quê:**

```cpp
// Character deletion still performs the legacy clan check before
// sending OPCODE_DELRECORDDATA. Some servers no longer provide that service.
```

Isso seria especialmente útil aqui, porque atualmente o fluxo de delete passa por `isDELETE_character()` antes de sequer mandar o opcode ao DataServer. 

**2. Invariantes:**

```cpp
// Must remain <= MAX_MONSTER_DROP_ITEMS.
```

**3. Ownership:**

```cpp
// Owned by CharacterManager. Do not delete here.
```

**4. Formato/protocolo:**

```cpp
// PetMessage.txt is legacy CP949 text.
// Quoted strings may contain spaces.
```

Esses comentários sobrevivem ao tempo.

---

# Seu roadmap de arquitetura

Eu faria nessa ordem:

| Etapa | Trabalho                       | O que você aprende            |
| ----- | ------------------------------ | ----------------------------- |
| 1     | mapa/documentação da codebase  | leitura de sistemas           |
| 2     | build reproduzível + Git limpo | engenharia de software        |
| 3     | AssetPath/FileSystem           | APIs, filesystem, strings     |
| 4     | TextReader/Encoding            | parsing, memória, encoding    |
| 5     | characterization tests         | testing                       |
| 6     | UI pequena / Pet               | classes, lifecycle, ownership |
| 7     | packet handlers                | networking e arquitetura      |
| 8     | dividir `OnSever` por domínio  | modularização real            |
| 9     | separar game loop/render       | engine architecture           |
| 10    | novos gameplay systems         | design de sistemas            |

E só então eu começaria projetos grandes como mineração/fishing/hunting.

---

## Como eu desenharia os sistemas novos

Uma das coisas que eu evitaria é criar:

```text
MiningSystem
FishingSystem
HuntingSystem
```

com três implementações completamente independentes.

Eles compartilham um conceito maior:

**Gathering.**

Então provavelmente acabaria existindo:

```text
Game/
  Gathering/
    GatheringSystem
    GatheringNode
    GatheringSkill
    GatheringLootTable

    Mining/
    Fishing/
    Hunting/
```

Por exemplo:

```cpp
enum class GatheringType
{
    Mining,
    Fishing,
    Hunting
};

struct GatheringNode
{
    GatheringType type;
    int requiredLevel;
    int lootTable;
    int respawnTime;
};
```

Então uma quest pode dizer:

```text
Gather 5 Iron Ore
```

sem saber como o sistema de mining foi implementado.

Isso te leva ao seu objetivo maior: sistemas que **conversam entre si** em vez de features isoladas.

Mining gera minério.

Crafting usa minério.

Quest pede minério.

Economia compra minério.

World spawning controla veios.

Skill progression melhora mineração.

É aí que o jogo começa a parecer moderno.

---

# E sua pipeline de modelos deve influenciar a arquitetura

Esse é um dos motivos pelos quais eu começaria por assets.

Seu objetivo não deveria terminar em:

```text
ASE -> Blender -> ASE
```

Eu criaria gradualmente um formato intermediário seu:

```text
Game Model
    ├─ skeleton
    ├─ meshes
    ├─ materials
    ├─ textures
    ├─ animations
    └─ metadata
```

Então:

```text
SMD/ASE/SMB
     ↓
 Importer
     ↓
GameModel
     ↓
 Blender/export
     ↓
GameModel
     ↓
 Exporter
     ↓
SMD/ASE/SMB
```

Isso é muito mais poderoso.

Quando compreender os formatos, você poderá eventualmente fazer:

```text
GLTF/FBX
  ↓
Summit Asset Compiler
  ↓
formato original do jogo
```

E mais tarde talvez nem precise manter o renderer preso aos formatos originais.

---

# O primeiro trabalho que eu faria amanhã

Eu faria **uma semana sem feature nova**.

O objetivo seria somente:

```text
1. criar docs/architecture
2. mapear os diretórios SrcGame e SrcServer
3. descobrir os 20 maiores .cpp
4. escrever a responsabilidade presumida de cada um
5. escolher 1 sistema pequeno
6. desenhar o call graph dele manualmente
7. extrair 1 função sem alterar comportamento
8. compilar
9. executar smoke test
10. commit
```

E eu escolheria **asset loading** como o primeiro sistema pequeno.

Não escolheria networking.

Não escolheria character records.

Não escolheria inventory.

Não escolheria combate.

Essas áreas têm acoplamentos escondidos. O caso do delete é um exemplo perfeito: uma ação aparentemente simples de login depende de um check legado de clan antes do envio da operação real. 

## Regra que vale ouro nesse projeto

Faça commits pequenos como:

```text
refactor: extract asset path normalization

refactor: move texture path constants

docs: document HoSky lifecycle

refactor: extract monster file reader

test: add Hopi monster parser fixture
```

e evite:

```text
refactor entire server architecture
```

Se um commit alterar 3.000 linhas e o jogo quebrar, você não aprendeu onde errou.

Se alterar 40 linhas, quebrar, e você descobrir por quê, **isso é justamente o aprendizado que você está procurando**.

E eu acho que o melhor primeiro exercício concreto seria pegarmos **um subsystem pequeno do seu source e você mesmo fazer o primeiro refactor**, comigo apenas revisando suas decisões. `AssetPath/FileSystem`, `cHELPPET` ou `cSettingsWindow` seriam bons candidatos; eu começaria pelo **AssetPath/FileSystem**, porque prepara diretamente o terreno para sua pipeline gráfica.

