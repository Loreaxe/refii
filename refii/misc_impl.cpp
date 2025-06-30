#include "stdafx.h"
#include <kernel/function.h>

uint32_t QueryPerformanceCounterImpl(LARGE_INTEGER* lpPerformanceCount)
{
    lpPerformanceCount->QuadPart = ByteSwap(std::chrono::steady_clock::now().time_since_epoch().count());
    return TRUE;
}

uint32_t QueryPerformanceFrequencyImpl(LARGE_INTEGER* lpFrequency)
{
    constexpr auto Frequency = std::chrono::steady_clock::period::den / std::chrono::steady_clock::period::num;
    lpFrequency->QuadPart = ByteSwap(Frequency);
    return TRUE;
}

uint32_t GetTickCountImpl()
{
    return uint32_t(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
}

void GlobalMemoryStatusImpl(XLPMEMORYSTATUS lpMemoryStatus)
{
    lpMemoryStatus->dwLength = sizeof(XMEMORYSTATUS);
    lpMemoryStatus->dwMemoryLoad = 0;
    lpMemoryStatus->dwTotalPhys = 0x20000000;
    lpMemoryStatus->dwAvailPhys = 0x20000000;
    lpMemoryStatus->dwTotalPageFile = 0x20000000;
    lpMemoryStatus->dwAvailPageFile = 0x20000000;
    lpMemoryStatus->dwTotalVirtual = 0x20000000;
    lpMemoryStatus->dwAvailVirtual = 0x20000000;
}

//GUEST_FUNCTION_HOOK(sub_82274140, GuestThread::SetThreadName);

//#ifdef _WIN32
GUEST_FUNCTION_HOOK(sub_82170010, OutputDebugStringA);

GUEST_FUNCTION_HOOK(sub_8221EA28, QueryPerformanceCounterImpl);
GUEST_FUNCTION_HOOK(sub_82CC6E78, QueryPerformanceFrequencyImpl);
GUEST_FUNCTION_HOOK(sub_822660B0, GetTickCountImpl);

GUEST_FUNCTION_HOOK(sub_82CC2440, GlobalMemoryStatusImpl);
//
//// sprintf
//PPC_FUNC(sub_82BD4AE8)
//{
//    sub_831B1630(ctx, base);
//}

void GameLogMsg(char* msg)
{
    LOG_UTILITY(msg);
}
GUEST_FUNCTION_HOOK(sub_82173070, GameLogMsg);


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
GUEST_FUNCTION_STUB(sub_83236B98); 
