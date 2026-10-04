from pathlib import Path
import re
root=Path('/mnt/data/audit_game/out/patched/src')
h=root/'Shared/content/v1/commerce/CoinShopCatalog.h';h.parent.mkdir(parents=True,exist_ok=True)
h.write_text(r'''#pragma once
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

namespace summit { namespace content { namespace commerce {
struct CoinShopCatalogEntry { std::string code; int section=0; int price=0; std::string currency; };
inline bool ReadCoinShopFile(std::string& out){
 const char* paths[]={"Content/canonical/commerce/coinshop.json","../Content/canonical/commerce/coinshop.json",0};
 for(int i=0;paths[i];++i){FILE*f=std::fopen(paths[i],"rb");if(!f)continue;std::fseek(f,0,SEEK_END);long n=std::ftell(f);std::fseek(f,0,SEEK_SET);if(n<0){std::fclose(f);continue;}out.resize((size_t)n);if(n&&std::fread(&out[0],1,(size_t)n,f)!=(size_t)n){std::fclose(f);continue;}std::fclose(f);return true;}return false;
}
inline bool JsonStringField(const std::string&o,const char*k,std::string&v){std::string q="\""+std::string(k)+"\"";size_t p=o.find(q);if(p==std::string::npos)return false;p=o.find(':',p+q.size());if(p==std::string::npos)return false;p=o.find('"',p+1);if(p==std::string::npos)return false;size_t e=o.find('"',p+1);if(e==std::string::npos)return false;v=o.substr(p+1,e-p-1);return true;}
inline bool JsonIntField(const std::string&o,const char*k,int&v){std::string q="\""+std::string(k)+"\"";size_t p=o.find(q);if(p==std::string::npos)return false;p=o.find(':',p+q.size());if(p==std::string::npos)return false;v=std::atoi(o.c_str()+p+1);return true;}
inline bool LoadCoinShopCatalog(const char* wanted,std::vector<CoinShopCatalogEntry>&out,std::string*error=0){out.clear();std::string s;if(!ReadCoinShopFile(s)){if(error)*error="coinshop.json not found";return false;}size_t items=s.find("\"items\"");if(items==std::string::npos){if(error)*error="items array missing";return false;}size_t p=s.find('[',items);if(p==std::string::npos)return false;for(;;){size_t b=s.find('{',p+1);size_t end=s.find(']',p+1);if(b==std::string::npos||(end!=std::string::npos&&end<b))break;size_t e=s.find('}',b+1);if(e==std::string::npos)break;std::string o=s.substr(b,e-b+1);CoinShopCatalogEntry x;if(JsonStringField(o,"code",x.code)&&JsonIntField(o,"section",x.section)&&JsonIntField(o,"price",x.price)&&JsonStringField(o,"currency",x.currency)){if(x.currency==wanted&&x.section>=1&&x.section<=7&&x.price>0&&!x.code.empty())out.push_back(x);}p=e;}return true;}
}}}
''',encoding='utf8')

def patch_file(p,currency):
 s=p.read_text(encoding='utf8',errors='ignore')
 if 'CoinShopCatalog.h' not in s:
  s=s.replace('#include "systems/commerce/coin_shop/', '#include "content/v1/commerce/CoinShopCatalog.h"\n#include "systems/commerce/coin_shop/',1)
 # replace LoadItems body
 start=s.index('void '+('cCOINSHOP_T' if currency=='time' else 'cCOINSHOP')+'::LoadItems()')
 brace=s.index('{',start); depth=0; end=None
 for i in range(brace,len(s)):
  if s[i]=='{': depth+=1
  elif s[i]=='}':
   depth-=1
   if depth==0: end=i+1;break
 cls='cCOINSHOP_T' if currency=='time' else 'cCOINSHOP'
 body=f'''void {cls}::LoadItems()\n{{\n\tfor (int i = 0; i < 7; ++i) {{ for (auto &itemShop : vItemShop[i]) delete itemShop; vItemShop[i].clear(); }}\n\tstd::vector<summit::content::commerce::CoinShopCatalogEntry> entries;\n\tstd::string error;\n\tif (!summit::content::commerce::LoadCoinShopCatalog("{currency}", entries, &error)) {{ std::printf("CoinShop catalog: %s\\n", error.c_str()); return; }}\n\tfor (const auto &entry : entries) {{\n\t\tsITEMSHOP *item = new sITEMSHOP; std::memset(item, 0, sizeof(*item));\n\t\tstd::strncpy(item->szItemCode, entry.code.c_str(), sizeof(item->szItemCode)-1); item->Price = entry.price;\n\t\tvItemShop[entry.section-1].push_back(item);\n\t}}\n}}'''
 s=s[:start]+body+s[end:]
 # Guard null DefaultItems resolution before memcpy
 needle='\t\t\t\t\tmemcpy(&TransShopData.sCompItem[itemCount], lpDefItem->DefCompressData, lpDefItem->DefCompressDataLen);'
 repl='\t\t\t\t\tif (!lpDefItem) { std::printf("CoinShop invalid item code: %s\\n", itemShop->szItemCode); continue; }\n'+needle
 s=s.replace(needle,repl)
 # Replace item price SELECT block in buy with in-memory catalog lookup, conservatively.
 old=re.compile(r'\t\tif \(db->Prepare\("SELECT \* FROM ItemList(?:_T)? WHERE szItemCode=\?"\)\)\n\t\t\{\n\t\t\tdb->BindInputParameter\(lpTransBuy->szItemCode, 1, PARAMTYPE_String\);\n\t\t\tif \(db->Execute\(\)\)\n\t\t\t\{\n\t\t\t\tint Price = 0;',re.M)
 replacement='''\t\tint CatalogPrice = 0;\n\t\tfor (int section = 0; section < 7 && !CatalogPrice; ++section)\n\t\t\tfor (auto *shopItem : vItemShop[section])\n\t\t\t\tif (shopItem && !lstrcmpi(shopItem->szItemCode, lpTransBuy->szItemCode)) { CatalogPrice = shopItem->Price; break; }\n\t\tif (CatalogPrice > 0)\n\t\t{\n\t\t\t{\n\t\t\t\tint Price = CatalogPrice;'''
 s,n=old.subn(replacement,s,count=1)
 if n!=1: print('WARN buy pattern',p,n)
 # Remove db GetData Price after catalog price
 s=s.replace('\n\t\t\t\tdb->GetData(&Price, 4, 3, PARAMTYPE_Integer);\n','\n',1)
 p.write_text(s,encoding='utf8')

for base in ['SrcServer/src','SrcGame/src']:
 patch_file(root/base/'systems/commerce/coin_shop/CoinShop.cpp','coin')
 patch_file(root/base/'systems/commerce/coin_shop/CoinShop_T.cpp','time')
