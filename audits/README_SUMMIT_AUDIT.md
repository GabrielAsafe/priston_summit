# Summit Asset Audit Toolkit

Ferramentas e metodologia usadas para catalogar, auditar e reduzir lixo no projeto **Summit Tale** sem apagar recursos de forma prematura.

## 1. Objetivo

Esta toolkit foi criada para responder quatro perguntas:

1. **O que existe no cliente e no servidor?**
2. **O que o código realmente usa?**
3. **Quais assets parecem antigos, duplicados ou órfãos?**
4. **Como testar uma possível remoção de forma reversível?**

A regra principal do processo é:

> **"Não apareceu no código" não significa "pode apagar".**

O cliente usa carregamento indireto e dinâmico. Modelos, mapas, texturas e configurações podem ser ligados por `.ini`, `.inx`, `.smd`, `.smb`, nomes construídos em runtime e outros formatos.

Por isso o fluxo usado é:

```text
Inventário
   ↓
Referências diretas
   ↓
Dependências indiretas
   ↓
Auditoria dos loaders
   ↓
Quarentena reversível
   ↓
Teste no jogo
   ↓
Remoção futura, somente depois de validação
```

---

## 2. Estrutura principal do projeto

Raiz:

```text
/home/gabriel/Área de Trabalho/summit
```

Diretório operacional das auditorias:

```text
/home/gabriel/Área de Trabalho/summit/audits
```

Principais árvores analisadas:

```text
Summit Tale Client Full/cliente/
Servidor/
SourceCode/SrcGame/
SourceCode/SrcServer/
SourceCode/Shared/
SourceCode/Shared2/
```

Diretórios DirectX são ignorados pela auditoria de assets/código sempre que possível.

---

## 3. Ferramentas

### `summit_inventory_audit.py`

Fase 1.

Faz a catalogação inicial do cliente e servidor e cruza o conteúdo com o código-fonte.

Principais saídas:

```text
summit_inventory_audit/
├── RESUMO.txt
├── catalog_cliente.csv
├── catalog_servidor.csv
├── catalog_diretorios_cliente.csv
├── catalog_diretorios_servidor.csv
├── cliente_assets_referencias.csv
├── cliente_assets_soltos_candidatos.csv
├── referencias_codigo_ausentes_no_cliente.csv
├── shared2_usage.csv
├── duplicados_cliente_servidor.csv
└── codigo_ativo_grafo_include.csv
```

O scanner reconstrói parte do código ativo a partir dos scripts de build e do grafo de `#include`.

### `summit_phase2_assets.py`

Fase 2.

Analisa especialmente `char/` e `Field/` e procura relações entre assets, inclusive strings embutidas em arquivos texto e binários.

Principais saídas:

```text
summit_phase2/
├── RESUMO_FASE2.txt
├── phase2_directory_summary.csv
├── phase2_asset_status_char_field.csv
├── phase2_orphans_high_confidence.csv
├── phase2_orphans_review.csv
├── phase2_edges.csv
├── phase2_broken_references.csv
└── phase2_ambiguous_references.csv
```

Importante: a Fase 2 é heurística. O resultado precisa ser interpretado com o comportamento real dos loaders.

### `summit_quarantine.py`

Move candidatos para fora do runtime, sem apagar.

Possui quatro comandos:

```text
plan
apply
status
restore
```

A quarentena é separada por lado:

```text
audits/quarantine/
├── cliente/<RUN_ID>/
├── servidor/<RUN_ID>/
└── manifests/
```

O servidor e o cliente nunca compartilham o mesmo destino físico.

---

## 4. Estado da auditoria em 25/08/2026

### Fase 1

Foram catalogados aproximadamente:

```text
Cliente:  28.965 arquivos / ~4,13 GiB
Servidor: 153.248 arquivos / ~5,33 GiB
```

A primeira busca estática indicou mais de 25 mil arquivos do cliente sem referência textual direta. Isso **não foi interpretado como lixo**, porque várias famílias são carregadas indiretamente.

