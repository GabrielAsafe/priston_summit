#include "systems/presentation/v2/legacy/LegacyTextureAdapter.h"

#include <cstring>

// Narrow ABI declaration. DWORD is unsigned long on the Windows target.
extern int CreateTextureMaterial(char* bitmap,
                                 char* opacity,
                                 int twoSide,
                                 int shade,
                                 unsigned long texSwap,
                                 unsigned long blendType);

namespace presentation
{
namespace legacy
{
namespace
{
const unsigned long kBlendAlpha = 0x01ul; // SMMAT_BLEND_ALPHA

struct TextureEntry
{
    const char* key;
    const char* path;
};

const TextureEntry kTextures[] =
{
    { "ui.teleport.window", "game//teleport//window.png" },
    { "ui.teleport.action.teleport", "game//teleport//btnteleport.png" },
    { "ui.teleport.action.teleport.alt", "game//teleport//btnteleport_.png" },
    { "ui.teleport.action.close", "game//teleport//close.png" },
    { "ui.teleport.action.close.alt", "game//teleport//close_.png" },
    { "ui.teleport.category.exp", "game//teleport//exp.png" },
    { "ui.teleport.category.hunt", "game//teleport//hunt.png" },
    { "ui.teleport.category.gold", "game//teleport//gold.png" },
    { "ui.teleport.category.town", "game//teleport//town.png" },
    { "ui.teleport.category.sheltons", "game//teleport//sheltons.png" },
    { "ui.teleport.category.pvp", "game//teleport//pvp.png" },
    { "ui.teleport.category.huntpvp", "game//teleport//huntpvp.png" },
    { "ui.teleport.map.border", "game//teleport//Mapas//border.png" },
    { "ui.teleport.page.1.base", "game//teleport//page1.png" },
    { "ui.teleport.page.1.alt", "game//teleport//page1_.png" },
    { "ui.teleport.page.2.base", "game//teleport//page2.png" },
    { "ui.teleport.page.2.alt", "game//teleport//page2_.png" },
    { "ui.teleport.page.3.base", "game//teleport//page3.png" },
    { "ui.teleport.page.3.alt", "game//teleport//page3_.png" },
    { "ui.teleport.expdrop", "game//teleport//expdrop.png" },
    { "ui.teleport.map.estrada-dos-ventos", "game//teleport//Mapas//estrada-dos-ventos.png" },
    { "ui.teleport.map.vale-tranquilo", "game//teleport//Mapas//vale-tranquilo.png" },
    { "ui.teleport.map.floresta-espirito", "game//teleport//Mapas//floresta-espirito.png" },
    { "ui.teleport.map.mata-acacias", "game//teleport//Mapas//mata-acacias.png" },
    { "ui.teleport.map.jardim-liberdade", "game//teleport//Mapas//jardim-liberdade.png" },
    { "ui.teleport.map.refugio-ancioes", "game//teleport//Mapas//refugio-ancioes.png" },
    { "ui.teleport.map.castelo-perdidos", "game//teleport//Mapas//castelo-perdidos.png" },
    { "ui.teleport.map.terra-maldita", "game//teleport//Mapas//terra-maldita.png" },
    { "ui.teleport.map.terra-esquecida", "game//teleport//Mapas//terra-esquecida.png" },
    { "ui.teleport.map.oasis", "game//teleport//Mapas//oasis.png" },
    { "ui.teleport.map.cba", "game//teleport//Mapas//cba.png" },
    { "ui.teleport.map.terra-proibida", "game//teleport//Mapas//terra-proibida.png" },
    { "ui.teleport.map.bcave", "game//teleport//Mapas//bcave.png" },
    { "ui.teleport.map.mcave", "game//teleport//Mapas//mcave.png" },
    { "ui.teleport.map.ds", "game//teleport//Mapas//ds.png" },
    { "ui.teleport.map.dun1", "game//teleport//Mapas//dun1.png" },
    { "ui.teleport.map.dun2", "game//teleport//Mapas//dun2.png" },
    { "ui.teleport.map.dun3", "game//teleport//Mapas//dun3.png" },
    { "ui.teleport.map.s1", "game//teleport//Mapas//s1.png" },
    { "ui.teleport.map.s2", "game//teleport//Mapas//s2.png" },
    { "ui.teleport.map.s3", "game//teleport//Mapas//s3.png" },
    { "ui.teleport.map.gl", "game//teleport//Mapas//gl.png" },
    { "ui.teleport.map.iron1", "game//teleport//Mapas//iron1.png" },
    { "ui.teleport.map.iron2", "game//teleport//Mapas//iron2.png" },
    { "ui.teleport.map.eura", "game//teleport//Mapas//eura.png" },
    { "ui.teleport.map.ice1", "game//teleport//Mapas//ice1.png" },
    { "ui.teleport.map.ice2", "game//teleport//Mapas//ice2.png" },
    { "ui.teleport.map.kcave", "game//teleport//Mapas//kcave.png" },
    { "ui.teleport.map.lost1", "game//teleport//Mapas//lost1.png" },
    { "ui.teleport.map.lost2", "game//teleport//Mapas//lost2.png" },
    { "ui.teleport.map.et1", "game//teleport//Mapas//et1.png" },
    { "ui.teleport.map.et2", "game//teleport//Mapas//et2.png" },
    { "ui.teleport.map.et3", "game//teleport//Mapas//et3.png" },
    { "ui.teleport.map.mine-ice", "game//teleport//Mapas//mine-ice.png" },
    { "ui.teleport.map.slab", "game//teleport//Mapas//slab.png" },
    { "ui.teleport.map.ancientw", "game//teleport//Mapas//AncientW.png" },
    { "ui.teleport.map.seaa", "game//teleport//Mapas//SeaA.png" },
    { "ui.teleport.map.stemple", "game//teleport//Mapas//Stemple.png" },
};

const char* ResolveTexturePath(const TextureAssetId& texture)
{
    for (const TextureEntry& entry : kTextures)
    {
        if (texture.key == entry.key)
            return entry.path;
    }
    return 0;
}
}

TextureHandle LegacyTextureAdapter::Load(const TextureAssetId& texture)
{
    if (!texture.Valid())
        return TextureHandle();

    const char* path = ResolveTexturePath(texture);
    if (!path)
        return TextureHandle();

    char mutablePath[260] = {};
    std::strncpy(mutablePath, path, sizeof(mutablePath) - 1u);
    return TextureHandle(CreateTextureMaterial(
        mutablePath, 0, 0, 0, 0ul, kBlendAlpha));
}

LegacyTextureAdapter& ClientLegacyTextureAdapter()
{
    static LegacyTextureAdapter adapter;
    return adapter;
}
}
}
