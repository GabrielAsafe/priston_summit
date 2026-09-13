#ifndef _SINLINK_HEADER_ 
#define _SINLINK_HEADER_ 


#include <windows.h>
#include <math.h>
#include <stdio.h>


#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/rendering/legacy_2d/drawsub.h"
#include "systems/network/transport/legacy/smwsock.h"
#include "smPacket.h"
#include "global_def.h"
#include "systems/presentation/opening/legacy_runtime/HoLinkHeader.h" 
#include "systems/audio/effects/legacy/effectsnd.h"
#include "core/gameplay/legacy_support/playsub.h"
#include "systems/localization/config/language.h"

#include "core/server/legacy_sin_runtime/sinUtil.h"
#include "systems/inventory/legacy/sinMonster.h"
#include "core/server/legacy_sin_runtime/sinMain.h"
#include "systems/commerce/legacy/sinTrade.h"
#include "systems/gameplay/character/legacy_ui/sinSkinChange.h"
#include "core/server/legacy_sin_runtime/sinSubMain.h"
#include "systems/inventory/legacy/sinItem.h"
#include "systems/inventory/legacy/sinInvenTory.h"
#include "systems/gameplay/character/legacy_ui/sinCharStatus.h"
#include "systems/ui/hud/legacy_sin/sinInterFace.h"
#include "systems/commerce/legacy/sinShop.h"
#include "systems/combat/skills/legacy/sinSkill.h"
#include "systems/ui/messages/legacy_sin/sinMessageBox.h"
#include "systems/rendering/effects/sin_legacy/sin3D.h"
#include "systems/rendering/effects/sin_legacy/sinEffect.h"
#include "systems/rendering/effects/sin_legacy/sinEffect2.h"

#include "systems/rendering/effects/sin_legacy/sinParticle.h"	
#include "systems/ui/messages/legacy_sin/sinMsg.h"
#include "systems/ui/help/legacy_sin/sinHelp.h"
#include "systems/gameplay/quests/legacy/sinQuest.h"
#include "systems/combat/skills/legacy/sinSkillEffect.h"
#include "systems/gameplay/world_events/legacy/sinEvent.h"
#include "systems/rendering/effects/sin_legacy/sinPublicEffect.h"
#include "systems/rendering/effects/sin_legacy/AssaUtil.h"
#include "systems/rendering/effects/sin_legacy/AssaEffect.h"
#include "systems/rendering/effects/sin_legacy/sinAssaParticle.h"
#include "systems/rendering/effects/sin_legacy/AssaTestParticle.h"
#include "systems/combat/skills/legacy/sinAssaSkillEffect.h"

#include "systems/rendering/effects/sin_legacy/YameEffect.h"
#include "systems/gameplay/world_events/legacy/sinWarpGate.h"
#include "systems/ui/help/legacy_sin/sinHelpPet.h"
#include "systems/gameplay/events/bellatra/legacy/sinSOD2.h"

#include "systems/gameplay/quests/legacy/HaQuest.h"
#include "systems/commerce/premium/HaPremiumItem.h"
#include "systems/gameplay/pets/legacy_sin/sinParkPcBangPet.h"   
#include "systems/persistence/database/odbc/SQL.h"
#include "systems/combat/skills/legacy/Tempskron.h"
#include "systems/combat/skills/legacy/Morayion.h"
#include "systems/social/clan/customization/ClanCustom.h"
#include "systems/crafting/professions/JOBS.h"
#include "systems/progression/class_rank/Rank.h"
#define ITEM_KIND_PERFECT			9
#endif


