#include "opcodes.h"
#include <stdio.h>
#include "compat/legacy/sin/sinLinkHeader.h"
#include "systems/presentation/opening/legacy_runtime/HoLinkHeader.h"
#include "platform/windows/registry/smreg.h"
#include "systems/localization/legacy_catalog/language.h"
#include "systems/combat/pvp/war_mode/WarMode.h"
#include "systems/rendering/assets/v3/AssetStreamingV3.h"
#include "systems/rendering/effects/legacy_sin/AssaUtil.h"

HoLogin		*lpLogin = 0;
HoOpening	*lpOpening = 0;

static inline FILE *SummitOpeningLinkTraceFile(const char *path)
{
#if defined(SUMMIT_ENABLE_MINGW_RUNTIME_TRACE)
	return fopen(path, "a");
#else
	(void)path;
	return NULL;
#endif
}

char CharacterName1[256];
char CharacterName2[256];

int GameStartField = -1;

int WorkMode = OPENING_MODE;

int CursorShowFlag = TRUE;
int LoginEndFlag = FALSE;
bool bCaptureOpening = FALSE;
DWORD dwTickCap = 0;

// Network/fade can finish before the future character-selection scene has
// published its async assets. Keep the faded opening visible until ready.
static bool gCharacterSelectionPreloadWait = false;
static DWORD gCharacterSelectionPreloadWaitStart = 0;
static const DWORD kCharacterSelectionPreloadMaxWaitMs = 8000;

void HoReLoad()
{
	if(lpLogin != NULL && WorkMode == LOGIN_MODE)
	{
		lpLogin->ReLoad();
	}
}

void resizeLogin()
{
	if(lpLogin)
	{
		lpLogin->resize();
	}
}

void CaptureOpening()
{
	dwTickCap = GetTickCount();
	bCaptureOpening = TRUE;
}

int InitOpening()
{
	{ FILE *f=SummitOpeningLinkTraceFile("LOGS\\mingw_initopening_probe.txt"); if(f){ fprintf(f,"ENTER InitOpening WorkMode=%d lpLogin=%p lpOpening=%p\n", WorkMode, (void*)lpLogin, (void*)lpOpening); fclose(f);} }

	memset(CharacterName1, 0, sizeof(CharacterName1));
	memset(CharacterName2, 0, sizeof(CharacterName2));
	{ FILE *f=SummitOpeningLinkTraceFile("LOGS\\mingw_initopening_probe.txt"); if(f){ fprintf(f,"after memset names\n"); fclose(f);} }

	InitCursor();
	{ FILE *f=SummitOpeningLinkTraceFile("LOGS\\mingw_initopening_probe.txt"); if(f){ fprintf(f,"after InitCursor\n"); fclose(f);} }

	lpLogin = new HoLogin();
	{ FILE *f=SummitOpeningLinkTraceFile("LOGS\\mingw_initopening_probe.txt"); if(f){ fprintf(f,"after new HoLogin lpLogin=%p\n", (void*)lpLogin); fclose(f);} }

	lpOpening = new HoOpening();
	{ FILE *f=SummitOpeningLinkTraceFile("LOGS\mingw_initopening_probe.txt"); if(f){ fprintf(f,"after new HoOpening lpOpening=%p\n", (void*)lpOpening); fclose(f);} }

	// Start the future selection scene before opening initialization finishes.
	// Its CPU parse can overlap login interaction instead of beginning at transition.
	lpLogin->BeginPreload();

	if(WorkMode == OPENING_MODE)
	{
		{ FILE *f=SummitOpeningLinkTraceFile("LOGS\\mingw_initopening_probe.txt"); if(f){ fprintf(f,"before lpOpening->Init\n"); fclose(f);} }
		lpOpening->Init();
		{ FILE *f=SummitOpeningLinkTraceFile("LOGS\\mingw_initopening_probe.txt"); if(f){ fprintf(f,"after lpOpening->Init\n"); fclose(f);} }
	}
	else if(WorkMode == LOGIN_MODE)
	{
		{ FILE *f=SummitOpeningLinkTraceFile("LOGS\\mingw_initopening_probe.txt"); if(f){ fprintf(f,"before lpLogin->Init\n"); fclose(f);} }
		lpLogin->Init();
		{ FILE *f=SummitOpeningLinkTraceFile("LOGS\\mingw_initopening_probe.txt"); if(f){ fprintf(f,"after lpLogin->Init\n"); fclose(f);} }
	}

	CursorShowFlag = TRUE;
	LoginEndFlag = FALSE;

	{ FILE *f=SummitOpeningLinkTraceFile("LOGS\\mingw_initopening_probe.txt"); if(f){ fprintf(f,"LEAVE InitOpening\n"); fclose(f);} }

	return TRUE;
}

