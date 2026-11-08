#pragma once
// SUMMIT415A: strict runtime rules (ASCII JSON); no chat, no SQLite writes in parser.
// Edit Servidor/Content/crafting/gm_drops_415.json and restart Server.exe.
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <random>
#include <set>
#include <string>
#include <vector>

namespace summit_gm415 {
struct Rule {
  std::string modelContains;
  std::string resource;
  std::string alias;
  std::string native;
  bool physical;
  unsigned quantity;
  unsigned chance;
};
struct Reader {
  const std::string& s; std::size_t at=0;
  explicit Reader(const std::string& v):s(v){}
  void ws(){while(at<s.size()&&(s[at]==' '||s[at]=='\n'||s[at]=='\t'||s[at]=='\r'))++at;}
  bool take(char c){ws();if(at>=s.size()||s[at]!=c)return false;++at;return true;}
  bool str(std::string& out){ws();if(at>=s.size()||s[at]!='"')return false;++at;out.clear();
    while(at<s.size()&&s[at]!='"'){unsigned char c=static_cast<unsigned char>(s[at++]);
      if(c<33||c>126||c=='\\'||out.size()>=96)return false;
      out.push_back(static_cast<char>(c));}
    if(at>=s.size())return false;
    ++at;return true;}
  bool field(const char* k){std::string v;return str(v)&&v==k&&take(':');}
  bool number(unsigned& value){ws();if(at>=s.size()||s[at]<'0'||s[at]>'9')return false;
    if(s[at]=='0'&&at+1<s.size()&&s[at+1]>='0'&&s[at+1]<='9')return false;
    value=0;unsigned digits=0;
    while(at<s.size()&&s[at]>='0'&&s[at]<='9'){
      if(++digits>6)return false;
      value=value*10+unsigned(s[at++]-'0');}
    return true;}
  bool end(){ws();return at==s.size();}
};
struct NativeEntry {const char* alias;const char* code;const char* resource;};
static const NativeEntry NativeItems[] = {
  {"C_it0x0001","CI101","raw_ore_crystalline"},
  {"C_it0x0002","CI102","core_crystalline"},
  {"C_it0x0003","CI103","raw_ore_energetic"},
  {"C_it0x0004","CI104","core_energetic"},
  {"C_it0x0005","CI105","raw_ore_deep"},
  {"C_it0x0006","CI106","core_deep"},
  {"C_it0x0007","CI107","raw_ore_arcane"},
  {"C_it0x0008","CI108","core_arcane"},
  {"C_it0x0009","CI109","raw_ore_runic"},
  {"C_it0x000A","CI110","core_runic"},
  {"C_it0x000B","CI111","raw_ore_primordial"},
  {"C_it0x000C","CI112","core_primordial"},
  {"C_it0x000D","CI113","common_logs"},
  {"C_it0x000E","CI114","sap"},
  {"C_it0x000F","CI115","bark"},
  {"C_it0x0010","CI116","resistant_logs"},
  {"C_it0x0011","CI117","dense_resin"},
  {"C_it0x0012","CI118","crystalline_wood"},
  {"C_it0x0013","CI119","energetic_sap"},
  {"C_it0x0014","CI120","arcane_wood"},
  {"C_it0x0015","CI121","mystic_resin"},
  {"C_it0x0016","CI122","tree_heart"},
  {"C_it0x0017","CI123","ore_amethyst"},
  {"C_it0x0018","CI124","ore_silver"},
  {"C_it0x0019","CI125","ore_gold"},
  {"C_it0x001A","CI126","ore_topaz"},
  {"C_it0x001B","CI127","ore_sapphire"},
  {"C_it0x001C","CI128","ore_citrine"},
  {"C_it0x001D","CI129","ore_ruby"},
  {"C_it0x001E","CI130","ore_emerald"},
  {"C_it0x001F","CI131","silver_dust"},
  {"C_it0x0020","CI132","ground_amethyst"},
  {"C_it0x0021","CI133","gold_dust"},
  {"C_it0x0022","CI134","topaz_dust"},
  {"C_it0x0023","CI135","ground_sapphire"},
  {"C_it0x0024","CI136","ground_citrine"},
  {"C_it0x0025","CI137","ground_ruby"},
  {"C_it0x0026","CI138","ground_emerald"},
  {"C_it0x0027","CI139","iron_ingot"},
  {"C_it0x0028","CI140","planks"},
  {"C_it0x0029","CI141","resistant_planks"},
  {"C_it0x002A","CI142","treated_planks"},
  {"C_it0x002B","CI143","charcoal"},
  {"C_it0x002C","CI144","reinforced_handle"},
  {"C_it0x002D","CI145","purified_resin"},
  {"C_it0x002E","CI146","arcane_charcoal"},
  {"C_it0x002F","CI147","crystalline_sap"},
  {"C_it0x0030","CI148","dark_resin"},
  {"C_it0x0031","CI149","arcane_ash"},
  {"C_it0x0032","CI150","primordial_essence"},
  {"C_it0x0033","CI151","dark_essence"},
  {"C_it0x0034","CI152","purified_salt"},
  {"C_it0x0035","CI153","illusory_essence"},
  {"C_it0x0036","CI154","arcane_essence"},
  {"C_it0x0037","CI155","war_fragment"},
  {"C_it0x0038","CI156","purified_gold"},
  {"C_it0x0039","CI157","scroll_blank"},
  {"C_it0x003A","CI158","scroll_enhancement"},
  {"C_it0x003B","CI159","mystic_solvent"},
  {"C_it0x003C","CI160","machine_treated"},
  {"C_it0x003D","CI161","tool_reinforced"},
  {"C_it0x003E","OS101","sheltom_lucidi"},
  {"C_it0x003F","OS102","sheltom_sereneo"},
  {"C_it0x0040","OS103","sheltom_fadeo"},
  {"C_it0x0041","OS104","sheltom_sparky"},
  {"C_it0x0042","OS105","sheltom_raident"},
  {"C_it0x0043","OS106","sheltom_transparo"},
  {"C_it0x0044","OS107","sheltom_murky"},
  {"C_it0x0045","OS108","sheltom_devine"},
  {"C_it0x0046","OS109","sheltom_celesto"},
  {"C_it0x0047","OS110","sheltom_mirage"},
  {"C_it0x0048","OS111","sheltom_inferna"},
  {"C_it0x0049","OS112","sheltom_enigma"},
  {"C_it0x004A","OS113","sheltom_bellum"},
  {"C_it0x004B","OS114","sheltom_ordo"},
  {"C_it0x004C","OS115","sheltom_gemma"},
  {"C_it0x004D","OS116","sheltom_purus"},
  {"C_it0x004E","OS117","sheltom_veritas"},
  {"C_it0x004F","CI162","potion_hp_enhanced"},
  {"C_it0x0050","CI163","potion_mp_enhanced"},
  {"C_it0x0051","CI164","potion_stamina_enhanced"},
  {"C_it0x0052","WA103","crafted_wa103"},
  {"C_it0x0053","DA106","crafted_da106"},
  {"C_it0x0054","WS103","crafted_ws103"},
  {"C_it0x0055","WM110","crafted_wm110"},
  {"C_it0x0056","CI165","timber"},
  {"C_it0x0057","CI166","iron_ore"},
  {"C_it0x0058","CI167","herb_bundle"},
  {"C_it0x0059","CI168","river_trout"},
};
inline bool resolve(const std::string& alias,const std::string& resource,std::string& out){
 for(const auto& i:NativeItems)if(alias==i.alias&&resource==i.resource){out=i.code;return true;}
 return false;
}
inline bool parse(const std::string& s,std::vector<Rule>& output){
 Reader p(s);std::string schema;
 if(!p.take('{')||!p.field("schema")||!p.str(schema)||schema!="summit.gm-crafting-drops/v1"||
    !p.take(',')||!p.field("entries")||!p.take('['))return false;
 std::set<std::string> seen;
 if(!p.take(']')){
  do {
   Rule r;std::string mode;
   if(!p.take('{')||!p.field("model_contains")||!p.str(r.modelContains)||
      !p.take(',')||!p.field("resource_key")||!p.str(r.resource)||
      !p.take(',')||!p.field("item_alias")||!p.str(r.alias)||
      !p.take(',')||!p.field("storage")||!p.str(mode)||
      !p.take(',')||!p.field("quantity")||!p.number(r.quantity)||
      !p.take(',')||!p.field("chance_bp")||!p.number(r.chance)||!p.take('}'))return false;
   r.physical=(mode=="physical");
   if((mode!="physical"&&mode!="sqlite")||r.modelContains.size()<3||r.modelContains.size()>64||
      r.quantity==0||r.quantity>(r.physical?1u:10u)||r.chance>10000u||
      !seen.insert(r.modelContains).second||!resolve(r.alias,r.resource,r.native)||
      output.size()>=64)return false;
   for(char c:r.modelContains)if((c<'a'||c>'z')&&(c<'0'||c>'9')&&c!='_'&&c!='-')return false;
   output.push_back(r);
   if(p.take(']'))break;
   if(!p.take(','))return false;
  }while(true);
 }
 return p.take('}')&&p.end()&&!output.empty();
}
inline const std::vector<Rule>& rules(){
 static const std::vector<Rule> cached=[](){
  const char* name="Content/crafting/gm_drops_415.json";
  std::ifstream f(name,std::ios::binary);
  if(!f.good()){std::fprintf(stderr,"[SUMMIT415] CONFIG_MISSING %s\n",name);return std::vector<Rule>{};}
  std::string src((std::istreambuf_iterator<char>(f)),std::istreambuf_iterator<char>());
  std::vector<Rule> v;
  if(src.size()>16384||!parse(src,v)){
    std::fprintf(stderr,"[SUMMIT415] CONFIG_INVALID %s\n",name);return std::vector<Rule>{};}
  std::fprintf(stderr,"[SUMMIT415] CONFIG_LOADED rules=%u file=%s\n",static_cast<unsigned>(v.size()),name);
  return v;
 }();return cached;
}
inline bool match(const char* model,const char* display,Rule& out){
 std::string m=model?model:"",d=display?display:"";
 if(m.size()>512||d.size()>512)return false;
 for(char& c:m)c=static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
 for(char& c:d)c=static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
 for(const Rule& rule:rules())if(m.find(rule.modelContains)!=std::string::npos||
                                 d.find(rule.modelContains)!=std::string::npos){out=rule;return true;}
 return false;
}
inline bool roll(unsigned chance){
 if(chance==0)return false;
 if(chance==10000)return true;
 static thread_local std::mt19937 engine(std::random_device{}());
 return std::uniform_int_distribution<unsigned>(0,9999)(engine)<chance;
}
} // namespace summit_gm415
