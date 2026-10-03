-- Fishing V2 starter catalog. Fisher remains a V2 profession identity; no legacy
-- JOB_CODE is assigned or inferred here. World node coordinates remain data-owned
-- and are placed through the same persistent node boundary as other gathering.
INSERT OR IGNORE INTO ServerDB__GatheringResourceDefinition
(ResourceId,ResourceKey,Kind,RequiredProfession,RequiredLevel,MinimumYield,MaximumYield,Enabled)
VALUES
(4001,'river_trout',1,6,0,1,2,1),
(4002,'silver_carp',1,6,2,1,2,1),
(4003,'deepwater_eel',1,6,5,1,1,1);
