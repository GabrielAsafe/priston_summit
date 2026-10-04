#include "compat/server_build/server_compat_packets.h"
#include <cstring>
#include "systems/network/protocol/server/opcodes.h"
#include "systems/gameplay/quests/server/cQuestHandler.h"
#include "systems/commerce/coin_shop/CoinShop.h"
#include "systems/commerce/coin_shop/CoinShop_T.h"
#include "systems/gameplay/inventory/v2/legacy/LegacyItemCatalogAdapter.h"
#include "systems/gameplay/progression/v2/ProgressionTriggerRuntime.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"
#include "systems/inventory/v2/legacy/LegacyInventoryRuntime.h"
#include "systems/gameplay/quests/v2/legacy/LegacyQuestRepository.h"
#include "systems/gameplay/quests/v2/legacy/LegacyQuestViewQuery.h"
#include "systems/gameplay/quests/v2/legacy/LegacyQuestLifecycleService.h"
#include "systems/network/v2/legacy/LegacyPacketSender.h"

cQUESTHANDLER cQuestHandler;

namespace
{
bool SummitQuestSend(rsPLAYINFO* recipient, const void* data, int size)
{
    if(!recipient || !data || size <= 0)
        return false;
    return gameplay::network::legacy::ServerLegacyPacketSender().Send(
        gameplay::PlayerId(static_cast<std::uint32_t>(recipient->dwObjectSerial)),
        gameplay::network::PacketView(data, static_cast<std::size_t>(size), 0u)).Succeeded();
}

struct ProgressionQuestStateSnapshot
{
    bool readable;
    bool completed;
    bool inProgress;

    ProgressionQuestStateSnapshot() : readable(false), completed(false), inProgress(false) {}
};

ProgressionQuestStateSnapshot CaptureProgressionQuestState(rsPLAYINFO* player, int questId)
{
    ProgressionQuestStateSnapshot snapshot;
    if (!player || questId <= 0 || !player->getName() || !player->getName()[0])
        return snapshot;

    bool completed = false;
    bool inProgress = false;
    const bool completedRead = cQuestHandler.QueryCompletedQuest(player->getName(), questId, &completed);
    const bool progressRead = cQuestHandler.QueryQuestInProgress(player->getName(), questId, &inProgress);
    if (!completedRead || !progressRead)
        return snapshot;

    snapshot.readable = true;
    snapshot.completed = completed;
    snapshot.inProgress = inProgress;
    return snapshot;
}

void NotifyProgressionIfQuestStateChanged(
    rsPLAYINFO* player, int questId, const ProgressionQuestStateSnapshot& before)
{
    if (!before.readable)
        return;

    const ProgressionQuestStateSnapshot after = CaptureProgressionQuestState(player, questId);
    if (!after.readable)
        return;

    if (before.completed != after.completed || before.inProgress != after.inProgress)
        gameplay::progression::NotifyServerProgressionPlayerStateChanged(
            gameplay::player::legacy::ServerLegacyOnlinePlayerRegistry().IdOf(player));
}
}

