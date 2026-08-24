#include "stdafx.h"

#include <cstdint>
#include <cstdlib>
#include <new>

static DWORD dwAllocs = 0;
static DWORD dwPAllocs = 0;

DWORD GetMajorAllocator()
{
    return dwAllocs;
}

DWORD GetMajorAllocatorP()
{
    return dwPAllocs;
}

static void RegisterAllocation(std::size_t size)
{
    if (size > dwAllocs)
    {
        dwAllocs = static_cast<DWORD>(size);

        dwPAllocs = static_cast<DWORD>(
            reinterpret_cast<std::uintptr_t>(
                __builtin_return_address(0)
            )
        );
    }
}

void* operator new(std::size_t size)
{
    RegisterAllocation(size);

    if (void* memory = std::malloc(size))
        return memory;

    throw std::bad_alloc();
}

void* operator new[](std::size_t size)
{
    RegisterAllocation(size);

    if (void* memory = std::malloc(size))
        return memory;

    throw std::bad_alloc();
}

void operator delete(void* memory) noexcept
{
    std::free(memory);
}

void operator delete[](void* memory) noexcept
{
    std::free(memory);
}

/*
 * GCC em C++14/C++17 pode emitir chamadas aos operadores delete
 * dimensionados.
 */
void operator delete(void* memory, std::size_t) noexcept
{
    std::free(memory);
}

void operator delete[](void* memory, std::size_t) noexcept
{
    std::free(memory);
}
