#include "../opcodes.h"

#include "sinLinkHeader.h"
#include "..\\field.h"
#include "..\\AreaServer.h"
#include "ConfirmationBox\ConfirmationBox.h"
#include "cRestaureWindow.h"
#include <string>
#include "..\\WarMode.h"

using namespace std;

cHaPremiumItem  chaPremiumitem;


int TelePort_FieldNum[TELEPORT_FIELDMAX][3] = {
	// EXPLICANDO A PRIMEIRA LINHA, { 0, 20, 0 }, 0 = Posição na lista do Core Mágico, 20 = Field 20, 0 = Level para entrar.
	{ 0, 20, 0 }, // Estrada dos Ventos
	{ 1, 19, 0 }, // Vale Tranquilo
	{ 2, 17, 0 }, // Floresta dos Espíritos
	{ 3, 0, 0 }, // Mata das Acácias
	{ 4, 2, 0 }, // Jardim da Liberdade
	{ 5, 4, 0 }, // Refugio dos Ancioes
	{ 6, 5, 0 }, // Castelo dos Perdidos
	{ 7, 7, 0 },  // Terra Maldita
	{ 8, 8, 0 }, // Terra Esquecida
	{ 9, 10, 0 }, // Oasis
	{ 10, 11, 55 }, // Campo de Batalha
	{ 11, 12, 55 }, // Terra Proibida
	{ 12, 25, 55 }, // Caverna das Abelhas
	{ 13, 24, 55 }, // Caverna dos Cogumelos
	{ 14, 26, 65 }, // Santuário Sombrio 
	{ 15, 13, 40 }, // Calabouço Antigo 1º Andar 
	{ 16, 14, 50 }, // Calabouço Antigo 2º Andar
	{ 17, 15, 60 }, // Calabouço Antigo 3º Andar
	{ 18, 22, 70 }, // Templo Maldito 1º Andar 
	{ 19, 23, 75 }, // Templo Maldito 2º Andar 
	{ 20, 42, 80 }, // Templo Maldito 3º Andar
	{ 21, 34, 70 }, // Lago da Ganância
	{ 22, 27, 80 }, // Estrada de Ferro do Caos
	{ 23, 28, 90 }, // Coração de Perum
	{ 24, 29, 70 }, // Eura
	{ 25, 31, 70 }, // Vale Gallubia
	{ 26, 35, 90 }, // Santuário Congelado
	{ 27, 36,90},   // Covil de Kelvezu 
	{ 28, 37, 95 }, // Ilha Perdida
	{ 29, 38, 100 }, // Templo Perdido
	{ 30, 40, 105 }, // Torre Sem Fim 1o Andar
	{ 31, 41, 110 }, // Torre Sem Fim 2º Andar 
	{ 32, 43, 121 }, // Torre Sem Fim 3º Andar
	{ 33, 44, 125 }, // Mina de Gelo 1º Andar 
	{ 34, 46, 132 }, // Laboratório Secreto
	{ 35, 47, 132 }, // Arma Secreta 
	{ 36, 48, 132 }, // Abismo do Mar 1º Andar
	{ 37, 49, 132 }, // Abismo do Mar 2º andar
	/*{ 38, 51, 132 }, // Ilha da Morte 3
	{ 39, 52, 132 }, // Lar do Eragon
	{ 42, 53, 132 }, // Floresta Misteriosa 3º
	{ 41, 54, 132 }, // Floresta Misteriosa 2º
	{ 40, 55, 132 }, // Floresta Misteriosa 1º
	{ 43, 56, 132 }, // Atlantis
	{ 44, 57, 132 }, // Calabouço Perdido 1º
	{ 45, 58, 132 }, // Calabouço Perdido 2º
	{ 46, 59, 132 }, // Calabouço Perdido 3º
	{ 47, 60, 132 }, // Templo Esquecido da Morte 1º
	{ 48, 61, 132 }, // Templo Esquecido da Morte 2º
	{ 49, 62, 132 }, // Arena de Batalha
	{ 50, 63, 132 }, // Nucleo de Ferro
	{ 51, 64, 132 }  // Coração de Fogo*/
};

char *haTeleport_Field[4] = {
	"image\\Sinimage\\help\\haTeleport_Field[1].sin",
	"image\\Sinimage\\help\\haTeleport_Field[2].sin",
	"image\\Sinimage\\help\\haTeleport_Field[3].sin",
	"image\\Sinimage\\help\\haTeleport_Field[4].sin",
};

cHaPremiumItem::cHaPremiumItem()
{
	TelePortCore.PageButtonFlag[0] = 0;
	TelePortCore.PageButtonFlag[1] = 0;
	TelePortCore.FieldIndex[0] = 0;
	TelePortCore.FieldIndex[1] = 0;
	TelePortCore.PageIndex = 0;
	Using_ItemKind = 0;
	Using_SelectInvenItemIndex = 0;
	int i;
	for(i = 0; i < SINUPKEEPITEM_MAX; i++)
	{
		UpKeepItem[i].PosiX = 0;
		UpKeepItem[i].PosiY = 0;
		UpKeepItem[i].Visible = false;
		UpKeepItem[i].IconTime[0] = 0;
		UpKeepItem[i].IconTime[1] = 0;
		UpKeepItem[i].Alpha = 255;
		UpKeepItem[i].Alphakind = 0;
		UpKeepItem[i].Per = 0;
		memset(UpKeepItem[i].Id, NULL, sizeof(UpKeepItem[i].Id));
	}

	UseClickPremiumItemFlag = false;


	m_ThirdEyesTime = 0;
	m_ExpUpPotionTime = 0;
	m_VampiricCuspidTime = 0;
	m_ManaRechargingPTime = 0;
	m_ManaRechargingPTime = 0;
	m_ManaReducePotiontime = 0;
	m_ManaReducePotionValue = 0;
	m_MightofAwellTime = 0;
	m_MightofAwellWeight = 0;
	m_PhenixPetTime = 0;
	m_HelpPetTime = 0;
	m_VampiricCuspidEXTime = 0;
	m_StaminaReducePotiontime = 0;
	m_StaminaReducePotionValue = 0;
	m_StaminaRechargingPTime = 0;

	m_HPBoosterPTime = 0;
	m_MPBoosterPTime = 0;
	m_RESBoosterPTime = 0;


	m_BigHead1 = 0;
	m_BigHead2 = 0;
	m_BigHead3 = 0;
	m_BigHead4 = 0;


	iBigHeadLeftTime = 0;
	iBigHeadCounter = 0;
	iBigHeadType = 0;
}

cHaPremiumItem::~cHaPremiumItem()
{







}


