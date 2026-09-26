#pragma once

#include <cstddef>
#include <cstdint>

namespace summit {
namespace rendering {
namespace assets {
namespace v2 {

enum AssetLoadState
{
    ASSET_UNLOADED = 0,
    ASSET_LOADING,
    ASSET_LOADED,
    ASSET_FAILED_TRANSIENT,
    ASSET_FAILED_PERMANENT,
    ASSET_WAITING_DEVICE,
    ASSET_EVICTED
};

enum AssetOwnerMask
{
    ASSET_OWNER_UNKNOWN   = 0,
    ASSET_OWNER_GLOBAL    = 1u << 0,
    ASSET_OWNER_STAGE     = 1u << 1,
    ASSET_OWNER_EFFECT    = 1u << 2,
    ASSET_OWNER_UI        = 1u << 3,
    ASSET_OWNER_CHARACTER = 1u << 4,
    ASSET_OWNER_SKY       = 1u << 5
};

struct AssetLoadPolicy
{
    std::uint32_t maxSourceDimension;
    std::uint64_t maxSourcePixels;
    std::uint32_t maxRuntimeDimension;
    std::uint64_t maxRuntimePixels;
    std::uint32_t maxExternalRuntimeDimension;
    std::uint64_t maxExternalRuntimePixels;
    std::uint32_t maxTransientFailures;
    std::uint64_t residentBudgetBytes;
    std::uint64_t largeTextureRgbaBytes;
    std::uint64_t recentTouchProtection;
    std::uint32_t globalPrewarmLimit;
    std::uint32_t scopedPrewarmLimit;

    static AssetLoadPolicy Defaults()
    {
        AssetLoadPolicy p;
        p.maxSourceDimension = 8192u;
        p.maxSourcePixels = 8192ull * 8192ull;
        p.maxRuntimeDimension = 4096u;
        p.maxRuntimePixels = 4096ull * 4096ull;
        p.maxExternalRuntimeDimension = 2048u;
        p.maxExternalRuntimePixels = 2048ull * 2048ull;
        p.maxTransientFailures = 3u;
        p.residentBudgetBytes = 1024ull * 1024ull * 1024ull;
        p.largeTextureRgbaBytes = 16ull * 1024ull * 1024ull;
        p.recentTouchProtection = 16ull;
        p.globalPrewarmLimit = 32u;
        p.scopedPrewarmLimit = 32u;
        return p;
    }
};

struct AssetLoadRecord
{
    AssetLoadState state;
    std::uint32_t transientFailures;
    std::uint32_t loadAttempts;
    std::uint32_t successfulLoads;
    std::uint32_t evictionCount;
    std::uint32_t sourceWidth;
    std::uint32_t sourceHeight;
    std::uint32_t residentWidth;
    std::uint32_t residentHeight;
    std::uint32_t ownerMask;
    std::uint64_t residentBytes;
    std::uint64_t lastTouchSerial;
    bool pendingGlobalPrewarm;

