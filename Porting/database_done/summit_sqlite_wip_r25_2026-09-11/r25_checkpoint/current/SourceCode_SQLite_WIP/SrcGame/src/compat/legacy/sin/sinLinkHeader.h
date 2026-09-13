#ifndef _SINLINK_HEADER_ 
#define _SINLINK_HEADER_ 


#include <windows.h>
#include <math.h>
#include <stdio.h>



#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/rendering/2d/legacy/drawsub.h"
#include "systems/network/transport/legacy/smwsock.h"
#include "smPacket.h"
#include "global_def.h"
#include "systems/presentation/opening/legacy_runtime/HoLinkHeader.h" 
#include "systems/audio/effects/legacy/effectsnd.h"
#include "core/gameplay/legacy_support/playsub.h"
#include "systems/localization/legacy_catalog/language.h"

#include "compat/legacy/sin/util/sinUtil.h"
#include "systems/gameplay/events/bellatra/legacy_sin/sinMonster.h"
#include "core/client/legacy_sin_runtime/sinMain.h"
#include "systems/commerce/legacy/sinTrade.h"
#include "systems/rendering/effects/legacy_sin/sinSkinChange.h"
#include "core/client/legacy_sin_runtime/sinSubMain.h"
#include "systems/inventory/legacy/sinItem.h"
#include "systems/inventory/legacy/sinInvenTory.h"
#include "systems/gameplay/character/status_ui/sinCharStatus.h"
#include "systems/ui/hud/legacy_sin/sinInterFace.h"
#include "systems/commerce/legacy/sinShop.h"
#include "systems/combat/skills/legacy_sin/sinSkill.h"
#include "systems/ui/messages/legacy_sin/sinMessageBox.h"
#include "systems/rendering/effects/legacy_sin/sin3D.h"
#include "systems/rendering/effects/legacy_sin/sinEffect.h"
#include "systems/rendering/effects/legacy_sin/sinEffect2.h"

#include "systems/rendering/effects/legacy_sin/sinParticle.h"	
#include "systems/ui/messages/legacy_sin/sinMsg.h"
#include "systems/ui/help/legacy_sin/sinHelp.h"
#include "systems/gameplay/quests/legacy/sinQuest.h"
#include "systems/combat/skills/legacy_sin/sinSkillEffect.h"
#include "systems/gameplay/world_events/legacy_sin/sinEvent.h"
#include "systems/rendering/effects/legacy_sin/sinPublicEffect.h"
#include "systems/rendering/effects/legacy_sin/AssaUtil.h"
#include "systems/rendering/effects/legacy_sin/AssaEffect.h"
#include "systems/rendering/effects/legacy_sin/sinAssaParticle.h"
#include "systems/rendering/effects/legacy_sin/AssaTestParticle.h"
#include "systems/combat/skills/legacy_sin/sinAssaSkillEffect.h"

#include "systems/rendering/effects/legacy_sin/YameEffect.h"
#include "systems/gameplay/world_events/legacy_sin/sinWarpGate.h"
#include "systems/ui/help/legacy_sin/sinHelpPet.h"
#include "systems/gameplay/events/bellatra/legacy_sin/sinSOD2.h"

#include "systems/gameplay/quests/legacy/HaQuest.h"
#include "systems/commerce/premium/HaPremiumItem.h"
#include "systems/gameplay/pets/legacy_sin/sinParkPcBangPet.h"   


#include "systems/combat/skills/legacy_sin/Tempskron.h"
#include "systems/combat/skills/legacy_sin/Morayion.h"
#include "systems/social/clan/customization/ClanCustom.h"
#include "systems/progression/battle_pass/BattlePass.h"
#include "systems/rendering/font/newFont.h"
#include "systems/combat/damage_display/ShowDamage.h"
#include "systems/gameplay/teleport/ui/TCore.h"
#include "systems/crafting/mix_guide/cMixGuide.h"
#include "systems/crafting/professions/JOBS.h"
#include "systems/progression/rankings/class_rank/Rank.h"
#include "systems/progression/rankings/general/Ranking.h"
#include "platform/database/odbc/SQL.h"

//STD:
#include <ostream>
#include <vector>
#include <list>
#include <map>
#include <functional>
#include <utility>
#include <algorithm>
//#include <any>
#include <regex>
#include <memory>
#include <stack>
#include <mutex>
#include <deque>
#include <sstream>
#include <locale>
#include <cctype>
#include <queue>
#include <clocale>
#include <unordered_map>
#include <array>
#include <limits>
#include <chrono>
#include <random>
#include <filesystem>
#include <future>

//Windows Header Files:
#include <windows.h>
#include <shellapi.h>
#include <tlhelp32.h>
#include <shobjidl.h>
#include <intrin.h>
#include <psapi.h>
#include <immintrin.h>
#include <winternl.h>
#include <assert.h>
#include <shlwapi.h>
#include <winnt.h>

#include <mmsystem.h>

#include <stdio.h>
#include <stdlib.h>

#include <math.h>

#include <d3d9.h>
#include <d3dx9.h>
#include <d3dx9math.h>
#include <dsound.h>
#include <d3dcompiler.h>

#endif



