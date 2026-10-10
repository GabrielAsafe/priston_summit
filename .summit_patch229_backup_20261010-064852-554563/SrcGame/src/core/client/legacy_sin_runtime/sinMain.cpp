
#define _SINBARAM_TEST   

#include "compat/legacy/sin/sinLinkHeader.h"
#include "systems/ui/settings/UI.h"
#include "systems/ui/v2/legacy/LegacyUIV2Bridge.h"
#include "systems/security/anticheat/AntiCheat.h"
#include "systems/combat/damage_display/DrawDamage.h"

smCHAR_INFO *sinChar;
smCHAR_INFO sinCharDisplay;
int sinbaram_Stop = 0;


AntiCheat * pcAntiCheat = NULL;

static bool LegacyInterfaceInputEnabled()
{
    const bool skillNpcFallback = (SkillNpcFlag || SkillMasterFlag) && !ui::v2::legacy::OwnsLegacySkillNpcUI();
    return (!ui::v2::legacy::SuppressLegacyMainHud() || skillNpcFallback || ReStartFlag) &&
           !(ReStartFlag && ui::v2::legacy::OwnsLegacyDeathUI());
}

static bool LegacySkillInterfaceEnabled()
{
    if (!ui::v2::legacy::SuppressLegacyMainHud() || SkillMasterFlag)
        return true;

    // Keep the old skill panel available as a rollback path when CharacterWindow
    // V2 is not owning it, while hiding the always-on L/R HUD slots otherwise.
    return !ui::v2::legacy::SuppressLegacyCharacterPanels() && cSkill.OpenFlag;
}

