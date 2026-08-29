#include <iostream>
#include <string>
#include <cctype>

static std::string LegacyGetFileExtension(const std::string &path)
{
    std::string::size_type slash;
    std::string::size_type backslash;
    std::string::size_type dot;

    slash     = path.rfind('/');
    backslash = path.rfind('//');
    dot       = path.rfind('.');

    if (dot != std::string::npos)
    {
        if (slash != std::string::npos && slash > dot)
            return "";

        if (backslash != std::string::npos && backslash > dot)
            return "";

        std::string ext = path.substr(dot + 1);

        for (char &c : ext)
            c = static_cast<char>(tolower(static_cast<unsigned char>(c)));

        return ext;
    }

    return "";
}

int main()
{
    const char *tests[] = {
        "C:/folder/file.txt",
        "C:\\folder\\file.txt",
        "C:/folder.ext/file",
        "C:\\folder.ext\\file",
        "folder/file.dds",
        "folder\\file.dds",
        "file.tga",
        "file"
    };

    for (const char *s : tests)
        std::cout << "[" << s << "] -> ["
                  << LegacyGetFileExtension(s) << "]\n";
}
