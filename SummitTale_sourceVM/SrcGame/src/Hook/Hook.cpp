#include "Hook.h"
#include "drawsub.h"
#include "smLib3d\\smd3d.h"
#include <winbase.h>
#include <vector>

using namespace std;

#ifndef _W_SERVER

bool HookAPI(char* moduleName, char* apiName, void* callBack, void* backup = NULL)
{
	void * apiH = NULL;
	DWORD old;

	apiH = GetProcAddress(GetModuleHandle(moduleName), apiName);

	if (apiH == NULL)
		return false;

	if (backup != NULL)
	{
		MoveMemory(backup, apiH, 5);
	}

	VirtualProtect(apiH, 5, PAGE_EXECUTE_READWRITE, &old);
	*(BYTE*)((DWORD)apiH) = 0xE9;
	*(DWORD*)((DWORD)apiH + 1) = (DWORD)callBack - (DWORD)apiH - 5;
	VirtualProtect(apiH, 5, old, &old);

	return true;
}

int  __stdcall GetFontExtentPoint(HDC hdc, char* lpString, int c, SIZE* lpSize)
{
	RECT rcRect = { 0,0,0,0 };

	ID3DXFont * pCurrentFont = nullptr;

	switch (FontType)
	{
	case 0:
		if (Font)
			pCurrentFont = Font;
		break;
	case 1:
		if (FontBold)
			pCurrentFont = FontBold;
		break;
	}

	if (pCurrentFont)
	{
		if (lpString[c - 1] == ' ')
		{
			char buff[2048] = {};
			RECT rcRect2 = { 0,0,0,0 };

			lstrcpy(buff, lpString);
			lstrcat(buff, "_");

			pCurrentFont->DrawText(NULL, buff, c+1, &rcRect, DT_CALCRECT, FontColor);
			pCurrentFont->DrawText(NULL, "_", 1, &rcRect2, DT_CALCRECT, FontColor);

			lpSize->cx = rcRect.right - rcRect.left - (rcRect2.right - rcRect2.left);
			lpSize->cy = rcRect.bottom - rcRect.top;
		}
		else
		{
			pCurrentFont->DrawText(NULL, lpString, c, &rcRect, DT_CALCRECT, FontColor);

			lpSize->cx = rcRect.right - rcRect.left;
			lpSize->cy = rcRect.bottom - rcRect.top;
		}
	}
	return TRUE;
}

int __stdcall SetFontColor(DWORD textcolor)
{
	FontColor = D3DCOLOR_RGBA(GetRValue(textcolor), GetGValue(textcolor), GetBValue(textcolor), 255);
	return TRUE;
}

extern int WinSizeX;
extern int WinSizeY;

int  DrawFontText(int x, int y, LPCTSTR lpszString, COLORREF Color, int FontTypeDraw)
{
	SetFontTextColor(Color);

	if (dsDrawOffset_X || dsDrawOffset_Y)
	{
		if (dsDrawOffsetArray & dsARRAY_RIGHT)
			x += dsDrawOffset_X;

		if (dsDrawOffsetArray & dsARRAY_BOTTOM)
			y += dsDrawOffset_Y;
	}

	RECT rc;
	rc.left = x;
	rc.right = x + smScreenWidth;
	rc.top = y;
	rc.bottom = y + smScreenHeight;

	char dwFontGame[32] = { 0 };

	if ((!Font && FontTypeDraw == 0) || (!FontBold && FontTypeDraw == 1))
	{
		GetPrivateProfileString("FontG", "Type", "", dwFontGame, 32, ".\\game\\Font\\Config.ini");
	}

	switch (FontTypeDraw)
	{
	case 0:
		if (Font) 
			return Font->DrawText(NULL, lpszString, strlen(lpszString), &rc, 0, FontColor);
		else
			D3DXCreateFont(d3ddev, 16, 0, FW_NORMAL, 0, 0, ANSI_CHARSET | FS_LATIN1, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY | VARIABLE_PITCH, dwFontGame, &Font);
		break;
	case 1:
		if (FontBold)
			return FontBold->DrawText(NULL, lpszString, strlen(lpszString), &rc, 0, FontColor);
		else
			D3DXCreateFont(d3ddev, 16, 0, FW_BOLD, 1, 0,  ANSI_CHARSET | FS_LATIN1, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY | VARIABLE_PITCH, dwFontGame, &FontBold);
		break;
	case 2:
		if (FontTitle)
			return FontTitle->DrawText(NULL, lpszString, strlen(lpszString), &rc, 0, FontColor);
		else
			D3DXCreateFont(d3ddev, 16, 0, FW_NORMAL, 1, 0, ANSI_CHARSET | FS_LATIN1, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY | VARIABLE_PITCH, dwFontGame, &FontTitle);
		break;
	case 3:
		if (FontRank)
			return FontRank->DrawText(NULL, lpszString, strlen(lpszString), &rc, 0, FontColor);
		else
			D3DXCreateFont(d3ddev, 16, 0, FW_NORMAL, 1, 0, ANSI_CHARSET | FS_LATIN1, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY | VARIABLE_PITCH, dwFontGame, &FontRank);
		break;
	}

	return 0;
}

