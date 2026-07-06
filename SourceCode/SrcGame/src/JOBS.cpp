#include "sinbaram/sinLinkHeader.h"
#include "JOBS.h"
#include <string>

JOBS_PACK JOBSList;
JOBS_ITEM Jselectitem;

extern void DrawImage(int Image, int x, int y, int w, int h);

char jobnam[32];

void JOBS::SendJOBItemToServer(char name[32], int job_code, int forje_code)
{
	{
		JOBS_RESGATE packet;
		char vsfvtnc[32];

		jobc = job_code;
		forgejobc = forje_code;

		wsprintf(vsfvtnc, "Deseja Proceder?");

		cMessageBox.ShowMessage3(MESSAGE_JOBS, vsfvtnc);

	}

}

void JOBS::SendSellJOBToServer(int minecode, int qtd, int price)
{
	{
		char vsfvtnc[32];

		if (contsellqtd > 0 && contsellprice > 0)
		{

			wsprintf(vsfvtnc, "Deseja vender %d atributo?", contsellqtd);

			cMessageBox.ShowMessage3(MESSAGE_JOBS_SELL, vsfvtnc);

		}
		else
		{
			cMessageBox.ShowMessage(34);
		}
	}

}

void JOBS::RecvJOBmarketToServer(JOBS_PACK* itens)
{
	CopyMemory(&JOBSList, itens, sizeof(JOBS_PACK));


	int position1 = 0;
	int position2 = 0;
	int position3 = 0;
	int position4 = 0;
	int position5 = 0;
	int poseall;
	int posetemp;
	int posegeral;

		

	}

void JOBS::SendJOBmarketToServer(JOBS_ITEM item)
{
	{
		Jselectitem = item;
		cMessageBox.ShowMessage3(MESSAGE_JOBS_BUY, "Deseja comprar?");
	}

}



void JOBS::RecvJOBInfoToServer(JOBS_INFO* JOBS)
{
	Exp = JOBS->exp;
	VSFlvl = JOBS->VTNClvl;
	Job = JOBS->Job;
	m_a = JOBS->m_a;
	m_f = JOBS->m_f;
	m_l = JOBS->m_l;
	m_m = JOBS->m_m;
	m_r = JOBS->m_r;

}


bool JOBS::MouseAction(int x, int y, int w, int h)
{
	if (pRealCursorPos.x > x&& pRealCursorPos.x < x + w && pRealCursorPos.y > y&& pRealCursorPos.y < y + h)
		return TRUE;

	return FALSE;
}

void JOBS::Open()
{
	fOpen = true;
}