extern void showFPS();
extern int Client_Version;

int DrawOpening()
{
	static int drawLogCount = 0;
	drawLogCount++;
	{ FILE *f=SummitOpeningLinkTraceFile("NUL"); if(f){ fprintf(f,"DRAW #%d ENTER WorkMode=%d lpOpening=%p lpLogin=%p LoginEndFlag=%d\n", drawLogCount, WorkMode, (void*)lpOpening, (void*)lpLogin, LoginEndFlag); fflush(f); fclose(f);} }
	if(lpOpening == NULL || lpLogin == NULL)
	{
		return FALSE;
	}

	// Use opening/login frames as look-ahead for the next screen and for
	// commonly used effect textures. Both operations are bounded per frame.
	lpLogin->PumpOpeningPreload();
	SummitPrewarmAssaEffectTexturesStep(2, 2);

    if (!BeginRender())
        return FALSE;

	if(WorkMode == OPENING_MODE)
	{
		{ FILE *f=SummitOpeningLinkTraceFile("NUL"); if(f){ fprintf(f,"DRAW #%d before lpOpening->Draw\n", drawLogCount); fflush(f); fclose(f);} }
		lpOpening->Draw();
		{ FILE *f=SummitOpeningLinkTraceFile("NUL"); if(f){ fprintf(f,"DRAW #%d after lpOpening->Draw\n", drawLogCount); fflush(f); fclose(f);} }
	}
	else if(WorkMode == LOGIN_MODE)
	{
		lpLogin->Draw();
	}

	if (gCharacterSelectionPreloadWait)
	{
		const char *loadingText = "Carregando personagens...";
		const int loadingTextLength = static_cast<int>(strlen(loadingText));
		SetFontTextColor(RGB(255, 255, 255));
		// Keep this overlay independent from the legacy font-measurement path:
		// the login transition itself must never pull extra UI dependencies.
		dsTextLineOut(smScreenWidth / 2 - 90, smScreenHeight / 2,
			const_cast<char *>(loadingText), loadingTextLength);
	}

	if(d3ddev && LoginEndFlag == FALSE)
	{
		
		DrawCursor();
		
	}

	showFPS();
	Client_Version;

	{ FILE *f=SummitOpeningLinkTraceFile("NUL"); if(f){ fprintf(f,"DRAW #%d before EndRender\n", drawLogCount); fflush(f); fclose(f);} }
	EndRender();
	{ FILE *f=SummitOpeningLinkTraceFile("NUL"); if(f){ fprintf(f,"DRAW #%d after EndRender\n", drawLogCount); fflush(f); fclose(f);} }

	if(LoginEndFlag)
	{
		CursorShowFlag = FALSE;
	}

	return TRUE;
}

void resizeOpening()
{
	if(lpOpening)
	{
		lpOpening->resize();
	}
}