extern bool StartTier5ArenaQuest(rsPLAYINFO* lpPlayInfo);
extern void StopTier5ArenaQuest(rsPLAYINFO* lpPlayInfo, bool bWarpOut);
void cQUESTHANDLER::GetSpoilerInfo(rsPLAYINFO* lpPlayInfo, int SpoilerNum)
{
	smTRANS_QUEST_INFO info = {};
	info.code = OpCode::OPCODE_SPOILER_INFO;
	info.SpoilerNum = SpoilerNum;
	info.size = sizeof(smTRANS_QUEST_INFO);
	info.QuestCount = -1;
	SummitQuestSend(lpPlayInfo, &info, info.size);

	std::vector<gameplay::quests::QuestSpoilerBatchView> batches;
	if (!gameplay::quests::legacy::ServerLegacyQuestViewQuery().TryGetSpoilerBatches(
		lpPlayInfo->getName(), lpPlayInfo->smCharInfo.Level, SpoilerNum, batches))
		return;

	for (std::vector<gameplay::quests::QuestSpoilerBatchView>::const_iterator batchIt = batches.begin();
		batchIt != batches.end(); ++batchIt)
	{
		smTRANS_QUEST_INFO packet = {};
		packet.code = OpCode::OPCODE_SPOILER_INFO;
		packet.SpoilerNum = SpoilerNum;

		for (std::vector<gameplay::quests::QuestSpoilerEntryView>::const_iterator entryIt = batchIt->entries.begin();
			entryIt != batchIt->entries.end(); ++entryIt)
		{
			const int cnt = packet.QuestCount;
			packet.QuestInfo[cnt].QuestID = static_cast<int>(entryIt->quest.value);
			std::strncpy(packet.QuestInfo[cnt].QuestName, entryIt->name.c_str(),
				sizeof(packet.QuestInfo[cnt].QuestName) - 1);
			packet.QuestInfo[cnt].QuestKind = entryIt->kind;
			packet.QuestInfo[cnt].QuestActive = entryIt->active;
			packet.QuestCount++;

			if (packet.QuestCount == MAX_QUEST_PACKET_INFO)
			{
				packet.size = sizeof(smTRANS_QUEST_INFO);
				SummitQuestSend(lpPlayInfo, &packet, packet.size);
				packet.QuestCount = 0;
				ZeroMemory(packet.QuestInfo, sizeof(packet.QuestInfo));
			}
		}

		packet.size = sizeof(smTRANS_QUEST_INFO);
		SummitQuestSend(lpPlayInfo, &packet, packet.size);
	}
}

#include "core/server/legacy_runtime/onserver.h"


void cQUESTHANDLER::GetQuestInfo(rsPLAYINFO* lpPlayInfo, int QuestID, int QuestKind, int SpoilerNum)
{
	smTRANS_QUEST_FULL TransQuestInfo = {};
	TransQuestInfo.code = OpCode::OPCODE_QUEST_INFO;
	TransQuestInfo.size = sizeof(smTRANS_QUEST_FULL);
	TransQuestInfo.QuestKind = SpoilerNum;

	gameplay::quests::QuestDetailView detail;
	if (lpPlayInfo && lpPlayInfo->getName() && lpPlayInfo->getName()[0] &&
		gameplay::quests::legacy::ServerLegacyQuestViewQuery().TryGetDetail(
			lpPlayInfo->getName(),
			gameplay::QuestId(static_cast<std::uint32_t>(QuestID)),
			QuestKind,
			detail))
	{
		TransQuestInfo.QuestID = static_cast<int>(detail.quest.value);
		lstrcpyn(TransQuestInfo.QuestName, detail.name.c_str(), 128);
		lstrcpyn(TransQuestInfo.QuestTitle, detail.title.c_str(), 128);
		TransQuestInfo.LevelMin = detail.levelMin;
		TransQuestInfo.LevelMax = detail.levelMax;
		TransQuestInfo.PartyFlag = detail.partyFlag;
		lstrcpyn(TransQuestInfo.QuestDesc, detail.description.c_str(), 500);
		TransQuestInfo.QuestType = detail.type;

		for (std::vector<gameplay::quests::QuestObjectiveView>::const_iterator it = detail.objectives.begin();
			 it != detail.objectives.end() && TransQuestInfo.MonsterCount < QUEST_FULL_ENTRY_MAX; ++it)
		{
			const int i = TransQuestInfo.MonsterCount;
			lstrcpyn(TransQuestInfo.MonsterNames[i], it->monsterName.c_str(), 32);
			TransQuestInfo.MonsterCounts[i] = it->required;
			TransQuestInfo.MonsterKilled[i] = it->killed;
			TransQuestInfo.MonsterMaps[i] = it->map;
			++TransQuestInfo.MonsterCount;
		}

		TransQuestInfo.RewardGold = detail.rewardGold;
		TransQuestInfo.RewardExp = detail.rewardExperience;
		TransQuestInfo.RewardCoin = detail.rewardCoin;
		TransQuestInfo.RewardTCoin = detail.rewardTCoin;

		for (std::vector<gameplay::quests::QuestItemRewardView>::const_iterator it = detail.itemRewards.begin();
			 it != detail.itemRewards.end() && TransQuestInfo.RewardCount < QUEST_FULL_ENTRY_MAX; ++it)
		{
			const int i = TransQuestInfo.RewardCount;
			TransQuestInfo.RewardCounts[i] = it->count;

			gameplay::inventory::ItemDefinition itemDefinition;
			if (gameplay::inventory::legacy::ServerLegacyItemCatalogAdapter().TryGetDefinitionByContentKey(
				it->contentKey, itemDefinition) && !itemDefinition.displayName.empty())
			{
				lstrcpyn(TransQuestInfo.RewardNames[i], itemDefinition.displayName.c_str(), 32);
			}
			++TransQuestInfo.RewardCount;
		}
	}

	if (lpPlayInfo && lpPlayInfo->lpsmSock)
		SummitQuestSend(lpPlayInfo, &TransQuestInfo, TransQuestInfo.size);
}


