#include <cstdio>
#include <cstring>
#include "systems/network/protocol/server/opcodes.h"

#include "core/server/legacy_sin_runtime/sinLinkHeader.h"
#include "systems/presentation/opening/legacy_runtime/HoLinkHeader.h"
#include "systems/localization/legacy/ho_text/HoTextFile.h"
#include "systems/rendering/effects/legacy_ho/HoMinMax.h"
#include "systems/rendering/effects/particles/ho/HoNewParticle.h"
#include "systems/rendering/effects/particles/ho/HoNewParticleMgr.h"

#include "systems/social/clan/legacy/tjclanDEF.h"
#include "systems/social/clan/legacy/tjclan.h"
#include "systems/social/chat/filter/CurseFilter.h"		

#include "systems/localization/config/language.h"
#include "compat/legacy/text/jts.h"
#include "authentication/character_selection/ClassSelectionAdapter.h"



int  CharacterPosX = 0;
int  CharacterPosZ = 0;

int  MouseDblClick;

char SelectCharacterName1[3][500];
char SelectCharacterName2[3][500];

char JobSelectCharacterName1[CHARACTER_SELECTION_SLOT_CAPACITY][500];
char JobSelectCharacterName2[CHARACTER_SELECTION_SLOT_CAPACITY][500];


static int UserCharacterPos[5][2] =
{
	{ 0 ,-110 },
	{ -35 ,-90 },
	{ 35 ,-90 },
	{ -70 ,-60 },
	{ 70 ,-60 }
};


static int CharacterInfoPosi[5] = { 362, 243, 488, 154, 588 };

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
	Init_UseMor();



	Load();

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

int HoLogin::Init()
{
	return TRUE;
}


int HoLogin::Load()
{

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
					  "Comic Sans MS");
