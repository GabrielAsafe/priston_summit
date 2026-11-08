// SUMMIT456L: live legacy inventory craft. Included exactly ONCE at the end
// of record_legacy_server.inl, after the original rsRECORD_DBASE entrypoints.
// Recipe metadata is canonical; execution uses the old item factory and
// inventory ledger. It never writes or decodes a character DAT on disk.
#include "gameplay/crafting/v2/CraftLegacyLiveApi456.h"
#include "gameplay/crafting/v2/CraftPhysicalXP467.h" // SUMMIT467A_PHYSICAL_XP_HOOK
#include "gameplay/crafting/v2/CraftLegacyLiveWire456.h"
#include "systems/gameplay/crafting/v2/CraftRegistry425.generated.h"
#include "gameplay/crafting/v2/CraftLiveCatalog460.generated.h" // SUMMIT460A
#include "systems/gameplay/crafting/v2/CraftRecipeActivation444.generated.h"
#include "systems/gameplay/crafting/v2/CraftJsonOnlyDispatch432.h"
#include "systems/gameplay/crafting/v2/CraftPhysicalSelection425.h"
#include "systems/inventory/v2/legacy/LegacyInventoryRuntime.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"
#include "gameplay/companions/v2/CompanionTradeParticipantIdentity.h"
#include "gameplay/companions/v2/CompanionTradeParticipantGate350B.h"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <deque>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <vector>
#include <cstdint>
#include <atomic>
#include <memory>
#include <random>
#include "systems/persistence/database/core/Database.h"
#include "gameplay/crafting/v2/CraftQuality463.generated.h" // SUMMIT463A

extern int CreateItem(psITEM*,char*,int);
extern int rsRegist_ItemSecCode(rsPLAYINFO*,TRANS_ITEMINFO*,int);
extern int SendQuickMessage(rsPLAYINFO*,char*);

