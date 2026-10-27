# Summit Priston Tale — Guia operacional de Content, tools e assets físicos

**Versão de referência:** 2.0 — actualizada em 24 de outubro de 2026  
**Objectivo:** manter o fluxo já existente, evitar caminhos inventados e evitar retrabalho.  
**Base documental:** `contentTesteTools.zip` fornecido pelo projecto (pastas `Content/`, `tools/content_port/`, `tools/content_workbench/`), comandos e estrutura Fedora mostrados pelo responsável do Summit, o log de build/deploy `Texto colado(20261024-083012).txt` e confirmações de teste de Pillai e cópia de BI240.  
**Âmbito:** documentação do processo existente e do caso real Pillai/BI240; este documento não instala, publica nem altera o projecto. As operações descritas foram executadas pelo responsável no Fedora, não por este documento.

**Actualização v2:** incorporados o publish dos 18 JSON, a correção de `FALLITEM_MAX`, a compilação/distribuição efectivas dos packs, os spawns do mapa 57 com aparição confirmada em jogo e a cópia validada do ícone BI240.

## 1. Regra fundamental

O Summit tem **duas naturezas de conteúdo**, que não se confundem:

1. **Definições e regras textuais:** fonte autoritativa em **`SourceCode/Content/canonical/`**, nos ficheiros JSON de cada família. Os scripts existentes compilam os packs e distribuem-nos para as localizações de execução.
2. **Assets físicos (ficheiros binários, imagens, animações, sons e efeitos):** são colocados nas **pastas respectivas de `cliente/` e `Servidor/`**, conforme o consumidor. Não é necessário inventar uma nova pasta `SourceCode/Content/assets`, um manifesto de distribuição novo ou outra ferramenta de deploy para os gerir.

A **presença de uma definição de monstro no Content não faz o monstro aparecer no mapa**. A associação a um campo está no JSON canónico `stage_spawns/`, compilado para `field_runtime.sfld`. A configuração de `stage_spawns` e a definição do monstro são duas peças necessárias e distintas.

O código-fonte do motor é uma terceira dimensão: `SourceCode/Shared/`, `SourceCode/SrcGame/`, `SourceCode/SrcServer/`. Alterações em C++ exigem compilação do executável; compilar Content não é equivalente a compilar C++.

**Princípio do responsável do projecto:** usar as estruturas e ferramentas existentes; mover os ficheiros para os destinos corretos; depois executar os processos de compilação já estabelecidos.

## 2. Estrutura real

Na raiz `~/Área de Trabalho/summit/` existem `SourceCode/`, `cliente/`, `Servidor/`, `compCannonContent.sh` e documentação/scripts auxiliares.

| Conteúdo | Origem / localização correcta | Nota |
|---|---|---|
| Definições de monstros e NPCs | `SourceCode/Content/canonical/text/resources/Servidor/GameServer/Monster/` e `.../NPC/` | JSON canónico; não editar directamente packs binários. |
| Nomes, traduções e ligações | `SourceCode/Content/canonical/text/` | Inclui recursos, índices e localisation. |
| Metadados/contratos de modelos | `SourceCode/Content/canonical/model_configs/` | Apontam para os ficheiros físicos; não são os próprios modelos. |
| Itens e perfis de execução | `SourceCode/Content/canonical/text/` e `.../item_profiles/` | OpenItem e resolução dos perfis. |
| Mapas e entidades de campo | `SourceCode/Content/canonical/maps/`, `stage_spawns/`, `stage_start_points/`, `fixed_npcs/` | Os monstros só aparecem quando têm referências nos spawns. Editar o JSON canónico e recompilar. |
| Packs compilados | `SourceCode/Content/packs/` | Gerados; não são a fonte de verdade. |
| Packs em execução | `cliente/Content/packs/` e `Servidor/Content/packs/` | Destinos geridos pelo distribuidor já existente. |
| Modelos de monstros | `cliente/char/monster/` e, quando existentes/necessários, `Servidor/char/monster/` | `.inx`, `.smd`, `.smb` etc.; não substituir exemplares protegidos. |
| Sons | `cliente/wav/` | Manter estrutura e maiúsculas/minúsculas exactas. |
| Partículas e texturas dos efeitos | `cliente/Effect/Particle/` | `.part` em `Script/`; texturas em `Particle/`. |
| Código de cliente e servidor | `SourceCode/Shared/`, `SrcGame/`, `SrcServer/` | Compilação C++ separada da compilação de dados canónicos. |

