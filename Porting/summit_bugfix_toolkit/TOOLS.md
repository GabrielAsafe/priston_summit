# Ferramentas de Investigação de Bugs

Este guia reúne os comandos usados durante investigação de source C/C++, assets e runtime, com foco no significado das opções.

## `grep`: procurar referências no source

### Busca recursiva com linha e sem diferenciar caixa

```bash
grep -Rni 'texto' SrcGame/src
```

- `-R`: recursivo; segue a árvore de diretórios.
- `-n`: mostra número da linha.
- `-i`: ignora diferença entre maiúsculas e minúsculas.

### Expressões regulares estendidas

```bash
grep -RniE 'buff|skill|icon|reduce' SrcGame/src
```

- `-E`: usa regex estendida. Permite `|`, grupos etc. sem escapar tanto.

Exemplo:

```bash
grep -RniE 'S_Reduce|STAMINA_REDUCE_P|TGAImageNumber' SrcGame/src
```

### Restringir extensões

```bash
grep -RniE \
  --include='*.cpp' \
  --include='*.h' \
  'STAMINA_REDUCE_P|S_Reduce' \
  SrcGame/src
```

Isso evita lixo de `.o`, `.obj`, SDKs e binários.

### Contexto: `-B`, `-A`, `-C`

```bash
grep -n -B10 -A30 'S_Reduce' arquivo.cpp
```

- `-B10`: 10 linhas **before** do match.
- `-A30`: 30 linhas **after** do match.
- `-C20`: 20 linhas antes e 20 depois.

Use contexto quando encontrar uma linha importante. A linha isolada mostra o fato; o contexto mostra a lógica.

### Tratar arquivo textual antigo como texto

```bash
grep -a -n 'S_Reduce' HaPremiumItem.cpp
```

- `-a` / `--text`: força `grep` a tratar bytes como texto.

Útil em sources antigos com CP949/ANSI/CRLF que o `grep` chama de "binary file matches".

### Match literal

```bash
grep -RniF 'UpKeepItemTGAImage[12]' SrcGame/src
```

- `-F`: procura string literal, sem interpretar regex.

Quando você já sabe exatamente o token, `-F` reduz ambiguidades.

### Excluir ruído

```bash
grep -RniE 'termo' . \
  --exclude-dir=.git \
  --exclude-dir=build_mingw_client_i686 \
  --exclude-dir=build_mingw_server_i686
```

## `find`: procurar arquivos e diretórios

### Nome exato sem diferenciar caixa

```bash
find . -type f -iname 'S_Reduce.tga' -print
```

- `-type f`: somente arquivos.
- `-type d`: somente diretórios.
- `-iname`: nome case-insensitive.
- `-name`: nome case-sensitive.

### Vários nomes

```bash
find . -type f \( \
  -iname 'S_Reduce.tga' -o \
  -iname 'S_Reduce_back.bmp' \
\) -print
```

- `\(` e `\)`: agrupam condições no shell.
- `-o`: OR.

### Procurar pelo caminho inteiro

```bash
find . -type f -ipath '*sinimage*premium*' -print
```

- `-ipath`: compara o caminho completo ignorando caixa.

Útil quando nomes de arquivos não contêm `buff`, `skill` etc., mas o diretório identifica o domínio.

### Limitar profundidade

```bash
find ./image/Sinimage/Premium -maxdepth 1 -type f -print
```

- `-maxdepth 1`: não desce em subdiretórios.

Muito importante em HDs grandes: localize primeiro diretórios candidatos, depois procure somente neles.

### Formatar a saída

```bash
find . -type f -printf '%s %p\n'
```

- `%s`: tamanho em bytes.
- `%p`: caminho.
- `%f`: basename.

Exemplo útil para procurar assets de dimensões/formato semelhantes pelo tamanho:

```bash
find ./Premium -maxdepth 1 -type f -iname '*.dds' -printf '%s %f\n' | sort -n
```

## `sed`: abrir uma região conhecida

```bash
sed -n '145,210p' HaPremiumItem.cpp
```

- `-n`: não imprime tudo automaticamente.
- `'145,210p'`: imprime somente esse intervalo.

Use depois que `grep -n` já mostrou as linhas relevantes.

