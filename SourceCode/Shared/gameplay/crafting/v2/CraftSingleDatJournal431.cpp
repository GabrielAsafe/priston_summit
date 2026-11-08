#include "gameplay/crafting/v2/CraftSingleDatJournal431.h"
#include "gameplay/companions/v2/CompanionTradeExactRecordStore.h"
#include "gameplay/companions/v2/CompanionTradeSha256.h"
#include <algorithm>
#include <atomic>
#include <cstring>
#include <utility>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <cerrno>
#endif

namespace gameplay { namespace crafting { namespace v2 {
#ifdef SUMMIT431_TEST_FAULTS
bool summit431_test_fault_after_replace = false;
bool summit431_test_fault_after_intent = false;
bool summit431_test_fault_after_archive = false;
#endif
namespace {
using Bytes = CraftBytes431;
using gameplay::companions::v2::CompanionTradeExactRecordStore;
using gameplay::companions::v2::TradeApplyResult;
using gameplay::companions::v2::TradeSha256;
constexpr std::size_t kMaxImage=CompanionTradeExactRecordStore::kMaximumRecordBytes;
constexpr std::size_t kMaxPlan=2*kMaxImage+256;
constexpr unsigned kVersion=1;
constexpr char kMagic[8]={'S','M','C','R','F','T','3','1'};
enum class ReadState { Missing, Good, Error };
std::atomic<unsigned> serial{0};
void Put64(Bytes& out,std::uint64_t v) {for(unsigned i=0;i<8;++i)out.push_back(static_cast<std::uint8_t>(v>>(8*i)));}
void Put32(Bytes& out,std::uint32_t v) {for(unsigned i=0;i<4;++i)out.push_back(static_cast<std::uint8_t>(v>>(8*i)));}
bool Get64(const Bytes& x,std::size_t& p,std::uint64_t& v) {
    if(p>x.size()||x.size()-p<8)return false;
    v=0;for(unsigned i=0;i<8;++i)v|=static_cast<std::uint64_t>(x[p++])<<(8*i);return true;
}
bool Get32(const Bytes& x,std::size_t& p,std::uint32_t& v) {
    if(p>x.size()||x.size()-p<4)return false;
    v=0;for(unsigned i=0;i<4;++i)v|=static_cast<std::uint32_t>(x[p++])<<(8*i);return true;
}
bool Valid(const CraftDatPlan431& r) {
    return r.characterId!=0 && r.requestNonce!=0 && r.recipeRevision!=0 &&
           !r.before.empty() && !r.after.empty() &&
           r.before.size()<=kMaxImage && r.after.size()<=kMaxImage && r.before!=r.after;
}
bool Same(const CraftDatPlan431& a,const CraftDatPlan431& b) {
    return a.characterId==b.characterId && a.requestNonce==b.requestNonce &&
           a.recipeRevision==b.recipeRevision && a.before==b.before && a.after==b.after;
}
Bytes Hash(const Bytes& in) {
    auto h=TradeSha256::Hash(in.empty()?nullptr:in.data(),in.size());return Bytes(h.begin(),h.end());
}
Bytes Encode(std::uint32_t type,const CraftDatPlan431& p) {
    Bytes x(kMagic,kMagic+8);Put32(x,kVersion);Put32(x,type);
    Put64(x,p.characterId);Put64(x,p.requestNonce);Put64(x,p.recipeRevision);
    if(type==1) {
        Put32(x,static_cast<std::uint32_t>(p.before.size()));
        Put32(x,static_cast<std::uint32_t>(p.after.size()));
        x.insert(x.end(),p.before.begin(),p.before.end());x.insert(x.end(),p.after.begin(),p.after.end());
    } else {
        const auto before=Hash(p.before),after=Hash(p.after);
        x.insert(x.end(),before.begin(),before.end());x.insert(x.end(),after.begin(),after.end());
    }
    auto digest=Hash(x);x.insert(x.end(),digest.begin(),digest.end());return x;
}
bool Decode(const Bytes& x,unsigned type,CraftDatPlan431& p) {
    const std::size_t prefix=8+4+4+8+8+8;
    if(x.size()<prefix+32 || x.size()>kMaxPlan ||
       !std::equal(x.begin(),x.begin()+8,kMagic))return false;
    Bytes b(x.begin(),x.end()-32);
    const auto digest=Hash(b);
    if(!std::equal(digest.begin(),digest.end(),x.end()-32))return false;
    std::size_t at=8;std::uint32_t version=0,kind=0;
    if(!Get32(b,at,version)||!Get32(b,at,kind)||version!=kVersion||kind!=type ||
       !Get64(b,at,p.characterId)||!Get64(b,at,p.requestNonce)||!Get64(b,at,p.recipeRevision)||
       !p.characterId||!p.requestNonce||!p.recipeRevision)return false;
    if(type==1) {
        std::uint32_t n=0,m=0;
        if(!Get32(b,at,n)||!Get32(b,at,m)||!n||!m||n>kMaxImage||m>kMaxImage ||
           at>b.size() || static_cast<std::uint64_t>(n)+m!=b.size()-at) return false;
        p.before.assign(b.begin()+at,b.begin()+at+n);
        p.after.assign(b.begin()+at+n,b.end());
        return Valid(p);
    }
    return type==2 && b.size()-at==64;
}
std::string Filename(const std::string& root,const char* prefix,std::uint64_t id) {
    return root+((!root.empty() && (root.back()=='/'||root.back()=='\\'))?"":"/")+prefix+std::to_string(id)+".bin";
}
#ifdef _WIN32
std::wstring Wide(const std::string& s) {
    if(s.empty())return {};
    int n=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,s.data(),static_cast<int>(s.size()),nullptr,0);
    if(n<=0)return {};
    std::wstring out(static_cast<std::size_t>(n),L'\0');
    if(MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,s.data(),static_cast<int>(s.size()),&out[0],n)!=n)return {};
    return out;
}
bool SafeDirectory(const std::string& root) {
    auto w=Wide(root);if(w.empty())return false;
    DWORD a=GetFileAttributesW(w.c_str());return a!=INVALID_FILE_ATTRIBUTES &&
        (a&FILE_ATTRIBUTE_DIRECTORY) && !(a&FILE_ATTRIBUTE_REPARSE_POINT);
}
ReadState Read(const std::string& path,Bytes& out) {
    auto w=Wide(path);if(w.empty())return ReadState::Error;
    HANDLE h=CreateFileW(w.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_FLAG_OPEN_REPARSE_POINT,nullptr);
    if(h==INVALID_HANDLE_VALUE)return GetLastError()==ERROR_FILE_NOT_FOUND?ReadState::Missing:ReadState::Error;
    BY_HANDLE_FILE_INFORMATION inf{};
    bool ok=GetFileInformationByHandle(h,&inf) && !(inf.dwFileAttributes&(FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_REPARSE_POINT)) &&
         inf.nNumberOfLinks==1 && !inf.nFileSizeHigh && inf.nFileSizeLow>0 && inf.nFileSizeLow<=kMaxPlan;
    Bytes next;
    if(ok) {next.resize(inf.nFileSizeLow);DWORD done=0;
        ok=ReadFile(h,next.data(),static_cast<DWORD>(next.size()),&done,nullptr)!=0 && done==next.size();}
    if(!CloseHandle(h))ok=false;
    if(ok)out.swap(next);
    return ok?ReadState::Good:ReadState::Error;
}
bool Publish(const std::string& path,const Bytes& data) {
    auto tmp=path+".tmp."+std::to_string(GetCurrentProcessId())+"."+std::to_string(++serial);
    auto wt=Wide(tmp),wp=Wide(path);if(wt.empty()||wp.empty())return false;
    HANDLE h=CreateFileW(wt.c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL|FILE_FLAG_WRITE_THROUGH,nullptr);
    if(h==INVALID_HANDLE_VALUE)return false;
    DWORD done=0;bool ok=WriteFile(h,data.data(),static_cast<DWORD>(data.size()),&done,nullptr)!=0 &&
         done==data.size() && FlushFileBuffers(h)!=0;
    if(!CloseHandle(h))ok=false;
    if(ok)ok=MoveFileExW(wt.c_str(),wp.c_str(),MOVEFILE_WRITE_THROUGH)!=0;
    if(!ok)DeleteFileW(wt.c_str());return ok;
}
bool Delete(const std::string& path) {auto w=Wide(path);return !w.empty() && DeleteFileW(w.c_str())!=0;}
#else
bool SafeDirectory(const std::string& root) {
    struct stat s{};return lstat(root.c_str(),&s)==0 && S_ISDIR(s.st_mode) &&
       s.st_uid==geteuid() && !(s.st_mode&(S_IWOTH|S_IWGRP));
}
bool SyncDir(const std::string& path) {
    auto i=path.find_last_of('/');std::string d=i==std::string::npos?".":path.substr(0,i);
    int fd=open(d.c_str(),O_DIRECTORY|O_RDONLY|O_CLOEXEC|O_NOFOLLOW);
    if(fd<0)return false;
    bool ok=fsync(fd)==0;close(fd);return ok;
}
ReadState Read(const std::string& path,Bytes& out) {
    int fd=open(path.c_str(),O_RDONLY|O_CLOEXEC|O_NOFOLLOW);
    if(fd<0)return errno==ENOENT?ReadState::Missing:ReadState::Error;
    struct stat s{};bool ok=fstat(fd,&s)==0 && S_ISREG(s.st_mode) && s.st_nlink==1 &&
        s.st_size>0 && static_cast<std::uint64_t>(s.st_size)<=kMaxPlan;
    Bytes b;if(ok){b.resize(static_cast<std::size_t>(s.st_size));std::size_t at=0;
        while(at<b.size()){ssize_t n=read(fd,b.data()+at,b.size()-at);
            if(n<0 && errno==EINTR)continue;
            if(n<=0){ok=false;break;}
            at+=static_cast<std::size_t>(n);}}
    if(close(fd)!=0)ok=false;
    if(ok)out.swap(b);
    return ok?ReadState::Good:ReadState::Error;
}
bool Publish(const std::string& path,const Bytes& b) {
    const auto tmp=path+".tmp."+std::to_string(getpid())+"."+std::to_string(++serial);
    int fd=open(tmp.c_str(),O_CREAT|O_EXCL|O_WRONLY|O_CLOEXEC|O_NOFOLLOW,0600);
    if(fd<0)return false;
    bool ok=true;std::size_t at=0;
    while(at<b.size()){ssize_t n=write(fd,b.data()+at,b.size()-at);
        if(n<0&&errno==EINTR)continue;
        if(n<=0){ok=false;break;}
        at+=static_cast<std::size_t>(n);}
    if(ok)ok=fsync(fd)==0;
    if(close(fd)!=0)ok=false;
    if(ok)ok=link(tmp.c_str(),path.c_str())==0;
    if(ok)ok=SyncDir(path);
    unlink(tmp.c_str());return ok;
}
bool Delete(const std::string& path) {
    return unlink(path.c_str())==0 && SyncDir(path);
}
#endif
struct Guard {
    ICraftDatAdmission431& host;std::uint64_t id;bool acquired;
    Guard(ICraftDatAdmission431& h,std::uint64_t i):host(h),id(i),acquired(h.LockCharacter(i)){}
    ~Guard(){if(acquired)host.UnlockCharacter(id);}
};
} // namespace

CraftSingleDatJournal431::CraftSingleDatJournal431(std::string root)
    :root_(std::move(root)),healthy_(SafeDirectory(root_)){}
std::string CraftSingleDatJournal431::Intent(std::uint64_t c) const {return Filename(root_,"pending_",c);}
std::string CraftSingleDatJournal431::Archive(std::uint64_t n) const {return Filename(root_,"archive_",n);}
std::string CraftSingleDatJournal431::Receipt(std::uint64_t n) const {return Filename(root_,"receipt_",n);}

bool CraftSingleDatJournal431::DurableReceipt(std::uint64_t n,std::uint64_t c,std::uint64_t rev) {
    if(!healthy_||!n||!c||!rev)return false;
    Bytes raw,archived;
    const auto rs=Read(Receipt(n),raw),as=Read(Archive(n),archived);
    if(rs==ReadState::Missing && as==ReadState::Missing)return false;
    if(rs!=ReadState::Good||as!=ReadState::Good){Poison();return false;}
    CraftDatPlan431 r,p;
    if(!Decode(raw,2,r)||!Decode(archived,1,p)||p.characterId!=c||p.requestNonce!=n||
       p.recipeRevision!=rev||raw!=Encode(2,p)) {
        Poison();return false;
    }
    return true;
}

CraftCommitState431 CraftSingleDatJournal431::Finish(const CraftDatPlan431& p,const std::string& path) {
    if(path.empty()){Poison();return CraftCommitState431::RecoveryRequired;}
    auto applied=CompanionTradeExactRecordStore::Replace(path,p.before,p.after);
    if(applied==TradeApplyResult::Conflict){Poison();return CraftCommitState431::RecoveryRequired;}
    if(applied==TradeApplyResult::IoUncertain){Poison();return CraftCommitState431::RecoveryRequired;}
    // Deliberate fault injection is compiled ONLY in the native test binary.
#ifdef SUMMIT431_TEST_FAULTS
    if(summit431_test_fault_after_replace) {Poison();return CraftCommitState431::RecoveryRequired;}
#endif
    // Archive is immutable and carries the full before+after images.
    Bytes existing;auto state=Read(Archive(p.requestNonce),existing);
    const Bytes archive=Encode(1,p);
    if(state==ReadState::Good && existing!=archive){Poison();return CraftCommitState431::RecoveryRequired;}
    if(state==ReadState::Error || (state==ReadState::Missing && !Publish(Archive(p.requestNonce),archive))) {
        Poison();return CraftCommitState431::RecoveryRequired;
    }
#ifdef SUMMIT431_TEST_FAULTS
    if(summit431_test_fault_after_archive) {Poison();return CraftCommitState431::RecoveryRequired;}
#endif
    existing.clear();state=Read(Receipt(p.requestNonce),existing);
    const Bytes receipt=Encode(2,p);
    if(state==ReadState::Good && existing!=receipt){Poison();return CraftCommitState431::RecoveryRequired;}
    if(state==ReadState::Error || (state==ReadState::Missing && !Publish(Receipt(p.requestNonce),receipt))) {
        Poison();return CraftCommitState431::RecoveryRequired;
    }
    if(!Delete(Intent(p.characterId))) {
        // Receipt is durable, but recovery must clean up the intent before next mutation.
        Poison();return CraftCommitState431::RecoveryRequired;
    }
    return applied==TradeApplyResult::AlreadyApplied?CraftCommitState431::AlreadyCommitted:CraftCommitState431::Committed;
}

CraftCommitState431 CraftSingleDatJournal431::Commit(const CraftDatPlan431& p,ICraftDatAdmission431& host) {
    if(!healthy_||!Valid(p))return CraftCommitState431::Rejected;
    Guard lock(host,p.characterId);
    if(!lock.acquired || !host.QuiescedAllWriters(p.characterId))return CraftCommitState431::Rejected;
    // The prior session must be sealed even if disk I/O later fails.
    if(!host.SealOldSessionForReconnect(p.characterId))return CraftCommitState431::Rejected;
    const std::string path=host.TrustedDatPath(p.characterId);
    if(path.empty())return CraftCommitState431::Rejected;
    Bytes raw;auto pending=Read(Intent(p.characterId),raw);
    if(pending==ReadState::Error){Poison();return CraftCommitState431::RecoveryRequired;}
    if(pending==ReadState::Good) {
        CraftDatPlan431 saved;
        if(!Decode(raw,1,saved)||!Same(saved,p)){Poison();return CraftCommitState431::RecoveryRequired;}
        return Finish(saved,path);
    }
    Bytes receipt;auto rs=Read(Receipt(p.requestNonce),receipt);
    Bytes archive;auto as=Read(Archive(p.requestNonce),archive);
    if(rs==ReadState::Good || as==ReadState::Good) {
        if(rs==ReadState::Good && as==ReadState::Good &&
           receipt==Encode(2,p) && archive==Encode(1,p))return CraftCommitState431::AlreadyCommitted;
        Poison();return CraftCommitState431::RecoveryRequired;
    }
    if(rs==ReadState::Error||as==ReadState::Error){Poison();return CraftCommitState431::RecoveryRequired;}
    // A single character has at most one pending intent; publication is immutable.
    if(!Publish(Intent(p.characterId),Encode(1,p))) {Poison();return CraftCommitState431::RecoveryRequired;}
#ifdef SUMMIT431_TEST_FAULTS
    if(summit431_test_fault_after_intent) {Poison();return CraftCommitState431::RecoveryRequired;}
#endif
    return Finish(p,path);
}

CraftCommitState431 CraftSingleDatJournal431::Recover(std::uint64_t id,ICraftDatAdmission431& host) {
    if(!healthy_||!id)return CraftCommitState431::Rejected;
    Guard lock(host,id);
    if(!lock.acquired || !host.QuiescedAllWriters(id))return CraftCommitState431::Rejected;
    // Hold the authoritative lease before examining pending recovery state.
    Bytes raw;const auto state=Read(Intent(id),raw);
    if(state==ReadState::Missing)return CraftCommitState431::NoPending;
    if(state==ReadState::Error){Poison();return CraftCommitState431::RecoveryRequired;}
    // Seal before a pending transaction may replace its DAT. Never let an old
    // client save overwrite a recovered record.
    if(!host.SealOldSessionForReconnect(id))return CraftCommitState431::Rejected;
    CraftDatPlan431 p;
    if(!Decode(raw,1,p)||p.characterId!=id){Poison();return CraftCommitState431::RecoveryRequired;}
    return Finish(p,host.TrustedDatPath(id));
}
}}} // namespace gameplay::crafting::v2
