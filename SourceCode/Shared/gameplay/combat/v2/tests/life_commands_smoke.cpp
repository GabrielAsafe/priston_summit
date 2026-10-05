#include "gameplay/combat/v2/LifeCommands.h"

#include <cassert>
#include <map>

namespace
{
struct LifeState
{
    int current;
    int maximum;
};

class FakeLifeCommands final : public gameplay::combat::ILifeCommands
{
public:
    std::map<unsigned int, LifeState> entities;
    std::map<unsigned int, LifeState> players;

    gameplay::combat::LifeCommandResult Restore(gameplay::EntityId target, int amount) override
    {
        return RestoreState(entities, target.value, amount);
    }

    gameplay::combat::LifeCommandResult Restore(gameplay::PlayerId target, int amount) override
    {
        return RestoreState(players, target.value, amount);
    }

    gameplay::combat::LifeCommandResult ConsumeNonLethal(gameplay::EntityId target, int amount) override
    {
        gameplay::combat::LifeCommandResult result;
        auto it = entities.find(target.value);
        if(!target.Valid() || amount <= 0 || it == entities.end())
            return result;
        result.before = it->second.current;
        result.maximum = it->second.maximum;
        if(result.before <= amount)
        {
            result.code = gameplay::combat::LifeCommandCode::WouldKill;
            result.after = result.before;
            return result;
        }
        it->second.current -= amount;
        result.after = it->second.current;
        result.code = gameplay::combat::LifeCommandCode::Ok;
        return result;
    }

    gameplay::combat::LifeCommandResult ConsumeLethal(gameplay::EntityId target, int amount) override
    {
        gameplay::combat::LifeCommandResult result;
        auto it = entities.find(target.value);
        if(!target.Valid() || amount <= 0 || it == entities.end())
            return result;
        result.before = it->second.current;
        result.maximum = it->second.maximum;
        if(result.before <= 0 || result.maximum <= 0)
        {
            result.code = gameplay::combat::LifeCommandCode::InvalidState;
            result.after = result.before;
            return result;
        }
        it->second.current -= amount;
        if(it->second.current < 0)
            it->second.current = 0;
        result.after = it->second.current;
        result.code = gameplay::combat::LifeCommandCode::Ok;
        return result;
    }

    gameplay::combat::LifeCommandResult Deplete(gameplay::EntityId target) override
    {
        gameplay::combat::LifeCommandResult result;
        auto it = entities.find(target.value);
        if(!target.Valid() || it == entities.end())
            return result;
        result.before = it->second.current;
        result.maximum = it->second.maximum;
        if(result.before <= 0 || result.maximum <= 0)
        {
            result.code = gameplay::combat::LifeCommandCode::InvalidState;
            result.after = result.before;
            return result;
        }
        it->second.current = 0;
        result.after = 0;
        result.code = gameplay::combat::LifeCommandCode::Ok;
        return result;
    }

    gameplay::combat::LifeCommandResult EnsureMinimum(gameplay::EntityId target, int minimumLife) override
    {
        gameplay::combat::LifeCommandResult result;
        auto it = entities.find(target.value);
        if(!target.Valid() || minimumLife <= 0 || it == entities.end())
            return result;
        result.before = it->second.current;
        result.maximum = it->second.maximum;
        if(result.maximum <= 0)
        {
            result.code = gameplay::combat::LifeCommandCode::InvalidState;
            result.after = result.before;
            return result;
        }
        const int floorLife = minimumLife < result.maximum ? minimumLife : result.maximum;
        if(it->second.current < floorLife)
            it->second.current = floorLife;
        result.after = it->second.current;
        result.code = gameplay::combat::LifeCommandCode::Ok;
        return result;
    }

    gameplay::combat::LifeCommandResult ClampToValidRange(gameplay::EntityId target) override
    {
        gameplay::combat::LifeCommandResult result;
        auto it = entities.find(target.value);
        if(!target.Valid() || it == entities.end())
            return result;
        result.before = it->second.current;
        result.maximum = it->second.maximum;
        if(result.maximum <= 0)
        {
            result.code = gameplay::combat::LifeCommandCode::InvalidState;
            result.after = result.before;
            return result;
        }
        if(it->second.current < 0) it->second.current = 0;
        if(it->second.current > result.maximum) it->second.current = result.maximum;
        result.after = it->second.current;
        result.code = gameplay::combat::LifeCommandCode::Ok;
        return result;
    }

