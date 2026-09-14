#include "compat/legacy/sin/sinLinkHeader.h"

CCSHOP CList;
CCSHOP_ITEM Cselectitem;
float mm = 1;
//extern void DrawImage(int Image, int x, int y, int w, int h);

void DrawImage(int Image, int x, int y, int w, int h)
{
	dsDrawTexImage(Image, x, y, w - 1, h - 1, 255);
}

void ClanCustom::RecvItemToServer(CCSHOP * Shop)
{
	char Dir[1024];
	CopyMemory(&CList, Shop, sizeof(CCSHOP));

	for (int cnt = 0; cnt < 10; cnt++)
	{
		switch (CList.item[cnt].SubTag)
		{
		case 1:
			wsprintf(Dir, "game//images//Crown//BMP//it%s.bmp", CList.item[cnt].itCode);
			CList.item[cnt].Image = LoadDibSurfaceOffscreen(Dir);
			break;
		}
	}

	for (int cnt2 = 0; cnt2 < (int)(sizeof(CList.item2) / sizeof(CList.item2[0])); cnt2++)
	{
		switch (CList.item2[cnt2].SubTag)
		{
		case 2:
			wsprintf(Dir, "game//images//Crown//TAGS//%s.bmp", CList.item2[cnt2].itCode);
			CList.item2[cnt2].Image = LoadDibSurfaceOffscreen(Dir);
			break;
		}
	}
}

void ClanCustom::SendItemToServer(CCSHOP_ITEM item)
{
	Cselectitem = item;
	cMessageBox.ShowMessage3(MESSAGE_CLAN_CUSTOM, "Deseja Alterar?");

}



bool ClanCustom::MouseAction(int x, int y, int w, int h)
{
	if (pRealCursorPos.x > x && pRealCursorPos.x < x + w && pRealCursorPos.y > y && pRealCursorPos.y < y + h)
		return TRUE;

	return FALSE;
}

void ClanCustom::Open()
{
	fOpen = true;
}

