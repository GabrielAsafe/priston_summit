# Summit Tale — Web e Painéis

## Raízes Docker

```text
docker/web/wwwroot - BPT
docker/web/wwwroot - EPIC SIMPLE
```

## Containers

```text
summit-web-bpt
summit-web-epic
```

## Portas

```text
BPT:  8088 -> 80
EPIC: 8089 -> 80
```

## URLs testadas

```text
http://localhost:8088/
http://localhost:8089/padm/
http://localhost:8089/painel/
```

Resultado: HTTP 200 OK.

## Próximos pontos

- Centralizar configs PHP.
- Procurar IPs hardcoded.
- Validar login dos painéis.
- Corrigir charset se necessário.
