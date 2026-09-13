USE [QuestDB];
GO

IF NOT EXISTS (
    SELECT 1
    FROM dbo.QuestMonsters
    WHERE QuestID = 16
      AND QuestKind = 7
      AND MonsterName = 'Hopi'
      AND QuestType = 16
      AND QuestMap = 2
)
BEGIN
    INSERT INTO dbo.QuestMonsters
        (QuestID, QuestKind, MonsterName, Quantity, QuestType, QuestMap)
    VALUES
        (16, 7, 'Hopi', 5, 16, 2);
END;
GO
