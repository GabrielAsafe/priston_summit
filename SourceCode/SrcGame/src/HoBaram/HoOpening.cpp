#include "sinbaram/sinLinkHeader.h"
#include "../HoBaram/HoLinkHeader.h"
#include "../smreg.h"
#include "../HoBaram/HoTextFile.h"
#include "../HoBaram/HoMinMax.h"
#include "../HoBaram/HoNewParticle.h"
#include "../HoBaram/HoNewParticleMgr.h"
#include "Language/language.h"

#include "../CAutoCamera.h"

#include "Font/Font.h"
#include "Settings/Settings.h"
#include "Settings/UI.h"
#include "File/File.h"
#include "Login/Login.h"
#include "cSettingsWindow.h"
#include <stdarg.h>


extern int			iSettingsBg;
extern int			iSettingsButton[2];
extern int			iSettingsRelog;
extern int			iSettingsSave;
extern int			iSettingsCheckBox[2];



static CAutoCamera		s_AutoCamera;

#ifdef _LANGUAGE_BRAZIL	
char* szJoinWebUrl = "http://www.PristonTale.com.br";
#endif

extern POINT3D WaveCameraPosi;
extern int	   WaveCameraFactor;
extern int	   WaveCameraTimeCount;
extern int	   WaveCameraDelay;
extern BOOL	   WaveCameraFlag;

char	UserAccount[ACCOUNT_SIZE];
char	UserPassword[PASSWORD_SIZE];


static void MINGW_LoginTrace(const char* fmt, ...)
{
    FILE* f = fopen("mingw_login_thread.txt", "a");
    if (!f) return;

    va_list args;
    va_start(args, fmt);
    vfprintf(f, fmt, args);
    fprintf(f, "\n");
    va_end(args);

    fflush(f);
    fclose(f);
}


static unsigned int MINGW_SafeStringLength(
	const char *text,
	unsigned int capacity)
{
	unsigned int length = 0;

	if (!text)
		return 0;

	while (length < capacity && text[length] != '\0')
		++length;

	return length;
}


static void MINGW_CheckUserAccount(const char *place)
{
	static unsigned char previous[ACCOUNT_SIZE];
	static int initialized = 0;

	if (!initialized)
	{
		memcpy(previous, UserAccount, sizeof(previous));
		initialized = 1;
		return;
	}

	if (memcmp(previous, UserAccount, sizeof(previous)) != 0)
	{
		FILE *f = fopen("mingw_useraccount_changes.txt", "a");

		if (f)
		{
			unsigned int i;

			fprintf(
				f,
				"CHANGE at %s addr=%p strlen=%u text=[",
				place ? place : "(null)",
				(void *)UserAccount,
				MINGW_SafeStringLength(
					UserAccount,
					(unsigned int)sizeof(UserAccount))
			);

			/*
			 * N�o usar diretamente %s aqui:
			 * durante a corrup��o o buffer pode n�o ter terminador NUL.
			 */
			for (
				i = 0;
				i < MINGW_SafeStringLength(
					UserAccount,
					(unsigned int)sizeof(UserAccount));
				++i)
			{
				unsigned char c = (unsigned char)UserAccount[i];

				if (c >= 32 && c <= 126)
					fputc(c, f);
				else
					fprintf(f, "\\x%02X", (unsigned int)c);
			}

			fprintf(f, "]\nOLD: ");

			for (i = 0; i < sizeof(previous); ++i)
				fprintf(f, "%02X ", (unsigned int)previous[i]);

			fprintf(f, "\nNEW: ");

			for (i = 0; i < sizeof(UserAccount); ++i)
			{
				fprintf(
					f,
					"%02X ",
					(unsigned int)(unsigned char)UserAccount[i]
				);
			}

			fprintf(f, "\n\n");
			fflush(f);
			fclose(f);
		}

		memcpy(previous, UserAccount, sizeof(previous));
	}
}


HoOpening::HoOpening()
{


	Stage = NULL;
	PatCamera = NULL;

	LogoBlendValue = 0;
	LoginBlendValue = 0;

	Load();
	StartConnectServerFlag = FALSE;
	WorkState = STATE_OPENING;

	MidY = (600 / 2);
	SetWindowText(hTextWnd, NULL);

	memset(UserAccount, 0, sizeof(UserAccount));
	memset(UserPassword, 0, sizeof(UserPassword));

	TarDist = 1600 * fONE;
	CaAngle.x = 220;
	CaAngle.y = 0;
	CaAngle.z = 0;




	MouseDownServerIndex = -1;

	{ FILE *f=fopen("mingw_hoopening_init.txt","a"); if(f){ fprintf(f,"after MouseDownServerIndex\\n"); fclose(f);} }
	CameraFrame = 0;
}

HoOpening::~HoOpening()
{
	Close();
}

int HoOpening::Init()
{

	{ FILE *f=fopen("mingw_hoopening_init.txt","a"); if(f){ fprintf(f,"ENTER HoOpening::Init\\n"); fclose(f);} }
	GameErrorValue = -1;
	ConnectErrorCount = 0;
	FocusWndFlag = FALSE;
	SetWindowText(hTextWnd, "");
	SendMessage(hTextWnd, EM_SETLIMITTEXT, 31, 0);

	{ FILE *f=fopen("mingw_hoopening_init.txt","a"); if(f){ fprintf(f,"after SendMessage EM_SETLIMITTEXT\\n"); fclose(f);} }

	LoginInputState = 0;
	ConnectError = ERROR_NONE;
	ServerWaitTimeCount = 0;
	StartQuitFlag = FALSE;

	WorkState = STATE_OPENING;
	StartConnectServerFlag = FALSE;
	ConnectServerSucessFlag = FALSE;

	FadeCurrentState = FADE_OUT;
	FadeNextState = FADE_OUT;

	LogoBlendValue = 0;
	LoginBlendValue = 0;
	ServerSelectBlendValue = 0;

	MidX = WinSizeX / 2;
	MidY = WinSizeY / 2;

	smRender.SMMULT_PERSPECTIVE_HEIGHT = RENDCLIP_WIDE_MULT_PERSPECTIVE_HEIGHT;

	SetWindowText(hTextWnd, NULL);

	memset(UserAccount, 0, sizeof(UserAccount));
	memset(UserPassword, 0, sizeof(UserPassword));

	TarDist = 400 * fONE;

	CaAngle.x = 100;
	CaAngle.y = 0;
	CaAngle.z = 0;

	SetFade();

	{ FILE *f=fopen("mingw_hoopening_init.txt","a"); if(f){ fprintf(f,"after SetFade\\n"); fclose(f);} }

	smInitColorZClip(0, 0, 0);

	{ FILE *f=fopen("mingw_hoopening_init.txt","a"); if(f){ fprintf(f,"after smInitColorZClip\\n"); fclose(f);} }

	smRender.Color_R = 40;
	smRender.Color_G = 0;
	smRender.Color_B = -30;
	smRender.Color_A = 0;
	smRender.ClearLight();

	{ FILE *f=fopen("mingw_hoopening_init.txt","a"); if(f){ fprintf(f,"after smRender.ClearLight\\n"); fclose(f);} }

	StopBGM();

	{ FILE *f=fopen("mingw_hoopening_init.txt","a"); if(f){ fprintf(f,"after StopBGM\\n"); fclose(f);} }
	OpenBGM("game//GUI//Opening//Login//UI//Intro.wav");

	{ FILE *f=fopen("mingw_hoopening_init.txt","a"); if(f){ fprintf(f,"after OpenBGM Intro.wav\\n"); fclose(f);} }
	PlayBGM();

	{ FILE *f=fopen("mingw_hoopening_init.txt","a"); if(f){ fprintf(f,"after PlayBGM\\n"); fclose(f);} }

	SetVolumeBGM(OpeningBgmVolume);

	{ FILE *f=fopen("mingw_hoopening_init.txt","a"); if(f){ fprintf(f,"after SetVolumeBGM\\n"); fclose(f);} }

	std::string id;

	try
	{
		File file("Settings.ini");

	{ FILE *f=fopen("mingw_hoopening_init.txt","a"); if(f){ fprintf(f,"after File Settings.ini constructor\\n"); fclose(f);} }

		id = file.readString("Game", "LastAccount");

	{ FILE *f=fopen("mingw_hoopening_init.txt","a"); if(f){ fprintf(f,"after read LastAccount\\n"); fclose(f);} }
	}
	catch (FileException& e)
	{
		char buffer[100] = { 0 };
		strcpy_s(buffer, sizeof(buffer), e.what());
		// // Utils_Log(LOG_GAME, buffer);
	}

	if (!id.empty())
	{
		memset(UserAccount, 0, sizeof(UserAccount));
		memcpy(UserAccount, id.c_str(), id.size());
		LoginInputState = 1;
	}

	CurrentWorkState = STATE_OPENING;

	{ FILE *f=fopen("mingw_hoopening_init.txt","a"); if(f){ fprintf(f,"after CurrentWorkState STATE_OPENING\\n"); fclose(f);} }

	NextWorkState = STATE_OPENING;
	{ FILE *f=fopen("mingw_hoopening_init.txt","a"); if(f){ fprintf(f,"after NextWorkState STATE_OPENING\\n"); fclose(f);} }

	MouseDownServerIndex = -1;
	{ FILE *f=fopen("mingw_hoopening_init.txt","a"); if(f){ fprintf(f,"after Init MouseDownServerIndex\\n"); fclose(f);} }

	MouseDownFlag = FALSE;
	{ FILE *f=fopen("mingw_hoopening_init.txt","a"); if(f){ fprintf(f,"after MouseDownFlag FALSE\\n"); fclose(f);} }

	CameraFrame = 160;
	{ FILE *f=fopen("mingw_hoopening_init.txt","a"); if(f){ fprintf(f,"after CameraFrame 160\\n"); fclose(f);} }

	{ FILE *f=fopen("mingw_hoopening_init.txt","a"); if(f){ fprintf(f,"before g_NewParticleMgr.Init\\n"); fclose(f);} }
	g_NewParticleMgr.Init();
	{ FILE *f=fopen("mingw_hoopening_init.txt","a"); if(f){ fprintf(f,"after g_NewParticleMgr.Init\\n"); fclose(f);} }

	{ FILE *f=fopen("mingw_hoopening_init.txt","a"); if(f){ fprintf(f,"before InitAssaEffect\\n"); fclose(f);} }
	InitAssaEffect();
	{ FILE *f=fopen("mingw_hoopening_init.txt","a"); if(f){ fprintf(f,"after InitAssaEffect\\n"); fclose(f);} }

	{ FILE *f=fopen("mingw_hoopening_init.txt","a"); if(f){ fprintf(f,"before InitCinemaFadeEffect\\n"); fclose(f);} }
	InitCinemaFadeEffect();
	{ FILE *f=fopen("mingw_hoopening_init.txt","a"); if(f){ fprintf(f,"after InitCinemaFadeEffect\\n"); fclose(f);} }

	smIdentityMatrix(m_CameraRotInvert);
	{ FILE *f=fopen("mingw_hoopening_init.txt","a"); if(f){ fprintf(f,"after smIdentityMatrix\\n"); fclose(f);} }

	m_IsDrawOpening = TRUE;
	{ FILE *f=fopen("mingw_hoopening_init.txt","a"); if(f){ fprintf(f,"after m_IsDrawOpening TRUE\\n"); fclose(f);} }
	m_AutoCameraFrameTime = 50;
	m_ParticleSpawnTimeCount = 0;

	s_AutoCamera.m_Frame = 0;


	m_AddWidth = (SETTINGSHANDLER->getWidth() - 800) / 2;
	m_AddHeight = (SETTINGSHANDLER->getHeight() - 600) / 2;



	startLogoX = 100 + m_AddWidth;
	startLogoY = 63 + m_AddHeight;
	startLogoAddX = 32;
	startAccountAddX = 230;
	startAccountAddY = 280;

	return TRUE;
}


