# Runbook — Deployment

## 1. Validar canonical

Executar validators relevantes antes de build.

## 2. Compilar conteúdo

Usar o pipeline oficial `build_from_canonical.py` e family compilers.

## 3. Inspecionar packs

Executar inspectors e runtime contract validation.

## 4. Compilar executáveis

Build/link de client e server.

## 5. Deploy

Copiar executáveis, DLLs e packs para os roots de runtime.

## 6. Smoke server

Confirmar:

- SQLite abriu;
- definitions carregou;
- maps carregou;
- listener 1997 abriu.

## 7. Smoke client

Confirmar:

- startup;
- login;
- character select;
- map entry;
- interação NPC.

## 8. Gameplay regressions

Executar checklist dos domínios alterados.
