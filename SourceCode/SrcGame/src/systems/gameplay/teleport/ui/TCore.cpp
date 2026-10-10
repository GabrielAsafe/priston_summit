#include "compat/legacy/sin/sinLinkHeader.h"
#include "systems/gameplay/teleport/ui/TCore.h"
#include "systems/presentation/v2/legacy/LegacyTextureAdapter.h"
#include "systems/world/field/field.h"
char Text[38][32];
const char* InfoMapa[38] = {
	"Drops e Exp",
	"Drops e Exp",
	"Drops e Exp",
	"Drops e Exp",
	"Drops e Exp",
	"Drops e Exp",
	"Evento Mini Boss",
	"Drops e Exp",
	"Drops e Exp",
	"PVP Drops Exp Gold Mana",
	"Drops e Exp",
	"Drops e Exp",
	"Drops e Exp",
	"Drops e Exp",
	"Drops e Exp",
	"Drops e Exp",
	"Drops e Exp",
	"Drops e Exp",
	"Drops e Exp",
	"Drops e Exp",
	"Drops e Exp",
	"Drops e Exp",
	"Drops e Exp",
	"Drops e Exp",
	"Drops e Exp",
	"Drops e Exp",
	"Drops e Exp",
	"Drops e Exp",
	"Drops e Exp",
	"Drops e Exp",
	"Drops e Exp",
	"Drops e Exp",
	"Drops e Exp",
	"Drops e Exp",
	"EM BREVE",
	"EM BREVE",
	"EM BREVE",
	"EM BREVE",
};
const char* FieldNum[38][4] = {
	//Nº   Field	Level   Nome
	{ "0","20",		"0",	"Estrada dos Ventos"},//ok
	{ "1","19",		"0",	"Vale Tranquilo"},//ok
	{ "2","17",		"0",	"Floresta dos Esp\355ritos"},//ok
	{ "3","0",		"0",	"Mata das Ac\341cias"},//ok	
	{ "4","2",		"0",	"Jardim da Liberdade"},//ok
	{ "5","4",		"20",	"Refugio dos Ancioes"},//ok
	{ "6","5",		"70",	"Castelo dos Perdidos"},//ok
	{ "7","7",		"30",	"Terra Maldita"},//ok
	{ "8","8",		"30",	"Terra Esquecida"},//ok
	{ "9","10",		"40",	"Oasis"},//ok
	{ "10","11",	"55",	"Campo de Batalha"},//ok
	{ "11","12",	"55",	"Terra Proibida"},//ok
	{ "12","25",	"55",	"Caverna das Abelhas"},//ok
	{ "13","24",	"55",	"Caverna dos Cogumelos"},//ok
	{ "14","26",	"65",	"Santu\341rio Sombrio"},//ok
	{ "15","13",	"40",	"Calabou\347o Antigo 1\272 Andar"},//ok
	{ "16","14",	"50",	"Calabou\347o Antigo 2\272 Andar"},//ok
	{ "17","15",	"60",	"Calabou\347o Antigo 3\272 Andar"},//ok
	{ "18","22",	"70",	"Templo Maldito 1\272 Andar"},//ok
	{ "19","23",	"75",	"Templo Maldito 2\272 Andar"},//ok
	{ "20","42",	"80",	"Templo Maldito 3\272 Andar"},//ok
	{ "21","34",	"70",	"Lago da Gan\342ncia"},//ok
	{ "22","27",	"80",	"Estrada de Ferro do Caos"},//ok
	{ "23","28",	"85",	"Cora\347\343o de Perum"},//ok
	{ "24","29",	"90",	"Eura"},//ok
	{ "25","31",	"90",	"Vale Gallubia"},//ok
	{ "26","35",	"90",	"Santu\341rio Congelado"},//ok
	{ "27","36",	"90",	"Covil de Kelvezu"},//ok
	{ "28","37",	"95",	"Ilha Perdida"},//ok
	{ "29","38",	"100",	"Templo Perdido"},
	{ "30","40",	"102",	"Torre Sem Fim 1o Andar"},
	{ "31","41",	"105",	"Torre Sem Fim 2\272 Andar"},
	{ "32","43",	"108",	"Torre Sem Fim 3\272 Andar"},
	{ "33","44",	"110",	"Mina de Gelo 1\272 Andar"},
	{ "34","46",	"132",	"Laboratorio Secreto"},
	{ "35","47",	"132",	"Arma Secreta"},
	{ "36","48",	"132",	"Abismo do Mar 1\272 Andar"},
	{ "37","49",	"132",	"Abismo do Mar 2\272 Andar"}
};

int mapa, index = 1, click, lvl, imagemapa;
BOOL stop;
int basex = 450;
int basey = 90;
SIZE size4[38] = {};
extern bool isMousePos(int x, int y, int w, int h);

static int LoadTeleportTexture(const char* key)
{
	return presentation::legacy::ClientLegacyTextureAdapter().Load(
		presentation::TextureAssetId(key)).value;
}

