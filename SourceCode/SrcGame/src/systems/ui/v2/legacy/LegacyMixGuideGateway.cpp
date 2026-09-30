#include "systems/ui/v2/legacy/LegacyMixGuideGateway.h"

#include <utility>

#include "content/v1/gameplay_reference/content/GameplayReferenceRuntime.h"

namespace ui::v2
{
MixGuideSnapshot LegacyMixGuideGateway::GetSnapshot() const
{
    MixGuideSnapshot out;
    auto& source = summit::content::gameplay_reference::GameplayReferenceRuntime();
    if (!source.IsValid())
        return out;

    summit::content::gameplay_reference::MixGuideCatalog catalog;
    std::vector<summit::content::gameplay_reference::MixGuideRecipe> recipes;
    if (!source.MixCatalog(catalog) || !source.MixRecipes(recipes))
        return out;

    out.valid = true;
    out.title = catalog.title;
    out.sourceUrl = catalog.sourceUrl;
    out.retrievedDate = catalog.retrievedDate;
    out.totalCombinations = catalog.totalCombinations;
    out.rules = catalog.rules;
    out.sheltoms = catalog.sheltoms;
    out.recipes.reserve(recipes.size());
    for (const auto& recipe : recipes)
    {
        MixRecipeSnapshot item;
        item.recipeId = recipe.recipeId;
        item.itemType = recipe.itemType;
        item.itemTypeLabel = recipe.itemTypeLabel;
        item.sourceIngredients = recipe.sourceIngredients;
        item.sourceBonuses = recipe.sourceBonuses;
        item.totalSheltoms = recipe.totalSheltoms;
        item.minimumCharacterLevel = recipe.minimumCharacterLevel;
        item.ingredients = recipe.ingredients;
        item.bonuses = recipe.bonuses;
        out.recipes.push_back(std::move(item));
    }
    return out;
}
}
