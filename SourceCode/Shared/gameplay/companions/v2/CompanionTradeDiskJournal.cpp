// SUMMIT322: cross-platform immutable write-ahead journal for Companion V2.
#include "gameplay/companions/v2/CompanionTradeDiskJournal.h"
#include <algorithm>
#include <atomic>
#include <utility>
#include <cerrno>
#include <cstring>
#include <limits>
#include <string>
#include <vector>
#ifdef _WIN32
# define NOMINMAX
# include <windows.h>
#else
# include <fcntl.h>
# include <sys/stat.h>
# include <sys/types.h>
# include <unistd.h>
#endif

namespace gameplay { namespace companions { namespace v2 {
namespace {
using Bytes = std::vector<std::uint8_t>;
constexpr std::size_t kMaxImage = 8u * 1024u * 1024u;
constexpr std::size_t kMaxRecord = 32u * 1024u * 1024u;
constexpr std::uint32_t kVersion = 1;
constexpr std::uint32_t kIntent = 1;
constexpr std::uint32_t kReceipt = 2;
constexpr char kMagic[8] = {'S','M','T','R','D','J','0','1'};

void Put32(Bytes& b, std::uint32_t v) { for(int i=0;i<4;++i) b.push_back(static_cast<std::uint8_t>(v>>(i*8))); }
void Put64(Bytes& b, std::uint64_t v) { for(int i=0;i<8;++i) b.push_back(static_cast<std::uint8_t>(v>>(i*8))); }
bool Get32(const Bytes& b, std::size_t& p, std::uint32_t& v) {
    if(p>b.size() || b.size()-p<4) return false;
    v=0; for(int i=0;i<4;++i) v |= std::uint32_t(b[p++])<<(8*i);
    return true;
}
bool Get64(const Bytes& b, std::size_t& p, std::uint64_t& v) {
    if(p>b.size() || b.size()-p<8) return false;
    v=0; for(int i=0;i<8;++i) v |= std::uint64_t(b[p++])<<(8*i);
    return true;
}
std::uint64_t Fingerprint(const Bytes& b) {
    std::uint64_t h=UINT64_C(14695981039346656037);
    for(auto c:b) {h^=c;h*=UINT64_C(1099511628211);} return h;
}
bool PutImage(Bytes& b, const Bytes& v) {
    if(v.empty() || v.size()>kMaxImage) return false;
    Put32(b,static_cast<std::uint32_t>(v.size()));b.insert(b.end(),v.begin(),v.end());return true;
}
bool GetImage(const Bytes& b, std::size_t& p, Bytes& v) {
    std::uint32_t n=0;
    if(!Get32(b,p,n) || !n || n>kMaxImage || n>b.size()-p) return false;
    v.assign(b.begin()+p,b.begin()+p+n);p+=n;return true;
}
void PutOffer(Bytes& b, const CompanionTradeOffer& o) {
    Put64(b,o.tradeNonce); Put64(b,o.authenticatedOwner);Put64(b,o.companionCharacter);
    Put64(b,o.offerDigest);Put32(b,o.offerRevision);
}
bool GetOffer(const Bytes& b,std::size_t& p,CompanionTradeOffer& o) {
    return Get64(b,p,o.tradeNonce) && Get64(b,p,o.authenticatedOwner) &&
           Get64(b,p,o.companionCharacter) && Get64(b,p,o.offerDigest) &&
           Get32(b,p,o.offerRevision) && o.tradeNonce && o.authenticatedOwner &&
           o.companionCharacter && o.authenticatedOwner!=o.companionCharacter &&
           o.offerDigest && o.offerRevision;
}
Bytes Wrap(std::uint32_t type,const Bytes& payload) {
    Bytes out(kMagic,kMagic+8);
    Put32(out,kVersion);Put32(out,type);Put32(out,static_cast<std::uint32_t>(payload.size()));
    Put64(out,Fingerprint(payload));out.insert(out.end(),payload.begin(),payload.end());return out;
}
bool Unwrap(const Bytes& data,std::uint32_t type,Bytes& payload) {
    if(data.size()<28 || data.size()>kMaxRecord ||
       !std::equal(data.begin(),data.begin()+8,kMagic)) return false;
    std::size_t p=8;std::uint32_t version=0,kind=0,size=0;std::uint64_t check=0;
    if(!Get32(data,p,version)||!Get32(data,p,kind)||!Get32(data,p,size)||
       !Get64(data,p,check)|| version!=kVersion||kind!=type||size!=data.size()-p)
        return false;
    payload.assign(data.begin()+p,data.end());return check==Fingerprint(payload);
}
bool EncodePlan(const CompanionTradePrepared& p,Bytes& out) {
    const auto& o=p.offer;
    if(!o.tradeNonce || !o.authenticatedOwner || !o.companionCharacter ||
       o.authenticatedOwner==o.companionCharacter || !o.offerDigest || !o.offerRevision) return false;
    Bytes body; PutOffer(body,o);
    if(!PutImage(body,p.ownerBefore)||!PutImage(body,p.ownerAfter)||
       !PutImage(body,p.companionBefore)||!PutImage(body,p.companionAfter)||
       body.size()>kMaxRecord-28) return false;
    out=Wrap(kIntent,body);return true;
}
bool DecodePlan(const Bytes& data,CompanionTradePrepared& plan) {
    Bytes b;if(!Unwrap(data,kIntent,b)) return false;
    std::size_t p=0;
    return GetOffer(b,p,plan.offer) && GetImage(b,p,plan.ownerBefore) &&
           GetImage(b,p,plan.ownerAfter) && GetImage(b,p,plan.companionBefore) &&
           GetImage(b,p,plan.companionAfter) && p==b.size();
}
Bytes EncodeReceipt(const CompanionTradeOffer& offer) {
    Bytes b;PutOffer(b,offer);return Wrap(kReceipt,b);
}
bool DecodeReceipt(const Bytes& b,CompanionTradeOffer& offer) {
    Bytes p;if(!Unwrap(b,kReceipt,p)) return false;
    std::size_t pos=0;return GetOffer(p,pos,offer) && pos==p.size();
}
std::string Name(const std::string& root,const char* type,std::uint64_t a,std::uint64_t b=0) {
    std::string out=root;
    if(!out.empty() && out.back()!='/' && out.back()!='\\') out+='/';
    out+=type;out+=std::to_string(a);
    if(b) {out+='_';out+=std::to_string(b);}
    return out+".bin";
}
// Outcome must distinguish ENOENT from broken final records.
enum class ReadResult {Missing,Valid,Failure};
#ifdef _WIN32
std::wstring Wide(const std::string& path) {
    if(path.empty()) return {};
    int n=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,path.data(),static_cast<int>(path.size()),nullptr,0);
    if(n<=0) return {};
    std::wstring out(static_cast<std::size_t>(n),L'\0');
    if(MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,path.data(),static_cast<int>(path.size()),&out[0],n)!=n)
        return {};
    return out;
}
bool IsDirectorySafe(const std::string& root) {
    const auto w=Wide(root);if(w.empty()) return false;
    DWORD attrs=GetFileAttributesW(w.c_str());
    return attrs!=INVALID_FILE_ATTRIBUTES && (attrs&FILE_ATTRIBUTE_DIRECTORY) &&
           !(attrs&FILE_ATTRIBUTE_REPARSE_POINT);
}
ReadResult ReadExact(const std::string& path,Bytes& result) {
    const auto w=Wide(path); if(w.empty()) return ReadResult::Failure;
    HANDLE h=CreateFileW(w.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,
                         FILE_FLAG_OPEN_REPARSE_POINT,nullptr);
    if(h==INVALID_HANDLE_VALUE)
        return GetLastError()==ERROR_FILE_NOT_FOUND?ReadResult::Missing:ReadResult::Failure;
    BY_HANDLE_FILE_INFORMATION info{};
    if(!GetFileInformationByHandle(h,&info) || (info.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT) ||
       info.nFileSizeHigh!=0 || info.nFileSizeLow>kMaxRecord) {CloseHandle(h);return ReadResult::Failure;}
    result.resize(info.nFileSizeLow);DWORD read=0;
    bool ok=result.empty()|| (ReadFile(h,result.data(),static_cast<DWORD>(result.size()),&read,nullptr) && read==result.size());
    CloseHandle(h);return ok?ReadResult::Valid:ReadResult::Failure;
}
bool Publish(const std::string& path,const Bytes& b) {
    // Fail closed on existing final path. Create temp atomically; publish without replacement.
    static std::atomic<unsigned int> serial{0};
    const std::string tmp=path+".tmp."+std::to_string(GetCurrentProcessId())+"."+std::to_string(serial.fetch_add(1, std::memory_order_relaxed)+1);
    const auto wtmp=Wide(tmp),wpath=Wide(path);
    if(wtmp.empty() || wpath.empty()) return false;
    HANDLE h=CreateFileW(wtmp.c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL|FILE_FLAG_WRITE_THROUGH,nullptr);
    if(h==INVALID_HANDLE_VALUE) return false;
    DWORD written=0;
    const bool ok=WriteFile(h,b.data(),static_cast<DWORD>(b.size()),&written,nullptr) &&
                  written==b.size() && FlushFileBuffers(h);
    CloseHandle(h);
    if(!ok) {DeleteFileW(wtmp.c_str());return false;}
    const bool moved=MoveFileExW(wtmp.c_str(),wpath.c_str(),MOVEFILE_WRITE_THROUGH)!=0;
    if(!moved) DeleteFileW(wtmp.c_str());
    return moved;
}
bool DeleteDurable(const std::string& path) {const auto w=Wide(path);return !w.empty() && DeleteFileW(w.c_str())!=0;}
#else
bool IsDirectorySafe(const std::string& root) {
    struct stat st{};return lstat(root.c_str(),&st)==0 && S_ISDIR(st.st_mode) &&
                          st.st_uid==geteuid() && !(st.st_mode & (S_IWOTH|S_IWGRP));
}
ReadResult ReadExact(const std::string& path,Bytes& result) {
    int fd=open(path.c_str(),O_RDONLY|O_CLOEXEC|O_NOFOLLOW);
    if(fd<0) return errno==ENOENT?ReadResult::Missing:ReadResult::Failure;
    struct stat st{};
    if(fstat(fd,&st)!=0 || !S_ISREG(st.st_mode) || st.st_size<0 ||
       static_cast<std::uint64_t>(st.st_size)>kMaxRecord) {close(fd);return ReadResult::Failure;}
    result.resize(static_cast<std::size_t>(st.st_size));std::size_t done=0;
    while(done<result.size()) {
        ssize_t n=read(fd,result.data()+done,result.size()-done);
        if(n<0 && errno==EINTR) continue;
        if(n<=0) {close(fd);return ReadResult::Failure;}
        done+=static_cast<std::size_t>(n);
    }
    close(fd);return ReadResult::Valid;
}
bool SyncDir(const std::string& path) {
    auto slash=path.find_last_of('/');const std::string directory=slash==std::string::npos?".":path.substr(0,slash);
    int fd=open(directory.c_str(),O_RDONLY|O_DIRECTORY|O_CLOEXEC|O_NOFOLLOW);
    if(fd<0) return false;
    const bool ok=fsync(fd)==0;close(fd);return ok;
}
bool Publish(const std::string& path,const Bytes& b) {
    // Hard link gives an atomic NOREPLACE final name. Temp left on crash is inert.
    static std::atomic<unsigned int> serial{0};
    const std::string tmp=path+".tmp."+std::to_string(getpid())+"."+std::to_string(serial.fetch_add(1, std::memory_order_relaxed)+1);
    int fd=open(tmp.c_str(),O_CREAT|O_EXCL|O_WRONLY|O_CLOEXEC|O_NOFOLLOW,0600);
    if(fd<0) return false;
    std::size_t done=0;
    while(done<b.size()) {
        ssize_t n=write(fd,b.data()+done,b.size()-done);
        if(n<0 && errno==EINTR) continue;
        if(n<=0) break;
        done+=static_cast<std::size_t>(n);
    }
    bool good=done==b.size() && fsync(fd)==0;
    if(close(fd)!=0) good=false;
    if(good) good=link(tmp.c_str(),path.c_str())==0;
    if(good) good=SyncDir(path);
    unlink(tmp.c_str());
    return good;
}
bool DeleteDurable(const std::string& path) {
    return unlink(path.c_str())==0 && SyncDir(path);
}
#endif
} // namespace

CompanionTradeDiskJournal::CompanionTradeDiskJournal(std::string root,ICompanionTradeSnapshotBackend& backend)
    :root_(std::move(root)),backend_(backend),healthy_(IsDirectorySafe(root_)) {}
std::string CompanionTradeDiskJournal::IntentPath(std::uint64_t a,std::uint64_t b) const {return Name(root_,"intent_",a,b);}
std::string CompanionTradeDiskJournal::ReceiptPath(std::uint64_t n) const {return Name(root_,"receipt_",n);}
std::string CompanionTradeDiskJournal::ArchivePath(std::uint64_t n) const {return Name(root_,"completed_",n);}
bool CompanionTradeDiskJournal::LockPair(std::uint64_t a,std::uint64_t b) {return healthy_ && backend_.LockPair(a,b);}
void CompanionTradeDiskJournal::UnlockPair(std::uint64_t a,std::uint64_t b) {backend_.UnlockPair(a,b);}
CompanionTradeReceipt CompanionTradeDiskJournal::Receipt(std::uint64_t n) {
    CompanionTradeReceipt out;
    if(!healthy_ || !n) {out.found=true;return out;}
    Bytes b; const auto state=ReadExact(ReceiptPath(n),b);
    if(state==ReadResult::Missing) return out;
    out.found=true;
    CompanionTradeOffer offer;
    if(state!=ReadResult::Valid || !DecodeReceipt(b,offer) || offer.tradeNonce!=n) {
        Poison();return out;
    }
    // A receipt without the immutable after-image archive is not safely
    // deliverable after disconnect/restart. NEVER call it AlreadyCommitted.
    // Older receipts lacking this archive fail closed until explicitly
    // migrated from verified backups; do not manufacture an after-image.
    Bytes archived;
    CompanionTradePrepared proof;
    if(ReadExact(ArchivePath(n),archived)!=ReadResult::Valid ||
       !DecodePlan(archived,proof) || !SameOffer(proof.offer,offer)) {
        Poison();return out;
    }
    out.committed=true;
    out.offerDigest=offer.offerDigest;
    out.authenticatedOwner=offer.authenticatedOwner;
    out.companionCharacter=offer.companionCharacter;
    out.offerRevision=offer.offerRevision;
    return out;
}
CompanionTradePending CompanionTradeDiskJournal::PendingForPair(std::uint64_t a,std::uint64_t b) {
    CompanionTradePending out;
    if(!healthy_ || !a || !b || a==b) {out.present=true;return out;}
    Bytes raw;const auto state=ReadExact(IntentPath(a,b),raw);
    if(state==ReadResult::Missing) return out;
    out.present=true;
    if(state!=ReadResult::Valid || !DecodePlan(raw,out.plan) ||
       out.plan.offer.authenticatedOwner!=a || out.plan.offer.companionCharacter!=b) Poison();
    return out;
}
bool CompanionTradeDiskJournal::Prepare(const CompanionTradeOffer& o, CompanionTradePrepared& p) {
    return healthy_ && backend_.Prepare(o,p);
}
bool CompanionTradeDiskJournal::SealIntent(const CompanionTradePrepared& p) {
    Bytes raw;
    if(!healthy_ || !EncodePlan(p,raw)) return false;
    if(!Publish(IntentPath(p.offer.authenticatedOwner,p.offer.companionCharacter),raw)) {
        Poison();return false;
    }
    return true;
}
TradeApplyResult CompanionTradeDiskJournal::ApplyExact(TradeParticipant who,const CompanionTradePrepared& p) {
    if(!healthy_) return TradeApplyResult::IoUncertain;
    Bytes actual;const auto state=ReadExact(IntentPath(p.offer.authenticatedOwner,p.offer.companionCharacter),actual);
    Bytes wanted;
    if(state!=ReadResult::Valid || !EncodePlan(p,wanted) || actual!=wanted) {
        Poison();return TradeApplyResult::IoUncertain;
    }
    return backend_.ApplyExact(who,p);
}
bool CompanionTradeDiskJournal::CompletedPlan(std::uint64_t nonce, CompanionTradePrepared& out) {
    if(!healthy_ || !nonce) return false;
    const auto receipt=Receipt(nonce);
    if(!receipt.found || !receipt.committed) return false;
    Bytes encoded;
    const auto state=ReadExact(ArchivePath(nonce),encoded);
    if(state!=ReadResult::Valid) {Poison();return false;}
    CompanionTradePrepared decoded;
    if(!DecodePlan(encoded,decoded) || decoded.offer.tradeNonce!=nonce ||
       !TradeReceiptMatches(receipt,decoded.offer)) {Poison();return false;}
    out=std::move(decoded);
    return true;
}

bool CompanionTradeDiskJournal::SealReceipt(const CompanionTradePrepared& p) {
    if(!healthy_) return false;
    const auto& o=p.offer;
    Bytes wanted; if(!EncodePlan(p,wanted)) return false;
    // Archive is published durably BEFORE the success receipt, so a crash
    // after acknowledgement cannot discard the last copy of the after-image.
    Bytes existing;
    auto state=ReadExact(ArchivePath(o.tradeNonce),existing);
    if(state==ReadResult::Missing) {
        if(!Publish(ArchivePath(o.tradeNonce),wanted)) {Poison();return false;}
    } else if(state!=ReadResult::Valid || existing!=wanted) {
        Poison();return false;
    }
    const auto receipt=Receipt(o.tradeNonce);
    if(receipt.found) return TradeReceiptMatches(receipt,o);
    const Bytes raw=EncodeReceipt(o);
    if(!Publish(ReceiptPath(o.tradeNonce),raw)) {Poison();return false;}
    return true;
}
bool CompanionTradeDiskJournal::RemoveIntent(const CompanionTradePrepared& p) {
    if(!healthy_) return false;
    const auto r=Receipt(p.offer.tradeNonce);
    if(!r.found || !TradeReceiptMatches(r,p.offer)) return false;
    CompanionTradePrepared completed;
    if(!CompletedPlan(p.offer.tradeNonce,completed) ||
       !SameOffer(completed.offer,p.offer) ||
       completed.ownerBefore!=p.ownerBefore || completed.ownerAfter!=p.ownerAfter ||
       completed.companionBefore!=p.companionBefore ||
       completed.companionAfter!=p.companionAfter) return false;
    Bytes raw,wanted;
    const auto state=ReadExact(IntentPath(p.offer.authenticatedOwner,p.offer.companionCharacter),raw);
    if(state==ReadResult::Missing) return true;
    if(state!=ReadResult::Valid || !EncodePlan(p,wanted) || raw!=wanted) {Poison();return false;}
    if(!DeleteDurable(IntentPath(p.offer.authenticatedOwner,p.offer.companionCharacter))) {Poison();return false;}
    return true;
}
}}} // namespace gameplay::companions::v2
