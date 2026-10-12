#pragma once
#include <string>
#include <windows.h>
#include "gameplay/commerce/v2/PremiumPurchaseWire.h"
#include "content/v1/localization/content/LocalizationRuntime.h"

// Called only after a request-correlated GameServer response.
// cMESSAGEBOX accepts Windows-1252 char[64], not UTF-8 or arbitrary lengths.
inline void SummitPremiumPurchaseFeedback(int status)
{
    const char* key = "ui.premium.purchase.transaction";
    switch (status) {
    case gameplay::commerce::premium_wire::Purchased: key = "ui.premium.purchase.success"; break;
    case gameplay::commerce::premium_wire::InvalidRequest: key = "ui.premium.purchase.request"; break;
    case gameplay::commerce::premium_wire::ReplayedRequest: key = "ui.premium.purchase.replayed"; break;
    case gameplay::commerce::premium_wire::UnavailableProduct: key = "ui.premium.purchase.unavailable"; break;
    case gameplay::commerce::premium_wire::WalletUnavailable: key = "ui.premium.purchase.wallet"; break;
    case gameplay::commerce::premium_wire::InsufficientFunds: key = "ui.premium.purchase.funds"; break;
    case gameplay::commerce::premium_wire::DeliveryRejected: key = "ui.premium.purchase.delivery"; break;
    default: break;
    }
    std::string text;
    if (!summit::content::localization::GetLocalizationValue(key, text) || text.empty())
        text = key;
    const int wideCount = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
        text.data(), static_cast<int>(text.size()), nullptr, 0);
    if (wideCount > 0) {
        std::wstring wide(static_cast<std::size_t>(wideCount), L'\0');
        if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.data(),
            static_cast<int>(text.size()), &wide[0], wideCount)) {
            const int bytes = WideCharToMultiByte(1252, 0, wide.data(), wideCount, nullptr, 0, nullptr, nullptr);
            if (bytes > 0) {
                std::string legacy(static_cast<std::size_t>(bytes), '\0');
                if (WideCharToMultiByte(1252, 0, wide.data(), wideCount, &legacy[0], bytes, nullptr, nullptr))
                    text = legacy;
            }
        }
    }
    if (text.size() > 63) text.resize(63);
    cMessageBox.ShowMessage(text.c_str());
}
