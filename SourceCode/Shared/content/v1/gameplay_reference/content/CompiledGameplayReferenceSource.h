#pragma once

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "content/v1/common/Sha256.h"

namespace summit { namespace content { namespace gameplay_reference {

enum QuestGuideListKind
{
    QUEST_GUIDE_PREREQUISITE = 1,
    QUEST_GUIDE_CLASS = 2,
    QUEST_GUIDE_FACTION = 3,
    QUEST_GUIDE_STAGE = 4,
    QUEST_GUIDE_OBJECTIVE = 5,
    QUEST_GUIDE_NPC = 6,
    QUEST_GUIDE_LOCATION = 7,
    QUEST_GUIDE_QUEST_ITEM = 8,
    QUEST_GUIDE_REWARD = 9,
    QUEST_GUIDE_NOTE = 10,
    QUEST_GUIDE_RUNTIME_ID = 11,
    QUEST_GUIDE_RUNTIME_TAG = 12,
    MIX_GUIDE_INGREDIENT = 100,
    MIX_GUIDE_BONUS = 101,
    MIX_GUIDE_RULE = 102,
    MIX_GUIDE_SHELTOM = 103,
};

struct QuestGuideEntry
{
    std::string guideId;
    std::string name;
    std::string type;
    std::string category;
    std::string source;
    std::string lore;
    int levelMin = -1;
    int levelMax = -1;
    bool repeatable = false;
    bool supported = false;
    std::vector<std::string> prerequisites;
    std::vector<std::string> classes;
    std::vector<std::string> factions;
    std::vector<std::string> stages;
    std::vector<std::string> objectives;
    std::vector<std::string> npcs;
    std::vector<std::string> locations;
    std::vector<std::string> questItems;
    std::vector<std::string> rewards;
    std::vector<std::string> notes;
    std::vector<int> runtimeIds;
    std::vector<std::string> runtimeTags;
};

struct MixGuideRecipe
{
    std::string recipeId;
    std::string itemType;
    std::string itemTypeLabel;
    std::string sourceIngredients;
    std::string sourceBonuses;
    int totalSheltoms = 0;
    int minimumCharacterLevel = -1;
    std::vector<std::string> ingredients;
    std::vector<std::string> bonuses;
};

struct MixGuideCatalog
{
    std::string title;
    std::string sourceUrl;
    std::string retrievedDate;
    int totalCombinations = 0;
    std::vector<std::string> rules;
    std::vector<std::string> sheltoms;
};

class CompiledGameplayReferenceSource
{
public:
    explicit CompiledGameplayReferenceSource(const char* packPath)
        : valid_(false)
    {
        if (packPath)
            path_ = packPath;
        LoadPack();
    }

    bool IsValid() const { return valid_; }
    const std::string& LastError() const { return error_; }

    bool Quests(std::vector<QuestGuideEntry>& out) const
    {
        if (!valid_)
            return false;
        out.clear();
        out.reserve(header_.questCount);
        for (std::uint32_t i = 0; i < header_.questCount; ++i)
        {
            QuestGuideEntry entry;
            if (!ReadQuest(i, entry))
                return false;
            out.push_back(entry);
        }
        return true;
    }

    bool SupportedQuests(std::vector<QuestGuideEntry>& out) const
    {
        std::vector<QuestGuideEntry> all;
        if (!Quests(all))
            return false;
        out.clear();
        for (std::size_t i = 0; i < all.size(); ++i)
            if (all[i].supported)
                out.push_back(all[i]);
        return true;
    }

    bool FindByRuntimeId(int runtimeId, QuestGuideEntry& out) const
    {
        std::vector<QuestGuideEntry> all;
        if (!Quests(all))
            return false;
        for (std::size_t i = 0; i < all.size(); ++i)
            for (std::size_t n = 0; n < all[i].runtimeIds.size(); ++n)
                if (all[i].runtimeIds[n] == runtimeId)
                {
                    out = all[i];
                    return true;
                }
        return false;
    }

    bool FindByName(const char* value, QuestGuideEntry& out) const
    {
        const std::string wanted = NormalizeName(value ? value : "");
        if (wanted.empty())
            return false;
        std::vector<QuestGuideEntry> all;
        if (!Quests(all))
            return false;
        for (std::size_t i = 0; i < all.size(); ++i)
            if (NormalizeName(all[i].name.c_str()) == wanted || NormalizeName(all[i].guideId.c_str()) == wanted)
            {
                out = all[i];
                return true;
            }
        return false;
    }

