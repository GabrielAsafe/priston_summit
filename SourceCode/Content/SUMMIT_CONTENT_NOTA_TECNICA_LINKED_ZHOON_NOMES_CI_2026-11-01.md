# SUMMIT — Nota técnica: nomes de itens CI, Content canónico e ficheiros `.zhoon`

**Data:** 2026-11-01  
**Incidente:** os materiais `CI101–CI168` aparecem no jogo com o nome «Pedra Azul»; o `419A inspect` terminou em `SUMMIT419A_BLOCKED UNEXPECTED_LINKED_FILE CI101: Name\BI101.zhoon`.  
**Estado:** causa estrutural identificada para **CI101**; **sem correção aplicada** por esta nota. A ligação dos restantes 67 códigos ainda requer auditoria na versão atual do projeto.

## 1. Facto observado no Fedora

```text
SUMMIT419A_BLOCKED UNEXPECTED_LINKED_FILE CI101: Name\BI101.zhoon
```

O 419A esperava um ficheiro de nome exclusivo (`Name\CI101.zhoon`), mas a definição canónica `CI101.txt.json` da instalação do utilizador aponta para `Name\BI101.zhoon`. O `inspect` deteve-se antes da fase de escrita; este bloqueio **não significa que tenha alterado um item**.

**Atenção:** o código `CI101` é um item novo; `BI101` é um item legacy distinto. **Nunca editar `BI101.zhoon` para renomear CI101**, pois esse recurso pode ser partilhado e a alteração afetaria o item legacy e outros consumidores.

## 2. Confirmação na árvore de fontes reconstruídas

O snapshot de referência anterior aos patches recentes contém:

- `SourceCode/Content/canonical/text/resources/Servidor/GameServer/OpenItem/BI101.txt.json`
  - `kind: open_item_definition`, `id: item.bi101`, `semantic.fields.code: BI101`;
  - `semantic.fields.name: Pedra Azul`;
  - `semantic.fields.linked_file: Name\\BI101.zhoon`;
  - as diretivas legacy têm `name`, `english_name` e `linked_file` e uma representação em `compatibility.legacy_payload_base64`.
- `SourceCode/Content/canonical/text/resources/Servidor/GameServer/OpenItem/name/BI101.zhoon.json`
  - `kind: localization_resource`, `id: text.bi101`;
  - a diretiva legacy `*J_NAME` contém `Pedra Azul`.

A associação de **CI101** a `Name\BI101.zhoon` foi confirmada pelo **log da instalação Fedora**, não por esse snapshot antigo, no qual os CI ainda não estavam incorporados.

## 3. Como o Content processa os nomes

### Camadas relacionadas

1. **Definição de item** — `OpenItem/CI101.txt.json` armazena `semantic.fields.name`, `english_name`, `linked_file` e `semantic.directives`.
2. **Bytes legacy da definição** — `compatibility.legacy_payload_base64` guarda a representação do ficheiro `.txt` original; não deve divergir das diretivas canónicas.
3. **Ficheiro de nome associado** — `OpenItem/name/BI101.zhoon.json` ou futuro `CI101.zhoon.json`, com `semantic.entries` e payload legacy (`*J_NAME`).
4. **Índice canónico** — `SourceCode/Content/canonical/text/index.json` declara os documentos disponíveis, os IDs e os `runtime_paths`; criar apenas um JSON sem o indexar pode deixá-lo fora da compilação.
5. **Compiladores/geradores de Content** — produzem dados de definições, localizações e packs a partir do catálogo, e esses outputs precisam de ser publicados e recarregados.

### Evidência no código de compilação da fonte reconstruída

- `SourceCode/tools/content_port/compile_definitions.py`: ao encontrar a diretiva `linked_file`, resolve o recurso associado e incorpora as diretivas do `.zhoon` no registo compilado (`flattened_links`); essa ramificação termina o processamento das restantes diretivas da definição com `break`. Isto torna perigoso mudar apenas `semantic.fields.name`.
- `SourceCode/tools/content_port/materialize_localization_catalog.py`: em `build_catalog`, resolve o ficheiro associado e dá prioridade ao nome `*J_NAME` do `.zhoon` no catálogo `pt-BR`, antes de usar `semantic.fields.name` como alternativa (`fallback`).

Consequentemente, **um `CI101` com campo `name` correto pode continuar a apresentar «Pedra Azul»** se o Content compilado escolher o `.zhoon` de `BI101`. Esta é uma explicação técnica consistente com o sintoma; a proveniência exata do nome da mochila no runtime instalado ainda deve ser confirmada com testes de Content e cliente.

## 4. Contrato para uma correção segura (futuro 419B)

**Não reescrever o `.zhoon` legacy de BI101.** Para os materiais CI:

1. **Inventário só de leitura dos 68 itens**: por código, registar `semantic.fields.name`, diretivas `name`, `english_name` e `linked_file`, nome no payload, destino do `.zhoon`, identidade do documento, `runtime_paths`, `*J_NAME` e quantidade de definições que referem o mesmo destino.
2. **Detetar aliases e partilhas**: separar `CI→CI`, `CI→BI`, ficheiros ausentes, links duplicados e referências não indexadas. Não assumir que todos estão ligados a `BI101` com base numa única falha.
3. **Criar um recurso de localização próprio por CI quando necessário**, por exemplo `OpenItem/name/CI101.zhoon.json` com `id: text.ci101`, `runtime_paths` únicos e `*J_NAME` do material real; adicionar a entrada correspondente a `canonical/text/index.json`.
4. **Atualizar a definição CI** em todas as representações relevantes: `semantic.fields.linked_file`, diretiva `linked_file` e `raw_values_hex`, `semantic.fields.name` / `english_name`, diretivas de nome e `compatibility.legacy_payload_base64`. Atualizar metadados/hash de origem conforme o contrato da migração e preservar backups dos bytes originais.
5. **Não alterar** `BI101.txt.json`, `BI101.zhoon.json`, IDs numéricos, receitas, loot, BMPs ou DAT.
6. **Validar sem instalar primeiro**: unicidade dos 68 códigos e dos destinos de nomes, inexistência de regressões legacy, todos os links resolvidos, Content canónico válido, compilação dos packs com 68 nomes diferentes/conformes ao mapa oficial.
7. **Publicar e testar**: recompilar Content (`sh compCannonContent.sh`), distribuir os packs para os destinos efetivamente usados pelo servidor e cliente (se necessário), reiniciar os processos que os mantêm em memória e comparar, no jogo, pelo menos `CI101`, `CI113`, `CI125`, `CI151` e `CI165`.

> Um instalador de correção deve usar `inspect`, `apply`, `verify` e `undo` com hashes e backups, bloquear alterações inesperadas e **não modificar os recursos legacy partilhados**.

## 5. Exemplo de configuração pretendida (ilustrativo; não é um patch executável)

```text
ANTES (confirmado para CI101 no log):
  OpenItem/CI101.txt.json
     code = CI101
     linked_file = Name\BI101.zhoon   ← referência errada/partilhada
  OpenItem/name/BI101.zhoon.json
     *J_NAME "Pedra Azul"              ← pertence ao item legacy BI101

DEPOIS (objetivo técnico a validar):
  OpenItem/CI101.txt.json
     code = CI101
     name = "Minério Cristalino Bruto"
     linked_file = Name\CI101.zhoon
  OpenItem/name/CI101.zhoon.json
     id = text.ci101
     *J_NAME "Minério Cristalino Bruto"
  canonical/text/index.json
     ... registo novo/único para text.ci101 ...

  OpenItem/BI101.txt.json         ← intocado
  OpenItem/name/BI101.zhoon.json  ← intocado: "Pedra Azul"
```

**Nota:** a grafia final dos nomes deve vir do mapa de nomes oficial validado contra `professions_ecosystem_366.json`; o exemplo anterior não autoriza conversões automáticas por semelhança.

## 6. Testes de aceitação

| Controlo | Critério |
|---|---|
| Integridade dos CI | `CI101–CI168`: 68 códigos, IDs e BMPs inalterados |
| Isolamento legacy | conteúdo de `BI101.txt.json` e `name/BI101.zhoon.json` com os mesmos SHA-256 pré e pós-patch |
| Relações do Content | cada CI resolve o `.zhoon` próprio ou outro recurso intencionalmente partilhado; zero links acidentais para BI |
| Compilação | sem links órfãos ou colisões de `runtime_paths`/IDs no índice canónico |
| Nomes na mochila | CI101 não é «Pedra Azul»; outros CI exibem nomes correspondentes aos recursos; BI101 continua «Pedra Azul» |
| Continuidade funcional | os 26 nodes do mapa 58 continuam a entregar itens físicos; receitas, IDs e DAT não sofrem regressão |

## 7. Limites e conclusões

- **Confirmado:** o 419A `inspect` bloqueia no `CI101` porque o seu `linked_file` aponta para `Name\BI101.zhoon`.
- **Confirmado no snapshot:** o `.zhoon` de BI101 contém «Pedra Azul» e o compilador de Content incorpora os nomes dos `.zhoon` ligados; a materialização de `pt-BR` dá prioridade a `*J_NAME`.
- **Provável, ainda por testar no runtime:** isto explica o nome «Pedra Azul» mostrado para CI101.
- **Não confirmado:** que os 67 restantes CI tenham exatamente a mesma referência; ou que o binário de cliente em execução já tenha recarregado os packs mais recentes.
- **Decisão de segurança:** não aplicar a estratégia antiga do 419A sem adaptar os links partilhados e o índice canónico. Não editar `BI101.zhoon` diretamente.

**Fontes examinadas:** log do utilizador, `SUMMIT_PATCH_419A_NOMES_NATIVOS_68_2026-11-01/tools/fix_item_names_419.py` e snapshot reconstruído de `SourceCode/Content/canonical/text` / `SourceCode/tools/content_port`. Os detalhes do ambiente atual que não estão nesses materiais exigem verificação no Fedora.
