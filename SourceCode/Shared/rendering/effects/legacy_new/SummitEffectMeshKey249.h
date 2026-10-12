#ifndef SUMMIT_EFFECT_MESH_KEY_249_H
#define SUMMIT_EFFECT_MESH_KEY_249_H
#include <string>
#include <utility>
namespace summit { namespace effects {
typedef std::pair<std::string, std::string> EffectMeshKey249;
inline EffectMeshKey249 MakeEffectMeshKey249(const std::string &mesh,
                                             const std::string &bone)
{
    return EffectMeshKey249(mesh, bone);
}
} }
#endif
