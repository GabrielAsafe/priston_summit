#pragma once
// SUMMIT 350J: finish an ACKed, validated legacy save before opening the
// two-DAT trade intent. Only call this with BOTH participant writes excluded
// and an authenticated session lease. This is a normal, exact single-DAT
// save: it does not transfer items or report success to the client.
#include "gameplay/companions/v2/CompanionTradeExactRecordStore.h"
#include <cstdint>
#include <string>
#include <vector>

namespace gameplay { namespace companions { namespace v2 {
enum class AcceptedOwnerBaseline350J { Ready, Refused, Uncertain };

inline AcceptedOwnerBaseline350J MaterializeAcceptedOwner350J(
    const std::string& ownerPath,
    const std::vector<std::uint8_t>& currentOnDisk,
    const std::vector<std::uint8_t>& validatedAcceptedImage)
{
    if(ownerPath.empty() || currentOnDisk.empty() ||
       validatedAcceptedImage.empty() ||
       currentOnDisk.size()>CompanionTradeExactRecordStore::kMaximumRecordBytes ||
       validatedAcceptedImage.size()>CompanionTradeExactRecordStore::kMaximumRecordBytes)
        return AcceptedOwnerBaseline350J::Refused;
    if(currentOnDisk==validatedAcceptedImage)
        return AcceptedOwnerBaseline350J::Ready;
    const TradeApplyResult result=CompanionTradeExactRecordStore::Replace(
        ownerPath,currentOnDisk,validatedAcceptedImage);
    if(result==TradeApplyResult::Conflict)
        return AcceptedOwnerBaseline350J::Refused;
    if(result!=TradeApplyResult::AppliedDurably &&
       result!=TradeApplyResult::AlreadyApplied)
        return AcceptedOwnerBaseline350J::Uncertain;
    std::vector<std::uint8_t> check;
    if(!CompanionTradeExactRecordStore::Read(ownerPath,check) ||
       check!=validatedAcceptedImage)
        return AcceptedOwnerBaseline350J::Uncertain;
    return AcceptedOwnerBaseline350J::Ready;
}

}}} // gameplay::companions::v2
