#pragma once
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include "content/v1/common/Sha256.h"
#include "content/v1/text_resource/content/CompiledTextResourceSource.h"

namespace summit { namespace content { namespace definition {

enum DefinitionKind { DefinitionKindUnknown=0, DefinitionKindMonster=1, DefinitionKindNpc=2, DefinitionKindOpenItem=3 };
struct CompiledDirective { std::string command; std::string values; std::uint32_t flags=0; };
struct CompiledDefinition { std::string logicalPath; std::string contentId; std::uint32_t kind=0; std::vector<CompiledDirective> directives; };
struct DefinitionEntry { std::string logicalPath; std::string contentId; std::uint32_t kind=0; };

class CompiledDefinitionSource {
public:
 explicit CompiledDefinitionSource(const char*path):valid_(false){if(path)path_=path;LoadPack();}
 bool IsValid()const{return valid_;}
 const std::string& LastError()const{return error_;}
 bool Load(const char*logicalPath,CompiledDefinition&out,std::string*error=0)const{
  if(!valid_)return Fail(error,error_.c_str());
  const std::string wanted=summit::content::text_resource::NormalizeTextResourcePath(logicalPath);
  for(std::uint32_t i=0;i<h_.recordCount;++i){Record r;if(!ReadRecord(i,r))return Fail(error,"compiled definition record is corrupt");std::string path;if(!ReadString(r.pathOffset,path))return Fail(error,"compiled definition path is corrupt");if(summit::content::text_resource::NormalizeTextResourcePath(path.c_str())!=wanted)continue;
   out=CompiledDefinition();out.logicalPath=path;out.kind=r.kind;if(!ReadString(r.idOffset,out.contentId))return Fail(error,"compiled definition id is corrupt");
   if(r.directiveStart>h_.directiveCount||r.directiveCount>h_.directiveCount-r.directiveStart)return Fail(error,"compiled definition directive range is invalid");
   out.directives.reserve(r.directiveCount);for(std::uint32_t n=0;n<r.directiveCount;++n){Directive d;if(!ReadDirective(r.directiveStart+n,d))return Fail(error,"compiled definition directive is corrupt");CompiledDirective x;if(!ReadString(d.commandOffset,x.command)||!ReadString(d.valuesOffset,x.values))return Fail(error,"compiled definition directive string is corrupt");x.flags=d.flags;out.directives.push_back(x);} if(error)error->clear();return true;
  } return Fail(error,"definition path is not present in compiled pack");
 }
 bool List(const char*prefix,const char*extension,bool directOnly,std::vector<DefinitionEntry>&out,std::uint32_t kindFilter=0,std::string*error=0)const{
  if(!valid_)
   return Fail(error,error_.c_str());
  const std::string base=summit::content::text_resource::NormalizeTextResourcePath(prefix);
  std::string ext=extension?extension:"";
  for(std::size_t i=0;i<ext.size();++i)
   if(ext[i]>='A'&&ext[i]<='Z')
    ext[i]=static_cast<char>(ext[i]-'A'+'a');
  out.clear();
  for(std::uint32_t i=0;i<h_.recordCount;++i){Record r;if(!ReadRecord(i,r))return Fail(error,"compiled definition record is corrupt");if(kindFilter&&r.kind!=kindFilter)continue;DefinitionEntry e;if(!ReadString(r.pathOffset,e.logicalPath)||!ReadString(r.idOffset,e.contentId))return Fail(error,"compiled definition identity is corrupt");e.kind=r.kind;const std::string n=summit::content::text_resource::NormalizeTextResourcePath(e.logicalPath.c_str());if(!base.empty()){if(n.size()<base.size()||n.compare(0,base.size(),base)!=0)continue;if(n.size()>base.size()&&n[base.size()]!='/')continue;}std::size_t pos=base.empty()?0:base.size();if(pos<n.size()&&n[pos]=='/')++pos;const std::string rel=n.substr(pos);if(directOnly&&rel.find('/')!=std::string::npos)continue;if(!ext.empty()&&(n.size()<ext.size()||n.compare(n.size()-ext.size(),ext.size(),ext)!=0))continue;out.push_back(e);}
  std::sort(out.begin(),out.end(),[](const DefinitionEntry&a,const DefinitionEntry&b){return summit::content::text_resource::NormalizeTextResourcePath(a.logicalPath.c_str())<summit::content::text_resource::NormalizeTextResourcePath(b.logicalPath.c_str());});if(error)error->clear();return true;
 }
private:
 struct Header{std::uint16_t version=0,flags=0;std::uint32_t recordCount=0,recordOffset=0,recordSize=0,directiveCount=0,directiveOffset=0,directiveSize=0,stringOffset=0,stringSize=0,res0=0,res1=0;unsigned char digest[32];};
 struct Record{std::uint32_t pathOffset=0,idOffset=0,kind=0,directiveStart=0,directiveCount=0;};
 struct Directive{std::uint32_t commandOffset=0,valuesOffset=0,flags=0;};
 static std::uint16_t U16(const unsigned char*p){return static_cast<std::uint16_t>(p[0]|(static_cast<std::uint16_t>(p[1])<<8));}
 static std::uint32_t U32(const unsigned char*p){return static_cast<std::uint32_t>(p[0])|(static_cast<std::uint32_t>(p[1])<<8)|(static_cast<std::uint32_t>(p[2])<<16)|(static_cast<std::uint32_t>(p[3])<<24);}
 bool Range(std::uint64_t o,std::uint64_t n)const{return o<=data_.size()&&n<=data_.size()-static_cast<std::size_t>(o);}
 static bool Fail(std::string*e,const char*m){if(e)*e=m?m:"compiled definition failure";return false;}
 bool LoadPack(){valid_=false;data_.clear();error_.clear();if(path_.empty()){error_="compiled definition pack path is empty";return false;}FILE*fp=std::fopen(path_.c_str(),"rb");if(!fp){error_="compiled definition pack not found";return false;}std::fseek(fp,0,SEEK_END);long sz=std::ftell(fp);if(sz<80||std::fseek(fp,0,SEEK_SET)!=0){std::fclose(fp);error_="compiled definition pack is too small";return false;}data_.resize(static_cast<std::size_t>(sz));if(std::fread(&data_[0],1,data_.size(),fp)!=data_.size()){std::fclose(fp);error_="cannot read compiled definition pack";return false;}std::fclose(fp);const unsigned char*p=&data_[0];if(std::memcmp(p,"SCFG",4)!=0){error_="compiled definition magic mismatch";return false;}h_.version=U16(p+4);h_.flags=U16(p+6);if(h_.version!=1){error_="unsupported compiled definition version";return false;}h_.recordCount=U32(p+8);h_.recordOffset=U32(p+12);h_.recordSize=U32(p+16);h_.directiveCount=U32(p+20);h_.directiveOffset=U32(p+24);h_.directiveSize=U32(p+28);h_.stringOffset=U32(p+32);h_.stringSize=U32(p+36);h_.res0=U32(p+40);h_.res1=U32(p+44);std::memcpy(h_.digest,p+48,32);
  const std::uint64_t directives=80ull+static_cast<std::uint64_t>(h_.recordCount)*20ull,strings=directives+static_cast<std::uint64_t>(h_.directiveCount)*12ull,end=strings+h_.stringSize;
  if(h_.recordOffset!=80||h_.recordSize!=20||h_.directiveOffset!=directives||h_.directiveSize!=12||h_.stringOffset!=strings||end!=data_.size()||h_.res0||h_.res1){error_="compiled definition layout is non-canonical";return false;}unsigned char digest[32];summit::content::common::Sha256(&data_[80],data_.size()-80,digest);if(std::memcmp(digest,h_.digest,32)!=0){error_="compiled definition SHA-256 mismatch";return false;}
  for(std::uint32_t i=0;i<h_.recordCount;++i){Record r;std::string s;if(!ReadRecord(i,r)||!ReadString(r.pathOffset,s)||!ReadString(r.idOffset,s)||r.directiveStart>h_.directiveCount||r.directiveCount>h_.directiveCount-r.directiveStart){error_="compiled definition record is invalid";return false;}}valid_=true;return true;}
 bool ReadRecord(std::uint32_t i,Record&r)const{if(i>=h_.recordCount)return false;std::uint64_t o=h_.recordOffset+static_cast<std::uint64_t>(i)*20;if(!Range(o,20))return false;const unsigned char*p=&data_[static_cast<std::size_t>(o)];r.pathOffset=U32(p);r.idOffset=U32(p+4);r.kind=U32(p+8);r.directiveStart=U32(p+12);r.directiveCount=U32(p+16);return true;}
 bool ReadDirective(std::uint32_t i,Directive&d)const{if(i>=h_.directiveCount)return false;std::uint64_t o=h_.directiveOffset+static_cast<std::uint64_t>(i)*12;if(!Range(o,12))return false;const unsigned char*p=&data_[static_cast<std::size_t>(o)];d.commandOffset=U32(p);d.valuesOffset=U32(p+4);d.flags=U32(p+8);return true;}
 bool ReadString(std::uint32_t o,std::string&out)const{if(o>=h_.stringSize)return false;std::size_t b=h_.stringOffset+o,e=h_.stringOffset+h_.stringSize,p=b;while(p<e&&data_[p])++p;if(p>=e)return false;out.assign(reinterpret_cast<const char*>(&data_[b]),p-b);return true;}
 std::string path_,error_;std::vector<unsigned char>data_;Header h_;bool valid_;
};

}}}
