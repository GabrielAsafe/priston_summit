#pragma once

#include <cstdio>
#include <string>
#include <utility>

namespace gameplay::community
{
class NewsContentService
{
public:
    static bool FormatName(std::string& output, const std::string& format, const char* name)
    {
        return FormatOne(output, format, name ? name : "");
    }

    static bool FormatNumber(std::string& output, const std::string& format, int value)
    {
        return FormatOne(output, format, value);
    }

    static bool FormatNotice(std::string& output,
                             const std::string& storedMessage,
                             int messageType,
                             int bossMinutes)
    {
        if (messageType == 1)
            return FormatOne(output, storedMessage, bossMinutes);
        output = storedMessage;
        return true;
    }

private:
    template <typename T>
    static bool FormatOne(std::string& output, const std::string& format, T value)
    {
        const int required = std::snprintf(nullptr, 0, format.c_str(), value);
        if (required < 0)
        {
            output.clear();
            return false;
        }
        std::string buffer(static_cast<std::size_t>(required) + 1u, '\0');
        const int written = std::snprintf(buffer.data(), buffer.size(), format.c_str(), value);
        if (written != required)
        {
            output.clear();
            return false;
        }
        buffer.resize(static_cast<std::size_t>(written));
        output = std::move(buffer);
        return true;
    }
};
}
