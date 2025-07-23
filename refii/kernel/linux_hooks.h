#pragma once

#ifdef __linux__

#include <cstdint>
#include <cstdio>
#include "kernel/kernel.h"        // for RtlInitializeCriticalSection
#include "kernel/platform_defs.h" // for XRTL_CRITICAL_SECTION, be<>

namespace refii::kernel {

// ———— List node for ExRegisterCallback ————
struct CallbackNode {
    uint32_t Flink;
    uint32_t Blink;
    uint32_t CallbackFunc;
};

// ———— Initialize the guest’s CRITICAL_SECTION at 0x832EFFE0 ————
inline void InitializeGlobalCriticalSections()
{
    constexpr uint32_t addr = 0x832EFFE0;
    auto* cs = reinterpret_cast<XRTL_CRITICAL_SECTION*>(g_memory.Translate(addr));

    if (cs->LockCount != -1 || cs->RecursionCount != 0) {
        std::fprintf(stderr,
            "[InitCS] Warning: CriticalSection at 0x%08X may already be initialized\n",
            addr);
    }
    RtlInitializeCriticalSection(cs);
}

// ———— Pre‑populate the two static arrays in the guest image ————
inline void InitializeCallbackArray()
{
    auto* table = reinterpret_cast<be<uint32_t>*>(g_memory.Translate(0x832D0000));
    table[0] = 0x82CA9DB8;
    table[1] = 0x82CB5E28;
    table[2] = 0;
}

inline void InitializeCallbackRdata()
{
    auto* rdata = reinterpret_cast<be<uint32_t>*>(g_memory.Translate(0x82010CD8));
    rdata[0] = 0x82CA9B30;
    rdata[1] = 0x82000000;
    rdata[2] = 0x82000000;
}

// ———— Set up an empty doubly‑linked callback list at 0x832EFFFC ————
inline void InitializeCallbackList()
{
    constexpr uint32_t headAddr = 0x832EFFFC;
    auto* head = reinterpret_cast<CallbackNode*>(g_memory.Translate(headAddr));
    head->Flink        = headAddr;
    head->Blink        = headAddr;
    head->CallbackFunc = 0;
}

inline void InitializeImportThunkGlobals() {
    // guest addresses
    constexpr uint32_t BLOCK_BASE    = 0x832EF310;
    constexpr uint32_t TABLE_OFFSET  = 0xC8;
    constexpr uint32_t THUNK_ADDRESS = 0x82011C08;

    // 1) at BLOCK_BASE, write BLOCK_BASE in big‑endian
    {
        auto* p = reinterpret_cast<uint32_t*>(
            g_memory.Translate(BLOCK_BASE));
        *p = __builtin_bswap32(BLOCK_BASE);
    }

    // 2) at BLOCK_BASE + TABLE_OFFSET, write THUNK_ADDRESS in big‑endian
    {
        auto* p = reinterpret_cast<uint32_t*>(
            g_memory.Translate(BLOCK_BASE + TABLE_OFFSET));
        *p = __builtin_bswap32(THUNK_ADDRESS);
    }
}

} // namespace refii::kernel

#endif // __linux__
