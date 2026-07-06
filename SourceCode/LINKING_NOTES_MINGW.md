# SummitTale — Fase de Linkagem MinGW

## Estado antes da linkagem

Compilação estabilizada com:

```text
202 cpp
202 objetos
0 erros de compilação
Scripts
link_client_mingw.sh
analyze_link_errors.sh
Logs
build_mingw_client_link.log
build_link_error_report/
Exclusões documentadas
HoSkyOld.cpp
HoOpeningOFF.cpp
FilterEffect.cpp
CChatBox.cpp
Motivos
HoSkyOld.cpp:
  órfão/substituído por HoSky.cpp

HoOpeningOFF.cpp:
  órfão/substituído por HoOpening.cpp

FilterEffect.cpp:
  legacy render DirectDraw/D3D7, incompatível com cliente D3D9 atual

CChatBox.cpp:
  legacy chat/UI incompleto, depende de UI::Window_ptr, UIEventArgs,
  PacketChatBoxMessage, PacketItemLinkChat, EChatColor e handlers ausentes
Próximo objetivo

Executar linkagem e resolver:

undefined reference
multiple definition
cannot find library

sem voltar a chamar isso de compilação.
