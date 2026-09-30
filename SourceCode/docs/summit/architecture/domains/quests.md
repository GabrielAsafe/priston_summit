# Domain — Quests

## Responsabilidade

Gerir elegibilidade, início, progresso, objetivos, conclusão e recompensas.

## Separação obrigatória

### Estado dinâmico — autoridade do servidor

- accepted/in progress/completed;
- kill counters;
- timestamps;
- regras de repetição;
- reward grant;
- flags de progressão;
- persistência de personagem.

### Referência estática — conteúdo

- título;
- lore/resumo;
- objetivos documentados;
- NPCs;
- monstros;
- mapas;
- prerequisites;
- itens;
- recompensas documentadas;
- notas.

## Arquitetura atual

A source REC174 organiza responsabilidades de quest em componentes v2, incluindo repository/view/lifecycle sobre a persistência existente.

## Quest Codex

O Codex deve enriquecer a apresentação **somente quando a entrada está vinculada a uma quest real do runtime**. Conteúdo externo não pode criar automaticamente estado de quest.

## Tier 5

A implementação Rank5/Tier5 existente precisa de validação de equivalência com o guia PT-BR antes de declarar o catálogo externo como autoridade de gameplay.

## Recompensas

Testes funcionais devem distinguir:

- inventário;
- PostBox/Distribuidor;
- Gold/EXP;
- Coin;
- TCoin;
- rewards especiais/hardcoded.
