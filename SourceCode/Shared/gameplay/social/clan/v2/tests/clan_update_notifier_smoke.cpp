#include <cassert>
#include "gameplay/social/clan/v2/ClanUpdateNotifier.h"

namespace
{
class FakeNotifier final : public gameplay::social::clan::IClanUpdateNotifier
{
public:
    gameplay::social::clan::ClanUpdateNotifyResult Notify(
        gameplay::PlayerId source,
        const std::vector<gameplay::social::clan::ClanUpdateRecipient>& recipients) override
    {
        gameplay::social::clan::ClanUpdateNotifyResult result;
        if(!source.Valid()) return result;
        result.attempted = recipients.size();
        result.delivered = recipients.size();
        return result;
    }
};
}
int main()
{
    FakeNotifier notifier;
    std::vector<gameplay::social::clan::ClanUpdateRecipient> recipients;
    recipients.push_back(gameplay::social::clan::ClanUpdateRecipient("Alice", 42u));
    gameplay::social::clan::ClanUpdateNotifyResult result = notifier.Notify(gameplay::PlayerId(9u), recipients);
    assert(result.attempted == 1u && result.delivered == 1u);
    return 0;
}
