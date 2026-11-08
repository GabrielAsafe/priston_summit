# Priston Tale / Summit — Modelos jogáveis e organização de assets

**Guia técnico de nomenclatura, aparências e migração de diretórios**  
**Edição:** 1.0 · **Data:** 28 de outubro de 2026  
**Âmbito:** cliente V2, `Content/canonical/playable_classes/` e assets físicos de `cli/char/`.

> **Estado do documento.** Este é um plano técnico, não um registo de migração executada. As conclusões sobre o Content foram verificadas na source `src(20261027-205413).zip`; a existência dos ficheiros-base novos foi informada pelo inventário local apresentado pelo responsável do projeto. Não foram abertos os modelos binários `.inx`, `.smd` ou `.smb` da instalação.

## 1. Resumo executivo

O projeto já suporta Assassina (ContentId **11**) e Chamã / Shaman (ContentId **12**) no motor V2. O problema visual identificado não é a ausência das classes, mas a **seleção do corpo inicial** no Content: as duas definições canónicas indicavam `ce001.inx` e `cme001.inx`, que, no projeto, são trajes genéricos adquiridos na loja. Os corpos-base indicados para uso inicial são `e001.inx` (Assassina) e `me001.inx` (Chamã).

A correção deve distinguir **corpo-base**, **cabeça/cabelo**, **armadura de progressão**, **traje cosmético de loja**, **malha 3D**, **animação** e **textura**. Substituir o corpo inicial não implica apagar o traje cosmético, alterar automaticamente as cabeças ou renomear os ficheiros internos.

**Orientação:** corrigir e validar os caminhos no Content primeiro; depois inventariar dependências e migrar `char/tmABCD/` por fases, mantendo compatibilidade com caminhos gravados em personagens existentes. Não mover os assets todos de uma vez.

## 2. Siglas e nomenclatura histórica

### 2.1. Códigos de tribo, género, peça e classe

A comunidade técnica documenta convenções para os **nomes das malhas e dos modelos**, mas elas não substituem a leitura dos ficheiros individuais. A caixa `tmABCD` designa historicamente o conjunto de assets jogáveis dos Tempskron e Morion; **não** significa que só existam quatro classes.

| Código ou campo | Significado | Observações |
|:--|:--|:--|
| `T` | Tempskron | Prefixo de nomes de algumas malhas. |
| `M` | Morion / Moryon | Prefixo de nomes de algumas malhas. |
| `TM` | Tempskron + Morion | Designação convencional da pasta `tmABCD`. |
| `m` / `f` | Masculino / feminino | Posição contextual dentro do nome da malha. |
| `b` / `h` | Body / head (corpo / cabeça-cabelo) | Convenção em nomes como `TfbE01` e `Mmh-E01`. |
| `A`–`E` | Identificador de classe dentro da tribo | `E` é a quinta classe de cada tribo: Assassina / Chamã. |
| `F` | Guerreira / Martial Artist | Presente em variantes que incluem uma classe adicional. |
| `01`, `02`, `03`... | Índice de variante | O significado exato depende da família: corpo, armadura, cabelo ou traje. |
| `C` inicial em certos ficheiros | Costume / traje | Confirmado em padrões longos, p. ex. `CTFBD36`. **Não** classificar automaticamente qualquer nome iniciado por `c` como traje. |

**Exemplo de malha:** `TfbE01` = `T` (Tempskron) + `f` (feminino) + `b` (corpo) + `E` (Assassina) + `01` (variante).  
**Outro exemplo:** `MmbE01` = `M` (Morion) + `m` (masculino) + `b` (corpo) + `E` (Chamã) + `01` (variante).  
**Exemplo de traje:** `CTFBD36` = `C` (costume) + `T` (Tempskron) + `F` (feminino) + `B` (corpo) + `D` (Arqueira) + `36` (índice do traje).

### 2.2. Prefixos por classe

