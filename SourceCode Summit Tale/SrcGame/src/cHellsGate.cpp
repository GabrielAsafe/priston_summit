#include "opcodes.h"
#include "sinbaram/sinLinkHeader.h"
#include "field.h"
#include "cHellsGate.h"

extern int DrawFontImage(char *lpStr, int xPos, int yPos, DWORD rgba, float fRatio);

cHELLSGATE cHg;

void cHELLSGATE::RecvOpenNpc()
{
	cMessageBox.ShowMessage3(MESSAGE_ENTER_HG, "Deseja entrar na arena?");
}

void cHELLSGATE::ConfirmEnterHg()
{
	smTRANS_COMMAND sPacket;
	sPacket.code = OpCode::OPCODE_OPEN_HG;
	sPacket.size = sizeof(smTRANS_COMMAND);
	smWsockServer->Send((char*)&sPacket, sPacket.size, TRUE);
}

void cHELLSGATE::Draw()
{
	if (lpCurPlayer->OnStageField >= 0 && StageField[lpCurPlayer->OnStageField]->FieldCode == 23)
	{
		if (bInEvent)
		{
			char buff[255];

			int x, y;

			x = (WinSizeX / 2) - 100;
			y = 8;

			int sec = dwTime % 60;
			int min = (dwTime / 60) % 60;
			int hour = ((dwTime / 60) / 60);

			wsprintf(buff, "Tempo restante: %02d:%02d:%02d", hour, min, sec);
			DrawFontImage(buff, x, y + 2, D3DCOLOR_RGBA(80, 160, 255, 255), 0.7f);

			if (iState == 1)
			{
				DrawFontImage("Aguardando jogadores...", x, y + 32, D3DCOLOR_RGBA(80, 160, 255, 255), 0.7f);
			}
		}
	}
}

void cHELLSGATE::RecvUpdateHg(smTRANS_COMMAND * psPacket)
{
	bInEvent = true;

	dwTime = psPacket->WParam;

	iState = psPacket->LParam;
}

void cHELLSGATE::RecvLeaveHg()
{
	bInEvent = false;
	dwTime = 0;
	iState = 0;
	WarpField2(3);
}





