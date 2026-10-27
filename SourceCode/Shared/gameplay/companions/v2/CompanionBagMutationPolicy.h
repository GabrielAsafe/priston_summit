#pragma once
// Pure V2 placement/order planner. DAT parsing and durable writes are performed
// only by the legacy persistence adapter. No external item payloads are accepted.
#include <vector>
#include <algorithm>
#include "gameplay/inventory/v2/CompanionBagSpacePolicy.h"

namespace gameplay { namespace companions { namespace v2 {
enum class BagMutationCode { Ok, Invalid, NoSpace, Corrupt, FootprintMissing };
struct BagMutationItem {
    int position=0, bag=0, x=0, y=0, width=0, height=0;
    bool hidden=false;
};
struct BagMutationPlan {
    BagMutationCode status=BagMutationCode::Invalid;
    std::vector<BagMutationItem> items;
    std::vector<int> order; // output record index -> input record index
    int split=0;
    bool Allowed() const { return status==BagMutationCode::Ok; }
};
class CompanionBagMutationPolicy final {
public:
    static BagMutationPlan Prepare(const std::vector<BagMutationItem>& input,
        int split, int recordIndex, int destinationBag, int cellX=-1, int cellY=-1) {
        BagMutationPlan p;
        p.items=input;
        p.split=split;
        if(split<0 || split>static_cast<int>(input.size()) ||
           (destinationBag!=1 && destinationBag!=2)) return p;
        p.order.reserve(input.size());
        for(int i=0;i<static_cast<int>(input.size());++i) {
            p.order.push_back(i);
            if(input[i].bag!=(i<split?1:2)) return p;
        }
        using namespace gameplay::inventory;
        if((cellX<0)!=(cellY<0) || cellX>=12 || cellY>=6 || cellX < -1 || cellY < -1)
            return p;
        if(recordIndex>=0) {
            if(recordIndex>=static_cast<int>(input.size())) return p;
            const auto& selected=input[recordIndex];
            if((selected.bag==destinationBag) != (cellX>=0) ||
               selected.position!=0 || selected.hidden)
                return p;
            if(selected.width<=0 || selected.height<=0) {
                p.status=BagMutationCode::FootprintMissing; return p;
            }
            std::vector<BagRect> occupied;
            for(int i=0;i<static_cast<int>(input.size());++i) {
                if(i==recordIndex || input[i].bag!=destinationBag ||
                   input[i].position!=0 || input[i].hidden) continue;
                if(input[i].width<=0 || input[i].height<=0) {
                    p.status=BagMutationCode::FootprintMissing; return p;
                }
                occupied.emplace_back(input[i].x,input[i].y,input[i].width,input[i].height);
            }
            BagSpaceResult fit(BagSpaceStatus::NoSpace);
            if(cellX>=0) {
                const BagRect candidate(CompanionBagSpacePolicy::OriginX+cellX*22,
                    CompanionBagSpacePolicy::OriginY+cellY*22,selected.width,selected.height);
                if(!CompanionBagSpacePolicy::InBounds(candidate)) return p;
                for(std::size_t i=0;i<occupied.size();++i) {
                    if(!CompanionBagSpacePolicy::InBounds(occupied[i])) {
                        p.status=BagMutationCode::Corrupt;return p;
                    }
                    if(CompanionBagSpacePolicy::Intersects(candidate,occupied[i])) {
                        p.status=BagMutationCode::NoSpace;return p;
                    }
                    for(std::size_t j=0;j<i;++j)
                        if(CompanionBagSpacePolicy::Intersects(occupied[i],occupied[j])) {
                            p.status=BagMutationCode::Corrupt;return p;
                        }
                }
                fit=BagSpaceResult(BagSpaceStatus::Available,candidate.x,candidate.y);
            } else fit=CompanionBagSpacePolicy::FindFree(occupied,selected.width,selected.height);
            if(!fit.Available()) {
                p.status=fit.status==BagSpaceStatus::NoSpace ? BagMutationCode::NoSpace : BagMutationCode::Corrupt;
                return p;
            }
            p.items[recordIndex].x=fit.x;
            p.items[recordIndex].y=fit.y;
            p.items[recordIndex].bag=destinationBag;
            if(cellX<0) {
                if(destinationBag==2) {
                    std::rotate(p.order.begin()+recordIndex,p.order.begin()+recordIndex+1,p.order.begin()+split);
                    --p.split;
                } else {
                    std::rotate(p.order.begin()+split,p.order.begin()+recordIndex,p.order.begin()+recordIndex+1);
                    ++p.split;
                }
            }
        } else if(recordIndex==-1) {
            // Deterministically repack only physical items in the chosen bag.
            // Quest-hidden items and equipment retain exact coordinates.
            std::vector<BagRect> occupied;
            for(int i=0;i<static_cast<int>(input.size());++i) {
                if(input[i].bag!=destinationBag || input[i].position!=0 || input[i].hidden) continue;
                if(input[i].width<=0 || input[i].height<=0) {
                    p.status=BagMutationCode::FootprintMissing; return p;
                }
                const auto fit=CompanionBagSpacePolicy::FindFree(occupied,input[i].width,input[i].height);
                if(!fit.Available()) {
                    p.status=fit.status==BagSpaceStatus::NoSpace ? BagMutationCode::NoSpace : BagMutationCode::Corrupt;
                    return p;
                }
                p.items[i].x=fit.x;
                p.items[i].y=fit.y;
                occupied.emplace_back(fit.x,fit.y,input[i].width,input[i].height);
            }
        } else return p;
        p.status=BagMutationCode::Ok;
        return p;
    }
};
}}} // gameplay::companions::v2