extern void ReadTexSettings();

void HoOpening::SetCameraCharPos(void)
{
	int x = 0;
	int y = 0;
	int z = (440 << FLOATNS);

	smEMATRIX eCameraRotInvert;
	smEMatrixInvert(eCameraRotInvert, s_AutoCamera.m_eCameraRotMat);

	memcpy((smEMATRIX*)&m_CameraRotInvert, &eCameraRotInvert, sizeof(smEMATRIX));
	smMatrixShift(m_CameraRotInvert, 4);

	int rx = x * m_CameraRotInvert._11 +
		y * m_CameraRotInvert._21 +
		z * m_CameraRotInvert._31;

	int ry = x * m_CameraRotInvert._12 +
		y * m_CameraRotInvert._22 +
		z * m_CameraRotInvert._32;

	int rz = x * m_CameraRotInvert._13 +
		y * m_CameraRotInvert._23 +
		z * m_CameraRotInvert._33;

	rx >>= FLOATS_EM;
	ry >>= FLOATS_EM;
	rz >>= FLOATS_EM;

	lpCurPlayer->pX = int(s_AutoCamera.m_fCameraPos_x) + rx;
	lpCurPlayer->pY = int(s_AutoCamera.m_fCameraPos_y) + ry;
	lpCurPlayer->pZ = int(s_AutoCamera.m_fCameraPos_z) + rz;
}





void HoOpening::SetParticleEffectSpawn(void)
{
	m_ParticleSpawnTimeCount++;
	if ((m_ParticleSpawnTimeCount % 70) != 0)
		return;

	m_ParticleSpawnTimeCount = 0;


	POINT3D curPos;
	POINT3D desPos;
	int		rx, ry, rz;
	int		Type = (rand() % 100);
	int		rand_1 = (rand() % 1000) << 8;
	int		rand_2 = (rand() % 1000) << 8;
	int		rand_3 = (rand() % 1000) << 8;
	int		rand_4 = (rand() % 1000) << 8;


	if (Type < 10)
	{
		curPos.x = -rand_1;
		desPos.x = rand_2;
	}
	else
	{
		curPos.x = rand_1;
		desPos.x = -rand_2;
	}


	curPos.y = 200000;
	desPos.y = -200000;

	if (Type < 30)
	{
		curPos.z = rand_1;
		desPos.z = rand_2;
	}
	else
	{
		curPos.z = rand_3;
		desPos.z = rand_4;
	}


	rx = curPos.x * m_CameraRotInvert._11 +
		curPos.y * m_CameraRotInvert._21 +
		curPos.z * m_CameraRotInvert._31;

	ry = curPos.x * m_CameraRotInvert._12 +
		curPos.y * m_CameraRotInvert._22 +
		curPos.z * m_CameraRotInvert._32;

	rz = curPos.x * m_CameraRotInvert._13 +
		curPos.y * m_CameraRotInvert._23 +
		curPos.z * m_CameraRotInvert._33;

	rx >>= FLOATS_EM;
	ry >>= FLOATS_EM;
	rz >>= FLOATS_EM;

	curPos.x = lpCurPlayer->pX + rx;
	curPos.y = lpCurPlayer->pY + ry;
	curPos.z = lpCurPlayer->pZ + rz;


	rx = desPos.x * m_CameraRotInvert._11 +
		desPos.y * m_CameraRotInvert._21 +
		desPos.z * m_CameraRotInvert._31;

	ry = desPos.x * m_CameraRotInvert._12 +
		desPos.y * m_CameraRotInvert._22 +
		desPos.z * m_CameraRotInvert._32;

	rz = desPos.x * m_CameraRotInvert._13 +
		desPos.y * m_CameraRotInvert._23 +
		desPos.z * m_CameraRotInvert._33;

	rx >>= FLOATS_EM;
	ry >>= FLOATS_EM;
	rz >>= FLOATS_EM;

	desPos.x = lpCurPlayer->pX + rx;
	desPos.y = lpCurPlayer->pY + ry;
	desPos.z = lpCurPlayer->pZ + rz;


	AssaParticle_TerrainFire(&curPos, &desPos);
}






int HoOpening::DebugModeLoadFieldInfo(void)
{
	FILE* fp = fopen("game//GUI//Opening//Login//UI//OpeningField.txt", "rb");
	if (fp == NULL)
		return FALSE;

	Stage = NULL;


	char* p, line[256], decode[256], strBuff[256];
	while (!feof(fp))
	{
		if (fgets(line, 255, fp) == NULL)
		{
			fclose(fp);
			break;
		}

		p = GetWord(decode, line);


		if (lstrcmp(decode, "*�ʵ��") == 0)
		{
			p = GetWord(strBuff, p);
			Stage = smSTAGE3D_ReadASE(strBuff);
			if (Stage)
				Stage->Head = TRUE;
		}
		else if (lstrcmp(decode, "*ī�޶��") == 0)
		{
			p = GetWord(strBuff, p);
			s_AutoCamera.ReadASE_AutoCamera(strBuff);
		}
		else if (lstrcmp(decode, "*ī�޶�ӵ�") == 0)
		{
			p = GetWord(strBuff, p);
			m_AutoCameraFrameTime = atoi(strBuff);
		}
		else if (lstrcmp(decode, "*����������") == 0)
		{
			p = GetWord(strBuff, p);
			int StFrame = atoi(strBuff);

			p = GetWord(strBuff, p);
			int EndFrame = atoi(strBuff);

			s_AutoCamera.SetWarpFrame(StFrame, EndFrame);
		}
		else if (lstrcmp(decode, "*������ü") == 0)
		{
			p = GetWord(strBuff, p);
			Stage->StageObject->AddObjectFile(strBuff);
		}
		else if (lstrcmp(decode, "*�����ִ�") == 0)
		{
			p = GetWord(strBuff, p);
			Stage->StageObject->AddObjectFile(strBuff, strBuff);
		}
	}

	fclose(fp);

	if (Stage == NULL)
		return FALSE;

	return TRUE;
}