void cHaPremiumItem::init()
{

	UpKeepItemTGAImage[0] = CreateTextureMaterial("image\\Sinimage\\Premium\\ThirdEyes.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	UpKeepItemTGAImage[1] = CreateTextureMaterial("image\\Sinimage\\Premium\\ExpUpPotion.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	UpKeepItemTGAImage[2] = CreateTextureMaterial("image\\Sinimage\\Premium\\Vampiric.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	UpKeepItemTGAImage[3] = CreateTextureMaterial("image\\Sinimage\\Premium\\M_Rechar.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);

	UpKeepItemTGAImage[4] = CreateTextureMaterial("image\\Sinimage\\Premium\\M_Might.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	UpKeepItemTGAImage[5] = CreateTextureMaterial("image\\Sinimage\\Premium\\M_Reduce.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	UpKeepItemTGAImage[6] = CreateTextureMaterial("image\\Sinimage\\Premium\\M_Phoenix.TGA", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	UpKeepItemTGAImage[7] = CreateTextureMaterial("image\\Sinimage\\Premium\\Pet_terry-pose.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	UpKeepItemTGAImage[8] = CreateTextureMaterial("image\\Sinimage\\Premium\\Pet_nepsys-pose.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	UpKeepItemTGAImage[9] = CreateTextureMaterial("image\\Sinimage\\Premium\\Pet_io-pose.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	UpKeepItemTGAImage[10] = CreateTextureMaterial("image\\Sinimage\\Premium\\Pet-mute-pose.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	UpKeepItemTGAImage[11] = CreateTextureMaterial("image\\Sinimage\\Premium\\Vampiric_EX.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	UpKeepItemTGAImage[12] = CreateTextureMaterial("image\\Sinimage\\Premium\\S_Reduce.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);

	UpKeepItemTGAImage[13] = CreateTextureMaterial("image\\Sinimage\\Premium\\BigHead.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	UpKeepItemTGAImage[14] = CreateTextureMaterial("image\\Sinimage\\Premium\\BigHead.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	UpKeepItemTGAImage[15] = CreateTextureMaterial("image\\Sinimage\\Premium\\BigHead.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	UpKeepItemTGAImage[16] = CreateTextureMaterial("image\\Sinimage\\Premium\\BigHead.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);

	UpKeepItemTGAImage[17] = CreateTextureMaterial("image\\Sinimage\\Premium\\S_Rechar.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	UpKeepItemTGAImage[18] = CreateTextureMaterial("image\\Sinimage\\Premium\\Caravan.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	UpKeepItemTGAImage[19] = CreateTextureMaterial("image\\Sinimage\\Premium\\GoldLooter.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);

	UpKeepItemTGAImage[20] = CreateTextureMaterial("image\\Sinimage\\Premium\\HPBooster.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	UpKeepItemTGAImage[21] = CreateTextureMaterial("image\\Sinimage\\Premium\\MPBooster.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	UpKeepItemTGAImage[22] = CreateTextureMaterial("image\\Sinimage\\Premium\\RESBooster.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);

	lpUpKeepItemBackImage[0] = LoadDibSurfaceOffscreen("image\\Sinimage\\Premium\\ThirdEyes_Back.bmp");
	lpUpKeepItemBackImage[1] = LoadDibSurfaceOffscreen("image\\Sinimage\\Premium\\ExpUpPotion_Back.bmp");
	lpUpKeepItemBackImage[2] = LoadDibSurfaceOffscreen("image\\Sinimage\\Premium\\Vampiric_back.bmp");
	lpUpKeepItemBackImage[3] = LoadDibSurfaceOffscreen("image\\Sinimage\\Premium\\M_Rechar_back.bmp");

	lpUpKeepItemBackImage[4] = LoadDibSurfaceOffscreen("image\\Sinimage\\Premium\\M_Might_back.bmp");
	lpUpKeepItemBackImage[5] = LoadDibSurfaceOffscreen("image\\Sinimage\\Premium\\M_Reduce_back.bmp");
	lpUpKeepItemBackImage[6] = LoadDibSurfaceOffscreen("image\\Sinimage\\Premium\\M_Phoenix_back.BMP");
	lpUpKeepItemBackImage[7] = LoadDibSurfaceOffscreen("image\\Sinimage\\Premium\\M_Phoenix_back.BMP");
	lpUpKeepItemBackImage[8] = LoadDibSurfaceOffscreen("image\\Sinimage\\Premium\\M_Phoenix_back.BMP");
	lpUpKeepItemBackImage[9] = LoadDibSurfaceOffscreen("image\\Sinimage\\Premium\\M_Phoenix_back.BMP");
	lpUpKeepItemBackImage[10] = LoadDibSurfaceOffscreen("image\\Sinimage\\Premium\\M_Phoenix_back.BMP");
	lpUpKeepItemBackImage[11] = LoadDibSurfaceOffscreen("image\\Sinimage\\Premium\\Vampiric_back_EX.bmp");
	lpUpKeepItemBackImage[12] = LoadDibSurfaceOffscreen("image\\Sinimage\\Premium\\S_Reduce_back.bmp");

	lpUpKeepItemBackImage[13] = LoadDibSurfaceOffscreen("image\\Sinimage\\Premium\\BigHead_Back.bmp");
	lpUpKeepItemBackImage[14] = LoadDibSurfaceOffscreen("image\\Sinimage\\Premium\\BigHead_Back.bmp");
	lpUpKeepItemBackImage[15] = LoadDibSurfaceOffscreen("image\\Sinimage\\Premium\\BigHead_Back.bmp");
	lpUpKeepItemBackImage[16] = LoadDibSurfaceOffscreen("image\\Sinimage\\Premium\\BigHead_Back.bmp");
    //lpUpKeepItemBackImage[17] = LoadDibSurfaceOffscreen("image\\Sinimage\\Premium\\S_Rechar_back.bmp");
	lpUpKeepItemBackImage[18] = LoadDibSurfaceOffscreen("image\\Sinimage\\Premium\\Caravan_.bmp");

	lpUpKeepItemBackImage[19] = LoadDibSurfaceOffscreen("image\\Sinimage\\Premium\\GoldLooter_.bmp");

	lpUpKeepItemBackImage[20] = LoadDibSurfaceOffscreen("image\\Sinimage\\Premium\\HPBooster_.png");
	lpUpKeepItemBackImage[21] = LoadDibSurfaceOffscreen("image\\Sinimage\\Premium\\MPBooster_.png");
	lpUpKeepItemBackImage[22] = LoadDibSurfaceOffscreen("image\\Sinimage\\Premium\\RESBooster_.png");

	haItemUseDelayFlag = false;
}

//Posiçao Premiuns
int ItemUseLeftPosi[SINUPKEEPITEM_MAX][2] = {
	{ 8,80 },{ 8,116 },{ 8,152 },{ 8,188 },{ 8,224 },
    { 48,80 },{ 48, 116 }, {48, 152}, {48, 188}, {48, 224},
    {88, 80}, {88, 116}, {88, 152}, {88, 188}, {88, 224}
};

DWORD dwDelayTime = 0;
DWORD dwUpdateDelayTime = 0;
extern int iSecondsInCity;

DWORD dwTimeSave = -1;

void cHaPremiumItem::Main()
{

	if(chaPremiumitem.haItemUseDelayFlag == true)
	{
		dwDelayTime++;
		if(dwDelayTime > 210)
		{
			chaPremiumitem.haItemUseDelayFlag = false;
			dwDelayTime = 0;
		}
	}

	if (chaPremiumitem.bUpdateTimeDelayFlag == true)
	{
		dwUpdateDelayTime++;
		if (dwUpdateDelayTime > 70)
		{
			chaPremiumitem.bUpdateTimeDelayFlag = false;
			dwUpdateDelayTime = 0;
		}
	}


	int i, cnt = 0;
	UseClickPremiumItemFlag = false;
	for(i = 0; i < SINUPKEEPITEM_MAX; i++){

		if(UpKeepItem[i].IconTime[0] < UpKeepItem[i].IconTime[1]){
			if(UpKeepItem[i].Visible == true){
				if(UpKeepItem[i].IconTime[1] - UpKeepItem[i].IconTime[0] <= 0){
					UpKeepItem[i].Visible = false;
					CheckUpKeepItem();
				}
				cnt++;

				DWORD NowTime = GetPlayTime_T();

				/*switch(UpKeepItem[i].TGAImageNumber)
				{
				case nsPremiumItem::THIRD_EYES:
					if(m_ThirdEyesTime > 0 && UpKeepItem[i].IconTime[0] <= 1)
					{
						UpKeepItem[i].IconTime[0] = NowTime;
					}
					break;
				case nsPremiumItem::EXPUP_POTION:
					if(m_ExpUpPotionTime > 0 && UpKeepItem[i].IconTime[0] <= 1)
					{
						UpKeepItem[i].IconTime[0] = NowTime;
					}
					break;
				case nsPremiumItem::VAMPIRIC_CUS:
					if(m_VampiricCuspidTime > 0 && UpKeepItem[i].IconTime[0] <= 1)
					{
						UpKeepItem[i].IconTime[0] = NowTime;
					}
					break;


				case nsPremiumItem::VAMPIRIC_CUS_EX:
					if(m_VampiricCuspidEXTime > 0 && UpKeepItem[i].IconTime[0] <= 1)
					{
						UpKeepItem[i].IconTime[0] = NowTime;
					}
					break;


				case nsPremiumItem::MANA_RECHAR_P:
					if(m_ManaRechargingPTime > 0 && UpKeepItem[i].IconTime[0] <= 1)
					{
						UpKeepItem[i].IconTime[0] = NowTime;
					}
					break;
				case nsPremiumItem::MIGHT_OF_AWELL:
					if(m_MightofAwellTime > 0 && UpKeepItem[i].IconTime[0] <= 1)
					{
						UpKeepItem[i].IconTime[0] = NowTime;
					}
					break;
				case nsPremiumItem::MANA_REDUCE_P:
					if(m_ManaReducePotiontime > 0 && UpKeepItem[i].IconTime[0] <= 1)
					{
						UpKeepItem[i].IconTime[0] = NowTime;
					}
					break;
				case nsPremiumItem::PHENIX_PET:
					if(m_PhenixPetTime > 0 && UpKeepItem[i].IconTime[0] <= 1)
					{
						UpKeepItem[i].IconTime[0] = NowTime;
					}
					break;
				case nsPremiumItem::HELP_PET_TERRY:
					if(m_HelpPetTime > 0 && UpKeepItem[i].IconTime[0] <= 1)
					{
						UpKeepItem[i].IconTime[0] = NowTime;
					}
				break;
				case nsPremiumItem::HELP_PET_NEPSYS:
					if(m_HelpPetTime > 0 && UpKeepItem[i].IconTime[0] <= 1)
					{
						UpKeepItem[i].IconTime[0] = NowTime;
					}
					break;
				case nsPremiumItem::HELP_PET_IO:
					if(m_HelpPetTime > 0 && UpKeepItem[i].IconTime[0] <= 1)
					{
						UpKeepItem[i].IconTime[0] = NowTime;
					}
					break;
				case nsPremiumItem::HELP_PET_MUTE:
					if(m_HelpPetTime > 0 && UpKeepItem[i].IconTime[0] <= 1)
					{
						UpKeepItem[i].IconTime[0] = NowTime;
					}
					break;
				case nsPremiumItem::STAMINA_REDUCE_P:
					if(m_StaminaReducePotiontime > 0 && UpKeepItem[i].IconTime[0] <= 1)
					{
						UpKeepItem[i].IconTime[0] = NowTime;
					}
					break;
				case nsPremiumItem::BIGHEAD1:
					if(m_BigHead1 > 0 && UpKeepItem[i].IconTime[0] <= 1)
						UpKeepItem[i].IconTime[0] = NowTime;
					break;*/
			  //  }
				
			}
		}
		else
			UpKeepItem[i].Visible = false;

		if(UpKeepItem[i].Visible == true)
		{

            if (cnt > 7 && cnt < 15)
            {
                UpKeepItem[i].PosiX = WinSizeX - 88;
                UpKeepItem[i].PosiY = WinSizeY / 2 - 145 + (36 * (cnt - 8));
            }
            else if (cnt > 14)
            {
                UpKeepItem[i].PosiX = WinSizeX - 126;
                UpKeepItem[i].PosiY = WinSizeY / 2 - 145 + (36 * (cnt - 15));
            }
            else
            {
                UpKeepItem[i].PosiX = WinSizeX - 50;
                UpKeepItem[i].PosiY = WinSizeY / 2 - 145 + (36 * (cnt - 1));
            }

            int PosX = 0;
            int PosY = 0;
            if (cnt > 7 && cnt < 15)
            {
                PosX = (WinSizeX - 88);
                PosY = WinSizeY / 2 - 145 + (36 * (cnt - 8));
            }
            else if (cnt > 14)
            {
                PosX = (WinSizeX - 126);
                PosY = WinSizeY / 2 - 145 + (36 * (cnt - 15));
            }
            else
            {
                PosX = (WinSizeX - 50);
                PosY = WinSizeY / 2 - 145 + (36 * (cnt - 1));
            }
			if(pRealCursorPos.x > UpKeepItem[i].PosiX && pRealCursorPos.x < UpKeepItem[i].PosiX + 30 &&
                pRealCursorPos.y > UpKeepItem[i].PosiY && pRealCursorPos.y < UpKeepItem[i].PosiY + 30){
				UpKeepItem[i].Infoflag = true;
				UseClickPremiumItemFlag = true;
			}
			else{
				UpKeepItem[i].Infoflag = false;
				UseClickPremiumItemFlag = false;
			}
		}
	}


	for(int i = 0; i < SINUPKEEPITEM_MAX; i++)
	{
		if(UpKeepItem[i].Visible == true)
		{
			switch(UpKeepItem[i].TGAImageNumber)
			{
			case nsPremiumItem::BIGHEAD1:
				SetBigHeadTime(UpKeepItem[i].IconTime[1] - UpKeepItem[i].IconTime[0]);
				break;
			}
		}
	}


	switch(Using_ItemKind){        
	case 8:
		if(pCursorPos.x > 100 && pCursorPos.x < 100 + 20 &&
			pCursorPos.y > 365 && pCursorPos.y < 365 + 20)
			TelePortCore.PageButtonFlag[0] = 1;
		else if(pCursorPos.x > 330 && pCursorPos.x < 330 + 20 &&
			pCursorPos.y > 365 && pCursorPos.y < 365 + 20)
			TelePortCore.PageButtonFlag[1] = 1;
		else{
			TelePortCore.PageButtonFlag[0] = 0;
			TelePortCore.PageButtonFlag[1] = 0;
		}
		break;
	}

	extern void ResetHairModel();
	if(MouseButton[1] && cShop.OpenFlag == FALSE && cWareHouse.OpenFlag == FALSE &&
		cCraftItem.OpenFlag == FALSE && cMixtureReset.OpenFlag == FALSE && cSkinChange.OpenFlag == FALSE && cAging.OpenFlag == FALSE &&
		cTrade.OpenFlag == FALSE && ManufactureItem.m_OpenFlag == FALSE && SmeltingItem.OpenFlag == FALSE)
	{
		for(i = 0; i < SINUPKEEPITEM_MAX; i++)
		{
			if(UpKeepItem[i].Visible == true)
			{
				if(UpKeepItem[i].Infoflag)
				{
					_ConfirmationBox.SetTitle(UpKeepItem[i].Id);
					_ConfirmationBox.SetText("Deseja cancelar", "este item premium?");
					_ConfirmationBox.SetCallback([i](bool isChecked)
					{
						if(isChecked)
						{
							chaPremiumitem.SetItemPremiumTime(chaPremiumitem.UpKeepItem[i].TGAImageNumber, NULL);
							switch(chaPremiumitem.UpKeepItem[i].TGAImageNumber)
							{
							case nsPremiumItem::BIGHEAD1:
							case nsPremiumItem::BIGHEAD2:
							case nsPremiumItem::BIGHEAD3:
							case nsPremiumItem::BIGHEAD4:
								chaPremiumitem.SetBigHead1Time(0);
								chaPremiumitem.SetBigHead2Time(0);
								chaPremiumitem.SetBigHead3Time(0);
								chaPremiumitem.SetBigHead4Time(0);
								chaPremiumitem.SetBigHeadTime(0);
								chaPremiumitem.SetBigHeadType(0);
								ResetHairModel();
								cInvenTory.SetItemToChar();
								break;
							case nsPremiumItem::EXPUP_POTION: chaPremiumitem.m_ExpUpPotionTime = 0; break;
							case nsPremiumItem::HELP_PET_IO:
							case nsPremiumItem::HELP_PET_MUTE:
							case nsPremiumItem::HELP_PET_NEPSYS:
							case nsPremiumItem::HELP_PET_TERRY: chaPremiumitem.m_HelpPetTime = 0; break;
							case nsPremiumItem::MANA_RECHAR_P: chaPremiumitem.m_ManaRechargingPTime = 0; break;
							case nsPremiumItem::STAMINA_RECHAR_P: chaPremiumitem.m_StaminaRechargingPTime = 0; break;
							case nsPremiumItem::MANA_REDUCE_P: chaPremiumitem.m_ManaReducePotiontime = 0; break;
							case nsPremiumItem::MIGHT_OF_AWELL: chaPremiumitem.m_MightofAwellTime = 0; break;
							case nsPremiumItem::PHENIX_PET: chaPremiumitem.m_PhenixPetTime = 0; break;
							case nsPremiumItem::STAMINA_REDUCE_P: chaPremiumitem.m_StaminaReducePotiontime = 0; break;
							case nsPremiumItem::THIRD_EYES: chaPremiumitem.m_ThirdEyesTime = 0; break;
							case nsPremiumItem::VAMPIRIC_CUS: chaPremiumitem.m_VampiricCuspidTime = 0; break;
							case nsPremiumItem::VAMPIRIC_CUS_EX: chaPremiumitem.m_VampiricCuspidEXTime = 0; break;
							case nsPremiumItem::CARAVAN: chaPremiumitem.m_CaravanTime = 0; break;
							case nsPremiumItem::GoldLooter: chaPremiumitem.m_GoldLooterPTime = 0; break;
							case nsPremiumItem::HP_BOOSTER: chaPremiumitem.m_HPBoosterPTime = 0; break;
							case nsPremiumItem::MP_BOOSTER: chaPremiumitem.m_MPBoosterPTime = 0; break;
							case nsPremiumItem::RES_BOOSTER: chaPremiumitem.m_RESBoosterPTime = 0; break;
							}
							smTRANS_COMMAND Packet;
							Packet.size = sizeof(Packet);
							Packet.code = OpCode::DELETE_ITEM_PREMIUM;
							Packet.LParam = chaPremiumitem.UpKeepItem[i].TGAImageNumber;
							if(smWsockServer)
								smWsockServer->Send((char*)&Packet, Packet.size, TRUE);
						}
					});
					_ConfirmationBox.Start();
				}
			}
		}
	}
}

void cHaPremiumItem::Draw()
{

	switch(Using_ItemKind)
	{
	case 8:
		if(TelePortCore.PageButtonFlag[0])
			DrawSprite(100, 365, chaQuest.lpQuestButtonImage[2], 0, 0, 20, 20);
		else
			DrawSprite(100, 365, chaQuest.lpQuestButtonImage[0], 0, 0, 20, 20);
		if(TelePortCore.PageButtonFlag[1])
			DrawSprite(330, 365, chaQuest.lpQuestButtonImage[3], 0, 0, 20, 20);
		else
			DrawSprite(330, 365, chaQuest.lpQuestButtonImage[1], 0, 0, 20, 20);
	}
}

DWORD dwKeepItemTime[8] = { 0, SINITEM_TIME_1HOURS, SINITEM_TIME_3HOURS, SINITEM_TIME_6HOURS, SINITEM_TIME_12HOURS, SINITEM_TIME_ONEDAY, SINITEM_TIME_SEVENDAY, SINITEM_TIME_30DAY };

int  cHaPremiumItem::CheckTime(DWORD CurrentTime)
{


	for(int i = 0; i < 7; i++)
	{
		if(dwKeepItemTime[i] < CurrentTime  &&  CurrentTime < dwKeepItemTime[i + 1])
		{
			return i + 1;
		}
	}

	return false;
}

void cHaPremiumItem::SetBigHead1Time(DWORD Time)
{
	m_BigHead1 = Time;

	if(Time == 0)
	{
		for(int i = 0; i < SINUPKEEPITEM_MAX; i++)
		{
			if(UpKeepItem[i].TGAImageNumber == nsPremiumItem::BIGHEAD1)
			{
				UpKeepItem[i].IconTime[0] = UpKeepItem[i].IconTime[1];
			}
		}
	}
}

extern void ResetHairModel();

void cHaPremiumItem::DrawUp()
{

    int i;
    for (i = 0; i < SINUPKEEPITEM_MAX; i++)
    {
        if (UpKeepItem[i].Visible == true)
        {
            dsDrawTexImage(UpKeepItemTGAImage[UpKeepItem[i].TGAImageNumber - 1], UpKeepItem[i].PosiX, UpKeepItem[i].PosiY, 32, 32, UpKeepItem[i].Alpha);

            if (UpKeepItem[i].Infoflag == true && InterfaceParty.PartyPosState == PARTY_NONE)
            {
                if (i > 6 && i < 14)
                {
                    InfoBox(WinSizeX - 92 - 208 - 10, (WinSizeY / 2) - 145 + (36 * (i - 7)), 10, 6);
                }
                else if (i > 13)
                {
                    InfoBox(WinSizeX - 92 - 208 - 10, (WinSizeY / 2) - 145 + (36 * (i - 14)), 10, 6);
                }
                else
                {
                    InfoBox(WinSizeX - 92 - 208 - 10, (WinSizeY / 2) - 145 + (36 * (i)), 10, 6);

                }
            }

			int    MinLen = 0;
			DWORD  dwCurrentTime = UpKeepItem[i].IconTime[1] - UpKeepItem[i].IconTime[0];
			switch(UpKeepItem[i].TGAImageNumber)
			{
			case nsPremiumItem::THIRD_EYES:
				switch(CheckTime(dwCurrentTime))
				{
				case 1:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_1HOURS / 32));
					break;
				case 2:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_3HOURS / 32));
					break;
				case 3:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_6HOURS / 32));
					break;
				case 4:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_12HOURS / 32));
					break;
				case 5:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_ONEDAY / 32));
					break;
				case 6:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_SEVENDAY / 32));
					break;
				case 7:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_30DAY / 32));
					break;
				}

				if(dwCurrentTime <= 0)
				{
					chaPremiumitem.m_ThirdEyesTime = 0;
				}

				break;
			case nsPremiumItem::EXPUP_POTION:
				switch (CheckTime(dwCurrentTime))
				{
				case 1:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_1HOURS / 32));
					break;
				case 2:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_3HOURS / 32));
					break;
				case 3:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_6HOURS / 32));
					break;
				case 4:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_12HOURS / 32));
					break;
				case 5:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_ONEDAY / 32));
					break;
				case 6:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_SEVENDAY / 32));
					break;
				case 7:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_30DAY / 32));
					break;
				}

				if(dwCurrentTime <= 0)
				{
					chaPremiumitem.m_ExpUpPotionTime = 0;
				}

				break;
			case nsPremiumItem::MIGHT_OF_AWELL:
				switch (CheckTime(dwCurrentTime))
				{
				case 1:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_1HOURS / 32));
					break;
				case 2:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_3HOURS / 32));
					break;
				case 3:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_6HOURS / 32));
					break;
				case 4:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_12HOURS / 32));
					break;
				case 5:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_ONEDAY / 32));
					break;
				case 6:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_SEVENDAY / 32));
					break;
				case 7:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_30DAY / 32));
					break;
				}
				break;
			case nsPremiumItem::MANA_REDUCE_P:
				switch (CheckTime(dwCurrentTime))
				{
				case 1:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_1HOURS / 32));
					break;
				case 2:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_3HOURS / 32));
					break;
				case 3:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_6HOURS / 32));
					break;
				case 4:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_12HOURS / 32));
					break;
				case 5:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_ONEDAY / 32));
					break;
				case 6:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_SEVENDAY / 32));
					break;
				case 7:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_30DAY / 32));
					break;
				}

				if(dwCurrentTime <= 0)
				{
					chaPremiumitem.m_ManaReducePotiontime = 0;
					chaPremiumitem.m_ManaReducePotionValue = 0;
					chaPremiumitem.SetManaReducePotionTime(chaPremiumitem.m_ManaReducePotiontime);
				}
				break;


			case nsPremiumItem::VAMPIRIC_CUS:
				switch (CheckTime(dwCurrentTime))
				{
				case 1:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_1HOURS / 32));
					break;
				case 2:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_3HOURS / 32));
					break;
				case 3:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_6HOURS / 32));
					break;
				case 4:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_12HOURS / 32));
					break;
				case 5:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_ONEDAY / 32));
					break;
				case 6:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_SEVENDAY / 32));
					break;
				case 7:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_30DAY / 32));
					break;
				}

				if(dwCurrentTime <= 0)
				{
					chaPremiumitem.m_VampiricCuspidTime = 0;
				}
				break;


			case nsPremiumItem::VAMPIRIC_CUS_EX:
				switch (CheckTime(dwCurrentTime))
				{
				case 1:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_1HOURS / 32));
					break;
				case 2:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_3HOURS / 32));
					break;
				case 3:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_6HOURS / 32));
					break;
				case 4:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_12HOURS / 32));
					break;
				case 5:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_ONEDAY / 32));
					break;
				case 6:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_SEVENDAY / 32));
					break;
				case 7:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_30DAY / 32));
					break;
				}

				if(dwCurrentTime <= 0)
				{
					chaPremiumitem.m_VampiricCuspidEXTime = 0;
				}
				break;


			case nsPremiumItem::MANA_RECHAR_P:
				switch (CheckTime(dwCurrentTime))
				{
				case 1:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_1HOURS / 32));
					break;
				case 2:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_3HOURS / 32));
					break;
				case 3:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_6HOURS / 32));
					break;
				case 4:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_12HOURS / 32));
					break;
				case 5:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_ONEDAY / 32));
					break;
				case 6:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_SEVENDAY / 32));
					break;
				case 7:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_30DAY / 32));
					break;
				}

				if(dwCurrentTime <= 0)
				{
					chaPremiumitem.m_ManaRechargingPTime = 0;
					chaPremiumitem.SetManaRechargingPTime(chaPremiumitem.m_ManaRechargingPTime);
				}
				break;

			case nsPremiumItem::STAMINA_RECHAR_P:
				switch (CheckTime(dwCurrentTime))
				{
				case 1:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_1HOURS / 32));
					break;
				case 2:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_3HOURS / 32));
					break;
				case 3:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_6HOURS / 32));
					break;
				case 4:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_12HOURS / 32));
					break;
				case 5:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_ONEDAY / 32));
					break;
				case 6:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_SEVENDAY / 32));
					break;
				case 7:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_30DAY / 32));
					break;
				}

				if (dwCurrentTime <= 0)
				{
					chaPremiumitem.m_StaminaRechargingPTime = 0;
					chaPremiumitem.SetStaminaRechargingPTime(chaPremiumitem.m_StaminaRechargingPTime);
				}
				break;

			case nsPremiumItem::GoldLooter:
				switch (CheckTime(dwCurrentTime))
				{
				case 1:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_1HOURS / 32));
					break;
				case 2:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_3HOURS / 32));
					break;
				case 3:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_6HOURS / 32));
					break;
				case 4:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_12HOURS / 32));
					break;
				case 5:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_ONEDAY / 32));
					break;
				case 6:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_SEVENDAY / 32));
					break;
				case 7:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_30DAY / 32));
					break;
				}

				if (dwCurrentTime <= 0)
				{
					chaPremiumitem.m_GoldLooterPTime = 0;
					chaPremiumitem.SetGoldLooterPTime(chaPremiumitem.m_GoldLooterPTime);
				}
				break;


			case nsPremiumItem::HP_BOOSTER:
				switch (CheckTime(dwCurrentTime))
				{
				case 1:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_1HOURS / 32));
					break;
				case 2:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_3HOURS / 32));
					break;
				case 3:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_6HOURS / 32));
					break;
				case 4:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_12HOURS / 32));
					break;
				case 5:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_ONEDAY / 32));
					break;
				case 6:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_SEVENDAY / 32));
					break;
				case 7:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_30DAY / 32));
					break;
				}

				if (dwCurrentTime <= 0)
				{
					chaPremiumitem.m_HPBoosterPTime = 0;
					chaPremiumitem.SetHPBoosterPTime(chaPremiumitem.m_HPBoosterPTime);
				}
				break;

			case nsPremiumItem::MP_BOOSTER:
				switch (CheckTime(dwCurrentTime))
				{
				case 1:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_1HOURS / 32));
					break;
				case 2:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_3HOURS / 32));
					break;
				case 3:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_6HOURS / 32));
					break;
				case 4:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_12HOURS / 32));
					break;
				case 5:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_ONEDAY / 32));
					break;
				case 6:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_SEVENDAY / 32));
					break;
				case 7:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_30DAY / 32));
					break;
				}

				if (dwCurrentTime <= 0)
				{
					chaPremiumitem.m_MPBoosterPTime = 0;
					chaPremiumitem.SetMPBoosterPTime(chaPremiumitem.m_MPBoosterPTime);
				}
				break;

			case nsPremiumItem::RES_BOOSTER:
				switch (CheckTime(dwCurrentTime))
				{
				case 1:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_1HOURS / 32));
					break;
				case 2:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_3HOURS / 32));
					break;
				case 3:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_6HOURS / 32));
					break;
				case 4:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_12HOURS / 32));
					break;
				case 5:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_ONEDAY / 32));
					break;
				case 6:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_SEVENDAY / 32));
					break;
				case 7:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_30DAY / 32));
					break;
				}

				if (dwCurrentTime <= 0)
				{
					chaPremiumitem.m_RESBoosterPTime = 0;
					chaPremiumitem.SetRESBoosterPTime(chaPremiumitem.m_RESBoosterPTime);
				}
				break;

			case nsPremiumItem::STAMINA_REDUCE_P:
				switch (CheckTime(dwCurrentTime))
				{
				case 1:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_1HOURS / 32));
					break;
				case 2:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_3HOURS / 32));
					break;
				case 3:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_6HOURS / 32));
					break;
				case 4:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_12HOURS / 32));
					break;
				case 5:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_ONEDAY / 32));
					break;
				case 6:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_SEVENDAY / 32));
					break;
				case 7:
					MinLen = (32 - (dwCurrentTime) / (SINITEM_TIME_30DAY / 32));
					break;
				}

				if(dwCurrentTime <= 0)
				{
					chaPremiumitem.m_StaminaReducePotiontime = 0;
					chaPremiumitem.m_StaminaReducePotionValue = 0;
					chaPremiumitem.SetStaminaReducePotionTime(chaPremiumitem.m_StaminaReducePotiontime);
				}
				break;
				
			case nsPremiumItem::BIGHEAD1:	
				switch( CheckTime( dwCurrentTime ) )
				{
					case 1:
							MinLen =(32-(dwCurrentTime)/(SINITEM_TIME_1HOURS/32));     
					break;
					case 2:
						MinLen =(32-(dwCurrentTime)/(SINITEM_TIME_3HOURS/32));    
					break;
					case 3:
						MinLen =(32-(dwCurrentTime)/(SINITEM_TIME_ONEDAY/32));    
					break;
					case 4:
						MinLen =(32-(dwCurrentTime)/(SINITEM_TIME_SEVENDAY/32));  
					break;
					case 5:
						MinLen =(32-(dwCurrentTime)/(SINITEM_TIME_30DAY/32));     
					break;
				}

				if(dwCurrentTime <= 1)
				{
					SetBigHeadTime( 0 );
					SetBigHeadType( 0 );
					ResetHairModel();
					cInvenTory.SetItemToChar();
				}
				break;

			case nsPremiumItem::CARAVAN:
				switch (CheckTime(dwCurrentTime))
				{
				case 1:
					MinLen = (32 - (dwCurrentTime) / ((SINITEM_TIME_ONEDAY * 3) / 32));
					break;
				case 2:
					MinLen = (32 - (dwCurrentTime) / ((SINITEM_TIME_ONEDAY * 7) / 32));
					break;
				}

				if (dwCurrentTime <= 1)
				{
					chaPremiumitem.m_CaravanTime = 0;
					chaPremiumitem.SetCaravanTime(chaPremiumitem.m_CaravanTime);
				}
				break;
			}



			if(UpKeepItem[i].IconTime[1] - UpKeepItem[i].IconTime[0] <= 10)
			{
				if(UpKeepItem[i].TGAImageNumber == nsPremiumItem::HELP_PET_TERRY || UpKeepItem[i].TGAImageNumber == nsPremiumItem::HELP_PET_NEPSYS ||
					UpKeepItem[i].TGAImageNumber == nsPremiumItem::HELP_PET_IO || UpKeepItem[i].TGAImageNumber == nsPremiumItem::HELP_PET_MUTE)
				{
					if(UpKeepItem[i].IconTime[1] - UpKeepItem[i].IconTime[0] <= 0)
					{
						chaPremiumitem.UpKeepItem[i].PosiX = 0;
						chaPremiumitem.UpKeepItem[i].PosiY = 0;
						chaPremiumitem.UpKeepItem[i].Visible = false;
						chaPremiumitem.UpKeepItem[i].TGAImageNumber = 0;
						chaPremiumitem.UpKeepItem[i].IconTime[0] = 0;
						chaPremiumitem.UpKeepItem[i].IconTime[1] = 0;
						chaPremiumitem.UpKeepItem[i].Alpha = 255;
						chaPremiumitem.UpKeepItem[i].Alphakind = 0;
						chaPremiumitem.UpKeepItem[i].Per = 0;
						memset(chaPremiumitem.UpKeepItem[i].Id, NULL, sizeof(chaPremiumitem.UpKeepItem[i].Id));

						m_HelpPetTime = 0;

						sinChar->GravityScroolCheck[1] = 0;
						cHelpPet.ClosePet();
					}
				}

				if(UpKeepItem[i].Alpha == 255)
					UpKeepItem[i].Alphakind = 1;
				if(UpKeepItem[i].Alpha == 50)
					UpKeepItem[i].Alphakind = 2;
			}
			else
			{
				if(UpKeepItem[i].Alpha == 0)
				{
					UpKeepItem[i].Alphakind = 0;
				}
			}
			if(UpKeepItem[i].Alphakind == 0)
			{
				DrawSprite(UpKeepItem[i].PosiX, UpKeepItem[i].PosiY + (32 - MinLen), lpUpKeepItemBackImage[UpKeepItem[i].TGAImageNumber - 1], 0, 32 - MinLen, 32, 32);
			}

			switch(UpKeepItem[i].Alphakind)
			{
			case 0:
				if(UpKeepItem[i].Alpha < 255)
					UpKeepItem[i].Alpha++;
				else
					UpKeepItem[i].Alpha = 255;
				break;
			case 1:
				if(UpKeepItem[i].Alpha > 50)
					UpKeepItem[i].Alpha -= 5;
				else
					UpKeepItem[i].Alpha = 50;
				break;
			case 2:
				if(UpKeepItem[i].Alpha < 255)
					UpKeepItem[i].Alpha += 5;
				else
					UpKeepItem[i].Alpha = 255;
				break;
			}
		}
	}
}

