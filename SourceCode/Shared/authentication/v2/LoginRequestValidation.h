#pragma once

#include <cstddef>

namespace authentication
{
namespace v2
{
enum class LoginRequestValidationCode
{
    Ok = 0,
    NullField,
    UnterminatedAccount,
    UnterminatedPassword,
    UnterminatedHash,
    UnterminatedMac,
    AccountTooLong,
    PasswordTooLong,
    HashRequired,
    HashLengthInvalid,
    MacRequired,
    MacTooLong
};

struct LoginRequestView
{
    const char* account;
    std::size_t accountCapacity;
    const char* password;
    std::size_t passwordCapacity;
    const char* hash;
    std::size_t hashCapacity;
    const char* mac;
    std::size_t macCapacity;

    LoginRequestView(const char* accountValue,
                     std::size_t accountCapacityValue,
                     const char* passwordValue,
                     std::size_t passwordCapacityValue,
                     const char* hashValue,
                     std::size_t hashCapacityValue,
                     const char* macValue,
                     std::size_t macCapacityValue)
        : account(accountValue), accountCapacity(accountCapacityValue),
          password(passwordValue), passwordCapacity(passwordCapacityValue),
          hash(hashValue), hashCapacity(hashCapacityValue),
          mac(macValue), macCapacity(macCapacityValue) {}
};

struct LoginRequestValidationResult
{
    LoginRequestValidationCode code;
    std::size_t observedLength;

    LoginRequestValidationResult(LoginRequestValidationCode codeValue = LoginRequestValidationCode::Ok,
                                 std::size_t observedLengthValue = 0u)
        : code(codeValue), observedLength(observedLengthValue) {}

    bool Accepted() const { return code == LoginRequestValidationCode::Ok; }

    const char* Description() const
    {
        switch(code)
        {
        case LoginRequestValidationCode::Ok: return "ok";
        case LoginRequestValidationCode::NullField: return "login.null_field";
        case LoginRequestValidationCode::UnterminatedAccount: return "login.account.unterminated";
        case LoginRequestValidationCode::UnterminatedPassword: return "login.password.unterminated";
        case LoginRequestValidationCode::UnterminatedHash: return "login.hash.unterminated";
        case LoginRequestValidationCode::UnterminatedMac: return "login.mac.unterminated";
        case LoginRequestValidationCode::AccountTooLong: return "login.account.maximum";
        case LoginRequestValidationCode::PasswordTooLong: return "login.password.maximum";
        case LoginRequestValidationCode::HashRequired: return "login.hash.required";
        case LoginRequestValidationCode::HashLengthInvalid: return "login.hash.length";
        case LoginRequestValidationCode::MacRequired: return "login.mac.required";
        case LoginRequestValidationCode::MacTooLong: return "login.mac.maximum";
        }
        return "login.validation.unknown";
    }
};

class LoginRequestValidationService final
{
public:
    LoginRequestValidationResult Validate(const LoginRequestView& request) const
    {
        if(!request.account || !request.password || !request.hash || !request.mac)
            return LoginRequestValidationResult(LoginRequestValidationCode::NullField);

        std::size_t accountLength = 0u;
        if(!BoundedLength(request.account, request.accountCapacity, accountLength))
            return LoginRequestValidationResult(LoginRequestValidationCode::UnterminatedAccount, request.accountCapacity);
        if(accountLength > 31u)
            return LoginRequestValidationResult(LoginRequestValidationCode::AccountTooLong, accountLength);

        std::size_t passwordLength = 0u;
        if(!BoundedLength(request.password, request.passwordCapacity, passwordLength))
            return LoginRequestValidationResult(LoginRequestValidationCode::UnterminatedPassword, request.passwordCapacity);
        if(passwordLength > 31u)
            return LoginRequestValidationResult(LoginRequestValidationCode::PasswordTooLong, passwordLength);

        std::size_t hashLength = 0u;
        if(!BoundedLength(request.hash, request.hashCapacity, hashLength))
            return LoginRequestValidationResult(LoginRequestValidationCode::UnterminatedHash, request.hashCapacity);
        if(hashLength == 0u)
            return LoginRequestValidationResult(LoginRequestValidationCode::HashRequired, hashLength);
        if(hashLength != 32u)
            return LoginRequestValidationResult(LoginRequestValidationCode::HashLengthInvalid, hashLength);

        std::size_t macLength = 0u;
        if(!BoundedLength(request.mac, request.macCapacity, macLength))
            return LoginRequestValidationResult(LoginRequestValidationCode::UnterminatedMac, request.macCapacity);
        if(macLength == 0u)
            return LoginRequestValidationResult(LoginRequestValidationCode::MacRequired, macLength);
        if(macLength > 29u)
            return LoginRequestValidationResult(LoginRequestValidationCode::MacTooLong, macLength);

        return LoginRequestValidationResult(LoginRequestValidationCode::Ok);
    }

private:
    static bool BoundedLength(const char* text, std::size_t capacity, std::size_t& outLength)
    {
        if(!text || capacity == 0u)
            return false;
        for(std::size_t index = 0u; index < capacity; ++index)
        {
            if(text[index] == '\0')
            {
                outLength = index;
                return true;
            }
        }
        outLength = capacity;
        return false;
    }
};
}
}
