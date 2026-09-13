#include <cstring>
#include <windows.h>
#include <stdio.h>
#include "systems/audio/voice/legacy_ygy/Util.h"

void MiLae_Debug_txt(char *fmt, ...)
{
	char Buff[4096];
	ZeroMemory(Buff, sizeof(Buff));

	va_list args;
	HANDLE hFile;
	DWORD dwAcess = 0;

	hFile = CreateFile("LOGS\\game_debug.txt", GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
					   NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if(hFile == INVALID_HANDLE_VALUE) return;




	va_start(args, fmt);
	vsprintf(Buff, fmt, args);
	va_end(args);

	strcat(Buff, "/r/n");
	SetFilePointer(hFile, 0, NULL, FILE_END);
	WriteFile(hFile, Buff, lstrlen(Buff), &dwAcess, NULL);
	CloseHandle(hFile);

}

void ErrorMessage(char *msg, ...)
{
	char Buff[4096];
	va_list args;

	va_start(args, msg);
	vsprintf(Buff, msg, args);
	va_end(args);

	MessageBox(NULL, Buff, "Error Message", MB_OK | MB_ICONWARNING);


}

void ReadScript(char *filename)
{
}

VOID SocketError(HWND Wnd, DWORD err)
{
#define CAPTION  "WinSock Error"
	char szTemp[256];
	switch(err)
	{
		case 10004:
		MessageBox(Wnd, "Opera\347\343o bloqueante interrompida.", CAPTION, MB_OK | MB_ICONSTOP);
		break;
		case 10009:
		MessageBox(Wnd, "Identificador de socket inv\341lido.", CAPTION, MB_OK | MB_ICONSTOP);
		break;
		case 10013:
		MessageBox(Wnd, "Permiss\343o negada.", CAPTION, MB_OK | MB_ICONSTOP);
		break;
		case 10014:
		MessageBox(Wnd, "Endere\347o de mem\363ria inv\341lido.", CAPTION, MB_OK | MB_ICONSTOP);
		break;
		case 10022:
		MessageBox(Wnd, "Argumento inv\341lido.", CAPTION, MB_OK | MB_ICONSTOP);
		break;
		case 10024:
		MessageBox(Wnd, "Muitos arquivos ou sockets abertos.", CAPTION, MB_OK | MB_ICONSTOP);
		break;
		case 10035:
		MessageBox(Wnd, "Opera\347\343o n\343o pode ser conclu\355da imediatamente.", CAPTION, MB_OK | MB_ICONSTOP);
		break;
		case 10036:
		MessageBox(Wnd, "J\341 existe uma opera\347\343o bloqueante em andamento.", CAPTION, MB_OK | MB_ICONSTOP);
		break;
		case 10037:
		MessageBox(Wnd, "A opera\347\343o solicitada j\341 est\341 em andamento.", CAPTION, MB_OK | MB_ICONSTOP);
		break;
		case 10038:
		MessageBox(Wnd, "Opera\347\343o de socket aplicada a um descritor que n\343o \351 socket.", CAPTION, MB_OK | MB_ICONSTOP);
		break;
		case 10039:
		MessageBox(Wnd, "Endere\347o de destino obrigat\363rio.", CAPTION, MB_OK | MB_ICONSTOP);
		break;
		case 10040:
		MessageBox(Wnd, "Mensagem de rede grande demais.", CAPTION, MB_OK | MB_ICONSTOP);
		break;
		case 10041:
		MessageBox(Wnd, "Tipo de protocolo incorreto para este socket.", CAPTION, MB_OK | MB_ICONSTOP);
		break;
		case 10042:
		MessageBox(Wnd, "Op\347\343o de protocolo inv\341lida.", CAPTION, MB_OK | MB_ICONSTOP);
		break;
		case 10043:
		MessageBox(Wnd, "Protocolo n\343o suportado.", CAPTION, MB_OK | MB_ICONSTOP);
		break;
		case 10044:
		MessageBox(Wnd, "Tipo de socket n\343o suportado.", CAPTION, MB_OK | MB_ICONSTOP);
		break;
		case 10045:
		MessageBox(Wnd, "Opera\347\343o n\343o suportada neste socket.", CAPTION, MB_OK | MB_ICONSTOP);
		break;
		case 10046:
		MessageBox(Wnd, "Fam\355lia de protocolos n\343o suportada.", CAPTION, MB_OK | MB_ICONSTOP);
		break;
		case 10047:
		MessageBox(Wnd, "Fam\355lia de endere\347os n\343o suportada.", CAPTION, MB_OK | MB_ICONSTOP);
		break;
		case 10048:
		MessageBox(Wnd, "Endere\347o de rede j\341 est\341 em uso.", CAPTION, MB_OK | MB_ICONSTOP);
		break;
		case 10049:
		MessageBox(Wnd, "N\343o foi poss\355vel atribuir o endere\347o solicitado.", CAPTION, MB_OK | MB_ICONSTOP);
		break;
		case 10050:
		MessageBox(Wnd, "A rede est\341 indispon\355vel.", CAPTION, MB_OK | MB_ICONSTOP);
		break;
		case 10051:
		MessageBox(Wnd, "A rede de destino est\341 inacess\355vel.", CAPTION, MB_OK | MB_ICONSTOP);
		break;
		case 10052:
		MessageBox(Wnd, "A conex\343o foi interrompida por uma reinicializa\347\343o da rede.", CAPTION, MB_OK | MB_ICONSTOP);
		break;
		case 10053:
		MessageBox(Wnd, "A conex\343o foi abortada pelo software local.", CAPTION, MB_OK | MB_ICONSTOP);
		break;
		case 10054:
		MessageBox(Wnd, "A conex\343o foi redefinida pelo host remoto.", CAPTION, MB_OK | MB_ICONSTOP);
		break;
		case 10055:
		MessageBox(Wnd, "Mem\363ria de buffer insuficiente para a opera\347\343o de rede.", CAPTION, MB_OK | MB_ICONSTOP);
		break;
		case 10056:
		MessageBox(Wnd, "O socket j\341 est\341 conectado.", CAPTION, MB_OK | MB_ICONSTOP);
		break;
		case 10057:
		MessageBox(Wnd, "O socket n\343o est\341 conectado.", CAPTION, MB_OK | MB_ICONSTOP);
		break;
		case 10058:
		MessageBox(Wnd, "N\343o \351 poss\355vel enviar dados ap\363s shutdown().", CAPTION, MB_OK | MB_ICONSTOP);
		break;
		case 10059:
		MessageBox(Wnd, "Muitas refer\352ncias ao objeto de rede.", CAPTION, MB_OK | MB_ICONSTOP);
		break;
		case 10060:
		MessageBox(Wnd, "Tempo limite da conex\343o esgotado.", CAPTION, MB_OK | MB_ICONSTOP);
		break;
		case 10061:
		MessageBox(Wnd, "A conex\343o foi recusada pelo host de destino.", CAPTION, MB_OK | MB_ICONSTOP);
		case 10062:
		MessageBox(Wnd, "Muitas refer\352ncias simb\363licas na resolu\347\343o do nome.", CAPTION, MB_OK | MB_ICONSTOP);
		break;
		case 10063:
		MessageBox(Wnd, "Nome de host grande demais.", CAPTION, MB_OK | MB_ICONSTOP);
		break;
		case 10064:
		MessageBox(Wnd, "O host de destino est\341 indispon\355vel.", CAPTION, MB_OK | MB_ICONSTOP);
		break;
		case 10065:
		MessageBox(Wnd, "N\343o h\341 rota para o host de destino.", CAPTION, MB_OK | MB_ICONSTOP);
		break;
		case 10091:
		MessageBox(Wnd, "O subsistema de rede ainda n\343o est\341 pronto.", CAPTION, MB_OK | MB_ICONSTOP);
		break;
		case 10092:
		MessageBox(Wnd, "A vers\343o solicitada do Winsock n\343o \351 suportada.", CAPTION, MB_OK | MB_ICONSTOP);
		break;
		case 10093:
		MessageBox(Wnd, "WSAStartup() ainda n\343o foi chamado com sucesso.", CAPTION, MB_OK | MB_ICONSTOP);
		break;
		case 11001:
		MessageBox(Wnd, "Host n\343o encontrado.", CAPTION, MB_OK | MB_ICONSTOP);
		break;
		case 11002:
		MessageBox(Wnd, "Falha tempor\341ria na resolu\347\343o do nome. Tente novamente.", CAPTION, MB_OK | MB_ICONSTOP);
		break;
		case 11003:
		MessageBox(Wnd, "Erro irrecuper\341vel na resolu\347\343o do nome.", CAPTION, MB_OK | MB_ICONSTOP);
		break;
		case 11004:
		MessageBox(Wnd, "Nenhum dado encontrado para o nome solicitado.", CAPTION, MB_OK | MB_ICONSTOP);
		break;
		default:
		wsprintf(szTemp, "Erro Winsock desconhecido./nC\363digo: %d", err);
		MessageBox(Wnd, szTemp, CAPTION, MB_OK | MB_ICONSTOP);
		break;
	}
}