    AssetLoadRecord()
        : state(ASSET_UNLOADED),
          transientFailures(0),
          loadAttempts(0),
          successfulLoads(0),
          evictionCount(0),
          sourceWidth(0),
          sourceHeight(0),
          residentWidth(0),
          residentHeight(0),
          ownerMask(ASSET_OWNER_UNKNOWN),
          residentBytes(0),
          lastTouchSerial(0),
          pendingGlobalPrewarm(false)
    {
    }
};

struct AssetLoadStats
{
    std::size_t registeredCount;
    std::size_t loadedCount;
    std::size_t unloadedCount;
    std::size_t transientFailureCount;
    std::size_t permanentFailureCount;
    std::size_t waitingDeviceCount;
    std::size_t evictedCount;
    std::uint64_t residentBytes;
    std::uint64_t serial;
};

struct AssetTargetDimensions
{
    std::uint32_t width;
    std::uint32_t height;
    bool downscaled;
};

inline bool IsStructurallyValidSource(
    std::uint32_t width,
    std::uint32_t height,
    const AssetLoadPolicy &policy)
{
    if (!width || !height)
        return false;
    if (width > policy.maxSourceDimension || height > policy.maxSourceDimension)
        return false;
    return static_cast<std::uint64_t>(width) * static_cast<std::uint64_t>(height) <= policy.maxSourcePixels;
}

inline std::uint32_t AlignDown4(std::uint32_t value)
{
    if (value < 4u)
        return value;
    const std::uint32_t aligned = value & ~3u;
    return aligned ? aligned : 4u;
}

inline AssetTargetDimensions ComputeTargetDimensions(
    std::uint32_t sourceWidth,
    std::uint32_t sourceHeight,
    const AssetLoadPolicy &policy,
    unsigned qualityShift,
    bool requireBlockAlignment)
{
    AssetTargetDimensions result = { sourceWidth, sourceHeight, false };
    if (!sourceWidth || !sourceHeight)
        return result;

    std::uint32_t maxDimension = policy.maxRuntimeDimension;
    std::uint64_t maxPixels = policy.maxRuntimePixels;
    if (qualityShift > 0)
    {
        maxDimension >>= qualityShift;
        maxPixels >>= (qualityShift * 2u);
        if (maxDimension < 256u)
            maxDimension = 256u;
        if (maxPixels < 256ull * 256ull)
            maxPixels = 256ull * 256ull;
    }

    double scale = 1.0;
    if (sourceWidth > maxDimension || sourceHeight > maxDimension)
    {
        const double sx = static_cast<double>(maxDimension) / static_cast<double>(sourceWidth);
        const double sy = static_cast<double>(maxDimension) / static_cast<double>(sourceHeight);
        scale = sx < sy ? sx : sy;
    }

    const std::uint64_t sourcePixels = static_cast<std::uint64_t>(sourceWidth) * sourceHeight;
    if (sourcePixels > maxPixels)
    {
        const double pixelScale = static_cast<double>(maxPixels) / static_cast<double>(sourcePixels);
        // Avoid libm dependency here: shrink iteratively until the pixel budget fits.
        while (scale * scale > pixelScale)
            scale *= 0.875;
    }

    if (scale < 1.0)
    {
        std::uint32_t width = static_cast<std::uint32_t>(sourceWidth * scale);
        std::uint32_t height = static_cast<std::uint32_t>(sourceHeight * scale);
        if (!width) width = 1;
        if (!height) height = 1;
        if (requireBlockAlignment)
        {
            width = AlignDown4(width);
            height = AlignDown4(height);
        }
        result.width = width;
        result.height = height;
        result.downscaled = width != sourceWidth || height != sourceHeight;
    }
    else if (requireBlockAlignment && sourceWidth >= 4u && sourceHeight >= 4u)
    {
        // Do not alter ordinary source dimensions only for alignment; DXT can be skipped instead.
    }

    return result;
}

template <std::size_t Capacity>
class AssetLoadRegistry
{
public:
    AssetLoadRegistry()
        : policy_(AssetLoadPolicy::Defaults()), residentBytes_(0), serial_(0), registeredCount_(0)
    {
        Reset();
    }

    void Reset()
    {
        for (std::size_t i = 0; i < Capacity; ++i)
            records_[i] = AssetLoadRecord();
        residentBytes_ = 0;
        serial_ = 0;
        registeredCount_ = 0;
    }

    void SetPolicy(const AssetLoadPolicy &policy) { policy_ = policy; }
    const AssetLoadPolicy &Policy() const { return policy_; }

    bool Valid(std::size_t index) const { return index < Capacity; }