static bool EnsureTier5ArenaObjective(rsPLAYINFO* lpPlayInfo, int QuestType)
{
	if (!lpPlayInfo || !lpPlayInfo->getName() || !lpPlayInfo->getName()[0])
		return false;

	return gameplay::quests::legacy::ServerLegacyQuestRepository().EnsureObjective(
		lpPlayInfo->getName(),
		gameplay::QuestId(static_cast<std::uint32_t>(QUESTID_T5_QUEST_ARENA)),
		9,
		QuestType,
		"Crystal Tower",
		1,
		0,
		32);
}



void cQUESTHANDLER::StartQuest(rsPLAYINFO* lpPlayInfo, int QuestID, int QuestKind, int QuestType, int PartyFlag)
{
	if (!lpPlayInfo)
		return;

	const ProgressionQuestStateSnapshot progressionStateBefore =
		CaptureProgressionQuestState(lpPlayInfo, QuestID);

	// Final Rank 5 arena gate from the EU sequence. Quest 10012 itself has no
	// special C++ handler there; its requirements stay data-driven in QuestDB.
	if (QuestID == QUESTID_T5_QUEST_ARENA)
	{
		if (QuestKind != 9 || lpPlayInfo->smCharInfo.ChangeJob != CHARACTER_RANK4)
			return;

		if (!HasCompletedQuest(lpPlayInfo, QUESTID_T5_A_MATTER_OF_STRENGTH) ||
			!HasCompletedQuest(lpPlayInfo, QUESTID_T5_CRY_ME_A_RIVER) ||
			!HasCompletedQuest(lpPlayInfo, QUESTID_T5_THE_LAST_STAGE))
			return;

		if (HasCompletedQuest(lpPlayInfo, QUESTID_T5_QUEST_ARENA) ||
			IsQuestInProgress(lpPlayInfo, QUESTID_T5_QUEST_ARENA))
			return;
	}

	if ((QuestKind == 21 || QuestKind == 7 || QuestKind == 9) &&
		(QuestKind != 9 || !HasCompletedQuest(lpPlayInfo, QuestID)))
	{
		gameplay::quests::legacy::ServerLegacyQuestRepository().StartProgress(
			lpPlayInfo->getName(),
			gameplay::QuestId(static_cast<std::uint32_t>(QuestID)),
			QuestKind,
			QuestType,
			PartyFlag);
	}

	if (QuestID == QUESTID_T5_QUEST_ARENA && IsQuestInProgress(lpPlayInfo, QUESTID_T5_QUEST_ARENA))
	{
		if (!EnsureTier5ArenaObjective(lpPlayInfo, QuestType) || !StartTier5ArenaQuest(lpPlayInfo))
		{
			CancelQuest(lpPlayInfo, QuestID, QuestKind, QuestType);
			return;
		}
	}

	GetSpoilerInfo(lpPlayInfo, QuestKind);
	if (QuestKind != 10)
		GetSpoilerInfo(lpPlayInfo, 10);

	NotifyProgressionIfQuestStateChanged(lpPlayInfo, QuestID, progressionStateBefore);
}