### O que dizem os ficheiros de referência

- `Content/CANONICAL_SOURCE.json`: `source_of_truth = SourceCode/Content/canonical`; não há dependência runtime de um canonical externo.
- `Content/BUILD.json`: documenta as famílias de entrada e os packs compilados com tamanhos/SHA-256. Este ficheiro é produto do build: não o alterar manualmente para tentar publicar um asset.
- `tools/content_port/consolidate_canonical_content.py`: valida as fontes, compila packs e, por omissão, chama a distribuição e a validação. Opção `--no-deploy` disponível para builds sem distribuição.
- `tools/content_port/build_from_canonical.py`: compila e valida os packs, incluindo definições client/server, localização, itens, gameplay, campos e mapas.
- `tools/content_port/compile_field_runtime.py`: agrega os JSON canónicos de `stage_spawns`, `stage_start_points` e `fixed_npcs` no pack `field_runtime.sfld`; o runtime usa esse pack, não o antigo `.spm` físico.
- `tools/content_port/validate_monster_runtime_limits.py`: verifica as entradas de drops realmente materializadas contra o `FALLITEM_MAX` do motor, incluindo a consistência entre `semantic.fields.fall_item`, `semantic.directives` e `raw_values_hex`.
- `tools/content_port/deploy_runtime_content.py`: distribui **packs compilados** para `cliente/Content/packs/` e `Servidor/Content/packs/`, com verificação de SHA. Isto não deve ser confundido com os modelos/sons/texturas físicos.
- `tools/content_port/validate_runtime_pack_resolution.py`: confere resolução dos packs relativamente aos executáveis e, havendo cliente/servidor presentes, compara hashes fonte/destino.
- `tools/content_port/README.md`: explicita que `SMD`, `SMB`, `INX` e `ASE` ficam fora do compilador textual e são referidos como caminhos opacos.

## 3. Comandos de compilação existentes

### Content canónico

O script presente na raiz, confirmado directamente pelo responsável, é:

```bash
cd "$HOME/Área de Trabalho/summit" && \
python3 SourceCode/tools/content_port/consolidate_canonical_content.py \
  --repo-root "$PWD" && \
python3 SourceCode/tools/content_port/validate_runtime_pack_resolution.py \
  --repo-root "$PWD" && \
echo "CONTENT_CANONICO_COMPILADO_E_DISTRIBUIDO_COM_SUCESSO"
```

Na prática utiliza-se `bash compCannonContent.sh`, **após** as alterações JSON desejadas já estarem no canonical. O comando compila e faz deploy de conteúdo; não é uma operação de leitura.

Para um build isolado de packs, o compilador suporta `--no-deploy`, mas isso **continua a escrever os packs e manifestos no Content da raiz indicada**; não executar no checkout principal se se pretender não alterar nada.

### Código C++

O projecto dispõe de `SourceCode/compboth.sh`, dos builds MinGW i686 e dos scripts de link. O responsável utiliza `compboth` para compilar código do cliente/servidor. **Não confundir com compCannonContent**, que trabalha o conteúdo canónico.

Os detalhes exactos dos efeitos de `compboth.sh` e da distribuição dos executáveis devem ser lidos no script da **instalação corrente** antes da sua execução; o ZIP `contentTesteTools.zip` só trouxe `Content/`, `tools/` e `tests/`, não os scripts principais de C++.

### Sequência operacional

1. Confirmar o que é uma **definição JSON** e o que é um **asset físico**.
2. Colocar/actualizar os JSON aprovados **nos caminhos canónicos existentes**, sem apagar variantes anteriores nem sobrescrever alterações posteriores inadvertidamente.
3. Colocar os ficheiros físicos novos **nos caminhos correspondentes do cliente/servidor**, sem substituir originais com hashes diferentes.
4. Se houver alterações de C++, compilá-las pelo processo de build do projecto (`compboth`), verificando previamente o destino do executável.
5. Compilar/distribuir os JSON com `compCannonContent.sh`, quando se pretender efectivamente aplicar o conteúdo.
6. Confirmar se as entidades novas têm **spawns canónicos** no campo pretendido; alterar os pesos do campo só quando aprovado e preservar os actores existentes. Compilar/distribuir novamente o Content se a configuração de campo mudar.
7. Testar o comportamento no jogo e guardar o registo de validação. Um `PASS` do build não prova que um monstro surge no mapa ou que reproduz partículas/áudio.

