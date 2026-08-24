#ifndef QUEST_HANDLER_H
#define QUEST_HANDLER_H

class rsPLAYINFO;
class smCHAR;

class cQUESTHANDLER
{
public:
	void GetSpoilerInfo(rsPLAYINFO* lpPlayInfo, int SpoilerNum);
	void GetQuestInfo(rsPLAYINFO* lpPlayInfo, int QuestID, int QuestKind, int SpoilerNum);

	void StartQuest(rsPLAYINFO* lpPlayInfo, int QuestID, int QuestKind, int QuestType, int PartyFlag);
	void CancelQuest(rsPLAYINFO* lpPlayInfo, int QuestID, int QuestKind, int QuestType);
	void GetReward(rsPLAYINFO* lpPlayInfo, int QuestID, int QuestKind, int QuestType);

	void VerifyLevel(rsPLAYINFO* lpPlayInfo);

	struct sPROGRESS_INFO
	{
		int QuestID;
		int QuestKind;
		int QuestType;
	};

	bool inQuest(rsPLAYINFO* lpPlayInfo, smCHAR* lpChar);

	bool KillMonster(rsPLAYINFO* lpPlayInfo, smCHAR* lpChar, int PartyFlag);
	bool updateKilledMonsterOfPartyMember(rsPLAYINFO* killer, smCHAR* killed, rsPLAYINFO* partyMember);

	void DebugQuest(rsPLAYINFO* player, int QuestID);

};

extern cQUESTHANDLER cQuestHandler;

#endif // QUEST_HANDLER_H


