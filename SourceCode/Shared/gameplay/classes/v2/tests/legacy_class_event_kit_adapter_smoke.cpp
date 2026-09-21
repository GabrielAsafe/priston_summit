#include <cassert>
#include <cstddef>

#include "gameplay/events/legacy/LegacyClassEquipmentKitAdapter.h"

static std::uint64_t MixKitHash(std::uint64_t hash, std::uint64_t value)
{
    for(int byte = 0; byte < 8; ++byte)
    {
        hash ^= (value >> (byte * 8)) & 0xFFu;
        hash *= 1099511628211ull;
    }
    return hash;
}

static std::uint64_t KitHash(int boxType, int jobCode,
                             const gameplay::events::legacy::LegacyEquipmentKit& kit)
{
    std::uint64_t hash = 1469598103934665603ull;
    hash = MixKitHash(hash, static_cast<unsigned>(boxType));
    hash = MixKitHash(hash, static_cast<unsigned>(jobCode));
    hash = MixKitHash(hash, kit.count);
    for(std::size_t i = 0; i < kit.count; ++i)
    {
        hash = MixKitHash(hash, kit.grants[i].itemCode);
        hash = MixKitHash(hash, static_cast<unsigned>(kit.grants[i].specializationJobCode));
        hash = MixKitHash(hash, static_cast<unsigned>(kit.grants[i].kind));
        hash = MixKitHash(hash, static_cast<unsigned>(kit.grants[i].agingLevel));
    }
    return hash;
}

int main()
{
    using namespace gameplay::events::legacy;

    LegacyEquipmentKit kit;
    assert(!TryBuildRandomBoxClassKit(11, 9, kit));
    assert(kit.count == 0u);
    assert(!TryBuildRandomBoxClassKit(99, 1, kit));

    assert(TryBuildRandomBoxClassKit(11, 1, kit));
    assert(kit.count == 13u);
    assert(kit.grants[0].itemCode == 0x02012A00u);
    assert(kit.grants[0].agingLevel == 20);
    assert(kit.grants[11].itemCode == 0x01014100u);
    assert(kit.grants[12].itemCode == 0x01011900u);

    assert(TryBuildRandomBoxClassKit(11, 2, kit));
    assert(kit.count == 13u);
    assert(kit.grants[11].itemCode == 0x01024100u);
    assert(kit.grants[12].itemCode == 0x01050F00u);
    assert(kit.grants[12].kind == LegacyEquipmentGrantKind::SpecializedItem);

    assert(TryBuildRandomBoxClassKit(11, 7, kit));
    assert(kit.count == 13u);
    assert(kit.grants[0].itemCode == 0x02052A00u);
    assert(kit.grants[11].itemCode == 0x01042A00u);
    assert(kit.grants[12].itemCode == 0x01044200u);

    assert(TryBuildRandomBoxClassKit(14, 1, kit));
    assert(kit.count == 11u);
    assert(kit.grants[0].itemCode == 0x02011000u);
    assert(kit.grants[4].itemCode == 0x02040E00u);
    assert(kit.grants[5].itemCode == 0x01010E00u);
    for(std::size_t i = 6u; i < 11u; ++i)
    {
        assert(kit.grants[i].itemCode == 0x080B0800u);
        assert(kit.grants[i].specializationJobCode == 0);
    }

    assert(TryBuildRandomBoxClassKit(14, 2, kit));
    assert(kit.grants[5].itemCode == 0x01030F00u);

    assert(TryBuildRandomBoxClassKit(14, 8, kit));
    assert(kit.grants[0].itemCode == 0x02051000u);
    assert(kit.grants[4].itemCode == 0x02040800u);
    assert(kit.grants[5].itemCode == 0x01040F00u);


    // These signatures were captured after byte-for-byte comparison with the
    // original CaixaItensRandom class switches (189 grants total). They protect
    // order, item code, specialization job, grant kind and aging level.
    static const std::uint64_t highTierHashes[8] = {
        18006092653421592650ull, 191763732911541746ull,
        7288440974603157315ull, 5769247074104534357ull,
        15029021257723649883ull, 11833449581536007444ull,
        14747386646445177710ull, 5005708726740440446ull
    };
    static const std::uint64_t divineHashes[8] = {
        7854749084202595754ull, 13820787497650289712ull,
        12131602350970916981ull, 5241919470974901540ull,
        18127585945205561240ull, 7067007194755372790ull,
        13317501642103829596ull, 14979690979890442547ull
    };
    for(int legacyIndex = 0; legacyIndex < 8; ++legacyIndex)
    {
        const int jobCode = legacyIndex + 1;
        assert(TryBuildRandomBoxClassKit(11, jobCode, kit));
        assert(KitHash(11, jobCode, kit) == highTierHashes[legacyIndex]);
        assert(TryBuildRandomBoxClassKit(14, jobCode, kit));
        assert(KitHash(14, jobCode, kit) == divineHashes[legacyIndex]);
    }

    return 0;
}
