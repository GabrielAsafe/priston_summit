
#define _SINBARAM_TEST   

#include "compat/legacy/sin/sinLinkHeader.h"
#include "systems/ui/settings/UI.h"
#include "systems/security/anticheat/AntiCheat.h"
#include "systems/combat/damage_display/DrawDamage.h"

smCHAR_INFO *sinChar;
smCHAR_INFO sinCharDisplay;
int sinbaram_Stop = 0;


AntiCheat * pcAntiCheat = NULL;

#include "systems/commerce/coin_shop/CoinShop.h"
#include "systems/commerce/coin_shop/CoinShop_T.h"
#include "systems/social/party/ui/cPartyWindow.h"
#include "systems/commerce/restoration/cRestaureWindow.h"
#include "systems/gameplay/quests/ui/cQuestWindow.h"
#include "systems/ui/targeting/cTarget.h"
#include "systems/gameplay/npc/ui/NPCinfo.h"
#include "systems/combat/pvp/war_mode/WarMode.h"
extern void InitFont();
#include "systems/gameplay/teleport/ui/TCore.h"
#include "systems/gameplay/events/hells_gate/cHellsGate.h"
#include "systems/combat/pvp/kill_notifications/KillHandler.h"
#include "systems/ui/settings/legacy_window/cSettingsWindow.h"
#include "systems/gameplay/loot/roll/CRollHandler.h"

void sinInit()
{
	InitFont();
	InitCursor();
	cItem.Init();
	cSkill.Init();
	cInvenTory.Init();
	cCharStatus.Init();
	cInterFace.Init();
	cShop.Init();
	cTrade.Init();
	cMessageBox.Init();
	cSin3D.Init();
	cSinHelp.Init();
	cSinWarpGate.Init();
	g_CreatePetSystem();
	cHelpPet.Init();
	cSinSod2.Init();
	cCoinShop_T.Init();
	cCoinShop.Init();
	cQuestWindow.Init();
	cRestaure.Load();
	cParty.LoadImages();
	cKill.Init();
	g_PCBang_CreatePetSystem();
	cPCBANGPet.Init();
	cSettings.Init();

	cTarget.Init();
	InitBmpFont();
	chaSiege.init();
	chaQuest.init();
	chaPremiumitem.init();



	void web_DB_start();


	void clanMENU_INIT(int setN);
	clanMENU_INIT(1);
	WarMode.Init();
	//DrawDamage::GetInstance()->Init();
	Rank::GetInstance()->Init();
	cMixGuide::GetInstance()->Init();
	RankGame::GetInstance()->init();
	ClanCustom::GetInstance()->Init();
	NPCinfo::GetInstance()->Init();
	BattlePass::GetInstance()->Init();
	JOBS::GetInstance()->Init();
	TCORE::GetInstance()->init();
}

DWORD dwSinTime = 0;
void sinMain()
{
	//DrawDamage::GetInstance()->UpdateTime();
	//DrawDamage::GetInstance()->CurUpdateTime();

	sinMouseMain();
	sinKeyClickMain();



	if(sinGetMouseButton(0) == TRUE)
		sinProc(SINLBUTTONDOWN);
	if(sinGetMouseButton(1) == TRUE)
		sinProc(SINRBUTTONDOWN);

	if(MouseButtonUpEvent == SINLBUTTONUP)
	{
		sinProc(SINLBUTTONUP);
		MouseButtonUpEvent = 0;

	}

	if(MouseButtonUpEvent == SINRBUTTONUP)
	{
		sinProc(SINRBUTTONUP);
		MouseButtonUpEvent = 0;

	}

	sinProc(SINMOUSEMOVE);


	MainSub();
	cItem.Main();
	cInterFace.Main();
	cSkill.Main();
	cInvenTory.Main();
	cCharStatus.Main();
	cShop.Main();
	cTrade.Main();
	cMessageBox.Main();

	cSinHelp.Main();
	cSinWarpGate.Main();
	cHelpPet.Main();
	cSinSod2.Main();


	chaSiege.Main();
	chaQuest.Main();
	chaPremiumitem.Main();

	void ktj_imsi_clanMENU_Main();
	ktj_imsi_clanMENU_Main();

	ShowInterFace();


	cPCBANGPet.Main();




	if(sinbaram_Stop)
	{
		dwSinTime++;
		if(dwSinTime >= 3 * 70) sinbaram_Stop = 0;
	}
	else dwSinTime = 0;

}

