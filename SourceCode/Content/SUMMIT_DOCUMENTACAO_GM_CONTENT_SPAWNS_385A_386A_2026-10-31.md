# SUMMIT — Arquitetura, publicação e execução de spawns do Editor GM no Content

**Documento de referência e passagem de contexto**  
**Data:** 31 de outubro de 2026  
**Projeto:** SUMMIT (cliente real: `summit/cliente/`; servidor real: `summit/Servidor/`)  
**Implementações abrangidas:** patches **385A** e **386A**; relação com os nodes de profissões 371A/384A.  
**Âmbito:** Editor GM → SQLite → TEMP JSON → Content canónico → pack de campo `field_runtime.sfld` → servidor → monstros no mapa.  
**Estado:** 385A executado com sucesso no projeto do utilizador; 386A entregue, validado em testes locais e num Content de referência, **ainda sem confirmação de compilação MinGW e funcionamento real em Wine na instalação do utilizador**.

> **Regra principal:** guardar um ponto no editor GM, publicar um JSON canónico, compilar o pack e ver um monstro no jogo são **quatro etapas distintas**. Nenhuma delas deve ser declarada concluída só porque a etapa anterior passou.

---

## 1. Porque foi necessária esta integração

O utilizador colocou 11 pontos de monstros no **mapa 58 — StarArena** através da janela de GM. A aplicação registou os pontos na tabela SQLite `ServerDB__SpawnPlan361`, todos com `Enabled=0`. O servidor arrancava, os quatro nodes de teste das profissões eram criados, e Lumberjack conseguia recolher `common_logs`/timber; no entanto, os monstros colocados no editor GM não apareciam.

**Causa funcional identificada:** o comando antigo de colocação `/spawnplan` guardava **planos inativos**, sem um circuito completo que os publicasse no Content e os materializasse no mundo. Não bastava trocar `Enabled=0` por `1` à mão. Também não bastava editar o ficheiro SPM: o formato SPM não guarda as posições tridimensionais individuais do editor GM.

O objetivo acordado foi criar o fluxo:

```text
Editor GM / comando de colocação
       │ grava os pontos
       ▼
Servidor/Database/summit.sqlite3
  ServerDB__SpawnPlan361
       │ export 385A (leitura apenas)
       ▼
bugfixes/GM_SPAWN_TEMP_385A_MAP058_*.json
       │ preview / validação / publish 385A
       ▼
SourceCode/Content/canonical/gm_spawn_points/map_058.json
       │ aprovação explícita da identidade dos monstros (386A)
       ▼
SourceCode/Content/canonical/gm_spawn_points/map_058.published.json
       │ compilação do Content (386A)
       ▼
SourceCode/Content/packs/field_runtime.sfld
  SSPB + SPPT + SPCN + GMSP
       │ deploy verificado por SHA-256
       ▼
Servidor/Content/packs/field_runtime.sfld
       │ Server.exe atualizado + restart + jogador no mapa
       ▼
Instâncias de monstros em posições XYZ, com limite e respawn
```

**Atenção:** no 385A o ficheiro TEMP ainda é gerado com um comando de terminal a partir da SQLite; o clique em «Guardar» da janela GM **não exporta automaticamente** o TEMP. O fluxo guarda os pontos primeiro na SQLite.

---

## 2. Os três componentes clássicos do Content: SPM, SPP e SPC

O SUMMIT usa três famílias canónicas de informação de campo, que são compiladas no pack `field_runtime.sfld`:

| Componente | Formato canónico | Função | Quando atualizar |
|---|---|---|---|
| **Spawns de monstros** | `*.spm.json` | Define os tipos de monstros, limites, pesos, composição e regras gerais de geração de cada cenário | Quando se altera a **composição/regras** dos spawns existentes |
| **Pontos do cenário** | `*.spp.json` | Define pontos de referência/entrada/arranque do cenário; as estruturas atuais têm sobretudo coordenadas X/Z | Quando se modificam os **pontos de cenário** correspondentes |
| **NPCs fixos** | `*.spc.json` | Define os NPCs fixos associados ao cenário | Quando se cria/altera/remove um **NPC fixo** |

