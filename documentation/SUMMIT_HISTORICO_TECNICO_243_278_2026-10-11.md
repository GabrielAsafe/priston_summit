# SUMMIT — Histórico técnico, validações e operação segura dos modelos (PATCHES 243–278)

**Data de referência:** 11 de outubro de 2026  
**Ambiente do utilizador:** Fedora/Linux, Wine-Staging 11.0 nos ensaios reportados, MinGW i686, cliente Direct3D9 (`Game.exe`) e servidor Win32 (`Server.exe`).  
**Âmbito deste documento:** o trabalho desta frente de modelos, texturas, efeitos, áudio e ferramentas de build/cook; o estado dos bugs de renderização/NPC que ficaram a cargo de outro chat; e instruções operacionais para comparar, implantar e reverter caches nativas.

> **Resultado real confirmado:** `MODEL_COOK_263=PASS converted=134 reused=0 total=134 complete=True no_client_writes=1` seguido por `NATIVE_STAGE_267=PASS verified=134 expected=134 missing=0 complete=True no_client_writes=1`. Isto comprova que todos os **134 artefactos** exigidos pelo catálogo ASE daquela execução foram reconstruídos por conversão nativa, reabertos pelos validadores e aprovados pelo gate de integridade; **não comprova equivalência visual com o conteúdo anteriormente instalado**.

## 1. Regras e divisão de responsabilidades

- Não modificar `SourceCode/Content/canonical/**`. Existe um fluxo separado proprietário dos assets canónicos.
- Não mexer no Git staged existente: não executar `git reset`, `git stash`, `git clean`, `git add -A`; instaladores verificam o conteúdo e preservam o índice.
- Não introduzir novos logs permanentes nem opções de profiling do arranque do jogo. Para testes visuais, iniciar com `wine Game.exe` na pasta do cliente, com as DLLs atuais.
- Patches entregues em ZIP, com hashes SHA-256 do estado anterior e do novo, preflight, backups, rollback e instalação idempotente. Mudanças inesperadas e symlinks bloqueiam a aplicação.
- O cooker nativo é **offline**: reconverte cada ASE usando `Server.exe --summit-cook-model-one`, verifica os binários com o servidor e `Game.exe --summit-verify-model`, escreve só no staging em `bugfixes/`. O `Game.exe` normal não converte automaticamente todo o catálogo.
- O requisito explícito do utilizador é reconstruir SMD/SMB **mesmo quando as caches antigas estão perdidas/corrompidas**. Não substituir este contrato pela simples reutilização de SMD/SMB preexistentes.
- O PATCH 278 apenas gere a comparação/implantação posterior; **não altera o parser, nem o cooker, nem os assets da origem**.

## 2. Estrutura e build

| Área | Caminho relativo à raiz `summit/` |
|---|---|
| Source | `SourceCode/` |
| Cliente e DLLs de execução | `cliente/` |
| Executável normal | `cliente/Game.exe` |
| Servidor nativo | `Servidor/Server.exe` |
| Cliente compilado | `SourceCode/dist_mingw_client_i686/Game.exe` |
| Compilação total | `cd SourceCode && sh compboth.sh` |
| Cooker incremental | `SourceCode/tools/cook_native_models_incremental_263.py` |
| Verificador de staging | `SourceCode/tools/verify_native_model_stage_267.py` |
| Comparação/implantação/rollback | `SourceCode/tools/deploy_native_model_stage_278.py` |
| Staging validado | `bugfixes/SUMMIT_NATIVE_MODELS_INCREMENTAL_263/` |
| Patches e backups | `bugfixes/` |

O utilizador compilou e linkou cliente e servidor repetidamente (num build, 268 fontes do cliente e 299 do servidor, sem falhas). Verificar sempre `Servidor/Server.exe` depois da compilação antes de concluir algo sobre o cooker. Um teste de SHA-256 demonstrou que `cliente/Game.exe` e `SourceCode/dist_mingw_client_i686/Game.exe` eram idênticos naquele momento, apesar de os contextos de execução terem DLLs diferentes; **igualdade dos binários não prova igualdade do ambiente Wine**.

## 3. Contexto anterior ao cooker — patches 243 a 258

