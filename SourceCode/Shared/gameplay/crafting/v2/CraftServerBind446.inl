// SUMMIT 446 — actual server-only bridge from the authenticated chat command
// to canonical recipe, native CreateItem, two-bag DAT image and durable journal.
// This inl is included at the END of record_legacy_server.inl, after all
// legacy save/load/owner functions have been declared. No source-side SQL.
#include "gameplay/crafting/v2/CraftNativeDat446.h"
#include "gameplay/crafting/v2/CraftIdentityReconcile447.h"
#include "gameplay/crafting/v2/CraftDatDiskPort446.h"
#include "systems/gameplay/crafting/v2/CraftRegistry425.generated.h"
#include "systems/gameplay/crafting/v2/CraftRecipeActivation444.generated.h"
#include "systems/inventory/v2/legacy/LegacyInventoryRuntime.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"
#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <memory>
#include <set>
#include <string>
#include <tuple>
#include <vector>
// SUMMIT448A: request a normal client save only AFTER releasing the craft gate.
extern int SendSaveClientData(rsPLAYINFO*);

extern int CreateItem(psITEM*,char*,int);
namespace summit446 {
namespace {
static const char* kCraftRoot446="LOGS/SummitCraftJournal";
// No account, character name, DAT path or secret is written to diagnostics.
static bool Deny447(const char* stage) {
    std::fprintf(stderr,"[SUMMIT447][CRAFT] deny=%s\n",stage);
    return false;
}

static bool Key446(summit_ci440::Key& key){
    // 32-byte persistent key generated outside the game and kept private.
    // A missing key is a hard stop. Never use the demo/test HMAC key.
    FILE* fp=nullptr;
    if(fopen_s(&fp,"LOGS/SummitCraftJournal/ci_hmac_key.bin","rb")!=0||!fp)return false;
    unsigned char buf[33]{};
    const size_t n=fread(buf,1,sizeof(buf),fp);
    const int closeResult=fclose(fp);
    if(closeResult || n!=32)return false;
    std::copy(buf,buf+32,key.begin());return summit_ci440::ValidKey(key);
}
// SUMMIT447A: the online InvenItemInfo ledger created by MakeRecordItemList
// includes equipped records and the mouse cursor, not just ItemPosition==0.
// Only potion identities and tracked thrown-item identities are excluded.
static bool FullSlotEquivalence446(const std::vector<NativeItem<sRECORD_ITEM>>& records,
                                  rsPLAYINFO* player){
    if(!player)return false;
    auto& registry=gameplay::player::legacy::ServerLegacyOnlinePlayerRegistry();
    const auto handle=registry.HandleOf(player);
    if(!handle.Valid()||registry.Resolve(handle)!=player)return false;
    gameplay::inventory::InventorySnapshot snapshot;
    if(!gameplay::inventory::legacy::ServerLegacyInventoryRuntime()
        .TryGetSnapshot(handle.id,snapshot)||registry.Resolve(handle)!=player)return false;
    using Identity=summit447::Identity;
    if(player->ThrowItemCount<0||player->ThrowItemCount>THROW_ITEM_INFO_MAX)return false;
    std::set<Identity> thrown;
    for(int n=0;n<player->ThrowItemCount;++n){
        const auto& e=player->ThrowItemInfo[n];
        const Identity id(static_cast<std::uint32_t>(e.dwCode),
            static_cast<std::uint32_t>(e.dwKey),static_cast<std::uint32_t>(e.dwSum));
        if(!summit447::ValidIdentity(id)||!thrown.insert(id).second)return false;
    }
    std::vector<summit447::NativeRecord> dat;
    dat.reserve(records.size());
    for(const auto& e:records){
        // Synthetic offhand mirror has no identity in MakeRecordItemList().
        // Decode() already required a real, signed position-1 parent.
        if(IsLegacyTwoHandMirror455(e))continue;
        const auto& item=e.item.sItemInfo;
        const Identity id(static_cast<std::uint32_t>(item.CODE),
            static_cast<std::uint32_t>(item.ItemHeader.Head),
            static_cast<std::uint32_t>(item.ItemHeader.dwChkSum));
        dat.emplace_back(id,e.item.ItemPosition,thrown.count(id)!=0);
    }
    std::vector<Identity> live;
    live.reserve(snapshot.slots.size());
    for(const auto& s:snapshot.slots){
        live.emplace_back(static_cast<std::uint32_t>(s.item.item.value),
            static_cast<std::uint32_t>(s.item.key),
            static_cast<std::uint32_t>(s.item.checksum));
    }
    const summit447::Result result=summit447::Check(dat,live,thrown);
    if(result!=summit447::Result::Match)
        std::fprintf(stderr,"[SUMMIT447][CRAFT] deny=dat_online result=%d dat=%lu live=%lu thrown=%lu\n",
            static_cast<int>(result),static_cast<unsigned long>(dat.size()),
            static_cast<unsigned long>(live.size()),static_cast<unsigned long>(thrown.size()));
    return result==summit447::Result::Match;
}
static bool Place446(const std::vector<NativeItem<sRECORD_ITEM>>& items,
                     const sITEMINFO& output,bool& secondary,int& slot,int& x,int& y) {
    const auto& footprints=gameplay::inventory::legacy::ServerLegacyItemCatalogAdapter();
    gameplay::inventory::ItemFootprint resultSize;
    const gameplay::ItemId resultId(static_cast<std::uint32_t>(output.CODE));
    if(!footprints.TryGetFootprint(resultId,resultSize)||!resultSize.ValidFor(resultId))return false;
    for(unsigned bag=0;bag<2;++bag){
        std::vector<gameplay::inventory::BagRect> occupied;
        std::set<int> used;
        for(const auto& e:items)if(static_cast<unsigned>(e.secondary)==bag){
            // The ordinal belongs to the native item array even for equipped
            // records and synthetic two-hand mirrors; never reuse that slot.
            used.insert(e.item.ItemCount);
            if(e.item.ItemPosition!=0)continue;
            if(e.item.x>=10000)continue;
            gameplay::inventory::ItemFootprint size;
            const gameplay::ItemId id(static_cast<std::uint32_t>(e.item.sItemInfo.CODE));
            if(!footprints.TryGetFootprint(id,size)||!size.ValidFor(id))return false;
            occupied.emplace_back(e.item.x,e.item.y,size.widthPixels,size.heightPixels);
        }
        const auto fit=gameplay::inventory::CompanionBagSpacePolicy::FindFree(
                   occupied,resultSize.widthPixels,resultSize.heightPixels);
        if(!fit.Available())continue;
        for(int index=0;index<INVENTORY_MAXITEM && index<100;++index)
            if(used.count(index)==0){
                secondary=bag!=0;slot=index;x=fit.x;y=fit.y;return true;
            }
    }
    return false;
}
class CraftRuntimeHost446 final:public ICraftHost446 {
    std::string account_,name_,path_;
    std::uint64_t participant_;
    rsPLAYINFO* player_;
    rsPLAYINFO* loginPlayer_;
    bool reserved_=false,saveReserved_=false,lockedData_=false;
    bool needsFreshSave448_=false;
    bool loginLoadGateHeld_;
    gameplay::companions::v2::SaveSessionKey session_;
    std::uint64_t token_=0;
public:
    CraftRuntimeHost446(const char* account,const char* player,rsPLAYINFO* session,bool loginGateHeld=false,rsPLAYINFO* loginPlayer=nullptr)
      :account_(account?account:""),name_(player?player:""),path_(),
       participant_(gameplay::companions::v2::TradeParticipantId(account,player)),player_(session),
       loginPlayer_(loginPlayer),
       loginLoadGateHeld_(loginGateHeld),
       session_(session?static_cast<std::uint32_t>(session->dwObjectSerial):0u,
                session?static_cast<std::uint32_t>(session->dwConnectCount):0u){
      char path[256]{};
      if(!name_.empty()&&GetUserDataFile(name_.c_str(),path))path_=path;
    }
    const std::string& Path()const{return path_;}
    std::uint64_t Id()const{return participant_;}
    bool NeedsFreshSave448() const { return needsFreshSave448_; }
    gameplay::companions::v2::SaveSessionKey SessionKey448()const{return session_;}
    bool Reserve(std::uint64_t id)override{
        if(!id||id!=participant_||path_.empty()||reserved_||
           (loginLoadGateHeld_ && FindUserFromName(const_cast<char*>(name_.c_str()))!=nullptr &&
             FindUserFromName(const_cast<char*>(name_.c_str()))!=loginPlayer_)||
           ! (loginLoadGateHeld_ ?
               gameplay::companions::v2::ServerTradeParticipantGate350B()
                  .TryReserveSingleUnderLoadGuard446(participant_) :
               gameplay::companions::v2::ServerTradeParticipantGate350B()
                  .TryReserveSingle(participant_)))return Deny447("reserve_gate_or_path");
        reserved_=true;
        if(player_){
            auto& fence448=gameplay::companions::v2::ServerSaveActivityFence();
            if(!fence448.TryReserve(session_,token_)) {
                const auto reason448=fence448.DiagnoseLease448(session_);
                needsFreshSave448_ = reason448==gameplay::companions::v2::SaveLeaseReason448::NotSaved ||
                     reason448==gameplay::companions::v2::SaveLeaseReason448::IncompleteSave;
                std::fprintf(stderr,"[SUMMIT448][CRAFT] save_lease_reason=%s refresh_eligible=%d\n",
                    gameplay::companions::v2::SaveLeaseReasonName448(reason448),
                    needsFreshSave448_?1:0);
                return Deny447("save_lease");
            }
            saveReserved_=true;
            std::uint64_t sequence=0;
            if(!gameplay::companions::v2::ServerSaveActivityFence()
                .TryLeasedAcceptedSaveSequence(session_,token_,sequence))return Deny447("accepted_save_missing");
        }
        // Save queue must exist so we can hold the SAME critical section as
        // the legacy DAT writer. A null queue is not permission to craft.
        if(!sRecDataBuff)return Deny447("save_queue_missing");
        EnterCriticalSection(&cSaveDataSection);
        lockedData_=true;
        return true;
    }
    void Release(std::uint64_t id)override{
        if(lockedData_){LeaveCriticalSection(&cSaveDataSection);lockedData_=false;}
        if(saveReserved_){
            (void)gameplay::companions::v2::ServerSaveActivityFence()
                .ReleaseReservation(session_,token_);saveReserved_=false;token_=0;
        }
        if(reserved_){
            (void)gameplay::companions::v2::ServerTradeParticipantGate350B()
                .ReleaseSingleVerified(participant_);reserved_=false;
        }
    }
    ~CraftRuntimeHost446(){Release(participant_);}
    bool Session(const summit443::CraftRequest& r)override{
        if(!player_ || !reserved_ || !saveReserved_ || !lockedData_ ||
           r.character!=participant_ ||
           r.sessionGeneration!=static_cast<std::uint64_t>(session_.generation)+1u ||
           !player_->lpsmSock || !player_->dwObjectSerial ||
           !player_->getAccount() || !player_->getName() ||
           account_!=player_->getAccount()||name_!=player_->getName()||
           !rsCheckAccountChar(account_.c_str(),const_cast<char*>(name_.c_str())))return false;
        const auto& registry=gameplay::player::legacy::ServerLegacyOnlinePlayerRegistry();
        const auto handle=registry.HandleOf(player_);
        return handle.Valid()&&registry.Resolve(handle)==player_;
    }
    bool Prepare(const summit443::CraftRequest& req,summit443::Image& image)override {
        if(!Session(req))return Deny447("session");
        const auto* activated=summit_craft_444::Find(req.recipeCode.c_str());
        const auto* rule=summit_craft_425::Find(req.recipeCode.c_str());
        if(req.recipeCode!="WA103"||!activated||!activated->jsonEnabled||!activated->complete ||
           !rule||!rule->idValid||rule->outputCount!=1||rule->native==0 ||
           rule->size==0||rule->size>16||rule->zero||rule->bad)return Deny447("recipe");
        summit_ci440::Key key{};if(!Key446(key))return Deny447("hmac_key");
        std::vector<Need> needs;
        for(std::size_t k=rule->first;k<rule->first+rule->size;++k){
            const auto& ingredient=summit_craft_425::kIngredients[k];
            if(!summit_ci440::IsNativeCi(ingredient.native)||ingredient.count<=0)return Deny447("recipe_ingredient");
            needs.push_back({ingredient.native,static_cast<std::uint32_t>(ingredient.count)});
        }
        Bytes before;
        if(!gameplay::companions::v2::CompanionTradeExactRecordStore::Read(path_,before) ||
           before.size()<offsetof(TRANS_RECORD_DATA,Data)||
           before.size()>sizeof(TRANS_RECORD_DATA))return Deny447("dat_read");
        std::unique_ptr<TRANS_RECORD_DATA> source(new TRANS_RECORD_DATA());
        std::memcpy(source.get(),before.data(),before.size());
        if(!std::memchr(source->smCharInfo.szName,0,sizeof(source->smCharInfo.szName))||
           lstrcmpiA(source->smCharInfo.szName,name_.c_str())!=0)return Deny447("dat_owner");
        const auto validate=[](const sITEMINFO& info){
            return CheckItemForm(const_cast<sITEMINFO*>(&info))==TRUE;
        };
        std::vector<NativeItem<sRECORD_ITEM>> decoded;
        summit446::DecodeDetail454 detail454;
        const BuildStatus decodeStatus454=summit446::Decode<TRANS_RECORD_DATA,sRECORD_ITEM>(
            *source,before.size(),validate,decoded,&detail454);
        if(decodeStatus454!=BuildStatus::Ready){
            std::fprintf(stderr,"[SUMMIT454][CRAFT] dat_decode_reason=%s index=%d offset=%lu frame=%lu codec=%d position=%d ordinal=%d items=%d sub=%d data_size=%d stored_size=%d file_bytes=%lu result=%d\n",
                summit446::DecodeStageName454(detail454.stage),detail454.itemIndex,
                static_cast<unsigned long>(detail454.offset),
                static_cast<unsigned long>(detail454.frame),detail454.codecStatus,
                detail454.itemPosition,detail454.bagOrdinal,source->ItemCount,
                source->ItemSubStart,source->DataSize,source->size,
                static_cast<unsigned long>(before.size()),static_cast<int>(decodeStatus454));
            return Deny447("dat_decode");
        }
        unsigned preservedMirrors455=0;
        for(const auto& e:decoded)if(IsLegacyTwoHandMirror455(e))++preservedMirrors455;
        if(preservedMirrors455)
            std::fprintf(stderr,"[SUMMIT455][CRAFT] twohand_mirror=accepted count=%u inert=1\n",
                preservedMirrors455);
        if(!FullSlotEquivalence446(decoded,player_))return Deny447("dat_online_mismatch");
        auto mint=[&](sITEMINFO& info,const char* code)->bool{
            if(std::strlen(code)!=5)return false;
            char native[16]{};std::memcpy(native,code,5);
            psITEM generated{};
            if(CreateItem(&generated,native,player_->smCharInfo.JOB_CODE)!=TRUE)return false;
            ReformItem(&generated.ItemInfo);
            if(!validate(generated.ItemInfo))return false;
            info=generated.ItemInfo;return true;
        };
        auto place=[&](const std::vector<NativeItem<sRECORD_ITEM>>& entries,
                       const sITEMINFO& minted,bool& second,int& index,int& x,int& y){
            return Place446(entries,minted,second,index,x,y);
        };
        std::unique_ptr<TRANS_RECORD_DATA> next(new TRANS_RECORD_DATA());
        std::vector<IngredientCheck> conservation;
        const BuildStatus buildStatus447=summit446::Build<TRANS_RECORD_DATA,sRECORD_ITEM>(
            *source,before.size(),needs,rule->native,req.recipeCode.c_str(),key,
            validate,mint,place,*next,conservation);
        if(buildStatus447!=BuildStatus::Ready || conservation.size()!=needs.size()) {
            std::fprintf(stderr,"[SUMMIT447][CRAFT] deny=dat_build status=%d\n",
                         static_cast<int>(buildStatus447));
            return false;
        }
        const std::size_t required=std::max(before.size(),
            std::max(static_cast<std::size_t>(srRECORD_DEFAULT_SIZE),
                     static_cast<std::size_t>(next->size)));
        if(required>sizeof(TRANS_RECORD_DATA))return Deny447("after_capacity");
        Bytes after(required,0);
        std::memcpy(after.data(),next.get(),required);
        // Check that the write/readback target remains the same character.
        if(after==before)return Deny447("unchanged_after");
        image.before.swap(before);image.after.swap(after);
        image.authenticatedOwner=image.verifiedRecipe=image.verifiedMaterials=true;
        image.nativeOutputCreated=image.provenConservation=image.capacityOk=true;
        return true;
    }
    bool Quarantine(std::uint64_t id)override{
        if(id!=participant_||!reserved_||!lockedData_)return false;
        if(!player_){
            rsPLAYINFO* current=FindUserFromName(const_cast<char*>(name_.c_str()));
            return current==nullptr || current==loginPlayer_;
        }
        return saveReserved_ && gameplay::companions::v2::ServerSaveActivityFence()
                                     .SealCommittedForReconnect(session_,token_);
    }
};
} // anonymous

summit443::CraftResult CraftMakeFromChat446(rsPLAYINFO* player,const char* code,const char* request){
    if(!player||!player->getAccount()||!player->getName()||
       !code||!request)return summit443::CraftResult::Rejected;
    CraftRuntimeHost446 host(player->getAccount(),player->getName(),player);
    if(!host.Id()||host.Path().empty())return summit443::CraftResult::Rejected;
    CraftDatDiskPort446 port(kCraftRoot446,host.Path(),host);
    if(!port.Healthy()) {
        Deny447("journal_dir_unavailable");
        return summit443::CraftResult::Rejected;
    }
    summit443::CraftDatCoordinator443 coordinator(port);
    const summit443::CraftRequest req(host.Id(),
          static_cast<std::uint64_t>(player->dwConnectCount)+1u,request,code);
    const summit443::CraftResult result=coordinator.Commit(req);
    // If there has been no complete validated client save, the fence cannot
    // legitimately grant a lease. Ask for one using the EXISTING protocol;
    // do not consume materials or automatically replay the craft request.
    // Release the participant gate before asking: client saves must not be
    // blocked by the very failed craft reservation that requested them.
    if(result==summit443::CraftResult::Busy && host.NeedsFreshSave448()) {
        const auto session448=host.SessionKey448();
        host.Release(host.Id());
        if(player->lpsmSock &&
           gameplay::companions::v2::ServerSaveActivityFence()
               .TryRequestFreshSave448(session448,static_cast<std::uint32_t>(GetTickCount()))) {
            // SUMMIT452A: the next accepted save (possibly on DATA socket)
            // must be credited to THIS authenticated participant and session.
            const bool expected452=gameplay::companions::v2::ServerSaveActivityFence()
                .ExpectValidatedCraftSave452(host.Id(),session448,
                    static_cast<std::uint32_t>(GetTickCount()));
            if(expected452) {
                (void)SendSaveClientData(player);
                std::fprintf(stderr,"[SUMMIT452][CRAFT] expected_save_registered=1\n");
            } else {
                std::fprintf(stderr,"[SUMMIT452][CRAFT] expected_save_registered=0 no_save_sent=1\n");
            }

        }
    }
    std::fprintf(stderr,"[SUMMIT447][CRAFT] outcome=%d\n",static_cast<int>(result));
    return result;
}

bool CraftRecoverBeforeLogin446(const char* account,const char* name,rsPLAYINFO* loginSession){
    if(!account||!*account||!name||!*name||!rsCheckAccountChar(account,const_cast<char*>(name)))return false;
    const DWORD attrs=GetFileAttributesA(kCraftRoot446);
    if(attrs==INVALID_FILE_ATTRIBUTES&&GetLastError()==ERROR_FILE_NOT_FOUND)return true;
    if(attrs==INVALID_FILE_ATTRIBUTES||!(attrs&FILE_ATTRIBUTE_DIRECTORY)||
       (attrs&FILE_ATTRIBUTE_REPARSE_POINT))return false;
    CraftRuntimeHost446 host(account,name,nullptr,true,loginSession);
    if(!host.Id()||host.Path().empty())return false;
    CraftDatDiskPort446 port(kCraftRoot446,host.Path(),host);
    const auto pending=port.PendingFor(host.Id());
    if(!port.Healthy())return false;
    if(!pending.exists)return true;
    rsPLAYINFO* current=FindUserFromName(const_cast<char*>(name));
    if(current && current!=loginSession)return false;
    summit443::CraftDatCoordinator443 coordinator(port);
    const auto result=coordinator.RecoverBeforeLogin(host.Id());
    return result==summit443::CraftResult::Committed ||
           result==summit443::CraftResult::ReplayCommitted;
}
} // namespace summit446
