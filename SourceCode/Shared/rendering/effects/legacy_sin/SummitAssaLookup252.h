#ifndef SUMMIT_ASSA_LOOKUP_252_H
#define SUMMIT_ASSA_LOOKUP_252_H
#include <cctype>
#include <string>
#include <unordered_map>
namespace summit { namespace effects {
class AssaLookup252 {
    std::unordered_map<std::string, int> first_;
    int count_;
    static std::string Key(const char *name) {
        std::string normalized;
        if (!name) return normalized;
        for (const unsigned char *p = reinterpret_cast<const unsigned char*>(name); *p; ++p)
            normalized.push_back(static_cast<char>(std::tolower(*p)));
        return normalized;
    }
public:
    AssaLookup252() : count_(-1) {}
    void Invalidate() { first_.clear(); count_ = -1; }
    int Count() const { return count_; }
    template<class Entry>
    void Rebuild(const Entry *entries, int count) {
        first_.clear(); count_ = -1;
        if (!entries || count < 0 || count > 1000) return;
        for (int i=0; i<count; ++i)
            if (entries[i].resName[0])
                first_.insert(std::make_pair(Key(entries[i].resName),i));
        count_ = count;
    }
    int Find(const char *name) const {
        if (!name || !*name) return -1;
        const std::unordered_map<std::string,int>::const_iterator it=first_.find(Key(name));
        return it==first_.end() ? -1 : it->second;
    }
};
} }
#endif
