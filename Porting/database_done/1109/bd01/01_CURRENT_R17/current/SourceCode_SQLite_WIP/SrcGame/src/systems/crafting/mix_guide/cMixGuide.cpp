#include "compat/legacy/sin/sinLinkHeader.h"
#include "systems/rendering/font/FontImage.h"

bool TextureBox1 = false;
int bTexture;
int WinX = 40, WinY = 70;
char szBuff[256] = { 0 };

void cMixGuide::DrawImage(int Mat, int x, int y, int w, int h)
{
	HoDrawTexImage(Mat, x, y, w, h, w, h, 255);
}


bool cMixGuide::MouseAction(int x, int y, int w, int h)
{
	if (pRealCursorPos.x > x && pRealCursorPos.x < x + w && pRealCursorPos.y > y && pRealCursorPos.y < y + h)
		return TRUE;

	return FALSE;
}

void cMixGuide::Init()
{
	cImages[0] = CreateTextureMaterial("game//mix//window.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	cImages[1] = CreateTextureMaterial("game//mix//btnok.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);

	cImages[2] = CreateTextureMaterial("game//mix//combobox//drop.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	cImages[3] = CreateTextureMaterial("game//mix//combobox//dropon.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	cImages[4] = CreateTextureMaterial("game//mix//combobox//drop_.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	cImages[5] = CreateTextureMaterial("game//mix//combobox//dropdown.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	cImages[6] = CreateTextureMaterial("game//mix//combobox//dropbg.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);

	//sheltons
	
	cImages[12] = CreateTextureMaterial("game//mix//Sheltoms//Transparo.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	cImages[7] = CreateTextureMaterial("game//mix//Sheltoms//Devine.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	cImages[8] = CreateTextureMaterial("game//mix//Sheltoms//Celesto.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	cImages[13] = CreateTextureMaterial("game//mix//Sheltoms//Mirage.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	cImages[14] = CreateTextureMaterial("game//mix//Sheltoms//Inferna.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	cImages[15] = CreateTextureMaterial("game//mix//Sheltoms//Enigma.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	cImages[10] = CreateTextureMaterial("game//mix//Sheltoms//Murki.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
	cImages[11] = CreateTextureMaterial("game//mix//Sheltoms//Bellum.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);

	cImages[9] = CreateTextureMaterial("game//mix//Mix.png", 0, 0, 0, 0, SMMAT_BLEND_ALPHA);
    
}

void cMixGuide::Draw()
{
	if (cOpen)
	{
		DrawImage(cImages[0], WinX, WinY, 762, 553);
		DrawImage(cImages[1], WinX + 337, WinY + 511, 88, 35);
		DrawImage(cImages[9], WinX + 316, WinY + 10, 125, 29);

		
		SetFontTextColor(RGB(220, 220, 220));
		DescriptionsMix();

		
		if (bTexture == 0)
			Weapons();
		else if (bTexture == 1)
			ArmorsRobe();
		else if (bTexture == 2)
			Bracelet();
		else if (bTexture == 3)
			Gauntlet();
		else if (bTexture == 4)
			Boot();
		else if (bTexture == 5)
		    ShieldOrb();
		else if (bTexture == 6)
			Ring();
		else if (bTexture == 7)
			Amulet();
		
		DrawComboBox();

	}
}


void cMixGuide::Button()
{
	if(cOpen)
	{ 
		if (MouseAction(WinX + 337, WinY + 511, 88, 35))
		{
			Close();
		}

	

		if (TextureBox1)
		{
			TextureBox1 = false;
			if (MouseAction(WinX + 630, WinY + 66, 86, 24))
				bTexture = 0;
			if (MouseAction(WinX + 630, WinY + 88, 86, 24))
				bTexture = 1;
			if (MouseAction(WinX + 630, WinY + 112, 86, 24))
				bTexture = 2;
			if (MouseAction(WinX + 630, WinY + 136, 86, 25))
				bTexture = 3;
			if (MouseAction(WinX + 630, WinY + 160, 86, 25))
				bTexture = 4;
			if (MouseAction(WinX + 630, WinY + 184, 86, 25))
				bTexture = 5;
			if (MouseAction(WinX + 630, WinY + 208, 86, 25))
				bTexture = 6;
			if (MouseAction(WinX + 630, WinY + 232, 86, 25))
				bTexture = 7;

		}
		else
			if (MouseAction(WinX + 650, WinY + 40, 86, 26))
				TextureBox1 = true;
	}

}


void cMixGuide::DescriptionsMix()
{   
	
	//FontType = 2;

	wsprintf(szBuff, "Nome Mix");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 23, WinY + 45, szBuff, strlen(szBuff));


	wsprintf(szBuff, "Sheltoms");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 200, WinY + 45, szBuff, strlen(szBuff));

	wsprintf(szBuff, "Descri\347\343o");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 400, WinY + 45, szBuff, strlen(szBuff));
	//FontType = 0;
}

void cMixGuide::DrawComboBox()
{
	SetFontTextColor(RGB(235, 235, 235));
	if (TextureBox1)
	{
		
		DrawImage(cImages[4], (WinX + 630), (WinY + 40), 110, 26); // 287 = 40
		DrawImage(cImages[5], (WinX + 630), (WinY + 66), 110, 26); // 313 == 66
		DrawImage(cImages[5], (WinX + 630), (WinY + 88), 110, 26); // 337 == 88
		DrawImage(cImages[5], (WinX + 630), (WinY + 112), 110, 26); // 361 == 112
		DrawImage(cImages[5], (WinX + 630), (WinY + 136), 110, 26); // 385 == 136
		DrawImage(cImages[5], (WinX + 630), (WinY + 160), 110, 26); 
		DrawImage(cImages[5], (WinX + 630), (WinY + 184), 110, 26); 
		DrawImage(cImages[5], (WinX + 630), (WinY + 208), 110, 26); 
		DrawImage(cImages[5], (WinX + 630), (WinY + 232), 110, 26); 

		if (MouseAction(WinX + 630, WinY + 64, 110, 26))
			DrawImage(cImages[6], (WinX + 630), (WinY + 64), 110, 26);

		if (MouseAction(WinX + 630, WinY + 88, 110, 26))
			DrawImage(cImages[6], (WinX + 630), (WinY + 88), 110, 26);

		if (MouseAction(WinX + 630, WinY + 112, 110, 26))
			DrawImage(cImages[6], (WinX + 630), (WinY + 112), 110, 26);

		if (MouseAction(WinX + 630, WinY + 136, 110, 26))
			DrawImage(cImages[6], (WinX + 630), (WinY + 136), 10, 26);

		if (MouseAction(WinX + 630, WinY + 160, 110, 26))
			DrawImage(cImages[6], (WinX + 630), (WinY + 160), 10, 26);

		if (MouseAction(WinX + 630, WinY + 184, 110, 26))
			DrawImage(cImages[6], (WinX + 630), (WinY + 184), 10, 26);

		if (MouseAction(WinX + 630, WinY + 208, 110, 26))
			DrawImage(cImages[6], (WinX + 630), (WinY + 208), 10, 26);

		if (MouseAction(WinX + 630, WinY + 232, 110, 26))
			DrawImage(cImages[6], (WinX + 630), (WinY + 232), 10, 26);

		dsTextLineOut(WinX + 658, WinY + 71,  "Armas", 8);//178 -> +28  // WINY = WINY +5
		dsTextLineOut(WinX + 658, WinY + 93,  "Armadura/Robes", 13);//170 -> +20 // 
		dsTextLineOut(WinX + 658, WinY + 117, "Braceletes", 9);//178 -> 28  //
		dsTextLineOut(WinX + 658, WinY + 141, "Luvas", 9);//170 -> + 20 //
		dsTextLineOut(WinX + 658, WinY + 165, "Botas", 5);//170 -> + 20 //
		dsTextLineOut(WinX + 658, WinY + 189, "Escudos/Orb", 11);//170 -> + 20 //
		dsTextLineOut(WinX + 658, WinY + 213, "Aneis", 5);//170 -> + 20 //
		dsTextLineOut(WinX + 658, WinY + 237, "Amuleto", 7);//170 -> + 20 //
	}
	else
	{

		if (MouseAction(WinX + 630, WinY + 40, 110, 26))
			DrawImage(cImages[3], (WinX + 630), (WinY + 40), 110, 26);
		else
			DrawImage(cImages[2], (WinX + 630), (WinY + 40), 110, 26);

	}


	if (bTexture == 0)
		dsTextLineOut(WinX + 658, WinY + 45, "Armas", 8); // 170 e 292 . 292 = 287 + 5 , 40 +5 = 45
	else if (bTexture == 1)
		dsTextLineOut(WinX + 638, WinY + 45, "Armaduras/Robes", 13); // 162 e 292
	else if (bTexture == 2)
		dsTextLineOut(WinX + 658, WinY + 45, "Braceletes", 9); // 170 e 292
	else if (bTexture == 3)
		dsTextLineOut(WinX + 658, WinY + 45, "Luvas", 9); // 162 e 292
	else if (bTexture == 4)
		dsTextLineOut(WinX + 658, WinY + 45, "Botas", 5); // 162 e 292
	else if (bTexture == 5)
		dsTextLineOut(WinX + 653, WinY + 45, "Escudo/Orb", 11); // 162 e 292
	else if (bTexture == 6)
		dsTextLineOut(WinX + 658, WinY + 45, "Aneis", 5); // 162 e 292
	else if (bTexture == 7)
		dsTextLineOut(WinX + 658, WinY + 45, "Amuleto", 7); // 162 e 292
}


void cMixGuide::Weapons()
{
	wsprintf(szBuff, "Mix Devine");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 23, WinY + 85, szBuff, strlen(szBuff));

	wsprintf(szBuff, "6 X         3 X");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 200, WinY + 85, szBuff, strlen(szBuff));
	DrawImage(cImages[7], (WinX + 223), (WinY + 85), 16, 16);
	DrawImage(cImages[10], (WinX + 267), (WinY + 85), 16, 16);

	wsprintf(szBuff, "ATK(+6)  TAXA(+40)  CRIT(+2%)  MANA(+25)");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 400, WinY + 85, szBuff, strlen(szBuff));
	//Mix Celestro
	wsprintf(szBuff, "Mix Celesto");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 23, WinY + 120, szBuff, strlen(szBuff));

	wsprintf(szBuff, "6 X         3 X");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 200, WinY + 120, szBuff, strlen(szBuff));
	DrawImage(cImages[8], (WinX + 223), (WinY + 120), 16, 16);
	DrawImage(cImages[7], (WinX + 267), (WinY + 120), 16, 16);

	wsprintf(szBuff, "ATK(+12)  TAXA(+80)  CRIT(+6%)  MANA(+50)");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 400, WinY + 120, szBuff, strlen(szBuff));

	//Mix Bellum
	wsprintf(szBuff, "Mix Bellum");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 23, WinY + 155, szBuff, strlen(szBuff));

	wsprintf(szBuff, "3 X         3 X       X3");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 200, WinY + 155, szBuff, strlen(szBuff));
	DrawImage(cImages[11], (WinX + 223), (WinY + 155), 16, 16);
	DrawImage(cImages[8], (WinX + 267), (WinY + 155), 16, 16);
	DrawImage(cImages[7], (WinX + 297), (WinY + 155), 16, 16);

	wsprintf(szBuff, "ATK(+16)  TAXA(+160)  CRIT(+10%)  HP+MANA(+100)  Vel.(+1)");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 400, WinY + 155, szBuff, strlen(szBuff));
}

