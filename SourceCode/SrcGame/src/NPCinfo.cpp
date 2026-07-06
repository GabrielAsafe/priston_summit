#include "sinbaram/sinLinkHeader.h"
#include "NPCinfo.h"
#include <string>


extern void DrawImage(int Image, int x, int y, int w, int h);


bool NPCinfo::MouseAction(int x, int y, int w, int h)
{
	if (pRealCursorPos.x > x && pRealCursorPos.x < x + w && pRealCursorPos.y > y && pRealCursorPos.y < y + h)
		return TRUE;

	return FALSE;
}

void NPCinfo::Open()
{
	fOpen = true;
}

void NPCinfo::Init()
{
	Image[0] = CreateTextureMaterial("game//images//Infos//MAINS//AGE.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[1] = CreateTextureMaterial("game//images//Infos//MAINS//AGE_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[2] = CreateTextureMaterial("game//images//Infos//MAINS//BATTLEPASS.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[3] = CreateTextureMaterial("game//images//Infos//MAINS//BATTLEPASS_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[4] = CreateTextureMaterial("game//images//Infos//MAINS//BLESS.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[5] = CreateTextureMaterial("game//images//Infos//MAINS//BLESS_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[6] = CreateTextureMaterial("game//images//Infos//MAINS//DISCORD.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[7] = CreateTextureMaterial("game//images//Infos//MAINS//DISCORD_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[8] = CreateTextureMaterial("game//images//Infos//MAINS//Jogo.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[9] = CreateTextureMaterial("game//images//Infos//MAINS//Jogo_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[10] = CreateTextureMaterial("game//images//Infos//MAINS//MAIN.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[11] = CreateTextureMaterial("game//images//Infos//MAINS//MIX.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[12] = CreateTextureMaterial("game//images//Infos//MAINS//MIX_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[13] = CreateTextureMaterial("game//images//Infos//BOSS.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[14] = CreateTextureMaterial("game//images//Infos//BOSS_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[15] = CreateTextureMaterial("game//images//Infos//COROACUSTOM.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[16] = CreateTextureMaterial("game//images//Infos//COROACUSTOM_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[17] = CreateTextureMaterial("game//images//Infos//DROPBOSS.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[18] = CreateTextureMaterial("game//images//Infos//DROPBOSS_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[19] = CreateTextureMaterial("game//images//Infos//DURACAO.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[20] = CreateTextureMaterial("game//images//Infos//DURACAO_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[21] = CreateTextureMaterial("game//images//Infos//FALHA.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[22] = CreateTextureMaterial("game//images//Infos//FALHA_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[23] = CreateTextureMaterial("game//images//Infos//FREE.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[24] = CreateTextureMaterial("game//images//Infos//FREE_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[25] = CreateTextureMaterial("game//images//Infos//ITEMBOSS.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[26] = CreateTextureMaterial("game//images//Infos//ITEMBOSS_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[27] = CreateTextureMaterial("game//images//Infos//ITENS.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[28] = CreateTextureMaterial("game//images//Infos//ITENS_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[29] = CreateTextureMaterial("game//images//Infos//LIDERBLESS.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[30] = CreateTextureMaterial("game//images//Infos//LIDERBLESS_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[31] = CreateTextureMaterial("game//images//Infos//LIDERCLASSE.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[32] = CreateTextureMaterial("game//images//Infos//LIDERCLASSE_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[33] = CreateTextureMaterial("game//images//Infos//line.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[34] = CreateTextureMaterial("game//images//Infos//line2.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[35] = CreateTextureMaterial("game//images//Infos//line3.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[36] = CreateTextureMaterial("game//images//Infos//LINK.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[37] = CreateTextureMaterial("game//images//Infos//LINK_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[38] = CreateTextureMaterial("game//images//Infos//MAINS", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[39] = CreateTextureMaterial("game//images//Infos//MIXES.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[40] = CreateTextureMaterial("game//images//Infos//MIXES_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[41] = CreateTextureMaterial("game//images//Infos//pageage.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[42] = CreateTextureMaterial("game//images//Infos//pageboss.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[43] = CreateTextureMaterial("game//images//Infos//pageclanwin.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[44] = CreateTextureMaterial("game//images//Infos//pagecoroacustom.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[45] = CreateTextureMaterial("game//images//Infos//pagedropboss.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[46] = CreateTextureMaterial("game//images//Infos//pageduracaobp.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[47] = CreateTextureMaterial("game//images//Infos//pagefalha.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[48] = CreateTextureMaterial("game//images//Infos//pagefreebp.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[49] = CreateTextureMaterial("game//images//Infos//pageitemboss.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[50] = CreateTextureMaterial("game//images//Infos//pageitens.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[51] = CreateTextureMaterial("game//images//Infos//pagelink.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[52] = CreateTextureMaterial("game//images//Infos//pagemixes.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[53] = CreateTextureMaterial("game//images//Infos//pagepedraperf.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[54] = CreateTextureMaterial("game//images//Infos//pagepedras.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[55] = CreateTextureMaterial("game//images//Infos//pagepedrasage.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[56] = CreateTextureMaterial("game//images//Infos//pagepremiumbp.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[57] = CreateTextureMaterial("game//images//Infos//pagerecompensas.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[58] = CreateTextureMaterial("game//images//Infos//pagerecompensasbp.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[59] = CreateTextureMaterial("game//images//Infos//pagereiclasse.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[60] = CreateTextureMaterial("game//images//Infos//pagereidobc.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[61] = CreateTextureMaterial("game//images//Infos//pagereipvp.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[62] = CreateTextureMaterial("game//images//Infos//pagerespec.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[63] = CreateTextureMaterial("game//images//Infos//pagerestaure.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[64] = CreateTextureMaterial("game//images//Infos//pagereuniao.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[65] = CreateTextureMaterial("game//images//Infos//pagesala.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[66] = CreateTextureMaterial("game//images//Infos//pagesistemabp.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[67] = CreateTextureMaterial("game//images//Infos//pagetoys.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[68] = CreateTextureMaterial("game//images//Infos//pagetraje.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[69] = CreateTextureMaterial("game//images//Infos//pagevalelvl.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[70] = CreateTextureMaterial("game//images//Infos//PEDRAPERF.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[71] = CreateTextureMaterial("game//images//Infos//PEDRAPERF_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[72] = CreateTextureMaterial("game//images//Infos//PEDRAS.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[73] = CreateTextureMaterial("game//images//Infos//PEDRAS_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[74] = CreateTextureMaterial("game//images//Infos//PREMIOS.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[75] = CreateTextureMaterial("game//images//Infos//PREMIOS_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[76] = CreateTextureMaterial("game//images//Infos//PREMIUM.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[77] = CreateTextureMaterial("game//images//Infos//PREMIUM_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[78] = CreateTextureMaterial("game//images//Infos//PROCESSO.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[79] = CreateTextureMaterial("game//images//Infos//PROCESSO_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[80] = CreateTextureMaterial("game//images//Infos//REIDOBC.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[81] = CreateTextureMaterial("game//images//Infos//REIDOBC_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[82] = CreateTextureMaterial("game//images//Infos//REIPVP.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[83] = CreateTextureMaterial("game//images//Infos//REIPVP_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[84] = CreateTextureMaterial("game//images//Infos//RESPEC.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[85] = CreateTextureMaterial("game//images//Infos//RESPEC_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[86] = CreateTextureMaterial("game//images//Infos//RESTAURE.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[87] = CreateTextureMaterial("game//images//Infos//RESTAURE_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[88] = CreateTextureMaterial("game//images//Infos//REUNIAO.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[89] = CreateTextureMaterial("game//images//Infos//REUNIAO_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[90] = CreateTextureMaterial("game//images//Infos//SALAS.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[91] = CreateTextureMaterial("game//images//Infos//SALAS_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[92] = CreateTextureMaterial("game//images//Infos//SISTEMA.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[93] = CreateTextureMaterial("game//images//Infos//SISTEMA_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[94] = CreateTextureMaterial("game//images//Infos//TOYS.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[95] = CreateTextureMaterial("game//images//Infos//TOYS_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[96] = CreateTextureMaterial("game//images//Infos//TRAJES.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[97] = CreateTextureMaterial("game//images//Infos//TRAJES_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[98] = CreateTextureMaterial("game//images//Infos//VALELEVEL.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[99] = CreateTextureMaterial("game//images//Infos//VALELVEL_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[100] = CreateTextureMaterial("game//images//Infos//btnclose.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[101] = CreateTextureMaterial("game//images//Infos//btnclose_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	ReadTextures();
}


void NPCinfo::Draw()
{
	char szBuff[10] = { 0 };
	int WinX = 40, WinY = 70, cnt2 = 0;

	if (fOpen)
	{
		DrawImage(Image[10], WinX, WinY, 316, 362);


		if (MouseAction(255, 400, 73, 23))
			DrawImage(Image[101], 255, 400, 73, 23);
		else
			DrawImage(Image[100], 255, 400, 73, 23);

		if (MouseAction(WinX + 30, WinY + 68, 189, 34) || Tag == 1) //Y +20 line = 33 //15 line 2= 34/16 line3 = 35/29
			DrawImage(Image[9], WinX + 30, WinY + 68, 189, 34);
		else
			DrawImage(Image[8], WinX + 30, WinY + 68, 189, 34);
		DrawImage(Image[33], WinX + 21, WinY + 105 + 3, 191, 2);
		DrawImage(Image[35], WinX + 212, WinY + 60, 2, 285);

		///////////////
		if (MouseAction(WinX + 30, WinY + 105 + 7, 189, 34) || Tag == 2)
			DrawImage(Image[3], WinX + 30, WinY + 105 + 7, 189, 34);
		else
			DrawImage(Image[2], WinX + 30, WinY + 105 + 7, 189, 34);
		DrawImage(Image[33], WinX + 21, WinY + 105 + 42 + 3, 191, 2);

		//////////////

		if (MouseAction(WinX + 30, WinY + 105 + 42 + 7, 189, 34) || Tag == 3)
			DrawImage(Image[12], WinX + 30, WinY + 105 + 42 + 7, 189, 34);
		else
			DrawImage(Image[11], WinX + 30, WinY + 105 + 42 + 7, 189, 34);
		DrawImage(Image[33], WinX + 21, WinY + 105 + 84 + 3, 191, 2);

		///////////////

		if (MouseAction(WinX + 30, WinY + 105 + 84 + 7, 189, 34) || Tag == 4)
			DrawImage(Image[1], WinX + 30, WinY + 105 + 84 + 7, 189, 34);
		else
			DrawImage(Image[0], WinX + 30, WinY + 105 + 84 + 7, 189, 34);
		DrawImage(Image[33], WinX + 21, WinY + 105 + 126 + 3, 191, 2);

		///////////////

		if (MouseAction(WinX + 30, WinY + 105 + 126 + 3 + 7, 189, 34) || Tag == 5)
			DrawImage(Image[5], WinX + 30, WinY + 105 + 126 + 3 + 7, 189, 34);
		else
			DrawImage(Image[4], WinX + 30, WinY + 105 + 126 + 3 + 7, 189, 34);
		DrawImage(Image[33], WinX + 21, WinY + 105 + 168 + 6, 191, 2);

		//////////////

		if (MouseAction(WinX + 30, WinY + 105 + 168 + 3 + 7, 189, 34) || Tag == 6)
			DrawImage(Image[7], WinX + 30, WinY + 105 + 168 + 3 + 7, 189, 34);
		else
			DrawImage(Image[6], WinX + 30, WinY + 105 + 168 + 3 + 7, 189, 34);
		DrawImage(Image[33], WinX + 21, WinY + 105 + 210 + 6, 191, 2);

		//////////////


		if (Tag == 1)
		{
			if (MouseAction(WinX + 214, WinY + 63, 79, 26) || SubTag == 1) { //Y +20 line = 33 //15 line 2= 34/16 line3 = 35/29
				DrawImage(Image[28], WinX + 214, WinY + 63, 79, 26);
				DrawImage(Image[34], WinX + 212, WinY + 90, 84, 2);
				if (SubTag == 1)
					DrawImage(Image[50], 450, WinY, 316, 362);
			}
			else
			{
				DrawImage(Image[27], WinX + 214, WinY + 63, 79, 26);
				DrawImage(Image[34], WinX + 212, WinY + 90, 84, 2);
			}

			if (MouseAction(WinX + 214, WinY + 93, 79, 26) || SubTag == 2) {
				DrawImage(Image[99], WinX + 214, WinY + 93, 79, 26);
				DrawImage(Image[34], WinX + 212, WinY + 121, 84, 2);
				if (SubTag == 2)
					DrawImage(Image[69], 450, WinY, 316, 362);
			}
			else
			{
				DrawImage(Image[98], WinX + 214, WinY + 93, 79, 26);
				DrawImage(Image[34], WinX + 212, WinY + 121, 84, 2);
			}

			if (MouseAction(WinX + 214, WinY + 124, 79, 26) || SubTag == 3) {
				DrawImage(Image[97], WinX + 214, WinY + 124, 79, 26);
				DrawImage(Image[34], WinX + 212, WinY + 152, 84, 2);
				if (SubTag == 3)
					DrawImage(Image[68], 450, WinY, 316, 362);
			}
			else
			{
				DrawImage(Image[96], WinX + 214, WinY + 124, 79, 26);
				DrawImage(Image[34], WinX + 212, WinY + 152, 84, 2);
			}

			if (MouseAction(WinX + 214, WinY + 155, 79, 26) || SubTag == 4) {
				DrawImage(Image[95], WinX + 214, WinY + 155, 79, 26);
				DrawImage(Image[34], WinX + 212, WinY + 183, 84, 2);
				if (SubTag == 4)
					DrawImage(Image[67], 450, WinY, 316, 362);
			}
			else
			{
				DrawImage(Image[94], WinX + 214, WinY + 155, 79, 26);
				DrawImage(Image[34], WinX + 212, WinY + 183, 84, 2);
			}

			if (MouseAction(WinX + 214, WinY + 186, 79, 26) || SubTag == 5) {
				DrawImage(Image[14], WinX + 214, WinY + 186, 79, 26);
				DrawImage(Image[34], WinX + 212, WinY + 214, 84, 2);
				if (SubTag == 5)
					DrawImage(Image[42], 450, WinY, 316, 362);
			}
			else
			{
				DrawImage(Image[13], WinX + 214, WinY + 186, 79, 26);
				DrawImage(Image[34], WinX + 212, WinY + 214, 84, 2);
			}

			if (MouseAction(WinX + 214, WinY + 217, 79, 26) || SubTag == 6) {
				DrawImage(Image[26], WinX + 214, WinY + 217, 79, 26);
				DrawImage(Image[34], WinX + 212, WinY + 245, 84, 2);
				if (SubTag == 6)
					DrawImage(Image[49], 450, WinY, 316, 362);
			}
			else
			{
				DrawImage(Image[25], WinX + 214, WinY + 217, 79, 26);
				DrawImage(Image[34], WinX + 212, WinY + 245, 84, 2);
			}

			if (MouseAction(WinX + 214, WinY + 248, 79, 26) || SubTag == 7) {
				DrawImage(Image[18], WinX + 214, WinY + 248, 79, 26);
				DrawImage(Image[34], WinX + 212, WinY + 276, 84, 2);
				if (SubTag == 7)
					DrawImage(Image[45], 450, WinY, 316, 362);
			}
			else
			{
				DrawImage(Image[17], WinX + 214, WinY + 248, 79, 26);
				DrawImage(Image[34], WinX + 212, WinY + 276, 84, 2);
			}
		}

		if (Tag == 2)
		{
			if (MouseAction(WinX + 214, WinY + 63, 79, 26) || SubTag == 8) {//Y +20 line = 33 //15 line 2= 34/16 line3 = 35/29
				DrawImage(Image[93], WinX + 214, WinY + 63, 79, 26);
				DrawImage(Image[34], WinX + 212, WinY + 90, 84, 2);
				if (SubTag == 8)
					DrawImage(Image[66], 450, WinY, 316, 362);
			}
			else
				DrawImage(Image[92], WinX + 214, WinY + 63, 79, 26);
			DrawImage(Image[34], WinX + 212, WinY + 90, 84, 2);

			if (MouseAction(WinX + 214, WinY + 93, 79, 26) || SubTag == 9) {
				DrawImage(Image[75], WinX + 214, WinY + 93, 79, 26);
				DrawImage(Image[34], WinX + 212, WinY + 121, 84, 2);
				if (SubTag == 9)
					DrawImage(Image[58], 450, WinY, 316, 362);
			}
			else
				DrawImage(Image[74], WinX + 214, WinY + 93, 79, 26);
			DrawImage(Image[34], WinX + 212, WinY + 121, 84, 2);

			if (MouseAction(WinX + 214, WinY + 124, 79, 26) || SubTag == 10) {
				DrawImage(Image[77], WinX + 214, WinY + 124, 79, 26);
				DrawImage(Image[34], WinX + 212, WinY + 152, 84, 2);
				if (SubTag == 10)
					DrawImage(Image[56], 450, WinY, 316, 362);
			}
			else
				DrawImage(Image[76], WinX + 214, WinY + 124, 79, 26);
			DrawImage(Image[34], WinX + 212, WinY + 152, 84, 2);

			if (MouseAction(WinX + 214, WinY + 155, 79, 26) || SubTag == 11) {
				DrawImage(Image[24], WinX + 214, WinY + 155, 79, 26);
				DrawImage(Image[34], WinX + 212, WinY + 183, 84, 2);
				if (SubTag == 11)
					DrawImage(Image[48], 450, WinY, 316, 362);
			}
			else
				DrawImage(Image[23], WinX + 214, WinY + 155, 79, 26);
			DrawImage(Image[34], WinX + 212, WinY + 183, 84, 2);

			if (MouseAction(WinX + 214, WinY + 186, 79, 26) || SubTag == 12) {
				DrawImage(Image[20], WinX + 214, WinY + 186, 79, 26);
				DrawImage(Image[34], WinX + 212, WinY + 214, 84, 2);
				if (SubTag == 12)
					DrawImage(Image[46], 450, WinY, 316, 362);
			}
			else
				DrawImage(Image[19], WinX + 214, WinY + 186, 79, 26);
			DrawImage(Image[34], WinX + 212, WinY + 214, 84, 2);

		}

		if (Tag == 3)
		{
			if (MouseAction(WinX + 214, WinY + 63, 79, 26) || SubTag == 13) { //Y +20 line = 33 //15 line 2= 34/16 line3 = 35/29
				DrawImage(Image[40], WinX + 214, WinY + 63, 79, 26);
				DrawImage(Image[34], WinX + 212, WinY + 90, 84, 2);
				if (SubTag == 13)
					DrawImage(Image[52], 450, WinY, 316, 362);
			}
			else
				DrawImage(Image[39], WinX + 214, WinY + 63, 79, 26);
			DrawImage(Image[34], WinX + 212, WinY + 90, 84, 2);

			if (MouseAction(WinX + 214, WinY + 93, 79, 26) || SubTag == 14) {
				DrawImage(Image[85], WinX + 214, WinY + 93, 79, 26);
				DrawImage(Image[34], WinX + 212, WinY + 121, 84, 2);
				if (SubTag == 14)
					DrawImage(Image[62], 450, WinY, 316, 362);
			}
			else
				DrawImage(Image[84], WinX + 214, WinY + 93, 79, 26);
			DrawImage(Image[34], WinX + 212, WinY + 121, 84, 2);

			if (MouseAction(WinX + 214, WinY + 124, 79, 26) || SubTag == 15) {
				DrawImage(Image[71], WinX + 214, WinY + 124, 79, 26);
				DrawImage(Image[34], WinX + 212, WinY + 152, 84, 2);
				if (SubTag == 15)
					DrawImage(Image[53], 450, WinY, 316, 362);
			}
			else
				DrawImage(Image[70], WinX + 214, WinY + 124, 79, 26);
			DrawImage(Image[34], WinX + 212, WinY + 152, 84, 2);

			if (MouseAction(WinX + 214, WinY + 155, 79, 26) || SubTag == 16) {
				DrawImage(Image[73], WinX + 214, WinY + 155, 79, 26);
				DrawImage(Image[34], WinX + 212, WinY + 183, 84, 2);
				if (SubTag == 16)
					DrawImage(Image[54], 450, WinY, 316, 362);
			}
			else
				DrawImage(Image[72], WinX + 214, WinY + 155, 79, 26);
			DrawImage(Image[34], WinX + 212, WinY + 183, 84, 2);

		}

		if (Tag == 4)
		{
			if (MouseAction(WinX + 214, WinY + 63, 79, 26) || SubTag == 17) { //Y +20 line = 33 //15 line 2= 34/16 line3 = 35/29
				DrawImage(Image[79], WinX + 214, WinY + 63, 79, 26);
				DrawImage(Image[34], WinX + 212, WinY + 90, 84, 2);
				if (SubTag == 17)
					DrawImage(Image[41], 450, WinY, 316, 362);
			}
			else
			{
				DrawImage(Image[78], WinX + 214, WinY + 63, 79, 26);
				DrawImage(Image[34], WinX + 212, WinY + 90, 84, 2);
			}

			if (MouseAction(WinX + 214, WinY + 93, 79, 26) || SubTag == 18) {
				DrawImage(Image[22], WinX + 214, WinY + 93, 79, 26);
				DrawImage(Image[34], WinX + 212, WinY + 121, 84, 2);
				if (SubTag == 18)
					DrawImage(Image[47], 450, WinY, 316, 362);
			}
			else
				DrawImage(Image[21], WinX + 214, WinY + 93, 79, 26);
			DrawImage(Image[34], WinX + 212, WinY + 121, 84, 2);

			if (MouseAction(WinX + 214, WinY + 124, 79, 26) || SubTag == 19) {
				DrawImage(Image[87], WinX + 214, WinY + 124, 79, 26);
				DrawImage(Image[34], WinX + 212, WinY + 152, 84, 2);
				if (SubTag == 19)
					DrawImage(Image[63], 450, WinY, 316, 362);
			}
			else
				DrawImage(Image[86], WinX + 214, WinY + 124, 79, 26);
			DrawImage(Image[34], WinX + 212, WinY + 152, 84, 2);

			if (MouseAction(WinX + 214, WinY + 155, 79, 26) || SubTag == 20) {
				DrawImage(Image[73], WinX + 214, WinY + 155, 79, 26);
				DrawImage(Image[34], WinX + 212, WinY + 183, 84, 2);
				if (SubTag == 20)
					DrawImage(Image[55], 450, WinY, 316, 362);
			}
			else
				DrawImage(Image[72], WinX + 214, WinY + 155, 79, 26);
			DrawImage(Image[34], WinX + 212, WinY + 183, 84, 2);

		}

		if (Tag == 5)
		{
			if (MouseAction(WinX + 214, WinY + 63, 79, 26) || SubTag == 21) {//Y +20 line = 33 //15 line 2= 34/16 line3 = 35/29
				DrawImage(Image[83], WinX + 214, WinY + 63, 79, 26);
				DrawImage(Image[34], WinX + 212, WinY + 90, 84, 2);
				if (SubTag == 21)
					DrawImage(Image[61], 450, WinY, 316, 362);
			}
			else
				DrawImage(Image[82], WinX + 214, WinY + 63, 79, 26);
			DrawImage(Image[34], WinX + 212, WinY + 90, 84, 2);

			if (MouseAction(WinX + 214, WinY + 93, 79, 26) || SubTag == 22) {
				DrawImage(Image[32], WinX + 214, WinY + 93, 79, 26);
				DrawImage(Image[34], WinX + 212, WinY + 121, 84, 2);
				if (SubTag == 22)
					DrawImage(Image[59], 450, WinY, 316, 362);
			}
			else
				DrawImage(Image[31], WinX + 214, WinY + 93, 79, 26);
			DrawImage(Image[34], WinX + 212, WinY + 121, 84, 2);

			if (MouseAction(WinX + 214, WinY + 124, 79, 26) || SubTag == 23) {
				DrawImage(Image[30], WinX + 214, WinY + 124, 79, 26);
				DrawImage(Image[34], WinX + 212, WinY + 152, 84, 2);
				if (SubTag == 23)
					DrawImage(Image[43], 450, WinY, 316, 362);
			}
			else
				DrawImage(Image[29], WinX + 214, WinY + 124, 79, 26);
			DrawImage(Image[34], WinX + 212, WinY + 152, 84, 2);

			if (MouseAction(WinX + 214, WinY + 155, 79, 26) || SubTag == 24) {
				DrawImage(Image[16], WinX + 214, WinY + 155, 79, 26);
				DrawImage(Image[34], WinX + 212, WinY + 183, 84, 2);
				if (SubTag == 24)
					DrawImage(Image[44], 450, WinY, 316, 362);
			}
			else
				DrawImage(Image[15], WinX + 214, WinY + 155, 79, 26);
			DrawImage(Image[34], WinX + 212, WinY + 183, 84, 2);

			if (MouseAction(WinX + 214, WinY + 186, 79, 26) || SubTag == 25) {
				DrawImage(Image[81], WinX + 214, WinY + 186, 79, 26);
				DrawImage(Image[34], WinX + 212, WinY + 214, 84, 2);
				if (SubTag == 25)
					DrawImage(Image[60], 450, WinY, 316, 362);
			}
			else
				DrawImage(Image[80], WinX + 214, WinY + 186, 79, 26);
			DrawImage(Image[34], WinX + 212, WinY + 214, 84, 2);
		}

		if (Tag == 6)
		{
			if (MouseAction(WinX + 214, WinY + 63, 79, 26) || SubTag == 26) { //Y +20 line = 33 //15 line 2= 34/16 line3 = 35/29
				DrawImage(Image[89], WinX + 214, WinY + 63, 79, 26);
				DrawImage(Image[34], WinX + 212, WinY + 90, 84, 2);
				if (SubTag == 26)
					DrawImage(Image[64], 450, WinY, 316, 362);
			}
			else
				DrawImage(Image[88], WinX + 214, WinY + 63, 79, 26);
			DrawImage(Image[34], WinX + 212, WinY + 90, 84, 2);

			if (MouseAction(WinX + 214, WinY + 93, 79, 26) || SubTag == 27) {
				DrawImage(Image[91], WinX + 214, WinY + 93, 79, 26);
				DrawImage(Image[34], WinX + 212, WinY + 121, 84, 2);
				if (SubTag == 27)
					DrawImage(Image[65], 450, WinY, 316, 362);
			}
			else
				DrawImage(Image[90], WinX + 214, WinY + 93, 79, 26);
			DrawImage(Image[34], WinX + 212, WinY + 121, 84, 2);

			if (MouseAction(WinX + 214, WinY + 124, 79, 26) || SubTag == 28) {
				DrawImage(Image[37], WinX + 214, WinY + 124, 79, 26);
				DrawImage(Image[34], WinX + 212, WinY + 152, 84, 2);
				if (SubTag == 28)
					DrawImage(Image[51], 450, WinY, 316, 362);
			}
			else
				DrawImage(Image[36], WinX + 214, WinY + 124, 79, 26);
			DrawImage(Image[34], WinX + 212, WinY + 152, 84, 2);
		}
	}
}

void NPCinfo::Close()
{
	fOpen = false;
	SubTag = 0;
	Tag = 0;
}

void NPCinfo::Button()
{
	if (fOpen)
	{
		int WinX = 40, WinY = 70, cnt2 = 0;//Y +20 line = 33 //15 line 2= 34/16 line3 = 35/29
		if (MouseAction(WinX + 30, WinY + 68, 189, 34) && Tag != 1)
			Tag = 1;
		if (MouseAction(WinX + 30, WinY + 105 + 7, 189, 34) && Tag != 2)
			Tag = 2;
		if (MouseAction(WinX + 30, WinY + 105 + 42 + 7, 189, 34) && Tag != 3)
			Tag = 3;
		if (MouseAction(WinX + 30, WinY + 105 + 84 + 7, 189, 34) && Tag != 4)
			Tag = 4;
		if (MouseAction(WinX + 30, WinY + 105 + 126 + 3 + 7, 189, 34) && Tag != 5)
			Tag = 5;
		if (MouseAction(WinX + 30, WinY + 105 + 168 + 3 + 7, 189, 34) && Tag != 6)
			Tag = 6;


		if (Tag == 1)
		{
			if (MouseAction(WinX + 214, WinY + 63, 79, 26) && SubTag != 1)
			{
				SubTag = 1;
				Scroll = 0;
			}

			if (MouseAction(WinX + 214, WinY + 93, 79, 26) && SubTag != 2)
			{
				SubTag = 2;
				Scroll = 0;
			}

			if (MouseAction(WinX + 214, WinY + 124, 79, 26) && SubTag != 3)
			{
				SubTag = 3;
				Scroll = 0;
			}

			if (MouseAction(WinX + 214, WinY + 155, 79, 26) && SubTag != 4)
			{
				SubTag = 4;
				Scroll = 0;
			}

			if (MouseAction(WinX + 214, WinY + 186, 79, 26) && SubTag != 5)
			{
				SubTag = 5;
				Scroll = 0;
			}

			if (MouseAction(WinX + 214, WinY + 217, 79, 26) && SubTag != 6)
			{
				SubTag = 6;
				Scroll = 0;
			}

			if (MouseAction(WinX + 214, WinY + 248, 79, 26) && SubTag != 7)
			{
				SubTag = 7;
				Scroll = 0;
			}
		}



		if (Tag == 2)
		{
			if (MouseAction(WinX + 214, WinY + 63, 79, 26) && SubTag != 8)
			{
				SubTag = 8;
				Scroll = 0;
			}

			if (MouseAction(WinX + 214, WinY + 93, 79, 26) && SubTag != 9)
			{
				SubTag = 9;
				Scroll = 0;
			}

			if (MouseAction(WinX + 214, WinY + 124, 79, 26) && SubTag != 10)
			{
				SubTag = 10;
				Scroll = 0;
			}

			if (MouseAction(WinX + 214, WinY + 155, 79, 26) && SubTag != 11)
			{
				SubTag = 11;
				Scroll = 0;
			}

			if (MouseAction(WinX + 214, WinY + 186, 79, 26) && SubTag != 12)
			{
				SubTag = 12;
				Scroll = 0;
			}
		}

		if (Tag == 3)
		{
			if (MouseAction(WinX + 214, WinY + 63, 79, 26) && SubTag != 13)
			{
				SubTag = 13;
				Scroll = 0;
			}

			if (MouseAction(WinX + 214, WinY + 93, 79, 26) && SubTag != 14)
			{
				SubTag = 14;
				Scroll = 0;
			}

			if (MouseAction(WinX + 214, WinY + 124, 79, 26) && SubTag != 15)
			{
				SubTag = 15;
				Scroll = 0;
			}

			if (MouseAction(WinX + 214, WinY + 155, 79, 26) && SubTag != 16)
			{
				SubTag = 16;
				Scroll = 0;
			}
		}

		if (Tag == 4)
		{
			if (MouseAction(WinX + 214, WinY + 63, 79, 26) && SubTag != 17)
			{
				SubTag = 17;
				Scroll = 0;
			}

			if (MouseAction(WinX + 214, WinY + 93, 79, 26) && SubTag != 18)
			{
				SubTag = 18;
				Scroll = 0;
			}

			if (MouseAction(WinX + 214, WinY + 124, 79, 26) && SubTag != 19)
			{
				SubTag = 19;
				Scroll = 0;
			}

			if (MouseAction(WinX + 214, WinY + 155, 79, 26) && SubTag != 20)
			{
				SubTag = 20;
				Scroll = 0;
			}
		}

		if (Tag == 5)
		{
			if (MouseAction(WinX + 214, WinY + 63, 79, 26) && SubTag != 21)
			{
				SubTag = 21;
				Scroll = 0;
			}

			if (MouseAction(WinX + 214, WinY + 93, 79, 26) && SubTag != 22)
			{
				SubTag = 22;
				Scroll = 0;
			}

			if (MouseAction(WinX + 214, WinY + 124, 79, 26) && SubTag != 23)
			{
				SubTag = 23;
				Scroll = 0;
			}

			if (MouseAction(WinX + 214, WinY + 155, 79, 26) && SubTag != 24)
			{
				SubTag = 24;
				Scroll = 0;
			}

			if (MouseAction(WinX + 214, WinY + 186, 79, 26) && SubTag != 25)
			{
				SubTag = 25;
				Scroll = 0;
			}
		}

		if (Tag == 6)
		{
			if (MouseAction(WinX + 214, WinY + 63, 79, 26) && SubTag != 26)
			{
				SubTag = 26;
				Scroll = 0;
			}

			if (MouseAction(WinX + 214, WinY + 93, 79, 26) && SubTag != 27)
			{
				SubTag = 27;
				Scroll = 0;
			}

			if (MouseAction(WinX + 214, WinY + 124, 79, 26) && SubTag != 28)
			{
				SubTag = 28;
				Scroll = 0;
			}
		}
	}
	if (MouseAction(255, 400, 73, 23))
		Close();
}







