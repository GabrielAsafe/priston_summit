# System Context

```text
                         ┌─────────────────────┐
                         │   Asset Tool / CI   │
                         └──────────┬──────────┘
                                    │ canonical JSON
                                    ▼
                         ┌─────────────────────┐
                         │ Asset Build Pipeline │
                         └──────────┬──────────┘
                                    │ compiled packs
                 ┌──────────────────┴──────────────────┐
                 ▼                                     ▼
        ┌─────────────────┐                   ┌─────────────────┐
        │   Game Client   │◄──── TCP 1997 ───►│   Game Server   │
        │    Game.exe     │                   │   Server.exe    │
        └─────────────────┘                   └────────┬────────┘
                                                      │
                                                      ▼
                                             ┌─────────────────┐
                                             │ SQLite / repos  │
                                             │ summit.sqlite3  │
                                             └─────────────────┘
```

## Game Client

Responsável por apresentação, input, UI, renderização e protocolo cliente-servidor. Não é autoridade de estado crítico.

## Game Server

Autoridade de gameplay e persistência. Inicializa conteúdo compilado, mapas, NPCs, items e networking antes de aceitar sessões.

## Authentication / session boundary

O login ainda depende de compatibilidade com protocolo legacy. O router gameplay V2 opera depois/ao lado dessa boundary e só deve consumir opcodes explicitamente registados.

## Persistence

SQLite é a implementação local observada. Domínios migrados devem utilizar repositories/services em vez de espalhar SQL pelo gameplay.

## Asset Pipeline

Transforma canonical editável em runtime packs. Validation e compilation fazem parte do contrato.

## Asset Tool

Arquitetura alvo: canonical-first. Writers legacy ficam restritos a import/recovery durante a transição.
