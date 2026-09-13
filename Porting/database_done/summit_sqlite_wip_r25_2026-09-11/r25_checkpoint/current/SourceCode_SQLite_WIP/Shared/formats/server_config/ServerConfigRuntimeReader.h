#pragma once

#include "formats/server_config/ServerConfigCommon.h"

namespace summit
{
namespace formats
{
namespace server_config
{

namespace runtime_token
{
static const char DefaultItemUser[] = "\x2A\xBD\xC5\xB1\xD4\xC0\xAF\xC0\xFA\xBC\xB1\xB9\xB0";
static const char RecordJobChanger[] = "\x2A\xC0\xFC\xBE\xF7\xC0\xDA\xB1\xE2\xB7\xCF";
static const char EnableBlockForever[] = "\x2A\xBF\xB5\xB1\xB8\xBA\xED\xB7\xB0\xC7\xE3\xB0\xA1";
static const char PermitExp[] = "\x2A\xB0\xE6\xC7\xE8\xC4\xA1\xBF\xC0\xC2\xF7\xB9\xFC\xC0\xA7";
static const char PermitMoney[] = "\x2A\xB5\xB7\xBF\xC0\xC2\xF7\xB9\xFC\xC0\xA7";
static const char ClientCheckProcessTime[] = "\x2A\xC7\xC1\xB7\xCE\xBC\xBC\xBD\xBA\xBD\xC3\xB0\xA3\xB0\xA8\xBD\xC3";
static const char ClientProcessTimeOut[] = "\x2A\xC7\xC1\xB7\xCE\xBC\xBC\xBD\xBA\xBD\xC3\xB0\xA3\xC1\xBE\xB7\xE1";
static const char CheckPotionCount[] = "\x2A\xB9\xB0\xBE\xE0\xB0\xB9\xBC\xF6\xC8\xAE\xC0\xCE";
static const char EnableServerExp[] = "\x2A\xBC\xAD\xB9\xF6\xB0\xE6\xC7\xE8\xC4\xA1\xBB\xE7\xBF\xEB";
static const char BlockUserPath[] = "\x2A\xBA\xED\xB7\xB0\xC0\xAF\xC0\xFA\xB0\xE6\xB7\xCE";
static const char DispBillingTime[] = "\x2A\xBB\xE7\xBF\xEB\xBD\xC3\xB0\xA3\xC7\xA5\xBD\xC3";
static const char DisableLogQue[] = "\x2A\xB7\xCE\xB1\xD7\xC5\xA5\xB1\xDD\xC1\xF6";
static const char DisableCheckLogout[] = "\x2A\xB7\xCE\xB1\xD7\xBE\xC6\xBF\xF4\xC8\xAE\xC0\xCE\xB1\xDD\xC1\xF6";
static const char FreeLevel[] = "\x2A\xB9\xAB\xB7\xE1\xB7\xB9\xBA\xA7";
static const char PresentLevel[] = "\x2A\xBC\xB1\xB9\xB0\xB7\xB9\xBA\xA7";
static const char EnablePartyItem[] = "\x2A\xB5\xBF\xB7\xE1\xBE\xC6\xC0\xCC\xC5\xDB\xC3\xE2\xB7\xC2";
static const char CheckPlayField[] = "\x2A\xC7\xCA\xB5\xE5\xC1\xF6\xC7\xFC\xB0\xCB\xBB\xE7";
static const char CheckModelFile[] = "\x2A\xB8\xF0\xB5\xA8\xC6\xC4\xC0\xCF\xB0\xCB\xBB\xE7";
static const char ServerInventory[] = "\x2A\xBC\xAD\xB9\xF6\xC0\xCE\xBA\xA5\xC5\xE4\xB8\xAE";
static const char BackupPath[] = "\x2A\xB9\xE9\xBE\xF7\xC6\xFA\xB4\xF5\xC0\xCC\xB8\xA7";
static const char DisableDecSkillPoint[] = "\x2A\xBD\xBA\xC5\xB3\xC6\xF7\xC0\xCE\xC6\xAE\xB0\xA8\xBC\xD2\xBA\xD2\xB0\xA1";
static const char DisableQuest[] = "\x2A\xC4\xF9\xBD\xBA\xC6\xAE\xB1\xDD\xC1\xF6";
static const char StartField[] = "\x2A\xBD\xC3\xC0\xDB\xC7\xCA\xB5\xE5\xB0\xED\xC1\xA4";
static const char DisableThrowItem[] = "\x2A\xBE\xC6\xC0\xCC\xC5\xDB\xB9\xF6\xB8\xB2\xB1\xDD\xC1\xF6";
static const char CrazyPacket[] = "\x2A\xC5\xA9\xB7\xB9\xC0\xCC\xC1\xF6\xC6\xD0\xC5\xB6";
static const char NoticeUrl[] = "\x2A\xC5\xEB\xBA\xB8\xC1\xD6\xBC\xD2";
static const char RecordAdminCmd[] = "\x2A\xB0\xFC\xB8\xAE\xB8\xED\xB7\xC9\xB1\xE2\xB7\xCF";
}

template <typename Config>
inline ParseResult ApplyRuntimeDirective(const char *command, const char *values, Config &config)
{
	if(!command)
		return ParseResult::NotHandled;

	Config staged = config;
	legacy_config::LineCursor cursor(values);
	char text[256];

	if(Matches(command, runtime_token::DefaultItemUser, "*DEFAULT_ITEM_USER")) { if(!ReadAtoiOrDefault(cursor, 1, staged.StartUser_PresentItem)) return ParseResult::Error; }
	else if(Matches(command, runtime_token::RecordJobChanger, "*RECORD_JOBCHANGER")) staged.Record_JobChanger = 1;
	else if(Matches(command, runtime_token::EnableBlockForever, "*ENABLE_BLOCK_FOREVER")) staged.ForeverBlockMode = 1;
	else if(Matches(command, runtime_token::PermitExp, "*PERMIT_EXP")) { if(!ReadAtoiOrDefault(cursor, 0, staged.Permit_Exp)) return ParseResult::Error; }
	else if(Matches(command, runtime_token::PermitMoney, "*PERMIT_MONEY")) { if(!ReadAtoiOrDefault(cursor, 0, staged.Permit_Money)) return ParseResult::Error; }
	else if(Matches(command, runtime_token::ClientCheckProcessTime, "*CLIENT_CHECK_PROCESS_TIME")) { if(!ReadAtoiOrDefault(cursor, 0, staged.ClientProcessTimeChk)) return ParseResult::Error; }
	else if(Matches(command, runtime_token::ClientProcessTimeOut, "*CLIENT_PROCESS_TIME_OUT")) staged.ClientProcessTimeChkOut = 1;
	else if(Matches(command, runtime_token::CheckPotionCount, "*CHECK_POTION_COUNT")) { if(!ReadAtoiOrDefault(cursor, 0, staged.Dis_PotionError)) return ParseResult::Error; }
	else if(Matches(command, runtime_token::EnableServerExp, "*ENABLE_SERVER_EXP")) { if(!ReadAtoiOrDefault(cursor, 50, staged.ExpFixMode)) return ParseResult::Error; }
	else if(Matches(command, runtime_token::BlockUserPath, "*BLOCK_USER_PATH"))
	{
		if(!ReadValueOrEmpty(cursor, text, sizeof(text)) || !CopyText(staged.szBlockUserPath, text)) return ParseResult::Error;
	}
	else if(Matches(command, runtime_token::DispBillingTime, "*DISP_BILLING_TIME")) { if(!ReadAtoiOrDefault(cursor, 1, staged.DispBillingTime)) return ParseResult::Error; }
	else if(Matches(command, runtime_token::DisableLogQue, "*DISABLE_LOGQUE")) staged.Disable_LogQue = 1;
	else if(Matches(command, runtime_token::DisableCheckLogout, "*DISABLE_CHECK_LOGOUT")) staged.Disable_CheckLogOut = 1;
	else if(Matches(command, runtime_token::FreeLevel, "*FREE_LEVEL"))
	{
		if(!ReadAtoiOrDefault(cursor, 0, staged.FreeLevel)) return ParseResult::Error;
		if(!ReadValueOrEmpty(cursor, text, sizeof(text))) return ParseResult::Error;
		if(text[0] && !CopyText(staged.szFreeGpCode, text)) return ParseResult::Error;
	}
	else if(Matches(command, runtime_token::PresentLevel, "*PRESENT_LEVEL")) { if(!ReadAtoiOrDefault(cursor, 0, staged.PresentLevel)) return ParseResult::Error; }
	else if(Matches(command, runtime_token::EnablePartyItem, "*ENABLE_PARTYITEM")) staged.Enable_PartyItem = 1;
	else if(Matches(command, runtime_token::CheckPlayField, "*CHECK_PLAYFIELD")) staged.CheckPlayField = 1;
	else if(Matches(command, runtime_token::CheckModelFile, "*CHECK_MODELFILE")) staged.CheckModelFile = 1;
	else if(Matches(command, runtime_token::ServerInventory, "*SERVER_INVENTORY")) staged.ServerInventoryMode = 1;
	else if(Matches(command, runtime_token::BackupPath, "*BACKUP_PATH"))
	{
		if(!ReadValueOrEmpty(cursor, text, sizeof(text)) || !CopyText(staged.szBackupPath, text)) return ParseResult::Error;
	}
	else if(Matches(command, runtime_token::DisableDecSkillPoint, "*DISABLE_DEC_SKILLPOINT")) staged.Disable_DecSkillPoint = 1;
	else if(Matches(command, runtime_token::DisableQuest, "*DISABLE_QUEST")) { if(!ReadAtoiOrDefault(cursor, 0, staged.Disable_Quest)) return ParseResult::Error; }
	else if(Matches(command, runtime_token::StartField, "*START_FIELD")) { if(!ReadAtoiIfPresent(cursor, staged.FixedStartField)) return ParseResult::Error; }
	else if(Matches(command, runtime_token::DisableThrowItem, "*DISABLE_THROWITEM")) { if(!ReadAtoiOrDefault(cursor, 1, staged.DisableThrowItem)) return ParseResult::Error; }
	else if(Matches(command, runtime_token::CrazyPacket, "*CRAZY_PACKET")) staged.CrazyPacketMode = 1;
	else if(Matches(command, runtime_token::NoticeUrl, "*NOTICE_URL"))
	{
		if(!ReadValueOrEmpty(cursor, text, sizeof(text)) || !CopyText(staged.szNoticeURL, text)) return ParseResult::Error;
	}
	else if(Matches(command, runtime_token::RecordAdminCmd, "*REC_ADMIN_CMD")) { if(!ReadAtoiOrDefault(cursor, 2, staged.RecordAdminCommand)) return ParseResult::Error; }
	else return ParseResult::NotHandled;

	config = staged;
	return ParseResult::Handled;
}

} // namespace server_config
} // namespace formats
} // namespace summit
