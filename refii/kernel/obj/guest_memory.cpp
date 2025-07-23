#include <stdafx.h>
#include "guest_memory.h"
#include <cstdio>    // For perror
#include <cassert>   // For assert

using namespace refii::kernel;

// Patch: Always allocate 4 GB guest memory for compatibility!
GuestMemory::GuestMemory()
{
#ifdef _WIN32
    base = (uint8_t*)VirtualAlloc((void*)0x100000000ull, PPC_MEMORY_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    if (base == nullptr)
        base = (uint8_t*)VirtualAlloc(nullptr, PPC_MEMORY_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    if (base == nullptr)
    {
        fprintf(stderr, "FATAL: Could not allocate 4GB guest memory!\n");
        abort();
    }

    DWORD oldProtect;
    VirtualProtect(base, 4096, PAGE_NOACCESS, &oldProtect);

#else  // Linux
    base = (uint8_t*)mmap((void*)0x100000000ULL, PPC_MEMORY_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANON | MAP_PRIVATE | MAP_FIXED, -1, 0);
    if (base == MAP_FAILED) {
        base = (uint8_t*)mmap(nullptr, PPC_MEMORY_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANON | MAP_PRIVATE, -1, 0);
    }
    if (base == MAP_FAILED) {
        perror("mmap()");
        fprintf(stderr, "FATAL: Could not allocate 4GB guest memory!\n");
        abort();
    }
    // 4 KiB guard page
    if (mprotect(base, 4096, PROT_NONE) < 0) {
        perror("mprotect()");
        fprintf(stderr, "FATAL: Could not create guard page in guest memory!\n");
        abort();
    }
#endif

    // Register guest function mappings
    for (size_t i = 0; PPCFuncMappings[i].guest != 0; i++)
    {
        if (PPCFuncMappings[i].host != nullptr)
            InsertFunction(PPCFuncMappings[i].guest, PPCFuncMappings[i].host);
    }
}