void cHaPremiumItem::LButtonDown(int x, int y)
{

	if(TelePortCore.PageButtonFlag[1])
	{
		if(TelePortCore.PageIndex < TELEPORT_PAGEMAX)
			TelePortCore.PageIndex++;
		cSinHelp.sinShowHelp(SIN_HELP_KIND_TELEPORT_SCROLL, QuestMessageBoxPosi2.x, QuestMessageBoxPosi2.y, QuestMessageBoxSize2.x, QuestMessageBoxSize2.y, D3DCOLOR_RGBA(0, 15, 128, 125), haTeleport_Field[TelePortCore.PageIndex]);
	}
	if(TelePortCore.PageButtonFlag[0])
	{
		if(0 < TelePortCore.PageIndex)
			TelePortCore.PageIndex--;
		cSinHelp.sinShowHelp(SIN_HELP_KIND_TELEPORT_SCROLL, QuestMessageBoxPosi2.x, QuestMessageBoxPosi2.y, QuestMessageBoxSize2.x, QuestMessageBoxSize2.y, D3DCOLOR_RGBA(0, 15, 128, 125), haTeleport_Field[TelePortCore.PageIndex]);
	}
}

void cHaPremiumItem::LButtonUp(int x, int y)
{

}
DWORD dwPremiumItemCode[] = { (sinBI1 | sin01),(sinBI1 | sin02),(sinBI1 | sin03),(sinBI1 | sin04),(sinBI1 | sin05),(sinBI1 | sin06),(sinBI1 | sin07) };

