#pragma once
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include "content/v1/common/Sha256.h"
#include "content/v1/text_resource/content/CompiledTextResourceSource.h"
namespace summit { namespace content { namespace gameplay_config {
struct GameplayConfigEntry { std::string section,key,value; std::uint32_t line=0; };
class CompiledGameplayConfigSource {
public:
 explicit CompiledGameplayConfigSource(const char*p):valid_(false){if(p)path_=p;LoadPack();}
 bool IsValid()const{return valid_;} const std::string&LastError()const{return error_;}
 bool Get(const char*path,const char*section,const char*key,std::string&out)const{
  std::vector<GameplayConfigEntry> v;if(!Entries(path,v))return false;const std::string s=Fold(section),k=Fold(key);bool found=false;for(std::size_t i=0;i<v.size();++i)if(Fold(v[i].section.c_str())==s&&Fold(v[i].key.c_str())==k){out=v[i].value;found=true;}return found;
 }
 bool Entries(const char*path,std::vector<GameplayConfigEntry>&out)const{
  if(!valid_)
   return false;
  const std::string wanted=summit::content::text_resource::NormalizeTextResourcePath(path);
  for(std::uint32_t i=0;i<h_.rc;++i){R r;if(!ReadR(i,r))return false;std::string p;if(!Str(r.path,p))return false;if(summit::content::text_resource::NormalizeTextResourcePath(p.c_str())!=wanted)continue;if(r.start>h_.ec||r.count>h_.ec-r.start)return false;out.clear();out.reserve(r.count);for(std::uint32_t n=0;n<r.count;++n){E e;if(!ReadE(r.start+n,e))return false;GameplayConfigEntry x;if(!Str(e.section,x.section)||!Str(e.key,x.key)||!Str(e.value,x.value))return false;x.line=e.line;out.push_back(x);}return true;}return false;
 }
private:
 struct H{std::uint16_t version=0,flags=0;std::uint32_t rc=0,ro=0,rs=0,ec=0,eo=0,es=0,so=0,ss=0,z0=0,z1=0;unsigned char digest[32];}h_;struct R{std::uint32_t path=0,id=0,start=0,count=0,flags=0;};struct E{std::uint32_t section=0,key=0,value=0,line=0;};
 static std::uint16_t U16(const unsigned char*p){return p[0]|(static_cast<std::uint16_t>(p[1])<<8);}static std::uint32_t U32(const unsigned char*p){return p[0]|(static_cast<std::uint32_t>(p[1])<<8)|(static_cast<std::uint32_t>(p[2])<<16)|(static_cast<std::uint32_t>(p[3])<<24);}bool Range(std::uint64_t o,std::uint64_t n)const{return o<=data_.size()&&n<=data_.size()-static_cast<std::size_t>(o);}static std::string Fold(const char*s){std::string x=s?s:"";for(std::size_t i=0;i<x.size();++i)if(x[i]>='A'&&x[i]<='Z')x[i]=static_cast<char>(x[i]-'A'+'a');return x;}
 bool LoadPack(){if(path_.empty()){error_="gameplay config pack path is empty";return false;}FILE*f=std::fopen(path_.c_str(),"rb");if(!f){error_="gameplay config pack not found";return false;}std::fseek(f,0,SEEK_END);long z=std::ftell(f);if(z<80||std::fseek(f,0,SEEK_SET)!=0){std::fclose(f);error_="gameplay config pack too small";return false;}data_.resize(static_cast<std::size_t>(z));if(std::fread(&data_[0],1,data_.size(),f)!=data_.size()){std::fclose(f);error_="cannot read gameplay config pack";return false;}std::fclose(f);const unsigned char*p=&data_[0];if(std::memcmp(p,"SGCF",4)!=0){error_="gameplay config magic mismatch";return false;}h_.version=U16(p+4);h_.flags=U16(p+6);h_.rc=U32(p+8);h_.ro=U32(p+12);h_.rs=U32(p+16);h_.ec=U32(p+20);h_.eo=U32(p+24);h_.es=U32(p+28);h_.so=U32(p+32);h_.ss=U32(p+36);h_.z0=U32(p+40);h_.z1=U32(p+44);std::memcpy(h_.digest,p+48,32);if(h_.version!=1||h_.flags||h_.ro!=80||h_.rs!=20||h_.eo!=80ull+h_.rc*20ull||h_.es!=16||h_.so!=h_.eo+static_cast<std::uint64_t>(h_.ec)*16ull||h_.so+h_.ss!=data_.size()||h_.z0||h_.z1){error_="gameplay config layout is non-canonical";return false;}unsigned char d[32];summit::content::common::Sha256(&data_[80],data_.size()-80,d);if(std::memcmp(d,h_.digest,32)!=0){error_="gameplay config SHA-256 mismatch";return false;}valid_=true;return true;}
 bool ReadR(std::uint32_t i,R&r)const{std::uint64_t o=h_.ro+static_cast<std::uint64_t>(i)*20;if(i>=h_.rc||!Range(o,20))return false;const unsigned char*p=&data_[o];r.path=U32(p);r.id=U32(p+4);r.start=U32(p+8);r.count=U32(p+12);r.flags=U32(p+16);return true;}bool ReadE(std::uint32_t i,E&e)const{std::uint64_t o=h_.eo+static_cast<std::uint64_t>(i)*16;if(i>=h_.ec||!Range(o,16))return false;const unsigned char*p=&data_[o];e.section=U32(p);e.key=U32(p+4);e.value=U32(p+8);e.line=U32(p+12);return true;}bool Str(std::uint32_t o,std::string&x)const{if(o>=h_.ss)return false;std::size_t b=h_.so+o,e=h_.so+h_.ss,p=b;while(p<e&&data_[p])++p;if(p>=e)return false;x.assign(reinterpret_cast<const char*>(&data_[b]),p-b);return true;}
 std::string path_,error_;std::vector<unsigned char>data_;bool valid_;
};
}}}