**Estes três ficheiros não são obrigatoriamente preenchidos ou alterados ao mesmo tempo.** O publicador deve verificar quais são relevantes para a operação concreta, preservar os restantes e não gerar documentos falsos para satisfazer um número fixo de ficheiros.

### Distinção essencial: SPM não é um editor de posições XYZ

O SPM existente descreve **quem pode aparecer e sob que regras**. Não é um formato para as 11 posições `X,Y,Z` que foram colocadas manualmente no editor GM. Copiar os registos GM para o campo `actors` de um SPM **perderia as coordenadas**, a identidade de cada `PlanId` e os seus limites/respawn particulares.

Por isso, a solução adotada não força 11 entradas GM a caber em SPM/SPP/SPC. O 386A acrescenta **um quarto componente**, `GMSP`, para as posições XYZ criadas no editor.

### Conteúdo físico compilado no SFLD

| Secção do pack | Versão utilizada | Origem funcional | Tratamento no 386A |
|---|---:|---|---|
| `SSPB` | 2 | SPM — composição dos monstros | **Preservar** os dados existentes |
| `SPPT` | 1 | SPP — pontos do cenário | **Preservar** os dados existentes |
| `SPCN` | 1 | SPC — NPCs fixos | **Preservar** os dados existentes |
| **`GMSP`** | **1** | `gm_spawn_points/map_XXX.published.json` | **Adicionar** posições GM validadas |

Um pack que já tinha três secções passa, após a integração e a publicação de pontos aprovados, a conter quatro. **Não significa que os três ficheiros canónicos tenham sido alterados.**

---

## 3. Confirmação específica do mapa 58 — StarArena

No `Content1.zip` fornecido pelo utilizador, foi possível confirmar os seguintes documentos de referência:

| Caminho relativo a `SourceCode/Content/canonical/` | Estado observado no ZIP de referência |
|---|---|
| `stage_spawns/spawns/GameServer/Field/StarArena.ASE.spm.json` | Existe com `actors: []` e `bosses: []`; define `limit_max: 10` e regras de intervalo, **não** as 11 posições GM |
| `stage_start_points/points/GameServer/Field/StarArena.ASE.spp.json` | Existe com **4 pontos X/Z** associados aos quatro nodes experimentais anteriores |
| `fixed_npcs/npcs/GameServer/Field/StarArena.ASE.spc.json` | **Não existe** no ZIP; não é obrigatório se não houver NPCs fixos a publicar |
| `maps/server.maps.json` | Regista o mapa 58 como `map.star_arena`; não precisa de alteração só para adicionar monstros a um mapa existente |
| `gm_spawn_points/map_058.json` | Novo rascunho publicado pelo **385A** no projeto do utilizador |
| `gm_spawn_points/map_058.published.json` | Documento aprovado criado pelo **386A** após validação das identidades |

**Não confundir os quatro pontos SPP dos nodes antigos com os 11 pontos GM.** Também não confundir monstros GM comuns com os nodes de recolha das profissões.

### Os 11 pontos guardados pelo utilizador

Todos os registos estavam em `MapId=58`, `Enabled=0`, antes do fluxo de publicação:

| PlanId | `ContentKey` | Densidade | X | Y | Z | `MaxAlive` |
|---:|---|---|---:|---:|---:|---:|
| 1 | `monplant` | low | 250385 | 721 | 225276 | 2 |
| 2 | `monplant` | low | 250357 | 721 | 225432 | 2 |
| 3 | `monplant` | low | 250369 | 721 | 225440 | 2 |
| 4 | `monplant` | low | 250381 | 721 | 225448 | 2 |
| 5 | `undeadminer` | high | 250389 | 817 | 226422 | 12 |
| 6 | `undeadminer` | high | 250306 | 810 | 226671 | 12 |
| 7 | `undeadminer` | high | 250399 | 810 | 226748 | 12 |
| 8 | `monplant` | low | 250369 | 810 | 226667 | 2 |
| 9 | `undeadminer` | low | 250277 | 810 | 226824 | 2 |
| 10 | `undeadminer` | low | 250306 | 810 | 226852 | 2 |
| 11 | `turtlecannon` | medium | 250468 | 810 | 226882 | 6 |

**Total:** 11 pontos, distribuídos por 5 `monplant`, 5 `undeadminer` e 1 `turtlecannon`; soma de `MaxAlive` **56**. O número 56 representa a **capacidade máxima configurada**, não uma promessa de que 56 monstros apareçam instantaneamente.

O log enviado confirmou uma execução bem-sucedida do 385A:

```text
SUMMIT385A_PREVIEW map=58 points=11 new=11 changed=0 unchanged=0
MODEL_REFERENCES={"model_family_candidate_only": 5, "model_variant_ambiguous": 6}
CONTENT=SourceCode/Content/canonical/gm_spawn_points/map_058.json
RUNTIME_ACTIVATION=NO (novo formato ainda nao consumido pelo Server.exe)
SUMMIT385A_PUBLISHED points=11 backup=bugfixes/_backups_385a/...
```

Assim, **os 11 pontos já estão registados no Content canónico na instalação do utilizador**, mas esta confirmação diz respeito ao **385A**, não à aprovação 386A nem à sua execução real.

---

## 4. Patch 385A — SQLite → TEMP → rascunho do Content

**Pacote:** `SUMMIT_PATCH_385A_GM_TEMP_PARA_CONTENT_2026-10-31.zip`.

O 385A é uma ferramenta Python portátil: **não altera o servidor, o cliente nem a SQLite**. A exportação abre a base em modo de leitura e cria um JSON temporário em `bugfixes/`; `preview` compara o TEMP com a base atual; `publish` faz merge protegido no documento canónico.

### Comandos de referência

Executar na raiz real do SUMMIT:

```bash
cd "$HOME/Área de Trabalho/summit"
unzip -n bugfixes/SUMMIT_PATCH_385A_GM_TEMP_PARA_CONTENT_2026-10-31.zip -d bugfixes
P385="bugfixes/SUMMIT_PATCH_385A_GM_TEMP_PARA_CONTENT_2026-10-31"
(cd "$P385" && sha256sum -c SHA256SUMS.txt)
sh "$P385/run_385a_tests.sh"

TOOL385="$P385/tools/gm_spawn_content_385a.py"
TEMP="$(python3 "$TOOL385" export --root . --map 58 --print-path)"
python3 "$TOOL385" preview --root . --temp "$TEMP"
python3 "$TOOL385" publish --root . --temp "$TEMP"
```

**No estado atual do utilizador, esta etapa já foi executada com sucesso.** Não voltar a correr desnecessariamente para «ativar monstros»: o 385A nunca foi um comando de ativação. Uma nova exportação só é necessária se os pontos da SQLite tiverem sido modificados.

### Regras de segurança do 385A

- O rascunho preserva `PlanId`, `ContentKey`, mapa, coordenadas, densidade, `MaxAlive`, raio e respawn.
- Não altera `ServerDB__SpawnPlan361.Enabled`, que pode continuar a `0` após publicação.
- Não inventa variantes de modelos: regista candidatos e ambiguidades.
- Recusa TEMP adulterado ou desatualizado relativamente à SQLite; recusa sobrescrever conteúdo divergente sem autorização explícita.
- A reaplicação idempotente não duplica os pontos.
- O estado de reversão fica em `bugfixes/_backups_385a/.../undo.json` e é validado por SHA-256.