#endif



	Stage = new smSTAGE3D();


	Stage->VectLight.x = smConfig.MapLightVector.x;
	Stage->VectLight.y = smConfig.MapLightVector.y;
	Stage->VectLight.z = smConfig.MapLightVector.z;

	Stage->Bright = smConfig.MapBright;
	Stage->Contrast = smConfig.MapContrast;

	//Stage = smSTAGE3D_ReadASE("Assets//GUI//Opening//Select//Stage//Stage.ASE", smStage);

	Stage = smSTAGE3D_ReadASE("cSelect//Login.ASE", smStage);

	if(Stage)
	{
		Stage->Head = TRUE;
	}

	ReadTextures();

	if(Stage)
	{
		Stage->smMaterialGroup->CheckMatreialTextureSwap();
	}

	OkButton.Init("Assets//GUI//Opening//Select//UI//CharSelect//Bt_select.tga", 128, 32);
	OkButton.InitCrashRect(10, 0, 110, 32);

	BackButton.Init("Assets//GUI//Opening//Select//UI//CharSelect//Bt_back.tga", 128, 32);
	BackButton.InitCrashRect(20, 0, 103, 32);

	NewCharButton.Init("Assets//GUI//Opening//Select//UI//CharSelect//bt-NewChar.tga", 128, 32);
	NewCharButton.InitCrashRect(10, 0, 110, 32);

	QuitButton.Init("Assets//GUI//Opening//Select//UI//CharSelect//Bt_exit.tga", 128, 32);
	QuitButton.InitCrashRect(15, 0, 108, 32);

	DeleteButton.Init("Assets//GUI//Opening//Select//UI//CharSelect//Bt_delet.tga", 128, 32);
	DeleteButton.InitCrashRect(10, 0, 108, 32);


	UserCharacterInfoBackImage = CreateTextureMaterial("Assets//GUI//Opening//Select//UI//UserCharacterInfo.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	UserCharacterInfoImage = CreateTextureMaterial("Assets//GUI//Opening//Select//UI//CharacterInfo.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);


	TempscronMarkImage = CreateTextureMaterial("Assets//GUI//Opening//Select//UI//CharSelect//mark_T.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	TempscronLogoImage = CreateTextureMaterial("Assets//GUI//Opening//Select//UI//CharSelect//T-tempscron.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);

	MoryonMarkImage = CreateTextureMaterial("Assets//GUI//Opening//Select//UI//CharSelect//mark_M.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	MoryonLogoImage = CreateTextureMaterial("Assets//GUI//Opening//Select//UI//CharSelect//T-Morion.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);

	FaceSelectImage = LoadDibSurfaceOffscreen("Assets//GUI//Opening//Select//UI//SelectFace.bmp");

	MatEachBox = -1;

	BlackBoxImage = CreateTextureMaterial("Assets//GUI//Opening//Select//UI//BlackBox.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);

	SYSTEMTIME st;
	GetLocalTime(&st);

	Message.Init();
	MessageBox.Init();
	InitMessageBox();

	MainBoxFrameImage1[0] = CreateTextureMaterial("Assets//GUI//Opening//Select//UI//CharSelect//mainbox_L.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);
	MainBoxFrameImage1[1] = CreateTextureMaterial("Assets//GUI//Opening//Select//UI//CharSelect//mainbox_T.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);
	MainBoxFrameImage1[2] = CreateTextureMaterial("Assets//GUI//Opening//Select//UI//CharSelect//mainbox_R.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);
	MainBoxFrameImage1[3] = CreateTextureMaterial("Assets//GUI//Opening//Select//UI//CharSelect//mainbox_D.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);

	MainBoxFrameImage2[0] = CreateTextureMaterial("Assets//GUI//Opening//Select//UI//CharSelect//mainbox_CN1.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);
	MainBoxFrameImage2[1] = CreateTextureMaterial("Assets//GUI//Opening//Select//UI//CharSelect//mainbox_CN2.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);
	MainBoxFrameImage2[2] = CreateTextureMaterial("Assets//GUI//Opening//Select//UI//CharSelect//mainbox_CN3.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);
	MainBoxFrameImage2[3] = CreateTextureMaterial("Assets//GUI//Opening//Select//UI//CharSelect//mainbox_CN4.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);

	TitleBoxFrameImage = CreateTextureMaterial("Assets//GUI//Opening//Select//UI//CharSelect//title-box.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);
	TitleNameImage = CreateTextureMaterial("Assets//GUI//Opening//Select//UI//CharSelect//title-txt30.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);

	TempscronTextImage = CreateTextureMaterial("Assets//GUI//Opening//Select//UI//CharSelect//T-ex_tscr.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);
	MoryonTextImage = CreateTextureMaterial("Assets//GUI//Opening//Select//UI//CharSelect//T-ex_mrin.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);

	RaceTextBoxImage[0] = CreateTextureMaterial("Assets//GUI//Opening//Select//UI//CharSelect//mark_box1.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);
	RaceTextBoxImage[1] = CreateTextureMaterial("Assets//GUI//Opening//Select//UI//CharSelect//mark_box2.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);
	
	JobTextBoxImage[0] = CreateTextureMaterial("Assets//GUI//Opening//Select//UI//CharSelect//T_cls-box1.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);
	JobTextBoxImage[1] = CreateTextureMaterial("Assets//GUI//Opening//Select//UI//CharSelect//T_cls-box2.tga", 0, TRUE, FALSE, FALSE, SMMAT_BLEND_ALPHA);

	LoadClassSelectionAssets("Assets//GUI//Opening//Select//UI//", "Assets//GUI//Opening//Select//UI//CharSelect//",
	                         TRIBE_Tempscron, TempscronJobImage, TempscronJobTextImage, TempFaceImage);
	LoadClassSelectionAssets("Assets//GUI//Opening//Select//UI//", "Assets//GUI//Opening//Select//UI//CharSelect//",
	                         TRIBE_Moryon, MoryonJobImage, MoryonJobTextImage, MorFaceImage);
	FaceBoxBigImage[0] = CreateTextureMaterial("Assets//GUI//Opening//Select//UI//CharSelect//face_bigbox1.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	FaceBoxBigImage[1] = CreateTextureMaterial("Assets//GUI//Opening//Select//UI//CharSelect//face_bigbox2.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);

	FaceBoxSmallImage[0] = CreateTextureMaterial("Assets//GUI//Opening//Select//UI//CharSelect//face_box_.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	FaceBoxSmallImage[1] = CreateTextureMaterial("Assets//GUI//Opening//Select//UI//CharSelect//face_box.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	FaceBoxSmallImage[2] = CreateTextureMaterial("Assets//GUI//Opening//Select//UI//CharSelect//face_box2.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);

	NewCharacterImage = CreateTextureMaterial("Assets//GUI//Opening//Select//UI//CharSelect//New_char.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);

	ReadTextures();

	return TRUE;
}


void HoLogin::LoadLoadingImage(int level)
{
	GameMapImage = LoadDibSurfaceOffscreen("Assets//GUI//Opening//Loading//0.bmp");
}

int	HoLogin::AddUserCharacter(char *chrFile1, char *chrFile2, int jobCode)
{
	for(int index = 0; index < CHARACTER_USER_MAX; index++)
	{
		if(UserCharacter[index].DisplayFlag == FALSE)
		{
			UserCharacter[index].Init();
			UserCharacter[index].lpStage = Stage;

			SetLoadPattern(&UserCharacter[index], chrFile1, chrFile2, 0, 0);
			UserCharacter[index].SetPosi(UserCharacterPos[index][0] * fONE,
										 Stage->GetHeight(UserCharacterPos[index][0] * fONE, UserCharacterPos[index][1] * fONE)
										 , UserCharacterPos[index][1] * fONE, 0, ANGLE_180, 0);
			UserCharacter[index].ActionPattern = 0;


			UserCharacter[index].ChangeMotion(10);

			UserCharacterCount++;
			ReadTextures();

			UserCharacter[index].lpDinaPattern->Pat->smMaterialGroup->CheckMatreialTextureSwap();
			
			if(UserCharacter[index].lpDinaPattern2)
			{
				UserCharacter[index].lpDinaPattern2->Pat->smMaterialGroup->CheckMatreialTextureSwap();
			}
			
			break;
		}
	}


	return TRUE;
}

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
	gameplay::classes::CharacterCreationTemplate creation;
	if(!authentication::character_selection::DefaultClassSelectionAdapter().BuildCreation(
		tribeNum, JobBodySelectNum, 0u, static_cast<std::size_t>(faceIndex), false, creation))
		return FALSE;
	return SetChangeJobFace(const_cast<char*>(creation.bodyModel.c_str()),
	                        const_cast<char*>(creation.faceModel.c_str()),
	                        JobBodySelectNum);
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
	if(index == -1)
	{
		JobCharacter[JobCharacterCount].Init();
		JobCharacter[JobCharacterCount].lpStage = Stage;
		JobCharacter[JobCharacterCount].smCharInfo.wPlayerKilling[0] = 0;


		SetLoadPattern(&JobCharacter[JobCharacterCount], chrFile1, chrFile2, 0, 0);

		int classPosX = 0;
		int classPosZ = 0;
		GetJobCharacterWorldPosition(JobCharacterCount, classPosX, classPosZ);
		JobCharacter[JobCharacterCount].SetPosi(classPosX * fONE,
			Stage->GetHeight(classPosX * fONE, classPosZ * fONE),
			classPosZ * fONE, 0, ANGLE_180, 0);
		JobCharacter[JobCharacterCount].ActionPattern = 0;
		memset(JobCharacter[JobCharacterCount].szChatMessage, 0, sizeof(JobCharacter[JobCharacterCount].szChatMessage));
		memset(JobCharacter[JobCharacterCount].smCharInfo.szName, 0, sizeof(JobCharacter[JobCharacterCount].smCharInfo.szName));

		JobCharacter[JobCharacterCount].ChangeMotion(10);

		JobCharacterCount++;
		ReadTextures();
		JobCharacter[JobCharacterCount - 1].lpDinaPattern->Pat->smMaterialGroup->CheckMatreialTextureSwap();
		JobCharacter[JobCharacterCount - 1].lpDinaPattern2->Pat->smMaterialGroup->CheckMatreialTextureSwap();

		JobCharacter[JobCharacterCount - 1].smCharInfo.Stamina[0] = 100;
		JobCharacter[JobCharacterCount - 1].smCharInfo.Stamina[1] = 100;

	}
	else
	{
		if(index >= 0 && index < CHARACTER_SELECTION_SLOT_CAPACITY)
		{
			JobCharacter[index].Init();
			JobCharacter[index].lpStage = Stage;
			JobCharacter[index].smCharInfo.wPlayerKilling[0] = 0;


			SetLoadPattern(&JobCharacter[index], chrFile1, chrFile2, 0, 0);
			int classPosX = 0;
			int classPosZ = 0;
			GetJobCharacterWorldPosition(index, classPosX, classPosZ);
			JobCharacter[index].SetPosi(classPosX * fONE,
				Stage->GetHeight(classPosX * fONE, classPosZ * fONE),
				classPosZ * fONE, 0, ANGLE_180, 0);
			JobCharacter[index].ActionPattern = 0;
			memset(JobCharacter[index].szChatMessage, 0, sizeof(JobCharacter[JobCharacterCount].szChatMessage));
			memset(JobCharacter[index].smCharInfo.szName, 0, sizeof(JobCharacter[JobCharacterCount].smCharInfo.szName));

			ReadTextures();
			JobCharacter[index].lpDinaPattern->Pat->smMaterialGroup->CheckMatreialTextureSwap();
			JobCharacter[index].lpDinaPattern2->Pat->smMaterialGroup->CheckMatreialTextureSwap();

			JobCharacter[index].smCharInfo.Stamina[0] = 100;
			JobCharacter[index].smCharInfo.Stamina[1] = 100;

		}
		else
			return FALSE;
	}


	return TRUE;
}




int HoLogin::GetSelectCharacter(smCHAR *lpChar)
{
	if(lpChar == NULL)
		return FALSE;
	if(lpChar->DisplayFlag == FALSE)
		return FALSE;

	RECT rect;
	POINT	sPos;
	int sez;
	int SelZ = 65536 * 16384;


	if(lpChar)
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


		if(sez > 0 && sez < SelZ &&
		   rect.left<pCursorPos.x && rect.right>pCursorPos.x &&
		   rect.top<pCursorPos.y && rect.bottom>pCursorPos.y)
		{

			if(MouseButton[0])
				return 2;

			return 1;
		}
	}
	return FALSE;
}

int HoLogin::SetChangeJobFace(char *charFile1, char *chrFile2, int jobIndex)
{
	int PosX, PosZ;
	PosX = JobCharacter[jobIndex].pX;
	PosZ = JobCharacter[jobIndex].pZ;
	AddJobCharacter(charFile1, chrFile2, jobIndex);
	JobCharacter[jobIndex].SetPosi(PosX, 10, PosZ, 0, ANGLE_180, 0);
	ReadTextures();
	JobCharacter[jobIndex].lpDinaPattern2->Pat->smMaterialGroup->CheckMatreialTextureSwap();
	return TRUE;
}

int HoLogin::Close()
{

	if(Font != NULL)
		DeleteObject(Font);

	CloseEachPlayer();

	if(Stage)
	{
		delete Stage;
		Stage = NULL;
	}

	for(int index = 0; index < UserCharacterCount; index++)
		UserCharacter[index].Init();

	for(int index = 0; index < JobCharacterCount; index++)
		JobCharacter[index].Init();


	if(GameMapImage)
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
	if(cameraUpDown < 80)
	{
		TarPosi.y += 4;
	}

	if(cameraUpDown >= 80 && cameraUpDown < 160)
	{
		TarPosi.y -= 4;
	}

	cameraUpDown++;
	if(cameraUpDown >= 160)
		cameraUpDown = 0;

	if(SelectCurrentState == CHARACTER_SELECT_STATE)
	{
		if(NewCharacterImageBlendFlag)
		{
			NewCharacterImageBlendCount -= 1.25f;
		}
		else
		{
			NewCharacterImageBlendCount += 1.25f;
		}

		if(NewCharacterImageBlendCount >= 255.f)
		{
			NewCharacterImageBlendFlag = true;
			NewCharacterImageBlendCount = 255.f;

		}


		if(NewCharacterImageBlendCount < 100.f)
		{
			NewCharacterImageBlendFlag = false;
			NewCharacterImageBlendCount = 100.f;
		}

	}

	return TRUE;
}

int HoLogin::Main()
{
	if(ErrorSelect != ERROR_NONE)
	{
		if(StartErrorDelayFlag == FALSE)
		{
			StartErrorDelayFlag = TRUE;
			ErrorDelayCount = 0;
		}
		else
		{
			ErrorDelayCount++;
		}

		if(ErrorDelayCount > 100)
		{
			if(ErrorSelect == ERROR_NONAME_NEWPLAYER)
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

	if(main_chaC() && ClanMarkUpdateSuccessFlag == FALSE)
	{
		for(int m = 0; m < 5; m++)
		{
			if(strlen(chacldata[m].ChaName) >= 1)
			{
				for(int n = 0; n < DownUserCharacterCount; n++)
				{
					if(stricmp(chacldata[m].ChaName, DownUserCharacterInfo[n].Name) == 0 &&
					   DownUserCharacterInfo[n].ClanFlag == FALSE)
					{
						strcpy(DownUserCharacterInfo[n].ClanName, chacldata[m].name);
						DownUserCharacterInfo[n].ClanFlag = TRUE;
						if(chacldata[m].hClanMark16)
							DownUserCharacterInfo[n].ClanMark = chacldata[m].hClanMark16;
					}
				}
			}
		}
		ClanMarkUpdateSuccessFlag = TRUE;
	}

	MainEffect();

	CameraUpDown();

	if(SelectCurrentState == START_GAME_STATE && FadeCurrentState == FADE_IN)
	{
		switch(SelectSubState)
		{
			case SUB_SELECT_CHARACTER_USER:
			{
				if(MouseDownCharacterNum != -1)
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

	if(SelectCurrentState != COMPLETE_SELECT_STATE && SelectCurrentState != START_GAME_STATE)
	{

		for(int index = 0; index < UserCharacterCount; index++)
		{
			if(UserCharacter[index].DisplayFlag)
				UserCharacter[index].Main();
		}

		for(int index = 0; index < JobCharacterCount; index++)
			JobCharacter[index].Main();
	}


	if(SelectCurrentState == SelectNextState)
	{
		if(SelectCurrentState == RACE_SELECT_STATE)
		{

			if(RaceMarkBlendCount < 200)
				RaceMarkBlendCount += 4;
		}
		else if(SelectCurrentState == JOB_SELECT_STATE)
		{


			if(CharacterReturnFlag)
			{
				CharacterStopFlag = FALSE;
				CharacterReturnFlag = FALSE;
				for(int index = 0; index < JobCharacterCount; index++)
				{
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
		else if(SelectCurrentState == FACE_SELECT_STATE)
		{
			if(chrEachMaster != NULL)
				chrEachMaster->Flag = 1;
			EachPlayerMain();
		}
		else if(SelectCurrentState == COMPLETE_SELECT_STATE)
		{
			if(FadeCurrentState == FADE_OUT)
			{
				SelectNextState = START_GAME_STATE;
				SetFade();
			}
			if(MouseButton[0] || MouseDblClick || MapBlendCount >= 255)
				SelectNextState = START_GAME_STATE;
		}
	}


	if(FadeCurrentState == FADE_OUT && FadeNextState == FADE_OUT && SelectNextState == RETURN_OPENING_STATE)
	{
		if(smWsockDataServer != NULL)
		{
			DisconnectServerFull();

		}
		InitUserCharacter();
		return 2;
	}


	if(SelectCurrentState != SelectNextState)
	{
		if(SelectCurrentState != COMPLETE_SELECT_STATE && SelectNextState != COMPLETE_SELECT_STATE &&
		   SelectCurrentState != START_GAME_STATE && SelectNextState != START_GAME_STATE)
		{

			if((SelectCurrentState == RACE_SELECT_STATE) && (SelectNextState == JOB_SELECT_STATE))
				RaceMarkScaleSize--;

			MouseDownCharacterNum = -1;
			MouseMoveCharacterNum = -1;
			RaceMarkBlendCount = 0;


			static int step = 1000;
			if(SelectNextState >= 0 && SelectNextState < COMPLETE_SELECT_STATE &&
			   SelectCurrentState >= 0 && SelectCurrentState < COMPLETE_SELECT_STATE)
			{
				if(SelectNextState > SelectCurrentState)
				{

					TarDist += step;
					if(TarDist > CameraZPos[SelectNextState] + 800)
					{
						TarDist = CameraZPos[SelectNextState];
						SelectCurrentState = SelectNextState;
						step = 1000;
					}

					if((CameraZPos[SelectNextState] - CameraZPos[SelectCurrentState]) / 16 * 15 + CameraZPos[SelectCurrentState] < TarDist)
						step = 100;
				}
				else if(SelectNextState < SelectCurrentState)
				{
					TarDist -= step;
					if(TarDist < CameraZPos[SelectNextState] - 800)
					{
						TarDist = CameraZPos[SelectNextState];
						SelectCurrentState = SelectNextState;
						step = 1000;
					}

					if((CameraZPos[SelectCurrentState] - CameraZPos[SelectNextState]) / 16 + CameraZPos[SelectNextState] > TarDist)
						step = 100;
				}
			}
		}

		if(SelectNextState == COMPLETE_SELECT_STATE)
		{
			if(FadeCurrentState == FadeNextState)
			{
				SelectCurrentState = COMPLETE_SELECT_STATE;
			}
		}

		if(SelectNextState == START_GAME_STATE)
		{
			SelectCurrentState = START_GAME_STATE;
		}

		if(SelectCurrentState == FACE_SELECT_STATE && SelectNextState == JOB_SELECT_STATE)
		{
			JobBodySelectNum = -1;
			JobFaceSelectNum = -1;
			memset(UserCharacterNewName, 0, sizeof(UserCharacterNewName));
			SetWindowText(hTextWnd, 0);
			hFocusWnd = NULL;
		}


		if(SelectCurrentState == JOB_SELECT_STATE && SelectNextState == FACE_SELECT_STATE)
		{
			if(JobFaceSelectNum == -1)
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


	if(SelectNextState == FACE_SELECT_STATE)
	{

		if(JobCharacter[JobBodySelectNum].pZ < -415 * fONE && CharacterStopFlag == FALSE)
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
	if(SelectSubState != SUB_SELECT_NONE || ErrorSelect != ERROR_NONE)
		return FALSE;

	if(SelectCurrentState == SelectNextState && SelectCurrentState == CHARACTER_SELECT_STATE)
	{
		if(MouseDownCharacterNum != -1)
		{

			if(DeleteButton.Main(pCursorPos.x, pCursorPos.y, MouseButton[0]))
			{
				SelectSubState = SUB_SELECT_CHARACTER_MANAGE;
				MessageBox.Flag = TRUE;
				PlayWav2(4 - 1);


			}
		}

		int userCount = 0;
		for(int index = 0; index < DownUserCharacterCount; index++)
		{
			if(UserCharacter[index].DisplayFlag)
				userCount++;
		}

		if(userCount == 0)
		{
			if(NewCharButton.Main(pCursorPos.x, pCursorPos.y, MouseButton[0]))
			{
				if(userCount == 0)
				{
					SelectNextState = RACE_SELECT_STATE;
					PlayWav2(9 - 1);
				}
			}
		} 
	}


	if(OkButton.Main(pCursorPos.x, pCursorPos.y, MouseButton[0]))
	{
		PlayWav(4 - 1);

		if(SelectCurrentState == CHARACTER_SELECT_STATE)
		{
			if(MouseDownCharacterNum != -1)
			{
				MessageBox.Flag = TRUE;
				SelectSubState = SUB_SELECT_CHARACTER_USER;
			}

			if(MouseDownCharacterNum == -1)
				ErrorSelect = ERROR_SELECT_CHAR;
		}
		else if(SelectCurrentState == RACE_SELECT_STATE)
		{
			ErrorSelect = ERROR_SELECT_RACE;
		}
		else if(SelectCurrentState == JOB_SELECT_STATE)
		{
			if(MouseDownCharacterNum != -1)
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
				OpenEachPlayer(&JobCharacter[JobBodySelectNum]);
				MouseDownCharacterNum = -1;
				CharacterReturnFlag = TRUE;
			}

			if(JobBodySelectNum == -1)
				ErrorSelect = ERROR_SELECT_JOB;

		}
		else if(SelectCurrentState == FACE_SELECT_STATE)
		{

			if(JobBodySelectNum != -1 && JobFaceSelectNum != -1)
			{
				hFocusWnd = NULL;
				if(strlen(UserCharacterNewName) > 0)
				{
					MessageBox.Flag = TRUE;
					SelectSubState = SUB_SELECT_CHARACTER_NEW;
				}
				else
					ErrorSelect = ERROR_NONAME_NEWPLAYER;
			}

			if(JobFaceSelectNum == -1)
			{ 
				hFocusWnd = NULL;
				ErrorSelect = ERROR_SELECT_FACE;
			}
		}

	}


	if(QuitButton.Main(pCursorPos.x, pCursorPos.y, MouseButton[0]))
	{ 
		PlayWav(4 - 1);
		SelectSubState = SUB_SELECT_BACK;
	}

	if(BackButton.Main(pCursorPos.x, pCursorPos.y, MouseButton[0]) && SelectCurrentState != CHARACTER_SELECT_STATE)
	{
		PlayWav(7 - 1);
		SelectNextState = SelectCurrentState - 1;
	}

	return TRUE;
}

int HoLogin::InitUserCharacter()
{
	for(int index = 0; index < CHARACTER_USER_MAX; index++)
	{
		if(UserCharacter[index].DisplayFlag != FALSE)
			UserCharacter[index].Init();
	}
	UserCharacterCount = 0;

	return TRUE;
}

int HoLogin::DrawJobCharacter(int x, int y, int z, int ax, int ay, int az)
{
	return TRUE;
}

int HoLogin::DrawUserCharacter(int x, int y, int z, int ax, int ay, int az){
	return TRUE;
}

int HoLogin::DrawTitle(int state){
	return TRUE;
}

int HoLogin::DrawGameFrame(){
	return TRUE;
}

int HoLogin::DrawLight(){
	return TRUE;
}

int HoLogin::DrawMap(int alpha)
{

	return TRUE;
}

int HoLogin::DrawCharacterInfo()
{
	return TRUE;
}

int HoLogin::DrawSelectSubState()
{
	if(SelectNextState == COMPLETE_SELECT_STATE || SelectNextState == START_GAME_STATE)
		return FALSE;

	int flag = 0;
	switch(SelectSubState)
	{
		case SUB_SELECT_CHARACTER_MANAGE:
		{
			flag = MessageBox.Main(pCursorPos.x, pCursorPos.y, MouseButton[0]);
			if(flag == 1)
			{
				PlayWav2(4 - 1);

				int isDELETE_character(char *charName);
				int chk = isDELETE_character(DownUserCharacterInfo[MouseDownCharacterNum].Name);

				if(chk == 102 || chk == 1)
				{
					PlayWav2(6 - 1);
					UserCharacter[MouseDownCharacterNum].Init();
					UserCharacterCount--;


					if(smWsockDataServer)
					{
						TransUserCommand(OpCode::OPCODE_DELRECORDDATA, UserAccount, DownUserCharacterInfo[MouseDownCharacterNum].Name);
					}
					MouseDownCharacterNum = -1;
					SelectSubState = SUB_SELECT_NONE;
				}
				else if(chk == 101 || chk == 100)
				{
					ErrorSelect = ERROR_DELETE_CLAN_MEMBER;
					SelectSubState = SUB_SELECT_NONE;
				}
				else if(chk == 103)
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
			else if(flag == 2)
			{
				PlayWav2(4 - 1);
				SelectSubState = SUB_SELECT_NONE;
			}

			break;
		}

		case SUB_SELECT_CHARACTER_USER:
		{
			flag = MessageBox.Main(pCursorPos.x, pCursorPos.y, MouseButton[0]);
			if(flag == 1 || VRKeyBuff[VK_RETURN])
			{
				PlayWav2(12 - 1);
				SelectNextState = COMPLETE_SELECT_STATE;
				MessageBox.Flag = FALSE;

				LoadLoadingImage(DownUserCharacterInfo[MouseDownCharacterNum].Level);
				SetFade();
			}
			else if(flag == 2)
			{
				PlayWav2(4 - 1);
				SelectSubState = SUB_SELECT_NONE;
				MessageBox.Flag = FALSE;
			}
			break;
		}

		case SUB_SELECT_CHARACTER_NEW:
		{
			flag = MessageBox.Main(pCursorPos.x, pCursorPos.y, MouseButton[0]);
			if(flag == 1 || VRKeyBuff[VK_RETURN])
			{
				PlayWav2(12 - 1);
				ServerReturn = NULL;
				TransUserCommand(OpCode::OPCODE_INSRECORDDATA, UserAccount, UserCharacterNewName);
				SelectSubState = SUB_SELECT_CHARACTER_NEW_SERVER;
				LoadLoadingImage(0);
			}
			else if(flag == 2)
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
			if(ServerReturn != NULL)
			{
				if(ServerReturn->WParam == 1)
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

int HoLogin::KeyFilter(){
	return TRUE;
}

int HoLogin::Draw(){
	return TRUE;
}

int HoLogin::DrawFade(){
	return TRUE;
}

int HoLogin::MainFade()
{

	if(FadeCurrentState != FadeNextState)
	{
		if(FadeNextState == FADE_IN)
			MainFadeIn();
		else
			MainFadeOut();
	}

	return TRUE;
}

int HoLogin::MainFadeIn()
{

	if(BlendValue > 0)
	{
		BlendValue -= 6;
		if(SelectCurrentState != COMPLETE_SELECT_STATE &&
		   SelectCurrentState != START_GAME_STATE)
		{
			LoginVolume += 10;
			if(LoginVolume > 350)
				LoginVolume = 350;
		}
	}

	if(BlendValue <= 0)
	{
		BlendValue = 0;
		FadeCurrentState = FADE_IN;
		FadeNextState = FADE_IN;
	}
	return TRUE;
}

int HoLogin::MainFadeOut()
{
	if(BlendValue < 255)
	{
		BlendValue += 8;

		if(SelectCurrentState != COMPLETE_SELECT_STATE &&
		   SelectCurrentState != START_GAME_STATE)
		{
			LoginVolume -= 10;
			if(LoginVolume < 0)
				LoginVolume = 0;
		}
	}

	if(BlendValue >= 255)
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


	if(FadeCurrentState == FADE_OUT)
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
	if(GameMapImage)
	{
		GameMapImage->Release();
		GameMapImage = NULL;
		GameMapImage = LoadDibSurfaceOffscreen("Assets//GUI//Opening//Loading//0.bmp");
	}

	return TRUE;
}

void HoLogin::resize()
{

}

int GetSelRectChar(smCHAR *lpChar)
{
	RECT rect;
	POINT	sPos;
	int sez;
	int SelZ = 65536 * 16384;

	if(lpChar)
	{



		sez = smRender.GetRect2D(lpChar->pX, lpChar->pY + (32 * fONE), lpChar->pZ,
								 24 * fONE, 32 * fONE, &rect, &sPos);


		if(sez > 0 && sez < SelZ &&
		   rect.left<pCursorPos.x && rect.right>pCursorPos.x &&
		   rect.top<pCursorPos.y && rect.bottom>pCursorPos.y)
		{

			if(MouseButton[0])
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

	if(lpChar)
	{

		sez = smRender.GetRect2D(lpChar->pX, lpChar->pY + (32 * fONE), lpChar->pZ,
								 24 * fONE, 32 * fONE, &rect, &sPos);
		if(sez > 0 && sez < SelZ &&
		   rect.left<pCursorPos.x && rect.right>pCursorPos.x &&
		   rect.top<pCursorPos.y && rect.bottom>pCursorPos.y)
		{

			if(MouseButton[0])
				return 2;

			return 1;

		}
	}

	return FALSE;
}


