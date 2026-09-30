#include "gameplay/inventory/v2/PotionStack.h"

#include <cassert>
#include <iostream>
#include <map>

namespace
{
class FakePotionStacks final : public gameplay::inventory::IPotionStackQuery,
                               public gameplay::inventory::IPotionStackCommands
{
public:
    bool TryGetStack(gameplay::PlayerId player, gameplay::ItemId potion,
                     gameplay::inventory::PotionStackSnapshot& out) const override
    {
        const auto it = counts.find(Key(player, potion));
        if(!player.Valid() || !potion.Valid() || it == counts.end()) return false;
        out = gameplay::inventory::PotionStackSnapshot(player, potion, it->second);
        return true;
    }
    gameplay::inventory::PotionStackMutationResult AddStack(
        const gameplay::inventory::PotionStackMutationRequest& request) override
    {
        gameplay::inventory::PotionStackMutationResult result; result.request=request;
        auto it=counts.find(Key(request.player, request.potion));
        if(request.quantity<=0 || it==counts.end()) { result.code=gameplay::inventory::PotionStackMutationCode::InvalidRequest; return result; }
        result.before=it->second; it->second+=request.quantity; result.after=it->second;
        result.code=gameplay::inventory::PotionStackMutationCode::Applied; return result;
    }
    gameplay::inventory::PotionStackMutationResult RemoveStack(
        const gameplay::inventory::PotionStackMutationRequest& request) override
    {
        gameplay::inventory::PotionStackMutationResult result; result.request=request;
        auto it=counts.find(Key(request.player, request.potion));
        if(request.quantity<=0 || it==counts.end()) { result.code=gameplay::inventory::PotionStackMutationCode::InvalidRequest; return result; }
        result.before=it->second;
        if(it->second<request.quantity) { result.code=gameplay::inventory::PotionStackMutationCode::InsufficientQuantity; return result; }
        it->second-=request.quantity; result.after=it->second; result.code=gameplay::inventory::PotionStackMutationCode::Applied; return result;
    }
    gameplay::inventory::PotionStackTransferResult TransferStack(
        const gameplay::inventory::PotionStackTransferRequest& request) override
    {
        gameplay::inventory::PotionStackTransferResult result; result.request=request;
        auto s=counts.find(Key(request.source, request.potion));
        auto d=counts.find(Key(request.destination, request.potion));
        if(request.quantity<=0 || s==counts.end() || d==counts.end()) { result.code=gameplay::inventory::PotionStackTransferCode::InvalidRequest; return result; }
        result.sourceBefore=s->second; result.destinationBefore=d->second;
        if(s->second<request.quantity) { result.code=gameplay::inventory::PotionStackTransferCode::InsufficientQuantity; return result; }
        s->second-=request.quantity; d->second+=request.quantity;
        result.sourceAfter=s->second; result.destinationAfter=d->second; result.code=gameplay::inventory::PotionStackTransferCode::Applied; return result;
    }
    std::map<std::uint64_t, gameplay::inventory::PotionQuantity> counts;
    static std::uint64_t Key(gameplay::PlayerId p, gameplay::ItemId i) { return (static_cast<std::uint64_t>(p.value)<<32u)|i.value; }
};
}
int main(){ using namespace gameplay; using namespace gameplay::inventory; FakePotionStacks s; ItemId potion(0x100u); s.counts[FakePotionStacks::Key(PlayerId(1),potion)]=10; s.counts[FakePotionStacks::Key(PlayerId(2),potion)]=2; auto a=s.AddStack(PotionStackMutationRequest(PlayerId(1),potion,3)); assert(a.Applied() && a.before==10 && a.after==13); auto d=s.RemoveStack(PotionStackMutationRequest(PlayerId(1),potion,3)); assert(d.Applied() && d.before==13 && d.after==10); auto r=s.TransferStack(PotionStackTransferRequest(PlayerId(1),PlayerId(2),potion,4)); assert(r.Applied()); assert(r.sourceAfter==6 && r.destinationAfter==6); std::cout<<"potion stack smoke: PASS\n"; }