static int GameErrorTimeCount = 0;
int MainOpening()
{
	static int mainLogCount = 0;
	mainLogCount++;
	{ FILE *f=SummitOpeningLinkTraceFile("NUL"); if(f){ fprintf(f,"MAIN #%d ENTER WorkMode=%d lpOpening=%p lpLogin=%p LoginEndFlag=%d\n", mainLogCount, WorkMode, (void*)lpOpening, (void*)lpLogin, LoginEndFlag); fflush(f); fclose(f);} }
	if(lpOpening == NULL || lpLogin == NULL)
		return FALSE;

	if(WorkMode == OPENING_MODE)
	{
		{ FILE *f=SummitOpeningLinkTraceFile("NUL"); if(f){ fprintf(f,"MAIN #%d before lpOpening->Main\n", mainLogCount); fflush(f); fclose(f);} }
		int openingMainResult = lpOpening->Main();
		{ FILE *f=SummitOpeningLinkTraceFile("NUL"); if(f){ fprintf(f,"MAIN #%d after lpOpening->Main result=%d WorkMode=%d\n", mainLogCount, openingMainResult, WorkMode); fflush(f); fclose(f);} }
		if(openingMainResult == 2)
		{
			lpLogin->PumpOpeningPreload();
			if (!lpLogin->IsOpeningPreloadReady())
			{
				if (!gCharacterSelectionPreloadWait)
				{
					gCharacterSelectionPreloadWait = true;
					gCharacterSelectionPreloadWaitStart = GetTickCount();
				}

				// Never turn an asset failure into a permanent login lock. The normal
				// V3 first-use fallback can still finish an individual resource later.
				if (GetTickCount() - gCharacterSelectionPreloadWaitStart <
					kCharacterSelectionPreloadMaxWaitMs)
					return FALSE;
			}

			gCharacterSelectionPreloadWait = false;
			gCharacterSelectionPreloadWaitStart = 0;
			if(mainLogCount < 20){ FILE *f=SummitOpeningLinkTraceFile("LOGS\\mingw_opening_loop.txt"); if(f){ fprintf(f,"before lpLogin->Init from opening transition\n"); fclose(f);} }
			lpLogin->Init();
			if(mainLogCount < 20){ FILE *f=SummitOpeningLinkTraceFile("LOGS\\mingw_opening_loop.txt"); if(f){ fprintf(f,"after lpLogin->Init from opening transition\n"); fclose(f);} }
			WorkMode = LOGIN_MODE;
		}
	}
	else if(WorkMode == LOGIN_MODE)
	{
		int num;
		num = lpLogin->Main();
		if(num == 2)
		{
			lpOpening->Init();
			WorkMode = OPENING_MODE;
		}
		else if(num == 3)
			LoginEndFlag = TRUE;

		if(LoginEndFlag && CursorShowFlag == FALSE)
		{
			AssetLoadV3SetLoadingBoost(false);
			WorkMode = MODE_NONE;
			StopAllWav();
			StopBGM();

			if(hTextWnd)
			{
				SetWindowText(hTextWnd, 0);
			}

			return TRUE;
		}

	}
	return FALSE;
}

int CloseOpening()
{
	if(lpOpening != NULL)
	{
		delete lpOpening;
		lpOpening = NULL;
	}

	if(lpLogin != NULL)
	{
		delete lpLogin;
		lpLogin = NULL;
	}
	return TRUE;
}

TRANS_USERCHAR_INFO		 *ServerUserCharacterInfo = NULL;
smTRANS_CHAR_COMMAND2	 *ServerReturn = NULL;
smTRANS_COMMAND			 *ServerLoginError = NULL;
TRANS_SERVER_LIST		 *DownServerListInfo = NULL;


hoServerListInfo		UserServerListInfo[MAX_DOWN_SERVER_LIST_INFO];
int						UserServerListInfoCount;


HoUserCharacterInfo		DownUserCharacterInfo[MAX_DOWN_USER_LIST_INFO];
int						DownUserCharacterCount;

