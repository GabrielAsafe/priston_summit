# Milestone M11 — Reduzir `/Servidor/char` com segurança

## Objetivo

Parar de distribuir uma cópia quase completa de assets visuais no servidor sem quebrar model/motion dependencies.

## Estado confirmado

O servidor atual ainda depende de parte de `char`:

- `char/tmABCD` é referenciado por criação/aparência/equipamento;
- mobs/NPCs apontam para `char/monster` / `char/npc`;
- `smModelDecode` consome model config/cache;
- models/motions SMD/SMB são abertos;
- em caminhos com `renderDevice`, o loader ainda pode carregar texturas.

Logo, apagar a pasta agora não é seguro.

## Fase 1 — Manifest render-safe

Gerar dependências a partir do canonical e manter diretórios/assets de cada model config ativo. Validar boot/login/spawn/event mobs.

Ferramenta incluída: `tools/build_server_char_manifest.py`.

## Fase 2 — Model loader headless

Criar uma via server-only que carregue somente:

- model identity;
- geometry/collision se realmente necessária;
- motion metadata necessária a estados;
- sem material/texture upload.

Remover `ReadTextures()` e dependência gráfica do caminho server.

## Fase 3 — Manifest runtime-core

Depois do headless loader, gerar pacote mínimo de configs/cache + SMD/SMB necessários. Texturas/previews permanecem só no cliente.

## Critérios de aceite

- servidor inicia sem assets client-only removidos;
- todos mobs/NPCs/classes ativos carregam;
- collision/combat parity;
- nenhum file-open fora do manifest durante suite/staging;
- ausência de texture renderer no server path;
- missing references list fica zerada ou explicitamente justificada.