| Patches | Trabalho principal | Situação |
|---|---|---|
| 243 | Preparação global de imagens/texturas cooked e defaults | Entregue; gate de Cooked Images repetidamente PASS |
| 244 | Preferência pelas texturas cooked nos efeitos legacy | Entregue; aparência em jogo necessita validação |
| 245 | Censo dos ASE e caminhos de cache | Base de auditoria do cook nativo |
| 246 | Classificação de papéis mesh/bone, cobertura esperada SMD/SMB, caminhos legacy | Base de auditoria do cook nativo |
| 247 | Reutilização de slots WAV | Entregue; stress de áudio pendente |
| 248 | Segurança das animações/frames em efeitos | Entregue |
| 249 | Identidade de cache de meshes/bones nos efeitos HoEffect | Entregue |
| 250 | Proteções de áudio/caminhos/locks | Entregue |
| 251–253 | Catálogo de efeitos Assa, limites, buffers e ciclo de vida DirectSound | Entregues em bundle |
| 254 e 254R | Guardas nos slots de modelos no arranque e portabilidade de testes | Testes reportados PASS |
| 255 | Carregamento assíncrono de meshes HoEffect | Entregue |
| 256 | Prewarm progressivo Assa | Entregue |
| 257 | Antecipação de texturas de efeito animadas | Entregue |
| 258 | Limites das tabelas WAV/efeitos de áudio | Entregue |

**Limite do conhecimento:** gates estáticos e builds não garantem ausência dos antigos stutters, nem a correção do defeito atual de ecrã branco. O trabalho de renderização ficou separado.

## 4. Arquitetura do cooker e histórico dos PATCHES 259–277

### 4.1 Primeira conversão offline — 259–263

- **259:** Python orquestra conversões nativas via `Server.exe` isolado numa pasta temporária, nunca em `cliente/`; valida o cabeçalho do SMD/SMB e faz readback nativo do servidor. Primeiro ensaio real em `Effect/NewEffect/Res/Object/123.ASE` falhou com **crash Win32** (`rc=5`, EIP 0x0076725f).
- **260:** Guardas de recursos de partículas independentes do cook.
- **261:** `Game.exe --summit-verify-model` valida cada binário recém-gerado com o loader do cliente, além da releitura no servidor.
- **262:** Inicialização de materiais no modo offline. Testes passaram, mas o crash de `123.ASE` persistiu.
- **263:** Orquestrador incremental, censo/roles recalculados, digests ASE e das versões do Game/Server/conversor, staging íntegro com rollback, reutilização apenas de artefactos validados, sem escrita no cliente; utilitário para resolver os endereços de crash (apontaram para `smRead3d.cpp`, sem números de linha).

### 4.2 Proteções do parser e casos reais — 264–271

- **264:** Guardas de tokens, materiais e geometrias de `smRead3d.cpp`; o crash em `123.ASE` passou a rejeição controlada `geometry_missing_uv` (`rc=3`).
- **265:** Tratamento distinto de canais UV opcionais e códigos `reason=...` emitidos apenas no cook offline; inspetores de ASE de leitura.
- **266:** Permite `GEOMOBJECT` inteiramente sem UV, conservando as faces sem fabricar coordenadas; rejeita UV parciais/inválidos. Inspeção real do `123.ASE`: dez objetos, cada um com 80 faces e **zero UV**, convertidos com piloto `PASS`.
- **267:** Gate independente `verify_native_model_stage_267.py`: confere universo de caches esperadas, hashes das fontes e dos binários, provas de readback, identidade de executáveis e `complete=True`/`scope=all`.
- **268:** Bones não devem depender da tabela de materiais da malha; erro anterior `geometry_material_reference` em `b_death_knight.ASE`.
- **269:** Normalização controlada de IDs de faces `MESH_MTLID` nos materiais Multi/Sub-Object, com contagem real de submateriais.
- **270:** Alinhamento dos IDs de material com o índice da face, mesmo quando uma face não declara `MESH_MTLID`.
- **271:** Permissão estrita de geometria inteiramente sem tabela e com IDs neutros; não resolveu `Field/ani/test-ani.ASE`, que tinha tabela **existente mas vazia** e IDs 0..5.

No `death_knight.ASE`, a inspeção exibiu 43 objetos: um corpo com materiais Multi/Sub-Object e 42 partes `Bip*` auxiliares sem `MATERIAL_REF`. Não tratar automaticamente isso como corrupção.

### 4.3 Verificação cliente, hierarquia óssea e materiais vazios — 272–276

