#include "systems/ui/settings/UI.h"
#include "systems/rendering/legacy_3d/smDsx.h"
#include "systems/rendering/legacy_3d/smType.h"
#include "systems/rendering/legacy_3d/smTexture.h"
#include "systems/rendering/2d/legacy/drawsub.h"
#include "systems/configuration/client/Settings.h"
#include "systems/rendering/font/Font.h"
#include "systems/rendering/2d/legacy/drawsub.h"

SettingsUI settingsUI;

SettingsUI::SettingsUI()
{
	status = false;
}

SettingsUI::~SettingsUI()
{

}

void SettingsUI::open()
{
	static unsigned long long timer = 0;

	if((GetTickCount() - timer) > 2000)
	{
		// Stop char from walking before open the settings ( this function MUST come before opened = true )
		int SetMousePlay(int flag);
		SetMousePlay(0);

		status = true;

		timer = GetTickCount();
	}
	else
	{
		int AddChatBuff(char *szMsg);

		AddChatBuff("> Aguarde 2 segundos para abrir o menu novamente.");
	}

}

void SettingsUI::close()
{
	status = false;
}

bool SettingsUI::getStatus() const
{
	return status;
}




