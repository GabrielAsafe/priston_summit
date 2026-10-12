#include "io/LegacyPath.h"
#include <cassert>
#include <cstring>
#include <string>
int main() {
    char out[512] = {};
    std::string longpath(200, 'a');
    longpath += ".ASE";
    assert(summit::io::ReplaceFileExtension(longpath.c_str(), "smd", out, sizeof(out)));
    assert(std::strlen(out) == 204);
    assert(out[200] == '.' && std::strcmp(out + 201, "smd") == 0);
    char small[12] = {'X'};
    assert(!summit::io::ReplaceFileExtension(longpath.c_str(), "smd", small, sizeof(small)));
    assert(small[0] == 0);
    return 0;
}
