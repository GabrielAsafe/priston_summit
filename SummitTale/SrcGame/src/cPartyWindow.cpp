#include "opcodes.h"
#include "sinbaram/sinLinkHeader.h"
#include "cPartyWindow.h"
#include "CoinShop.h"
#include "CoinShop_T.h"
#include "cQuestWindow.h"

cPARTY cParty;

extern bool isMousePos(int x, int y, int w, int h);

void cPARTY::LoadImages()
{
	MemberCount = 0;
	bExpand = true;
	MenuFlag = 0;

	mClass[0][0] = CreateTextureMaterial("Game\\Party\\Class\\Fighter.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	mClass[0][1] = CreateTextureMaterial("Game\\Party\\Class\\Mechanician.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	mClass[0][2] = CreateTextureMaterial("Game\\Party\\Class\\Archer.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	mClass[0][3] = CreateTextureMaterial("Game\\Party\\Class\\Pikeman.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	mClass[0][4] = CreateTextureMaterial("Game\\Party\\Class\\Atalanta.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	mClass[0][5] = CreateTextureMaterial("Game\\Party\\Class\\Knight.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	mClass[0][6] = CreateTextureMaterial("Game\\Party\\Class\\Magician.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	mClass[0][7] = CreateTextureMaterial("Game\\Party\\Class\\Priestess.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);

	mClass[1][0] = CreateTextureMaterial("Game\\Party\\Class\\Fighter_.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	mClass[1][1] = CreateTextureMaterial("Game\\Party\\Class\\Mechanician_.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	mClass[1][2] = CreateTextureMaterial("Game\\Party\\Class\\Archer_.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	mClass[1][3] = CreateTextureMaterial("Game\\Party\\Class\\Pikeman_.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	mClass[1][4] = CreateTextureMaterial("Game\\Party\\Class\\Atalanta_.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	mClass[1][5] = CreateTextureMaterial("Game\\Party\\Class\\Knight_.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	mClass[1][6] = CreateTextureMaterial("Game\\Party\\Class\\Magician_.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	mClass[1][7] = CreateTextureMaterial("Game\\Party\\Class\\Priestess_.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);

	mMembers[0] = CreateTextureMaterial("Game\\Party\\Members\\member1.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	mMembers[1] = CreateTextureMaterial("Game\\Party\\Members\\member2.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	mMembers[2] = CreateTextureMaterial("Game\\Party\\Members\\member3.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	mMembers[3] = CreateTextureMaterial("Game\\Party\\Members\\member4.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	mMembers[4] = CreateTextureMaterial("Game\\Party\\Members\\member5.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	mMembers[5] = CreateTextureMaterial("Game\\Party\\Members\\member6.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);

	mDefault[DEFAULT::BACKGROUND] = CreateTextureMaterial("Game\\Party\\background.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	mDefault[DEFAULT::EXPAND] = CreateTextureMaterial("Game\\Party\\expand.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	mDefault[DEFAULT::EXPAND_] = CreateTextureMaterial("Game\\Party\\expand_.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	mDefault[DEFAULT::LEADER] = CreateTextureMaterial("Game\\Party\\leader.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);

	LifeBar = CreateTextureMaterial("Game\\Party\\lifebar.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	ManaBar = CreateTextureMaterial("Game\\Party\\manabar.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);

	mMenu[MENU::TOP] = CreateTextureMaterial("Game\\Party\\Menu\\top.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	mMenu[MENU::MID] = CreateTextureMaterial("Game\\Party\\Menu\\mid.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	mMenu[MENU::BOT] = CreateTextureMaterial("Game\\Party\\Menu\\bot.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	mMenu[MENU::MAIN] = CreateTextureMaterial("Game\\Party\\Menu\\main.tga", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);

}

void cPARTY::Draw()
{
	if (!MemberCount)
		return;

	BaseY = ((WinSizeY - 90) - (50 * MemberCount)) / 2;

	dsDrawTexImage(mDefault[bExpand ? DEFAULT::EXPAND : DEFAULT::EXPAND_], 5, BaseY - 30, 19, 15, 255);

	if (!bExpand)
		return;

	for (int i = 0; i < MemberCount; i++)
	{
		int iY = BaseY + (i * 50);

		dsDrawTexImage(mDefault[DEFAULT::BACKGROUND], 5, iY, 174, 50, 255);

		if (i == 0)
			dsDrawTexImage(mDefault[DEFAULT::LEADER], 18, iY - 10, 22, 15, 255);

		dsDrawTexImage(mMembers[i], 39, iY + 40, 10, 10, 255);

		dsDrawTexImage(mClass[Members[i].Life[0] > 0 ? 0 : 1][Members[i].JobCode - 1], 11, iY + 8, 35, 35, 255);

		dsDrawTexImage(LifeBar, 51, iY + 26, (Members[i].Life[0] * 120) / Members[i].Life[1], 7, 255);

		dsDrawTexImage(ManaBar, 47, iY + 36, (Members[i].Mana[0] * 124) / Members[i].Mana[1], 6, 255);

		char buff[32];
		wsprintf(buff, "%d", Members[i].Level);

		SetFontTextColor( RGB(255, 238, 100));
		dsTextLineOut(50, iY + 8, buff, lstrlen(buff));
		SetFontTextColor( RGB(255, 255, 255));
		dsTextLineOut(74, iY + 8, Members[i].szName, lstrlen(Members[i].szName));

		if (MenuFlag && MenuFlag == i + 1)
		{
			if (menu.vOption.size() == 1)
			{
				dsDrawTexImage(mMenu[MENU::MAIN], menu.iX, menu.iY, 96, 31, 255);

				if (menu.vOption.size())
				{
					for (auto& opt : menu.vOption)
					{
						if (opt)
						{
							if (isMousePos(opt->iX, opt->iY, opt->w, opt->h))
								SetFontTextColor(opt->hoverColor);
							else
								SetFontTextColor(opt->textColor);

							dsTextLineOut(opt->iX, opt->iY + 3, opt->szText, lstrlen(opt->szText));
						}
					}
				}
			}
			else
			{
				dsDrawTexImage(mMenu[MENU::TOP], menu.iX, menu.iY, 96, 25, 255);

				for (int i = 0; i < menu.vOption.size() - 2; i++)
				{
					dsDrawTexImage(mMenu[MENU::MID], menu.iX, menu.iY + 24 + (24 * i), 96, 25, 255);
				}

				dsDrawTexImage(mMenu[MENU::BOT], menu.iX, menu.iY + 24 + (24 * (menu.vOption.size() - 2)), 96, 26, 255);

				if (menu.vOption.size())
				{
					for (auto& opt : menu.vOption)
					{
						if (opt)
						{
							if (isMousePos(opt->iX, opt->iY, opt->w, opt->h))
								SetFontTextColor(opt->hoverColor);
							else
								SetFontTextColor(opt->textColor);

							dsTextLineOut(opt->iX, opt->iY, opt->szText, lstrlen(opt->szText));
						}
					}
				}
			}


		}
	}
}

void cPARTY::AddMember(_PARTY_USER * pUser, int MemberNum)
{
	if (MemberNum > 5)
		return;

	if (MemberNum == 0)
	{
		ZeroMemory(Members, sizeof(_PARTY_USER) * 6);
		MemberCount = 0;
		if (pUser->dwObjectSerial == lpCurPlayer->dwObjectSerial)
			LeaderFlag = true;
		else
			LeaderFlag = false;
	}

	memcpy(&Members[MemberNum], pUser, sizeof(_PARTY_USER));

	if (MemberNum >= MemberCount)
	{
		MemberCount = MemberNum + 1;
	}
}

void cPARTY::UpdateMember(_PARTY_PLAYINFO * pUser)
{

	for (int i = 0; i < MemberCount; i++)
	{
		if (Members[i].dwObjectSerial == pUser->dwObjectSerial)
		{
			if (i == 0)
			{
				if (pUser->dwObjectSerial == lpCurPlayer->dwObjectSerial)
					LeaderFlag = true;
				else
					LeaderFlag = false;
			}

			Members[i].Life[0] = pUser->Life[0];
			Members[i].Life[1] = pUser->Life[1];
			Members[i].Mana[0] = pUser->Mana[0];
			Members[i].Mana[1] = pUser->Mana[1];
			Members[i].Level = pUser->Level;
			Members[i].x = pUser->x;
			Members[i].z = pUser->z;
			break;
		}
	}
}

bool cPARTY::LButtonDown()
{
	if (!MemberCount)
		return false;

	BaseY = ((WinSizeY - 90) - (50 * MemberCount)) / 2;

	if (isMousePos(5, BaseY - 30, 18, 14))
	{
		bExpand = !bExpand;
		return true;
	}

	if (!bExpand)
		return false;

	if (MenuFlag)
	{
		smTRANS_COMMAND smTransCommand = {};

		for (auto& opt : menu.vOption)
		{
			if (opt)
			{
				if (isMousePos(opt->iX, opt->iY, opt->w, opt->h))
				{
					smTransCommand.size = sizeof(smTRANS_COMMAND);
					smTransCommand.code = OpCode::OPCODE_PARTY_COMMAND;
					smTransCommand.WParam = Members[MenuFlag - 1].dwObjectSerial;
					smTransCommand.LParam = opt->ButtonNum;
					smTransCommand.SParam = 0;

					smWsockServer->Send((char*)&smTransCommand, smTransCommand.size, TRUE);
				}

				delete opt;
			}
		}

		menu.vOption.clear();
		MenuFlag = 0;
	}


	return false;
}

bool cPARTY::RButtonDown()
{
	if (!MemberCount || cCraftItem.OpenFlag || cAging.OpenFlag || cTrade.OpenFlag || cMixtureReset.OpenFlag || ManufactureItem.m_OpenFlag ||
		cWareHouse.OpenFlag || SmeltingItem.OpenFlag || cShop.OpenFlag || cCoinShop.isOpen() || cCoinShop_T.isOpen() || cQuestWindow.OpenFlag)
		return false;

	BaseY = ((WinSizeY - 90) - (50 * MemberCount)) / 2;

	for (int i = 0; i < MemberCount; i++)
	{
		if (isMousePos(5, BaseY + (i * 50), 173, 49) && (Members[i].dwObjectSerial == lpCurPlayer->dwObjectSerial || LeaderFlag))
		{
			MenuFlag = i + 1;

			SetMenu(178, BaseY + (i * 50) + 10, i);

			if (LeaderFlag)
			{
				if (i == 0)
				{
					AddOpt("Sair", 3);
					AddOpt("Separar", 1);
				}
				else
				{
					AddOpt("Delegar", 0);
					AddOpt("Remover", 2);
				}
			}
			else
			{
				AddOpt("Sair", 3);
			}
			return true;
		}
	}
	return false;
}

void cPARTY::Clear()
{
	MemberCount = 0;
	ZeroMemory(Members, sizeof(_PARTY_USER) * 6);
	bExpand = true;
	LeaderFlag = false;
}

int cPARTY::RequestPartyPlayers(smCHAR * lpChar)
{
	if (lpChar)
	{
		lpChar->PartyFlag = FALSE;

		for (int cnt = 0; cnt < MemberCount; cnt++)
		{
			if (Members[cnt].dwObjectSerial == lpChar->dwObjectSerial)
			{
				lpChar->PartyFlag = TRUE;
				return TRUE;
			}
		}
	}
	else
	{

		for (int cnt2 = 0; cnt2 < OTHER_PLAYER_MAX; cnt2++)
		{
			if (chrOtherPlayer[cnt2].Flag && chrOtherPlayer[cnt2].dwObjectSerial)
			{
				chrOtherPlayer[cnt2].PartyFlag = FALSE;
				for (int cnt = 0; cnt < MemberCount; cnt++)
				{
					if (Members[cnt].dwObjectSerial == chrOtherPlayer[cnt2].dwObjectSerial)
					{
						chrOtherPlayer[cnt2].PartyFlag = TRUE;
						break;
					}
				}
			}
		}
	}

	return FALSE;
}

bool cPARTY::StopMotion()
{
	if (isMousePos(5, BaseY - 30, 18, 14))
		return true;

	return false;
}

extern int FieldMapViewSize2;
extern int	MatPartyPos;

bool cPARTY::DrawMapPartyPlayers(int px, int py)
{
	int cnt;
	int	x, y;
	int	size;
	float sz;
	float sx, sy;
	float fx, fy;
	int dist;

	size = FieldMapViewSize2;
	sz = (float)(FieldMapViewSize2 / 64);
	fx = (float)px;
	fy = (float)py;

	for (cnt = 0; cnt < MemberCount; cnt++)
	{
		if (Members[cnt].dwObjectSerial != lpCurPlayer->dwObjectSerial)
		{
			x = Members[cnt].x - lpCurPlayer->pX;
			y = Members[cnt].z - lpCurPlayer->pZ;
			if (abs(x) < size && abs(y) < size)
			{
				sx = fx + (((float)x) / sz);
				sy = fy - (((float)y) / sz);

				x >>= FLOATNS;
				y >>= FLOATNS;
				dist = x * x + y * y;
				if (dist < PARTY_GETTING_DIST2)
					dsDrawTexImageFloat(MatPartyPos, sx - 3, sy - 3, 8, 8, 255, 0);
				else
					dsDrawTexImageFloat(MatPartyPos, sx - 3, sy - 3, 8, 8, D3DCOLOR_RGBA(255, 0, 0, 255), 0);

			}
		}
	}

	return TRUE;
}

bool cPARTY::isMember(DWORD dwObjectSerial)
{
	for (int i = 0; i < 6; i++)
	{
		if (Members[i].dwObjectSerial == dwObjectSerial)
			return true;
	}
	return false;
}

void cPARTY::SetMenu(int x, int y, int memberNum)
{
	menu.iX = x;
	menu.iY = y;
	menu.MemberNum = memberNum;

	for (auto& opt : menu.vOption)
	{
		if (opt)
			delete opt;
	}

	menu.vOption.clear();
}

void cPARTY::AddOpt(char * szText, int buttonNum)
{
	sMENUOPT* opt = new sMENUOPT;

	opt->ButtonNum = buttonNum;
	lstrcpy(opt->szText, szText);

	SIZE size = {};
	GetFontTextExtentPoint(opt->szText, lstrlen(opt->szText), &size);
	opt->w = size.cx;
	opt->h = size.cy;

	opt->iX = menu.iX + 11 + ((83 - opt->w) / 2);
	opt->iY = menu.iY + (24 * menu.vOption.size()) + ((24 - opt->h) / 2);

	opt->textColor = RGB(255, 255, 255);
	opt->hoverColor = RGB(230, 180, 150);

	menu.vOption.push_back(opt);
}


