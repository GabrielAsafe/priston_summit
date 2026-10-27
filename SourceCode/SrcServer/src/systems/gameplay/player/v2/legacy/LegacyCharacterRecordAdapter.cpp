#include "systems/gameplay/player/v2/legacy/LegacyCharacterRecordAdapter.h"
#include "gameplay/companions/v2/CompanionTradeBoundedItemCodec.h"
#include "systems/gameplay/inventory/v2/legacy/LegacyItemCatalogAdapter.h"

#include <algorithm>
#include <cstddef>
#include <cstring>

// Keep the historical server include order inside the AMBER boundary.
#include "systems/network/transport/legacy/smwsock.h"
#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "systems/persistence/character_record/record.h"
#include "core/gameplay/legacy_support/playsub.h"
#include "systems/inventory/legacy/sinInvenTory.h"

namespace gameplay
{
namespace player
{
namespace legacy
{
namespace
{
bool LoadLegacyRecord(const CharacterRecordKey& key, TRANS_RECORD_DATA& record)
{
    return key.Valid() &&
        rsLoadAccountCharacterRecord(key.accountKey.c_str(), key.characterName.c_str(), &record) != 0;
}

CharacterProgressionRecord TranslateProgression(const TRANS_RECORD_DATA& record)
{
    CharacterProgressionRecord snapshot;
    snapshot.statePoint = record.smCharInfo.StatePoint;
    snapshot.nextExperience = record.smCharInfo.Next_Exp;
    snapshot.changeJob = static_cast<std::uint32_t>(record.smCharInfo.ChangeJob);
    snapshot.jobBitMask = static_cast<std::uint32_t>(record.smCharInfo.JobBitMask);
    return snapshot;
}

void TranslateSkillPoints(const TRANS_RECORD_DATA& record, std::vector<std::uint8_t>& out)
{
    out.assign(20u, 0u);
    for (int useSlot = 1; useSlot <= 16; ++useSlot)
        out[static_cast<std::size_t>(useSlot - 1)] = static_cast<std::uint8_t>(
            std::min(10, static_cast<int>(record.GameSaveInfo.SkillInfo.bSkillPoint[useSlot & 15])));

    for (int tier5Index = 0; tier5Index < 4; ++tier5Index)
        out[static_cast<std::size_t>(16 + tier5Index)] = static_cast<std::uint8_t>(
            std::min(10, GetTier5RecordSkillPoint(&record.GameSaveInfo, tier5Index)));
}

void TranslatePrimaryItems(const TRANS_RECORD_DATA& record, std::vector<CharacterRecordItem>& out)
{
    out.clear();
    if (record.DataSize <= 0)
        return;

    const BYTE* cursor = record.Data;
    const BYTE* end = record.Data + record.DataSize;
    int primaryItemCount = record.ItemSubStart;
    if (primaryItemCount <= 0 || primaryItemCount > record.ItemCount)
        primaryItemCount = record.ItemCount;

    out.reserve(static_cast<std::size_t>(std::max(0, primaryItemCount)));
    for (int item = 0; item < primaryItemCount && cursor + sizeof(int) <= end; ++item)
    {
        const int packedSize = *reinterpret_cast<const int*>(cursor);
        if (packedSize <= static_cast<int>(sizeof(int)) || cursor + packedSize > end)
            break;

        sRECORD_ITEM legacyItem;
        ZeroMemory(&legacyItem, sizeof(legacyItem));
        DecodeCompress(const_cast<BYTE*>(cursor), reinterpret_cast<BYTE*>(&legacyItem), sizeof(legacyItem));
        cursor += packedSize;

        if (!legacyItem.sItemInfo.CODE)
            continue;

        CharacterRecordItem translated;
        translated.itemCode = static_cast<std::uint32_t>(legacyItem.sItemInfo.CODE);
        translated.position = legacyItem.ItemPosition;
        translated.itemName = legacyItem.sItemInfo.ItemName;
        out.push_back(translated);
    }
}
}

gameplay::companions::v2::CompanionTradeInventoryReadCode
LegacyCharacterRecordAdapter::TryLoadTradeInventorySnapshot(
    const CharacterRecordKey& key,
    gameplay::companions::v2::CompanionTradeInventorySnapshot& out) const
{
    namespace trade = gameplay::companions::v2;
    TRANS_RECORD_DATA record;
    // Existing rsLoadAccountCharacterRecord verifies account membership and
    // character name before reading. Never derive a path from network input.
    if (!LoadLegacyRecord(key, record))
        return trade::CompanionTradeInventoryReadCode::InvalidEnvelope;
    const std::size_t header = offsetof(TRANS_RECORD_DATA, Data);
    if (record.size < static_cast<int>(header) ||
        record.DataSize < 0 ||
        static_cast<std::size_t>(record.DataSize) > sizeof(record.Data) ||
        static_cast<std::size_t>(record.size) < header +
            static_cast<std::size_t>(record.DataSize))
        return trade::CompanionTradeInventoryReadCode::InvalidEnvelope;

    return trade::ReadCompanionTradeInventory(
        record.Data, static_cast<std::size_t>(record.DataSize),
        record.ItemCount, record.ItemSubStart, RECORD_ITEM_MAX,
        [](const std::uint8_t* encoded, std::size_t length,
           trade::CompanionTradeDecodedItem& outItem) {
            if (length <= sizeof(std::int32_t))
                return false;
            sRECORD_ITEM decoded;
            ZeroMemory(&decoded, sizeof(decoded));
            // Trade snapshots require a strictly bounded, fully decoded item.
            // Padding around an unchecked legacy decoder is not sufficient:
            // malformed literal runs or incomplete items must fail closed.
            if (trade::CompanionTradeBoundedItemCodec::Decode(encoded, length,
                    reinterpret_cast<std::uint8_t*>(&decoded), sizeof(decoded)) !=
                trade::TradeItemDecodeStatus::Ok)
                return false;
            outItem.code = static_cast<std::uint32_t>(decoded.sItemInfo.CODE);
            outItem.key = static_cast<std::uint32_t>(decoded.sItemInfo.ItemHeader.Head);
            outItem.checksum = static_cast<std::uint32_t>(decoded.sItemInfo.ItemHeader.dwChkSum);
            outItem.position = decoded.ItemPosition;
            outItem.x = decoded.x;
            outItem.y = decoded.y;
            return true;
        }, out);
}

gameplay::companions::v2::BufferedTradeCaptureCode
LegacyCharacterRecordAdapter::TryDecodeTradeBufferedPayloadCandidate(
    const std::uint8_t* stableCopy, std::size_t copyLength,
    const char* serverCharacterName,
    gameplay::companions::v2::CompanionTradeInventorySnapshot& out) const
{
    namespace trade = gameplay::companions::v2;
    // srRECORD_MEMORY_SIZE is the historical fixed per-session save buffer.
    // If the record is larger, this interface refuses it; it NEVER reads
    // outside the caller-supplied stable copy and does not fetch from a DAT.
    const trade::BufferedTradeEnvelopeLayout layout(
        offsetof(TRANS_RECORD_DATA, size),
        offsetof(TRANS_RECORD_DATA, ItemCount),
        offsetof(TRANS_RECORD_DATA, ItemSubStart),
        offsetof(TRANS_RECORD_DATA, DataSize),
        offsetof(TRANS_RECORD_DATA, smCharInfo) + offsetof(smCHAR_INFO, szName),
        sizeof(((smCHAR_INFO*)0)->szName),
        offsetof(TRANS_RECORD_DATA, Data), srRECORD_MEMORY_SIZE);
    return trade::BufferedTradePayloadPolicy::DecodeBounded(
        stableCopy, copyLength, layout, serverCharacterName, RECORD_ITEM_MAX,
        [](const std::uint8_t* encoded, std::size_t length,
           trade::CompanionTradeDecodedItem& outItem) {
            if (length <= sizeof(std::int32_t))
                return false;
            sRECORD_ITEM decoded;
            ZeroMemory(&decoded, sizeof(decoded));
            // Trade snapshots require a strictly bounded, fully decoded item.
            // Padding around an unchecked legacy decoder is not sufficient:
            // malformed literal runs or incomplete items must fail closed.
            if (trade::CompanionTradeBoundedItemCodec::Decode(encoded, length,
                    reinterpret_cast<std::uint8_t*>(&decoded), sizeof(decoded)) !=
                trade::TradeItemDecodeStatus::Ok)
                return false;
            outItem.code = static_cast<std::uint32_t>(decoded.sItemInfo.CODE);
            outItem.key = static_cast<std::uint32_t>(decoded.sItemInfo.ItemHeader.Head);
            outItem.checksum = static_cast<std::uint32_t>(decoded.sItemInfo.ItemHeader.dwChkSum);
            outItem.position = decoded.ItemPosition;
            outItem.x = decoded.x;
            outItem.y = decoded.y;
            return true;
        }, out);
}

bool LegacyCharacterRecordAdapter::BuildTradePairedRecordImages(
    const std::vector<std::uint8_t>& ownerSavedBytes,
    const std::vector<std::uint8_t>& companionSavedBytes,
    const char* ownerCharacter, const char* companionCharacter,
    const gameplay::companions::v2::CompanionTradeOffer& offer,
    const std::vector<gameplay::companions::v2::TradeSelectedItem>& selected,
    const std::function<bool(const gameplay::companions::v2::CompanionTradeItemRecord&,
                             gameplay::companions::v2::TradeSide)>& serverItemPermission,
    gameplay::companions::v2::CompanionTradePrepared& prepared,
    TradePairBuildDiagnostic350M* diagnostics) const
{
    namespace trade = gameplay::companions::v2;
    if(diagnostics) *diagnostics=TradePairBuildDiagnostic350M{};
    if(!ownerCharacter || !*ownerCharacter || !companionCharacter ||
       !*companionCharacter || !serverItemPermission ||
       ownerSavedBytes.empty() || companionSavedBytes.empty() ||
       ownerSavedBytes.size() > sizeof(TRANS_RECORD_DATA) ||
       companionSavedBytes.size() > sizeof(TRANS_RECORD_DATA)) {
        if(diagnostics) diagnostics->stage=TradePairBuildStage350M::BadArguments;
        return false;
    }
    const trade::BufferedTradeEnvelopeLayout envelope(
        offsetof(TRANS_RECORD_DATA,size),
        offsetof(TRANS_RECORD_DATA,ItemCount),
        offsetof(TRANS_RECORD_DATA,ItemSubStart),
        offsetof(TRANS_RECORD_DATA,DataSize),
        offsetof(TRANS_RECORD_DATA,smCharInfo)+offsetof(smCHAR_INFO,szName),
        sizeof(((smCHAR_INFO*)0)->szName),
        offsetof(TRANS_RECORD_DATA,Data),sizeof(TRANS_RECORD_DATA));
    const auto decodeItem=[](const std::uint8_t* encoded,std::size_t length,
                             trade::CompanionTradeDecodedItem& output) {
        sRECORD_ITEM item{};
        if(trade::CompanionTradeBoundedItemCodec::Decode(encoded,length,
            reinterpret_cast<std::uint8_t*>(&item),sizeof(item)) !=
                trade::TradeItemDecodeStatus::Ok) return false;
        output.code=static_cast<std::uint32_t>(item.sItemInfo.CODE);
        output.key=static_cast<std::uint32_t>(item.sItemInfo.ItemHeader.Head);
        output.checksum=static_cast<std::uint32_t>(item.sItemInfo.ItemHeader.dwChkSum);
        output.position=item.ItemPosition;
        output.x=item.x;
        output.y=item.y;
        return true;
    };
    trade::CompanionTradeInventorySnapshot online,offline;
    trade::CompanionTradeInventorySnapshot unsafeOwner350N;
    const auto ownerDecode=trade::BufferedTradePayloadPolicy::DecodeBounded(
        ownerSavedBytes.data(),ownerSavedBytes.size(),envelope,ownerCharacter,
        RECORD_ITEM_MAX,decodeItem,online,&unsafeOwner350N);
    if(ownerDecode!=trade::BufferedTradeCaptureCode::CandidateReadOnly) {
        if(diagnostics) {
            diagnostics->stage=TradePairBuildStage350M::OwnerDecode;
            diagnostics->decode=static_cast<int>(ownerDecode);
            diagnostics->unsafeFlags350N=
                (unsafeOwner350N.hasMissingInstanceIdentity?1u:0u) |
                (unsafeOwner350N.hasDuplicateInstanceIdentity?2u:0u) |
                (unsafeOwner350N.hasCursorItem?4u:0u) |
                (unsafeOwner350N.hasUnknownPosition?8u:0u);
        }
        return false;
    }
    trade::CompanionTradeInventorySnapshot unsafeCompanion350N;
    const auto companionDecode=trade::BufferedTradePayloadPolicy::DecodeBounded(
        companionSavedBytes.data(),companionSavedBytes.size(),envelope,
        companionCharacter,RECORD_ITEM_MAX,decodeItem,offline,&unsafeCompanion350N);
    if(companionDecode!=trade::BufferedTradeCaptureCode::CandidateReadOnly) {
        if(diagnostics) {
            diagnostics->stage=TradePairBuildStage350M::CompanionDecode;
            diagnostics->decode=static_cast<int>(companionDecode);
            diagnostics->unsafeFlags350N=
                (unsafeCompanion350N.hasMissingInstanceIdentity?1u:0u) |
                (unsafeCompanion350N.hasDuplicateInstanceIdentity?2u:0u) |
                (unsafeCompanion350N.hasCursorItem?4u:0u) |
                (unsafeCompanion350N.hasUnknownPosition?8u:0u);
        }
        return false;
    }
    // The trade page's ONLINE index is an identity-ledger slot, not a
    // TRANS_RECORD_DATA array offset. Convert it by full item identity,
    // under the caller's authenticated ledger lease. Companion indices are
    // already raw record indices and remain unchanged.
    std::vector<trade::TradeSelectedItem> resolved=selected;
    for(auto& selection:resolved) {
        if(selection.from!=trade::TradeSide::Player) continue;
        bool found=false;
        for(const auto& record:online.records) {
            if(record.itemCode!=selection.code || record.itemKey!=selection.key ||
               record.itemChecksum!=selection.checksum) continue;
            if(found) {
                if(diagnostics) diagnostics->stage=TradePairBuildStage350M::SelectedAmbiguous;
                return false;
            }
            found=true;
            selection.recordIndex=record.recordIndex;
        }
        if(!found) {
            if(diagnostics) diagnostics->stage=TradePairBuildStage350M::SelectedMissing;
            return false;
        }
    }
    const trade::TradeRecordLayout layout{
        offsetof(TRANS_RECORD_DATA,size),offsetof(TRANS_RECORD_DATA,ItemCount),
        offsetof(TRANS_RECORD_DATA,ItemSubStart),
        offsetof(TRANS_RECORD_DATA,DataSize),offsetof(TRANS_RECORD_DATA,Data),
        sizeof(((TRANS_RECORD_DATA*)0)->Data),srRECORD_DEFAULT_SIZE};
    const auto readSlot=[](const trade::CompanionTradeItemRecord& source,
                           std::uint32_t& slot) {
        sRECORD_ITEM item{};
        if(trade::CompanionTradeBoundedItemCodec::Decode(source.encodedRecord.data(),
           source.encodedRecord.size(),reinterpret_cast<std::uint8_t*>(&item),
           sizeof(item)) != trade::TradeItemDecodeStatus::Ok ||
           item.ItemCount<0 || item.ItemCount>=INVENTORY_MAXITEM)
            return false;
        slot=static_cast<std::uint32_t>(item.ItemCount);
        return true;
    };
    const auto relocated=[&](const trade::CompanionTradeItemRecord& from,
                        bool secondary,int x,int y,std::uint32_t slot,
                        trade::CompanionTradeItemRecord& result) {
        sRECORD_ITEM item{};
        if(trade::CompanionTradeBoundedItemCodec::Decode(
            from.encodedRecord.data(),from.encodedRecord.size(),
            reinterpret_cast<std::uint8_t*>(&item),sizeof(item)) !=
               trade::TradeItemDecodeStatus::Ok || item.ItemPosition!=0 ||
           static_cast<std::uint32_t>(item.sItemInfo.CODE)!=from.itemCode ||
           static_cast<std::uint32_t>(item.sItemInfo.ItemHeader.Head)!=from.itemKey ||
           static_cast<std::uint32_t>(item.sItemInfo.ItemHeader.dwChkSum)!=from.itemChecksum)
            return false;
        item.x=x; item.y=y;
        item.ItemCount=static_cast<int>(slot);
        std::vector<std::uint8_t> bytes(sizeof(item)*2u+64u,0);
        const int packed=EecodeCompress(reinterpret_cast<BYTE*>(&item),
            bytes.data(),sizeof(item),static_cast<int>(bytes.size()));
        if(packed<=4 || static_cast<std::size_t>(packed)>bytes.size())return false;
        bytes.resize(static_cast<std::size_t>(packed));
        trade::CompanionTradeDecodedItem roundtrip;
        sRECORD_ITEM decodedAgain{};
        if(trade::CompanionTradeBoundedItemCodec::Decode(bytes.data(),bytes.size(),
             reinterpret_cast<std::uint8_t*>(&decodedAgain),sizeof(decodedAgain))!=
               trade::TradeItemDecodeStatus::Ok || decodedAgain.ItemCount!=static_cast<int>(slot) ||
           !decodeItem(bytes.data(),bytes.size(),roundtrip) ||
           roundtrip.code!=from.itemCode || roundtrip.key!=from.itemKey ||
           roundtrip.checksum!=from.itemChecksum ||
           roundtrip.position!=0 || roundtrip.x!=x || roundtrip.y!=y)
            return false;
        result=from;
        result.x=x;result.y=y;result.inSecondaryBag=secondary;
        result.encodedRecord=std::move(bytes);
        return true;
    };
    trade::CompanionTradePrepared candidate;
    const auto status=trade::BuildConcreteTradeTransfer(offer,
        ownerSavedBytes,companionSavedBytes,online,offline,
        layout,layout,resolved,
        gameplay::inventory::legacy::ServerLegacyItemCatalogAdapter(),
        trade::TradeLimits(RECORD_ITEM_MAX,sizeof(((TRANS_RECORD_DATA*)0)->Data),
                           16u),INVENTORY_MAXITEM,serverItemPermission,readSlot,relocated,candidate);
    if(status.code!=trade::ConcreteTransferCode::Ready) {
        if(diagnostics) {
            diagnostics->stage=TradePairBuildStage350M::ConcreteTransfer;
            diagnostics->transfer=static_cast<int>(status.code);
            diagnostics->selection350N=static_cast<int>(status.selectionReject);
            diagnostics->plan=static_cast<int>(status.planning);
            diagnostics->ownerImage=static_cast<int>(status.ownerImage);
            diagnostics->companionImage=static_cast<int>(status.companionImage);
        }
        return false;
    }
    if(diagnostics) diagnostics->stage=TradePairBuildStage350M::Ready;
    prepared=std::move(candidate);
    return true;
}

bool LegacyCharacterRecordAdapter::TryLoadProgression(
    const CharacterRecordKey& key,
    CharacterProgressionRecord& out) const
{
    TRANS_RECORD_DATA record;
    if (!LoadLegacyRecord(key, record))
        return false;
    out = TranslateProgression(record);
    return true;
}

bool LegacyCharacterRecordAdapter::TryLoadSnapshot(
    const CharacterRecordKey& key,
    CharacterPersistentSnapshot& out) const
{
    TRANS_RECORD_DATA record;
    if (!LoadLegacyRecord(key, record))
        return false;

    CharacterPersistentSnapshot snapshot;
    snapshot.characterName = record.smCharInfo.szName;
    snapshot.level = record.smCharInfo.Level;
    snapshot.legacyJobCode = static_cast<int>(record.smCharInfo.JOB_CODE);
    snapshot.life = record.smCharInfo.Life[0];
    snapshot.maxLife = record.smCharInfo.Life[1];
    snapshot.mana = record.smCharInfo.Mana[0];
    snapshot.maxMana = record.smCharInfo.Mana[1];
    snapshot.progression = TranslateProgression(record);
    TranslateSkillPoints(record, snapshot.logicalSkillPoints);
    TranslatePrimaryItems(record, snapshot.items);
    out = snapshot;
    return true;
}

const LegacyCharacterRecordAdapter& ServerLegacyCharacterRecordAdapter()
{
    static LegacyCharacterRecordAdapter adapter;
    return adapter;
}
}
}
}