## `cat -v`: revelar bytes/caracteres estranhos

```bash
sed -n '100,150p' arquivo.cpp | cat -v
```

Mostra CRLF (`^M`) e bytes não imprimíveis. É útil para distinguir problema de encoding de problema lógico.

## `file`: identificar o formato real

```bash
file S_Reduce.dds S_Reduce.tga S_Reduce_back.bmp
```

Não confie somente na extensão. `file` inspeciona a assinatura/estrutura.

## ImageMagick: `identify` e `magick`

### Dimensões e formato

```bash
identify S_Reduce.dds
```

Exemplo de informação útil:

```text
DDS 32x32 ...
```

### Conversão

ImageMagick 7:

```bash
magick S_Reduce.dds S_Reduce.tga
magick S_Reduce_back.dds S_Reduce_back.bmp
```

O antigo `convert` ainda pode funcionar, mas em IMv7 é deprecated.

Depois da conversão, valide novamente com `file` e `identify`.

## `xxd`: inspecionar header/binário

```bash
xxd -l 128 S_Reduce.dds
```

- `-l 128`: limita aos primeiros 128 bytes.

Útil para comparar headers sem abrir arquivos inteiros.

Para DDS, por exemplo, os primeiros bytes normalmente começam com `DDS `.

## `stat`: metadados de um arquivo

```bash
stat caminho/arquivo.tga
```

Mostra tamanho, datas, inode, permissões etc.

É útil para provar que o runtime contém um arquivo específico e distinguir cópias diferentes.

## `sha256sum`: identidade dos bytes

```bash
sha256sum arquivo1 arquivo2
```

Use para:

- BEFORE/AFTER;
- assets recuperados de upstream;
- provar que o arquivo instalado é o mesmo armazenado no bugfix;
- comparar duas cópias sem depender de nome/data.

Comparação rápida:

```bash
sha256sum origem destino
```

Hashes iguais = bytes iguais.

## `comm`: comparar duas listas ordenadas

```bash
comm -3 \
  <(find A -maxdepth 1 -type f -printf '%f\n' | sort) \
  <(find B -maxdepth 1 -type f -printf '%f\n' | sort)
```

`comm` exige entradas ordenadas.

- coluna esquerda: somente na primeira lista;
- coluna direita: somente na segunda;
- `-3`: esconde a coluna dos arquivos presentes nas duas.

Excelente para comparar diretórios de assets entre clientes/forks.

## `git` para investigar origem e regressão

### Commit base

```bash
git rev-parse HEAD
```

### Histórico de um arquivo/path

```bash
git log --all --name-status -- '*S_Reduce.tga'
```

### Histórico completo de um diretório

```bash
git log --all --full-history -- caminho/do/diretorio
```

### Descobrir quando uma linha mudou

```bash
git blame -L 150,170 -- SrcGame/src/sinbaram/HaPremiumItem.cpp
```

### Procurar commits que adicionaram/removeram uma string

```bash
git log -S'S_Reduce' --all --oneline -- SrcGame/src/sinbaram/HaPremiumItem.cpp
```

- `-S`: pickaxe; procura mudança na quantidade daquela string.

Regex de diff:

```bash
git log -G'S_Reduce|STAMINA_REDUCE' --all -p -- arquivo.cpp
```

- `-G`: procura diffs cujas linhas adicionadas/removidas casam com regex.

## Pipes e redirecionamento

### Remover erros irrelevantes

```bash
comando 2>/dev/null
```

- `2>` redireciona stderr.

Use quando permissões/arquivos irrelevantes geram muito ruído. Não use se o erro em si pode ser importante.

### Encadear filtros

```bash
find . -type f | grep -Ei 'reduce|stamina'
```

O `find` reduz o universo; o `grep` classifica o resultado.

## Processo recomendado ao usar comandos

Não comece com a busca mais ampla possível.

Prefira:

```text
1. termo exato no source
2. contexto da linha
3. identificador/enum encontrado
4. função que usa o identificador
5. path/asset final
6. filesystem do runtime
7. upstream/backups somente se necessário
```

Em HD grande:

```text
achar diretórios candidatos
  -> limitar a esses diretórios
  -> buscar nome exato
  -> só então buscar variantes
```