static bool LegacyInventoryPointerEnabled()
{
    // Mirror the legacy inventory draw condition. The potion belt is part of
    // cINVENTORY and keeps absolute hit boxes even when its old HUD chrome is
    // suppressed. Never let invisible inventory/belt coordinates receive real
    // mouse input. MyShopExpBox remains the one explicit legacy fallback.
    return !ui::v2::legacy::SuppressLegacyCharacterPanels() &&
           (!ui::v2::legacy::SuppressLegacyMainHud() || MyShopExpBox);
}

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
	ui::v2::legacy::Initialize();

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
	ui::v2::legacy::Update();



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
	// Keep legacy skill timers/state alive, but do not let the hidden legacy HUD
	// create hover/tooltip state from its old absolute-coordinate hit boxes.
	if (ui::v2::legacy::SuppressLegacyCharacterPanels() || !LegacySkillInterfaceEnabled())
	{
		const POINT savedCursor = pCursorPos;
		pCursorPos.x = -10000; pCursorPos.y = -10000;
		cSkill.Main();
		pCursorPos = savedCursor;
	}
	else cSkill.Main();
	cInvenTory.Main(LegacyInventoryPointerEnabled());
	cCharStatus.Main();
	// Migrated contextual UIs still need the legacy Main() bookkeeping/timers,
	// but their old absolute-coordinate hover logic must not see the real mouse.
	// Feed an off-screen cursor only for those Main() calls and restore it
	// immediately so no old tooltip/selection state leaks under UI V2.
	if (ui::v2::legacy::OwnsLegacyShopUI())
	{
		const POINT savedCursor = pCursorPos;
		pCursorPos.x = -10000; pCursorPos.y = -10000;
		cShop.Main();
		pCursorPos = savedCursor;
	}
	else cShop.Main();

	if (ui::v2::legacy::OwnsLegacyTradeUI())
	{
		const POINT savedCursor = pCursorPos;
		pCursorPos.x = -10000; pCursorPos.y = -10000;
		cTrade.Main();
		pCursorPos = savedCursor;
	}
	else cTrade.Main();
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

	if (!ui::v2::legacy::SuppressLegacyCharacterPanels())
	{
		cCharStatus.Draw();
		cCharStatus.DrawCharText();
	}


	dsDrawOffsetArray = dsARRAY_TOP;

	// UI V2 owns party presentation; cParty remains the authoritative legacy data backend.
	// cParty.Draw();

	chaPremiumitem.DrawUp();
	
	if (!ui::v2::legacy::OwnsQuestJournalUI())
		cQuestWindow.Draw();

	dsDrawOffsetArray = dsARRAY_BOTTOM;


	
	if (!ui::v2::legacy::SuppressLegacyCharacterPanels() && LegacySkillInterfaceEnabled())
		cSkill.Draw();
	

	dsDrawOffsetArray = dsARRAY_TOP;
	
	// Continuous-skill timers are gameplay status, not legacy HUD chrome.
	// Keep their top-left presentation alive while Player HUD V2 owns the main HUD.
	cSkill.DrawUp();
	


	dsDrawOffsetArray = dsARRAY_BOTTOM;

	
	if (!ui::v2::legacy::SuppressLegacyCharacterPanels() &&
	    (!ui::v2::legacy::SuppressLegacyMainHud() || MyShopExpBox))
	{
		cInvenTory.Draw();
		cInvenTory.DrawInvenText();
	}

	dsDrawOffsetArray = dsARRAY_TOP;



	
	if (!ui::v2::legacy::OwnsLegacyShopUI())
	{
		cShop.Draw();
		cShop.DrawShopText();
	}

	if (!ui::v2::legacy::OwnsLegacyTradeUI())
	{
		cTrade.Draw();
		cTrade.DrawTradeText();
	}


	dsDrawOffsetArray = dsARRAY_BOTTOM;

	
	cInterFace.Draw();
	
	cInterFace.DrawInterText();
	if (!ui::v2::legacy::OwnsLegacyCoinShopUI())
	{
		cCoinShop_T.Draw();
		cCoinShop.Draw();
	}
	if (!ui::v2::legacy::OwnsLegacyRestorationUI()) cRestaure.Draw();
	cItem.Draw();
	
	cItem.DrawItemText();


	dsDrawOffsetArray = dsARRAY_TOP;

	
	if (!ui::v2::legacy::OwnsLegacyHelpUI())
	{
		cSinHelp.Draw();
		cSinHelp.DrawText();
	}


	
	cSinWarpGate.Draw();
	
	cSinWarpGate.DrawText();


	if (!ui::v2::legacy::SuppressLegacyContextEffects()) sinDrawInterEffect();


	dsDrawOffsetArray = dsARRAY_TOP;
	void ktj_imsi_clanMENU_Draw();
	ktj_imsi_clanMENU_Draw();






	dsDrawOffsetArray = dsARRAY_BOTTOM;

	ROLLHANDLER->Render();

	sinDrawText();

	dsDrawOffsetArray = dsARRAY_TOP;

	
	// PATCH 195: HelpPet remains a legacy state/message producer only.
	// LegacyNotificationHud is the sole presentation surface, so never invoke
	// the old pet chrome renderer from the frame loop.
	ui::v2::legacy::ClearLegacyCharacterHelpOverlay();



	
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

	if (!ui::v2::legacy::SuppressLegacyMixGuide()) cMixGuide::GetInstance()->Draw();

	if (!ui::v2::legacy::OwnsLegacyMessage3UI())
	{
		cMessageBox.Draw();
		cMessageBox.DrawMessageText();
	}

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
	
	ui::v2::legacy::Render();

	if(d3ddev)
	{
		
		DrawCursor();
		
	}	

}


