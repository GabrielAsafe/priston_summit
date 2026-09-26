#pragma once
#include <string>
#include <vector>
namespace summit { namespace content { namespace text_resource {
struct LegacyContentIdBinding { std::string id; void* pointer; };
inline std::vector<LegacyContentIdBinding>& LegacyContentIdBindings(){ static std::vector<LegacyContentIdBinding> v; return v; }
inline void ClearLegacyContentIdBindings(){ LegacyContentIdBindings().clear(); }
inline void RegisterLegacyContentPointer(const std::string&id,void*pointer){
  if(id.empty()||!pointer)
    return;
  std::vector<LegacyContentIdBinding>&v=LegacyContentIdBindings();
  for(std::size_t i=0;i<v.size();++i)
    if(v[i].id==id){v[i].pointer=pointer;return;}
  LegacyContentIdBinding b;b.id=id;b.pointer=pointer;v.push_back(b);
}
inline void* FindLegacyContentPointer(const std::string&id){
  if(id.empty())
    return 0;
  std::vector<LegacyContentIdBinding>&v=LegacyContentIdBindings();
  for(std::size_t i=0;i<v.size();++i)
    if(v[i].id==id)
      return v[i].pointer;
  return 0;
}
}}}