A ordem dos dois comandos de build deve respeitar a prática do projecto e as dependências da alteração concreta. **Mover ficheiros para o cliente e executar compCannonContent não aplica automaticamente um patch C++ que apenas esteja num overlay externo.**

## 4. `tools`: para que servem

| Ferramenta existente | Papel |
|---|---|
| `tools/content_port/consolidate_canonical_content.py` | Orquestra compilação, validações e distribuição dos packs. |
| `tools/content_port/build_from_canonical.py` | Compila os formatos runtime a partir de JSON. |
| `tools/content_port/deploy_runtime_content.py` | Coloca os packs compilados nos destinos de cliente/servidor. |
| `tools/content_port/validate_runtime_pack_resolution.py` | Confere a resolução e paridade dos packs instalados. |
| `tools/content_port/validate_monster_runtime_limits.py` | Impede publicar monstros cuja tabela de drops exceda a capacidade do runtime (`FALLITEM_MAX`); valida campos/directivas/bytes CP949. |
| `tools/content_port/compile_field_runtime.py` | Materializa spawns, start points e NPCs fixos em `field_runtime.sfld`. |
| `tools/content_port/validate_legacy_localization_bridge.py` | Validador da ponte de localização legada, relevante quando o stage indica gate pendente. |
| `tools/content_workbench/summit_content_workbench.py` | Inventaria evidência de `SourceCode`, cliente, servidor e donors. |
| `tools/content_workbench/summit_content_workbench_v2.py` | Catálogo comparável (entidades, versões, órfãos, relações); **não publica** alterações automaticamente. |

**Separação importante do README do Workbench:** `DONOR = evidência somente leitura`; `CANONICAL = estado seleccionado do Summit`; `OVERRIDE = alteração deliberada`. Não copiar um donor inteiro por cima do cliente/servidor.

### Modelos e ícones

- Um contrato JSON de modelo pode apontar para um `.ini`/`.inx`; esse contrato não substitui o asset nativo, e não se deve inventar um `.ini` ausente.
- Para ícones de itens, o README descreve uma identidade canónica opcional `assets.icon.logical_path` **sem extensão** e runtime `dds`; é exigido *cooked-asset gate* antes de desactivar fallback. Uma imagem PNG/BMP encontrada num donor não é, por si só, prova de identidade ou de DDS correcto.

## 5. Caso real Pillai — ciclo concluído de assets, JSON, spawns e Content

### 5.1. Linha do tempo comprovada

| Fase | Acção efectivamente executada / verificada | Resultado |
|---|---|---|
| Originais | RGuard, DMystery e WLord usaram modelos já existentes no cliente/servidor; sem substituir INX protegidos. | **Mantidos** |
| Ficheiros físicos | Copiados do overlay `PILLAI_PREVIEW_OVERLAY/cliente_overlay/` para `cliente/`, sem sobrescrever ficheiros existentes. | **37/37 SHA-256 PASS** |
| Content canónico | Publicada com `pt_asset_tool canonical publish-source` a transacção `20261020T194414Z-daf148ca`. | **18/18 JSON PASS**; fase `SOURCE_APPLIED_PACKS_PENDING` antes do build |
| Índices após publish | Índice de texto e catálogo de modelos. | **5 624 documentos** e **366 modelos** |
| Primeiro build Content | `compCannonContent.sh` bloqueado pelo validador de limites. | **FAIL** em `monster.123_wlord`: 217 entradas de drops para limite 200 |
| Drops da Bruxa Mestre | Reduzidas repetições sem peso útil numa única linha da definição canónica. | **217 → 199**, `FALLITEM_MAX=200`, **PASS** |
| Segundo build Content | `compCannonContent.sh` concluiu compilação, inspecção e distribuição. | **PASS**; packs no cliente e no servidor, paridade de hashes verificada |
| Diagnóstico de aparição | A definição estava no Content, mas não existiam spawns para os três monstros. | **0 spawns** para os três novos, antes da integração do campo |
| Mapa de Pillai | Introduzidos os três monstros nos `actors` do mapa **57 — Pillai em Ruínas**, preservando os quatro anteriores; recompilado Content. | **O responsável confirmou que os monstros já aparecem em jogo.** |
| Som, partículas e código | Alterações C++ estavam no overlay de preview; compilar Content não as integra no `Game.exe`. | **Ainda sem confirmação explícita de som/partículas novos em jogo** |

Não voltar a executar `publish-source` da transacção já aplicada. Os dados actuais têm precedência sobre o stage antigo, sobretudo porque o WLord foi corrigido e o spawn alterado **depois** desse stage.

