# Domain — Networking

## Responsabilidade

Transportar packets entre cliente e servidor preservando boundaries de sessão, autenticação e gameplay.

## Componentes relevantes

- socket/transport legacy;
- `network/v2`;
- `PacketRouter`;
- handlers de gameplay;
- registry que associa conexão a `PlayerId`;
- fallback legacy de session/login.

## Invariante crítica

**Opcode desconhecido pelo router moderno deve retornar `UnknownOpcode` antes de exigir `PlayerId`.**

### Motivo

Durante pré-autenticação ainda pode não existir um `PlayerId` válido. Validar player antes de determinar ownership do opcode transforma um pacote legacy válido em `InvalidPacket` e impede o fluxo de login.

## Contrato do router

| Opcode | Player | Resultado |
|---|---|---|
| moderno registado | válido | handler moderno |
| moderno registado | inválido | `InvalidPacket` |
| desconhecido/legacy | válido | `UnknownOpcode` |
| desconhecido/legacy | pré-auth/inválido | `UnknownOpcode` |

## Testes obrigatórios

- known + valid;
- known + invalid;
- unknown + valid;
- unknown + invalid/pre-auth.
