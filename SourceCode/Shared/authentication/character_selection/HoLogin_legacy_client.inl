#include <cstdio>
#include <atomic>
#include <mutex>
#include <new>
#include "opcodes.h"
#include "compat/legacy/sin/sinLinkHeader.h"
#include "systems/presentation/opening/legacy_runtime/HoLinkHeader.h"
#include "systems/localization/legacy/ho_text/HoTextFile.h"
#include "systems/rendering/effects/legacy_ho/core/HoMinMax.h"
#include "systems/rendering/effects/particles/ho/HoNewParticle.h"
#include "systems/rendering/effects/particles/ho/HoNewParticleMgr.h"

#include "systems/social/clan/tjclanDEF.h"
#include "systems/social/clan/tjclan.h"
#include "systems/social/chat/filter/CurseFilter.h"		

#include "systems/localization/legacy_catalog/language.h"
#include "compat/legacy/text/jts.h"

#include "systems/configuration/client/Settings.h"
#include "authentication/character_selection/ClassSelectionAdapter.h"
#include "authentication/character_selection/CharacterSelectionTrace.h"
#include "core/concurrency/JobSystem.h"
#include "core/concurrency/SourceAssetLock.h"
#include "systems/rendering/legacy_3d/smRead3d.h"
#include "systems/rendering/assets/v3/AssetStreamingV3.h"
#include "systems/gameplay/character/core/CharacterModelStreamingV3.h"

int  CharacterPosX = 0;
int  CharacterPosZ = 0;

int  MouseDblClick;

char SelectCharacterName1[3][500];
char SelectCharacterName2[3][500];

char JobSelectCharacterName1[CHARACTER_SELECTION_SLOT_CAPACITY][500];
char JobSelectCharacterName2[CHARACTER_SELECTION_SLOT_CAPACITY][500];


static int UserCharacterPos[CHARACTER_USER_MAX][2] =
{
	{ 0 ,-110 },
	{ -35 ,-90 },
	{ 35 ,-90 },
	{ -70 ,-60 },
	{ 70 ,-60 }
};


static int CharacterInfoPosi[5] = { 362, 243, 488, 154, 588 };

// Character-selection stage preload. The old HoLogin constructor parsed
// cSelect/login.ASE synchronously before the actual login screen could appear.
// V3 starts that CPU/IO work after the opening screen is initialized and only
// publishes the prepared stage from the main thread.
enum SummitHoLoginStagePreloadState
{
    SUMMIT_HOLOGIN_STAGE_IDLE = 0,
    SUMMIT_HOLOGIN_STAGE_LOADING,
    SUMMIT_HOLOGIN_STAGE_READY,
    SUMMIT_HOLOGIN_STAGE_FAILED
};

static std::atomic<int> gSummitHoLoginStageState(SUMMIT_HOLOGIN_STAGE_IDLE);
static std::mutex gSummitHoLoginStageMutex;
static smSTAGE3D *gSummitHoLoginPreparedStage = NULL;

static void SummitQueueHoLoginStagePreload()
{
    int state = gSummitHoLoginStageState.load(std::memory_order_acquire);
    if (state == SUMMIT_HOLOGIN_STAGE_FAILED)
        gSummitHoLoginStageState.compare_exchange_strong(
            state, SUMMIT_HOLOGIN_STAGE_IDLE, std::memory_order_acq_rel);

    int expected = SUMMIT_HOLOGIN_STAGE_IDLE;
    if(!gSummitHoLoginStageState.compare_exchange_strong(
        expected, SUMMIT_HOLOGIN_STAGE_LOADING, std::memory_order_acq_rel))
        return;

    const bool submitted = summit::core::concurrency::JobSystem::Instance().Submit([]() {
        smSTAGE3D *stage = new(std::nothrow) smSTAGE3D;
        if(!stage)
        {
            gSummitHoLoginStageState.store(SUMMIT_HOLOGIN_STAGE_FAILED, std::memory_order_release);
            return;
        }

        stage->VectLight.x = smConfig.MapLightVector.x;
        stage->VectLight.y = smConfig.MapLightVector.y;
        stage->VectLight.z = smConfig.MapLightVector.z;
        stage->Bright = smConfig.MapBright;
        stage->Contrast = smConfig.MapContrast;

        {
            // Serialize the two login/selection scene builders because legacy
            // material registration is still global even though parser scratch
            // has been made thread-local.
            summit::core::concurrency::SourceAssetLock sceneBuildLock("__summit_login_scene_build__");
            if(!smSTAGE3D_ReadASE(const_cast<char *>("cSelect//login.ASE"), stage))
            {
                delete stage;
                gSummitHoLoginStageState.store(SUMMIT_HOLOGIN_STAGE_FAILED, std::memory_order_release);
                return;
            }
        }

        stage->Head = TRUE;
        {
            std::lock_guard<std::mutex> lock(gSummitHoLoginStageMutex);
            gSummitHoLoginPreparedStage = stage;
        }
        gSummitHoLoginStageState.store(SUMMIT_HOLOGIN_STAGE_READY, std::memory_order_release);
    }, summit::core::concurrency::JobPriority::Normal);

    if(!submitted)
        gSummitHoLoginStageState.store(SUMMIT_HOLOGIN_STAGE_IDLE, std::memory_order_release);
}

static smSTAGE3D *SummitTakeHoLoginPreparedStage()
{
    if(gSummitHoLoginStageState.load(std::memory_order_acquire) != SUMMIT_HOLOGIN_STAGE_READY)
        return NULL;

    smSTAGE3D *stage = NULL;
    {
        std::lock_guard<std::mutex> lock(gSummitHoLoginStageMutex);
        stage = gSummitHoLoginPreparedStage;
        gSummitHoLoginPreparedStage = NULL;
    }
    gSummitHoLoginStageState.store(SUMMIT_HOLOGIN_STAGE_IDLE, std::memory_order_release);
    return stage;
}

