/*
  QuestDB runtime evidence export v1.
  Run against the SAME QuestDB used by the target GameServer. Export every result
  set losslessly (UTF-8 where possible); do not edit IDs/names to match the Codex.
*/
SET NOCOUNT ON;

SELECT 'RepeatedQuest' AS [TableName], QuestID, QuestName, QuestTitle, LevelMin, LevelMax, PartyFlag, QuestDesc, QuestType
FROM RepeatedQuest ORDER BY QuestID, QuestType;
SELECT 'DailyQuest' AS [TableName], QuestID, QuestName, QuestTitle, LevelMin, LevelMax, PartyFlag, QuestDesc, QuestType
FROM DailyQuest ORDER BY QuestID, QuestType;
SELECT 'SingleQuest' AS [TableName], QuestID, QuestName, QuestTitle, LevelMin, LevelMax, PartyFlag, QuestDesc, QuestType
FROM SingleQuest ORDER BY QuestID, QuestType;

SELECT QuestID, QuestKind, MonsterName, Quantity, QuestType, QuestMap
FROM QuestMonsters ORDER BY QuestKind, QuestID, QuestType, MonsterName;
SELECT QuestID, QuestKind, Gold, Exp, QuestType, Coin, TCoin
FROM Rewards ORDER BY QuestKind, QuestID, QuestType;
SELECT QuestID, QuestKind, ItemCode, ItemCount, QuestType
FROM ItemRewards ORDER BY QuestKind, QuestID, QuestType, ItemCode;
