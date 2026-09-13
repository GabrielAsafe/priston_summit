#pragma once

#include <string>

class JoiResult
{
public:
    bool error() const { return false; }
    std::string description() const { return ""; }
};

class JoiString
{
public:
    static JoiString build() { return JoiString(); }

    JoiString& maximum(int) { return *this; }
    JoiString& length(int) { return *this; }
    JoiString& minimum(int) { return *this; }
    JoiString& required() { return *this; }
    JoiString& alphanum() { return *this; }
};

class JoiNumber
{
public:
    static JoiNumber build() { return JoiNumber(); }

    JoiNumber& maximum(int) { return *this; }
    JoiNumber& minimum(int) { return *this; }
};

namespace Joi
{
    template<typename A, typename B>
    int key(A const&, B const&)
    {
        return 0;
    }

    template<typename... Args>
    JoiResult validate(Args...)
    {
        return JoiResult();
    }
}
