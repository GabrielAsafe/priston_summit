#pragma once
#include <string>

#ifndef RSPLAYINFO_PATCH_METHODS
#define RSPLAYINFO_PATCH_METHODS

#define RSPLAYINFO_METHODS_PATCH \
public: \
    std::string getAccount() const { return szID; } \
    std::string getName() const { return smCharInfo.szName; } \
    AccountTypes getSecurity() const { return security.sec; } \
    void setAccount(const std::string& v) { strcpy_s(szID, v.c_str()); } \
    void setName(const std::string& v) { strcpy_s(smCharInfo.szName, v.c_str()); }

#endif
