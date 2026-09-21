-- TAKEOFF final hardening: durable per-node StoryState history.
--
-- StoryProgress stores the current node. StoryNodeState stores historical node
-- state so ConditionType::StoryNodeState remains truthful after a transition.

CREATE TABLE IF NOT EXISTS "ServerDB__StoryNodeState"
(
    "PlayerKey" TEXT NOT NULL,
    "StoryId" INTEGER NOT NULL,
    "NodeId" INTEGER NOT NULL,
    "State" INTEGER NOT NULL CHECK ("State" IN (0, 1, 2)),
    "UpdatedAt" TEXT NOT NULL DEFAULT (datetime('now','localtime')),
    PRIMARY KEY ("PlayerKey", "StoryId", "NodeId")
);

CREATE INDEX IF NOT EXISTS "ServerDB__StoryNodeState_ByStory"
    ON "ServerDB__StoryNodeState" ("PlayerKey", "StoryId", "State");
