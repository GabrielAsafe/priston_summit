// AMBER adapter extracted from frozen OnServer chat dispatch.
// Keeps player-facing compatibility commands while delegating authority to V2 services.

#include <cstdio>
#include <cstring>
#include <memory>
#include "systems/persistence/database/core/Database.h"
#include "systems/gameplay/covenants/v2/CovenantChatCommand.h"
#include "systems/gameplay/mining/v2/MiningRuntime.h"
#include "systems/gameplay/mining/v2/MiningInteractionRuntime.h"
#include "systems/gameplay/gathering/v2/GatheringRuntime.h"
#include "systems/gameplay/fishing/v2/FishingRuntime.h"
#include "systems/gameplay/crafting/v2/SQLiteProfessionCraftRuntime378.h"
#include "systems/gameplay/crafting/v2/CraftJsonPlan425.h" // SUMMIT425A
#include "gameplay/crafting/v2/CraftServerApi446.h" // SUMMIT446A
#include "gameplay/crafting/v2/CraftLegacyLiveApi456.h" // SUMMIT456L
#include "systems/gameplay/crafting/v2/CraftJsonOnlyDispatch432.h" // SUMMIT432A
#include "systems/gameplay/crafting/v2/PhysicalCraftReadiness423.h" // SUMMIT423A
#include "systems/gameplay/fishing/v2/FishingMinigame378.h"

static const char* SummitModernHarvestStatusText(gameplay::gathering::HarvestStatus status)
{
    using gameplay::gathering::HarvestStatus;
    switch(status)
    {
    case HarvestStatus::Success: return "success";
    case HarvestStatus::InvalidPlayer: return "invalid player";
    case HarvestStatus::UnknownNode: return "unknown node";
    case HarvestStatus::UnknownResource: return "unknown resource";
    case HarvestStatus::WrongArea: return "wrong area";
    case HarvestStatus::TooFar: return "too far from node";
    case HarvestStatus::WrongProfession: return "wrong profession";
    case HarvestStatus::ProfessionLevelTooLow: return "profession level too low";
    case HarvestStatus::Depleted: return "node depleted / respawning";
    case HarvestStatus::PersistenceFailed: return "resource persistence failed; retry";
    }
    return "unknown harvest error";
}

