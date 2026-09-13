#pragma once

#include "formats/character_config/CharacterConfigCommon.h"
#include "formats/character_config/CharacterConfigServices.h"
#include "formats/character_config/CharacterConfigTokens.h"

namespace summit
{
namespace formats
{
namespace character_config
{

inline ParseResult AppendShopItems(
	legacy_config::LineCursor &cursor,
	const CharacterConfigServices &services,
	void **items,
	int &count,
	int capacity)
{
	char value[128];
	while(cursor.NextValue(value,sizeof(value)))
	{
		if(count < 0 || count >= capacity) break;
		DWORD code=0;
		if(services.resolveItemCode && services.resolveItemCode(value,code))
			items[count++]=LegacyCodePointer(code);
	}
	return ParseResult::Handled;
}

inline int ReadOptionalInt(legacy_config::LineCursor &cursor, int defaultValue)
{
	return cursor.Empty() ? defaultValue : ReadInt(cursor,defaultValue);
}

inline ParseResult HandleServerNpcExtensions(const char *command, smCHAR_MONSTER_INFO &monster, const CharacterConfigServices &services)
{
	if(!services.enableServerNpcExtensions) return ParseResult::NotHandled;
	if(IsToken(command,token::ClanCustom)) { monster.ClanCustom=TRUE; return ParseResult::Handled; }
	if(IsToken(command,token::NpcInfo)) { monster.NPCinfo=TRUE; return ParseResult::Handled; }
	if(IsToken(command,token::Rank)) { monster.Rank=TRUE; return ParseResult::Handled; }
	if(IsToken(command,token::BattlePass)) { monster.BattlePass=TRUE; return ParseResult::Handled; }
	if(IsToken(command,token::SkinChange)) { monster.SkinChange=TRUE; return ParseResult::Handled; }
	if(IsToken(command,token::Shop)) { monster.ItemShop=TRUE; return ParseResult::Handled; }
	if(IsToken(command,token::TimeShop)) { monster.TimeShop=TRUE; return ParseResult::Handled; }
	if(IsToken(command,token::Ranking)) { monster.Ranking=TRUE; return ParseResult::Handled; }
	return ParseResult::NotHandled;
}

inline ParseResult HandleNpcServices(
	const char *command,
	legacy_config::LineCursor &cursor,
	smCHAR_MONSTER_INFO &monster,
	const CharacterConfigServices &services)
{
	if(IsToken(command,token::SellWeapon)) return AppendShopItems(cursor,services,monster.SellAttackItem,monster.SellAttackItemCount,32);
	if(IsToken(command,token::SellDefence)) return AppendShopItems(cursor,services,monster.SellDefenceItem,monster.SellDefenceItemCount,32);
	if(IsToken(command,token::SellEtc)) return AppendShopItems(cursor,services,monster.SellEtcItem,monster.SellEtcItemCount,32);
	if(IsToken(command,token::SkillMaster)) { monster.SkillMaster=TRUE; return ParseResult::Handled; }
	if(IsToken(command,token::SkillChangeJob)) { monster.SkillChangeJob=ReadOptionalInt(cursor,TRUE); return ParseResult::Handled; }
	if(IsToken(command,token::EventNpc)) { monster.EventNPC=ReadOptionalInt(cursor,1); return ParseResult::Handled; }
	if(IsToken(command,token::Warehouse)) { monster.WareHouseMaster=TRUE; return ParseResult::Handled; }
	if(IsToken(command,token::ItemMix)) { monster.ItemMix=TRUE; return ParseResult::Handled; }
	if(IsToken(command,token::Smelting)) { monster.Smelting=TRUE; return ParseResult::Handled; }
	if(IsToken(command,token::Manufacture)) { monster.Manufacture=TRUE; return ParseResult::Handled; }
	if(IsToken(command,token::Refining)) { monster.ItemMix=200; return ParseResult::Handled; }
	if(IsToken(command,token::ItemAging)) { monster.ItemAging=TRUE; return ParseResult::Handled; }
	if(IsToken(command,token::MixtureReset)) { monster.MixtureReset=TRUE; return ParseResult::Handled; }

	ParseResult extension=HandleServerNpcExtensions(command,monster,services);
	if(extension != ParseResult::NotHandled) return extension;

	if(IsToken(command,token::CollectMoney)) { monster.CollectMoney=TRUE; return ParseResult::Handled; }
#ifdef _LANGUAGE_VEITNAM
	if(IsToken(command,token::WowEvent)) { monster.WowEvent=TRUE; return ParseResult::Handled; }
#endif
	if(IsToken(command,token::EventGift)) { monster.EventGift=TRUE; return ParseResult::Handled; }
	if(IsToken(command,token::ClanNpc)) { monster.ClanNPC=TRUE; return ParseResult::Handled; }
	if(IsToken(command,token::GiftExpress)) { monster.GiftExpress=TRUE; return ParseResult::Handled; }
	if(IsToken(command,token::WingQuest)) { monster.WingQuestNpc=ReadOptionalInt(cursor,1); return ParseResult::Handled; }
	if(IsToken(command,token::QuestEvent)) { monster.WingQuestNpc=ReadOptionalInt(cursor,2); return ParseResult::Handled; }
	if(IsToken(command,token::StarPoint)) { monster.StarPointNpc=ReadOptionalInt(cursor,20); return ParseResult::Handled; }
	if(IsToken(command,token::GiveMoney)) { monster.GiveMoneyNpc=TRUE; return ParseResult::Handled; }
	if(IsToken(command,token::Teleport)) { monster.TelePortNpc=ReadOptionalInt(cursor,1); return ParseResult::Handled; }
	if(IsToken(command,token::BlessCastle)) { monster.BlessCastleNPC=ReadOptionalInt(cursor,1); return ParseResult::Handled; }
	if(IsToken(command,token::Polling)) { monster.PollingNpc=ReadOptionalInt(cursor,1); return ParseResult::Handled; }
	if(IsToken(command,token::MediaTitle) || IsToken(command,token::MediaPath))
	{
		char value[256];
		if(!cursor.NextValue(value,sizeof(value))) return ParseResult::Handled;
		char *stored=(value[0] && services.internNpcMessage) ? services.internNpcMessage(value) : 0;
		if(IsToken(command,token::MediaTitle)) monster.szMediaPlayNPC_Title=stored;
		else monster.szMediaPlayNPC_Path=stored;
		return ParseResult::Handled;
	}
	if(IsToken(command,token::OpenCount))
	{
		monster.OpenCount[0]=static_cast<short>(ReadInt(cursor));
		monster.OpenCount[1]=static_cast<short>(ReadInt(cursor));
		return ParseResult::Handled;
	}
	if(IsToken(command,token::QuestCode))
	{
		monster.QuestCode=ReadInt(cursor);
		monster.QuestParam=ReadInt(cursor);
		return ParseResult::Handled;
	}
	return ParseResult::NotHandled;
}

} // namespace character_config
} // namespace formats
} // namespace summit