void cQUESTHANDLER::CancelQuest(rsPLAYINFO* lpPlayInfo, int QuestID, int QuestKind, int QuestType)
{
	if (!lpPlayInfo)
		return;

	const ProgressionQuestStateSnapshot progressionStateBefore =
		CaptureProgressionQuestState(lpPlayInfo, QuestID);

	if (QuestID == QUESTID_T5_QUEST_ARENA)
		StopTier5ArenaQuest(lpPlayInfo, true);

	gameplay::quests::legacy::ServerLegacyQuestRepository().CancelProgressVariant(
		lpPlayInfo->getName(),
		gameplay::QuestId(static_cast<std::uint32_t>(QuestID)),
		QuestKind,
		QuestType);

	GetSpoilerInfo(lpPlayInfo, QuestKind);
	if (QuestKind != 10)
		GetSpoilerInfo(lpPlayInfo, 10);

	NotifyProgressionIfQuestStateChanged(lpPlayInfo, QuestID, progressionStateBefore);
}


bool cQUESTHANDLER::CancelQuestProgressSilent(rsPLAYINFO* lpPlayInfo, int QuestID)
{
	if (!lpPlayInfo || QuestID <= 0 || !lpPlayInfo->getName() || !lpPlayInfo->getName()[0])
		return false;

	return gameplay::quests::legacy::ServerLegacyQuestRepository().CancelProgress(
		lpPlayInfo->getName(), gameplay::QuestId(static_cast<std::uint32_t>(QuestID)));
}


extern int CreatePerfCommandItem(rsPLAYINFO* lpPlayInfo, rsPLAYINFO* lpPlayInfo2, char* szItem);

extern int GetPostBoxFile(const char* szID, char* szFileName);
extern int rsSavePostBox(rsPLAYINFO* lpPlayInfo);
extern int rsLoadPostBox(rsPLAYINFO* lpPlayInfo);

void SendPostBox(char* szID, char* szItemCode, int spec)
{
	char szFileName[255];
	char strBuff[255];
	DWORD dwAcess = 0;
	GetPostBoxFile(szID, szFileName);
	HANDLE hFile = CreateFile(szFileName, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return;
	}
	else
	{
		int FileLength = GetFileSize(hFile, NULL);
		SetFilePointer(hFile, FileLength, NULL, FILE_BEGIN);

		sprintf_s(strBuff, "* %s %d \"Shop\" 0 \"\"\r\n", szItemCode, spec);
		WriteFile(hFile, strBuff, lstrlen(strBuff), &dwAcess, NULL);
		CloseHandle(hFile);
	}
}


bool cQUESTHANDLER::HasCompletedQuest(rsPLAYINFO* lpPlayInfo, int QuestID, int QuestKind, int QuestType)
{
	if (!lpPlayInfo || QuestID <= 0 || !lpPlayInfo->getName() || !lpPlayInfo->getName()[0])
		return false;

	bool completed = false;
	return gameplay::quests::legacy::ServerLegacyQuestRepository().TryIsCompletedVariant(
		lpPlayInfo->getName(),
		gameplay::QuestId(static_cast<std::uint32_t>(QuestID)),
		QuestKind,
		QuestType,
		completed) && completed;
}