void TCORE::init() {

	imagens[JANELA] = LoadTeleportTexture("ui.teleport.window");
	imagens[BTNTELEPORT] = LoadTeleportTexture("ui.teleport.action.teleport");
	imagens[BTNTELEPORT_] = LoadTeleportTexture("ui.teleport.action.teleport.alt");
	imagens[BTNCLOSE] = LoadTeleportTexture("ui.teleport.action.close");
	imagens[BTNCLOSE_] = LoadTeleportTexture("ui.teleport.action.close.alt");
	imagens[EXP] = LoadTeleportTexture("ui.teleport.category.exp");
	imagens[HUNT] = LoadTeleportTexture("ui.teleport.category.hunt");
	imagens[GOLD] = LoadTeleportTexture("ui.teleport.category.gold");
	imagens[TOWN] = LoadTeleportTexture("ui.teleport.category.town");
	imagens[SHELTONS] = LoadTeleportTexture("ui.teleport.category.sheltons");
	imagens[PVP] = LoadTeleportTexture("ui.teleport.category.pvp");
	imagens[HUNTPVP] = LoadTeleportTexture("ui.teleport.category.huntpvp");
	imagens[12] = LoadTeleportTexture("ui.teleport.map.border");
	imagens[13] = LoadTeleportTexture("ui.teleport.page.1.base");
	imagens[14] = LoadTeleportTexture("ui.teleport.page.1.alt");
	imagens[15] = LoadTeleportTexture("ui.teleport.page.2.base");
	imagens[16] = LoadTeleportTexture("ui.teleport.page.2.alt");
	imagens[17] = LoadTeleportTexture("ui.teleport.page.3.base");
	imagens[18] = LoadTeleportTexture("ui.teleport.page.3.alt");
	imagens[19] = LoadTeleportTexture("ui.teleport.expdrop");
	imgs[0] = LoadTeleportTexture("ui.teleport.map.estrada-dos-ventos");
	imgs[1] = LoadTeleportTexture("ui.teleport.map.vale-tranquilo");
	imgs[2] = LoadTeleportTexture("ui.teleport.map.floresta-espirito");
	imgs[3] = LoadTeleportTexture("ui.teleport.map.mata-acacias");
	imgs[4] = LoadTeleportTexture("ui.teleport.map.jardim-liberdade");
	imgs[5] = LoadTeleportTexture("ui.teleport.map.refugio-ancioes");
	imgs[6] = LoadTeleportTexture("ui.teleport.map.castelo-perdidos");
	imgs[7] = LoadTeleportTexture("ui.teleport.map.terra-maldita");
	imgs[8] = LoadTeleportTexture("ui.teleport.map.terra-esquecida");
	imgs[9] = LoadTeleportTexture("ui.teleport.map.oasis");
	imgs[10] = LoadTeleportTexture("ui.teleport.map.cba");
	imgs[11] = LoadTeleportTexture("ui.teleport.map.terra-proibida");
	imgs[12] = LoadTeleportTexture("ui.teleport.map.bcave");
	imgs[13] = LoadTeleportTexture("ui.teleport.map.mcave");
	imgs[14] = LoadTeleportTexture("ui.teleport.map.ds");
	imgs[15] = LoadTeleportTexture("ui.teleport.map.dun1");
	imgs[16] = LoadTeleportTexture("ui.teleport.map.dun2");
	imgs[17] = LoadTeleportTexture("ui.teleport.map.dun3");
	imgs[18] = LoadTeleportTexture("ui.teleport.map.s1");
	imgs[19] = LoadTeleportTexture("ui.teleport.map.s2");
	imgs[20] = LoadTeleportTexture("ui.teleport.map.s3");
	imgs[21] = LoadTeleportTexture("ui.teleport.map.gl");
	imgs[22] = LoadTeleportTexture("ui.teleport.map.iron1");
	imgs[23] = LoadTeleportTexture("ui.teleport.map.iron2");
	imgs[24] = LoadTeleportTexture("ui.teleport.map.eura");
	imgs[25] = LoadTeleportTexture("ui.teleport.map.ice1");
	imgs[26] = LoadTeleportTexture("ui.teleport.map.ice2");
	imgs[27] = LoadTeleportTexture("ui.teleport.map.kcave");
	imgs[28] = LoadTeleportTexture("ui.teleport.map.lost1");
	imgs[29] = LoadTeleportTexture("ui.teleport.map.lost2");
	imgs[30] = LoadTeleportTexture("ui.teleport.map.et1");
	imgs[31] = LoadTeleportTexture("ui.teleport.map.et2");
	imgs[32] = LoadTeleportTexture("ui.teleport.map.et3");
	imgs[33] = LoadTeleportTexture("ui.teleport.map.mine-ice");
	imgs[34] = LoadTeleportTexture("ui.teleport.map.slab");
	imgs[35] = LoadTeleportTexture("ui.teleport.map.ancientw");
	imgs[36] = LoadTeleportTexture("ui.teleport.map.seaa");
	imgs[37] = LoadTeleportTexture("ui.teleport.map.stemple");


	for (int i = 0; i < 38; i++) {
		wsprintf(Text[i], "%d. %s", atoi(FieldNum[i][0]), FieldNum[i][3]);
		GetFontTextExtentPoint(Text[i], lstrlen(Text[i]), &size4[i]);
	}
	//AddChatBuff("szLevelEntrar");

};


void TCORE::DrawImage(int Mat, int x, int y, int w, int h)
{
	HoDrawTexImage(Mat, x, y, w, h, w, h, 255);
}


