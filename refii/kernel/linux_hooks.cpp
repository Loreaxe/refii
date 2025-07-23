#include <stdafx.h>
#include <kernel/kernel.h>
#include <kernel/function.h>
#include <kernel/platform_defs.h>
#include <kernel/linux_hooks.h>

#ifdef __linux__

namespace refii::kernel {

void CallRegisteredCallbacks(uint32_t arg)
{
    constexpr uint32_t kCallbackCriticalSectionAddr = 0x832EFFE0;
    constexpr uint32_t kCallbackListHeadAddr        = 0x832EFFFC;

    auto* cs   = reinterpret_cast<XRTL_CRITICAL_SECTION*>(g_memory.Translate(kCallbackCriticalSectionAddr));
    auto* head = reinterpret_cast<CallbackNode*>(g_memory.Translate(kCallbackListHeadAddr));

    refii::kernel::RtlEnterCriticalSection(cs);

    uint32_t nodeAddr = head->Flink;
    while (nodeAddr && nodeAddr != kCallbackListHeadAddr)
    {
        auto* node = reinterpret_cast<CallbackNode*>(g_memory.Translate(nodeAddr));
        uint32_t callback = node->CallbackFunc;

        if (callback)
            GuestToHostFunction<void>(callback, arg);

        nodeAddr = node->Flink;
    }

    refii::kernel::RtlLeaveCriticalSection(cs);
}

void CallbackArrayInvoker() {
    constexpr uint32_t kCallbackStart = 0x832D0000;
    constexpr uint32_t kCallbackEnd   = 0x832D000C;

    for (uint32_t addr = kCallbackStart; addr < kCallbackEnd; addr += 4) {
        // Access using existing translation and endian-safe read
        const auto* ptr = reinterpret_cast<const be<uint32_t>*>(g_memory.Translate(addr));
        uint32_t func = *ptr; // automatically byteswapped by be<T>

        if (func == 0)
            continue;

        if (func >= 0x82000000 && func < 0x83000000 && (func % 4 == 0)) {
            GuestToHostFunction<void>(func);
        } else {
            std::fprintf(stderr,
                "[CallbackArrayInvoker] ⚠️ Invalid function 0x%08X at 0x%08X — skipped.\n",
                func, addr);
        }
    }
}

static uint32_t HostImportThunk(PPCContext* ctx, uint8_t* /*base*/)
{
    // 1. Load guest pointer from guest .data
    uint32_t base_addr = *reinterpret_cast<const be<uint32_t>*>(
        refii::kernel::g_memory.Translate(0x832EF3F0));
    printf("[HostImportThunk] base_addr (guest VA): %08X\n", base_addr);

    // 2. Translate base_addr to host pointer and read next guest pointer from it
    uint32_t table_base = *reinterpret_cast<const be<uint32_t>*>(
        refii::kernel::g_memory.Translate((base_addr & 0xFFFFFFFF) + 0xC8));
    printf("[HostImportThunk] table_base (guest VA): %08X\n", table_base);

    // 3. Calculate index as in PPC code
    uint32_t index = (ctx->r3.u32 & 0xFF) << 1;

    // 4. Translate table_base, then add index, then read entry
    uint16_t entry = *reinterpret_cast<const be<uint16_t>*>(
        refii::kernel::g_memory.Translate((table_base & 0xFFFFFFFF) + index));
    printf("[HostImportThunk] entry: %04X\n", entry);

    // 5. Mask upper bits
    ctx->r3.u32 = entry & 0xFFFF8000;

    return ctx->r3.u32;
}

void MainGameLoop() {
    printf("We made it!");
}

} // namespace refii::kernel

GUEST_FUNCTION_HOOK(sub_82CC7F18, refii::kernel::CallRegisteredCallbacks);
GUEST_FUNCTION_HOOK(sub_82CC82C8, refii::kernel::CallbackArrayInvoker);
GUEST_FUNCTION_HOOK(sub_822F2868, refii::kernel::CallbackArrayInvoker);
//GUEST_FUNCTION_HOOK(sub_82CAD400, refii::kernel::HostImportThunk);

#endif // __linux__

