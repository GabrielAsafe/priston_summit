#include "compat/legacy/sin/sinLinkHeader.h"
#include "systems/progression/battle_pass/BattlePass.h"
#include <string>

BP_PACK BPList;
BP_ITEM BPselectitem;

extern void DrawImage(int Image, int x, int y, int w, int h);

void BattlePass::RecvItemToServer(BP_PACK * Shop)
{
	char Dir[1024];
	CopyMemory(&BPList, Shop, sizeof(BP_PACK));

	for (int cnt = 0; cnt < 70; cnt++)
	{
		wsprintf(Dir, "game//images//battlepass//Resgate//%d.bmp", BPList.item[cnt].Tag);
		BPList.item[cnt].Image = LoadDibSurfaceOffscreen(Dir);
	}
}

void BattlePass::SendItemToServer(BP_ITEM item)
{
		sITEM TempIt;
		
		if (22 > 110)
		{
			TempIt.w = 22;
			TempIt.h = 110;
		}
		else if (22 < 110)
		{
			TempIt.w = 22;
			TempIt.h = 110;
		}
		else
		{
			TempIt.w = 22;
			TempIt.h = 110;
		}


		if (sinChar->Weight[0] + 20 >= sinChar->Weight[1])
		{
			cMessageBox.ShowMessage(MESSAGE_OVER_WEIGHT);
		}
		else if (!cInvenTory.CheckSetEmptyArea(&TempIt))
		{
			cMessageBox.ShowMessage(MESSAGE_OVER_SPACE);
		}
		else if (sinChar->Money + 2500000 > 1000000000)
		{
			cMessageBox.ShowMessage(MESSAGE_OVER_MONEY);
		}
		else
		{
			BPselectitem = item;
			BP_RESGATE packet;
			CopyMemory(&packet.item, &BPselectitem, sizeof(BP_ITEM));
			packet.size = sizeof(BP_RESGATE);
			if (BattlePass::GetInstance()->IsOpen())
				packet.code = BP_PACKET_SEND_ITEM_SERVER;
			if (smWsockUserServer)
				smWsockUserServer->Send((char *)&packet, packet.size, TRUE);
		}

}

void BattlePass::RecvBPInfoToServer(BP_INFO * BattlePass)
{
	Exp = BattlePass->exp;
	Lvl = BattlePass->lvl;
	tipoBP = BattlePass->type;
	for (int i = 0; i < sizeof(itemResg); i++) {
		itemResg[i] = BattlePass->itemResg[i];
	}
	
}

bool BattlePass::MouseAction(int x, int y, int w, int h)
{
	if (pRealCursorPos.x > x && pRealCursorPos.x < x + w && pRealCursorPos.y > y && pRealCursorPos.y < y + h)
		return TRUE;

	return FALSE;
}

void BattlePass::Open()
{
	fOpen = true;
}

void BattlePass::Init()
{
	Image[0] = CreateTextureMaterial("game//images//battlepass//Select_Item_Free.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[1] = CreateTextureMaterial("game//images//battlepass//btnclose.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[2] = CreateTextureMaterial("game//images//battlepass//btnclose_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[3] = CreateTextureMaterial("game//images//battlepass//Select_Item_Premium.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[4] = CreateTextureMaterial("game//images//battlepass//Page_1-6.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[5] = CreateTextureMaterial("game//images//battlepass//Page_2-6.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[6] = CreateTextureMaterial("game//images//battlepass//Page_3-6.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[7] = CreateTextureMaterial("game//images//battlepass//Page_4-6.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[8] = CreateTextureMaterial("game//images//battlepass//Page_5-6.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[9] = CreateTextureMaterial("game//images//battlepass//Page_6-6.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[10] = CreateTextureMaterial("game//images//battlepass//Page_Left.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[11] = CreateTextureMaterial("game//images//battlepass//Page_Left_1.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[12] = CreateTextureMaterial("game//images//battlepass//Page_Right.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[13] = CreateTextureMaterial("game//images//battlepass//Page_Right_1.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[14] = CreateTextureMaterial("game//images//battlepass//Resgate//0.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[15] = CreateTextureMaterial("game//images//battlepass//Resgate//1.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[16] = CreateTextureMaterial("game//images//battlepass//Resgate//2.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[17] = CreateTextureMaterial("game//images//battlepass//Resgate//3.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[18] = CreateTextureMaterial("game//images//battlepass//Resgate//4.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[19] = CreateTextureMaterial("game//images//battlepass//Resgate//5.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[20] = CreateTextureMaterial("game//images//battlepass//Resgate//6.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[21] = CreateTextureMaterial("game//images//battlepass//Resgate//7.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[22] = CreateTextureMaterial("game//images//battlepass//Resgate//8.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[23] = CreateTextureMaterial("game//images//battlepass//Resgate//9.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[24] = CreateTextureMaterial("game//images//battlepass//Resgate//10.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[25] = CreateTextureMaterial("game//images//battlepass//Resgate//11.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[26] = CreateTextureMaterial("game//images//battlepass//Resgate//12.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[27] = CreateTextureMaterial("game//images//battlepass//Resgate//13.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[28] = CreateTextureMaterial("game//images//battlepass//Resgate//14.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[29] = CreateTextureMaterial("game//images//battlepass//Resgate//15.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[30] = CreateTextureMaterial("game//images//battlepass//Resgate//16.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[31] = CreateTextureMaterial("game//images//battlepass//Resgate//17.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[32] = CreateTextureMaterial("game//images//battlepass//Resgate//18.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[33] = CreateTextureMaterial("game//images//battlepass//Resgate//19.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[34] = CreateTextureMaterial("game//images//battlepass//Resgate//20.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[35] = CreateTextureMaterial("game//images//battlepass//Resgate//21.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[36] = CreateTextureMaterial("game//images//battlepass//Resgate//22.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[37] = CreateTextureMaterial("game//images//battlepass//Resgate//23.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[38] = CreateTextureMaterial("game//images//battlepass//Resgate//24.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[39] = CreateTextureMaterial("game//images//battlepass//Resgate//25.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[40] = CreateTextureMaterial("game//images//battlepass//Resgate//26.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[41] = CreateTextureMaterial("game//images//battlepass//Resgate//27.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[42] = CreateTextureMaterial("game//images//battlepass//Resgate//28.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[43] = CreateTextureMaterial("game//images//battlepass//Resgate//29.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[44] = CreateTextureMaterial("game//images//battlepass//Resgate//30.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[45] = CreateTextureMaterial("game//images//battlepass//Resgate//31.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[46] = CreateTextureMaterial("game//images//battlepass//Resgate//32.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[47] = CreateTextureMaterial("game//images//battlepass//Resgate//33.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[48] = CreateTextureMaterial("game//images//battlepass//Resgate//34.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[49] = CreateTextureMaterial("game//images//battlepass//Resgate//35.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[50] = CreateTextureMaterial("game//images//battlepass//Resgate//36.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[51] = CreateTextureMaterial("game//images//battlepass//Resgate//37.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[52] = CreateTextureMaterial("game//images//battlepass//Resgate//38.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[53] = CreateTextureMaterial("game//images//battlepass//Resgate//39.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[54] = CreateTextureMaterial("game//images//battlepass//Resgate//0_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[55] = CreateTextureMaterial("game//images//battlepass//Resgate//1_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[56] = CreateTextureMaterial("game//images//battlepass//Resgate//2_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[57] = CreateTextureMaterial("game//images//battlepass//Resgate//3_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[58] = CreateTextureMaterial("game//images//battlepass//Resgate//4_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[59] = CreateTextureMaterial("game//images//battlepass//Resgate//5_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[60] = CreateTextureMaterial("game//images//battlepass//Resgate//6_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[61] = CreateTextureMaterial("game//images//battlepass//Resgate//7_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[62] = CreateTextureMaterial("game//images//battlepass//Resgate//8_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[63] = CreateTextureMaterial("game//images//battlepass//Resgate//9_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[64] = CreateTextureMaterial("game//images//battlepass//Resgate//10_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[65] = CreateTextureMaterial("game//images//battlepass//Resgate//11_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[66] = CreateTextureMaterial("game//images//battlepass//Resgate//12_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[67] = CreateTextureMaterial("game//images//battlepass//Resgate//13_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[68] = CreateTextureMaterial("game//images//battlepass//Resgate//14_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[69] = CreateTextureMaterial("game//images//battlepass//Resgate//15_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[70] = CreateTextureMaterial("game//images//battlepass//Resgate//16_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[71] = CreateTextureMaterial("game//images//battlepass//Resgate//17_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[72] = CreateTextureMaterial("game//images//battlepass//Resgate//18_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[73] = CreateTextureMaterial("game//images//battlepass//Resgate//19_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[74] = CreateTextureMaterial("game//images//battlepass//Resgate//20_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[75] = CreateTextureMaterial("game//images//battlepass//Resgate//21_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[76] = CreateTextureMaterial("game//images//battlepass//Resgate//22_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[77] = CreateTextureMaterial("game//images//battlepass//Resgate//23_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[78] = CreateTextureMaterial("game//images//battlepass//Resgate//24_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[79] = CreateTextureMaterial("game//images//battlepass//Resgate//25_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[80] = CreateTextureMaterial("game//images//battlepass//Resgate//26_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[81] = CreateTextureMaterial("game//images//battlepass//Resgate//27_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[82] = CreateTextureMaterial("game//images//battlepass//Resgate//28_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[83] = CreateTextureMaterial("game//images//battlepass//Balao_Info_BP.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[84] = CreateTextureMaterial("game//images//battlepass//HUD_PRINCIPAL_PASSE_BATALHA.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[85] = CreateTextureMaterial("game//images//battlepass//EXP_BAR.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[86] = CreateTextureMaterial("game//images//battlepass//BUTTON_REDEEN.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[87] = CreateTextureMaterial("game//images//battlepass//BUTTON_REDEEN_1.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[88] = CreateTextureMaterial("game//images//battlepass//0.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[89] = CreateTextureMaterial("game//images//battlepass//1.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[90] = CreateTextureMaterial("game//images//battlepass//2.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[91] = CreateTextureMaterial("game//images//battlepass//3.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[92] = CreateTextureMaterial("game//images//battlepass//4.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[93] = CreateTextureMaterial("game//images//battlepass//5.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[94] = CreateTextureMaterial("game//images//battlepass//6.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[95] = CreateTextureMaterial("game//images//battlepass//7.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[96] = CreateTextureMaterial("game//images//battlepass//8.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[97] = CreateTextureMaterial("game//images//battlepass//9.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[98] = CreateTextureMaterial("game//images//battlepass//Resgate//29_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[99] = CreateTextureMaterial("game//images//battlepass//Resgate//30_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[100] = CreateTextureMaterial("game//images//battlepass//Resgate//31_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[101] = CreateTextureMaterial("game//images//battlepass//Resgate//32_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[102] = CreateTextureMaterial("game//images//battlepass//Resgate//33_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[103] = CreateTextureMaterial("game//images//battlepass//Resgate//34_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[104] = CreateTextureMaterial("game//images//battlepass//Resgate//35_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[105] = CreateTextureMaterial("game//images//battlepass//Resgate//36_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[106] = CreateTextureMaterial("game//images//battlepass//Resgate//37_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[107] = CreateTextureMaterial("game//images//battlepass//Resgate//38_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	Image[108] = CreateTextureMaterial("game//images//battlepass//Resgate//39_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	ReadTextures();
}

void BattlePass::Draw()
{
	char strBuff[64];
	memset(strBuff, 0, sizeof(strBuff));
	char szBuff[10] = { 0 };
	int WinX = 40, WinY = 70, cnt2 = 0;

	if (fOpen)
	{
		//MOSTRA A IMAGEM DO HUD PRINCPAL
		DrawImage(Image[84], WinX, WinY, 800, 600);

		//MOSTRA A IMAGEM DA BARRA DE EXP
		DrawImage(Image[85], WinX + 174, WinY + 480, Exp/3, 39);

		//MOSTRA O VALOR ATUAL DA EXP
		wsprintf(szBuff, "%d / 999", Exp);
		DrawFontText(WinX + 338 - strlen(szBuff) * 3, WinY + 493, szBuff, RGB(255, 255, 255), false);

		//MOSTRA AS IMAGENS DAS RECOMPENSAS DO PASSE DE BATALHA
		if (page == 0)
		{
			DrawImage(Image[14], 124 + (65 * 0), WinY + 175, 130, 130);

			DrawImage(Image[15], 60 + (65 * 1), WinY + 313, 130, 130);

			DrawImage(Image[16], 124 + (65 * 2), WinY + 175, 130, 130);

			DrawImage(Image[17], 60 + (65 * 3), WinY + 313, 130, 130);

			DrawImage(Image[18], 124 + (65 * 4), WinY + 175, 130, 130);

			DrawImage(Image[19], 60 + (65 * 5), WinY + 313, 130, 130);

			DrawImage(Image[20], 124 + (65 * 6), WinY + 175, 130, 130);

			DrawImage(Image[21], 60 + (65 * 7), WinY + 313, 130, 130);

			DrawImage(Image[22], 124 + (65 * 8), WinY + 175, 130, 130);

			DrawImage(Image[23], 60 + (65 * 9), WinY + 313, 130, 130);

			//MOSTRA A IMAGEM DO ITEM RESGATADO
			for (int i = 0; i < 10; i++) {
				if (itemResg[i] == 1) {
					if (i % 2 == 0) {
						DrawImage(Image[54 + i], 124 + (65 * i), WinY + 175, 130, 130);
					}
					else {
						DrawImage(Image[54 + i], 60 + (65 * i), WinY + 313, 130, 130);
					}
				}
			}
		}
		else if (page == 1)
		{
			DrawImage(Image[24], 124 + (65 * 0), WinY + 175, 130, 130);

			DrawImage(Image[25], 60 + (65 * 1), WinY + 313, 130, 130);

			DrawImage(Image[26], 124 + (65 * 2), WinY + 175, 130, 130);

			DrawImage(Image[27], 60 + (65 * 3), WinY + 313, 130, 130);

			DrawImage(Image[28], 124 + (65 * 4), WinY + 175, 130, 130);

			DrawImage(Image[29], 60 + (65 * 5), WinY + 313, 130, 130);

			DrawImage(Image[30], 124 + (65 * 6), WinY + 175, 130, 130);

			DrawImage(Image[31], 60 + (65 * 7), WinY + 313, 130, 130);

			DrawImage(Image[32], 124 + (65 * 8), WinY + 175, 130, 130);

			DrawImage(Image[33], 60 + (65 * 9), WinY + 313, 130, 130);

			//MOSTRA A IMAGEM DO ITEM RESGATADO
			for (int i = 10; i < 20; i++) {
				if (itemResg[i] == 1) {
					if (i % 2 == 0) {
						DrawImage(Image[54 + i], 124 + (65 * (i-10)), WinY + 175, 130, 130);
					}
					else {
						DrawImage(Image[54 + i], 60 + (65 * (i - 10)), WinY + 313, 130, 130);
					}
				}
			}
		}
		else if (page == 2)
		{
			DrawImage(Image[34], 124 + (65 * 0), WinY + 175, 130, 130);

			DrawImage(Image[35], 60 + (65 * 1), WinY + 313, 130, 130);

			DrawImage(Image[36], 124 + (65 * 2), WinY + 175, 130, 130);

			DrawImage(Image[37], 60 + (65 * 3), WinY + 313, 130, 130);

			DrawImage(Image[38], 124 + (65 * 4), WinY + 175, 130, 130);

			DrawImage(Image[39], 60 + (65 * 5), WinY + 313, 130, 130);

			DrawImage(Image[40], 124 + (65 * 6), WinY + 175, 130, 130);

			DrawImage(Image[41], 60 + (65 * 7), WinY + 313, 130, 130);

			DrawImage(Image[42], 124 + (65 * 8), WinY + 175, 130, 130);

			DrawImage(Image[43], 60 + (65 * 9), WinY + 313, 130, 130);

			//MOSTRA A IMAGEM DO ITEM RESGATADO
			for (int i = 20; i < 30; i++) {
				if (itemResg[i] == 1) {
					if (i % 2 == 0) {
						if (i < 29)
							DrawImage(Image[54 + i], 124 + (65 * (i - 20)), WinY + 175, 130, 130);
						else
							DrawImage(Image[98], 124 + (65 * (i - 20)), WinY + 175, 130, 130);
					}
					else {
						if (i < 29)
							DrawImage(Image[54 + i], 60 + (65 * (i - 20)), WinY + 313, 130, 130);
						else
							DrawImage(Image[98], 60 + (65 * (i - 20)), WinY + 313, 130, 130);
					}
				}
			}
		}
		else if (page == 3)
		{
			DrawImage(Image[44], 124 + (65 * 0), WinY + 175, 130, 130);

			DrawImage(Image[45], 60 + (65 * 1), WinY + 313, 130, 130);

			DrawImage(Image[46], 124 + (65 * 2), WinY + 175, 130, 130);

			DrawImage(Image[47], 60 + (65 * 3), WinY + 313, 130, 130);

			DrawImage(Image[48], 124 + (65 * 4), WinY + 175, 130, 130);

			DrawImage(Image[49], 60 + (65 * 5), WinY + 313, 130, 130);

			DrawImage(Image[50], 124 + (65 * 6), WinY + 175, 130, 130);

			DrawImage(Image[51], 60 + (65 * 7), WinY + 313, 130, 130);

			DrawImage(Image[52], 124 + (65 * 8), WinY + 175, 130, 130);

			DrawImage(Image[53], 60 + (65 * 9), WinY + 313, 130, 130);

			//MOSTRA A IMAGEM DO ITEM RESGATADO
			for (int i = 30; i < 40; i++) {
				if (itemResg[i] == 1) {
					if (i % 2 == 0) {
						DrawImage(Image[99 + (i-30)], 124 + (65 * (i - 30)), WinY + 175, 130, 130);
					}
					else {
						DrawImage(Image[99 + (i-30)], 60 + (65 * (i - 30)), WinY + 313, 130, 130);
					}
				}
			}
		}
		//MOSTRA A IMAGEM DO BOTÃO FECHAR
		if (MouseAction(800, 80, 22, 22))
			DrawImage(Image[2], 800, 80, 22, 22);
		else
			DrawImage(Image[1], 800, 80, 22, 22);

		//MOSTRA A IMAGEM DO BOTÃO RESGATAR
		if (MouseAction(670, 539, 101, 51))
			DrawImage(Image[87], 670, 539, 101, 51);
		else
			DrawImage(Image[86], 670, 539, 101, 51);

		//MOSTRA A IMAGEM DO BOTÃO PAGINA ESQUERDA
		if (MouseAction(545, 539, 52, 52))
			DrawImage(Image[11], 545, 539, 52, 52);
		else
			DrawImage(Image[10], 545, 539, 52, 52);

		//MOSTRA A IMAGEM DO BOTÃO PAGINA DIREIRA
		if (MouseAction(605, 539, 52, 52))
			DrawImage(Image[13], 605, 539, 52, 52);
		else
			DrawImage(Image[12], 605, 539, 52, 52);

		//MOSTRA A IMAGEM DA REFERÊNCIA DA PÁGINA ATUAL
		for (int cnt = 0; cnt < 4; cnt++)
		{
			if (cnt == page)
				DrawImage(Image[4 + cnt], 554, 594, 95, 28);

			if (page < 0)
				DrawImage(Image[9], 554, 594, 95, 28);

			if (page > 5)
				DrawImage(Image[4], 554, 594, 95, 28);
		}

		//MOSTRA A IMAGEM DE LAYOUT AO SELECIONAR UMA RECOMPENSA
		for (int cnt = 0; cnt < 10; cnt++)
		{
			if (Itselect == cnt + 1)
			{
				if (cnt % 2 == 0 && itemResg[(Itselect + (page * 10) - 1)] == 0)
					DrawImage(Image[0], 115 + (65 * cnt), WinY + 168, 150, 150);
				else if (cnt % 2 > 0 && itemResg[(Itselect + (page * 10) - 1)] == 0)
					DrawImage(Image[3], 115 + (cnt * 65) - 65, WinY + 302, 150, 150);
			}
		}

		//MOSTRA AS IMAGENS DO NIVEL REQUERIDO
		for (int cnt = 0 + (page * 5); cnt < 5 + (page * 5); cnt++)
		{
			if (cnt < 9)
				DrawImage(Image[89 + cnt], WinX + 125 + (cnt * 133) - (665 * page), WinY + 138, 32, 32);
			else if (cnt < 19)
			{
				DrawImage(Image[89], WinX + 120 + (cnt * 133) - (665 * page), WinY + 138, 32, 32);
				DrawImage(Image[(88 + cnt) - 9], WinX + 140 + (cnt * 133) - (665 * page), WinY + 138, 32, 32);
			}
			else if (cnt < 29)
			{
				DrawImage(Image[90], WinX + 120 + (cnt * 133) - (665 * page), WinY + 138, 32, 32);
				DrawImage(Image[(88 + cnt) - 19], WinX + 140 + (cnt * 133) - (665 * page), WinY + 138, 32, 32);
			}
			else if (cnt < 30)
			{
				DrawImage(Image[91], WinX + 120 + (cnt * 133) - (665 * page), WinY + 138, 32, 32);
				DrawImage(Image[88], WinX + 140 + (cnt * 133) - (665 * page), WinY + 138, 32, 32);
			}
		}

		//MOSTRA AS IMAGENS DO NIVEL ATUAL DO PASSE DE BATALHA
		for (int cnt = 0; cnt < 99; cnt++)
		{
			if (cnt == Lvl - 1)
			{
				if (cnt < 9)
					DrawImage(Image[89 + cnt], WinX + 110, WinY + 482, 32, 32);
				else if (cnt < 19)
				{
					DrawImage(Image[89], WinX + 99, WinY + 482, 32, 32);
					DrawImage(Image[(88 + cnt) - 9], WinX + 118, WinY + 482, 32, 32);
				}
				else if (cnt < 29)
				{
					DrawImage(Image[90], WinX + 99, WinY + 482, 32, 32);
					DrawImage(Image[(88 + cnt) - 19], WinX + 118, WinY + 482, 32, 32);
				}
				else if (cnt < 39)
				{
					DrawImage(Image[91], WinX + 99, WinY + 482, 32, 32);
					DrawImage(Image[(88 + cnt) - 29], WinX + 118, WinY + 482, 32, 32);
				}
				else if (cnt < 49)
				{
					DrawImage(Image[92], WinX + 99, WinY + 482, 32, 32);
					DrawImage(Image[(88 + cnt) - 39], WinX + 118, WinY + 482, 32, 32);
				}
				else if (cnt < 59)
				{
					DrawImage(Image[93], WinX + 99, WinY + 482, 32, 32);
					DrawImage(Image[(88 + cnt) - 49], WinX + 118, WinY + 482, 32, 32);
				}
				else if (cnt < 69)
				{
					DrawImage(Image[94], WinX + 99, WinY + 482, 32, 32);
					DrawImage(Image[(88 + cnt) - 59], WinX + 118, WinY + 482, 32, 32);
				}
				else if (cnt < 79)
				{
					DrawImage(Image[95], WinX + 99, WinY + 482, 32, 32);
					DrawImage(Image[(88 + cnt) - 69], WinX + 118, WinY + 482, 32, 32);
				}
				else if (cnt < 89)
				{
					DrawImage(Image[96], WinX + 99, WinY + 482, 32, 32);
					DrawImage(Image[(88 + cnt) - 79], WinX + 118, WinY + 482, 32, 32);
				}
				else if (cnt < 99)
				{
					DrawImage(Image[97], WinX + 99, WinY + 482, 32, 32);
					DrawImage(Image[(88 + cnt) - 89], WinX + 118, WinY + 482, 32, 32);
				}
			}

		}
		//MOSTRA A IMAGEM DO BALÃO DE INFORMAÇÃO DOS ITENS
		/*if (page == 0)
		{
			if (MouseAction(124 + (65 * 0), WinY + 175, 130, 130))
			{
				DrawImage(Image[83], pCursorPos.x - 105, pCursorPos.y - 115, 210, 88);

				wsprintf(strBuff, BPItemInfo[0][0]);
				DrawFontText(pCursorPos.x - strlen(strBuff) * 3, pCursorPos.y - 95, strBuff, RGB(255, 255, 255), false);
				wsprintf(strBuff, BPItemInfo[0][1]);
				DrawFontText(pCursorPos.x - strlen(strBuff) * 3, pCursorPos.y - 83, strBuff, RGB(255, 255, 255), false);
				wsprintf(strBuff, BPItemInfo[0][2]);
				DrawFontText(pCursorPos.x - strlen(strBuff) * 3, pCursorPos.y - 71, strBuff, RGB(255, 255, 255), false);
				wsprintf(strBuff, BPItemInfo[0][3]);
				DrawFontText(pCursorPos.x - strlen(strBuff) * 3, pCursorPos.y - 59, strBuff, RGB(255, 255, 255), false);
			}

			if (MouseAction(60 + (65 * 1), WinY + 313, 130, 130))
			{
				DrawImage(Image[83], pCursorPos.x - 105, pCursorPos.y - 115, 210, 88);

				wsprintf(strBuff, BPItemInfo[1][0]);
				DrawFontText(pCursorPos.x - strlen(strBuff) * 3, pCursorPos.y - 95, strBuff, RGB(255, 255, 255), false);
				wsprintf(strBuff, BPItemInfo[1][1]);
				DrawFontText(pCursorPos.x - strlen(strBuff) * 3, pCursorPos.y - 83, strBuff, RGB(255, 255, 255), false);
				wsprintf(strBuff, BPItemInfo[1][2]);
				DrawFontText(pCursorPos.x - strlen(strBuff) * 3, pCursorPos.y - 71, strBuff, RGB(255, 255, 255), false);
				wsprintf(strBuff, BPItemInfo[1][3]);
				DrawFontText(pCursorPos.x - strlen(strBuff) * 3, pCursorPos.y - 59, strBuff, RGB(255, 255, 255), false);
			}

			if (MouseAction(124 + (65 * 2), WinY + 175, 130, 130))
			{
				DrawImage(Image[83], pCursorPos.x - 105, pCursorPos.y - 115, 210, 88);

				wsprintf(strBuff, BPItemInfo[2][0]);
				DrawFontText(pCursorPos.x - strlen(strBuff) * 3, pCursorPos.y - 95, strBuff, RGB(255, 255, 255), false);
				wsprintf(strBuff, BPItemInfo[2][1]);
				DrawFontText(pCursorPos.x - strlen(strBuff) * 3, pCursorPos.y - 83, strBuff, RGB(255, 255, 255), false);
				wsprintf(strBuff, BPItemInfo[2][2]);
				DrawFontText(pCursorPos.x - strlen(strBuff) * 3, pCursorPos.y - 71, strBuff, RGB(255, 255, 255), false);
				wsprintf(strBuff, BPItemInfo[2][3]);
				DrawFontText(pCursorPos.x - strlen(strBuff) * 3, pCursorPos.y - 59, strBuff, RGB(255, 255, 255), false);
			}

			if (MouseAction(60 + (65 * 3), WinY + 313, 130, 130))
			{
				DrawImage(Image[83], pCursorPos.x - 105, pCursorPos.y - 115, 210, 88);

				wsprintf(strBuff, BPItemInfo[3][0]);
				DrawFontText(pCursorPos.x - strlen(strBuff) * 3, pCursorPos.y - 95, strBuff, RGB(255, 255, 255), false);
				wsprintf(strBuff, BPItemInfo[3][1]);
				DrawFontText(pCursorPos.x - strlen(strBuff) * 3, pCursorPos.y - 83, strBuff, RGB(255, 255, 255), false);
				wsprintf(strBuff, BPItemInfo[3][2]);
				DrawFontText(pCursorPos.x - strlen(strBuff) * 3, pCursorPos.y - 71, strBuff, RGB(255, 255, 255), false);
				wsprintf(strBuff, BPItemInfo[3][3]);
				DrawFontText(pCursorPos.x - strlen(strBuff) * 3, pCursorPos.y - 59, strBuff, RGB(255, 255, 255), false);
			}

			if (MouseAction(124 + (65 * 4), WinY + 175, 130, 130))
			{
				DrawImage(Image[83], pCursorPos.x - 105, pCursorPos.y - 115, 210, 88);

				wsprintf(strBuff, BPItemInfo[4][0]);
				DrawFontText(pCursorPos.x - strlen(strBuff) * 3, pCursorPos.y - 95, strBuff, RGB(255, 255, 255), false);
				wsprintf(strBuff, BPItemInfo[4][1]);
				DrawFontText(pCursorPos.x - strlen(strBuff) * 3, pCursorPos.y - 83, strBuff, RGB(255, 255, 255), false);
				wsprintf(strBuff, BPItemInfo[4][2]);
				DrawFontText(pCursorPos.x - strlen(strBuff) * 3, pCursorPos.y - 71, strBuff, RGB(255, 255, 255), false);
				wsprintf(strBuff, BPItemInfo[4][3]);
				DrawFontText(pCursorPos.x - strlen(strBuff) * 3, pCursorPos.y - 59, strBuff, RGB(255, 255, 255), false);
			}

			if (MouseAction(60 + (65 * 5), WinY + 313, 130, 130))
			{
				DrawImage(Image[83], pCursorPos.x - 105, pCursorPos.y - 115, 210, 88);

				wsprintf(strBuff, BPItemInfo[5][0]);
				DrawFontText(pCursorPos.x - strlen(strBuff) * 3, pCursorPos.y - 95, strBuff, RGB(255, 255, 255), false);
				wsprintf(strBuff, BPItemInfo[5][1]);
				DrawFontText(pCursorPos.x - strlen(strBuff) * 3, pCursorPos.y - 83, strBuff, RGB(255, 255, 255), false);
				wsprintf(strBuff, BPItemInfo[5][2]);
				DrawFontText(pCursorPos.x - strlen(strBuff) * 3, pCursorPos.y - 71, strBuff, RGB(255, 255, 255), false);
				wsprintf(strBuff, BPItemInfo[5][3]);
				DrawFontText(pCursorPos.x - strlen(strBuff) * 3, pCursorPos.y - 59, strBuff, RGB(255, 255, 255), false);
			}

			if (MouseAction(124 + (65 * 6), WinY + 175, 130, 130))
			{
				DrawImage(Image[83], pCursorPos.x - 105, pCursorPos.y - 115, 210, 88);

				wsprintf(strBuff, BPItemInfo[6][0]);
				DrawFontText(pCursorPos.x - strlen(strBuff) * 3, pCursorPos.y - 95, strBuff, RGB(255, 255, 255), false);
				wsprintf(strBuff, BPItemInfo[6][1]);
				DrawFontText(pCursorPos.x - strlen(strBuff) * 3, pCursorPos.y - 83, strBuff, RGB(255, 255, 255), false);
				wsprintf(strBuff, BPItemInfo[6][2]);
				DrawFontText(pCursorPos.x - strlen(strBuff) * 3, pCursorPos.y - 71, strBuff, RGB(255, 255, 255), false);
				wsprintf(strBuff, BPItemInfo[6][3]);
				DrawFontText(pCursorPos.x - strlen(strBuff) * 3, pCursorPos.y - 59, strBuff, RGB(255, 255, 255), false);
			}

			if (MouseAction(60 + (65 * 7), WinY + 313, 130, 130))
			{
				DrawImage(Image[83], pCursorPos.x - 105, pCursorPos.y - 115, 210, 88);

				wsprintf(strBuff, BPItemInfo[7][0]);
				DrawFontText(pCursorPos.x - strlen(strBuff) * 3, pCursorPos.y - 95, strBuff, RGB(255, 255, 255), false);
				wsprintf(strBuff, BPItemInfo[7][1]);
				DrawFontText(pCursorPos.x - strlen(strBuff) * 3, pCursorPos.y - 83, strBuff, RGB(255, 255, 255), false);
				wsprintf(strBuff, BPItemInfo[7][2]);
				DrawFontText(pCursorPos.x - strlen(strBuff) * 3, pCursorPos.y - 71, strBuff, RGB(255, 255, 255), false);
				wsprintf(strBuff, BPItemInfo[7][3]);
				DrawFontText(pCursorPos.x - strlen(strBuff) * 3, pCursorPos.y - 59, strBuff, RGB(255, 255, 255), false);
			}

			if (MouseAction(124 + (65 * 8), WinY + 175, 130, 130))
			{
				DrawImage(Image[83], pCursorPos.x - 105, pCursorPos.y - 115, 210, 88);

				wsprintf(strBuff, BPItemInfo[8][0]);
				DrawFontText(pCursorPos.x - strlen(strBuff) * 3, pCursorPos.y - 95, strBuff, RGB(255, 255, 255), false);
				wsprintf(strBuff, BPItemInfo[8][1]);
				DrawFontText(pCursorPos.x - strlen(strBuff) * 3, pCursorPos.y - 83, strBuff, RGB(255, 255, 255), false);
				wsprintf(strBuff, BPItemInfo[8][2]);
				DrawFontText(pCursorPos.x - strlen(strBuff) * 3, pCursorPos.y - 71, strBuff, RGB(255, 255, 255), false);
				wsprintf(strBuff, BPItemInfo[8][3]);
				DrawFontText(pCursorPos.x - strlen(strBuff) * 3, pCursorPos.y - 59, strBuff, RGB(255, 255, 255), false);
			}

			if (MouseAction(60 + (65 * 9), WinY + 313, 130, 130))
			{
				DrawImage(Image[83], pCursorPos.x - 105, pCursorPos.y - 115, 210, 88);

				wsprintf(strBuff, BPItemInfo[9][0]);
				DrawFontText(pCursorPos.x - strlen(strBuff) * 3, pCursorPos.y - 95, strBuff, RGB(255, 255, 255), false);
				wsprintf(strBuff, BPItemInfo[9][1]);
				DrawFontText(pCursorPos.x - strlen(strBuff) * 3, pCursorPos.y - 83, strBuff, RGB(255, 255, 255), false);
				wsprintf(strBuff, BPItemInfo[9][2]);
				DrawFontText(pCursorPos.x - strlen(strBuff) * 3, pCursorPos.y - 71, strBuff, RGB(255, 255, 255), false);
				wsprintf(strBuff, BPItemInfo[9][3]);
				DrawFontText(pCursorPos.x - strlen(strBuff) * 3, pCursorPos.y - 59, strBuff, RGB(255, 255, 255), false);
			}


		}*/


	}

	cnt2 = 0;
}

void BattlePass::Close()
{
	fOpen = false;
	page = 0;
	Tag = 0;
}

void BattlePass::Button()
{
	if (fOpen)
	{
		int WinX = 40, WinY = 70, cnt2 = 0;

		//AÇÃO DE CLIQUE NO LAYOUT DO ITEM
		for (int cnt = 0; cnt < 10; cnt++)
		{
			if (cnt % 2 == 0)
			{
				if (MouseAction(124 + (65 * cnt), WinY + 177, 150, 150))
					Itselect = cnt + 1;
			}
			else if (cnt % 2 > 0)
			{
				if (MouseAction(124 + (65 * cnt) - 66, WinY + 302, 150, 150))
					Itselect = cnt + 1;
			}
		}

		//AÇÃO DE CLIQUE DO BOTÃO PAGINA ESQUERDA
		if (MouseAction(545, 539, 52, 52))
			if (page > 0)
			{
				Itselect = 0;
				page--;
			}

		//AÇÃO DE CLIQUE DO BOTÃO PAGINA DIREITA
		if (MouseAction(605, 539, 52, 52))
			if (page < 3)
			{
				Itselect = 0;
				page++;
			}

		//AÇÃO DE CLIQUE DO BOTÃO RESGATAR
		if (MouseAction(670, 539, 101, 51) && Itselect > 0)
		{
			if (itemResg[(Itselect + (page * 10) - 1)] == 0) {
				SendItemToServer(BPList.item[(Itselect + (page * 10)) - 1]);
				Itselect = 0;
			}
		}
		//AÇÃO DE CLIQUE DO BOTÃO FECHAR
		if (MouseAction(800, 80, 22, 22))
		{
			Itselect = 0;
			Close();
		}
	}

}