void sinDrawText()
{

	if (!ui::v2::legacy::SuppressLegacyCharacterPanels() && LegacySkillInterfaceEnabled())
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
		
		if (cCoinShop.isOpen() && !ui::v2::legacy::OwnsLegacyCoinShopUI())
		{
			if (!cCoinShop.LButtonDown())
			{
				if (LegacyInventoryPointerEnabled())
				{
					cInvenTory.CheckMousePotionNumForm();
					cInvenTory.LButtonDown(pCursorPos.x, pCursorPos.y);
					cInvenTory.ReFormMousePotionNum();
				}
			}
			break;
		}

		if (cCoinShop_T.isOpen() && !ui::v2::legacy::OwnsLegacyCoinShopUI())
		{
			if (!cCoinShop_T.LButtonDown())
			{
				if (LegacyInventoryPointerEnabled())
				{
					cInvenTory.CheckMousePotionNumForm();
					cInvenTory.LButtonDown(pCursorPos.x, pCursorPos.y);
					cInvenTory.ReFormMousePotionNum();
				}
			}
			break;
		}
		// Legacy party HUD input is disabled; SocialWindow/PartyStatusHud own presentation.
		// if (cParty.LButtonDown()) break;

		if (!ui::v2::legacy::OwnsLegacyRestorationUI() && cRestaure.LButtonDown())
			break;
		cTarget.LButtonDown();
		if (!ui::v2::legacy::OwnsQuestJournalUI())
			cQuestWindow.LButtonDown();
		cInvenTory.CheckMousePotionNumForm();
		if (!ui::v2::legacy::OwnsLegacyMessage3UI()) cMessageBox.LButtonDown(pCursorPos.x, pCursorPos.y);
		cSinSod2.LButtonDown(pCursorPos.x, pCursorPos.y);
		if(sinMessageBoxShowFlag && !ui::v2::legacy::OwnsLegacyMessage3UI() && !MyShopExpBox && !nName)return;
		// Legacy modals/commerce controls above keep priority over UI V2.
		// Once they decline the click, the new character window gets first
		// chance before the remaining legacy character panels.
		if (ui::v2::legacy::HandleLegacyMessage(Message))
		{
			cInvenTory.ReFormMousePotionNum();
			return;
		}

		// UI V2 intentionally disables the hidden legacy inventory hit boxes.
		// Keep the one world interaction that a dragged item still needs: when
		// the Inventory tab owns the drag and no V2 surface consumed this click,
		// route it through the original validated ground-drop path.
		if (ui::v2::legacy::OwnsInventory() && MouseItem.Flag)
		{
			cInvenTory.ThrowInvenItemToField(&MouseItem);
			cInvenTory.ReFormMousePotionNum();
			return;
		}

		if (LegacyInterfaceInputEnabled()) cInterFace.LButtonDown(pCursorPos.x, pCursorPos.y);
		if (!ui::v2::legacy::SuppressLegacyCharacterPanels()) cCharStatus.LButtonDown(pCursorPos.x, pCursorPos.y);
		if (LegacyInventoryPointerEnabled()) cInvenTory.LButtonDown(pCursorPos.x, pCursorPos.y);
		if (!ui::v2::legacy::SuppressLegacyCharacterPanels() && LegacySkillInterfaceEnabled()) cSkill.LButtonDown(pCursorPos.x, pCursorPos.y);
		if (!ui::v2::legacy::OwnsLegacyTradeUI()) cTrade.LButtonDown(pCursorPos.x, pCursorPos.y);
		cItem.LButtonDown(pCursorPos.x, pCursorPos.y);
		if (!ui::v2::legacy::OwnsLegacyShopUI()) cShop.LButtonDown(pCursorPos.x, pCursorPos.y);
		cSin3D.LButtonDown(pCursorPos.x, pCursorPos.y);
		if (!ui::v2::legacy::OwnsLegacyHelpUI()) cSinHelp.LButtonDown(pCursorPos.x, pCursorPos.y);
		cSinWarpGate.LButtonDown(pCursorPos.x, pCursorPos.y);
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
		if (!ui::v2::legacy::SuppressLegacyMixGuide()) cMixGuide::GetInstance()->Button();
		RankGame::GetInstance()->Button();
		break;
		case SINLBUTTONUP:
		if(sinbaram_Stop)return;
		ROLLHANDLER->OnMouseClick(false);
		if (!ui::v2::legacy::OwnsLegacyMessage3UI()) cMessageBox.LButtonUp(pCursorPos.x, pCursorPos.y);
		if(sinMessageBoxShowFlag && !ui::v2::legacy::OwnsLegacyMessage3UI())return;
		// Legacy party HUD input is disabled; SocialWindow owns member actions.
		// if(cParty.RButtonDown())return;
		if (ui::v2::legacy::HandleLegacyMessage(Message))
			return;
		if (LegacyInterfaceInputEnabled()) cInterFace.LButtonUp(pCursorPos.x, pCursorPos.y);
		if (!ui::v2::legacy::SuppressLegacyCharacterPanels()) cCharStatus.LButtonUp(pCursorPos.x, pCursorPos.y);
		if (LegacyInventoryPointerEnabled()) cInvenTory.LButtonUp(pCursorPos.x, pCursorPos.y);
		if (!ui::v2::legacy::OwnsLegacyShopUI()) cShop.LButtonUp(pCursorPos.x, pCursorPos.y);
		if (!ui::v2::legacy::SuppressLegacyCharacterPanels() && LegacySkillInterfaceEnabled()) cSkill.LButtonUp(pCursorPos.x, pCursorPos.y);
		if (!ui::v2::legacy::OwnsLegacyTradeUI()) cTrade.LButtonUp(pCursorPos.x, pCursorPos.y);
		cSin3D.LButtonUp(pCursorPos.x, pCursorPos.y);
		if (!ui::v2::legacy::OwnsLegacyHelpUI()) cSinHelp.LButtonUp(pCursorPos.x, pCursorPos.y);
		cSinWarpGate.LButtonUp(pCursorPos.x, pCursorPos.y);
		cSinSod2.LButtonUp(pCursorPos.x, pCursorPos.y);

		chaSiege.LButtonUp(pCursorPos.x, pCursorPos.y);
		break;

		case SINRBUTTONDOWN:
		if(MyShopSendButton)return;
		if(sinbaram_Stop)return;
		if (!ui::v2::legacy::OwnsLegacyMessage3UI()) cMessageBox.RButtonDown(pCursorPos.x, pCursorPos.y);
		if(sinMessageBoxShowFlag && !ui::v2::legacy::OwnsLegacyMessage3UI())return;
		if (ui::v2::legacy::HandleLegacyMessage(Message))
			return;
		if (LegacyInterfaceInputEnabled()) cInterFace.RButtonDown(pCursorPos.x, pCursorPos.y);
		if (!ui::v2::legacy::SuppressLegacyCharacterPanels()) cCharStatus.RButtonDown(pCursorPos.x, pCursorPos.y);
		if (!ui::v2::legacy::OwnsLegacyShopUI()) cShop.RButtonDown(pCursorPos.x, pCursorPos.y);
		if (LegacyInventoryPointerEnabled()) cInvenTory.RButtonDown(pCursorPos.x, pCursorPos.y);
		if (!ui::v2::legacy::SuppressLegacyCharacterPanels() && LegacySkillInterfaceEnabled()) cSkill.RButtonDown(pCursorPos.x, pCursorPos.y);
		if (!ui::v2::legacy::OwnsLegacyTradeUI()) cTrade.RButtonDown(pCursorPos.x, pCursorPos.y);
		cItem.RButtonDown(pCursorPos.x, pCursorPos.y);
		cSin3D.RButtonDown(pCursorPos.x, pCursorPos.y);
		if (!ui::v2::legacy::OwnsLegacyHelpUI()) cSinHelp.RButtonDown(pCursorPos.x, pCursorPos.y);
		cSinWarpGate.RButtonDown(pCursorPos.x, pCursorPos.y);
		cSinSod2.RButtonDown(pCursorPos.x, pCursorPos.y);

		break;

		case SINRBUTTONUP:
		if(sinbaram_Stop)return;
		if (!ui::v2::legacy::OwnsLegacyMessage3UI()) cMessageBox.RButtonUp(pCursorPos.x, pCursorPos.y);
		if(sinMessageBoxShowFlag && !ui::v2::legacy::OwnsLegacyMessage3UI())return;
		if (ui::v2::legacy::HandleLegacyMessage(Message))
			return;
		if (LegacyInterfaceInputEnabled()) cInterFace.RButtonUp(pCursorPos.x, pCursorPos.y);
		if (!ui::v2::legacy::SuppressLegacyCharacterPanels()) cCharStatus.RButtonUp(pCursorPos.x, pCursorPos.y);
		if (LegacyInventoryPointerEnabled()) cInvenTory.RButtonUp(pCursorPos.x, pCursorPos.y);
		if (!ui::v2::legacy::OwnsLegacyShopUI()) cShop.RButtonUp(pCursorPos.x, pCursorPos.y);
		if (!ui::v2::legacy::SuppressLegacyCharacterPanels() && LegacySkillInterfaceEnabled()) cSkill.RButtonUp(pCursorPos.x, pCursorPos.y);
		if (!ui::v2::legacy::OwnsLegacyTradeUI()) cTrade.RButtonUp(pCursorPos.x, pCursorPos.y);
		if (!ui::v2::legacy::OwnsLegacyMessage3UI()) cMessageBox.RButtonUp(pCursorPos.x, pCursorPos.y);
		cSin3D.RButtonUp(pCursorPos.x, pCursorPos.y);
		if (!ui::v2::legacy::OwnsLegacyHelpUI()) cSinHelp.RButtonUp(pCursorPos.x, pCursorPos.y);
		cSinWarpGate.RButtonUp(pCursorPos.x, pCursorPos.y);
		cSinSod2.RButtonUp(pCursorPos.x, pCursorPos.y);
		break;

		case SINMOUSEMOVE:
		ROLLHANDLER->OnMouseMove();
		ui::v2::legacy::HandleLegacyMessage(Message);
		break;

		case SINKEYDOWN:
		if (cSettings.isOpen())return;
		if (ui::v2::legacy::IsSettingsWindowOpen())
		{
			// Match the old settings modal behavior: do not let gameplay/UI
			// shortcuts fire behind the settings window. X and Escape close it.
			if (sinGetKeyClick('X') || sinGetKeyClick(VK_ESCAPE))
				ui::v2::legacy::ToggleSettingsWindow();
			return;
		}
		if(sinbaram_Stop)return;
		if (ui::v2::legacy::HandleModalHotkeys()) return;
		if (!ui::v2::legacy::OwnsLegacyMessage3UI()) cMessageBox.KeyDown();
		if(sinMessageBoxShowFlag && !ui::v2::legacy::OwnsLegacyMessage3UI())return;

		if (LegacyInterfaceInputEnabled()) cInterFace.KeyDown();
		if (!ui::v2::legacy::HandleCharacterHotkeys())
			cCharStatus.KeyDown();
		cInvenTory.KeyDown();
		if (!ui::v2::legacy::OwnsLegacyShopUI()) cShop.KeyDown();
		ui::v2::legacy::HandleSkillShortcutHotkeys();
		cSkill.KeyDown();
		if (!ui::v2::legacy::OwnsLegacyTradeUI()) cTrade.KeyDown();
		cSin3D.KeyDown();
		if (!ui::v2::legacy::OwnsLegacyHelpUI()) cSinHelp.KeyDown();
		cSinWarpGate.KeyDown();
		cSinSod2.KeyDown();
		chaSiege.KeyDown();
		if (!ui::v2::legacy::HandleQuestHotkey())
			cQuestWindow.KeyDown();
		const bool mainHudHotkeyHandled = ui::v2::legacy::HandleMainHudHotkeys();
		
		if(!mainHudHotkeyHandled && sinGetKeyClick('X'))
		{
			cHelpPet.PetMessage("*System", 3);
			if(!ExitButtonClick)ExitButtonClick = 1;
			else ExitButtonClick = 0;
			cInterFace.CheckAllBox(SIN_SYSTEM);
		}
		if (sinGetKeyClick('K') && !VRKeyBuff[VK_CONTROL] && !VRKeyBuff[VK_SHIFT] && !VRKeyBuff[VK_MENU])
		{
			if (!WarMode.fOpen)
				WarMode.Open();
			else
				WarMode.Close();
		}

			
		if(!mainHudHotkeyHandled && sinGetKeyClick('R'))
		{
			cHelpPet.PetMessage("*RunAndWalk", 3);
			if(cInterFace.sInterFlags.RunFlag)cInterFace.sInterFlags.RunFlag = 0;
			else cInterFace.sInterFlags.RunFlag = 1;
		}
		if(!mainHudHotkeyHandled && sinGetKeyClick(VK_TAB))
		{
			cHelpPet.PetMessage("*Mini_Map", 3);
			if(cInterFace.sInterFlags.MapOnFlag)cInterFace.sInterFlags.MapOnFlag = 0;
			else cInterFace.sInterFlags.MapOnFlag = 1;
		}
		if(!mainHudHotkeyHandled && sinGetKeyClick('Z'))
		{
			cInterFace.sInterFlags.CameraAutoFlag++;
			if(cInterFace.sInterFlags.CameraAutoFlag > 2) cInterFace.sInterFlags.CameraAutoFlag = 0;
		}
		if(!mainHudHotkeyHandled && sinGetKeyClick('D'))
		{
			cHelpPet.PetMessage("*Party", 3);
			ShowParty();
		}
		
		if(!mainHudHotkeyHandled && sinGetKeyClick('Q'))
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
			if (sinGetKeyClick('T') && VRKeyBuff[VK_CONTROL] && !VRKeyBuff[VK_SHIFT] && !VRKeyBuff[VK_MENU])
			{
				if (sinTestFlag)sinTestFlag = 0;
				else sinTestFlag = 1;
			}

			if (sinGetKeyClick('F') && VRKeyBuff[VK_CONTROL] && !VRKeyBuff[VK_SHIFT] && !VRKeyBuff[VK_MENU])
			{
				if (cShop.OpenFlag == SIN_CLOSE)cShop.OpenFlag = SIN_OPEN;
				else cShop.OpenFlag = SIN_CLOSE;

			}
			if (sinGetKeyClick('G') && VRKeyBuff[VK_CONTROL] && !VRKeyBuff[VK_SHIFT] && !VRKeyBuff[VK_MENU])
			{
				if (cCraftItem.OpenFlag == SIN_CLOSE)cCraftItem.OpenFlag = SIN_OPEN;
				else cCraftItem.OpenFlag = SIN_CLOSE;
			}
			if (sinGetKeyClick('H') && VRKeyBuff[VK_CONTROL] && !VRKeyBuff[VK_SHIFT] && !VRKeyBuff[VK_MENU])
			{
				if (cAging.OpenFlag == SIN_CLOSE)cAging.OpenFlag = SIN_OPEN;
				else cAging.OpenFlag = SIN_CLOSE;
			}
			if (sinGetKeyClick('J') && VRKeyBuff[VK_CONTROL] && !VRKeyBuff[VK_SHIFT] && !VRKeyBuff[VK_MENU])
			{
				if (cMixtureReset.OpenFlag == SIN_CLOSE)cMixtureReset.OpenFlag = SIN_OPEN;
				else cMixtureReset.OpenFlag = SIN_CLOSE;
			}
			// SUMMIT_COMMAND_HOTKEYS_SAFETY_V17_1: legacy Manufacture/Smelting
			// keyboard toggles stay disabled. These surfaces assume NPC/context state and
			// are not safe to open/close repeatedly from a global hotkey.
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


	ui::v2::legacy::Shutdown();
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





