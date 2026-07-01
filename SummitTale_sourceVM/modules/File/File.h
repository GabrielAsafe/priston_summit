#pragma once

#include <windows.h>
#include <string>
#include <exception>

class FileException : public std::exception
{
private:
    std::string message;

public:
    explicit FileException(const std::string & msg) : message(msg) {}

    const char * what() const throw()
    {
        return message.c_str();
    }
};

class File
{
private:
    std::string path;

public:
    explicit File(const std::string & filePath) : path(filePath) {}

    int readInt(const std::string & section, const std::string & key, int defaultValue = 0)
    {
        return GetPrivateProfileIntA(section.c_str(), key.c_str(), defaultValue, path.c_str());
    }

    std::string readString(const std::string & section, const std::string & key, const std::string & defaultValue = "")
    {
        char buffer[4096] = { 0 };
        GetPrivateProfileStringA(section.c_str(), key.c_str(), defaultValue.c_str(), buffer, sizeof(buffer), path.c_str());
        return std::string(buffer);
    }

    void write(const std::string & section, const std::string & key, int value)
    {
        char buffer[64] = { 0 };
        wsprintfA(buffer, "%d", value);

        if (!WritePrivateProfileStringA(section.c_str(), key.c_str(), buffer, path.c_str()))
            throw FileException("Failed to write integer to INI file: " + path);
    }

    void write(const std::string & section, const std::string & key, const std::string & value)
    {
        if (!WritePrivateProfileStringA(section.c_str(), key.c_str(), value.c_str(), path.c_str()))
            throw FileException("Failed to write string to INI file: " + path);
    }

    void write(const std::string & section, const std::string & key, const char * value)
    {
        write(section, key, std::string(value ? value : ""));
    }
};
