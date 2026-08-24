#include "MD5Wrapper.h"

#include <windows.h>
#include <wincrypt.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>

#pragma comment(lib, "Advapi32.lib")

static std::string BytesToHex(const BYTE * data, DWORD size)
{
    std::ostringstream oss;

    for (DWORD i = 0; i < size; ++i)
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)data[i];

    return oss.str();
}

static std::string HashBytes(const BYTE * data, DWORD size)
{
    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
    BYTE rgbHash[16] = { 0 };
    DWORD cbHash = 16;
    std::string result;

    if (!CryptAcquireContextA(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
        return "";

    if (!CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash))
    {
        CryptReleaseContext(hProv, 0);
        return "";
    }

    if (CryptHashData(hHash, data, size, 0) && CryptGetHashParam(hHash, HP_HASHVAL, rgbHash, &cbHash, 0))
        result = BytesToHex(rgbHash, cbHash);

    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);

    return result;
}

std::string md5wrapper::getHashFromString(const std::string & text)
{
    return HashBytes(reinterpret_cast<const BYTE *>(text.data()), (DWORD)text.size());
}

std::string md5wrapper::getHashFromFile(const std::string & filename)
{
    std::ifstream file(filename.c_str(), std::ios::binary);

    if (!file)
        return "";

    std::vector<char> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    if (data.empty())
        return HashBytes(reinterpret_cast<const BYTE *>(""), 0);

    return HashBytes(reinterpret_cast<const BYTE *>(&data[0]), (DWORD)data.size());
}