| Tribo | Classe | Identificador de família | Corpo-base no Content de referência |
|:--|:--|:--|:--|
| Tempskron | Mecânico | `A` | `a001.ini` |
| Tempskron | Lutador | `B` | `b001.ini` |
| Tempskron | Pikeman | `C` | `c001.ini` |
| Tempskron | Arqueira | `D` | `d001.ini` |
| Tempskron | **Assassina** | **`E`** | **`e001.inx` — base indicada para o V2** |
| Morion | Cavaleiro | `mA` | `ma001.ini` |
| Morion | Atalanta | `mB` | `mb001.ini` |
| Morion | Sacerdotisa | `mC` | `mc001.ini` |
| Morion | Mago | `mD` | `md001.ini` |
| Morion | **Chamã** | **`mE`** | **`me001.inx` — base indicada para o V2** |

Nas oito classes clássicas, a coluna de corpo indica o primeiro ficheiro listado na criação da source inspecionada. As classes novas usam `.inx` nesta integração; **não** é necessário converter para `.ini` só por consistência visual. Um tópico do RaGEZONE confirma independentemente `e001.ini` como corpo de uma implementação de Assassina — formato textual equivalente à família, mas não prova que esse `.ini` esteja presente na instalação V2.

### 2.3. Extensões relevantes

| Extensão | Papel típico | Cuidado na migração |
|:--|:--|:--|
| `.ini` | Configuração textual do modelo / malhas e animações | Pode referir outros nomes de ficheiros; não é textura. |
| `.inx` | Configuração binária/compilada do modelo | Não assumir que todos os `.inx` são caches dispensáveis. |
| `.inf` | Definição textual em certos modelos de cabeça e outros subsistemas | A extensão é reutilizada noutros contextos do jogo. |
| `.smd` | Malha / dados geométricos do modelo 3D | Pode conter materiais e referências a texturas. |
| `.smb` | Ossos, coordenadas e informação de animação | Não confundir com o ficheiro de configuração `.inx`. |
| `.ase` | Representação textual de intercâmbio do modelo | Usada em fluxos de conversão/edição. |
| `.bmp` / `.tga` | Texturas / materiais / imagens | A aparência pode mudar sem trocar toda a malha. |

Há **partilha de malhas e de texturas** entre variantes de armadura, pelo que não existe necessariamente um `.smd` exclusivo para cada item da loja ou nível de armadura.

## 3. Inventário relevante: Assassina e Chamã

### 3.1. Ficheiros indicados como disponíveis no cliente

| Categoria | Assassina | Chamã |
|:--|:--|:--|
| Corpo-base | `e001.inx` | `me001.inx` |
| Geometria principal | `TfbE01.smd` | `MmbE01.smd` |
| Dados de animação/ossos | `TfbE01.smb` | `MmbE01.smb` |
| Definição textual de cabeça | `Tfh-E01.inf` | `Mmh-E01.inf` |
| Definição binária de cabeça | `tfh-e01.inx` | `Mmh-E01.inx` |
| Movimento configurado no Content | `M6Bip.inx` | `M7Bip.inx` |

Os nomes da geometria e dos ficheiros de cabeça foram comunicados como existentes no cliente. Esta lista não prova, por si só, que `e001.inx` referencie especificamente `TfbE01.smd`, nem que `me001.inx` referencie especificamente `MmbE01.smd`: essa relação só pode ser validada pela configuração/binário ou pela carga observada no V2.

### 3.2. Referências incorretas e propostas de base

| Campo | Antes (source analisada) | Destino para corpo-base |
|:--|:--|:--|
| Assassina — `creation.bodyModels[0]` | `char/tmABCD/ce001.inx` | `char/tmABCD/e001.inx` |
| Assassina — `armor.normalModels[0]` | `ce001.inx` | `e001.inx` |
| Chamã — `creation.bodyModels[0]` | `char/tmABCD/cme001.inx` | `char/tmABCD/me001.inx` |
| Chamã — `armor.normalModels[0]` | `cme001.inx` | `me001.inx` |