static int RouteModernChatIngress(rsPLAYINFO* lpPlayInfo, char* szChatCommand)
{
    if(!lpPlayInfo || !szChatCommand)
        return FALSE;

    char* lpBuff = 0;
    char szCmdBuff[256] = {0};
    char szTempBuff[256] = {0};

    // SUMMIT361A: GM-only spawn plan authoring, using the player's precise
    // in-game coordinates. Plans remain disabled: no live monsters are created.
    // A future V2 widget may invoke this same authenticated server boundary.
    if (rsCompString("/spawnplan", szChatCommand))
    {
        if (lpPlayInfo->getSecurity() < AccountTypes::SEC_MODERATOR)
        {
            SendQuickMessage(lpPlayInfo, "Spawn plans require Moderator privileges.");
            return TRUE;
        }
        char sub[32]={0},key[65]={0},density[24]={0};
        const char* args=szChatCommand+rsCompString("/spawnplan",szChatCommand);
        if(sscanf(args,"%31s %64s %23s",sub,key,density)!=3)
        {
            SendQuickMessage(lpPlayInfo,"Usage: /spawnplan add <monster_key> <low|medium|high>");
            return TRUE;
        }
        if(lstrcmpi(sub,"add")!=0 || !key[0] || !density[0])
        {
            SendQuickMessage(lpPlayInfo,"Usage: /spawnplan add <monster_key> <low|medium|high>");
            return TRUE;
        }
        bool safeKey=true;
        for(const char* p=key;*p;++p)
            if(!((*p>='A'&&*p<='Z')||(*p>='a'&&*p<='z')||
                 (*p>='0'&&*p<='9')||*p=='_'||*p=='-')) safeKey=false;
        if(!safeKey)
        {
            SendQuickMessage(lpPlayInfo,"Invalid spawn content key.");
            return TRUE;
        }
        int maxAlive=0;
        if(lstrcmpi(density,"low")==0) maxAlive=2;
        else if(lstrcmpi(density,"medium")==0) maxAlive=6;
        else if(lstrcmpi(density,"high")==0) maxAlive=12;
        if(!maxAlive)
        {
            SendQuickMessage(lpPlayInfo,"Density must be low, medium or high.");
            return TRUE;
        }
        std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_ServerDB);
        if(!db || !db->Open())
        {
            SendQuickMessage(lpPlayInfo,"Spawn plan DB unavailable.");return TRUE;
        }
        char safeContent[65]={0};
        char safeDensity[24]={0};
        strcpy_s(safeContent,sizeof(safeContent),key);
        strcpy_s(safeDensity,sizeof(safeDensity),density);
        int map=lpPlayInfo->Position.Area;
        int x=lpPlayInfo->Position.x>>FLOATNS;
        int y=lpPlayInfo->Position.y>>FLOATNS;
        int z=lpPlayInfo->Position.z>>FLOATNS;
        int radius=160,respawn=30,enabled=0;
        const bool ok=db->Prepare(
            "INSERT INTO SpawnPlan361(MapId,X,Y,Z,Kind,ContentKey,Density,MaxAlive,Radius,RespawnSeconds,Enabled) "
            "VALUES(?,?,?,?, 'monster',?,?,?,?,?,?)") &&
            db->BindInputParameter(&map,1,PARAMTYPE_Integer) &&
            db->BindInputParameter(&x,2,PARAMTYPE_Integer) &&
            db->BindInputParameter(&y,3,PARAMTYPE_Integer) &&
            db->BindInputParameter(&z,4,PARAMTYPE_Integer) &&
            db->BindInputParameter(safeContent,5,PARAMTYPE_String) &&
            db->BindInputParameter(safeDensity,6,PARAMTYPE_String) &&
            db->BindInputParameter(&maxAlive,7,PARAMTYPE_Integer) &&
            db->BindInputParameter(&radius,8,PARAMTYPE_Integer) &&
            db->BindInputParameter(&respawn,9,PARAMTYPE_Integer) &&
            db->BindInputParameter(&enabled,10,PARAMTYPE_Integer) && db->Execute(false);
        db->Close();
        SendQuickMessage(lpPlayInfo,ok ?
            "Spawn point saved (inactive): awaiting V2 publish/validation." :
            "Could not save spawn plan; check migration 048.");
        return TRUE;
    }

    // 372-378: verified character session, never a client-chosen identity.
    if(rsCompString("/professions",szChatCommand))
    {
        const char* player=lpPlayInfo->getName();
        if(!player || !player[0]) { SendQuickMessage(lpPlayInfo,"Invalid session."); return TRUE; }
        static const char* labels[]={"Arcanist","Blacksmith","Lumberjack","Miner","Artisan","Fisher","Hunter"};
        for(int i=1;i<=7;++i)
        {
            gameplay::professions::ProfessionProgress progress;
            const bool found=gameplay::professions::ServerProfessionService().GetProgress(
                player,static_cast<gameplay::professions::ProfessionType>(i),progress);
            sprintf_s(szTempBuff,sizeof(szTempBuff),"%s: level %d, XP %llu",
                labels[i-1],found?progress.level:0,
                (unsigned long long)(found?progress.experience:0));
            SendQuickMessage(lpPlayInfo,szTempBuff);
        }
        return TRUE;
    }
    if(rsCompString("/craft",szChatCommand))
    {
        char action[24]={0},recipe[100]={0},requestId[49]={0};
        const int len=rsCompString("/craft",szChatCommand);
        const char* args=szChatCommand+len;
        sscanf(args,"%23s %99s %48s",action,recipe,requestId);
        if(lstrcmpi(action,"sync")==0)
        {
            // SUMMIT410A: no resource/XP chat-streaming protocol. The client
            // displays its actual backpack; opening Crafting sends nothing.
            SendQuickMessage(lpPlayInfo,"Crafting: sync legado desativado; usa a mochila.");
            return TRUE;
        }
        // SUMMIT432A_JSON_AUTHORITY: runtime recipes come only from JSON 424A/425A.
        // Do NOT call ProfessionRecipe378/Ingredient378 or debit a SQL balance.
        if(lstrcmpi(action,"status")==0) {
            const std::string status=summit_craft_432::Status();
            SendQuickMessage(lpPlayInfo,status.c_str());
            const std::string status444="JSON 424A: "+std::to_string(summit_craft_444::kCount)+
                " receitas; "+std::to_string(summit_craft_444::ConfiguredCount())+
                " configuradas; "+std::to_string(summit_craft_444::EnabledCount())+
                " habilitadas; DAT WA103=TRANSACTIONAL, restantes=BLOCKED.";
            SendQuickMessage(lpPlayInfo,status444.c_str());return TRUE;
        }
        if(!action[0] || lstrcmpi(action,"list")==0) {
            const auto lines=summit_craft_432::List(recipe);
            for(const auto& line:lines)SendQuickMessage(lpPlayInfo,line.c_str());
            return TRUE;
        }
        // SUMMIT425A: universal JSON recipe preview, no DAT mutation.
        if(lstrcmpi(action,"check")==0 || lstrcmpi(action,"plan")==0) {
            const std::vector<std::string> lines=summit_craft_425::Plan(lpPlayInfo,recipe);
            for(const auto& line:lines)SendQuickMessage(lpPlayInfo,line.c_str());
            return TRUE;
        }
        if(lstrcmpi(action,"make")!=0) {
            SendQuickMessage(lpPlayInfo,"/craft list [CODE] | /craft plan CODE | /craft make CODE REQUEST");
            return TRUE;
        }
        // SUMMIT456L: online legacy item flow. The old direct-DAT branch is
        // retained for login-journal recovery only; it is NOT called here.
        const auto result456=summit456l::Begin(lpPlayInfo,recipe,requestId);
        const bool started456=result456==summit456l::BeginResult::Queued;
        const bool replay456=result456==summit456l::BeginResult::Replay;
        if(!started456 && !replay456 && summit_craft_432::RequestId(requestId)) {
            const std::string ack=std::string("@S407|")+requestId+"|R|ERR";
            SendQuickMessage(lpPlayInfo,ack.c_str());
        }
        SendQuickMessage(lpPlayInfo,started456 ?
            "Crafting: receita validada; aguarda confirmacao da mochila e save normal." :
            replay456 ? "Crafting: pedido repetido, sem novo consumo ou criacao." :
            result456==summit456l::BeginResult::Busy ?
                "Crafting anterior ainda pendente; sem consumo adicional." :
            "Crafting recusado; materiais nao consumidos nesta tentativa.");

        return TRUE;
    }

    // Sea minigame shares the same server-owned node/profession contract as /fish.
    if(rsCompString("/fish",szChatCommand))
    {
        const int length=rsCompString("/fish",szChatCommand);
        char action[32]={0};sscanf(szChatCommand+length,"%31s",action);
        if(!lstrcmpi(action,"cast")||!lstrcmpi(action,"hook")||
           !lstrcmpi(action,"reel")||!lstrcmpi(action,"collect"))
        {
            const int x=lpPlayInfo->Position.x>>FLOATNS;
            const int y=lpPlayInfo->Position.y>>FLOATNS;
            const int z=lpPlayInfo->Position.z>>FLOATNS;
            gameplay::gathering::GatheringNodeDefinition node;
            gameplay::gathering::ResourceDefinition resource;
            const bool found=gameplay::gathering::FindNearestGatheringNode(
                lpPlayInfo->Position.Area,x,y,z,gameplay::gathering::GatheringKind::Fishing,
                true,node,resource);
            const long long dx=found?(long long)x-node.x:0;
            const long long dy=found?(long long)y-node.y:0;
            const long long dz=found?(long long)z-node.z:0;
            const long long radius=found?node.interactionRadius:0;
            const bool withinFishingRadius=found && dx*dx+dy*dy+dz*dz<=radius*radius;
            const summit_fishing_378::Reply reply=summit_fishing_378::Advance(
                lpPlayInfo->getName()?lpPlayInfo->getName():"",action,
                lpPlayInfo->Position.Area,(std::uint64_t)dwPlayServTime,
                found?node.nodeId:0u,withinFishingRadius);
            if(!reply.collect)
            {SendQuickMessage(lpPlayInfo,reply.message.c_str());return TRUE;}
            const gameplay::gathering::HarvestResult caught=gameplay::fishing::FishNodeForPlayer(
                gameplay::player::legacy::ServerLegacyOnlinePlayerRegistry().IdOf(lpPlayInfo),
                reply.nodeId,(std::uint64_t)dwPlayServTime,(std::uint32_t)rand());
            if(!caught.Succeeded())
                SendQuickMessage(lpPlayInfo,SummitModernHarvestStatusText(caught.status));
            else
            {
                sprintf_s(szTempBuff,sizeof(szTempBuff),"Sea: %s x%d (SQLite)",
                    caught.resourceKey.c_str(),caught.amount);
                SendQuickMessage(lpPlayInfo,szTempBuff);
            }
            return TRUE;
        }
        // Prevent the old one-message instant fish path from bypassing the stages.
        // Moderator node placement and list remain available below.
        if(lstrcmpi(action,"list")!=0 && lstrcmpi(action,"place")!=0)
        {SendQuickMessage(lpPlayInfo,"Sea: /fish cast > hook > reel > collect");return TRUE;}
    }

	// Mining/Gathering V2: command-first world interaction until 3D node entities
	// are available. Nodes are persistent and data-driven; the future entity only
	// needs to invoke the same harvest-by-nodeId contract.
	if(rsCompString("/resources", szChatCommand))
	{
		std::vector<gameplay::gathering::GatheringInventoryEntry> inventory;
		if(!gameplay::gathering::ServerGatheringRepository().GetInventory(
			lpPlayInfo->getName() ? lpPlayInfo->getName() : "", inventory))
		{
			SendQuickMessage(lpPlayInfo, "Resource inventory unavailable.");
			return TRUE;
		}
		if(inventory.empty())
			SendQuickMessage(lpPlayInfo, "Resource inventory is empty.");
		for(std::vector<gameplay::gathering::GatheringInventoryEntry>::const_iterator it=inventory.begin();
			it!=inventory.end(); ++it)
		{
			sprintf_s(szTempBuff,sizeof(szTempBuff),"%s x%d",it->resourceKey.c_str(),it->quantity);
			SendQuickMessage(lpPlayInfo,szTempBuff);
		}
		return TRUE;
	}

	int summitHarvestCommandLen = rsCompString("/mine", szChatCommand);
	const bool summitMiningCommand = summitHarvestCommandLen != 0;
	bool summitFishingCommand = false;
	if(!summitHarvestCommandLen)
	{
		summitHarvestCommandLen = rsCompString("/fish", szChatCommand);
		summitFishingCommand = summitHarvestCommandLen != 0;
	}
	if(!summitHarvestCommandLen)
		summitHarvestCommandLen = rsCompString("/gather", szChatCommand);
	if(summitHarvestCommandLen)
	{
		char sub[64] = {0};
		char arg1[64] = {0};
		char arg2[64] = {0};
		char arg3[64] = {0};
		char* cursor = szChatCommand + summitHarvestCommandLen;
		cursor = rsGetWord(sub,cursor);
		if(cursor) cursor = rsGetWord(arg1,cursor);
		if(cursor) cursor = rsGetWord(arg2,cursor);
		if(cursor) cursor = rsGetWord(arg3,cursor);

		const int playerArea = lpPlayInfo->Position.Area;
		const int playerX = lpPlayInfo->Position.x >> FLOATNS;
		const int playerY = lpPlayInfo->Position.y >> FLOATNS;
		const int playerZ = lpPlayInfo->Position.z >> FLOATNS;

		if(sub[0] && lstrcmpi(sub,"list") == 0)
		{
			std::vector<gameplay::gathering::GatheringNodeDefinition> nodes;
			std::vector<gameplay::gathering::ResourceDefinition> resources;
			if(summitMiningCommand)
			{
				gameplay::mining::ServerMiningService().ListNodes(nodes);
				gameplay::mining::ServerMiningService().ListResources(resources);
			}
			else if(summitFishingCommand)
			{
				gameplay::fishing::ServerFishingService().ListNodes(nodes);
				gameplay::fishing::ServerFishingService().ListResources(resources);
			}
			else
			{
				gameplay::gathering::ServerGatheringService().ListNodes(nodes);
				gameplay::gathering::ServerGatheringService().ListResources(resources);
			}
			int shown=0;
			for(std::vector<gameplay::gathering::GatheringNodeDefinition>::const_iterator n=nodes.begin();
				n!=nodes.end() && shown<12; ++n)
			{
				if(n->area!=playerArea) continue;
				const char* key="resource";
				for(std::vector<gameplay::gathering::ResourceDefinition>::const_iterator r=resources.begin();r!=resources.end();++r)
					if(r->resourceId==n->resourceId){ key=r->key.c_str(); break; }
				const int dx=playerX-n->x, dz=playerZ-n->z;
				const int distance=(int)sqrt((double)(dx*dx+dz*dz));
				sprintf_s(szTempBuff,sizeof(szTempBuff),"node %u | %s | distance %d",(unsigned)n->nodeId,key,distance);
				SendQuickMessage(lpPlayInfo,szTempBuff); ++shown;
			}
			if(!shown) SendQuickMessage(lpPlayInfo,"No resource nodes registered in this area.");
			return TRUE;
		}

		if(sub[0] && lstrcmpi(sub,"place") == 0)
		{
			if(lpPlayInfo->getSecurity() < AccountTypes::SEC_MODERATOR)
			{
				SendQuickMessage(lpPlayInfo,"Only moderators can place resource nodes.");
				return TRUE;
			}
			if(!arg1[0])
			{
				SendQuickMessage(lpPlayInfo, summitFishingCommand ?
                    "Usage: /fish place <resourceKey> [radius] [respawnSec]" :
                    "Usage: /mine place <resourceKey> [radius] [respawnSec]");
				return TRUE;
			}
			const int radius=arg2[0]?atoi(arg2):160;
			const int respawnSeconds=arg3[0]?atoi(arg3):30;
			gameplay::gathering::GatheringNodeDefinition node;
			std::string error;
			if(!gameplay::gathering::AddServerGatheringNode(arg1,playerArea,playerX,playerY,playerZ,
				radius,(std::uint64_t)(respawnSeconds>0?respawnSeconds:30)*1000ULL,node,&error))
			{
				sprintf_s(szTempBuff,sizeof(szTempBuff),"Node placement failed: %s",error.c_str());
				SendQuickMessage(lpPlayInfo,szTempBuff); return TRUE;
			}
			sprintf_s(szTempBuff,sizeof(szTempBuff),"Resource node %u placed: %s",(unsigned)node.nodeId,arg1);
			SendQuickMessage(lpPlayInfo,szTempBuff); return TRUE;
		}

		std::uint32_t nodeId=0;
		if(sub[0] && sub[0]>='0' && sub[0]<='9') nodeId=(std::uint32_t)strtoul(sub,0,10);
		if(!nodeId)
		{
			gameplay::gathering::GatheringNodeDefinition nearest;
			gameplay::gathering::ResourceDefinition resource;
			const gameplay::gathering::GatheringKind kind=summitMiningCommand ?
				gameplay::gathering::GatheringKind::Mining : (summitFishingCommand ?
				gameplay::gathering::GatheringKind::Fishing : gameplay::gathering::GatheringKind::Custom);
			if(!gameplay::gathering::FindNearestGatheringNode(playerArea,playerX,playerY,playerZ,kind,
				summitMiningCommand || summitFishingCommand,nearest,resource))
			{
				SendQuickMessage(lpPlayInfo,"No resource node registered in this area."); return TRUE;
			}
			nodeId=nearest.nodeId;
		}

		gameplay::gathering::HarvestResult harvest;
		if(summitMiningCommand)
			harvest=gameplay::mining::HarvestMiningNodeForPlayer(gameplay::player::legacy::ServerLegacyOnlinePlayerRegistry().IdOf(lpPlayInfo),nodeId,(std::uint64_t)dwPlayServTime,(std::uint32_t)rand());
		else if(summitFishingCommand)
			harvest=gameplay::fishing::FishNodeForPlayer(gameplay::player::legacy::ServerLegacyOnlinePlayerRegistry().IdOf(lpPlayInfo),nodeId,(std::uint64_t)dwPlayServTime,(std::uint32_t)rand());
		else
			harvest=gameplay::gathering::HarvestGatheringNodeForPlayer(gameplay::player::legacy::ServerLegacyOnlinePlayerRegistry().IdOf(lpPlayInfo),nodeId,(std::uint64_t)dwPlayServTime,(std::uint32_t)rand());
		if(!harvest.Succeeded())
		{
			sprintf_s(szTempBuff,sizeof(szTempBuff),"Harvest failed: %s",SummitModernHarvestStatusText(harvest.status));
			SendQuickMessage(lpPlayInfo,szTempBuff); return TRUE;
		}
		sprintf_s(szTempBuff,sizeof(szTempBuff),"Harvested %s x%d",harvest.resourceKey.c_str(),harvest.amount);
		SendQuickMessage(lpPlayInfo,szTempBuff); return TRUE;
	}

	// Covenant V1: chat is the first player-facing surface. Keep business rules in
	// ProgressionRuntime/CovenantService so a future UI or world altar can reuse them.
	if(rsCompString("/covenant", szChatCommand))
	{
		gameplay::covenants::CovenantChatCommandContext covenantContext;
		covenantContext.playerKey = lpPlayInfo->getName() ? lpPlayInfo->getName() : "";
		covenantContext.playerLevel = lpPlayInfo->smCharInfo.Level;
		covenantContext.legacyJobCode = lpPlayInfo->smCharInfo.JOB_CODE;
		covenantContext.allowAdminCommands =
			lpPlayInfo->getSecurity() >= AccountTypes::SEC_MODERATOR;

		// Switching is deliberately blocked during volatile player states. The
		// persisted cooldown is enforced again inside ProgressionRuntime.
		if(lpPlayInfo->smCharInfo.Life[0] <= 0)
		{
			covenantContext.canSwitch = false;
			covenantContext.switchBlockReason = "cannot switch Covenant while dead";
		}
		else if(lpPlayInfo->dwLastAttackTime &&
			(dwPlayServTime - lpPlayInfo->dwLastAttackTime) < 10000)
		{
			covenantContext.canSwitch = false;
			covenantContext.switchBlockReason = "cannot switch Covenant during combat";
		}
		else if(lpPlayInfo->dwLastTradeTime &&
			(dwPlayServTime - lpPlayInfo->dwLastTradeTime) < 8000)
		{
			covenantContext.canSwitch = false;
			covenantContext.switchBlockReason = "cannot switch Covenant during trade";
		}

		if(lpPlayInfo->lpChrTarget)
		{
			covenantContext.currentTargetSerial = lpPlayInfo->lpChrTarget->dwObjectSerial;
			covenantContext.currentTargetIsHuntTarget =
				lpPlayInfo->lpChrTarget->smCharInfo.wPlayClass[0] == MONSTER_CLASS_BOSS;
		}

		// Surveyor consumes the Mining registry but intentionally exposes only
		// resource/area/count, never exact node coordinates. The same registry can
		// later feed a map UI without changing Covenant rules.
		{
			std::vector<gameplay::gathering::ResourceDefinition> covenantResources;
			std::vector<gameplay::gathering::GatheringNodeDefinition> covenantNodes;
			gameplay::mining::ServerMiningService().ListResources(covenantResources);
			gameplay::mining::ServerMiningService().ListNodes(covenantNodes);
			std::vector<gameplay::gathering::ResourceDefinition> extraResources;
			std::vector<gameplay::gathering::GatheringNodeDefinition> extraNodes;
			gameplay::gathering::ServerGatheringService().ListResources(extraResources);
			gameplay::gathering::ServerGatheringService().ListNodes(extraNodes);
			covenantResources.insert(covenantResources.end(),extraResources.begin(),extraResources.end());
			covenantNodes.insert(covenantNodes.end(),extraNodes.begin(),extraNodes.end());
			for(std::vector<gameplay::gathering::GatheringNodeDefinition>::const_iterator node = covenantNodes.begin();
				node != covenantNodes.end(); ++node)
			{
				std::string resourceKey;
				for(std::vector<gameplay::gathering::ResourceDefinition>::const_iterator resource = covenantResources.begin();
					resource != covenantResources.end(); ++resource)
				{
					if(resource->resourceId == node->resourceId)
					{
						resourceKey = resource->key;
						break;
					}
				}
				if(resourceKey.empty())
					continue;
				bool merged = false;
				for(std::vector<gameplay::covenants::CovenantSurveyArea>::iterator area = covenantContext.surveyAreas.begin();
					area != covenantContext.surveyAreas.end(); ++area)
				{
					if(area->area == node->area && area->resourceKey == resourceKey)
					{
						++area->nodeCount;
						merged = true;
						break;
					}
				}
				if(!merged)
				{
					gameplay::covenants::CovenantSurveyArea area;
					area.resourceKey = resourceKey;
					area.area = node->area;
					area.nodeCount = 1;
					covenantContext.surveyAreas.push_back(area);
				}
			}
		}

		const gameplay::covenants::CovenantChatCommandResponse covenantResponse =
			gameplay::covenants::HandleServerCovenantChatCommand(covenantContext, szChatCommand);
		if(covenantResponse.handled)
		{
			for(std::vector<std::string>::const_iterator line = covenantResponse.lines.begin();
				line != covenantResponse.lines.end(); ++line)
			{
				sprintf_s(szTempBuff, sizeof(szTempBuff), "%s", line->c_str());
				SendQuickMessage(lpPlayInfo, szTempBuff);
			}
			return TRUE;
		}
	}

	int len = rsCompString("/companion", szChatCommand);
	if(len)
	{
		lpBuff = szChatCommand + len;
		lpBuff = rsGetWord(szCmdBuff, lpBuff);
		const gameplay::PlayerId companionOwnerId =
			gameplay::player::legacy::ServerLegacyOnlinePlayerRegistry().IdOf(lpPlayInfo);
		gameplay::companions::legacy::LegacyCompanionCommandsAdapter& companionCommands =
			gameplay::companions::legacy::ServerLegacyCompanionCommandsAdapter();

		if(!szCmdBuff[0] || lstrcmpi(szCmdBuff, "status") == 0)
		{
			gameplay::companions::CompanionStateSnapshot companionState;
			gameplay::companions::legacy::ServerLegacyCompanionQueryAdapter().TryGetState(
				gameplay::player::legacy::ServerLegacyOnlinePlayerRegistry().IdOf(lpPlayInfo), companionState);
			sprintf_s(szTempBuff, sizeof(szTempBuff),
				"Companions: %d/%d | selected: %s | mode: %d | crystal: %u",
				static_cast<int>(companionState.activeCount),
				gameplay::companions::COMPANION_MAX_ACTIVE,
				companionState.selectedName.empty() ? "none" : companionState.selectedName.c_str(),
				static_cast<int>(companionState.mode),
				(unsigned int)gameplay::companions::legacy::ServerLegacyCompanionQueryAdapter().ConfiguredCrystalItemCode());
			SendQuickMessage(lpPlayInfo, szTempBuff);
			return TRUE;
		}

		if(lstrcmpi(szCmdBuff, "list") == 0)
		{
			char companionNames[sPLAY_CHAR_MAX][32];
			ZeroMemory(companionNames, sizeof(companionNames));
			const int companionNameCount = rsGetAccountCharacterNames(
				lpPlayInfo->getAccount(), companionNames, sPLAY_CHAR_MAX);
			if(companionNameCount <= 0)
			{
				SendQuickMessage(lpPlayInfo, "No characters found on this account.");
				return TRUE;
			}
			for(int companionIndex = 0; companionIndex < companionNameCount; ++companionIndex)
			{
				if(!companionNames[companionIndex][0]) continue;
				sprintf_s(szTempBuff, sizeof(szTempBuff), "[%d] %s%s",
					companionIndex + 1, companionNames[companionIndex],
					(lpPlayInfo->getName() && lstrcmpi(lpPlayInfo->getName(), companionNames[companionIndex]) == 0)
						? " (current)" : "");
				SendQuickMessage(lpPlayInfo, szTempBuff);
			}
			return TRUE;
		}

		if(lstrcmpi(szCmdBuff, "select") == 0)
		{
			lpBuff = rsGetWord(szTempBuff, lpBuff);
			if(!szTempBuff[0])
			{
				SendQuickMessage(lpPlayInfo, "Usage: /companion select <character>");
				return TRUE;
			}
			const int result = companionCommands.Select(companionOwnerId, szTempBuff);
			sprintf_s(szCmdBuff, sizeof(szCmdBuff), result == TRUE ?
				"Selected companion: %s" : "Could not select companion: %s", szTempBuff);
			SendQuickMessage(lpPlayInfo, szCmdBuff);
			return TRUE;
		}

		if(lstrcmpi(szCmdBuff, "summon") == 0)
		{
			lpBuff = rsGetWord(szTempBuff, lpBuff);
			const int result = szTempBuff[0] ?
				companionCommands.Summon(companionOwnerId, szTempBuff) :
				companionCommands.SummonSelected(companionOwnerId);
			sprintf_s(szCmdBuff, sizeof(szCmdBuff), "Companion summon result: %d", result);
			SendQuickMessage(lpPlayInfo, szCmdBuff);
			return TRUE;
		}

		if(lstrcmpi(szCmdBuff, "dismiss") == 0)
		{
			lpBuff = rsGetWord(szTempBuff, lpBuff);
			int result = TRUE;
			if(!szTempBuff[0] || lstrcmpi(szTempBuff, "all") == 0)
				result = companionCommands.DismissAll(companionOwnerId);
			else
				result = companionCommands.Dismiss(companionOwnerId, szTempBuff);
			sprintf_s(szCmdBuff, sizeof(szCmdBuff), "Companion dismiss result: %d", result);
			SendQuickMessage(lpPlayInfo, szCmdBuff);
			return TRUE;
		}

		if(lstrcmpi(szCmdBuff, "mode") == 0)
		{
			lpBuff = rsGetWord(szTempBuff, lpBuff);
			gameplay::companions::CompanionBehaviorMode modernMode = gameplay::companions::CompanionBehaviorMode::Aggressive;
			if(lstrcmpi(szTempBuff, "follow") == 0)
				modernMode = gameplay::companions::CompanionBehaviorMode::Follow;
			else if(lstrcmpi(szTempBuff, "passive") == 0)
				modernMode = gameplay::companions::CompanionBehaviorMode::Passive;
			else if(lstrcmpi(szTempBuff, "aggressive") != 0)
			{
				SendQuickMessage(lpPlayInfo, "Usage: /companion mode aggressive|follow|passive");
				return TRUE;
			}
			companionCommands.SetMode(companionOwnerId, modernMode);
			SendQuickMessage(lpPlayInfo, "Companion mode updated.");
			return TRUE;
		}

		SendQuickMessage(lpPlayInfo,
			"/companion list|select <name>|summon [name]|dismiss [name|all]|mode <...>|status");
		return TRUE;
	}

	return FALSE;
}
