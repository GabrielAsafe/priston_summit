#pragma once
// SUMMIT439A: pure in-memory, server-authoritative stack operations.
// Does NOT reinterpret PotionCount or write any player DAT. No live activation.
#include <algorithm>
#include <cstdint>
#include <limits>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace summit_ci439 {
constexpr std::uint32_t kMaxStack=999u;  // Policy pending canonical Content publication.
constexpr std::uint32_t kMaxSlot=99u;
struct Stack {
    std::string code;
    std::uint64_t identity=0; // server-issued; never from unauthenticated client
    std::uint64_t fingerprint=0; // includes ALL binding/quality/expiry/variant metadata
    std::uint32_t quantity=0;
    unsigned bag=0;  // 1 or 2
    unsigned slot=0; // native zero-based slot
};
struct State {
    std::vector<Stack> stacks;
    std::set<std::string> acceptedRequests; // runtime must persist these in journal
    std::set<std::uint64_t> seenNativeIds;  // identity provenance; MUST persist in journal
};
enum class Result { Applied, Replay, Invalid, Missing, NoSpace, Mismatch, Insufficient, Overflow };
inline bool IsCi(const std::string& code) {
    if(code.size()!=5||code[0]!='C'||code[1]!='I')return false;
    for(int i=2;i<5;++i)if(code[i]<'0'||code[i]>'9')return false;
    const int value=(code[2]-'0')*100+(code[3]-'0')*10+(code[4]-'0');
    return value>=101&&value<=168;
}
inline bool ValidRequest(const std::string& request) {
    if(request.size()<8||request.size()>64)return false;
    for(char ch:request)if(!((ch>='a'&&ch<='z')||(ch>='A'&&ch<='Z')||
        (ch>='0'&&ch<='9')||ch=='-'||ch=='_'))return false;
    return true;
}
inline bool Valid(const State& state) {
    if(state.stacks.size()>200u)return false;
    std::set<std::pair<unsigned,unsigned>> places;
    std::set<std::uint64_t> ids;
    for(const auto& s:state.stacks) {
        if(!IsCi(s.code)||!s.identity||!s.fingerprint||s.quantity<1||s.quantity>kMaxStack||
            (s.bag!=1&&s.bag!=2)||s.slot>kMaxSlot||
            !places.insert(std::make_pair(s.bag,s.slot)).second||
            !ids.insert(s.identity).second)return false;
    }
    for(const auto& req:state.acceptedRequests)if(!ValidRequest(req))return false;
    for(auto id:state.seenNativeIds)if(!id)return false;
    return true;
}
inline std::uint64_t Total(const State& state,const std::string& code) {
    std::uint64_t n=0;
    for(const auto& s:state.stacks)if(s.code==code)n+=s.quantity;
    return n;
}
inline int Find(const State& state,unsigned bag,unsigned slot) {
    for(std::size_t i=0;i<state.stacks.size();++i)
        if(state.stacks[i].bag==bag&&state.stacks[i].slot==slot)return static_cast<int>(i);
    return -1;
}
inline Result Begin(const State& state,const std::string& request) {
    if(!Valid(state)||!ValidRequest(request))return Result::Invalid;
    return state.acceptedRequests.count(request)?Result::Replay:Result::Applied;
}
inline void Commit(State& target,State&& next,const std::string& request) {
    next.acceptedRequests.insert(request);
    target=std::move(next);
}
// All operations use copy-on-write: on failure no field is mutated.
inline Result Merge(State& state,const std::string& request,
                    unsigned srcBag,unsigned srcSlot,unsigned dstBag,unsigned dstSlot,
                    std::uint32_t amount) {
    const Result gate=Begin(state,request);if(gate!=Result::Applied)return gate;
    if(!amount || (srcBag==dstBag&&srcSlot==dstSlot))return Result::Invalid;
    State n=state;
    int src=Find(n,srcBag,srcSlot),dst=Find(n,dstBag,dstSlot);
    if(src<0||dst<0)return Result::Missing;
    const Stack a=n.stacks[src], b=n.stacks[dst];
    if(a.code!=b.code||a.fingerprint!=b.fingerprint)return Result::Mismatch;
    if(amount>a.quantity)return Result::Insufficient;
    if(amount>kMaxStack-b.quantity)return Result::Overflow;
    n.stacks[dst].quantity+=amount;
    n.stacks[src].quantity-=amount;
    if(n.stacks[src].quantity==0)n.stacks.erase(n.stacks.begin()+src);
    n.seenNativeIds.insert(a.identity); n.seenNativeIds.insert(b.identity);
    if(!Valid(n))return Result::Invalid;
    Commit(state,std::move(n),request);
    return Result::Applied;
}
inline Result Split(State& state,const std::string& request,
                    unsigned srcBag,unsigned srcSlot,unsigned dstBag,unsigned dstSlot,
                    std::uint32_t amount,std::uint64_t freshServerIdentity) {
    const Result gate=Begin(state,request);if(gate!=Result::Applied)return gate;
    if(!amount||!freshServerIdentity||(srcBag==dstBag&&srcSlot==dstSlot)||
        (dstBag!=1&&dstBag!=2)||dstSlot>kMaxSlot)return Result::Invalid;
    State n=state;
    int src=Find(n,srcBag,srcSlot);
    if(src<0)return Result::Missing;
    if(Find(n,dstBag,dstSlot)>=0)return Result::NoSpace;
    for(const auto& s:n.stacks)if(s.identity==freshServerIdentity)return Result::Mismatch;
    if(amount>=n.stacks[src].quantity)return Result::Insufficient;
    Stack part=n.stacks[src];part.identity=freshServerIdentity;
    part.quantity=amount;part.bag=dstBag;part.slot=dstSlot;
    n.stacks[src].quantity-=amount;
    n.stacks.push_back(part);
    if(n.seenNativeIds.count(freshServerIdentity))return Result::Mismatch;
    n.seenNativeIds.insert(freshServerIdentity);
    if(!Valid(n))return Result::Invalid;
    Commit(state,std::move(n),request);
    return Result::Applied;
}
inline Result Consume(State& state,const std::string& request,
                      unsigned bag,unsigned slot,std::uint32_t amount) {
    const Result gate=Begin(state,request);if(gate!=Result::Applied)return gate;
    if(!amount)return Result::Invalid;
    State n=state;int at=Find(n,bag,slot);if(at<0)return Result::Missing;
    if(amount>n.stacks[at].quantity)return Result::Insufficient;
    n.seenNativeIds.insert(n.stacks[at].identity);
    n.stacks[at].quantity-=amount;
    if(n.stacks[at].quantity==0)n.stacks.erase(n.stacks.begin()+at);
    if(!Valid(n))return Result::Invalid;
    Commit(state,std::move(n),request);
    return Result::Applied;
}
inline Result Receive(State& state,const std::string& request,const Stack& addition,
                      bool authenticatedServerDrop) {
    const Result gate=Begin(state,request);if(gate!=Result::Applied)return gate;
    if(!authenticatedServerDrop||!IsCi(addition.code)||!addition.identity||!addition.fingerprint||
        addition.quantity<1||addition.quantity>kMaxStack)return Result::Invalid;
    State n=state;
    if(Find(n,addition.bag,addition.slot)>=0)return Result::NoSpace;
    if(n.seenNativeIds.count(addition.identity))return Result::Mismatch;
    for(const auto& s:n.stacks)if(s.identity==addition.identity)return Result::Mismatch;
    n.stacks.push_back(addition);
    n.seenNativeIds.insert(addition.identity);
    if(!Valid(n))return Result::Invalid;
    Commit(state,std::move(n),request);
    return Result::Applied;
}
// Auto-fill compatible partial stacks in A+B, then create slots only for overflow.
// New slot identities must come from a trusted, server-side allocator.
// The calling runtime MUST journal this entire plan together with the DAT image.
inline Result ReceiveAuto(State& state,const std::string& request,
                          const std::string& code,std::uint64_t incomingIdentity,
                          std::uint64_t fingerprint,std::uint32_t quantity,
                          const std::vector<std::uint64_t>& freshServerIdentities,
                          bool authenticatedServerDrop) {
    const Result gate=Begin(state,request);if(gate!=Result::Applied)return gate;
    if(!authenticatedServerDrop||!IsCi(code)||!incomingIdentity||!fingerprint||
       quantity<1||quantity>10000u||state.seenNativeIds.count(incomingIdentity))return Result::Invalid;
    for(const auto& s:state.stacks)if(s.identity==incomingIdentity)return Result::Mismatch;
    State n=state;
    const std::uint64_t before=Total(n,code);
    std::uint32_t remaining=quantity;
    for(auto& s:n.stacks) {
        if(s.code!=code||s.fingerprint!=fingerprint||s.quantity>=kMaxStack)continue;
        const std::uint32_t add=std::min(remaining,kMaxStack-s.quantity);
        s.quantity+=add;remaining-=add;
        if(!remaining)break;
    }
    std::size_t extra=0;
    while(remaining) {
        unsigned bag=0,slot=0;
        for(unsigned b=1;b<=2&&!bag;++b)
            for(unsigned sl=0;sl<=kMaxSlot;++sl)
                if(Find(n,b,sl)<0){bag=b;slot=sl;break;}
        if(!bag)return Result::NoSpace;
        std::uint64_t identity=0;
        if(extra==0)identity=incomingIdentity;
        else if(extra-1<freshServerIdentities.size())identity=freshServerIdentities[extra-1];
        if(!identity||n.seenNativeIds.count(identity))return Result::Invalid;
        for(const auto& s:n.stacks)if(s.identity==identity)return Result::Mismatch;
        Stack created;created.code=code;created.identity=identity;
        created.fingerprint=fingerprint;created.quantity=std::min(remaining,kMaxStack);
        created.bag=bag;created.slot=slot;
        n.stacks.push_back(created);
        remaining-=created.quantity;++extra;
        n.seenNativeIds.insert(identity);
    }
    n.seenNativeIds.insert(incomingIdentity); // even when all units were absorbed
    if(Total(n,code)!=before+quantity||!Valid(n))return Result::Invalid;
    Commit(state,std::move(n),request);
    return Result::Applied;
}
} // namespace summit_ci439
