# Relatório técnico do port Summit Tale / Priston Tale

**Estado do projeto, correções de compatibilidade, DLLs, pipeline de assets, encoding e plano de evolução gráfica**  
**Snapshot técnico: 22 de agosto de 2026**

> Este documento registra o estado conhecido do projeto antes da correção definitiva de `DecodeOpenMonster()` para os tokens CP949 dos arquivos `.spm`. Ele separa fatos comprovados de hipóteses e evita assumir compatibilidade entre revisões de assets.

## 1. Resumo executivo

O port MinGW i686 chegou ao ponto em que o cliente autentica, cria/seleciona personagem, entra no mapa, exibe o próprio personagem, permite movimento, permanece conectado e já exibe NPCs fixos. O principal bloqueio funcional restante no mundo é o spawn de monstros. O diagnóstico atual é forte: os mapas e pontos de spawn `.spp` carregam, os 374 monstros globais são carregados, mas `DecodeOpenMonster()` não reconhece os comandos coreanos CP949 dos `.spm`, por isso `rsMonsterList.Counter` e `PecetageCount` permanecem zero.

Em paralelo, restauramos a maior parte da árvore `image/` do cliente de forma incremental e estável. O conjunto `PetData` permanece isolado porque sua presença ativa o sistema LowLevelPet depois de aproximadamente três segundos e leva a crash. O pet em questão é o dragão iniciante automático (Terry/Napsys/EOR/Mut, associado aos quatro dragões elementais), que deve funcionar até o nível 10; ele não deve ser removido definitivamente.

Há ainda um problema transversal de idioma/encoding. Os `.spm` são CP949 e usam comandos coreanos legados; partes do source mostram mojibake e alguns textos portugueses já contêm `?` literal no arquivo, o que indica perda irreversível de caracteres em certos pontos. O tratamento correto exige inventário por arquivo e por origem, em vez de converter tudo para UTF-8 de uma vez.

## 2. Ambiente e fluxo de trabalho

| Papel | Caminho / comando |
|---|---|
| Source principal | `/home/gabriel/Área de Trabalho/summit/SourceCode` |
| Cliente runtime | `/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente` |
| Servidor staging no Fedora | `/home/gabriel/Área de Trabalho/summit/Servidor` |
| Servidor Windows live | `C:\SummitTale\Servidor` |
| Arquivo de donors | `/home/gabriel/armazem/priston` |
| Build cliente | `sh cliente.sh` |
| Build servidor | `sh build_server_mingw_i686_j8.sh` |
| Link servidor | `sh link_server_mingw_i686.sh` |
| Deploy servidor | `sh send_to_s.sh` |
| Runtime Windows | usar a shell Windows aberta diretamente; evitar SSH quando não é necessário |

Regra operacional consolidada: Linux para source, patch e build; shell Windows para inspeção e operação do runtime live. O cliente só deve ser rebuildado quando a alteração for realmente de código cliente, porque o build é caro. O servidor pode ser rebuildado com mais frequência, mas com patches pequenos e rastreáveis.

## 3. Classes de problemas encontradas

### 3.1 Portabilidade de código legado 32-bit

O código foi escrito com pressupostos de compilador, ABI, signedness e comportamento de Windows/MSVC antigos. No port MinGW, esses pressupostos apareceram como comparações incorretas de ponteiros, literais multicaractere usados como se fossem `char`, diferenças de tratamento de path e acessos nulos que no ambiente original podiam ficar mascarados.

**Padrão de solução:** localizar o ponto exato por trace/disassembly, preservar o comportamento original válido e corrigir somente o pressuposto incompatível. Evitamos reescrever subsistemas inteiros.

### 3.2 Paths e separadores `/` vs `\\`

Esta foi uma classe recorrente e particularmente importante. Várias rotinas tratavam `'//'` como se fosse um caractere separador. Em C/C++, `'//'` é um literal multicaractere inteiro, não um `char '/'`. Em outros pontos o código exigia exclusivamente `/`, enquanto o runtime Windows produzia `\\`.

**Padrão de solução:** aceitar explicitamente `/` e `\\`, ou canonicalizar o path antes de checksum/comparação.

### 3.3 Filtros de opcode e protocolo legado