- **272:** Permite escolher `--client-exe` e valida antecipadamente o modo offline. O piloto de `b_death_knight.ASE` passou, gerando dois artefactos; binários Game iguais tinham sido verificados por SHA-256. Problema anterior incluía timeout de 180 s no readback via `cliente/Game.exe`.
- **273:** Validação de hierarquia óssea no cook, sem modificar o arranque normal. `char/tmABCD/MfhB03bt.ASE` tinha 48 bones, uma raiz, zero pais em falta e topologia válida; piloto `PASS` após timeout anterior.
- **274:** Caso especial da tabela `MATERIAL_COUNT 0` com IDs de face sem material real, **só no cook offline**. O `test-ani.ASE` (8 objetos) revelou `std::bad_array_new_length` na primeira tentativa.
- **275:** Corrige a escrita/leitura SMD que, com zero materiais, gravava um grupo vazio desalinhando os offsets; deixou de surgir o erro de alocação, mas apareceu `material_registry_empty`.
- **276:** Corrige a rejeição indevida do grupo nulo legítimo após liberar a tabela vazia. **Piloto real `Field/ani/test-ani.ASE` PASS**, duas conversões, sem escrita no cliente.

### 4.4 Timeouts Wine e conclusão — 277

- O cook completo falhou em `char/monster/MonSRazorArcher/MonSRazorArcher-model.ASE` (`bone`) por timeout de 180 segundos.
- Inspeção real: 30 objetos geométricos, 27 bones, uma raiz, sem pais em falta, topologia **VALID**. Piloto isolado **PASS** com dois artefactos. A causa específica do timeout no catálogo não ficou provada.
- **277:** Repetição única **apenas** em `subprocess.TimeoutExpired` da conversão nativa, numa pasta temporária nova. Não repete falhas de parser/hash/readback. Mantém 180 segundos por tentativa e continua `FAIL_CLOSED` depois da segunda falha.
- O utilizador instalou o 277: `PREFLIGHT_277=PASS affected=2`, `PATCH_277=PASS changed=2 git_index_untouched=1`; testes do retry **13/13 PASS** (incluem testes 259) e regressões incrementais **16/16 PASS**.
- **Resultado decisivo real depois do 277:** `MODEL_COOK_263=PASS converted=134 reused=0 total=134 complete=True no_client_writes=1`.
- **Gate final real:** `NATIVE_STAGE_267=PASS verified=134 expected=134 missing=0 complete=True no_client_writes=1`.

**Interpretação exata:** 134 artefactos SMD/SMB resultantes de ASE atuais passaram os dois loaders e a verificação de proveniência e cobertura. Não foi medido em produção que estes binários tenham a mesma aparência de todos os modelos originais. Nada foi implantado no cliente pelo cook.

## 5. PATCH 278 — comparação, canário visual e implantação reversível

### 5.1 O que faz

`deploy_native_model_stage_278.py` acrescenta quatro comandos separados:

1. **`compare`:** valida novamente o gate 267, compara cada cache do staging com o ficheiro da localização correspondente em `cliente/` usando SHA-256; classifica `SAME`, `DIFFERENT` e `MISSING`. Não escreve no cliente.
2. **`plan`:** guarda num JSON externo ao cliente as identidades do staging, os digests antigos e novos e a seleção autorizada. Suporta `--source <ASE>` (inclui todos os tipos exigidos por esse ASE) ou `--all`; o ficheiro de plano não pode já existir.
3. **`apply`:** revalida o catálogo inteiro e todos os destinos; recusa modificações desde o plano; guarda previamente as caches originais num backup externo, depois publica as novas com substituição atómica **por ficheiro**. Requer `--confirm-offline-client`; para `--all`, também `--confirm-full-deployment`. Se falhar, tenta rollback automático.
4. **`rollback`:** restaura os originais a partir do backup, ou remove os caches originalmente ausentes, exigindo que os atuais sejam o valor original ou o implantado. Recusa alterações externas e backups corrompidos. Não depende de continuar a existir o staging.

**Notas de segurança:** o PATCH 278 é só de ferramentas Python, não mexe no C++, em `SourceCode/Content/canonical`, nas DLLs, nos NPCs ou no arranque normal. O gate 267 continua obrigatório antes de criar/aplicar um plano, sem atalhos. Compara por conteúdo, não por timestamp. Rejeita symlinks e colisões de maiúsculas/minúsculas de caminhos para evitar ambiguidades Wine/Linux. Os backups e planos ficam preferencialmente em `bugfixes/`; **não apagar backups** antes de terminar a validação visual.