bool cQUESTHANDLER::HasCompletedQuest(rsPLAYINFO* lpPlayInfo, int QuestID)
{
	if (!lpPlayInfo || QuestID <= 0 || !lpPlayInfo->getName() || !lpPlayInfo->getName()[0])
		return false;

	bool completed = false;
	return gameplay::quests::legacy::ServerLegacyQuestRepository().TryIsCompleted(
		lpPlayInfo->getName(),
		gameplay::QuestId(static_cast<std::uint32_t>(QuestID)),
		completed) && completed;
}



bool cQUESTHANDLER::QueryCompletedQuest(const char* pszPlayerName, int QuestID, bool* pbCompleted)
{
	if (!pbCompleted)
		return false;

	*pbCompleted = false;
	if (!pszPlayerName || !pszPlayerName[0] || QuestID <= 0)
		return false;

	return gameplay::quests::legacy::ServerLegacyQuestRepository().TryIsCompleted(
		pszPlayerName, gameplay::QuestId(static_cast<std::uint32_t>(QuestID)), *pbCompleted);
}


bool cQUESTHANDLER::HasCompletedQuest(const char* pszPlayerName, int QuestID)
{
	bool completed = false;
	return QueryCompletedQuest(pszPlayerName, QuestID, &completed) && completed;
}


bool cQUESTHANDLER::HasCompletedRank5Quest(rsPLAYINFO* lpPlayInfo)
{
	return HasCompletedQuest(lpPlayInfo, QUESTID_T5_QUEST_ARENA);
}


bool cQUESTHANDLER::HasCompletedRank5Quest(const char* pszPlayerName)
{
	return HasCompletedQuest(pszPlayerName, QUESTID_T5_QUEST_ARENA);
}



bool cQUESTHANDLER::QueryQuestInProgress(const char* pszPlayerName, int QuestID, bool* pbInProgress)
{
	if (!pbInProgress)
		return false;

	*pbInProgress = false;
	if (!pszPlayerName || !pszPlayerName[0] || QuestID <= 0)
		return false;

	return gameplay::quests::legacy::ServerLegacyQuestRepository().TryIsInProgress(
		pszPlayerName, gameplay::QuestId(static_cast<std::uint32_t>(QuestID)), *pbInProgress);
}


bool cQUESTHANDLER::IsQuestInProgress(rsPLAYINFO* lpPlayInfo, int QuestID)
{
	if (!lpPlayInfo || QuestID <= 0 || !lpPlayInfo->getName() || !lpPlayInfo->getName()[0])
		return false;

	bool inProgress = false;
	return gameplay::quests::legacy::ServerLegacyQuestRepository().TryIsInProgress(
		lpPlayInfo->getName(), gameplay::QuestId(static_cast<std::uint32_t>(QuestID)), inProgress) &&
		inProgress;
}


bool cQUESTHANDLER::HasPendingQuestObjectives(rsPLAYINFO* lpPlayInfo, int QuestID)
{
	if (!lpPlayInfo || QuestID <= 0 || !lpPlayInfo->getName() || !lpPlayInfo->getName()[0])
		return false;

	bool pending = false;
	return gameplay::quests::legacy::ServerLegacyQuestRepository().TryHasPendingObjectives(
		lpPlayInfo->getName(), gameplay::QuestId(static_cast<std::uint32_t>(QuestID)), pending) &&
		pending;
}


bool cQUESTHANDLER::IsQuestObjectivePending(rsPLAYINFO* lpPlayInfo, int QuestID, const char* pszMonsterName)
{
	if (!lpPlayInfo || QuestID <= 0 || !pszMonsterName || !pszMonsterName[0] ||
		!lpPlayInfo->getName() || !lpPlayInfo->getName()[0])
		return false;

	bool pending = false;
	return gameplay::quests::legacy::ServerLegacyQuestRepository().TryIsObjectivePending(
		lpPlayInfo->getName(), gameplay::QuestId(static_cast<std::uint32_t>(QuestID)),
		pszMonsterName, pending) && pending;
}


