#pragma once
// SUMMIT443A: fail-closed coordinator for a FULL opaque DAT before/after image.
// This is a real reusable transaction *contract*, not hooked to the game.
// The port MUST provide durable barriers, path provenance, save-queue lease,
// authenticated generation and real CreateItem output in Prepare().
#include <cstdint>
#include <cstdio> // SUMMIT452A: precise refusal stage, no character data
#include <string>
#include <vector>
#include <utility>
namespace summit443 {
struct CraftRequest {
    std::uint64_t character,sessionGeneration;
    std::string requestId,recipeCode;
    CraftRequest(std::uint64_t c=0,std::uint64_t g=0,std::string r="",std::string p=""):
        character(c),sessionGeneration(g),requestId(std::move(r)),recipeCode(std::move(p)){}
};
inline bool ValidRequest(const CraftRequest& r) {
    if(!r.character||!r.sessionGeneration||r.requestId.size()<8||r.requestId.size()>48||
       r.recipeCode.empty()||r.recipeCode.size()>16)return false;
    for(auto c:r.requestId)if(!((c>='a'&&c<='z')||(c>='A'&&c<='Z')||
        (c>='0'&&c<='9')||c=='-'||c=='_'))return false;
    for(auto c:r.recipeCode)if(!((c>='A'&&c<='Z')||(c>='0'&&c<='9')))return false;
    return true;
}
struct Image {
    std::vector<std::uint8_t> before,after;
    // prepare must prove the real item was generated and physical material
    // conservation in A+B, server-side recipe/catalog, session and slots.
    bool authenticatedOwner=false,verifiedRecipe=false,verifiedMaterials=false;
    bool nativeOutputCreated=false,provenConservation=false,capacityOk=false;
};
inline bool ValidImage(const Image& img){
    static const std::size_t max=8u*1024u*1024u;
    return img.authenticatedOwner&&img.verifiedRecipe&&img.verifiedMaterials&&
      img.nativeOutputCreated&&img.provenConservation&&img.capacityOk&&
      !img.before.empty()&&!img.after.empty()&&img.before.size()<=max&&
      img.after.size()<=max&&img.before!=img.after;
}
enum class CraftResult { Committed, ReplayCommitted, Rejected, Busy, RecoveryRequired };
enum class StoreWrite { Durable, AlreadyApplied, Conflict, Uncertain };
enum class ReceiptStatus { Absent, SameCommitted, DifferentCommitted, Corrupt };
struct Pending {
    bool exists=false;
    CraftRequest request;
    Image image;
};
class ICraftDatPort443 {
public:
    virtual ~ICraftDatPort443(){}
    virtual bool LockCharacter(std::uint64_t)=0; // whole save queue, all writers
    virtual void UnlockCharacter(std::uint64_t)=0;
    virtual bool SessionAuthentic(const CraftRequest&)=0;
    virtual ReceiptStatus Receipt(const CraftRequest&)=0; // durable replay receipt
    virtual Pending PendingFor(std::uint64_t)=0; // fail closed on corrupt intent
    virtual bool Prepare(const CraftRequest&,Image&)=0; // data server-side, CreateItem
    virtual bool SealIntent(const CraftRequest&,const Image&)=0; // sync journal FIRST
    virtual StoreWrite WriteExactDat(const CraftRequest&,const Image&)=0; // compare+fsync
    virtual bool SealReceipt(const CraftRequest&,const Image&)=0; // fsync receipt
    virtual bool QuarantineOldSession(std::uint64_t)=0; // no stale save overwrite
    virtual void CleanCommittedIntent(const CraftRequest&)=0; // best effort only
};
class CraftDatCoordinator443 {
    ICraftDatPort443& port;
    struct Lock {
        ICraftDatPort443& p;std::uint64_t id;bool ok;
        Lock(ICraftDatPort443& q,std::uint64_t n):p(q),id(n),ok(q.LockCharacter(n)){}
        ~Lock(){if(ok)p.UnlockCharacter(id);}Lock(const Lock&)=delete;Lock& operator=(const Lock&)=delete;
    };
    CraftResult Finish(const CraftRequest& req,const Image& image) {
        const auto r=port.WriteExactDat(req,image);
        if(r!=StoreWrite::Durable&&r!=StoreWrite::AlreadyApplied)
            return CraftResult::RecoveryRequired;
        if(!port.SealReceipt(req,image))return CraftResult::RecoveryRequired;
        // A real server MUST reject every pending old client-save; otherwise
        // it could overwrite committed DAT with stale inventory after ACK.
        if(!port.QuarantineOldSession(req.character))return CraftResult::RecoveryRequired;
        port.CleanCommittedIntent(req);
        return CraftResult::Committed;
    }
public:
    explicit CraftDatCoordinator443(ICraftDatPort443& p):port(p){}
    CraftResult Commit(const CraftRequest& req) {
        if(!ValidRequest(req)){std::fprintf(stderr,"[SUMMIT452][CRAFT] deny=invalid_request\n");return CraftResult::Rejected;}
        Lock lock(port,req.character);if(!lock.ok)return CraftResult::Busy;
        if(!port.SessionAuthentic(req)){std::fprintf(stderr,"[SUMMIT452][CRAFT] deny=session_authentication\n");return CraftResult::Rejected;}
        const auto receipt=port.Receipt(req);
        // A durable receipt may precede session quarantine. Do not reply as
        // committed while a pending intent still needs pre-login recovery.
        if(port.PendingFor(req.character).exists)return CraftResult::RecoveryRequired;
        if(receipt==ReceiptStatus::SameCommitted)return CraftResult::ReplayCommitted;
        if(receipt!=ReceiptStatus::Absent){std::fprintf(stderr,"[SUMMIT452][CRAFT] deny=receipt status=%d\n",static_cast<int>(receipt));return CraftResult::Rejected;}
        Image image;
        if(!port.Prepare(req,image)){std::fprintf(stderr,"[SUMMIT452][CRAFT] deny=prepare\n");return CraftResult::Rejected;}
        if(!ValidImage(image)){std::fprintf(stderr,"[SUMMIT452][CRAFT] deny=invalid_image\n");return CraftResult::Rejected;}
        if(!port.SealIntent(req,image))return CraftResult::RecoveryRequired;
        return Finish(req,image);
    }
    // Must run BEFORE allowing a character to log in or write anything.
    CraftResult RecoverBeforeLogin(std::uint64_t character) {
        if(!character)return CraftResult::Rejected;
        Lock lock(port,character);if(!lock.ok)return CraftResult::Busy;
        const auto pending=port.PendingFor(character);
        if(!pending.exists)return CraftResult::Rejected;
        if(pending.request.character!=character||!ValidRequest(pending.request)||
            !ValidImage(pending.image))return CraftResult::RecoveryRequired;
        const auto receipt=port.Receipt(pending.request);
        if(receipt==ReceiptStatus::Corrupt||receipt==ReceiptStatus::DifferentCommitted)
            return CraftResult::RecoveryRequired;
        if(receipt==ReceiptStatus::SameCommitted){
            if(!port.QuarantineOldSession(character))return CraftResult::RecoveryRequired;
            port.CleanCommittedIntent(pending.request);
            return CraftResult::ReplayCommitted;
        }
        return Finish(pending.request,pending.image);
    }
};
} // namespace summit443
