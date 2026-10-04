#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

namespace gameplay
{
namespace commerce
{
enum class PremiumPurchaseScope
{
    CoinShop = 0,
    TimeShop
};

struct PremiumPurchaseOperation
{
    std::string purchaserKey;
    PremiumPurchaseScope scope;
    std::uint32_t requestId;
    std::string itemCode;

    PremiumPurchaseOperation()
        : purchaserKey(), scope(PremiumPurchaseScope::CoinShop), requestId(0u), itemCode() {}
};

enum class PremiumPurchaseReplayCode
{
    Reserved = 0,
    LegacyUntracked,
    DuplicateCommitted,
    DuplicateInFlight,
    RequestConflict,
    InvalidOperation
};

struct PremiumPurchaseReplayResult
{
    PremiumPurchaseReplayCode code;
    explicit PremiumPurchaseReplayResult(PremiumPurchaseReplayCode value)
        : code(value) {}

    bool MayExecute() const
    {
        return code == PremiumPurchaseReplayCode::Reserved ||
               code == PremiumPurchaseReplayCode::LegacyUntracked;
    }

    bool Reserved() const { return code == PremiumPurchaseReplayCode::Reserved; }
};

class IPremiumPurchaseReplayStore
{
public:
    virtual ~IPremiumPurchaseReplayStore() = default;
    virtual PremiumPurchaseReplayResult Reserve(const PremiumPurchaseOperation& operation) = 0;
    virtual bool Commit(const PremiumPurchaseOperation& operation) = 0;
    virtual bool Abort(const PremiumPurchaseOperation& operation) = 0;
};

// Process-local replay protection for premium purchases.  Account identity is
// used instead of PlayerId so a reconnect that receives a new runtime serial
// cannot replay a request that was already committed in this server process.
// requestId==0 preserves compatibility with older clients as an explicitly
// untracked request; upgraded clients always send a non-zero request id.
class PremiumPurchaseReplayGuardService final : public IPremiumPurchaseReplayStore
{
public:
    explicit PremiumPurchaseReplayGuardService(std::size_t maxReceiptsPerPurchaser = 128u)
        : maxReceiptsPerPurchaser_(maxReceiptsPerPurchaser) {}

    PremiumPurchaseReplayResult Reserve(const PremiumPurchaseOperation& operation) override
    {
        if(operation.purchaserKey.empty() || operation.itemCode.empty())
            return PremiumPurchaseReplayResult(PremiumPurchaseReplayCode::InvalidOperation);
        if(operation.requestId == 0u)
            return PremiumPurchaseReplayResult(PremiumPurchaseReplayCode::LegacyUntracked);

        std::lock_guard<std::mutex> lock(mutex_);
        Receipt* existing = FindMutable(operation);
        if(existing)
        {
            if(existing->itemCode != operation.itemCode)
                return PremiumPurchaseReplayResult(PremiumPurchaseReplayCode::RequestConflict);
            return PremiumPurchaseReplayResult(existing->committed
                ? PremiumPurchaseReplayCode::DuplicateCommitted
                : PremiumPurchaseReplayCode::DuplicateInFlight);
        }

        if(maxReceiptsPerPurchaser_ == 0u)
            return PremiumPurchaseReplayResult(PremiumPurchaseReplayCode::InvalidOperation);

        PruneFor(operation.purchaserKey, operation.scope);
        Receipt receipt;
        receipt.purchaserKey = operation.purchaserKey;
        receipt.scope = operation.scope;
        receipt.requestId = operation.requestId;
        receipt.itemCode = operation.itemCode;
        receipt.committed = false;
        receipts_.push_back(receipt);
        return PremiumPurchaseReplayResult(PremiumPurchaseReplayCode::Reserved);
    }

    bool Commit(const PremiumPurchaseOperation& operation) override
    {
        if(operation.requestId == 0u)
            return true;
        std::lock_guard<std::mutex> lock(mutex_);
        Receipt* receipt = FindMutable(operation);
        if(!receipt || receipt->itemCode != operation.itemCode)
            return false;
        receipt->committed = true;
        return true;
    }

    bool Abort(const PremiumPurchaseOperation& operation) override
    {
        if(operation.requestId == 0u)
            return true;
        std::lock_guard<std::mutex> lock(mutex_);
        const std::size_t before = receipts_.size();
        receipts_.erase(std::remove_if(receipts_.begin(), receipts_.end(),
            [&operation](const Receipt& receipt)
            {
                return MatchesIdentity(receipt, operation) && !receipt.committed;
            }), receipts_.end());
        return receipts_.size() != before;
    }

private:
    struct Receipt
    {
        std::string purchaserKey;
        PremiumPurchaseScope scope;
        std::uint32_t requestId;
        std::string itemCode;
        bool committed;

        Receipt()
            : purchaserKey(), scope(PremiumPurchaseScope::CoinShop), requestId(0u),
              itemCode(), committed(false) {}
    };

    static bool MatchesIdentity(const Receipt& receipt,
                                const PremiumPurchaseOperation& operation)
    {
        return receipt.purchaserKey == operation.purchaserKey &&
               receipt.scope == operation.scope &&
               receipt.requestId == operation.requestId;
    }

    Receipt* FindMutable(const PremiumPurchaseOperation& operation)
    {
        for(std::vector<Receipt>::iterator it = receipts_.begin(); it != receipts_.end(); ++it)
            if(MatchesIdentity(*it, operation))
                return &*it;
        return 0;
    }

    void PruneFor(const std::string& purchaserKey, PremiumPurchaseScope scope)
    {
        std::size_t count = 0u;
        for(std::vector<Receipt>::const_iterator it = receipts_.begin(); it != receipts_.end(); ++it)
            if(it->purchaserKey == purchaserKey && it->scope == scope)
                ++count;

        while(count >= maxReceiptsPerPurchaser_)
        {
            std::vector<Receipt>::iterator oldest = receipts_.end();
            for(std::vector<Receipt>::iterator it = receipts_.begin(); it != receipts_.end(); ++it)
            {
                if(it->purchaserKey == purchaserKey && it->scope == scope && it->committed)
                {
                    oldest = it;
                    break;
                }
            }
            if(oldest == receipts_.end())
                break; // Never evict an in-flight reservation.
            receipts_.erase(oldest);
            --count;
        }
    }

    std::size_t maxReceiptsPerPurchaser_;
    std::vector<Receipt> receipts_;
    std::mutex mutex_;
};
}
}