bool cQUESTHANDLER::CompleteQuestObjective(rsPLAYINFO* lpPlayInfo, int QuestID, const char* pszMonsterName)
{
	if (!lpPlayInfo || QuestID <= 0 || !pszMonsterName || !pszMonsterName[0] ||
		!lpPlayInfo->getName() || !lpPlayInfo->getName()[0])
		return false;

	return gameplay::quests::legacy::ServerLegacyQuestRepository().CompleteObjective(
		lpPlayInfo->getName(), gameplay::QuestId(static_cast<std::uint32_t>(QuestID)), pszMonsterName);
}


bool cQUESTHANDLER::CompleteQuestObjectives(rsPLAYINFO* lpPlayInfo, int QuestID)
{
	if (!lpPlayInfo || QuestID <= 0 || !lpPlayInfo->getName() || !lpPlayInfo->getName()[0])
		return false;

	return gameplay::quests::legacy::ServerLegacyQuestRepository().CompleteObjectives(
		lpPlayInfo->getName(), gameplay::QuestId(static_cast<std::uint32_t>(QuestID)));
}


namespace
{
bool GrantTier5QuestItem(rsPLAYINFO* player, DWORD itemCode)
{
    if(!player || !player->dwObjectSerial)
        return false;

    return gameplay::inventory::legacy::ServerLegacyInventoryRuntime().Grant(
        gameplay::inventory::ItemGrant(
            gameplay::PlayerId(static_cast<std::uint32_t>(player->dwObjectSerial)),
            gameplay::ItemId(static_cast<std::uint32_t>(itemCode)),
            1u)).Granted();
}
}

bool cQUESTHANDLER::HandleTier5MonsterKill(rsPLAYINFO* lpKiller, smCHAR* lpChar, rsPLAYINFO** lpDamageUsers, int DamageUserCount)
{
	if (!lpKiller || !lpChar)
		return false;

	const char* monsterName = lpChar->smCharInfo.szName;
	if (!monsterName || !monsterName[0])
		return false;

	if (lstrcmpi(monsterName, "Bloody Knight") == 0)
	{
		if (!IsQuestInProgress(lpKiller, QUESTID_T5_A_MATTER_OF_STRENGTH) ||
			!IsQuestObjectivePending(lpKiller, QUESTID_T5_A_MATTER_OF_STRENGTH, monsterName))
			return false;

		if ((rand() % 100) >= 10)
			return false;

		if (!GrantTier5QuestItem(lpKiller, sinQT134))
			return false;

		return CompleteQuestObjective(lpKiller, QUESTID_T5_A_MATTER_OF_STRENGTH, monsterName);
	}

	if (lstrcmpi(monsterName, "Prince") != 0)
		return false;

	bool granted = false;
	bool killerSeen = false;
	const int maxDistanceSq = 24 * 24;

	for (int i = 0; lpDamageUsers && i < DamageUserCount; i++)
	{
		rsPLAYINFO* player = lpDamageUsers[i];
		if (!player || !player->lpsmSock || !player->dwObjectSerial)
			continue;

		if (player == lpKiller)
			killerSeen = true;

		if (player->Position.Area != lpKiller->Position.Area)
			continue;

		int dx = (player->Position.x - lpChar->pX) >> FLOATNS;
		int dz = (player->Position.z - lpChar->pZ) >> FLOATNS;
		if ((dx * dx + dz * dz) > maxDistanceSq)
			continue;

		if (!IsQuestInProgress(player, QUESTID_T5_A_MATTER_OF_STRENGTH) ||
			!IsQuestObjectivePending(player, QUESTID_T5_A_MATTER_OF_STRENGTH, monsterName))
			continue;

		if (GrantTier5QuestItem(player, sinQT135) &&
			CompleteQuestObjective(player, QUESTID_T5_A_MATTER_OF_STRENGTH, monsterName))
		{
			granted = true;
		}
	}

	if (!killerSeen && lpKiller->lpsmSock && lpKiller->dwObjectSerial)
	{
		int dx = (lpKiller->Position.x - lpChar->pX) >> FLOATNS;
		int dz = (lpKiller->Position.z - lpChar->pZ) >> FLOATNS;
		if ((dx * dx + dz * dz) <= maxDistanceSq &&
			IsQuestInProgress(lpKiller, QUESTID_T5_A_MATTER_OF_STRENGTH) &&
			IsQuestObjectivePending(lpKiller, QUESTID_T5_A_MATTER_OF_STRENGTH, monsterName) &&
			GrantTier5QuestItem(lpKiller, sinQT135) &&
			CompleteQuestObjective(lpKiller, QUESTID_T5_A_MATTER_OF_STRENGTH, monsterName))
		{
			granted = true;
		}
	}

	return granted;
}


