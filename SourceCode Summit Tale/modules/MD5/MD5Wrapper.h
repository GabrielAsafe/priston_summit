#pragma once

#include <string>

class md5wrapper
{
public:
    md5wrapper() {}
    ~md5wrapper() {}

    std::string getHashFromString(const std::string & text);
    std::string getHashFromFile(const std::string & filename);
};


