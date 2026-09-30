# Constraints

## Plataforma

- Runtime principal: executáveis Windows de 32 bits.
- Build atual: MinGW i686.
- Ambiente de desenvolvimento/teste Linux executa cliente/servidor com Wine.
- Wine pode introduzir sinais/erros próprios que devem ser separados de bugs determinísticos do jogo.

## Compatibilidade

- A codebase contém fronteiras legacy ainda necessárias durante migração incremental.
- Protocolos antigos continuam relevantes para login/session e sistemas não migrados.
- Adapters podem existir temporariamente, mas devem ter ownership e plano de remoção.

## Persistência

- Ambiente atual usa SQLite local em `Servidor/Database/summit.sqlite3`.
- Mudanças destrutivas na persistência exigem migration explícita.
- Alterar manualmente a base não é estratégia de produção.

## Assets

- Canonical JSON é a fonte editável alvo.
- Packs binários são artefactos derivados.
- Ficheiros físicos legacy só podem permanecer como import/recovery quando o runtime correspondente já foi migrado.

## Networking

- Porta observada do servidor: TCP 1997.
- Pre-auth não pode depender de `PlayerId` já registado.
- O router V2 não pode engolir opcodes não registados.

## Build e deploy

- Cliente e servidor precisam de binários e packs compatíveis.
- Deploy deve verificar presença e, quando aplicável, hash dos runtime packs.
