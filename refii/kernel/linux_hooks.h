#pragma once

#ifdef __linux__

#include <cstdint>
#include <cstdio>
#include "kernel/kernel.h"    // for RtlInitializeCriticalSection
#include "kernel/platform_defs.h"

namespace refii::kernel {

inline void InitializeGlobalCriticalSections()
{
    constexpr uint32_t kCallbackCriticalSectionAddr = 0x832EFFE0;

    auto* cs = reinterpret_cast<XRTL_CRITICAL_SECTION*>(g_memory.Translate(kCallbackCriticalSectionAddr));

    if (cs->LockCount != -1 || cs->RecursionCount != 0)
    {
        std::fprintf(stderr, "[InitCS] Warning: CriticalSection at 0x%08X may already be initialized\n", kCallbackCriticalSectionAddr);
    }

    RtlInitializeCriticalSection(cs);
}

inline void InitializeCallbackArray() {
    auto* table = reinterpret_cast<be<uint32_t>*>(g_memory.Translate(0x832D0000));
    table[0] = 0x82CA9DB8;  // Valid guest function
    table[1] = 0x82CB5E28;  // Another valid guest function
    table[2] = 0;           // Null terminator
}

inline void InitializeCallbackRdata() {
    auto* rdata = reinterpret_cast<be<uint32_t>*>(g_memory.Translate(0x82010CD8));
    rdata[0] = 0x82CA9B30;  // Pointer to sub_82CA9B30
    rdata[1] = 0x82000000;  // nullsub_1
    rdata[2] = 0x82000000;  // nullsub_1
}

struct CallbackNode {
    uint32_t Flink;
    uint32_t Blink;
    uint32_t CallbackFunc;
};

} // namespace refii::kernel

#endif // __linux__
