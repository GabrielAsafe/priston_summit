#include "opcodes.h"
#include "systems/combat/pvp/Score.h"
#include <string>
#include "systems/rendering/font/FontImage.h"
#include "systems/rendering/legacy_3d/smDsx.h"
#include "core/gameplay/runtime/playmain.h"
#include "systems/gameplay/character/core/character.h"
#include "systems/world/field/field.h"
#include "systems/configuration/client/Settings.h"
#include "systems/rendering/2d/legacy/drawsub.h"
#include "systems/presentation/opening/legacy_runtime/HoLinkHeader.h"
#include "compat/legacy/sin/sinLinkHeader.h"
#include "systems/network/v2/legacy/LegacyBackendChannelSender.h"

extern int haSiegeBoardFlag;

Score::Score()
{
}

Score::~Score()
{

}

void Score::sendAlert()
{
	unsigned int stage = StageField[lpCurPlayer->OnStageField]->FieldCode;

	extern BOOL bReiPVP;

	static unsigned int tick = 0;
	static bool sent = false;

	if (stage == 33 && bReiPVP)
	{

		if ((GetTickCount() - tick) > 120000 && !sent)
		{
			smTRANS_COMMAND smPacket;
			smPacket.code = OpCode::SEND_ALERT_PVP;
			smPacket.WParam = 0;
			smPacket.EParam = 0;
			smPacket.size = sizeof(smPacket);

			gameplay::network::legacy::ClientLegacyBackendChannelSender().Send(
				gameplay::network::BackendChannel::Data,
				gameplay::network::PacketView(&smPacket, static_cast<std::size_t>(smPacket.size),
				                              static_cast<std::uint32_t>(smPacket.code)));

			tick = GetTickCount();
			sent = true;
		}
	}
	else
	{
		if ((GetTickCount() - tick) > 120000 && sent)
		{
			sent = !sent;
		}
	}


}

bool Score::drawZone()
{
	int zPosition = lpCurPlayer->pZ >> 0x8;
	char* text = nullptr;
	COLORREF colour = NULL;
	int widthPVP;

	if (zPosition >= -23840)
	{
		text = "SAFE ZONE";
		colour = D3DCOLOR_RGBA(0, 255, 0, 255);
		widthPVP = 60;
	}
	else if (zPosition >= -24059 && zPosition < -23840)
	{
		text = "CAUTION ZONE";
		colour = D3DCOLOR_RGBA(255, 180, 0, 255);
		widthPVP = 73;
	}
	else
	{
		text = "DANGER ZONE";
		colour = D3DCOLOR_RGBA(255, 0, 0, 255);
		widthPVP = 70;
	}

	DrawFontImage(text, SETTINGSHANDLER->getWidth() / 2 - widthPVP, 46, colour, 0.6f);
	
	return true;
}

bool Score::drawScore()
{
	char Bps[255];
	wsprintf(Bps, "Pontos de Batalha: %d", lpCurPlayer->PontosPvP);
	//DrawFontImage("Pontos de Batalha", Settings::getInstance()->getWidth() / 2 - 100, 60, RGBA_MAKE(255, 255, 255, 255), 0.7f);
	DrawFontImage(const_cast<char*>(std::to_string(lpCurPlayer->sBlessCastle_Damage[1]).c_str()), SETTINGSHANDLER->getWidth() / 2 + 30, 9, D3DCOLOR_RGBA(0, 255, 0, 255), 0.7f);

	return true;
}