    gameplay::combat::LifeCommandResult RestoreToMaximum(gameplay::EntityId target) override
    {
        gameplay::combat::LifeCommandResult result;
        auto it = entities.find(target.value);
        if(!target.Valid() || it == entities.end())
            return result;
        result.before = it->second.current;
        result.maximum = it->second.maximum;
        if(result.maximum <= 0)
        {
            result.code = gameplay::combat::LifeCommandCode::InvalidState;
            result.after = result.before;
            return result;
        }
        it->second.current = result.maximum;
        result.after = it->second.current;
        result.code = gameplay::combat::LifeCommandCode::Ok;
        return result;
    }

    gameplay::combat::LifeCommandResult Revive(gameplay::EntityId target, int percentOfMaximum) override
    {
        gameplay::combat::LifeCommandResult result;
        auto it = entities.find(target.value);
        if(!target.Valid() || percentOfMaximum < 1 || percentOfMaximum > 100 || it == entities.end())
            return result;
        result.before = it->second.current;
        result.maximum = it->second.maximum;
        if(result.before > 0 || result.maximum <= 0)
        {
            result.code = gameplay::combat::LifeCommandCode::InvalidState;
            result.after = result.before;
            return result;
        }
        it->second.current = result.maximum * percentOfMaximum / 100;
        if(it->second.current < 1)
            it->second.current = 1;
        result.after = it->second.current;
        result.code = gameplay::combat::LifeCommandCode::Ok;
        return result;
    }

private:
    static gameplay::combat::LifeCommandResult RestoreState(std::map<unsigned int, LifeState>& states,
                                                             unsigned int id,
                                                             int amount)
    {
        gameplay::combat::LifeCommandResult result;
        auto it = states.find(id);
        if(id == 0u || amount <= 0 || it == states.end())
            return result;
        result.before = it->second.current;
        result.maximum = it->second.maximum;
        if(result.before <= 0 || result.maximum <= 0)
        {
            result.code = gameplay::combat::LifeCommandCode::InvalidState;
            result.after = result.before;
            return result;
        }
        it->second.current += amount;
        if(it->second.current > result.maximum)
            it->second.current = result.maximum;
        result.after = it->second.current;
        result.code = gameplay::combat::LifeCommandCode::Ok;
        return result;
    }
};
}

int main()
{
    FakeLifeCommands life;
    life.entities[1u] = LifeState{50, 100};
    life.entities[2u] = LifeState{0, 100};
    life.players[3u] = LifeState{80, 100};

    assert(life.Restore(gameplay::EntityId(1u), 70).after == 100);
    assert(life.ConsumeNonLethal(gameplay::EntityId(1u), 99).Ok());
    assert(life.ConsumeNonLethal(gameplay::EntityId(1u), 1).code == gameplay::combat::LifeCommandCode::WouldKill);
    assert(life.ConsumeLethal(gameplay::EntityId(1u), 5).after == 0);
    life.entities[5u] = LifeState{75, 100};
    assert(life.Deplete(gameplay::EntityId(5u)).after == 0);
    assert(life.Deplete(gameplay::EntityId(5u)).code == gameplay::combat::LifeCommandCode::InvalidState);
    assert(life.Revive(gameplay::EntityId(2u), 50).after == 50);
    life.entities[4u] = LifeState{0, 160};
    assert(life.EnsureMinimum(gameplay::EntityId(4u), 10).after == 10);
    life.entities[4u].current = 7;
    assert(life.EnsureMinimum(gameplay::EntityId(4u), 5).after == 7);
    assert(life.RestoreToMaximum(gameplay::EntityId(4u)).after == 160);
    life.entities[6u] = LifeState{-10, 100};
    assert(life.ClampToValidRange(gameplay::EntityId(6u)).after == 0);
    life.entities[6u].current = 150;
    assert(life.ClampToValidRange(gameplay::EntityId(6u)).after == 100);
    assert(life.Restore(gameplay::PlayerId(3u), 30).after == 100);
    return 0;
}