void ClanCustom::Init()
{
	Image[0] = CreateTextureMaterial("game//images//Crown//main.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[1] = CreateTextureMaterial("game//images//Crown//btnclose.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[2] = CreateTextureMaterial("game//images//Crown//btnclose_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[3] = CreateTextureMaterial("game//images//Crown//buy.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[4] = CreateTextureMaterial("game//images//Crown//buy_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[5] = CreateTextureMaterial("game//images//Crown//Page_Left.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[6] = CreateTextureMaterial("game//images//Crown//Page_Left_1.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[7] = CreateTextureMaterial("game//images//Crown//Page_Right.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[8] = CreateTextureMaterial("game//images//Crown//Page_Right_1.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[9] = CreateTextureMaterial("game//images//Crown//coroas.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[10] = CreateTextureMaterial("game//images//Crown//select.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[11] = CreateTextureMaterial("game//images//Crown//tags.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	ReadTextures();
}



void ClanCustom::Draw()
{
	char szBuff[10] = { 0 };
	int WinX = 485, WinY = 70, cnt2 = 0;
	int auxx = 0, auxy = 0;
	char numpage[64];
	char Clan[64];

	if (fOpen)
	{
		DrawImage(Image[0], WinX, WinY, 590, 450);


		if (MouseAction(998, 80, 22, 22))
			DrawImage(Image[2], 998, 80, 22, 22);
		else
			DrawImage(Image[1], 998, 80, 22, 22);

		if (Tag == 1)
		{
			DrawImage(Image[9], WinX, WinY, 590, 450);

			wsprintf(numpage, "%d/2", (SubTag + 1));
			DrawFontText(795, 488, numpage, RGB(255, 255, 255), false);

			/*if (MouseAction(998, 80, 22, 22))
				DrawImage(Image[2], 998, 80, 22, 22);
			else
				DrawImage(Image[1], 998, 80, 22, 22);*/

			if (MouseAction(580, 485, 26, 26))
			{
				DrawImage(Image[6], 580, 485, 26, 26);
			}
			else
			{
				DrawImage(Image[5], 580, 485, 26, 26);
			}

			if (MouseAction(980, 490, 26, 26))
			{
				DrawImage(Image[8], 980, 485, 26, 26);
			}
			else
			{
				DrawImage(Image[7], 980, 485, 26, 26);
			}

			//275 x 170
			//475 x 170
			//275 x 345
			//475 x 345

			if (SubTag == 0)
			{

				for (int cnt = 0; cnt < 4; cnt++)
				{
					switch (cnt)
					{
					case 0:
						auxx = 720;
						auxy = 170;
						break;
					case 1:
						auxx = 920;
						auxy = 170;
						break;
					case 2:
						auxx = 920;
						auxy = 345;
						break;
					case 3:
						auxx = 720;
						auxy = 345;
						break;
					}

					DrawSprite(auxx, auxy, CList.item[cnt].Image, 0, 0, 80, 80);

					if (CList.item[cnt].Name[10] == 0)
					{
						DrawFontText(auxx + 15, auxy - 20, CList.item[cnt].Name, RGB(255, 255, 255), false);
					}
					else
					{
						DrawFontText(auxx - 5, auxy - 20, CList.item[cnt].Name, RGB(255, 255, 255), false);
					}


					if (ClanCustom::PQPCROWN == cnt + 1)
					{
						DrawImage(Image[10], auxx + 15, auxy + 90, 52, 20);
					}
					else
					{
						if (MouseAction(auxx + 15, auxy + 90, 52, 20))
							DrawImage(Image[4], auxx + 15, auxy + 90, 52, 20);
						else
							DrawImage(Image[3], auxx + 15, auxy + 90, 52, 20);
					}
				}
			}
		}

		if (Tag == 2)
		{
			DrawImage(Image[11], WinX, WinY, 590, 450);

			wsprintf(numpage, "%d/3", (SubTag2 + 1));
			DrawFontText(795, 488, numpage, RGB(255, 255, 255), false);

			if (MouseAction(998, 80, 22, 22))
				DrawImage(Image[2], 998, 80, 22, 22);
			else
				DrawImage(Image[1], 998, 80, 22, 22);

			if (MouseAction(580, 485, 26, 26))
			{
				DrawImage(Image[6], 580, 485, 26, 26);
			}
			else
			{
				DrawImage(Image[5], 580, 485, 26, 26);
			}

			if (MouseAction(980, 490, 26, 26))
			{
				DrawImage(Image[8], 980, 485, 26, 26);
			}
			else
			{
				DrawImage(Image[7], 980, 485, 26, 26);
			}

			//250 x 150 /240 / 330 /420
			//350 x 150
			//450 x 150
			//550 x 150

			if (SubTag2 == 0)
			{

				for (int cnt = 0; cnt < 16; cnt++)
				{
					switch (cnt)
					{
					case 0:
						auxx = 695;
						auxy = 156;
						break;
					case 1:
						auxx = 795;
						auxy = 156;
						break;
					case 2:
						auxx = 895;
						auxy = 156;
						break;
					case 3:
						auxx = 995;
						auxy = 156;
						break;
					case 4:
						auxx = 695;
						auxy = 243;
						break;
					case 5:
						auxx = 795;
						auxy = 243;
						break;
					case 6:
						auxx = 895;
						auxy = 243;
						break;
					case 7:
						auxx = 995;
						auxy = 243;
						break;
					case 8:
						auxx = 695;
						auxy = 330;
						break;
					case 9:
						auxx = 795;
						auxy = 330;
						break;
					case 10:
						auxx = 895;
						auxy = 330;
						break;
					case 11:
						auxx = 995;
						auxy = 330;
						break;
					case 12:
						auxx = 695;
						auxy = 418;
						break;
					case 13:
						auxx = 795;
						auxy = 418;
						break;
					case 14:
						auxx = 895;
						auxy = 418;
						break;
					case 15:
						auxx = 995;
						auxy = 418;
						break;
					}


					DrawSprite(auxx, auxy - 10, CList.item2[cnt].Image, 0, 0, 32, 32);

					if (CList.item2[cnt].spec == 1)
					{
						if (CList.item2[cnt].Name[8] == 0)
						{
							wsprintf(Clan, "%s", CList.item2[cnt].Name);
							DrawFontText(auxx, auxy + 30, Clan, RGB(228, 169, 52), false);
						}
						else
						{
							wsprintf(Clan, "%s", CList.item2[cnt].Name);
							DrawFontText(auxx - 10, auxy + 30, Clan, RGB(228, 169, 52), false);
						}

					}
					else
					{
						if (CList.item2[cnt].SubTag == 2)
						{
							if (MouseAction(auxx, auxy + 30, 52, 20))
								DrawImage(Image[4], auxx, auxy + 30, 52, 20);
							else
								DrawImage(Image[3], auxx, auxy + 30, 52, 20);
						}
					}
				}
			}
			else if (SubTag2 == 1)
			{

				for (int cnt = 0; cnt < 16; cnt++)
				{
					switch (cnt)
					{
					case 0:
						auxx = 695;
						auxy = 156;
						break;
					case 1:
						auxx = 795;
						auxy = 156;
						break;
					case 2:
						auxx = 895;
						auxy = 156;
						break;
					case 3:
						auxx = 995;
						auxy = 156;
						break;
					case 4:
						auxx = 695;
						auxy = 243;
						break;
					case 5:
						auxx = 795;
						auxy = 243;
						break;
					case 6:
						auxx = 895;
						auxy = 243;
						break;
					case 7:
						auxx = 995;
						auxy = 243;
						break;
					case 8:
						auxx = 695;
						auxy = 330;
						break;
					case 9:
						auxx = 795;
						auxy = 330;
						break;
					case 10:
						auxx = 895;
						auxy = 330;
						break;
					case 11:
						auxx = 995;
						auxy = 330;
						break;
					case 12:
						auxx = 695;
						auxy = 418;
						break;
					case 13:
						auxx = 795;
						auxy = 418;
						break;
					case 14:
						auxx = 895;
						auxy = 418;
						break;
					case 15:
						auxx = 995;
						auxy = 418;
						break;
					}

					DrawSprite(auxx, auxy - 10, CList.item2[cnt + 16].Image, 0, 0, 32, 32);

					if (CList.item2[cnt + 16].spec == 1)
					{
						if (CList.item2[cnt + 16].Name[8] == 0)
						{
							wsprintf(Clan, "%s", CList.item2[cnt + 16].Name);
							DrawFontText(auxx, auxy + 30, Clan, RGB(228, 169, 52), false);
						}
						else
						{
							wsprintf(Clan, "%s", CList.item2[cnt + 16].Name);
							DrawFontText(auxx - 10, auxy + 30, Clan, RGB(228, 169, 52), false);
						}
					}
					else
					{
						if (CList.item2[cnt + 16].SubTag == 2)
						{
							if (MouseAction(auxx, auxy + 30, 52, 20))
								DrawImage(Image[4], auxx, auxy + 30, 52, 20);
							else
								DrawImage(Image[3], auxx, auxy + 30, 52, 20);
						}
					}
				}
			}
			else if (SubTag2 == 2)
			{

				for (int cnt = 0; cnt < 16; cnt++)
				{
					switch (cnt)
					{
					case 0:
						auxx = 695;
						auxy = 156;
						break;
					case 1:
						auxx = 795;
						auxy = 156;
						break;
					case 2:
						auxx = 895;
						auxy = 156;
						break;
					case 3:
						auxx = 995;
						auxy = 156;
						break;
					case 4:
						auxx = 695;
						auxy = 243;
						break;
					case 5:
						auxx = 795;
						auxy = 243;
						break;
					case 6:
						auxx = 895;
						auxy = 243;
						break;
					case 7:
						auxx = 995;
						auxy = 243;
						break;
					case 8:
						auxx = 695;
						auxy = 330;
						break;
					case 9:
						auxx = 795;
						auxy = 330;
						break;
					case 10:
						auxx = 895;
						auxy = 330;
						break;
					case 11:
						auxx = 995;
						auxy = 330;
						break;
					case 12:
						auxx = 695;
						auxy = 418;
						break;
					case 13:
						auxx = 795;
						auxy = 418;
						break;
					case 14:
						auxx = 895;
						auxy = 418;
						break;
					case 15:
						auxx = 995;
						auxy = 418;
						break;
					}

					DrawSprite(auxx, auxy - 10, CList.item2[cnt + 32].Image, 0, 0, 32, 32);

					if (CList.item2[cnt + 32].spec == 1)
					{
						if (CList.item2[cnt + 32].Name[8] == 0)
						{
							wsprintf(Clan, "%s", CList.item2[cnt + 32].Name);
							DrawFontText(auxx, auxy + 30, Clan, RGB(228, 169, 52), false);
						}
						else
						{
							wsprintf(Clan, "%s", CList.item2[cnt + 32].Name);
							DrawFontText(auxx - 10, auxy + 30, Clan, RGB(228, 169, 52), false);
						}
					}
					else
					{
						if (CList.item2[cnt + 32].SubTag == 2)
						{
							if (MouseAction(auxx, auxy + 30, 52, 20))
								DrawImage(Image[4], auxx, auxy + 30, 52, 20);
							else
								DrawImage(Image[3], auxx, auxy + 30, 52, 20);
						}
					}
				}
			}
		}
	}
}

void ClanCustom::Close()
{
	fOpen = false;
	Tag = 0;
	SubTag = 0;
	SubTag2 = 0;
}

void ClanCustom::Button()
{
	int auxx = 0, auxy = 0;

	if (fOpen)
	{
		int WinX = 40, WinY = 70, cnt2 = 0;

		if (MouseAction(510, 140, 120, 35))
			Tag = 1;

		if (MouseAction(510, 195, 120, 35))
			Tag = 2;

		if (MouseAction(998, 80, 22, 22))
			Close();

		if (Tag == 1)
		{
			if (MouseAction(580, 485, 26, 26))
			{
				if (SubTag <= 0)
				{
					SubTag = 0;
				}
				else
				{
					SubTag--;
				}
			}

			if (MouseAction(980, 485, 26, 26))
			{
				if (SubTag >= 1)
				{
				}
				else
				{
					SubTag++;
				}
			}
			//coroas
			if (SubTag == 0)
			{
				if (MouseAction(720 + 15, 170 + 90, 52, 20))
				{
					if (ClanCustom::PQPCROWN == 1)
					{
					}
					else
					{
						SendItemToServer(CList.item[0]);
					}
				}
				if (MouseAction(920 + 15, 170 + 90, 52, 20))
				{
					if (ClanCustom::PQPCROWN == 2)
					{
					}
					else
					{
						SendItemToServer(CList.item[1]);
					}
				}
				if (MouseAction(720 + 15, 345 + 90, 52, 20))
				{
					if (ClanCustom::PQPCROWN == 3)
					{
					}
					else
					{
						SendItemToServer(CList.item[2]);
					}
				}
				if (MouseAction(920 + 15, 345 + 90, 52, 20))
				{
					if (ClanCustom::PQPCROWN == 4)
					{
					}
					else
					{
						SendItemToServer(CList.item[3]);
					}
				}
			}
		}

		if (Tag == 2)
		{
			if (MouseAction(580, 485, 26, 26))
			{
				if (SubTag2 <= 0)
				{
					SubTag2 = 0;
				}
				else
				{
					SubTag2--;
				}
			}

			if (MouseAction(980, 485, 26, 26))
			{
				if (SubTag2 >= 2)
				{
				}
				else
				{
					SubTag2++;
				}
			}

			if (SubTag2 == 0)
			{

				for (int cnt = 0; cnt < 16; cnt++)
				{
					switch (cnt)
					{
					case 0:
						auxx = 695;
						auxy = 156;
						break;
					case 1:
						auxx = 795;
						auxy = 156;
						break;
					case 2:
						auxx = 895;
						auxy = 156;
						break;
					case 3:
						auxx = 995;
						auxy = 156;
						break;
					case 4:
						auxx = 695;
						auxy = 243;
						break;
					case 5:
						auxx = 795;
						auxy = 243;
						break;
					case 6:
						auxx = 895;
						auxy = 243;
						break;
					case 7:
						auxx = 995;
						auxy = 243;
						break;
					case 8:
						auxx = 695;
						auxy = 330;
						break;
					case 9:
						auxx = 795;
						auxy = 330;
						break;
					case 10:
						auxx = 895;
						auxy = 330;
						break;
					case 11:
						auxx = 995;
						auxy = 330;
						break;
					case 12:
						auxx = 695;
						auxy = 418;
						break;
					case 13:
						auxx = 795;
						auxy = 418;
						break;
					case 14:
						auxx = 895;
						auxy = 418;
						break;
					case 15:
						auxx = 995;
						auxy = 418;
						break;
					}

					if (MouseAction(auxx, auxy + 30, 52, 20))
					{
						if (CList.item2[cnt].spec == 1)
						{
						}
						else if ((CList.item2[cnt].SubTag == 2))
						{
							SendItemToServer(CList.item2[cnt]);
						}
					}
				}
			}
			else if (SubTag2 == 1)
			{

				for (int cnt = 0; cnt < 16; cnt++)
				{
					switch (cnt)
					{
					case 0:
						auxx = 695;
						auxy = 156;
						break;
					case 1:
						auxx = 795;
						auxy = 156;
						break;
					case 2:
						auxx = 895;
						auxy = 156;
						break;
					case 3:
						auxx = 995;
						auxy = 156;
						break;
					case 4:
						auxx = 695;
						auxy = 243;
						break;
					case 5:
						auxx = 795;
						auxy = 243;
						break;
					case 6:
						auxx = 895;
						auxy = 243;
						break;
					case 7:
						auxx = 995;
						auxy = 243;
						break;
					case 8:
						auxx = 695;
						auxy = 330;
						break;
					case 9:
						auxx = 795;
						auxy = 330;
						break;
					case 10:
						auxx = 895;
						auxy = 330;
						break;
					case 11:
						auxx = 995;
						auxy = 330;
						break;
					case 12:
						auxx = 695;
						auxy = 418;
						break;
					case 13:
						auxx = 795;
						auxy = 418;
						break;
					case 14:
						auxx = 895;
						auxy = 418;
						break;
					case 15:
						auxx = 995;
						auxy = 418;
						break;
					}

					if (MouseAction(auxx, auxy + 30, 52, 20))
					{
						if (CList.item2[cnt + 16].spec == 1)
						{
						}
						else if ((CList.item2[cnt + 16].SubTag == 2))
						{
							SendItemToServer(CList.item2[cnt + 16]);
						}
					}
				}
			}
			else if (SubTag2 == 2)
			{

				for (int cnt = 0; cnt < 16; cnt++)
				{
					switch (cnt)
					{
					case 0:
						auxx = 695;
						auxy = 156;
						break;
					case 1:
						auxx = 795;
						auxy = 156;
						break;
					case 2:
						auxx = 895;
						auxy = 156;
						break;
					case 3:
						auxx = 995;
						auxy = 156;
						break;
					case 4:
						auxx = 695;
						auxy = 243;
						break;
					case 5:
						auxx = 795;
						auxy = 243;
						break;
					case 6:
						auxx = 895;
						auxy = 243;
						break;
					case 7:
						auxx = 995;
						auxy = 243;
						break;
					case 8:
						auxx = 695;
						auxy = 330;
						break;
					case 9:
						auxx = 795;
						auxy = 330;
						break;
					case 10:
						auxx = 895;
						auxy = 330;
						break;
					case 11:
						auxx = 995;
						auxy = 330;
						break;
					case 12:
						auxx = 695;
						auxy = 418;
						break;
					case 13:
						auxx = 795;
						auxy = 418;
						break;
					case 14:
						auxx = 895;
						auxy = 418;
						break;
					case 15:
						auxx = 995;
						auxy = 418;
						break;
					}

					if (MouseAction(auxx, auxy + 30, 52, 20))
					{
						if (CList.item2[cnt + 32].spec == 1)
						{
						}
						else if ((CList.item2[cnt + 32].SubTag == 2))
						{
							SendItemToServer(CList.item2[cnt + 32]);
						}
					}
				}
			}

		}

	}
}







