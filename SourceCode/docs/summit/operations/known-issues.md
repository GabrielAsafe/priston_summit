# Known Issues

## KI-001 — Wine/MSVCRT `_invalid_parameter`

### Estado

Open / não root-caused.

### Evidência

Foi observado no server durante `[STARTUP][MONSTER]` sem impedir a continuação do startup. Também foi observado no client.

### Política

Não tratar a mensagem isolada como crash determinístico. Capturar call stack e reproduzir antes de alterar lógica.

---

## KI-002 — Client `wined3d_swapchain_resize_buffers`

### Estado

Open / intermitente.

### Evidência observada

```text
wined3d_swapchain_resize_buffers Something's still holding back buffer 0
```

### Hipóteses

- recurso Direct3D ainda referenciado durante resize/reset;
- diferença Wine/wined3d;
- lifecycle incorreto de back buffer/device reset.

Nenhuma hipótese está confirmada.

### Próximo diagnóstico

- reproduzir resize/alt-tab/fullscreen transition;
- ativar logging do device-reset path;
- procurar `GetBackBuffer`/surface references sem release;
- comparar execução Windows nativa, se disponível.

---

## KI-003 — Wine SEH invalid frame / page fault

### Estado

Open.

Foi observado `invalid frame` e exception frame fora dos stack limits. Ainda não há evidência que ligue este crash ao content port, quests ou persistência.

---

## KI-004 — Quest reward gameplay verification

### Estado

Em teste.

Confirmar inventário vs Distribuidor/PostBox, Coin/TCoin, rewards especiais e quests originais.

---

## KI-005 — Tier 5 equivalence

### Estado

Não validado.

Existe implementação Rank5/Tier5 na source, mas a equivalência com o catálogo PT-BR precisa de teste e mapeamento explícitos.
