#include <cassert>
#include <string>

#include "presentation/v2/AnimationService.h"

class FakeAnimationService final : public presentation::IAnimationService
{
public:
    bool Play(gameplay::EntityId entity, const presentation::AnimationId& animation) override
    {
        lastEntity = entity;
        lastKey = animation.key;
        return entity.Valid() && animation.Valid();
    }

    gameplay::EntityId lastEntity;
    std::string lastKey;
};

int main()
{
    FakeAnimationService service;
    assert(service.Play(gameplay::EntityId(77u), presentation::AnimationId("character.restart")));
    assert(service.lastEntity == gameplay::EntityId(77u));
    assert(service.lastKey == "character.restart");
    assert(!service.Play(gameplay::EntityId(), presentation::AnimationId("character.restart")));
    return 0;
}