**Resultado do 385A:** documento de **rascunho**, ainda não materializado no pack que o servidor usa.

---

## 5. Patch 386A — Aprovação → `GMSP` → runtime

**Pacote:** `SUMMIT_PATCH_386A_GM_CONTENT_RUNTIME_2026-10-31.zip`.

O 386A acrescenta quatro capacidades técnicas:

1. **Aprovação do GM Content:** resolve os IDs de monstros contra o catálogo canónico `Content/canonical/text`, cria `map_058.published.json` e mantém intacto `map_058.json`.
2. **Compilação do Content:** acrescenta a secção `GMSP` ao `field_runtime.sfld`, sem substituir as secções `SSPB`, `SPPT` e `SPCN`.
3. **Carregamento pelo `Server.exe`:** lê posições validadas, cria instâncias de monstros perto de jogadores do mapa e controla limites/respawn.
4. **Deploy seguro do pack:** copia explicitamente o `field_runtime.sfld` compilado para `Servidor/Content/packs/` com SHA-256, backup e undo.

### Resolução de identidades, não apenas nomes de ficheiros

| Código do Editor GM | ID de monstro canónico usado no 386A | Observação |
|---|---|---|
| `undeadminer` | `monster.111_mineiro_das_cinzas` | Identidade resolvida pelo catálogo canónico |
| `turtlecannon` | `monster.89_tartaruga_blindada` | Identidade resolvida pelo catálogo canónico |
| `monplant` | `monster.chefe_14_u00c1rvore_maldita` **ou** `monster.13_planta` | **Ambíguo**: requer escolha explícita |

O código técnico `monplant` não prova por si só qual das duas variantes se pretendia. O publicador exige a opção `--monplant arvore_maldita` ou `--monplant planta`. Para os testes do mapa 58 realizados anteriormente, a seleção pretendida foi `arvore_maldita` (referência `monplant-d.ini`), **mas o modelo e a aparência em jogo continuam a exigir validação visual**.

### Validações antes de publicar

O 386A recusa a publicação se encontrar ID de monstro não resolvido, modelo divergente do catálogo, coordenada inválida, pontos duplicados, documento rascunho alterado após aprovação, ou limites de densidade superiores ao suportado.

Entre as restrições implementadas no builder atual estão `MapId` entre 0 e 255, `MaxAlive` por ponto de 1 a 12, raio de 30 a 1500, respawn de 10 a 3600 segundos, e **limite global de 96** para o total publicado (pontos e capacidade de monstros).

### Formato binário

O `GMSP` é uma secção do SFLD que contém uma estrutura interna `GMS1`:

```text
SFLD (pack de campo)
  SSPB v2  — composição/regras de monstros
  SPPT v1  — pontos do cenário
  SPCN v1  — NPCs fixos
  GMSP v1  — pontos XYZ aprovados pelo GM
      GMS1 header: 48 bytes
      cada registo: 96 bytes
      campos: plan_id, map_id, x, y, z,
              max_alive, radius, respawn_seconds,
              monster_name (campo fixo de 64 bytes)
      SHA-256 interno do conjunto de registos
```

São verificadas as integridades do pack SFLD e da secção `GMSP` antes de se criarem monstros. O cliente e servidor têm de receber as versões compiladas correspondentes.

---

## 6. Instalação e publicação do 386A — sequência completa

**Pré-condições:** a pasta real é `~/Área de Trabalho/summit`; o 385A já publicou o rascunho do mapa 58; cliente e servidor estão encerrados; o outro chat **não está simultaneamente a publicar Content**. Não usar OLD como fonte do jogo.

### A. Descompactar, verificar, testar e instalar

