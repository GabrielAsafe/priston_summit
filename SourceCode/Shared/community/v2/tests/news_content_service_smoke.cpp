#include "Shared/community/v2/NewsContentService.h"
#include <cassert>
#include <iostream>

int main()
{
    using gameplay::community::NewsContentService;
    std::string out;
    assert(NewsContentService::FormatName(out, "Welcome %s", "Hero"));
    assert(out == "Welcome Hero");
    assert(NewsContentService::FormatNumber(out, "Coins: %d", 42));
    assert(out == "Coins: 42");
    assert(NewsContentService::FormatNotice(out, "Boss in %d minutes", 1, 15));
    assert(out == "Boss in 15 minutes");
    assert(NewsContentService::FormatNotice(out, "Server maintenance", 0, 15));
    assert(out == "Server maintenance");
    std::cout << "news content service smoke: PASS\n";
    return 0;
}
