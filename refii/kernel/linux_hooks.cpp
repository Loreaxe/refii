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

} // namespace refii::kernel

GUEST_FUNCTION_HOOK(sub_82CC7F18, refii::kernel::CallRegisteredCallbacks);
GUEST_FUNCTION_HOOK(sub_82CC82C8, refii::kernel::CallbackArrayInvoker);

#endif // __linux__