### 5.2. Ficheiros físicos Pillai — o que realmente foi instalado

- 26 ficheiros WAV sob `cliente/wav/Effects/monster/`.
- 8 ficheiros `.part` sob `cliente/Effect/Particle/Script/`.
- 3 texturas sob `cliente/Effect/Particle/`.
- Total: **37/37 com hash verificado no destino**, conforme a saída partilhada pelo responsável.
- Não houve novo lote físico para o servidor neste conjunto; modelos já existentes e protegidos não foram substituídos.

Origem da cópia no Fedora:

```text
~/summit_auditorias/assets_eventos_20261024/PILLAI_PREVIEW_OVERLAY/cliente_overlay/
```

Manifesto de verificação da cópia:

```text
~/Transferências/SUMMIT_PILLAI_INTEGRACAO_PREVIEW_20261024/PILLAI_37_MANIFEST.json
```

**Lição:** não declarar instalado o que esteja apenas em overlay; exigir hash do ficheiro no destino. Um ZIP de `Content` e `tools` não prova a presença ou ausência de pastas reais do PC.

### 5.3. Publicação dos 18 JSON canónicos já efectuada

Operação utilizada no Fedora, pela ferramenta existente:

```text
pt_asset_tool canonical publish-source
manifest: pt_build/source_content_stage/20261020T194414Z-daf148ca/manifest.json
source_root: ~/Área de Trabalho/summit/SourceCode
```

O resultado apresentou `source_phase=APPLIED`, `pack_phase=PENDING`, `deploy_phase=PENDING` na altura, até se executar a compilação canónica. Os 18 hashes foram conferidos depois da aplicação; o JSON protegido `129_Bruxa da Noite.inf.json` conservou o mesmo hash. A Bruxa Mestre (`123_WLord`) mantém a identidade de som `PILLAI_WLORD`; a Bruxa da Noite permanece `WLORD`.

**Lição:** `publish-source` introduz definições no canonical; só `compCannonContent.sh` gera e distribui os packs. O stage canónico antigo não substitui modificações subsequentes feitas e validadas no canonical.

### 5.4. Bloqueio real: limite de drops do WLord

O primeiro `compCannonContent.sh` passou a validação de JSON e esquemas, mas parou em:

```text
MONSTER_DEFINITIONS=393
FALLITEM_MAX=200
MAX_MATERIALIZED_DROP_ENTRIES=217
ERROR=monster.123_wlord: FALLITEM_COUNT=217 > FALLITEM_MAX=200
RESULT=FAIL
```

A causa estava na tabela de drops importada do donor. A correcção aprovada pelo responsável alterou **somente** o JSON canónico da Bruxa Mestre:

```text
SourceCode/Content/canonical/text/resources/Servidor/GameServer/Monster/123_WLord.inf.json
```

Numa linha com peso `1` havia 24 ocorrências de `ec105`, `ec106`, `ec107`; foram mantidas 2 ocorrências de cada, ficando 6 em vez de 24. A remoção de 18 entradas levou o total materializado de 217 a **199**. `semantic.fields.fall_item`, `semantic.directives` e `raw_values_hex` ficaram sincronizados, conforme requerido pelo validador. O código do motor, `FALLITEM_MAX` e a Bruxa da Noite **não foram alterados**. A proveniência legada foi preservada; uma cópia anterior do JSON foi guardada em `~/summit_auditorias/assets_eventos_20261024/123_WLord.inf.json.antes_drops.bak` pelo procedimento usado.

O build subsequente confirmou:

```text
FALLITEM_MAX=200
MAX_MATERIALIZED_DROP_ENTRIES=199
MAP53_HP_WARNINGS=0
RESULT=PASS
```

**Lição:** o compilador aplica limites reais do jogo. Não aumentar capacidades do runtime ou desligar validadores por causa de conteúdo donor incompatível. Em vez disso, corrigir exclusivamente o JSON canónico do monstro afectado, sempre validando as representações semânticas e os bytes de directiva.

### 5.5. Porque não apareciam — spawns do mapa 57

A consulta **antes da correcção** mostrou:

```text
pilai.ASE.spm.json:          actors=[]; bosses=[]
ruined-pilai.ASE.spm.json:  Inferno(25), Ignis(25), Najan(25), Crios(25)
RGuard / DMystery / WLord:  0 entradas de spawn
```

A origem canónica do mapa 57 é:

```text
SourceCode/Content/canonical/stage_spawns/spawns/GameServer/Field/ruined-pilai.ASE.spm.json
```