int GetClick(DWORD x, DWORD y, DWORD L, DWORD A)
{
	DWORD XMouse = pCursorPos.x;
	DWORD YMouse = pCursorPos.y;
	int Resultado = 0;

	extern bool clickable;

	if (clickable)
	{

		if (XMouse > x)
		{
			if (XMouse < (x + L - 1))
			{
				if (YMouse > y)
				{
					if (YMouse < (y + A - 1))
					{
						Resultado = 1;
					}
				}
			}
		}
	}
	return Resultado;
}

char ratio[4] = { 0 };
int BackGround;

int HoOpening::Load()
{


	int IsLoad = FALSE;
	PatCamera = NULL;
	ObjCamera = NULL;

	if (smConfig.getSecurity() >= AccountTypes::SEC_MODERATOR)
		IsLoad = DebugModeLoadFieldInfo();

	if (IsLoad == FALSE)
	{
		Stage = smSTAGE3D_ReadASE("StartImage//Opening//map//stage.ASE");

		if (Stage)
		{
			Stage->Head = TRUE;

			s_AutoCamera.ReadASE_AutoCamera("StartImage//Opening//opening_camera.ASE");
			m_AutoCameraFrameTime = 50;
			s_AutoCamera.SetWarpFrame(472, 473);

			Stage->StageObject->AddObjectFile("StartImage//Opening//map//st-ani02.ASE");
			Stage->StageObject->AddObjectFile("StartImage//Opening//map//st-ani03.ASE");

			Stage->StageObject->AddObjectFile("StartImage//Opening//babel//st-ani04.ASE", "StartImage//Opening//babel//st-ani04.ASE");
			Stage->StageObject->AddObjectFile("StartImage//Opening//npc//opening_NPC_A.ASE", "StartImage//Opening//npc//opening_NPC_A.ASE");
			Stage->StageObject->AddObjectFile("StartImage//Opening//npc//opening_NPC_B.ASE", "StartImage//Opening//npc//opening_NPC_B.ASE");
			Stage->StageObject->AddObjectFile("StartImage//Opening//npc//opening_NPC_C.ASE", "StartImage//Opening//npc//opening_NPC_C.ASE");
			Stage->StageObject->AddObjectFile("StartImage//Opening//npc//opening_NPC_D.ASE", "StartImage//Opening//npc//opening_NPC_D.ASE");
			Stage->StageObject->AddObjectFile("StartImage//Opening//npc//opening_NPC_E.ASE", "StartImage//Opening//npc//opening_NPC_E.ASE");
			Stage->StageObject->AddObjectFile("StartImage//Opening//npc//opening_NPC_F.ASE", "StartImage//Opening//npc//opening_NPC_F.ASE");

		}
	}

	if (Stage == NULL)
		return FALSE;

	smGameStage[0] = Stage;
	smGameStage[1] = NULL;


	/*GameLogo[0] = CreateTextureMaterial("game//GUI//Opening//Login//UI//GameLogo1.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	GameLogo[1] = CreateTextureMaterial("game//GUI//Opening//Login//UI//GameLogo2.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	GameLogo[2] = CreateTextureMaterial("game//GUI//Opening//Login//UI//GameLogo3.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	GameLogo[3] = CreateTextureMaterial("game//GUI//Opening//Login//UI//GameLogo4.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	GameLogo[4] = CreateTextureMaterial("game//GUI//Opening//Login//UI//GameLogo5.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	GameLogo[5] = CreateTextureMaterial("game//GUI//Opening//Login//UI//GameLogo6.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);*/

	ButtonLoginImage[0] = CreateTextureMaterial("game//Login//UI//cad.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	ButtonLoginImage[1] = CreateTextureMaterial("game//Login//UI//cad_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);

	ButtonQuitImage[0] = CreateTextureMaterial("game//Login//UI//S_Exit_.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	ButtonQuitImage[1] = CreateTextureMaterial("game//Login//UI//S_Exit.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);

	iInputID = CreateTextureMaterial("game//UI//insertbox.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	iBtnLogin[0] = CreateTextureMaterial("game//UI//btn_play.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	iBtnLogin[1] = CreateTextureMaterial("game//UI//btn_play_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	iBtnExit[0] = CreateTextureMaterial("game//UI//btn_exit.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	iBtnExit[1] = CreateTextureMaterial("game//UI//btn_exit_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Back = CreateTextureMaterial("game//UI//bg.jpg", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	bk = CreateTextureMaterial("game//UI//logo.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
#ifdef _LANGUAGE_JAPANESE
	PristonLogoImage = CreateTextureMaterial("game//GUI//Opening//Login//UI//BlackBox.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
#else
	PristonLogoImage = CreateTextureMaterial("game//Login//UI//priston_txt.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
#endif

	BlackBoxImage = CreateTextureMaterial("game//Login//UI//BlackBox.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);
	WhiteBoxImage = CreateTextureMaterial("game//Login//UI//WhiteBox.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);


	Message.Init();
	MessageBox.Init();

	ReadTexSettings();
	ReadTextures();

	if (Stage)
	{
		Stage->smMaterialGroup->CheckMatreialTextureSwap();
	}

	return TRUE;
}

BOOL bIsWidescreen = FALSE;


void HoOpening::resize()
{
	MidX = SETTINGSHANDLER->getWidth() / 2;
	MidY = SETTINGSHANDLER->getHeight() / 2;

	m_AddWidth = (SETTINGSHANDLER->getWidth() - 800) / 2;
	m_AddHeight = (SETTINGSHANDLER->getHeight() - 600) / 2;

	startLogoX = 100 + m_AddWidth;
	startLogoY = 63 + m_AddHeight;
}




int		CaretCount = 0;
DWORD	End = 0;
extern int Client_Version;

#define	DISP_LOGIN_LEN		15


BOOL bDone = FALSE;
int iSave = 0;

extern BOOL bAdjustWindow;

void SaveConfig()
{

}

extern int QuitGame();

static BOOL bs = FALSE;
int iTime = 0;
extern bool bCaptureOpening;
extern DWORD dwTickCap;

void Relog()
{
	if (bs == FALSE)
	{
		STARTUPINFOA si;
		PROCESS_INFORMATION pi;

		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));

		CreateProcessA(NULL, "Game.exe", NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

		bs = TRUE;
	}

	if (iTime >= 3)
		QuitGame();
	else
		iTime++;
}

extern void DrawImage(int Image, int x, int y, int w, int h);
bool clickable = true;

int HoOpening::Draw()
{
	smRender.SMMULT_PERSPECTIVE_HEIGHT = RENDCLIP_WIDE_MULT_PERSPECTIVE_HEIGHT;

	MidX = SETTINGSHANDLER->getWidth() / 2;
	MidY = SETTINGSHANDLER->getHeight() / 2;

	m_AddWidth = (SETTINGSHANDLER->getWidth() - 800) / 2;
	m_AddHeight = (SETTINGSHANDLER->getHeight() - 600) / 2;

	startLogoX = 100 + m_AddWidth;
	startLogoY = 63 + m_AddHeight;
	startLogoAddX = 32;
	startAccountAddX = 230;
	startAccountAddY = 290;

	{ FILE *f=fopen("mingw_mousebutton_frame.txt","a"); if(f){ fprintf(f,"DRAW before FastFade MouseButton0=%d x=%ld y=%ld CWS=%d NWS=%d LB=%d SB=%d\n", MouseButton[0], pCursorPos.x, pCursorPos.y, CurrentWorkState, NextWorkState, LoginBlendValue, ScreenBlendValue); fclose(f);} }
	FastFade();
	{ FILE *f=fopen("mingw_mousebutton_frame.txt","a"); if(f){ fprintf(f,"DRAW after FastFade MouseButton0=%d x=%ld y=%ld CWS=%d NWS=%d LB=%d SB=%d\n", MouseButton[0], pCursorPos.x, pCursorPos.y, CurrentWorkState, NextWorkState, LoginBlendValue, ScreenBlendValue); fclose(f);} }

	dsDrawTexImage(Back, (smScreenWidth / 2 - smScreenWidth / 2), (smScreenHeight / 2 - smScreenHeight / 2), smScreenWidth, smScreenHeight, 255);

	dsDrawTexImage(bk, MidX - 310, MidY - 330, 644, 318, LogoBlendValue);

	//DrawImage(iWindowID, smScreenWidth / 2 - 250, smScreenHeight / 2 - 270, 500, 500);
	dsDrawTexImage(iInputID, MidX - 158, MidY - 30, 283, 53, LogoBlendValue);

	dsDrawTexImage(iInputID, MidX - 158 , MidY + 37, 283, 53, LogoBlendValue);


	char Version[250];
	int v1 = Client_Version / 5000;
	int v2 = (Client_Version / 10) % 100;
	int v3 = Client_Version % 10;
	/*wsprintf(Version, "Vers�o %d", Client_Version);*/
	wsprintf(Version, "%s%d.%d.%d", "Ver: ", v1, v2, v3);
	SetFontTextColor(RGB(0, 0, 0));
	dsTextLineOut(SETTINGSHANDLER->getWidth() - 60, 15, Version, strlen(Version));


	{ FILE *f=fopen("mingw_mousebutton_frame.txt","a"); if(f){ fprintf(f,"DRAW before cSettings.isOpen block isOpen=%d MouseButton0=%d\n", cSettings.isOpen(), MouseButton[0]); fclose(f);} }
	if (cSettings.isOpen())
	{
		if (MouseButton[0])
			MouseButton[0] = 0;
	}
	{ FILE *f=fopen("mingw_mousebutton_frame.txt","a"); if(f){ fprintf(f,"DRAW after cSettings.isOpen block MouseButton0=%d\n", MouseButton[0]); fclose(f);} }

	//setings engrenagem
	{ FILE *f=fopen("mingw_mousebutton_frame.txt","a"); if(f){ fprintf(f,"DRAW before settings gear check MouseButton0=%d x=%ld y=%ld\n", MouseButton[0], pCursorPos.x, pCursorPos.y); fclose(f);} }
	if ((pCursorPos.x > 590) && (pCursorPos.x < 750) &&
		(pCursorPos.y > 15) && (pCursorPos.y < 51))
	{
		dsDrawTexImage(iSettingsButton[1], MidX - 98, MidY - 350, 161, 59, 255);

		if (MouseButton[0])
		{
			if (cSettings.isOpen())
				cSettings.Close();
			else
				cSettings.Open();

			MouseButton[0] = 0;
		}
	}
	else {
		dsDrawTexImage(iSettingsButton[0], MidX - 98, MidY - 350, 161, 59, 255);
	};

	{ FILE *f=fopen("mingw_mousebutton_frame.txt","a"); if(f){ fprintf(f,"DRAW before login hover/button checks MouseButton0=%d x=%ld y=%ld startLogoX=%d startLogoY=%d\n", MouseButton[0], pCursorPos.x, pCursorPos.y, startLogoX, startLogoY); fclose(f);} }
	if (pCursorPos.x > (startLogoX + 165) &&
		pCursorPos.y > (startLogoY + 320) &&
		pCursorPos.x < (startLogoX + 286) &&
		pCursorPos.y < (startLogoY + 371))
	{
		DrawImage(iBtnLogin[1], startLogoX + 137,
			startLogoY + 305,
			148, 70);
	}
	else
	{
		DrawImage(iBtnLogin[0], startLogoX + 137,
			startLogoY + 305,
			148, 70);
	}


	if (pCursorPos.x > (startLogoX + 285) &&
		pCursorPos.y > (startLogoY + 320) &&
		pCursorPos.x < (startLogoX + 395) &&
		pCursorPos.y < (startLogoY + 371))
	{
		DrawImage(iBtnExit[1], startLogoX + 284,
			startLogoY + 305,
			148, 70);

	}
	else
	{
		DrawImage(iBtnExit[0], startLogoX + 284,
			startLogoY + 305,
			148, 70);	
	}

	char buffer[256];
	memset(buffer, 0, sizeof(buffer));

	SetFontTextColor(RGB(255, 255, 255));

	if (bCaptureOpening)
	{
		time_t t = time(0);
		struct tm* now = localtime(&t);

		char localtime[128] = { 0 };
		strftime(localtime, sizeof(localtime), "Hora Local: %d-%m-%Y %X", now);

		dsTextLineOut((smScreenWidth / 2) - 90, 20, localtime, strlen(localtime));

		if (GetTickCount() - dwTickCap >= 170)
		{
			dwTickCap = 0;
			bCaptureOpening = false;
		}
	}



	{ FILE *f=fopen("mingw_mousebutton_frame.txt","a"); if(f){ fprintf(f,"DRAW before login input block CWS=%d NWS=%d ScreenBlend=%d LoginBlend=%d MouseButton0=%d\n", CurrentWorkState, NextWorkState, ScreenBlendValue, LoginBlendValue, MouseButton[0]); fclose(f);} }
	if (CurrentWorkState == STATE_LOGIN_INPUT && NextWorkState == STATE_LOGIN_INPUT && ScreenBlendValue <= 100)
	{
		{ FILE *f=fopen("mingw_mousebutton_frame.txt","a"); if(f){ fprintf(f,"DRAW ENTER login input block\n"); fclose(f);} }

		if (LoginBlendValue >= 255)
		{
			if (FocusWndFlag == FALSE)
			{
				FILE *f=fopen("mingw_login_input_deep.txt","a"); if(f){ fprintf(f,"before focus assign hTextWnd=%p hFocusWnd=%p\n",(void*)hTextWnd,(void*)hFocusWnd); fclose(f); }
				FocusWndFlag = TRUE;
				hFocusWnd = hTextWnd;
				f=fopen("mingw_login_input_deep.txt","a"); if(f){ fprintf(f,"after focus assign hTextWnd=%p hFocusWnd=%p\n",(void*)hTextWnd,(void*)hFocusWnd); fclose(f); }
			}

			if (pCursorPos.x > (startLogoX + 155) &&
				pCursorPos.y > (startLogoY + 215) &&
				pCursorPos.x < (startLogoX + 415) &&
				pCursorPos.y < (startLogoY + 260) &&
				MouseButton[0])
			{
				LoginInputState = 0;
			}

			if (pCursorPos.x > (startLogoX + 155) &&
				pCursorPos.y > (startLogoY + 285) &&
				pCursorPos.x < (startLogoX + 415) &&
				pCursorPos.y < (startLogoY + 335) &&
				MouseButton[0])
			{
				LoginInputState = 1;
			}

			if (StartConnectServerFlag == FALSE && ConnectError == ERROR_NONE)
			{
				if (pCursorPos.x > (startLogoX + 165) &&
					pCursorPos.y > (startLogoY + 320) &&
					pCursorPos.x < (startLogoX + 286) &&
					pCursorPos.y < (startLogoY + 371) &&
					MouseButton[0])
				{
					if (LoginInputState == 1)
					{
						MINGW_CheckUserAccount("CLICK_PLAY entered");
				MINGW_LoginTrace("CLICK_PLAY before LoginDataServer user=%s LoginInputState=%d StartConnectServerFlag=%d ConnectError=%d", UserAccount, LoginInputState, StartConnectServerFlag, ConnectError);
						CurrentWorkState = STATE_LOGIN_INPUT;
						LoginDataServer();
						MINGW_LoginTrace("CLICK_PLAY after LoginDataServer");
						LoginInputState = 0;
						MINGW_CheckUserAccount("before click SetWindowText");
				SetWindowText(hFocusWnd, UserAccount);
				MINGW_CheckUserAccount("after click SetWindowText");
						SendMessage(hTextWnd, EM_SETSEL, lstrlen(UserAccount), lstrlen(UserAccount) + 1);
					}
				}
				if (pCursorPos.x > (startLogoX + 285) &&
					pCursorPos.y > (startLogoY + 320) &&
					pCursorPos.x < (startLogoX + 395) &&
					pCursorPos.y < (startLogoY + 371) &&
					MouseButton[0])
				{
					FILE *f=fopen("mingw_login_exit_button.txt","a");
					if(f)
					{
						fprintf(f,"EXIT BUTTON HIT x=%ld y=%ld startLogoX=%d startLogoY=%d MouseButton0=%d LoginInputState=%d CurrentWorkState=%d NextWorkState=%d\n",
							pCursorPos.x, pCursorPos.y, startLogoX, startLogoY, MouseButton[0], LoginInputState, CurrentWorkState, NextWorkState);
						fclose(f);
					}

					/* TEMP_MINGW_BLOCK_AUTO_EXIT:
					   Não sair automaticamente enquanto diagnosticamos MouseButton/pCursorPos. */
					MouseButton[0] = 0;
				}
			}



			if (hFocusWnd && ConnectError == ERROR_NONE && StartConnectServerFlag == FALSE)
			{
				if (LoginInputState == 0)
				{
					{
						char className[64] = { 0 };
						GetClassNameA(hFocusWnd, className, sizeof(className));

						FILE *f = fopen("mingw_login_input_deep.txt", "a");
						if (f)
						{
							fprintf(
								f,
								"before GetWindowTextA UserAccount hFocusWnd=%p class=%s textLengthA=%d bufferSize=%u\\n",
								(void *)hFocusWnd,
								className,
								GetWindowTextLengthA(hFocusWnd),
								(unsigned int)sizeof(UserAccount)
							);
							fclose(f);
						}
					}

					MINGW_CheckUserAccount("before account memset/GetWindowTextA");
				memset(UserAccount, 0, sizeof(UserAccount));
					SetLastError(ERROR_SUCCESS);

					int copiedAccount = GetWindowTextA(
						hFocusWnd,
						UserAccount,
						(int)sizeof(UserAccount)
					);

					DWORD accountTextError = GetLastError();

				MINGW_CheckUserAccount("after account KeyFilter");

				{
					FILE *f = fopen("mingw_login_input_deep.txt", "a");
					if (f)
					{
						fprintf(
							f,
							"after KeyFilter UserAccount strlen=%u text=[%s] bytes=",
							(unsigned int)strlen(UserAccount),
							UserAccount
						);

						for (unsigned int i = 0;
							i < sizeof(UserAccount) && i < 32;
							++i)
						{
							fprintf(
								f,
								"%02X ",
								(unsigned int)(unsigned char)UserAccount[i]
							);
						}

						fprintf(f, "\\n");
						fclose(f);
					}
				}

				MINGW_CheckUserAccount("after account debug-file block");

				MINGW_CheckUserAccount("before account KeyFilter");
				KeyFilter(UserAccount);
				MINGW_CheckUserAccount("after account KeyFilter");

					{
						FILE *f = fopen("mingw_login_input_deep.txt", "a");
						if (f)
						{
							fprintf(
								f,
								"after KeyFilter UserAccount strlen=%u text=[%s] bytes=",
								(unsigned int)strlen(UserAccount),
								UserAccount
							);

							for (unsigned int i = 0;
								i < sizeof(UserAccount) && i < 32;
								++i)
							{
								fprintf(
									f,
									"%02X ",
									(unsigned int)(unsigned char)UserAccount[i]
								);
							}

							fprintf(f, "\\n");
							fclose(f);
						}
					}

				}
				else if (LoginInputState == 1)
				{
					{ FILE *f=fopen("mingw_login_input_deep.txt","a"); if(f){ fprintf(f,"before GetWindowText UserPassword hFocusWnd=%p\n",(void*)hFocusWnd); fclose(f);} }
					
#ifdef __MINGW32__
				/* MINGW_PASSWORD_PROBE_V1 */
				{
					FILE *f = fopen("mingw_password_probe.txt", "a");
					if (f)
					{
						fprintf(
							f,
							"BEFORE password GetWindowText state=%d hwnd=%p windowLen=%d bufferLen=%u\n",
							LoginInputState,
							(void *)hFocusWnd,
							GetWindowTextLengthA(hFocusWnd),
							(unsigned int)strlen(UserPassword)
						);
						fclose(f);
					}
				}
#endif
				GetWindowText(hFocusWnd, UserPassword, sizeof(UserPassword));
					{ FILE *f=fopen("mingw_login_input_deep.txt","a"); if(f){ fprintf(f,"after GetWindowText UserPassword\n"); fclose(f);} }
					KeyFilter(UserPassword);
#ifdef __MINGW32__
				{
					FILE *f = fopen("mingw_password_probe.txt", "a");
					if (f)
					{
						fprintf(
							f,
							"AFTER password GetWindowText state=%d hwnd=%p windowLen=%d passwordLen=%u\n",
							LoginInputState,
							(void *)hFocusWnd,
							GetWindowTextLengthA(hFocusWnd),
							(unsigned int)strlen(UserPassword)
						);
						fclose(f);
					}
				}
#endif

					{ FILE *f=fopen("mingw_login_input_deep.txt","a"); if(f){ fprintf(f,"after KeyFilter UserPassword\n"); fclose(f);} }
				}
			}

			if (hFocusWnd)
			{
				MINGW_CheckUserAccount("before account EM_GETSEL");

				FILE *f = fopen("mingw_login_input_deep.txt", "a");
				if (f)
				{
					fprintf(
						f,
						"before EM_GETSEL hFocusWnd=%p End_before=%d\n",
						(void *)hFocusWnd,
						End
					);
					fclose(f);
				}

				SendMessage(hFocusWnd, EM_GETSEL, 0, (LPARAM)&End);

				MINGW_CheckUserAccount("after account EM_GETSEL");

				f = fopen("mingw_login_input_deep.txt", "a");
				if (f)
				{
					fprintf(f, "after EM_GETSEL End=%d\n", End);
					fclose(f);
				}
			}

			{ FILE *f=fopen("mingw_login_text_draw.txt","a"); if(f){ fprintf(f,"before SetFontTextColor End=%d CaretCount=%d LoginInputState=%d hFocusWnd=%p\n", End, CaretCount, LoginInputState, (void*)hFocusWnd); fclose(f);} }
			MINGW_CheckUserAccount("before account SetFontTextColor");
			SetFontTextColor(RGB(255, 255, 255));
			MINGW_CheckUserAccount("after account SetFontTextColor");
			{ FILE *f=fopen("mingw_login_text_draw.txt","a"); if(f){ fprintf(f,"after SetFontTextColor\n"); fclose(f);} }

			if (hFocusWnd)
			{
				{ FILE *f=fopen("mingw_login_text_draw.txt","a"); if(f){ fprintf(f,"enter caret block CaretCount=%d End=%d\n", CaretCount, End); fclose(f);} }
				if (CaretCount < 16)
				{
					if (End > DISP_LOGIN_LEN) End = DISP_LOGIN_LEN;

					if (LoginInputState == 0)
					{
						{ FILE *f=fopen("mingw_login_text_draw.txt","a"); if(f){ fprintf(f,"before caret account draw x=%d y=%d End=%d\n", startLogoX + startAccountAddX + 25 + End * 7 + 1, startLogoY + startAccountAddY - 60, End); fclose(f);} }
						/* TEMP_MINGW_SKIP_LOGIN_CARET: dsTextLineOut("|") crashes in MinGW build */
						/* dsTextLineOut(startLogoX + startAccountAddX + 25 + End * 7 + 1, startLogoY + startAccountAddY - 60, "|", 1); */
						{ FILE *f=fopen("mingw_login_text_draw.txt","a"); if(f){ fprintf(f,"after caret account draw\n"); fclose(f);} }
					}
					else if (LoginInputState == 1)
					{
						{ FILE *f=fopen("mingw_login_text_draw.txt","a"); if(f){ fprintf(f,"before caret pass draw x=%d y=%d End=%d\n", startLogoX + startAccountAddX + 25 + End * 7 + 1, startLogoY + startAccountAddY + 4, End); fclose(f);} }
						/* TEMP_MINGW_SKIP_LOGIN_CARET: dsTextLineOut("|") crashes in MinGW build */
						/* dsTextLineOut(startLogoX + startAccountAddX + 25 + End * 7 + 1, startLogoY + startAccountAddY + 4, "|", 1); */
						{ FILE *f=fopen("mingw_login_text_draw.txt","a"); if(f){ fprintf(f,"after caret pass draw\n"); fclose(f);} }
					}
				}

				CaretCount++;
				if (CaretCount > 32)
					CaretCount = 0;
				
				{ 
					FILE *f=fopen("mingw_login_text_draw.txt","a"); if(f){ fprintf(f,"after caret block CaretCount=%d\n", CaretCount); fclose(f);} 
					MINGW_CheckUserAccount("after account caret block");
				}
			}

			int sp, len;

			sp = 0;
			len = lstrlen(UserAccount);
			{ FILE *f=fopen("mingw_login_text_draw.txt","a"); if(f){ fprintf(f,"before account len=%d text=%s\n", len, UserAccount); fclose(f);} }
			if (len > DISP_LOGIN_LEN)
			{
				sp = len - DISP_LOGIN_LEN;
				len -= sp;
			}
			{ FILE *f=fopen("mingw_login_text_draw.txt","a"); if(f){ fprintf(f,"before account text draw sp=%d len=%d ptr=%p\n", sp, len, (void*)(UserAccount + sp)); fclose(f);} }
			MINGW_CheckUserAccount("before account dsTextLineOut");
			/* MINGW_SAFE_ACCOUNT_DRAW_BEGIN
			 *
			 * Nunca entrega o buffer global UserAccount diretamente ao
			 * sistema de desenho. O cliente MinGW apresentou corrup��o deste
			 * buffer imediatamente depois de dsTextLineOut().
			 */
			{
				char mingwAccountBefore[ACCOUNT_SIZE];
				char mingwAccountDraw[ACCOUNT_SIZE];
				int mingwAccountDrawLen;
				int mingwAccountDrawOffset;

				memcpy(
					mingwAccountBefore,
					UserAccount,
					sizeof(mingwAccountBefore)
				);

				memcpy(
					mingwAccountDraw,
					UserAccount,
					sizeof(mingwAccountDraw)
				);

				mingwAccountBefore[ACCOUNT_SIZE - 1] = 0;
				mingwAccountDraw[ACCOUNT_SIZE - 1] = 0;

				mingwAccountDrawLen = (int)strlen(mingwAccountDraw);
				mingwAccountDrawOffset = sp;

				if (mingwAccountDrawOffset < 0)
					mingwAccountDrawOffset = 0;

				if (mingwAccountDrawOffset > mingwAccountDrawLen)
					mingwAccountDrawOffset = mingwAccountDrawLen;

				mingwAccountDrawLen -= mingwAccountDrawOffset;

				dsTextLineOut(
					startLogoX + startAccountAddX + 25,
					startLogoY + startAccountAddY - 60,
					mingwAccountDraw + mingwAccountDrawOffset,
					mingwAccountDrawLen
				);

				/*
				 * Se alguma rotina de desenho modificar o buffer global,
				 * regista a altera��o e recupera imediatamente a conta.
				 */
				if (memcmp(
					mingwAccountBefore,
					UserAccount,
					sizeof(mingwAccountBefore)
				) != 0)
				{
					FILE *mingwAccountLog =
						fopen("mingw_useraccount_draw_guard.txt", "a");

					if (mingwAccountLog)
					{
						unsigned int mingwIndex;

						fprintf(
							mingwAccountLog,
							"CORRUPTION after dsTextLineOut "
							"UserAccount=%p before=[%s] after-bytes=",
							(void *)UserAccount,
							mingwAccountBefore
						);

						for (
							mingwIndex = 0;
							mingwIndex < sizeof(mingwAccountBefore);
							++mingwIndex
						)
						{
							fprintf(
								mingwAccountLog,
								"%02X ",
								(unsigned int)
								(unsigned char)UserAccount[mingwIndex]
							);
						}

						fprintf(mingwAccountLog, "\n");
						fclose(mingwAccountLog);
					}

					memcpy(
						UserAccount,
						mingwAccountBefore,
						sizeof(mingwAccountBefore)
					);

					UserAccount[ACCOUNT_SIZE - 1] = 0;
				}
			}
			/* MINGW_SAFE_ACCOUNT_DRAW_END */
			MINGW_CheckUserAccount("after account dsTextLineOut");
			
			{ FILE *f=fopen("mingw_login_text_draw.txt","a"); if(f){ fprintf(f,"after account text draw\n"); fclose(f);} }

			len = lstrlen(UserPassword);
			{ FILE *f=fopen("mingw_login_text_draw.txt","a"); if(f){ fprintf(f,"before password mask len=%d\n", len); fclose(f);} }
			if (len > DISP_LOGIN_LEN) len = DISP_LOGIN_LEN;

			for (int count = 0; count < len; count++)
			{
				{ FILE *f=fopen("mingw_login_text_draw.txt","a"); if(f){ fprintf(f,"before password star draw count=%d\n", count); fclose(f);} }
				/* TEMP_MINGW_SKIP_LOGIN_PASSWORD_STAR: dsTextLineOut("*") crashes in MinGW build */
				/* dsTextLineOut( startLogoX + startAccountAddX + 25 + count * 6, startLogoY + startAccountAddY + 8, "*", 1); */
				{ FILE *f=fopen("mingw_login_text_draw.txt","a"); if(f){ fprintf(f,"after password star draw count=%d\n", count); fclose(f);} }
			}

		}
	}

	if (StartConnectServerFlag && ConnectError == ERROR_NONE)
		DrawMessage(295 + m_AddWidth, 300 + m_AddHeight, HoTextLoginMessage[0]);

	if (ConnectError == ERROR_CONNECT)
	{
		StartConnectServerFlag = FALSE;
		DrawMessage(295 + m_AddWidth, 300 + m_AddHeight, HoTextLoginMessage[1]);
	}
	else if (ConnectError == ERROR_NO_ACCOUNT)
		DrawMessage(295 + m_AddWidth, 300 + m_AddHeight, HoTextLoginMessage[2]);
	else if (ConnectError == ERROR_NO_PASSWORD)
		DrawMessage(295 + m_AddWidth, 300 + m_AddHeight, HoTextLoginMessage[3]);
	else if (ConnectError == ERROR_NO_BETA_TESTER)
		DrawMessage(205 + m_AddWidth, 300 + m_AddHeight, HoTextLoginMessage[4]);
	else if (ConnectError == ERROR_NO_ACCOUNT_IN)
		DrawMessage(295 + m_AddWidth, 300 + m_AddHeight, HoTextLoginMessage[5]);
	else if (ConnectError == ERROR_NO_VERSION)
		DrawMessage(295 + m_AddWidth, 300 + m_AddHeight, HoTextLoginMessage[6]);
	else if (ConnectError == ERROR_NO_LIMIT)
		DrawMessage(295 + m_AddWidth, 300 + m_AddHeight, HoTextLoginMessage[7]);
	else if (ConnectError == ERROR_NO_SERVER_CONNECT)
		DrawMessage(275 + m_AddWidth, 300 + m_AddHeight, HoTextLoginMessage[8]);
	else if (ConnectError == ERROR_NO_USE_DATE_OVER)
		DrawMessage(295 + m_AddWidth, 300 + m_AddHeight, HoTextLoginMessage[9]);
	else if (ConnectError == ERROR_NO_USE_TIME_OVER)
		DrawMessage(295 + m_AddWidth, 300 + m_AddHeight, HoTextLoginMessage[10]);
	else if (ConnectError == ERROR_DELETE_ACCOUNT_NOTIFY)
		DrawMessage(295 + m_AddWidth, 300 + m_AddHeight, HoTextLoginMessage[11]);
	else if (ConnectError == ERROR_SERVER_TRAFFIC_BUSY)
		DrawMessage(275 + m_AddWidth, 300 + m_AddHeight, HoTextLoginMessage[12]);
	else if (ConnectError == ERROR_SERVER_WAIT_MOMNENT)
		DrawMessage(295 + m_AddWidth, 300 + m_AddHeight, HoTextLoginMessage[13]);
	else if (ConnectError == ERROR_THIRD_CONNECT)
		DrawMessage(285 + m_AddWidth, 300 + m_AddHeight, HoTextLoginMessage[14], 30);
	else if (ConnectError == ERROR_HAN_PASSWORD)
		DrawMessage(285 + m_AddWidth, 300 + m_AddHeight, HoTextLoginMessage[15], 30);
	else if (ConnectError == ERROR_12YEARS)
		DrawMessage(285 + m_AddWidth, 300 + m_AddHeight, HoTextLoginMessage[16], 30);
	else if (ConnectError == ERROR_HOMEAGREE)
		DrawMessage(285 + m_AddWidth, 300 + m_AddHeight, HoTextLoginMessage[17], 30);
	else if (ConnectError == ERROR_UNTIL_LOGINDENIED)
		DrawMessage(295 + m_AddWidth, 300 + m_AddHeight, HoTextLoginMessage[18], 30);

	{ FILE *f=fopen("mingw_mousebutton_frame.txt","a"); if(f){ fprintf(f,"DRAW before cSettings.Draw\n"); fclose(f);} }
	MINGW_CheckUserAccount("before cSettings.Draw");
	cSettings.Draw();
	MINGW_CheckUserAccount("after cSettings.Draw");


	{ FILE *f=fopen("mingw_mousebutton_frame.txt","a"); if(f){ fprintf(f,"DRAW after cSettings.Draw return TRUE\n"); fclose(f);} }

	MINGW_CheckUserAccount("before HoOpening Draw return");
	return TRUE;
}




HANDLE	hHoLoginThread = 0;

DWORD WINAPI HoLoginThreadProc(void* pInfo)
{
	MINGW_LoginTrace("ENTER HoLoginThreadProc");
	HoOpening* lpHoOpening = (HoOpening*)pInfo;

	if (!smWsockDataServer)
	{
		MINGW_LoginTrace("before ConnectServer_InfoMain");
	ConnectServer_InfoMain();
	MINGW_LoginTrace("after ConnectServer_InfoMain smWsockDataServer=%p", smWsockDataServer);

		if (smWsockDataServer)
		{
			ServerUserCharacterInfo = NULL;
			DownServerListInfo = NULL;
			ServerLoginError = NULL;
			lpHoOpening->ServerWaitTimeCount = 0;

			if (HoCommandLineFlag)
			{
				memset(UserAccount, 0, sizeof(UserAccount));
				memset(UserPassword, 0, sizeof(UserPassword));

				memcpy(UserAccount, HoCommandInfo.Id, strlen(HoCommandInfo.Id));
				memcpy(UserPassword, HoCommandInfo.Pass, strlen(HoCommandInfo.Pass));
			}

			// MINGW_LoginTrace(
			// 	"before requestLogin user=%s",
			// 	UserAccount
			// );
			{
			FILE *f = fopen("mingw_login_thread.txt", "a");
			if (f)
			{
				fprintf(
					f,
					"before requestLogin addr=%p strlen=%u text=[%s] bytes=",
					(void *)UserAccount,
					(unsigned)strlen(UserAccount),
					UserAccount
				);

				for (unsigned i = 0; i < 32; ++i)
					fprintf(f, "%02X ", (unsigned char)UserAccount[i]);

				fprintf(f, "\n");
				fclose(f);
			}
		}

			MINGW_CheckUserAccount("immediately before requestLogin");

			LoginResult result =
				requestLogin(UserAccount, UserPassword);

			MINGW_LoginTrace(
				"after requestLogin result=%d",
				static_cast<int>(result)
			);

			if (result != LoginResult::OK)
			{
				MINGW_LoginTrace("before DisconnectServerFull");
	DisconnectServerFull();
	MINGW_LoginTrace("after DisconnectServerFull");
				lpHoOpening->ConnectError = ERROR_NO_VERSION;
				lpHoOpening->StartConnectServerFlag = FALSE;
				GameErrorValue = -1;
			}
		}
		else
		{
			lpHoOpening->ConnectError = ERROR_CONNECT;
			lpHoOpening->StartConnectServerFlag = FALSE;
		}
	}

	hHoLoginThread = 0;

	ExitThread(TRUE);
	MINGW_LoginTrace("LEAVE HoLoginThreadProc return TRUE");
	return TRUE;
}

int HoOpening::LoginDataServer()
{
	MINGW_LoginTrace("ENTER LoginDataServer");
	DWORD dwHoLoginThreadId;

	if (!hHoLoginThread)
	{
		MINGW_LoginTrace("before CreateThread");

		hHoLoginThread =
		CreateThread(
			NULL,
			0,
			HoLoginThreadProc,
			this,
			0,
			&dwHoLoginThreadId
		);

		MINGW_LoginTrace(
			"after CreateThread handle=%p threadId=%lu",
			hHoLoginThread,
			static_cast<unsigned long>(dwHoLoginThreadId)
		);

		//PlayWav2(13 - 1);
		StartConnectServerFlag = TRUE;
		return TRUE;
	}
	return FALSE;
}

int HoOpening::LoginGameServer(int serverIndex)
{

	PlayWav2(13 - 1);
	DisconnectServer_GameMain();


	if (ConnectServer_GameMain(UserServerListInfo[serverIndex].szServerIp1,
		UserServerListInfo[serverIndex].dwServerPort1,
		UserServerListInfo[serverIndex].szServerIp2,
		UserServerListInfo[serverIndex].dwServerPort2,
		UserServerListInfo[serverIndex].szServerIp3,
		UserServerListInfo[serverIndex].dwServerPort3))
	{
		StartConnectServerFlag = TRUE;
	}
	else
		ConnectError = ERROR_CONNECT;

	return TRUE;
}


int HoOpening::KeyFilter(char* message)
{
	if (message == NULL || hFocusWnd == NULL)
		return FALSE;

	if (strlen(message) >= (ACCOUNT_SIZE - 1) && message[ACCOUNT_SIZE - 2] != '/t')
	{
		message[ACCOUNT_SIZE - 2] = 0;
		SetWindowText(hFocusWnd, 0);
		SetWindowText(hFocusWnd, message);
		SendMessage(hFocusWnd, EM_SETSEL, lstrlen(message) + 1, lstrlen(message) + 1);
	}

	for (int index = 0; index < (int)strlen(message); index++)
	{
		if (message[index] == '\t')
		{
			message[index] = 0;
			SetWindowText(hFocusWnd, 0);

			if (LoginInputState == 0)
			{
				SetIME_Mode(0);
				LoginInputState = 1;
				SetWindowText(hFocusWnd, UserPassword);
				SendMessage(hTextWnd, EM_SETSEL, lstrlen(UserPassword) + 1, lstrlen(UserPassword) + 1);
			}
			else if (LoginInputState == 1)
			{
				LoginInputState = 0;
				SetWindowText(hFocusWnd, UserAccount);
				SendMessage(hTextWnd, EM_SETSEL, lstrlen(UserAccount) + 1, lstrlen(UserAccount) + 1);
			}
		}
		else if (message[index] >= 'a' && message[index] <= 'z'
			|| message[index] >= 'A' && message[index] <= 'Z'
			|| message[index] >= '0' && message[index] <= '9')
		{
			continue;
		}
		else
		{
			message[index] = 0;
			SetWindowText(hFocusWnd, message);
			SendMessage(hTextWnd, EM_SETSEL, lstrlen(message) + 1, lstrlen(message) + 1);
			break;
		}


	}
	return TRUE;
};


int HoOpening::MainFadeIn()
{
	if (FadeNextState == FADE_IN)
	{
		ScreenBlendValue -= 2;
		if (ScreenBlendValue < 0)
			ScreenBlendValue = 0;

		if (ScreenBlendValue <= 0)
		{
			ScreenBlendValue = 0;
			FadeCurrentState = FADE_IN;
			FadeNextState = FADE_IN;
		}
	}
	return TRUE;
}


int HoOpening::MainFadeOut()
{
	if (FadeNextState == FADE_OUT)
	{
		if (ScreenBlendValue < 255)
		{
			OpeningBgmVolume -= 2;
			if (OpeningBgmVolume < 0)
				OpeningBgmVolume = 0;
			ScreenBlendValue += 3;
		}

		if (ScreenBlendValue >= 255)
		{
			ScreenBlendValue = 255;
			LogoBlendValue = 255;
			LoginBlendValue = 255;
			FadeCurrentState = FADE_OUT;
			FadeNextState = FADE_OUT;
		}
	}
	return TRUE;
}


void HoOpening::InitCinemaFadeEffect()
{
	CinemaFadeOutFlag = FALSE;
	CinemaBlendValue = 0;
	CinemaFadeState = STATE_FADE_NONE;
}


void HoOpening::MainCinemaFadeEffect()
{
	if (CinemaFadeState == STATE_FADE_IN)
	{
		CinemaBlendValue += 5;
		if (CinemaBlendValue >= 255)
			CinemaBlendValue = 255;
	}

	if (CinemaFadeState == STATE_FADE_IN && CinemaBlendValue >= 255)
		CinemaFadeState = STATE_FADE_OUT;

	if (CinemaFadeState == STATE_FADE_OUT)
	{
		CinemaBlendValue -= 5;
		if (CinemaBlendValue <= 0)
			CinemaBlendValue = 0;
	}
}



void HoOpening::DrawCinemaFadeEffect()
{
	if (CinemaFadeState != STATE_FADE_NONE)
	{
		if (CinemaBlendValue >= 0 && CinemaBlendValue <= 255)
		{

			dsDrawTexImage(WhiteBoxImage, 0, 0, smScreenWidth, smScreenHeight, CinemaBlendValue);

		}
	}
}


int HoOpening::FastFade()
{

	if ((FadeNextState == FADE_IN && LoginBlendValue < 255) ||
		(FadeNextState == FADE_OUT && LoginBlendValue > 0))
	{

		static bool fastBlendFlag = false;
		for (int index = 0; index < 256; index++)
		{
			if (VRKeyBuff[index] == TRUE)
			{
				fastBlendFlag = true;
				break;
			}
		}

		if (MouseDownFlag == FALSE)
		{
			if (MouseButton[0] && fastBlendFlag == false)
			{
				fastBlendFlag = true;
				MouseDownFlag = TRUE;
			}
		}

		if (fastBlendFlag)
		{
			if (FadeNextState == FADE_IN)
			{
				if ((CurrentWorkState == STATE_OPENING &&
					NextWorkState == STATE_OPENING) ||
					(CurrentWorkState == STATE_OPENING && NextWorkState == STATE_LOGIN_INPUT))
				{
					LogoBlendValue = 255;
					ScreenBlendValue = 0;
					FadeCurrentState = FADE_IN;

					if (HoCommandLineFlag == FALSE)
					{
						LoginBlendValue = 255;
						CurrentWorkState = STATE_LOGIN_INPUT;
						NextWorkState = STATE_LOGIN_INPUT;
					}

					if (HoCommandLineFlag)
						NextWorkState = STATE_LOGIN_INPUT;

				}
			}
			else if (FadeNextState == FADE_OUT)
			{

				LogoBlendValue = 0;
				LoginBlendValue = 0;
				ScreenBlendValue = 255;
				OpeningBgmVolume = 0;
				FadeCurrentState = FADE_OUT;

			}
			fastBlendFlag = false;
		}
	}
	return TRUE;
}

int HoOpening::DrawFade()
{
	if (ScreenBlendValue >= 0 && ScreenBlendValue <= 255)
		dsDrawTexImage(BlackBoxImage, 0, 0, smScreenWidth, smScreenHeight, ScreenBlendValue);

	return TRUE;
}

int HoOpening::MainFade()
{
	if ((FadeNextState == FADE_IN) ||
		(FadeNextState == FADE_OUT))
	{
		if (FadeCurrentState != FadeNextState)
		{
			if (FadeNextState == FADE_IN)
				MainFadeIn();
			else
			{
				SetVolumeBGM(OpeningBgmVolume);
				MainFadeOut();
			}
		}
	}
	return TRUE;
}

void HoOpening::SetFade()
{


	if (FadeCurrentState == FADE_OUT)
	{
		FadeNextState = FADE_IN;
		ScreenBlendValue = 254;
		OpeningBgmVolume = 350;
	}
	else
	{
		FadeNextState = FADE_OUT;
		ScreenBlendValue = 1;
	}
}


void HoOpening::Close()
{
	if (Stage)
	{
		delete Stage;
		Stage = NULL;



		lpCurPlayer->pX = lpCurPlayer->pY = lpCurPlayer->pZ = 0;
		smGameStage[0] = NULL;
		smGameStage[1] = NULL;

	}

	if (PatCamera)
	{
		delete PatCamera;
		PatCamera = NULL;
	}

	BackColor_R = 0;
	BackColor_G = 0;
	BackColor_B = 0;

	BackColor_mR = 0;
	BackColor_mG = 0;
	BackColor_mB = 0;

	InitSky();








}






int HoOpening::Main()
{
	if (ConnectError != ERROR_NONE)
	{
		if (ConnectErrorCount >= 160)
		{
			MouseDownServerIndex = -1;
			bool errorEndFlag = false;

			if (MouseButton[0] || MouseDblClick)
				errorEndFlag = true;

			for (int index = 0; index < 256; index++)
			{
				if (VRKeyBuff[index] == TRUE)
				{
					errorEndFlag = true;
					break;
				}
			}

			if (ConnectErrorCount >= 250)
				errorEndFlag = true;

			if (errorEndFlag)
			{
				ConnectError = ERROR_NONE;
				FocusWndFlag = FALSE;
				ConnectErrorCount = 0;
			}

			if (errorEndFlag && HoCommandLineFlag
				&& CurrentWorkState == STATE_LOGIN_INPUT
				&& NextWorkState == STATE_LOGIN_INPUT)
			{

				HoCommandLineFlag = FALSE;
				NextWorkState = STATE_LOGIN_INPUT;
				CurrentWorkState = STATE_OPENING;
				ConnectServerSucessFlag = FALSE;
				DisconnectServerFull();
			}
		}
		ConnectErrorCount++;
	}

	MainFade();

	if (CurrentWorkState == STATE_OPENING && NextWorkState == STATE_OPENING)
	{
		if (CinemaFadeState == STATE_FADE_OUT)
			NextWorkState = STATE_LOGIN_INPUT;


	}

	if (CurrentWorkState == STATE_OPENING && NextWorkState == STATE_LOGIN_INPUT)
	{
		if (FadeNextState == FADE_IN)
		{
			LogoBlendValue += 1;
			if (LogoBlendValue >= 255)
				LogoBlendValue = 255;

			if (LogoBlendValue >= 255)
			{
				if (HoCommandLineFlag)
				{
					CurrentWorkState = STATE_LOGIN_INPUT;
					LoginDataServer();
				}
				else
				{
					LoginBlendValue += 1;
					if (LoginBlendValue >= 255)
					{
						LoginBlendValue = 255;
						CurrentWorkState = STATE_LOGIN_INPUT;
					}
				}
			}
		}
	}

	if (CurrentWorkState == STATE_LOGIN_INPUT
		&& NextWorkState == STATE_LOGIN_INPUT
		&& (LoginBlendValue >= 255)
		&& (ScreenBlendValue <= 0)
		&& StartConnectServerFlag == FALSE
		&& ConnectError == ERROR_NONE)
	{

		if (FocusWndFlag && hFocusWnd == NULL
			&& ConnectServerSucessFlag == FALSE
			&& ConnectError == ERROR_NONE)
		{
			LoginDataServer();
		}

		int downFlag = -1;
	}

	if (CurrentWorkState == STATE_SERVER_SELECT
		&& NextWorkState == STATE_SERVER_SELECT)
	{
		for (int index = 0; index < UserServerListInfoCount; index++)
		{
			if (ConnectError == ERROR_NONE && !StartConnectServerFlag)
			{

				MouseDownFlag = TRUE;
				MouseDownServerIndex = index;
				LoginGameServer(index);

			}
		}
	}

	if (StartConnectServerFlag)
	{
		if (ConnectError != ERROR_NONE)
		{
			StartConnectServerFlag = FALSE;
			ServerWaitTimeCount = 0;
		}

		if (ServerLoginError)
		{
			if (CurrentWorkState == STATE_LOGIN_INPUT
				&& NextWorkState == STATE_LOGIN_INPUT)
				DisconnectServerFull();

			if (CurrentWorkState == STATE_SERVER_SELECT
				&& NextWorkState == STATE_SERVER_SELECT)
				DisconnectServer_GameMain();


			if (ServerLoginError->WParam == 0)
				ConnectError = ERROR_CONNECT;
			else if (ServerLoginError->WParam == -1)
				ConnectError = ERROR_NO_ACCOUNT;
			else if (ServerLoginError->WParam == -2)
				ConnectError = ERROR_NO_PASSWORD;
			else if (ServerLoginError->WParam == -3)
				ConnectError = ERROR_NO_BETA_TESTER;
			else if (ServerLoginError->WParam == -4)
				ConnectError = ERROR_NO_ACCOUNT_IN;
			else if (ServerLoginError->WParam == -5)
				ConnectError = ERROR_NO_USE_DATE_OVER;
			else if (ServerLoginError->WParam == -6)
				ConnectError = ERROR_NO_USE_TIME_OVER;
			else if (ServerLoginError->WParam == -8)
				ConnectError = ERROR_DELETE_ACCOUNT_NOTIFY;
			else if (ServerLoginError->WParam == -16)
				ConnectError = ERROR_SERVER_TRAFFIC_BUSY;
			else if (ServerLoginError->WParam == -17)
				ConnectError = ERROR_SERVER_WAIT_MOMNENT;
			else if (ServerLoginError->WParam == -18)
				ConnectError = ERROR_THIRD_CONNECT;
			else if (ServerLoginError->WParam == -19)
				ConnectError = ERROR_HAN_PASSWORD;
			else if (ServerLoginError->WParam == -12)
				ConnectError = ERROR_12YEARS;
			else if (ServerLoginError->WParam == -13)
				ConnectError = ERROR_HOMEAGREE;
			else if (ServerLoginError->WParam == -23)
				ConnectError = ERROR_UNTIL_LOGINDENIED;
			else if (ServerLoginError->WParam == -24)
				ConnectError = ERROR_LOGIN_DENIED;

			StartConnectServerFlag = FALSE;
			ServerWaitTimeCount = 0;
			ServerLoginError = NULL;
		}

		ServerWaitTimeCount++;
		if (ServerWaitTimeCount >= 3000)
		{
			ServerWaitTimeCount = 0;
			ConnectError = ERROR_CONNECT;
			StartConnectServerFlag = FALSE;
			ConnectServerSucessFlag = FALSE;
		}

		if (CurrentWorkState == STATE_LOGIN_INPUT &&
			NextWorkState == STATE_LOGIN_INPUT)
		{
			if (ServerUserCharacterInfo
				&& DownServerListInfo
				&& ConnectError == ERROR_NONE
				&& GameErrorValue == 0)
			{
				StartConnectServerFlag = FALSE;
				ConnectServerSucessFlag = TRUE;
				CurrentWorkState = STATE_SERVER_SELECT;
				NextWorkState = STATE_SERVER_SELECT;
				GameErrorValue = -1;
			}
		}

		if (CurrentWorkState == STATE_SERVER_SELECT
			&& NextWorkState == STATE_SERVER_SELECT)
		{
			if (ConnectError == ERROR_NONE && GameErrorValue == 0)
			{
				StartConnectServerFlag = FALSE;
				SetFade();
				NextWorkState = STATE_END;
				GameErrorValue = -1;
			}
		}

		if (GameErrorValue != 0)
		{
			if (GameErrorValue == 1)
			{
				DisconnectServerFull();
				ConnectError = ERROR_NO_VERSION;
				StartConnectServerFlag = FALSE;
				GameErrorValue = -1;
			}

			if (GameErrorValue == 2)
			{
				if (CurrentWorkState == STATE_LOGIN_INPUT
					&& NextWorkState == STATE_LOGIN_INPUT)
					DisconnectServerFull();

				if (CurrentWorkState == STATE_SERVER_SELECT
					&& NextWorkState == STATE_SERVER_SELECT)
					DisconnectServer_GameMain();

				ConnectError = ERROR_NO_LIMIT;
				StartConnectServerFlag = FALSE;
				GameErrorValue = -1;
			}
		}
	}

	if (ConnectServerSucessFlag)
	{
		if (FadeCurrentState == FADE_OUT && FadeNextState == FADE_OUT)
		{
			try
			{
				File file("game.ini");

				if (file.readInt("Game", "RememberID"))
				{
					file.write("Game", "LastAccount", UserAccount);
				}
			}
			catch (FileException& e)
			{
				char buffer[100] = { 0 };
				strcpy_s(buffer, sizeof(buffer), e.what());
				//Log::getInstance()->output(LOG_GAME, buffer);
			}

			SetWindowText(hTextWnd, NULL);
			SendMessage(hTextWnd, EM_SETLIMITTEXT, 80, 0);

			StopBGM();
			return 2;
		}
	}

	if (StartQuitFlag)
	{
		if (FadeCurrentState == FADE_OUT && FadeNextState == FADE_OUT)
			quit = 1;
	}

	return 1;
}