namespace summit456l {
namespace {
struct Pending {
    std::uint64_t participant=0;
    gameplay::PlayerHandle session;
    rsPLAYINFO* player=nullptr;
    std::string request;
    std::string recipe460;
    std::uint32_t token=0, created=0;
    int stage=0; // 1=prepare, 2=commit sent, 3=client saved, 4=done, 5=uncertain
    bool saveAccepted=false;
    bool waitingForSave=false;
    unsigned commitResends457=0;
    std::vector<Identity> inputs;
    TRANS_ITEMINFO reward{};
};
std::mutex& Lock() {static std::mutex m;return m;}
std::map<std::uint64_t,Pending>& State() {static std::map<std::uint64_t,Pending> m;return m;}
// Suppress reuse of older request IDs within a live server session, even
// after a newer request has overwritten the last completed Pending state.
std::map<std::uint64_t,std::deque<std::string>>& Seen() {
    static std::map<std::uint64_t,std::deque<std::string>> m;return m;
}
std::atomic<std::uint32_t>& Counter(){static std::atomic<std::uint32_t> n(0);return n;}
// SUMMIT463A: profession data belongs to ServerDB, keyed by authenticated
// player name, same as existing 378A CraftMaterial and 407A snapshots.
// This SELECT runs BEFORE item creation and before the 456L ledger debit.
// If the migration/DB is missing, the recipe is refused, not made free.
bool Mastery463(rsPLAYINFO* player,int type,int minimum,int& level){
    level=0;
    if(!player||!player->getName()||type<1||type>7||minimum<0||minimum>50)return false;
    std::shared_ptr<SQLConnection> db=SQLConnection::GetConnection(DATABASEID_ServerDB);
    if(!db||!db->Open())return false;
    char key[256]={0};std::strncpy(key,player->getName(),sizeof(key)-1);
    bool ok=db->Prepare("SELECT Level FROM ProfessionProgress WHERE PlayerKey=? AND ProfessionType=?") &&
        db->BindInputParameter(key,1,PARAMTYPE_String) &&
        db->BindInputParameter(&type,2,PARAMTYPE_Integer) && db->Execute(false);
    if(ok && db->NextRow())
        ok=db->GetData(&level,sizeof(level),1,PARAMTYPE_Integer)!=FALSE;
    else ok=false; // no profession record: must learn profession first
    db->Close();return ok && level>=minimum && level<=50;
}
void Diagnostic(const char* text){std::fprintf(stderr,"[SUMMIT456L][CRAFT] %s\n",text);}
bool Current(const Pending& p) {
    auto& r=gameplay::player::legacy::ServerLegacyOnlinePlayerRegistry();
    return p.player && p.player->lpsmSock && p.session.Valid() &&
           r.Resolve(p.session)==p.player;
}
bool Send(rsPLAYINFO* p,const Delivery<TRANS_ITEMINFO>& message) {
    return p && p->lpsmSock &&
        p->lpsmSock->Send((char*)&message,static_cast<int>(sizeof(message)),TRUE)==TRUE;
}
void Quick(rsPLAYINFO* p,const std::string& message) {
    if(p) (void)SendQuickMessage(p,const_cast<char*>(message.c_str()));
}
Delivery<TRANS_ITEMINFO> Packet(const Pending& p,int step){
    Delivery<TRANS_ITEMINFO> out={};
    out.size=static_cast<int>(sizeof(out));out.code=kOpcode;
    out.magic=kMagic;out.token=p.token;out.step=step;
    out.count=static_cast<int>(p.inputs.size());
    for(std::size_t i=0;i<p.inputs.size();++i)out.inputs[i]=p.inputs[i];
    out.reward=p.reward;return out;
}
using Runtime=gameplay::inventory::legacy::LegacyInventoryRuntime;
using Instance=gameplay::inventory::ItemInstanceId;
Instance ToInstance(const Identity& x){
    return Instance(gameplay::ItemId(x.code),x.head,x.checksum);
}
bool SnapshotHas(rsPLAYINFO* p,const std::vector<Identity>& inputs){
    if(!p)return false;
    auto& registry=gameplay::player::legacy::ServerLegacyOnlinePlayerRegistry();
    const auto handle=registry.HandleOf(p);
    if(!handle.Valid()||registry.Resolve(handle)!=p)return false;
    gameplay::inventory::InventorySnapshot fresh;
    if(!gameplay::inventory::legacy::ServerLegacyInventoryRuntime().TryGetSnapshot(handle.id,fresh))return false;
    for(const auto& x:inputs){
        unsigned occurrences=0;
        for(const auto& s:fresh.slots)if(
            s.item.item.value==x.code && s.item.key==x.head &&
            s.item.checksum==x.checksum)++occurrences;
        if(occurrences!=1)return false;
    }
    return registry.Resolve(handle)==p;
}
bool ApplyLedger(const Pending& p){
    if(!Current(p)||!SnapshotHas(p.player,p.inputs))return false;
    const std::uint64_t participant=p.participant;
    auto& gate=gameplay::companions::v2::ServerTradeParticipantGate350B();
    if(!gate.TryReserveSingle(participant))return false;
    struct Unlock {gameplay::companions::v2::TradeParticipantGate350B& g;
        std::uint64_t id;~Unlock(){(void)g.ReleaseSingleVerified(id);} } unlock{gate,participant};
    if(!SnapshotHas(p.player,p.inputs))return false;
    auto& inventory=gameplay::inventory::legacy::ServerLegacyInventoryRuntime();
    const auto playerId=p.session.id;
    std::vector<Identity> removed;
    for(const auto& x:p.inputs){
        if(inventory.Remove(playerId,ToInstance(x)).code!=
           gameplay::inventory::InventoryMutationCode::Applied) {
            for(auto it=removed.rbegin();it!=removed.rend();++it)
                if(inventory.Add(playerId,ToInstance(*it)).code!=
                     gameplay::inventory::InventoryMutationCode::Applied)
                    Diagnostic("fatal=ledger_rollback_failed");
            return false;
        }
        removed.push_back(x);
    }
    const auto& it=p.reward.Item;
    const Identity output(it.CODE,it.ItemHeader.Head,it.ItemHeader.dwChkSum);
    if(inventory.Add(playerId,ToInstance(output)).code!=
       gameplay::inventory::InventoryMutationCode::Applied) {
        for(auto it=removed.rbegin();it!=removed.rend();++it)
            if(inventory.Add(playerId,ToInstance(*it)).code!=
                gameplay::inventory::InventoryMutationCode::Applied)
                Diagnostic("fatal=ledger_rollback_failed");
        return false;
    }
    return true;
}
// The server's EXISTING RecordData validator has already accepted this image.
// We use the same legacy DecodeCompress used by MakeRecordItemList, on the
// just-accepted online save (not an independently decoded DAT file).
bool SavedResultMatches(const Pending& p,const TRANS_RECORD_DATA* record){
    if(!record || record->ItemCount<0 || record->ItemCount>RECORD_ITEM_MAX ||
       record->DataSize<0 || record->DataSize>static_cast<int>(sizeof(record->Data)))return false;
    const sITEMINFO& output=p.reward.Item;
    int foundOutput=0;
    std::size_t offset=0;
    for(int i=0;i<record->ItemCount;++i){
        if(offset+sizeof(int)>static_cast<std::size_t>(record->DataSize))return false;
        int length=0;std::memcpy(&length,record->Data+offset,sizeof(length));
        if(length<=static_cast<int>(sizeof(int)) ||
           static_cast<std::size_t>(length)>static_cast<std::size_t>(record->DataSize)-offset)return false;
        sRECORD_ITEM item={};
        DecodeCompress(const_cast<BYTE*>(record->Data+offset),
                       reinterpret_cast<BYTE*>(&item),RECORD_ITEM_INFO_HEAD_SIZE);
        const Identity x(item.sItemInfo.CODE,item.sItemInfo.ItemHeader.Head,
                         item.sItemInfo.ItemHeader.dwChkSum);
        if(x==Identity(output.CODE,output.ItemHeader.Head,output.ItemHeader.dwChkSum))++foundOutput;
        for(const auto& consumed:p.inputs)if(x==consumed)return false;
        offset+=static_cast<std::size_t>(length);
    }
    return offset==static_cast<std::size_t>(record->DataSize) && foundOutput==1;
}
void Finish(Pending& p){
    if(p.saveAccepted && !summit467xp::GrantVerified(p.player->getName(),p.request.c_str()))
        Diagnostic("xp467=DEFERRED_AT_FINISH");
    p.stage=4;
    if(Current(p)) {
        Quick(p.player,std::string("@S407|")+p.request+"|R|OK");
        Quick(p.player,p.recipe460+": materiais consumidos, equipamento criado e save validado pelo servidor.");
    }
    Diagnostic("result=SAVE_ACCEPTED_INGREDIENTS_REMOVED_OUTPUT_PRESENT");
}
} // namespace
BeginResult Begin(rsPLAYINFO* player,const char* recipe,const char* request){
    if(!player||!player->getAccount()||!player->getName()||!player->lpsmSock||
       !recipe||!request)return BeginResult::Rejected;
    // SUMMIT460A: recipe authorization is compiled from the canonical 424A
    // Content and the synchronized 425A native item registry. It is NOT an
    // inferred free recipe, nor is "all items" a blanket activation.
    const auto* live=summit460::Find(recipe);
    if(!live)return BeginResult::Unsupported;
    const auto* rule=summit_craft_425::Find(recipe);
    const auto* activation=summit_craft_444::Find(recipe);
    const bool activatedBy444=activation && activation->jsonEnabled && activation->complete;
    if(!live->catalogEnabled && !activatedBy444)return BeginResult::Unsupported;
    if(!rule||!rule->idValid||rule->native==0||rule->outputCount!=1||
       rule->native!=live->native||rule->size!=live->size||
       !rule->size||rule->size>kMaximumInputs||rule->zero||rule->bad||
       rule->positive!=static_cast<int>(rule->size)||
       !summit_craft_432::RequestId(request))return BeginResult::Rejected;
    std::vector<summit_craft_423::Ingredient> needs;
    std::set<std::uint32_t> codes;
    int total=0;
    for(std::size_t i=0;i<live->size;++i){
        const auto& n=summit460::kIngredients[live->first+i];
        const auto& authoritative=summit_craft_425::kIngredients[rule->first+i];
        if(std::strcmp(n.code,authoritative.code)!=0 ||
           n.native!=authoritative.native || n.count!=authoritative.count ||
           !n.native || n.count<=0 || n.count>static_cast<int>(kMaximumInputs)||
           !codes.insert(n.native).second)return BeginResult::Rejected;
        total+=n.count; needs.emplace_back(n.code,n.native,n.count);
    }
    if(total<=0||total>static_cast<int>(kMaximumInputs))return BeginResult::Rejected;
    // SUMMIT463A: server-owned skill check, no client-supplied level.
    int mastery463=0;
    if(live->professionType!=0 &&
       !Mastery463(player,live->professionType,live->minimumLevel,mastery463)) {
        Diagnostic("deny=profession_level_or_sqlite_missing before_mutation=1");
        return BeginResult::Rejected;
    }
    auto& registry=gameplay::player::legacy::ServerLegacyOnlinePlayerRegistry();
    const auto handle=registry.HandleOf(player);
    if(!handle.Valid()||registry.Resolve(handle)!=player)return BeginResult::Rejected;
    // SUMMIT457A: refuse an unresolved previous craft BEFORE selecting inputs.
    // Once the ledger was committed, missing CI is not proof that repeating
    // the operation is safe; the previous request may still be uncertain.
    const std::uint64_t participant=gameplay::companions::v2::TradeParticipantId(
        player->getAccount(),player->getName());
    if(!participant)return BeginResult::Rejected;
    {
        std::lock_guard<std::mutex> guard(Lock());
        const auto& recent=Seen()[participant];
        if(std::find(recent.begin(),recent.end(),std::string(request))!=recent.end())
            return BeginResult::Replay;
        auto it=State().find(participant);
        if(it!=State().end()){
            if(it->second.request==request)return BeginResult::Replay;
            if(it->second.stage!=4){
                if(it->second.stage==1 && (std::uint32_t)(GetTickCount()-it->second.created)>30000u)
                    State().erase(it);
                else return BeginResult::Busy;
            }else State().erase(it);
        }
    }
    gameplay::inventory::InventorySnapshot snapshot;
    if(!gameplay::inventory::legacy::ServerLegacyInventoryRuntime()
        .TryGetSnapshot(handle.id,snapshot))return BeginResult::Rejected;
    std::vector<summit_craft_423::Instance> available;
    for(const auto& s:snapshot.slots)if(codes.count(s.item.item.value))
        available.emplace_back(s.index,s.item.item.value,s.item.key,s.item.checksum);
    const auto selected=summit_craft_425::SelectInstances(needs,available);
    if(selected.status!=summit_craft_425::SelectionStatus::Ready)return BeginResult::Rejected;
    psITEM native{};
    if(CreateItem(&native,const_cast<char*>(recipe),player->smCharInfo.JOB_CODE)!=TRUE)
        return BeginResult::Rejected;
    // SUMMIT463A: generate quality only for newly authorized recipes, never
    // rewrite an old WA103 or mutate stored DAT data. The quality is stored
    // in native item stats + native ItemName, re-signed by legacy ReformItem.
    // Any incompatibility is rejected BEFORE materials are debited.
    if(live->professionType!=0){
        int tier463=0;
        std::random_device entropy463;
        const std::uint64_t seed463=(static_cast<std::uint64_t>(entropy463())<<32) ^
            static_cast<std::uint64_t>(entropy463()) ^
            static_cast<std::uint64_t>(GetTickCount()) ^
            static_cast<std::uint64_t>(Counter().fetch_add(1)+1);
        std::mt19937_64 rng463(seed463);
        if(!summit463_quality::Apply(native.ItemInfo,recipe,mastery463,rng463,tier463))
            return BeginResult::Rejected;
        std::fprintf(stderr,"[SUMMIT463A][CRAFT] recipe=%s level=%d quality_tier=%d before_debit=1\n",
            recipe,mastery463,tier463);
    }
    ReformItem(&native.ItemInfo);
    if(CheckItemForm(&native.ItemInfo)!=TRUE ||
       static_cast<std::uint32_t>(native.ItemInfo.CODE)!=rule->native||
       !native.ItemInfo.ItemHeader.Head||!native.ItemInfo.ItemHeader.dwChkSum)
        return BeginResult::Rejected;
    Pending prepared;
    prepared.participant=participant;prepared.session=handle;prepared.player=player;
    prepared.request=request;prepared.recipe460=recipe;prepared.created=static_cast<std::uint32_t>(GetTickCount());
    prepared.token=Counter().fetch_add(1)+1;if(!prepared.token)prepared.token=Counter().fetch_add(1)+1;
    prepared.stage=1;
    for(const auto& row:selected.chosen)
        prepared.inputs.emplace_back(row.nativeCode,row.key,row.checksum);
    prepared.reward.code=OpCode::OPCODE_PUTITEM;
    prepared.reward.size=sizeof(TRANS_ITEMINFO);
    prepared.reward.Item=native.ItemInfo;
    if(rsRegist_ItemSecCode(player,&prepared.reward,1)!=TRUE)
        return BeginResult::Rejected;
    if(!SnapshotHas(player,prepared.inputs))return BeginResult::Rejected;
    {
        std::lock_guard<std::mutex> guard(Lock());
        if(State().count(participant))return BeginResult::Busy;
        if(!summit467xp::ReserveIntent(player->getName(),request,recipe)) return BeginResult::Rejected;
        State()[participant]=prepared;
        auto& recent=Seen()[participant];
        recent.push_back(request);
        if(recent.size()>256u)recent.pop_front();
    }
    if(!Send(player,Packet(prepared,Prepare))){
        std::lock_guard<std::mutex> guard(Lock());
        State().erase(participant);
        // The request was transmitted unreliably; retain its nonce as seen
        // because delivery to a disconnected client cannot be disproved.
        return BeginResult::Rejected;
    }
    Diagnostic("step=PREPARE_SENT native_factory=1 dat_decode=0");
    return BeginResult::Queued;
}
void ReceiveReply(rsPLAYINFO* sender,const void* bytes,int length){
    if(!sender||!sender->getAccount()||!sender->getName()||!bytes||
       length!=static_cast<int>(sizeof(Reply)))return;
    Reply response{};std::memcpy(&response,bytes,sizeof(response));
    if(!ValidReply(response))return;
    const auto id=gameplay::companions::v2::TradeParticipantId(
         sender->getAccount(),sender->getName());
    if(!id)return;
    Pending p;
    bool resend457=false;
    {
        std::lock_guard<std::mutex> guard(Lock());
        auto it=State().find(id);
        if(it==State().end()||it->second.token!=response.token||
           !Current(it->second))return;
        if(response.step==WaitSave && it->second.stage==3){
            it->second.waitingForSave=true;
            Diagnostic("step=CLIENT_WAITING_PREVIOUS_LEGACY_SAVE no_second_debit=1");
            return;
        }
        if(response.step==Denied||response.step==SaveFailed){
            const bool beforeDebit=it->second.stage==1;
            const std::string failedRequest=it->second.request;
            std::fprintf(stderr,
                "[SUMMIT457][CRAFT] client_reply=%s reason=%s stage=%d\n",
                response.step==Denied?"denied":"save_failed",
                ReasonName(response.reason),it->second.stage);
            if(beforeDebit){
                State().erase(it);
                Quick(sender,std::string("@S407|")+failedRequest+"|R|ERR");
                Diagnostic("deny=client_inventory_prepare_no_consumption");
            }else{
                // An authenticated packet is not independent evidence of
                // a client's persisted state. Never roll back a committed
                // ledger only because the client CLAIMS it did not mutate.
                it->second.stage=5;
                Quick(sender,"Crafting inconclusivo: nao repetir. Verifique materiais e equipamento apos relogin.");
                Diagnostic("step=CLIENT_REJECTED_AFTER_LEDGER no_retry_relogin_required=1");
            }
            return;
        }
        if(response.step==Ready && it->second.stage==1){
            it->second.stage=2; // block duplicate READY before any I/O
            p=it->second;
        }else if(response.step==Ready && it->second.stage==3 &&
                 it->second.waitingForSave && it->second.commitResends457<3){
            it->second.waitingForSave=false;
            ++it->second.commitResends457;
            p=it->second;
            resend457=true;
        }else if(response.step==Applied && it->second.stage==3){
            it->second.stage=6;
            if(it->second.saveAccepted)Finish(it->second);
            else Diagnostic("step=CLIENT_APPLIED_WAITING_VALIDATED_SAVE");
            return;
        }else return;
    }
    if(resend457){
        // Same authenticated request, same output, same token. In particular
        // DO NOT call ApplyLedger twice after a legacy save ACK.
        if(!Send(p.player,Packet(p,Commit))){
            std::lock_guard<std::mutex> guard(Lock());
            auto it=State().find(id);
            if(it!=State().end()&&it->second.token==p.token)it->second.stage=5;
            Diagnostic("uncertain=resend_commit_delivery no_retry=1");
        }else Diagnostic("step=COMMIT_RESENT_AFTER_LEGACY_SAVE no_second_debit=1");
        return;
    }
    // This is the ONLY point where the server debits the legacy ledger.
    if(!ApplyLedger(p)){
        std::lock_guard<std::mutex> guard(Lock());
        auto it=State().find(id);if(it!=State().end()&&it->second.token==p.token)State().erase(it);
        Quick(p.player,std::string("@S407|")+p.request+"|R|ERR");
        Diagnostic("deny=ledger_before_commit");return;
    }
    {
        std::lock_guard<std::mutex> guard(Lock());
        auto it=State().find(id);if(it==State().end()||it->second.token!=p.token)return;
        it->second.stage=3;
    }
    if(!Send(p.player,Packet(p,Commit))){
        std::lock_guard<std::mutex> guard(Lock());
        auto it=State().find(id);if(it!=State().end()&&it->second.token==p.token)it->second.stage=5;
        Diagnostic("uncertain=commit_delivery no_retry_relogin_required=1");return;
    }
    Diagnostic("step=LEGACY_LEDGER_DEBIT_AND_GRANT_COMMIT_SENT");
}
void ValidatedSave(rsPLAYINFO* sender,const TRANS_RECORD_DATA* record){
    if(!sender||!sender->getAccount()||!sender->getName()||!record)return;
    const auto id=gameplay::companions::v2::TradeParticipantId(
        sender->getAccount(),sender->getName());
    std::lock_guard<std::mutex> guard(Lock());
    auto it=State().find(id);
    if(it==State().end()||!Current(it->second)||
       (it->second.stage!=3&&it->second.stage!=6))return;
    if(!SavedResultMatches(it->second,record)){
        Diagnostic("save=VALIDATED_BUT_CRAFT_INVENTORY_MISMATCH");return;
    }
    it->second.saveAccepted=true;
    if(!summit467xp::GrantVerified(sender->getName(),it->second.request.c_str()))
        Diagnostic("xp467=DEFERRED_SQLITE_RETRY_AFTER_VALIDATED_SAVE");
    if(it->second.stage==6)Finish(it->second);
    else Diagnostic("save=VALIDATED_AWAITING_CLIENT_ACK");
}
} // namespace summit456l
