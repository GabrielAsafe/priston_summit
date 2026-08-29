# Projeto Priston Tale Server/Game – Retomada (13/07/2026)

## Estado Atual

O projeto encontrava-se numa fase de migração e reconciliação entre múltiplas versões do código:

* SrcServer
* SrcGame
* Shared
* Shared2
* vários backups criados automaticamente durante tentativas anteriores de correção

O objetivo principal foi restaurar a compilação do servidor MinGW i686 sem introduzir alterações funcionais perigosas.

---

# Alterações Realizadas

## 1. Correção de conflito de opcodes

Foi identificado que existiam múltiplas versões de:

```cpp
opcodes.h
```

Locais encontrados:

```text
./Shared/opcodes.h
./SrcGame/src/opcodes.h
./SrcServer/src/opcodes.h
```

Foi confirmado através de:

```bash
find . -iname "opcodes.h"
grep -n '#include.*opcodes.h'
g++ -H -E
```

que o compilador estava a incluir simultaneamente:

```text
SrcServer/src/opcodes.h
Shared/opcodes.h
```

O problema gerava:

* redefinições
* conflitos de namespace
* símbolos ausentes dependendo da ordem dos includes

### Situação final

Foi mantido:

```text
SrcServer/src/opcodes.h
```

como ficheiro principal do servidor.

Foi evitado substituir totalmente pelo Shared devido a:

```text
RenderDevice
DIRECTDRAWSURFACE
redefinitions de smType.h
```

que apareceram quando o bridge automático foi testado.

---

## 2. Correção da estrutura sCRAFTITEM_INFO

Foi encontrado um desalinhamento entre:

```cpp
SrcGame/src/sinbaram/sinTrade.h
```

e

```cpp
SrcServer/src/sinbaram/sinTrade.h
```

O servidor possuía:

```cpp
int iMixType;
```

enquanto o cliente já não possuía esse campo.

Durante a tentativa de remoção surgiram erros em:

```cpp
sCraftItem_Info[i].iMixType
```

Foi decidido não alterar a estrutura original do sistema de craft.

---

## 3. Correção do sinCraftItem_Server.h

Foi detectado:

```text
too many initializers for DWORD[8]
```

Foi analisado:

```cpp
DWORD MainItemCode[8];
```

e foram encontrados vários blocos com:

```cpp
{0,0,5,0,0,0,0,0,0}
```

(9 valores)

Foram criados backups e efetuadas correções.

Posteriormente verificou-se que a estrutura estava alinhada com a versão original do projeto e o ficheiro foi restaurado.

---

## 4. Correção temporária do Craft

Para permitir compilação imediata:

Linha:

```cpp
sCraftItem_Recv.DesCraftItem.sItemInfo.iMixType =
    sCraftItem_Info[i].iMixType;
```

foi temporariamente substituída por:

```cpp
sCraftItem_Recv.DesCraftItem.sItemInfo.iMixType = 0;
```

Resultado:

```text
SINTRADE OK
```

---

## 5. Investigação dos OpCodes ausentes

Foram procurados os seguintes símbolos:

```text
CHECK_SOD_CROWN
OPCODE_BAD_PLAYER
OPCODE_CHECK_EXPDATA
OPCODE_CONTINUEDATA
OPCODE_FORCEORB_DATA
OPCODE_BOOSTER_DATA
OPCODE_SKILLDELAY_DATA
OPCODE_PRIMEITEM_DATA
OPCODE_PRIMEITEM_DATA2
OPCODE_PRIMEITEM_DATA3
OPCODE_PRIMEITEM_DATA4
```

Comandos usados:

```bash
grep -RIn
find
wc -l
```

Resultado:

```text
Não existem em:
Shared
Shared2
SrcGame
SrcServer
```

Conclusão:

Esses opcodes pertencem a uma revisão diferente do protocolo.

---

## 6. Desativação de funcionalidades incompatíveis

Foi criado script para desativar blocos legacy em:

```cpp
SrcServer/src/Server/OnSever.cpp
```

Funcionalidades removidas temporariamente:

* WARMODE
* UPDATE_ITEM_PREMIUM
* OPCODE_CONTINUEDATA
* OPCODE_CHECK_EXPDATA
* OPCODE_SET_OBJSERIAL
* OPCODE_FUNCTON_RESULT
* OPCODE_BAD_PLAYER
* OPCODE_FORCEORB_DATA
* OPCODE_BOOSTER_DATA
* OPCODE_SKILLDELAY_DATA
* OPCODE_PRIMEITEM_DATA
* OPCODE_PRIMEITEM_DATA2
* OPCODE_PRIMEITEM_DATA3
* OPCODE_UPDATE_SERVER_PARAM
* OPCODE_EVENT_GAME

Objetivo:

```text
Compilar primeiro.
Restaurar funcionalidade depois.
```

---

# Ferramentas Linux Utilizadas

## Pesquisa de ficheiros

```bash
find
```

Exemplo:

```bash
find . -iname "opcodes.h"
```

---

## Pesquisa textual

```bash
grep
```

Exemplos:

```bash
grep -RIn "OPCODE_BAD_PLAYER"
grep -n "iMixType"
```

---

## Diferenças entre ficheiros

```bash
diff
```

Exemplo:

```bash
diff -u ficheiroA ficheiroB
```

---

## Visualização parcial

```bash
sed
```

Exemplos:

```bash
sed -n '1,80p'
sed -n '4795,4820p'
```

---

## Edição automática

```bash
sed -i
```

Exemplo:

```bash
sed -i 's/antigo/novo/'
```

---

## Backups

```bash
cp -a
```

Exemplo:

```bash
cp -a ficheiro ficheiro.bak
```

---

## Pré-processamento do compilador

```bash
g++ -E
```

Usado para descobrir:

```text
ordem real dos includes
```

---

## Árvore de includes

```bash
g++ -H
```

Permitiu confirmar:

```text
SrcServer/src/opcodes.h
Shared/opcodes.h
```

a serem incluídos simultaneamente.

---

## Compilação isolada

```bash
g++ -c
```

Exemplo:

```bash
g++ -c sinTrade.cpp
```

Permitiu validar módulos individualmente.

---

## Scripts Python

Foram usados para:

* identificar arrays inválidos
* localizar funções
* desativar blocos automaticamente
* criar patches em massa

---

# Estado Atual dos Problemas

Ainda existem erros em:

```cpp
OnSever.cpp
```

relacionados com:

```text
CHECK_SOD_CROWN
PLAYER_GM
OPCODE_IPLIST
OPCODE_BAD_PLAYER
OPCODE_PRIMEITEM_DATA*
OPCODE_FORCEORB_DATA
OPCODE_BOOSTER_DATA
OPCODE_SKILLDELAY_DATA
OPCODE_UPDATE_SERVER_PARAM
```

e também:

```cpp
smTRANS_PLAYDATA::Mp
```

```cpp
sMYSHOP_SELL_INFO::CoinPrice
```

Estes membros não existem na revisão atual das estruturas.

---

# Próximos Passos

## Fase 1 — Fazer OnSever.cpp compilar

Objetivo:

```text
0 erros em OnSever.cpp
```

Ações:

1. Desativar funções completas que dependem dos opcodes inexistentes.
2. Resolver o erro:

```cpp
goto quit
```

que atravessa:

```cpp
char accountId[32]
```

3. Corrigir referências:

```cpp
Mp
CoinPrice
```

para os campos existentes.

---

## Fase 2 — Build completo

Executar:

```bash
bash build_server_mingw_i686_j8.sh
```

e obter:

```text
Todos os .cpp compilados
```

---

## Fase 3 — Linkagem

Resolver:

```text
undefined reference
```

ou bibliotecas em falta.

Provavelmente surgirão problemas em:

```text
DirectX
Lua
MySQL
Sockets
```

---

## Fase 4 — Arranque do servidor

Validar:

```text
Login
Personagem
Mapa
Inventário
NPC
Craft
```

---

## Fase 5 — Restaurar funcionalidades removidas

Reativar gradualmente:

```text
Prime Item
Booster
Force Orb
WarMode
Event Game
Bad Player
IP List
```

mas apenas após descobrir os opcodes corretos da revisão original.

---

# Recomendação

Neste momento não deve ser adicionada qualquer constante inventada ao namespace:

```cpp
OpCode
```

A ausência dos símbolos indica mistura de revisões diferentes do projeto.

O caminho mais seguro é:

```text
1. Fazer compilar.
2. Fazer arrancar.
3. Restaurar funcionalidades uma a uma.
```

Isso evita criar incompatibilidades de rede difíceis de diagnosticar posteriormente.
