#include <windows.h>
#include <stdio.h>
#include "Util.h"

void MiLae_Debug_txt(char *fmt, ...)
{
	char Buff[4096];
	ZeroMemory(Buff, sizeof(Buff));

	va_list args;
	HANDLE hFile;
	DWORD dwAcess = 0;

	hFile = CreateFile("game_debug.txt", GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
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
		MessageBox(Wnd, CAPTION, "���ŷ ������ WSACancelBlockingCall �Լ�����  ��ҵǾ����ϴ�", MB_OK | MB_ICONSTOP);
		break;
		case 10009:
		MessageBox(Wnd, CAPTION, "�߸��� �����(���� �ڵ�)�̴�", MB_OK | MB_ICONSTOP);
		break;
		case 10013:
		MessageBox(Wnd, CAPTION, "��ε�ĳ��Ʈ ��巹���� ���� �����ͱ׷� ������  ���ӽõ��� setsockopt �Լ��� SO_BROADCAST��  �����Ǿ����� ���� ���¿��� ���� �߽��ϴ�.", MB_OK | MB_ICONSTOP);
		break;
		case 10014:
		MessageBox(Wnd, CAPTION, "name �Ǵ� namelen �Ű������� �ùٸ� ���°� �ƴմϴ�.", MB_OK | MB_ICONSTOP);
		break;
		case 10022:
		MessageBox(Wnd, CAPTION, "accept �ϱ� ���� listen �Լ��� �ҷ����� �ʾҽ��ϴ�", MB_OK | MB_ICONSTOP);
		break;
		case 10024:
		MessageBox(Wnd, CAPTION, "���ο� ���Ͽ� �Ҵ��ϱ� ���� ���� ����ڰ� �� �̻� �������� �ʽ��ϴ�", MB_OK | MB_ICONSTOP);
		break;
		case 10035:
		MessageBox(Wnd, CAPTION, "���� �Լ��� ����ŷ ���� �������̴�", MB_OK | MB_ICONSTOP);
		break;
		case 10036:
		MessageBox(Wnd, CAPTION, "���ȭ �Լ��� ȣ�� �Ǵ� ���� �������� ���� �Լ���  ȣ��Ǿ��� ", MB_OK | MB_ICONSTOP);
		break;
		case 10037:
		MessageBox(Wnd, CAPTION, "�̹� �Ϸ�� �񵿱� ��ɿ� ���� ��Ұ� �õ��� ", MB_OK | MB_ICONSTOP);
		break;
		case 10038:
		MessageBox(Wnd, CAPTION, "������ ����ڰ� ���� ����ڰ� �ƴմϴ�", MB_OK | MB_ICONSTOP);
		break;
		case 10039:
		MessageBox(Wnd, CAPTION, "�ش� �Լ��� ������ ��巹���� �ʿ������� �������� �ʾ���", MB_OK | MB_ICONSTOP);
		break;
		case 10040:
		MessageBox(Wnd, CAPTION, "���ŵ� �޽����� ������ ���ۿ� �����ϱ⿡ �ʹ� Ŀ�� �ս� �Ǿ����ϴ�", MB_OK | MB_ICONSTOP);
		break;
		case 10041:
		MessageBox(Wnd, CAPTION, "������ ���������� �߸��Ǿ��ų� �� ���Ͽ� ���ؼ�  �߸��� �����Դϴ�", MB_OK | MB_ICONSTOP);
		break;
		case 10042:
		MessageBox(Wnd, CAPTION, "�� �� ���� �ɼ��̰ų�, �������� �ʴ� �ɼ��� ����߽��ϴ�. ", MB_OK | MB_ICONSTOP);
		break;
		case 10043:
		MessageBox(Wnd, CAPTION, "������ ���������� �������� �ʴ� �����Դϴ�. ", MB_OK | MB_ICONSTOP);
		break;
		case 10044:
		MessageBox(Wnd, CAPTION, "������ ���� Ÿ���� ������ ��巹�� ü�迡�� �������� �ʴ� �����Դϴ�", MB_OK | MB_ICONSTOP);
		break;
		case 10045:
		MessageBox(Wnd, CAPTION, "socket�� ���������� ����(SOCK_STREAM)���°� �ƴմϴ�. ex) listen�� UDP socket���� ȣ�� ", MB_OK | MB_ICONSTOP);
		break;
		case 10046:
		MessageBox(Wnd, CAPTION, "������ �������� ü�谡(PF_*) �������� �ʽ��ϴ�", MB_OK | MB_ICONSTOP);
		break;
		case 10047:
		MessageBox(Wnd, CAPTION, "������ ��巹�� ü�谡(AF_*) �������� �ʽ��ϴ�", MB_OK | MB_ICONSTOP);
		break;
		case 10048:
		MessageBox(Wnd, CAPTION, "������ ��巹��(IP)�� �̹� ������̴�", MB_OK | MB_ICONSTOP);
		break;
		case 10049:
		MessageBox(Wnd, CAPTION, "������ ��巹���� ���� �ӽſ��� ����� ���� ����", MB_OK | MB_ICONSTOP);
		break;
		case 10050:
		MessageBox(Wnd, CAPTION, "��Ʈ�� ���� �ý��ۿ� ������ �߻��߽��ϴ�", MB_OK | MB_ICONSTOP);
		break;
		case 10051:
		MessageBox(Wnd, CAPTION, "���� �ý��۱��� ��Ʈ���� ������ �� �����ϴ�", MB_OK | MB_ICONSTOP);
		break;
		case 10052:
		MessageBox(Wnd, CAPTION, "������ ����ǰ� �ִ� ���� ������ ���ܹ��Ƚ��ϴ�", MB_OK | MB_ICONSTOP);
		break;
		case 10053:
		MessageBox(Wnd, CAPTION, "������ out-of-band�� �ٸ� ���� ������ ������ ���Ƚ��ϴ�.", MB_OK | MB_ICONSTOP);
		break;
		case 10054:
		MessageBox(Wnd, CAPTION, "���� ���������� 'hard'�� 'abortive' ���Ḧ �����ؼ� ���µǾ����ϴ�.", MB_OK | MB_ICONSTOP);
		break;
		case 10055:
		MessageBox(Wnd, CAPTION, "������ ���� �ý����� ���� ������ ���ڶ�ų�, ���ø����̼ǿ� ���� API���� ������ ������ �ʹ� �۾Ƽ� ��û�� ������ ���� �� ���� ����", MB_OK | MB_ICONSTOP);
		break;
		case 10056:
		MessageBox(Wnd, CAPTION, "������ ������ �̹� ���� �Ǿ� ����", MB_OK | MB_ICONSTOP);
		break;
		case 10057:
		MessageBox(Wnd, CAPTION, "������ ������ �̹� ���� �Ǿ� ���� ����", MB_OK | MB_ICONSTOP);
		break;
		case 10058:
		MessageBox(Wnd, CAPTION, "������ �˴ٿ�(shutdown()) �Ǿ����ϴ�.", MB_OK | MB_ICONSTOP);
		break;
		case 10059:
		MessageBox(Wnd, CAPTION, "������ �Լ��� ���� ���ڰ� �ʹ� ����", MB_OK | MB_ICONSTOP);
		break;
		case 10060:
		MessageBox(Wnd, CAPTION, "���� �õ��� �ð��ʰ� �Ǿ����ϴ�.", MB_OK | MB_ICONSTOP);
		break;
		case 10061:
		MessageBox(Wnd, CAPTION, "���ӽõ��� ������ ����Ǿ����ϴ�", MB_OK | MB_ICONSTOP);
		case 10062:
		MessageBox(Wnd, CAPTION, "WSAELOOP ERROR", MB_OK | MB_ICONSTOP);
		break;
		case 10063:
		MessageBox(Wnd, CAPTION, "WSAENAMETOOLONG ERROR", MB_OK | MB_ICONSTOP);
		break;
		case 10064:
		MessageBox(Wnd, CAPTION, "����ȣ��Ʈ�� �ٿ� �Ǿ���", MB_OK | MB_ICONSTOP);
		break;
		case 10065:
		MessageBox(Wnd, CAPTION, "��Ʈ�� �ý��� ��� � ���ؼ� ����ȣ��Ʈ������ �� �� �� �����ϴ�.", MB_OK | MB_ICONSTOP);
		break;
		case 10091:
		MessageBox(Wnd, CAPTION, "��Ʈ��ũ ���� �ý����� ���� ����� �غ� �Ǿ� ���� ����(WSAStartup()�� ��ȯ)", MB_OK | MB_ICONSTOP);
		break;
		case 10092:
		MessageBox(Wnd, CAPTION, "��û�� �������� ���� ������ ���� �������� ���� �ý��ۿ��� �������� �ʽ��ϴ�.", MB_OK | MB_ICONSTOP);
		break;
		case 10093:
		MessageBox(Wnd, CAPTION, "�� �Լ��� ����ϱ� ���� �������� WSAStartup �Լ��� ȣ���� �������ϴ�.", MB_OK | MB_ICONSTOP);
		break;
		case 11001:
		MessageBox(Wnd, CAPTION, "ȣ��Ʈ�� ã�Ƴ� �� �����ϴ�.", MB_OK | MB_ICONSTOP);
		break;
		case 11002:
		MessageBox(Wnd, CAPTION, "��û�� ������ �߰ߵ��� ����", MB_OK | MB_ICONSTOP);
		break;
		case 11003:
		MessageBox(Wnd, CAPTION, "ȸ���� �� ���� ���� �߻�", MB_OK | MB_ICONSTOP);
		break;
		case 11004:
		MessageBox(Wnd, CAPTION, "�߸��� �̸�(name)���� �ƹ��� �����Ͱ� ��ϵ��� �ʾҽ��ϴ�", MB_OK | MB_ICONSTOP);
		break;
		default:
		wsprintf(szTemp, "�˼����� �����Դϴ�/n������ȣ : %d", err);
		MessageBox(Wnd, CAPTION, szTemp, MB_OK | MB_ICONSTOP);
		break;
	}
}