void cMixGuide::ArmorsRobe()
{
	(szBuff, "Mix Devine");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 23, WinY + 85, szBuff, strlen(szBuff));

	wsprintf(szBuff, "6 X         3 X");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 200, WinY + 85, szBuff, strlen(szBuff));
	DrawImage(cImages[7], (WinX + 223), (WinY + 85), 16, 16);
	DrawImage(cImages[10], (WinX + 267), (WinY + 85), 16, 16);

	wsprintf(szBuff, "DEF(+150)  ABS(+5.0) BLOCK(+2%) HP(+50)");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 400, WinY + 85, szBuff, strlen(szBuff));
	//Mix Celestro
	wsprintf(szBuff, "Mix Celesto");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 23, WinY + 120, szBuff, strlen(szBuff));

	wsprintf(szBuff, "6 X         3 X");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 200, WinY + 120, szBuff, strlen(szBuff));
	DrawImage(cImages[8], (WinX + 223), (WinY + 120), 16, 16);
	DrawImage(cImages[7], (WinX + 267), (WinY + 120), 16, 16);

	wsprintf(szBuff, "DEF(+250)  ABS(+7.0) BLOCK(+4%) HP(+100) ");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 400, WinY + 120, szBuff, strlen(szBuff));

	
	//Mix Bellum
	wsprintf(szBuff, "Mix Bellum");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 23, WinY + 155, szBuff, strlen(szBuff));

	wsprintf(szBuff, "3 X         3 X       X3");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 200, WinY + 155, szBuff, strlen(szBuff));
	DrawImage(cImages[11], (WinX + 223), (WinY + 155), 16, 16);
	DrawImage(cImages[8], (WinX + 267), (WinY + 155), 16, 16);
	DrawImage(cImages[7], (WinX + 297), (WinY + 155), 16, 16);

	wsprintf(szBuff, "DEF(+350)  ABS(+10.0) BLOCK(+7%) HP(+150)");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 400, WinY + 155, szBuff, strlen(szBuff));
	
}

