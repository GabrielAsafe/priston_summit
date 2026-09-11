Como estamos usando o Git

O fluxo daqui para frente fica assim:

HEAD = último estado funcional aprovado. Agora é:

85de906c fix(client): activate starter premium items on first login
Um bug → uma investigação → um fix → um commit. Nada de misturar Guide, QuestTimer, Tier, starter gear etc.

Antes de alterar código:

git rev-parse HEAD

registramos o commit-base, fazemos before/ e SHA-256 dos ficheiros que realmente serão tocados.

Fazemos primeiro probe/investigação sem alterar comportamento. Só quando a causa estiver PROVADA fazemos o patch funcional mínimo.

Antes de build:

git diff --check -- caminho/do/ficheiro
git diff -- caminho/do/ficheiro
Buildamos somente o componente afetado e testamos no jogo.

Só depois do runtime aprovado:

git add caminho/exato1 caminho/exato2
git diff --cached --check
git diff --cached
git commit -m "..."
Nunca git add ., nunca reset --hard por conveniência e nunca empilhar um novo fix sobre uma regressão.

Um detalhe do 056: o git diff --cached --check acusou whitespace legado dentro de after/netplay.cpp, mas o commit foi feito mesmo assim. Não precisamos reescrever esse commit agora. Nos próximos fixes vou separar melhor a validação do ficheiro funcional/patch dos snapshots históricos para não repetirmos isso.



