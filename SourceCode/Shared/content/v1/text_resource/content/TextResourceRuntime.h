#pragma once
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include "content/v1/text_resource/content/CompiledTextResourceSource.h"

namespace summit { namespace content { namespace text_resource {

inline bool TextResourceStrictMode(){ const char*e=std::getenv("SUMMIT_TEXT_RESOURCE_STRICT"); return e&&e[0]=='1'; }
inline const char* TextResourcePackPath(){ const char*p=std::getenv("SUMMIT_TEXT_RESOURCE_PACK"); return (p&&*p)?p:0; }

inline const CompiledTextResourceSource* ActiveTextResourceSource(){
  struct Holder {
    std::string path;
    CompiledTextResourceSource *source;
    Holder():source(0){ const char*p=TextResourcePackPath(); if(p){ path=p; source=new CompiledTextResourceSource(path.c_str()); } }
    ~Holder(){ delete source; }
  };
  static Holder h;
  return h.source;
}

inline bool CompiledTextResourcesEnabled(){ return TextResourcePackPath()!=0; }
inline bool CompiledTextResourcesReady(){ const CompiledTextResourceSource*s=ActiveTextResourceSource(); return s&&s->IsValid(); }

inline bool LoadTextResourceBytes(const char*path,std::vector<unsigned char>&out,std::string*id=0,std::uint32_t*kind=0){
  const CompiledTextResourceSource*s=ActiveTextResourceSource();
  return s&&s->IsValid()&&s->Load(path,out,id,kind,0);
}

inline bool TextResourceContentId(const char*path,std::string&out){
  const CompiledTextResourceSource*s=ActiveTextResourceSource(); return s&&s->IsValid()&&s->ContentIdForPath(path,out);
}

inline bool ListTextResources(const char*prefix,const char*extension,bool directOnly,std::vector<TextResourceEntry>&out,std::uint32_t kind=0){
  const CompiledTextResourceSource*s=ActiveTextResourceSource(); return s&&s->IsValid()&&s->List(prefix,extension,directOnly,out,kind,0);
}

inline char TextResourceAsciiLower(char c){ return (c>='A'&&c<='Z')?char(c-'A'+'a'):c; }
inline bool TextResourceAsciiIEquals(const std::string&a,const char*b){
  if(!b||a.size()!=std::char_traits<char>::length(b)) return false;
  for(std::size_t i=0;i<a.size();++i) if(TextResourceAsciiLower(a[i])!=TextResourceAsciiLower(b[i])) return false;
  return true;
}
inline std::string TextResourceTrimAscii(const std::string&s){
  std::size_t b=0,e=s.size();
  while(b<e&&(s[b]==' '||s[b]=='\t'||s[b]=='\r'||s[b]=='\n')) ++b;
  while(e>b&&(s[e-1]==' '||s[e-1]=='\t'||s[e-1]=='\r'||s[e-1]=='\n')) --e;
  return s.substr(b,e-b);
}

// Small Win32-profile compatible bridge for immutable UI INIs stored in SRES.
// Section/key names in this corpus are ASCII. Values remain byte-preserving
// apart from surrounding whitespace, matching the behavior callers rely on.
inline bool GetTextResourceProfileString(const char*path,const char*section,const char*key,std::string&out){
  out.clear(); if(!path||!section||!key) return false;
  std::vector<unsigned char> bytes; if(!LoadTextResourceBytes(path,bytes)) return false;
  std::string current; std::size_t pos=0;
  while(pos<=bytes.size()){
    std::size_t end=pos; while(end<bytes.size()&&bytes[end]!='\r'&&bytes[end]!='\n') ++end;
    std::string line(bytes.begin()+pos,bytes.begin()+end); line=TextResourceTrimAscii(line);
    if(line.size()>=2&&line[0]=='['&&line[line.size()-1]==']') current=TextResourceTrimAscii(line.substr(1,line.size()-2));
    else if(!line.empty()&&line[0]!=';'&&line[0]!='#'&&TextResourceAsciiIEquals(current,section)){
      std::size_t eq=line.find('=');
      if(eq!=std::string::npos){
        std::string k=TextResourceTrimAscii(line.substr(0,eq));
        if(TextResourceAsciiIEquals(k,key)){ out=TextResourceTrimAscii(line.substr(eq+1)); return true; }
      }
    }
    if(end>=bytes.size()) break;
    pos=end+1; if(end+1<bytes.size()&&bytes[end]=='\r'&&bytes[end+1]=='\n') ++pos;
  }
  return false;
}

// Compatibility bridge for legacy line parsers. In compiled mode the parser
// consumes bytes from the pack via a temporary CRT stream; the proprietary
// text file no longer needs to exist in the deployment. This is deliberately
// isolated so domains can later replace the parser with direct semantic
// adapters without changing callers again.
inline FILE* OpenTextResourceFile(const char*path,const char*mode){
  if(!path||!mode) return 0;
  const bool readOnly=(mode[0]=='r');
  if(!readOnly) return std::fopen(path,mode);
  const char*pack=TextResourcePackPath();
  if(pack){
    std::vector<unsigned char> bytes;
    const CompiledTextResourceSource*s=ActiveTextResourceSource();
    if(s&&s->IsValid()&&s->Load(path,bytes,0,0,0)){
      FILE*fp=std::tmpfile(); if(!fp) return TextResourceStrictMode()?0:std::fopen(path,mode);
      if(!bytes.empty()&&std::fwrite(&bytes[0],1,bytes.size(),fp)!=bytes.size()){std::fclose(fp);return TextResourceStrictMode()?0:std::fopen(path,mode);}
      std::rewind(fp); return fp;
    }
    if(TextResourceStrictMode()) return 0;
  }
  return std::fopen(path,mode);
}

}}}