O servidor novo/portado descartava pacotes válidos do cliente legado porque os filtros assumiam famílias de opcode diferentes. Isso bloqueou login e criação de personagem mesmo quando a camada de socket estava funcionando.

**Padrão de solução:** identificar o opcode real no wire e ampliar somente a família necessária, sem desligar o filtro inteiro.

### 3.4 Espaço de endereço 32-bit

O servidor i686 precisava ser linkado como Large Address Aware. Sem `--large-address-aware`, grandes alocações e o layout do processo aumentavam a chance de falhas em cenários reais.

### 3.5 UI/input e estado de janela

O port apresentou divergências entre resolução real, estado de `SettingsHandler` e coordenadas do mouse. A criação de personagem também dependia corretamente do evento de release. O estado interno de window mode precisava refletir a janela usada de fato.

### 3.6 Loader de modelos e assets incompletos

O cliente original referencia muitos arquivos por nome lógico `.ASE`, mas o pipeline de runtime também usa formatos compilados próprios (`.smd`/`.smb`). Paths, checksums, arquivos ausentes e ponteiros nulos no loader causaram crashes durante login/seleção e no sistema de pets.

### 3.7 Dados do mundo e revisões incompatíveis

`Field`, `Monster`, `NPC` e assets de cliente pertencem a uma revisão específica do jogo. O runtime atual não é cópia direta do donor ExMachina nem do donor `pristontale-1-master`. Copiar árvores inteiras mistura revisões e pode criar crashes difíceis de atribuir.

### 3.8 Encoding/code pages

O projeto mistura fontes e dados antigos de Coreia/Brasil. Há CP949 coreano, strings portuguesas em encodings de 8 bits, arquivos que hoje aparecem com mojibake e alguns caracteres já substituídos por `?`. A engine é majoritariamente `char*` e APIs Win32 ANSI; portanto “converter tudo para UTF-8” sem adaptar a engine não é uma solução segura.

## 4. Registro das correções já realizadas

| Área | Problema | Correção / marcador | Estado |
|---|---|---|---|
| Login | pacote legado rejeitado pelo filtro | permitir opcode de login legado `0x48470085` | resolvido |
| Auth | retorno SQL legado `-5` não tratado como sucesso | normalização do retorno | resolvido |
| Font/DirectX | crash na inicialização de fontes | ajuste de inicialização | resolvido |
| HoLogin/Clan | crash de inicialização | correções/guards de init | resolvido |
| Model checksum | `/` e `\\` produziam comportamento diferente | `MINGW_MODEL_PATH_CHECKSUM_V2` | resolvido |
| AddJobCharacter | off-by-one / carga inválida | correção + `MINGW_ADDJOB_LOAD_FAIL_GUARD_V2` | resolvido |
| Motion | ponteiro nulo em troca de animação | `MINGW_CHANGEMOTION_NULL_GUARD_V1` | proteção ativa |
| Mouse/window | coordenadas/estado divergentes | `SETTINGSHANDLER->setWindowMode(1)` / `MINGW_WINDOWMODE_STATE_FIX_V1` | resolvido |
| Criação | opcode `0x7000006A` filtrado | `MINGW_GAME_OPCODE_FAMILY_FILTER_V2` | resolvido |
| Server linker | limite de endereço i686 | `--large-address-aware` | obrigatório |
| CheckUserModel | separador `'//'` inválido | `MINGW_CHECKUSERMODEL_SEPARATOR_FIX_V1` | resolvido |
| PROCESSINFO | disconnect por path sem `/` | aceitar/canonicalizar separadores; `MINGW_PROCESSINFO_PATH_SEPARATOR_FIX_V1` | resolvido |
| World sidecars | `SetFieldInfoPath()` procurava `'//'` | aceitar `/` e `\\`; `MINGW_SETFIELDINFOPATH_SEPARATOR_FIX_V1` | resolvido; NPCs apareceram |
| World trace | ausência de mobs sem causa clara | `MINGW_STAGE_SPAWN_LOAD_TRACE_V1` | diagnóstico ativo |

### Resultado funcional atual

Login, seleção/criação, entrada no campo, personagem local, movimento, conexão e NPCs estão funcionais. Monstros continuam ausentes. O cliente permanece estável com os grupos de assets normais restaurados até `Bellatra` e `GuideMap`.

