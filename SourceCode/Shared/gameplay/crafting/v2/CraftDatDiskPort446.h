#pragma once
// SUMMIT 446 — one-character durable craft journal with full before/after DAT
// images, CAS replace, archived proof, idempotent receipts, pre-login recovery.
// The authenticated game adapter MUST hold a single-participant lease and
// session save freeze for the ENTIRE coordinator call, including receipt seal.
#include "gameplay/crafting/v2/CraftCommit443.h"
#include "gameplay/companions/v2/CompanionTradeExactRecordStore.h"
#include "gameplay/companions/v2/CompanionTradeSha256.h"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#endif
namespace summit446 {
using Bytes=std::vector<std::uint8_t>;
namespace disk {
inline void Put64(Bytes& b,std::uint64_t v){for(int i=0;i<8;++i)b.push_back(static_cast<std::uint8_t>(v>>(8*i)));}
inline void Put32(Bytes& b,std::uint32_t v){for(int i=0;i<4;++i)b.push_back(static_cast<std::uint8_t>(v>>(8*i)));}
inline bool Get32(const Bytes& b,std::size_t& p,std::uint32_t& v){
 if(p>b.size() || b.size()-p<4)return false;
 v=0;for(int i=0;i<4;++i)v|=std::uint32_t(b[p++])<<(8*i);
 return true;
}
inline bool Get64(const Bytes& b,std::size_t& p,std::uint64_t& v){
 if(p>b.size() || b.size()-p<8)return false;
 v=0;for(int i=0;i<8;++i)v|=std::uint64_t(b[p++])<<(8*i);
 return true;
}
inline void PutString(Bytes& b,const std::string& s){Put32(b,static_cast<std::uint32_t>(s.size()));b.insert(b.end(),s.begin(),s.end());}
inline bool GetString(const Bytes& b,std::size_t& p,std::string& s,std::size_t bound){
 std::uint32_t n=0;if(!Get32(b,p,n)||n>bound||n>b.size()-p)return false;
 s.assign(reinterpret_cast<const char*>(b.data()+p),n);p+=n;return true;
}
inline void PutBytes(Bytes& b,const Bytes& raw){Put32(b,static_cast<std::uint32_t>(raw.size()));b.insert(b.end(),raw.begin(),raw.end());}
inline bool GetBytes(const Bytes& b,std::size_t& p,Bytes& v,std::size_t bound){
 std::uint32_t n=0;if(!Get32(b,p,n)||n>bound||n>b.size()-p)return false;
 v.assign(b.begin()+p,b.begin()+p+n);p+=n;return true;
}
inline void Digest(Bytes& b){
 const auto hash=gameplay::companions::v2::TradeSha256::Hash(b.data(),b.size());
 b.insert(b.end(),hash.begin(),hash.end());
}
inline bool CheckDigest(const Bytes& b) {
 if(b.size()<32)return false;
 const auto hash=gameplay::companions::v2::TradeSha256::Hash(b.data(),b.size()-32);
 std::uint8_t difference=0;
 for(int i=0;i<32;++i)difference|=hash[i]^b[b.size()-32+i];
 return difference==0;
}
inline Bytes Encode(const summit443::CraftRequest& req,const summit443::Image& image){
 Bytes out;Put32(out,0x36494353u); // SCI6 version 1
 Put64(out,req.character);Put64(out,req.sessionGeneration);
 PutString(out,req.requestId);PutString(out,req.recipeCode);
 PutBytes(out,image.before);PutBytes(out,image.after);
 Digest(out);return out;
}
inline bool Decode(const Bytes& b,summit443::CraftRequest& req,summit443::Image& image){
 if(b.size()<90||b.size()>16u*1024u*1024u||!CheckDigest(b))return false;
 std::size_t p=0;std::uint32_t magic=0;
 if(!Get32(b,p,magic)||magic!=0x36494353u||
    !Get64(b,p,req.character)||!Get64(b,p,req.sessionGeneration)||
    !GetString(b,p,req.requestId,48)||!GetString(b,p,req.recipeCode,16)||
    !GetBytes(b,p,image.before,8u*1024u*1024u)||
    !GetBytes(b,p,image.after,8u*1024u*1024u)||
    p!=b.size()-32||!summit443::ValidRequest(req))return false;
 image.authenticatedOwner=true;image.verifiedRecipe=true;image.verifiedMaterials=true;
 image.nativeOutputCreated=true;image.provenConservation=true;image.capacityOk=true;
 return summit443::ValidImage(image);
}
inline Bytes ReceiptBytes(const summit443::CraftRequest& req,const Bytes& archive){
 Bytes out;Put32(out,0x36524353u);Put64(out,req.character);Put64(out,req.sessionGeneration);
 PutString(out,req.requestId);PutString(out,req.recipeCode);
 const auto h=gameplay::companions::v2::TradeSha256::Hash(archive.data(),archive.size());
 out.insert(out.end(),h.begin(),h.end());Digest(out);return out;
}
inline bool ReceiptValid(const Bytes& b,const summit443::CraftRequest& req,const Bytes& archive){
 return b==ReceiptBytes(req,archive);
}
inline std::string Folder(const std::string& p){auto n=p.find_last_of("/\\");return n==std::string::npos?".":p.substr(0,n);}
inline bool DirectorySafe(const std::string& p){
#ifdef _WIN32
 auto attributes=GetFileAttributesA(p.c_str());return attributes!=INVALID_FILE_ATTRIBUTES &&
  (attributes&FILE_ATTRIBUTE_DIRECTORY) && !(attributes&FILE_ATTRIBUTE_REPARSE_POINT);
#else
 struct stat st{};return lstat(p.c_str(),&st)==0&&S_ISDIR(st.st_mode)&&
        st.st_uid==geteuid() && !(st.st_mode&(S_IWOTH|S_IWGRP));
#endif
}
// Read returns 0 absent, 1 valid, -1 corrupt/inaccessible. No silent errors.
inline int ReadFile(const std::string& path,Bytes& out){
#ifdef _WIN32
 HANDLE h=CreateFileA(path.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,
             FILE_FLAG_OPEN_REPARSE_POINT,nullptr);
 if(h==INVALID_HANDLE_VALUE)return GetLastError()==ERROR_FILE_NOT_FOUND?0:-1;
 BY_HANDLE_FILE_INFORMATION st{};
 if(!GetFileInformationByHandle(h,&st)||st.nNumberOfLinks!=1||
    (st.dwFileAttributes&(FILE_ATTRIBUTE_REPARSE_POINT|FILE_ATTRIBUTE_DIRECTORY))||
    st.nFileSizeHigh||st.nFileSizeLow>16u*1024u*1024u||st.nFileSizeLow<32){CloseHandle(h);return -1;}
 Bytes b(st.nFileSizeLow);DWORD got=0;
 const bool ok=::ReadFile(h,b.data(),static_cast<DWORD>(b.size()),&got,nullptr)!=0&&got==b.size();
 CloseHandle(h);if(!ok)return -1;out.swap(b);return 1;
#else
 int fd=open(path.c_str(),O_RDONLY|O_NOFOLLOW|O_CLOEXEC);
 if(fd<0)return errno==ENOENT?0:-1;
 struct stat st{};bool ok=fstat(fd,&st)==0&&S_ISREG(st.st_mode)&&st.st_nlink==1&&
       st.st_size>=32&&st.st_size<=16*1024*1024;
 Bytes b;if(ok){b.resize(static_cast<std::size_t>(st.st_size));std::size_t pos=0;
  while(pos<b.size()){auto n=read(fd,b.data()+pos,b.size()-pos);
   if(n<0&&errno==EINTR)continue;
   if(n<=0){ok=false;break;}pos+=static_cast<std::size_t>(n);}}
 if(close(fd)!=0)ok=false;
 if(!ok)return -1;
 out.swap(b);return 1;
#endif
}
inline bool Publish(const std::string& path,const Bytes& b){
 if(b.empty()||b.size()>16u*1024u*1024u||!DirectorySafe(Folder(path)))return false;
#ifdef _WIN32
 static volatile LONG serial=0;
 const auto tmp=path+".tmp."+std::to_string(GetCurrentProcessId())+"."+
        std::to_string(InterlockedIncrement(&serial));
 HANDLE h=CreateFileA(tmp.c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,
           FILE_ATTRIBUTE_NORMAL|FILE_FLAG_WRITE_THROUGH,nullptr);
 if(h==INVALID_HANDLE_VALUE)return false;
 DWORD done=0;bool ok=WriteFile(h,b.data(),static_cast<DWORD>(b.size()),&done,nullptr)!=0&&
                      done==b.size()&&FlushFileBuffers(h)!=0;
 if(!CloseHandle(h))ok=false;
 if(ok)ok=MoveFileExA(tmp.c_str(),path.c_str(),MOVEFILE_WRITE_THROUGH)!=0;
 if(!ok)DeleteFileA(tmp.c_str());return ok;
#else
 static unsigned serial=0;
 const auto tmp=path+".tmp."+std::to_string(getpid())+"."+std::to_string(++serial);
 int fd=open(tmp.c_str(),O_WRONLY|O_CREAT|O_EXCL|O_NOFOLLOW|O_CLOEXEC,0600);
 if(fd<0)return false;
 std::size_t done=0;bool ok=true;
 while(done<b.size()){auto n=write(fd,b.data()+done,b.size()-done);
  if(n<0&&errno==EINTR)continue;
  if(n<=0){ok=false;break;}done+=static_cast<std::size_t>(n);}
 if(done!=b.size()||fsync(fd)!=0)ok=false;
 if(close(fd)!=0)ok=false;
 if(ok)ok=link(tmp.c_str(),path.c_str())==0;
 if(ok){int dfd=open(Folder(path).c_str(),O_RDONLY|O_DIRECTORY|O_NOFOLLOW|O_CLOEXEC);
  ok=dfd>=0&&fsync(dfd)==0;if(dfd>=0)close(dfd);}
 unlink(tmp.c_str());return ok;
#endif
}
inline bool Remove(const std::string& path){
#ifdef _WIN32
 return DeleteFileA(path.c_str())!=0;
#else
 if(unlink(path.c_str())!=0)return false;
 int fd=open(Folder(path).c_str(),O_RDONLY|O_DIRECTORY|O_NOFOLLOW|O_CLOEXEC);
 if(fd<0)return false;
 const bool ok=fsync(fd)==0;close(fd);return ok;
#endif
}
} // disk

class ICraftHost446 {
public:
 virtual ~ICraftHost446(){}
 virtual bool Reserve(std::uint64_t)=0;
 virtual void Release(std::uint64_t)=0;
 virtual bool Session(const summit443::CraftRequest&)=0;
 virtual bool Prepare(const summit443::CraftRequest&,summit443::Image&)=0;
 virtual bool Quarantine(std::uint64_t)=0;
};

class CraftDatDiskPort446 final:public summit443::ICraftDatPort443 {
 std::string root_,datPath_;
 ICraftHost446& host_;
 bool healthy_;
 std::string Intent(std::uint64_t c)const{return root_+"/intent_"+std::to_string(c)+".bin";}
 std::string Archive(const summit443::CraftRequest& r)const{
  return root_+"/archive_"+std::to_string(r.character)+"_"+r.requestId+".bin";
 }
 std::string ReceiptName(const summit443::CraftRequest& r)const{
  return root_+"/receipt_"+std::to_string(r.character)+"_"+r.requestId+".bin";
 }
 public:
 CraftDatDiskPort446(std::string root,std::string path,ICraftHost446& host):
  root_(std::move(root)),datPath_(std::move(path)),host_(host),healthy_(disk::DirectorySafe(root_)){}
 bool Healthy()const{return healthy_;}
 bool LockCharacter(std::uint64_t c)override{return healthy_&&c&&host_.Reserve(c);}
 void UnlockCharacter(std::uint64_t c)override{host_.Release(c);}
 bool SessionAuthentic(const summit443::CraftRequest& r)override{return healthy_&&host_.Session(r);}
 summit443::ReceiptStatus Receipt(const summit443::CraftRequest& r)override{
  if(!healthy_)return summit443::ReceiptStatus::Corrupt;
  Bytes b,archive;
  int state=disk::ReadFile(ReceiptName(r),b);
  if(state==0)return summit443::ReceiptStatus::Absent;
  if(state!=1||disk::ReadFile(Archive(r),archive)!=1){healthy_=false;return summit443::ReceiptStatus::Corrupt;}
  summit443::CraftRequest recorded;summit443::Image image;
  if(!disk::Decode(archive,recorded,image)||!disk::ReceiptValid(b,recorded,archive)){
   healthy_=false;return summit443::ReceiptStatus::Corrupt;
  }
  if(recorded.character==r.character && recorded.requestId==r.requestId &&
     recorded.recipeCode==r.recipeCode && recorded.sessionGeneration==r.sessionGeneration)
    return summit443::ReceiptStatus::SameCommitted;
  return summit443::ReceiptStatus::DifferentCommitted;
 }
 summit443::Pending PendingFor(std::uint64_t c)override{
  summit443::Pending p;
  if(!healthy_){p.exists=true;return p;}
  Bytes raw;int state=disk::ReadFile(Intent(c),raw);
  if(state==0)return p;
  p.exists=true;
  if(state!=1||!disk::Decode(raw,p.request,p.image)||p.request.character!=c){healthy_=false;}
  return p;
 }
 bool Prepare(const summit443::CraftRequest& r,summit443::Image& img)override{
  return healthy_&&host_.Prepare(r,img);
 }
 bool SealIntent(const summit443::CraftRequest& r,const summit443::Image& img)override{
  if(!healthy_||!summit443::ValidRequest(r)||!summit443::ValidImage(img))return false;
  // Seal the old client session BEFORE publishing the intent or changing DAT.
  // If the write becomes uncertain, no queued stale client save can overwrite
  // a committed item after this lease is released.
  if(!host_.Quarantine(r.character))return false;
  const Bytes raw=disk::Encode(r,img);
  if(!disk::Publish(Intent(r.character),raw)){healthy_=false;return false;}
  return true;
 }
 summit443::StoreWrite WriteExactDat(const summit443::CraftRequest& r,
                                       const summit443::Image& img)override{
  if(!healthy_||!summit443::ValidRequest(r)||!summit443::ValidImage(img))return summit443::StoreWrite::Uncertain;
  const auto state=gameplay::companions::v2::CompanionTradeExactRecordStore::Replace(
                datPath_,img.before,img.after);
  if(state==gameplay::companions::v2::TradeApplyResult::AppliedDurably)return summit443::StoreWrite::Durable;
  if(state==gameplay::companions::v2::TradeApplyResult::AlreadyApplied)return summit443::StoreWrite::AlreadyApplied;
  return state==gameplay::companions::v2::TradeApplyResult::Conflict ?
      summit443::StoreWrite::Conflict:summit443::StoreWrite::Uncertain;
 }
 bool SealReceipt(const summit443::CraftRequest& r,const summit443::Image& img)override{
  if(!healthy_)return false;
  Bytes original; if(disk::ReadFile(Intent(r.character),original)!=1||
      original!=disk::Encode(r,img)){healthy_=false;return false;}
  const std::string archivePath=Archive(r);
  Bytes existing;int state=disk::ReadFile(archivePath,existing);
  if(state==0){
   if(!disk::Publish(archivePath,original)){healthy_=false;return false;}
  } else if(state!=1||existing!=original){healthy_=false;return false;}
  const Bytes expected=disk::ReceiptBytes(r,original);
  state=disk::ReadFile(ReceiptName(r),existing);
  if(state==0){
   if(!disk::Publish(ReceiptName(r),expected)){healthy_=false;return false;}
  }else if(state!=1||existing!=expected){healthy_=false;return false;}
  return true;
 }
 bool QuarantineOldSession(std::uint64_t c)override{return healthy_&&host_.Quarantine(c);}
 void CleanCommittedIntent(const summit443::CraftRequest& r)override{
  if(!healthy_ || Receipt(r)!=summit443::ReceiptStatus::SameCommitted)return;
  const auto pending=PendingFor(r.character);
  if(!pending.exists || !healthy_ || pending.request.character!=r.character ||
     pending.request.requestId!=r.requestId ||
     pending.request.recipeCode!=r.recipeCode)return;
  Bytes old;
  if(disk::ReadFile(Intent(r.character),old)==1 &&
     old==disk::Encode(pending.request,pending.image))
    (void)disk::Remove(Intent(r.character));
 }
};
} // summit446
