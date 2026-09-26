#include <cassert>
#include <cstring>
#include "gameplay/companions/CompanionProtocol.h"

int main()
{
    using namespace gameplay::companions::protocol;
    CompanionUIActionPacket action{};
    action.size = sizeof(action);
    action.code = OPCODE_COMPANION_UI_ACTION;
    action.version = COMPANION_UI_PROTOCOL_VERSION;
    action.action = COMPANION_UI_SUMMON;
    std::strncpy(action.characterName, "MageAlt", sizeof(action.characterName) - 1);
    assert(action.size == static_cast<std::int32_t>(sizeof(CompanionUIActionPacket)));
    assert(action.code != OPCODE_COMPANION_UI_STATE);

    CompanionUIStatePacket state{};
    state.size = sizeof(state);
    state.code = OPCODE_COMPANION_UI_STATE;
    state.version = COMPANION_UI_PROTOCOL_VERSION;
    state.entryCount = 5;
    assert(state.entryCount <= COMPANION_UI_MAX_CHARACTERS);
    assert(sizeof(CompanionUIStatePacket) < 4096);

    CompanionRuntimeStatePacket runtime{};
    runtime.size = sizeof(runtime);
    runtime.code = OPCODE_COMPANION_RUNTIME_STATE;
    runtime.version = COMPANION_RUNTIME_PROTOCOL_VERSION;
    runtime.entryCount = 4;
    assert(runtime.entryCount <= COMPANION_RUNTIME_MAX_ACTIVE);
    assert(sizeof(CompanionRuntimeStatePacket) < 1024);

    CompanionVisualStatePacket visual{};
    visual.size = sizeof(visual);
    visual.code = OPCODE_COMPANION_VISUAL_STATE;
    visual.version = COMPANION_VISUAL_PROTOCOL_VERSION;
    visual.entryCount = 1;
    visual.entries[0].objectSerial = 1234;
    visual.entries[0].motionState = 0x100;
    visual.entries[0].motionSequence = 7;
    visual.entries[0].rightItemCode = 0x01060100u;
    visual.entries[0].equipmentRevision = 3;
    visual.entries[0].logicalSkillCode = 0x05120001u;
    visual.entries[0].skillPlayCode = 0x000000E1u;
    assert(visual.entryCount <= COMPANION_VISUAL_MAX_ACTIVE);
    assert(visual.entries[0].motionSequence > 0);
    assert(visual.entries[0].equipmentRevision > 0);
    assert(visual.entries[0].logicalSkillCode != 0);
    assert(visual.entries[0].logicalSkillCode != visual.entries[0].skillPlayCode);
    assert(sizeof(CompanionVisualStatePacket) < 1024);
    return 0;
}
