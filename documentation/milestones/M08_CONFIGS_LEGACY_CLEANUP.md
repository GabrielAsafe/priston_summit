# Milestone M08 — Configs e resíduos legados

## Objetivo

Separar conteúdo de gameplay, configuração operacional e secrets; aposentar arquivos físicos que já possuem fonte canônica sem remover dependências reais.

## `/Servidor/Configs`

- `ExpManager.ini`: gameplay; migrar para canonical/materializado;
- `Sistemas.ini`: mistura gameplay, mutable runtime e credenciais; separar, sem colocar secrets em pack compartilhado;
- `hotuk.ini`: bootstrap/topologia + controles; manter server-only e extrair apenas gameplay adequado;
- `Welcome.ini`: nenhum consumidor relevante encontrado na source fornecida; validar com trace antes de remover.

## `/Servidor/GameServer/Monster/*.inf`

Canonical + `definitions_server.scfg` já é o caminho principal na build analisada. Após parity final, mover `.inf` para import/archive, não production runtime.

## `GameserverFilesToPortJson.zip`

Preservar diferenças reais como `legacy_variants`; não sobrescrever canonical automaticamente. `120_Acero.inf` precisa de identidade/reconciliação específica porque os stats divergem fortemente do Acero já canônico.

## `/Servidor/Field`

É asset repository de geometry/material, não config de gameplay. Manter assets necessários e referenciá-los por map content; não JSON-izar mesh.

## Critérios de aceite

- secrets não entram em Content packs;
- gameplay config possui source of truth canônica;
- legado removido apenas após runtime trace/parity;
- servidor de produção não depende de `.inf` de monstro quando strict compiled definitions está ativo.