void TCORE::Draw() {
	if (Open) {

		DWORD CORSELECT = RGB(255, 191, 89);
		DWORD COR = RGB(255, 255, 255);
		DWORD COR1 = RGB(255, 100, 0);
		int cd = (smScreenWidth / 2) - 180;
		stop = TRUE;
		DrawImage(imagens[JANELA], smScreenWidth / 2 - 240, 80, 480, 500);
		DrawImage(imgs[click], smScreenWidth / 2 + 60, 136, 119, 119);

		DrawImage(imagens[12], smScreenWidth / 2 + 50, 130, 145, 130);
		if (index == 1) {
			DrawImage(imagens[13], cd + 44, basey + 400, 26, 26);
		}
		else {
			DrawImage(imagens[14], cd + 44, basey + 400, 26, 26);
		}
		if (index == 2) {
			DrawImage(imagens[15], cd + 74, basey + 400, 26, 26);
		}
		else {
			DrawImage(imagens[16], cd + 74, basey + 400, 26, 26);
		}
		if (index == 3) {
			DrawImage(imagens[17], cd + 104, basey + 400, 26, 26);
		}
		else {
			DrawImage(imagens[18], cd + 104, basey + 400, 26, 26);
		}
		SIZE size4 = {};
		GetFontTextExtentPoint(FieldNum[click][3], lstrlen(FieldNum[click][3]), &size4);
		DrawFontText((smScreenWidth / 2 + 115) - (size4.cx / 2), basey + 170, FieldNum[click][3], CORSELECT, 3);
		size4 = {};
		GetFontTextExtentPoint(InfoMapa[click], lstrlen(InfoMapa[click]), &size4);
		DrawFontText((smScreenWidth / 2 + 115) - (size4.cx / 2), basey + 200, InfoMapa[click], COR, 3);


		char LvlReq[32];
		wsprintf(LvlReq, "Level Requerido %d.", atoi(FieldNum[click][2]));
		size4 = {};
		GetFontTextExtentPoint(LvlReq, lstrlen(LvlReq), &size4);
		DrawFontText((smScreenWidth / 2 + 115) - (size4.cx / 2), basey + 230, LvlReq, COR, 3);

		if (isMousePos(smScreenWidth / 2 + 35, 450, 182, 48)) {
			DrawImage(imagens[BTNTELEPORT_], smScreenWidth / 2 + 35, 450, 182, 48);
		}
		else {
			DrawImage(imagens[BTNTELEPORT], smScreenWidth / 2 + 35, 450, 182, 48);
		}
		if (isMousePos(smScreenWidth / 2 + 210, 82, 26, 26)) {
			DrawImage(imagens[BTNCLOSE_], smScreenWidth / 2 + 210, 82, 26, 26);
		}
		else {
			DrawImage(imagens[BTNCLOSE], smScreenWidth / 2 + 210, 82, 26, 26);
		}

		if (index == 1) {


			DrawImage(imagens[EXP], smScreenWidth / 2 - 223, basey + 42, 40, 13);
			DrawImage(imagens[EXP], smScreenWidth / 2 - 223, basey + 62, 40, 13);
			DrawImage(imagens[EXP], smScreenWidth / 2 - 223, basey + 82, 40, 13);
			DrawImage(imagens[EXP], smScreenWidth / 2 - 223, basey + 102, 40, 13);
			DrawImage(imagens[EXP], smScreenWidth / 2 - 223, basey + 122, 40, 13);
			DrawImage(imagens[EXP], smScreenWidth / 2 - 223, basey + 142, 40, 13);
			DrawImage(imagens[EXP], smScreenWidth / 2 - 223, basey + 162, 40, 13);
			DrawImage(imagens[EXP], smScreenWidth / 2 - 223, basey + 182, 40, 13);
			DrawImage(imagens[EXP], smScreenWidth / 2 - 223, basey + 202, 40, 13);
			DrawImage(imagens[EXP], smScreenWidth / 2 - 223, basey + 222, 40, 13);
			DrawImage(imagens[EXP], smScreenWidth / 2 - 223, basey + 242, 40, 13);
			DrawImage(imagens[EXP], smScreenWidth / 2 - 223, basey + 262, 40, 13);
			DrawImage(imagens[EXP], smScreenWidth / 2 - 223, basey + 282, 40, 13);
			DrawImage(imagens[EXP], smScreenWidth / 2 - 223, basey + 302, 40, 13);
			DrawImage(imagens[EXP], smScreenWidth / 2 - 223, basey + 322, 40, 13);
			DrawImage(imagens[EXP], smScreenWidth / 2 - 223, basey + 342, 40, 13);
			DrawImage(imagens[EXP], smScreenWidth / 2 - 223, basey + 362, 40, 13);
			DrawImage(imagens[EXP], smScreenWidth / 2 - 223, basey + 382, 40, 13);




			size4 = {};
			GetFontTextExtentPoint(Text[0], lstrlen(Text[0]), &size4);
			if (isMousePos(cd, basey + 40, size4.cx + 13, 15)) {

				DrawFontText(cd, basey + 40, Text[0], COR1, 3);
			}
			else if (click == 0) {
				DrawFontText(cd, basey + 40, Text[0], COR1, 3);


			}
			else {
				DrawFontText(cd, basey + 40, Text[0], COR, 3);
			}

			//
			size4 = {};
			GetFontTextExtentPoint(Text[1], lstrlen(Text[1]), &size4);
			if (isMousePos(cd, basey + 60, size4.cx + 13, 15)) {
				DrawFontText(cd, basey + 60, Text[1], COR1, 3);
			}
			else if (click == 1) {
				DrawFontText(cd, basey + 60, Text[1], COR1, 3);
			}
			else {
				DrawFontText(cd, basey + 60, Text[1], COR, 3);
			}
			//
			size4 = {};
			GetFontTextExtentPoint(Text[2], lstrlen(Text[2]), &size4);
			if (isMousePos(cd, basey + 80, size4.cx + 13, 15)) {
				DrawFontText(cd, basey + 80, Text[2], COR1, 3);
			}
			else if (click == 2) {
				DrawFontText(cd, basey + 80, Text[2], COR1, 3);
			}
			else {
				DrawFontText(cd, basey + 80, Text[2], COR, 3);
			}
			//
			size4 = {};
			GetFontTextExtentPoint(Text[3], lstrlen(Text[3]), &size4);
			if (isMousePos(cd, basey + 100, size4.cx + 13, 15)) {
				DrawFontText(cd, basey + 100, Text[3], COR1, 3);
			}
			else if (click == 3) {
				DrawFontText(cd, basey + 100, Text[3], COR1, 3);
			}
			else {
				DrawFontText(cd, basey + 100, Text[3], COR, 3);
			}
			//
			size4 = {};
			GetFontTextExtentPoint(Text[4], lstrlen(Text[4]), &size4);
			if (isMousePos(cd, basey + 120, size4.cx + 13, 15)) {
				DrawFontText(cd, basey + 120, Text[4], COR1, 3);
			}
			else if (click == 4) {
				DrawFontText(cd, basey + 120, Text[4], COR1, 3);
			}
			else {
				DrawFontText(cd, basey + 120, Text[4], COR, 3);
			}
			//
			size4 = {};
			GetFontTextExtentPoint(Text[5], lstrlen(Text[5]), &size4);
			if (isMousePos(cd, basey + 140, size4.cx + 13, 15)) {
				DrawFontText(cd, basey + 140, Text[5], COR1, 3);
			}
			else if (click == 5) {
				DrawFontText(cd, basey + 140, Text[5], COR1, 3);
			}
			else {
				DrawFontText(cd, basey + 140, Text[5], COR, 3);
			}
			//
			size4 = {};
			GetFontTextExtentPoint(Text[6], lstrlen(Text[6]), &size4);
			if (isMousePos(cd, basey + 160, size4.cx + 13, 15)) {
				DrawFontText(cd, basey + 160, Text[6], COR1, 3);
			}
			else if (click == 6) {
				DrawFontText(cd, basey + 160, Text[6], COR1, 3);
			}
			else {
				DrawFontText(cd, basey + 160, Text[6], COR, 3);
			}
			size4 = {};
			GetFontTextExtentPoint(Text[7], lstrlen(Text[7]), &size4);
			if (isMousePos(cd, basey + 180, size4.cx + 13, 15)) {
				DrawFontText(cd, basey + 180, Text[7], COR1, 3);
			}
			else if (click == 7) {
				DrawFontText(cd, basey + 180, Text[7], COR1, 3);
			}
			else {
				DrawFontText(cd, basey + 180, Text[7], COR, 3);
			}
			size4 = {};
			GetFontTextExtentPoint(Text[8], lstrlen(Text[8]), &size4);
			if (isMousePos(cd, basey + 200, size4.cx + 13, 15)) {
				DrawFontText(cd, basey + 200, Text[8], COR1, 3);
			}
			else if (click == 8) {
				DrawFontText(cd, basey + 200, Text[8], COR1, 3);
			}
			else {
				DrawFontText(cd, basey + 200, Text[8], COR, 3);
			}
			size4 = {};
			GetFontTextExtentPoint(Text[9], lstrlen(Text[9]), &size4);
			if (isMousePos(cd, basey + 220, size4.cx + 13, 15)) {
				DrawFontText(cd, basey + 220, Text[9], COR1, 3);
			}
			else if (click == 9) {
				DrawFontText(cd, basey + 220, Text[9], COR1, 3);
			}
			else {
				DrawFontText(cd, basey + 220, Text[9], COR, 3);
			}
			size4 = {};
			GetFontTextExtentPoint(Text[10], lstrlen(Text[10]), &size4);
			if (isMousePos(cd, basey + 240, size4.cx + 13, 15)) {
				DrawFontText(cd, basey + 240, Text[10], COR1, 3);
			}
			else if (click == 10) {
				DrawFontText(cd, basey + 240, Text[10], COR1, 3);
			}
			else {
				DrawFontText(cd, basey + 240, Text[10], COR, 3);
			}
			size4 = {};
			GetFontTextExtentPoint(Text[11], lstrlen(Text[11]), &size4);
			if (isMousePos(cd, basey + 260, size4.cx + 13, 15)) {
				DrawFontText(cd, basey + 260, Text[11], COR1, 3);
			}
			else if (click == 11) {
				DrawFontText(cd, basey + 260, Text[11], COR1, 3);
			}
			else {
				DrawFontText(cd, basey + 260, Text[11], COR, 3);
			}
			size4 = {};
			GetFontTextExtentPoint(Text[12], lstrlen(Text[12]), &size4);
			if (isMousePos(cd, basey + 280, size4.cx + 13, 15)) {
				DrawFontText(cd, basey + 280, Text[12], COR1, 3);
			}
			else if (click == 12) {
				DrawFontText(cd, basey + 280, Text[12], COR1, 3);
			}
			else {
				DrawFontText(cd, basey + 280, Text[12], COR, 3);
			}
			size4 = {};
			GetFontTextExtentPoint(Text[13], lstrlen(Text[13]), &size4);
			if (isMousePos(cd, basey + 300, size4.cx + 13, 15)) {
				DrawFontText(cd, basey + 300, Text[13], COR1, 3);
			}
			else if (click == 13) {
				DrawFontText(cd, basey + 300, Text[13], COR1, 3);

			}
			else {
				DrawFontText(cd, basey + 300, Text[13], COR, 3);
			}
			size4 = {};
			GetFontTextExtentPoint(Text[14], lstrlen(Text[14]), &size4);
			if (isMousePos(cd, basey + 320, size4.cx + 13, 15)) {
				DrawFontText(cd, basey + 320, Text[14], COR1, 3);
			}
			else if (click == 14) {
				DrawFontText(cd, basey + 320, Text[14], COR1, 3);
			}
			else {
				DrawFontText(cd, basey + 320, Text[14], COR, 3);
			}
			size4 = {};
			GetFontTextExtentPoint(Text[15], lstrlen(Text[15]), &size4);
			if (isMousePos(cd, basey + 340, size4.cx + 13, 15)) {
				DrawFontText(cd, basey + 340, Text[15], COR1, 3);
			}
			else if (click == 15) {
				DrawFontText(cd, basey + 340, Text[15], COR1, 3);
			}
			else {
				DrawFontText(cd, basey + 340, Text[15], COR, 3);
			}
			size4 = {};
			GetFontTextExtentPoint(Text[16], lstrlen(Text[16]), &size4);
			if (isMousePos(cd, basey + 360, size4.cx + 13, 15)) {
				DrawFontText(cd, basey + 360, Text[16], COR1, 3);
			}
			else if (click == 16) {
				DrawFontText(cd, basey + 360, Text[16], COR1, 3);
			}
			else {
				DrawFontText(cd, basey + 360, Text[16], COR, 3);
			}
			size4 = {};
			GetFontTextExtentPoint(Text[17], lstrlen(Text[17]), &size4);
			if (isMousePos(cd, basey + 380, size4.cx + 13, 15)) {
				DrawFontText(cd, basey + 380, Text[17], COR1, 3);
			}
			else if (click == 17) {
				DrawFontText(cd, basey + 380, Text[17], COR1, 3);
			}
			else {
				DrawFontText(cd, basey + 380, Text[17], COR, 3);
			}
		}
		else if (index == 2) {
			//Imagens do Core pvp , sheltons exp dropexp //Caso Queira add mais, So diminui ou aumentar  Basey +20 ou -20 Rubinho
			DrawImage(imagens[EXP], smScreenWidth / 2 - 223, basey + 42, 40, 13);
			DrawImage(imagens[EXP], smScreenWidth / 2 - 223, basey + 62, 40, 13);
			DrawImage(imagens[EXP], smScreenWidth / 2 - 223, basey + 82, 40, 13);
			DrawImage(imagens[EXP], smScreenWidth / 2 - 223, basey + 102, 40, 13);
			DrawImage(imagens[EXP], smScreenWidth / 2 - 223, basey + 122, 40, 13);
			DrawImage(imagens[EXP], smScreenWidth / 2 - 223, basey + 142, 40, 13);
			DrawImage(imagens[EXP], smScreenWidth / 2 - 223, basey + 162, 40, 13);
			DrawImage(imagens[EXP], smScreenWidth / 2 - 223, basey + 182, 40, 13);
			DrawImage(imagens[EXP], smScreenWidth / 2 - 223, basey + 202, 40, 13);
			DrawImage(imagens[EXP], smScreenWidth / 2 - 223, basey + 222, 40, 13);
			DrawImage(imagens[EXP], smScreenWidth / 2 - 223, basey + 242, 40, 13);
			DrawImage(imagens[EXP], smScreenWidth / 2 - 223, basey + 262, 40, 13);
			DrawImage(imagens[EXP], smScreenWidth / 2 - 223, basey + 282, 40, 13);
			DrawImage(imagens[EXP], smScreenWidth / 2 - 223, basey + 302, 40, 13);
			DrawImage(imagens[EXP], smScreenWidth / 2 - 223, basey + 322, 40, 13);
			DrawImage(imagens[EXP], smScreenWidth / 2 - 223, basey + 342, 40, 13);
			DrawImage(imagens[EXP], smScreenWidth / 2 - 223, basey + 362, 40, 13);
			DrawImage(imagens[EXP], smScreenWidth / 2 - 223, basey + 382, 40, 13);
			size4 = {};
			GetFontTextExtentPoint(Text[18], lstrlen(Text[18]), &size4);
			if (isMousePos(cd, basey + 40, size4.cx + 13, 15)) {

				DrawFontText(cd, basey + 40, Text[18], COR1, 3);
			}
			else if (click == 18) {
				DrawFontText(cd, basey + 40, Text[18], COR1, 3);


			}
			else {
				DrawFontText(cd, basey + 40, Text[18], COR, 3);
			}

			//
			size4 = {};
			GetFontTextExtentPoint(Text[19], lstrlen(Text[19]), &size4);
			if (isMousePos(cd, basey + 60, size4.cx + 13, 15)) {
				DrawFontText(cd, basey + 60, Text[19], COR1, 3);
			}
			else if (click == 19) {
				DrawFontText(cd, basey + 60, Text[19], COR1, 3);
			}
			else {
				DrawFontText(cd, basey + 60, Text[19], COR, 3);
			}
			//
			size4 = {};
			GetFontTextExtentPoint(Text[20], lstrlen(Text[20]), &size4);
			if (isMousePos(cd, basey + 80, size4.cx + 13, 15)) {
				DrawFontText(cd, basey + 80, Text[20], COR1, 3);
			}
			else if (click == 20) {
				DrawFontText(cd, basey + 80, Text[20], COR1, 3);
			}
			else {
				DrawFontText(cd, basey + 80, Text[20], COR, 3);
			}
			//
			size4 = {};
			GetFontTextExtentPoint(Text[21], lstrlen(Text[21]), &size4);
			if (isMousePos(cd, basey + 100, size4.cx + 13, 15)) {
				DrawFontText(cd, basey + 100, Text[21], COR1, 3);
			}
			else if (click == 21) {
				DrawFontText(cd, basey + 100, Text[21], COR1, 3);
			}
			else {
				DrawFontText(cd, basey + 100, Text[21], COR, 3);
			}
			//
			size4 = {};
			GetFontTextExtentPoint(Text[22], lstrlen(Text[22]), &size4);
			if (isMousePos(cd, basey + 120, size4.cx + 13, 15)) {
				DrawFontText(cd, basey + 120, Text[22], COR1, 3);
			}
			else if (click == 22) {
				DrawFontText(cd, basey + 120, Text[22], COR1, 3);
			}
			else {
				DrawFontText(cd, basey + 120, Text[22], COR, 3);
			}
			//
			size4 = {};
			GetFontTextExtentPoint(Text[23], lstrlen(Text[23]), &size4);
			if (isMousePos(cd, basey + 140, size4.cx + 13, 15)) {
				DrawFontText(cd, basey + 140, Text[23], COR1, 3);
			}
			else if (click == 23) {
				DrawFontText(cd, basey + 140, Text[23], COR1, 3);
			}
			else {
				DrawFontText(cd, basey + 140, Text[23], COR, 3);
			}
			//
			size4 = {};
			GetFontTextExtentPoint(Text[24], lstrlen(Text[24]), &size4);
			if (isMousePos(cd, basey + 160, size4.cx + 13, 15)) {
				DrawFontText(cd, basey + 160, Text[24], COR1, 3);
			}
			else if (click == 24) {
				DrawFontText(cd, basey + 160, Text[24], COR1, 3);
			}
			else {
				DrawFontText(cd, basey + 160, Text[24], COR, 3);
			}
			size4 = {};
			GetFontTextExtentPoint(Text[25], lstrlen(Text[25]), &size4);
			if (isMousePos(cd, basey + 180, size4.cx + 13, 15)) {
				DrawFontText(cd, basey + 180, Text[25], COR1, 3);
			}
			else if (click == 25) {
				DrawFontText(cd, basey + 180, Text[25], COR1, 3);
			}
			else {
				DrawFontText(cd, basey + 180, Text[25], COR, 3);
			}
			size4 = {};
			GetFontTextExtentPoint(Text[26], lstrlen(Text[26]), &size4);
			if (isMousePos(cd, basey + 200, size4.cx + 13, 15)) {
				DrawFontText(cd, basey + 200, Text[26], COR1, 3);
			}
			else if (click == 26) {
				DrawFontText(cd, basey + 200, Text[26], COR1, 3);
			}
			else {
				DrawFontText(cd, basey + 200, Text[26], COR, 3);
			}
			size4 = {};
			GetFontTextExtentPoint(Text[27], lstrlen(Text[27]), &size4);
			if (isMousePos(cd, basey + 220, size4.cx + 13, 15)) {
				DrawFontText(cd, basey + 220, Text[27], COR1, 3);
			}
			else if (click == 27) {
				DrawFontText(cd, basey + 220, Text[27], COR1, 3);
			}
			else {
				DrawFontText(cd, basey + 220, Text[27], COR, 3);
			}
			size4 = {};
			GetFontTextExtentPoint(Text[28], lstrlen(Text[28]), &size4);
			if (isMousePos(cd, basey + 240, size4.cx + 13, 15)) {
				DrawFontText(cd, basey + 240, Text[28], COR1, 3);
			}
			else if (click == 28) {
				DrawFontText(cd, basey + 240, Text[28], COR1, 3);
			}
			else {
				DrawFontText(cd, basey + 240, Text[28], COR, 3);
			}
			size4 = {};
			GetFontTextExtentPoint(Text[29], lstrlen(Text[29]), &size4);
			if (isMousePos(cd, basey + 260, size4.cx + 13, 15)) {
				DrawFontText(cd, basey + 260, Text[29], COR1, 3);
			}
			else if (click == 29) {
				DrawFontText(cd, basey + 260, Text[29], COR1, 3);
			}
			else {
				DrawFontText(cd, basey + 260, Text[29], COR, 3);
			}
			size4 = {};
			GetFontTextExtentPoint(Text[30], lstrlen(Text[30]), &size4);
			if (isMousePos(cd, basey + 280, size4.cx + 13, 15)) {
				DrawFontText(cd, basey + 280, Text[30], COR1, 3);
			}
			else if (click == 30) {
				DrawFontText(cd, basey + 280, Text[30], COR1, 3);
			}
			else {
				DrawFontText(cd, basey + 280, Text[30], COR, 3);
			}
			size4 = {};
			GetFontTextExtentPoint(Text[31], lstrlen(Text[31]), &size4);
			if (isMousePos(cd, basey + 300, size4.cx + 13, 15)) {
				DrawFontText(cd, basey + 300, Text[31], COR1, 3);
			}
			else if (click == 31) {
				DrawFontText(cd, basey + 300, Text[31], COR1, 3);

			}
			else {
				DrawFontText(cd, basey + 300, Text[31], COR, 3);
			}
			size4 = {};
			GetFontTextExtentPoint(Text[32], lstrlen(Text[32]), &size4);
			if (isMousePos(cd, basey + 320, size4.cx + 13, 15)) {
				DrawFontText(cd, basey + 320, Text[32], COR1, 3);
			}
			else if (click == 32) {
				DrawFontText(cd, basey + 320, Text[32], COR1, 3);
			}
			else {
				DrawFontText(cd, basey + 320, Text[32], COR, 3);
			}
			size4 = {};
			GetFontTextExtentPoint(Text[33], lstrlen(Text[33]), &size4);
			if (isMousePos(cd, basey + 340, size4.cx + 13, 15)) {
				DrawFontText(cd, basey + 340, Text[33], COR1, 3);
			}
			else if (click == 33) {
				DrawFontText(cd, basey + 340, Text[33], COR1, 3);
			}
			else {
				DrawFontText(cd, basey + 340, Text[33], COR, 3);
			}
			size4 = {};
			GetFontTextExtentPoint(Text[34], lstrlen(Text[34]), &size4);
			if (isMousePos(cd, basey + 360, size4.cx + 13, 15)) {
				DrawFontText(cd, basey + 360, Text[34], COR1, 3);
			}
			else if (click == 34) {
				DrawFontText(cd, basey + 360, Text[34], COR1, 3);
			}
			else {
				DrawFontText(cd, basey + 360, Text[34], COR, 3);
			}
			size4 = {};
			GetFontTextExtentPoint(Text[35], lstrlen(Text[35]), &size4);
			if (isMousePos(cd, basey + 380, size4.cx + 13, 15)) {
				DrawFontText(cd, basey + 380, Text[35], COR1, 3);
			}
			else if (click == 35) {
				DrawFontText(cd, basey + 380, Text[35], COR1, 3);
			}
			else {
				DrawFontText(cd, basey + 380, Text[35], COR, 3);
			}



		}
		else if (index == 3) {
			DrawImage(imagens[EXP], smScreenWidth / 2 - 223, basey + 42, 40, 13);
			DrawImage(imagens[EXP], smScreenWidth / 2 - 223, basey + 62, 40, 13);


			size4 = {};
			GetFontTextExtentPoint(Text[36], lstrlen(Text[36]), &size4);
			if (isMousePos(cd, basey + 40, size4.cx + 13, 15)) {

				DrawFontText(cd, basey + 40, Text[36], COR1, 3);
			}
			else if (click == 36) {
				DrawFontText(cd, basey + 40, Text[36], COR1, 3);


			}
			else {
				DrawFontText(cd, basey + 40, Text[36], COR, 3);
			}

			//
			size4 = {};
			GetFontTextExtentPoint(Text[37], lstrlen(Text[37]), &size4);
			if (isMousePos(cd, basey + 60, size4.cx + 13, 15)) {
				DrawFontText(cd, basey + 60, Text[37], COR1, 3);
			}
			else if (click == 37) {
				DrawFontText(cd, basey + 60, Text[37], COR1, 3);
			}
			else {
				DrawFontText(cd, basey + 60, Text[37], COR, 3);
			}


		}
	}

}