bool Score::drawBoss(int stage)
{
	int zPosition = lpCurPlayer->pZ >> 0x8;
	int xPosition = lpCurPlayer->pX >> 0x8;

	char* text = nullptr;
	if ((zPosition >= 11000 && zPosition < 11400) && (xPosition >= 2087 && xPosition < 2487) && stage == 5)//sergio
	{
		DrawFontImage("Local de Nascimento do Boss: Greedy/Draxos",SETTINGSHANDLER->getWidth() / 2 - 200, 30, D3DCOLOR_RGBA(255, 210, 14, 255), 0.8f);
	}

	/*if ((zPosition >= 11000 && zPosition < 11400) && (xPosition >= 2087 && xPosition < 2487) && stage == 46)
	{
		DrawFontImage("Local de Nascimento do Boss: Draxos",SETTINGSHANDLER->getWidth() / 2 - 200, 30, D3DCOLOR_RGBA(255, 210, 14, 255), 0.8f);
	}*/

	if ((zPosition >= 30465 && zPosition < 30665) && (xPosition >= 33815 && xPosition < 33998) && stage == 31)
	{
		DrawFontImage("Local de Nascimento do Boss: Valento",SETTINGSHANDLER->getWidth() / 2 - 200, 30, D3DCOLOR_RGBA(255, 210, 14, 255), 0.8f);
	}

	if ((zPosition >= 13025 && zPosition < 13425) && (xPosition >= -11857 && xPosition < -12257) && stage == 38)
	{
		DrawFontImage("Local de Nascimento do Boss: Mokova", SETTINGSHANDLER->getWidth() / 2 - 200, 30, D3DCOLOR_RGBA(255, 210, 14, 255), 0.8f);
	}

	if ((zPosition >= -36430 && zPosition < -36060) && (xPosition >= 19010 && xPosition < 19415) && stage == 44)
	{
		DrawFontImage("Local de Nascimento do Boss: Tulla", SETTINGSHANDLER->getWidth() / 2 - 200, 30, D3DCOLOR_RGBA(255, 210, 14, 255), 0.8f);
	}

	if ((zPosition >= 21484 && zPosition < 21884) && (xPosition >= 37943 && xPosition < 38343) && stage == 28)
	{
		DrawFontImage("Local de Nascimento do Boss: Babel", SETTINGSHANDLER->getWidth() / 2 - 200, 30, D3DCOLOR_RGBA(255, 210, 14, 255), 0.8f);
	}

	if ((zPosition >= 14099 && zPosition < 14499) && (xPosition >= 46899 && xPosition < 47299) && stage == 27)
	{
		DrawFontImage("Local de Nascimento do Boss: Babel", SETTINGSHANDLER->getWidth() / 2 - 200, 30, D3DCOLOR_RGBA(255, 210, 14, 255), 0.8f);
	}

	if ((zPosition >= 38643 && zPosition < 38996) && (xPosition >= 38201 && xPosition < 38601) && stage == 35)
	{
		DrawFontImage("Local de Nascimento do Boss: Tartaruga Dragao", SETTINGSHANDLER->getWidth() / 2 - 200, 30, D3DCOLOR_RGBA(255, 210, 14, 255), 0.8f);
	}

	if ((zPosition >= 52729 && zPosition < 52329) && (xPosition >= 32823 && xPosition < 33223) && stage == 36)
	{
		DrawFontImage("Local de Nascimento do Boss: Kelvezu", SETTINGSHANDLER->getWidth() / 2 - 200, 30, D3DCOLOR_RGBA(255, 210, 14, 255), 0.8f);
	}

	if ((zPosition >= -1267 && zPosition < -1667) && (xPosition >= -15802 && xPosition < -16202) && stage == 37)
	{
		DrawFontImage("Local de Nascimento do Boss: Cao Esqueleto", SETTINGSHANDLER->getWidth() / 2 - 200, 30, D3DCOLOR_RGBA(255, 210, 14, 255), 0.8f);
	}

	if ((zPosition >= -40130 && zPosition < -40530) && (xPosition >= 14044 && xPosition < 14444) && stage == 40)
	{
		DrawFontImage("Local de Nascimento do Boss: Boss Mundo", SETTINGSHANDLER->getWidth() / 2 - 200, 30, D3DCOLOR_RGBA(255, 210, 14, 255), 0.8f);
	}

	if ((zPosition >= -38384 && zPosition < -38784) && (xPosition >= 5067 && xPosition < 5467) && stage == 41)
	{
		DrawFontImage("Local de Nascimento do Boss: Boss Mundo", SETTINGSHANDLER->getWidth() / 2 - 200, 30, D3DCOLOR_RGBA(255, 210, 14, 255), 0.8f);
	}

	if ((zPosition >= -42406 && zPosition < -42806) && (xPosition >= 6259 && xPosition < 6659) && stage == 43)
	{
		DrawFontImage("Local de Nascimento do Boss: Devil Shy", SETTINGSHANDLER->getWidth() / 2 - 200, 30, D3DCOLOR_RGBA(255, 210, 14, 255), 0.8f);
	}

	return true;
}


bool Score::drawImgPVP()
{
	extern int	imgPVP;

	
	int WinX = (WinSizeX / 2) - 160, WinY = 0;
	dsDrawTexImage(imgPVP, WinX, WinY, 300 - 1, 80 - 1, 255);
	return true;
}

bool Score::draw()
{
	unsigned int stage = StageField[lpCurPlayer->OnStageField]->FieldCode;
	bool result = false;
	extern bool ativarImgPVP;

	if (stage == 33 && !haSiegeBoardFlag)
	{

		if (ativarImgPVP)
		{
			drawImgPVP();
			drawZone();
			drawScore();
		}


		result = true;
	}

	if (stage == 47 || stage == 46 || stage == 44)
	{
		drawBoss(stage);

		result = true;
	}

	return result;
}