    void Register(std::size_t index, std::uint32_t ownerMask = ASSET_OWNER_GLOBAL)
    {
        if (!Valid(index)) return;
        AssetLoadRecord &r = records_[index];
        if (r.ownerMask == ASSET_OWNER_UNKNOWN && r.loadAttempts == 0 && r.successfulLoads == 0 && r.residentBytes == 0)
            ++registeredCount_;
        r.ownerMask |= ownerMask;
        r.pendingGlobalPrewarm = true;
        if (r.state == ASSET_FAILED_TRANSIENT)
            r.state = ASSET_UNLOADED;
    }

    void AddOwner(std::size_t index, std::uint32_t ownerMask)
    {
        if (!Valid(index)) return;
        records_[index].ownerMask |= ownerMask;
    }

    void MarkPendingGlobal(std::size_t index)
    {
        if (Valid(index)) records_[index].pendingGlobalPrewarm = true;
    }

    bool IsPendingGlobal(std::size_t index) const
    {
        return Valid(index) && records_[index].pendingGlobalPrewarm;
    }

    void ConsumePendingGlobal(std::size_t index)
    {
        if (Valid(index)) records_[index].pendingGlobalPrewarm = false;
    }

    bool CanAttempt(std::size_t index) const
    {
        if (!Valid(index)) return false;
        const AssetLoadRecord &r = records_[index];
        return r.state != ASSET_LOADING &&
               r.state != ASSET_FAILED_PERMANENT &&
               r.state != ASSET_WAITING_DEVICE;
    }

    void BeginLoad(std::size_t index)
    {
        if (!Valid(index)) return;
        AssetLoadRecord &r = records_[index];
        r.state = ASSET_LOADING;
        ++r.loadAttempts;
        r.pendingGlobalPrewarm = false;
    }

    void MarkLoaded(
        std::size_t index,
        std::uint32_t sourceWidth,
        std::uint32_t sourceHeight,
        std::uint32_t residentWidth,
        std::uint32_t residentHeight,
        std::uint64_t residentBytes)
    {
        if (!Valid(index)) return;
        AssetLoadRecord &r = records_[index];
        RemoveResidentBytes(r.residentBytes);
        r.state = ASSET_LOADED;
        r.transientFailures = 0;
        ++r.successfulLoads;
        r.sourceWidth = sourceWidth;
        r.sourceHeight = sourceHeight;
        r.residentWidth = residentWidth;
        r.residentHeight = residentHeight;
        r.residentBytes = residentBytes;
        residentBytes_ += residentBytes;
        Touch(index);
    }

    void MarkPermanentFailure(std::size_t index, std::uint32_t width = 0, std::uint32_t height = 0)
    {
        if (!Valid(index)) return;
        AssetLoadRecord &r = records_[index];
        RemoveResidentBytes(r.residentBytes);
        r.residentBytes = 0;
        r.sourceWidth = width;
        r.sourceHeight = height;
        r.state = ASSET_FAILED_PERMANENT;
        r.pendingGlobalPrewarm = false;
    }

    void MarkWaitingForDevice(std::size_t index, std::uint32_t width = 0, std::uint32_t height = 0)
    {
        if (!Valid(index)) return;
        AssetLoadRecord &r = records_[index];
        RemoveResidentBytes(r.residentBytes);
        r.residentBytes = 0;
        r.residentWidth = 0;
        r.residentHeight = 0;
        r.sourceWidth = width;
        r.sourceHeight = height;
        r.state = ASSET_WAITING_DEVICE;
        r.pendingGlobalPrewarm = false;
    }

    bool MarkTransientFailure(std::size_t index, std::uint32_t width = 0, std::uint32_t height = 0)
    {
        if (!Valid(index)) return true;
        AssetLoadRecord &r = records_[index];
        RemoveResidentBytes(r.residentBytes);
        r.residentBytes = 0;
        r.sourceWidth = width;
        r.sourceHeight = height;
        ++r.transientFailures;
        if (r.transientFailures >= policy_.maxTransientFailures)
        {
            r.state = ASSET_FAILED_PERMANENT;
            r.pendingGlobalPrewarm = false;
            return true;
        }
        r.state = ASSET_FAILED_TRANSIENT;
        r.pendingGlobalPrewarm = true;
        return false;
    }