**Limite importante:** a publicação de 134 ficheiros não é uma única transação atómica em termos de sistema de ficheiros. Cada substituição é atómica e existe uma receção/backup recuperável caso a máquina pare a meio; executar o jogo apenas depois de `APPLIED`. Se ocorrer interrupção abrupta, usar `rollback` sobre o backup antes de qualquer novo teste. A ferramenta não controla o processo Wine; **fechar o jogo antes do apply/rollback**.

### 5.2 Primeiro passo — comparação sem escrita

```bash
cd ~/Área\ de\ Trabalho/summit
python3 SourceCode/tools/deploy_native_model_stage_278.py compare \
  --client ./cliente \
  --server ./Servidor/Server.exe \
  --client-exe ./SourceCode/dist_mingw_client_i686/Game.exe \
  --out ./bugfixes/SUMMIT_NATIVE_MODELS_INCREMENTAL_263 \
  --details
```

`SAME`: o conteúdo já coincide. `DIFFERENT`: existe outro binário. `MISSING`: não existe cache com aquele caminho no cliente. Nenhuma destas categorias implica, sozinha, um defeito visual.

### 5.3 Canário reversível de um ASE

Exemplo com `Effect/NewEffect/Res/Object/123.ASE`, cujo piloto de conversão já passou. Usar nomes distintos de plano/backup em cada ensaio e garantir que não existem ainda:

```bash
python3 SourceCode/tools/deploy_native_model_stage_278.py plan \
  --client ./cliente \
  --server ./Servidor/Server.exe \
  --client-exe ./SourceCode/dist_mingw_client_i686/Game.exe \
  --out ./bugfixes/SUMMIT_NATIVE_MODELS_INCREMENTAL_263 \
  --source Effect/NewEffect/Res/Object/123.ASE \
  --plan ./bugfixes/SUMMIT_CANARIO_278_PLAN.json
```

**Com o Game.exe fechado**, e só quando estiver preparado para um ensaio controlado:

```bash
python3 SourceCode/tools/deploy_native_model_stage_278.py apply \
  --plan ./bugfixes/SUMMIT_CANARIO_278_PLAN.json \
  --backup-dir ./bugfixes/SUMMIT_CANARIO_278_BACKUP \
  --confirm-offline-client
```

Iniciar normalmente a partir de `cliente/` com as DLLs existentes:

```bash
cd cliente
wine Game.exe
```

O modelo escolhido pode não aparecer automaticamente no mapa atual: é preciso criar um cenário de teste que realmente o mostre. Comparar geometria, UV, texturas, materiais, animação, culling e ausência de regressões. O defeito de ecrã branco e os NPCs de Ricarten **não** são critérios de aceitação automática desta ferramenta, pois são bugs ainda separados.

Para reverter o canário, **fechar o jogo primeiro**:

```bash
cd ~/Área\ de\ Trabalho/summit
python3 SourceCode/tools/deploy_native_model_stage_278.py rollback \
  --backup-dir ./bugfixes/SUMMIT_CANARIO_278_BACKUP \
  --confirm-rollback
```

O backup fica guardado. Se qualquer cache tiver sido alterada desde a implantação, o rollback recusa substituí-la silenciosamente.

### 5.4 Implantação total — apenas após canário visual aprovado

Criar um plano **novo**, com `--all`:

```bash
python3 SourceCode/tools/deploy_native_model_stage_278.py plan \
  --client ./cliente \
  --server ./Servidor/Server.exe \
  --client-exe ./SourceCode/dist_mingw_client_i686/Game.exe \
  --out ./bugfixes/SUMMIT_NATIVE_MODELS_INCREMENTAL_263 \
  --all \
  --plan ./bugfixes/SUMMIT_ALL_278_PLAN.json
```

**Depois de desligar o jogo e confirmar espaço livre para os backups:**

```bash
python3 SourceCode/tools/deploy_native_model_stage_278.py apply \
  --plan ./bugfixes/SUMMIT_ALL_278_PLAN.json \
  --backup-dir ./bugfixes/SUMMIT_ALL_278_BACKUP \
  --confirm-offline-client \
  --confirm-full-deployment
```

Rollback do conjunto total:

```bash
python3 SourceCode/tools/deploy_native_model_stage_278.py rollback \
  --backup-dir ./bugfixes/SUMMIT_ALL_278_BACKUP \
  --confirm-rollback
```

**Nunca copiar indiscriminadamente os 134 ficheiros com `cp` ou `rsync` sem backup.** Não aplicar o conjunto total enquanto a aparência e compatibilidade ainda estiverem por testar em exemplares representativos.