**Não apagar** `ce001.inx` ou `cme001.inx`: segundo a identificação funcional feita no projeto, são trajes genéricos da loja. Devem continuar acessíveis no circuito de cosméticos/commerce, mas não ser usados como roupa inicial nem como única opção normal da classe.

**Atenção às armaduras:** `armor.normalModels` é um conjunto indexado que o serviço V2 usa para escolher modelos por índice. O facto de agora conter apenas o modelo-base não constitui ainda um catálogo completo de armaduras. As armaduras equipáveis, skins e trajes cosméticos devem manter mapeamentos próprios, validados contra os índices e as regras da loja/equipamento.

### 3.3. Cabeças iniciais e variantes de cabelo

A source inspecionada apresenta `hassassin_02.inx` / `hassassin_03.inx` (Assassina) e `hshaman_02.inx` / `hshaman_03.inx` (Chamã) em três locais: `creation.faces`, `appearance.headModels` e `appearance.defaultHeadModels`.

Os ficheiros `tfh-e01.inx` e `Mmh-E01.inx` **existem segundo o inventário local**, mas não foram abertos nem foi confirmado que representem a escolha visual padrão pretendida. Caso a cabeça `01` seja a variante inicial, deve ser incluída/coordenada nos três campos, com as miniaturas corretas. Não substituir `_02` e `_03` às cegas: podem ser variantes de cabelo legítimas.

**Miniaturas atuais:** `TfE02-a/b.tga`, `TfE03-a/b.tga`, `Moryon/MmE02-a/b.tga` e `Moryon/MmE03-a/b.tga`. Uma nova opção `_01` necessita de pré-visualização compatível, se a interface a apresentar.

## 4. Onde o Content V2 participa no carregamento

As definições canónicas pertinentes são:

```
SourceCode/Content/canonical/playable_classes/
├── assassina.json    # ContentId 11; classe V2 ativa
├── shaman.json       # ContentId 12; classe V2 ativa
├── assassin.json     # outro registo, desativado na source analisada
└── index.json
```

**Não confundir `assassin.json` com `assassina.json`.** Na source consultada, o primeiro tem `id=9` e `enabled=false`; o segundo tem `id=11` e `enabled=true`.

| Campo | Função no V2 | O que rever na migração |
|:--|:--|:--|
| `creation.bodyModels` | Corpo de criação/seleção | Referência completa ao modelo-base. |
| `creation.faces[].model` | Cabeça escolhida na criação | Rever catálogo de variantes e pré-visualizações. |
| `appearance.bodyModelRoots` | Raízes permitidas dos corpos | Atualizar se a classe mudar de diretório. |
| `appearance.headModels` | Matriz de variantes de cabeça | Manter coerência com os ficheiros físicos. |
| `appearance.defaultHeadModels` | Cabeças padrão / fallback | Deve apontar para a mesma família da classe. |
| `armor.modelRoot` | Diretório-base dos modelos de armadura | Atualizar em conjunto com os nomes relativos. |
| `armor.normalModels` | Variantes normais indexadas | Separar o corpo-base dos trajes cosméticos. |
| `armor.comicModels`, `newSetModels`, `newSet2Models` | Famílias visuais alternativas | Confirmar fallback e índices. |
| `animation.motionModelFile` | Referência a modelo/configuração de movimento | Verificar `M6Bip.inx` e `M7Bip.inx` após migração. |

**Evidência de código:** `ClassService::BuildV2CreationTemplateR80()` copia `creation.bodyModels[index]` para o corpo inicial; `ClassService::TryContentArmorModelPath()` constrói o caminho juntando `armor.modelRoot` com `armor.normalModels[index]` (ou variante); e `NativeZeroJobModelGuardR98.h` valida pares corpo/cabeça permitidos pelas definições canónicas. O pack gerado utilizado pelo runtime é `Content/packs/playable_classes.scls`.

### 4.1. Fluxo de resolução