int DrawFontRect(RECT rc, char* lpszString, int nCount, UINT uFormat)
{
	char dwFontGame[32] = { 0 };
	//int SizeFontGame = 0;
	
	if ((!Font && FontType == 0) || (!FontBold && FontType == 1))
	{
		GetPrivateProfileString("FontG", "Type", "", dwFontGame, 32, ".\\game\\Font\\Config.ini");
	}

	switch (FontType)
	{
	case 0:
		if (Font)  
			return Font->DrawText(NULL, lpszString, nCount, &rc, 0, FontColor);
		else
			D3DXCreateFont(d3ddev, 16, 0, FW_NORMAL, 0, 0, ANSI_CHARSET | FS_LATIN1, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY | VARIABLE_PITCH, dwFontGame, &Font);

		break;
	case 1:
		if (FontBold)
			return FontBold->DrawText(NULL, lpszString, nCount, &rc, 0, FontColor);
		else
			D3DXCreateFont(d3ddev, 16, 0, FW_BOLD, 1, 0, ANSI_CHARSET | FS_LATIN1, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY | VARIABLE_PITCH, dwFontGame, &FontBold);
		break;
	case 2:
		if (FontTitle)
			return FontTitle->DrawText(NULL, lpszString, nCount, &rc, 0,  FontColor);
		else
			D3DXCreateFont(d3ddev, 16, 0, FW_NORMAL, 1, 0, ANSI_CHARSET | FS_LATIN1, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY | VARIABLE_PITCH, dwFontGame, &FontTitle);
		break;
	case 3:
		if (FontRank)
			return FontRank->DrawText(NULL, lpszString, strlen(lpszString), &rc, 0, FontColor);
		else
			D3DXCreateFont(d3ddev, 16, 0, FW_NORMAL, 1, 0, ANSI_CHARSET | FS_LATIN1, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY | VARIABLE_PITCH, dwFontGame, &FontRank);
		break;
	}
	return 0;
}

void SetBold(bool bold)
{
	FontType = bold;
}

void InitFont()
{
	char dwFontGame[32] = { 0 };

	GetPrivateProfileString("FontG", "Type", "", dwFontGame, 32, ".\\game\\Font\\Config.ini");

	if (!Font)
		D3DXCreateFont(d3ddev, 16, 0, FW_NORMAL, 1, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLEARTYPE_NATURAL_QUALITY, DEFAULT_PITCH | FF_DONTCARE, dwFontGame, &Font);

	if (!FontBold)
		D3DXCreateFont(d3ddev, 16, 0, FW_BOLD, 1, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLEARTYPE_NATURAL_QUALITY, DEFAULT_PITCH | FF_DONTCARE, dwFontGame, &FontBold);

	if (!FontRank)
		D3DXCreateFont(d3ddev, 16, 0, FW_NORMAL, 1, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLEARTYPE_NATURAL_QUALITY, DEFAULT_PITCH | FF_DONTCARE, dwFontGame, &FontRank);

	HookAPI("gdi32.dll", "TextOutA", &DrawFontText);
	HookAPI("gdi32.dll", "SetFontTextColor", &SetFontColor);
	HookAPI("gdi32.dll", "GetTextExtentPointA", &GetFontExtentPoint);
	HookAPI("gdi32.dll", "GetTextExtentPoint32A", &GetFontExtentPoint);
}

#endif
