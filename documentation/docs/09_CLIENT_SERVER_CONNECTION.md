# Conexão cliente-servidor

## Objetivo

Obter a primeira conexão TCP funcional entre o cliente SummitTale e o servidor compilado em MinGW.

## Ambiente

### Servidor

- Sistema operativo: Windows
- Executável: `Server.exe`
- Compilador: MinGW i686
- Subsistema: Windows Console
- IP: a confirmar
- Porta: `9000/TCP`
- SQL Server: conectado

### Cliente

- Sistema operativo: Windows
- Executável: a confirmar
- Compilador: a confirmar
- IP configurado: a confirmar
- Porta configurada: a confirmar

## Fluxo esperado

1. O cliente inicia.
2. O cliente resolve o endereço.
3. O cliente cria um socket.
4. O cliente executa `connect()`.
5. O servidor executa `accept()`.
6. O cliente envia o pacote inicial.
7. O servidor interpreta o opcode.
8. O servidor envia uma resposta.
9. O cliente prossegue para o login.

## Instrumentação do cliente

- `socket()`;
- resolução de endereço;
- `connect()`;
- `send()`;
- `recv()`;
- `shutdown()`;
- `closesocket()`.

## Instrumentação do servidor

- `bind()`;
- `listen()`;
- `accept()`;
- `recv()`;
- dispatch de pacotes;
- opcode desconhecido;
- `send()`;
- encerramento do socket.

## Critério de conclusão

- IP e porta usados pelo cliente;
- sucesso de `connect()`;
- execução de `accept()`;
- IP remoto aceite;
- primeiro pacote recebido;
- tamanho e opcode do pacote;
- resposta enviada pelo servidor.

## Registo de testes

### Teste 1

- Data:
- Build do cliente:
- Build do servidor:
- IP:
- Porta:
- Resultado no cliente:
- Resultado no servidor:
- Captura de rede:
- Conclusão:
