// SUMMIT_ITEM_VISUAL_REGISTRY_V1 — GERADO A PARTIR DE Content/canonical/item_visuals/catalog.json
// NAO EDITAR: python3 SourceCode/tools/content_port/compile_item_visual_registry.py
#pragma once
#include <unordered_map>
#include <string>
#include <cstdint>
namespace summit { namespace content { namespace item_visuals {
struct ItemVisual {
  std::uint32_t itemId; int slot; const char* kind; const char* icon; const char* dropModel;
  std::unordered_map<int, std::string> equippedModels;
};
inline const std::unordered_map<std::uint32_t, ItemVisual>& Registry() {
  static const std::unordered_map<std::uint32_t, ItemVisual> catalog = {
    {0x02105F00u, {0x02105F00u, 14, "costume", "", "", {{JOBCODE_ARCHER, "char/tmABCD/CtfbD11.smd"}, {JOBCODE_ASSASSIN, "char/tmABCD/CtfbE11.smd"}, {JOBCODE_ATALANTA, "char/tmABCD/CmfbB11.smd"}, {JOBCODE_FIGHTER, "char/tmABCD/CtmbB11.smd"}, {JOBCODE_KNIGHT, "char/tmABCD/CmmbA11.smd"}, {JOBCODE_MAGICIAN, "char/tmABCD/CmmbD11.smd"}, {JOBCODE_MECHANICIAN, "char/tmABCD/CtmbA11.smd"}, {JOBCODE_PIKEMAN, "char/tmABCD/CtmbC11.smd"}, {JOBCODE_PRIESTESS, "char/tmABCD/CmfbC11.smd"}, {JOBCODE_SHAMAN, "char/tmABCD/CmmbE11.smd"}}}}, // CA115
    {0x02115F00u, {0x02115F00u, 14, "costume", "", "", {{JOBCODE_ARCHER, "char/tmABCD/CtfbD11.smd"}, {JOBCODE_ASSASSIN, "char/tmABCD/CtfbE11.smd"}, {JOBCODE_ATALANTA, "char/tmABCD/CmfbB11.smd"}, {JOBCODE_FIGHTER, "char/tmABCD/CtmbB11.smd"}, {JOBCODE_KNIGHT, "char/tmABCD/CmmbA11.smd"}, {JOBCODE_MAGICIAN, "char/tmABCD/CmmbD11.smd"}, {JOBCODE_MECHANICIAN, "char/tmABCD/CtmbA11.smd"}, {JOBCODE_PIKEMAN, "char/tmABCD/CtmbC11.smd"}, {JOBCODE_PRIESTESS, "char/tmABCD/CmfbC11.smd"}, {JOBCODE_SHAMAN, "char/tmABCD/CmmbE11.smd"}}}}, // CA715
    {0x02116800u, {0x02116800u, 14, "costume", "image/Sinimage/Items/Defense/itCA871.BMP", "image/Sinimage/Items/DropItem/itca871.smd", {{JOBCODE_FIGHTER, "char/tmABCD/CA871_FS.smd"}, {JOBCODE_KNIGHT, "char/tmABCD/CA871_KS.smd"}, {JOBCODE_MAGICIAN, "char/tmABCD/CA871_MG.smd"}, {JOBCODE_MECHANICIAN, "char/tmABCD/CA871_MS.smd"}, {JOBCODE_PIKEMAN, "char/tmABCD/CA871_PS.smd"}, {JOBCODE_SHAMAN, "char/tmABCD/CA871_XM.smd"}}}}, // CA871
    {0x02116900u, {0x02116900u, 14, "costume", "image/Sinimage/Items/Defense/itCA872.BMP", "image/Sinimage/Items/DropItem/itca872.smd", {{JOBCODE_ARCHER, "char/tmABCD/CA872_AS.smd"}, {JOBCODE_ASSASSIN, "char/tmABCD/CA872_SS.smd"}, {JOBCODE_ATALANTA, "char/tmABCD/CA872_AT.smd"}, {JOBCODE_PRIESTESS, "char/tmABCD/CA872_SC.smd"}}}}, // CA872
    {0x02116A00u, {0x02116A00u, 14, "costume", "image/Sinimage/Items/Defense/itCA873.BMP", "image/Sinimage/Items/DropItem/itca873.smd", {{JOBCODE_FIGHTER, "char/tmABCD/CA873_FS.smd"}, {JOBCODE_KNIGHT, "char/tmABCD/CA873_KS.smd"}, {JOBCODE_MAGICIAN, "char/tmABCD/CA873_MG.smd"}, {JOBCODE_MECHANICIAN, "char/tmABCD/CA873_MS.smd"}, {JOBCODE_PIKEMAN, "char/tmABCD/CA873_PS.smd"}, {JOBCODE_SHAMAN, "char/tmABCD/CA873_XM.smd"}}}}, // CA873
    {0x02116B00u, {0x02116B00u, 14, "costume", "image/Sinimage/Items/Defense/itCA874.BMP", "image/Sinimage/Items/DropItem/itca874.smd", {{JOBCODE_ARCHER, "char/tmABCD/CA874_AS.smd"}, {JOBCODE_ASSASSIN, "char/tmABCD/CA874_SS.smd"}, {JOBCODE_ATALANTA, "char/tmABCD/CA874_AT.smd"}, {JOBCODE_PRIESTESS, "char/tmABCD/CA874_SC.smd"}}}}, // CA874
    {0x02013F00u, {0x02013F00u, 14, "costume", "", "", {{JOBCODE_ARCHER, "char/tmABCD/StylishUniform.smd"}, {JOBCODE_ASSASSIN, "char/tmABCD/StylishUniform.smd"}, {JOBCODE_ATALANTA, "char/tmABCD/StylishUniform.smd"}, {JOBCODE_FIGHTER, "char/tmABCD/StylishUniform.smd"}, {JOBCODE_KNIGHT, "char/tmABCD/StylishUniform.smd"}, {JOBCODE_MECHANICIAN, "char/tmABCD/StylishUniform.smd"}, {JOBCODE_PIKEMAN, "char/tmABCD/StylishUniform.smd"}}}}, // DA147
    {0x02053F00u, {0x02053F00u, 14, "costume", "", "", {{JOBCODE_MAGICIAN, "char/tmABCD/StylishUniform.smd"}, {JOBCODE_PRIESTESS, "char/tmABCD/StylishUniform.smd"}, {JOBCODE_SHAMAN, "char/tmABCD/StylishUniform.smd"}}}}, // DA247
  }; return catalog;
}
inline const ItemVisual* Find(std::uint32_t code) {
  const auto& all = Registry(); auto it = all.find(code); return it==all.end()?nullptr:&it->second;
}
inline bool HasModelForJob(const ItemVisual& record, int jobCode) {
  return record.equippedModels.find(jobCode)!=record.equippedModels.end();
}
inline bool ResolveModel(const ItemVisual& record, int jobCode, std::string& path) {
  auto it=record.equippedModels.find(jobCode); if(it==record.equippedModels.end()) return false;
  path=it->second; return true;
}
}}} // summit::content::item_visuals
