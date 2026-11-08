#pragma once
#include <cstdint>

// Restricoes explicitamente verificadas para as quatro variantes de Dragao.
// Nao inferir genero de prefixos CA/DA nem renumerar codigos historicos.
namespace gameplay { namespace classes { namespace summit_costumes {
enum class RequiredGender { Any, Male, Female };
inline RequiredGender RequiredGenderForCode(std::uint32_t code) {
    switch (code) {
    case 0x02116800u: // CA871 7D masculino
    case 0x02116A00u: // CA873 30D masculino
        return RequiredGender::Male;
    case 0x02116900u: // CA872 7D feminino
    case 0x02116B00u: // CA874 30D feminino
        return RequiredGender::Female;
    default:
        return RequiredGender::Any;
    }
}
inline bool GenderAllowed(std::uint32_t code, bool knownGender, bool characterIsFemale) {
    const RequiredGender required = RequiredGenderForCode(code);
    if (required == RequiredGender::Any) return true;
    return knownGender && ((required == RequiredGender::Female) == characterIsFemale);
}
} } }
