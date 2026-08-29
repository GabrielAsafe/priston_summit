# SummitTale — Retomada do Desenvolvimento do Cliente

**Data:** Julho de 2026

---

# Estado Atual do Projeto

O projeto SummitTale encontra-se atualmente dividido em duas partes principais:

- Servidor
- Cliente

O trabalho realizado nos últimos meses permitiu concluir com sucesso a migração do servidor para MinGW.

Neste momento o foco deixou de ser a compilação do servidor e passou integralmente para a estabilização do cliente e posterior ligação ao servidor.

---

# Estado Atual do Servidor

O servidor encontra-se funcional.

Foi confirmado que:

- compila integralmente em MinGW i686;
- o executável utiliza o subsistema Windows Console (`-mconsole`);
- a ligação ao SQL Server funciona;
- todos os mapas são carregados;
- NPCs e itens são carregados corretamente;
- `InitAll()` termina;
- o servidor entra no ciclo principal de mensagens Windows;
- permanece em execução;
- abre a porta TCP;
- encontra-se preparado para aceitar ligações.

A investigação do servidor encontra-se concluída.

Documentos relevantes:

- docs/04_SERVER.md
- docs/milestone_servidorcompilado_funcional.md

---

# Estado Atual do Cliente

A compilação do cliente encontra-se concluída.

Toda a fase de migração para MinGW foi terminada.

Os problemas de:

- compilação;
- linkagem;
- DirectX;
- Lua;
- símbolos indefinidos;

foram resolvidos durante a migração.

Essa documentação permanece apenas como referência histórica em:

docs/deprecated/

---

# Evolução da Investigação do Cliente

## Fase 1

O cliente não iniciava.

Não existia qualquer janela.

Não existiam mensagens de debug.

Objetivo:

Descobrir se o processo chegava ao WinMain.

Resultado:

Foram adicionadas MessageBox e instrumentação.

---

## Fase 2

Confirmou-se que o WinMain era executado.

O executável iniciava corretamente.

Resultado:

O problema deixou de ser o arranque do processo.

---

## Fase 3

O cliente passou a abrir.

Contudo encerrava quando começava a receber input.

A origem exata ainda não foi identificada.

---

## Estado Atual

O cliente:

- inicia;
- cria a janela;
- permanece ativo até existir interação do utilizador.

Atualmente foi observado que:

- pressionar ENTER provoca imediatamente o encerramento do cliente.

Ainda não foi determinado se o encerramento é provocado por:

- exceção;
- processamento de mensagens Windows;
- callback de teclado;
- erro interno;
- encerramento intencional.

---

# Objetivo Atual

Antes de investigar qualquer problema de rede é necessário estabilizar completamente o cliente.

O cliente deve:

- iniciar;
- permanecer aberto;
- aceitar teclado e rato;
- apresentar o ecrã de login;
- apenas depois tentar estabelecer ligação TCP.

Enquanto o cliente terminar ao receber input, qualquer investigação da ligação cliente-servidor será inconclusiva.

---

# Próxima Investigação

A prioridade absoluta passa por descobrir porque motivo o cliente termina quando ENTER é pressionado.

Os principais pontos de instrumentação serão:

- WinMain()
- Loop principal
- PeekMessage()
- GetMessage()
- TranslateMessage()
- DispatchMessage()
- WndProc()
- WM_KEYDOWN
- WM_CHAR
- WM_COMMAND
- WM_QUIT
- PostQuitMessage()
- DestroyWindow()

Também deverá ser confirmada a existência de:

- exceções C++;
- SEH;
- chamadas para ExitProcess();
- TerminateProcess();
- abort();
- std::terminate();
- return antecipado do WinMain.

---

# Apenas Depois

Quando o cliente permanecer estável será iniciada a investigação da ligação ao servidor.

A sequência esperada será:

1. socket()
2. resolução do endereço
3. connect()
4. accept() no servidor
5. primeiro pacote
6. opcode inicial
7. resposta do servidor

Toda esta fase encontra-se documentada em:

docs/09_CLIENT_SERVER_CONNECTION.md

---

# Documentação Ativa

Os documentos atualmente relevantes são:

README.md

MINGW_PORT_STATUS_2026_07.md

investigando cliente e server não conectarem.md

opcodes.md

docs/04_SERVER.md

docs/05_CLIENT.md

docs/08_TROUBLESHOOTING.md

docs/09_CLIENT_SERVER_CONNECTION.md

docs/milestone_servidorcompilado_funcional.md

---

# Documentação Histórica

A documentação da migração foi preservada apenas para consulta.

Encontra-se em:

docs/deprecated/

Não deve ser utilizada como estado atual do projeto.

---

# Critério para Conclusão desta Fase

Esta fase será considerada concluída quando:

- o cliente permanecer aberto indefinidamente;
- o teclado funcionar corretamente;
- o rato funcionar corretamente;
- o ecrã de login permanecer ativo;
- não existir encerramento ao pressionar ENTER.

Após esse objetivo será iniciada a fase de comunicação cliente-servidor.

---

# Próximo Marco

Primeira ligação TCP entre o cliente e o servidor MinGW com receção do primeiro pacote de login.