void cMixGuide::Bracelet()
{

	wsprintf(szBuff, "Mix Devine");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 23, WinY + 85, szBuff, strlen(szBuff));

	wsprintf(szBuff, "6 X         3 X");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 200, WinY + 85, szBuff, strlen(szBuff));
	DrawImage(cImages[7], (WinX + 223), (WinY + 85), 16, 16);
	DrawImage(cImages[10], (WinX + 267), (WinY + 85), 16, 16);

	wsprintf(szBuff, "DEF(+80)  TAXA(+80)  HP(+20) ");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 400, WinY + 85, szBuff, strlen(szBuff));
	//Mix Celestro
	wsprintf(szBuff, "Mix Celesto");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 23, WinY + 120, szBuff, strlen(szBuff));

	wsprintf(szBuff, "6 X         3 X");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 200, WinY + 120, szBuff, strlen(szBuff));
	DrawImage(cImages[8], (WinX + 223), (WinY + 120), 16, 16);
	DrawImage(cImages[7], (WinX + 267), (WinY + 120), 16, 16);

	wsprintf(szBuff, "DEF(+140)  TAXA(+140)  HP(+50)");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 400, WinY + 120, szBuff, strlen(szBuff));

	////Mix Bellum
	wsprintf(szBuff, "Mix Bellum");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 23, WinY + 155, szBuff, strlen(szBuff));

	wsprintf(szBuff, "8 X         4 X");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 200, WinY + 155, szBuff, strlen(szBuff));
	DrawImage(cImages[11], (WinX + 223), (WinY + 155), 16, 16);
	DrawImage(cImages[15], (WinX + 267), (WinY + 155), 16, 16);

	wsprintf(szBuff, "DEF(+200)  TAXA(+200)  HP(+100) ");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 400, WinY + 155, szBuff, strlen(szBuff));

}

