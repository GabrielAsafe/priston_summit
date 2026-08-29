# BUGFIX 045 — Legacy multi-character path separator comparisons

## Resultado

Foram identificadas 24 comparações de caracteres usando incorretamente o
literal multi-character:

    '//'

Essas comparações foram substituídas por verificações explícitas dos dois
separadores de caminho aceitos pelo código legado:

    '/'
    '\\'

## Validação

- 24 ocorrências identificadas antes da correção.
- 0 ocorrências do padrão defeituoso após a correção.
- Diff isolado: 24 linhas removidas / 24 linhas adicionadas.
- Build concluído com sucesso.
- Servidor iniciou sem crash.
- Teste funcional revelou recuperação de NPCs que anteriormente não abriam
  interfaces de loja/armazém.

## Impacto observado

A correção resolveu indiretamente parte dos problemas de interação com NPCs,
especialmente casos envolvendo resolução/manipulação de caminhos.

Alguns NPCs ainda apresentam o problema de loja/armazém não abrir.

Isso indica que BUGFIX 045 corrigiu uma causa real, mas não representa a causa
única do problema restante.

## Próxima investigação

Usar um NPC funcional e um NPC ainda defeituoso como pares de controle e
comparar:

- definição do NPC;
- código de interação;
- shop/warehouse identifiers;
- arquivos de configuração;
- paths referenciados;
- carregamento dos assets/dados;
- mensagens cliente/servidor;
- condições que determinam abertura da interface.

Não ampliar BUGFIX 045 para tentar resolver esses casos.
Eles devem ser tratados como novos bugs independentes.