int cHaPremiumItem::UsePremiumItem(int kind)
{
	Using_ItemKind = 0;

	chaPremiumitem.bUpdateTimeDelayFlag = true;
	smTRANS_COMMAND smPacket;
	switch(kind)
	{
	case 1:
		cCharStatus.InitCharStatus(1);
		cInvenTory.SetItemToChar();
		if(sinChar->Life[1] < sinGetLife()) {
			sinSetLife(sinChar->Life[1]);
			ReformCharForm();
			ResetEnergyGraph(4);
		}
		haCastleSkillUseFlag = 1;
		break;
	case 2:
		cSkill.InitSkillPoint(1);
		haCastleSkillUseFlag = 1;
		break;
	case 3:
		cSkill.InitSkillPoint(1);
		cCharStatus.InitCharStatus(1);
		cInvenTory.SetItemToChar();
		if(sinChar->Life[1] < sinGetLife()) {
			sinSetLife(sinChar->Life[1]);
			ReformCharForm();
			ResetEnergyGraph(4);
		}
		haCastleSkillUseFlag = 1;
		break;
	case 5:
		SetPremiumItemSkill(1);
		cInvenTory.SetItemToChar();
		break;
	case 6:
		SetPremiumItemSkill(2);
		break;
	case 7:
		SetPremiumItemSkill(3);
		break;
	case 8:
		if (!CheckWindowOpen())break;

		TCORE::GetInstance()->tcoreOpen();
		//Using_ItemKind = kind;
	//cSinHelp.sinShowHelp(SIN_HELP_KIND_TELEPORT_SCROLL, QuestMessageBoxPosi2.x, QuestMessageBoxPosi2.y, QuestMessageBoxSize2.x, QuestMessageBoxSize2.y,
	//		D3DCOLOR_RGBA(0, 15, 128, 125), haTeleport_Field[TelePortCore.PageIndex]);


		haCastleSkillUseFlag = 0;
		//HelpBoxKindIndex = 0;

		break;
	case 9:
		haCastleSkillUseFlag = 1;
		break;
	case 10:
	case 11:
		haCastleSkillUseFlag = 0;
		break;
	case 12:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::THIRD_EYES, SINITEM_TIME_12HOURS, true, UpKeepItemName[0], 50);

		break;
	case 13:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::EXPUP_POTION, SINITEM_TIME_12HOURS, true, UpKeepItemName[1], 30);

		break;
	case 14:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::THIRD_EYES, SINITEM_TIME_24HOURS, true, UpKeepItemName[0], 50);

		break;
	case 15:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::EXPUP_POTION, SINITEM_TIME_24HOURS, true, UpKeepItemName[1], 30);

		break;
	case 16:
		if(UseHairtintPotion(1))
			haCastleSkillUseFlag = 1;
		break;
	case 17:
		if(UseHairtintPotion(2))
			haCastleSkillUseFlag = 1;
		break;
	case 18:
		if(UseHairtintPotion(3))
			haCastleSkillUseFlag = 1;
		break;
	case 19:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::VAMPIRIC_CUS, SINITEM_TIME_12HOURS, true, UpKeepItemName[2]);
		SetVampiricCuspidTime(SINITEM_TIME_12HOURS);
		break;
	case 20:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::VAMPIRIC_CUS, SINITEM_TIME_24HOURS, true, UpKeepItemName[2]);
		SetVampiricCuspidTime(SINITEM_TIME_24HOURS);
		break;
	case 21:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::MANA_RECHAR_P, SINITEM_TIME_12HOURS, true, UpKeepItemName[3]);
		SetManaRechargingPTime(SINITEM_TIME_12HOURS);
		break;
	case 22:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::MANA_RECHAR_P, SINITEM_TIME_24HOURS, true, UpKeepItemName[3]);
		SetManaRechargingPTime(SINITEM_TIME_24HOURS);
		break;
	case 23:
		haCastleSkillUseFlag = 1;
		break;

	case 24:
		haCastleSkillUseFlag = 1;
		if(SetUpKeepItem(nsPremiumItem::THIRD_EYES, SINITEM_TIME_1HOUR, true, UpKeepItemName[0], 20, 1))
			SetUpKeepItem(nsPremiumItem::EXPUP_POTION, SINITEM_TIME_1HOUR, true, UpKeepItemName[1], 10);

		break;
	case 25:
		haCastleSkillUseFlag = 1;
		if(SetUpKeepItem(nsPremiumItem::THIRD_EYES, SINITEM_TIME_6HOURS, true, UpKeepItemName[0], 20, 1))
			SetUpKeepItem(nsPremiumItem::EXPUP_POTION, SINITEM_TIME_6HOURS, true, UpKeepItemName[1], 10);

		break;
	case 26:
		haCastleSkillUseFlag = 1;
		if(SetUpKeepItem(nsPremiumItem::THIRD_EYES, SINITEM_TIME_12HOURS, true, UpKeepItemName[0], 20, 1))
			SetUpKeepItem(nsPremiumItem::EXPUP_POTION, SINITEM_TIME_12HOURS, true, UpKeepItemName[1], 10);

		break;
	case 27:
		haCastleSkillUseFlag = 1;
		if(SetUpKeepItem(nsPremiumItem::THIRD_EYES, SINITEM_TIME_24HOURS, true, UpKeepItemName[0], 20, 1))
			SetUpKeepItem(nsPremiumItem::EXPUP_POTION, SINITEM_TIME_24HOURS, true, UpKeepItemName[1], 10);

		break;
	case 28:
		haCastleSkillUseFlag = 1;
		if(SetUpKeepItem(nsPremiumItem::THIRD_EYES, SINITEM_TIME_1HOUR, true, UpKeepItemName[0], 30, 1))
			SetUpKeepItem(nsPremiumItem::EXPUP_POTION, SINITEM_TIME_1HOUR, true, UpKeepItemName[1], 20);

		break;
	case 29:
		haCastleSkillUseFlag = 1;
		if(SetUpKeepItem(nsPremiumItem::THIRD_EYES, SINITEM_TIME_6HOURS, true, UpKeepItemName[0], 30, 1))
			SetUpKeepItem(nsPremiumItem::EXPUP_POTION, SINITEM_TIME_6HOURS, true, UpKeepItemName[1], 20);

		break;
	case 30:
		haCastleSkillUseFlag = 1;
		if(SetUpKeepItem(nsPremiumItem::THIRD_EYES, SINITEM_TIME_12HOURS, true, UpKeepItemName[0], 30, 1))
			SetUpKeepItem(nsPremiumItem::EXPUP_POTION, SINITEM_TIME_12HOURS, true, UpKeepItemName[1], 20);


		break;
	case 31:
		haCastleSkillUseFlag = 1;
		if(SetUpKeepItem(nsPremiumItem::THIRD_EYES, SINITEM_TIME_24HOURS, true, UpKeepItemName[0], 30, 1))
			SetUpKeepItem(nsPremiumItem::EXPUP_POTION, SINITEM_TIME_24HOURS, true, UpKeepItemName[1], 20);

		break;

	case 32:
		haCastleSkillUseFlag = 1;
		if(SetUpKeepItem(nsPremiumItem::THIRD_EYES, SINITEM_TIME_1HOUR, true, UpKeepItemName[0], 40, 1))
			SetUpKeepItem(nsPremiumItem::EXPUP_POTION, SINITEM_TIME_1HOUR, true, UpKeepItemName[1], 30);

		break;

	case 33:
		haCastleSkillUseFlag = 1;
		if(SetUpKeepItem(nsPremiumItem::THIRD_EYES, SINITEM_TIME_6HOURS, true, UpKeepItemName[0], 40, 1))
			SetUpKeepItem(nsPremiumItem::EXPUP_POTION, SINITEM_TIME_6HOURS, true, UpKeepItemName[1], 30);

		break;
	case 34:
		haCastleSkillUseFlag = 1;
		if(SetUpKeepItem(nsPremiumItem::THIRD_EYES, SINITEM_TIME_12HOURS, true, UpKeepItemName[0], 40, 1))
			SetUpKeepItem(nsPremiumItem::EXPUP_POTION, SINITEM_TIME_12HOURS, true, UpKeepItemName[1], 30);

		break;
	case 35:
		haCastleSkillUseFlag = 1;
		if(SetUpKeepItem(nsPremiumItem::THIRD_EYES, SINITEM_TIME_24HOURS, true, UpKeepItemName[0], 40, 1))
			SetUpKeepItem(nsPremiumItem::EXPUP_POTION, SINITEM_TIME_24HOURS, true, UpKeepItemName[1], 30);
		break;


	case 36:
		if(UseHairtintPotion(4))
			haCastleSkillUseFlag = 1;
		break;
	case 37:
		if(UseHairtintPotion(5))
			haCastleSkillUseFlag = 1;
		break;
	case 38:
		if(UseHairtintPotion(6))
			haCastleSkillUseFlag = 1;
		break;
	case 39:
		if(
			UseHairtintPotion(7))
			haCastleSkillUseFlag = 1;
		break;
	case 40:
		if(UseHairtintPotion(8))
			haCastleSkillUseFlag = 1;
		break;
	case 41:
		if(UseHairtintPotion(9))
			haCastleSkillUseFlag = 1;
		break;
	case 42:
		if(UseHairtintPotion(10))
			haCastleSkillUseFlag = 1;
		break;
	case 43:
		if(UseHairtintPotion(11))
			haCastleSkillUseFlag = 1;
		break;
	case 44:
		if(UseHairtintPotion(12))
			haCastleSkillUseFlag = 1;
		break;
	case 45:
		if(UseHairtintPotion(13))
			haCastleSkillUseFlag = 1;
		break;


	case 46:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::EXPUP_POTION, SINITEM_TIME_24HOURS, true, UpKeepItemName[1], 50);

		break;
	case 47:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::EXPUP_POTION, SINITEM_TIME_SEVENDAY, true, UpKeepItemName[1], 50);
		break;
	case 48:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::MIGHT_OF_AWELL, SINITEM_TIME_SEVENDAY, true, UpKeepItemName[4], 300);
		break;
	case 49:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::MIGHT_OF_AWELL, SINITEM_TIME_30DAY, true, UpKeepItemName[4], 300);
		break;
	case 50:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::MANA_REDUCE_P, SINITEM_TIME_12HOURS, true, UpKeepItemName[5], 30);
		break;
	case 51:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::MANA_REDUCE_P, SINITEM_TIME_24HOURS, true, UpKeepItemName[5], 30);
		break;
	case 52:
		haCastleSkillUseFlag = 1;
		if(SetUpKeepItem(nsPremiumItem::MANA_REDUCE_P, SINITEM_TIME_6HOURS, true, UpKeepItemName[5], 10, 2))
		{
			SetUpKeepItem(nsPremiumItem::EXPUP_POTION, SINITEM_TIME_6HOURS, true, UpKeepItemName[1], 10);
		}
		break;
	case 53:
		haCastleSkillUseFlag = 1;
		if(SetUpKeepItem(nsPremiumItem::MANA_REDUCE_P, SINITEM_TIME_12HOURS, true, UpKeepItemName[5], 10, 2))
		{
			SetUpKeepItem(nsPremiumItem::EXPUP_POTION, SINITEM_TIME_12HOURS, true, UpKeepItemName[1], 10);
		}
		break;
	case 54:
		haCastleSkillUseFlag = 1;
		if(SetUpKeepItem(nsPremiumItem::MANA_REDUCE_P, SINITEM_TIME_24HOURS, true, UpKeepItemName[5], 10, 2))
		{
			SetUpKeepItem(nsPremiumItem::EXPUP_POTION, SINITEM_TIME_24HOURS, true, UpKeepItemName[1], 10);
		}
		break;
	case 55:
		haCastleSkillUseFlag = 1;
		if(SetUpKeepItem(nsPremiumItem::MANA_REDUCE_P, SINITEM_TIME_6HOURS, true, UpKeepItemName[5], 20, 2))
		{
			SetUpKeepItem(nsPremiumItem::EXPUP_POTION, SINITEM_TIME_6HOURS, true, UpKeepItemName[1], 20);
		}
		break;
	case 56:
		haCastleSkillUseFlag = 1;
		if(SetUpKeepItem(nsPremiumItem::MANA_REDUCE_P, SINITEM_TIME_12HOURS, true, UpKeepItemName[5], 20, 2))
		{
			SetUpKeepItem(nsPremiumItem::EXPUP_POTION, SINITEM_TIME_12HOURS, true, UpKeepItemName[1], 20);
		}
		break;
	case 57:
		haCastleSkillUseFlag = 1;
		if(SetUpKeepItem(nsPremiumItem::MANA_REDUCE_P, SINITEM_TIME_24HOURS, true, UpKeepItemName[5], 20, 2))
		{
			SetUpKeepItem(nsPremiumItem::EXPUP_POTION, SINITEM_TIME_24HOURS, true, UpKeepItemName[1], 20);
		}
		break;
	case 58:
		haCastleSkillUseFlag = 1;
		if(SetUpKeepItem(nsPremiumItem::MANA_REDUCE_P, SINITEM_TIME_6HOURS, true, UpKeepItemName[5], 30, 2))
		{
			SetUpKeepItem(nsPremiumItem::EXPUP_POTION, SINITEM_TIME_6HOURS, true, UpKeepItemName[1], 30);
		}
		break;
	case 59:
		haCastleSkillUseFlag = 1;
		if(SetUpKeepItem(nsPremiumItem::MANA_REDUCE_P, SINITEM_TIME_12HOURS, true, UpKeepItemName[5], 30, 2))
		{
			SetUpKeepItem(nsPremiumItem::EXPUP_POTION, SINITEM_TIME_12HOURS, true, UpKeepItemName[1], 30);
		}
		break;
	case 60:
		haCastleSkillUseFlag = 1;
		if(SetUpKeepItem(nsPremiumItem::MANA_REDUCE_P, SINITEM_TIME_24HOURS, true, UpKeepItemName[5], 30, 2))
		{
			SetUpKeepItem(nsPremiumItem::EXPUP_POTION, SINITEM_TIME_24HOURS, true, UpKeepItemName[1], 30);
		}
		break;
	case 61:
		haCastleSkillUseFlag = 1;
		if(SetUpKeepItem(nsPremiumItem::MANA_REDUCE_P, SINITEM_TIME_6HOURS, true, UpKeepItemName[5], 40, 2))
		{
			SetUpKeepItem(nsPremiumItem::EXPUP_POTION, SINITEM_TIME_6HOURS, true, UpKeepItemName[1], 40);
		}
		break;
	case 62:
		haCastleSkillUseFlag = 1;
		if(SetUpKeepItem(nsPremiumItem::MANA_REDUCE_P, SINITEM_TIME_12HOURS, true, UpKeepItemName[5], 40, 2))
		{
			SetUpKeepItem(nsPremiumItem::EXPUP_POTION, SINITEM_TIME_12HOURS, true, UpKeepItemName[1], 40);
		}
		break;
	case 63:
		haCastleSkillUseFlag = 1;
		if(SetUpKeepItem(nsPremiumItem::MANA_REDUCE_P, SINITEM_TIME_24HOURS, true, UpKeepItemName[5], 40, 2))
		{
			SetUpKeepItem(nsPremiumItem::EXPUP_POTION, SINITEM_TIME_24HOURS, true, UpKeepItemName[1], 40);
		}
		break;
	case 64:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::MIGHT_OF_AWELL, SINITEM_TIME_SEVENDAY, true, UpKeepItemName[4], 500);
		break;
	case 65:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::MIGHT_OF_AWELL, SINITEM_TIME_30DAY, true, UpKeepItemName[4], 500);
		break;
	case 66:
		haCastleSkillUseFlag = 1;
		if(SetUpKeepItem(nsPremiumItem::THIRD_EYES, SINITEM_TIME_6HOURS, true, UpKeepItemName[0], 50, 1))
		{
			SetUpKeepItem(nsPremiumItem::EXPUP_POTION, SINITEM_TIME_6HOURS, true, UpKeepItemName[1], 40);
		}
		break;
	case 67:
		haCastleSkillUseFlag = 1;
		if(SetUpKeepItem(nsPremiumItem::THIRD_EYES, SINITEM_TIME_12HOURS, true, UpKeepItemName[0], 50, 1))
		{
			SetUpKeepItem(nsPremiumItem::EXPUP_POTION, SINITEM_TIME_12HOURS, true, UpKeepItemName[1], 40);
		}
		break;
	case 68:
		haCastleSkillUseFlag = 1;
		if(SetUpKeepItem(nsPremiumItem::THIRD_EYES, SINITEM_TIME_24HOURS, true, UpKeepItemName[0], 50, 1))
		{
			SetUpKeepItem(nsPremiumItem::EXPUP_POTION, SINITEM_TIME_24HOURS, true, UpKeepItemName[1], 40);
		}
		break;
	case 69:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::PHENIX_PET, SINITEM_TIME_12HOURS, true, UpKeepItemName[6], 30);
		break;
	case 70:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::PHENIX_PET, SINITEM_TIME_24HOURS, true, UpKeepItemName[6], 30);
		break;
	case 71:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::PHENIX_PET, SINITEM_TIME_6HOURS, true, UpKeepItemName[6], 30);
		break;



	case 73:
		haCastleSkillUseFlag = 1;
		UseAgingMaster(0);
		break;
	case 74:
		haCastleSkillUseFlag = 1;
		UseAgingMaster(1);
		break;
	case 75:
		haCastleSkillUseFlag = 1;
		UseAgingMaster(2);
		break;
	case 76:
		haCastleSkillUseFlag = 1;
		UseSkillMaster(1);
		break;
	case 77:
		haCastleSkillUseFlag = 1;
		UseSkillMaster(2);
		break;
	case 78:
		haCastleSkillUseFlag = 1;
		UseSkillMaster(3);
		break;
	case 79:
		haCastleSkillUseFlag = 1;
		cInvenTory.InvenItem[Using_SelectInvenItemIndex].sItemInfo.PotionCount = 300;
		sinThrowItemToFeild(&cInvenTory.InvenItem[Using_SelectInvenItemIndex]);
		cInvenTory.InvenItem[Using_SelectInvenItemIndex].Flag = 0;
		cInvenTory.CheckWeight();
		cInvenTory.ReFormInvenItem();
		SaveGameData();
		UseMoveShop();
		break;
	case 80:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::EXPUP_POTION, SINITEM_TIME_6HOURS, true, UpKeepItemName[1], 100);
		break;
	case 81:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::EXPUP_POTION, SINITEM_TIME_12HOURS, true, UpKeepItemName[1], 100);
		break;
	case 82:
		cCharStatus.InitCharStatus_Attribute(1);
		cInvenTory.SetItemToChar();
		if(sinChar->Life[1] < sinGetLife()) {
			sinSetLife(sinChar->Life[1]);
			ReformCharForm();
			ResetEnergyGraph(4);
		}
		haCastleSkillUseFlag = 1;
		break;
	case 83:
		cCharStatus.InitCharStatus_Attribute(2);
		cInvenTory.SetItemToChar();
		if(sinChar->Life[1] < sinGetLife()) {
			sinSetLife(sinChar->Life[1]);
			ReformCharForm();
			ResetEnergyGraph(4);
		}
		haCastleSkillUseFlag = 1;
		break;
	case 84:
		cCharStatus.InitCharStatus_Attribute(3);
		cInvenTory.SetItemToChar();
		if(sinChar->Life[1] < sinGetLife()) {
			sinSetLife(sinChar->Life[1]);
			ReformCharForm();
			ResetEnergyGraph(4);
		}
		haCastleSkillUseFlag = 1;
		break;
	case 85:
		cCharStatus.InitCharStatus_Attribute(4);
		cInvenTory.SetItemToChar();
		if(sinChar->Life[1] < sinGetLife()) {
			sinSetLife(sinChar->Life[1]);
			ReformCharForm();
			ResetEnergyGraph(4);
		}
		haCastleSkillUseFlag = 1;
		break;
	case 86:
		cCharStatus.InitCharStatus_Attribute(5);
		cInvenTory.SetItemToChar();
		if(sinChar->Life[1] < sinGetLife()) {
			sinSetLife(sinChar->Life[1]);
			ReformCharForm();
			ResetEnergyGraph(4);
		}
		haCastleSkillUseFlag = 1;
		break;
	case 87:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::EXPUP_POTION, SINITEM_TIME_24HOURS, true, UpKeepItemName[1], 100);
		break;
	case 88:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::PHENIX_PET, SINITEM_TIME_72HOURS, true, UpKeepItemName[6], 30);
		break;

	case 97:
	case 98:
		haCastleSkillUseFlag = 0;
		break;
	case 99:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::HELP_PET_TERRY, SINITEM_TIME_1HOUR, true, UpKeepItemName[7], 30, 3);
		sinChar->GravityScroolCheck[1] = 1;
		break;
	case 100:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::HELP_PET_NEPSYS, SINITEM_TIME_1HOUR, true, UpKeepItemName[8], 30, 3);
		break;
	case 101:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::HELP_PET_IO, SINITEM_TIME_1HOUR, true, UpKeepItemName[9], 30, 3);
		break;
	case 102:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::HELP_PET_MUTE, SINITEM_TIME_1HOUR, true, UpKeepItemName[10], 30, 3);
		sinChar->GravityScroolCheck[1] = 2;
		break;

	case 103:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::HELP_PET_TERRY, SINITEM_TIME_12HOURS, true, UpKeepItemName[7], 30, 3);
		sinChar->GravityScroolCheck[1] = 1;
		break;
	case 104:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::HELP_PET_NEPSYS, SINITEM_TIME_12HOURS, true, UpKeepItemName[8], 30, 3);
		break;
	case 105:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::HELP_PET_IO, SINITEM_TIME_12HOURS, true, UpKeepItemName[9], 30, 3);
		break;
	case 106:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::HELP_PET_MUTE, SINITEM_TIME_12HOURS, true, UpKeepItemName[10], 30, 3);
		sinChar->GravityScroolCheck[1] = 2;
		break;

	case 107:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::EXPUP_POTION, SINITEM_TIME_1HOUR, true, UpKeepItemName[1], 100);
		break;

	case 108:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::PHENIX_PET, SINITEM_TIME_1HOUR, true, UpKeepItemName[6], 30);
		break;

	case 109:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::HELP_PET_TERRY, SINITEM_TIME_6HOURS, true, UpKeepItemName[7], 30, 3);
		sinChar->GravityScroolCheck[1] = 1;
		break;
	case 110:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::HELP_PET_NEPSYS, SINITEM_TIME_6HOURS, true, UpKeepItemName[8], 30, 3);
		break;
	case 111:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::HELP_PET_IO, SINITEM_TIME_6HOURS, true, UpKeepItemName[9], 30, 3);
		break;
	case 112:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::HELP_PET_MUTE, SINITEM_TIME_6HOURS, true, UpKeepItemName[10], 30, 3);
		sinChar->GravityScroolCheck[1] = 2;
		break;
	case 113:
		haCastleSkillUseFlag = 1;
		UseAgingMaster(3);
		break;
	case 114:
		haCastleSkillUseFlag = 1;
		UseAgingMaster(4);
		break;
	case 115:
		haCastleSkillUseFlag = 1;
		UseAgingMaster(5);
		break;

	case 116:

		if(sinChar->GravityScroolCheck[0] == 5)
		{
			cMessageBox.ShowMessage(MESSAGE_GRAVITYSCROOLFAIL);
			return FALSE;
		}

		cMessageBox.ShowMessage(MESSAGE_GRAVITYSCROOLOK);

		sinChar->GravityScroolCheck[0] += 1;


		cCharStatus.UseGravityScrool();
		cInvenTory.SetItemToChar();
		ReformCharForm();

		haCastleSkillUseFlag = 1;

		break;


	case 117:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::HELP_PET_TERRY, SINITEM_TIME_24HOURS, true, UpKeepItemName[7], 30, 3);
		sinChar->GravityScroolCheck[1] = 1;
		break;
	case 118:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::HELP_PET_NEPSYS, SINITEM_TIME_24HOURS, true, UpKeepItemName[8], 30, 3);
		break;
	case 119:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::HELP_PET_IO, SINITEM_TIME_24HOURS, true, UpKeepItemName[9], 30, 3);
		break;
	case 120:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::HELP_PET_MUTE, SINITEM_TIME_24HOURS, true, UpKeepItemName[10], 30, 3);
		sinChar->GravityScroolCheck[1] = 2;
		break;

	case 121:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::THIRD_EYES, SINITEM_TIME_6HOURS, true, UpKeepItemName[0], 50);

		break;

	case 122:
		haCastleSkillUseFlag = 0;
		break;

	case 123:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::EXPUP_POTION, SINITEM_TIME_6HOURS, true, UpKeepItemName[1], 50);
		break;

	case 124:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::VAMPIRIC_CUS, SINITEM_TIME_6HOURS, true, UpKeepItemName[2]);
		SetVampiricCuspidTime(SINITEM_TIME_6HOURS);
		break;

	case 125:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::MANA_RECHAR_P, SINITEM_TIME_6HOURS, true, UpKeepItemName[3]);
		SetManaRechargingPTime(SINITEM_TIME_6HOURS);
		break;

	case 126:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::MANA_REDUCE_P, SINITEM_TIME_6HOURS, true, UpKeepItemName[5], 30);
		break;

	case 127:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::VAMPIRIC_CUS_EX, SINITEM_TIME_6HOURS, true, UpKeepItemName[11]);
		SetVampiricCuspidEXTime(SINITEM_TIME_6HOURS);
		break;

	case 128:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::VAMPIRIC_CUS_EX, SINITEM_TIME_12HOURS, true, UpKeepItemName[11]);
		SetVampiricCuspidEXTime(SINITEM_TIME_12HOURS);
		break;

	case 129:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::VAMPIRIC_CUS_EX, SINITEM_TIME_24HOURS, true, UpKeepItemName[11]);
		SetVampiricCuspidEXTime(SINITEM_TIME_24HOURS);
		break;
	case 130:
		haCastleSkillUseFlag = 0;
		break;
	case 131:
		haCastleSkillUseFlag = 0;
		break;
	case 132:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::STAMINA_REDUCE_P, SINITEM_TIME_12HOURS, true, UpKeepItemName[12], 30);
		break;
	case 133:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::STAMINA_REDUCE_P, SINITEM_TIME_24HOURS, true, UpKeepItemName[12], 30);
		break;
	case 136:
		if(GetBigHeadType() != 0)
			break;

		SetUpKeepItem(nsPremiumItem::BIGHEAD1, SINITEM_TIME_ONEDAY, true, UpKeepItemName[13], 30);
		SetItemPremiumTime(nsPremiumItem::BIGHEAD1, SINITEM_TIME_ONEDAY);
		SetBigHeadType(1);
		ChangeHairModel(13);
	case 137:
		if(GetBigHeadType() != 0)
			break;

		SetUpKeepItem(nsPremiumItem::BIGHEAD1, SINITEM_TIME_ONEDAY, true, UpKeepItemName[13], 30);
		SetItemPremiumTime(nsPremiumItem::BIGHEAD1, SINITEM_TIME_ONEDAY);
		SetBigHeadType(2);
		ChangeHairModel(14);
	case 138:
		if(GetBigHeadType() != 0)
			break;

		SetUpKeepItem(nsPremiumItem::BIGHEAD1, SINITEM_TIME_ONEDAY, true, UpKeepItemName[13], 30);
		SetItemPremiumTime(nsPremiumItem::BIGHEAD1, SINITEM_TIME_ONEDAY);
		SetBigHeadType(3);
		ChangeHairModel(15);
	case 139:
		if(GetBigHeadType() != 0)
			break;

		SetUpKeepItem(nsPremiumItem::BIGHEAD1, SINITEM_TIME_ONEDAY, true, UpKeepItemName[13], 30);
		SetItemPremiumTime(nsPremiumItem::BIGHEAD1, SINITEM_TIME_ONEDAY);
		SetBigHeadType(4);
		ChangeHairModel(16);
		break;
	case 140:
		if(UseHairtintPotion(14))
			haCastleSkillUseFlag = 1;
		break;
	case 141:
		if(UseHairtintPotion(15))
			haCastleSkillUseFlag = 1;
		break;
	case 142:
		if(UseHairtintPotion(16))
			haCastleSkillUseFlag = 1;
		break;
	case 143:
		if(UseHairtintPotion(17))
			haCastleSkillUseFlag = 1;
		break;
	case 144:
		if(UseHairtintPotion(18))
			haCastleSkillUseFlag = 1;
		break;
	case 145:
		if (UseHairtintPotion(19))
			haCastleSkillUseFlag = 1;
		break;
	case 146:
		if (UseHairtintPotion(20))
			haCastleSkillUseFlag = 1;
		break;
	case 147:
		if (UseHairtintPotion(21))
			haCastleSkillUseFlag = 1;
		break;
	case 148:
		if (UseHairtintPotion(22))
			haCastleSkillUseFlag = 1;
		break;
	case 149:
		if (UseHairtintPotion(23))
			haCastleSkillUseFlag = 1;
		break;
	case 150:
		if (UseHairtintPotion(24))
			haCastleSkillUseFlag = 1;
		break;
	case 151:
		if (UseHairtintPotion(25))
			haCastleSkillUseFlag = 1;
		break;
	case 152:
		if (UseHairtintPotion(26))
			haCastleSkillUseFlag = 1;
		break;
	case 153:
		if (UseHairtintPotion(27))
			haCastleSkillUseFlag = 1;
		break;
	case 154:
		//vale level

		break;
	case 155:
		//vale Creditos
		break;
	case 156:
		haCastleSkillUseFlag = 1;
		cInvenTory.InvenItem[Using_SelectInvenItemIndex].sItemInfo.PotionCount = 300;
		sinThrowItemToFeild(&cInvenTory.InvenItem[Using_SelectInvenItemIndex]);
		cInvenTory.InvenItem[Using_SelectInvenItemIndex].Flag = 0;
		cInvenTory.CheckWeight();
		cInvenTory.ReFormInvenItem();
		SaveGameData();
		UseMoveAging();
		break;
	case 157:
		haCastleSkillUseFlag = 1;
		cInvenTory.InvenItem[Using_SelectInvenItemIndex].sItemInfo.PotionCount = 300;
		sinThrowItemToFeild(&cInvenTory.InvenItem[Using_SelectInvenItemIndex]);
		cInvenTory.InvenItem[Using_SelectInvenItemIndex].Flag = 0;
		cInvenTory.CheckWeight();
		cInvenTory.ReFormInvenItem();
		SaveGameData();
		UseMoveMix();
		break;
	case 158:
		haCastleSkillUseFlag = 1;
		cInvenTory.InvenItem[Using_SelectInvenItemIndex].sItemInfo.PotionCount = 300;
		sinThrowItemToFeild(&cInvenTory.InvenItem[Using_SelectInvenItemIndex]);
		cInvenTory.InvenItem[Using_SelectInvenItemIndex].Flag = 0;
		cInvenTory.CheckWeight();
		cInvenTory.ReFormInvenItem();
		SaveGameData();
		UseMoveForce();
		break;
	case 159:
		haCastleSkillUseFlag = 1;
		cInvenTory.InvenItem[Using_SelectInvenItemIndex].sItemInfo.PotionCount = 300;
		sinThrowItemToFeild(&cInvenTory.InvenItem[Using_SelectInvenItemIndex]);
		cInvenTory.InvenItem[Using_SelectInvenItemIndex].Flag = 0;
		cInvenTory.CheckWeight();
		cInvenTory.ReFormInvenItem();
		SaveGameData();
		UseMoveBau();
		break;
	case 160:
		haCastleSkillUseFlag = 1;
		cInvenTory.InvenItem[Using_SelectInvenItemIndex].sItemInfo.PotionCount = 300;
		sinThrowItemToFeild(&cInvenTory.InvenItem[Using_SelectInvenItemIndex]);
		cInvenTory.InvenItem[Using_SelectInvenItemIndex].Flag = 0;
		cInvenTory.CheckWeight();
		cInvenTory.ReFormInvenItem();
		SaveGameData();
		UseMoveCraft();
		break;
	case 161:
		haCastleSkillUseFlag = 1;
		cInvenTory.InvenItem[Using_SelectInvenItemIndex].sItemInfo.PotionCount = 300;
		sinThrowItemToFeild(&cInvenTory.InvenItem[Using_SelectInvenItemIndex]);
		cInvenTory.InvenItem[Using_SelectInvenItemIndex].Flag = 0;
		cInvenTory.CheckWeight();
		cInvenTory.ReFormInvenItem();
		SaveGameData();
		UseMoveRuna();
		break;
	case 162:
		haCastleSkillUseFlag = 1;
		cInvenTory.InvenItem[Using_SelectInvenItemIndex].sItemInfo.PotionCount = 300;
		sinThrowItemToFeild(&cInvenTory.InvenItem[Using_SelectInvenItemIndex]);
		cInvenTory.InvenItem[Using_SelectInvenItemIndex].Flag = 0;
		cInvenTory.CheckWeight();
		cInvenTory.ReFormInvenItem();
		SaveGameData();
		UseMoveClear();
		break;

	case 163:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::STAMINA_RECHAR_P, SINITEM_TIME_3HOURS, true, UpKeepItemName[17]);
		SetStaminaRechargingPTime(SINITEM_TIME_3HOURS);
		break;
	case 164:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::STAMINA_RECHAR_P, SINITEM_TIME_ONEDAY, true, UpKeepItemName[17]);
		SetStaminaRechargingPTime(SINITEM_TIME_ONEDAY);
		break;
	case 165:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::STAMINA_RECHAR_P, SINITEM_TIME_SEVENDAY, true, UpKeepItemName[17]);
		SetStaminaRechargingPTime(SINITEM_TIME_SEVENDAY);
		break;

	case 167:
		haCastleSkillUseFlag = 1;

		smPacket.code = OpCode::OPCODE_USEITEM_CODE;
		smPacket.WParam = sinBI2 | sin90;
		smPacket.EParam = 0;
		smPacket.size = sizeof(smPacket);

		extern smWINSOCK* smWsockDataServer;

		if (smWsockDataServer)
		{
			smWsockDataServer->Send((char*)& smPacket, smPacket.size, TRUE);
		};
		break;

	case 168:
		haCastleSkillUseFlag = 1;
		chaPremiumitem.SetCaravanTime((3 * 24 * 60 * 60));
		chaPremiumitem.SetUpKeepItem(nsPremiumItem::CARAVAN, (3 * 24 * 60 * 60), true, "Caravana");
		break;
	case 169:
		haCastleSkillUseFlag = 1;
		chaPremiumitem.SetCaravanTime((7 * 24 * 60 * 60));
		chaPremiumitem.SetUpKeepItem(nsPremiumItem::CARAVAN, (7 * 24 * 60 * 60), true, "Caravana");

	case 170:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::GoldLooter, SINITEM_TIME_6HOURS, true, "Detector de Moeda");
		SetGoldLooterPTime(SINITEM_TIME_6HOURS);
		break;
	case 171:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::GoldLooter, SINITEM_TIME_12HOURS, true, "Detector de Moeda");
		SetGoldLooterPTime(SINITEM_TIME_12HOURS);
		break;
	case 172:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::GoldLooter, SINITEM_TIME_24HOURS, true, "Detector de Moeda");
		SetGoldLooterPTime(SINITEM_TIME_24HOURS);
		break;


	case 173:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::HP_BOOSTER, SINITEM_TIME_6HOURS, true, "HP BOOSTER");
		SetHPBoosterPTime(SINITEM_TIME_6HOURS);
		cInvenTory.SetItemToChar();
		ReformCharForm();
		break;
	case 174:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::HP_BOOSTER, SINITEM_TIME_12HOURS, true, "HP BOOSTER");
		SetHPBoosterPTime(SINITEM_TIME_12HOURS);
		cInvenTory.SetItemToChar();
		ReformCharForm();
		break;
	case 175:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::HP_BOOSTER, SINITEM_TIME_24HOURS, true, "HP BOOSTER");
		SetHPBoosterPTime(SINITEM_TIME_24HOURS);
		cInvenTory.SetItemToChar();
		ReformCharForm();
		break;

	case 176:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::MP_BOOSTER, SINITEM_TIME_6HOURS, true, "MP BOOSTER");
		SetMPBoosterPTime(SINITEM_TIME_6HOURS);
		break;
	case 177:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::MP_BOOSTER, SINITEM_TIME_12HOURS, true, "MP BOOSTER");
		SetMPBoosterPTime(SINITEM_TIME_12HOURS);
		cInvenTory.SetItemToChar();
		ReformCharForm();
		break;
	case 178:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::MP_BOOSTER, SINITEM_TIME_24HOURS, true, "MP BOOSTER");
		SetMPBoosterPTime(SINITEM_TIME_24HOURS);
		cInvenTory.SetItemToChar();
		ReformCharForm();
		break;

	case 179:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::RES_BOOSTER, SINITEM_TIME_6HOURS, true, "RES BOOSTER");
		SetRESBoosterPTime(SINITEM_TIME_6HOURS);
		cInvenTory.SetItemToChar();
		ReformCharForm();
		break;
	case 180:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::RES_BOOSTER, SINITEM_TIME_12HOURS, true, "RES BOOSTER");
		SetRESBoosterPTime(SINITEM_TIME_12HOURS);
		cInvenTory.SetItemToChar();
		ReformCharForm();
		break;
	case 181:
		haCastleSkillUseFlag = 1;
		SetUpKeepItem(nsPremiumItem::RES_BOOSTER, SINITEM_TIME_24HOURS, true, "RES BOOSTER");
		SetRESBoosterPTime(SINITEM_TIME_24HOURS);
		cInvenTory.SetItemToChar();
		ReformCharForm();
		break;


	}
	return TRUE;
}