```bash
cd "$HOME/Área de Trabalho/summit"

unzip -n \
  bugfixes/SUMMIT_PATCH_386A_GM_CONTENT_RUNTIME_2026-10-31.zip \
  -d bugfixes

P386="bugfixes/SUMMIT_PATCH_386A_GM_CONTENT_RUNTIME_2026-10-31"

(
  set -e
  (cd "$P386" && sha256sum -c SHA256SUMS.txt)
  bash "$P386/run_386a_tests.sh"
  python3 "$P386/install_386a.py" inspect --root .
  python3 "$P386/install_386a.py" apply --root .
)
```

### B. Inspecionar identidades e aprovar os 11 pontos

```bash
python3 "$P386/tools/publish_gm_content_386a.py" \
  inspect --root . --map 58 --monplant arvore_maldita

# Confirmar os IDs, nomes e modelos que a inspeção mostra ANTES de executar:
python3 "$P386/tools/publish_gm_content_386a.py" \
  publish --root . --map 58 --monplant arvore_maldita --approve
```

O publicador cria:

`SourceCode/Content/canonical/gm_spawn_points/map_058.published.json`

e imprime um `UNDO_STATE` em `bugfixes/_backups_386a/`. **Isto ainda não faz aparecer monstros no jogo**: falta compilar, distribuir e reiniciar.

### C. Compilar cliente/servidor/Content e inspecionar o pack

```bash
sh SourceCode/compboth.sh

python3 SourceCode/tools/content_port/inspect_field_runtime.py \
  SourceCode/Content/packs/field_runtime.sfld
```

**Só avançar para o deploy se** a compilação terminar sem erros e a inspeção confirmar as secções `SSPB`, `SPPT`, `SPCN` e `GMSP` com **11 pontos**. Se `compboth.sh` falhar durante uma importação de itens realizada noutro chat, corrigir o Content canónico dessa importação primeiro; **não distribuir um pack antigo**.

### D. Deploy explícito do pack

```bash
python3 "$P386/tools/deploy_field_pack_386a.py" inspect --root .
python3 "$P386/tools/deploy_field_pack_386a.py" apply --root .
```

Origem do deploy:

`SourceCode/Content/packs/field_runtime.sfld`

Destino:

`Servidor/Content/packs/field_runtime.sfld`

O deploy valida as quatro secções, o SHA interno e os limites GM, cria backup e evita substituir um pack por conteúdo inválido. A **cópia do executável** do servidor é uma etapa separada; caso o envio do `Server.exe` tenha falhado por estar aberto, depois de o encerrar executar o procedimento habitual:

```bash
sh SourceCode/send_to_s.sh
```

### E. Arrancar o servidor e validar em Wine

```bash
cd "$HOME/Área de Trabalho/summit/Servidor"
wine Server.exe
```

Quando um jogador entrar no mapa 58, procurar mensagens como:

```text
[SUMMIT_GM_386] LOADED plans=11 capacity=56 pack=...
[SUMMIT_GM_386] SPAWN plan=... serial=... map=58 name=...
```

O gestor GM cria no máximo **quatro monstros por ciclo de aproximadamente um segundo** e só gera os monstros com jogadores no respetivo mapa. Pode haver atrasos de visibilidade. Se surgirem mensagens `SPAWN_FAIL plan=... reason=...`, recolher essas linhas; as causas possíveis incluem nome de template não resolvido, mapa/tabela de área indisponível ou capacidade global esgotada.

**Não considerar a publicação validada apenas com `LOADED`:** também são necessárias linhas `SPAWN` e confirmação visual dos monstros no cliente.

---

## 7. O que é alterado e o que fica intacto