int HoRecvMessage(DWORD dwCode, void *RecvBuff)
{
#if defined(__MINGW32__) && defined(SUMMIT_ENABLE_MINGW_RUNTIME_TRACE)
	/* MINGW_HORECV_TRACE_V1 */
	{
		FILE *fp = fopen("LOGS\\mingw_ho_recv.txt", "a");
		if(fp)
		{
			fprintf(
				fp,
				"HoRecvMessage code=0x%08lX RecvBuff=%p "
				"SETUSERINFO=0x%08lX SERVER_INFO=0x%08lX "
				"FAILCONNECT=0x%08lX\n",
				(unsigned long)dwCode,
				RecvBuff,
				(unsigned long)OpCode::OPCODE_ID_SETUSERINFO,
				(unsigned long)OpCode::OPCODE_SERVER_INFO,
				(unsigned long)OpCode::OPCODE_FAILCONNECT
			);
			fclose(fp);
		}
	}
#endif
	if(dwCode == OpCode::OPCODE_ID_SETUSERINFO)
	{
		ServerUserCharacterInfo = (TRANS_USERCHAR_INFO *)RecvBuff;

#if defined(__MINGW32__) && defined(SUMMIT_ENABLE_MINGW_RUNTIME_TRACE)
		{
			FILE *fp = fopen("LOGS\\mingw_ho_recv.txt", "a");
			if(fp)
			{
				fprintf(
					fp,
					"SETUSERINFO local=[%s] server=[%s] count=%d\n",
					UserAccount,
					ServerUserCharacterInfo->szID,
					ServerUserCharacterInfo->PlayUserCount
				);
				fclose(fp);
			}
		}
#endif

		if(strcmp(UserAccount, ServerUserCharacterInfo->szID) != 0)
		{
#if defined(__MINGW32__) && defined(SUMMIT_ENABLE_MINGW_RUNTIME_TRACE)
			FILE *fp = fopen("LOGS\\mingw_ho_recv.txt", "a");
			if(fp)
			{
				fprintf(fp, "SETUSERINFO rejected: ID mismatch\n");
				fclose(fp);
			}
#endif
			return FALSE;
		}

		memset(DownUserCharacterInfo, 0, sizeof(DownUserCharacterInfo));
		DownUserCharacterCount = 0;


		// BUGFIX_015_CHARACTER_LIST_BOUNDS
		int characterCount = ServerUserCharacterInfo->PlayUserCount;

		if(characterCount < 0)
			characterCount = 0;

		if(characterCount > CHARACTER_USER_MAX)
			characterCount = CHARACTER_USER_MAX;

		for(int index = 0; index < characterCount; index++)
		{
			wsprintf(DownUserCharacterInfo[index].Name, "%s", ServerUserCharacterInfo->CharInfo[index].szName);
			DownUserCharacterInfo[index].Level = ServerUserCharacterInfo->CharInfo[index].Level;

			wsprintf(DownUserCharacterInfo[index].BodyName, "%s", ServerUserCharacterInfo->CharInfo[index].szModelName);
			wsprintf(DownUserCharacterInfo[index].FaceName, "%s", ServerUserCharacterInfo->CharInfo[index].szModelName2);

			DownUserCharacterInfo[index].Job = ServerUserCharacterInfo->CharInfo[index].JOB_CODE;
			DownUserCharacterInfo[index].StartField = ServerUserCharacterInfo->CharInfo[index].StartField;

			DownUserCharacterInfo[index].PosX = ServerUserCharacterInfo->CharInfo[index].PosX;
			DownUserCharacterInfo[index].PosZ = ServerUserCharacterInfo->CharInfo[index].PosZ;


		}
		DownUserCharacterCount = characterCount;

		// The selection object already exists while the opening screen is active.
		// Start parsing the visible character models immediately when the list
		// arrives, so the fade/transition overlaps the expensive legacy ASE work.
		if (lpLogin)
			lpLogin->BeginUserCharacterPreload();
	}
	else if(dwCode == OpCode::OPCODE_SERVER_INFO)
	{
		DownServerListInfo = (TRANS_SERVER_LIST *)RecvBuff;

#if defined(__MINGW32__) && defined(SUMMIT_ENABLE_MINGW_RUNTIME_TRACE)
		{
			FILE *fp = fopen("LOGS\\mingw_ho_recv.txt", "a");
			if(fp)
			{
				fprintf(
					fp,
					"SERVER_INFO name=[%s] count=%d\n",
					DownServerListInfo->szServerName,
					DownServerListInfo->ServerCount
				);
				fclose(fp);
			}
		}
#endif

		memset(UserServerListInfo, 0, sizeof(UserServerListInfo));
		UserServerListInfoCount = 0;

		strcpy_s(szConnServerName, DownServerListInfo->szServerName);


		for(int index = 0; index < DownServerListInfo->ServerCount; index++)
		{
			memcpy(UserServerListInfo[index].szServerName, DownServerListInfo->ServerInfo[index].szServerName, strlen(DownServerListInfo->ServerInfo[index].szServerName));
			memcpy(UserServerListInfo[index].szServerIp1, DownServerListInfo->ServerInfo[index].szServerIp1, strlen(DownServerListInfo->ServerInfo[index].szServerIp1));
			memcpy(UserServerListInfo[index].szServerIp2, DownServerListInfo->ServerInfo[index].szServerIp2, strlen(DownServerListInfo->ServerInfo[index].szServerIp2));
			memcpy(UserServerListInfo[index].szServerIp3, DownServerListInfo->ServerInfo[index].szServerIp3, strlen(DownServerListInfo->ServerInfo[index].szServerIp3));

			UserServerListInfo[index].dwServerPort1 = DownServerListInfo->ServerInfo[index].dwServerPort1;
			UserServerListInfo[index].dwServerPort2 = DownServerListInfo->ServerInfo[index].dwServerPort2;
			UserServerListInfo[index].dwServerPort3 = DownServerListInfo->ServerInfo[index].dwServerPort3;

			UserServerListInfoCount++;
		}
	}
	else if(dwCode == OpCode::OPCODE_FAILCONNECT)
	{
		ServerLoginError = (smTRANS_COMMAND *)RecvBuff;

#if defined(__MINGW32__) && defined(SUMMIT_ENABLE_MINGW_RUNTIME_TRACE)
		FILE *fp = fopen("LOGS\\mingw_ho_recv.txt", "a");
		if(fp)
		{
			fprintf(
				fp,
				"FAILCONNECT WParam=%d LParam=%d\n",
				ServerLoginError->WParam,
				ServerLoginError->LParam
			);
			fclose(fp);
		}
#endif
	}
	else
		ServerReturn = (smTRANS_CHAR_COMMAND2 *)RecvBuff;



	return TRUE;
}

