BUG-005 — RESOLVIDO / TESTE RUNTIME PENDENTE

Correção:
- alinhado SACER_INFO do client ao wire format atual do server;
- removidos 7 campos obsoletos da estrutura client;
- removidas leituras inválidas de HolyBolt_Damage e Glacial_Spike_Damage;
- restaurada sincronização de HolyBolt_DamagePercent;
- restaurada sincronização de Glacial_Spike_Damage_Percent.

Validação:
- contrato client/server revisado campo a campo;
- tabelas locais de dano comparadas e idênticas;
- build client OK;
- link client OK;
- diff check OK.

Pendente:
- validar em personagem com Holy Bolt / Glacial Spike disponíveis;
- confirmar percentuais exibidos no tooltip contra Sacer.ini.

Se houver divergência em runtime, reabrir BUG-005 para novo diagnóstico.