void cQUESTHANDLER::GetReward(rsPLAYINFO* lpPlayInfo, int QuestID, int QuestKind, int QuestType)
{
	if (!lpPlayInfo)
		return;

	const ProgressionQuestStateSnapshot progressionStateBefore =
		CaptureProgressionQuestState(lpPlayInfo, QuestID);

	gameplay::quests::QuestRewardPayload reward;
	const bool claimed = gameplay::quests::legacy::ServerLegacyQuestRepository().ClaimReward(
		lpPlayInfo->getName() ? lpPlayInfo->getName() : "",
		gameplay::QuestId(static_cast<std::uint32_t>(QuestID)),
		QuestKind,
		QuestType,
		reward);

	if (claimed)
	{
		char playerID[32] = {};
		wsprintf(playerID, "%s", lpPlayInfo->getAccount());

		if (reward.experience)
		{
			smTRANS_COMMAND_QWORD smTransCommandQ = {};

			smTransCommandQ.code = OpCode::OPCODE_ADDEXP;
			smTransCommandQ.size = sizeof(smTRANS_COMMAND_QWORD);
			smTransCommandQ.SParam = reward.experience;

			SummitQuestSend(lpPlayInfo, &smTransCommandQ, smTransCommandQ.size);
		}

		if (reward.gold)
		{
			char goldCode[] = "GG101";
			SendPostBox(playerID, goldCode, reward.gold);
		}

		if (reward.coin)
			cCoinShop.AddPlayerCoin(lpPlayInfo, reward.coin);

		if (reward.tCoin)
			cCoinShop_T.AddPlayerCoin(lpPlayInfo, reward.tCoin);

		if (!reward.items.empty())
		{
			if (lpPlayInfo->lpPostBoxItem)
				rsSavePostBox(lpPlayInfo);

			for (std::vector<gameplay::quests::QuestItemRewardRecord>::const_iterator it = reward.items.begin();
				 it != reward.items.end(); ++it)
			{
				char itemCode[32] = {};
				std::strncpy(itemCode, it->itemCode.c_str(), sizeof(itemCode) - 1);

				if (toupper(itemCode[0]) == 'P' ||
					(toupper(itemCode[0]) == 'G' && toupper(itemCode[1]) == 'G'))
				{
					SendPostBox(playerID, itemCode, it->count);
				}
				else
				{
					for (int j = 0; j < it->count; ++j)
						SendPostBox(playerID, itemCode, lpPlayInfo->smCharInfo.JOB_CODE);
				}
			}

			if (lpPlayInfo->lpPostBoxItem)
			{
				delete lpPlayInfo->lpPostBoxItem;
				lpPlayInfo->lpPostBoxItem = NULL;
			}
			rsLoadPostBox(lpPlayInfo);

			TRANS_CHATMESSAGE Message = {};
			Message.code = OpCode::OPCODE_MESSAGEBOX;
			wsprintf(Message.szMessage, "Itens enviados ao Distribuidor.");
			Message.size = lstrlen(Message.szMessage) + 32;
			SummitQuestSend(lpPlayInfo, &Message, Message.size);
		}
	}

	GetSpoilerInfo(lpPlayInfo, QuestKind);
	if (QuestKind != 10)
		GetSpoilerInfo(lpPlayInfo, 10);
	if (QuestKind != 11)
		GetSpoilerInfo(lpPlayInfo, 11);

	NotifyProgressionIfQuestStateChanged(lpPlayInfo, QuestID, progressionStateBefore);
}