void JOBS::Init()
{
	Image[0] = CreateTextureMaterial("game//images//Jobs//main.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[1] = CreateTextureMaterial("game//images//Jobs//1.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[2] = CreateTextureMaterial("game//images//Jobs//2.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[3] = CreateTextureMaterial("game//images//Jobs//3.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[4] = CreateTextureMaterial("game//images//Jobs//job1_1.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[5] = CreateTextureMaterial("game//images//Jobs//job1_2.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[6] = CreateTextureMaterial("game//images//Jobs//job1_3.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[7] = CreateTextureMaterial("game//images//Jobs//job2_1.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[8] = CreateTextureMaterial("game//images//Jobs//job2_2.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[9] = CreateTextureMaterial("game//images//Jobs//job2_3.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[10] = CreateTextureMaterial("game//images//Jobs//job3_1.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[11] = CreateTextureMaterial("game//images//Jobs//job3_2.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[12] = CreateTextureMaterial("game//images//Jobs//job3_3.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[13] = CreateTextureMaterial("game//images//Jobs//job4_1.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[14] = CreateTextureMaterial("game//images//Jobs//job4_2.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[15] = CreateTextureMaterial("game//images//Jobs//job4_3.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[16] = CreateTextureMaterial("game//images//Jobs//job5_1.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[17] = CreateTextureMaterial("game//images//Jobs//job5_2.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[18] = CreateTextureMaterial("game//images//Jobs//job5_3.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[19] = CreateTextureMaterial("game//images//Jobs//xpbar.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[20] = CreateTextureMaterial("game//images//Jobs//nojob.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[21] = CreateTextureMaterial("game//images//Jobs//3_1.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[22] = CreateTextureMaterial("game//images//Jobs//3_2.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[23] = CreateTextureMaterial("game//images//battlepass//Balao_Info_BP.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[24] = CreateTextureMaterial("game//images//Jobs//bt1.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[25] = CreateTextureMaterial("game//images//Jobs//4.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[26] = CreateTextureMaterial("game//images//Jobs//5.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[27] = CreateTextureMaterial("game//images//Jobs//6.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[28] = CreateTextureMaterial("game//images//Jobs//7.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[29] = CreateTextureMaterial("game//images//Jobs//8.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[30] = CreateTextureMaterial("game//images//Jobs//m1.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[31] = CreateTextureMaterial("game//images//Jobs//m2.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[32] = CreateTextureMaterial("game//images//Jobs//m3.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[33] = CreateTextureMaterial("game//images//Jobs//m4.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[34] = CreateTextureMaterial("game//images//Jobs//m5.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[35] = CreateTextureMaterial("game//images//Jobs//venda.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[36] = CreateTextureMaterial("game//images//Jobs//sell.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[37] = CreateTextureMaterial("game//images//Jobs//sell_.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[38] = CreateTextureMaterial("game//images//Jobs//mercado.PNG", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[39] = CreateTextureMaterial("game//images//Jobs//Page_Left.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[40] = CreateTextureMaterial("game//images//Jobs//Page_Left_1.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[41] = CreateTextureMaterial("game//images//Jobs//Page_Right.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[42] = CreateTextureMaterial("game//images//Jobs//Page_Right_1.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[43] = CreateTextureMaterial("game//images//Jobs//buy.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[44] = CreateTextureMaterial("game//images//Jobs//buy_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	ReadTextures();
}


void JOBS::Draw()
{
	char szBuff2[32] = { 0 };
	char strBuff[64];
	char nomejob[64];
	char VSFlvljob[64];
	char xpjob[64];
	char _m_a[64];
	char _m_f[64];
	char _m_l[64];
	char _m_m[64];
	char _m_r[64];
	char ccontsellqtd[64];
	char ccontsellprice[64];
	int contjob = 0;
	char ID[64];
	char atributo[64];
	char qtd[64];
	char price[64];
	char vendedor[64];
	memset(strBuff, 0, sizeof(strBuff));
	char szBuff[10] = { 0 };
	int WinX = (WinSizeX - 586) / 2, WinY = (WinSizeY - 445) / 2, cnt2 = 0;
	//636 -- x
	//305 -- y


	if (fOpen)
	{
		//MOSTRA A IMAGEM DO HUD PRINCPAL
		DrawImage(Image[0], WinX, WinY, 590, 450);

		 if (VSFlvl >= 15)
		{
			contjob = 2;
		}
		else if (VSFlvl >= 10)
		{
			contjob = 1;
		}
		

		//MOSTRA AS IMAGENS 
		if (page == 1)
		{
			DrawImage(Image[1], WinX, WinY, 590, 450);

			switch (Job)
			{
			case 0:
				wsprintf(nomejob, "SEM PROFISS�O");
				DrawFontText(WinX - 636 + 930, WinY - 305 + 400, nomejob, RGB(214, 158, 17), false);
				DrawImage(Image[20], WinX + 189 , WinY + 71, 100, 116);
				DrawFontText(WinX - 636 + 1180, WinY - 305 + 383, "?", RGB(214, 158, 17), false);
				break;
			case 1:
				wsprintf(nomejob, "ARCANISTA");
				DrawFontText(WinX - 636 + 930, WinY - 305 + 400, nomejob, RGB(214, 158, 17), false);
				DrawImage(Image[4 + contjob], WinX + 189, WinY + 71, 100, 116);
				DrawImage(Image[30], WinX - 636 + 1180, WinY - 305 + 383, 26, 25);
				break;
			case 2:
				wsprintf(nomejob, "FERREIRO");
				DrawFontText(WinX - 636 + 930, WinY - 305 + 400, nomejob, RGB(214, 158, 17), false);
				DrawImage(Image[7 + contjob], WinX + 189, WinY + 71, 100, 116);
				DrawImage(Image[31], WinX - 636 + 1180, WinY - 305 + 383, 26, 25);
				break;
			case 3:
				wsprintf(nomejob, "LENHADOR");
				DrawFontText(WinX - 636 + 930, WinY - 305 + 400, nomejob, RGB(214, 158, 17), false);
				DrawImage(Image[10 + contjob], WinX + 189, WinY + 71, 100, 116);
				DrawImage(Image[32], WinX - 636 + 1180, WinY - 305 + 383, 26, 25);
				break;
			case 4:
				wsprintf(nomejob, "MINERADOR");
				DrawFontText(WinX - 636 + 930, WinY - 305 + 400, nomejob, RGB(214, 158, 17), false);
				DrawImage(Image[13 + contjob], WinX + 189, WinY + 71, 100, 116);
				DrawImage(Image[33], WinX - 636 + 1180, WinY - 305 + 383, 26, 25);
				break;
			case 5:
				wsprintf(nomejob, "ARTES�O");
				DrawFontText(WinX - 636 + 930, WinY - 305 + 400, nomejob, RGB(214, 158, 17), false);
				DrawImage(Image[16 + contjob], WinX + 189, WinY + 71, 100, 116);
				DrawImage(Image[34], WinX - 636 + 1180, WinY - 305 + 383, 26, 25);
				break;
			}
		

		wsprintf(VSFlvljob, "%d", VSFlvl);
		DrawFontText(WinX - 636 + 970, WinY - 305 + 455, VSFlvljob, RGB(255, 255, 255), false);


		if (VSFlvl > 19)
		{
			wsprintf(xpjob, "999 / 999");
		}
		else
		{
			wsprintf(xpjob, "%d / 999", Exp);
			
		}
		DrawFontText(WinX - 636 + 933, WinY - 305 + 425, xpjob, RGB(255, 255, 255), false);

		if (VSFlvl > 19)
		{
			DrawImage(Image[19], WinX + 295, WinY + 136, 999 * 0.122, 10);
		}
		else
		{
			DrawImage(Image[19], WinX + 295, WinY + 136, Exp * 0.122, 10);
		}

		wsprintf(_m_r, "%d", m_r);
		DrawFontText(WinX - 636 + 995, WinY - 305 + 517, _m_r, RGB(255, 255, 255), false);

		if (MouseAction(WinX - 636 + 1095, WinY - 305 + 517, 63, 19))
			DrawImage(Image[37], WinX - 636 + 1095, WinY - 305 + 517, 63, 19);
		else
			DrawImage(Image[36], WinX - 636 + 1095, WinY - 305 + 517, 63, 19);
		
		wsprintf(_m_a, "%d", m_a);
		DrawFontText(WinX - 636 + 995, WinY - 305 + 552, _m_a, RGB(255, 255, 255), false);

		if (MouseAction(WinX - 636 + 1095, WinY - 305 + 552, 63, 19))
			DrawImage(Image[37], WinX - 636 + 1095, WinY - 305 + 552, 63, 19);
		else
			DrawImage(Image[36], WinX - 636 + 1095, WinY - 305 + 552, 63, 19);

		wsprintf(_m_f, "%d", m_f);
		DrawFontText(WinX - 636 + 995, WinY - 305 + 587, _m_f, RGB(255, 255, 255), false);

		if (MouseAction(WinX - 636 + 1095, WinY - 305 + 587, 63, 19))
			DrawImage(Image[37], WinX - 636 + 1095, WinY - 305 + 587, 63, 19);
		else
			DrawImage(Image[36], WinX - 636 + 1095, WinY - 305 + 587, 63, 19);

		wsprintf(_m_l, "%d", m_l);
		DrawFontText(WinX - 636 + 995, WinY - 305 + 622, _m_l, RGB(255, 255, 255), false);

		if (MouseAction(WinX - 636 + 1095, WinY - 305 + 622, 63, 19))
			DrawImage(Image[37], WinX - 636 + 1095, WinY - 305 + 622, 63, 19);
		else
			DrawImage(Image[36], WinX - 636 + 1095, WinY - 305 + 622, 63, 19);

		wsprintf(_m_m, "%d", m_m);
		DrawFontText(WinX - 636 + 995, WinY - 305 + 657, _m_m, RGB(255, 255, 255), false);

		if (MouseAction(WinX - 636 + 1095, WinY - 305 + 657, 63, 19))
			DrawImage(Image[37], WinX - 636 + 1095, WinY - 305 + 657, 63, 19);
		else
			DrawImage(Image[36], WinX - 636 + 1095, WinY - 305 + 657, 63, 19);

		}

		if (page == 2)
		{
			DrawImage(Image[2], WinX, WinY, 590, 450);

		}

		// ma x mr	
		// mf x ma
		// ml x mf
		// mm x ml
		// mr x mm


		if (page == 3)
		{
			if (contjob == 0)
			{
				DrawImage(Image[3], WinX, WinY, 590, 450);

				switch (Job)
				{
				case 1:
				DrawFontText(WinX - 636 + 940, WinY - 305 + 415, "50", RGB(255, 255, 255), false);
				DrawFontText(WinX - 636 + 940, WinY - 305 + 433, "200", RGB(255, 255, 255), false);
				DrawFontText(WinX - 636 + 940, WinY - 305 + 451, "50", RGB(255, 255, 255), false);
				DrawFontText(WinX - 636 + 940, WinY - 305 + 469, "50", RGB(255, 255, 255), false);
				DrawFontText(WinX - 636 + 940, WinY - 305 + 487, "50", RGB(255, 255, 255), false);

				DrawFontText(WinX - 636 + 940, WinY - 305 + 516, "?", RGB(255, 255, 255), false);
				DrawFontText(WinX - 636 + 940, WinY - 305 + 534, "?", RGB(255, 255, 255), false);
				DrawFontText(WinX - 636 + 940, WinY - 305 + 552, "?", RGB(255, 255, 255), false);
				DrawFontText(WinX - 636 + 940, WinY - 305 + 570, "?", RGB(255, 255, 255), false);
				DrawFontText(WinX - 636 + 940, WinY - 305 + 588, "?", RGB(255, 255, 255), false);

				DrawFontText(WinX - 636 + 940, WinY - 305 + 617, "?", RGB(255, 255, 255), false);
				DrawFontText(WinX - 636 + 940, WinY - 305 + 635, "?", RGB(255, 255, 255), false);
				DrawFontText(WinX - 636 + 940, WinY - 305 + 653, "?", RGB(255, 255, 255), false);
				DrawFontText(WinX - 636 + 940, WinY - 305 + 671, "?", RGB(255, 255, 255), false);
				DrawFontText(WinX - 636 + 940, WinY - 305 + 689, "?", RGB(255, 255, 255), false);
					break;

				case 2:
					DrawFontText(WinX - 636 + 940, WinY - 305 + 415, "50", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 433, "50", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 451, "200", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 469, "50", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 487, "50", RGB(255, 255, 255), false);

					DrawFontText(WinX - 636 + 940, WinY - 305 + 516, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 534, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 552, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 570, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 588, "?", RGB(255, 255, 255), false);

					DrawFontText(WinX - 636 + 940, WinY - 305 + 617, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 635, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 653, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 671, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 689, "?", RGB(255, 255, 255), false);
					break;

				case 3:
					DrawFontText(WinX - 636 + 940, WinY - 305 + 415, "50", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 433, "50", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 451, "50", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 469, "200", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 487, "50", RGB(255, 255, 255), false);

					DrawFontText(WinX - 636 + 940, WinY - 305 + 516, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 534, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 552, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 570, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 588, "?", RGB(255, 255, 255), false);

					DrawFontText(WinX - 636 + 940, WinY - 305 + 617, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 635, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 653, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 671, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 689, "?", RGB(255, 255, 255), false);
					break;

				case 4:
					DrawFontText(WinX - 636 + 940, WinY - 305 + 415, "50", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 433, "50", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 451, "50", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 469, "50", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 487, "200", RGB(255, 255, 255), false);

					DrawFontText(WinX - 636 + 940, WinY - 305 + 516, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 534, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 552, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 570, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 588, "?", RGB(255, 255, 255), false);

					DrawFontText(WinX - 636 + 940, WinY - 305 + 617, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 635, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 653, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 671, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 689, "?", RGB(255, 255, 255), false);
					break;

				case 5:
					DrawFontText(WinX - 636 + 940, WinY - 305 + 415, "200", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 433, "50", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 451, "50", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 469, "50", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 487, "50", RGB(255, 255, 255), false);

					DrawFontText(WinX - 636 + 940, WinY - 305 + 516, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 534, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 552, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 570, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 588, "?", RGB(255, 255, 255), false);

					DrawFontText(WinX - 636 + 940, WinY - 305 + 617, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 635, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 653, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 671, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 689, "?", RGB(255, 255, 255), false);
					break;
				}

				if (VSFlvl >= 5)
				{
					DrawFontText(WinX - 636 + 1140, 482, "45%", RGB(214, 158, 17), false);
				}
				else {
					DrawFontText(WinX - 636 + 1140, 482, "30%", RGB(214, 158, 17), false);
				}

				DrawFontText(WinX - 636 + 1140, 583, "?", RGB(214, 158, 17), false);
				DrawFontText(WinX - 636 + 1140, 678, "?", RGB(214, 158, 17), false);

				if (pCursorPos.x > WinX - 636 + 985 && pCursorPos.x< WinX - 636 + 1045 && pCursorPos.y> WinY - 305 + 440 && pCursorPos.y < WinY - 305 + 470)
				{
					DrawImage(Image[23], pCursorPos.x + 25, pCursorPos.y -10, 74, 32);
					wsprintf(szBuff2, "FORJAR ?");
					DrawFontText(pCursorPos.x + 34, pCursorPos.y -2, szBuff2, RGB(214, 158, 17), false);
				}
				
			}
			else if (contjob == 1)
			{
				DrawImage(Image[21], WinX, WinY, 590, 450);

				switch (Job)
				{
				case 1:
					DrawFontText(WinX - 636 + 940, WinY - 305 + 415, "50", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 433, "200", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 451, "50", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 469, "50", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 487, "50", RGB(255, 255, 255), false);

					DrawFontText(WinX - 636 + 940, WinY - 305 + 516, "100", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 534, "400", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 552, "100", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 570, "100", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 588, "100", RGB(255, 255, 255), false);

					DrawFontText(WinX - 636 + 940, WinY - 305 + 617, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 635, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 653, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 671, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 689, "?", RGB(255, 255, 255), false);
					break;

				case 2:
					DrawFontText(WinX - 636 + 940, WinY - 305 + 415, "50", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 433, "50", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 451, "200", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 469, "50", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 487, "50", RGB(255, 255, 255), false);

					DrawFontText(WinX - 636 + 940, WinY - 305 + 516, "100", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 534, "100", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 552, "400", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 570, "100", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 588, "100", RGB(255, 255, 255), false);

					DrawFontText(WinX - 636 + 940, WinY - 305 + 617, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 635, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 653, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 671, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 689, "?", RGB(255, 255, 255), false);
					break;

				case 3:
					DrawFontText(WinX - 636 + 940, WinY - 305 + 415, "50", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 433, "50", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 451, "50", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 469, "200", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 487, "50", RGB(255, 255, 255), false);

					DrawFontText(WinX - 636 + 940, WinY - 305 + 516, "100", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 534, "100", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 552, "100", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 570, "400", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 588, "100", RGB(255, 255, 255), false);

					DrawFontText(WinX - 636 + 940, WinY - 305 + 617, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 635, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 653, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 671, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 689, "?", RGB(255, 255, 255), false);
					break;

				case 4:
					DrawFontText(WinX - 636 + 940, WinY - 305 + 415, "50", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 433, "50", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 451, "50", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 469, "50", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 487, "200", RGB(255, 255, 255), false);

					DrawFontText(WinX - 636 + 940, WinY - 305 + 516, "100", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 534, "100", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 552, "100", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 570, "100", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 588, "400", RGB(255, 255, 255), false);

					DrawFontText(WinX - 636 + 940, WinY - 305 + 617, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 635, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 653, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 671, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 689, "?", RGB(255, 255, 255), false);
					break;

				case 5:
					DrawFontText(WinX - 636 + 940, WinY - 305 + 415, "200", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 433, "50", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 451, "50", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 469, "50", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 487, "50", RGB(255, 255, 255), false);

					DrawFontText(WinX - 636 + 940, WinY - 305 + 516, "400", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 534, "100", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 552, "100", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 570, "100", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 588, "100", RGB(255, 255, 255), false);

					DrawFontText(WinX - 636 + 940, WinY - 305 + 617, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 635, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 653, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 671, "?", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 689, "?", RGB(255, 255, 255), false);
					break;
				}

				if (VSFlvl >= 5)
				{
					DrawFontText(WinX - 636 + 1140, WinY - 305 + 482, "45%", RGB(214, 158, 17), false);
				}
				else {
					DrawFontText(WinX - 636 + 1140, WinY - 305 + 482, "30%", RGB(214, 158, 17), false);
				}

				if (VSFlvl > 5)
				{
					DrawFontText(WinX - 636 + 1140, WinY - 305 + 583, "30%", RGB(214, 158, 17), false);
				}
				else {
					DrawFontText(WinX - 636 + 1140, WinY - 305 + 583, "20%", RGB(214, 158, 17), false);
				}

				DrawFontText(WinX - 636 + 1140, WinY - 305 + 678, "?", RGB(214, 158, 17), false);


				if (pCursorPos.x > WinX - 636 + 985 && pCursorPos.x < WinX - 636 + 1045 && pCursorPos.y> WinY - 305 + 440 && pCursorPos.y < WinY - 305 + 470)
				{
					DrawImage(Image[23], pCursorPos.x + 25, pCursorPos.y - 10, 74, 32);
					wsprintf(szBuff2, "FORJAR ?");
					DrawFontText(pCursorPos.x + 34, pCursorPos.y - 2, szBuff2, RGB(214, 158, 17), false);
				}

				if (pCursorPos.x > WinX - 636 + 985 && pCursorPos.x < WinX - 636 + 1045 && pCursorPos.y> WinY - 305 + 540 && pCursorPos.y < WinY - 305 + 570)
				{
					DrawImage(Image[23], pCursorPos.x + 25, pCursorPos.y - 10, 74, 32);
					wsprintf(szBuff2, "FORJAR ?");
					DrawFontText(pCursorPos.x + 34, pCursorPos.y - 2, szBuff2, RGB(214, 158, 17), false);
				}
			}
			else if (contjob == 2)
			{
				DrawImage(Image[22], WinX, WinY, 590, 450);

				switch (Job)
				{
				case 1:
					DrawFontText(WinX - 636 + 940, WinY - 305 + 415, "50", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 433, "200", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 451, "50", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 469, "50", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 487, "50", RGB(255, 255, 255), false);

					DrawFontText(WinX - 636 + 940, WinY - 305 + 516, "100", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 534, "400", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 552, "100", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 570, "100", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 588, "100", RGB(255, 255, 255), false);

					DrawFontText(WinX - 636 + 940, WinY - 305 + 617, "150", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 635, "750", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 653, "150", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 671, "150", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 689, "150", RGB(255, 255, 255), false);
					break;

				case 2:
					DrawFontText(WinX - 636 + 940, WinY - 305 + 415, "50", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 433, "50", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 451, "200", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 469, "50", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 487, "50", RGB(255, 255, 255), false);

					DrawFontText(WinX - 636 + 940, WinY - 305 + 516, "100", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 534, "100", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 552, "400", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 570, "100", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 588, "100", RGB(255, 255, 255), false);

					DrawFontText(WinX - 636 + 940, WinY - 305 + 617, "150", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 635, "150", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 653, "750", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 671, "150", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 689, "150", RGB(255, 255, 255), false);
					break;

				case 3:
					DrawFontText(WinX - 636 + 940, WinY - 305 + 415, "50", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 433, "50", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 451, "50", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 469, "200", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 487, "50", RGB(255, 255, 255), false);

					DrawFontText(WinX - 636 + 940, WinY - 305 + 516, "100", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 534, "100", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 552, "100", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 570, "400", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 588, "100", RGB(255, 255, 255), false);

					DrawFontText(WinX - 636 + 940, WinY - 305 + 617, "150", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 635, "150", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 653, "150", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 671, "750", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 689, "150", RGB(255, 255, 255), false);
					break;

				case 4:
					DrawFontText(WinX - 636 + 940, WinY - 305 + 415, "50", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 433, "50", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 451, "50", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 469, "50", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 487, "200", RGB(255, 255, 255), false);

					DrawFontText(WinX - 636 + 940, WinY - 305 + 516, "100", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 534, "100", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 552, "100", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 570, "100", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 588, "400", RGB(255, 255, 255), false);

					DrawFontText(WinX - 636 + 940, WinY - 305 + 617, "150", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 635, "150", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 653, "150", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 671, "150", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 689, "750", RGB(255, 255, 255), false);
					break;

				case 5:
					DrawFontText(WinX - 636 + 940, WinY - 305 + 415, "200", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 433, "50", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 451, "50", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 469, "50", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 487, "50", RGB(255, 255, 255), false);

					DrawFontText(WinX - 636 + 940, WinY - 305 + 516, "400", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 534, "100", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 552, "100", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 570, "100", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 588, "100", RGB(255, 255, 255), false);

					DrawFontText(WinX - 636 + 940, WinY - 305 + 617, "750", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 635, "150", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 653, "150", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 671, "150", RGB(255, 255, 255), false);
					DrawFontText(WinX - 636 + 940, WinY - 305 + 689, "150", RGB(255, 255, 255), false);
					break;
				}

				if (VSFlvl == 20)
				{
					DrawFontText(WinX - 636 + 1140, WinY - 305 + 482, "60%", RGB(214, 158, 17), false);
				}
				else {
					DrawFontText(WinX - 636 + 1140, WinY - 305 + 482, "45%", RGB(214, 158, 17), false);
				}
				
				if (VSFlvl == 20)
				{
					DrawFontText(WinX - 636 + 1140, WinY - 305 + 583, "40%", RGB(214, 158, 17), false);
				}
				else {
					DrawFontText(WinX - 636 + 1140, WinY - 305 + 583, "30%", RGB(214, 158, 17), false);
				}
				
				if (VSFlvl == 20)
				{
					DrawFontText(WinX - 636 + 1140, WinY - 305 + 678, "20%", RGB(214, 158, 17), false);
				}
				else
				{
					DrawFontText(WinX - 636 + 1140, WinY - 305 + 678, "10%", RGB(214, 158, 17), false);
				}


				if (pCursorPos.x > WinX - 636 + 985 && pCursorPos.x < WinX - 636 + 1045 && pCursorPos.y> WinY - 305 + 440 && pCursorPos.y < WinY - 305 + 470)
				{
					DrawImage(Image[23], pCursorPos.x + 25, pCursorPos.y - 10, 74, 32);
					wsprintf(szBuff2, "FORJAR ?");
					DrawFontText(pCursorPos.x + 34, pCursorPos.y - 2, szBuff2, RGB(214, 158, 17), false);
				}

				if (pCursorPos.x > WinX - 636 + 985 && pCursorPos.x < WinX - 636 + 1045 && pCursorPos.y> WinY - 305 + 540 && pCursorPos.y < WinY - 305 + 570)
				{
					DrawImage(Image[23], pCursorPos.x + 25, pCursorPos.y - 10, 74, 32);
					wsprintf(szBuff2, "FORJAR ?");
					DrawFontText(pCursorPos.x + 34, pCursorPos.y - 2, szBuff2, RGB(214, 158, 17), false);
				}

				if (pCursorPos.x > WinX - 636 + 985 && pCursorPos.x < WinX - 636 + 1045 && pCursorPos.y> WinY - 305 + 645 && pCursorPos.y < WinY - 305 + 675)
				{
					DrawImage(Image[23], pCursorPos.x + 25, pCursorPos.y - 10, 74, 32);
					wsprintf(szBuff2, "FORJAR ?");
					DrawFontText(pCursorPos.x + 34, pCursorPos.y - 2, szBuff2, RGB(214, 158, 17), false);
				}
			}
			

		}

		if (page == 4)
		{
		
			DrawImage(Image[25], WinX, WinY, 590, 450);

			if (Job < 1)
			{
				DrawImage(Image[24], WinX - 636 + 1060, WinY - 305 + 717, 104, 32);
			}

		}

		if (page == 5)
		{
			

			DrawImage(Image[26], WinX, WinY, 590, 450);

			if (Job < 1)
			{
				DrawImage(Image[24], WinX - 636 + 1060, WinY - 305 + 717, 104, 32);
			}

		}

		if (page == 6)
		{
			

			DrawImage(Image[27], WinX, WinY, 590, 450);

			if (Job < 1)
			{
				DrawImage(Image[24], WinX - 636 + 1060, WinY - 305 + 717, 104, 32);
			}

		}

		if (page == 7)
		{
		

			DrawImage(Image[28], WinX, WinY, 590, 450);

			if (Job < 1)
			{
				DrawImage(Image[24], WinX - 636 + 1060, WinY - 305 + 717, 104, 32);
			}

		}

		if (page == 8)
		{
			

			DrawImage(Image[29], WinX, WinY, 590, 450);

			if (Job < 1)
			{
				DrawImage(Image[24], WinX - 636 + 1060, WinY - 305 + 717, 104, 32);
			}

		}

		if (page == 99)
		{
			DrawImage(Image[35], WinX, WinY, 590, 450);

			wsprintf(ccontsellqtd, "%d", contsellqtd);
			DrawFontText(WinX - 636 + 920, WinY - 305 + 480, ccontsellqtd, RGB(255, 255, 255), false);


			wsprintf(ccontsellprice, "%d", contsellprice);
			DrawFontText(WinX - 636 + 1035, WinY - 305 + 482, ccontsellprice, RGB(255, 255, 255), false);

			switch (auxmine)
			{
			case 1:
				DrawImage(Image[30], WinX - 636 + 822, WinY - 305 + 515, 26, 25);
				wsprintf(_m_a, "%d", (m_a + auxmine3));
				DrawFontText(WinX - 636 + 824, WinY - 305 + 550, _m_a, RGB(255, 255, 255), false);
				auxmine2 = m_a;
				break;
			case 2:
				DrawImage(Image[31], WinX - 636 + 822, WinY - 305 + 515, 26, 25);
				wsprintf(_m_f, "%d", (m_f + auxmine3));
				DrawFontText(WinX - 636 + 824, WinY - 305 + 550, _m_f, RGB(255, 255, 255), false);
				auxmine2 = m_f;
				break;
			case 3:
				DrawImage(Image[32], WinX - 636 + 822, WinY - 305 + 515, 26, 25);
				wsprintf(_m_l, "%d",( m_l + auxmine3));
				DrawFontText(WinX - 636 + 824, WinY - 305 + 550, _m_l, RGB(255, 255, 255), false);
				auxmine2 = m_l;
				break;
			case 4:
				DrawImage(Image[33], WinX - 636 + 822, WinY - 305 + 515, 26, 25);
				wsprintf(_m_m, "%d", (m_m + auxmine3));
				DrawFontText(WinX - 636 + 824, WinY - 305 + 550, _m_m, RGB(255, 255, 255), false);
				auxmine2 = m_m;
				break;
			case 5:
				DrawImage(Image[34], WinX - 636 + 822, WinY - 305 + 515, 26, 25);
				wsprintf(_m_r, "%d", (m_r + auxmine3));
				DrawFontText(WinX - 636 + 824, WinY - 305 + 550, _m_r, RGB(255, 255, 255), false);
				auxmine2 = m_r;
				break;
			}
		}

		if (page == 9)
		{

			DrawImage(Image[38], WinX, WinY, 590, 450);
			//825 + 55 + 140 + 200 + 270 + 350, 450 + 50 


			if (MouseAction(WinX - 636 + 825, WinY - 305 + 722, 26, 26))
			{
				DrawImage(Image[40], WinX - 636 + 825, WinY - 305 + 722, 26, 26);
			}
			else
			{
				DrawImage(Image[39], WinX - 636 + 825, WinY - 305 + 722, 26, 26);
			}
			
			if (MouseAction(WinX - 636 + 1170, WinY - 305 + 722, 26, 26))
			{
				DrawImage(Image[42], WinX - 636 + 1170, WinY - 305 + 722, 26, 26);
			}
			else
			{
				DrawImage(Image[41], WinX - 636 + 1170, WinY - 305 + 722, 26, 26);
			}
	

			if (subpage == 0)
			{
						for (int j = 0; j <= 5; j++)
						{

							int vsfmermao;
							switch (j)
							{
							case 0:
								vsfmermao = 0;
								break;
							case 1:
								vsfmermao = 3;
								break;
							case 2:
								vsfmermao = 5;
								break;
							case 3:
								vsfmermao = 6;
								break;
							case 4:
								vsfmermao = 9;
								break;
							case 5:
								vsfmermao = 10;
								break;
							}
							if (JOBSList.item[j].mine_qtd == 0 && JOBSList.item[j].mine_price == 0)
							{

							}
							else
							{
								wsprintf(ID, "%d", j);
								DrawFontText(WinX - 636 + 840, WinY - 305 + 450 + (j  * 45), ID, RGB(255, 255, 255), false);
							}
								//wsprintf(atributo, "%d", JOBSList.item[j].mine_type);
								switch (JOBSList.item[j].mine_type)
								{
								case 1:
									DrawImage(Image[30], WinX - 636 + 900, WinY - 305 + 440 + (j * 45) + vsfmermao, 26, 25);
									break;
								case 2:
									
									DrawImage(Image[31], WinX - 636 + 900, WinY - 305 + 440 + (j * 45) + vsfmermao, 26, 25);
									break;
								case 3:
									DrawImage(Image[32], WinX - 636 + 900, WinY - 305 + 440 + (j * 45) + vsfmermao, 26, 25);
									break;
								case 4:
									DrawImage(Image[33], WinX - 636 + 900, WinY - 305 + 440 + (j * 45) + vsfmermao, 26, 25);
									break;
								case 5:
									DrawImage(Image[34], WinX - 636 + 900, WinY - 305 + 440 + (j * 45) + vsfmermao, 26, 25);
									break;
								}
								if (JOBSList.item[j].mine_qtd == 0)
								{

								}
								else
								{
									wsprintf(qtd, "%d", JOBSList.item[j].mine_qtd);
									DrawFontText(WinX - 636 + 980, WinY - 305 + 450 + (j * 45), qtd, RGB(255, 255, 255), false);
								}
								if (JOBSList.item[j].mine_price == 0)
								{

								}
								else
								{
									wsprintf(price, "%d", JOBSList.item[j].mine_price);
									DrawFontText(WinX - 636 + 1045, WinY - 305 + 450 + (j * 45), price, RGB(255, 255, 255), false);
								}
								wsprintf(vendedor, "%s", JOBSList.item[j].Name);
								DrawFontText(WinX - 636 + 1095, WinY - 305 + 450 + (j * 45), vendedor, RGB(255, 255, 255), false);


								if (JOBSList.item[j].mine_qtd == 0 && JOBSList.item[j].mine_price == 0)
								{

								}
								else
								{
									if (MouseAction(WinX - 636 + 1180, WinY - 305 + 440 + ((j) * 45) + vsfmermao, 20, 20))
									{
										DrawImage(Image[44], WinX - 636 + 1180, WinY - 305 + 440 + ((j) * 45) + vsfmermao, 20, 20);
									}
									else
									{
										DrawImage(Image[43], WinX - 636 + 1180, WinY - 305 + 440 + ((j) * 45) + vsfmermao, 20, 20);
									}
								}
							
						}
					
				
			}
			else if (subpage == 1)
			{
				for (int j = 6; j <= 11; j++)
				{

					int vsfmermao;
					switch ((j-6))
					{
					case 0:
						vsfmermao = 0;
						break;
					case 1:
						vsfmermao = 3;
						break;
					case 2:
						vsfmermao = 5;
						break;
					case 3:
						vsfmermao = 6;
						break;
					case 4:
						vsfmermao = 9;
						break;
					case 5:
						vsfmermao = 10;
						break;
					}
					if (JOBSList.item[j].mine_qtd == 0 && JOBSList.item[j].mine_price == 0)
					{

					}
					else
					{
						wsprintf(ID, "%d", j);
						DrawFontText(WinX - 636 + 840, WinY - 305 + 450 + ((j - 6) * 45), ID, RGB(255, 255, 255), false);
					}
					//wsprintf(atributo, "%d", JOBSList.item[j].mine_type);
					switch (JOBSList.item[j].mine_type)
					{
					case 1:
						DrawImage(Image[30], WinX - 636 + 900, WinY - 305 + 440 + ((j - 6) * 45) + vsfmermao, 26, 25);
						break;
					case 2:

						DrawImage(Image[31], WinX - 636 + 900, WinY - 305 + 440 + ((j - 6) * 45) + vsfmermao, 26, 25);
						break;
					case 3:
						DrawImage(Image[32], WinX - 636 + 900, WinY - 305 + 440 + ((j - 6) * 45) + vsfmermao, 26, 25);
						break;
					case 4:
						DrawImage(Image[33], WinX - 636 + 900, WinY - 305 + 440 + ((j - 6) * 45) + vsfmermao, 26, 25);
						break;
					case 5:
						DrawImage(Image[34], WinX - 636 + 900, WinY - 305 + 440 + ((j - 6) * 45) + vsfmermao, 26, 25);
						break;
					}
					if (JOBSList.item[j].mine_qtd == 0)
					{

					}
					else
					{
						wsprintf(qtd, "%d", JOBSList.item[j].mine_qtd);
						DrawFontText(WinX - 636 + 980, WinY - 305 + 450 + ((j - 6) * 45), qtd, RGB(255, 255, 255), false);
					}
					if (JOBSList.item[j].mine_price == 0)
					{

					}
					else
					{
						wsprintf(price, "%d", JOBSList.item[j].mine_price);
						DrawFontText(WinX - 636 + 1045, WinY - 305 + 450 + ((j - 6) * 45), price, RGB(255, 255, 255), false);
					}					
					wsprintf(vendedor, "%s", JOBSList.item[j].Name);
					DrawFontText(WinX - 636 + 1095, WinY - 305 + 450 + ((j - 6) * 45), vendedor, RGB(255, 255, 255), false);


					if (JOBSList.item[j].mine_qtd == 0 && JOBSList.item[j].mine_price == 0)
					{

					}
					else
					{
						if (MouseAction(WinX - 636 + 1180, WinY - 305 + 440 + ((j - 6) * 45) + vsfmermao, 20, 20))
						{
							DrawImage(Image[44], WinX - 636 + 1180, WinY - 305 + 440 + ((j - 6) * 45) + vsfmermao, 20, 20);
						}
						else
						{
							DrawImage(Image[43], WinX - 636 + 1180, WinY - 305 + 440 + ((j - 6) * 45) + vsfmermao, 20, 20);
						}
					}

				}


			}
			

		}


	}

	cnt2 = 0;
}

void JOBS::Close()
{
	fOpen = false;
	page = 0;
	Tag = 0;
	auxmine3 = 0;
	contsellqtd = 0;
	contsellprice = 0;
	subpage = 0;
}


void JOBS::Button()
{

	

	if (fOpen)
	{
		int WinX = (WinSizeX - 586) / 2, WinY = (WinSizeY - 445) / 2, cnt2 = 0;


		//A��O DE CLIQUE DO BOT�O
		if (MouseAction(WinX - 636 + 655, WinY - 305 + 390, 155, 30) && page != 99)
			page = 1;

		//A��O DE CLIQUE DO BOT�O 
		if (MouseAction(WinX - 636 + 655, WinY - 305 + 425, 155, 30) && page != 99)
			page = 2;

		//A��O DE CLIQUE DO BOT�O
		if (Job > 0)
		{
			if (MouseAction(WinX - 636 + 655, WinY - 305 + 675, 155, 30) && page != 99)
				page = 3;
		}


		if (MouseAction(WinX - 636 + 655, WinY - 305 + 450, 155, 22) && page != 99)
			page = 4;
		if (MouseAction(WinX - 636 + 655, WinY - 305 + 470, 155, 22) && page != 99 )
			page = 5;
		if (MouseAction(WinX - 636 + 655, WinY - 305 + 490, 155, 22) && page != 99 )
			page = 6;
		if (MouseAction(WinX - 636 + 655, WinY - 305 + 507, 155, 22) && page != 99 )
			page = 7;
		if (MouseAction(WinX - 636 + 655, WinY - 305 + 525, 155, 22) && page != 99 )
			page = 8;

		if (Job > 0)
		{
			if (MouseAction(WinX - 636 + 655, WinY - 305 + 635, 155, 30) && page != 99)
				page = 9;
		}


		if (page == 1)
		{
			if (MouseAction(WinX - 636 + 1095, WinY - 305 + 517, 63, 19))
			{
				page = 99;
				auxmine = 5;
			}
			if (MouseAction(WinX - 636 + 1095, WinY - 305 + 552, 63, 19))
			{
				page = 99;
				auxmine = 1;
			}
			if (MouseAction(WinX - 636 + 1095, WinY - 305 + 587, 63, 19))
			{
				page = 99;
				auxmine = 2;
			}
			if (MouseAction(WinX - 636 + 1095, WinY - 305 + 622, 63, 19))
			{
				page = 99;
				auxmine = 3;
			}
			if (MouseAction(WinX - 636 + 1095, WinY - 305 + 657, 63, 19))
			{
				page = 99;
				auxmine = 4;
			}
		}

		if (page == 9)
		{
			if (MouseAction(WinX - 636 + 825, WinY - 305 + 722, 26, 26))
			{
				if (subpage <= 0)
				{
					subpage = 0;
				}
				else
				{
					subpage--;
				}
			}

			if (MouseAction(WinX - 636 + 1170, WinY - 305 + 725, 26, 26))
			{
					subpage++;		
			}
		}

		if (page == 99)
		{
			if (MouseAction(WinX - 636 + 891, WinY - 305 + 557, 21, 17))
			{
				if (contsellqtd + 1 > auxmine2)
				{
				}
				else
				{
					contsellqtd += 1;
					auxmine3 -= 1;
				}
				
			}

			if (MouseAction(WinX - 636 + 920, WinY - 305 + 557, 21, 17))
			{
				if (contsellqtd + 10 > auxmine2)
				{
				}
				else
				{
					contsellqtd += 10;
					auxmine3 -= 10;
				}
			}

			if (MouseAction(WinX - 636 + 947, WinY - 305 + 557, 21, 17))
			{
				if (contsellqtd + 50 > auxmine2)
				{
				}
				else
				{
					contsellqtd += 50;
					auxmine3 -= 50;
				}
			}


			if (MouseAction(WinX - 636 + 1004, WinY - 305 + 557, 21, 17))
			{
				contsellprice += 1;
			}

			if (MouseAction(WinX - 636 + 1033, WinY - 305 + 557, 21, 17))
			{
				contsellprice += 10;
			}

			if (MouseAction(WinX - 636 + 1060, WinY - 305 + 557, 21, 17))
			{
				contsellprice += 50;
			}


			if (MouseAction(WinX - 636 + 991, WinY - 305 + 599, 61, 17))
			{
				page = 1;
				auxmine3 = 0;
				contsellqtd = 0; 
				contsellprice = 0;
			}

			if (MouseAction(WinX - 636 + 920, WinY - 305 + 599, 61, 17))
			{
					SendSellJOBToServer(auxmine, contsellqtd, contsellprice);			
			}
		}
		

		if (page > 3 && Job < 1)
		{
			//A��O DE CLIQUE DO BOT�O JOB
			if (MouseAction(WinX - 636 + 1060, WinY - 305 + 717, 104, 32))
			{
				switch (page)
				{
				case 4:
					SendJOBItemToServer("", 1, 0);
					break;
				case 5:
					SendJOBItemToServer("", 5, 0);
					break;
				case 6:
					SendJOBItemToServer("", 2, 0);
					break;
				case 7:
					SendJOBItemToServer("", 3, 0);
					break;
				case 8:
					SendJOBItemToServer("", 4, 0);
					break;
				}
				
			}
		}

		if (page == 3 && VSFlvl >= 0)
		{
			//A��O DE CLIQUE DO BOT�O FORJA
			if (MouseAction(WinX - 636 + 985, WinY - 305 + 440, 60, 30))
			{
				SendJOBItemToServer("Forja Iniciante", 0, 1);
			}
		}

		if (page == 3 && VSFlvl >= 10)
		{
			//A��O DE CLIQUE DO BOT�O FORJA
			if (MouseAction(WinX - 636 + 985, WinY - 305 + 540, 60, 30))
			{
				SendJOBItemToServer("Forja Veterano", 0, 2);
			}
		}

		if (page == 3 && VSFlvl >= 15)
		{
			//A��O DE CLIQUE DO BOT�O FORJA
			if (MouseAction(WinX - 636 + 985, WinY - 305 + 645, 60, 30))
			{
				SendJOBItemToServer("Forja Experiente", 0, 3);
			}
		}

		if (page == 9)
		{
			RecvJOBmarketToServer(&JOBSList);

			if (subpage == 0)
			{
				for (int j = 0; j <= 5; j++)
				{

					int vsfmermao;
					switch (j)
					{
					case 0:
						vsfmermao = 0;
						break;
					case 1:
						vsfmermao = 3;
						break;
					case 2:
						vsfmermao = 5;
						break;
					case 3:
						vsfmermao = 6;
						break;
					case 4:
						vsfmermao = 9;
						break;
					case 5:
						vsfmermao = 10;
						break;
					}


					if (JOBSList.item[j].mine_qtd == 0 && JOBSList.item[j].mine_price == 0)
					{

					}
					else
					{
						if (MouseAction(WinX - 636 + 1180, WinY - 305 + 440 + ((j) * 45) + vsfmermao, 20, 20))
						{
							SendJOBmarketToServer(JOBSList.item[j]);
						}
					}

				}


			}
			else if (subpage == 1)
			{
				for (int j = 6; j <= 11; j++)
				{

					int vsfmermao;
					switch ((j - 6))
					{
					case 0:
						vsfmermao = 0;
						break;
					case 1:
						vsfmermao = 3;
						break;
					case 2:
						vsfmermao = 5;
						break;
					case 3:
						vsfmermao = 6;
						break;
					case 4:
						vsfmermao = 9;
						break;
					case 5:
						vsfmermao = 10;
						break;
					}
					
					if (JOBSList.item[j].mine_qtd == 0 && JOBSList.item[j].mine_price == 0)
					{

					}
					else
					{
						if (MouseAction(WinX - 636 + 1180, WinY - 305 + 440 + ((j - 6) * 45) + vsfmermao, 20, 20))
						{
							SendJOBmarketToServer(JOBSList.item[j]);
						}
					}
					


				}


			}


		}
		

	}

}