Também foram encontrados milhares de conteúdos duplicados e uma quantidade significativa de arquivos idênticos presentes em cliente e servidor.

### `Shared2`

`Shared2` contém os mesmos nomes principais existentes em `Shared`:

```text
b_sinCraftItem.h
LevelTable.h
smPacket.h
```

Os scripts de build adicionam `Shared2` ao include path, portanto ele não pode ser ignorado somente pela configuração.

Porém, no grafo de `#include` analisado, os três arquivos de `Shared2` não foram alcançados, enquanto os equivalentes de `Shared` foram.

Estado atual:

```text
Shared2 = LEGACY_PROBABLE / verificar conteúdo antes de quarentena
```

### Fase 2 — `char` e `Field`

Foram analisados aproximadamente:

```text
19.287 arquivos
~3,30 GiB
```

A primeira propagação encontrou assets usados diretamente, indiretamente e milhares de candidatos sem cadeia resolvida.

Essa fase revelou que uma comparação somente por basename produzia falsos positivos.

### Correção importante: `.ASE` → `.SMD`

A auditoria do loader mostrou que o código pode pedir:

```text
forest//3ani-08.ASE
```

enquanto o runtime carrega:

```text
Field/forest/3ani-08.smd
```

O loader tenta o `.SMD` correspondente no mesmo caminho.

Foram identificados **414 SMDs** que precisavam ser reclassificados como usados implicitamente.

Regra para auditorias futuras:

> Uma referência a `foo.ASE` deve verificar primeiro `foo.smd` no mesmo diretório antes de considerar o modelo ausente ou o SMD órfão.

### `Field/map` e `Field/Title`

Essas pastas não podem ser classificadas por busca literal simples.

`SetName()` constrói nomes de forma dinâmica, por exemplo:

```text
Field/map/<nome>.tga
Field/Title/<nome>t.tga
```

Portanto assets dessas pastas podem estar ativos mesmo sem o caminho completo existir como string no código.

### `.INX`

No fluxo analisado do servidor, `.inx` funciona como marcador de nome para localizar o `.ini` correspondente.

O conteúdo interno do `.inx` não deve ser automaticamente tratado como uma lista de dependências runtime.

### Bug encontrado no parser ASE

Foram encontradas comparações do tipo:

```cpp
if (DataDirectory[cnt] == '//')
```

e:

```cpp
if (strBuff[cnt] == '//' || strBuff[cnt] == ':')
```

`'//'` é uma constante multicaractere em C/C++, não significa `/` nem `\`.

Esse bug foi separado para a área de `bugfixes`.

Também existem buffers de caminho/nome curtos no código legado que merecem revisão ao corrigir o parser.

---

## 5. Primeira quarentena executada

Foram colocados em quarentena, entre outros:

```text
Field/old_forest/
Field/old_dungeon/

mapas e títulos legacy associados