### 5.5 Testes e limites do PATCH 278

Os testes portáteis `python3 SourceCode/tools/test_native_model_deploy_278.py` cobrem comparação só de leitura, seleção de ASE, hash e preflight, cópia atómica, rollback, ficheiro originalmente ausente, corrupção de backup, alterações concorrentes, symlinks/casefold e falha a meio de um conjunto. Estes testes usam fixtures pequenas; **não substituem a prova real de implantação e visualização com os 134 modelos do SUMMIT**, que não estão disponíveis no ambiente de criação.

A ferramenta não mede FPS, não modifica estados Direct3D9 e não corrige a aparência dos materiais. A integração real só deve ser considerada fechada após um canário visual aprovado, implantação deliberada e capacidade de rollback ensaiada.

## 6. Estado dos bugs que ficaram para outro chat

**VIS-01 — Ecrã totalmente branco dependente da câmara:** confirmado pelo utilizador: ao apontar aproximadamente para as costas do personagem (0°), todo o ecrã fica branco, inclusive HUD/chat; a aproximadamente 180° (frente/cabeça), a imagem reaparece. Alterar o céu não resolve e ocorreu num mapa sem fog. A causa exata ainda **não** foi identificada. Outro chat investiga passes de mundo/efeitos/overlays/UI, materiais, blending/Z e render targets, sem novos logs ou flags.

**NPC-01 — NPCs desaparecidos de Ricarten:** aberto. Não foi distinguida ausência de spawn/replicação de invisibilidade visual. O outro chat precisa de confirmar interatividade, presença no servidor, dados runtime e modelos. Não atribuir ao cooker sem evidência.

**PERF — stutters/texturas/efeitos:** houve melhorias de cache DDS, áudio e HoEffect, mas o desempenho real de sessões prolongadas e a aparência final ainda precisam de testes com o jogo.

**Backlog histórico de funcionalidades:** UI V2/fontes/tooltips/comparação de equipamento; NPC shops/venda de poções por quantidade; CoinShop/TimeShop/ACKs; aging, mix, fundição e fabricação; inventário/armazém, combate, companions, quests, localização e testes prolongados DirectSound/Direct3D. Confirmar cada problema na versão atual antes de reabrir ou refatorar. Para mais pormenor sobre este trabalho separado, consultar o documento `SUMMIT_RETOMADA_BUGFIXES_E_IMPLEMENTACOES_2026-10-11.md` disponibilizado ao outro chat.

## 7. Estado de encerramento e próximos responsáveis

| Trabalho | Situação em 11/10/2026 | Próximo responsável |
|---|---|---|
| Reconstrução offline completa 134/134 ASE→SMD/SMB | **Concluída e validada** na máquina do utilizador | Fluxo de modelos |
| Gate de integridade 267 | **PASS 134/134, missing=0** | Fluxo de modelos |
| Comparação e canário/rollback 278 | Ferramenta preparada, **ainda sem teste real no cliente** | Fluxo de modelos + utilizador |
| Instalação dos 134 no cliente | **Não realizada**; só após validação visual | Utilizador, mediante plano explícito |
| Ecrã branco 0°/180° | **Aberto** | Outro chat de renderização |
| NPCs Ricarten | **Aberto** | Outro chat de gameplay/renderização |
| Qualidade visual e regressões de modelos/efeitos | **Pendente** | Utilizador, após canário |

### Evidência principal preservável no handoff

```text
PREFLIGHT_277=PASS affected=2
PATCH_277=PASS changed=2 git_index_untouched=1
Ran 13 tests ... OK
Ran 16 tests ... OK
MODEL_COOK_263=PASS converted=134 reused=0 total=134 complete=True no_client_writes=1
NATIVE_STAGE_267=PASS verified=134 expected=134 missing=0 complete=True no_client_writes=1
```

Nenhum destes resultados significa que um novo SMD/SMB tenha sido publicado ao cliente de produção. **A implantação é um passo novo, controlado e reversível.**

---

**Conclusão:** o grande bloqueio da conversão ASE nativa foi resolvido para o catálogo de 134 artefactos; o PATCH 278 proporciona uma ponte segura entre *staging verificado* e *validação visual no cliente*. A reconversão a partir do ASE continua disponível se as caches forem eliminadas. A correção do ecrã branco e dos NPCs de Ricarten segue em paralelo noutro chat, sem interferência com esta frente.