void sinDraw()
{

	DrawSub();
	

	dsDrawOffsetArray = dsARRAY_BOTTOM;

	cCharStatus.Draw();
	
	cCharStatus.DrawCharText();


	dsDrawOffsetArray = dsARRAY_TOP;

	cParty.Draw();

	chaPremiumitem.DrawUp();
	
	cQuestWindow.Draw();

	dsDrawOffsetArray = dsARRAY_BOTTOM;


	
	cSkill.Draw();
	

	dsDrawOffsetArray = dsARRAY_TOP;
	
	cSkill.DrawUp();
	


	dsDrawOffsetArray = dsARRAY_BOTTOM;

	
	cInvenTory.Draw();
	
	cInvenTory.DrawInvenText();

	dsDrawOffsetArray = dsARRAY_TOP;



	
	cShop.Draw();
	
	cShop.DrawShopText();

	cTrade.Draw();
	
	cTrade.DrawTradeText();


	dsDrawOffsetArray = dsARRAY_BOTTOM;

	
	cInterFace.Draw();
	
	cInterFace.DrawInterText();
	cCoinShop_T.Draw();
	cCoinShop.Draw();
	cRestaure.Draw();
	cItem.Draw();
	
	cItem.DrawItemText();


	dsDrawOffsetArray = dsARRAY_TOP;

	
	cSinHelp.Draw();
	
	cSinHelp.DrawText();


	
	cSinWarpGate.Draw();
	
	cSinWarpGate.DrawText();


	sinDrawInterEffect();


	dsDrawOffsetArray = dsARRAY_TOP;
	void ktj_imsi_clanMENU_Draw();
	ktj_imsi_clanMENU_Draw();






	dsDrawOffsetArray = dsARRAY_BOTTOM;

	ROLLHANDLER->Render();

	sinDrawText();

	dsDrawOffsetArray = dsARRAY_TOP;

	
	cHelpPet.Draw();
	
	cHelpPet.DrawText();



	
	cSinSod2.Draw();
	
	cSinSod2.DrawText();


	
	chaSiege.Draw();
	
	chaSiege.DrawText();

	if(lpCurPlayer)
	{
		lpCurPlayer->score.draw();
	}

	
	chaQuest.Draw();
	
	
    chaPremiumitem.DrawUp();
	
     if (InterfaceParty.PartyPosState == PARTY_NONE)
    {
        chaPremiumitem.Draw();
        chaPremiumitem.DrawText();
    }

    dsDrawOffsetArray = dsARRAY_TOP;
	
	if (cCraftItem.OpenFlag && cCraftItem.ForceFlag == 0) {
	    
	}
	
	cHg.Draw();

	cMixGuide::GetInstance()->Draw();

	cMessageBox.Draw();

	cMessageBox.DrawMessageText();

	dsDrawOffsetArray = dsARRAY_TOP;
	if (RankGame::GetInstance()->IsOpen()) {
		RankGame::GetInstance()->Draw();
	}
	WarMode.Draw();
	cSettings.Draw();
	TCORE::GetInstance()->Draw();
	cKill.Draw();
	Rank::GetInstance()->Draw();
	BattlePass::GetInstance()->Draw();
	NPCinfo::GetInstance()->Draw();
	ClanCustom::GetInstance()->Draw();
	JOBS::GetInstance()->Draw();
	
	dsDrawOffset_X = WinSizeX - 800;
    dsDrawOffset_Y = WinSizeY - 600;
    dsDrawOffsetArray = dsARRAY_RIGHT | dsARRAY_BOTTOM;
    dsMenuCursorPos(&pCursorPos, 3);

    DrawInterfaceParty();
    dsMenuCursorPos(&pCursorPos, 0);
    dsDrawOffsetArray = dsARRAY_TOP;
    dsDrawOffset_X = 0;
    dsDrawOffset_Y = 0;
	
	if(d3ddev)
	{
		
		DrawCursor();
		
	}	

}