    bool MixCatalog(MixGuideCatalog& out) const
    {
        if (!valid_) return false;
        for (std::uint32_t i = 0; i < header_.mixCount; ++i)
        {
            MixRecord record;
            if (!ReadMixRecord(i, record)) return false;
            std::string id;
            if (!String(record.recipeId, id)) return false;
            if (id != "__catalog__") continue;
            if (!String(record.itemTypeLabel, out.title) ||
                !String(record.sourceIngredients, out.sourceUrl) ||
                !String(record.sourceBonuses, out.retrievedDate))
                return false;
            out.totalCombinations = static_cast<int>(record.totalSheltoms);
            for (std::uint32_t n = 0; n < record.listCount; ++n)
            {
                ListRecord item; std::string value;
                if (!ReadListRecord(record.listStart + n, item) || !String(item.value, value)) return false;
                if (item.kind == MIX_GUIDE_RULE) AddString(out.rules, value);
                else if (item.kind == MIX_GUIDE_SHELTOM) AddString(out.sheltoms, value);
            }
            return true;
        }
        return false;
    }

    bool MixRecipes(std::vector<MixGuideRecipe>& out) const
    {
        if (!valid_) return false;
        out.clear();
        for (std::uint32_t i = 0; i < header_.mixCount; ++i)
        {
            MixRecord record;
            if (!ReadMixRecord(i, record)) return false;
            MixGuideRecipe recipe;
            if (!String(record.recipeId, recipe.recipeId) || !String(record.itemType, recipe.itemType) ||
                !String(record.itemTypeLabel, recipe.itemTypeLabel) ||
                !String(record.sourceIngredients, recipe.sourceIngredients) ||
                !String(record.sourceBonuses, recipe.sourceBonuses)) return false;
            if (recipe.recipeId == "__catalog__") continue;
            recipe.totalSheltoms = static_cast<int>(record.totalSheltoms);
            recipe.minimumCharacterLevel = record.minimumLevel;
            for (std::uint32_t n = 0; n < record.listCount; ++n)
            {
                ListRecord item; std::string value;
                if (!ReadListRecord(record.listStart + n, item) || !String(item.value, value)) return false;
                if (item.kind == MIX_GUIDE_INGREDIENT) AddString(recipe.ingredients, value);
                else if (item.kind == MIX_GUIDE_BONUS) AddString(recipe.bonuses, value);
            }
            out.push_back(recipe);
        }
        return true;
    }

private:
    struct Header
    {
        std::uint16_t version = 0;
        std::uint16_t flags = 0;
        std::uint32_t questCount = 0;
        std::uint32_t questOffset = 0;
        std::uint32_t questSize = 0;
        std::uint32_t entryCount = 0;
        std::uint32_t entryOffset = 0;
        std::uint32_t entrySize = 0;
        std::uint32_t mixCount = 0;
        std::uint32_t mixOffset = 0;
        std::uint32_t mixSize = 0;
        std::uint32_t stringOffset = 0;
        unsigned char digest[32]{};
    } header_;

    struct QuestRecord
    {
        std::uint32_t guideId = 0;
        std::uint32_t name = 0;
        std::uint32_t type = 0;
        std::uint32_t category = 0;
        std::uint32_t source = 0;
        std::uint32_t lore = 0;
        std::int32_t levelMin = -1;
        std::int32_t levelMax = -1;
        std::uint32_t flags = 0;
        std::uint32_t listStart = 0;
        std::uint32_t listCount = 0;
    };

    struct ListRecord
    {
        std::uint32_t kind = 0;
        std::uint32_t value = 0;
    };

    struct MixRecord
    {
        std::uint32_t recipeId = 0;
        std::uint32_t itemType = 0;
        std::uint32_t itemTypeLabel = 0;
        std::uint32_t sourceIngredients = 0;
        std::uint32_t sourceBonuses = 0;
        std::uint32_t totalSheltoms = 0;
        std::int32_t minimumLevel = -1;
        std::uint32_t listStart = 0;
        std::uint32_t listCount = 0;
    };

    static std::uint16_t U16(const unsigned char* p)
    {
        return static_cast<std::uint16_t>(p[0] | (static_cast<std::uint16_t>(p[1]) << 8));
    }

    static std::uint32_t U32(const unsigned char* p)
    {
        return static_cast<std::uint32_t>(p[0]) |
               (static_cast<std::uint32_t>(p[1]) << 8) |
               (static_cast<std::uint32_t>(p[2]) << 16) |
               (static_cast<std::uint32_t>(p[3]) << 24);
    }

    static std::int32_t I32(const unsigned char* p)
    {
        return static_cast<std::int32_t>(U32(p));
    }

