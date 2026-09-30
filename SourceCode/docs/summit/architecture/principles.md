# Architecture Principles

## P1 — Servidor autoritativo

Estado crítico de conta, personagem, quest, economia, inventário e progressão pertence ao servidor. O cliente apresenta e solicita; não é fonte de verdade.

## P2 — Fronteiras explícitas

Networking, persistência, assets e gameplay devem comunicar-se por contratos claros. Adapters legacy são mecanismos de migração, não a arquitetura final.

## P3 — Canonical authoring separado do runtime

Conteúdo editável vive em formatos canonical estruturados. O runtime consome packs compilados e validados.

```text
Canonical JSON
    ↓
schema validation
    ↓
semantic validation
    ↓
compilation
    ↓
runtime packs
    ↓
Game.exe / Server.exe
```

## P4 — Asset Model != Runtime Model

O formato editável não precisa reproduzir estruturas C/C++ legacy. A compilação materializa estruturas otimizadas para runtime.

## P5 — Persistência != gameplay

Gameplay não deve conhecer SQL, layout SQLite ou detalhes de tabelas quando existir uma fronteira de repository/service apropriada.

## P6 — Unknown network traffic must preserve compatibility boundaries

Um router moderno só pode consumir opcodes que ele possui. Pacotes não reconhecidos precisam cair na boundary compatível correta, inclusive antes da autenticação.

## P7 — Migrações são verificáveis e reversíveis quando possível

Mudanças de formatos, protocolos, assets ou persistência devem incluir validação, estratégia de rollback e critério de remoção do legado.

## P8 — Ferramentas usam os mesmos contratos dos runtimes

O Asset Tool deve editar canonical e usar validators/compilers oficiais. Não deve existir um formato que apenas o runtime compreenda.

## P9 — Erros acionáveis

Falhas críticas devem indicar subsistema, recurso, fase e causa observável. Mensagens genéricas de “failed to load” são insuficientes.

## P10 — Refatoração só termina com operação e documentação

Compilar não é critério suficiente. Testes, runbooks, observabilidade e documentação fazem parte da Definition of Done.
