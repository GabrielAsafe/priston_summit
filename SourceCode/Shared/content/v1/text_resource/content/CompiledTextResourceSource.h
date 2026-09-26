#pragma once
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include "content/v1/common/Sha256.h"

namespace summit { namespace content { namespace text_resource {

enum TextResourceKind {
  TextResourceKindUnknown=0, TextResourceKindMonster=1, TextResourceKindNpc=2,
  TextResourceKindOpenItem=3, TextResourceKindLocalization=4,
  TextResourceKindSin=5, TextResourceKindMessage=6, TextResourceKindGameplayConfig=7,
  TextResourceKindGenericText=8
};

struct TextResourceEntry {
  std::string logicalPath;
  std::string contentId;
  std::uint32_t kind=0;
};

inline std::string NormalizeTextResourcePath(const char *path) {
  if(!path) return std::string();
  std::vector<std::string> parts; std::string part;
  for(const unsigned char *p=reinterpret_cast<const unsigned char*>(path);; ++p) {
    unsigned char c=*p;
    if(!c || c=='/' || c=='\\') {
      if(!part.empty() && part!=".") {
        if(part=="..") { if(!parts.empty()) parts.pop_back(); }
        else parts.push_back(part);
      }
      part.clear(); if(!c) break; continue;
    }
    part.push_back((c>='A'&&c<='Z')?static_cast<char>(c-'A'+'a'):static_cast<char>(c));
  }
  std::string out; for(std::size_t i=0;i<parts.size();++i){ if(i) out.push_back('/'); out+=parts[i]; }
  return out;
}

class CompiledTextResourceSource {
public:
  explicit CompiledTextResourceSource(const char *packPath):valid_(false){ if(packPath) packPath_=packPath; LoadPack(); }
  bool IsValid() const { return valid_; }
  const std::string& LastError() const { return lastError_; }
  const std::string& PackPath() const { return packPath_; }

  bool Load(const char *logicalPath,std::vector<unsigned char>& payload,std::string *contentId=0,
            std::uint32_t *kind=0,std::string *error=0) const {
    if(!valid_) return Fail(error,lastError_.empty()?"compiled text resource pack is invalid":lastError_.c_str());
    const std::string wanted=NormalizeTextResourcePath(logicalPath);
    if(wanted.empty()) return Fail(error,"text resource path is empty");
    for(std::uint32_t i=0;i<header_.recordCount;++i) {
      Record r; if(!ReadRecord(i,r)) return Fail(error,"compiled text resource record is corrupt");
      std::string stored; if(!ReadString(r.pathOffset,stored)) return Fail(error,"compiled text resource path is corrupt");
      if(NormalizeTextResourcePath(stored.c_str())!=wanted) continue;
      if(r.payloadOffset>header_.payloadSize || r.payloadSize>header_.payloadSize-r.payloadOffset)
        return Fail(error,"compiled text resource payload is outside pack");
      const std::size_t begin=static_cast<std::size_t>(header_.payloadOffset+r.payloadOffset);
      payload.assign(data_.begin()+begin,data_.begin()+begin+r.payloadSize);
      if(contentId && !ReadString(r.idOffset,*contentId)) return Fail(error,"compiled text resource id is corrupt");
      if(kind) *kind=r.kind;
      if(error) error->clear();
      return true;
    }
    return Fail(error,"text resource path is not present in compiled pack");
  }

  bool ContentIdForPath(const char *logicalPath,std::string &out) const {
    std::vector<unsigned char> ignored; return Load(logicalPath,ignored,&out,0,0);
  }

  bool List(const char *prefix,const char *extension,bool directOnly,std::vector<TextResourceEntry>& out,
            std::uint32_t kindFilter=0,std::string *error=0) const {
    if(!valid_) return Fail(error,lastError_.empty()?"compiled text resource pack is invalid":lastError_.c_str());
    const std::string base=NormalizeTextResourcePath(prefix);
    std::string ext=extension?extension:"";
    for(std::size_t i=0;i<ext.size();++i){ if(ext[i]=='\\') ext[i]='/'; if(ext[i]>='A'&&ext[i]<='Z') ext[i]=static_cast<char>(ext[i]-'A'+'a'); }
    out.clear(); std::vector<std::string> seenIds;
    for(std::uint32_t i=0;i<header_.recordCount;++i) {
      Record r; if(!ReadRecord(i,r)) return Fail(error,"compiled text resource record is corrupt");
      if((r.flags&1u)==0) continue; // runtime alias only
      if(kindFilter && r.kind!=kindFilter) continue;
      std::string path,id; if(!ReadString(r.pathOffset,path)||!ReadString(r.idOffset,id)) return Fail(error,"compiled text resource identity is corrupt");
      const std::string n=NormalizeTextResourcePath(path.c_str());
      if(!base.empty()) {
        if(n.size()<base.size() || n.compare(0,base.size(),base)!=0) continue;
        if(n.size()>base.size() && n[base.size()]!='/') continue;
      }
      std::size_t relStart=base.empty()?0:base.size(); if(relStart<n.size()&&n[relStart]=='/') ++relStart;
      const std::string rel=n.substr(relStart); if(directOnly && rel.find('/')!=std::string::npos) continue;
      if(!ext.empty() && (n.size()<ext.size() || n.compare(n.size()-ext.size(),ext.size(),ext)!=0)) continue;
      if(std::find(seenIds.begin(),seenIds.end(),id)!=seenIds.end()) continue;
      seenIds.push_back(id); TextResourceEntry e; e.logicalPath=path; e.contentId=id; e.kind=r.kind; out.push_back(e);
    }
    std::sort(out.begin(),out.end(),EntryLess); if(error) error->clear(); return true;
  }

private:
  struct Header { std::uint16_t version=0,flags=0; std::uint32_t recordCount=0,recordOffset=0,recordSize=0,stringOffset=0,stringSize=0,payloadOffset=0,payloadSize=0,profile=0,reserved0=0,reserved1=0; unsigned char digest[32]; };
  struct Record { std::uint32_t pathOffset=0,idOffset=0,kind=0,flags=0,payloadOffset=0,payloadSize=0; };
  static bool EntryLess(const TextResourceEntry&a,const TextResourceEntry&b){ return NormalizeTextResourcePath(a.logicalPath.c_str())<NormalizeTextResourcePath(b.logicalPath.c_str()); }
  static std::uint16_t U16(const unsigned char*p){ return static_cast<std::uint16_t>(p[0]|(static_cast<std::uint16_t>(p[1])<<8)); }
  static std::uint32_t U32(const unsigned char*p){ return static_cast<std::uint32_t>(p[0])|(static_cast<std::uint32_t>(p[1])<<8)|(static_cast<std::uint32_t>(p[2])<<16)|(static_cast<std::uint32_t>(p[3])<<24); }
  bool Range(std::uint64_t off,std::uint64_t len) const { return off<=data_.size() && len<=data_.size()-static_cast<std::size_t>(off); }
  static bool Fail(std::string*e,const char*m){ if(e) *e=m?m:"compiled text resource failure"; return false; }