    void MarkEvicted(std::size_t index)
    {
        if (!Valid(index)) return;
        AssetLoadRecord &r = records_[index];
        RemoveResidentBytes(r.residentBytes);
        r.residentBytes = 0;
        r.residentWidth = 0;
        r.residentHeight = 0;
        r.state = ASSET_EVICTED;
        ++r.evictionCount;
    }

    void MarkReleased(std::size_t index)
    {
        if (!Valid(index)) return;
        AssetLoadRecord &r = records_[index];
        RemoveResidentBytes(r.residentBytes);
        r = AssetLoadRecord();
        if (registeredCount_ > 0)
            --registeredCount_;
    }

    void MarkDeviceRestoreNeeded(std::size_t index)
    {
        if (!Valid(index)) return;
        AssetLoadRecord &r = records_[index];
        RemoveResidentBytes(r.residentBytes);
        r.residentBytes = 0;
        if (r.state != ASSET_FAILED_PERMANENT)
        {
            r.state = ASSET_UNLOADED;
            r.pendingGlobalPrewarm = true;
        }
    }

    void Touch(std::size_t index)
    {
        if (!Valid(index)) return;
        ++serial_;
        records_[index].lastTouchSerial = serial_;
    }

    std::size_t SelectEvictionCandidate(std::size_t excludeIndex, bool relaxProtection = false) const
    {
        std::size_t best = Capacity;
        std::uint64_t bestSerial = ~static_cast<std::uint64_t>(0);
        for (std::size_t i = 0; i < Capacity; ++i)
        {
            if (i == excludeIndex) continue;
            const AssetLoadRecord &r = records_[i];
            if (r.state != ASSET_LOADED || !r.residentBytes) continue;
            if (!relaxProtection && serial_ >= r.lastTouchSerial &&
                serial_ - r.lastTouchSerial <= policy_.recentTouchProtection)
                continue;
            if (r.lastTouchSerial < bestSerial)
            {
                bestSerial = r.lastTouchSerial;
                best = i;
            }
        }
        return best;
    }

    const AssetLoadRecord &Record(std::size_t index) const { return records_[index]; }
    AssetLoadRecord &Record(std::size_t index) { return records_[index]; }
    std::uint64_t ResidentBytes() const { return residentBytes_; }
    std::uint64_t Serial() const { return serial_; }
    std::size_t CapacityValue() const { return Capacity; }

    AssetLoadStats Stats() const
    {
        AssetLoadStats s = {};
        s.registeredCount = registeredCount_;
        s.residentBytes = residentBytes_;
        s.serial = serial_;
        for (std::size_t i = 0; i < Capacity; ++i)
        {
            switch (records_[i].state)
            {
            case ASSET_LOADED: ++s.loadedCount; break;
            case ASSET_FAILED_TRANSIENT: ++s.transientFailureCount; break;
            case ASSET_FAILED_PERMANENT: ++s.permanentFailureCount; break;
            case ASSET_WAITING_DEVICE: ++s.waitingDeviceCount; break;
            case ASSET_EVICTED: ++s.evictedCount; break;
            case ASSET_UNLOADED: ++s.unloadedCount; break;
            default: break;
            }
        }
        return s;
    }

private:
    void RemoveResidentBytes(std::uint64_t bytes)
    {
        residentBytes_ = bytes <= residentBytes_ ? residentBytes_ - bytes : 0;
    }

    AssetLoadPolicy policy_;
    AssetLoadRecord records_[Capacity];
    std::uint64_t residentBytes_;
    std::uint64_t serial_;
    std::size_t registeredCount_;
};

} // namespace v2
} // namespace assets
} // namespace rendering
} // namespace summit