## 5. Patches binários em DLLs legadas

### 5.1 `Database.dll`

A DLL é um PE i386 legado e permanece binária; portanto as correções foram feitas diretamente em bytes após análise do fluxo de execução.

**Patch de comparação/ponteiro**

- RVA: `0x347F0`
- bytes antigos: `3B CB 0F 8E 80 00 00 00`
- bytes novos: `3B C2 0F 86 80 00 00 00`
- SHA-256 conhecido da DLL patched: `c9fd54668cba6e00fa760bda541587635f51c962ab6ef2b19179d0651e7220a7`

O código antigo fazia uma comparação signed sobre um valor derivado de diferença de ponteiros e podia seguir para um copy usando um endereço inválido. O patch muda a comparação para a relação de ponteiros apropriada/unsigned no caminho analisado, impedindo o walk inválido.

**SQL**

Também foi ajustado o alvo/instância SQL usado pela DLL para o ambiente atual. O endpoint live ficou acessível do Windows em `10.0.2.2,1433`. O offset exato da alteração de string/endpoint não foi reconstituído neste snapshot; para reprodutibilidade futura, deve-se guardar o binário original, o binário patched e gerar um binary diff antes de qualquer nova alteração.

### 5.2 `clan.dll`

**Patch de comparação/ponteiro**

- RVA: `0xA248`
- bytes antigos: `85 C9 7E 7C`
- bytes novos: `39 D8 76 7C`
- SHA-256 conhecido da DLL patched: `082b9968dcb9eec4478532f516b429e41d1999fcbc7f35ca0d23cfdab0f3b2dd`

A falha observada ficava perto de RVA `0xA254`. O fluxo antigo testava uma diferença de ponteiros como signed antes de copiar a partir de `eax`; o patch compara diretamente os ponteiros com condição unsigned, evitando avançar quando o endereço está fora da ordem esperada.

**SQL**

O endereço/alvo SQL embutido na DLL também foi adaptado ao ambiente atual. Assim como em `Database.dll`, o relatório registra a existência da mudança, mas recomenda recuperar a localização exata por diff binário dos backups se for necessário reproduzir o patch do zero.

### 5.3 Política para DLLs binariamente patched

Nunca sobrescrever a única cópia. Manter pelo menos: `original`, `patched conhecido`, SHA-256, offsets e bytes alterados. Qualquer futura alteração deve partir de um hash conhecido; se o hash divergir, não aplicar patch por offset cegamente.

## 6. Pipeline do mundo no servidor

O carregamento de um campo passa por `STG_AREA::LoadStage()`:

```text
stage .ASE / compilado
    |
    +-- .spp -> LoadStartPoint()       -> pontos de nascimento/spawn
    |
    +-- .spc -> LoadCharInfoFixed()    -> TransCharFixed[] -> OpenNpc()
    |
    +-- .spm -> DecodeOpenMonster()    -> rsMonsterList -> STG_AREA::Main() -> OpenMonster()
```

O patch de `SetFieldInfoPath()` foi decisivo: depois de corrigir o separador, os NPCs começaram a aparecer. Isso comprova que o `.spc` e o caminho base do campo estão funcionando.

### 6.1 Estado comprovado dos mobs

O trace de startup mostrou, para dezenas de mapas:

- `chrMonsterCnt=374`: a lista global de monstros foi carregada.
- muitos `starts>0`: os `.spp` estão sendo lidos; exemplos observados vão de dezenas até 200 pontos.
- `counter=0`, `pct=0`, `limit=0` em todos os mapas: o `.spm` abre, mas sua semântica não é reconhecida.
- `openLimit=3` e `interval=127`: são exatamente os defaults definidos antes do parse, evidência de que o parser não aplicou as diretivas do arquivo.

Logo, neste momento **não** faz sentido investigar `StartPointNearPlay`, transmissão de entidades ou renderização cliente para explicar a ausência total de mobs. A falha está antes, em `DecodeOpenMonster()`.

## 7. Descoberta de encoding nos `.spm`

Os `.spm` do runtime são texto legado em CP949. O inventário real encontrou estes tokens:

