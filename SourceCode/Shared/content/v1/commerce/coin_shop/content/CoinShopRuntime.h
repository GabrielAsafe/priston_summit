#pragma once

#include <string>
#include "content/v1/commerce/coin_shop/content/CompiledCoinShopSource.h"
#include "content/v1/runtime/RuntimeContentPath.h"

namespace summit { namespace content { namespace commerce { namespace coin_shop {

inline const char *CoinShopPackPath()
{
    static const std::string path = summit::content::runtime::ResolveRuntimePackPath("coin_shop.scsp");
    return path.c_str();
}

inline const CompiledCoinShopSource *ActiveCoinShopSource()
{
    struct Holder
    {
        std::string path;
        CompiledCoinShopSource *source;
        Holder() : source(0)
        {
            const char *configured = CoinShopPackPath();
            if(configured)
            {
                path = configured;
                source = new CompiledCoinShopSource(path.c_str());
            }
        }
        ~Holder() { delete source; }
    };
    static Holder holder;
    return holder.source;
}

inline bool CompiledCoinShopReady()
{
    const CompiledCoinShopSource *source = ActiveCoinShopSource();
    return source && source->IsValid();
}

}}}} // namespace summit::content::commerce::coin_shop
