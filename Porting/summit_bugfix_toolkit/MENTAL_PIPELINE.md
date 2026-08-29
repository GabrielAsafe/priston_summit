# Pipeline Mental para Resolver Bugs

Use este roteiro como uma máquina de estados mental. A pergunta principal é sempre: **qual é o próximo elo ainda não provado?**

## Pipeline de 10 passos

### 1. OBSERVAR

Defina o comportamento observável.

```text
O que funciona?
O que não funciona?
Quando acontece?
Cliente ou servidor?
```

Nunca comece pela explicação.

### 2. CLASSIFICAR

Escolha o domínio mais provável:

```text
lógica
UI/render
asset
path/filesystem
dados/parser/encoding
rede/protocolo
persistência/save
build/deploy
```

Essa classificação só decide onde procurar primeiro; não prova causa.

### 3. ANCORAR

Ache uma âncora concreta:

```text
string visível
nome de função
CODE de item
opcode
enum
nome de asset
mensagem de log
símbolo de crash
```

Uma âncora transforma “o jogo está estranho” em algo pesquisável.

### 4. SEGUIR

Siga o dado para frente e para trás.

Perguntas:

```text
Quem cria isso?
Quem escreve esse campo?
Quem lê esse campo?
Qual ID representa isso?
Qual função desenha/carrega/envia/salva isso?
```

Não pule de uma string de interface direto para um patch.

### 5. CONSTRUIR A CADEIA

Escreva em texto simples:

```text
A -> B -> C -> D -> falha
```

Se você não consegue escrever a cadeia, ainda falta investigação.

### 6. TESTAR HIPÓTESES BARATAS

Antes de editar:

```text
arquivo existe?
path bate?
case bate?
ID bate?
índice bate?
asset está no runtime?
valor está zerado?
função é realmente chamada?
```

Procure evidência que possa **refutar** sua hipótese, não só confirmá-la.

### 7. COMPARAR

Quando algo parece perdido/corrompido:

```text
runtime atual vs source
atual vs before
atual vs upstream
cliente A vs cliente B
arquivo esperado vs arquivo presente
```

A comparação deve ser estreita e dirigida pelo elo faltante.

### 8. PROVAR

Marque a causa como provada apenas quando houver:

```text
sintoma
+ caminho de execução/dados
+ ponto defeituoso
+ observação objetiva do defeito
```

Exemplo:

```text
STAMINA_REDUCE_P = 13
renderer usa [13 - 1]
slot 12 pede S_Reduce.tga
arquivo não existe
=> imagem do buff não pode ser carregada
```

### 9. CORRIGIR O MENOR ELEMENTO

Pergunte:

```text
Qual é a menor coisa que, se corrigida, restaura a intenção original?
```

Se falta um asset legítimo, restaure o asset. Não reescreva o renderer.

Se um path tem uma letra errada, corrija o path. Não crie fallback global sem necessidade.

### 10. VALIDAR E ARQUIVAR

Validação em três níveis:

```text
estático: diff/hash/path/formato
build: compila/linka quando aplicável
funcional: o sintoma original desapareceu
```

Depois arquive BEFORE/AFTER, evidências e proveniência.

## Heurística rápida: "efeito funciona, visual não"

Se a lógica funciona mas algo visual não aparece:

```text
estado está ativo?
  sim
-> ID/slot visual existe?
-> material/textura foi criado?
-> asset existe no runtime?
-> renderer usa o slot?
-> alpha/visible/posição permitem desenho?
```

## Heurística rápida: "funciona no Windows antigo, falha no ambiente atual"

Verifique primeiro:

```text
case-sensitive
path absoluto/relativo
barra / vs \\
extensão/nome real
encoding/locale
32-bit vs 64-bit
arquivo ausente no deploy
```

Mas trate cada item como hipótese até comparar com runtime e loader.

## Heurística rápida: crash

```text
símbolo/endereço
-> função
-> argumento/ponteiro
-> origem do dado
-> condição que permite valor inválido
-> guarda mínima ou correção na origem
```

Prefira corrigir a origem do estado inválido quando ela for conhecida. Use null guard quando o contrato permite ausência ou como proteção adicional comprovadamente necessária.

## Pergunta de ouro

Quando estiver travado, escreva:

> "Para provar minha hipótese, qual fato eu ainda preciso observar?"

Depois crie **um comando** que responda somente a esse fato.

Isso impede buscas gigantes e reduz a chance de se perder no projeto.
