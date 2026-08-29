# Bugfix 021 — Separadores de caminho incorretos no parser/loader legado

## Resumo

Foi identificado uso da constante multicaractere C/C++ `'//'` em comparações que aparentemente pretendem detectar um separador de diretório. `'//'` não representa `'/'` nem `'\\'`; por isso as condições não executam como esperado quando o código examina um `char` de um caminho.

O defeito afeta principalmente o parser ASE do cliente e uma rotina de extração de diretório usada pela lista de arquivos de personagem.

## Sistema

**Produto:** Summit Tale Client  
**Área principal:** loader 3D / conversão ASE → SMD / resolução de texturas  
**Área relacionada:** descoberta/listagem de arquivos de personagem no cliente  
**Severidade sugerida:** Alta para manutenção e importação de assets; Média em runtime quando já existe SMD pré-convertido.

## Arquivos encontrados

### 1. `SourceCode/SrcGame/src/smLib3d/smRead3d.cpp`

Locais observados no snapshot auditado:

- `SetDirectoryFromFile()` — aproximadamente linha 99
- `ReadASE_MATERIAL()` — aproximadamente linha 425
- `ReadASE_LIGHTMAP_MATERIAL()` — aproximadamente linha 837

Padrões encontrados:

```cpp
if (DataDirectory[cnt] == '//') break;
```

```cpp
if (strBuff[cnt] == '//' || strBuff[cnt] == ':')
```

O primeiro deveria detectar o último separador do diretório. Os outros dois deveriam remover o caminho exportado e manter o nome do bitmap antes de concatená-lo ao diretório do ASE.

### 2. `SourceCode/SrcGame/src/playmain.cpp`

Local observado no snapshot auditado:

- `playmain_SetDirectoryFromFile()` — aproximadamente linha 1154

Padrão encontrado:

```cpp
if (szDataDirectory[cnt] == '//') break;
```

A rotina sofre do mesmo problema ao tentar extrair o diretório de um caminho.

## Risco relacionado de tamanho

Em `SourceCode/SrcGame/src/smLib3d/smType.h` foram encontrados buffers legados de caminho/nome com 64 bytes, incluindo:

```cpp
char Name[64];
char NameA[64];
char BITMAP[8][64];
```

Mesmo corrigindo os separadores, caminhos maiores, nomes longos ou texto multibyte podem atingir essa capacidade. Esta é uma revisão relacionada, não necessariamente a mesma causa-raiz.

## Impacto provável

1. Diretório do arquivo pode ser extraído incorretamente ou ficar vazio.
2. `*BITMAP` de ASE com caminho absoluto ou relativo pode produzir um caminho de textura incorreto.
3. Conversão/importação de ASE pode falhar em localizar texturas.
4. Assets antigos podem parecer depender de pastas erradas durante auditorias estáticas.
5. SMDs pré-convertidos podem mascarar o problema em runtime, porque o loader tende a usar o `.smd` correspondente quando ele já existe.

## Solução recomendada

### Etapa 1 — corrigir o teste de separador

Substituir comparações com `'//'` por testes explícitos para os dois separadores suportados:

```cpp
if (DataDirectory[cnt] == '/' || DataDirectory[cnt] == '\\')
    break;
```

E, no parser de `*BITMAP`:

```cpp
if (strBuff[cnt] == '/' || strBuff[cnt] == '\\' || strBuff[cnt] == ':')
{
    cnt++;
    break;
}
```

Aplicar o mesmo princípio em `playmain_SetDirectoryFromFile()`.

### Etapa 2 — centralizar a lógica

Evitar manter três implementações independentes da mesma regra. Criar uma função auxiliar única para:

- encontrar último `/` ou `\\`;
- lidar com caminho sem diretório;
- lidar com `C:\\...`;
- retornar dirname e basename com capacidade conhecida.

Se a compatibilidade permitir, considerar `std::filesystem::path`; caso contrário, usar uma função C/C++ pequena e testada.

### Etapa 3 — revisar capacidade dos buffers

Auditar pelo menos:

- `DataDirectory`;
- `strFile`;
- `smTEXTUREHANDLE::Name`;
- `smTEXTUREHANDLE::NameA`;
- `aseMaterial[].BITMAP[][]`.

Toda concatenação deve conhecer a capacidade real do destino. Não assumir que 64 bytes são suficientes para caminhos atuais ou Unicode/multibyte.

### Etapa 4 — testes unitários/fixtures

Testar a função de parsing com:

```text
texture.bmp
folder/texture.bmp
folder\\texture.bmp
C:\\project\\folder\\texture.bmp
C:/project/folder/texture.bmp
folder com espaço\\texture.bmp
```

Adicionar também um nome com caracteres portugueses e, se o runtime suportar a code page/Unicode escolhida, um nome coreano.

### Etapa 5 — teste de integração ASE

Usar uma cópia de teste de um asset conhecido, por exemplo `death_knight.ASE` ou `ji_woo.ASE`:

1. manter backup/quarentena do SMD correspondente;
2. forçar o caminho que faz leitura/conversão do ASE;
3. verificar quais nomes de bitmap são produzidos;
4. confirmar que todas as texturas são abertas a partir do diretório correto;
5. regenerar o SMD;
6. reiniciar e validar o asset já pelo SMD gerado.

Não fazer esse teste diretamente no cliente de produção sem backup reversível.

### Etapa 6 — regressão do cliente

Validar:

- inicialização;
- seleção de personagem;
- NPCs/personagens que usam ASE/SMD;
- mapas que possuem objetos animados;
- logs de textura/modelo ausente;
- carregamento com `/` e `\\`.

## Critério de aceite

O bugfix pode ser considerado concluído quando:

1. não existir comparação de `char` com `'//'` nas rotinas auditadas;
2. dirname/basename funcionarem para `/` e `\\`;
3. ASE de teste resolver suas texturas no diretório esperado;
4. SMD gerado carregar sem regressão;
5. nenhuma nova falha de path/buffer aparecer nos testes de cliente.

## Rollback

Manter a alteração em commit isolado. Em caso de regressão, reverter o commit e restaurar qualquer SMD/asset de teste a partir da quarentena/backup.

## Relação com a auditoria de assets

Esta descoberta foi feita ao investigar se `Field/old_forest` e `Field/old_dungeon` funcionavam como fallback de texturas. O loader analisado não demonstrou busca global nessas pastas. A auditoria também mostrou que referências `.ASE` no código frequentemente resolvem para `.smd` no mesmo caminho, portanto buscas estáticas por basename não devem ser usadas sozinhas para decidir se um asset está ativo.
