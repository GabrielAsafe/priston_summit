#pragma once
// SUMMIT443A: deterministic, side-effect-free A+B CI stack operations.
// No DAT/SQLite/network access; an authenticated server adapter must persist
// ALL modifications atomically via the CraftCommit443 coordinator.
#include <algorithm>
#include <cstdint>
#include <limits>
#include <set>
#include <string>
#include <utility>
#include <vector>
namespace summit443 {
static const std::uint32_t kMaxStack=999;
static const unsigned kSlotsPerBag=100;
struct Stack {
    std::string code; std::uint64_t id, variant; std::uint32_t quantity;
    unsigned bag, slot;
    Stack(std::string c="",std::uint64_t i=0,std::uint64_t v=0,std::uint32_t n=0,unsigned b=0,unsigned s=0):
        code(std::move(c)),id(i),variant(v),quantity(n),bag(b),slot(s){}
};
struct Inventory { std::vector<Stack> stacks; std::set<std::uint64_t> everUsedIds; };
enum class StackStatus { OK, BadInput, Missing, Duplicate, NoSpace, Insufficient, Incompatible, Overflow };
struct Ingredient {std::string code;std::uint32_t quantity;
    Ingredient(std::string c="",std::uint32_t q=0):code(std::move(c)),quantity(q){} };
inline bool IsCi(const std::string& c) {
    if(c.size()!=5||c[0]!='C'||c[1]!='I')return false;
    for(unsigned i=2;i<5;++i)if(c[i]<'0'||c[i]>'9')return false;
    unsigned n=100u*(c[2]-'0')+10u*(c[3]-'0')+(c[4]-'0');
    return n>=101&&n<=168;
}
inline bool Valid(const Inventory& inv) {
    if(inv.stacks.size()>2*kSlotsPerBag)return false;
    std::set<std::pair<unsigned,unsigned>> positions;
    std::set<std::uint64_t> identities;
    for(const auto& x:inv.stacks){
        if(!IsCi(x.code)||!x.id||!x.variant||!x.quantity||x.quantity>kMaxStack||
           (x.bag!=1&&x.bag!=2)||x.slot>=kSlotsPerBag||
           !positions.insert({x.bag,x.slot}).second||!identities.insert(x.id).second)
            return false;
    }
    for(auto x:inv.everUsedIds)if(!x)return false;
    return true;
}
inline std::uint64_t Total(const Inventory& inv,const std::string& c){
    std::uint64_t total=0;for(const auto& x:inv.stacks)if(x.code==c)total+=x.quantity;
    return total;
}
inline int At(const Inventory& inv,unsigned bag,unsigned slot){
    for(std::size_t i=0;i<inv.stacks.size();++i)if(inv.stacks[i].bag==bag&&inv.stacks[i].slot==slot)
        return static_cast<int>(i);
    return -1;
}
inline StackStatus Credit(Inventory& target,const Stack& drop,
                          const std::vector<std::uint64_t>& freshIds,bool serverAuthorised){
    if(!serverAuthorised||!Valid(target)||!IsCi(drop.code)||!drop.id||!drop.variant||
       !drop.quantity||drop.quantity>10000||target.everUsedIds.count(drop.id))return StackStatus::BadInput;
    for(const auto& s:target.stacks)if(s.id==drop.id)return StackStatus::Duplicate;
    Inventory next=target;
    unsigned remaining=drop.quantity;
    for(auto& s:next.stacks){
        if(s.code!=drop.code||s.variant!=drop.variant||s.quantity==kMaxStack)continue;
        unsigned n=std::min(remaining,kMaxStack-s.quantity);s.quantity+=n;remaining-=n;
        if(!remaining)break;
    }
    std::size_t more=0;
    while(remaining){
        unsigned bag=0,slot=0;
        for(unsigned b=1;b<=2&&!bag;++b)
            for(unsigned q=0;q<kSlotsPerBag;++q)if(At(next,b,q)<0){bag=b;slot=q;break;}
        if(!bag)return StackStatus::NoSpace;
        const std::uint64_t id= more==0 ? drop.id : (more-1<freshIds.size()?freshIds[more-1]:0);
        if(!id||next.everUsedIds.count(id))return StackStatus::BadInput;
        for(const auto& s:next.stacks)if(s.id==id)return StackStatus::Duplicate;
        const unsigned n=std::min(remaining,kMaxStack);
        next.stacks.emplace_back(drop.code,id,drop.variant,n,bag,slot);
        next.everUsedIds.insert(id);remaining-=n;++more;
    }
    next.everUsedIds.insert(drop.id);
    if(!Valid(next)||Total(next,drop.code)!=Total(target,drop.code)+drop.quantity)
        return StackStatus::BadInput;
    target=std::move(next);return StackStatus::OK;
}
inline StackStatus MoveOrMerge(Inventory& target,unsigned srcBag,unsigned srcSlot,
                               unsigned dstBag,unsigned dstSlot,unsigned n,
                               std::uint64_t mintedSplitId=0){
    if(!Valid(target)||!n||(srcBag==dstBag&&srcSlot==dstSlot)||
       (dstBag!=1&&dstBag!=2)||dstSlot>=kSlotsPerBag)return StackStatus::BadInput;
    Inventory next=target;int a=At(next,srcBag,srcSlot),b=At(next,dstBag,dstSlot);
    if(a<0)return StackStatus::Missing;
    const Stack before=next.stacks[a];
    if(n>before.quantity)return StackStatus::Insufficient;
    if(b>=0){
        Stack& other=next.stacks[b];
        if(other.code!=before.code||other.variant!=before.variant)return StackStatus::Incompatible;
        if(n>kMaxStack-other.quantity)return StackStatus::Overflow;
        other.quantity+=n;next.stacks[a].quantity-=n;
        if(!next.stacks[a].quantity)next.stacks.erase(next.stacks.begin()+a);
        next.everUsedIds.insert(before.id);
    }else if(n==before.quantity){
        next.stacks[a].bag=dstBag;next.stacks[a].slot=dstSlot;
    }else{
        if(!mintedSplitId||next.everUsedIds.count(mintedSplitId))return StackStatus::BadInput;
        for(const auto& s:next.stacks)if(s.id==mintedSplitId)return StackStatus::Duplicate;
        next.stacks[a].quantity-=n;
        next.stacks.emplace_back(before.code,mintedSplitId,before.variant,n,dstBag,dstSlot);
        next.everUsedIds.insert(mintedSplitId);
    }
    if(!Valid(next)||Total(target,before.code)!=Total(next,before.code))return StackStatus::BadInput;
    target=std::move(next);return StackStatus::OK;
}
inline StackStatus Consume(Inventory& target,const std::vector<Ingredient>& requirements,
                           bool authorisedRecipe){
    if(!authorisedRecipe||!Valid(target)||requirements.empty()||requirements.size()>16)
        return StackStatus::BadInput;
    std::set<std::string> seen;
    for(const auto& r:requirements)
        if(!IsCi(r.code)||!r.quantity||r.quantity>10000||!seen.insert(r.code).second)
            return StackStatus::BadInput;
    Inventory next=target;
    for(const auto& r:requirements){
        unsigned remaining=r.quantity;
        // Stable slot ordering: A before B, smaller existing stacks first.
        std::vector<std::pair<unsigned,unsigned>> sorted;
        for(const auto& s:next.stacks)if(s.code==r.code) sorted.push_back({s.bag,s.slot});
        std::sort(sorted.begin(),sorted.end(),[&next](const std::pair<unsigned,unsigned>& a,
                                                      const std::pair<unsigned,unsigned>& b){
            const Stack& x=next.stacks[At(next,a.first,a.second)],&y=next.stacks[At(next,b.first,b.second)];
            if(x.quantity!=y.quantity)return x.quantity<y.quantity;
            return a<b;
        });
        for(const auto& addr:sorted){
            if(!remaining)break;
            int at=At(next,addr.first,addr.second);if(at<0)continue;
            Stack& s=next.stacks[at];unsigned n=std::min(remaining,s.quantity);
            s.quantity-=n;remaining-=n;next.everUsedIds.insert(s.id);
            if(!s.quantity)next.stacks.erase(next.stacks.begin()+at);
        }
        if(remaining)return StackStatus::Insufficient;
    }
    if(!Valid(next))return StackStatus::BadInput;
    for(const auto& r:requirements)
        if(Total(target,r.code)!=Total(next,r.code)+r.quantity)return StackStatus::BadInput;
    target=std::move(next);return StackStatus::OK;
}
// Pure two-character transfer plan. Both full DAT records MUST later commit
// under a cross-character lease and a pairwise durable journal. This function
// does not write either live character or claim pairwise atomicity on disk.
inline StackStatus Transfer(Inventory& sender,Inventory& receiver,
                            unsigned bag,unsigned slot,std::uint32_t quantity,
                            std::uint64_t mintedTransferId,
                            const std::vector<std::uint64_t>& additionalIds,
                            bool authenticatedBoth){
    if(!authenticatedBoth||!Valid(sender)||!Valid(receiver)||!quantity)
        return StackStatus::BadInput;
    const int at=At(sender,bag,slot);
    if(at<0)return StackStatus::Missing;
    const Stack original=sender.stacks[at];
    if(quantity>original.quantity)return StackStatus::Insufficient;
    const std::uint64_t transferId=quantity==original.quantity?original.id:mintedTransferId;
    if(!transferId||(sender.everUsedIds.count(transferId)&&quantity!=original.quantity))
        return StackStatus::BadInput;
    // All identities are global server identifiers, no collision across sides.
    if(receiver.everUsedIds.count(transferId))return StackStatus::Duplicate;
    for(const auto& s:receiver.stacks)if(s.id==transferId)return StackStatus::Duplicate;
    Inventory a=sender,b=receiver;
    const auto beforeTotal=Total(a,original.code)+Total(b,original.code);
    a.stacks[at].quantity-=quantity;
    a.everUsedIds.insert(original.id);
    if(!a.stacks[at].quantity)a.stacks.erase(a.stacks.begin()+at);
    if(quantity!=original.quantity)a.everUsedIds.insert(transferId);
    const auto result=Credit(b,Stack(original.code,transferId,original.variant,quantity,1,0),
                             additionalIds,true);
    if(result!=StackStatus::OK)return result;
    if(!Valid(a)||!Valid(b)||Total(a,original.code)+Total(b,original.code)!=beforeTotal)
        return StackStatus::BadInput;
    sender=std::move(a);receiver=std::move(b);
    return StackStatus::OK;
}
} // namespace summit443