int cHaPremiumItem::SetPremiumItemSkill(int kind)
{

	haCastleSkillUseFlag = 0;
	DWORD SkillCODE;
	int   SkillTime;
	switch(kind)
	{
	case 1:
		SkillCODE = SCROLL_P_INVULNERABILITY;
		SkillTime = 30;
		break;
	case 2:
		SkillCODE = SCROLL_P_CRITICAL;
		SkillTime = 60 * 60 * 24;
		break;
	case 3:
		SkillCODE = SCROLL_P_EVASION;
		SkillTime = 60 * 60 * 24;
		break;
	}

	sSKILL TempSkill;

	if(cSkill.SearchContiueSkillCODE(SkillCODE) == SkillCODE && SkillCODE != 0)
	{
		cMessageBox.ShowMessage(MESSAGE_CLANSKILL_USE);
		haCastleSkillUseFlag = 0;
		return TRUE;
	}
	haCastleSkillUseFlag = 1;

	for(int j = 0; j < SIN_MAX_SKILL; j++)
	{
		if(sSkill[j].CODE == SkillCODE)
		{
			memcpy(&TempSkill, &sSkill[j], sizeof(sSKILL));
			TempSkill.UseTime = SkillTime;
			sinContinueSkillSet(&TempSkill);
			SwitchSkill(&TempSkill);
			break;
		}
	}
	return TRUE;
}

