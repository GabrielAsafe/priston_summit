#include "compat/legacy/sin/sinLinkHeader.h"

Rank_PACK RankList;
Rank_Class Rselectitem;



void Rank::RecvRankclassToGame(Rank_PACK* infos)
{
	CopyMemory(&RankList, infos, sizeof(Rank_PACK));

}

void Rank::SendRankclassToServer(int type, int Job_code)
{
	{

		ranktypevar = type;
		rankjobvar = Job_code;
		Rank_Job packet;
		packet.size = sizeof(Rank_Job);
		packet.type = Rank::GetInstance()->ranktypevar;
		packet.job_code = Rank::GetInstance()->rankjobvar;

		
		if (Rank::GetInstance()->IsOpen())
			packet.code = RANK_PACKET_SEND_RANKCLASS;
		if (smWsockUserServer)
			smWsockUserServer->Send((char*)&packet, packet.size, TRUE);
	}

}

bool Rank::MouseAction(int x, int y, int w, int h)
{
	if (pRealCursorPos.x > x && pRealCursorPos.x < x + w && pRealCursorPos.y > y && pRealCursorPos.y < y + h)
		return TRUE;

	return FALSE;
}

void Rank::Open()
{
	fOpen = true;
}

void Rank::Init()
{
	Image[0] = CreateTextureMaterial("game//Rank//main.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[1] = CreateTextureMaterial("game//Rank//Alllvl.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[2] = CreateTextureMaterial("game//Rank//Allpvp.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[3] = CreateTextureMaterial("game//Rank//lvl1.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[4] = CreateTextureMaterial("game//Rank//lvl2.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[5] = CreateTextureMaterial("game//Rank//lvl3.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[6] = CreateTextureMaterial("game//Rank//lvl4.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[7] = CreateTextureMaterial("game//Rank//lvl5.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[8] = CreateTextureMaterial("game//Rank//lvl6.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[9] = CreateTextureMaterial("game//Rank//lvl7.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[10] = CreateTextureMaterial("game//Rank//lvl8.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[11] = CreateTextureMaterial("game//Rank//pvp1.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[12] = CreateTextureMaterial("game//Rank//pvp2.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[13] = CreateTextureMaterial("game//Rank//pvp3.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[14] = CreateTextureMaterial("game//Rank//pvp4.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[15] = CreateTextureMaterial("game//Rank//pvp5.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[16] = CreateTextureMaterial("game//Rank//pvp6.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[17] = CreateTextureMaterial("game//Rank//pvp7.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[18] = CreateTextureMaterial("game//Rank//pvp8.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[19] = CreateTextureMaterial("game//Rank//t1.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[20] = CreateTextureMaterial("game//Rank//t2.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[21] = CreateTextureMaterial("game//Rank//t3.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[22] = CreateTextureMaterial("game//Rank//t4.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[23] = CreateTextureMaterial("game//Rank//t5.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[24] = CreateTextureMaterial("game//Rank//t6.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[25] = CreateTextureMaterial("game//Rank//t7.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[26] = CreateTextureMaterial("game//Rank//t8.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[27] = CreateTextureMaterial("game//Rank//btnclose.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[28] = CreateTextureMaterial("game//Rank//btnclose_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[29] = CreateTextureMaterial("game//Rank//Page_Left.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[30] = CreateTextureMaterial("game//Rank//Page_Left_1.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[31] = CreateTextureMaterial("game//Rank//Page_Right.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[32] = CreateTextureMaterial("game//Rank//Page_Right_1.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[33] = CreateTextureMaterial("game//Rank//t0.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[34] = CreateTextureMaterial("game//Rank//icon//Fighter.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[35] = CreateTextureMaterial("game//Rank//icon//Mechanician.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[36] = CreateTextureMaterial("game//Rank//icon//Archer.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[37] = CreateTextureMaterial("game//Rank//icon//Pikeman.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[38] = CreateTextureMaterial("game//Rank//icon//Atalanta.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[39] = CreateTextureMaterial("game//Rank//icon//Knight.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[40] = CreateTextureMaterial("game//Rank//icon//Magician.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[41] = CreateTextureMaterial("game//Rank//icon//Priestess_.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	ReadTextures();
}

extern void DrawImage(int Image, int x, int y, int w, int h);

void Rank::Draw()
{
	char szBuff[10] = { 0 };
	int WinX = 40, WinY = 70, cnt2 = 0;
	int auxx = 0, auxy = 0;
	char numpage[64];
	char Clan[64];
	char showrankname[64], showranklvl[64], showrankjob[64], showrankkills[64];


	if (fOpen)
	{
		DrawImage(Image[0], WinX, WinY, 442, 507);

		if (MouseAction(450, 80, 22, 22))
			DrawImage(Image[28], 450, 80, 22, 22);
		else
			DrawImage(Image[27], 450, 80, 22, 22);


		if (Tag == 1)
		{
			DrawImage(Image[1], WinX, WinY, 442, 507);

			if (MouseAction(450, 80, 22, 22))
				DrawImage(Image[28], 450, 80, 22, 22);
			else
				DrawImage(Image[27], 450, 80, 22, 22);


			wsprintf(numpage, "%d/2", (SubTag + 1));
			DrawFontText(340, 552, numpage, RGB(255, 255, 255), false);


			if (MouseAction(225, 542, 26, 26))
			{
				DrawImage(Image[30], 225, 542, 26, 26);
			}
			else
			{
				DrawImage(Image[29], 225, 542, 26, 26);
			}

			if (MouseAction(440, 542, 26, 26))
			{
				DrawImage(Image[32], 440, 542, 26, 26);
			}
			else
			{
				DrawImage(Image[31], 440, 542, 26, 26);
			}



			switch (classtype)
			{
			case 0:
				DrawImage(Image[1], WinX, WinY, 442, 507);

				if (MouseAction(450, 80, 22, 22))
					DrawImage(Image[28], 450, 80, 22, 22);
				else
					DrawImage(Image[27], 450, 80, 22, 22);


				break;
			case 1:
				DrawImage(Image[3], WinX, WinY, 442, 507);


				if (MouseAction(450, 80, 22, 22))
					DrawImage(Image[28], 450, 80, 22, 22);
				else
					DrawImage(Image[27], 450, 80, 22, 22);


				break;
			case 2:
				DrawImage(Image[4], WinX, WinY, 442, 507);


				if (MouseAction(450, 80, 22, 22))
					DrawImage(Image[28], 450, 80, 22, 22);
				else
					DrawImage(Image[27], 450, 80, 22, 22);


				break;
			case 3:
				DrawImage(Image[5], WinX, WinY, 442, 507);


				if (MouseAction(450, 80, 22, 22))
					DrawImage(Image[28], 450, 80, 22, 22);
				else
					DrawImage(Image[27], 450, 80, 22, 22);


				break;
			case 4:
				DrawImage(Image[6], WinX, WinY, 442, 507);


				if (MouseAction(450, 80, 22, 22))
					DrawImage(Image[28], 450, 80, 22, 22);
				else
					DrawImage(Image[27], 450, 80, 22, 22);

				break;
			case 5:
				DrawImage(Image[7], WinX, WinY, 442, 507);

				if (MouseAction(450, 80, 22, 22))
					DrawImage(Image[28], 450, 80, 22, 22);
				else
					DrawImage(Image[27], 450, 80, 22, 22);


				break;
			case 6:
				DrawImage(Image[8], WinX, WinY, 442, 507);


				if (MouseAction(450, 80, 22, 22))
					DrawImage(Image[28], 450, 80, 22, 22);
				else
					DrawImage(Image[27], 450, 80, 22, 22);


				break;
			case 7:
				DrawImage(Image[9], WinX, WinY, 442, 507);


				if (MouseAction(450, 80, 22, 22))
					DrawImage(Image[28], 450, 80, 22, 22);
				else
					DrawImage(Image[27], 450, 80, 22, 22);


				break;
			case 8:
				DrawImage(Image[10], WinX, WinY, 442, 507);


				if (MouseAction(450, 80, 22, 22))
					DrawImage(Image[28], 450, 80, 22, 22);
				else
					DrawImage(Image[27], 450, 80, 22, 22);

				break;
			}

			if (MouseAction(232, 125, 25, 25))
			{
				DrawImage(Image[19], WinX, WinY, 442, 507);
			}
			if (MouseAction(232 + 29, 125, 25, 25))
			{
				DrawImage(Image[20], WinX, WinY, 442, 507);
			}
			if (MouseAction(232 + (29 * 2), 125, 25, 25))
			{
				DrawImage(Image[21], WinX, WinY, 442, 507);
			}
			if (MouseAction(232 + (29 * 3), 125, 25, 25))
			{
				DrawImage(Image[22], WinX, WinY, 442, 507);
			}
			if (MouseAction(232 + (29 * 4), 125, 25, 25))
			{
				DrawImage(Image[23], WinX, WinY, 442, 507);
			}
			if (MouseAction(232 + (29 * 5), 125, 25, 25))
			{
				DrawImage(Image[24], WinX, WinY, 442, 507);
			}
			if (MouseAction(232 + (29 * 6), 125, 25, 25))
			{
				DrawImage(Image[25], WinX, WinY, 442, 507);
			}
			if (MouseAction(232 + (29 * 7), 125, 25, 25))
			{
				DrawImage(Image[26], WinX, WinY, 442, 507);
			}


			if (MouseAction(191, 125, 25, 25))
			{
				DrawImage(Image[33], WinX, WinY, 442, 507);
			}

			for (int j = 0; j <= 8; j++)
			{
				if(!RankList.infos[j].Job_code==0){
					switch (RankList.infos[j].Job_code)
					{
					case 1:
						DrawImage(Image[34], 255, 209 + (j * 34.5), 26, 26);
						break;
					case 2:
						DrawImage(Image[35], 255, 209 + (j * 34.5), 26, 26);
						break;
					case 3:
						DrawImage(Image[36], 255, 209 + (j * 34.5), 26, 26);
						break;
					case 4:
						DrawImage(Image[37], 255, 209 + (j * 34.5), 26, 26);
						break;
					case 5:
						DrawImage(Image[38], 255, 209 + (j * 34.5), 26, 26);
						break;
					case 6:
						DrawImage(Image[39], 255, 209 + (j * 34.5), 26, 26);
						break;
					case 7:
						DrawImage(Image[40], 255, 209 + (j * 34.5), 26, 26);
						break;
					case 8:
						DrawImage(Image[41], 255, 209 + (j * 34.5), 26, 26);
						break;
					default:
							break;
					}

					wsprintf(showrankname, "%s", RankList.infos[j].Name);
					SetFontTextColor( RGB(255, 255, 255));
					dsTextLineOut(330, 212 + (j * 34.5), showrankname, lstrlen(showrankname));

					if (RankList.infos[j].Level != 0)
					{
						wsprintf(showranklvl, "%d", RankList.infos[j].Level);
						SetFontTextColor( RGB(255, 255, 255));
						dsTextLineOut(435, 212 + (j * 34.5), showranklvl, lstrlen(showranklvl));
					}
				}
			}

		}

		if (Tag == 2)
		{
			DrawImage(Image[2], WinX, WinY, 442, 507);

			if (MouseAction(450, 80, 22, 22))
				DrawImage(Image[28], 450, 80, 22, 22);
			else
				DrawImage(Image[27], 450, 80, 22, 22);




			switch (classtype)
			{
			case 0:
				DrawImage(Image[2], WinX, WinY, 442, 507);

				if (MouseAction(450, 80, 22, 22))
					DrawImage(Image[28], 450, 80, 22, 22);
				else
					DrawImage(Image[27], 450, 80, 22, 22);


				break;
			case 1:
				DrawImage(Image[11], WinX, WinY, 442, 507);


				if (MouseAction(450, 80, 22, 22))
					DrawImage(Image[28], 450, 80, 22, 22);
				else
					DrawImage(Image[27], 450, 80, 22, 22);


				break;
			case 2:
				DrawImage(Image[12], WinX, WinY, 442, 507);


				if (MouseAction(450, 80, 22, 22))
					DrawImage(Image[28], 450, 80, 22, 22);
				else
					DrawImage(Image[27], 450, 80, 22, 22);


				break;
			case 3:
				DrawImage(Image[13], WinX, WinY, 442, 507);


				if (MouseAction(450, 80, 22, 22))
					DrawImage(Image[28], 450, 80, 22, 22);
				else
					DrawImage(Image[27], 450, 80, 22, 22);


				break;
			case 4:
				DrawImage(Image[14], WinX, WinY, 442, 507);


				if (MouseAction(450, 80, 22, 22))
					DrawImage(Image[28], 450, 80, 22, 22);
				else
					DrawImage(Image[27], 450, 80, 22, 22);

				break;
			case 5:
				DrawImage(Image[15], WinX, WinY, 442, 507);

				if (MouseAction(450, 80, 22, 22))
					DrawImage(Image[28], 450, 80, 22, 22);
				else
					DrawImage(Image[27], 450, 80, 22, 22);


				break;
			case 6:
				DrawImage(Image[16], WinX, WinY, 442, 507);


				if (MouseAction(450, 80, 22, 22))
					DrawImage(Image[28], 450, 80, 22, 22);
				else
					DrawImage(Image[27], 450, 80, 22, 22);


				break;
			case 7:
				DrawImage(Image[17], WinX, WinY, 442, 507);


				if (MouseAction(450, 80, 22, 22))
					DrawImage(Image[28], 450, 80, 22, 22);
				else
					DrawImage(Image[27], 450, 80, 22, 22);


				break;
			case 8:
				DrawImage(Image[18], WinX, WinY, 442, 507);


				if (MouseAction(450, 80, 22, 22))
					DrawImage(Image[28], 450, 80, 22, 22);
				else
					DrawImage(Image[27], 450, 80, 22, 22);

				break;
			}

			if (MouseAction(232, 125, 25, 25))
			{
				DrawImage(Image[19], WinX, WinY, 442, 507);
			}
			if (MouseAction(232 + 29, 125, 25, 25))
			{
				DrawImage(Image[20], WinX, WinY, 442, 507);
			}
			if (MouseAction(232 + (29 * 2), 125, 25, 25))
			{
				DrawImage(Image[21], WinX, WinY, 442, 507);
			}
			if (MouseAction(232 + (29 * 3), 125, 25, 25))
			{
				DrawImage(Image[22], WinX, WinY, 442, 507);
			}
			if (MouseAction(232 + (29 * 4), 125, 25, 25))
			{
				DrawImage(Image[23], WinX, WinY, 442, 507);
			}
			if (MouseAction(232 + (29 * 5), 125, 25, 25))
			{
				DrawImage(Image[24], WinX, WinY, 442, 507);
			}
			if (MouseAction(232 + (29 * 6), 125, 25, 25))
			{
				DrawImage(Image[25], WinX, WinY, 442, 507);
			}
			if (MouseAction(232 + (29 * 7), 125, 25, 25))
			{
				DrawImage(Image[26], WinX, WinY, 442, 507);
			}


			if (MouseAction(191, 125, 25, 25))
			{
				DrawImage(Image[33], WinX, WinY, 442, 507);
			}

			for (int j = 0; j <= 8; j++)
			{
				switch (RankList.infos[j].Job_code)
				{
				case 1:
					DrawImage(Image[34], 255, 209 + (j * 34.5), 26, 26);
					break;
				case 2:
					DrawImage(Image[35], 255, 209 + (j * 34.5), 26, 26);
					break;
				case 3:
					DrawImage(Image[36], 255, 209 + (j * 34.5), 26, 26);
					break;
				case 4:
					DrawImage(Image[37], 255, 209 + (j * 34.5), 26, 26);
					break;
				case 5:
					DrawImage(Image[38], 255, 209 + (j * 34.5), 26, 26);
					break;
				case 6:
					DrawImage(Image[39], 255, 209 + (j * 34.5), 26, 26);
					break;
				case 7:
					DrawImage(Image[40], 255, 209 + (j * 34.5), 26, 26);
					break;
				case 8:
					DrawImage(Image[41], 255, 209 + (j * 34.5), 26, 26);
					break;
				}


				wsprintf(showrankname, "%s", RankList.infos[j].Name);
				SetFontTextColor( RGB(255, 255, 255));
				dsTextLineOut(330, 212 + (j * 34.5), showrankname, lstrlen(showrankname));

				if (RankList.infos[j].Kills != 0)
				{
					wsprintf(showrankkills, "%d", RankList.infos[j].Kills);
					SetFontTextColor( RGB(255, 255, 255));
					dsTextLineOut(435, 212 + (j * 34.5), showrankkills, lstrlen(showrankkills));
				}

			}

		}
	}
}

void Rank::Close()
{
	fOpen = false;
	Tag = 0;
	SubTag = 0;
	SubTag2 = 0;
	classtype = 0;
	classtype2 = 0;

}

void Rank::Button()
{
	int auxx = 0, auxy = 0;

	if (fOpen)
	{
		int WinX = 40, WinY = 70, cnt2 = 0;

		if (MouseAction(70, 159, 120, 50)) {
			Tag = 1;
			classtype = 0;
			SendRankclassToServer(1, 0);
		}

		if (MouseAction(70, 214, 120, 50)) {
			Tag = 2;
			classtype = 0;
			SendRankclassToServer(2, 0);
		}

		if (MouseAction(450, 80, 22, 22))
			Close();

		if (Tag == 1)
		{

			if (classtype != 1)
			{
				if (MouseAction(232, 125, 25, 25))
				{
					classtype = 1;
					SendRankclassToServer(1, 3);
				}
			}
			if (classtype != 2)
			{
				if (MouseAction(232 + 29, 125, 25, 25))
				{
					classtype = 2;
					SendRankclassToServer(1, 5);
				}
			}
			if (classtype != 3)
			{
				if (MouseAction(232 + (29 * 2), 125, 25, 25))
				{
					classtype = 3;
					SendRankclassToServer(1, 8);
				}
			}
			if (classtype != 4)
			{
				if (MouseAction(232 + (29 * 3), 125, 25, 25))
				{
					classtype = 4;
					SendRankclassToServer(1, 6);
				}
			}
			if (classtype != 5)
			{
				if (MouseAction(232 + (29 * 4), 125, 25, 25))
				{
					classtype = 5;
					SendRankclassToServer(1, 1);
				}
			}
			if (classtype != 6)
			{
				if (MouseAction(232 + (29 * 5), 125, 25, 25))
				{
					classtype = 6;
					SendRankclassToServer(1, 7);
				}
			}
			if (classtype != 7)
			{
				if (MouseAction(232 + (29 * 6), 125, 25, 25))
				{
					classtype = 7;
					SendRankclassToServer(1, 2);
				}
			}
			if (classtype != 8)
			{
				if (MouseAction(232 + (29 * 7), 125, 25, 25))
				{
					classtype = 8;
					SendRankclassToServer(1, 4);
				}
			}

			if (classtype != 0)
			{
				if (MouseAction(191, 125, 25, 25))
				{
					classtype = 0;
					SendRankclassToServer(1, 0);
				}
			}
		}

		if (Tag == 2)
		{

			if (classtype != 1)
			{
				if (MouseAction(232, 125, 25, 25))
				{
					classtype = 1;
					SendRankclassToServer(2, 3);
				}
			}
			if (classtype != 2)
			{
				if (MouseAction(232 + 29, 125, 25, 25))
				{
					classtype = 2;
					SendRankclassToServer(2, 5);
				}
			}
			if (classtype != 3)
			{
				if (MouseAction(232 + (29 * 2), 125, 25, 25))
				{
					classtype = 3;
					SendRankclassToServer(2, 8);
				}
			}
			if (classtype != 4)
			{
				if (MouseAction(232 + (29 * 3), 125, 25, 25))
				{
					classtype = 4;
					SendRankclassToServer(2, 6);
				}
			}
			if (classtype != 5)
			{
				if (MouseAction(232 + (29 * 4), 125, 25, 25))
				{
					classtype = 5;
					SendRankclassToServer(2, 1);
				}
			}
			if (classtype != 6)
			{
				if (MouseAction(232 + (29 * 5), 125, 25, 25))
				{
					classtype = 6;
					SendRankclassToServer(2, 7);
				}
			}
			if (classtype != 7)
			{
				if (MouseAction(232 + (29 * 6), 125, 25, 25))
				{
					classtype = 7;
					SendRankclassToServer(2, 2);
				}
			}
			if (classtype != 8)
			{
				if (MouseAction(232 + (29 * 7), 125, 25, 25))
				{
					classtype = 8;
					SendRankclassToServer(2, 4);
				}
			}

			if (classtype != 0)
			{
				if (MouseAction(191, 125, 25, 25))
				{
					classtype = 0;
					SendRankclassToServer(2, 0);
				}
			}
		}


	}
}



