# Summit Bugfix Workflow

Este documento define o formato padrão para investigar e corrigir bugs no projeto Summit.

O objetivo não é apenas fazer o bug desaparecer. O objetivo é conseguir responder, depois do fix:

1. Qual era o sintoma?
2. Onde ele realmente acontecia?
3. Qual evidência provou a causa?
4. Qual foi a menor alteração necessária?
5. Como sabemos que a correção resolveu o problema sem criar outro?
6. Como reproduzir a investigação no futuro?

## Regra principal

**Não alterar código ou assets enquanto a causa ainda for apenas uma hipótese.**

Uma investigação deve passar por três estados:

- **Hipótese**: explicação possível, ainda não comprovada.
- **Evidência**: fato observado no source, runtime, filesystem, log ou upstream.
- **Causa provada**: cadeia completa ligando o sintoma ao defeito.

Exemplo da cadeia usada no bug de ícone de redução de stamina:

```text
buff ativo
  -> STAMINA_REDUCE_P
  -> TGAImageNumber = 13
  -> renderer usa índice 13 - 1
  -> UpKeepItemTGAImage[12]
  -> source pede S_Reduce.tga
  -> asset não existe no cliente
  -> material gráfico não pode ser carregado
  -> buff funciona, mas fica sem imagem
```

## Formato de um bugfix

Cada bug deve ficar isolado em seu próprio diretório:

```text
bugfixes/
  NNN_nome_curto_do_bug/
    README.md
    base_commit.txt
    before/
    after/
    hashes_before.sha256
    hashes_after.sha256
    change.patch              # quando houver alteração textual
    before_after.diff         # quando fizer sentido
    source_assets.sha256      # quando assets externos forem recuperados
```

Nem todo fix precisa de todos os arquivos. Um fix só de assets, por exemplo, pode não ter `change.patch`.

### `base_commit.txt`

Registrar o estado de onde a investigação partiu:

```bash
git rev-parse HEAD > bugfixes/NNN_fix/base_commit.txt
```

### `before/`

Guardar o estado anterior necessário para auditoria.

Se o problema for um arquivo existente, copie o original mantendo o caminho relativo.

Se o problema for justamente um arquivo ausente, **não crie um arquivo falso**. Registre a ausência:

```text
before/files_missing.txt
```

com os caminhos que deveriam existir.

### `after/`

Guardar exatamente os arquivos que constituem o fix, preservando os caminhos relativos do projeto/runtime.

### Hashes

Use SHA-256 para provar quais bytes foram analisados e quais bytes foram produzidos:

```bash
sha256sum arquivo1 arquivo2 > hashes_after.sha256
```

Quando o fix recupera assets de outra base, registre também os hashes da fonte.

## Fluxo padrão

### 1. Descrever o sintoma sem explicar a causa

Bom:

```text
Poção de redução de RES aplica o efeito, mas o ícone do buff não aparece.
```

Ruim:

```text
O path do ícone está errado.
```

A segunda frase já assume uma causa antes da investigação.

### 2. Identificar o subsistema

Pergunte:

- É cliente ou servidor?
- É lógica, UI, asset, protocolo, persistência ou loader?
- O efeito funciona e só a apresentação falha?
- O erro acontece no startup, durante load, durante uso ou durante render?

Comece procurando termos ligados ao sintoma e depois siga os identificadores encontrados.

### 3. Seguir dados, não nomes humanos

Strings de interface ajudam a encontrar o domínio, mas normalmente não são a causa.

Depois de encontrar uma string, siga:

```text
texto -> item CODE -> função de uso -> estado/buff -> ID interno -> renderer/loader -> asset/path
```

Em código antigo, IDs, enums, índices e arrays costumam ser mais importantes que o texto exibido.

### 4. Fechar a cadeia de execução

Não pare no primeiro `grep` interessante.

Exemplo:

```text
STAMINA_REDUCE_P = 13
DrawUp usa UpKeepItemTGAImage[TGAImageNumber - 1]
13 - 1 = 12
UpKeepItemTGAImage[12] carrega S_Reduce.tga
S_Reduce.tga está ausente
```

Quando todos os elos são observáveis, a causa está provada.

### 5. Comparar com runtime real

O source pode estar correto e o deploy errado.

Sempre que houver path/asset/configuração, compare com o filesystem do cliente/servidor que realmente roda.

Verifique:

- arquivo existe?
- nome exato?
- extensão?
- maiúsculas/minúsculas?
- diretório correto?
- tamanho esperado?
- tipo de arquivo?
- asset chegou ao runtime?

### 6. Usar upstream/recovery apenas como evidência

Bases antigas, backups, HDs, forks e upstreams são referências.

Não copie árvores inteiras para a árvore ativa.

Procure primeiro pelo elemento exato. Depois compare implementação e asset.

Uma fonte externa pode provar:

- que um nome de asset era legítimo;
- que uma enumeração já existia;
- que a regressão não foi introduzida localmente;
- qual era o conteúdo original perdido.

### 7. Escolher o menor fix possível

Prioridade:

```text
restaurar dado/asset faltante
  > corrigir configuração/path
  > corrigir uma linha de lógica
  > alterar loader
  > refatorar subsistema
```

Não altere um loader global para resolver um único asset ausente se o código já pede o arquivo correto.

### 8. Criar BEFORE/AFTER antes de instalar

Prepare o fix dentro de `bugfixes/NNN.../after/`.

Só então copie/aplique na árvore ativa.

Isso evita perder a origem do que foi alterado.

### 9. Build somente quando necessário

- Mudou `.cpp/.h` ou recursos compilados: build.
- Mudou apenas asset externo carregado em runtime: normalmente não há rebuild.
- Mudou configuração interpretada em runtime: normalmente não há rebuild.

### 10. Testar o sintoma original

O teste deve responder à pergunta inicial.

Para um buff sem imagem:

```text
ativar o buff
confirmar que o efeito continua funcionando
confirmar que o ícone aparece
confirmar timer/overlay
relogar/reabrir se o estado é persistido
```

### 11. Só depois fechar e commitar

Antes do commit:

```bash
git status --short
git diff --check
git diff
```

Confirme que o commit contém somente o bug investigado.

## Modelo de README de um bugfix

```markdown
# NNN - Nome do bug

## Sintoma
Descrição observável, sem hipótese.

## Escopo
Cliente/servidor e subsistema afetado.

## Causa provada
Cadeia completa de evidências.

## Evidências
Comandos e resultados relevantes.

## Fix
Arquivos adicionados/alterados e por quê.

## Proveniência
Origem de assets/código recuperado e hashes quando aplicável.

## Validação
Build realizado (se aplicável) e teste funcional.

## Resultado
PASS/FAIL e observações.
```

## Checklist de encerramento

- [ ] Sintoma reproduzido ou claramente identificado.
- [ ] Hipótese separada de evidência.
- [ ] Causa ligada ao sintoma por uma cadeia completa.
- [ ] Runtime conferido quando o bug envolve arquivos/paths.
- [ ] Fix mínimo escolhido.
- [ ] BEFORE registrado.
- [ ] AFTER registrado.
- [ ] Hashes registrados.
- [ ] Build executado somente se necessário.
- [ ] Teste funcional do sintoma original passou.
- [ ] `git diff --check` passou.
- [ ] Commit contém somente esse bug.