bool TCORE::MouseIn() {
	int cd = (smScreenWidth / 2) - 180;
	DWORD COR = RGB(255, 100, 0);
	if (Open) {
		if (isMousePos((smScreenWidth / 2) - 180 + 44, basey + 400, 26, 26)) {
			index = 1;
		}

		if (isMousePos((smScreenWidth / 2) - 180 + 74, basey + 400, 26, 26)) {
			index = 2;
		}
		if (isMousePos((smScreenWidth / 2) - 180 + 104, basey + 400, 26, 26)) {
			index = 3;
		}
		if (isMousePos(smScreenWidth / 2 + 210, 82, 26, 26)) {
			tcoreClose();
			stop = FALSE;
		}
		if (index == 1) {
			if (isMousePos(cd, basey + 40, 120, 15)) {

				mapa = atoi(FieldNum[0][1]);
				click = atoi(FieldNum[0][0]);
				lvl = atoi(FieldNum[0][2]);

			}
			if (isMousePos(cd, basey + 60, 120, 15)) {
				mapa = atoi(FieldNum[1][1]);
				click = atoi(FieldNum[1][0]);
				lvl = atoi(FieldNum[1][2]);
			}
			if (isMousePos(cd, basey + 80, size4[2].cx + 13, 15)) {
				mapa = atoi(FieldNum[2][1]);
				click = atoi(FieldNum[2][0]);
				lvl = atoi(FieldNum[2][2]);
			}
			if (isMousePos(cd, basey + 100, size4[3].cx + 13, 15)) {
				mapa = atoi(FieldNum[3][1]);
				click = atoi(FieldNum[3][0]);
				lvl = atoi(FieldNum[3][2]);

			}
			if (isMousePos(cd, basey + 120, size4[4].cx + 13, 15)) {
				mapa = atoi(FieldNum[4][1]);
				click = atoi(FieldNum[4][0]);
				lvl = atoi(FieldNum[4][2]);
			}
			if (isMousePos(cd, basey + 140, size4[5].cx + 13, 15)) {
				mapa = atoi(FieldNum[5][1]);
				click = atoi(FieldNum[5][0]);
				lvl = atoi(FieldNum[5][2]);
			}
			if (isMousePos(cd, basey + 160, size4[6].cx + 13, 15)) {
				mapa = atoi(FieldNum[6][1]);
				click = atoi(FieldNum[6][0]);
				lvl = atoi(FieldNum[6][2]);
			}
			if (isMousePos(cd, basey + 180, size4[7].cx + 13, 15)) {
				mapa = atoi(FieldNum[7][1]);
				click = atoi(FieldNum[7][0]);
				lvl = atoi(FieldNum[7][2]);
			}
			if (isMousePos(cd, basey + 200, size4[8].cx + 13, 15)) {
				mapa = atoi(FieldNum[8][1]);
				click = atoi(FieldNum[8][0]);
				lvl = atoi(FieldNum[8][2]);
			}
			if (isMousePos(cd, basey + 220, size4[9].cx + 13, 15)) {
				mapa = atoi(FieldNum[9][1]);
				click = atoi(FieldNum[9][0]);
				lvl = atoi(FieldNum[9][2]);
			}
			if (isMousePos(cd, basey + 240, size4[10].cx + 13, 15)) {
				mapa = atoi(FieldNum[10][1]);
				click = atoi(FieldNum[10][0]);
				lvl = atoi(FieldNum[10][2]);
			}
			if (isMousePos(cd, basey + 260, size4[11].cx + 13, 15)) {
				mapa = atoi(FieldNum[11][1]);
				click = atoi(FieldNum[11][0]);
				lvl = atoi(FieldNum[11][2]);
			}
			if (isMousePos(cd, basey + 280, size4[12].cx + 13, 15)) {
				mapa = atoi(FieldNum[12][1]);
				click = atoi(FieldNum[12][0]);
				lvl = atoi(FieldNum[12][2]);
			}
			if (isMousePos(cd, basey + 300, size4[13].cx + 13, 15)) {
				mapa = atoi(FieldNum[13][1]);
				click = atoi(FieldNum[13][0]);
				lvl = atoi(FieldNum[13][2]);
			}
			if (isMousePos(cd, basey + 320, size4[14].cx + 13, 15)) {
				mapa = atoi(FieldNum[14][1]);
				click = atoi(FieldNum[14][0]);
				lvl = atoi(FieldNum[14][2]);
			}
			if (isMousePos(cd, basey + 340, size4[15].cx + 13, 15)) {
				mapa = atoi(FieldNum[15][1]);
				click = atoi(FieldNum[15][0]);
				lvl = atoi(FieldNum[15][2]);
			}
			if (isMousePos(cd, basey + 360, size4[16].cx + 13, 15)) {
				mapa = atoi(FieldNum[16][1]);
				click = atoi(FieldNum[16][0]);
				lvl = atoi(FieldNum[16][2]);
			}
			if (isMousePos(cd, basey + 380, size4[17].cx + 13, 15)) {
				mapa = atoi(FieldNum[17][1]);
				click = atoi(FieldNum[17][0]);
				lvl = atoi(FieldNum[17][2]);
			}
		}
		else if (index == 2) {
			////////////
			if (isMousePos(cd, basey + 40, size4[18].cx + 13, 15)) {
				mapa = atoi(FieldNum[18][1]);
				click = atoi(FieldNum[18][0]);
				lvl = atoi(FieldNum[18][2]);

			}
			if (isMousePos(cd, basey + 60, size4[19].cx + 13, 15)) {
				mapa = atoi(FieldNum[19][1]);
				click = atoi(FieldNum[19][0]);
				lvl = atoi(FieldNum[19][2]);
			}
			if (isMousePos(cd, basey + 80, size4[20].cx + 13, 15)) {
				mapa = atoi(FieldNum[20][1]);
				click = atoi(FieldNum[20][0]);
				lvl = atoi(FieldNum[20][2]);
			}
			if (isMousePos(cd, basey + 100, size4[21].cx + 13, 15)) {
				mapa = atoi(FieldNum[21][1]);
				click = atoi(FieldNum[21][0]);
				lvl = atoi(FieldNum[21][2]);

			}
			if (isMousePos(cd, basey + 120, size4[22].cx + 13, 15)) {
				mapa = atoi(FieldNum[22][1]);
				click = atoi(FieldNum[22][0]);
				lvl = atoi(FieldNum[22][2]);
			}
			if (isMousePos(cd, basey + 140, size4[23].cx + 13, 15)) {
				mapa = atoi(FieldNum[23][1]);
				click = atoi(FieldNum[23][0]);
				lvl = atoi(FieldNum[23][2]);
			}
			if (isMousePos(cd, basey + 160, size4[24].cx + 13, 15)) {
				mapa = atoi(FieldNum[24][1]);
				click = atoi(FieldNum[24][0]);
				lvl = atoi(FieldNum[24][2]);
			}
			if (isMousePos(cd, basey + 180, size4[25].cx + 13, 15)) {
				mapa = atoi(FieldNum[25][1]);
				click = atoi(FieldNum[25][0]);
				lvl = atoi(FieldNum[25][2]);
			}
			if (isMousePos(cd, basey + 200, size4[26].cx + 13, 15)) {
				mapa = atoi(FieldNum[26][1]);
				click = atoi(FieldNum[26][0]);
				lvl = atoi(FieldNum[26][2]);
			}
			if (isMousePos(cd, basey + 220, size4[27].cx + 13, 15)) {
				mapa = atoi(FieldNum[27][1]);
				click = atoi(FieldNum[27][0]);
				lvl = atoi(FieldNum[27][2]);
			}
			if (isMousePos(cd, basey + 240, size4[28].cx + 13, 15)) {
				mapa = atoi(FieldNum[28][1]);
				click = atoi(FieldNum[28][0]);
				lvl = atoi(FieldNum[28][2]);
			}
			if (isMousePos(cd, basey + 260, size4[29].cx + 13, 15)) {
				mapa = atoi(FieldNum[29][1]);
				click = atoi(FieldNum[29][0]);
				lvl = atoi(FieldNum[29][2]);
			}
			if (isMousePos(cd, basey + 280, size4[30].cx + 13, 15)) {
				mapa = atoi(FieldNum[30][1]);
				click = atoi(FieldNum[30][0]);
				lvl = atoi(FieldNum[30][2]);
			}
			if (isMousePos(cd, basey + 300, size4[31].cx + 13, 15)) {
				mapa = atoi(FieldNum[31][1]);
				click = atoi(FieldNum[31][0]);
				lvl = atoi(FieldNum[31][2]);
			}
			if (isMousePos(cd, basey + 320, size4[32].cx + 13, 15)) {
				mapa = atoi(FieldNum[32][1]);
				click = atoi(FieldNum[32][0]);
				lvl = atoi(FieldNum[32][2]);
			}
			if (isMousePos(cd, basey + 340, size4[33].cx + 13, 15)) {
				mapa = atoi(FieldNum[33][1]);
				click = atoi(FieldNum[33][0]);
				lvl = atoi(FieldNum[33][2]);
			}
			if (isMousePos(cd, basey + 360, size4[34].cx + 13, 15)) {
				mapa = atoi(FieldNum[34][1]);
				click = atoi(FieldNum[34][0]);
				lvl = atoi(FieldNum[34][2]);
			}
			if (isMousePos(cd, basey + 380, size4[35].cx + 13, 15)) {
				mapa = atoi(FieldNum[35][1]);
				click = atoi(FieldNum[35][0]);
				lvl = atoi(FieldNum[35][2]);
			}
		}

		//////////////////
		else if (index == 3) {


			if (isMousePos(cd, basey + 40, size4[36].cx + 13, 15)) {
				mapa = atoi(FieldNum[36][1]);
				click = atoi(FieldNum[36][0]);
				lvl = atoi(FieldNum[36][2]);
			}
			if (isMousePos(cd, basey + 60, size4[37].cx + 13, 15)) {
				mapa = atoi(FieldNum[37][1]);
				click = atoi(FieldNum[37][0]);
				lvl = atoi(FieldNum[37][2]);
			}

		}


		if (isMousePos(smScreenWidth / 2 + 35, 450, 182, 48)) {


			if (lpCurPlayer->smCharInfo.Level < lvl) {
				cMessageBox.ShowMessage(91);
				return FALSE;
			}
			else if (StageField[lpCurPlayer->OnStageField]->FieldCode == 16 || StageField[lpCurPlayer->OnStageField]->FieldCode == 30) {
				cMessageBox.ShowMessage(92);
				return FALSE;
			}
			WarpField2(mapa);
			tcoreClose();
			stop = FALSE;
			//sinThrowItemToFeild(&cInvenTory.InvenItem[chaPremiumitem.TelePortCore.ItemIndex]);
			if (lpCurPlayer->PlayerVip <= 0)
			{
				cInvenTory.InvenItem[chaPremiumitem.TelePortCore.ItemIndex].Flag = 0;
				cInvenTory.CheckWeight();
				cInvenTory.ReFormInvenItem();
			}
		}

	}
	return TRUE;
}