```
Content/canonical/playable_classes/*.json
        │ compilação/publicação
        ▼
Content/packs/playable_classes.scls
        │ leitura pelo serviço V2
        ├── criação  → creation.bodyModels + creation.faces
        ├── aparência → appearance.headModels/defaultHeadModels
        ├── armadura → armor.modelRoot + normalModels/variantes
        └── movimento → animation.motionModelFile
                            │
                            ▼
                  cli/char/... (ficheiros reais)
```

Alterações apenas aos JSON não modificam automaticamente os packs instalados. Publicar no cliente e servidor conforme o pipeline da versão em uso; testes de source e teste visual in-game continuam necessários.

## 5. Proposta de organização de `cli/char/`

### 5.1. Objetivos e princípios

Manter monstros e NPCs onde estão; separar cada classe jogável; distinguir `base`, `armor`, `costume` e recursos partilhados no **catálogo/manifesto**, sem partir imediatamente o carregamento legado. Nunca classificar só pela primeira letra: analisar dependências entre `.ini`/`.inx`, `.smd`, `.smb` e texturas. Não renomear ficheiros físicos sem necessidade.

### 5.2. Estrutura recomendada — fase compatível com o legado

Esta estrutura privilegia **nomes de diretórios curtos**, porque a source ainda contém uma validação que trunca os campos de corpo e cabeça aos 31 primeiros caracteres.

```
cli/char/
├── Flag/
├── monster/                     # manter
├── npc/                         # manter
├── pc/                          # personagens jogáveis
│   ├── mechanician/
│   ├── fighter/
│   ├── pikeman/
│   ├── archer/
│   ├── assa/                    # Assassina (abreviado por limite de path)
│   ├── knight/
│   ├── atalanta/
│   ├── priestess/
│   ├── magician/
│   ├── shaman/
│   └── shared/                  # apenas assets comprovadamente partilhados
└── tmABCD/                      # compatibilidade temporária, não eliminar já
```

Dentro da pasta de cada classe, na **primeira fase**, manter os ficheiros referenciados diretamente na raiz dessa classe (`e001.inx`, `tfh-e01.inx`, `hassassin_02.inx` etc.), sem `body/` e `head/` extra. As categorias `base/armor/costume/head` podem ser registadas num **manifesto externo**, sem adicionar profundidade a caminhos persistidos.

**Exemplos compatíveis com 31 caracteres:**

| Referência proposta | Comprimento (caracteres ASCII) |
|:--|--:|
| `char/pc/assa/e001.inx` | 21 |
| `char/pc/assa/hassassin_02.inx` | 29 |
| `char/pc/shaman/me001.inx` | 22 |
| `char/pc/shaman/hshaman_02.inx` | 26 |
| `char/pc/mechanician/tmh-a01.inf` | 31 |

O nome completo `char/pc/assassin/hassassin_02.inx` tem **33** caracteres e excede o limite atualmente imposto em `CheckUserModel()` — por isso a abreviatura `assa/`. O objetivo não é perpetuar a limitação, mas evitar que uma melhoria de pastas introduza personagens invisíveis.

### 5.3. Estrutura futura — depois de remover o limite legado

Após alterar e testar **todos** os pontos de serialização, validação, rede, persistência e carregamento que assumam caminhos curtos, poderá adotar-se uma estrutura mais detalhada:

```
cli/char/
├── monster/
├── npc/
└── player/
    ├── assassin/
    │   ├── base/
    │   ├── heads/
    │   ├── armors/
    │   └── costumes/
    ├── shaman/
    │   ├── base/
    │   ├── heads/
    │   ├── armors/
    │   └── costumes/
    ├── ... outras classes ...
    └── shared/
        ├── animation/
        └── textures/
```

**Esta segunda árvore é apenas uma arquitetura-alvo.** Não é segura no runtime atual sem atualização das restrições de caminho e auditoria das referências internas.

### 5.4. Índice de assets e regras de propriedade

Criar um manifesto de migração gerado por inventário, com pelo menos:

