#pragma once

#include <cstddef>
#include <vector>

#include "gameplay/classes/v2/services/ClassService.h"

namespace authentication
{
namespace character_selection
{
// Legacy HoLogin uses tribe 0/1 while the class domain uses brood 1/2.
// Keep that translation at the edge so class content never depends on UI constants.
inline int BroodFromLegacyTribe(int tribe)
{
    return tribe + 1;
}

class ClassSelectionAdapter
{
public:
    explicit ClassSelectionAdapter(const gameplay::classes::ClassService& service)
        : service_(service)
    {
    }

    std::vector<const gameplay::classes::PlayableClassDefinition*> Classes(int legacyTribe) const
    {
        return service_.SelectableClasses(BroodFromLegacyTribe(legacyTribe));
    }

    const gameplay::classes::PlayableClassDefinition* ClassAt(int legacyTribe, int index) const
    {
        if (index < 0) return 0;
        const std::vector<const gameplay::classes::PlayableClassDefinition*> classes = Classes(legacyTribe);
        const std::size_t position = static_cast<std::size_t>(index);
        return position < classes.size() ? classes[position] : 0;
    }

    int FaceCount(int legacyTribe, int classIndex) const
    {
        const gameplay::classes::PlayableClassDefinition* definition = ClassAt(legacyTribe, classIndex);
        return definition ? static_cast<int>(definition->creation.faces.size()) : 0;
    }

    bool BuildCreation(int legacyTribe,
                       int classIndex,
                       std::size_t bodyIndex,
                       std::size_t faceIndex,
                       bool alternateFace,
                       gameplay::classes::CharacterCreationTemplate& out) const
    {
        const gameplay::classes::PlayableClassDefinition* definition = ClassAt(legacyTribe, classIndex);
        if (!definition) return false;
        return service_.BuildCreationTemplate(definition->brood,
                                              definition->creation.selectionOrder,
                                              bodyIndex,
                                              faceIndex,
                                              alternateFace,
                                              out);
    }

    // Preserve the exact four-class legacy composition. For any other count,
    // spread the selectable classes symmetrically without adding class-specific code.
    static void ClassWorldPosition(int index, int count, int& x, int& z)
    {
        static const int legacyFour[4][2] = {
            { -75, -320 }, { -25, -330 }, { 25, -330 }, { 75, -320 }
        };
        if (count == 4 && index >= 0 && index < 4)
        {
            x = legacyFour[index][0];
            z = legacyFour[index][1];
            return;
        }

        if (count <= 0) { x = 0; z = -330; return; }
        x = (index * 50) - ((count - 1) * 25);
        const int absX = x < 0 ? -x : x;
        z = -330 + (absX > 25 ? (absX - 25) / 5 : 0);
    }

    // The face strip historically centers 128px cards around x=400.
    static int FacePositionX(int index, int count)
    {
        if (count <= 0) return 400 - 64;
        return 400 - (count * 64) + (index * 128);
    }

private:
    const gameplay::classes::ClassService& service_;
};

inline const ClassSelectionAdapter& DefaultClassSelectionAdapter()
{
    static const ClassSelectionAdapter adapter(gameplay::classes::DefaultClassService());
    return adapter;
}
}
}