int cHaPremiumItem::UseTeleportScrollItem(short ItemIndex)
{
	if(!chaPremiumitem.TelePortCore.FieldIndex[1])return FALSE;

	if(StageField[lpCurPlayer->OnStageField]->State == FIELD_STATE_ROOM)
	{
		cMessageBox.ShowMessage(MESSAGE_NO_USE_ITEM);
		return FALSE;
	}

	int i;
	for(i = 0; i < TELEPORT_FIELDMAX; i++)
	{
		if((chaPremiumitem.TelePortCore.FieldIndex[1] - 1) == TelePort_FieldNum[i][0])
		{

			if(TelePort_FieldNum[i][2] <= sinChar->Level)
			{


				WarpField2(TelePort_FieldNum[i][1]);
				cInvenTory.InvenItem[chaPremiumitem.TelePortCore.ItemIndex].sItemInfo.PotionCount = 300;
				sinThrowItemToFeild(&cInvenTory.InvenItem[chaPremiumitem.TelePortCore.ItemIndex]);
				cInvenTory.InvenItem[chaPremiumitem.TelePortCore.ItemIndex].Flag = 0;
				cInvenTory.CheckWeight();
				cInvenTory.ReFormInvenItem();

			}

			else
				cMessageBox.ShowMessage(MESSAGE_NOT_ENOUGH_LEVEL);
		}
	}
	return TRUE;
}