void cMixGuide::Gauntlet()
{
	wsprintf(szBuff, "Mix Devine");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 23, WinY + 85, szBuff, strlen(szBuff));

	wsprintf(szBuff, "6 X         3 X");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 200, WinY + 85, szBuff, strlen(szBuff));
	DrawImage(cImages[7], (WinX + 223), (WinY + 85), 16, 16);
	DrawImage(cImages[10], (WinX + 267), (WinY + 85), 16, 16);

	wsprintf(szBuff, "DEF(+80)  ABS(+1.5) HP(+20)");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 400, WinY + 85, szBuff, strlen(szBuff));
	//Mix Celestro
	wsprintf(szBuff, "Mix Celesto");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 23, WinY + 120, szBuff, strlen(szBuff));

	wsprintf(szBuff, "6 X         3 X");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 200, WinY + 120, szBuff, strlen(szBuff));
	DrawImage(cImages[8], (WinX + 223), (WinY + 120), 16, 16);
	DrawImage(cImages[7], (WinX + 267), (WinY + 120), 16, 16);

	wsprintf(szBuff, "DEF(+140)  ABS(+2.5) HP(+100)");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 400, WinY + 120, szBuff, strlen(szBuff));

	////Mix Bellum
	wsprintf(szBuff, "Mix Bellum");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 23, WinY + 155, szBuff, strlen(szBuff));

	wsprintf(szBuff, "8 X         4 X");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 200, WinY + 155, szBuff, strlen(szBuff));
	DrawImage(cImages[11], (WinX + 223), (WinY + 155), 16, 16);
	DrawImage(cImages[15], (WinX + 267), (WinY + 155), 16, 16);

	wsprintf(szBuff, "DEF(+200)  ABS(+4.0) HP(+100)  ");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 400, WinY + 155, szBuff, strlen(szBuff));
}