Field/cave/Dcave_anii14.ASE
Field/forever-fall/3ani-08.ASE
Field/Ricarten/world_ani06.ASE
```

`old_forest` e `old_dungeon` mostraram forte evidência de árvores antigas:

- milhares de arquivos;
- grande quantidade de conteúdo duplicado;
- nenhuma dependência ativa externa resolvida;
- referências internas restritas à própria família antiga.

A primeira rodada moveu aproximadamente **161 MiB**.  
`world_ani06.ASE` precisou de uma segunda rodada porque a pasta `Field/Ricarten` estava em modo `0555`.

Depois de ajustar temporariamente a permissão do diretório, o arquivo também foi colocado em quarentena.

### Teste já realizado

Após a quarentena:

- o cliente iniciou;
- não houve crash imediato;
- Ricarten carregou normalmente.

Isto é um **smoke test**, não uma validação total.

Estado atual da família:

```text
QUARANTINE_TESTING
```

e ainda não:

```text
REMOVABLE_CONFIRMED
```

---

## 6. Política de classificação

Usar preferencialmente estes estados:

| Estado | Significado |
|---|---|
| `ACTIVE` | Uso confirmado |
| `USED_DIRECT` | Referência direta |
| `USED_INDIRECT` | Alcançado através de outro asset |
| `USED_IMPLICIT` | Uso provado por regra do loader, como ASE → SMD |
| `DYNAMIC` | Nome construído em runtime |
| `REVIEW` | Evidência insuficiente |
| `LEGACY_PROBABLE` | Forte sinal de conteúdo antigo |
| `QUARANTINE_HIGH` | Forte candidato para teste fora do runtime |
| `QUARANTINE_TESTING` | Já removido do runtime e em validação |
| `REMOVABLE_CONFIRMED` | Passou pelo protocolo de testes definido |
| `BROKEN_REFERENCE` | Código/asset aponta para recurso não encontrado |
| `LOG/DUMP/TEMP` | Artefato operacional ou temporário |

---

## 7. O que NÃO fazer

Não:

```text
rm -rf
```

em candidatos encontrados apenas por scanner.

Não considerar automaticamente:

```text
"sem grep" = "não usado"
```

Não remover duplicados cliente ↔ servidor apenas porque possuem o mesmo hash. Eles podem ser necessários em dois runtimes independentes.

Não remover `.SMD` porque o código menciona somente `.ASE`.

Não apagar manifestos de quarentena enquanto o lote ainda estiver em teste.

---

## 8. Próximos passos recomendados

### Etapa A — terminar o smoke test de `Field`

Testar:

```text
Ricarten                 OK inicial
Forest
Dungeon
Cave
Forever Fall
mapa-múndi / minimapa
títulos das áreas
teleportes
entrada e saída repetida das áreas
```

Observar:

```text
crashes
texturas ausentes
cenário invisível
objetos sem material
mapas/títulos ausentes
erros de arquivo nos logs
```

Se tudo passar, promover o lote para:

```text
REMOVABLE_CONFIRMED
```

após um período de retenção da quarentena.

### Etapa B — auditar `char/`

Próximo grande alvo.

É necessário mapear:

```text
INI
 ↓
INX / configuração
 ↓
SMD / SMB
 ↓
texturas
 ↓
animações / efeitos
```

Separar:

```text
monstros ativos
NPCs ativos
modelos antigos
skins alternativas
arquivos temp/copy
duplicados
```

### Etapa C — `Shared2`

Comparar conteúdo/hashes com `Shared`.

Depois verificar:

- includes condicionais;
- macros;
- builds alternativos;
- ferramentas auxiliares.

Se continuar sem uso, mover `Shared2` inteiro para quarentena de código.

### Etapa D — servidor

Auditar separadamente:

```text
logs
dumps
PDBs
backups
userdata histórico
LogItemData
arquivos operacionais
assets realmente necessários em runtime
```

Não misturar limpeza do servidor com limpeza do cliente.

### Etapa E — corrigir loader ASE

Usar o relatório em:

```text
bugfixes/021_ase_path_parser_audit/
```

Objetivos:

- corrigir separadores `/` e `\`;
- remover uso da constante `'//'`;
- preservar compatibilidade com assets legados;
- revisar limites de buffers;
- testar conversão ASE → SMD;
- testar caminhos relativos e absolutos.

### Etapa F — reexecutar os scanners

Depois de mudanças importantes:

1. executar Fase 1 novamente;
2. executar Fase 2 novamente;
3. comparar relatórios antigos e novos;
4. nunca usar relatórios antigos como verdade absoluta depois de mover assets.

---

## 9. Regra para retomada futura

Ao voltar a este trabalho no futuro:

1. leia este README;
2. rode `summit_quarantine.py status`;
3. veja quais lotes ainda estão em quarentena;
4. confirme o que já passou em testes;
5. atualize a classificação;
6. reexecute o inventário se a árvore mudou;
7. só então inicie uma nova família de assets.

A ferramenta deve ser tratada como um processo de **redução controlada de risco**, não como um limpador automático.
