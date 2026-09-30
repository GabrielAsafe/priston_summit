# Runbook — Database Recovery

## Antes de alterar

1. parar writes quando necessário;
2. identificar DB correta;
3. criar backup;
4. verificar integridade SQLite;
5. registar migration/version.

## Login troubleshooting

Validar separadamente:

- conta existe;
- `inuse`;
- block/delete flags;
- auth policy;
- password policy;
- listener;
- TCP;
- packet routing.

Um TCP saudável com login falhando aponta para protocolo/auth/session, não automaticamente para corrupção da DB.
