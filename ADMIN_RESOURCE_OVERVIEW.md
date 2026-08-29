# Summit — Overview administrativo de recursos

## Objetivo

Este documento é uma visão simples da estrutura do projeto para administração de recursos. Ele não substitui a auditoria técnica: serve para localizar rapidamente cada grupo, entender para que ele existe e saber o estado atual de revisão/limpeza.

## Estrutura principal

| Local | Função simples | Estado atual |
|---|---|---|
| `Summit Tale Client Full/cliente/` | Pacote de runtime do cliente: mapas, personagens, modelos, texturas, áudio, UI e binários | Inventariado; limpeza de assets em andamento |
| `Servidor/` | Pacote de runtime do servidor: dados de mundo/personagem, configs, binários e logs | Inventariado; limpeza operacional ainda deve ser tratada separadamente |
| `SourceCode/SrcGame/` | Código-fonte principal do cliente | Ativo; build auditado |
| `SourceCode/SrcServer/` | Código-fonte principal do servidor | Ativo; build auditado |
| `SourceCode/Shared/` | Headers/estruturas compartilhadas por cliente e servidor | Ativo |
| `SourceCode/Shared2/` | Segunda cópia de três headers compartilhados | Forte candidato a legado; ainda não remover sem diff/hash final |
| `audits/` | Scripts, inventários, relatórios e quarentenas | Área administrativa de auditoria |
| `bugfixes/` | Evidências e relatórios de bugs/correções | Área para histórico técnico das correções |
| `Dependency/DirectX*` | Dependências de terceiros do DirectX | Fora do escopo das auditorias de assets |

## Cliente

**Raiz:** `Summit Tale Client Full/cliente/`  
**Arquivos catalogados:** 28.965  
**Tamanho aproximado:** 4,13 GiB

### Maiores grupos

| Pasta | Arquivos | Tamanho aprox. | Descrição simples |
|---|---:|---:|---|
| `char/` | 7.678 | 2.083,8 MiB | Personagens, monstros, NPCs, modelos, skins, animações e índices |
| `Field/` | 11.609 | 1.292,8 MiB | Mapas, stages, objetos de cenário, modelos e texturas do mundo |
| `game/` | 1.378 | 188,6 MiB | Recursos de gameplay/UI, incluindo mapas auxiliares e seleção de personagem |
| `wav/` | 2.018 | 179,7 MiB | Áudio, efeitos sonoros e BGM |
| `StartImage/` | 443 | 170,2 MiB | Telas de inicialização, login, loading e recursos de entrada |
| `image/` | 4.050 | 109,6 MiB | Imagens de interface, itens e outros elementos 2D |
| `Effect/` | 1.629 | 94,5 MiB | Recursos de efeitos visuais |
| raiz do cliente | 26 | 75,7 MiB | Executáveis, DLLs, logs/configs e arquivos diretamente na raiz |

### Como os assets são carregados

Nem todo arquivo usado aparece literalmente no código. Existem pelo menos três formas importantes de carregamento:

1. **Direto:** caminho está escrito no código/configuração.
2. **Indireto:** um modelo/config aponta para outro asset.
3. **Convencional/dinâmico:** o código constrói o nome a partir de mapa, ID ou extensão.

Exemplo confirmado: o código pode pedir `foo.ASE`, mas o loader usa `foo.smd` existente no mesmo caminho. Por isso, um `.smd` não pode ser considerado solto apenas porque o nome `.smd` não aparece no código.

### Estado de limpeza de `Field`

`Field/old_forest` e `Field/old_dungeon` foram promovidos para **quarentena de confiança muito alta** no cliente. A auditoria não encontrou entrada ativa vinda de fora dessas árvores e não encontrou fallback global do loader para elas.

A quarentena inicial inclui também mapas/títulos legacy associados e três ASE individuais de alta confiança. A movimentação deve ser feita com `summit_quarantine.py`, nunca por exclusão direta.

### `Field/map` e `Field/Title`

Essas pastas **não são lixo**. O cliente constrói dinamicamente caminhos como:

```text
Field/map/<nome>.tga
Field/Title/<nome>t.tga
```

Portanto a ausência do nome completo no código não significa ausência de uso.

## Servidor

**Raiz:** `Servidor/`  
**Arquivos catalogados:** 153.248  
**Tamanho aproximado:** 5,33 GiB

### Maiores grupos