| Token CP949 | Ocorrências | Alias que o source já entende | Função |
|---|---:|---|---|
| `*출연자` | 238 | `*ACTOR` | monstro + peso/frequência |
| `*최대동시출현수` | 51 | `*MAX_ACTOR_POS` | limite máximo simultâneo |
| `*출현수` | 51 | `*MAX_ACTOR` | limite de abertura por ponto/ciclo |
| `*출현간격` | 51 | `*DELAY` | intervalo de spawn |
| `*출연자두목` | 16 | `*BOSS_ACTOR` | boss + slaves/horários |

Há apenas uma ocorrência de cada alias inglês em toda a árvore analisada, provavelmente arquivo de teste/revisão. O conteúdo real usa coreano.

Exemplo decodificado de `fore-1.ase.spm`:

```text
//몬스터 출현 비율

*최대동시출현수  300
*출현간격        1 2
*출현수          2

*출연자 "Hopi"              35
*출연자 "Cuepi"             12
*출연자 "Tatu"              20
*출연자 "Cogumelo Fantasma" 25
*출연자 "Duende Macabro"     8
```

O source de `DecodeOpenMonster()` tem comparações contra literais coreanos, mas no source atual eles aparecem corrompidos/mojibake. Os aliases ingleses estão intactos. O próximo patch recomendado é reconhecer explicitamente os bytes CP949 reais — preservando também os aliases ingleses — em vez de converter todos os `.spm` ou trocar a árvore por um donor.

## 8. Problema de idioma e caracteres corrompidos

### 8.1 Há pelo menos três fenômenos diferentes

**A. Texto em CP949 exibido/interpretado como outra code page.**  
É reversível enquanto os bytes originais ainda existem. Os comandos coreanos dos `.spm` são um exemplo: quando decodificados como CP949, aparecem corretamente.

**B. Mojibake em português.**  
Sequências como `Ol<U+FFFD>`, `Est<U+FFFD>`, `n<U+FFFD>o`, `Ã`, `Â` ou outras podem resultar de um arquivo CP1252/Latin-1 lido como UTF-8, ou de uma conversão anterior errada. Parte disso pode ser recuperável se os bytes originais não foram substituídos.

**C. Perda real de dados.**  
No próprio `fore-1.ase.spm` existem comentários com `Coelh? de P?coa`. Aqui o byte armazenado já é `?` em vez de `o`/`á` etc. Trocar a code page não recupera o caractere original; é necessário restaurar de uma fonte correta ou editar manualmente.

### 8.2 Por que os nomes dos NPCs podem aparecer corrompidos

O servidor e cliente antigos trabalham majoritariamente com `char*` e APIs ANSI. Se um `.npc`, `.inf`, `.spc`, header de idioma ou string de banco contém CP1252/CP949, mas a máquina/compilação interpreta em outra ACP, os bytes chegam ao cliente sem conversão coerente. Assim, o problema pode estar em qualquer elo:

```text
arquivo/banco -> parser servidor -> packet char[] -> cliente -> API de texto/font
```

Antes de escolher uma política global, é necessário registrar para cada origem: encoding real em disco, bytes enviados e encoding assumido na renderização.

### 8.3 Estratégia recomendada de modernização de texto

Curto prazo: preservar os encodings legados por formato e corrigir parsers específicos. Para os `.spm`, CP949 deve ser tratado deliberadamente. Para textos PT-BR, identificar CP1252/Latin-1 vs UTF-8 arquivo por arquivo.

Médio prazo: criar uma camada explícita de conversão na fronteira de I/O, mantendo structs/protocolo legados enquanto necessário. Por exemplo, ler arquivo CP949/CP1252 -> converter internamente para UTF-8 ou UTF-16 em subsistemas novos -> converter para a encoding esperada no packet/Win32 ANSI apenas na borda.

Longo prazo: somente depois de mapear protocolo, banco e renderização, avaliar Unicode end-to-end. Uma conversão massiva imediata de fontes/assets para UTF-8 quebraria comparações byte-a-byte como as vistas em `DecodeOpenMonster()`.

## 9. Relatório de assets do cliente

### 9.1 Estratégia usada

O `image/` original do runtime estava incompleto. O donor `pristontale-1-master/bin/Client/image` correspondeu muito melhor aos paths literais do source que o ExMachina. Porém copiar a árvore toda gerou crash, então passamos a restaurar por grupos pequenos, testando o cliente após cada grupo.

