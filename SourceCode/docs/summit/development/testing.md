# Testing Strategy

## Pirâmide prática

### Contract / unit

- PacketRouter;
- repositories;
- pack readers;
- serializers;
- validators.

### Asset smoke

- compile;
- inspect;
- C++ reader;
- hash/provenance.

### Integration

- Server startup;
- login;
- map loading;
- NPC interaction;
- quest lifecycle;
- reward grant;
- PostBox;
- Coin/TCoin.

### Runtime regression

- Navisko day/night;
- Field start points;
- fixed NPC visibility/click;
- monsters/spawns;
- shops;
- map transitions;
- Quest Codex/Mix UI.

## Gate de release

Uma alteração estrutural deve possuir pelo menos:

1. build;
2. automated contract test;
3. startup smoke;
4. login smoke quando networking/persistence mudar;
5. gameplay test do domínio afetado.