| Pasta | Arquivos | Tamanho aprox. | Descrição simples |
|---|---:|---:|---|
| `char/` | 9.386 | 2.640,2 MiB | Dados/assets de personagens e monstros usados pelo ambiente de servidor |
| `Field/` | 13.293 | 1.434,7 MiB | Dados de mapas/mundo presentes no pacote do servidor |
| raiz do servidor | 51 | 687,4 MiB | Executáveis, DLLs, logs, símbolos e outros arquivos operacionais |
| `Configs/` | 15 | 586,3 MiB | Configuração; tamanho atualmente dominado por artefatos grandes como dump |
| `LogItemData/` | 124.823 | 64,3 MiB | Grande volume de registros/logs de itens |
| `LogFile/` | 186 | 16,1 MiB | Logs gerais do servidor |
| `Maps/` | 41 | 11,9 MiB | Dados auxiliares de mapas |
| `Management/` | 21 | 5,7 MiB | Recursos administrativos/gerenciais do servidor |
| `GameServer/` | 5.047 | 3,3 MiB | Dados operacionais do GameServer |
| `DataServer/` | 277 | 2,4 MiB | Dados operacionais do DataServer |

### Artefatos operacionais grandes encontrados

Estes arquivos são diferentes de assets do jogo e devem ter política administrativa própria:

- `Configs/Crash.dmp` — aproximadamente 586 MiB; dump de crash.
- `wine.log` — aproximadamente 532 MiB; log do Wine.
- `PriTaTor_Errors.log` — aproximadamente 63 MiB; log de erros.
- `Server.pdb` — aproximadamente 42 MiB; símbolos de debug, úteis para diagnóstico mas não necessários para runtime normal.

Não foram incluídos no plano atual de quarentena do servidor. O CSV `quarantine_server_candidates.csv` começa vazio para impedir limpeza acidental sem uma decisão específica.

## Duplicação cliente x servidor

A auditoria encontrou **7.443 arquivos com mesmo caminho relativo e conteúdo idêntico** nos dois pacotes, somando aproximadamente **1,87 GiB**.

Isso não significa que uma das cópias possa ser apagada: cliente e servidor são runtimes separados e podem precisar do mesmo arquivo localmente. A duplicação serve principalmente para entender a arquitetura e avaliar, no futuro, empacotamento ou distribuição compartilhada.

## Código-fonte

### `SourceCode/SrcGame/`

Código do cliente. O build auditado compila principalmente `SrcGame/src` e módulos específicos. Contém lógica de UI, personagem, mapa, loader 3D, efeitos, rede e outros sistemas do cliente.

### `SourceCode/SrcServer/`

Código do servidor. Contém GameServer/DataServer e lógica de mundo/jogador correspondente ao ambiente servidor.

### `SourceCode/Shared/`

Estruturas e headers usados pelos dois lados. O grafo de includes confirmou uso ativo dos headers equivalentes a:

- `b_sinCraftItem.h`
- `LevelTable.h`
- `smPacket.h`

### `SourceCode/Shared2/`

Contém apenas três headers com esses mesmos nomes. Nenhum foi alcançado pelo grafo de includes, embora a pasta exista no include path dos builds. Classificação administrativa atual:

**LEGADO PROVÁVEL / NÃO REMOVER AINDA**

A última verificação necessária é comparar conteúdo/hash com `Shared/` e procurar includes explícitos com `Shared2/`.

## `audits/`

Área recomendada para todo material de análise, separado do runtime:

```text
audits/
  summit_inventory_audit/       inventário da Fase 1
  summit_phase2/                grafo de dependências de assets
  quarantine/                   itens temporariamente removidos do runtime
    cliente/<run-id>/
    servidor/<run-id>/
    manifests/
  *.py                          scripts de auditoria/manutenção
```

A quarentena não é lixeira. Itens permanecem preservados e restauráveis até que a validação seja concluída.

## `bugfixes/`

Deve guardar um diretório por problema/correção, com evidência, sistema afetado, arquivos, solução e teste. Para o bug encontrado nesta auditoria foi preparado:

```text
bugfixes/021_ase_path_parser_audit/
  README.txt
  BUG_REPORT.md
```

## Política administrativa recomendada

| Classe | Ação |
|---|---|
| `ACTIVE` | Não mover/remover |
| `INDIRECT_ACTIVE` | Não mover; usado por dependência ou convenção |
| `QUARANTINE_HIGH` | Pode ser movido para quarentena reversível e testado |
| `REVIEW` | Precisa de investigação antes de qualquer mudança |
| `LEGACY_PROBABLE` | Comparar/diff e confirmar ausência de uso |
| `LOG/DUMP` | Aplicar política de retenção/rotação, separada da auditoria de assets |
| `DUPLICATE` | Não remover só por hash; primeiro identificar qual caminho é canônico |

## Regra de ouro

**Inventário → referência → dependência → quarentena → teste → remoção.**

Nunca pular diretamente de “não apareceu no grep” para exclusão de asset.
