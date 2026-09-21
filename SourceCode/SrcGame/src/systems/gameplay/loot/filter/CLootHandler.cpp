#include <cstdint>
#include "compat/legacy/sin/sinLinkHeader.h"
#include "systems/gameplay/loot/filter/CLootHandler.h"
#include "gameplay/classes/v2/services/ClassService.h"

CLootFilter cLootFilter;

bool CLootFilter::Filter(TRANS_ITEM * pItem)
{
	if (getEnabled())
	{
		DWORD base = (pItem->ItemCode&0xFFFF0000);
		DWORD spec = pItem->type;

		if (!base)
			return false;

		/*char buff[255];
		wsprintf(buff, "%08X %08X", base, spec);
		AddChatBuff(buff);*/

		if (base == sinPL1 && !getHpPot())
			return false;
		else if (base == sinPL1)
			return true;

		if (base == sinPS1 && !getSpPot())
			return false;
		else if (base == sinPS1)
			return true;

		if (base == sinPM1 && !getMpPot())
			return false;
		else if (base == sinPM1)
			return true;

		if (base == sinFO1 && !getForce())
			return false;
		else if (base == sinFO1)
			return true;

		if ((base == sinBI1 || base == sinBI2 || base == sinBI3) && !getPremiums())
			return false;
		else if (base == sinBI1 || base == sinBI2 || base == sinBI3)
			return true;

		if (base == sinGG1 && !getGold())
			return false;
		else if (base == sinGG1)
			return true;

		if ((base == sinGP1 || base == sinGP2) && !getCrystals())
			return false;
		else if (base == sinGP1 || base == sinGP2)
			return true;

		if (base == sinOA1 && !getAmulets())
			return false;

		if (base == sinOR1 && !getRings())
			return false;

		if (base == sinOS1 && !getSheltoms())
			return false;

		if ((base == sinDA1 || base == sinDA2 || base == sinDS1 || base == sinOM1 || base == sinOA2 ||
			base == sinDG1 || base == sinDB1) && !getDefense())
			return false;

		if ((base&0xFF000000) == 0x01000000 && !getOffense())
			return false;

		if (base == sinOA1 || base == sinOR1 || base == sinOS1 || (base == sinDA1 || base == sinDA2 || base == sinDS1 || base == sinOM1 || base == sinOA2 ||
			base == sinDG1 || base == sinDB1) || (base & 0xFF000000) == 0x01000000)
		{
			if (!spec && !getNonSpec())
				return false;
			else if (!spec)
				return true;

			int specializationJobCode = 0;
			if (gameplay::classes::DefaultClassService().TryLegacyJobCodeByJobBitCode(
				static_cast<std::uint32_t>(spec), specializationJobCode))
				return getClassSpec(specializationJobCode);
		}

		if (!getEverything())
			return false;
	}

	return true;
}