void cMixGuide::Boot()
{
	wsprintf(szBuff, "Mix Devine");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 23, WinY + 85, szBuff, strlen(szBuff));

	wsprintf(szBuff, "6 X         3 X");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 200, WinY + 85, szBuff, strlen(szBuff));
	DrawImage(cImages[7], (WinX + 223), (WinY + 85), 16, 16);
	DrawImage(cImages[10], (WinX + 267), (WinY + 85), 16, 16);

	wsprintf(szBuff, "DEF(+80)  ABS(+1.5)  Velocidade(+2.0)  HP(+20)");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 400, WinY + 85, szBuff, strlen(szBuff));
	//Mix Celestro
	wsprintf(szBuff, "Mix Celesto");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 23, WinY + 120, szBuff, strlen(szBuff));

	wsprintf(szBuff, "6 X         3 X");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 200, WinY + 120, szBuff, strlen(szBuff));
	DrawImage(cImages[8], (WinX + 223), (WinY + 120), 16, 16);
	DrawImage(cImages[7], (WinX + 267), (WinY + 120), 16, 16);

	wsprintf(szBuff, "DEF(+140)  ABS(+4.0)  Velocidade(+3.0)  HP(+50)");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 400, WinY + 120, szBuff, strlen(szBuff));

	//Mix Bellum
	wsprintf(szBuff, "Mix Bellum");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 23, WinY + 155, szBuff, strlen(szBuff));

	wsprintf(szBuff, "8 X         4 X");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 200, WinY + 155, szBuff, strlen(szBuff));
	DrawImage(cImages[11], (WinX + 223), (WinY + 155), 16, 16);
	DrawImage(cImages[15], (WinX + 267), (WinY + 155), 16, 16);

	wsprintf(szBuff, "DEF(+200)  ABS(+7.0)  Velocidade(+5.0)  HP(+100)");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 400, WinY + 155, szBuff, strlen(szBuff));
}


void cMixGuide::ShieldOrb()
{
	wsprintf(szBuff, "Mix Devine");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 23, WinY + 85, szBuff, strlen(szBuff));

	wsprintf(szBuff, "6 X         3 X");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 200, WinY + 85, szBuff, strlen(szBuff));
	DrawImage(cImages[7], (WinX + 223), (WinY + 85), 16, 16);
	DrawImage(cImages[10], (WinX + 267), (WinY + 85), 16, 16);

	wsprintf(szBuff, "DEF(+100)  ABS(+1.0)  Block(+2.0)  HP(+20)");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 400, WinY + 85, szBuff, strlen(szBuff));
	//Mix Celestro
	wsprintf(szBuff, "Mix Celesto");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 23, WinY + 120, szBuff, strlen(szBuff));

	wsprintf(szBuff, "6 X         3 X");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 200, WinY + 120, szBuff, strlen(szBuff));
	DrawImage(cImages[8], (WinX + 223), (WinY + 120), 16, 16);
	DrawImage(cImages[7], (WinX + 267), (WinY + 120), 16, 16);

	wsprintf(szBuff, "DEF(+150)  ABS(+3.0)  Block(+5.0)  HP(+50)");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 400, WinY + 120, szBuff, strlen(szBuff));
	
	//Mix Bellum
	wsprintf(szBuff, "Mix Bellum");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 23, WinY + 155, szBuff, strlen(szBuff));

	wsprintf(szBuff, "8 X         4 X");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 200, WinY + 155, szBuff, strlen(szBuff));
	DrawImage(cImages[11], (WinX + 223), (WinY + 155), 16, 16);
	DrawImage(cImages[15], (WinX + 267), (WinY + 155), 16, 16);

	wsprintf(szBuff, "DEF(+250)  ABS(+5.0)  Block(+7.0)  HP(+150)");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 400, WinY + 155, szBuff, strlen(szBuff));
}