Fica distinta do mapa 21, `pilai.ASE.spm.json` (vila; sem actores). O mapa 57 é Pillai em Ruínas, com combate. O ficheiro usa `summit.stage-spawn/v1`, `actors` com `monster_id`, `legacy_name`, `legacy_name_bytes_hex`, `weight`, `resolution`, e `bosses`. Os seus restantes campos (`limit_max`, `delay`, `open_limit`, `legacy_source`) devem ser preservados.

A alteração proposta e seguida neste fluxo foi acrescentar três IDs sem remover os quatro actores originais, com **peso total 100**:

| Monstro no mapa 57 | Peso na distribuição proposta |
|---|---:|
| `monster.117_inferno` | 16 |
| `monster.119_ignis` | 16 |
| `monster.120_najan` | 16 |
| `monster.121_crios` | 16 |
| `monster.117_rguard` | 12 |
| `monster.118_dmystery` | 12 |
| `monster.123_wlord` | 12 |
| **Total** | **100** |

Os nomes legados e respectivos bytes CP949 devem vir das definições JSON existentes, nunca ser inventados. Após editar o spawn canónico, o procedimento do projecto volta a compilar e distribuir `field_runtime.sfld` por `compCannonContent.sh`; não basta alterar um `.spm` legado solto.

O responsável confirmou posteriormente: **«já aparecem os monstros»**. Isto confirma a **aparição no jogo**, não representa uma certificação ainda não realizada de ataques, partículas ou áudio. A tabela acima descreve a configuração da operação proposta; se os pesos forem alterados no futuro durante o balanceamento, consultar sempre o JSON actual da máquina em vez deste registo histórico.

**Lição central:** **monstro definido + modelo instalado ≠ spawn**. Para uma entidade surgir, é preciso também criar/associar o registo de spawn no mapa correcto, recompilar Content e permitir que o servidor carregue o novo pack.

### 5.6. Evidência da compilação e distribuição concluídas

O log real de `sh compCannonContent.sh` partilhado em 24/10/2026 confirma:

- `validate_canonical_text.py`: **5 624 recursos** — PASS.
- `validate_canonical_schemas.py`: **5 624 textos**, **61 mapas de spawn** e **24 mapas de NPCs fixos** — PASS.
- `validate_monster_runtime_limits.py`: **199/200** no máximo — PASS.
- `compile_text_resources.py`: `text_resources.sres`, compilado e inspeccionado — PASS.
- `compile_definitions.py`: `definitions_client.scfg` e `definitions_server.scfg` — PASS.
- `compile_field_runtime.py`: `field_runtime.sfld`, com secções `SSPB`, `SPPT` e `SPCN` — PASS.
- `deploy_runtime_content.py`: **7 packs para o cliente e 8 para o servidor** — PASS.
- `validate_runtime_pack_resolution.py`: hashes coincidentes entre fontes e destinos verificados — PASS.
- Resultado final do comando: **`CONTENT_CANONICO_COMPILADO_E_DISTRIBUIDO_COM_SUCESSO`**.

O log deste ponto confirma o build que resolveu os drops; a confirmação posterior de aparição no mapa 57 veio do teste em jogo após a alteração de spawn e nova compilação descritas pelo responsável. Não é prova de que alterações C++ do overlay tenham sido integradas.

### 5.7. O que falta testar em Pillai

- **Confirmado:** os três monstros já aparecem no mapa 57.
- **Por verificar explicitamente:** comportamento completo, animações, sons específicos, partículas/efeitos das skills e morte, bem como eventual necessidade de integrar o preview C++ no SourceCode efectivo.
- **Preservar sempre:** Bruxa da Noite (`WLORD`) e os quatro monstros originais do mapa 57; não substituir INX nem alterar gameplay protegido sem necessidade.

## 5A. Ícone BI240 — primeiro caminho físico recuperado

A definição de item `BI240` já existia no Content; o catálogo do cliente usa o destino da família **Premium**. O donor disponibilizava um `itBI240.bmp` em `image/Sinimage/Items/Event/`; **o caminho de destino não tem de ser igual ao caminho do donor** quando o código do item fornece uma referência mais precisa.

```text
Origem:  donor .../image/Sinimage/Items/Event/itBI240.bmp
Destino: cliente/image/Sinimage/Items/Premium/itBI240.bmp
```

O responsável executou a cópia sem sobrescrever outro conteúdo e comunicou:

```text
BI240: COPIADO
BI240: HASH_PASS
```

**Estado:** caminho físico recuperado com integridade verificada. A imagem abriu no visualizador, mas a aparência dentro do jogo e a identidade visual final do ícone ainda não foram certificadas explicitamente. Não foi necessário criar outro JSON de item nem mexer no C++.

No inventário inicial havia **87 caminhos de ícone ausentes**, incluindo BI240. Depois da cópia física validada, restam **86 caminhos desse grupo por resolver**, sem contar os outros casos já classificados separadamente (19 variantes de maiúsculas/minúsculas, 58 caminhos presentes por testar, 1 incerto e TW106 deliberadamente suprimido).

## 6. Checklist para próximas recuperações (401 alvos + Eragon)

- [ ] Identificar o alvo e a família (monstro, NPC, item, evento ou referência documental).
- [ ] Confirmar identidade por provas de nome, definição, modelo/caminho e, se necessário, bytes/hash.
- [ ] Verificar se o ficheiro já existe no cliente/servidor, inclusive diferenças de capitalização.
- [ ] Distinguir ausência física, identidade desconhecida e conteúdo já existente mas não testado.
- [ ] Modificar **apenas** o JSON canónico aprovado quando a mudança é textual; mover **apenas** os assets físicos relevantes para as pastas respectivas.
- [ ] Para monstros, confirmar **definição + contrato/modelo + referência em `stage_spawns`** no mapa correcto.
- [ ] Confirmar limites de drops, codificação CP949, pesos totais e compatibilidade com o validador antes de distribuir.
- [ ] Preservar originais protegidos, não alterar gameplay nem substituir assets só porque uma versão donor tem um nome semelhante.
- [ ] Usar os scripts de build/deploy **existentes**; não criar outro sistema.
- [ ] Verificar SHA/caminhos, compilação dos packs, hashes fonte/destino, aparição no campo e efeitos em jogo **separadamente** antes de marcar cada parte como resolvida.

## 7. Erros de processo que não devemos repetir

1. Não propor `SourceCode/Content/assets/` como nova arquitectura sem o projecto a requerer.
2. Não concluir que os ficheiros faltam no PC por não aparecerem num ZIP limitado a `Content/` e `tools/`.
3. Não pedir novo scan do que já foi provado por hashes.
4. Não declarar que uma proposta no overlay está aplicada ao `Game.exe`.
5. Não executar `compCannonContent.sh` acreditando que é um teste sem escrita: por omissão compila **e distribui**.
6. Não confundir um donor com a fonte canónica nem copiar INX de donor por cima dos originais.
7. Não tocar na Bruxa da Noite para corrigir a Bruxa Mestre.
8. Não inventar ferramentas, pastas ou scripts de publicação quando os existentes já cobrem o processo.
9. Não declarar um monstro funcional no mapa só porque a sua definição JSON passou: verificar `stage_spawns` e `field_runtime.sfld`.
10. Não ignorar `FALLITEM_MAX` quando se importa um donor; o limite em execução é verificado após materialização efectiva dos itens do perfil do servidor.
11. Não recolocar um stage antigo depois de corrigir drops ou spawns no canonical: isso poderia desfazer as correcções posteriores.
12. Não supor que `compboth` é necessário para cada alteração de spawn/Content; só compilar C++ quando há alterações de código, e verificar o script e os seus destinos.
13. Não confundir a exibição da imagem BMP no visualizador com a renderização comprovada da imagem no jogo.
14. Não confundir a aparição do monstro no jogo com a confirmação dos novos sons e efeitos.

---

**Limites da documentação:** o ZIP estudado não contém `cliente/`, `Servidor/` nem os scripts MinGW principais; a existência desses caminhos e comandos no Fedora foi confirmada pelos `ls` e `cat` partilhados. As compilações, cópias, validações e aparição em jogo foram comunicadas pelo responsável e, quando indicado, comprovadas pelo log fornecido. Este guia não afirma ter executado operações no seu computador nem certifica áudio/partículas ainda não testados.

## 8. Registo de alterações desta documentação

- **v1 — 24/10/2026:** estrutura existente de Content/Tools; separação entre JSON e assets físicos; política de não criar novos pipelines.
- **v2 — 24/10/2026:** estado Pillai realmente publicado; drops do WLord reduzidos para 199; evidência do build e do deploy; erro e correcção de `stage_spawns` do mapa 57; monstros visíveis em jogo; BI240 copiado e verificado; tarefas ainda pendentes discriminadas.