bool cQUESTHANDLER::inQuest(rsPLAYINFO* lpPlayInfo, smCHAR* lpChar)
{
	if (!lpPlayInfo || !lpChar)
		return false;

	bool pending = false;
	return gameplay::quests::legacy::ServerLegacyQuestRepository().HasPartyObjectiveTarget(
		lpPlayInfo->getName() ? lpPlayInfo->getName() : "",
		lpChar->smCharInfo.szName,
		lpPlayInfo->Position.Area,
		pending) && pending;
}


void cQUESTHANDLER::VerifyLevel(rsPLAYINFO* lpPlayInfo)
{
	if (!lpPlayInfo)
		return;

	gameplay::quests::legacy::ServerLegacyQuestLifecycleService().VerifyPlayerLevel(
		gameplay::player::legacy::ServerLegacyOnlinePlayerRegistry().IdOf(lpPlayInfo));
}



bool cQUESTHANDLER::KillMonster(rsPLAYINFO* lpPlayInfo, smCHAR* lpChar, int PartyFlag)
{
	if (!lpPlayInfo || !lpChar || !lpPlayInfo->getName() || !lpPlayInfo->getName()[0] ||
		!lpChar->smCharInfo.szName[0])
		return false;

	std::vector<gameplay::quests::QuestKillProgressRecord> updatedQuests;
	const bool success = gameplay::quests::legacy::ServerLegacyQuestRepository().RecordMonsterKill(
		lpPlayInfo->getName(),
		lpChar->smCharInfo.szName,
		lpPlayInfo->Position.Area,
		PartyFlag != 0,
		updatedQuests);

	for (std::vector<gameplay::quests::QuestKillProgressRecord>::const_iterator it = updatedQuests.begin();
		it != updatedQuests.end(); ++it)
	{
		if (lpPlayInfo->lpsmSock)
		{
			TRANS_CHATMESSAGE msg = {};
			msg.code = OpCode::OPCODE_WHISPERMESSAGE;
			wsprintf(
				msg.szMessage,
				"> %s killed %d/%d",
				lpChar->smCharInfo.szName,
				it->killed,
				it->count
			);
			msg.size = 32 + lstrlen(msg.szMessage);
			msg.dwIP = 9;
			msg.dwObjectSerial = 0;
			SummitQuestSend(lpPlayInfo, &msg, msg.size);
		}

		GetQuestInfo(
			lpPlayInfo,
			static_cast<int>(it->quest.value),
			it->kind,
			it->kind
		);
	}

	return success;
}



bool cQUESTHANDLER::updateKilledMonsterOfPartyMember(rsPLAYINFO* killer, smCHAR* killed, rsPLAYINFO* partyMember)
{
	bool result = false;

	// Check if pointers are valid. (avoiding null pointer exception)
	if (killer && killed && partyMember)
	{
		if (killer->Position.Area == partyMember->Position.Area)
		{
			result = KillMonster(partyMember, killed, true);
		}
	}

	return result;
}


void cQUESTHANDLER::DebugQuest(rsPLAYINFO* player, int QuestID)
{
	if (!player || QuestID <= 0 || !player->getName() || !player->getName()[0])
		return;

	gameplay::quests::legacy::ServerLegacyQuestRepository().CancelProgress(
		player->getName(), gameplay::QuestId(static_cast<std::uint32_t>(QuestID)));
}

