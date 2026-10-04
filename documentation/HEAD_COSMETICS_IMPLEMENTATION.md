# Capacetes e máscaras

## Estado atual

Não existe slot HEAD/HELMET/MASK no sistema legado de item placement. Existem slots de costume e brincos, e existem assets de máscara como `815mask_M` / `815mask_W`, além de referências históricas de modelo com comentário `mascara`, mas isso não equivale a um sistema de equipamento de cabeça.

## Escolha recomendada

Criar um slot **`head_cosmetic` sem stats de combate** primeiro. Não misturar a primeira implementação com helmet defensivo.

Motivos:

- menor risco de balance/protocolo;
- resolve máscaras/capacetes visuais;
- permite validar save/network/render antes de adicionar item stats;
- evita conflito com `COSTUME`.

`Content/canonical/head_cosmetics/head_cosmetics.json` registra o contrato proposto e os assets 815 conhecidos.

## Compatibilidade

O próximo bit aparentemente livre após earrings é `0x00020000`, mas ele está marcado como **proposto**, não autorizado automaticamente. Antes de usar, auditar:

- todos os masks de slot;
- serialize/save/load de inventory;
- packet structs;
- trade/warehouse/shop;
- aging/mix/crafting;
- equip validation;
- character model replication;
- UI inventory/equipment;
- item tooltip.

Somente após essa auditoria transformar o bit em contrato.

## Runtime necessário

1. adicionar `ITEM_POS_HEAD_COSMETIC` nos headers compartilhados client/server;
2. incluir o slot no save/load/equip validation;
3. incluir item identity/model variant no character appearance snapshot;
4. renderizar head overlay/model config compatível no client;
5. definir fallback quando asset estiver ausente;
6. garantir que o servidor valida o item equipado e não aceita packet client-side arbitrário.

## Futuro helmet com stats

Depois do slot cosmético estar estável, um helmet defensivo pode reutilizar o slot ou virar `head_armor`, mas a decisão deve ser explícita. Se máscaras e helmets precisarem coexistir, use dois slots lógicos e não uma flag ambígua.
