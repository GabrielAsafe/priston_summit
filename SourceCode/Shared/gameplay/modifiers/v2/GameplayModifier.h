#pragma once

#include <string>
#include <vector>

namespace gameplay
{
namespace modifiers
{
enum class ModifierOperation
{
    Flat,
    AdditivePercent,
    MultiplicativePercent
};

struct GameplayModifier
{
    std::string statKey;
    ModifierOperation operation;
    double value;
    std::string sourceKey;
    std::string scopeKey;

    GameplayModifier()
        : operation(ModifierOperation::Flat), value(0.0)
    {
    }
};

class ActiveModifierSet
{
public:
    void ReplaceSource(const std::string& sourceKey,
                       const std::vector<GameplayModifier>& modifiers)
    {
        RemoveSource(sourceKey);
        for (std::vector<GameplayModifier>::const_iterator it = modifiers.begin();
             it != modifiers.end(); ++it)
        {
            GameplayModifier copy = *it;
            copy.sourceKey = sourceKey;
            modifiers_.push_back(copy);
        }
    }

    void RemoveSource(const std::string& sourceKey)
    {
        std::vector<GameplayModifier> kept;
        kept.reserve(modifiers_.size());
        for (std::vector<GameplayModifier>::const_iterator it = modifiers_.begin();
             it != modifiers_.end(); ++it)
        {
            if (it->sourceKey != sourceKey)
                kept.push_back(*it);
        }
        modifiers_.swap(kept);
    }

    std::vector<GameplayModifier> ForStat(const std::string& statKey) const
    {
        std::vector<GameplayModifier> result;
        for (std::vector<GameplayModifier>::const_iterator it = modifiers_.begin();
             it != modifiers_.end(); ++it)
        {
            if (it->statKey == statKey)
                result.push_back(*it);
        }
        return result;
    }

    const std::vector<GameplayModifier>& All() const { return modifiers_; }

private:
    std::vector<GameplayModifier> modifiers_;
};
}
}
