#include "gameplay/companions/v2/CompanionTradeExactRecordStore.h"
#include <atomic>
#include <cstddef>
#include <cstring>
#include <vector>
#include <string>
#include <utility>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#endif

namespace gameplay { namespace companions { namespace v2 {
namespace {
using Bytes = std::vector<std::uint8_t>;
constexpr std::size_t kMax = CompanionTradeExactRecordStore::kMaximumRecordBytes;
std::atomic<std::uint32_t> tempSerial{0};

bool AllowedSize(const Bytes& x) { return !x.empty() && x.size() <= kMax; }
std::string Directory(const std::string& p) {
    auto slash=p.find_last_of("/\\");
    return slash==std::string::npos ? "." : (slash==0 ? p.substr(0,1) : p.substr(0,slash));
}
std::string Temporary(const std::string& path, std::uint32_t process) {
    return path+".summit-trade-tmp-"+std::to_string(process)+"-"+
       std::to_string(++tempSerial);
}
#ifdef _WIN32
std::wstring ToWide(const std::string& u8) {
    if(u8.empty()) return {};
    const int n=MultiByteToWideChar(CP_ACP,0,u8.c_str(),
                                    static_cast<int>(u8.size()),nullptr,0);
    if(n<=0) return {};
    std::wstring out(static_cast<std::size_t>(n),L'\0');
    if(MultiByteToWideChar(CP_ACP,0,u8.c_str(),
          static_cast<int>(u8.size()),&out[0],n)!=n) return {};
    return out;
}
// Reject both reparse points and multiple hard links, which could make an
// unrelated pathname unexpectedly reference the same DAT.
bool OpenRegular(const std::string& path,HANDLE& h,DWORD access) {
    auto w=ToWide(path);if(w.empty()) return false;
    h=CreateFileW(w.c_str(),access,FILE_SHARE_READ,nullptr,OPEN_EXISTING,
                  FILE_FLAG_OPEN_REPARSE_POINT,nullptr);
    if(h==INVALID_HANDLE_VALUE) return false;
    BY_HANDLE_FILE_INFORMATION info{};
    if(!GetFileInformationByHandle(h,&info) ||
       (info.dwFileAttributes&(FILE_ATTRIBUTE_REPARSE_POINT|FILE_ATTRIBUTE_DIRECTORY)) ||
       info.nNumberOfLinks!=1 || info.nFileSizeHigh || info.nFileSizeLow>kMax) {
        CloseHandle(h);h=INVALID_HANDLE_VALUE;return false;
    }
    return true;
}
bool ReadFileExact(const std::string& path,Bytes& result) {
    HANDLE h=INVALID_HANDLE_VALUE;
    if(!OpenRegular(path,h,GENERIC_READ)) return false;
    LARGE_INTEGER n{};
    bool ok=GetFileSizeEx(h,&n) && n.QuadPart>0 && n.QuadPart<=static_cast<LONGLONG>(kMax);
    Bytes next;
    if(ok) {
        next.resize(static_cast<std::size_t>(n.QuadPart));DWORD done=0;
        ok=ReadFile(h,next.data(),static_cast<DWORD>(next.size()),&done,nullptr)!=0 &&
           done==next.size();
    }
    CloseHandle(h);
    if(ok) result.swap(next);
    return ok;
}
bool WriteReplace(const std::string& path,const Bytes& next) {
    const std::string tmp=Temporary(path,GetCurrentProcessId());
    auto wt=ToWide(tmp), wp=ToWide(path);
    if(wt.empty()||wp.empty()) return false;
    HANDLE h=CreateFileW(wt.c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,
                         FILE_ATTRIBUTE_NORMAL|FILE_FLAG_WRITE_THROUGH,nullptr);
    if(h==INVALID_HANDLE_VALUE) return false;
    DWORD done=0;
    bool ok=WriteFile(h,next.data(),static_cast<DWORD>(next.size()),&done,nullptr)!=0 &&
            done==next.size() && FlushFileBuffers(h)!=0;
    if(!CloseHandle(h)) ok=false;
    if(ok) ok=MoveFileExW(wt.c_str(),wp.c_str(),
                    MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=0;
    if(!ok) DeleteFileW(wt.c_str());
    return ok;
}
#else
bool ReadFileExact(const std::string& path,Bytes& result) {
    const int fd=open(path.c_str(),O_RDONLY|O_NOFOLLOW|O_CLOEXEC);
    if(fd<0) return false;
    struct stat st{};
    bool ok=fstat(fd,&st)==0 && S_ISREG(st.st_mode) && st.st_nlink==1 &&
            st.st_size>0 && st.st_size<=static_cast<off_t>(kMax);
    Bytes next;
    if(ok) {
        next.resize(static_cast<std::size_t>(st.st_size));
        std::size_t done=0;
        while(done<next.size()) {
            const ssize_t n=read(fd,next.data()+done,next.size()-done);
            if(n<0 && errno==EINTR) continue;
            if(n<=0) {ok=false;break;}
            done+=static_cast<std::size_t>(n);
        }
    }
    if(close(fd)!=0) ok=false;
    if(ok) result.swap(next);
    return ok;
}
bool WriteReplace(const std::string& path,const Bytes& next) {
    const std::string dir=Directory(path);
    const int dirfd=open(dir.c_str(),O_RDONLY|O_NOFOLLOW|O_DIRECTORY|O_CLOEXEC);
    if(dirfd<0) return false;
    const std::string tmp=Temporary(path,static_cast<std::uint32_t>(getpid()));
    const int fd=open(tmp.c_str(),O_WRONLY|O_CREAT|O_EXCL|O_NOFOLLOW|O_CLOEXEC,0600);
    if(fd<0) {close(dirfd);return false;}
    std::size_t done=0;
    bool ok=true;
    while(done<next.size()) {
        const ssize_t n=write(fd,next.data()+done,next.size()-done);
        if(n<0 && errno==EINTR) continue;
        if(n<=0) {ok=false;break;}
        done+=static_cast<std::size_t>(n);
    }
    if(ok) ok=fsync(fd)==0;
    if(close(fd)!=0) ok=false;
    if(ok) ok=rename(tmp.c_str(),path.c_str())==0;
    if(ok) ok=fsync(dirfd)==0;
    if(!ok) unlink(tmp.c_str());
    close(dirfd);
    return ok;
}
#endif
} // namespace

bool CompanionTradeExactRecordStore::Read(const std::string& path,Bytes& output) {
    if(path.empty()) return false;
    return ReadFileExact(path,output);
}
TradeApplyResult CompanionTradeExactRecordStore::Replace(
    const std::string& path,const Bytes& before,const Bytes& after) {
    if(path.empty() || !AllowedSize(before) || !AllowedSize(after) ||
       before==after) return TradeApplyResult::Conflict;
    Bytes observed;
    // An error reading the target may be corruption, permissions or IO. Never
    // treat it as missing, and never create a new DAT on a trade.
    if(!Read(path,observed)) return TradeApplyResult::IoUncertain;
    if(observed==after) return TradeApplyResult::AlreadyApplied;
    if(observed!=before) return TradeApplyResult::Conflict;
    // An exclusive owner/companion lease is a REQUIRED precondition. This
    // API alone does not serialize external game writers.
    if(!WriteReplace(path,after)) return TradeApplyResult::IoUncertain;
    Bytes check;
    if(!Read(path,check) || check!=after) return TradeApplyResult::IoUncertain;
    return TradeApplyResult::AppliedDurably;
}
}}}
