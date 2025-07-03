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
GUEST_FUNCTION_HOOK(sub_82170010, OutputDebugStringA);

//
// // sprintf
// PPC_FUNC(sub_82BD4AE8)
//{
//    sub_831B1630(ctx, base);
//}
//
//void GameLogMsg(char* msg)
//{
//    LOG_UTILITY(msg);
//}
//GUEST_FUNCTION_HOOK(sub_82173070, GameLogMsg);
//


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

// dobious stubs
/*
    Probably safe to stub this one for now. I am not sure exactly what it's purpose is but let's find out later.

    __imp___xstart
        __imp__sub_82CC81E8 - _cinit(int initFPGR = 1)
            __imp__sub_83246CC8 - `dynamic_initializer_for_'initlocks''(void)
              __imp__sub_822EC3E0 - unknown
                __imp__sub_8221BD38 - unknown
                  __imp__sub_83238460 - unknown
                    __imp__sub_83236D60 - unknown
                      __imp__sub_83236B98 - the bad guy. 1 xref from sub_83238460
                        __imp__sub_822D6C88 - Spin-lock using lwarx and stwcx to exchange an atomic variable. Works in every other observed case.
                            -- Exception reading memory address 0x24 (translated == 0x100000024)
*/
//GUEST_FUNCTION_ALIAS_STUB(sub_8221BD38, "_unk_init_locks?");

