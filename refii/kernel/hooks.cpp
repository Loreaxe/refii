#include <stdafx.h>
#include <kernel/kernel.h>
#include <kernel/function.h>

// Misc Kernel funcs
GUEST_FUNCTION_HOOK(sub_82CC2E48, refii::kernel::SleepEx);
GUEST_FUNCTION_HOOK(sub_822D6D20, refii::kernel::WaitForSingleObjectEx);
GUEST_FUNCTION_HOOK(sub_82CC2670, refii::kernel::XSetThreadProcessor);
GUEST_FUNCTION_HOOK(sub_82B3DAE8, refii::kernel::SetThreadName);
GUEST_FUNCTION_HOOK(sub_8221EA28, refii::kernel::QueryPerformanceCounter);
GUEST_FUNCTION_HOOK(sub_82CC6E78, refii::kernel::QueryPerformanceFrequency);
GUEST_FUNCTION_HOOK(sub_822660B0, refii::kernel::GetTickCount);
GUEST_FUNCTION_HOOK(sub_82CC2440, refii::kernel::GlobalMemoryStatus);

// Heap Functions
#ifdef REFII_HEAP_EMU
    GUEST_FUNCTION_HOOK(sub_822387B8, refii::kernel::RtlAllocateHeap);
    GUEST_FUNCTION_HOOK(sub_82239490, refii::kernel::RtlFreeHeap);
    GUEST_FUNCTION_HOOK(sub_82CC62E0, refii::kernel::RtlReAllocateHeap);
    GUEST_FUNCTION_HOOK(sub_82239808, refii::kernel::RtlSizeHeap);
    GUEST_FUNCTION_HOOK(sub_82CC5D38, refii::kernel::RtlCreateHeap);
    GUEST_FUNCTION_HOOK(sub_82CC4F98, refii::kernel::RtlDestroyHeap);
#else 
    GUEST_FUNCTION_HOOK(sub_822387B8, refii::kernel::RtlAllocateHeap);
    GUEST_FUNCTION_HOOK(sub_82239490, refii::kernel::RtlFreeHeap);
    GUEST_FUNCTION_HOOK(sub_82CC62E0, refii::kernel::RtlReAllocateHeap);
    GUEST_FUNCTION_HOOK(sub_82239808, refii::kernel::RtlSizeHeap);
    GUEST_FUNCTION_ALIAS_STUB(sub_82CC5D38, "RtlCreateHeap");
    GUEST_FUNCTION_ALIAS_STUB(sub_82CC4F98, "RtlDestroyHeap");
    GUEST_FUNCTION_ALIAS_STUB(sub_82CC6BC0, "HeapCreate");
    GUEST_FUNCTION_ALIAS_STUB(sub_82CC6C40, "HeapDestroy");
#endif


GUEST_FUNCTION_HOOK(sub_822386D0, refii::kernel::XAllocMem);
GUEST_FUNCTION_HOOK(sub_822398B0, refii::kernel::XFreeMem);
GUEST_FUNCTION_HOOK(sub_82CC74F8, refii::kernel::VirtualAlloc);
GUEST_FUNCTION_HOOK(sub_82CC7548, refii::kernel::VirtualFree);
// native memory operations
GUEST_FUNCTION_HOOK(sub_82CB0B30, memmove);
GUEST_FUNCTION_HOOK(sub_826BF770, memcpy);
GUEST_FUNCTION_HOOK(sub_826BFCF0, memset);
//GUEST_FUNCTION_HOOK(sub_82170010, OutputDebugStringA);

//
// // sprintf
// PPC_FUNC(sub_82BD4AE8)
//{
//    sub_831B1630(ctx, base);
//}
//

// todo(crack): move this some where more gooder
std::string wchar_to_cstr(wchar_t const* wcstr) {
    auto s = std::mbstate_t();
    auto const target_char_count = std::wcsrtombs(nullptr, &wcstr, 0, &s);
    if (target_char_count == static_cast<std::size_t>(-1)) {
        throw std::logic_error("Illegal byte sequence");
    }

    // +1 because std::string adds a null terminator which isn't part of size
    auto str = std::string(target_char_count, '\0');
    std::wcsrtombs(str.data(), &wcstr, str.size() + 1, &s);
    return str;
}

void GuestDbgPrint(char* msg)
{
    LOG_UTILITY(msg);
}
void GuestDbgPrintW(wchar_t* msg)
{
    LOG_UTILITY(wchar_to_cstr(msg));
}
// OutputDebugStringA --> GuestDbgPrint
GUEST_FUNCTION_HOOK(sub_82170010, GuestDbgPrint);
// OutputDebugStringW --> GuestDbgPrintW
GUEST_FUNCTION_HOOK(sub_82CC7FF0, GuestDbgPrintW);


// stubs

GUEST_FUNCTION_STUB(__imp__vsprintf);
GUEST_FUNCTION_STUB(__imp___vsnprintf);
GUEST_FUNCTION_STUB(__imp__sprintf);
GUEST_FUNCTION_STUB(__imp___snprintf);
GUEST_FUNCTION_STUB(__imp___snwprintf);
GUEST_FUNCTION_STUB(__imp__vswprintf);
GUEST_FUNCTION_STUB(__imp___vscwprintf);
GUEST_FUNCTION_STUB(__imp__swprintf);
// sub_82C87FF0 attempts to init sockets
GUEST_FUNCTION_ALIAS_STUB(sub_82C87FF0, "_unk_init_wsa")

// sub_82CFF790 is attempting to init xbl services
GUEST_FUNCTION_ALIAS_STUB(sub_822EAAE0, "init_xbl")

// XRNM is used for network stuff and is a pain in the ass
GUEST_FUNCTION_ALIAS_STUB(sub_8258C398, "XrnmThreadStartup");

void FixR11CaseIssue(PPCRegister& r11)
{
    // r11.u64 is the base of the guest memory (0x0000000100000000) when it should be 0
    if (r11.u64 == 0x0000000100000000)
    {
        r11.u64 = 0;
    }
}
