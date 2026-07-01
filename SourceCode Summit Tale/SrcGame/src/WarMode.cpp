#include "sinbaram/sinLinkHeader.h"
#include "WarMode.h"

MODOWAR WarMode;
extern int PLayerPvP;
extern int ReiPVP;

void MODOWAR::Init()
{
	WarModeImg[0] = CreateTextureMaterial("game\\Warmode\\window.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	WarModeImg[1] = CreateTextureMaterial("game\\Warmode\\Button_ativar.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	WarModeImg[2] = CreateTextureMaterial("game\\Warmode\\Button_desativar.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	
}

BOOL MODOWAR::IsMousePos(int x, int y, int w, int h)
{
	if (pRealCursorPos.x > x&& pRealCursorPos.x < x + w && pRealCursorPos.y > y&& pRealCursorPos.y < y + h)
		return TRUE;

	return FALSE;

}

void MODOWAR::DrawImage(int Image, int x, int y, int w, int h)
{
	dsDrawTexImage(Image, x, y, w - 1, h - 1, 255);
}

void MODOWAR::Draw()
{
	if (fOpen)
	{
		
		BaseX = (WinSizeX - 300);
		BaseY = (WinSizeY - 331) /2;
		char szBuff[256];
		SIZE size = {};
		SetFontTextColor(RGB(255, 255, 255));
		dsDrawTexImage(WarModeImg[0], BaseX, BaseY, 270, 331, 255);
			wsprintf(szBuff, "%s", lpCurPlayer->smCharInfo.szName);
		
		GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
		dsTextLineOut(BaseX + (140 - size.cx / 2), BaseY + 80, szBuff, strlen(szBuff));
		FontType = 1;
		wsprintf(szBuff, "Você Matou: %d", lpCurPlayer->sBlessCastle_Damage[1]);
		GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
		dsTextLineOut(BaseX + 40, BaseY + 120, szBuff, strlen(szBuff));

		wsprintf(szBuff, "Você Morreu: %d", lpCurPlayer->MortesPvP);
		GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
		dsTextLineOut(BaseX + 40, BaseY + 140, szBuff, strlen(szBuff));
		FontType = 0;

		wsprintf(szBuff, "Bônus Modo PvP");
		GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
		dsTextLineOut(BaseX + 40, BaseY + 180, szBuff, strlen(szBuff));


		wsprintf(szBuff, "Tx. de XP 10%% adicional");
		if (!PLayerPvP)
			SetFontTextColor(RGB(255, 0, 0));
		else SetFontTextColor(RGB(0, 255, 0));
		GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
		dsTextLineOut(BaseX + 40, BaseY + 200, szBuff, strlen(szBuff));

		wsprintf(szBuff, "Tx. de Drop 5%% adicional");
		if (!PLayerPvP)
			SetFontTextColor(RGB(255, 0, 0));
		else SetFontTextColor(RGB(0, 255, 0));
		GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
		dsTextLineOut(BaseX + 40, BaseY + 220, szBuff, strlen(szBuff));

		if (!PLayerPvP)
			DrawImage(WarModeImg[1], BaseX+64, BaseY + 250, 147, 35);
		else
			DrawImage(WarModeImg[2], BaseX + 64, BaseY + 250, 147, 35);
	}
}

void MODOWAR::Button()
{
	if (fOpen)
	{
		BaseX = (WinSizeX - 300);
		BaseY = (WinSizeY - 330) / 2;

		if (!PLayerPvP)
		{
			if (IsMousePos(BaseX + 64, BaseY + 250, 147, 35))
				cMessageBox.ShowMessage3(MODOPVP, "Deseja ativar Modo PvP ?");
		}
		else
		{
			if (IsMousePos(BaseX + 64, BaseY + 250, 147, 35))
				cMessageBox.ShowMessage3(MODOPVP, "Deseja desativar Modo PvP ?");
		}
	}
}