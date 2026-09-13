# ENTREGA COMPLETA — Summit / SQLite

Data: 2026-09-06

Este ZIP foi criado a pedido do utilizador para preservar **tudo o que estava disponível no diretório de trabalho da sessão**.

## Estado principal

- **R6.1** continua sendo a baseline funcional comprovada pelo utilizador (compile/link/runtime e equivalência com `pre_refactor`).
- **R16** é o último checkpoint SQLite formalmente fechado e empacotado antes do trabalho R17.
- **R17** está presente em `r17_work/` e contém o estado de desenvolvimento mais recente, incluindo a redução da superfície HTTP do servidor. O último passo de R17 ainda não tinha sido novamente congelado/empacotado como checkpoint formal após as últimas neutralizações WinInet.
- O schema SQLite desenvolvido chegou a migrations **v29 / 332 tabelas**, cobrindo as **326 tabelas recuperadas dos backups SQL Server** e contratos adicionais source-derived documentados.

## Ficheiros essenciais para retomar

1. `r17_work/RETOMADA_COMPLETA_R17.md` — retomada detalhada e consolidada.
2. `r17_work/CONHECIMENTO_CONSOLIDADO_CHAT_R17.md` — conhecimento acumulado durante a conversa.
3. `RETOMADA(2)(1).md` — checkpoint/retomada original que iniciou esta sessão.
4. `summit_sqlite_wip_r16_2026-09-06.zip` — último checkpoint SQLite fechado.
5. `r17_work/` — árvore de trabalho mais recente.
6. `summit_full_checkpoint_2026-09-06(1).zip` — checkpoint completo original recebido.
7. `ServidorDBS.zip`, `db-fixed(1).zip`, `db_baks/` — materiais SQL Server/backups usados para reconstruir schemas.
8. `MANIFEST_SHA256_ENTREGA_COMPLETA.txt` — hashes de todos os ficheiros incluídos (exceto o próprio ZIP final e seu hash).

## Conteúdo preservado

O pacote inclui, sem tentar deduplicar:

- uploads originais do utilizador;
- checkpoints R2/R9/R11/R12/R13/R14/R15/R16 disponíveis;
- árvores R14/R15/R16/R17 e cópias usadas para validação de patches;
- `.bak` extraídos;
- migrations, patches, manifests e validadores;
- relatórios de validação;
- RETOMADAS e READMEs históricos;
- inventários SQL/HTTP disponíveis;
- material de referência `database.zip`, `files.zip`, `web-legado.md`;
- outputs e estados intermediários que ainda estavam presentes em `/mnt/data`.

## Regra para próxima sessão

Ler primeiro:

1. `r17_work/RETOMADA_COMPLETA_R17.md`
2. `r17_work/CONHECIMENTO_CONSOLIDADO_CHAT_R17.md`
3. `RETOMADA(2)(1).md`

Não promover R17/R16 a baseline funcional sem compile/link/runtime Windows. Não alterar gameplay enquanto a migração de persistência estiver sendo concluída.