  bool LoadPack(){
    valid_=false; data_.clear(); lastError_.clear();
    if(packPath_.empty()){ lastError_="compiled text resource pack path is empty"; return false; }
    FILE*fp=std::fopen(packPath_.c_str(),"rb"); if(!fp){ lastError_="compiled text resource pack not found"; return false; }
    if(std::fseek(fp,0,SEEK_END)!=0){ std::fclose(fp); lastError_="cannot size compiled text resource pack"; return false; }
    long sz=std::ftell(fp); if(sz<80||std::fseek(fp,0,SEEK_SET)!=0){ std::fclose(fp); lastError_="compiled text resource pack is too small"; return false; }
    data_.resize(static_cast<std::size_t>(sz)); if(std::fread(&data_[0],1,data_.size(),fp)!=data_.size()){ std::fclose(fp);data_.clear();lastError_="cannot read compiled text resource pack";return false;} std::fclose(fp);
    const unsigned char*p=&data_[0]; if(std::memcmp(p,"SRES",4)!=0){lastError_="compiled text resource magic mismatch";return false;}
    header_.version=U16(p+4);header_.flags=U16(p+6); if(header_.version!=2){lastError_="unsupported compiled text resource version";return false;}
    header_.recordCount=U32(p+8);header_.recordOffset=U32(p+12);header_.recordSize=U32(p+16);header_.stringOffset=U32(p+20);header_.stringSize=U32(p+24);header_.payloadOffset=U32(p+28);header_.payloadSize=U32(p+32);header_.profile=U32(p+36);header_.reserved0=U32(p+40);header_.reserved1=U32(p+44);std::memcpy(header_.digest,p+48,32);
    if(header_.recordSize!=24u||header_.recordOffset!=80u||header_.reserved0||header_.reserved1){lastError_="compiled text resource header is non-canonical";return false;}
    const std::uint64_t strings=static_cast<std::uint64_t>(header_.recordOffset)+static_cast<std::uint64_t>(header_.recordCount)*header_.recordSize;
    const std::uint64_t payload=strings+header_.stringSize,end=payload+header_.payloadSize;
    if(header_.stringOffset!=strings||header_.payloadOffset!=payload||end!=data_.size()){lastError_="compiled text resource layout is non-canonical";return false;}
    if(!Range(header_.recordOffset,static_cast<std::uint64_t>(header_.recordCount)*24u)||!Range(header_.stringOffset,header_.stringSize)||!Range(header_.payloadOffset,header_.payloadSize)){lastError_="compiled text resource section is outside file";return false;}
    unsigned char digest[32]; summit::content::common::Sha256(&data_[80],data_.size()-80,digest); if(std::memcmp(digest,header_.digest,32)!=0){lastError_="compiled text resource SHA-256 mismatch";return false;}
    for(std::uint32_t i=0;i<header_.recordCount;++i){Record r;std::string s;if(!ReadRecord(i,r)||!ReadString(r.pathOffset,s)||!ReadString(r.idOffset,s)||r.payloadOffset>header_.payloadSize||r.payloadSize>header_.payloadSize-r.payloadOffset){lastError_="compiled text resource record is invalid";return false;}}
    valid_=true;return true;
  }
  bool ReadRecord(std::uint32_t i,Record&r) const { if(i>=header_.recordCount)return false;std::uint64_t off=header_.recordOffset+static_cast<std::uint64_t>(i)*24u;if(!Range(off,24))return false;const unsigned char*p=&data_[static_cast<std::size_t>(off)];r.pathOffset=U32(p);r.idOffset=U32(p+4);r.kind=U32(p+8);r.flags=U32(p+12);r.payloadOffset=U32(p+16);r.payloadSize=U32(p+20);return true; }
  bool ReadString(std::uint32_t off,std::string&out) const { if(off>=header_.stringSize)return false;std::size_t b=static_cast<std::size_t>(header_.stringOffset)+off,e=static_cast<std::size_t>(header_.stringOffset)+header_.stringSize,p=b;while(p<e&&data_[p])++p;if(p>=e)return false;out.assign(reinterpret_cast<const char*>(&data_[b]),p-b);return true; }
  std::string packPath_,lastError_; std::vector<unsigned char> data_; Header header_; bool valid_;
};

}}} // namespace summit::content::text_resource