    static std::string NormalizeName(const char* value)
    {
        std::string out;
        const unsigned char* p = reinterpret_cast<const unsigned char*>(value ? value : "");
        while (*p)
        {
            unsigned char ch = *p++;
            if (ch < 0x80)
            {
                if (ch >= 'A' && ch <= 'Z')
                    ch = static_cast<unsigned char>(ch - 'A' + 'a');
                if ((ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9'))
                    out.push_back(static_cast<char>(ch));
                continue;
            }
            // Preserve UTF-8 bytes. The guide and runtime title normally use the
            // same UTF-8 spelling, while ASCII punctuation/spacing is ignored.
            out.push_back(static_cast<char>(ch));
        }
        return out;
    }

    bool Range(std::uint64_t offset, std::uint64_t size) const
    {
        return offset <= data_.size() && size <= data_.size() - static_cast<std::size_t>(offset);
    }

    bool LoadPack()
    {
        if (path_.empty())
        {
            error_ = "gameplay reference pack path is empty";
            return false;
        }
        FILE* file = std::fopen(path_.c_str(), "rb");
        if (!file)
        {
            error_ = "gameplay reference pack not found";
            return false;
        }
        std::fseek(file, 0, SEEK_END);
        const long size = std::ftell(file);
        if (size < 80 || std::fseek(file, 0, SEEK_SET) != 0)
        {
            std::fclose(file);
            error_ = "gameplay reference pack too small";
            return false;
        }
        data_.resize(static_cast<std::size_t>(size));
        if (std::fread(&data_[0], 1, data_.size(), file) != data_.size())
        {
            std::fclose(file);
            error_ = "cannot read gameplay reference pack";
            return false;
        }
        std::fclose(file);

        const unsigned char* p = &data_[0];
        if (std::memcmp(p, "SGRF", 4) != 0)
        {
            error_ = "gameplay reference magic mismatch";
            return false;
        }
        header_.version = U16(p + 4);
        header_.flags = U16(p + 6);
        header_.questCount = U32(p + 8);
        header_.questOffset = U32(p + 12);
        header_.questSize = U32(p + 16);
        header_.entryCount = U32(p + 20);
        header_.entryOffset = U32(p + 24);
        header_.entrySize = U32(p + 28);
        header_.mixCount = U32(p + 32);
        header_.mixOffset = U32(p + 36);
        header_.mixSize = U32(p + 40);
        header_.stringOffset = U32(p + 44);
        std::memcpy(header_.digest, p + 48, 32);

        const std::uint64_t expectedEntryOffset = 80ull + static_cast<std::uint64_t>(header_.questCount) * 44ull;
        const std::uint64_t expectedMixOffset = expectedEntryOffset + static_cast<std::uint64_t>(header_.entryCount) * 8ull;
        const std::uint64_t expectedStringOffset = expectedMixOffset + static_cast<std::uint64_t>(header_.mixCount) * 36ull;
        if (header_.version != 2 || header_.flags || header_.questOffset != 80 || header_.questSize != 44 ||
            header_.entryOffset != expectedEntryOffset || header_.entrySize != 8 ||
            header_.mixOffset != expectedMixOffset || header_.mixSize != 36 ||
            header_.stringOffset != expectedStringOffset || header_.stringOffset > data_.size())
        {
            error_ = "gameplay reference layout is non-canonical";
            return false;
        }
        unsigned char digest[32];
        summit::content::common::Sha256(&data_[80], data_.size() - 80, digest);
        if (std::memcmp(digest, header_.digest, 32) != 0)
        {
            error_ = "gameplay reference SHA-256 mismatch";
            return false;
        }
        valid_ = true;
        return true;
    }

    bool ReadQuestRecord(std::uint32_t index, QuestRecord& record) const
    {
        if (index >= header_.questCount)
            return false;
        const std::uint64_t offset = header_.questOffset + static_cast<std::uint64_t>(index) * 44ull;
        if (!Range(offset, 44))
            return false;
        const unsigned char* p = &data_[static_cast<std::size_t>(offset)];
        record.guideId = U32(p + 0);
        record.name = U32(p + 4);
        record.type = U32(p + 8);
        record.category = U32(p + 12);
        record.source = U32(p + 16);
        record.lore = U32(p + 20);
        record.levelMin = I32(p + 24);
        record.levelMax = I32(p + 28);
        record.flags = U32(p + 32);
        record.listStart = U32(p + 36);
        record.listCount = U32(p + 40);
        return record.listStart <= header_.entryCount && record.listCount <= header_.entryCount - record.listStart;
    }

    bool ReadListRecord(std::uint32_t index, ListRecord& record) const
    {
        if (index >= header_.entryCount)
            return false;
        const std::uint64_t offset = header_.entryOffset + static_cast<std::uint64_t>(index) * 8ull;
        if (!Range(offset, 8))
            return false;
        const unsigned char* p = &data_[static_cast<std::size_t>(offset)];
        record.kind = U32(p + 0);
        record.value = U32(p + 4);
        return (record.kind >= QUEST_GUIDE_PREREQUISITE && record.kind <= QUEST_GUIDE_RUNTIME_TAG) ||
               record.kind == MIX_GUIDE_INGREDIENT || record.kind == MIX_GUIDE_BONUS ||
               record.kind == MIX_GUIDE_RULE || record.kind == MIX_GUIDE_SHELTOM;
    }

    bool ReadMixRecord(std::uint32_t index, MixRecord& record) const
    {
        if (index >= header_.mixCount) return false;
        const std::uint64_t offset = header_.mixOffset + static_cast<std::uint64_t>(index) * 36ull;
        if (!Range(offset, 36)) return false;
        const unsigned char* p = &data_[static_cast<std::size_t>(offset)];
        record.recipeId = U32(p + 0); record.itemType = U32(p + 4); record.itemTypeLabel = U32(p + 8);
        record.sourceIngredients = U32(p + 12); record.sourceBonuses = U32(p + 16);
        record.totalSheltoms = U32(p + 20); record.minimumLevel = I32(p + 24);
        record.listStart = U32(p + 28); record.listCount = U32(p + 32);
        return record.listStart <= header_.entryCount && record.listCount <= header_.entryCount - record.listStart;
    }

    bool String(std::uint32_t offset, std::string& out) const
    {
        const std::size_t stringSize = data_.size() - header_.stringOffset;
        if (offset >= stringSize)
            return false;
        const std::size_t begin = header_.stringOffset + offset;
        const std::size_t end = data_.size();
        std::size_t pos = begin;
        while (pos < end && data_[pos])
            ++pos;
        if (pos >= end)
            return false;
        out.assign(reinterpret_cast<const char*>(&data_[begin]), pos - begin);
        return true;
    }

    static void AddString(std::vector<std::string>& out, const std::string& value)
    {
        if (!value.empty())
            out.push_back(value);
    }

    bool ReadQuest(std::uint32_t index, QuestGuideEntry& out) const
    {
        QuestRecord record;
        if (!ReadQuestRecord(index, record) ||
            !String(record.guideId, out.guideId) ||
            !String(record.name, out.name) ||
            !String(record.type, out.type) ||
            !String(record.category, out.category) ||
            !String(record.source, out.source) ||
            !String(record.lore, out.lore))
            return false;
        out.levelMin = record.levelMin;
        out.levelMax = record.levelMax;
        out.repeatable = (record.flags & 1u) != 0;
        out.supported = (record.flags & 2u) != 0;
        for (std::uint32_t i = 0; i < record.listCount; ++i)
        {
            ListRecord item;
            std::string value;
            if (!ReadListRecord(record.listStart + i, item) || !String(item.value, value))
                return false;
            switch (item.kind)
            {
                case QUEST_GUIDE_PREREQUISITE: AddString(out.prerequisites, value); break;
                case QUEST_GUIDE_CLASS: AddString(out.classes, value); break;
                case QUEST_GUIDE_FACTION: AddString(out.factions, value); break;
                case QUEST_GUIDE_STAGE: AddString(out.stages, value); break;
                case QUEST_GUIDE_OBJECTIVE: AddString(out.objectives, value); break;
                case QUEST_GUIDE_NPC: AddString(out.npcs, value); break;
                case QUEST_GUIDE_LOCATION: AddString(out.locations, value); break;
                case QUEST_GUIDE_QUEST_ITEM: AddString(out.questItems, value); break;
                case QUEST_GUIDE_REWARD: AddString(out.rewards, value); break;
                case QUEST_GUIDE_NOTE: AddString(out.notes, value); break;
                case QUEST_GUIDE_RUNTIME_ID:
                {
                    const long parsed = std::strtol(value.c_str(), nullptr, 10);
                    out.runtimeIds.push_back(static_cast<int>(parsed));
                    break;
                }
                case QUEST_GUIDE_RUNTIME_TAG: AddString(out.runtimeTags, value); break;
                default: return false;
            }
        }
        return true;
    }

    std::string path_;
    std::string error_;
    std::vector<unsigned char> data_;
    bool valid_;
};

}}}