| Campo | Exemplo / finalidade |
|:--|:--|
| `asset_id` | Identificador estável independente do caminho. |
| `original_path` | `char/tmABCD/e001.inx`. |
| `target_path` | `char/pc/assa/e001.inx`. |
| `category` | `base_body`, `head`, `armor`, `costume`, `animation`, `texture`. |
| `owner_class` | `assassina` ou `shared`. |
| `dependencies` | Ficheiros referidos pelo modelo ou material. |
| `referenced_by` | JSON Content, loja, UI, save, rede, código, outros assets. |
| `sha256` / `status` | Integridade e estado `planned`, `copied`, `validated`. |

O manifesto tem de registar se um `.smb` ou uma textura é usada por várias classes. Recursos partilhados não devem ser duplicados ou removidos sem necessidade.

## 6. Plano seguro de migração

1. **Congelar um baseline:** guardar cópia da pasta `cli/char/tmABCD`, dos JSON canónicos, do pack `.scls` e de personagens de teste; registar hashes e versão.
2. **Inventariar:** listar `.ini`, `.inx`, `.inf`, `.smd`, `.smb`, `.bmp`, `.tga`; detetar nomes com colisão case-insensitive e ficheiros referenciados por mais de uma classe.
3. **Corrigir os modelos iniciais no Content:** corpo da Assassina para `e001.inx` e da Chamã para `me001.inx`, atualizando a opção normal correspondente. Manter os trajes de loja.
4. **Rever cabeças e previews:** testar `tfh-e01.inx` e `Mmh-E01.inx` como possíveis variantes `01`; apenas então escolher a cabeça padrão e sincronizar `creation`/`appearance`.
5. **Publicar e testar sem mover pastas:** recompilar `playable_classes.scls`, distribuir packs e verificar criação, seleção, entrada no mundo, troca de armadura, uso/remoção do traje e relog.
6. **Migrar uma classe-piloto:** copiar (não mover) um grupo validado para `char/pc/assa/` ou `char/pc/shaman/`, atualizar as referências e manter o original como fallback temporário.
7. **Verificar todos os consumidores:** Content, carregadores `.ini/.inx`, referências de textura, scripts/loja, pré-visualizações, servidor, cache de cooked assets, save e tráfego de rede.
8. **Migrar as demais classes gradualmente:** repetir o processo por classe; só colocar algo em `shared/` se a reutilização estiver demonstrada.
9. **Desativar o caminho antigo depois da aceitação:** testar com `tmABCD` temporariamente indisponível em ambiente de desenvolvimento, procurar falhas, criar rollback e só então remover a compatibilidade.

### 6.1. Limites de caminho — bloqueador da reorganização

Em `SrcGame/src/core/gameplay/legacy_support/playsub.cpp`, `CheckUserModel()` executa `szModelName[31] = 0` e `szModelName2[31] = 0`, truncando ambos os caminhos. Por sua vez, `Shared/gameplay/classes/v2/domain/ClassDefinition.h` aceita caminhos com menos de **64 bytes** em `SafeCharacterCreationModelPath()`. Portanto, passar na validação V2 **não garante** passar pela fronteira legada. Antes de usar pastas longas, harmonizar as duas regras e validar o armazenamento e os protocolos que transportam os nomes.

**Verificação rápida dos caminhos de destino:**

```bash
python3 - <<'PY'
paths = [
    'char/pc/assa/e001.inx',
    'char/pc/assa/hassassin_02.inx',
    'char/pc/shaman/me001.inx',
    'char/pc/shaman/hshaman_02.inx',
]
for p in paths:
    print(f'{len(p.encode("utf-8")):2d}/31 bytes  {p}')
    assert len(p.encode('utf-8')) <= 31
PY
```

### 6.2. Critérios de aceitação