### 9.2 Grupos de `image/` restaurados e testados sem crash

- `Sinimage/help`
- `clanImage`
- `party`
- `Sinimage/Items`
- `Sinimage/MessageBox`
- `Sinimage/shopall`
- `Sinimage/skill`
- `Sinimage/Event`
- `Sinimage/indishop`
- `Sinimage/Inter`
- `Sinimage/Inven`
- `Sinimage/Cursor`
- `Sinimage/Player`
- `Sinimage/Quest`
- `Sinimage/Status`
- `Sinimage/WarpGateDoc`
- `Sinimage/Premium`
- `Sinimage/Siege`
- `Sinimage/Sound`
- `Bellatra`
- `GuideMap`

O jogo permaneceu estável após os últimos lotes. Isso cria uma baseline visual muito mais completa para continuar modernização.

### 9.3 Grupos ainda isolados

- `image/Sinimage/PetData` — ativa o LowLevelPet e leva a crash atrasado.
- `image/Sinimage/Effect` — ainda deve ser validado isoladamente.
- `image/Sinimage/AssaEffect` — ainda deve ser validado isoladamente.

Não confundir essas pastas com o **root** `Effect/`, que contém modelos, animações e texturas de efeitos 3D e já existe no cliente.

## 10. Tipos de asset e o que representam

| Extensão / grupo | Papel observado | Editabilidade / ferramenta recomendada |
|---|---|---|
| `.TGA` | textura/UI, frequentemente com alpha; muito usada por personagens, efeitos e interface | GIMP, Krita, Photoshop; preservar dimensões, alpha e nome |
| `.BMP` | texturas e imagens antigas, às vezes sem alpha | GIMP/Krita/Photoshop; preservar formato esperado |
| `.DDS` | textura comprimida/mais moderna em algumas revisões, muito comum no ExMachina | GIMP com DDS, Paint.NET, Photoshop plugin, texconv; não substituir TGA por DDS sem confirmar loader/path |
| `.ASE` | formato de autoria ASCII de cena/modelo; o source chama `smASE_Read`/`smASE_ReadBone` com nomes `.ASE` | 3ds Max historicamente exporta ASE; Blender pode exigir add-on/conversão e validação |
| `.SMD` | **formato proprietário desta engine**, header `SMD Model data Ver 0.62`; não é Valve SMD | usar pipeline/loader/editor do projeto; ferramentas genéricas de Valve SMD não são adequadas |
| `.SMB` | companion proprietário usado para bone/animação no pipeline `smASE_ReadBone` | requer pipeline/editor específico ou converter via código legado |
| `.INX` | arquivo associado a muitos modelos/monstros, provavelmente índice/metadados de animação | formato ainda não auditado; não editar cegamente |
| `.INF` em `GameServer/Monster` | definição de monstro lida por `smCharDecode` | editor de monstros pode ser relevante; validar em cópia |
| `.NPC` em `GameServer/NPC` | definição de NPC | provável edição via ferramentas legadas ou texto/binário conforme arquivo; validar formato |
| `.SPP` | pontos de spawn/start do mapa | carregado por `LoadStartPoint`; provável Level Editor |
| `.SPC` | NPCs fixos/placement | carregado por `LoadCharInfoFixed`; abre NPCs no stage |
| `.SPM` | composição e regras de spawn de monstros | texto CP949; pode ser editado em editor de texto que preserve CP949 |

### 10.1 O significado do `.ASE` no runtime

Não assumir que a presença de chamada `smASE_Read("x.ASE")` exige um arquivo ASE em runtime. O loader possui `ChangeFileExt()` e o projeto contém formatos compilados próprios. Há headers explícitos `SMD Model data Ver 0.62` e `SMD Stage data Ver 0.72`. Portanto o nome `.ASE` é parte do pipeline lógico/autoria; o runtime pode procurar/usar a versão compilada correspondente dependendo da função e configuração.

### 10.2 Estrutura típica de um monstro gráfico

Pelos diretórios de monstros observados, um monstro costuma reunir:

```text
char/monster/<monstro>/
    modelo .smd
    bone/animação .smb
    metadata/animação .inx
    texturas .tga/.bmp
```

No servidor, a identidade e stats são ligados por `GameServer/Monster/*.inf`, e a presença em cada mapa é definida pelo `.spm`; os pontos físicos de spawn vêm do `.spp`.