| Área | 385A | 386A |
|---|---|---|
| `Servidor/Database/summit.sqlite3` | **Lê** para exportação; não escreve | Não escreve |
| `SourceCode/Content/canonical/gm_spawn_points/map_058.json` | **Cria/atualiza**, com backup | **Lê e preserva** |
| `SourceCode/Content/canonical/gm_spawn_points/map_058.published.json` | Não cria | **Cria após aprovação** |
| `*.spm.json` / `*.spp.json` / `*.spc.json` | Não altera | **Não altera**; reutiliza ao compilar SFLD |
| `SourceCode/Content/packs/field_runtime.sfld` | Não altera | Recompila com `GMSP` |
| `Servidor/Content/packs/field_runtime.sfld` | Não altera | Copia com backup e SHA-256 no deploy explícito |
| Código-fonte do compilador e do runtime | Não altera | Altera pontos de integração e acrescenta helpers |
| `Game.exe`, `Server.exe` | Não altera | Novos executáveis **só** após compilação/deploy feitos pelo utilizador |
| `cliente/image`, `cliente/cooked`, inventário DAT, itens, receitas, lojas | Não altera | Não altera |
| Quatro nodes visuais das profissões | Não altera | **Permanecem independentes** dos 11 spawns GM |

No editor GM, **monstros, pontos de cenário e NPCs fixos são conceitos distintos**. Se amanhã o editor criar um NPC, será preciso um fluxo próprio para SPC; se alterar pontos de entrada, será preciso tratar SPP. Os dados não devem ser despejados indiscriminadamente em três ficheiros.

---

## 8. Validação realizada e pendências reais

### Confirmado no computador do utilizador

- A consulta SQL encontrou **11 pontos GM**, todos com `Enabled=0`, no mapa 58.
- O 385A passou o teste de integridade do ZIP e **6 testes automatizados**.
- `export`, `preview` e `publish` do 385A terminaram com sucesso.
- O ficheiro canónico `gm_spawn_points/map_058.json` foi criado.
- Na execução anterior, o servidor produziu nodes de recolha e creditou `common_logs`, o que **não** valida visualmente os 11 novos monstros GM.

### Confirmado localmente nos ficheiros entregues

- Integridade SHA-256 dos pacotes 385A e 386A: **PASS**.
- Testes automatizados de 385A: **6/6 PASS** (reexecutados para esta documentação).
- Testes automatizados de 386A: **5/5 PASS** (reexecutados para esta documentação).
- Os testes 386A cobrem publicação/roundtrip do pack, limites de densidade, proteção de identidades, instalador, deploy, backups e undo.
- Segundo a validação da entrega 386A, foi gerado SFLD com quatro secções sobre Content de referência, com **11 pontos** e **capacidade 56**.

### Não confirmado / não declarar como concluído

- Aplicação do 386A no projeto atual do utilizador (não foi recebido o log de instalação).
- Compilação MinGW i686 integral do cliente e servidor **depois** do 386A.
- Deploy do pack GM em `Servidor/Content/packs` na máquina do utilizador.
- Arranque do `Server.exe` com `GMSP` real, instâncias criadas e monstros visíveis em Wine.
- Resolução de eventuais falhas de modelo/visibilidade do **cliente**; `presence_sent=1` no servidor não garante desenho no cliente.
- Invisibilidade residual dos quatro nodes 3D das profissões: **não resolvida automaticamente pelo 386A**.
- Exportação automática de TEMP pelo botão Guardar da UI GM: **ainda não implementada**.

---

## 9. Diagnóstico operacional

