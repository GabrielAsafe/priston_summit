#pragma once

#include <string>
#include <vector>

namespace ui::v2
{
struct MixRecipeSnapshot
{
    std::string recipeId;
    std::string itemType;
    std::string itemTypeLabel;
    std::string sourceIngredients;
    std::string sourceBonuses;
    int totalSheltoms = 0;
    int minimumCharacterLevel = -1;
    std::vector<std::string> ingredients;
    std::vector<std::string> bonuses;
};

struct MixGuideSnapshot
{
    bool valid = false;
    std::string title;
    std::string sourceUrl;
    std::string retrievedDate;
    int totalCombinations = 0;
    std::vector<std::string> rules;
    std::vector<std::string> sheltoms;
    std::vector<MixRecipeSnapshot> recipes;
};
}