- Uma Assassina recém-criada surge com **`e001.inx`**, e uma Chamã com **`me001.inx`**, na criação, seleção e mundo.
- Corpo, cabeça/cabelo, movimentos e texturas carregam sem placeholders, peças invisíveis ou ficheiros em falta.
- Os trajes `ce001.inx` e `cme001.inx` continuam equipáveis/comercializáveis **apenas no fluxo cosmético apropriado**.
- Equipar/retirar uma armadura ou traje, teletransportar, relogar e reiniciar o cliente não repõem o visual errado.
- Personagens antigos que guardaram os caminhos dos trajes têm regra explícita de migração ou compatibilidade, sem perder itens comprados.
- Os packs do cliente e servidor são da mesma versão; logs não mostram paths quebrados; alterações são reversíveis.
- Uma classe não referencia indevidamente cabeça/armadura pertencente à outra; um asset partilhado só é movido após análise de dependências.

## 7. O que está confirmado e o que falta validar

| Afirmação | Estado da evidência |
|:--|:--|
| Prefixos `A`–`E`, `mA`–`mE` e convenção `T/M`, `m/f`, `b/h` | **Documentado publicamente** por guias da comunidade. |
| `e001.ini` como corpo em exemplo de Assassina | **Exemplo público** (RaGEZONE); não é a extensão escolhida pelo V2. |
| ContentId 11 e 12; caminhos `ce001.inx` e `cme001.inx` na source recebida | **Verificado nos JSON canónicos**. |
| Caminhos de `creation` e `armor` utilizados pelo serviço V2 | **Verificado no C++**. |
| `e001.inx`, `me001.inx`, `TfbE01.*`, `MmbE01.*`, `Tfh-E01.*`, `Mmh-E01.*` presentes no cliente | **Relatado pelo inventário do utilizador**; binários não inspecionados. |
| Trajes `ce001.inx` e `cme001.inx` vendidos na loja | **Informação funcional do projeto**, dada pelo utilizador. |
| Cabeças `_01` como opção visual inicial definitiva | **Ainda não validado**; recomendada verificação visual antes de alterar. |
| Associação binária exata `.inx` → `.smd/.smb` e textura concreta | **Ainda não validado**; exige inspeção de dependências ou logs de carregamento. |
| Migração para subpastas concluída | **Não**; este documento apresenta um plano. |

## 8. Referências

**Fontes internas inspecionadas na source de 27-10-2026**

- `Content/canonical/playable_classes/assassina.json` e `shaman.json`.
- `Content/canonical/playable_classes/index.json`; `assassin.json` (registo distinto).
- `Shared/gameplay/classes/v2/services/ClassService.cpp`.
- `Shared/gameplay/classes/v2/services/NativeZeroJobModelGuardR98.h`.
- `Shared/gameplay/classes/v2/domain/ClassDefinition.h`.
- `Shared/gameplay/classes/v2/docs/ADDING_A_CLASS.md`.
- `SrcGame/src/core/gameplay/legacy_support/playsub.cpp`.

**Fontes públicas consultadas**

1. SunnyZ, *Model and Texture Viewing + PT File Naming Info — Quick Start*, RaGEZONE (2023): https://forum.ragezone.com/threads/model-and-texture-viewing-pt-file-naming-info-quick-start.1212715/
2. ScreamingFox, *Notes on Armors (Armors 101)*, RaGEZONE (2011): https://forum.ragezone.com/threads/notes-on-armors-armors-101.789886/
3. *Help .dat panel*, RaGEZONE (2022), exemplo `e001.ini` e `tfh-e01.inf`: https://forum.ragezone.com/threads/help-dat-panel.1205051/
4. *Pristontale in Unity*, RaGEZONE (2017), reutilização de modelos e texturas: https://forum.ragezone.com/threads/pristontale-in-unity.1136700/
5. Karai, *pt-asset-tool*, GitHub, descodificação de `.inx`, `.smd`, `.smb` e texturas: https://github.com/karai17/pt-asset-tool

---

**Decisão recomendada:** corrigir primeiro os quatro apontamentos de corpo/armadura no Content, preservar os trajes cosméticos e só avançar com a migração depois de validar as cabeças, os limites de 31 caracteres e as dependências reais. Esta sequência permite melhorar a organização sem degradar a compatibilidade dos personagens existentes.
