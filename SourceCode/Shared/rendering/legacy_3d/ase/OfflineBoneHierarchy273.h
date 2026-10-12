#pragma once
// Strictly offline ASE-bone topology ordering. No changes to normal game model loading.
// Mirrors the legacy first-matching-parent name resolution, but never writes TmSort
// until all nodes have been proven acyclic and within its fixed 128-slot capacity.
#include <cctype>
#include <cstring>

namespace summit_ase_bone273 {

inline bool EqualCaseInsensitive(const char* lhs, const char* rhs)
{
    if (!lhs || !rhs) return false;
    for (;;) {
        const unsigned char a = static_cast<unsigned char>(*lhs++);
        const unsigned char b = static_cast<unsigned char>(*rhs++);
        if (std::tolower(a) != std::tolower(b)) return false;
        if (!a) return true;
    }
}

template<typename Pattern>
bool LinkAndSort(Pattern* pattern)
{
    if (!pattern || pattern->nObj3d < 0 || pattern->nObj3d > 128) return false;
    const int count = pattern->nObj3d;
    int parent[128];
    int order[128];
    int used = 0;
    for (int i = 0; i < count; ++i) {
        const auto* object = pattern->obj3d[i];
        if (!object || !std::memchr(object->NodeName, '\0', sizeof(object->NodeName)) ||
            !std::memchr(object->NodeParent, '\0', sizeof(object->NodeParent))) return false;
        parent[i] = -1;
        if (object->NodeParent[0]) {
            for (int j = 0; j < count; ++j) {
                if (!pattern->obj3d[j] ||
                    !std::memchr(pattern->obj3d[j]->NodeName, '\0', sizeof(pattern->obj3d[j]->NodeName))) return false;
                if (EqualCaseInsensitive(object->NodeParent, pattern->obj3d[j]->NodeName)) {
                    parent[i] = j;
                    break; // Same first-match semantics as legacy LinkObject().
                }
            }
        }
        if (parent[i] == i) return false;
        if (parent[i] < 0) order[used++] = i; // Missing parents are roots in legacy.
    }
    // A bounded breadth-first expansion. Each node has at most one parent and
    // enters the output once. Cycles leave nodes unreachable => fail closed.
    for (int k = 0; k < used; ++k) {
        for (int i = 0; i < count; ++i) {
            if (parent[i] == order[k]) {
                if (used >= 128) return false;
                order[used++] = i;
            }
        }
    }
    if (used != count) return false;
    for (int i = 0; i < count; ++i)
        pattern->obj3d[i]->pParent = parent[i] < 0 ? nullptr : pattern->obj3d[parent[i]];
    for (int i = 0; i < count; ++i) pattern->TmSort[i] = order[i];
    return true;
}
} // namespace summit_ase_bone273
