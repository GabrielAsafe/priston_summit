
#include "systems/rendering/legacy_3d/smd3d.h"

#include "systems/network/transport/legacy/smwsock.h"
#include "systems/gameplay/character/core/character.h"
#include "systems/media/video/legacy_avi/avictrl.h"
#include "core/server/gameplay_runtime/playmain.h"
#include "systems/audio/playback/legacy/Dxwav.h"
#include "core/data/legacy_loader/fileread.h"

#include "systems/rendering/effects/particle_legacy/particle.h"
#include "systems/network/server_gameplay/netplay.h"

#include "core/server/legacy_sin_runtime/sinLinkHeader.h"
#include "systems/presentation/opening/legacy_runtime/HoLinkHeader.h"

#include "systems/world/field/field.h"
#include "systems/audio/effects/legacy/effectsnd.h"

#include "systems/persistence/character_record/record.h"
#include "core/gameplay/legacy_support/playsub.h"
#include "platform/windows/resources/legacy_runtime/resource.h"
#include "systems/rendering/legacy_2d/drawsub.h"
#include "systems/combat/skills/server_support/SkillSub.h"
#include "systems/localization/config/language.h"

#include "core/server/legacy_runtime/onserver.h"

#include "core/server/action_game/ActionGame.h"


#include "gameplay/action/ActionGameLegacyCommon.inl"