void sinDrawText()
{

	cSkill.DrawSkillText();
	sinTestDrawText();
	EffectTestDrawText();
	EffectTestDrawText2();


}

void sinProc(int Message)
{

	switch(Message)
	{
		case SINLBUTTONDOWN:
		if(sinbaram_Stop)return;
		
		if (ROLLHANDLER->OnMouseClick(true))
			break;
		
		if (cCoinShop.isOpen())
		{
			if (!cCoinShop.LButtonDown())
			{
				cInvenTory.CheckMousePotionNumForm();
				cInvenTory.LButtonDown(pCursorPos.x, pCursorPos.y);
				cInvenTory.ReFormMousePotionNum();
			}
			break;
		}

		if (cCoinShop_T.isOpen())
		{
			if (!cCoinShop_T.LButtonDown())
			{
				cInvenTory.CheckMousePotionNumForm();
				cInvenTory.LButtonDown(pCursorPos.x, pCursorPos.y);
				cInvenTory.ReFormMousePotionNum();
			}
			break;
		}
		if (cParty.LButtonDown())
			break;

		if (cRestaure.LButtonDown())
			break;
		cTarget.LButtonDown();
		cQuestWindow.LButtonDown();
		cInvenTory.CheckMousePotionNumForm();
		cMessageBox.LButtonDown(pCursorPos.x, pCursorPos.y);
		cSinSod2.LButtonDown(pCursorPos.x, pCursorPos.y);
		if(sinMessageBoxShowFlag && !MyShopExpBox && !nName)return;
		cInterFace.LButtonDown(pCursorPos.x, pCursorPos.y);
		cCharStatus.LButtonDown(pCursorPos.x, pCursorPos.y);
		cInvenTory.LButtonDown(pCursorPos.x, pCursorPos.y);
		cSkill.LButtonDown(pCursorPos.x, pCursorPos.y);
		cTrade.LButtonDown(pCursorPos.x, pCursorPos.y);
		cItem.LButtonDown(pCursorPos.x, pCursorPos.y);
		cShop.LButtonDown(pCursorPos.x, pCursorPos.y);
		cSin3D.LButtonDown(pCursorPos.x, pCursorPos.y);
		cSinHelp.LButtonDown(pCursorPos.x, pCursorPos.y);
		cSinWarpGate.LButtonDown(pCursorPos.x, pCursorPos.y);
		cHelpPet.LButtonDown(pCursorPos.x, pCursorPos.y);
		cInvenTory.ReFormMousePotionNum();


		chaSiege.LButtonDown(pCursorPos.x, pCursorPos.y);
		chaQuest.LButtonDown(pCursorPos.x, pCursorPos.y);
		chaPremiumitem.LButtonDown(pCursorPos.x, pCursorPos.y);
		WarMode.Button();
		Rank::GetInstance()->Button();
		ClanCustom::GetInstance()->Button();
		NPCinfo::GetInstance()->Button();
		BattlePass::GetInstance()->Button();
		JOBS::GetInstance()->Button();
		TCORE::GetInstance()->MouseIn();
		cMixGuide::GetInstance()->Button();
		RankGame::GetInstance()->Button();
		break;
		case SINLBUTTONUP:
		if(sinbaram_Stop)return;
		ROLLHANDLER->OnMouseClick(false);
		cMessageBox.LButtonUp(pCursorPos.x, pCursorPos.y);
		if(sinMessageBoxShowFlag)return;
		if(cParty.RButtonDown())return;
		cInterFace.LButtonUp(pCursorPos.x, pCursorPos.y);
		cCharStatus.LButtonUp(pCursorPos.x, pCursorPos.y);
		cInvenTory.LButtonUp(pCursorPos.x, pCursorPos.y);
		cShop.LButtonUp(pCursorPos.x, pCursorPos.y);
		cSkill.LButtonUp(pCursorPos.x, pCursorPos.y);
		cTrade.LButtonUp(pCursorPos.x, pCursorPos.y);
		cSin3D.LButtonUp(pCursorPos.x, pCursorPos.y);
		cSinHelp.LButtonUp(pCursorPos.x, pCursorPos.y);
		cSinWarpGate.LButtonUp(pCursorPos.x, pCursorPos.y);
		cHelpPet.LButtonUp(pCursorPos.x, pCursorPos.y);
		cSinSod2.LButtonUp(pCursorPos.x, pCursorPos.y);

		chaSiege.LButtonUp(pCursorPos.x, pCursorPos.y);
		break;

		case SINRBUTTONDOWN:
		if(MyShopSendButton)return;
		if(sinbaram_Stop)return;
		cMessageBox.RButtonDown(pCursorPos.x, pCursorPos.y);
		if(sinMessageBoxShowFlag)return;
		cInterFace.RButtonDown(pCursorPos.x, pCursorPos.y);
		cCharStatus.RButtonDown(pCursorPos.x, pCursorPos.y);
		cShop.RButtonDown(pCursorPos.x, pCursorPos.y);
		cInvenTory.RButtonDown(pCursorPos.x, pCursorPos.y);
		cSkill.RButtonDown(pCursorPos.x, pCursorPos.y);
		cTrade.RButtonDown(pCursorPos.x, pCursorPos.y);
		cItem.RButtonDown(pCursorPos.x, pCursorPos.y);
		cSin3D.RButtonDown(pCursorPos.x, pCursorPos.y);
		cSinHelp.RButtonDown(pCursorPos.x, pCursorPos.y);
		cSinWarpGate.RButtonDown(pCursorPos.x, pCursorPos.y);
		cHelpPet.RButtonDown(pCursorPos.x, pCursorPos.y);
		cSinSod2.RButtonDown(pCursorPos.x, pCursorPos.y);

		break;

		case SINRBUTTONUP:
		if(sinbaram_Stop)return;
		cMessageBox.RButtonUp(pCursorPos.x, pCursorPos.y);
		if(sinMessageBoxShowFlag)return;
		cInterFace.RButtonUp(pCursorPos.x, pCursorPos.y);
		cCharStatus.RButtonUp(pCursorPos.x, pCursorPos.y);
		cInvenTory.RButtonUp(pCursorPos.x, pCursorPos.y);
		cShop.RButtonUp(pCursorPos.x, pCursorPos.y);
		cSkill.RButtonUp(pCursorPos.x, pCursorPos.y);
		cTrade.RButtonUp(pCursorPos.x, pCursorPos.y);
		cMessageBox.RButtonUp(pCursorPos.x, pCursorPos.y);
		cSin3D.RButtonUp(pCursorPos.x, pCursorPos.y);
		cSinHelp.RButtonUp(pCursorPos.x, pCursorPos.y);
		cSinWarpGate.RButtonUp(pCursorPos.x, pCursorPos.y);
		cHelpPet.RButtonUp(pCursorPos.x, pCursorPos.y);
		cSinSod2.RButtonUp(pCursorPos.x, pCursorPos.y);
		break;

		case SINMOUSEMOVE:
		ROLLHANDLER->OnMouseMove();
		break;

		case SINKEYDOWN:
		if (cSettings.isOpen())return;
		if(sinbaram_Stop)return;
		cMessageBox.KeyDown();
		if(sinMessageBoxShowFlag)return;

		cInterFace.KeyDown();
		cCharStatus.KeyDown();
		cInvenTory.KeyDown();
		cShop.KeyDown();
		cSkill.KeyDown();
		cTrade.KeyDown();
		cSin3D.KeyDown();
		cSinHelp.KeyDown();
		cSinWarpGate.KeyDown();
		cHelpPet.KeyDown();
		cSinSod2.KeyDown();
		chaSiege.KeyDown();
		cQuestWindow.KeyDown();
		
		if(sinGetKeyClick('X'))
		{
			cHelpPet.PetMessage("*System", 3);
			if(!ExitButtonClick)ExitButtonClick = 1;
			else ExitButtonClick = 0;
			cInterFace.CheckAllBox(SIN_SYSTEM);
		}
		if (sinGetKeyClick('K'))
		{
			if (!WarMode.fOpen)
				WarMode.Open();
			else
				WarMode.Close();
		}

			
		if(sinGetKeyClick('R'))
		{
			cHelpPet.PetMessage("*RunAndWalk", 3);
			if(cInterFace.sInterFlags.RunFlag)cInterFace.sInterFlags.RunFlag = 0;
			else cInterFace.sInterFlags.RunFlag = 1;
		}
		if(sinGetKeyClick(VK_TAB))
		{
			cHelpPet.PetMessage("*Mini_Map", 3);
			if(cInterFace.sInterFlags.MapOnFlag)cInterFace.sInterFlags.MapOnFlag = 0;
			else cInterFace.sInterFlags.MapOnFlag = 1;
		}
		if(sinGetKeyClick('Z'))
		{
			cInterFace.sInterFlags.CameraAutoFlag++;
			if(cInterFace.sInterFlags.CameraAutoFlag > 2) cInterFace.sInterFlags.CameraAutoFlag = 0;
		}
		if(sinGetKeyClick('D'))
		{
			cHelpPet.PetMessage("*Party", 3);
			ShowParty();
		}
		
		if(sinGetKeyClick('Q'))
		{
			cHelpPet.PetMessage("*Quest", 3);
			ShowQuest();
		}
	
		if(sinGetKeyClick(VK_SPACE))
		{
			cMessageBox.CloseMessage();
			if(!cCraftItem.OpenFlag && !cAging.OpenFlag && !SmeltingItem.OpenFlag && !ManufactureItem.m_OpenFlag || !cMixtureReset.OpenFlag || !cSkinChange.OpenFlag)
				cInterFace.CheckAllBox(SIN_ALL_CLOSE);

		}
		if (smConfig.getSecurity() >= AccountTypes::SEC_MODERATOR)
		{
			if (sinGetKeyClick('T') && sinGetKeyClick(VK_CONTROL))
			{
				if (sinTestFlag)sinTestFlag = 0;
				else sinTestFlag = 1;
			}

			if (sinGetKeyClick('F') && sinGetKeyClick(VK_CONTROL))
			{
				if (cShop.OpenFlag == SIN_CLOSE)cShop.OpenFlag = SIN_OPEN;
				else cShop.OpenFlag = SIN_CLOSE;

			}
			if (sinGetKeyClick('G') && sinGetKeyClick(VK_CONTROL))
			{
				if (cCraftItem.OpenFlag == SIN_CLOSE)cCraftItem.OpenFlag = SIN_OPEN;
				else cCraftItem.OpenFlag = SIN_CLOSE;
			}
			if (sinGetKeyClick('H') && sinGetKeyClick(VK_CONTROL))
			{
				if (cAging.OpenFlag == SIN_CLOSE)cAging.OpenFlag = SIN_OPEN;
				else cAging.OpenFlag = SIN_CLOSE;
			}
			if (sinGetKeyClick('J') && sinGetKeyClick(VK_CONTROL))
			{
				if (cMixtureReset.OpenFlag == SIN_CLOSE)cMixtureReset.OpenFlag = SIN_OPEN;
				else cMixtureReset.OpenFlag = SIN_CLOSE;
			}
			/*if (sinGetKeyClick('K') && sinGetKeyClick(VK_CONTROL))
			{
				if (ManufactureItem.m_OpenFlag == SIN_CLOSE)ManufactureItem.m_OpenFlag = SIN_OPEN;
				else ManufactureItem.m_OpenFlag = SIN_CLOSE;
			}
			if (sinGetKeyClick('L') && sinGetKeyClick(VK_CONTROL))
			{
				if (SmeltingItem.OpenFlag == SIN_CLOSE)SmeltingItem.OpenFlag = SIN_OPEN;
				else SmeltingItem.OpenFlag = SIN_CLOSE;
			}*/

		}
#ifdef _SINBARAM_TEST
		sinTestKey();
#endif


		break;

	}

}

void sinClose()
{

	void clanMENU_close();
	clanMENU_close();


	CloseSub();
	cInvenTory.Close();
	cCharStatus.Close();
	cInterFace.Close();
	cItem.Close();
	cSkill.Close();
	cShop.Close();
	cTrade.Close();
	cMessageBox.Close();
	cSin3D.Close();
	cSinHelp.Close();
	cSinWarpGate.Close();
	cHelpPet.Close();
	cSinSod2.Close();

	cPCBANGPet.Close();
}