## 11. LowLevelPet / dragão iniciante

O sistema esperado é original: para personagem abaixo do nível 10, um de quatro pets é escolhido e mostrado automaticamente.

`PetMessage.txt` identifica:

| Elemento | Linhagem | Nome curto |
|---|---|---|
| Fogo | ASTERYAN | Terry |
| Água | KRONAPSYS | Napsys |
| Vento | RINDBEOR | EOR |
| Terra | BAHAMUT | Mut |

`CheckPetData()` espera alguns ciclos, lê `PetMessage.txt`, escolhe `PetKind` 1..4, prepara mensagens e chama `ShowPet()`. Assim, retirar `PetData` evita o crash porque impede a ativação; isso não significa que o parser do TXT seja necessariamente o culpado.

O manager 3D carrega:

```text
Effect/LowLevelPet/Pet_motion.ASE -> bone/animação (runtime correspondente .smb)
Effect/LowLevelPet/Pet_terry.ASE  -> mesh (.smd)
Effect/LowLevelPet/Pet_nepsys.ASE -> mesh (.smd)
Effect/LowLevelPet/Pet_io.ASE     -> mesh (.smd)
Effect/LowLevelPet/Pet_mute.ASE   -> mesh (.smd)
```

O runtime contém `Pet_motion.smb`, quatro `.smd` e quatro texturas TGA. Comparação com o donor PT mostrou que `Pet_io.smd`, `Pet_nepsys.smd`, `Pet_terry.smd` e `Pet_motion.smb` coincidem; `Pet_mute.smd` e as texturas mostraram diferenças em testes anteriores. O código de `Create()` não mostrou validação robusta de `m_Bone`/meshes antes do uso.

**Próxima abordagem correta:** instrumentar `Create -> AddPet -> SetSpawn -> Draw`, registrar qual recurso é nulo/incompatível e adicionar guard apenas para evitar crash. Depois corrigir o asset/pipeline real; não desativar o pet permanentemente.

## 12. Efeitos e melhoria gráfica

Há duas famílias importantes:

**`image/Sinimage/Effect` / `image/Sinimage/AssaEffect`**: recursos ligados ao sistema `sinbaram`, com texturas e meshes de skills/efeitos. O source referencia paths como `image//Sinimage//Effect//SkillEffect/...` e carrega modelos via `smASE_Read`.

**`Effect/` na raiz do cliente**: árvore extensa de efeitos, incluindo `AnimationData`, `AssaEffect`, `ImageData`, `ObjAnimationData`, `LowLevelPet`, partículas, skill meshes e recursos de monstros.

Para melhorar visualmente sem quebrar compatibilidade, começar por texturas é mais seguro do que trocar geometria/animação. Uma textura pode ser refeita em maior qualidade, mas a engine pode depender de dimensão/power-of-two, alpha e UVs antigos. O primeiro teste deve preservar resolução original; depois testar aumento de resolução por família de material.

### Workflow recomendado para renovar uma textura

1. Identificar todas as referências ao arquivo no source e nos modelos/materials.
2. Fazer backup e hash da versão original.
3. Abrir em editor que preserve alpha.
4. Refazer/upscale mantendo nome, extensão e inicialmente as mesmas dimensões.
5. Testar no contexto real (UI, monstro, skill), procurando bleed, alpha incorreto, UV e performance.
6. Só depois testar dimensões maiores ou DDS, uma categoria por vez.

### Workflow recomendado para criar um monstro novo

**Parte gráfica (comprovada em alto nível):** criar/adaptar modelo e texturas, produzir os formatos que a engine realmente carrega (`.smd`/`.smb`, possivelmente `.inx`).

**Parte servidor:** criar definição `Monster/*.inf`; adicionar o nome ao `.spm` do mapa com peso; garantir `.spp` com spawn points.

**Ainda requer confirmação:** como gerar exatamente `.smd/.smb/.inx` com as ferramentas deste pacote. Isso deve ser testado em sandbox com os executáveis legados e comparando o arquivo produzido contra assets conhecidos.

### Workflow recomendado para criar um NPC

Criar/reutilizar modelo e textura; definir o NPC em `GameServer/NPC`; adicionar placement no `.spc` do mapa. `LoadStage()` prova que o `.spc` é o elo que chama `OpenNpc()`.