| Sintoma | Interpretação provável | Ação segura |
|---|---|---|
| `map_058.json` existe mas os monstros não aparecem | Só foi executado o 385A | Aprovar, compilar e distribuir com 386A |
| `map_058.published.json` existe mas `GMSP` não aparece no SFLD | Compilação do Content não atualizada ou interrompida | Rever saída de `compboth.sh`, não distribuir pack antigo |
| `GMSP` existe na source mas não no servidor | Deploy do pack omitido ou destino errado | Usar `deploy_field_pack_386a.py inspect/apply` |
| `[SUMMIT_GM_386] NO_GMSP_SECTION` | Servidor lê pack sem secção GM | Conferir Content e SHA do pack do runtime |
| `[SUMMIT_GM_386] LOADED ...` mas sem `SPAWN` | Ainda sem jogadores no mapa, espera de ciclo ou outro bloqueio | Entrar no mapa 58; procurar `SPAWN_FAIL` e confirmar distância/capacidade |
| `SPAWN_FAIL ... unresolved-runtime-monster-template` | Monstro não localizado no catálogo runtime | Rever ID/nome de monstro e conteúdo canónico; **não inventar ID** |
| Há linhas `SPAWN` mas criaturas invisíveis | Problema de rede/entidade/modelo/renderização | Verificar logs de cliente, assets e modelo publicado; não alterar SQLite |
| Publicação recusada por rascunho alterado | O JSON original mudou depois da aprovação | Rever diferenças, reinspecionar e republicar conscientemente |
| `SOURCE_DIVERGENT` ou conflito com outro chat | Código/Content modificado em paralelo | Parar, comparar mudanças e preservar o trabalho da outra sessão |

**Não editar manualmente `ServerDB__SpawnPlan361.Enabled=1` para “forçar” spawn.** A presença dos pontos em Content, o pack compilado e o servidor atualizado são as condições reais do fluxo novo.

---

## 10. Backups e reversão

O processo tem estados **independentes**, que devem ser guardados e usados na ordem adequada:

1. **385A:** `bugfixes/_backups_385a/.../undo.json` — reverte o rascunho de Content.
2. **386A publicação:** `bugfixes/_backups_386a/.../undo.json` — reverte a aprovação (`*.published.json`).
3. **386A deploy:** `bugfixes/_backups_386a_pack/.../state.json` — restaura o pack do servidor.
4. **386A instalador:** `bugfixes/_backups_386a_install/.../state.json` — restaura os ficheiros de código alterados.

**Exemplos** — substituir `PASTA_REAL` pelo diretório exato indicado na execução (não copiar literalmente):

```bash
cd "$HOME/Área de Trabalho/summit"
P386="bugfixes/SUMMIT_PATCH_386A_GM_CONTENT_RUNTIME_2026-10-31"
P385="bugfixes/SUMMIT_PATCH_385A_GM_TEMP_PARA_CONTENT_2026-10-31"

# Desfazer aprovação do GM Content:
python3 "$P386/tools/publish_gm_content_386a.py" undo --root . \
  --undo-state 'bugfixes/_backups_386a/PASTA_REAL/undo.json'

# Restaurar o pack anterior do servidor:
python3 "$P386/tools/deploy_field_pack_386a.py" undo --root . \
  --undo-state 'bugfixes/_backups_386a_pack/PASTA_REAL/state.json'

# Restaurar integração de código (requer recompilar para compatibilizar runtime):
python3 "$P386/install_386a.py" undo --root . \
  --state 'bugfixes/_backups_386a_install/PASTA_REAL/state.json'

# Reverter o rascunho do 385A, apenas se essa reversão for realmente pretendida:
python3 "$P385/tools/gm_spawn_content_385a.py" undo --root . \
  --state 'bugfixes/_backups_385a/PASTA_REAL/undo.json'
```

Cada ferramenta verifica hashes e pode recusar o undo caso o ficheiro tenha sido modificado entretanto. **Não apagar backups, não forçar a reversão e não misturar executáveis compilados com packs/código de versões incompatíveis.** Após reverter publicação/código, recompilar e redistribuir o Content/executável coerentes.

---

## 11. Critérios de aceitação para dar o Content como “fechado”

