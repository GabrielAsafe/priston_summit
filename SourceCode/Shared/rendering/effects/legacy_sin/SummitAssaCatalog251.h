#ifndef SUMMIT_ASSA_CATALOG_251_H
#define SUMMIT_ASSA_CATALOG_251_H
#include <cstddef>
#include <cstdio>
#include <cstring>
namespace summit { namespace effects {
template <std::size_t N>
inline bool JoinCatalogPath251(char (&out)[N], const char *a,
                               const char *b, const char *c, const char *d = "")
{
    if (!a || !b || !c || !d) return false;
    const int n = std::snprintf(out, N, "%s%s%s%s", a,b,c,d);
    if (n < 0 || static_cast<std::size_t>(n) >= N)
    { out[0] = 0; return false; }
    return true;
}
template <std::size_t N>
inline bool AppendCatalogPath251(char (&out)[N], const char *tail)
{
    if (!tail) return false;
    const std::size_t n = std::strlen(out);
    const std::size_t t = std::strlen(tail);
    if (n >= N || t >= N - n) return false;
    std::memcpy(out + n, tail, t + 1);
    return true;
}
} }
#endif