int cHaPremiumItem::CheckWindowOpen()
{
	haCastleSkillUseFlag = 0;
	if(cWareHouse.OpenFlag)return false;
	if(cTrade.OpenFlag)return false;
	if(cAging.OpenFlag)return false;
	if(cCraftItem.OpenFlag)return false;
	if(sSinWarpGate.ShowFlag)return false;
	if(cSinHelp.NPCTeleportFlag)return false;
	if(isDrawClanMenu)return false;
	if(haMovieFlag)return false;
	if (WarMode.fOpen)return false;
	
	if(SmeltingItem.OpenFlag)
	{
		return false;
	}

	if(ManufactureItem.m_OpenFlag)
	{
		return false;
	}
	if(cMixtureReset.OpenFlag)	return false;
	if(cSkinChange.OpenFlag)	return false;
	return true;
}

int cHaPremiumItem::CheckUpKeepItem()
{
	int i;

	for(i = 0; i < SINUPKEEPITEM_MAX; i++)
	{
		if(UpKeepItem[i].Visible == false)
		{
			if(UpKeepItem[i + 1].Visible == true)
			{
				memcpy(&UpKeepItem[i], &UpKeepItem[i + 1], sizeof(sUpKeepItem));
				UpKeepItem[i + 1].Visible = false;
				break;
			}
		}
	}
	return true;
}

int cHaPremiumItem::SetUpKeepItem(int kind, DWORD dwTime, bool visible, char* id, int Percent, short ItemType)
{
	haCastleSkillUseFlag = 0;
	int i;

	for(i = 0; i < SINUPKEEPITEM_MAX; i++)
	{
		if(m_VampiricCuspidEXTime > 0 && UpKeepItem[i].Visible == false)
		{
			if(UpKeepItem[i].TGAImageNumber == VAMPIRIC_CUS_EX)
			{
				UpKeepItem[i].TGAImageNumber = 0;
				UpKeepItem[i].Visible = false;
				haCastleSkillUseFlag = 0;

			}
		}

		if((UpKeepItem[i].TGAImageNumber == VAMPIRIC_CUS_EX || UpKeepItem[i].TGAImageNumber == VAMPIRIC_CUS) && (m_VampiricCuspidEXTime > 0 || m_VampiricCuspidTime > 0))
		{
			if(kind == 3 || kind == 12)
			{
				cMessageBox.ShowMessage(MESSAGE_ASATTRIBUTE_USE);
				haCastleSkillUseFlag = 0;
				return  FALSE;
			}
		}
	}



	for(i = 0; i < SINUPKEEPITEM_MAX; i++)
	{
		if(UpKeepItem[i].Visible == false)
		{

			UpKeepItem[i].TGAImageNumber = kind;
			UpKeepItem[i].Visible = true;
			UpKeepItem[i].IconTime[0] = 0;
			UpKeepItem[i].IconTime[1] = dwTime;
			UpKeepItem[i].Alpha = 0;
			UpKeepItem[i].Alphakind = 0;
			UpKeepItem[i].Per = Percent;

			strcpy(UpKeepItem[i].Id, id);
			haCastleSkillUseFlag = 1;
			return TRUE;
		}
		else
		{




			switch(ItemType)
			{
			case 0:
				if(UpKeepItem[i].TGAImageNumber == kind)
				{
					cMessageBox.ShowMessage(MESSAGE_CLANSKILL_USE);
					haCastleSkillUseFlag = 0;
					return FALSE;
				}

				break;
			case 1:
				for(int j = 0; j < SINUPKEEPITEM_MAX; j++)
				{
					if(UpKeepItem[j].TGAImageNumber == THIRD_EYES || UpKeepItem[j].TGAImageNumber == EXPUP_POTION)
					{
						cMessageBox.ShowMessage(MESSAGE_CLANSKILL_USE);
						haCastleSkillUseFlag = 0;
						return  FALSE;
					}
				}
				break;
			case 2:
				for(int j = 0; j < SINUPKEEPITEM_MAX; j++)
				{
					if(UpKeepItem[j].TGAImageNumber == MANA_REDUCE_P || UpKeepItem[j].TGAImageNumber == EXPUP_POTION)
					{
						cMessageBox.ShowMessage(MESSAGE_CLANSKILL_USE);
						haCastleSkillUseFlag = 0;
						return  FALSE;
					}
				}
				break;
			case 3:
				for(int j = 0; j < SINUPKEEPITEM_MAX; j++)
				{


					if(cHelpPet.PetShow == 1)
					{
						cMessageBox.ShowMessage(MESSAGE_ASATTRIBUTE_USE);
						haCastleSkillUseFlag = 0;
						return  FALSE;
					}
				}
				break;
			}
		}
	}
	return TRUE;

}

void cHaPremiumItem::InfoBox(int PoisX, int PosiY, int Width, int Height)
{
	for(int i = 0; i < Width; i++)
	{
		for(int j = 0; j < Height; j++)
		{
			if(i == 0 && j == 0)
				dsDrawTexImage(cItem.MatItemInfoBox_TopLeft, PoisX + (i * 16), PosiY + (j * 16), 16, 16, 255);
			if(j == 0 && i != 0 && i + 1 < Width)
				dsDrawTexImage(cItem.MatItemInfoBox_TopCenter, PoisX + (i * 16), PosiY + (j * 16), 16, 16, 255);
			if(j == 0 && i + 1 == Width)
				dsDrawTexImage(cItem.MatItemInfoBox_TopRight, PoisX + (i * 16), PosiY + (j * 16), 16, 16, 255);
			if(i == 0 && j != 0 && j + 1 != Height)
				dsDrawTexImage(cItem.MatItemInfoBox_Left, PoisX + (i * 16), PosiY + (j * 16), 16, 16, 255);
			if(i != 0 && j != 0 && i + 1 != Width && j + 1 != Height)
				dsDrawTexImage(cItem.MatItemInfoBox_Center, PoisX + (i * 16), PosiY + (j * 16), 16, 16, 255);
			if(i + 1 == Width && j != 0 && j + 1 != Height)
				dsDrawTexImage(cItem.MatItemInfoBox_Right, PoisX + (i * 16), PosiY + (j * 16), 16, 16, 255);
			if(i == 0 && j + 1 == Height)
				dsDrawTexImage(cItem.MatItemInfoBox_BottomLeft, PoisX + (i * 16), PosiY + (j * 16), 16, 16, 255);
			if(i != 0 && j + 1 == Height && i + 1 != Width)
				dsDrawTexImage(cItem.MatItemInfoBox_BottomCenter, PoisX + (i * 16), PosiY + (j * 16), 16, 16, 255);
			if(j + 1 == Height && i + 1 == Width)
				dsDrawTexImage(cItem.MatItemInfoBox_BottomRight, PoisX + (i * 16), PosiY + (j * 16), 16, 16, 255);
		}
	}
}

int cHaPremiumItem::DrawText()
{
	if (InterfaceParty.PartyPosState != PARTY_NONE)
        return FALSE;
	
	HDC	hdc = 0;

    int Line = 0;
	SetFontTextColor( RGB(255, 255, 000)); // TÍTULO DO ITEM PREMIUM, NOME...
	char strBuff[64], strBuff2[64] = { 0 };
	memset(strBuff, 0, sizeof(strBuff));
	int i, cnt = 0;
	for(i = 0; i < SINUPKEEPITEM_MAX; i++)
	{
		if(cSinHelp.sinGetHelpShowFlag())break;
		if(!CheckWindowOpen())break;
		if(UpKeepItem[i].Visible == true)
		{
			if(UpKeepItem[i].Infoflag == true)
			{
                if (i > 6)
                {
                    FontType = 1;
                    dsTextLineOut32( WinSizeX - 92 - 37- 208, UpKeepItem[i].PosiY + 15, UpKeepItem[i].Id, strlen(UpKeepItem[i].Id));
                    FontType = 0;
                }
                else
                {
                    FontType = 1;
                    dsTextLineOut32( WinSizeX - 92 - 37 - 170, (WinSizeY / 2 - 145 + (36 * (i))) + 13, UpKeepItem[i].Id, strlen(UpKeepItem[i].Id));
                    FontType = 0;
                }

				SetFontTextColor( RGB(255, 255, 255)); // PORCENTAGEM DO ITEM PREMIUM
				switch(UpKeepItem[i].TGAImageNumber)
				{
				case nsPremiumItem::THIRD_EYES:
					wsprintf(strBuff, UpKeepItemDoc[1], UpKeepItem[i].Per, "%");
					break;
				case nsPremiumItem::EXPUP_POTION:
					wsprintf(strBuff, UpKeepItemDoc[2], UpKeepItem[i].Per, "%");
					break;
				case nsPremiumItem::VAMPIRIC_CUS:
					wsprintf(strBuff, UpKeepItemDoc[4]);
					break;
				case nsPremiumItem::MANA_RECHAR_P:
					wsprintf(strBuff, UpKeepItemDoc[5]);
					break;
				case nsPremiumItem::MIGHT_OF_AWELL:
					wsprintf(strBuff, UpKeepItemDoc[6], UpKeepItem[i].Per);
					break;
				case nsPremiumItem::MANA_REDUCE_P:
					wsprintf(strBuff, UpKeepItemDoc[7], UpKeepItem[i].Per, "%");
					break;
				case nsPremiumItem::HELP_PET_TERRY:
					wsprintf(strBuff, UpKeepItemDoc[8]);
					break;
				case nsPremiumItem::HELP_PET_NEPSYS:
					wsprintf(strBuff, UpKeepItemDoc[9]);
					break;
				case nsPremiumItem::HELP_PET_IO:
					wsprintf(strBuff, UpKeepItemDoc[10]);
					break;
				case nsPremiumItem::HELP_PET_MUTE:
					wsprintf(strBuff, UpKeepItemDoc[11]);
					break;
				case nsPremiumItem::VAMPIRIC_CUS_EX:
					wsprintf(strBuff, UpKeepItemDoc[12]);
					break;
				case nsPremiumItem::STAMINA_REDUCE_P:
					wsprintf(strBuff, UpKeepItemDoc[13], UpKeepItem[i].Per, "%");
					break;
				case nsPremiumItem::PHENIX_PET:
					wsprintf(strBuff, UpKeepItemDoc[14], "%");
					break;
				case nsPremiumItem::STAMINA_RECHAR_P:
					wsprintf(strBuff, UpKeepItemDoc[15]);
					break;
				case nsPremiumItem::BIGHEAD1:
					wsprintf(strBuff, "+10%% Poder de Ataque");
					break;

				case nsPremiumItem::CARAVAN:
					wsprintf(strBuff, "Sua caravana acompanhará você!");
					break;

					break;
				case nsPremiumItem::GoldLooter:
					wsprintf(strBuff, "Mate os Monstros e Colete o Ouro.");
					break;

				case nsPremiumItem::HP_BOOSTER:
					wsprintf(strBuff, "Adicional HP +15%%");
					break;
				case nsPremiumItem::MP_BOOSTER:
					wsprintf(strBuff, "Adicional MP +15%%");
					break;
				case nsPremiumItem::RES_BOOSTER:
					wsprintf(strBuff, "Adicional RES +15%%");
					break;
				}

				if (i > 6 && i < 14)
                {
                    if (Line > 1)
                    {
                        dsTextLineOut32( WinSizeX  -92 - 37 - 170, (WinSizeY / 2 - 145 + (36 * (i - 7))) + 30, strBuff, strlen(strBuff));
                        dsTextLineOut32( WinSizeX  -92 - 37 - 170, (WinSizeY / 2 - 145 + (36 * (i - 7))) + 30 + 14, strBuff2, strlen(strBuff2));
                    }
                    else
                    {
                        dsTextLineOut32( WinSizeX  -92 - 37 - 170, (WinSizeY / 2 - 145 + (36 * (i - 7))) + 40, strBuff, strlen(strBuff));
                    }
                }
                else if (i > 13)
                {
                    if (Line > 1)
                    {
                        dsTextLineOut32( WinSizeX - 92 - 37 - 170, (WinSizeY / 2 - 145 + (36 * (i - 14))) + 30, strBuff, strlen(strBuff));
                        dsTextLineOut32( WinSizeX - 92 - 37 - 170, (WinSizeY / 2 - 145 + (36 * (i - 7))) + 30 + 14, strBuff2, strlen(strBuff2));
                    }
                    else
                    {
                        dsTextLineOut32( WinSizeX - 92 - 37 - 170, (WinSizeY / 2 - 145 + (36 * (i - 14))) + 40, strBuff, strlen(strBuff));
                    }
                }
                else
                {
                    if (Line > 1)
                    {
                        dsTextLineOut32( WinSizeX - 92 - 37 - 170, (WinSizeY / 2 - 145 + (36 * (i))) + 30, strBuff, strlen(strBuff));
                        dsTextLineOut32( WinSizeX - 92 - 37 - 170, (WinSizeY / 2 - 145 + (36 * (i))) + 30 + 14, strBuff2, strlen(strBuff2));
                    }
                    else
                    {
                        dsTextLineOut32( WinSizeX - 92 - 37 - 170, (WinSizeY / 2 - 145 + (36 * (i))) + 40, strBuff, strlen(strBuff));
                    }
                }

				UINT Days = UpKeepItem[i].IconTime[1] / 60 / 60 / 24;
				UINT Hours = (UpKeepItem[i].IconTime[1] / 60 / 60) % 24;
				UINT Minutes = (UpKeepItem[i].IconTime[1] / 60) % 60;
				UINT Seconds = UpKeepItem[i].IconTime[1] % 60;

				wsprintf(strBuff, "Tempo Restante: %02d:%02d:%02d:%02d", Days, Hours, Minutes, Seconds);

				//SetFontTextColor( RGB(146, 226, 255)); // COR DO TEMPO RESTANTE
				SetFontTextColor(RGB(50, 250, 50));
				dsTextLineOut32( UpKeepItem[i].PosiX + 55, UpKeepItem[i].PosiY + 64, strBuff, strlen(strBuff));
						
				SetFontTextColor(RGB( 50, 250, 50 ) );	
		   		
				if (i > 6 && i < 14)
                {
                    dsTextLineOut32( WinSizeX - 92 - 37 - 170, (WinSizeY / 2 - 145 + (36 * (i - 7))) + 65, strBuff, strlen(strBuff));
                }
                else if (i > 13)
                {
                    dsTextLineOut32( WinSizeX - 92 - 37 - 170, (WinSizeY / 2 - 145 + (36 * (i - 14))) + 65, strBuff, strlen(strBuff));
                }
                else
                {
                    dsTextLineOut32( WinSizeX - 92 - 37 - 170, (WinSizeY / 2 - 145 + (36 * (i))) + 65, strBuff, strlen(strBuff));
                }
			}
		}
	}

	return TRUE;
}