- [x] Os 11 pontos de mapa 58 foram guardados na SQLite e exportados pelo 385A.
- [x] O 385A publicou o rascunho canónico com preview e backups.
- [x] Os pacotes 385A/386A passaram os respetivos testes locais.
- [x] O 386A contém o compilador da secção `GMSP`, a integração de runtime e o publicador com aprovação explícita.
- [ ] O 386A foi aplicado na source **atual** do utilizador sem conflitos.
- [ ] O ficheiro `map_058.published.json` foi aprovado com a variante pretendida (`arvore_maldita`/`planta`).
- [ ] `sh SourceCode/compboth.sh` terminou com compilação **e** Content sem erros.
- [ ] A inspeção do SFLD atual confirma **SSPB, SPPT, SPCN, GMSP / 11 pontos / 56 vagas**.
- [ ] O pack foi entregue ao `Servidor/Content/packs` com SHA verificado.
- [ ] O `Server.exe` atualizado inicia em Wine e regista `LOADED plans=11` e `SPAWN`.
- [ ] Os monstros aparecem realmente nas coordenadas do editor GM; morte/respawn/quantidade são testados.
- [ ] O outro chat confirma que as suas alterações de itens e perfis não foram substituídas.

**A integração está preparada e testada a nível de patch/Content de referência; o Content só deve ser considerado operacionalmente fechado depois dos testes pendentes na instalação atual.**

---

## 12. Mensagem de passagem para o outro chat (Content e itens)

> **SUMMIT — estado da publicação de spawns GM (31/10/2026).** O cliente real do jogo é `summit/cliente/` e o servidor é `summit/Servidor/`; OLD não é runtime. O Editor GM guardou 11 pontos no mapa 58 na tabela `ServerDB__SpawnPlan361` (inicialmente `Enabled=0`). O patch 385A exportou esses pontos via TEMP e publicou `SourceCode/Content/canonical/gm_spawn_points/map_058.json`, sem os ativar. O patch 386A acrescenta a aprovação explícita `map_058.published.json` e um quarto bloco `GMSP` ao `field_runtime.sfld`, juntamente com os blocos existentes `SSPB` (SPM), `SPPT` (SPP) e `SPCN` (SPC). Para a StarArena, o SPM existe mas tem `actors/bosses` vazios, o SPP contém quatro referências de nodes experimentais, e o SPC pode estar ausente. Os 11 pontos XYZ **não** são copiados para o SPM ou SPP, pois os formatos têm funções distintas. A aprovação 386A usa identidade canónica de `undeadminer` e `turtlecannon` e exige escolher entre as variantes `monplant` (`arvore_maldita` ou `planta`). O limite somado é de 56 monstros. **Não editar nem substituir** itens, perfis, DAT, `cliente/image`, `cliente/cooked`, `SourceCode/Content/canonical/item_profiles` ou packs de itens por causa deste trabalho. O 386A ainda necessita da compilação/deploy e validação em Wine no projeto atual antes de declarar os spawns em jogo como concluídos. Evitar operações concorrentes de publicação de Content durante `compboth.sh` e deploy.

---

## 13. Ficheiros e fontes consultadas

- `SUMMIT_PATCH_385A_GM_TEMP_PARA_CONTENT_2026-10-31.zip`: `LEIA_PRIMEIRO.md`, `ESTADO.md`, ferramentas, testes.
- `SUMMIT_PATCH_386A_GM_CONTENT_RUNTIME_2026-10-31.zip`: `LEIA_PRIMEIRO.md`, `ESTADO_REAL.md`, `publish_gm_content_386a.py`, `build_gm_spawn_runtime_386a.py`, `GmSpawnRuntime386.inl`, `deploy_field_pack_386a.py`, testes.
- `Content1.zip` fornecido pelo utilizador: formatos SPM, SPP e catálogo de mapas do mapa 58.
- Log e resultados partilhados pelo utilizador: os 11 `SpawnPlan361`, `SUMMIT385A_PREVIEW` e `SUMMIT385A_PUBLISHED`, e os eventos `SUMMIT_NODE_3D` de recolha.

**Regra de manutenção:** o **Content canónico aprovado** é a fonte de publicação; a SQLite do GM é a origem de edição/rascunho; o pack compilado é o artefacto consumido pelo servidor; os assets visuais continuam a ser carregados pela pasta real do cliente SUMMIT.