static int LoadClassSelectionTexture(const char *root, const std::string& relativePath)
{
	if(!root || relativePath.empty()) return -1;
	char path[512];
	std::snprintf(path, sizeof(path), "%s%s", root, relativePath.c_str());
	return CreateTextureMaterial(path, 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
}

static void LoadClassSelectionAssets(const char *faceRoot,
                                     const char *classRoot,
                                     int legacyTribe,
                                     int jobImages[CHARACTER_SELECTION_SLOT_CAPACITY][2],
                                     int jobTextImages[CHARACTER_SELECTION_SLOT_CAPACITY],
                                     int faceImages[CHARACTER_SELECTION_SLOT_CAPACITY][CHARACTER_FACE_SLOT_CAPACITY][2])
{
	for(int classIndex = 0; classIndex < CHARACTER_SELECTION_SLOT_CAPACITY; ++classIndex)
	{
		jobImages[classIndex][0] = -1;
		jobImages[classIndex][1] = -1;
		jobTextImages[classIndex] = -1;
		for(int faceIndex = 0; faceIndex < CHARACTER_FACE_SLOT_CAPACITY; ++faceIndex)
		{
			faceImages[classIndex][faceIndex][0] = -1;
			faceImages[classIndex][faceIndex][1] = -1;
		}
	}

	const std::vector<const gameplay::classes::PlayableClassDefinition*> classes =
		authentication::character_selection::DefaultClassSelectionAdapter().Classes(legacyTribe);
	for(std::size_t classIndex = 0; classIndex < classes.size() && classIndex < static_cast<std::size_t>(CHARACTER_SELECTION_SLOT_CAPACITY); ++classIndex)
	{
		const gameplay::classes::PlayableClassDefinition& definition = *classes[classIndex];
		jobImages[classIndex][0] = LoadClassSelectionTexture(classRoot, definition.creation.presentation.labelNormal);
		jobImages[classIndex][1] = LoadClassSelectionTexture(classRoot, definition.creation.presentation.labelSelected);
		jobTextImages[classIndex] = LoadClassSelectionTexture(classRoot, definition.creation.presentation.description);
		for(std::size_t faceIndex = 0; faceIndex < definition.creation.faces.size() && faceIndex < static_cast<std::size_t>(CHARACTER_FACE_SLOT_CAPACITY); ++faceIndex)
		{
			faceImages[classIndex][faceIndex][0] = LoadClassSelectionTexture(faceRoot, definition.creation.faces[faceIndex].previewNormal);
			faceImages[classIndex][faceIndex][1] = LoadClassSelectionTexture(faceRoot, definition.creation.faces[faceIndex].previewSelected);
		}
	}
}


HoNewCharInfo	NewCharInfo;

extern smCHAR chrPlayer;
extern smCHAR chrEachPlayer;
extern smCHAR *chrEachMaster;
extern int MatEachBox;

static int GetSelRectChar(smCHAR *lpChar);


static int GetSelRectJobChar(smCHAR *lpChar);
static int CameraZPos[5] = { fONE * 300, fONE * 300 + fONE * 70, fONE * 300 + fONE * 250, fONE * 300 + fONE * 420, fONE * 300 + fONE * 400 };


HoLogin::HoLogin()
{
	ErrorSelect = ERROR_NONE;
	SelectCurrentState = CHARACTER_SELECT_STATE;
	SelectNextState = CHARACTER_SELECT_STATE;


	UserCharacterCount = 0;
	JobCharacterCount = 0;




	FaceSelectImage = NULL;


	TempscronMarkImage = -1;
	TempscronLogoImage = -1;

	GameMapImage = NULL;


	MoryonMarkImage = -1;
	MoryonLogoImage = -1;
	Font = NULL;
	Stage = NULL;
	ResourcesLoaded = FALSE;
	OpeningStageTexturesReady = false;
	Init_UseMor();

}

HoLogin::~HoLogin()
{
	Close();
}

void HoLogin::Init_UseMor()
{
	menuPos[0].left = 270;
	menuPos[0].right = menuPos[0].left + 128,
		menuPos[0].top = 166,
		menuPos[0].bottom = menuPos[0].top + 128;

	menuPos[1].left = 430;
	menuPos[1].right = menuPos[1].left + 128,
		menuPos[1].top = 166,
		menuPos[1].bottom = menuPos[1].top + 128;
}


/* MINGW_HOLOGIN_INIT_STEPS_V2 */
static inline FILE *SummitHoLoginRuntimeTraceFile(const char *path, const char *mode = "a")
{
#if defined(__MINGW32__) && defined(SUMMIT_ENABLE_MINGW_RUNTIME_TRACE)
    return fopen(path, mode ? mode : "a");
#else
    (void)path;
    (void)mode;
    return NULL;
#endif
}

static void MingwHoLoginInitStepV2(const char *step)
{
    FILE *f = SummitHoLoginRuntimeTraceFile("LOGS\\mingw_hologin_init_steps.txt", "a");
    if(f)
    {
        fprintf(f, "%s\n", step);
        fflush(f);
        fclose(f);
    }
}

void HoLogin::BeginUserCharacterPreload()
{
	if (UserCharacterCount > 0 || DownUserCharacterCount <= 0)
		return;

	const int count = DownUserCharacterCount < CHARACTER_USER_MAX ?
		DownUserCharacterCount : CHARACTER_USER_MAX;
	for (int index = 0; index < count; ++index)
	{
		AddUserCharacter(DownUserCharacterInfo[index].BodyName,
			DownUserCharacterInfo[index].FaceName,
			DownUserCharacterInfo[index].Job);
	}
}

void HoLogin::PumpOpeningPreload()
{
	// Register the character-selection UI while the account login is visible.
	// CreateTextureMaterial() is non-blocking under V3: it registers handles and
	// queues CPU decode/upload work instead of doing source I/O on this thread.
	// Previously this first happened in Init(), so reaching character selection
	// created a second visible loading burst even though the 3D stage was preloaded.
	if (!ResourcesLoaded)
		Load();

	PumpPreloadStage();
	if (Stage && Stage->smMaterialGroup)
	{
		// The future character-selection scene is already known while the user
		// is still on login. Warm a bounded slice every opening frame so the
		// environment does not begin texture residency only after transition.
		const int remainingStageTextures = AssetLoadV2PrewarmMaterialGroupStep(
			Stage->smMaterialGroup, SMASSET_OWNER_UI, 1, 2u, 2);
		OpeningStageTexturesReady = remainingStageTextures == 0;
		if (OpeningStageTexturesReady)
			Stage->smMaterialGroup->CheckMatreialTextureSwap();
	}
	else if (Stage)
	{
		OpeningStageTexturesReady = true;
	}
}

bool HoLogin::IsOpeningPreloadReady() const
{
	if (!ResourcesLoaded || !Stage || !OpeningStageTexturesReady)
		return false;

	// StartImage/login textures remain on the interactive retry queue until
	// resident (or conclusively failed), which is stronger than an FPS heuristic.
	if (AssetLoadV3HasPendingInteractiveLoginTextures())
		return false;

	const int expectedCharacters = DownUserCharacterCount < CHARACTER_USER_MAX ?
		DownUserCharacterCount : CHARACTER_USER_MAX;
	if (expectedCharacters > 0)
	{
		if (UserCharacterCount < expectedCharacters)
			return false;
		for (int index = 0; index < expectedCharacters; ++index)
		{
			if (UserCharacter[index].PatLoading)
				return false;
		}
	}

	return true;
}

int HoLogin::Init()
{
	AssetLoadV3SetLoadingBoost(true);
	Load();
	BeginPreload();
	PumpOpeningPreload();
	// The server character list is available before this transition completes.
	// Start visible previews immediately so model parsing overlaps the remaining
	// legacy selection initialization instead of beginning at the very end.
	BeginUserCharacterPreload();
	MingwHoLoginInitStepV2("STEP 00 ENTER HoLogin::Init");
	StartErrorDelayFlag = FALSE;
	ClanMarkUpdateSuccessFlag = FALSE;
	MingwHoLoginInitStepV2("STEP 01 BEFORE ParticleMgr.Init");
	g_NewParticleMgr.Init();
	MingwHoLoginInitStepV2("STEP 02 AFTER ParticleMgr.Init");
	MingwHoLoginInitStepV2("STEP 03 BEFORE InitAssaEffect");
	InitAssaEffect();
	MingwHoLoginInitStepV2("STEP 04 AFTER InitAssaEffect");
	POINT3D pos;

	pos.x = -70 * fONE;
	pos.y = 55 * fONE;
	pos.z = -156 * fONE;

	MingwHoLoginInitStepV2("STEP 05 BEFORE Fire#1");
	g_NewParticleMgr.Start("Fire", pos);
	MingwHoLoginInitStepV2("STEP 06 AFTER Fire#1");
	MingwHoLoginInitStepV2("STEP 07 BEFORE Fire#2");
	g_NewParticleMgr.Start("Fire", pos);
	MingwHoLoginInitStepV2("STEP 08 AFTER Fire#2");

	pos.x = -77 * fONE;
	pos.y = 60 * fONE;
	pos.z = -143 * fONE;
	MingwHoLoginInitStepV2("STEP 09 BEFORE Fire#3");
	g_NewParticleMgr.Start("Fire", pos);
	MingwHoLoginInitStepV2("STEP 10 AFTER Fire#3");

	pos.x = -81 * fONE;
	pos.y = 65 * fONE;
	pos.z = -143 * fONE;
	MingwHoLoginInitStepV2("STEP 11 BEFORE Fire#4");
	g_NewParticleMgr.Start("Fire", pos);
	MingwHoLoginInitStepV2("STEP 12 AFTER Fire#4");


	pos.x = 70 * fONE;
	pos.y = 56 * fONE;
	pos.z = -155 * fONE;
	MingwHoLoginInitStepV2("STEP 13 BEFORE Fire#5");
	g_NewParticleMgr.Start("Fire", pos);
	MingwHoLoginInitStepV2("STEP 14 AFTER Fire#5");

	pos.x = 75 * fONE;
	pos.y = 60 * fONE;
	pos.z = -152 * fONE;
	MingwHoLoginInitStepV2("STEP 15 BEFORE Fire#6");
	g_NewParticleMgr.Start("Fire", pos);
	MingwHoLoginInitStepV2("STEP 16 AFTER Fire#6");

	pos.x = 81 * fONE;
	pos.y = 66 * fONE;
	pos.z = -142 * fONE;
	MingwHoLoginInitStepV2("STEP 17 BEFORE Fire#7");
	g_NewParticleMgr.Start("Fire", pos);
	MingwHoLoginInitStepV2("STEP 18 AFTER Fire#7");

	FocusWndFlag = FALSE;
	CharacterStopFlag = FALSE;
	CharacterReturnFlag = FALSE;

	NewCharacterImageBlendCount = 50;
	NewCharacterImageBlendFlag = false;

	MapBlendCount = 0;
	MapBlendFlag = FALSE;
	memset(UserCharacterNewName, 0, sizeof(UserCharacterNewName));

	FadeCurrentState = FADE_OUT;
	FadeNextState = FADE_OUT;

	SelectCurrentState = CHARACTER_SELECT_STATE;
	SelectNextState = CHARACTER_SELECT_STATE;
	SelectSubState = SUB_SELECT_NONE;


	TarDist = CameraZPos[SelectCurrentState];
	CaAngle.x = 90;
	CaAngle.y = 0;
	CaAngle.z = 0;
	TarPosi.x = 0;
	TarPosi.y = 16 * fONE;
	TarPosi.z = 0;

	smRender.SMMULT_PERSPECTIVE_HEIGHT = RENDCLIP_DEFAULT_MULT_PERSPECTIVE_HEIGHT;
	MidX = WinSizeX / 2;
	MidY = WinSizeY / 2;

	MouseDownCharacterNum = -1;
	MouseMoveCharacterNum = -1;
	JobBodySelectNum = -1;
	JobFaceSelectNum = -1;

	RaceMarkBlendCount = 0;

	FadeCurrentState = FADE_OUT;
	FadeNextState = FADE_OUT;


	MingwHoLoginInitStepV2("STEP 20 BEFORE StopBGM");
	StopBGM();
	MingwHoLoginInitStepV2("STEP 21 AFTER StopBGM");
	MingwHoLoginInitStepV2("STEP 22 BEFORE OpenBGM");
	OpenBGM("StartImage//login//CharacterSelect.wav");
	MingwHoLoginInitStepV2("STEP 23 AFTER OpenBGM");
	MingwHoLoginInitStepV2("STEP 24 BEFORE PlayBGM");
	PlayBGM();
	MingwHoLoginInitStepV2("STEP 25 AFTER PlayBGM");

	MingwHoLoginInitStepV2("STEP 26 BEFORE PlayWav2#1");
	PlayWav2(9 - 1, 1);
	MingwHoLoginInitStepV2("STEP 27 AFTER PlayWav2#1");
	MingwHoLoginInitStepV2("STEP 28 BEFORE PlayWav2#2");
	PlayWav2(10 - 1, 1);
	MingwHoLoginInitStepV2("STEP 29 AFTER PlayWav2#2");
	MingwHoLoginInitStepV2("STEP 30 BEFORE PlayWav2#3");
	PlayWav2(11 - 1, 1);
	MingwHoLoginInitStepV2("STEP 31 AFTER PlayWav2#3");

	SetVolumeBGM(0);
	PlayWav_VolChange(9 - 1, 0);
	PlayWav_VolChange(10 - 1, 0);
	PlayWav_VolChange(11 - 1, 0);

	MingwHoLoginInitStepV2("STEP 32 BEFORE SetFade");
	SetFade();
	MingwHoLoginInitStepV2("STEP 33 AFTER SetFade");
	MingwHoLoginInitStepV2("STEP 34 BEFORE smInitColorZClip");
	smInitColorZClip(21, 17, 14);
	MingwHoLoginInitStepV2("STEP 35 AFTER smInitColorZClip"); // Loading apos selecionar personagem
	smRender.Color_R = 0;
	smRender.Color_G = 0;
	smRender.Color_B = 0;
	smRender.Color_A = 0;


	void InitSelectCha();
	MingwHoLoginInitStepV2("STEP 40 BEFORE InitSelectCha");
	InitSelectCha();
	MingwHoLoginInitStepV2("STEP 41 AFTER InitSelectCha");


	void web_DB_start();
	MingwHoLoginInitStepV2("STEP 42 BEFORE web_DB_start");
	web_DB_start();
	MingwHoLoginInitStepV2("STEP 43 AFTER web_DB_start");


	void clanMENU_INIT(int setN);
	MingwHoLoginInitStepV2("STEP 44 BEFORE clanMENU_INIT");
	clanMENU_INIT(0);
	MingwHoLoginInitStepV2("STEP 45 AFTER clanMENU_INIT");


	void ClearSelectCha();
	MingwHoLoginInitStepV2("STEP 46 BEFORE ClearSelectCha");
	ClearSelectCha();
	MingwHoLoginInitStepV2("STEP 47 AFTER ClearSelectCha");

	int HONG_GetChaClanName(char* id, char* chaname);
	if (DownUserCharacterCount > 0)
	{
		// Model requests were started at the top of Init() (or directly when the
		// list arrived from the server). Keep this point for legacy clan metadata.
		if (UserCharacterCount == 0)
			BeginUserCharacterPreload();
		for (int index = 0; index < DownUserCharacterCount; index++)
			HONG_GetChaClanName(UserAccount, DownUserCharacterInfo[index].Name);
	}

	m_ScreenHalf = SETTINGSHANDLER->getWidth() / 2;
	MingwHoLoginInitStepV2("STEP 99 LEAVE HoLogin::Init SUCCESS");
	return TRUE;
}


int HoLogin::Load()
{
	if (ResourcesLoaded)
		return TRUE;

	char dwFontGame[32] = { 0 };
	//int SizeFontGame = 0;
	GetPrivateProfileString("FontG", "Type", "", dwFontGame, 32, ".//game//font//Config.ini");
	//SizeFontGame = GetPrivateProfileInt("FontG", "Size", 0, ".//game//font//Config.ini");

#ifdef _LANGUAGE_BRAZIL
	Font = CreateFont(16,
		0,
		0,
		0,
		FW_NORMAL,
		FALSE,
		FALSE,
		FALSE,
		ANSI_CHARSET | FS_LATIN1,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		VARIABLE_PITCH,
		dwFontGame);
#endif

	OkButton.Init("StartImage//login//CharSelect//Bt_select.tga", 128, 32);
	OkButton.InitCrashRect(10, 0, 110, 32);

	BackButton.Init("StartImage//login//CharSelect//Bt_back.tga", 128, 32);
	BackButton.InitCrashRect(20, 0, 103, 32);

	NewCharButton.Init("StartImage//login//CharSelect//bt-Newchar.tga", 128, 32);
	NewCharButton.InitCrashRect(10, 0, 110, 32);

	QuitButton.Init("StartImage//login//CharSelect//Bt_exit.tga", 128, 32);
	QuitButton.InitCrashRect(0, 0, 128, 32);

	DeleteButton.Init("StartImage//login//CharSelect//Bt_delet.tga", 128, 32);
	DeleteButton.InitCrashRect(10, 0, 108, 32);


	UserCharacterInfoBackImage = CreateTextureMaterial("StartImage//login//UserCharacterInfo.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	UserCharacterInfoImage = CreateTextureMaterial("StartImage//login//CharacterInfo.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);


	TempscronMarkImage = CreateTextureMaterial("StartImage//login//CharSelect//mark_T.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	TempscronLogoImage = CreateTextureMaterial("StartImage//login//CharSelect//T-tempscron.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);

	MoryonMarkImage = CreateTextureMaterial("StartImage//login//CharSelect//mark_M.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	MoryonLogoImage = CreateTextureMaterial("StartImage//login//CharSelect//T-Morion.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);

	FaceSelectImage = LoadDibSurfaceOffscreen("StartImage//login//CharSelect//SelectFace.bmp"); //aqui

	MatEachBox = -1;

	BlackBoxImage = CreateTextureMaterial("StartImage//login//BlackBox.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);

	SYSTEMTIME st;
	GetLocalTime(&st);

	Message.Init();
	MessageBox.Init();
	InitMessageBox();

	MainBoxFrameImage1[0] = CreateTextureMaterial("StartImage//login//CharSelect//mainbox_L.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);
	MainBoxFrameImage1[1] = CreateTextureMaterial("StartImage//login//CharSelect//mainbox_T.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);
	MainBoxFrameImage1[2] = CreateTextureMaterial("StartImage//login//CharSelect//mainbox_R.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);
	MainBoxFrameImage1[3] = CreateTextureMaterial("StartImage//login//CharSelect//mainbox_D.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);

	MainBoxFrameImage2[0] = CreateTextureMaterial("StartImage//login//CharSelect//mainbox_CN1.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);
	MainBoxFrameImage2[1] = CreateTextureMaterial("StartImage//login//CharSelect//mainbox_CN2.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);
	MainBoxFrameImage2[2] = CreateTextureMaterial("StartImage//login//CharSelect//mainbox_CN3.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);
	MainBoxFrameImage2[3] = CreateTextureMaterial("StartImage//login//CharSelect//mainbox_CN4.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);

	TitleBoxFrameImage = CreateTextureMaterial("StartImage//login//CharSelect//title-box.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);
	TitleNameImage = CreateTextureMaterial("StartImage//login//CharSelect//title-txt30.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);

	TempscronTextImage = CreateTextureMaterial("StartImage//login//CharSelect//T-ex_tscr.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);
	MoryonTextImage = CreateTextureMaterial("StartImage//login//CharSelect//T-ex_mrin.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);

	RaceTextBoxImage[0] = CreateTextureMaterial("StartImage//login//CharSelect//mark_box1.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);
	RaceTextBoxImage[1] = CreateTextureMaterial("StartImage//login//CharSelect//mark_box2.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);

	JobTextBoxImage[0] = CreateTextureMaterial("StartImage//login//CharSelect//T_cls-box1.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);
	JobTextBoxImage[1] = CreateTextureMaterial("StartImage//login//CharSelect//T_cls-box2.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);

	LoadClassSelectionAssets("StartImage//login//", "StartImage//login//CharSelect//",
	                         TRIBE_Tempscron, TempscronJobImage, TempscronJobTextImage, TempFaceImage);
	LoadClassSelectionAssets("StartImage//login//", "StartImage//login//CharSelect//",
	                         TRIBE_Moryon, MoryonJobImage, MoryonJobTextImage, MorFaceImage);
	FaceBoxBigImage[0] = CreateTextureMaterial("StartImage//login//CharSelect//face_bigbox1.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	FaceBoxBigImage[1] = CreateTextureMaterial("StartImage//login//CharSelect//face_bigbox2.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);

	FaceBoxSmallImage[0] = CreateTextureMaterial("StartImage//login//CharSelect//face_box_.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	FaceBoxSmallImage[1] = CreateTextureMaterial("StartImage//login//CharSelect//face_box.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	FaceBoxSmallImage[2] = CreateTextureMaterial("StartImage//login//CharSelect//face_box2.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);

	NewCharacterImage = CreateTextureMaterial("StartImage//login//CharSelect//New_char.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);

	// Login/selection materials self-admit through the V3 HIGH UI lane. Avoid
	// a global prewarm sweep here: it can enqueue unrelated effects/world assets
	// exactly while the interactive screen is trying to become responsive.
	ResourcesLoaded = TRUE;
	return TRUE;
}

void HoLogin::BeginPreload()
{
	if (Stage)
		return;
	OpeningStageTexturesReady = false;
	SummitQueueHoLoginStagePreload();
}

void HoLogin::FinalizeStageReady()
{
	if (!Stage)
		return;

	if (Stage->smMaterialGroup)
	{
        // Prime only a small interactive slice. Remaining selection-scene
        // textures continue incrementally from Main() instead of one burst.
        AssetLoadV2PrewarmMaterialGroupStep(Stage->smMaterialGroup,
            SMASSET_OWNER_UI, 1, 4u, 1);
		Stage->smMaterialGroup->CheckMatreialTextureSwap();
	}

	for (int index = 0; index < UserCharacterCount; ++index)
	{
		UserCharacter[index].lpStage = Stage;
		UserCharacter[index].SetPosi(UserCharacterPos[index][0] * fONE,
			Stage->GetHeight(UserCharacterPos[index][0] * fONE, UserCharacterPos[index][1] * fONE),
			UserCharacterPos[index][1] * fONE, 0, ANGLE_180, 0);
	}

	for (int index = 0; index < JobCharacterCount; ++index)
	{
		int classPosX = 0;
		int classPosZ = 0;
		GetJobCharacterWorldPosition(index, classPosX, classPosZ);
		JobCharacter[index].lpStage = Stage;
		JobCharacter[index].SetPosi(classPosX * fONE,
			Stage->GetHeight(classPosX * fONE, classPosZ * fONE),
			classPosZ * fONE, 0, ANGLE_180, 0);
	}
}

void HoLogin::PumpPreloadStage()
{
	if (Stage)
		return;

	smSTAGE3D *prepared = SummitTakeHoLoginPreparedStage();
	if (!prepared)
		return;

	Stage = prepared;
	FinalizeStageReady();
}

static char *LoadingImageName[MAX_LOADING_IMAGE] =
{
	"StartImage//Loading//Loading1.bmp",
	"StartImage//Loading//Loading2.bmp",
	"StartImage//Loading//Loading3.bmp",
	"StartImage//Loading//Loading4.bmp",
	"StartImage//Loading//Loading5.bmp",
	"StartImage//Loading//Loading6.bmp",
	"StartImage//Loading//Loading7.bmp",
	"StartImage//Loading//Loading8.bmp",
};

void HoLogin::LoadLoadingImage(int level)
{
	if (level <= 10)
		GameMapImage = LoadDibSurfaceOffscreen(LoadingImageName[0]);
	else
	{
		GameMapImage = LoadDibSurfaceOffscreen(LoadingImageName[rand() % MAX_LOADING_IMAGE]);
	}
}

int	HoLogin::AddUserCharacter(char *chrFile1, char *chrFile2, int jobCode)
{
	if (UserCharacterCount < 0 || UserCharacterCount >= CHARACTER_USER_MAX)
		return FALSE;

	const int index = UserCharacterCount;
	UserCharacter[index].Init();
	UserCharacter[index].lpStage = Stage;
	if (jobCode)
		UserCharacter[index].smCharInfo.JOB_CODE = jobCode;

	// Character selection used to synchronously parse the body, motion and face
	// here. Use the same V3 model pipeline as gameplay so all user previews can
	// prepare concurrently and publish from the frame loop.
	if (!SummitRequestCharacterModelV3Interactive(&UserCharacter[index], chrFile1, chrFile2))
		return FALSE;

	const int y = Stage ?
		Stage->GetHeight(UserCharacterPos[index][0] * fONE, UserCharacterPos[index][1] * fONE) : 0;
	UserCharacter[index].SetPosi(UserCharacterPos[index][0] * fONE,
		y, UserCharacterPos[index][1] * fONE, 0, ANGLE_180, 0);
	UserCharacter[index].ActionPattern = 0;
	++UserCharacterCount;
	return TRUE;
}


// MINGW_JOB_LOAD_TRACE_V1
#ifdef __MINGW32__
static void MINGW_JobLoadTraceV1(
	const char *phase,
	int index,
	const char *body,
	const char *face,
	int result,
	smCHAR *ch
)
{
	FILE *f = SummitHoLoginRuntimeTraceFile(
		"LOGS\\mingw_job_load_trace.txt",
		"a"
	);

	if(!f)
		return;

	fprintf(
		f,
		"%s index=%d result=%d "
		"job=0x%08X char=%p "
		"body=[%s] face=[%s] "
		"motion=%p motion2=%p "
		"pattern=%p pattern2=%p "
		"dina1=%p dina2=%p\n",
		phase ? phase : "",
		index,
		result,
		ch ? (unsigned int)ch->smCharInfo.JOB_CODE : 0,
		(void *)ch,
		body ? body : "",
		face ? face : "",
		ch ? (void *)ch->smMotionInfo : NULL,
		ch ? (void *)ch->smMotionInfo2 : NULL,
		ch ? (void *)ch->Pattern : NULL,
		ch ? (void *)ch->Pattern2 : NULL,
		ch ? (void *)ch->lpDinaPattern : NULL,
		ch ? (void *)ch->lpDinaPattern2 : NULL
	);

	fflush(f);
	fclose(f);
}
#endif

int HoLogin::LoadSelectableJobCharacters()
{
	for(int index = 0; index < JobCharacterCount && index < CHARACTER_SELECTION_SLOT_CAPACITY; ++index)
		JobCharacter[index].Close();

	JobCharacterCount = 0;
	const std::vector<const gameplay::classes::PlayableClassDefinition*> classes =
		authentication::character_selection::DefaultClassSelectionAdapter().Classes(tribeNum);
	for(std::size_t index = 0; index < classes.size() && index < static_cast<std::size_t>(CHARACTER_SELECTION_SLOT_CAPACITY); ++index)
	{
		const gameplay::classes::PlayableClassDefinition* definition = classes[index];
		if(!definition || definition->creation.bodyModels.empty() || definition->creation.faces.empty())
			continue;
		JobCharacter[JobCharacterCount].smCharInfo.JOB_CODE = definition->legacyJobCode;
		AddJobCharacter(const_cast<char*>(definition->creation.bodyModels[0].c_str()),
		                const_cast<char*>(definition->creation.faces[0].model.c_str()));
	}
	return JobCharacterCount;
}

int HoLogin::ApplySelectedClassFace(int faceIndex)
{
	// SUMMIT_APPLY_FACE_TRACE_V14
	SummitCharacterSelectionTrace("HOLOGIN",
		"APPLY_FACE begin body=%d face=%d tribe=%d count=%d",
		JobBodySelectNum, faceIndex, tribeNum, JobCharacterCount);

	gameplay::classes::CharacterCreationTemplate creation;
	if(!authentication::character_selection::DefaultClassSelectionAdapter().BuildCreation(
		tribeNum, JobBodySelectNum, 0u, static_cast<std::size_t>(faceIndex), false, creation))
	{
		SummitCharacterSelectionTrace("HOLOGIN",
			"APPLY_FACE build_creation_failed body=%d face=%d tribe=%d",
			JobBodySelectNum, faceIndex, tribeNum);
		return FALSE;
	}

	SummitCharacterSelectionTrace("HOLOGIN",
		"APPLY_FACE models body=%d face=%d bodyModel='%s' faceModel='%s'",
		JobBodySelectNum, faceIndex, creation.bodyModel.c_str(), creation.faceModel.c_str());

	const int result = SetChangeJobFace(
		const_cast<char*>(creation.bodyModel.c_str()),
		const_cast<char*>(creation.faceModel.c_str()),
		JobBodySelectNum);
	SummitCharacterSelectionTrace("HOLOGIN",
		"APPLY_FACE end body=%d face=%d result=%d",
		JobBodySelectNum, faceIndex, result);
	return result;
}

int HoLogin::GetCurrentFaceCount() const
{
	return authentication::character_selection::DefaultClassSelectionAdapter().FaceCount(tribeNum, JobBodySelectNum);
}

int HoLogin::GetCurrentFacePositionX(int faceIndex) const
{
	return authentication::character_selection::ClassSelectionAdapter::FacePositionX(faceIndex, GetCurrentFaceCount());
}

void HoLogin::GetJobCharacterWorldPosition(int index, int &x, int &z) const
{
	const int count = static_cast<int>(authentication::character_selection::DefaultClassSelectionAdapter().Classes(tribeNum).size());
	authentication::character_selection::ClassSelectionAdapter::ClassWorldPosition(index, count, x, z);
}

int HoLogin::AddJobCharacter(char *chrFile1, char *chrFile2, int index)
{
	if (index == -1)
	{
		if (JobCharacterCount < 0 || JobCharacterCount >= CHARACTER_SELECTION_SLOT_CAPACITY)
			return FALSE;

		const int targetIndex = JobCharacterCount;
		JobCharacter[targetIndex].Init();
		JobCharacter[targetIndex].lpStage = Stage;
		JobCharacter[targetIndex].smCharInfo.wPlayerKilling[0] = 0;

		const int queued = SummitRequestCharacterModelV3Interactive(
			&JobCharacter[targetIndex], chrFile1, chrFile2);
#ifdef __MINGW32__
		MINGW_JobLoadTraceV1(
			"ADD NEW AFTER AddLoaderPattern V3",
			targetIndex, chrFile1, chrFile2, queued, &JobCharacter[targetIndex]);
#endif
		if (!queued)
			return FALSE;

		int classPosX = 0;
		int classPosZ = 0;
		GetJobCharacterWorldPosition(targetIndex, classPosX, classPosZ);
		const int y = Stage ? Stage->GetHeight(classPosX * fONE, classPosZ * fONE) : 0;
		JobCharacter[targetIndex].SetPosi(classPosX * fONE, y,
			classPosZ * fONE, 0, ANGLE_180, 0);
		JobCharacter[targetIndex].ActionPattern = 0;
		memset(JobCharacter[targetIndex].szChatMessage, 0, sizeof(JobCharacter[targetIndex].szChatMessage));
		memset(JobCharacter[targetIndex].smCharInfo.szName, 0, sizeof(JobCharacter[targetIndex].smCharInfo.szName));
		JobCharacter[targetIndex].smCharInfo.Stamina[0] = 100;
		JobCharacter[targetIndex].smCharInfo.Stamina[1] = 100;
		++JobCharacterCount;
		return TRUE;
	}

	if (index < 0 || index >= JobCharacterCount || index >= CHARACTER_SELECTION_SLOT_CAPACITY)
		return FALSE;

	// Face/body changes are also request based. Keep the old preview visible
	// until the replacement bundle is committed instead of clearing the model
	// and freezing the UI in SetLoadPattern(). Generation IDs already ensure a
	// stale face request cannot overwrite a newer click.
	JobCharacter[index].lpStage = Stage;
	JobCharacter[index].smCharInfo.wPlayerKilling[0] = 0;
	const int queued = SummitRequestCharacterModelV3Interactive(&JobCharacter[index], chrFile1, chrFile2);
#ifdef __MINGW32__
	MINGW_JobLoadTraceV1(
		"ADD INDEX AFTER AddLoaderPattern V3",
		index, chrFile1, chrFile2, queued, &JobCharacter[index]);
#endif
	if (!queued)
		return FALSE;

	int classPosX = 0;
	int classPosZ = 0;
	GetJobCharacterWorldPosition(index, classPosX, classPosZ);
	const int y = Stage ? Stage->GetHeight(classPosX * fONE, classPosZ * fONE) : JobCharacter[index].pY;
	JobCharacter[index].SetPosi(classPosX * fONE, y,
		classPosZ * fONE, 0, ANGLE_180, 0);
	JobCharacter[index].ActionPattern = 0;
	JobCharacter[index].smCharInfo.Stamina[0] = 100;
	JobCharacter[index].smCharInfo.Stamina[1] = 100;
	return TRUE;
}




int HoLogin::GetSelectCharacter(smCHAR *lpChar)
{
	if (lpChar == NULL)
		return FALSE;
	if (lpChar->DisplayFlag == FALSE)
		return FALSE;

	RECT rect;
	POINT	sPos;
	int sez;
	int SelZ = 65536 * 16384;


	if (lpChar)
	{
		int x, y, z;
		int anx, any;

		anx = CaAngle.x;
		any = CaAngle.y;

		any &= ANGCLIP;
		anx &= ANGCLIP;

		GetMoveLocation(0, 0, -TarDist, anx, any, 0);
		x = TarPosi.x + GeoResult_X;
		y = TarPosi.y + GeoResult_Y;
		z = TarPosi.z + GeoResult_Z;
		smRender.SetCameraPosi(x, y, z, anx, any, 0);

		sez = smRender.GetRect2D(lpChar->pX, lpChar->pY + (32 * fONE), lpChar->pZ,
			24 * fONE, 32 * fONE, &rect, &sPos);


		if (sez > 0 && sez < SelZ &&
			rect.left<pCursorPos.x && rect.right>pCursorPos.x &&
			rect.top<pCursorPos.y && rect.bottom>pCursorPos.y)
		{

			if (MouseButton[0])
				return 2;

			return 1;
		}
	}
	return FALSE;
}


// MINGW_CHAR_CREATE_TRACE_V4
#ifdef __MINGW32__
static void MINGW_CharCreateTraceV4(
	const char *step,
	int a,
	int b,
	const void *p1,
	const void *p2,
	const void *p3
)
{
	FILE *f = SummitHoLoginRuntimeTraceFile(
		"LOGS\\mingw_char_create_trace.txt",
		"a"
	);

	if(!f)
		return;

	fprintf(
		f,
		"%s a=%d b=%d p1=%p p2=%p p3=%p\n",
		step ? step : "",
		a,
		b,
		(void *)p1,
		(void *)p2,
		(void *)p3
	);

	fflush(f);
	fclose(f);
}
#endif

int HoLogin::SetChangeJobFace(char *charFile1, char *chrFile2, int jobIndex)
{
	// SUMMIT_SET_CHANGE_FACE_TRACE_V14
	SummitCharacterSelectionTrace("HOLOGIN",
		"SET_CHANGE_FACE enter job=%d count=%d body='%s' face='%s'",
		jobIndex, JobCharacterCount,
		charFile1 ? charFile1 : "", chrFile2 ? chrFile2 : "");

#ifdef __MINGW32__
	MINGW_CharCreateTraceV4(
		"SetChangeJobFace ENTER V3", jobIndex, JobCharacterCount,
		charFile1, chrFile2, NULL);
#endif

	if (jobIndex < 0 || jobIndex >= JobCharacterCount)
		return FALSE;

	const int posX = JobCharacter[jobIndex].pX;
	const int posY = JobCharacter[jobIndex].pY;
	const int posZ = JobCharacter[jobIndex].pZ;
	const int summitFaceQueueResult = AddJobCharacter(charFile1, chrFile2, jobIndex);
	SummitCharacterSelectionTrace("HOLOGIN",
		"SET_CHANGE_FACE queued job=%d result=%d char=%p flag=%d display=%d loading=%d pattern=%p",
		jobIndex, summitFaceQueueResult,
		(jobIndex >= 0 && jobIndex < JobCharacterCount) ? (void *)&JobCharacter[jobIndex] : NULL,
		(jobIndex >= 0 && jobIndex < JobCharacterCount) ? JobCharacter[jobIndex].Flag : -1,
		(jobIndex >= 0 && jobIndex < JobCharacterCount) ? JobCharacter[jobIndex].DisplayFlag : -1,
		(jobIndex >= 0 && jobIndex < JobCharacterCount) ? JobCharacter[jobIndex].PatLoading : -1,
		(jobIndex >= 0 && jobIndex < JobCharacterCount) ? (void *)JobCharacter[jobIndex].Pattern : NULL);
	if (!summitFaceQueueResult)
		return FALSE;

	// AddLoaderPattern publishes asynchronously. Preserve the current preview
	// position while the new face/model is prepared; completion swaps the bundle
	// atomically on the owner thread and prewarms its material textures.
	JobCharacter[jobIndex].SetPosi(posX, posY, posZ, 0, ANGLE_180, 0);
	return TRUE;
}

int HoLogin::Close()
{

	if (Font != NULL)
		DeleteObject(Font);

	CloseEachPlayer();

	if (Stage)
	{
		delete Stage;
		Stage = NULL;
	}
	OpeningStageTexturesReady = false;

	for (int index = 0; index < UserCharacterCount; index++)
		UserCharacter[index].Init();

	for (int index = 0; index < JobCharacterCount; index++)
		JobCharacter[index].Init();


	if (GameMapImage)
	{
		GameMapImage->Release();
		GameMapImage = NULL;
	}

	void ClearSelectCha();
	ClearSelectCha();
	return TRUE;
}

int HoLogin::CameraUpDown()
{

	static int	cameraUpDown = 0;
	if (cameraUpDown < 80)
	{
		TarPosi.y += 4;
	}

	if (cameraUpDown >= 80 && cameraUpDown < 160)
	{
		TarPosi.y -= 4;
	}

	cameraUpDown++;
	if (cameraUpDown >= 160)
		cameraUpDown = 0;

	if (SelectCurrentState == CHARACTER_SELECT_STATE)
	{
		if (NewCharacterImageBlendFlag)
		{
			NewCharacterImageBlendCount -= 1.25f;
		}
		else
		{
			NewCharacterImageBlendCount += 1.25f;
		}

		if (NewCharacterImageBlendCount >= 255.f)
		{
			NewCharacterImageBlendFlag = true;
			NewCharacterImageBlendCount = 255.f;

		}


		if (NewCharacterImageBlendCount < 100.f)
		{
			NewCharacterImageBlendFlag = false;
			NewCharacterImageBlendCount = 100.f;
		}

	}

	return TRUE;
}

int HoLogin::Main()
{
	// SUMMIT_HOLOGIN_STATE_TRACE_V14
	{
		static int s_lastCur = -10000;
		static int s_lastNext = -10000;
		static int s_lastSub = -10000;
		static int s_lastErr = -10000;
		static int s_lastBody = -10000;
		static int s_lastFace = -10000;
		static int s_lastFaceFirst = -10000;
		static int s_lastFlag = -10000;
		static int s_lastDisplay = -10000;
		static int s_lastLoading = -10000;
		static smPAT3D *s_lastPattern = reinterpret_cast<smPAT3D *>(~static_cast<std::uintptr_t>(0));
		static DWORD s_lastSerial = 0xFFFFFFFFu;

		smCHAR *traceSelected =
			(JobBodySelectNum >= 0 && JobBodySelectNum < JobCharacterCount) ?
			&JobCharacter[JobBodySelectNum] : NULL;
		const int traceFlag = traceSelected ? traceSelected->Flag : -1;
		const int traceDisplay = traceSelected ? traceSelected->DisplayFlag : -1;
		const int traceLoading = traceSelected ? traceSelected->PatLoading : -1;
		smPAT3D *tracePattern = traceSelected ? traceSelected->Pattern : NULL;
		const DWORD traceSerial = traceSelected ? traceSelected->dwObjectSerial : 0;

		if(s_lastCur != SelectCurrentState || s_lastNext != SelectNextState ||
			s_lastSub != SelectSubState || s_lastErr != ErrorSelect ||
			s_lastBody != JobBodySelectNum || s_lastFace != JobFaceSelectNum ||
			s_lastFaceFirst != FaceSelectFirstFlag || s_lastFlag != traceFlag ||
			s_lastDisplay != traceDisplay || s_lastLoading != traceLoading ||
			s_lastPattern != tracePattern || s_lastSerial != traceSerial)
		{
			SummitCharacterSelectionTrace("HOLOGIN",
				"STATE cur=%d next=%d sub=%d err=%d body=%d/%d face=%d faceFirst=%d selected=%p flag=%d display=%d loading=%d pattern=%p serial=%lu master=%p eachFlag=%d eachPattern=%p",
				SelectCurrentState, SelectNextState, SelectSubState, ErrorSelect,
				JobBodySelectNum, JobCharacterCount, JobFaceSelectNum, FaceSelectFirstFlag,
				(void *)traceSelected, traceFlag, traceDisplay, traceLoading,
				(void *)tracePattern, static_cast<unsigned long>(traceSerial),
				(void *)chrEachMaster, chrEachPlayer.Flag, (void *)chrEachPlayer.Pattern);
			s_lastCur = SelectCurrentState;
			s_lastNext = SelectNextState;
			s_lastSub = SelectSubState;
			s_lastErr = ErrorSelect;
			s_lastBody = JobBodySelectNum;
			s_lastFace = JobFaceSelectNum;
			s_lastFaceFirst = FaceSelectFirstFlag;
			s_lastFlag = traceFlag;
			s_lastDisplay = traceDisplay;
			s_lastLoading = traceLoading;
			s_lastPattern = tracePattern;
			s_lastSerial = traceSerial;
		}
	}

	PumpPreloadStage();
    if (Stage && Stage->smMaterialGroup)
    {
        // Keep scene residency moving without a frame-sized decode burst.
        AssetLoadV2PrewarmMaterialGroupStep(Stage->smMaterialGroup,
            SMASSET_OWNER_UI, 1, 2u, 1);
    }
	if (ErrorSelect != ERROR_NONE)
	{
		if (StartErrorDelayFlag == FALSE)
		{
			StartErrorDelayFlag = TRUE;
			ErrorDelayCount = 0;
		}
		else
		{
			ErrorDelayCount++;
		}

		if (ErrorDelayCount > 100)
		{
			if (ErrorSelect == ERROR_NONAME_NEWPLAYER)
			{
				hFocusWnd = hTextWnd;
				FocusWndFlag = TRUE;
#ifdef	_LANGUAGE_JAPANESE		
				SetIME_Mode(1);
#endif
			}
			ErrorSelect = ERROR_NONE;
			MouseButton[0] = FALSE;
			MouseDblClick = FALSE;

			StartErrorDelayFlag = FALSE;
		}
	}



	void ktj_imsi_clanMENU_Main();
	ktj_imsi_clanMENU_Main();




	int  main_chaC();

	if (main_chaC() && ClanMarkUpdateSuccessFlag == FALSE)
	{
		for (int m = 0; m < CHARACTER_USER_MAX; m++)
		{
			if (strlen(chacldata[m].ChaName) >= 1)
			{
				for (int n = 0; n < DownUserCharacterCount; n++)
				{
					if (stricmp(chacldata[m].ChaName, DownUserCharacterInfo[n].Name) == 0 &&
						DownUserCharacterInfo[n].ClanFlag == FALSE)
					{
						strcpy(DownUserCharacterInfo[n].ClanName, chacldata[m].name);
						DownUserCharacterInfo[n].ClanFlag = TRUE;
						if (chacldata[m].hClanMark16)
							DownUserCharacterInfo[n].ClanMark = chacldata[m].hClanMark16;
					}
				}
			}
		}
		ClanMarkUpdateSuccessFlag = TRUE;
	}

	MainEffect();

	CameraUpDown();

	if (SelectCurrentState == START_GAME_STATE && FadeCurrentState == FADE_IN)
	{
		switch (SelectSubState)
		{
		case SUB_SELECT_CHARACTER_USER:
		{
			if (MouseDownCharacterNum != -1)
			{
				memcpy(CharacterName1, DownUserCharacterInfo[MouseDownCharacterNum].BodyName, sizeof(CharacterName1));
				memcpy(CharacterName2, DownUserCharacterInfo[MouseDownCharacterNum].FaceName, sizeof(CharacterName2));
				memset(PlayerName, 0, sizeof(PlayerName));
				memcpy(PlayerName, DownUserCharacterInfo[MouseDownCharacterNum].Name, sizeof(PlayerName));
				CharacterJob = DownUserCharacterInfo[MouseDownCharacterNum].Job;
				GameStartField = DownUserCharacterInfo[MouseDownCharacterNum].StartField;

				CharacterPosX = DownUserCharacterInfo[MouseDownCharacterNum].PosX;
				CharacterPosZ = DownUserCharacterInfo[MouseDownCharacterNum].PosZ;
			}

			SelectSubState = SUB_SELECT_NONE;
			return 3;
			break;
		}

		case SUB_SELECT_CHARACTER_NEW_SERVER:
		{
			SelectSubState = SUB_SELECT_NONE;
			memset(CharacterName1, 0, sizeof(CharacterName1));
			memset(CharacterName2, 0, sizeof(CharacterName2));


			gameplay::classes::CharacterCreationTemplate creation;
				if(JobFaceSelectNum < 0 ||
				   !authentication::character_selection::DefaultClassSelectionAdapter().BuildCreation(
				       tribeNum, JobBodySelectNum, 0u, static_cast<std::size_t>(JobFaceSelectNum), false, creation))
				{
					ErrorSelect = ERROR_SELECT_JOB;
					return FALSE;
				}

				strcpy_s(CharacterName1, creation.bodyModel.c_str());
				strcpy_s(CharacterName2, creation.faceModel.c_str());
				NewCharInfo.JobCode = creation.legacyJobCode;
				NewCharInfo.Strength = creation.attributes.strength;
				NewCharInfo.Spirit = creation.attributes.spirit;
				NewCharInfo.Talent = creation.attributes.talent;
				NewCharInfo.Dexterity = creation.attributes.dexterity;
				NewCharInfo.Health = creation.attributes.health;
			return 3;
			break;
		}

		}
	}

	if (SelectCurrentState != COMPLETE_SELECT_STATE && SelectCurrentState != START_GAME_STATE)
	{

		for (int index = 0; index < UserCharacterCount; index++)
		{
			if (UserCharacter[index].DisplayFlag)
				UserCharacter[index].Main();
		}

		for (int index = 0; index < JobCharacterCount; index++)
		{
			if (JobCharacter[index].DisplayFlag)
				JobCharacter[index].Main();
		}
	}


	if (SelectCurrentState == SelectNextState)
	{
		if (SelectCurrentState == RACE_SELECT_STATE)
		{

			if (RaceMarkBlendCount < 200)
				RaceMarkBlendCount += 4;
		}
		else if (SelectCurrentState == JOB_SELECT_STATE)
		{


			if (CharacterReturnFlag && Stage)
			{
				CharacterStopFlag = FALSE;
				CharacterReturnFlag = FALSE;
				for (int index = 0; index < JobCharacterCount; index++)
				{
					if (!JobCharacter[index].DisplayFlag)
						continue;
					int classPosX = 0;
			int classPosZ = 0;
			GetJobCharacterWorldPosition(index, classPosX, classPosZ);
			JobCharacter[index].SetPosi(classPosX * fONE,
				Stage->GetHeight(classPosX * fONE, classPosZ * fONE),
				classPosZ * fONE, 0, ANGLE_180, 0);
					JobCharacter[index].ChangeMotion(10);
				}
			}

		}
		else if (SelectCurrentState == FACE_SELECT_STATE)
		{
			// SUMMIT_FACE_PREVIEW_READY_REBIND_V13A
			// Face models arrive asynchronously. OpenEachPlayer rejects a character
			// until DisplayFlag is ready, so retry/rebind automatically every frame
			// instead of requiring the user to click another face.
			smCHAR *facePreview =
				(JobBodySelectNum >= 0 && JobBodySelectNum < JobCharacterCount) ?
				&JobCharacter[JobBodySelectNum] : NULL;

			if (facePreview && facePreview->Flag && facePreview->DisplayFlag)
			{
				if (chrEachMaster != facePreview || !chrEachPlayer.Flag ||
					chrEachPlayer.Pattern != facePreview->Pattern)
				{
					OpenEachPlayer(facePreview);
				}

				if (chrEachMaster != NULL)
					chrEachMaster->Flag = 1;
			}

			EachPlayerMain();
		}
		else if (SelectCurrentState == COMPLETE_SELECT_STATE)
		{
#ifdef __MINGW32__
			// MINGW_COMPLETE_STATE_TRACE_V1
			{
				static int s_completeTraceCount = 0;
				if (s_completeTraceCount < 120)
				{
					FILE *fp = SummitHoLoginRuntimeTraceFile("LOGS\\mingw_complete_state.txt", "a");
					if (fp)
					{
						fprintf(fp,
							"COMPLETE cur=%d next=%d fadeCur=%d fadeNext=%d blend=%d mapBlend=%d mouse0=%d dbl=%d sub=%d\\n",
							SelectCurrentState,
							SelectNextState,
							FadeCurrentState,
							FadeNextState,
							BlendValue,
							MapBlendCount,
							MouseButton[0],
							MouseDblClick,
							SelectSubState);
						fclose(fp);
					}
					s_completeTraceCount++;
				}
			}
#endif

			if (FadeCurrentState == FADE_OUT)
			{
				SelectNextState = START_GAME_STATE;
				SetFade();
			}
			if (MouseButton[0] || MouseDblClick || MapBlendCount >= 255)
				SelectNextState = START_GAME_STATE;
		}
	}


	if (FadeCurrentState == FADE_OUT && FadeNextState == FADE_OUT && SelectNextState == RETURN_OPENING_STATE)
	{
		if (smWsockDataServer != NULL)
		{
			DisconnectServerFull();

		}
		InitUserCharacter();
		return 2;
	}


	if (SelectCurrentState != SelectNextState)
	{
		if (SelectCurrentState != COMPLETE_SELECT_STATE && SelectNextState != COMPLETE_SELECT_STATE &&
			SelectCurrentState != START_GAME_STATE && SelectNextState != START_GAME_STATE)
		{

			if ((SelectCurrentState == RACE_SELECT_STATE) && (SelectNextState == JOB_SELECT_STATE))
				RaceMarkScaleSize--;

			MouseDownCharacterNum = -1;
			MouseMoveCharacterNum = -1;
			RaceMarkBlendCount = 0;


			static int step = 1000;
			if (SelectNextState >= 0 && SelectNextState < COMPLETE_SELECT_STATE &&
				SelectCurrentState >= 0 && SelectCurrentState < COMPLETE_SELECT_STATE)
			{
				if (SelectNextState > SelectCurrentState)
				{

					TarDist += step;
					if (TarDist > CameraZPos[SelectNextState] + 800)
					{
						TarDist = CameraZPos[SelectNextState];
						SelectCurrentState = SelectNextState;
						step = 1000;
					}

					if ((CameraZPos[SelectNextState] - CameraZPos[SelectCurrentState]) / 16 * 15 + CameraZPos[SelectCurrentState] < TarDist)
						step = 100;
				}
				else if (SelectNextState < SelectCurrentState)
				{
					TarDist -= step;
					if (TarDist < CameraZPos[SelectNextState] - 800)
					{
						TarDist = CameraZPos[SelectNextState];
						SelectCurrentState = SelectNextState;
						step = 1000;
					}

					if ((CameraZPos[SelectCurrentState] - CameraZPos[SelectNextState]) / 16 + CameraZPos[SelectNextState] > TarDist)
						step = 100;
				}
			}
		}

		if (SelectNextState == COMPLETE_SELECT_STATE)
		{
			if (FadeCurrentState == FadeNextState)
			{
				SelectCurrentState = COMPLETE_SELECT_STATE;
			}
		}

		if (SelectNextState == START_GAME_STATE)
		{
			SelectCurrentState = START_GAME_STATE;
		}

		if (SelectCurrentState == FACE_SELECT_STATE && SelectNextState == JOB_SELECT_STATE)
		{
			JobBodySelectNum = -1;
			JobFaceSelectNum = -1;
			memset(UserCharacterNewName, 0, sizeof(UserCharacterNewName));
			SetWindowText(hTextWnd, 0);
			hFocusWnd = NULL;
		}


		if (SelectCurrentState == JOB_SELECT_STATE && SelectNextState == FACE_SELECT_STATE)
		{
			if (JobFaceSelectNum == -1)
			{
				JobFaceSelectNum = 1;
				hFocusWnd = hTextWnd;
				FocusWndFlag = TRUE;
#ifdef	_LANGUAGE_JAPANESE		
				SetIME_Mode(1);
#endif

				FaceSelectFirstFlag = FALSE;
			}
		}
	}


	if (SelectNextState == FACE_SELECT_STATE)
	{

		if (JobCharacter[JobBodySelectNum].pZ < -415 * fONE && CharacterStopFlag == FALSE)
		{
			JobCharacter[JobBodySelectNum].MoveFlag = FALSE;
			JobCharacter[JobBodySelectNum].Angle.x = 0;
			JobCharacter[JobBodySelectNum].Angle.y = ANGLE_180;
			JobCharacter[JobBodySelectNum].Angle.z = 0;
			JobCharacter[JobBodySelectNum].ActionPattern = 0;
			JobCharacter[JobBodySelectNum].pZ = -429 * fONE;

			CharacterStopFlag = TRUE;
		}
	}

	return TRUE;

}

int HoLogin::MainDefaultButton()
{
	if (SelectSubState != SUB_SELECT_NONE || ErrorSelect != ERROR_NONE)
		return FALSE;

	if (SelectCurrentState == SelectNextState && SelectCurrentState == CHARACTER_SELECT_STATE)
	{
		if (MouseDownCharacterNum != -1)
		{

			if (DeleteButton.Main(pCursorPos.x, pCursorPos.y, MouseButton[0]))
			{
				SelectSubState = SUB_SELECT_CHARACTER_MANAGE;
				MessageBox.Flag = TRUE;
				PlayWav2(4 - 1);


			}
		}

		int userCount = 0;
		for (int index = 0; index < DownUserCharacterCount; index++)
		{
			if (UserCharacter[index].DisplayFlag)
				userCount++;
		}

		// Character creation depends on free account slots, not on the account
		// being completely empty.  The old userCount == 0 guard made the
		// button stop receiving input as soon as one character existed.
				// SUMMIT_NO_LOWER_NEW_CHARACTER_INPUT_V13A
		// Bottom New Character action intentionally removed.

	}


	if (OkButton.Main(pCursorPos.x, pCursorPos.y, MouseButton[0]))
	{
		PlayWav(4 - 1);

		if (SelectCurrentState == CHARACTER_SELECT_STATE)
		{
			if (MouseDownCharacterNum != -1)
			{
				MessageBox.Flag = TRUE;
				SelectSubState = SUB_SELECT_CHARACTER_USER;
			}

			if (MouseDownCharacterNum == -1)
				ErrorSelect = ERROR_SELECT_CHAR;
		}
		else if (SelectCurrentState == RACE_SELECT_STATE)
		{
			ErrorSelect = ERROR_SELECT_RACE;
		}
		else if (SelectCurrentState == JOB_SELECT_STATE)
		{
			if (MouseDownCharacterNum != -1)
			{
				SelectNextState = FACE_SELECT_STATE;
				JobCharacter[JobBodySelectNum].SetTargetPosi(0, (-420 * fONE) >> FLOATNS);
				JobCharacter[JobBodySelectNum].MoveMode = TRUE;

				JobCharacter[JobBodySelectNum].smCharInfo.Weight[0] = 0;
				JobCharacter[JobBodySelectNum].smCharInfo.Weight[1] = 1000;
				JobCharacter[JobBodySelectNum].smCharInfo.Stamina[0] = 1000;

				JobCharacter[JobBodySelectNum].smCharInfo.ArrowPosi[0] = 0;
				JobCharacter[JobBodySelectNum].smCharInfo.ArrowPosi[1] = 0;

				JobCharacter[JobBodySelectNum].Flag = 1;
				#ifdef __MINGW32__
				MINGW_CharCreateTraceV4(
					"OpenEach #1 BEFORE",
					JobBodySelectNum,
					JobCharacterCount,
					(JobBodySelectNum >= 0 && JobBodySelectNum < JobCharacterCount) ? &JobCharacter[JobBodySelectNum] : NULL,
					(JobBodySelectNum >= 0 && JobBodySelectNum < JobCharacterCount) ? JobCharacter[JobBodySelectNum].lpDinaPattern2 : NULL,
					(JobBodySelectNum >= 0 && JobBodySelectNum < JobCharacterCount && JobCharacter[JobBodySelectNum].lpDinaPattern2) ? JobCharacter[JobBodySelectNum].lpDinaPattern2->Pat : NULL
				);
				#endif
				OpenEachPlayer(&JobCharacter[JobBodySelectNum]);
				#ifdef __MINGW32__
				MINGW_CharCreateTraceV4(
					"OpenEach #1 AFTER",
					JobBodySelectNum,
					JobCharacterCount,
					(JobBodySelectNum >= 0 && JobBodySelectNum < JobCharacterCount) ? &JobCharacter[JobBodySelectNum] : NULL,
					(JobBodySelectNum >= 0 && JobBodySelectNum < JobCharacterCount) ? JobCharacter[JobBodySelectNum].lpDinaPattern2 : NULL,
					NULL
				);
				#endif

				MouseDownCharacterNum = -1;
				CharacterReturnFlag = TRUE;
			}

			if (JobBodySelectNum == -1)
				ErrorSelect = ERROR_SELECT_JOB;

		}
		else if (SelectCurrentState == FACE_SELECT_STATE)
		{

			if (JobBodySelectNum != -1 && JobFaceSelectNum != -1)
			{
				hFocusWnd = NULL;
				if (strlen(UserCharacterNewName) > 0)
				{
					MessageBox.Flag = TRUE;
					SelectSubState = SUB_SELECT_CHARACTER_NEW;
				}
				else
					ErrorSelect = ERROR_NONAME_NEWPLAYER;
			}

			if (JobFaceSelectNum == -1)
			{
				hFocusWnd = NULL;
				ErrorSelect = ERROR_SELECT_FACE;
			}
		}

	}


	if (QuitButton.Main(pCursorPos.x, pCursorPos.y, MouseButton[0]))
	{

		// MINGW_SELECT_QUIT_TRACE_V1
		{
			FILE *mingwQuitTrace = SummitHoLoginRuntimeTraceFile(
				"LOGS\\mingw_select_quit_trace.txt",
				"a"
			);

			if(mingwQuitTrace)
			{
				fprintf(
					mingwQuitTrace,
					"QUIT HIT x=%ld y=%ld MouseButton0=%d Current=%d Next=%d SubBefore=%d\n",
					pCursorPos.x,
					pCursorPos.y,
					MouseButton[0],
					SelectCurrentState,
					SelectNextState,
					SelectSubState
				);

				fclose(mingwQuitTrace);
			}
		}

		PlayWav(4 - 1);
		SelectSubState = SUB_SELECT_BACK;
		{
			FILE *mingwQuitTrace = SummitHoLoginRuntimeTraceFile(
				"LOGS\\mingw_select_quit_trace.txt",
				"a"
			);

			if(mingwQuitTrace)
			{
				fprintf(
					mingwQuitTrace,
					"QUIT SET SUB_SELECT_BACK SubAfter=%d\n",
					SelectSubState
				);

				fclose(mingwQuitTrace);
			}
		}

	}

	if (BackButton.Main(pCursorPos.x, pCursorPos.y, MouseButton[0]) && SelectCurrentState != CHARACTER_SELECT_STATE)
	{
		PlayWav(7 - 1);
		SelectNextState = SelectCurrentState - 1;
	}

	return TRUE;
}

int HoLogin::InitUserCharacter()
{
	for (int index = 0; index < CHARACTER_USER_MAX; index++)
		UserCharacter[index].Init();
	UserCharacterCount = 0;

	return TRUE;
}

int HoLogin::DrawJobCharacter(int x, int y, int z, int ax, int ay, int az)
{
	if (!Stage)
		return TRUE;
	smCHAR_SetCameraPosi(x, y, z, ax, ay, az);
	int index;
	BOOL mouseDownFlag = FALSE;
	for (index = 0; index < JobCharacterCount; index++)
	{
		if (!JobCharacter[index].DisplayFlag)
			continue;

		if (SelectCurrentState == JOB_SELECT_STATE
			&& ErrorSelect == ERROR_NONE
			&& SelectCurrentState == SelectNextState)
		{
			smRender.Color_R = 0;
			smRender.Color_G = 0;
			smRender.Color_B = 0;


			if (GetSelectCharacter(&JobCharacter[index]) == 1)
			{
				MouseMoveCharacterNum = index;
				smRender.Color_R = 50;
				smRender.Color_G = 50;
				smRender.Color_B = 50;

			}
			else if (GetSelectCharacter(&JobCharacter[index]) == 2)
			{
				PlayWav(8 - 1);
				MouseDownCharacterNum = index;
				mouseDownFlag = TRUE;
			}

			if (mouseDownFlag == FALSE && MouseButton[0] && pCursorPos.x > 0 && pCursorPos.x < SETTINGSHANDLER->getWidth() && pCursorPos.y > 70 && pCursorPos.y < (SETTINGSHANDLER->getHeight() - 50))
				MouseDownCharacterNum = -1;

			if (MouseDownCharacterNum == index)
			{
				JobBodySelectNum = index;
				#ifdef __MINGW32__
				MINGW_CharCreateTraceV4(
					"CLASS INDEX ASSIGNED",
					index,
					JobCharacterCount,
					&JobCharacter[index],
					JobCharacter[index].lpDinaPattern2,
					NULL
				);
				#endif

				smRender.Color_R = 120;
				smRender.Color_G = 120;
				smRender.Color_B = 120;



				if (MouseDblClick)
				{

				if(
					JobBodySelectNum < 0 ||
					JobBodySelectNum >= JobCharacterCount
				)
				{
#ifdef __MINGW32__
					MINGW_CharCreateTraceV4(
						"DBLCLICK INVALID INDEX",
						JobBodySelectNum,
						JobCharacterCount,
						NULL,
						NULL,
						NULL
					);
#endif
					MouseDblClick = FALSE;
					MouseButton[0] = FALSE;
					return TRUE;
				}

				#ifdef __MINGW32__
				MINGW_CharCreateTraceV4(
					"DBLCLICK ENTER",
					index,
					JobBodySelectNum,
					&JobCharacter[index],
					JobCharacter[index].lpDinaPattern2,
					NULL
				);
				#endif

					SelectNextState = FACE_SELECT_STATE;

					JobCharacter[JobBodySelectNum].smCharInfo.Stamina[0] = 100;
					#ifdef __MINGW32__
					MINGW_CharCreateTraceV4(
						"DBLCLICK FIELD 01",
						JobBodySelectNum,
						JobCharacterCount,
						&JobCharacter[JobBodySelectNum],
						JobCharacter[JobBodySelectNum].lpDinaPattern2,
						NULL
					);
					#endif
					JobCharacter[JobBodySelectNum].smCharInfo.Stamina[1] = 100;
					#ifdef __MINGW32__
					MINGW_CharCreateTraceV4(
						"DBLCLICK FIELD 02",
						JobBodySelectNum,
						JobCharacterCount,
						&JobCharacter[JobBodySelectNum],
						JobCharacter[JobBodySelectNum].lpDinaPattern2,
						NULL
					);
					#endif

					JobCharacter[JobBodySelectNum].SetTargetPosi(0, (-420 * fONE) >> FLOATNS);
					#ifdef __MINGW32__
					MINGW_CharCreateTraceV4(
						"DBLCLICK FIELD 03",
						JobBodySelectNum,
						JobCharacterCount,
						&JobCharacter[JobBodySelectNum],
						JobCharacter[JobBodySelectNum].lpDinaPattern2,
						NULL
					);
					#endif
					JobCharacter[JobBodySelectNum].MoveMode = TRUE;
					#ifdef __MINGW32__
					MINGW_CharCreateTraceV4(
						"DBLCLICK FIELD 04",
						JobBodySelectNum,
						JobCharacterCount,
						&JobCharacter[JobBodySelectNum],
						JobCharacter[JobBodySelectNum].lpDinaPattern2,
						NULL
					);
					#endif
					CharacterReturnFlag = TRUE;

					JobCharacter[JobBodySelectNum].smCharInfo.Weight[0] = 0;
					#ifdef __MINGW32__
					MINGW_CharCreateTraceV4(
						"DBLCLICK FIELD 05",
						JobBodySelectNum,
						JobCharacterCount,
						&JobCharacter[JobBodySelectNum],
						JobCharacter[JobBodySelectNum].lpDinaPattern2,
						NULL
					);
					#endif
					JobCharacter[JobBodySelectNum].smCharInfo.Weight[1] = 1000;
					#ifdef __MINGW32__
					MINGW_CharCreateTraceV4(
						"DBLCLICK FIELD 06",
						JobBodySelectNum,
						JobCharacterCount,
						&JobCharacter[JobBodySelectNum],
						JobCharacter[JobBodySelectNum].lpDinaPattern2,
						NULL
					);
					#endif
					JobCharacter[JobBodySelectNum].smCharInfo.Stamina[0] = 1000;
					#ifdef __MINGW32__
					MINGW_CharCreateTraceV4(
						"DBLCLICK FIELD 07",
						JobBodySelectNum,
						JobCharacterCount,
						&JobCharacter[JobBodySelectNum],
						JobCharacter[JobBodySelectNum].lpDinaPattern2,
						NULL
					);
					#endif
					JobCharacter[JobBodySelectNum].smCharInfo.ArrowPosi[0] = 0;
					#ifdef __MINGW32__
					MINGW_CharCreateTraceV4(
						"DBLCLICK FIELD 08",
						JobBodySelectNum,
						JobCharacterCount,
						&JobCharacter[JobBodySelectNum],
						JobCharacter[JobBodySelectNum].lpDinaPattern2,
						NULL
					);
					#endif
					JobCharacter[JobBodySelectNum].smCharInfo.ArrowPosi[1] = 0;
					#ifdef __MINGW32__
					MINGW_CharCreateTraceV4(
						"DBLCLICK FIELD 09",
						JobBodySelectNum,
						JobCharacterCount,
						&JobCharacter[JobBodySelectNum],
						JobCharacter[JobBodySelectNum].lpDinaPattern2,
						NULL
					);
					#endif

					JobCharacter[JobBodySelectNum].Flag = 1;
					#ifdef __MINGW32__
					MINGW_CharCreateTraceV4(
						"DBLCLICK FIELD 10",
						JobBodySelectNum,
						JobCharacterCount,
						&JobCharacter[JobBodySelectNum],
						JobCharacter[JobBodySelectNum].lpDinaPattern2,
						NULL
					);
					#endif

					#ifdef __MINGW32__
					MINGW_CharCreateTraceV4(
						"OpenEach #2 BEFORE",
						JobBodySelectNum,
						JobCharacterCount,
						(JobBodySelectNum >= 0 && JobBodySelectNum < JobCharacterCount) ? &JobCharacter[JobBodySelectNum] : NULL,
						(JobBodySelectNum >= 0 && JobBodySelectNum < JobCharacterCount) ? JobCharacter[JobBodySelectNum].lpDinaPattern2 : NULL,
						(JobBodySelectNum >= 0 && JobBodySelectNum < JobCharacterCount && JobCharacter[JobBodySelectNum].lpDinaPattern2) ? JobCharacter[JobBodySelectNum].lpDinaPattern2->Pat : NULL
					);
					#endif
					OpenEachPlayer(&JobCharacter[JobBodySelectNum]);
					#ifdef __MINGW32__
					MINGW_CharCreateTraceV4(
						"OpenEach #2 AFTER",
						JobBodySelectNum,
						JobCharacterCount,
						(JobBodySelectNum >= 0 && JobBodySelectNum < JobCharacterCount) ? &JobCharacter[JobBodySelectNum] : NULL,
						(JobBodySelectNum >= 0 && JobBodySelectNum < JobCharacterCount) ? JobCharacter[JobBodySelectNum].lpDinaPattern2 : NULL,
						NULL
					);
					#endif

					MouseDblClick = FALSE;
					MouseButton[0] = FALSE;
				}
			}
		}
		JobCharacter[index].Draw();
		smRender.Color_R = 0;
		smRender.Color_G = 0;
		smRender.Color_B = 0;
	}

	return TRUE;
}

int HoLogin::DrawUserCharacter(int x, int y, int z, int ax, int ay, int az)
{
	if (!Stage)
		return TRUE;
	smCHAR_SetCameraPosi(x, y, z, ax, ay, az);

	BOOL mouseDownFlag = FALSE;
	MouseMoveCharacterNum = -1;

	for (int index = 0; index < CHARACTER_USER_MAX; index++)
	{
		if (SelectCurrentState == CHARACTER_SELECT_STATE &&
			ErrorSelect == ERROR_NONE)
		{
			smRender.Color_R = 0;
			smRender.Color_G = 0;
			smRender.Color_B = 0;

			if (SelectSubState == SUB_SELECT_NONE)
			{
				if (GetSelectCharacter(&UserCharacter[index]) == 1 &&
					SelectNextState == CHARACTER_SELECT_STATE)
				{
					MouseMoveCharacterNum = index;
					smRender.Color_R = 50;
					smRender.Color_G = 50;
					smRender.Color_B = 50;
				}
				else if (GetSelectCharacter(&UserCharacter[index]) == 2 &&
					SelectNextState == CHARACTER_SELECT_STATE)
				{
					PlayWav(8 - 1);
					MouseMoveCharacterNum = index;
					mouseDownFlag = TRUE;
					MouseDownCharacterNum = index;
				}

				if (mouseDownFlag == FALSE && MouseButton[0] &&
					pCursorPos.x > 0 && pCursorPos.x < SETTINGSHANDLER->getWidth() &&
					pCursorPos.y > 60 && pCursorPos.y < (SETTINGSHANDLER->getHeight() - 60))
				{
					MouseDownCharacterNum = -1;
				}
			}


			if (MouseDownCharacterNum == index)
			{
				smRender.Color_R = 120;
				smRender.Color_G = 120;
				smRender.Color_B = 120;
			}

		}


		if (UserCharacter[index].DisplayFlag)
			UserCharacter[index].Draw();

		smRender.Color_R = 0;
		smRender.Color_G = 0;
		smRender.Color_B = 0;
	}

	if (SelectCurrentState == CHARACTER_SELECT_STATE)
	{
		if (MouseDownCharacterNum != -1 && MouseDblClick && pCursorPos.y < SETTINGSHANDLER->getHeight() - 50)
		{
			MessageBox.Flag = TRUE;
			SelectSubState = SUB_SELECT_CHARACTER_USER;
		}
	}
	return TRUE;
}

int HoLogin::DrawTitle(int state)
{

	dsDrawTexImage(TitleBoxFrameImage, SETTINGSHANDLER->getWidth() / 2 - 256 / 2, 0, 256, 128, 255);
	if (state == CHARACTER_SELECT_STATE)
		HoDrawTexImage(TitleNameImage, float(SETTINGSHANDLER->getWidth() / 2 - 64), 53.f, 128.f, 30.f, 0.f, 0.f, 128.f, 30.f, 255);
	else if (state == RACE_SELECT_STATE)
		HoDrawTexImage(TitleNameImage, float(SETTINGSHANDLER->getWidth() / 2 - 64), 53.f, 128.f, 30.f, 0.f, 30.f, 128.f, 30.f, 255);
	else if (state == JOB_SELECT_STATE)
		HoDrawTexImage(TitleNameImage, float(SETTINGSHANDLER->getWidth() / 2 - 64), 53.f, 128.f, 30.f, 0.f, 60.f, 128.f, 30.f, 255);
	else if (state == FACE_SELECT_STATE)
		HoDrawTexImage(TitleNameImage, float(SETTINGSHANDLER->getWidth() / 2 - 64), 53.f, 128.f, 30.f, 0.f, 90.f, 128.f, 30.f, 255);

	return TRUE;
}

int HoLogin::DrawGameFrame()
{

	if (SelectNextState != START_GAME_STATE)
	{
		dsDrawTexImage(BlackBoxImage, 0, 0, SETTINGSHANDLER->getWidth(), 70, 150);
		dsDrawTexImage(BlackBoxImage, 0, SETTINGSHANDLER->getHeight() - 50, SETTINGSHANDLER->getWidth(), 70, 150);

		int frameViewportWidth = SETTINGSHANDLER->getWidth();
		int frameViewportHeight = SETTINGSHANDLER->getHeight() - 140;

		int lineWidthCount = frameViewportWidth / 64;
		int lineWidthRemain = frameViewportWidth % 64;
		int lineHeightCount = frameViewportHeight / 64;
		int lineHeightRemain = frameViewportHeight % 64;


		int index = 0;
		for (index = 0; index < lineWidthCount; index++)
		{
			dsDrawTexImage(MainBoxFrameImage1[1], 4 + 64 * index, 68, 64, 8, 255);
			dsDrawTexImage(MainBoxFrameImage1[3], 4 + 64 * index, SETTINGSHANDLER->getHeight() - 54, 64, 8, 255);
		}

		HoDrawTexImage(MainBoxFrameImage1[1], float(4 + (index * 64)), 68.f, (float)lineWidthRemain, 8.f,
			0.f, 0.f, (float)lineWidthRemain, 8.f, 255);
		HoDrawTexImage(MainBoxFrameImage1[3], float(4 + (index * 64)), float(SETTINGSHANDLER->getHeight() - 74), (float)lineWidthRemain, 8.f,
			0.f, 0.f, (float)lineWidthRemain, 8.f, 255);


		for (index = 0; index < lineHeightCount; index++)
		{
			dsDrawTexImage(MainBoxFrameImage1[0], 0, 70 + 64 * index, 8, 64, 255);
			dsDrawTexImage(MainBoxFrameImage1[2], SETTINGSHANDLER->getWidth() - 8, 70 + 64 * index, 8, 64, 255);
		}
		if (lineHeightRemain > 0)
		{
			HoDrawTexImage(MainBoxFrameImage1[0], 0.f, float(70 + (index * 64)), 8.f, (float)lineHeightRemain,
				0.f, 0.f, 8.f, (float)lineHeightRemain, 255);
			HoDrawTexImage(MainBoxFrameImage1[2], float(SETTINGSHANDLER->getWidth() - 8), float(70 + (index * 64)), 8.f, (float)lineHeightRemain,
				0.f, 0.f, 8.f, (float)lineHeightRemain, 255);
		}

		dsDrawTexImage(MainBoxFrameImage2[0], 0, 68, 130, 64, 255);
		dsDrawTexImage(MainBoxFrameImage2[1], SETTINGSHANDLER->getWidth() - 130, 68, 130, 64, 255);
		dsDrawTexImage(MainBoxFrameImage2[2], 0, SETTINGSHANDLER->getHeight() - (64 + 48), 130, 64, 255);
		dsDrawTexImage(MainBoxFrameImage2[3], SETTINGSHANDLER->getWidth() - 130, SETTINGSHANDLER->getHeight() - (64 + 48), 130, 64, 255);
	}

	return TRUE;
}

int HoLogin::DrawLight()
{

	static BOOL flag = TRUE;
	static int tt = 0;
	smRender.ClearLight();

	if (FadeCurrentState == FadeNextState && SelectCurrentState != COMPLETE_SELECT_STATE)
	{
		smRender.AddDynamicLight(-200 * fONE + tt*fONE, 0, -300 * fONE, 80, 30, 0, 255, 40000 + tt*fONE);
		smRender.AddDynamicLight(200 * fONE - tt*fONE, 0, -300 * fONE, 80, 30, 0, 255, 40000 + tt*fONE);

		if (SelectCurrentState == FACE_SELECT_STATE)
			smRender.AddDynamicLight(0, 0, -430 * fONE, 110, 40, 0, 255, 10000 + tt*fONE);

		if (tt == 40)
			flag = TRUE;
		else if (tt == 0)
			flag = FALSE;
		if (flag)
			tt--;
		else tt++;
	}

	return TRUE;
}

int HoLogin::DrawMap(int alpha)
{

	dsDrawTexImage(BlackBoxImage, 0, 0, smScreenWidth, smScreenHeight, 255);

	if (GameMapImage)
	{
		DrawSprite((smScreenWidth / 2 - 1920 / 2), (smScreenHeight / 2 - 1080 / 2), GameMapImage, 0, 0, 1920, 1080);
	}


	return TRUE;
}

int HoLogin::DrawCharacterInfo()
{
	if (ErrorSelect != ERROR_NONE)
		return FALSE;

	

	if (SelectCurrentState == CHARACTER_SELECT_STATE && MouseMoveCharacterNum != -1 && SelectCurrentState == SelectNextState
		&& FadeCurrentState == FadeNextState && ErrorSelect == ERROR_NONE)
	{
		dsDrawTexImage(UserCharacterInfoBackImage, SETTINGSHANDLER->getWidth() / 2 - 128, SETTINGSHANDLER->getHeight() / 2 + SETTINGSHANDLER->getHeight() / 5, 256, 128, 255);
		if (DownUserCharacterInfo[MouseMoveCharacterNum].ClanFlag)
			dsDrawTexImage(UserCharacterInfoImage, SETTINGSHANDLER->getWidth() / 2 - 128, SETTINGSHANDLER->getHeight() / 2 + SETTINGSHANDLER->getHeight() / 5, 256, 128, 255);
		else
			HoDrawTexImage(UserCharacterInfoImage, SETTINGSHANDLER->getWidth() / 2 - 128, SETTINGSHANDLER->getHeight() / 2 + SETTINGSHANDLER->getHeight() / 5 + 13, 0, 33, 256, 128 - 33, 255);
	}

	

#ifdef _LANGUAGE_VEITNAM
	int textX = smScreenWidth / 2 - 128 + 102 + 13;
	int textY = smScreenHeight / 2 + smScreenHeight / 5 - 5;
#else
	int textX = SETTINGSHANDLER->getWidth() / 2 - 128 + 102;
	int textY = SETTINGSHANDLER->getHeight() / 2 + SETTINGSHANDLER->getHeight() / 5;
#endif

	if (DownUserCharacterInfo[MouseMoveCharacterNum].ClanFlag == TRUE)
		textY += 20;

	HDC hdc = NULL;
	char *szJobName = nullptr;

	SetBkMode(hdc, TRANSPARENT);
	SelectObject(hdc, Font);


	if (DownUserCharacterInfo[MouseMoveCharacterNum].ClanFlag)
	{
		SetFontTextColor(RGB(150, 255, 200));
		dsTextLineOut(textX + 18, textY - 3, DownUserCharacterInfo[MouseMoveCharacterNum].ClanName, strlen(DownUserCharacterInfo[MouseMoveCharacterNum].ClanName));
	}

	SetFontTextColor(RGB(255, 220, 100));
	char buffer[256];
	memset(buffer, 0, sizeof(buffer));
	wsprintf(buffer, "%d", DownUserCharacterInfo[MouseMoveCharacterNum].Level);
	dsTextLineOut(textX + 2, textY + 36, buffer, strlen(buffer));

	SetFontTextColor(RGB(255, 250, 235));
	dsTextLineOut(textX + 2, textY + 16, DownUserCharacterInfo[MouseMoveCharacterNum].Name, strlen(DownUserCharacterInfo[MouseMoveCharacterNum].Name));

	const int jobCode = DownUserCharacterInfo[MouseMoveCharacterNum].Job & 0xFFFF;
	memset(buffer, 0, sizeof(buffer));
	const gameplay::classes::ClassService &classService = gameplay::classes::DefaultClassService();
	if (classService.IsInBrood(jobCode, gameplay::classes::brood::Tempskron))
		wsprintf(buffer, "%s", HoTextTempskron);
	else if (classService.IsInBrood(jobCode, gameplay::classes::brood::Morayion))
		wsprintf(buffer, "%s", HoTextMoryon);

	dsTextLineOut(textX + 2, textY + 56, buffer, strlen(buffer));

	JOB_DATA_CODE *jobData = GetJobDataCode(DownUserCharacterInfo[MouseMoveCharacterNum].Job, 0);
	if(jobData)
		szJobName = jobData->szName2;
	else
	{
		const gameplay::classes::PlayableClassDefinition *definition = classService.FindEnabled(jobCode);
		if(definition)
			szJobName = const_cast<char *>(definition->displayKey.c_str());
	}
	memset(buffer, 0, sizeof(buffer));
	if(szJobName)
	{
		wsprintf(buffer, "%s", szJobName);
		dsTextLineOut(textX + 2, textY + 76, buffer, strlen(buffer));
	}


	if (DownUserCharacterInfo[MouseMoveCharacterNum].ClanFlag)
	{
		if (DownUserCharacterInfo[MouseMoveCharacterNum].ClanMark != NULL)
		{
			DrawSprite(textX-1, textY-4, DownUserCharacterInfo[MouseMoveCharacterNum].ClanMark, 0, 0, 16, 16);
		}
	}

	return TRUE;
}

int HoLogin::DrawSelectSubState()
{
	if (SelectNextState == COMPLETE_SELECT_STATE || SelectNextState == START_GAME_STATE)
		return FALSE;

	int flag = 0;
	switch (SelectSubState)
	{
	case SUB_SELECT_CHARACTER_MANAGE:
	{
		flag = MessageBox.Main(pCursorPos.x, pCursorPos.y, MouseButton[0]);
		if (flag == 1)
		{
			PlayWav2(4 - 1);

			int isDELETE_character(char *charName);
			int chk = isDELETE_character(DownUserCharacterInfo[MouseDownCharacterNum].Name);

			if (chk == 102 || chk == 1)
			{
				PlayWav2(6 - 1);
				UserCharacter[MouseDownCharacterNum].Init();
				UserCharacterCount--;


				if (smWsockDataServer)
				{
					TransUserCommand(OpCode::OPCODE_DELRECORDDATA, UserAccount, DownUserCharacterInfo[MouseDownCharacterNum].Name);
				}
				MouseDownCharacterNum = -1;
				SelectSubState = SUB_SELECT_NONE;
			}
			else if (chk == 101 || chk == 100)
			{
				ErrorSelect = ERROR_DELETE_CLAN_MEMBER;
				SelectSubState = SUB_SELECT_NONE;
			}
			else if (chk == 103)
			{
				ErrorSelect = ERROR_CLAN_SERVER_NONE;
				SelectSubState = SUB_SELECT_NONE;
			}
			else
			{
				ErrorSelect = ERROR_CLAN_SERVER_NONE;
				SelectSubState = SUB_SELECT_NONE;
			}

		}
		else if (flag == 2)
		{
			PlayWav2(4 - 1);
			SelectSubState = SUB_SELECT_NONE;
		}

		break;
	}

	case SUB_SELECT_CHARACTER_USER:
	{
		flag = MessageBox.Main(pCursorPos.x, pCursorPos.y, MouseButton[0]);
		if (flag == 1 || VRKeyBuff[VK_RETURN])
		{
			PlayWav2(12 - 1);
			SelectNextState = COMPLETE_SELECT_STATE;
			MessageBox.Flag = FALSE;

			LoadLoadingImage(DownUserCharacterInfo[MouseDownCharacterNum].Level);
			SetFade();
		}
		else if (flag == 2)
		{
			PlayWav2(4 - 1);
			SelectSubState = SUB_SELECT_NONE;
			MessageBox.Flag = FALSE;
		}
		break;
	}

	case SUB_SELECT_CHARACTER_NEW:
	{
#ifdef __MINGW32__
		// MINGW_NEW_CONFIRM_TRACE_V1
		{
			static int s_newConfirmTraceCount = 0;

			if ((MouseButton[0] || VRKeyBuff[VK_RETURN]) &&
				s_newConfirmTraceCount < 40)
			{
				FILE *fp = SummitHoLoginRuntimeTraceFile("LOGS\\mingw_new_confirm.txt", "a");
				if (fp)
				{
					fprintf(fp,
						"BEFORE x=%d y=%d mouse=%d enter=%d msgFlag=%d sub=%d\\n",
						pCursorPos.x,
						pCursorPos.y,
						MouseButton[0],
						VRKeyBuff[VK_RETURN],
						MessageBox.Flag,
						SelectSubState);
					fclose(fp);
				}
			}
		}
#endif

		flag = MessageBox.Main(pCursorPos.x, pCursorPos.y, MouseButton[0]);

#ifdef __MINGW32__
		{
			static int s_newConfirmAfterCount = 0;

			if ((MouseButton[0] || VRKeyBuff[VK_RETURN] || flag != 0) &&
				s_newConfirmAfterCount < 40)
			{
				FILE *fp = SummitHoLoginRuntimeTraceFile("LOGS\\mingw_new_confirm.txt", "a");
				if (fp)
				{
					fprintf(fp,
						"AFTER x=%d y=%d mouse=%d enter=%d flag=%d msgFlag=%d sub=%d\\n",
						pCursorPos.x,
						pCursorPos.y,
						MouseButton[0],
						VRKeyBuff[VK_RETURN],
						flag,
						MessageBox.Flag,
						SelectSubState);
					fclose(fp);
				}
			}

			if (MouseButton[0] || VRKeyBuff[VK_RETURN] || flag != 0)
				s_newConfirmAfterCount++;
		}
#endif

		if (flag == 1 || VRKeyBuff[VK_RETURN])
		{
			PlayWav2(12 - 1);
			ServerReturn = NULL;

#ifdef __MINGW32__
			// MINGW_NEW_SERVER_TRACE_V1
			{
				FILE *fp = SummitHoLoginRuntimeTraceFile("LOGS\\mingw_new_server.txt", "a");
				if (fp)
				{
					fprintf(fp,
						"SEND BEFORE sub=%d serverReturn=%p name=[%s]\n",
						SelectSubState,
						(void*)ServerReturn,
						UserCharacterNewName);
					fclose(fp);
				}
			}
#endif

			TransUserCommand(OpCode::OPCODE_INSRECORDDATA, UserAccount, UserCharacterNewName);
			SelectSubState = SUB_SELECT_CHARACTER_NEW_SERVER;

#ifdef __MINGW32__
			{
				FILE *fp = SummitHoLoginRuntimeTraceFile("LOGS\\mingw_new_server.txt", "a");
				if (fp)
				{
					fprintf(fp,
						"SEND AFTER sub=%d serverReturn=%p\n",
						SelectSubState,
						(void*)ServerReturn);
					fclose(fp);
				}
			}
#endif

			LoadLoadingImage(0);
		}
		else if (flag == 2)
		{
			PlayWav2(4 - 1);
			SelectSubState = SUB_SELECT_NONE;
			hFocusWnd = hTextWnd;
			FocusWndFlag = TRUE;
#ifdef	_LANGUAGE_JAPANESE		
			SetIME_Mode(1);
#endif
		}
		break;
	}

	case SUB_SELECT_CHARACTER_NEW_SERVER:
	{
#ifdef __MINGW32__
		{
			static int s_newServerWaitTrace = 0;

			if (s_newServerWaitTrace < 120)
			{
				FILE *fp = SummitHoLoginRuntimeTraceFile("LOGS\\mingw_new_server.txt", "a");
				if (fp)
				{
					if (ServerReturn)
						fprintf(fp,
							"WAIT sub=%d serverReturn=%p wParam=%ld\n",
							SelectSubState,
							(void*)ServerReturn,
							(long)ServerReturn->WParam);
					else
						fprintf(fp,
							"WAIT sub=%d serverReturn=NULL\n",
							SelectSubState);

					fclose(fp);
				}

				s_newServerWaitTrace++;
			}
		}
#endif

		if (ServerReturn != NULL)
		{
			if (ServerReturn->WParam == 1)
			{
				SelectNextState = COMPLETE_SELECT_STATE;
				SetFade();
				memset(PlayerName, 0, sizeof(PlayerName));
				memcpy(PlayerName, UserCharacterNewName, sizeof(PlayerName));
			}
			else
			{
				memset(UserCharacterNewName, 0, sizeof(UserCharacterNewName));
				ErrorSelect = ERROR_EXHIST_NEWPLAYER;
				SelectSubState = SUB_SELECT_NONE;
			}
		}
		break;
	}

	case SUB_SELECT_BACK:
	{

		SetFade();
		SelectSubState = SUB_SELECT_NONE;
		SelectNextState = RETURN_OPENING_STATE;
		break;
	}
	}

	return TRUE;
}

int HoLogin::KeyFilter()
{

	int start;
	int result = 0;

	if (hFocusWnd != NULL && JobFaceSelectNum != -1 && hTextWnd != NULL)
	{

		SendMessage(hTextWnd, EM_SETLIMITTEXT, 15, 0);
		memset(UserCharacterNewName, 0, sizeof(UserCharacterNewName));
		GetWindowText(hFocusWnd, UserCharacterNewName, sizeof(UserCharacterNewName));

		for (int index = 0; index < (int)lstrlen(UserCharacterNewName); index++)
		{


#ifdef	_LANGUAGE_VEITNAM

			if (
				(UserCharacterNewName[index] < 0) ||
				(UserCharacterNewName[index] >= 0 && UserCharacterNewName[index] <= 47) ||
				(UserCharacterNewName[index] >= 58 && UserCharacterNewName[index] <= 64) ||
				(UserCharacterNewName[index] >= 91 && UserCharacterNewName[index] <= 96) ||
				(UserCharacterNewName[index] >= 123 && UserCharacterNewName[index] <= 127) ||
				(UserCharacterNewName[index] == 0x09) ||
				(UserCharacterNewName[index] == 0xff || UserCharacterNewName[index] == 0x20)
				)
			{
				UserCharacterNewName[index] = 0;
				SetWindowText(hFocusWnd, 0);
				SetWindowText(hFocusWnd, UserCharacterNewName);
				SendMessage(hFocusWnd, EM_SETSEL, lstrlen(UserCharacterNewName), lstrlen(UserCharacterNewName) + 1);
				break;
			}
#else

#ifdef _LANGUAGE_CHINESE

			if (IsCurseID(UserCharacterNewName) == TRUE)
			{
				UserCharacterNewName[index] = 0;
				SetWindowText(hFocusWnd, 0);
				SetWindowText(hFocusWnd, UserCharacterNewName);
				SendMessage(hFocusWnd, EM_SETSEL, lstrlen(UserCharacterNewName), lstrlen(UserCharacterNewName) + 1);
				break;
			}
			if (
				(UserCharacterNewName[index] == 86 || UserCharacterNewName[index] == 118) ||
				(UserCharacterNewName[index] < 48 && UserCharacterNewName[index] > 0) ||
				(UserCharacterNewName[index] >= 58 && UserCharacterNewName[index] <= 64) ||
				(UserCharacterNewName[index] >= 91 && UserCharacterNewName[index] <= 96) ||
				(UserCharacterNewName[index] >= 123 && UserCharacterNewName[index] <= 127) ||
				(UserCharacterNewName[index] == 0x09) ||
				(UserCharacterNewName[index] == ' ') ||
				(UserCharacterNewName[index] == 0xff || UserCharacterNewName[index] == 0x20)
				)
			{
				UserCharacterNewName[index] = 0;
				SetWindowText(hFocusWnd, 0);
				SetWindowText(hFocusWnd, UserCharacterNewName);
				SendMessage(hFocusWnd, EM_SETSEL, lstrlen(UserCharacterNewName), lstrlen(UserCharacterNewName) + 1);
				break;
			}


			if (UserCharacterNewName[index] == -93)
			{
				if (UserCharacterNewName[index + 1] >= -95 && UserCharacterNewName[index + 1] <= -2)
				{
					UserCharacterNewName[index] = 0;
					SetWindowText(hFocusWnd, 0);
					SetWindowText(hFocusWnd, UserCharacterNewName);
					SendMessage(hFocusWnd, EM_SETSEL, lstrlen(UserCharacterNewName), lstrlen(UserCharacterNewName) + 1);
					break;
				}
			}

#else
#ifdef _LANGUAGE_JAPANESE
			if (IsCurseID(UserCharacterNewName) == TRUE)
			{
				UserCharacterNewName[index] = 0;
				SetWindowText(hFocusWnd, 0);
				SetWindowText(hFocusWnd, UserCharacterNewName);
				SendMessage(hFocusWnd, EM_SETSEL, lstrlen(UserCharacterNewName), lstrlen(UserCharacterNewName) + 1);
				break;
			}
			if (index > 0)
			{
				if ((((UserCharacterNewName[index - 1] & 0xff) >= 0x81) && ((UserCharacterNewName[index - 1] & 0xff) <= 0x9F))
					|| (((UserCharacterNewName[index - 1] & 0xff) >= 0xE0) && ((UserCharacterNewName[index - 1] & 0xff) <= 0xEF)))
				{
					index = index;
				}
				else if ((UserCharacterNewName[index] >= 0x1f && UserCharacterNewName[index] <= 0x2f) ||
					(UserCharacterNewName[index] >= 0x3a && UserCharacterNewName[index] <= 0x40) ||
					(UserCharacterNewName[index] == 0x5c) ||
					(UserCharacterNewName[index] == 0x5e) ||
					(UserCharacterNewName[index] == 0x5f) ||
					(UserCharacterNewName[index] == 0x60) ||
					(UserCharacterNewName[index] == 0x09) ||

					(UserCharacterNewName[index] >= 0x7b && UserCharacterNewName[index] <= 0xff))
				{
					UserCharacterNewName[index] = 0;
					SetWindowText(hFocusWnd, 0);
					SetWindowText(hFocusWnd, UserCharacterNewName);
					SendMessage(hFocusWnd, EM_SETSEL, lstrlen(UserCharacterNewName), lstrlen(UserCharacterNewName) + 1);
					break;
				}
			}
			else
			{
				if ((UserCharacterNewName[index] >= 0x1f && UserCharacterNewName[index] <= 0x2f) ||
					(UserCharacterNewName[index] >= 0x3a && UserCharacterNewName[index] <= 0x40) ||
					(UserCharacterNewName[index] == 0x5c) ||
					(UserCharacterNewName[index] == 0x5e) ||
					(UserCharacterNewName[index] == 0x5f) ||
					(UserCharacterNewName[index] == 0x60) ||
					(UserCharacterNewName[index] == 0x09) ||

					(UserCharacterNewName[index] >= 0x7b && UserCharacterNewName[index] <= 0xff))
				{
					UserCharacterNewName[index] = 0;
					SetWindowText(hFocusWnd, 0);
					SetWindowText(hFocusWnd, UserCharacterNewName);
					SendMessage(hFocusWnd, EM_SETSEL, lstrlen(UserCharacterNewName), lstrlen(UserCharacterNewName) + 1);
					break;
				}
			}
#else
#ifdef _LANGUAGE_ARGENTINA
			if (
				(UserCharacterNewName[index] >= 0x41 && UserCharacterNewName[index] <= 0x5A) ||
				(UserCharacterNewName[index] >= 0x61 && UserCharacterNewName[index] <= 0x7A) ||
#ifdef	GAME_GM
				(UserCharacterNewName[index] == '[') ||
				(UserCharacterNewName[index] == ']') ||
#endif
				(UserCharacterNewName[index] == 0xC1) ||
				(UserCharacterNewName[index] == 0xC9) ||
				(UserCharacterNewName[index] == 0xCD) ||
				(UserCharacterNewName[index] == 0xD1) ||
				(UserCharacterNewName[index] == 0xD3) ||
				(UserCharacterNewName[index] == 0xDA) ||
				(UserCharacterNewName[index] == 0xDC) ||
				(UserCharacterNewName[index] == 0xE1) ||
				(UserCharacterNewName[index] == 0xE9) ||
				(UserCharacterNewName[index] == 0xED) ||
				(UserCharacterNewName[index] == 0xF1) ||
				(UserCharacterNewName[index] == 0xF3) ||
				(UserCharacterNewName[index] == 0xFA) ||
				(UserCharacterNewName[index] == 0xFC))
			{
			}
			else
			{
				UserCharacterNewName[index] = 0;
				SetWindowText(hFocusWnd, 0);
				SetWindowText(hFocusWnd, UserCharacterNewName);
				SendMessage(hFocusWnd, EM_SETSEL, lstrlen(UserCharacterNewName), lstrlen(UserCharacterNewName) + 1);
			}
#ifndef	GAME_GM
			if (
				(UserCharacterNewName[index] == 'G' && UserCharacterNewName[index + 1] == 'M') ||
				(UserCharacterNewName[index] == '[') ||
				(UserCharacterNewName[index] == ']')
				)
			{
				UserCharacterNewName[index] = 0;
				SetWindowText(hFocusWnd, 0);
				SetWindowText(hFocusWnd, UserCharacterNewName);
				SendMessage(hFocusWnd, EM_SETSEL, lstrlen(UserCharacterNewName), lstrlen(UserCharacterNewName) + 1);
			}
#endif
#else


			if ((UserCharacterNewName[index] >= 0x1f && UserCharacterNewName[index] <= 0x2f) ||
				(UserCharacterNewName[index] >= 0x3a && UserCharacterNewName[index] <= 0x40) ||
				(UserCharacterNewName[index] == 0x5c) ||
				(UserCharacterNewName[index] == 0x5e) ||
				(UserCharacterNewName[index] == 0x5f) ||
				(UserCharacterNewName[index] == 0x60) ||

				(UserCharacterNewName[index] == 0x09) ||
				(UserCharacterNewName[index] >= 0x0e && UserCharacterNewName[index] <= 0x0f) ||
				(UserCharacterNewName[index] == 0x16) ||
				(UserCharacterNewName[index] == 0x20) ||
				(UserCharacterNewName[index] == 0xff) ||
				(UserCharacterNewName[index] >= 0x7b && UserCharacterNewName[index] <= 0x7f))
			{
				UserCharacterNewName[index] = 0;
				SetWindowText(hFocusWnd, 0);
				SetWindowText(hFocusWnd, UserCharacterNewName);
				SendMessage(hFocusWnd, EM_SETSEL, lstrlen(UserCharacterNewName), lstrlen(UserCharacterNewName) + 1);
				break;
			}
#endif
#endif
#endif
#endif
		}

		if (lstrlen(UserCharacterNewName) >= 1)
		{
			for (int index = 0; index < (int)lstrlen(UserCharacterNewName); index++)
			{
#ifdef _LANGUAGE_BRAZIL
				if ((unsigned char)UserCharacterNewName[index] == 255 || (unsigned char)UserCharacterNewName[index] == 160)
				{
					UserCharacterNewName[index] = 0;
					SetWindowText(hFocusWnd, 0);
					SetWindowText(hFocusWnd, UserCharacterNewName);
					SendMessage(hFocusWnd, EM_SETSEL, lstrlen(UserCharacterNewName), lstrlen(UserCharacterNewName) + 1);
				}
#endif



				if ((BYTE)UserCharacterNewName[index] >= 0x80)
				{


#ifdef _LANGUAGE_CHINESE


					int fstByte = (BYTE)UserCharacterNewName[index];
					int sndByte = (BYTE)UserCharacterNewName[index + 1];

					if (fstByte < 0xA1 || fstByte>0xF7 || sndByte < 0xA1 || sndByte>0xFE)
					{
						UserCharacterNewName[index] = 0;
						SetWindowText(hFocusWnd, 0);
						SetWindowText(hFocusWnd, UserCharacterNewName);
						SendMessage(hFocusWnd, EM_SETSEL, lstrlen(UserCharacterNewName), lstrlen(UserCharacterNewName) + 1);
						break;
					}
					else
						index++;

#endif

#ifdef _LANGUAGE_KOREAN

					int fstByte = (BYTE)UserCharacterNewName[index];
					int sndByte = (BYTE)UserCharacterNewName[index + 1];

					result = (fstByte - 0xb0) * 94 + sndByte - 0xa1;

					if (result < 0 || result >= 2350)
					{
						UserCharacterNewName[index] = 0;
						SetWindowText(hFocusWnd, 0);
						SetWindowText(hFocusWnd, UserCharacterNewName);
						SendMessage(hFocusWnd, EM_SETSEL, lstrlen(UserCharacterNewName), lstrlen(UserCharacterNewName) + 1);
						break;
					}
					else
						index++;
#endif

				}
			}
		}

#ifdef _LANGUAGE_JAPANESE

		result = IsJTS(UserCharacterNewName);
		if (result >= 0)
		{
			UserCharacterNewName[result] = 0;

			SetWindowText(hFocusWnd, 0);
			SetWindowText(hFocusWnd, UserCharacterNewName);
			SendMessage(hFocusWnd, EM_SETSEL, lstrlen(UserCharacterNewName), lstrlen(UserCharacterNewName) + 1);
		}

#endif

#ifdef _LANGUAGE_ENGLISH
		result = IsENG(UserCharacterNewName);
		if (result >= 0)
		{
			UserCharacterNewName[result] = 0;

			SetWindowText(hFocusWnd, 0);
			SetWindowText(hFocusWnd, UserCharacterNewName);
			SendMessage(hFocusWnd, EM_SETSEL, lstrlen(UserCharacterNewName), lstrlen(UserCharacterNewName) + 1);
		}

#endif

#ifdef _LANGUAGE_THAI
		result = IsTHAI(UserCharacterNewName);
		if ((result >= 0) && (IsCurse(UserCharacterNewName) == 0))
		{
			UserCharacterNewName[result] = 0;

			SetWindowText(hFocusWnd, 0);
			SetWindowText(hFocusWnd, UserCharacterNewName);
			SendMessage(hFocusWnd, EM_SETSEL, lstrlen(UserCharacterNewName), lstrlen(UserCharacterNewName) + 1);
		}
		else if (IsCurse(UserCharacterNewName) != 0)
		{
			SetWindowText(hFocusWnd, "");
			ZeroMemory(UserCharacterNewName, sizeof(char) * 256);
		}
#endif
#ifdef _LANGUAGE_TAIWAN

		result = IsTaiwan(UserCharacterNewName);
		if (result >= 0)
		{
			UserCharacterNewName[result] = 0;

			SetWindowText(hFocusWnd, 0);
			SetWindowText(hFocusWnd, UserCharacterNewName);
			SendMessage(hFocusWnd, EM_SETSEL, lstrlen(UserCharacterNewName), lstrlen(UserCharacterNewName) + 1);
		}
#endif

		SendMessage(hFocusWnd, EM_GETSEL, (WPARAM)&start, NULL);
		HDC hdc = NULL;

		SetBkMode(hdc, TRANSPARENT);
		SetFontTextColor(RGB(255, 255, 255));
		SelectObject(hdc, Font);


		static int i = 0;
		if (i < 6)
		{
#ifdef	_LANGUAGE_VEITNAM
			int newTexPosX = int(((float)smScreenWidth*356.f) / 800.f + 13);
#else
			int newTexPosX = int(((float)SETTINGSHANDLER->getWidth()*356.f) / 800.f);
#endif
			int newTexPosY = int(((float)SETTINGSHANDLER->getHeight()*397.f) / 600.f);

			dsTextLineOut(newTexPosX + start * 6, newTexPosY, "|", 1);
		}

		i++;
		if (i > 12)
			i = 0;

	}
	return TRUE;
}

int HoLogin::Draw()
{
	MainFade();


	if (ErrorSelect != ERROR_NONE)
	{
		if (MouseButton[0] || MouseDblClick)
		{
			if (ErrorSelect == ERROR_NONAME_NEWPLAYER)
			{
				hFocusWnd = hTextWnd;
				FocusWndFlag = TRUE;
			}
			ErrorSelect = ERROR_NONE;
			MouseButton[0] = FALSE;
			MouseDblClick = FALSE;
#ifdef	_LANGUAGE_JAPANESE		
			SetIME_Mode(1);
#endif

			StartErrorDelayFlag = FALSE;
		}

		for (int index = 0; index < 256; index++)
		{
			if (VRKeyBuff[index] == TRUE)
			{
				if (ErrorSelect == ERROR_NONAME_NEWPLAYER)
				{
					hFocusWnd = hTextWnd;
					FocusWndFlag = TRUE;
#ifdef	_LANGUAGE_JAPANESE		
					SetIME_Mode(1);
#endif
				}

				ErrorSelect = ERROR_NONE;
				StartErrorDelayFlag = FALSE;
				break;
			}
		}
	}

	DrawSelectSubState();

	int x, y, z;
	int anx, any;


	anx = CaAngle.x;
	any = CaAngle.y;

	any &= ANGCLIP;
	anx &= ANGCLIP;

	GetMoveLocation(0, 0, -TarDist, anx, any, 0);
	x = TarPosi.x + GeoResult_X;
	y = TarPosi.y + GeoResult_Y;
	z = TarPosi.z + GeoResult_Z;


	d3ddev->Clear(0, 0, 2, smBackColor, 1.0f, 0);
	smRender.SetCameraPosi(x, y, z, anx, any, 0);







	if (Stage)
		Stage->DrawStage(x, y, z, anx, any, 0);


	DrawUserCharacter(x, y, z, anx, any, 0);
	DrawJobCharacter(x, y, z, anx, any, 0);

	DrawEffect(x, y, z, anx, any, 0);

	DrawGameFrame();
	DrawTitle(SelectCurrentState);

	int twoButtonStartPos = m_ScreenHalf - 120;
	int threeButtonStartPos = m_ScreenHalf - 150;
	int screenWidthHalf = SETTINGSHANDLER->getWidth() / 2;
	int screenHeightHalf = SETTINGSHANDLER->getHeight() / 2;

	if (SelectCurrentState == SelectNextState && FadeCurrentState == FadeNextState)
	{
		MainDefaultButton();


		if (SelectCurrentState == CHARACTER_SELECT_STATE)
		{
			int userCount = 0;
			for (int index = 0; index < DownUserCharacterCount; index++)
			{
				if (UserCharacter[index].DisplayFlag)
					userCount++;
			}
            //Impedir Criação de Char
			if (pCursorPos.x >(screenWidthHalf - 128) && pCursorPos.x < (screenWidthHalf + 128) &&
				pCursorPos.y >(screenHeightHalf / 2 - 80) && pCursorPos.y < (screenHeightHalf / 2 + 130) &&
				SelectSubState == SUB_SELECT_NONE && ErrorSelect == ERROR_NONE)
			{

				dsDrawTexImage(NewCharacterImage, (m_ScreenHalf)-128, 150, 256, 64, 255);

				if (MouseButton[0])
				{

					if (UserCharacterCount < CHARACTER_USER_MAX)
					{
						SelectNextState = RACE_SELECT_STATE;
						PlayWav2(9 - 1);
					}
					else
					{
						MouseButton[0] = FALSE;
						MouseDblClick = FALSE;
						ErrorSelect = ERROR_MAX_NEWPLAYER;
					}
				}
			}
			else
			{
				if (userCount == 0)
				{
					dsDrawTexImage(NewCharacterImage, (m_ScreenHalf)-128, 150, 256, 64, (int)NewCharacterImageBlendCount);
				}
			}

			if (MouseDownCharacterNum != -1)
				DeleteButton.Draw(0, SETTINGSHANDLER->getHeight() - 40);

			if (userCount == 0)
			{
				// SUMMIT_NO_LOWER_NEW_CHARACTER_DRAW_V13A
				// The character-card action owns creation; only Sair remains below.
				QuitButton.Draw(m_ScreenHalf - 64, SETTINGSHANDLER->getHeight() - 40);
			}
			else
			{
				// Existing characters keep only Select + Sair in the lower bar.
				OkButton.Draw(twoButtonStartPos, SETTINGSHANDLER->getHeight() - 40);
				QuitButton.Draw(twoButtonStartPos + 110, SETTINGSHANDLER->getHeight() - 40);
			}
		}
		else if (SelectCurrentState == RACE_SELECT_STATE)
		{
			OkButton.Draw(threeButtonStartPos, SETTINGSHANDLER->getHeight() - 40);
			BackButton.Draw(threeButtonStartPos + 100, SETTINGSHANDLER->getHeight() - 40);
			QuitButton.Draw(threeButtonStartPos + 200, SETTINGSHANDLER->getHeight() - 40);


			if (pCursorPos.x > (SETTINGSHANDLER->getWidth() / 2 - 140) &&
				pCursorPos.x < (SETTINGSHANDLER->getWidth() / 2 - 140 + 130) &&
				pCursorPos.y >(SETTINGSHANDLER->getHeight() / 5) &&
				pCursorPos.y < (SETTINGSHANDLER->getHeight() / 5 + 130) &&
				RaceMarkBlendCount >= 200 && ErrorSelect == ERROR_NONE)
			{


				dsDrawTexImage(TempscronMarkImage, SETTINGSHANDLER->getWidth() / 2 - 140, SETTINGSHANDLER->getHeight() / 5, 128, 128, 255);
				dsDrawTexImage(TempscronLogoImage, SETTINGSHANDLER->getWidth() / 2 - 140 - 6, SETTINGSHANDLER->getHeight() / 5 + 118, 128, 32, 255);

				dsDrawTexImage(RaceTextBoxImage[0], SETTINGSHANDLER->getWidth() / 2 - 140 - 175, SETTINGSHANDLER->getHeight() / 5 + 150, 256, 256, 255);
				dsDrawTexImage(RaceTextBoxImage[1], SETTINGSHANDLER->getWidth() / 2 - 140 + 256 - 175, SETTINGSHANDLER->getHeight() / 5 + 150, 32, 256, 255);
				dsDrawTexImage(TempscronTextImage, SETTINGSHANDLER->getWidth() / 2 - 140 - 175 + 19, SETTINGSHANDLER->getHeight() / 5 + 150 + 12, 256, 128, 255);

				if (MouseButton[0])
				{

					tribeNum = TRIBE_Tempscron;


					LoadSelectableJobCharacters();
					PlayWav2(9 - 1);
					SelectNextState = JOB_SELECT_STATE;
					RaceMarkScaleSize = 128;

				}
			}
			else
			{
				dsDrawTexImage(TempscronMarkImage, SETTINGSHANDLER->getWidth() / 2 - 140, SETTINGSHANDLER->getHeight() / 5, 128, 128, RaceMarkBlendCount);
				dsDrawTexImage(TempscronLogoImage, SETTINGSHANDLER->getWidth() / 2 - 140 - 6, SETTINGSHANDLER->getHeight() / 5 + 118, 128, 32, RaceMarkBlendCount);
			}


			if ((SETTINGSHANDLER->getWidth() / 2 + 12) < pCursorPos.x  && pCursorPos.x < (SETTINGSHANDLER->getWidth() / 2 + 12 + 130) &&
				(SETTINGSHANDLER->getHeight() / 5) < pCursorPos.y  && pCursorPos.y < (SETTINGSHANDLER->getHeight() / 5 + 130)
				&& RaceMarkBlendCount >= 200 && ErrorSelect == ERROR_NONE)
			{

				dsDrawTexImage(MoryonMarkImage, SETTINGSHANDLER->getWidth() / 2 + 12, SETTINGSHANDLER->getHeight() / 5, 128, 128, 255);
				dsDrawTexImage(MoryonLogoImage, SETTINGSHANDLER->getWidth() / 2 + 12 - 6, SETTINGSHANDLER->getHeight() / 5 + 118, 128, 32, 255);

				dsDrawTexImage(RaceTextBoxImage[0], SETTINGSHANDLER->getWidth() / 2 + 12, SETTINGSHANDLER->getHeight() / 5 + 150, 256, 256, 255);
				dsDrawTexImage(RaceTextBoxImage[1], SETTINGSHANDLER->getWidth() / 2 + 12 + 256, SETTINGSHANDLER->getHeight() / 5 + 150, 32, 256, 255);
				dsDrawTexImage(MoryonTextImage, SETTINGSHANDLER->getWidth() / 2 + 12 + 19, SETTINGSHANDLER->getHeight() / 5 + 150 + 12, 256, 256, 255);

				if (MouseButton[0])
				{

					tribeNum = TRIBE_Moryon;


					LoadSelectableJobCharacters();
					PlayWav2(9 - 1);
					SelectNextState = JOB_SELECT_STATE;
					RaceMarkScaleSize = 128;
				}
			}
			else
			{
				dsDrawTexImage(MoryonMarkImage, SETTINGSHANDLER->getWidth() / 2 + 12, SETTINGSHANDLER->getHeight() / 5, 128, 128, RaceMarkBlendCount);
				dsDrawTexImage(MoryonLogoImage, SETTINGSHANDLER->getWidth() / 2 + 12 - 6, SETTINGSHANDLER->getHeight() / 5 + 118, 128, 32, RaceMarkBlendCount);
			}
		}
		else if (SelectCurrentState == JOB_SELECT_STATE)
		{
			OkButton.Draw(threeButtonStartPos, SETTINGSHANDLER->getHeight() - 40);
			BackButton.Draw(threeButtonStartPos + 100, SETTINGSHANDLER->getHeight() - 40);
			QuitButton.Draw(threeButtonStartPos + 200, SETTINGSHANDLER->getHeight() - 40);


			RECT rect;
			POINT	sPos;
			smRender.SetCameraPosi(x, y, z, anx, any, 0);

			for (int index = 0; index < JobCharacterCount; index++)
			{
				smRender.GetRect2D(JobCharacter[index].pX, JobCharacter[index].pY + (32 * fONE), JobCharacter[index].pZ, 32 * fONE, 32 * fONE, &rect, &sPos);


				switch (tribeNum)
				{
				case TRIBE_Tempscron:
					if (MouseMoveCharacterNum == index || MouseDownCharacterNum == index)
					{
						dsDrawTexImage(TempscronJobImage[index][1], rect.left - 10, rect.top - 15, 128, 32, 255);

						if (MouseDownCharacterNum != index)
						{

							dsDrawTexImage(JobTextBoxImage[0], SETTINGSHANDLER->getWidth() / 2 - 140, SETTINGSHANDLER->getHeight() / 5, 256, 256, 255);
							dsDrawTexImage(JobTextBoxImage[1], SETTINGSHANDLER->getWidth() / 2 - 140 + 256, SETTINGSHANDLER->getHeight() / 5, 32, 256, 255);
							dsDrawTexImage(TempscronJobTextImage[index], SETTINGSHANDLER->getWidth() / 2 - 140 + 10, SETTINGSHANDLER->getHeight() / 5 + 4, 256, 256, 255);
						}
					}
					else
						dsDrawTexImage(TempscronJobImage[index][0], rect.left - 10, rect.top - 15, 128, 32, 255);
					break;
				case TRIBE_Moryon:
					if (MouseMoveCharacterNum == index || MouseDownCharacterNum == index)
					{
						dsDrawTexImage(MoryonJobImage[index][1], rect.left - 10, rect.top - 15, 128, 32, 255);
						if (MouseDownCharacterNum != index)
						{
							dsDrawTexImage(JobTextBoxImage[0], SETTINGSHANDLER->getWidth() / 2 - 140, SETTINGSHANDLER->getHeight() / 5, 256, 256, 255);
							dsDrawTexImage(JobTextBoxImage[1], SETTINGSHANDLER->getWidth() / 2 - 140 + 256, SETTINGSHANDLER->getHeight() / 5, 32, 256, 255);

							dsDrawTexImage(MoryonJobTextImage[index], SETTINGSHANDLER->getWidth() / 2 - 140 + 10, SETTINGSHANDLER->getHeight() / 5 + 4, 256, 256, 255);
						}
					}
					else
						dsDrawTexImage(MoryonJobImage[index][0], rect.left - 10, rect.top - 15, 128, 32, 255);
					break;

				}
			}
		}
		else if (SelectCurrentState == FACE_SELECT_STATE)
		{
			OkButton.Draw(threeButtonStartPos, SETTINGSHANDLER->getHeight() - 40);
			BackButton.Draw(threeButtonStartPos + 100, SETTINGSHANDLER->getHeight() - 40);
			QuitButton.Draw(threeButtonStartPos + 200, SETTINGSHANDLER->getHeight() - 40);

			dsDrawTexImage(BlackBoxImage, 150, 410, 500, 128 + 10, 20);
		}
	}

	if ((SelectCurrentState == RACE_SELECT_STATE) && (SelectNextState == JOB_SELECT_STATE))
	{
		if (tribeNum == TRIBE_Tempscron)
			dsDrawTexImage(TempscronMarkImage, SETTINGSHANDLER->getWidth() / 2 - RaceMarkScaleSize / 2, SETTINGSHANDLER->getHeight() / 3 - RaceMarkScaleSize / 2 - 40, RaceMarkScaleSize, RaceMarkScaleSize, RaceMarkScaleSize);

		if (tribeNum == TRIBE_Moryon)
			dsDrawTexImage(MoryonMarkImage, SETTINGSHANDLER->getWidth() / 2 - RaceMarkScaleSize / 2, SETTINGSHANDLER->getHeight() / 3 - RaceMarkScaleSize / 2 - 40, RaceMarkScaleSize, RaceMarkScaleSize, RaceMarkScaleSize);
	}

	

	IncTextureFrame();


	if (SelectCurrentState == SelectNextState && FadeCurrentState == FadeNextState)
	{

		if (SelectCurrentState == CHARACTER_SELECT_STATE)
		{

			if (MouseMoveCharacterNum != -1)
				DrawCharacterInfo();

			if (SelectSubState == SUB_SELECT_CHARACTER_MANAGE)

				MessageBox.Draw(SETTINGSHANDLER->getWidth() / 2 - 128, SETTINGSHANDLER->getHeight() / 2 + SETTINGSHANDLER->getHeight() / 6, HoTextSelectMessage[0], RGB(255, 0, 0));
			if (SelectSubState == SUB_SELECT_CHARACTER_USER)

				MessageBox.Draw(SETTINGSHANDLER->getWidth() / 2 - 128, SETTINGSHANDLER->getHeight() / 2 + SETTINGSHANDLER->getHeight() / 6, HoTextSelectMessage[1]);
			if (ErrorSelect == ERROR_MAX_NEWPLAYER)

				Message.Draw(SETTINGSHANDLER->getWidth() / 2 - 128, SETTINGSHANDLER->getHeight() / 2 + SETTINGSHANDLER->getHeight() / 6, HoTextSelectMessage[2]);
		}
		else if (SelectCurrentState == RACE_SELECT_STATE && ErrorSelect == ERROR_NONE)
		{
		}
		else if (SelectCurrentState == JOB_SELECT_STATE && ErrorSelect == ERROR_NONE)
		{
		}
		else if (SelectCurrentState == FACE_SELECT_STATE)
		{
			if (FaceSelectFirstFlag == FALSE && JobFaceSelectNum != -1)
			{
				FaceSelectFirstFlag = TRUE;
				ApplySelectedClassFace(JobFaceSelectNum);
				JobCharacter[JobBodySelectNum].Flag = 1;
				#ifdef __MINGW32__
				MINGW_CharCreateTraceV4(
					"OpenEach #3 BEFORE",
					JobBodySelectNum,
					JobCharacterCount,
					(JobBodySelectNum >= 0 && JobBodySelectNum < JobCharacterCount) ? &JobCharacter[JobBodySelectNum] : NULL,
					(JobBodySelectNum >= 0 && JobBodySelectNum < JobCharacterCount) ? JobCharacter[JobBodySelectNum].lpDinaPattern2 : NULL,
					(JobBodySelectNum >= 0 && JobBodySelectNum < JobCharacterCount && JobCharacter[JobBodySelectNum].lpDinaPattern2) ? JobCharacter[JobBodySelectNum].lpDinaPattern2->Pat : NULL
				);
				#endif
				OpenEachPlayer(&JobCharacter[JobBodySelectNum]);
				#ifdef __MINGW32__
				MINGW_CharCreateTraceV4(
					"OpenEach #3 AFTER",
					JobBodySelectNum,
					JobCharacterCount,
					(JobBodySelectNum >= 0 && JobBodySelectNum < JobCharacterCount) ? &JobCharacter[JobBodySelectNum] : NULL,
					(JobBodySelectNum >= 0 && JobBodySelectNum < JobCharacterCount) ? JobCharacter[JobBodySelectNum].lpDinaPattern2 : NULL,
					NULL
				);
				#endif

			}

			int addX = SETTINGSHANDLER->getWidth() / 2 - 800 / 2;

			int newPosX = int(((float)SETTINGSHANDLER->getWidth()*272.f) / 800.f);
			int newPosY = int(((float)SETTINGSHANDLER->getHeight()*82.f) / 600.f);

			if (SETTINGSHANDLER->getWidth() == 1280 && SETTINGSHANDLER->getHeight() == 1024)
				newPosY = int((float)SETTINGSHANDLER->getHeight()* (100.f / 600.f));

			int newWidth = int(((float)SETTINGSHANDLER->getWidth()*256.f) / 800.f);
			int newHeight = int(((float)SETTINGSHANDLER->getHeight()*128.f) / 600.f);

			int addY = SETTINGSHANDLER->getHeight() / 2 - 600 / 2;

			

			dsDrawTexImage(FaceBoxBigImage[0], newPosX, newPosY, newWidth, newWidth, 255);
			dsDrawTexImage(FaceBoxBigImage[1], newPosX, newPosY + newWidth, newWidth, newHeight, 255);

			

			DrawEachPlayer(0.5f, 0.4f, 1);

			int    mouseMoveFace = -1;
			bool   selectFaceFlag = false;


			if (JobBodySelectNum >= 0)
			{
				
				for (int index = 0; index < GetCurrentFaceCount(); index++)
				{
					if (pCursorPos.x >(GetCurrentFacePositionX(index) + addX) &&
						pCursorPos.x < (GetCurrentFacePositionX(index) + 128 + addX) &&
						pCursorPos.y >(SETTINGSHANDLER->getHeight() - (70 + 128)) &&
						pCursorPos.y < (SETTINGSHANDLER->getHeight() - (70 + 128) + 128) && ErrorSelect == ERROR_NONE && SelectSubState == SUB_SELECT_NONE)
					{
						mouseMoveFace = index;
						if (MouseButton[0])
						{
							selectFaceFlag = true;
							if (JobFaceSelectNum != index)
							{

								ApplySelectedClassFace(index);

								JobFaceSelectNum = index;

								// SUMMIT_FACE_CLICK_TRACE_V14
								SummitCharacterSelectionTrace("HOLOGIN",
									"FACE_CLICK body=%d face=%d char=%p flag=%d display=%d loading=%d pattern=%p",
									JobBodySelectNum, JobFaceSelectNum,
									(JobBodySelectNum >= 0 && JobBodySelectNum < JobCharacterCount) ? (void *)&JobCharacter[JobBodySelectNum] : NULL,
									(JobBodySelectNum >= 0 && JobBodySelectNum < JobCharacterCount) ? JobCharacter[JobBodySelectNum].Flag : -1,
									(JobBodySelectNum >= 0 && JobBodySelectNum < JobCharacterCount) ? JobCharacter[JobBodySelectNum].DisplayFlag : -1,
									(JobBodySelectNum >= 0 && JobBodySelectNum < JobCharacterCount) ? JobCharacter[JobBodySelectNum].PatLoading : -1,
									(JobBodySelectNum >= 0 && JobBodySelectNum < JobCharacterCount) ? (void *)JobCharacter[JobBodySelectNum].Pattern : NULL);

								JobCharacter[JobBodySelectNum].Flag = 1;
								#ifdef __MINGW32__
								MINGW_CharCreateTraceV4(
									"OpenEach #4 BEFORE",
									JobBodySelectNum,
									JobCharacterCount,
									(JobBodySelectNum >= 0 && JobBodySelectNum < JobCharacterCount) ? &JobCharacter[JobBodySelectNum] : NULL,
									(JobBodySelectNum >= 0 && JobBodySelectNum < JobCharacterCount) ? JobCharacter[JobBodySelectNum].lpDinaPattern2 : NULL,
									(JobBodySelectNum >= 0 && JobBodySelectNum < JobCharacterCount && JobCharacter[JobBodySelectNum].lpDinaPattern2) ? JobCharacter[JobBodySelectNum].lpDinaPattern2->Pat : NULL
								);
								#endif
								OpenEachPlayer(&JobCharacter[JobBodySelectNum]);
								#ifdef __MINGW32__
								MINGW_CharCreateTraceV4(
									"OpenEach #4 AFTER",
									JobBodySelectNum,
									JobCharacterCount,
									(JobBodySelectNum >= 0 && JobBodySelectNum < JobCharacterCount) ? &JobCharacter[JobBodySelectNum] : NULL,
									(JobBodySelectNum >= 0 && JobBodySelectNum < JobCharacterCount) ? JobCharacter[JobBodySelectNum].lpDinaPattern2 : NULL,
									NULL
								);
								#endif

							}

							if (JobBodySelectNum != -1)
							{
								if (SelectSubState == SUB_SELECT_NONE)
								{
									hFocusWnd = hTextWnd;
									FocusWndFlag = TRUE;
#ifdef	_LANGUAGE_JAPANESE		
									SetIME_Mode(1);
#endif
								}
							}
						}
					}


					// SUMMIT_FACE_THUMBNAILS_STABLE_V13A
					const bool selectedFace = (JobFaceSelectNum == index);
					const bool hoveredFace = (mouseMoveFace == index);
					const int faceBoxAlpha = selectedFace ? 255 : (hoveredFace ? 180 : 255);
					const int faceImageAlpha = selectedFace ? 255 : (hoveredFace ? 200 : 150);
					const int faceImageVariant = (selectedFace || hoveredFace) ? 1 : 0;

					dsDrawTexImage(
						FaceBoxSmallImage[(selectedFace || hoveredFace) ? 1 : 0],
						GetCurrentFacePositionX(index) + addX,
						SETTINGSHANDLER->getHeight() - (50 + 128),
						128, 128, faceBoxAlpha);

					switch (tribeNum)
					{
					case TRIBE_Tempscron:
						dsDrawTexImage(TempFaceImage[JobBodySelectNum][index][faceImageVariant],
							GetCurrentFacePositionX(index) + addX,
							SETTINGSHANDLER->getHeight() - (50 + 128) + 4,
							128, 128, faceImageAlpha);
						break;
					case TRIBE_Moryon:
						dsDrawTexImage(MorFaceImage[JobBodySelectNum][index][faceImageVariant],
							GetCurrentFacePositionX(index) + addX,
							SETTINGSHANDLER->getHeight() - (50 + 128),
							128, 128, faceImageAlpha);
						break;
					}

				}

				dsDrawTexImage(FaceBoxSmallImage[2], 305 + addX, SETTINGSHANDLER->getHeight() - (66), 64, 16, 255);
				dsDrawTexImage(FaceBoxSmallImage[2], 305 + 128 + addX, SETTINGSHANDLER->getHeight() - (66), 64, 16, 255);

				
			}

			if (selectFaceFlag == false && MouseButton[0] && SelectSubState == SUB_SELECT_NONE &&
				pCursorPos.x > 0 && pCursorPos.x < 800 && pCursorPos.y > 0 && pCursorPos.y < 546)
				JobFaceSelectNum = -1;

			if (JobBodySelectNum != -1 && JobFaceSelectNum != -1 && SelectSubState == SUB_SELECT_NONE &&
				ErrorSelect == ERROR_NONE)
			{

				if (MouseDblClick || (hFocusWnd == NULL && FocusWndFlag))
				{
					FocusWndFlag = FALSE;
					hFocusWnd = NULL;
#ifdef	_LANGUAGE_JAPANESE		
					SetIME_Mode(0);
#endif

					if (strlen(UserCharacterNewName) > 0)
					{
						MessageBox.Flag = TRUE;
						SelectSubState = SUB_SELECT_CHARACTER_NEW;
					}
					else
					{

						MouseDblClick = FALSE;
						MouseButton[0] = FALSE;
						ErrorSelect = ERROR_NONAME_NEWPLAYER;
					}
				}
			}

			HDC hdc = NULL;

			SetBkMode(hdc, TRANSPARENT);
			SetFontTextColor(RGB(255, 255, 255));
			SelectObject(hdc, Font);
#ifdef	_LANGUAGE_VEITNAM
			int newTexPosX = int(((float)SETTINGSHANDLER->getWidth()*358.f) / 800.f + 13);
#else
			int newTexPosX = int(((float)SETTINGSHANDLER->getWidth()*358.f) / 800.f);
#endif
			int newTexPosY = int(((float)SETTINGSHANDLER->getHeight()*398.f) / 600.f);


			dsTextLineOut(newTexPosX, newTexPosY, UserCharacterNewName, strlen(UserCharacterNewName));

			if (JobFaceSelectNum >= 0)
			{

				int nLength = strlen(UserCharacterNewName);

			}


			KeyFilter();


			if ((GetKeyState(VK_CONTROL) < 0))
			{
				if ((GetKeyState(118) < 0) || (GetKeyState(86) < 0))
				{
					for (int i = 0; i <= 255; i++)
					{
						UserCharacterNewName[i] = 0;
						SetWindowText(hFocusWnd, 0);
						SetWindowText(hFocusWnd, UserCharacterNewName);
					}
				}
			}
			if (GetKeyState(VK_SHIFT) < 0)
			{
				if (GetKeyState(VK_INSERT) < 0)
				{
					for (int i = 0; i <= 255; i++)
					{
						UserCharacterNewName[i] = 0;
						SetWindowText(hFocusWnd, 0);
						SetWindowText(hFocusWnd, UserCharacterNewName);
					}
				}
			}

		}
	}


	if (SelectSubState == SUB_SELECT_CHARACTER_NEW)
		MessageBox.Draw(SETTINGSHANDLER->getWidth() / 2 - 150, SETTINGSHANDLER->getHeight() / 2 + SETTINGSHANDLER->getHeight() / 6, HoTextSelectMessage[3]);
	if (ErrorSelect == ERROR_NONAME_NEWPLAYER)
		Message.Draw(SETTINGSHANDLER->getWidth() / 2 - 100, SETTINGSHANDLER->getHeight() / 2 + SETTINGSHANDLER->getHeight() / 6, HoTextSelectMessage[4]);
	if (ErrorSelect == ERROR_EXHIST_NEWPLAYER)
		Message.Draw(SETTINGSHANDLER->getWidth() / 2 - 128, SETTINGSHANDLER->getHeight() / 2 + SETTINGSHANDLER->getHeight() / 6, HoTextSelectMessage[5]);

	if (ErrorSelect == ERROR_SELECT_CHAR)
		Message.Draw(SETTINGSHANDLER->getWidth() / 2 - 100, SETTINGSHANDLER->getHeight() / 2 + SETTINGSHANDLER->getHeight() / 6, HoTextSelectMessage[6]);
	if (ErrorSelect == ERROR_SELECT_RACE)
		Message.Draw(SETTINGSHANDLER->getWidth() / 2 - 100, SETTINGSHANDLER->getHeight() / 2 + SETTINGSHANDLER->getHeight() / 6, HoTextSelectMessage[7]);
	if (ErrorSelect == ERROR_SELECT_JOB)
		Message.Draw(SETTINGSHANDLER->getWidth() / 2 - 100, SETTINGSHANDLER->getHeight() / 2 + SETTINGSHANDLER->getHeight() / 6, HoTextSelectMessage[8]);
	if (ErrorSelect == ERROR_SELECT_FACE)
		Message.Draw(SETTINGSHANDLER->getWidth() / 2 - 100, SETTINGSHANDLER->getHeight() / 2 + SETTINGSHANDLER->getHeight() / 6, HoTextSelectMessage[9]);
	if (ErrorSelect == ERROR_DELETE_CLAN_MEMBER)
		Message.Draw(SETTINGSHANDLER->getWidth() / 2 - 128, SETTINGSHANDLER->getHeight() / 2 + SETTINGSHANDLER->getHeight() / 6, HoTextSelectMessage[10]);
	if (ErrorSelect == ERROR_CLAN_SERVER_NONE)
		Message.Draw(SETTINGSHANDLER->getWidth() / 2 - 128, SETTINGSHANDLER->getHeight() / 2 + SETTINGSHANDLER->getHeight() / 6, HoTextSelectMessage[11]);

	if (MessageBox.Flag)
	{
		
		dsDrawTexImage(BlackBoxImage, 0, SETTINGSHANDLER->getHeight() - 47, SETTINGSHANDLER->getWidth(), SETTINGSHANDLER->getHeight(), 120);
		
	}

	if (SelectCurrentState == COMPLETE_SELECT_STATE || SelectCurrentState == START_GAME_STATE)
		DrawMap(MapBlendCount);

	DrawFade();

	return TRUE;
}

int HoLogin::DrawFade()
{
	if (FadeCurrentState == FADE_IN && FadeNextState == FADE_IN)
		return FALSE;

	bool fastBlendFlag = false;
	for (int index = 0; index < 256; index++)
	{
		if (VRKeyBuff[index] == TRUE)
		{
			fastBlendFlag = true;
			break;
		}
	}

	if (MouseButton[0])
	{
		fastBlendFlag = true;
		MouseButton[0] = FALSE;
	}

	if (fastBlendFlag)
	{
		if (FadeNextState == FADE_IN)
		{
			BlendValue = 0;
			FadeCurrentState = FADE_IN;
			LoginVolume = 350;
		}
		else if (FadeNextState == FADE_OUT)
		{
			BlendValue = 255;
			FadeCurrentState = FADE_OUT;
			LoginVolume = 0;
		}
	}

	if (SelectCurrentState != COMPLETE_SELECT_STATE && SelectCurrentState != START_GAME_STATE)
	{
		SetVolumeBGM(LoginVolume);
		PlayWav_VolChange(9 - 1, LoginVolume);
		PlayWav_VolChange(10 - 1, LoginVolume + 50);
		PlayWav_VolChange(11 - 1, LoginVolume + 50);
	}

	if (BlendValue > 0 && BlendValue <= 255)
	{
		
		dsDrawTexImage(BlackBoxImage, 0, 0, SETTINGSHANDLER->getWidth(), SETTINGSHANDLER->getHeight(), BlendValue);
		
	}

	return TRUE;
}

int HoLogin::MainFade()
{

	if (FadeCurrentState != FadeNextState)
	{
		if (FadeNextState == FADE_IN)
			MainFadeIn();
		else
			MainFadeOut();
	}

	return TRUE;
}

int HoLogin::MainFadeIn()
{

	if (BlendValue > 0)
	{
		BlendValue -= 6;
		if (SelectCurrentState != COMPLETE_SELECT_STATE &&
			SelectCurrentState != START_GAME_STATE)
		{
			LoginVolume += 10;
			if (LoginVolume > 350)
				LoginVolume = 350;
		}
	}

	if (BlendValue <= 0)
	{
		BlendValue = 0;
		FadeCurrentState = FADE_IN;
		FadeNextState = FADE_IN;
	}
	return TRUE;
}

int HoLogin::MainFadeOut()
{
	if (BlendValue < 255)
	{
		BlendValue += 8;

		if (SelectCurrentState != COMPLETE_SELECT_STATE &&
			SelectCurrentState != START_GAME_STATE)
		{
			LoginVolume -= 10;
			if (LoginVolume < 0)
				LoginVolume = 0;
		}
	}

	if (BlendValue >= 255)
	{
		BlendValue = 255;
		LoginVolume = 0;
		FadeCurrentState = FADE_OUT;
		FadeNextState = FADE_OUT;
	}
	return TRUE;
}

void HoLogin::SetFade()
{


	if (FadeCurrentState == FADE_OUT)
	{
		FadeNextState = FADE_IN;
		BlendValue = 255;
		LoginVolume = 0;
	}
	else
	{
		FadeNextState = FADE_OUT;
		BlendValue = 0;
		LoginVolume = 350;
	}
}

int HoLogin::ReLoad()
{
	if (GameMapImage)
	{
		GameMapImage->Release();
		GameMapImage = NULL;
		GameMapImage = LoadDibSurfaceOffscreen("StartImage//Login//Map.bmp");

	}

	return TRUE;
}

void HoLogin::resize()
{
	MidX = SETTINGSHANDLER->getWidth() / 2;
	MidY = SETTINGSHANDLER->getHeight() / 2;
	m_ScreenHalf = SETTINGSHANDLER->getWidth() / 2;

	TarDist = CameraZPos[SelectCurrentState];
	TarPosi.x = 0;
	TarPosi.y = 16 * fONE;
	TarPosi.z = 0;
}

int GetSelRectChar(smCHAR *lpChar)
{
	RECT rect;
	POINT	sPos;
	int sez;
	int SelZ = 65536 * 16384;

	if (lpChar)
	{



		sez = smRender.GetRect2D(lpChar->pX, lpChar->pY + (32 * fONE), lpChar->pZ,
			24 * fONE, 32 * fONE, &rect, &sPos);


		if (sez > 0 && sez < SelZ &&
			rect.left<pCursorPos.x && rect.right>pCursorPos.x &&
			rect.top<pCursorPos.y && rect.bottom>pCursorPos.y)
		{

			if (MouseButton[0])
				return 2;


			return 1;

		}
	}

	return FALSE;
}

int GetSelRectJobChar(smCHAR *lpChar)
{

	RECT rect;
	POINT	sPos;
	int sez;
	int SelZ = 65536 * 16384;

	if (lpChar)
	{

		sez = smRender.GetRect2D(lpChar->pX, lpChar->pY + (32 * fONE), lpChar->pZ,
			24 * fONE, 32 * fONE, &rect, &sPos);
		if (sez > 0 && sez < SelZ &&
			rect.left<pCursorPos.x && rect.right>pCursorPos.x &&
			rect.top<pCursorPos.y && rect.bottom>pCursorPos.y)
		{

			if (MouseButton[0])
				return 2;

			return 1;

		}
	}

	return FALSE;
}