## 13. Ferramentas encontradas no pacote do servidor

Foram observados no runtime Windows:

| Executável | O que o nome sugere | Confiança / ação segura |
|---|---|---|
| `@Editor de Monstros.exe` | editar dados de monstros | alta relevância; testar em cópia de `Monster/*.inf` |
| `@ItemEditor.exe` | editar itens | provável; não necessário para gráficos inicialmente |
| `@Level Editor.exe` | editar campo/level e possivelmente sidecars | alta relevância para mapas/spawns; validar quais arquivos escreve |
| `@Mudar nick do player.exe` | manutenção de personagem | administrativo |
| `@PriTaTor_1.8.857.exe` | ferramenta custom/multifunção | função não confirmada; não executar contra produção sem sandbox |
| `@Restarter.exe` | restart/watchdog | operacional |
| `@SoD Atualiza.exe` | atualização de SoD/evento | específico de evento |
| `@Tudo Sobre CLAN.exe` | administração/consulta de clan | específico de clan |

**Importante:** os nomes são evidência da existência das ferramentas, não prova do formato que cada uma escreve. Antes de usar qualquer editor antigo: copiar uma pasta mínima para sandbox, registrar hashes antes/depois, usar Process Monitor se necessário, e nunca apontar o primeiro teste ao runtime live.

## 14. Ferramentas modernas recomendadas

- Texturas/UI: GIMP, Krita ou Photoshop.
- DDS: GIMP/Paint.NET/Photoshop com suporte DDS ou `texconv` para conversão controlada.
- ASE: 3ds Max é o caminho histórico mais compatível; Blender pode participar da autoria, mas a exportação ASE e convenções de skeleton/material precisam ser validadas com um asset simples conhecido.
- Hex/binário: HxD no Windows ou `xxd`, `objdump`, Python no Linux.
- Encoding: VS Code/Notepad++ com seleção explícita de encoding; para `.spm`, abrir/salvar como Korean CP949 enquanto a engine permanecer legada.
- Diff de assets: `sha256sum`, `cmp`, `xxd`, scripts Python.

Não usar ferramentas de **Valve SMD** para estes `.smd`: o source confirma um formato proprietário chamado `SMD Model data Ver 0.62`.

## 15. Donors e política de compatibilidade

Donors principais encontrados:

```text
/home/gabriel/armazem/priston/ExMachina/server/GameServer
/home/gabriel/armazem/priston/pristontale-1-master/bin/Server/GameServer
/home/gabriel/armazem/priston/pristontale-1-master/bin/Client
```

A pasta `Field` do runtime tem composição diferente dos dois donors e poucos arquivos comuns idênticos. O runtime deve ser tratado como uma terceira revisão. Por isso:

- usar donor para recuperar **arquivo individual comprovadamente ausente**;
- comparar tamanho/hash/estrutura antes de copiar;
- não substituir `Field`, `Monster`, `NPC`, `Effect` ou `image` inteiros sem teste incremental;
- manter um manifesto de proveniência por asset restaurado.

## 16. Auditoria de caracteres corrompidos

Foram preparados dois scripts read-only junto com este relatório:

- `encoding_audit.py`: Linux; varre source, cliente e staging servidor.
- `encoding_audit_windows.ps1`: Windows; varre o `GameServer` live.

Eles procuram:

- UTF-8 inválido;
- arquivos com forte sinal de CP949 (Hangul válido);
- arquivos com sinal de CP1252;
- `U+FFFD` (caractere de substituição);
- sequências comuns de mojibake (`Ã`, `Â`, etc.);
- `?` literal no meio de palavra (`P?coa`), que é forte sinal de caractere já perdido.

### Comando Linux sugerido

```bash
cd "/home/gabriel/Área de Trabalho/summit/SourceCode"
python3 /mnt/data/encoding_audit.py \
  "/home/gabriel/Área de Trabalho/summit/SourceCode" \
  "/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente" \
  "/home/gabriel/Área de Trabalho/summit/Servidor" \
  --output "/tmp/summit_encoding_audit.tsv"
```

### Comando Windows sugerido

Copiar `encoding_audit_windows.ps1` para `C:\SummitTale\Servidor` e executar:

```bat
cd /d C:\SummitTale\Servidor
powershell -NoProfile -ExecutionPolicy Bypass -File .\encoding_audit_windows.ps1
```

Ele produz `encoding_audit_windows.tsv` sem modificar os assets.

## 17. Ordem recomendada das próximas etapas

1. Rodar a auditoria de encoding e separar CP949 legítimo de corrupção real.
2. Corrigir `DecodeOpenMonster()` para reconhecer os cinco tokens CP949 reais, mantendo aliases ingleses.
3. Rebuild/deploy apenas do servidor e validar `spawn_trace`: `counter>0`, `pct>0`, `limit>0`; confirmar mobs in-game.
4. Com mobs funcionais, auditar nomes de NPC/monstro no packet e no renderer para definir a política de encoding PT-BR.
5. Validar `image/Sinimage/Effect` e `AssaEffect` isoladamente.
6. Instrumentar e corrigir o LowLevelPet/dragão sem removê-lo.
7. Montar um pipeline formal de autoria: textura -> modelo -> formato compilado -> definição servidor -> placement/spawn.
8. Só depois iniciar modernização gráfica em escala, com manifesto de assets e testes por categoria.

## 18. Checklist de reprodutibilidade

Para cada patch futuro registrar: arquivo, hash antes, hash depois, função/offset, motivo, evidência, comando de aplicação, comando de rollback e teste de aceitação. Para cada asset restaurado registrar: donor, caminho, hash, grupo de teste e resultado (crash/estável).

Os traces atuais devem permanecer enquanto o fluxo de mundo não estiver estável. Remover instrumentação apenas depois de: mobs visíveis, NPCs corretos, pet estável, relogin/persistência validada e encoding minimamente consistente.

## 19. Estado final deste snapshot

**Resolvido:** autenticação, login, seleção/criação, entrada no mapa, movimento, conexão, modelos principais, mouse/window, criação opcode, checks de modelo/processo, carregamento de NPCs, grande parte da árvore de UI/images.

**Causa atual dos mobs identificada:** `.spm` CP949 + literais coreanos corrompidos no decoder; parser não reconhece os tokens reais.

**Aberto:** patch CP949 do decoder, corrupção de textos/NPC names, LowLevelPet/dragão, validação de `Effect`/`AssaEffect`, pipeline de criação de SMD/SMB/INX e modernização gráfica.

---

### Apêndice A - Tokens CP949 observados

```text
*출연자          2A-C3-E2-BF-AC-C0-DA
*최대동시출현수  2A-C3-D6-B4-EB-B5-BF-BD-C3-C3-E2-C7-F6-BC-F6
*출현수          2A-C3-E2-C7-F6-BC-F6
*출현간격        2A-C3-E2-C7-F6-B0-A3-B0-DD
*출연자두목      2A-C3-E2-BF-AC-C0-DA-B5-CE-B8-F1
```

### Apêndice B - Marcadores relevantes no source

```text
MINGW_MODEL_PATH_CHECKSUM_V2
MINGW_ADDJOB_LOAD_FAIL_GUARD_V2
MINGW_CHANGEMOTION_NULL_GUARD_V1
MINGW_WINDOWMODE_STATE_FIX_V1
MINGW_GAME_OPCODE_FAMILY_FILTER_V2
MINGW_CHECKUSERMODEL_SEPARATOR_FIX_V1
MINGW_PROCESSINFO_PATH_SEPARATOR_FIX_V1
MINGW_SETFIELDINFOPATH_SEPARATOR_FIX_V1
MINGW_STAGE_SPAWN_LOAD_TRACE_V1
```

### Apêndice C - Evidências de sessão usadas neste relatório

- Logs de startup, packet census e traces MinGW produzidos ao longo da sessão.
- `Texto colado (20260822-121307).txt`: `LoadStage`, `AutoPlayServer`, `STG_AREA::Main`.
- `Texto colado (20260822-124234).txt`: `spawn_trace.txt` demonstrando `starts>0`, `chrMonsterCnt=374`, `counter=0`.
- `Textos colados (2)(20260822-124403).txt`: implementação completa de `DecodeOpenMonster()`.
- Inventários de assets, hashes do LowLevelPet e árvores dos donors produzidos durante a investigação.