void DDB2DIB(HBITMAP hbit, char *Path, int fileFormatFlag)
{
	BITMAPFILEHEADER	fh;
	BITMAPINFOHEADER	ih;
	BITMAP				bit;
	BITMAPINFO			*pih = NULL;
	int					PalSize;
	HANDLE				hFile;
	DWORD				dwWritten, Size;
	HDC					hdc = NULL;


	hdc = GetDC(NULL);



	GetObject(hbit, sizeof(BITMAP), &bit);
	ih.biSize = sizeof(BITMAPINFOHEADER);
	ih.biWidth = bit.bmWidth;
	ih.biHeight = bit.bmHeight;
	ih.biPlanes = 1;
	ih.biBitCount = bit.bmPlanes*bit.bmBitsPixel;
	if(ih.biBitCount > 8) ih.biBitCount = 24;
	ih.biCompression = BI_RGB;
	ih.biSizeImage = 0;
	ih.biXPelsPerMeter = 0;
	ih.biYPelsPerMeter = 0;
	ih.biClrUsed = 0;
	ih.biClrImportant = 0;

	PalSize = (ih.biBitCount == 24 ? 0 : 1 << ih.biBitCount) * sizeof(RGBQUAD);
	pih = (BITMAPINFO *)malloc(ih.biSize + PalSize);
	pih->bmiHeader = ih;


	GetDIBits(hdc, hbit, 0, bit.bmHeight, NULL, pih, DIB_RGB_COLORS);
	ih = pih->bmiHeader;


	if(ih.biSizeImage == 0)
	{
		ih.biSizeImage = ((((ih.biWidth*ih.biBitCount) + 31)&~31) >> 3) * ih.biHeight;
	}


	Size = ih.biSize + PalSize + ih.biSizeImage;
	pih = (BITMAPINFO *)realloc(pih, Size);


	GetDIBits(hdc, hbit, 0, bit.bmHeight, (PBYTE)pih + ih.biSize + PalSize, pih, DIB_RGB_COLORS);

	ReleaseDC(NULL, hdc);


	fh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + PalSize;
	fh.bfReserved1 = 0;
	fh.bfReserved2 = 0;
	fh.bfSize = Size + sizeof(BITMAPFILEHEADER);
	fh.bfType = 0x4d42;



	if(fileFormatFlag == CAPTURE_BMP)
	{

		hFile = CreateFile(Path, GENERIC_WRITE, 0, NULL,
						   CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		WriteFile(hFile, &fh, sizeof(fh), &dwWritten, NULL);
		WriteFile(hFile, pih, Size, &dwWritten, NULL);
		CloseHandle(hFile);
	}

	if(fileFormatFlag == CAPTURE_JPG)
	{
		BYTE *buffer = new BYTE[ih.biSizeImage];
		BYTE *pSrc;
		int SrcPitch;
		BYTE *pDest = buffer;

		int DestPitch = ((bit.bmWidth * 3 + 3) & ~3);
		SrcPitch = -((bit.bmWidth * 3 + 3) & ~3);
		pSrc = (BYTE*)pih + ((bit.bmWidth * 3 + 3) & ~3)*(ih.biHeight - 1) + ih.biSize + PalSize;

		for(int i = 0; i < bit.bmHeight; i++)
		{
			CopyMemory(pDest, pSrc, bit.bmWidth * 3);
			pDest += DestPitch;
			pSrc += SrcPitch;
		}

		CJpeg tempJpeg;
		tempJpeg.SaveJPG(Path, bit.bmWidth, bit.bmHeight, buffer);

		if(buffer != NULL)
		{
			delete buffer;
			buffer = NULL;
		}
	}

	if(pih != NULL)
	{
		free(pih);
		pih = NULL;
	}

}

void Capture(HWND handle, int fileFormatFlag)
{

	if(handle == NULL)
		return;
	CreateDirectory("Capture", NULL);

	HBITMAP		bitmap = NULL, bitmapOld;
	HDC			scrDC, memDC;

	RECT	tempRect;
	POINT	pt;
	GetClientRect(handle, &tempRect);
	pt.x = tempRect.left;
	pt.y = tempRect.top;
	ClientToScreen(handle, &pt);
	OffsetRect(&tempRect, pt.x, pt.y);

	scrDC = CreateDC("DISPLAY", NULL, NULL, NULL);
	memDC = CreateCompatibleDC(scrDC);
	bitmap = CreateCompatibleBitmap(scrDC, tempRect.right - tempRect.left, tempRect.bottom - tempRect.top);
	bitmapOld = (HBITMAP)SelectObject(memDC, bitmap);

	SYSTEMTIME systemTime;
	GetLocalTime(&systemTime);


	if(fileFormatFlag == CAPTURE_JPG)
	{

		char Habuffer[256];
		char Tempbuff[64];

		SetFontTextColor( RGB(255, 255, 255));


		memset(Habuffer, 0, sizeof(Habuffer));
		memset(Tempbuff, 0, sizeof(Tempbuff));

		
		sprintf(Tempbuff, "%d%s", (int)systemTime.wDay, "-");
		lstrcat(Habuffer, Tempbuff);
		sprintf(Tempbuff, "%d%s", (int)systemTime.wMonth, "-");
		lstrcat(Habuffer, Tempbuff);
		sprintf(Tempbuff, "%d%s", (int)systemTime.wYear, "-");
		lstrcat(Habuffer, Tempbuff);
		sprintf(Tempbuff, "%d%s", (int)systemTime.wHour, "-");
		lstrcat(Habuffer, Tempbuff);
		sprintf(Tempbuff, "%d%s", (int)systemTime.wMinute, "-");
		lstrcat(Habuffer, Tempbuff);
		sprintf(Tempbuff, "%d", (int)systemTime.wSecond);
		lstrcat(Habuffer, Tempbuff);

	}


	BitBlt(memDC, 0, 0, tempRect.right, tempRect.bottom, scrDC, tempRect.left, tempRect.top, SRCCOPY);


	SelectObject(memDC, bitmapOld);
	DeleteDC(memDC);
	DeleteDC(scrDC);

	if(bitmap != NULL)
	{
		char buffer[256];
		char numBuffer[64];
		memset(buffer, 0, sizeof(buffer));

		strcpy(buffer, "Capture//");

	
		memset(numBuffer, 0, sizeof(numBuffer));
		sprintf(numBuffer, "%d%s", (int)systemTime.wDay, "-");
		strcat(buffer, numBuffer);

		memset(numBuffer, 0, sizeof(numBuffer));
		sprintf(numBuffer, "%d%s", (int)systemTime.wMonth, "-");
		strcat(buffer, numBuffer);

			memset(numBuffer, 0, sizeof(numBuffer));
		sprintf(numBuffer, "%d%s", (int)systemTime.wYear, "-");
		strcat(buffer, numBuffer);

		memset(numBuffer, 0, sizeof(numBuffer));
		sprintf(numBuffer, "%d%s", (int)systemTime.wHour, "-");
		strcat(buffer, numBuffer);

		memset(numBuffer, 0, sizeof(numBuffer));
		sprintf(numBuffer, "%d%s", (int)systemTime.wMinute, "-");
		strcat(buffer, numBuffer);

		memset(numBuffer, 0, sizeof(numBuffer));
		sprintf(numBuffer, "%d", (int)systemTime.wSecond);
		strcat(buffer, numBuffer);

		if(fileFormatFlag == CAPTURE_BMP)
		{
			strcat(buffer, ".Bmp");
		}
		else if(fileFormatFlag == CAPTURE_JPG)
		{
			strcat(buffer, ".jpg");
		}

		DDB2DIB(bitmap, buffer, fileFormatFlag);
		DeleteObject(bitmap);
		
		AddChatBuff(">> screenshot salva na pasta ", 6);
		AddChatBuff(buffer, 6);
	}
}

int GameErrorValue = -1;
int SetGameError(int num)
{
	GameErrorValue = num;
	return TRUE;
}

int DrawInterfaceParty()
{
	InterfaceParty.Draw();

	EffectHitBoard.Draw();


	return TRUE;
}

int MainInterfaceParty()
{
	InterfaceParty.Main();
	EffectHitBoard.Main();
	return TRUE;
}

int CloseInterfaceParty()
{
	InterfaceParty.Close();
	return TRUE;
}

int	SetInterfaceParty(char *chrName, DWORD chrCode)
{
	InterfaceParty.SetParty(chrName, chrCode);
	return TRUE;
}

int MainInterfaceParty(int x, int y)
{
	InterfaceParty.MainPartyMessage(x, y);
	return TRUE;
}

int DrawInterfaceParty(int x, int y)
{
	InterfaceParty.DrawPartyMessage(x, y);
	return TRUE;
}


extern int QuestnotCharMove;
extern int ClanCharMove;
extern int ClanCLCharMove;

#include "systems/commerce/coin_shop/CoinShop.h"
#include "systems/commerce/coin_shop/CoinShop_T.h"
#include "systems/social/party/ui/cPartyWindow.h"
#include "systems/ui/targeting/cTarget.h"
#include "systems/gameplay/quests/ui/cQuestWindow.h"
#include "systems/ui/settings/legacy_window/cSettingsWindow.h"

extern BOOL stop;
BOOL HoStopCharMotion()
{if (cCoinShop.isOpen() || cCoinShop_T.isOpen() || cParty.StopMotion() || cTarget.StopMotion() || cQuestWindow.OpenFlag || cSettings.isOpen() || stop)
		return TRUE;if(InterfaceParty.MessageBoxOpenFlag)
	{
		if(pCursorPos.x >= 653 && pCursorPos.x <= 653 + 23 && pCursorPos.y >= 14 && pCursorPos.y <= 14 + 23)
			return TRUE;

		if(pCursorPos.x >= 653 && pCursorPos.x <= 653 + 23 && pCursorPos.y >= 41 && pCursorPos.y <= 41 + 23)
			return TRUE;
	}



	if(InterfaceParty.PartyPosState == PARTY_PROCESS)
	{

		if(600 <= pCursorPos.x  && pCursorPos.x <= 800 && pCursorPos.y >= 125 + 33 && pCursorPos.y <= 371 + 33)
			return TRUE;
	}



	if(QuestnotCharMove)
	{

		if(600 <= pCursorPos.x && pCursorPos.x <= 800 && pCursorPos.y >= 125 + 33 && pCursorPos.y <= 371 + 33)
			return TRUE;
	}



	if(ClanCharMove && isDrawClanMenu)
	{

		extern POINT clanBoxPos;
		if(clanBoxPos.x <= pCursorPos.x && pCursorPos.x <= (clanBoxPos.x + 64 * 5) &&
		   clanBoxPos.y <= pCursorPos.y && pCursorPos.y <= (clanBoxPos.y + 64 * 5))
			return TRUE;
	}


	if(ClanCLCharMove && isDrawClanMenu)
	{


		extern POINT clanCLBoxPos;
		if(clanCLBoxPos.x <= pCursorPos.x && pCursorPos.x <= (clanCLBoxPos.x + 256) &&
		   clanCLBoxPos.y <= pCursorPos.y && pCursorPos.y <= (clanCLBoxPos.y + 256))
			return TRUE;
	}


	return FALSE;
}

int hoDrawTexImageFloat(int Mat, float putX, float putY, float putSizeX, float putSizeY, float getTexX, float getTexY, float getTexSizeX, float getTexSizeY, float texSizeX, float texSizeY, int alpha)
{
	D3DTLVERTEX tlVertex[4];
	int cnt;

	for(cnt = 0; cnt < 4; cnt++)
	{
		tlVertex[cnt].rhw = 1;
		tlVertex[cnt].color = D3DCOLOR_RGBA(255, 255, 255, alpha);
		tlVertex[cnt].specular = D3DCOLOR_RGBA(0, 0, 0, 0);
		tlVertex[cnt].sz = 0;
	}

	float surWidth;
	float surHeight;

	surWidth = (float)texSizeX;
	surHeight = (float)texSizeY;

	tlVertex[0].sx = putX;
	tlVertex[0].sy = putY;
	tlVertex[0].tu = (1.f / surWidth)*getTexX;
	tlVertex[0].tv = (1.f / surHeight)*getTexY;

	tlVertex[1].sx = putX;
	tlVertex[1].sy = putY + putSizeY;
	tlVertex[1].tu = (1.f / surWidth)*getTexX;
	tlVertex[1].tv = (1.f / surHeight)*(getTexY + getTexSizeY);

	tlVertex[2].sx = putX + putSizeX;
	tlVertex[2].sy = putY;
	tlVertex[2].tu = (1.f / surWidth)*(getTexX + getTexSizeX);
	tlVertex[2].tv = (1.f / surHeight)*getTexY;

	tlVertex[3].sx = putX + putSizeX;
	tlVertex[3].sy = putY + putSizeY;
	tlVertex[3].tu = (1.f / surWidth)*(getTexX + getTexSizeX);
	tlVertex[3].tv = (1.f / surHeight)*(getTexY + getTexSizeY);
	DrawTexture(AssetLoadV2GetResidentTexture(smMaterial[Mat].smTexture[0]), tlVertex, true);
	return TRUE;
}


static int CompString(char *src1, char *src2)
{
	int cnt;
	int len1, len2;
	int len;

	len1 = lstrlen(src1);
	len2 = lstrlen(src2);

	if(len1 < len2) len = len1;
	else len = len2;

	if(!len) return NULL;

	for(cnt = 0; cnt < len; cnt++)
	{
		if(src1[cnt] == 0 || src2[cnt] == 0) break;
		if(src1[cnt] != src2[cnt]) return NULL;
	}

	return cnt;
}


int szSpaceSorting(char *lpString)
{
	int cnt, len;
	len = lstrlen(lpString);
	for(cnt = len - 1; cnt >= 0; cnt--)
	{
		if(lpString[cnt] != ' ')
		{
			lpString[cnt + 1] = 0;
			break;
		}
	}
	if(cnt < 0) lpString[0] = 0;

	return cnt;
}

char *CpFindString(char *lpData, char *lpFindWord, char *lpSaveBuff, int buffSize)
{

	int cnt;
	int len;
	int ep;
	int cnt2;
	char *lpsp;

	len = lstrlen(lpData);

	for(cnt = 0; cnt < len; cnt++)
	{
		ep = CompString(lpData + cnt, lpFindWord);
		if(ep)
		{
			lpsp = lpData + cnt + ep;
			for(cnt2 = 0; cnt2 < buffSize - 1; cnt2++)
			{
				if(lpsp[cnt2] == 0x0D || lpsp[cnt2] == 0x0A || lpsp[cnt2] == 0 || lpsp[cnt2] == ' ') break;
			}
			if(lpSaveBuff)
			{
				memcpy(lpSaveBuff, lpsp, cnt2);
				lpSaveBuff[cnt2] = 0;
				szSpaceSorting(lpSaveBuff);
			}
			return lpsp;
		}
	}
	return NULL;
}


hoCommandInfo	HoCommandInfo;
BOOL			HoCommandLineFlag = FALSE;
int				HoCommunity = COMMUNITY_TRIGLOW;

BOOL		HoEffectResLoadingFlag = TRUE;

void		HoSetEffectResLoading(BOOL flag)
{
	HoEffectResLoadingFlag = flag;
}

extern POINT3D	WaveCameraPosi;
extern int		WaveCameraFactor;
extern BOOL		WaveCameraFlag;
extern int		WaveCameraDelay;

extern void		EffectWaveCamera(int factor, int delay)
{
	WaveCameraFlag = TRUE;
	WaveCameraFactor = factor;
	WaveCameraDelay = delay;
}







