#pragma once

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include "content/v1/common/Sha256.h"

namespace summit { namespace content { namespace commerce { namespace coin_shop {

enum CoinShopKind
{
    COIN_SHOP_KIND_COIN = 1,
    COIN_SHOP_KIND_TIME = 2
};

struct CoinShopCatalogEntry
{
    std::string code;
    int price;
    int section;
    int kind;
};

class CompiledCoinShopSource
{
public:
    explicit CompiledCoinShopSource(const char *path) : valid_(false)
    {
        if(path) path_ = path;
        LoadPack();
    }

    bool IsValid() const { return valid_; }
    const std::string &LastError() const { return error_; }

    bool Entries(int kind, std::vector<CoinShopCatalogEntry> &out) const
    {
        if(!valid_ || (kind != COIN_SHOP_KIND_COIN && kind != COIN_SHOP_KIND_TIME))
            return false;
        out.clear();
        for(std::uint32_t i = 0; i < entryCount_; ++i)
        {
            CoinShopCatalogEntry entry;
            if(!ReadEntry(i, entry)) return false;
            if(entry.kind == kind) out.push_back(entry);
        }
        return true;
    }

    bool Find(int kind, const char *code, CoinShopCatalogEntry &out) const
    {
        if(!valid_ || !code) return false;
        const std::string wanted = Fold(code);
        for(std::uint32_t i = 0; i < entryCount_; ++i)
        {
            CoinShopCatalogEntry entry;
            if(!ReadEntry(i, entry)) return false;
            if(entry.kind == kind && Fold(entry.code.c_str()) == wanted)
            {
                out = entry;
                return true;
            }
        }
        return false;
    }

private:
    static std::uint16_t U16(const unsigned char *p)
    {
        return static_cast<std::uint16_t>(p[0]) |
            (static_cast<std::uint16_t>(p[1]) << 8);
    }

    static std::uint32_t U32(const unsigned char *p)
    {
        return static_cast<std::uint32_t>(p[0]) |
            (static_cast<std::uint32_t>(p[1]) << 8) |
            (static_cast<std::uint32_t>(p[2]) << 16) |
            (static_cast<std::uint32_t>(p[3]) << 24);
    }

    static std::string Fold(const char *value)
    {
        std::string folded = value ? value : "";
        for(std::size_t i = 0; i < folded.size(); ++i)
            if(folded[i] >= 'A' && folded[i] <= 'Z')
                folded[i] = static_cast<char>(folded[i] - 'A' + 'a');
        return folded;
    }

    bool Range(std::uint64_t offset, std::uint64_t size) const
    {
        return offset <= data_.size() && size <= data_.size() - static_cast<std::size_t>(offset);
    }

    bool LoadPack()
    {
        if(path_.empty())
        {
            error_ = "coin shop pack path is empty";
            return false;
        }
        FILE *file = std::fopen(path_.c_str(), "rb");
        if(!file)
        {
            error_ = "coin shop pack not found";
            return false;
        }
        std::fseek(file, 0, SEEK_END);
        const long fileSize = std::ftell(file);
        if(fileSize < 64 || std::fseek(file, 0, SEEK_SET) != 0)
        {
            std::fclose(file);
            error_ = "coin shop pack too small";
            return false;
        }
        data_.resize(static_cast<std::size_t>(fileSize));
        if(std::fread(&data_[0], 1, data_.size(), file) != data_.size())
        {
            std::fclose(file);
            error_ = "cannot read coin shop pack";
            return false;
        }
        std::fclose(file);

        const unsigned char *p = &data_[0];
        if(std::memcmp(p, "SCSP", 4) != 0)
        {
            error_ = "coin shop magic mismatch";
            return false;
        }
        const std::uint16_t version = U16(p + 4);
        const std::uint16_t flags = U16(p + 6);
        entryCount_ = U32(p + 8);
        entryOffset_ = U32(p + 12);
        entrySize_ = U32(p + 16);
        const std::uint32_t reserved0 = U32(p + 20);
        const std::uint32_t reserved1 = U32(p + 24);
        const std::uint32_t reserved2 = U32(p + 28);
        if(version != 1 || flags != 0 || entryOffset_ != 64 || entrySize_ != 44 ||
           reserved0 || reserved1 || reserved2 ||
           entryOffset_ + static_cast<std::uint64_t>(entryCount_) * entrySize_ != data_.size())
        {
            error_ = "coin shop layout is non-canonical";
            return false;
        }
        unsigned char digest[32];
        summit::content::common::Sha256(&data_[64], data_.size() - 64, digest);
        if(std::memcmp(digest, p + 32, 32) != 0)
        {
            error_ = "coin shop SHA-256 mismatch";
            return false;
        }
        valid_ = true;
        return true;
    }

    bool ReadEntry(std::uint32_t index, CoinShopCatalogEntry &out) const
    {
        if(index >= entryCount_) return false;
        const std::uint64_t offset = entryOffset_ + static_cast<std::uint64_t>(index) * entrySize_;
        if(!Range(offset, entrySize_)) return false;
        const unsigned char *p = &data_[static_cast<std::size_t>(offset)];
        std::size_t codeLength = 0;
        while(codeLength < 32 && p[codeLength] != 0) ++codeLength;
        if(codeLength == 0 || codeLength >= 32) return false;
        out.code.assign(reinterpret_cast<const char *>(p), codeLength);
        out.price = static_cast<int>(U32(p + 32));
        out.section = static_cast<int>(U16(p + 36));
        out.kind = static_cast<int>(p[38]);
        const unsigned int flags = p[39];
        const std::uint32_t reserved = U32(p + 40);
        if(out.price <= 0 || out.section < 1 || out.section > 7 ||
           (out.kind != COIN_SHOP_KIND_COIN && out.kind != COIN_SHOP_KIND_TIME) || flags || reserved)
            return false;
        return true;
    }

    std::string path_;
    std::string error_;
    std::vector<unsigned char> data_;
    bool valid_;
    std::uint32_t entryCount_ = 0;
    std::uint32_t entryOffset_ = 0;
    std::uint32_t entrySize_ = 0;
};

}}}} // namespace summit::content::commerce::coin_shop