void cMixGuide::Ring()
{
	wsprintf(szBuff, "Mix Devine");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 23, WinY + 85, szBuff, strlen(szBuff));

	wsprintf(szBuff, "6 X         3 X");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 200, WinY + 85, szBuff, strlen(szBuff));
	DrawImage(cImages[7], (WinX + 223), (WinY + 85), 16, 16);
	DrawImage(cImages[10], (WinX + 267), (WinY + 85), 16, 16);

	wsprintf(szBuff, "HP(+50) MP(+50) rHP(+3.0) DEFESA(+50) ABS(+5) TX ATA(+100)");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 400, WinY + 85, szBuff, strlen(szBuff));
	//Mix Celestro
	wsprintf(szBuff, "Mix Celesto");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 23, WinY + 120, szBuff, strlen(szBuff));

	wsprintf(szBuff, "6 X         3 X");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 200, WinY + 120, szBuff, strlen(szBuff));
	DrawImage(cImages[8], (WinX + 223), (WinY + 120), 16, 16);
	DrawImage(cImages[7], (WinX + 267), (WinY + 120), 16, 16);

	wsprintf(szBuff, "HP(+100) MP(+100) rHP(+3.0) DEFESA(+150) ABS(+6) TX ATA(+150)");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 400, WinY + 120, szBuff, strlen(szBuff));

	//Mix Bellum
	wsprintf(szBuff, "Mix Bellum");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 23, WinY + 155, szBuff, strlen(szBuff));

	wsprintf(szBuff, "8 X         4 X");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 200, WinY + 155, szBuff, strlen(szBuff));
	DrawImage(cImages[11], (WinX + 223), (WinY + 155), 16, 16);
	DrawImage(cImages[15], (WinX + 267), (WinY + 155), 16, 16);

	wsprintf(szBuff, "HP(+150) MP(+150) rHP(+5.0) DEFESA(+200) ABS(+8) TX ATA(+200)");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 400, WinY + 155, szBuff, strlen(szBuff));
	


}


void cMixGuide::Amulet()
{
	wsprintf(szBuff, "Mix Devine ");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 23, WinY + 85, szBuff, strlen(szBuff));

	wsprintf(szBuff, "6 X         3 X");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 200, WinY + 85, szBuff, strlen(szBuff));
	DrawImage(cImages[7], (WinX + 223), (WinY + 85), 16, 16);
	DrawImage(cImages[10], (WinX + 267), (WinY + 85), 16, 16);

	wsprintf(szBuff, "HP(+50) MP(+50) rHP(+2.0) DEFESA(+50) ABS(+4) TX ATA(+100)");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 400, WinY + 85, szBuff, strlen(szBuff));
	//Mix Celestro
	wsprintf(szBuff, "Mix Celesto");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 23, WinY + 120, szBuff, strlen(szBuff));

	wsprintf(szBuff, "6 X         3 X");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 200, WinY + 120, szBuff, strlen(szBuff));
	DrawImage(cImages[8], (WinX + 223), (WinY + 120), 16, 16);
	DrawImage(cImages[7], (WinX + 267), (WinY + 120), 16, 16);

	wsprintf(szBuff, "HP(+100) MP(+100) rHP(+3.0) DEFESA(+150) ABS(+4) TX ATA(+150)");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 400, WinY + 120, szBuff, strlen(szBuff));

	//Mix Bellum
	wsprintf(szBuff, "Mix Bellum");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 23, WinY + 155, szBuff, strlen(szBuff));

	wsprintf(szBuff, "8 X         4 X");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 200, WinY + 155, szBuff, strlen(szBuff));
	DrawImage(cImages[11], (WinX + 223), (WinY + 155), 16, 16);
	DrawImage(cImages[15], (WinX + 267), (WinY + 155), 16, 16);

	wsprintf(szBuff, "HP(+150) MP(+150) rHP(+5.0) DEFESA(+200) ABS(+7) TX ATA(+200)");
	GetFontTextExtentPoint(szBuff, strlen(szBuff), &size);
	dsTextLineOut(WinX + 400, WinY + 155, szBuff, strlen(szBuff));
	
}