void cHaPremiumItem::UpdatePremiumTime()
{
	for( int j = 0; j < SINUPKEEPITEM_MAX; j++ )
	{
		if( UpKeepItem[j].Visible == true )
		{
			if( UpKeepItem[j].IconTime[0] > 1 )
				UpKeepItem[j].IconTime[0]++;
		}
	}
}

int cHaPremiumItem::UseHairtintPotion(int ItemKind)
{
	switch(ItemKind){
	case 1:
		if(ChangeHairModel(0))
			return TRUE;
		break;
	case 2:
		if(ChangeHairModel(1))
			return TRUE;
		break;
	case 3:
		if(ChangeHairModel(2))
			return TRUE;
		break;
	case 4:
		if(ChangeHairModel(3))
			return TRUE;
		break;
	case 5:
		if(ChangeHairModel(4))
			return TRUE;
		break;
	case 6:
		if(ChangeHairModel(5))
			return TRUE;
		break;
	case 7:
		if(ChangeHairModel(6))
			return TRUE;
		break;
	case 8:
		if(ChangeHairModel(7))
			return TRUE;
		break;

	case 9:
		if(ChangeHairModel(8))
			return TRUE;
		break;
	case 10:
		if(ChangeHairModel(9))
			return TRUE;
		break;
	case 11:
		if(ChangeHairModel(10))
			return TRUE;
		break;
	case 12:
		if(ChangeHairModel(11))
			return TRUE;
		break;
	case 13:
		if(ChangeHairModel(12))
			return TRUE;
		break;
	case 14:
		if(ChangeHairModel(17))
			return TRUE;
		break;
	case 15:
		if(ChangeHairModel(18))
			return TRUE;
		break;
	case 16:
		if(ChangeHairModel(19))
			return TRUE;
		break;
	case 17:
		if(ChangeHairModel(20))
			return TRUE;
		break;
	case 18:
		if(ChangeHairModel(21))
			return TRUE;
		break;
	case 19:
		if (ChangeHairModel(22))
			return TRUE;
		break;
	case 20:
		if (ChangeHairModel(23))
			return TRUE;
		break;
	case 21:
		if (ChangeHairModel(24))
			return TRUE;
		break;
	case 22:
		if (ChangeHairModel(25))
			return TRUE;
		break;
	case 23: // 148
		if (ChangeHairModel(26))
			return TRUE;
		break;
	case 24:
		if (ChangeHairModel(27))
			return TRUE;
		break;
	case 25:
		if (ChangeHairModel(28))
			return TRUE;
		break;
	case 26:
		if (ChangeHairModel(29))
			return TRUE;
		break;
	case 27:
		if (ChangeHairModel(30))
			return TRUE;
		break;
	case 28:
		if (ChangeHairModel(31))
			return TRUE;
		break;

	}
	cMessageBox.ShowMessage(MESSAGE_NOT_HAIRTINTPOTION);
	return FALSE;
}


//Pedra Mature
int cHaPremiumItem::UseAgingMaster(int ItemKind)
{
	switch(ItemKind)
	{
	case 0:
		sinCheckAgingLevel(SIN_AGING_ATTACK, true);
		sinCheckAgingLevel(SIN_AGING_CRITICAL, true);
		ReformCharForm();
		cMessageBox.ShowMessage(MESSAGE_AGING_MATURE_SUCCESS);
		break;
	case 1:
		if((cInvenTory.InvenItem[sInven[1].ItemIndex - 1].CODE &sinITEM_MASK2) == sinDS1)
		{
			sinCheckAgingLevel(SIN_AGING_BLOCK, true);
			ReformCharForm();
			cMessageBox.ShowMessage(MESSAGE_AGING_MATURE_SUCCESS);
		}
		else if((cInvenTory.InvenItem[sInven[1].ItemIndex - 1].CODE &sinITEM_MASK2) == sinOM1)
		{
			sinCheckAgingLevel(SIN_AGING_DEFENSE_ORB, true);
			ReformCharForm();
			cMessageBox.ShowMessage(MESSAGE_AGING_MATURE_SUCCESS);
		}
		break;
	case 2:
		sinCheckAgingLevel(SIN_AGING_DEFENSE_ARMOR, true);
		ReformCharForm();
		cMessageBox.ShowMessage(MESSAGE_AGING_MATURE_SUCCESS);
		break;
	case 3:
		sinCheckAgingLevel(SIN_AGING_DEFENSE_ARMLETS, true);
		ReformCharForm();
		cMessageBox.ShowMessage(MESSAGE_AGING_MATURE_SUCCESS);
		break;
	case 4:
		sinCheckAgingLevel(SIN_AGING_DEFENSE_GLOVES, true);
		ReformCharForm();
		cMessageBox.ShowMessage(MESSAGE_AGING_MATURE_SUCCESS);
		break;
	case 5:
		sinCheckAgingLevel(SIN_AGING_DEFENSE_BOOTS, true);
		ReformCharForm();
		cMessageBox.ShowMessage(MESSAGE_AGING_MATURE_SUCCESS);
		break;
	}

	return TRUE;
}


int cHaPremiumItem::UseSkillMaster(int ItemKind)
{
	switch(ItemKind)
	{
	case 1:
		for(int j = 1; j < 5; j++)
		{
			if(sinSkill.UseSkill[j].UseSkillCount < 10000)
			{
				if(sinSkill.UseSkill[j].Flag && sinSkill.UseSkill[j].Point)
				{
					sinSkill.UseSkill[j].UseSkillCount += 10000;
					cSkill.ReformSkillMasteryForm(0, j);
				}
			}
		}
		cMessageBox.ShowMessage(MESSAGE_SKILL_MATURE_SUCCESS);
		sinPlaySound(SIN_SOUND_EAT_POTION2);
		sinEffect_Agony(lpCurPlayer);
		break;
	case 2:
		for(int j = 5; j < 9; j++)
		{
			if(sinSkill.UseSkill[j].UseSkillCount < 10000)
			{
				if(sinSkill.UseSkill[j].Flag && sinSkill.UseSkill[j].Point)
				{
					sinSkill.UseSkill[j].UseSkillCount += 10000;
					cSkill.ReformSkillMasteryForm(0, j);
				}
			}
		}
		cMessageBox.ShowMessage(MESSAGE_SKILL_MATURE_SUCCESS);
		sinPlaySound(SIN_SOUND_EAT_POTION2);
		sinEffect_Agony(lpCurPlayer);
		break;
	case 3:
		for(int j = 9; j < 13; j++)
		{
			if(sinSkill.UseSkill[j].UseSkillCount < 10000)
			{
				if(sinSkill.UseSkill[j].Flag && sinSkill.UseSkill[j].Point)
				{
					sinSkill.UseSkill[j].UseSkillCount += 10000;
					cSkill.ReformSkillMasteryForm(0, j);
				}
			}
		}
		cMessageBox.ShowMessage(MESSAGE_SKILL_MATURE_SUCCESS);
		sinPlaySound(SIN_SOUND_EAT_POTION2);
		sinEffect_Agony(lpCurPlayer);
		break;
	}

	return TRUE;
}



int cHaPremiumItem::UseMoveShop()
{
	if (cShop.OpenFlag == SIN_CLOSE)
	{
		sinShopKind = 1;
		cShop.OpenFlag = SIN_OPEN;
		cShop.UseItemFlag = 1;
	}
	else
	{
		cShop.OpenFlag = SIN_CLOSE;
		cShop.UseItemFlag = 0;
	}

	return TRUE;
}

int cHaPremiumItem::UseMoveAging()
{
	if (cAging.OpenFlag == SIN_CLOSE)
	{
		cAging.OpenFlag = SIN_OPEN;
	}
	else
	{
		cAging.OpenFlag = SIN_CLOSE;
	}

	return TRUE;
}

int cHaPremiumItem::UseMoveMix()
{
	if (cCraftItem.OpenFlag == SIN_CLOSE)
	{
		cCraftItem.OpenFlag = SIN_OPEN;
	}
	else
	{
		cCraftItem.OpenFlag = SIN_CLOSE;
	}

	return TRUE;
}

int cHaPremiumItem::UseMoveForce()
{
	if (cRestaure.OpenFlag == SIN_CLOSE)
	{
		cRestaure.OpenFlag = SIN_OPEN;
	}
	else
	{
		cRestaure.OpenFlag = SIN_CLOSE;
	}

	return TRUE;
}

int cHaPremiumItem::UseMoveBau()
{
	if (cWareHouse.OpenFlag == SIN_CLOSE)
	{
		cWareHouse.OpenFlag = SIN_OPEN;
	}
	else
	{
		cWareHouse.OpenFlag = SIN_CLOSE;
	}

	return TRUE;
}

int cHaPremiumItem::UseMoveCraft()
{
	if (ManufactureItem.m_OpenFlag == SIN_CLOSE)
	{
		ManufactureItem.m_OpenFlag = SIN_OPEN;
	}
	else
	{
		ManufactureItem.m_OpenFlag = SIN_CLOSE;
	}

	return TRUE;
}

int cHaPremiumItem::UseMoveRuna()
{
	if (SmeltingItem.OpenFlag == SIN_CLOSE)
	{
		SmeltingItem.OpenFlag = SIN_OPEN;
	}
	else
	{
		SmeltingItem.OpenFlag = SIN_CLOSE;
	}

	return TRUE;
}

int cHaPremiumItem::UseMoveClear()
{
	if (cMixtureReset.OpenFlag == SIN_CLOSE)
	{
		cMixtureReset.OpenFlag = SIN_OPEN;
	}
	else
	{
		cMixtureReset.OpenFlag = SIN_CLOSE;
	}

	return TRUE;
}

void cHaPremiumItem::SetItemPremiumTime(int iType, int i)
{
	for (int j = 0; j < SINUPKEEPITEM_MAX; j++)
	{
		if (UpKeepItem[j].Visible == true)
		{
			if (UpKeepItem[j].TGAImageNumber == iType)
			{
				UpKeepItem[j].IconTime[0] = (UpKeepItem[j].IconTime[1]) - (i);
				break;
			}
		}
	}
}



